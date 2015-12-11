/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_RANDOM_HXX
#define INCLUDED_COMPHELPER_RANDOM_HXX

#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

namespace rng
{

// These functions obey the SAL_RAND_REPEATABLE environment
// variable: If it is set, use a fixed seed.

// note that uniform_int_distribution is inclusive of b, i.e. [a,b] while
// uniform_real_distribution is exclusive of b, i.e. [a,b), [std/boost]::nextafter may be your friend there

/// uniform distribution in [a,b)
COMPHELPER_DLLPUBLIC double uniform_real_distribution(double a = 0.0, double b = 1.0);

/// uniform distribution in [a,b]
COMPHELPER_DLLPUBLIC int uniform_int_distribution(int a, int b);

/// uniform distribution in [a,b]
COMPHELPER_DLLPUBLIC unsigned int uniform_uint_distribution(unsigned int a, unsigned int b);

/// uniform distribution in [a,b]
COMPHELPER_DLLPUBLIC size_t uniform_size_distribution(size_t a, size_t b);

} // namespace

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
