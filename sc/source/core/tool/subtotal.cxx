/*************************************************************************
 *
 *  $RCSfile: subtotal.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
// INCLUDE ---------------------------------------------------------------

#ifdef RS6000

#include <fptrap.h>
#include <fpxcp.h>

#elif defined ( MAC )

#include <MAC_START.h>
#include <fp.h>
#include <MAC_END.h>

#endif

#ifdef PCH
#include "core_pch.hxx"
#endif


#include <float.h>
#include <math.h>
#ifdef SOLARIS
#include <ieeefp.h>
#endif
#include <signal.h>

#include "subtotal.hxx"
#include "interpre.hxx"

// STATIC DATA -----------------------------------------------------------

jmp_buf SubTotal::aGlobalJumpBuf;

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
