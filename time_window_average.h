#ifndef TIME_WINDOW_AVERAGE_H_
#define TIME_WINDOW_AVERAGE_H_

#include <stdint.h>
#include <mutex>
#include <queue>
#include <chrono>

/* ! Thread SAFE ! */
template<typename T, T GET_VALUE_IF_EMPTY = T()>
class TimeWindowAverage
{
    const int64_t						timeWindowMS;
    mutable std::mutex					mutex;
    T 									sum;
    std::queue<std::pair<int64_t,T> > 	queue;
public:
    /**
    * Construct a TimeWindowAverage object
    * @param time_window_in_ms - The total number of values counted into average is determined by this time interval.
    */
    TimeWindowAverage(int64_t time_window_in_ms) : timeWindowMS(time_window_in_ms), mutex(), sum(static_cast<T>(0)), queue() 
    {}
    /**
    * Updates the moving average by the given value and drops out other values
    * that are out of the time window interval.
    */
    void update(const T& value)
    {
        std::lock_guard<std::mutex> lock(mutex);
        const int64_t now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
                          .time_since_epoch().count();
        queue.emplace(now, value);
        sum += value;

        if( !queue.empty() )
        {
            auto oldest = queue.front();
            while((now - oldest.first) > timeWindowMS)
            {
                sum -= oldest.second;
                queue.pop();//removes the FIRST element
                oldest = queue.front();
            }
        }
    }

    /**
    * Calculates and returns the actual average value.
    */
    T get() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        //since C++11, the complexity of std::queue::size is constant: O(1)
        return ( queue.empty() ? GET_VALUE_IF_EMPTY : (sum / static_cast<T>(queue.size())) );
    }
};

#endif /*TIME_WINDOW_AVERAGE_H_ */
