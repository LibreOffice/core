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

#ifndef INCLUDED_SC_INC_MATH_HXX
#define INCLUDED_SC_INC_MATH_HXX

#include <formula/errorcodes.hxx>
#include <rtl/math.hxx>
#include <cmath>
#include <cerrno>

namespace sc {

/** Return fNumerator/fDenominator if fDenominator!=0 else #DIV/0! error coded
    into double.
 */
inline double div( const double& fNumerator, const double& fDenominator )
{
    return (fDenominator != 0.0) ? (fNumerator / fDenominator) :
        CreateDoubleError( FormulaError::DivisionByZero);
}

/** Return fNumerator/fDenominator if fDenominator!=0 else +-Infinity if
    fNumerator!=0 or NaN if fNumerator==0.

    This allows to build/run with -fsanitize=float-divide-by-zero and have a
    defined behavior for the otherwise undefined division by zero case ("If the
    second operand of / or % is zero the behavior is undefined."
    ([expr.mul]/4)).

    The Calc interpreter gracefully handles Infinity or NaN double values
    encountered as interim or final results, using this function we can ensure
    defined behavior where desired.

    Use where the double coded error creating div() is not wanted.
 */
inline double divide( const double& fNumerator, const double& fDenominator )
{
    if (fDenominator == 0.0)
    {
        double fVal;
        if (rtl::math::isFinite( fNumerator) && fNumerator != 0.0)
        {
            rtl::math::setInf( &fVal, rtl::math::isSignBitSet( fNumerator));
        }
        else
        {
            rtl::math::setNan( &fVal);
        }
        return fVal;
    }
    return fNumerator / fDenominator;
}

inline double errno_pow( const double& fVal1, const double& fVal2 )
{
    // pow() is expected to set range error or pole error or return NaN or Inf.
    assert(math_errhandling & MATH_ERRNO);
    errno = 0;
    return pow( fVal1, fVal2);
}

inline double power( const double& fVal1, const double& fVal2 )
{
    double fPow;
    if (fVal1 < 0 && fVal2 != 0.0)
    {
        const double f = 1.0 / fVal2 + ((fVal2 < 0.0) ? -0.5 : 0.5);
        if (f < SAL_MIN_INT64 || f > SAL_MAX_INT64)
        {
            // Casting to int would be undefined behaviour.
            fPow = errno_pow( fVal1, fVal2);
        }
        else
        {
            const sal_Int64 i = static_cast<sal_Int64>(f);
            if (i % 2 != 0 && rtl::math::approxEqual(1 / static_cast<double>(i), fVal2))
                fPow = -errno_pow( -fVal1, fVal2);
            else
                fPow = errno_pow( fVal1, fVal2);
        }
    }
    else
    {
        fPow = errno_pow( fVal1, fVal2);
    }
    // The pow() call must had been the last call before this to check errno.
    if (errno == EDOM || errno == ERANGE || !rtl::math::isFinite(fPow))
        fPow = CreateDoubleError( FormulaError::IllegalFPOperation);
    return fPow;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
