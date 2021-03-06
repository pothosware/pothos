///
/// \file Framework/ThreadPool.hpp
///
/// Support for configuring and managing threading in the framework.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
///                    2019 Nicholas Corgan
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>

namespace Pothos {

/*!
 * Arguments used to configure a ThreadPool.
 */
class POTHOS_API ThreadPoolArgs
{
public:

    //! Create a default ThreadPoolArgs
    ThreadPoolArgs(void);

    //! Create a ThreadPoolArgs given a specific number of threads
    ThreadPoolArgs(const size_t numThreads);

    /*!
     * Create a ThreadPoolArgs from a JSON description.
     * All fields are optional and have defined defaults.
     *
     * Example JSON markup for a ThreadPoolArgs description:
     * \code {.json}
     * {
     *     "numThreads" : 2,
     *     "priority" : 0.5,
     *     "affinityMode" : "CPU",
     *     "affinity" : [0, 2, 4, 6],
     *     "yieldMode" : "SPIN"
     * }
     * \endcode
     * \param json a JSON object markup string
     */
    ThreadPoolArgs(const std::string &json);

    /*!
     * The number of threads to create in this pool.
     * The default value is 0, indicating the thread-per-block mechanic.
     * Positive values for numThreads indicate the thread-pool mechanic.
     */
    size_t numThreads;

    /*!
     * Scheduling priority for all threads in the pool.
     * The value can be in range -1.0 to 1.0.
     * A value of 0.0 is the default thread scheduling.
     * Positive values enable realtime scheduling mode.
     * Negative values enable sub-priority scheduling.
     *
     * The default is 0.0 (normal).
     */
    double priority;

    /*!
     * The affinity mode for this thread pool.
     * The affinityMode string can have the following values:
     *
     *  - "ALL" - affinitize to all available CPUs
     *  - "CPU" - affinity list specifies CPUs
     *  - "NUMA" - affinity list specifies NUMA nodes
     *
     * The default is "ALL".
     */
    std::string affinityMode;

    //! A list of CPUs or NUMA nodes (depends on mode setting)
    std::vector<size_t> affinity;

    /*!
     * The yieldMode specifies the internal threading mechanisms:
     * 
     *  - "CONDITION" - Threads wait on condition variables when no work is available.
     *  - "HYBRID" - Threads spin for a while, then yield to other threads, when no work is available.
     *  - "SPIN" - Threads busy-wait, without yielding, when no work is available.
     *
     * The default is "CONDITION".
     */
    std::string yieldMode;
};

/*!
 * A ThreadPool manages a group of threads that perform work in a Topology.
 * Not only can users configure the number of threads,
 * but there are a variety of other settings such as affinity,
 * real-time priority, and internal threading mechanisms.
 *
 * The thread pool can operate in two major modes:
 *
 *  - When numThreads is specified to be zero,
 *    the thread pool is in thread-per-block mode.
 *    Each block that is associated with this thread pool gets its own
 *    dedicated thread spawned explicitly for its execution alone.
 *
 *  - Positive values for numThreads indicate pool-mode where a
 *    fixed number of threads operate on the blocks in a round-robin fashion.
 *    The thread pool will never spawn more threads than there are blocks.
 */
class POTHOS_API ThreadPool
{
public:

    //! Create a null ThreadPool
    ThreadPool(void);

    //! Create a ThreadPool from an opaque shared_ptr
    ThreadPool(const std::shared_ptr<void> &);

    /*!
     * Create a new ThreadPool from args.
     * \param args the configuration struct
     * \throws ThreadPoolError on bad values
     */
    ThreadPool(const ThreadPoolArgs &args);

    /*!
     * Is this thread pool valid/non-empty?
     * \return true when the thread poll is non-null
     */
    explicit operator bool(void) const;

    /*!
     * Get access to the underlying container for the thread pool.
     * \return an opaque shared_ptr to the internal object
     */
    const std::shared_ptr<void> &getContainer(void) const;

private:
    std::shared_ptr<void> _impl;
};

//! Are these two thread pools the same?
POTHOS_API bool operator==(const ThreadPool &lhs, const ThreadPool &rhs);

//! Are these two thread pool different?
inline bool operator!=(const ThreadPool &lhs, const ThreadPool &rhs)
{
    return !(lhs == rhs);
}

} //namespace Pothos
