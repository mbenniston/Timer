#pragma once
#include <chrono>
#include <cmath>
#include <functional>

using Clock = std::chrono::high_resolution_clock;

class Timer
{
protected:
    Clock::time_point startTime, endTime;
public:

    //returns the number of seconds passed since the timer was stopped
    inline double getTimePassed() const noexcept {
        return (endTime - startTime).count() * pow(10, -9);
    }

    //starts the clock
    inline void start() noexcept  {
        startTime = Clock::now();
    }

    //stops the clock
    inline void stop() noexcept {
        endTime = Clock::now();
    }
};

//represents an event that happens at a regular interval and can repeat
class TimedEvent : private Timer
{
private:
    bool handled;
    bool repeated; //determines whether the event should be handled multiple timer
    double waitTime; //time that should be waited before handling the event
public:

    //returns the amount of seconds have passed since the event should have been handled
    inline double getLateness() const noexcept  {
        return ((Clock::now() - startTime).count() * pow(10, -9)) - waitTime;
    }

    //returns whether the event is due
    inline bool isDue() const noexcept {
        return getLateness() >= 0;
    }

    //returns if the event has been handled
    inline bool isHandled() const noexcept {
        return handled;
    }

    //returns whether the event should be handled
    inline bool shouldHandle() const noexcept {
        return !handled && isDue();
    }

    /*
        Used to handle the event
        Usage: 
        if(event.handle()) {
            //do stuff
        }
    */
    inline virtual bool handle() noexcept {
        if(shouldHandle()) {

            if(repeated) {
                start(); //restart the timer      
                //a repeated event is never handled 
            } else {
                handled = true;
            }

            return true;
        } else {
            return false;
        }
    }

    TimedEvent(bool rep, double wt) : repeated(rep), waitTime(wt), handled(false) { 
        start();
    }
};

//represents a timed event that executes a given function when handled
class JobEvent : public TimedEvent
{
public:
    using Callback = std::function<void()>;

private:
    Callback job;

public:

    //job getter
    Callback getJob() {
        return job;
    }

    //job setter
    void setJob(Callback cb) {
        job = cb;
    }

    inline virtual bool handle() noexcept override {
        bool result = TimedEvent::handle();
        
        //execute the job
        if(result){
            job();
        }

        return result;
    }

    JobEvent(Callback cb, bool rep, double wt) : TimedEvent(rep, wt), job(cb) {
    }
};

//represents an job event with a priority so it can be compared against other events
class PriorityEvent : public JobEvent {
private:
    int priority;
public:

    //priority getter
    inline int getPriority() const noexcept {
        return priority; 
    }

    //priority setter
    inline void setPriority(int p) noexcept {
        priority = p;
    }

    //returns whether this event has priority over another event
    inline bool hasPriority(const PriorityEvent& event) const noexcept {
        return priority > event.getPriority();
    }

    //returns whether this event has priority over another event
    inline bool operator>(const PriorityEvent& event) const noexcept {
        return hasPriority(event);
    }

    PriorityEvent(Callback cb, int p, bool rep, double wt) : JobEvent(cb, rep, wt), priority(p) {
    }
};
