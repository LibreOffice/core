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

#include <boost/random.hpp>

// this is nothing but a simple wrapper around
// the boost random generators

namespace sc
{
namespace rng
{

// underlying random number generator
// boost::mt19937 implements the Mersenne twister algorithm which
// is fast and has good statistical properties, it produces integers
// in the range of [0, 2^32-1] internally
// memory requirement: 625*sizeof(uint32_t)
// http://en.wikipedia.org/wiki/Mersenne_twister
#define BOOST_RNG_ALGO  boost::mt19937
BOOST_RNG_ALGO global_rng;

// initialises the state of the global random number generator
// should only be called once at the start of the main programme
// (note, a few boost::variate_generator<> (like normal) have their
// own state which would need a reset as well to guarantee identical
// sequence of numbers, e.g. via myrand.distribution().reset())
void seed(int i)
{
    global_rng.seed(i);
}

// uniform [0,1) or [a,b) distribution
double uniform()
{
    static boost::uniform_01<BOOST_RNG_ALGO&> myrand(global_rng);
    return myrand();
}

} // namespace
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
