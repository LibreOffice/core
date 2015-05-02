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
#include <rtl/instance.hxx>
#include <assert.h>
#include <time.h>
#include <random>

// this is nothing but a simple wrapper around
// the std::random generators

namespace comphelper
{
namespace rng
{

// underlying random number generator
// std::mt19937 implements the Mersenne twister algorithm which
// is fast and has good statistical properties, it produces integers
// in the range of [0, 2^32-1] internally
// memory requirement: 625*sizeof(uint32_t)
// http://en.wikipedia.org/wiki/Mersenne_twister
#define STD_RNG_ALGO  std::mt19937

struct RandomNumberGenerator
{
    STD_RNG_ALGO global_rng;
    RandomNumberGenerator()
    {
        std::random_device rd;
        // initialises the state of the global random number generator
        // should only be called once.
        // (note, a few std::variate_generator<> (like normal) have their
        // own state which would need a reset as well to guarantee identical
        // sequence of numbers, e.g. via myrand.distribution().reset())
        global_rng.seed(rd() ^ time(nullptr));
    }
};

class theRandomNumberGenerator : public rtl::Static<RandomNumberGenerator, theRandomNumberGenerator> {};

// re-initialises the state of the global random number generator
void reseed(int i)
{
    return theRandomNumberGenerator::get().global_rng.seed(i);
}

// uniform ints [a,b] distribution
int uniform_int_distribution(int a, int b)
{
    std::uniform_int_distribution<int> dist(a, b);
    return dist(theRandomNumberGenerator::get().global_rng);
}

// uniform ints [a,b] distribution
unsigned int uniform_uint_distribution(unsigned int a, unsigned int b)
{
    std::uniform_int_distribution<unsigned int> dist(a, b);
    return dist(theRandomNumberGenerator::get().global_rng);
}

// uniform size_t [a,b] distribution
size_t uniform_size_distribution(size_t a, size_t b)
{
    std::uniform_int_distribution<size_t> dist(a, b);
    return dist(theRandomNumberGenerator::get().global_rng);
}

// uniform size_t [a,b) distribution
double uniform_real_distribution(double a, double b)
{
    assert(a < b);
    std::uniform_real_distribution<double> dist(a, b);
    return dist(theRandomNumberGenerator::get().global_rng);
}

} // namespace
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
