/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: subtotal.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:45:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
// INCLUDE ---------------------------------------------------------------



#include "subtotal.hxx"
#include "interpre.hxx"

// -----------------------------------------------------------------------

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
