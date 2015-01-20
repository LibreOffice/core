/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FORMULA_RANDOM_HXX
#define INCLUDED_FORMULA_RANDOM_HXX

#include <formula/formuladllapi.h>

namespace formula
{

namespace rng
{

// These two functions obey the SC_RAND_REPEATABLE environment
// variable: If it is set, use a fixed seed.
double FORMULA_DLLPUBLIC fRandom(double a, double b);
sal_Int32 FORMULA_DLLPUBLIC nRandom(sal_Int32 a, sal_Int32 b);

} // rng
} // formula

#endif // INCLUDED_FORMULA_RANDOM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
