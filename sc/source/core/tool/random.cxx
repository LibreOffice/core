/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * Contributor(s):
 *   Copyright (C) 2012 Tino Kluge <tino.kluge@hrz.tu-chemnitz.de>
 */

#include <boost/random.hpp>




namespace sc
{
namespace rng
{







#define BOOST_RNG_ALGO  boost::mt19937
BOOST_RNG_ALGO global_rng;






void seed(int i)
{
    global_rng.seed(i);
}


double uniform()
{
    static boost::uniform_01<BOOST_RNG_ALGO&> myrand(global_rng);
    return myrand();
}

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
