/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 * @author Tomas Alvarez
 * @author Amnol Thakkar
 * @date Dec 1 2025
 * @brief updated wait queue manager and External Priority scheduler
 * 
 */

#include"interrupts_101310738_101304659.hpp"

void FCFS(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.arrival_time > second.arrival_time); 
                } 
            );
}

//sorts the ready queue based on priority 
void EP(std::vector<PCB> &ready_queue) {
    std::sort(ready_queue.begin(), ready_queue.end(),
              [](const PCB &a, const PCB &b){
                  return (a.PID > b.PID);
                }
            );
}

std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    PCB running;

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {//check if the AT = current time
                //if so, assign memory and put the process into the ready queue
                assign_memory(process);

                process.state = READY;  //Set the process state to READY
                ready_queue.push_back(process); //Add the process to the ready queue
                job_list.push_back(process); //Add it to the list of processes

                execution_status += print_exec_status(current_time, process.PID, NEW, READY);
            }
        }

        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        //This mainly involves keeping track of how long a process must remain in the ready queue
        
        //go through all processes currently doing I/O
        
        for (auto iter = wait_queue.begin(); iter != wait_queue.end();){
            PCB &process = *iter;
            //decrementing the I/O if any remains
            if (process.io_time_remaining > 0){
                process.io_time_remaining --;
            }
            //when I/O finishes, moce the associated process back into the READY QUEUE
            if (process.io_time_remaining == 0) {
                states old_state = process.state;
                process.state = READY;

                //reset the CPUburst I/O counter for the next I/O call and scyn with job_list
                process.time_since_last_io = 0;
                sync_queue(job_list, process);
                //loging the state
                execution_status += print_exec_status(current_time, process.PID, old_state, process.state);

                //push the process back into the ready queue
                ready_queue.push_back(process);

                //Remove it from wait_queue and advance iter safely
                iter = wait_queue.erase(iter);
            } else {
                ++iter;
            }

        }
        /////////////////////////////////////////////////////////////////

        //////////////////////////SCHEDULER//////////////////////////////
        //FCFS(ready_queue); //example of FCFS is shown here
        if (running.state == NOT_ASSIGNED && !ready_queue.empty()){
            //picks the first process at the back of the ready queue
            EP(ready_queue);
            run_process(running, job_list, ready_queue, current_time);
            execution_status += print_exec_status(current_time, running.PID, READY, running.state);

        }
        // executing 1ms of CPUburst when running
        if(running.state == RUNNING){
            running.remaining_time--;
            running.time_since_last_io++;
            
            //if the Process is completed
            if(running.remaining_time == 0){
                states old_state = running.state;
                running.state = TERMINATED;
                execution_status += print_exec_status(current_time, running.PID, old_state, running.state);
                terminate_process(running, job_list);
                //freeing CPU for next process
                idle_CPU(running);
            }

            //check if I/O frequency has been reached
            else if (running.io_freq > 0 && running.time_since_last_io >= running.io_freq) {
                states old_state = running.state;
                running.state = WAITING;
                running.time_since_last_io = 0;
                running.io_time_remaining = running.io_duration;
                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, old_state, running.state);
                wait_queue.push_back(running);
                idle_CPU(running);
            } 
        }

        current_time++;
        /////////////////////////////////////////////////////////////////

    }
    
    //Close the output table
    execution_status += print_exec_footer();

    return std::make_tuple(execution_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec] = run_simulation(list_process);

    write_output(exec, "execution.txt");

    return 0;
}