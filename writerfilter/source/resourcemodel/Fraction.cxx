/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <resourcemodel/Fraction.hxx>

namespace writerfilter {
namespace resourcemodel {

sal_uInt32 gcd(sal_uInt32 a, sal_uInt32 b)
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

sal_uInt32 lcm(sal_Int32 a, sal_Int32 b)
{
    return abs(a * b) / gcd(abs(a), abs(b));
}

Fraction::Fraction(sal_Int32 nNumerator, sal_Int32 nDenominator)
{
    init(nNumerator, nDenominator);
}

Fraction::Fraction(const Fraction & a, const Fraction & b)
{
    init(a.mnNumerator * b.mnDenominator, a.mnDenominator * b.mnNumerator);
}

Fraction::~Fraction()
{
}

void Fraction::init(sal_Int32 nNumerator, sal_Int32 nDenominator)
{
    sal_uInt32 nGCD = gcd(nNumerator, nDenominator);

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
