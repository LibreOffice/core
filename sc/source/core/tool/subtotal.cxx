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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
// INCLUDE ---------------------------------------------------------------



#include "subtotal.hxx"
#include "interpre.hxx"

// -----------------------------------------------------------------------

sal_Bool SubTotal::SafePlus(double& fVal1, double fVal2)
{
    sal_Bool bOk = sal_True;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 += fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = sal_False;
        if (fVal2 > 0.0)
            fVal1 = DBL_MAX;
        else
            fVal1 = -DBL_MAX;
    }
    return bOk;
}


sal_Bool SubTotal::SafeMult(double& fVal1, double fVal2)
{
    sal_Bool bOk = sal_True;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 *= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = sal_False;
        fVal1 = DBL_MAX;
    }
    return bOk;
}


sal_Bool SubTotal::SafeDiv(double& fVal1, double fVal2)
{
    sal_Bool bOk = sal_True;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 /= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = sal_False;
        fVal1 = DBL_MAX;
    }
    return bOk;
}
