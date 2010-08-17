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
#ifndef INCLUDED_FRACTION_HXX
#define INCLUDED_FRACTION_HXX

#include <sal/types.h>

namespace writerfilter {
namespace resourcemodel {

class Fraction
{
public:
    explicit Fraction(sal_Int32 nNumerator, sal_Int32 nDenominator = 1);
    explicit Fraction(const Fraction & a, const Fraction & b);
    virtual ~Fraction();

    void init(sal_Int32 nNumerator, sal_Int32 nDenominator);
    void assign(const Fraction & rFraction);

    Fraction inverse() const;

    Fraction operator=(const Fraction & rFraction);
    Fraction operator+(const Fraction & rFraction) const;
    Fraction operator-(const Fraction & rFraction) const;
    Fraction operator*(const Fraction & rFraction) const;
    Fraction operator/(const Fraction & rFraction) const;
    operator sal_Int32() const;
    operator float() const;

private:
    sal_Int32 mnNumerator;
    sal_Int32 mnDenominator;
};
}}
#endif // INCLUDED_FRACTION_HXX
