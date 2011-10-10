#ifndef TASK_HH
#define TASK_HH

#include <functional>
#include <mutex>
#include <deque>

//! user must define
extern size_t default_stacksize;

namespace fw {

struct task_interrupted : std::exception {};

#define SEC2MS(s) (s*1000)

struct task;
struct proc;
typedef std::deque<task *> tasklist;
typedef std::deque<proc *> proclist;

uint64_t taskspawn(const std::function<void ()> &f, size_t stacksize=default_stacksize);
int64_t taskyield();
//static void taskexit(void *r = 0);
bool taskcancel(uint64_t id);
void tasksystem();
void tasksetstate(const char *fmt, ...);
const char *taskgetstate();
void tasksetname(const char *fmt, ...);
const char *taskgetname();

uint64_t procspawn(const std::function<void ()> &f, size_t stacksize=default_stacksize);

struct qutex {
    std::timed_mutex m;
    task *owner;
    tasklist waiting;

    qutex() {
        std::unique_lock<std::timed_mutex> lk(m);
        owner = 0;
    }
    void lock();
    void unlock();
    bool try_lock();
    template<typename Rep,typename Period>
        bool try_lock_for(
                std::chrono::duration<Rep,Period> const&
                relative_time);
    template<typename Clock,typename Duration>
        bool try_lock_until(
                std::chrono::time_point<Clock,Duration> const&
                absolute_time);
};

struct rendez {
    qutex q;
    tasklist waiting;

    void sleep(std::unique_lock<qutex> &lk);

    template <typename Predicate>
    void sleep(std::unique_lock<qutex> &lk, Predicate pred) {
        while (!pred()) {
            sleep(lk);
        }
    }

    bool sleep_for(std::unique_lock<qutex> &lk, unsigned int ms);

    void wakeup();
    void wakeupall();
};

struct procmain {
    procmain();

    int main(int argc=0, char *argv[]=0);
};

void tasksleep(uint64_t ms);
bool fdwait(int fd, int rw, uint64_t ms=0);

} // end namespace fw

#endif // TASK_HH

