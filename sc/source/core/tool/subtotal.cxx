/*************************************************************************
 *
 *  $RCSfile: subtotal.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:19 $
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

#pragma options FLTTRAP
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

#pragma hdrstop

#include <float.h>
#include <math.h>
#ifdef SOLARIS
#include <ieeefp.h>
#endif
#include <signal.h>
#include "segmentc.hxx"

#include "subtotal.hxx"
#include "interpre.hxx"

#include "scdll.hxx"        // ScLibSignalFunc

// STATIC DATA -----------------------------------------------------------

jmp_buf SubTotal::aGlobalJumpBuf;

SEG_EOFGLOBALS()

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(subtotal_01)

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

#pragma SEG_FUNCDEF(subtotal_07)

SubTotal::~SubTotal()
{
}

#pragma SEG_FUNCDEF(subtotal_02)

void SubTotal::UpdateNoVal()
{
    nCount++;
}

#pragma SEG_FUNCDEF(subtotal_03)

void SubTotal::Update( double nVal )
{
    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (ScInterpreter::pSignalFunc)( SC_SIGFPE, ScInterpreter::MySigFunc );
    jmp_buf* pSaveBuf = ScInterpreter::pJumpBuf;
    ScInterpreter::pJumpBuf = &aGlobalJumpBuf;
    if (setjmp(aGlobalJumpBuf))
    {
        if (nProgress == 0)
        {
            bSumOk = FALSE;
            bProductOk = FALSE;
            bSumSqrOk = FALSE;
        }
        else if (nProgress == 1)
        {
            bProductOk = FALSE;
            bSumSqrOk = FALSE;
        }
        else
        {
            bSumSqrOk = FALSE;
            DBG_ASSERT(nProgress == 2,"subtotal.cxx: Update(1)");
        }
    }
    else
#endif  // SOMA_FPSIGNAL_JUMP
    {
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
    }
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(nSum))
        bSumOk = FALSE;
    if (!SOMA_FINITE(nProduct))
        bProductOk = FALSE;
    if (!SOMA_FINITE(nSumSqr))
        bSumSqrOk = FALSE;
#else   // SOMA_FPSIGNAL_JUMP
    ScInterpreter::pJumpBuf = pSaveBuf;
    (ScInterpreter::pSignalFunc)( SC_SIGFPE, pOldSigFunc );
#endif
}

#pragma SEG_FUNCDEF(subtotal_04)

void SubTotal::Update( const SubTotal& rVal )
{
    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (ScInterpreter::pSignalFunc)( SC_SIGFPE, ScInterpreter::MySigFunc );
    jmp_buf* pSaveBuf = ScInterpreter::pJumpBuf;
    ScInterpreter::pJumpBuf = &aGlobalJumpBuf;
    if (setjmp(aGlobalJumpBuf))
    {
        if (nProgress == 0)
        {
            bSumOk = FALSE;
            bProductOk = FALSE;
            bSumSqrOk = FALSE;
        }
        else if (nProgress == 1)
        {
            bProductOk = FALSE;
            bSumSqrOk = FALSE;
        }
        else
        {
            DBG_ASSERT(nProgress == 2,"subtotal.cxx: Update(2)");
            bSumSqrOk = FALSE;
        }
    }
    else
#endif  // SOMA_FPSIGNAL_JUMP
    {
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
    }
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(nSum))
        bSumOk = FALSE;
    if (!SOMA_FINITE(nProduct))
        bProductOk = FALSE;
    if (!SOMA_FINITE(nSumSqr))
        bSumSqrOk = FALSE;
#else // SOMA_FPSIGNAL_JUMP
    ScInterpreter::pJumpBuf = pSaveBuf;
    (ScInterpreter::pSignalFunc)( SC_SIGFPE, pOldSigFunc );
#endif
}

#pragma SEG_FUNCDEF(subtotal_05)

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

#pragma SEG_FUNCDEF(subtotal_06)

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

#pragma SEG_FUNCDEF(subtotal_08)

BOOL SubTotal::SafePlus(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (ScInterpreter::pSignalFunc)( SC_SIGFPE, ScInterpreter::MySigFunc );
    jmp_buf* pSaveBuf = ScInterpreter::pJumpBuf;
    ScInterpreter::pJumpBuf = &aGlobalJumpBuf;
    if (setjmp(aGlobalJumpBuf))
    {
        bOk = FALSE;
        if (fVal2 > 0.0)
            fVal1 = DBL_MAX;
        else
            fVal1 = -DBL_MAX;
    }
    else
#endif  // SOMA_FPSIGNAL_JUMP
        fVal1 += fVal2;
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(fVal1))
    {
        bOk = FALSE;
        if (fVal2 > 0.0)
            fVal1 = DBL_MAX;
        else
            fVal1 = -DBL_MAX;
    }
#else // SOMA_FPSIGNAL_JUMP
    ScInterpreter::pJumpBuf = pSaveBuf;
    (ScInterpreter::pSignalFunc)( SC_SIGFPE, pOldSigFunc );
#endif
    return bOk;
}

#pragma SEG_FUNCDEF(subtotal_09)

BOOL SubTotal::SafeMult(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (ScInterpreter::pSignalFunc)( SC_SIGFPE, ScInterpreter::MySigFunc );
    jmp_buf* pSaveBuf = ScInterpreter::pJumpBuf;
    ScInterpreter::pJumpBuf = &aGlobalJumpBuf;
    if (setjmp(aGlobalJumpBuf))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
    else
#endif  // SOMA_FPSIGNAL_JUMP
        fVal1 *= fVal2;
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(fVal1))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
#else // SOMA_FPSIGNAL_JUMP
    ScInterpreter::pJumpBuf = pSaveBuf;
    (ScInterpreter::pSignalFunc)( SC_SIGFPE, pOldSigFunc );
#endif
    return bOk;
}

#pragma SEG_FUNCDEF(subtotal_0a)

BOOL SubTotal::SafeDiv(double& fVal1, double fVal2)
{
    BOOL bOk = TRUE;
    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (ScInterpreter::pSignalFunc)( SC_SIGFPE, ScInterpreter::MySigFunc );
    jmp_buf* pSaveBuf = ScInterpreter::pJumpBuf;
    ScInterpreter::pJumpBuf = &aGlobalJumpBuf;
    if (setjmp(aGlobalJumpBuf))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
    else
#endif  // SOMA_FPSIGNAL_JUMP
        fVal1 /= fVal2;
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(fVal1))
    {
        bOk = FALSE;
        fVal1 = DBL_MAX;
    }
#else   // SOMA_FPSIGNAL_JUMP
    ScInterpreter::pJumpBuf = pSaveBuf;
    (ScInterpreter::pSignalFunc)( SC_SIGFPE, pOldSigFunc );
#endif
    return bOk;
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.25  2000/09/17 14:08:42  willem.vandorp
    OpenOffice header added.

    Revision 1.24  2000/08/31 16:38:04  willem.vandorp
    Header and footer replaced

    Revision 1.23  1999/05/05 19:50:32  ER
    SC_FP... -> SOMA_FP..., SC_FINITE -> SOMA_FINITE, Approx... -> SolarMath::Approx...


      Rev 1.22   05 May 1999 21:50:32   ER
   SC_FP... -> SOMA_FP..., SC_FINITE -> SOMA_FINITE, Approx... -> SolarMath::Approx...

      Rev 1.21   13 Jan 1998 17:24:38   ER
   Unterteilung SC_FPRESET und SC_FPCONTROL

      Rev 1.20   13 Nov 1997 21:07:54   NN
   ifndef PCH raus

      Rev 1.19   15 Sep 1997 14:21:10   HR
   SOLARIS

      Rev 1.18   09 Apr 1997 19:14:04   ER
   #38622# define SC_FPSIGNAL_JUMP etc.; OS2 jetzt wie WNT ohne setjmp/longjmp

      Rev 1.17   14 Nov 1996 16:38:18   ER
   SC_FINITE statt einzel-ifdefs

      Rev 1.16   20 Aug 1996 13:11:28   NN
   #30412# ScInterpreter::pJumpBuf auch wieder zuruecksetzen

      Rev 1.15   09 Jul 1996 12:06:50   NN
   #29279# fpreset

      Rev 1.14   08 May 1996 08:43:00   NN
   SaveDif

      Rev 1.13   08 Sep 1995 16:03:58   STE
   Absicherung Mac und WNT

      Rev 1.12   18 Aug 1995 13:12:38   mk
   jetzt RICHTIG: includes fuer RS6000

      Rev 1.11   18 Aug 1995 12:58:34   mk
   includes fuer RS6000

      Rev 1.10   24 Jul 1995 18:57:20   STE
   ein jmp_buf weniger

      Rev 1.9   12 Jul 1995 11:57:08   STE
   nVal auf MAX_DBL gesetzt

      Rev 1.8   08 Jul 1995 14:54:14   STE
   SafeAdd und SafeMult dazu

      Rev 1.7   19 Jun 1995 16:04:56   STE
   meory.h raus

      Rev 1.6   19 Jun 1995 09:19:08   STE
   Absicherung ueberarbeitet

      Rev 1.5   16 Jun 1995 13:28:52   STE
   Umstellung numerische ABsicherung

      Rev 1.4   18 May 1995 19:35:08   STE
   Fehler bei Mult beseitigt

      Rev 1.3   17 May 1995 17:47:14   STE
   Numerische Absicherung

      Rev 1.2   06 May 1995 18:29:34   STE
   Absturz Stddev beseitigt

      Rev 1.1   05 May 1995 12:54:54   STE
   Korrektur VARP und STDDEVP

      Rev 1.0   28 Apr 1995 15:55:30   NN
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


