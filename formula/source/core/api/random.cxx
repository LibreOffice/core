/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <time.h>

#include <random>

#include <formula/random.hxx>
#include <rtl/instance.hxx>

namespace {

struct CalcFormulaRandomGenerator
{
    std::mt19937 aRng;
    CalcFormulaRandomGenerator()
    {
        // initialises the state of this RNG.
        // should only be called once.
        bool bRepeatable = (getenv("SC_RAND_REPEATABLE") != 0);
        aRng.seed(bRepeatable ? 42 : time(NULL));
    }
};

class theCalcFormulaRandomGenerator : public rtl::Static<CalcFormulaRandomGenerator, theCalcFormulaRandomGenerator> {};

}

namespace formula
{

namespace rng
{

double fRandom(double a, double b)
{
    std::uniform_real_distribution<double> dist(a, b);
    return dist(theCalcFormulaRandomGenerator::get().aRng);
}

sal_Int32 nRandom(sal_Int32 a, sal_Int32 b)
{
    std::uniform_int_distribution<sal_Int32> dist(a, b);
    return dist(theCalcFormulaRandomGenerator::get().aRng);
}

} // rng
} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
