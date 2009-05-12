/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: subtotal.cxx,v $
 * $Revision: 1.7.32.1 $
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

#if OLD_PIVOT_IMPLEMENTATION
SubTotal::SubTotal()
{
    nIndex = 0;             //! test fuer Pivot

    nCount     = 0;
    nCount2    = 0;
    nSum       = 0.0;
    nSumSqr    = 0.0;
    nMax       = -MAXDOUBLE;
    nMin       = MAXDOUBLE;
    nProduct   = 1.0;
    bSumOk     = TRUE;
    bSumSqrOk  = TRUE;
    bProductOk = TRUE;
}


SubTotal::~SubTotal()
{
}


void SubTotal::UpdateNoVal()
{
    nCount++;
}


void SubTotal::Update( double nVal )
{
    SAL_MATH_FPEXCEPTIONS_OFF();
    nCount++;
    nCount2++;
    if (nVal > nMax) nMax = nVal;
    if (nVal < nMin) nMin = nVal;
    nProgress = 0;
    if (bSumOk) nSum += nVal;
    nProgress = 1;
    if (bProductOk) nProduct *= nVal;
    nProgress = 2;
    if (bSumSqrOk) nSumSqr += nVal*nVal;
    if (!::rtl::math::isFinite(nSum))
        bSumOk = FALSE;
    if (!::rtl::math::isFinite(nProduct))
        bProductOk = FALSE;
    if (!::rtl::math::isFinite(nSumSqr))
        bSumSqrOk = FALSE;
}


void SubTotal::Update( const SubTotal& rVal )
{
    SAL_MATH_FPEXCEPTIONS_OFF();
    nCount  += rVal.nCount;
    nCount2 += rVal.nCount2;
    if (rVal.nMax > nMax) nMax = rVal.nMax;
    if (rVal.nMin < nMin) nMin = rVal.nMin;
    nProgress = 0;
    if (rVal.bSumOk && bSumOk)
        nSum += rVal.nSum;
    else
        bSumOk = FALSE;
    nProgress = 1;
    if (rVal.bProductOk && bProductOk)
        nProduct *= rVal.nProduct;
    else
        bProductOk = FALSE;
    nProgress = 2;
    if (rVal.bSumSqrOk && bSumSqrOk)
        nSumSqr += rVal.nSumSqr;
    else
        bSumSqrOk = FALSE;
    if (!::rtl::math::isFinite(nSum))
        bSumOk = FALSE;
    if (!::rtl::math::isFinite(nProduct))
        bProductOk = FALSE;
    if (!::rtl::math::isFinite(nSumSqr))
        bSumSqrOk = FALSE;
}


short SubTotal::Valid( USHORT nFunction ) const
                            // return 0 => Fehler, -1 => kein Wert, 1 => ok
{
    short nRet;
    switch (nFunction)
    {
        case PIVOT_FUNC_AVERAGE:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = bSumOk;
            break;
        case PIVOT_FUNC_STD_DEVP:
        case PIVOT_FUNC_STD_VARP:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = bSumSqrOk;
            break;
        case PIVOT_FUNC_STD_DEV:
        case PIVOT_FUNC_STD_VAR:
            if (nCount2 < 2)
                nRet = -1;
            else
                nRet = bSumSqrOk;
            break;
        case PIVOT_FUNC_MAX:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = 1;
            break;
        case PIVOT_FUNC_MIN:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = 1;
            break;
        case PIVOT_FUNC_SUM:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = bSumOk;
            break;
        case PIVOT_FUNC_PRODUCT:
            if (nCount2 == 0)
                nRet = -1;
            else
                nRet = bProductOk;
            break;
        default:
            nRet = 1;
            break;
    }
    return nRet;
}


double SubTotal::Result( USHORT nFunction ) const
{
    double nRet = 0.0;
    switch (nFunction)
    {
        case PIVOT_FUNC_COUNT:      nRet = (double) nCount;     break;
        case PIVOT_FUNC_COUNT_NUM:  nRet = (double) nCount2;    break;
        case PIVOT_FUNC_SUM:        nRet = nSum;                break;
        case PIVOT_FUNC_MAX:        nRet = nMax;                break;
        case PIVOT_FUNC_MIN:        nRet = nMin;                break;
        case PIVOT_FUNC_PRODUCT:    nRet = nProduct;            break;
        case PIVOT_FUNC_AVERAGE:
                if (nCount2 > 0)
                    nRet = nSum / (double) nCount2;
                break;
        case PIVOT_FUNC_STD_DEV:
                if (nCount2 > 1)
                    nRet = sqrt((nSumSqr - nSum*nSum/(double)(nCount2))
                            / (double)(nCount2-1));
                break;
        case PIVOT_FUNC_STD_DEVP:
                if (nCount2 > 0)
                    nRet = sqrt((nSumSqr - nSum*nSum/(double)(nCount2))
                            / (double)nCount2);
                break;
        case PIVOT_FUNC_STD_VAR:
                if (nCount2 > 1)
                    nRet = (nSumSqr - nSum*nSum/(double)(nCount2))
                            / (double)(nCount2-1);
                break;
        case PIVOT_FUNC_STD_VARP:
                if (nCount2 > 0)
                    nRet = (nSumSqr - nSum*nSum/(double)(nCount2))
                            / (double)(nCount2);
                break;
        default:
                DBG_ERROR("unbekannte Funktion bei SubTotal::Result");
    }
    return nRet;
}
#endif

BOOL SubTotal::SafePlus(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 += fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = FALSE;
        if (fVal2 > 0.0)
            fVal1 = DBL_MAX;
        else
            fVal1 = -DBL_MAX;
    }
    return bOk;
}


BOOL SubTotal::SafeMult(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 *= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
    return bOk;
}


BOOL SubTotal::SafeDiv(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SAL_MATH_FPEXCEPTIONS_OFF();
    fVal1 /= fVal2;
    if (!::rtl::math::isFinite(fVal1))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
    return bOk;
}
