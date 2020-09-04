#include <iostream>
#include <random>
#include <ctime>
#include <thread>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // Done: FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this]{return !_queue.empty(); });

    // do first-in, first-out
    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // Done: FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // perform vector modification under the lock
    std::lock_guard<std::mutex> myLock(_mutex);

    // flush queue of old messages
    // This is a work around for this project. If the queue isn't flushed, when cars arrive the first message
    // in the deque can often be "stale." A better implementation would be to expose a "flush" method in the api
    // and allow the client to specify if it should be used. I would probably implement a "flush" in the function
    // TrafficLight::waitForGreen().
    _queue.clear();

    // add item to queue
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    int init = std::rand() % 2;
    
    if (init == 0){
        _currentPhase = TrafficLightPhase::red;
    } else {
        _currentPhase = TrafficLightPhase::green;
    }
}

void TrafficLight::waitForGreen()
{
    // Done: FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true){
        TrafficLightPhase phase = _queue.receive();
        if (phase == TrafficLightPhase::green) break;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // Done: FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // Done: FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    clock_t begin_time = clock();
    clock_t wait_time = _minWait + ( std::rand() % ( _maxWait - _minWait + 1 ) );

    while(true){
        if ((float( clock () - begin_time ) /  CLOCKS_PER_SEC) >= wait_time){
            begin_time = clock();
            wait_time = _minWait + ( std::rand() % ( _maxWait - _minWait + 1 ) );

            if(_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red;
            } else {
                _currentPhase = TrafficLightPhase::green;
            }
            // TODO: add update method using message queue
            TrafficLightPhase phase = _currentPhase;
            _queue.send(std::move(phase));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } 
}
