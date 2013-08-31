/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <resourcemodel/Fraction.hxx>

namespace writerfilter {
namespace resourcemodel {

// Stein's binary GCD for non-negative integers
// https://en.wikipedia.org/wiki/Binary_GCD_algorithm
static sal_uInt32 gcd(sal_uInt32 a, sal_uInt32 b)
{
    if (a == 0 || b == 0)
        return a | b;

    sal_uInt32 nShift = 0;
    while (((a | b) & 1) == 0)
    {
        a >>= 1;
        b >>= 1;
        ++nShift;
    }

    while ((a & 1) == 0)
        a >>= 1;

    do
    {
        while ((b & 1) == 0)
            b >>= 1;

        if (a < b)
        {
            b -= a;
        }
        else
        {
            sal_uInt32 nDiff = a - b;
            a = b;
            b = nDiff;
        }

        b >>= 1;
    }
    while (b != 0);

    return a << nShift;
}

static sal_uInt32 lcm(sal_Int32 a, sal_Int32 b)
{
    return abs(a * b) / gcd(abs(a), abs(b));
}

Fraction::Fraction(sal_Int32 nNumerator, sal_Int32 nDenominator)
{
    init(nNumerator, nDenominator);
}

Fraction::~Fraction()
{
}

void Fraction::init(sal_Int32 nNumerator, sal_Int32 nDenominator)
{
    sal_uInt32 nGCD = gcd(abs(nNumerator), abs(nDenominator));

    mnNumerator = nNumerator/ nGCD;
    mnDenominator = nDenominator / nGCD;
}

void Fraction::assign(const Fraction & rFraction)
{
    init(rFraction.mnNumerator, rFraction.mnDenominator);
}

Fraction Fraction::inverse() const
{
    return Fraction(mnDenominator, mnNumerator);
}

Fraction Fraction::operator + (const Fraction & rFraction) const
{
    sal_uInt32 nLCM = lcm(mnDenominator, rFraction.mnDenominator);

    return Fraction(mnNumerator * nLCM / mnDenominator + rFraction.mnNumerator * nLCM / rFraction.mnDenominator, nLCM);
}

Fraction Fraction::operator - (const Fraction & rFraction) const
{
    sal_uInt32 nLCM = lcm(mnDenominator, rFraction.mnDenominator);

    return Fraction(mnNumerator * nLCM / mnDenominator - rFraction.mnNumerator * nLCM / rFraction.mnDenominator, nLCM);
}

Fraction Fraction::operator * (const Fraction & rFraction) const
{
    return Fraction(mnNumerator * rFraction.mnNumerator, mnDenominator * rFraction.mnDenominator);
}

Fraction Fraction::operator / (const Fraction & rFraction) const
{
    return *this * rFraction.inverse();
}

Fraction Fraction::operator = (const Fraction & rFraction)
{
    assign(rFraction);

    return *this;
}

Fraction::operator sal_Int32() const
{
    return mnNumerator / mnDenominator;
}

Fraction::operator float() const
{
    return static_cast<float>(mnNumerator) / static_cast<float>(mnDenominator);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
