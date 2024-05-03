/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Contributor(s):
 *   Copyright (C) 2012 Tino Kluge <tino.kluge@hrz.tu-chemnitz.de>
 */

#include <comphelper/random.hxx>
#include <sal/log.hxx>
#include <assert.h>
#include <time.h>
#include <mutex>
#include <random>
#include <rtl/random.h>
#include <stdexcept>
#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

// this is nothing but a simple wrapper around
// the std::random generators

namespace comphelper::rng
{
// underlying random number generator
// std::mt19937 implements the Mersenne twister algorithm which
// is fast and has good statistical properties, it produces integers
// in the range of [0, 2^32-1] internally
// memory requirement: 625*sizeof(uint32_t)
// http://en.wikipedia.org/wiki/Mersenne_twister
#define STD_RNG_ALGO std::mt19937

namespace
{
struct RandomNumberGenerator
{
    std::mutex mutex;
    STD_RNG_ALGO global_rng;
    RandomNumberGenerator() { reseed(); }

    void reseed()
    {
        // make RR easier to use, breaks easily without the RNG being repeatable
        bool bRepeatable = (getenv("SAL_RAND_REPEATABLE") != nullptr) || (getenv("RR") != nullptr);
        // valgrind on some platforms (e.g.Ubuntu16.04) does not support the new Intel RDRAND instructions,
        // which leads to "Illegal Opcode" errors, so just turn off randomness.
#if defined HAVE_VALGRIND_HEADERS
        if (RUNNING_ON_VALGRIND)
            bRepeatable = true;
#endif
        if (bRepeatable)
        {
            global_rng.seed(42);
            return;
        }

        size_t seed = 0;
        if (rtl_random_getBytes(nullptr, &seed, sizeof(seed)) != rtl_Random_E_None)
            seed = 0;

        // initialises the state of the global random number generator
        // should only be called once.
        // (note, a few std::variate_generator<> (like normal) have their
        // own state which would need a reset as well to guarantee identical
        // sequence of numbers, e.g. via myrand.distribution().reset())
        global_rng.seed(seed ^ time(nullptr));
    }
};

RandomNumberGenerator& GetTheRandomNumberGenerator()
{
    static RandomNumberGenerator RANDOM;
    return RANDOM;
}
}

void reseed() { GetTheRandomNumberGenerator().reseed(); }

// uniform ints [a,b] distribution
int uniform_int_distribution(int a, int b)
{
    std::uniform_int_distribution<int> dist(a, b);
    auto& gen = GetTheRandomNumberGenerator();
    std::scoped_lock<std::mutex> g(gen.mutex);
    return dist(gen.global_rng);
}

// uniform ints [a,b] distribution
unsigned int uniform_uint_distribution(unsigned int a, unsigned int b)
{
    std::uniform_int_distribution<unsigned int> dist(a, b);
    auto& gen = GetTheRandomNumberGenerator();
    std::scoped_lock<std::mutex> g(gen.mutex);
    return dist(gen.global_rng);
}

// uniform size_t [a,b] distribution
size_t uniform_size_distribution(size_t a, size_t b)
{
    std::uniform_int_distribution<size_t> dist(a, b);
    auto& gen = GetTheRandomNumberGenerator();
    std::scoped_lock<std::mutex> g(gen.mutex);
    return dist(gen.global_rng);
}

// uniform size_t [a,b) distribution
double uniform_real_distribution(double a, double b)
{
    assert(a < b);
    std::uniform_real_distribution<double> dist(a, b);
    auto& gen = GetTheRandomNumberGenerator();
    std::scoped_lock<std::mutex> g(gen.mutex);
    return dist(gen.global_rng);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
