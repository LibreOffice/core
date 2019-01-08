/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <math.hxx>
#include <cmath>
#include <cerrno>
#include <cfenv>

namespace sc {

static double err_pow( const double& fVal1, const double& fVal2 )
{
    // pow() is expected to set domain error or pole error or range error (or
    // flag them via exceptions) or return NaN or Inf.
    assert((math_errhandling & (MATH_ERRNO | MATH_ERREXCEPT)) != 0);
    std::feclearexcept(FE_ALL_EXCEPT);
    errno = 0;
    return pow( fVal1, fVal2);
}

double power( const double& fVal1, const double& fVal2 )
{
    double fPow;
    if (fVal1 < 0 && fVal2 != 0.0)
    {
        const double f = 1.0 / fVal2 + ((fVal2 < 0.0) ? -0.5 : 0.5);
        if (f < SAL_MIN_INT64 || f > SAL_MAX_INT64)
        {
            // Casting to int would be undefined behaviour.
            fPow = err_pow( fVal1, fVal2);
        }
        else
        {
            const sal_Int64 i = static_cast<sal_Int64>(f);
            if (i % 2 != 0 && rtl::math::approxEqual(1 / static_cast<double>(i), fVal2))
                fPow = -err_pow( -fVal1, fVal2);
            else
                fPow = err_pow( fVal1, fVal2);
        }
    }
    else
    {
        fPow = err_pow( fVal1, fVal2);
    }
    // The pow() call must had been the most recent call to check errno or exception.
    if ((((math_errhandling & MATH_ERRNO) != 0) && (errno == EDOM || errno == ERANGE))
            || (((math_errhandling & MATH_ERREXCEPT) != 0)
                && std::fetestexcept( FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW))
            || !rtl::math::isFinite(fPow))
    {
        fPow = CreateDoubleError( FormulaError::IllegalFPOperation);
    }
    return fPow;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
