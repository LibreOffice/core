/*************************************************************************
 *
 *  $RCSfile: interpr2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:44:51 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/stritem.hxx>
#include <svtools/zforlist.hxx>
#include <string.h>
#include <math.h>

#include "interpre.hxx"
#include "attrib.hxx"
#include "sc.hrc"
#include "ddelink.hxx"
#include "scmatrix.hxx"
#include "compiler.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "unitconv.hxx"
#include "globstr.hrc"
#include "hints.hxx"

// STATIC DATA -----------------------------------------------------------

#define D_TIMEFACTOR              86400.0
#define SCdEpsilon                1.0E-7

//-----------------------------------------------------------------------------
// Datum und Zeit
//-----------------------------------------------------------------------------

double ScInterpreter::GetDate(INT16 nYear, INT16 nMonth, INT16 nDay)
{
    if ( nYear < 100 )
        nYear = pFormatter->ExpandTwoDigitYear( nYear );
    INT16 nY, nM;
    if (nMonth > 0)
    {
        nY = nYear + (nMonth-1) / 12;
        nM = ((nMonth-1) % 12) + 1;
    }
    else
    {
        nY = nYear + (nMonth-12) / 12;
        nM = 12 - (-nMonth) % 12;
    }
    Date aDate(1, nM, nY);
    aDate += nDay - 1;
    if (aDate.IsValid())
        return (double) (aDate - *(pFormatter->GetNullDate()));
    else
    {
        SetError(errNoValue);
        return 0;
    }
}

//-----------------------------------------------------------------------------
// Funktionen
//-----------------------------------------------------------------------------

void ScInterpreter::ScGetActDate()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    Date aActDate;
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    PushDouble((double) nDiff);
}

void ScInterpreter::ScGetActTime()
{
    nFuncFmtType = NUMBERFORMAT_DATETIME;
    Date aActDate;
    long nDiff = aActDate - *(pFormatter->GetNullDate());
    Time aActTime;
    double nTime = ((double)aActTime.Get100Sec() / 100 +
                    (double)(aActTime.GetSec()        +
                            (aActTime.GetMin()  * 60) +
                            (aActTime.GetHour() * 3600))) / D_TIMEFACTOR;
    PushDouble( (double) nDiff + nTime );
}

void ScInterpreter::ScGetYear()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) SolarMath::ApproxFloor(GetDouble());
    PushDouble( (double) aDate.GetYear() );
}

void ScInterpreter::ScGetMonth()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) SolarMath::ApproxFloor(GetDouble());
    PushDouble( (double) aDate.GetMonth() );
}

void ScInterpreter::ScGetDay()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long)SolarMath::ApproxFloor(GetDouble());
    PushDouble((double) aDate.GetDay());
}

void ScInterpreter::ScGetMin()
{
    double fTime = GetDouble();
    fTime -= SolarMath::ApproxFloor(fTime);     // Datumsanteil weg
    long nVal = (long)SolarMath::ApproxFloor(fTime*D_TIMEFACTOR+0.5) % 3600;
    PushDouble( (double) (nVal/60) );
}

void ScInterpreter::ScGetSec()
{
    double fTime = GetDouble();
    fTime -= SolarMath::ApproxFloor(fTime);     // Datumsanteil weg
    long nVal = (long)SolarMath::ApproxFloor(fTime*D_TIMEFACTOR+0.5) % 60;
    PushDouble( (double) nVal );
}

void ScInterpreter::ScGetHour()
{
    double fTime = GetDouble();
    fTime -= SolarMath::ApproxFloor(fTime);     // Datumsanteil weg
    long nVal = (long)SolarMath::ApproxFloor(fTime*D_TIMEFACTOR+0.5) / 3600;
    PushDouble((double) nVal);
}

void ScInterpreter::ScGetDateValue()
{
    String aInputString = GetString();
    ULONG nFIndex = 0;                  // damit default Land/Spr.
    double fVal;
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
    {
        short eType = pFormatter->GetType(nFIndex);
        if (eType == NUMBERFORMAT_DATE || eType == NUMBERFORMAT_DATETIME)
            PushDouble(fVal);
        else
            SetIllegalArgument();
    }
    else
        SetIllegalArgument();
}

void ScInterpreter::ScGetDayOfWeek()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        short nFlag;
        if (nParamCount == 2)
            nFlag = (short) SolarMath::ApproxFloor(GetDouble());
        else
            nFlag = 1;

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)SolarMath::ApproxFloor(GetDouble());
        int nVal = (int) aDate.GetDayOfWeek();
        if (nFlag == 1)
        {
            if (nVal == 6)
                nVal = 1;
            else
                nVal += 2;
        }
        else if (nFlag == 2)
            nVal += 1;
        PushInt( nVal );
    }
}

void ScInterpreter::ScGetWeekOfYear()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        short nFlag = (short) SolarMath::ApproxFloor(GetDouble());

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)SolarMath::ApproxFloor(GetDouble());
        PushInt( (int) aDate.GetWeekOfYear( nFlag == 1 ? SUNDAY : MONDAY ));
    }
}

void ScInterpreter::ScEasterSunday()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        INT16 nDay, nMonth, nYear;
        nYear = (INT16) SolarMath::ApproxFloor( GetDouble() );
        if ( nYear < 100 )
            nYear = pFormatter->ExpandTwoDigitYear( nYear );
        // don't worry, be happy :)
        int B,C,D,E,F,G,H,I,K,L,M,N,O;
        N = nYear % 19;
        B = int(nYear / 100);
        C = nYear % 100;
        D = int(B / 4);
        E = B % 4;
        F = int((B + 8) / 25);
        G = int((B - F + 1) / 3);
        H = (19 * N + B - D - G + 15) % 30;
        I = int(C / 4);
        K = C % 4;
        L = (32 + 2 * E + 2 * I - H - K) % 7;
        M = int((N + 11 * H + 22 * L) / 451);
        O = H + L - 7 * M + 114;
        nDay = O % 31 + 1;
        nMonth = int(O / 31);
        PushDouble( GetDate( nYear, nMonth, nDay ) );
    }
}

void ScInterpreter::ScGetDate()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        INT16 nDay   = (INT16) SolarMath::ApproxFloor(GetDouble());
        INT16 nMonth = (INT16) SolarMath::ApproxFloor(GetDouble());
        INT16 nYear  = (INT16) SolarMath::ApproxFloor(GetDouble());
        if (nYear < 0)
            SetIllegalParameter();
        else
        {
            PushDouble(GetDate(nYear, nMonth, nDay));
        }
    }
}

void ScInterpreter::ScGetTime()
{
    nFuncFmtType = NUMBERFORMAT_TIME;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nSec = GetDouble();
        double nMin = GetDouble();
        double nHour = GetDouble();
        PushDouble( ( (nHour * 3600) + (nMin * 60) + nSec ) / D_TIMEFACTOR );
    }
}

void ScInterpreter::ScGetDiffDate()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nDate2 = GetDouble();
        double nDate1 = GetDouble();
        PushDouble(nDate1 - nDate2);
    }
}

void ScInterpreter::ScGetDiffDate360()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        BOOL bFlag;
        if (nParamCount == 3)
            bFlag = GetBool();
        else
            bFlag = FALSE;
        double nDate2 = GetDouble();
        double nDate1 = GetDouble();
        double fSign;
        if (nGlobalError == 0)
        {
            if (nDate2 < nDate1)
            {
                fSign = nDate1;
                nDate1 = nDate2;
                nDate2 = fSign;
                fSign = -1.0;
            }
            else
                fSign = 1.0;
            Date aDate1 = *(pFormatter->GetNullDate());
            aDate1 += (long) SolarMath::ApproxFloor(nDate1);
            Date aDate2 = *(pFormatter->GetNullDate());
            aDate2 += (long) SolarMath::ApproxFloor(nDate2);
            if (aDate1.GetDay() == 31)
                aDate1 -= (ULONG) 1;
            else if (!bFlag)
            {
                if (aDate1.GetMonth() == 2)
                {
                    switch ( aDate1.GetDay() )
                    {
                        case 28 :
                            if ( !aDate1.IsLeapYear() )
                                aDate1.SetDay(30);
                        break;
                        case 29 :
                            aDate1.SetDay(30);
                        break;
                    }
                }
            }
            if (aDate2.GetDay() == 31)
            {
                if (!bFlag && aDate1.GetDay() != 30)
                        aDate2 += (ULONG) 1;            // -> 1.
                    else
                        aDate2.SetDay(30);
            }
            PushDouble( fSign * (double)
                (  (double) aDate2.GetDay() + (double) aDate2.GetMonth() * 30.0 +
                   (double) aDate2.GetYear() * 360.0
                 - (double) aDate1.GetDay() - (double) aDate1.GetMonth() * 30.0
                 - (double)aDate1.GetYear() * 360.0) );
        }
        else
            SetIllegalParameter();
    }
}

void ScInterpreter::ScGetTimeValue()
{
    String aInputString = GetString();
    ULONG nFIndex = 0;                  // damit default Land/Spr.
    double fVal;
    if (pFormatter->IsNumberFormat(aInputString, nFIndex, fVal))
    {
        short eType = pFormatter->GetType(nFIndex);
        if (eType == NUMBERFORMAT_TIME || eType == NUMBERFORMAT_DATETIME)
            PushDouble(fVal);
        else
            SetIllegalArgument();
    }
    else
        SetIllegalArgument();
}

void ScInterpreter::ScPlusMinus()
{
    double nVal = GetDouble();
    short n = 0;
    if (nVal < 0.0)
        n = -1;
    else if (nVal > 0.0)
        n = 1;
    PushInt( n );
}

void ScInterpreter::ScAbs()
{
    PushDouble(fabs(GetDouble()));
}

void ScInterpreter::ScInt()
{
    PushDouble(SolarMath::ApproxFloor(GetDouble()));
}


void ScInterpreter::RoundNumber( SolarMathRoundingMode eMode )
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fVal = 0.0;
        if (nParamCount == 1)
            fVal = SolarMath::Round( GetDouble(), 0, eMode );
        else
        {
            INT32 nDec = (INT32) SolarMath::ApproxFloor(GetDouble());
            if( nDec < -20 || nDec > 20 )
                SetIllegalArgument();
            else
                fVal = SolarMath::Round( GetDouble(), (short)nDec, eMode );
        }
        PushDouble(fVal);
    }
}

void ScInterpreter::ScRound()
{
    RoundNumber( SolarMathRoundCorrected );
}

void ScInterpreter::ScRoundDown()
{
    RoundNumber( SolarMathRoundDown );
}

void ScInterpreter::ScRoundUp()
{
    RoundNumber( SolarMathRoundUp );
}

void ScInterpreter::ScCeil()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        BOOL bAbs = ( nParamCount == 3 ? GetBool() : FALSE );
        double fDec = GetDouble();
        double fVal = GetDouble();
        if ( fDec == 0.0 )
            PushInt(0);
        else if (fVal*fDec < 0.0)
            SetIllegalArgument();
        else
        {
            if ( !bAbs && fVal < 0.0 )
                PushDouble(SolarMath::ApproxFloor(fVal/fDec) * fDec);
            else
                PushDouble(SolarMath::ApproxCeil(fVal/fDec) * fDec);
        }
    }
}

void ScInterpreter::ScFloor()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        BOOL bAbs = ( nParamCount == 3 ? GetBool() : FALSE );
        double fDec = GetDouble();
        double fVal = GetDouble();
        if ( fDec == 0.0 )
            PushInt(0);
        else if (fVal*fDec < 0.0)
            SetIllegalArgument();
        else
        {
            if ( !bAbs && fVal < 0.0 )
                PushDouble(SolarMath::ApproxCeil(fVal/fDec) * fDec);
            else
                PushDouble(SolarMath::ApproxFloor(fVal/fDec) * fDec);
        }
    }
}

void ScInterpreter::ScEven()
{
    double fVal = GetDouble();
    if (fVal < 0.0)
        PushDouble(SolarMath::ApproxFloor(fVal/2.0) * 2.0);
    else
        PushDouble(SolarMath::ApproxCeil(fVal/2.0) * 2.0);
}

void ScInterpreter::ScOdd()
{
    double fVal = GetDouble();
    if (fVal >= 0.0)
    {
        fVal = SolarMath::ApproxCeil(fVal);
        if (fmod(fVal, 2.0) == 0.0)
            fVal += 1.0;
    }
    else
    {
        fVal = SolarMath::ApproxFloor(fVal);
        if (fmod(fVal, 2.0) == 0.0)
            fVal -= 1.0;
    }
    PushDouble(fVal);
}

void ScInterpreter::ScArcTan2()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nVal2 = GetDouble();
        double nVal1 = GetDouble();
        PushDouble(atan2(nVal2, nVal1));
    }
}

void ScInterpreter::ScLog()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nBase = GetDouble();
        double nVal = GetDouble();
        if (nVal > 0.0 && nBase > 0.0 && nBase != 1.0)
            PushDouble(log(nVal) / log(nBase));
        else
            SetIllegalArgument();
    }
}

void ScInterpreter::ScLn()
{
    double fVal = GetDouble();
    if (fVal > 0.0)
        PushDouble(log(fVal));
    else
        SetIllegalArgument();
}

void ScInterpreter::ScLog10()
{
    double fVal = GetDouble();
    if (fVal > 0.0)
        PushDouble(log10(fVal));
    else
        SetIllegalArgument();
}

void ScInterpreter::ScNBW()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 31 ) )
    {
        double nVal = 0.0;
        // Wir drehen den Stack um!!
        ScToken* pTemp[ 31 ];
        for( short i = 0; i < nParamCount; i++ )
            pTemp[ i ] = pStack[ sp - i - 1 ];
        memcpy( &pStack[ sp - nParamCount ], pTemp, nParamCount * sizeof( ScToken* ) );
        if (nGlobalError == 0)
        {
            double  nCount = 1.0;
            double  nZins = GetDouble();
            ScRange aRange;
            for (short i = 2; i <= (short) nParamCount; i++)
            {
                switch (GetStackType())
                {
                    case svDouble :
                    {
                        nVal += (GetDouble() / pow(1.0 + nZins, nCount));
                        nCount++;
                    }
                    break;
                    case svSingleRef :
                    {
                        nVal += (GetDouble() / pow(1.0 + nZins, nCount));
                        nCount++;
                    }
                    break;
                    case svDoubleRef :
                    {
                        USHORT nErr = 0;
                        double nCellVal;
                        PopDoubleRef( aRange );
                        ScValueIterator aValIter(pDok, aRange, glSubTotal);
                        if (aValIter.GetFirst(nCellVal, nErr))
                        {
                            nVal += (nCellVal / pow(1.0 + nZins, nCount));
                            nCount++;
                            while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                            {
                                nVal += (nCellVal / pow(1.0 + nZins, nCount));
                                nCount++;
                            }
                            SetError(nErr);
                        }
                    }
                    break;
                    default : SetError(errIllegalParameter); break;
                }
            }
        }
        PushDouble(nVal);
    }
}

#if defined(WIN) && defined(MSC)
#pragma optimize("",off)
#endif

void ScInterpreter::ScIKV()
{
    double fSchaetzwert;
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 1, 2 ) )
        return;
    if (nParamCount == 2)
        fSchaetzwert = GetDouble();
    else
        fSchaetzwert = 0.1;
    USHORT sPos = sp;                       // Stack-Position merken
    double fEps = 1.0;
    double x, xNeu, fWert, fZaehler, fNenner, nCount;
    if (fSchaetzwert == -1.0)
        x = 0.1;                            // default gegen Nulldivisionen
    else
        x = fSchaetzwert;                   // Startwert
    switch (GetStackType())
    {
        case svDoubleRef :
        break;
        case svDouble :
        case svSingleRef :
        default:
        {
            SetError(errIllegalParameter);
            return;
        }
    }
    const USHORT nIterationsMax = 20;
    USHORT nItCount = 0;
    ScRange aRange;
    while (fEps > SCdEpsilon && nItCount < nIterationsMax)
    {                                       // Newton-Verfahren:
        sp = sPos;                          // Stack zuruecksetzen
        nCount = 0.0;
        fZaehler = 0.0;
        fNenner = 0.0;
        USHORT nErr = 0;
        PopDoubleRef( aRange );
        ScValueIterator aValIter(pDok, aRange, glSubTotal);
        if (aValIter.GetFirst(fWert, nErr))
        {
            fZaehler +=           fWert / pow(1.0+x,nCount);
            fNenner  += -nCount * fWert / pow(1.0+x,nCount+1.0);
            nCount++;
            while ((nErr == 0) && aValIter.GetNext(fWert, nErr))
            {
                fZaehler +=           fWert / pow(1.0+x,nCount);
                fNenner  += -nCount * fWert / pow(1.0+x,nCount+1.0);
                nCount++;
            }
            SetError(nErr);
        }
        xNeu = x - fZaehler / fNenner;  // x(i+1) = x(i)-f(x(i))/f'(x(i))
        nItCount++;
        fEps = fabs(xNeu - x);
        x = xNeu;
    }
    if (fSchaetzwert == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;                        // auf Null normieren
    if (fEps < SCdEpsilon)
        PushDouble(x);
    else
    {
        SetError(errNoConvergence);
        PushInt(0);
    }
}
#if defined(WIN) && defined(MSC)
#pragma optimize("",on)
#endif

//----------------------- Finanzfunktionen ------------------------------------

double ScInterpreter::ScGetBw(double fZins, double fZzr, double fRmz,
                              double fZw, double fF)
{
    double fBw;
    if (fZins == 0.0)
        fBw = fZw + fRmz * fZzr;
    else if (fF > 0.0)
        fBw = (fZw * pow(1.0 + fZins, -fZzr))
                + (fRmz * (1.0 - pow(1.0 + fZins, -fZzr + 1.0)) / fZins)
                + fRmz;
    else
        fBw = (fZw * pow(1.0 + fZins, -fZzr))
                + (fRmz * (1.0 - pow(1.0 + fZins, -fZzr)) / fZins);
    return -fBw;
}

void ScInterpreter::ScBW()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    double nRmz, nZzr, nZins, nZw = 0, nFlag = 0;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetBw(nZins, nZzr, nRmz, nZw, nFlag));
}

void ScInterpreter::ScDIA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 4 ) )
    {
        double nZr = GetDouble();
        double nDauer = GetDouble();
        double nRest = GetDouble();
        double nWert = GetDouble();
        double nDia = ((nWert - nRest) * (nDauer - nZr + 1.0)) /
                      ((nDauer * (nDauer + 1.0)) / 2.0);
        PushDouble(nDia);
    }
}

double ScInterpreter::ScGetGDA(double fWert, double fRest, double fDauer,
                double fPeriode, double fFaktor)
{
    double fGda, fZins, fAlterWert, fNeuerWert;
    fZins = fFaktor / fDauer;
    if (fZins >= 1.0)
    {
        fZins = 1.0;
        if (fPeriode == 1.0)
            fAlterWert = fWert;
        else
            fAlterWert = 0.0;
    }
    else
        fAlterWert = fWert * pow(1.0 - fZins, fPeriode - 1.0);
    fNeuerWert = fWert * pow(1.0 - fZins, fPeriode);

    if (fNeuerWert < fRest)
        fGda = fAlterWert - fRest;
    else
        fGda = fAlterWert - fNeuerWert;
    if (fGda < 0.0)
        fGda = 0.0;
    return fGda;
}

void ScInterpreter::ScGDA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 4, 5 ) )
    {
        double nFaktor;
        if (nParamCount == 5)
            nFaktor = GetDouble();
        else
            nFaktor = 2.0;
        double nPeriode = GetDouble();
        double nDauer   = GetDouble();
        double nRest    = GetDouble();
        double nWert    = GetDouble();
        if (nWert < 0.0 || nRest < 0.0 || nFaktor <= 0.0 || nRest > nWert
                        || nPeriode < 1.0 || nPeriode > nDauer)
            SetIllegalParameter();
        else
            PushDouble(ScGetGDA(nWert, nRest, nDauer, nPeriode, nFaktor));
    }
}

void ScInterpreter::ScGDA2()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 5 ) )
        return ;
    double nMonate;
    if (nParamCount == 4)
        nMonate = 12.0;
    else
        nMonate = SolarMath::ApproxFloor(GetDouble());
    double nPeriode = GetDouble();
    double nDauer = GetDouble();
    double nRest = GetDouble();
    double nWert = GetDouble();
    if (nMonate < 1.0 || nMonate > 12.0 || nDauer > 1200.0 || nRest < 0.0 ||
        nPeriode > (nDauer + 1.0) || nRest > nWert || nWert < 0.0)
    {
        SetIllegalParameter();
        return;
    }
    double nAbRate = 1.0 - pow(nRest / nWert, 1.0 / nDauer);
    nAbRate = SolarMath::ApproxFloor((nAbRate * 1000.0) + 0.5) / 1000.0;
    double nErsteAbRate = nWert * nAbRate * nMonate / 12.0;
    double nGda2;
    if (SolarMath::ApproxFloor(nPeriode) == 1)
        nGda2 = nErsteAbRate;
    else
    {
        double nSummAbRate = nErsteAbRate;
        double nMin = nDauer;
        if (nMin > nPeriode) nMin = nPeriode;
        USHORT iMax = (USHORT)SolarMath::ApproxFloor(nMin);
        for (USHORT i = 2; i <= iMax; i++)
        {
            nGda2 = (nWert - nSummAbRate) * nAbRate;
            nSummAbRate += nGda2;
        }
        if (nPeriode > nDauer)
            nGda2 = ((nWert - nSummAbRate) * nAbRate * (12.0 - nMonate)) / 12.0;
    }
    PushDouble(nGda2);
}


double ScInterpreter::ScInterVDB(double fWert,double fRest,double fDauer,
                             double fDauer1,double fPeriode,double fFaktor)
{
    double fVdb=0;
    double fIntEnd   = SolarMath::ApproxCeil(fPeriode);
    ULONG nLoopEnd   = (ULONG) fIntEnd;

    double fTerm, fLia;
    double fRestwert = fWert - fRest;
    double fRestwert1 = fRestwert;
    BOOL bNowLia = FALSE;
    BOOL bFirstFlag=TRUE;
    BOOL b2Flag=TRUE;
    double fAbschlag=0;

    double fGda;
    ULONG i;
    fLia=0;
    for ( i = 1; i <= nLoopEnd; i++)
    {
        if(!bNowLia)
        {
            fGda = ScGetGDA(fWert, fRest, fDauer, (double) i, fFaktor);
            fLia = fRestwert/ (fDauer1 - (double) (i-1));

            if (fLia > fGda)
            {
                fTerm = fLia;
                bNowLia = TRUE;
            }
            else
            {
                fTerm = fGda;
                fRestwert -= fGda;
            }
        }
        else
        {
            fTerm = fLia;
        }

        if ( i == nLoopEnd)
            fTerm *= ( fPeriode + 1.0 - fIntEnd );

        fVdb += fTerm;
    }
    return fVdb;
}


inline double DblMin( double a, double b )
{
    return (a < b) ? a : b;
}

void ScInterpreter::ScVDB()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 5, 7 ) )
    {
        double fWert, fRest, fDauer, fAnfang, fEnde, fFaktor, fVdb;
        BOOL bFlag;
        if (nParamCount == 7)
            bFlag = GetBool();
        else
            bFlag = FALSE;
        if (nParamCount >= 6)
            fFaktor = GetDouble();
        else
            fFaktor = 2.0;
        fEnde   = GetDouble();
        fAnfang = GetDouble();
        fDauer  = GetDouble();
        fRest   = GetDouble();
        fWert   = GetDouble();
        if (fAnfang < 0.0 || fEnde < fAnfang || fEnde > fDauer || fWert < 0.0
                          || fRest > fWert || fFaktor <= 0.0)
            SetIllegalParameter();
        else
        {
            double fIntStart = SolarMath::ApproxFloor(fAnfang);
            double fIntEnd   = SolarMath::ApproxCeil(fEnde);
            ULONG nLoopStart = (ULONG) fIntStart;
            ULONG nLoopEnd   = (ULONG) fIntEnd;

            fVdb = 0.0;
            if (bFlag)
            {
                for (ULONG i = nLoopStart + 1; i <= nLoopEnd; i++)
                {
                    double fTerm = ScGetGDA(fWert, fRest, fDauer, (double) i, fFaktor);

                    //  Teilperioden am Anfang / Ende beruecksichtigen:
                    if ( i == nLoopStart+1 )
                        fTerm *= ( DblMin( fEnde, fIntStart + 1.0 ) - fAnfang );
                    else if ( i == nLoopEnd )
                        fTerm *= ( fEnde + 1.0 - fIntEnd );

                    fVdb += fTerm;
                }
            }
            else
            {

                double fDauer1=fDauer;
                double fPart;

                //@Die Frage aller Fragen: "Ist das hier richtig"
                if(!SolarMath::ApproxEqual(fAnfang,SolarMath::ApproxFloor(fAnfang)))
                {
                    if(fFaktor>1)
                    {
                        if(fAnfang>fDauer/2 || SolarMath::ApproxEqual(fAnfang,fDauer/2))
                        {
                            fPart=fAnfang-fDauer/2;
                            fAnfang=fDauer/2;
                            fEnde-=fPart;
                            fDauer1+=1;
                        }
                    }
                }

                fWert-=ScInterVDB(fWert,fRest,fDauer,fDauer1,fAnfang,fFaktor);
                fVdb=ScInterVDB(fWert,fRest,fDauer,fDauer-fAnfang,fEnde-fAnfang,fFaktor);
            }
        }
        PushDouble(fVdb);
    }
}

void ScInterpreter::ScLaufz()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nZukunft = GetDouble();
        double nGegenwart = GetDouble();
        double nZins = GetDouble();
        PushDouble(log(nZukunft / nGegenwart) / log(1.0 + nZins));
    }
}

void ScInterpreter::ScLIA()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nDauer = GetDouble();
        double nRest = GetDouble();
        double nWert = GetDouble();
        PushDouble((nWert - nRest) / nDauer);
    }
}

double ScInterpreter::ScGetRmz(double fZins, double fZzr, double fBw,
                       double fZw, double fF)
{
    double fRmz;
    if (fZins == 0.0)
        fRmz = (fBw + fZw) / fZzr;
    else
    {
        double fTerm = pow(1.0 + fZins, fZzr);
        if (fF > 0.0)
            fRmz = (fZw * fZins / (fTerm - 1.0)
                        + fBw * fZins / (1.0 - 1.0 / fTerm)) / (1.0+fZins);
        else
            fRmz = fZw * fZins / (fTerm - 1.0)
                        + fBw * fZins / (1.0 - 1.0 / fTerm);
    }
    return -fRmz;
}

void ScInterpreter::ScRMZ()
{
    double nZins, nZzr, nBw, nZw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetRmz(nZins, nZzr, nBw, nZw, nFlag));
}

void ScInterpreter::ScZGZ()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        double nZukunftswert = GetDouble();
        double nGegenwartswert = GetDouble();
        double nZeitraum = GetDouble();
        PushDouble(pow(nZukunftswert / nGegenwartswert, 1.0 / nZeitraum) - 1.0);
    }
}

double ScInterpreter::ScGetZw(double fZins, double fZzr, double fRmz,
                              double fBw, double fF)
{
    double fZw;
    if (fZins == 0.0)
        fZw = fBw + fRmz * fZzr;
    else
    {
        double fTerm = pow(1.0 + fZins, fZzr);
        if (fF > 0.0)
            fZw = fBw * fTerm + fRmz*(1.0 + fZins)*(fTerm - 1.0)/fZins;
        else
            fZw = fBw * fTerm + fRmz*(fTerm - 1.0)/fZins;
    }
    return -fZw;
}

void ScInterpreter::ScZW()
{
    double nZins, nZzr, nRmz, nBw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nBw   = GetDouble();
    nRmz  = GetDouble();
    nZzr  = GetDouble();
    nZins = GetDouble();
    PushDouble(ScGetZw(nZins, nZzr, nRmz, nBw, nFlag));
}

void ScInterpreter::ScZZR()
{
    double nZins, nRmz, nBw, nZw = 0, nFlag = 0;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;
    if (nParamCount == 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nRmz  = GetDouble();
    nZins = GetDouble();
    if (nZins == 0.0)
        PushDouble(-(nBw + nZw)/nRmz);
    else if (nFlag > 0.0)
        PushDouble(log(-(nZins*nZw-nRmz*(1.0+nZins))/(nZins*nBw+nRmz*(1.0+nZins)))
                  /log(1.0+nZins));
    else
        PushDouble(log(-(nZins*nZw-nRmz)/(nZins*nBw+nRmz))/log(1.0+nZins));
}

double ScInterpreter::GetZinsIterationEps(double fZzr, double fRmz, double fBw,
                                          double fZw, double fF, double& fSchaetzwert)
{
    double fEps = 1.0;
    double x, xNeu, fTerm1, fTerm2;
    if (fSchaetzwert == 0.0)
        x = 0.1;                                // default gegen Nulldivisionen
    else
        x = fSchaetzwert;                       // Startwert
    const USHORT nIterationsMax = 150;
    USHORT nCount = 0;
    while (fEps > SCdEpsilon && nCount < nIterationsMax)
    {                                           // Newton-Verfahren:
        if (x == 0.0)
            xNeu = x -
                    (fBw + fRmz*fZzr + fZw) /
                    (fBw*fZzr + fRmz*(fZzr*(fZzr-1.0) + 2*fF*fZzr)/2.0);
        else
        {
            fTerm1 = pow(1.0+x, fZzr-1);
            fTerm2 = fTerm1*(1.0+x);
            xNeu = x*(1.0 -                     // x(i+1) = x(i) - f(x(i)) / f'(x(i))
                 (x*fBw*fTerm2 + fRmz*(1.0+x*fF)*(fTerm2-1.0) + x*fZw) /
                 (x*x*fZzr*fBw*fTerm1 - fRmz*(fTerm2-1.0)
                                      + x*fRmz*(1.0+x*fF)*fZzr*fTerm1) );
        }
        nCount++;
        fEps = fabs(xNeu - x);
        x = xNeu;
    }
    if (fSchaetzwert == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;                                // auf Null normieren
    fSchaetzwert = x;                           //n Rueckgabe
    return fEps;
}

void ScInterpreter::ScZins()
{
    double nZw = 0, nRmz, nZzr, nBw, nFlag = 0, nSchaetzwert = 0.1, fEps;
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
        return;
    if (nParamCount == 6)
        nSchaetzwert = GetDouble();
    if (nParamCount >= 5)
        nFlag = GetDouble();
    if (nParamCount >= 4)
        nZw  = GetDouble();
    nBw  = GetDouble();
    nRmz = GetDouble();
    nZzr = GetDouble();
    if (nFlag == 0.0)
        fEps = GetZinsIterationEps(nZzr, nRmz, nBw, nZw, 0.0, nSchaetzwert);
    else
        fEps = GetZinsIterationEps(nZzr, nRmz, nBw, nZw, 1.0, nSchaetzwert);
    if (fEps >= SCdEpsilon)
    {
        SetError(errNoConvergence);
        nSchaetzwert = 0;
    }
    PushDouble(nSchaetzwert);
}

double ScInterpreter::ScGetZinsZ(double fZins, double fZr, double fZzr, double fBw,
                                 double fZw, double fF, double& fRmz)
{
    fRmz = ScGetRmz(fZins, fZzr, fBw, fZw, fF);     // fuer kapz auch bei fZr == 1
    double fZinsZ;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if (fZr == 1.0)
    {
        if (fF > 0.0)
            fZinsZ = 0.0;
        else
            fZinsZ = -fBw;
    }
    else
    {
        if (fF > 0.0)
            fZinsZ = ScGetZw(fZins, fZr-2.0, fRmz, fBw, 1.0) - fRmz;
        else
            fZinsZ = ScGetZw(fZins, fZr-1.0, fRmz, fBw, 0.0);
    }
    return fZinsZ * fZins;
}

void ScInterpreter::ScZinsZ()
{
    double nZins, nZr, nRmz, nZzr, nBw, nZw = 0, nFlag = 0;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        nFlag = GetDouble();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nZins = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        SetIllegalParameter();
    else
        PushDouble(ScGetZinsZ(nZins, nZr, nZzr, nBw, nZw, nFlag, nRmz));
}

void ScInterpreter::ScKapz()
{
    double nZins, nZr, nZzr, nBw, nZw = 0, nFlag = 0, nRmz, nZinsz;
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    BYTE nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 6 ) )
        return;
    if (nParamCount == 6)
        nFlag = GetDouble();
    if (nParamCount >= 5)
        nZw   = GetDouble();
    nBw   = GetDouble();
    nZzr  = GetDouble();
    nZr   = GetDouble();
    nZins = GetDouble();
    if (nZr < 1.0 || nZr > nZzr)
        SetIllegalParameter();
    else
    {
        nZinsz = ScGetZinsZ(nZins, nZr, nZzr, nBw, nZw, nFlag, nRmz);
        PushDouble(nRmz - nZinsz);
    }
}

void ScInterpreter::ScKumZinsZ()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fZins, fZzr, fBw, fAnfang, fEnde, fF, fRmz, fZinsZ;
        fF      = GetDouble();
        fEnde   = SolarMath::ApproxFloor(GetDouble());
        fAnfang = SolarMath::ApproxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fZins   = GetDouble();
        if (fAnfang < 1.0 || fEnde < fAnfang || fZins <= 0.0 ||
            fEnde > fZzr  || fZzr <= 0.0 || fBw <= 0.0)
            SetIllegalParameter();
        else
        {
            ULONG nAnfang = (ULONG) fAnfang;
            ULONG nEnde = (ULONG) fEnde ;
            fRmz = ScGetRmz(fZins, fZzr, fBw, 0.0, fF);
            fZinsZ = 0.0;
            if (nAnfang == 1)
            {
                if (fF <= 0.0)
                    fZinsZ = -fBw;
                nAnfang++;
            }
            for (ULONG i = nAnfang; i <= nEnde; i++)
            {
                if (fF > 0.0)
                    fZinsZ += ScGetZw(fZins, (double)(i-2), fRmz, fBw, 1.0) - fRmz;
                else
                    fZinsZ += ScGetZw(fZins, (double)(i-1), fRmz, fBw, 0.0);
            }
            fZinsZ *= fZins;
            PushDouble(fZinsZ);
        }
    }
}

void ScInterpreter::ScKumKapZ()
{
    nFuncFmtType = NUMBERFORMAT_CURRENCY;
    if ( MustHaveParamCount( GetByte(), 6 ) )
    {
        double fZins, fZzr, fBw, fAnfang, fEnde, fF, fRmz, fKapZ;
        fF      = GetDouble();
        fEnde   = SolarMath::ApproxFloor(GetDouble());
        fAnfang = SolarMath::ApproxFloor(GetDouble());
        fBw     = GetDouble();
        fZzr    = GetDouble();
        fZins   = GetDouble();
        if (fAnfang < 1.0 || fEnde < fAnfang || fZins <= 0.0 ||
            fEnde > fZzr  || fZzr <= 0.0 || fBw <= 0.0)
            SetIllegalParameter();
        else
        {
            fRmz = ScGetRmz(fZins, fZzr, fBw, 0.0, fF);
            fKapZ = 0.0;
            ULONG nAnfang = (ULONG) fAnfang;
            ULONG nEnde = (ULONG) fEnde;
            if (nAnfang == 1)
            {
                if (fF <= 0.0)
                    fKapZ = fRmz + fBw * fZins;
                else
                    fKapZ = fRmz;
                nAnfang++;
            }
            for (ULONG i = nAnfang; i <= nEnde; i++)
            {
                if (fF > 0.0)
                    fKapZ += fRmz - (ScGetZw(fZins, (double)(i-2), fRmz, fBw, 1.0) - fRmz) * fZins;
                else
                    fKapZ += fRmz - ScGetZw(fZins, (double)(i-1), fRmz, fBw, 0.0) * fZins;
            }
            PushDouble(fKapZ);
        }
    }
}

void ScInterpreter::ScEffektiv()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPerioden = GetDouble();
        double fNominal = GetDouble();
        if (fPerioden < 1.0 || fNominal <= 0.0)
            SetIllegalParameter();
        else
        {
            fPerioden = SolarMath::ApproxFloor(fPerioden);
            PushDouble(pow(1.0 + fNominal/fPerioden, fPerioden) - 1.0);
        }
    }
}

void ScInterpreter::ScNominal()
{
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fPerioden = GetDouble();
        double fEffektiv = GetDouble();
        if (fPerioden < 1.0 || fEffektiv <= 0.0)
            SetIllegalParameter();
        else
        {
            fPerioden = SolarMath::ApproxFloor(fPerioden);
            PushDouble( (pow(fEffektiv + 1.0, 1.0 / fPerioden) - 1.0) * fPerioden );
        }
    }
}

void ScInterpreter::ScMod()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nVal2 = GetDouble();
        double nVal1 = GetDouble();
        PushDouble(nVal1 - (SolarMath::ApproxFloor(nVal1 / nVal2) * nVal2));
    }
}

void ScInterpreter::ScBackSolver()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        BOOL bRet = FALSE;
        ScAddress aVAdr, aFAdr;
        double nVal = GetDouble();
        PopSingleRef( aFAdr );
        PopSingleRef( aVAdr );
        if (nGlobalError == 0)
        {
            ScBaseCell* pVCell = GetCell( aVAdr );
            // CELLTYPE_NOTE: kein Value aber von Formel referiert
            BOOL bTempCell = (!pVCell || pVCell->GetCellType() == CELLTYPE_NOTE);
            ScBaseCell* pFCell = GetCell( aFAdr );
            if ( ((pVCell && pVCell->GetCellType() == CELLTYPE_VALUE) || bTempCell)
                && pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA )
            {
                ScRange aVRange( aVAdr, aVAdr );    // fuer SetDirty
                double nSaveVal;
                ScPostIt aNote;
                BOOL bHasNote;
                if ( bTempCell )
                {
                    if ( bHasNote = (pVCell != NULL) )
                        bHasNote = pVCell->GetNote( aNote );
                    nSaveVal = 0.0;
                    pVCell = new ScValueCell( nSaveVal );
                    pDok->PutCell( aVAdr, pVCell );
                }
                else
                    nSaveVal = GetCellValue( aVAdr, pVCell );
                const USHORT nMaxIter = 100;
                const double nEps = 1E-10;
                const double nDelta = 1E-3;
                double nBestX = nSaveVal;
                double nBestF, xn1, fn1;
                ScFormulaCell* pFormula = (ScFormulaCell*) pFCell;
                ScValueCell* pValue = (ScValueCell*) pVCell;
                pFormula->Interpret();
                fn1 = pFormula->GetValue();
                fn1 -= nVal;
                xn1 = nBestX;
                nBestF = fabs(fn1);
                if (nBestF < nDelta)
                    bRet = TRUE;
                double xn = xn1 + nEps;
                double fn = fn1;
                double fs;
                USHORT i = 0;
                                                // Nach der Regula Falsi Methode
                while (!bRet && (i < nMaxIter))
                {
                    i++;
                    pValue->SetValue(xn);
                    pDok->SetDirty( aVRange );
                    pFormula->Interpret();
                    fn = pFormula->GetValue();
                    fn -= nVal;
                    if (fabs(fn) < nDelta)
                    {
                        nBestX = xn;
                        bRet = TRUE;
                    }
                    else
                    {
                        if (fabs(fn) + nDelta < nBestF)
                        {
                            nBestX = xn;
                            nBestF = fabs(fn);
                        }
                        if ((xn1 - xn) != 0)
                        {
                            fs = (fn1 - fn) / (xn1 - xn);
                            if (fabs(fs) < nEps)
                                if (fs < 0.0)
                                    fs = -nEps;
                                else
                                    fs = nEps;
                        }
                        else
                            fs = nEps;
                        xn1 = xn;
                        fn1 = fn;
                        xn = xn - (fn / fs);
                    }
                }
                double nX = SolarMath::ApproxFloor((nBestX / nDelta) + 0.5) * nDelta;
                if ( bRet )
                {
                    pValue->SetValue( nX );
                    pDok->SetDirty( aVRange );
                    pFormula->Interpret();
                    if ( fabs( pFormula->GetValue() - nVal ) > fabs( fn ) )
                        nX = nBestX;
                }
                if ( bTempCell )
                {
                    if ( bHasNote )
                        pVCell = new ScNoteCell( aNote );
                    else
                        pVCell = NULL;
                    pDok->PutCell( aVAdr, pVCell );
                }
                else
                    pValue->SetValue(nSaveVal);
                pDok->SetDirty( aVRange );
                pFormula->Interpret();
                if (!bRet)
                    SetError(NOVALUE);
                PushDouble(nX);
            }
            else
            {
                if (!bRet)
                    SetError(NOVALUE);
                PushInt(0);         // falsche Zelltypen
            }
        }
        else
        {
            if (!bRet)
                SetError(NOVALUE);
            PushInt(0);             // nGlobalError
        }
    }
}

void ScInterpreter::ScIntersect()
{
    USHORT nCol11, nRow11, nTab11, nCol21, nRow21, nTab21,
           nCol12, nRow12, nTab12, nCol22, nRow22, nTab22,
           nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    BYTE eStackVar = GetStackType();
    if (eStackVar == svDoubleRef)
        PopDoubleRef(nCol11, nRow11, nTab11, nCol21, nRow21, nTab21);
    else if (eStackVar == svSingleRef)
    {
        PopSingleRef(nCol11, nRow11, nTab11);
        nCol21 = nCol11;
        nRow21 = nRow11;
        nTab21 = nTab11;
    }
    else
    {
        SetError(errNoRef);
        PushInt(0);
        return;
    }
    eStackVar = GetStackType();
    if (eStackVar == svDoubleRef)
        PopDoubleRef(nCol12, nRow12, nTab12, nCol22, nRow22, nTab22);
    else if (eStackVar == svSingleRef)
    {
        PopSingleRef(nCol12, nRow12, nTab12);
        nCol22 = nCol12;
        nRow22 = nRow12;
        nTab22 = nTab12;
    }
    else
    {
        SetError(errNoRef);
        PushInt(0);
        return;
    }
    nCol1 = Max(nCol11, nCol12);
    nRow1 = Max(nRow11, nRow12);
    nTab1 = Max(nTab11, nTab12);
    nCol2 = Min(nCol21, nCol22);
    nRow2 = Min(nRow21, nRow22);
    nTab2 = Min(nTab21, nTab22);
    if (nCol2 < nCol1 || nRow2 < nRow1 || nTab2 < nTab1)
    {
        SetError(errNoRef);
        PushInt(0);
    }
    else if (nCol2 == nCol1 && nRow2 == nRow1 && nTab2 == nTab1)
        PushSingleRef(nCol1, nRow1, nTab1);
    else
        PushDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
}


void ScInterpreter::ScCurrent()
{
    switch ( GetStackType() )
    {
        case svDouble :
        {
            double nVal = PopDouble();
            PushDouble( nVal );
            PushDouble( nVal );
        }
        break;
        case svString :
        {
            const sal_Unicode* pStr = PopString();
            PushString( pStr );
            PushString( pStr );
        }
        break;
        case svDoubleRef :
        case svSingleRef :
        {
            ScAddress aAdr;
            if ( !PopDoubleRefOrSingleRef( aAdr ) )
            {
                PushInt(0);
                break;
            }
            ScBaseCell* pCell = GetCell( aAdr );
            // NoteCell entsteht auch durch Referenz auf leere Zelle
            if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
            {
                if ( HasCellValueData( pCell ) )
                {
                    double nVal = GetCellValue( aAdr, pCell );
                    PushDouble( nVal );
                    PushDouble( nVal );
                }
                else
                {
                    String aStr;
                    GetCellString( aStr, pCell );
                    PushStringObject( aStr );
                    PushStringObject( aStr );
                }
            }
            else
            {
                PushSingleRef( aAdr.Col(), aAdr.Row(), aAdr.Tab() );
                PushSingleRef( aAdr.Col(), aAdr.Row(), aAdr.Tab() );
            }
        }
        break;
        default:
            SetIllegalParameter();
    }
}

void ScInterpreter::ScStyle()
{
    BYTE nParamCount = GetByte();
    if (nParamCount >= 1 && nParamCount <= 3)
    {
        String aStyle2;                             // Vorlage nach Timer
        if (nParamCount >= 3)
            aStyle2 = GetString();
        long nTimeOut = 0;                          // Timeout
        if (nParamCount >= 2)
            nTimeOut = (long)(GetDouble()*1000.0);
        String aStyle1 = GetString();               // Vorlage fuer sofort

        if (nTimeOut < 0)
            nTimeOut = 0;

        //
        //  Request ausfuehren, um Vorlage anzuwenden
        //

        if ( !pDok->IsClipOrUndo() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
            {
                //! notify object shell directly

                ScRange aRange(aPos);
                ScAutoStyleHint aHint( aRange, aStyle1, nTimeOut, aStyle2 );
                pShell->Broadcast( aHint );
            }
        }

        PushDouble(0.0);
    }
    else
        SetIllegalParameter();
}

ScDdeLink* lcl_GetDdeLink( SvLinkManager* pLinkMgr,
                                const String& rA, const String& rT, const String& rI, BYTE nM )
{
    USHORT nCount = pLinkMgr->GetLinks().Count();
    for (USHORT i=0; i<nCount; i++ )
    {
        SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
        if (pBase->ISA(ScDdeLink))
        {
            ScDdeLink* pLink = (ScDdeLink*)pBase;
            if ( pLink->GetAppl() == rA &&
                 pLink->GetTopic() == rT &&
                 pLink->GetItem() == rI &&
                 pLink->GetMode() == nM )
                return pLink;
        }
    }

    return NULL;
}

void ScInterpreter::ScDde()
{
    //  Applikation, Datei, Bereich
    //  Application, Topic, Item

    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 3, 4 ) )
    {
        BYTE nMode = SC_DDE_DEFAULT;
        if (nParamCount == 4)
            nMode = (BYTE) SolarMath::ApproxFloor(GetDouble());
        String aItem  = GetString();
        String aTopic = GetString();
        String aAppl  = GetString();

        if (nMode < SC_DDE_DEFAULT || nMode > SC_DDE_TEXT)
            nMode = SC_DDE_DEFAULT;

            //  Nach dem Laden muss neu interpretiert werden (Verknuepfungen aufbauen)

        if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
            pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();

            //  solange der Link nicht ausgewertet ist, Idle abklemmen
            //  (um zirkulaere Referenzen zu vermeiden)

        BOOL bOldDis = pDok->IsIdleDisabled();
        pDok->DisableIdle( TRUE );

            //  Link-Objekt holen / anlegen

        SvLinkManager* pLinkMgr = pDok->GetLinkManager();
        ScDdeLink* pLink = lcl_GetDdeLink( pLinkMgr, aAppl, aTopic, aItem, nMode );

        //! Dde-Links (zusaetzlich) effizienter am Dokument speichern !!!!!
        //      ScDdeLink* pLink = pDok->GetDdeLink( aAppl, aTopic, aItem );

        BOOL bWasError = ( pMyFormulaCell->GetCode()->GetError() != 0 );

        if (!pLink)
        {
            pLink = new ScDdeLink( pDok, aAppl, aTopic, aItem, nMode );
            pLinkMgr->InsertDDELink( *pLink, aAppl, aTopic, aItem );
            if ( pLinkMgr->GetLinks().Count() == 1 )                    // erster ?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enablen
            }

                                    //! asynchron auswerten ???
            pLink->TryUpdate();     //  TryUpdate ruft Update nicht mehrfach auf

            // StartListening erst nach dem Update, sonst circular reference
            pMyFormulaCell->StartListening( *pLink, TRUE );
        }
        else
        {
            if ( !pMyFormulaCell->IsListening( *pLink ) )
                pMyFormulaCell->StartListening( *pLink, TRUE );
        }

        //  Wenn aus dem Reschedule beim Ausfuehren des Links ein Fehler
        //  (z.B. zirkulaere Referenz) entstanden ist, der vorher nicht da war,
        //  das Fehler-Flag zuruecksetzen:

        if ( pMyFormulaCell->GetCode()->GetError() && !bWasError )
            pMyFormulaCell->GetCode()->SetError(0);

            //  Wert abfragen

        const ScMatrix* pLinkMat = pLink->GetResult();
        if (pLinkMat)
        {
            USHORT nC, nR, nMatInd;
            pLinkMat->GetDimensions(nC, nR);
            ScMatrix* pNewMat = GetNewMat( nC, nR, nMatInd );
            if (pNewMat)
            {
                pLinkMat->MatCopy(*pNewMat);        // kopieren
                PushMatrix( pNewMat );
                nRetMat = nMatInd;
            }
            // sonst Fehler schon in GetNewMat gesetzt
        }
        else
            SetNV();

        pDok->DisableIdle( bOldDis );
    }
}

void ScInterpreter::ScBase()
{   // Value, Base [, MinLen]
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 2, 3 ) )
    {
        static const sal_Unicode __FAR_DATA pDigits[] = {
            '0','1','2','3','4','5','6','7','8','9',
            'A','B','C','D','E','F','G','H','I','J','K','L','M',
            'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
            0
        };
        static const int nDigits = (sizeof(pDigits)/sizeof(sal_Unicode))-1;
        USHORT nMinLen;
        if ( nParamCount == 3 )
        {
            double fLen = SolarMath::ApproxFloor( GetDouble() );
            if ( 1 <= fLen && fLen < MAXSTRLEN )
                nMinLen = (USHORT) fLen;
            else
                nMinLen = 0;    // Error
        }
        else
            nMinLen = 1;
        double fBase = SolarMath::ApproxFloor( GetDouble() );
        double fVal = SolarMath::ApproxFloor( GetDouble() );

        if ( !nGlobalError && nMinLen && 2 <= fBase && fBase <= nDigits && 0 <= fVal )
        {
            const size_t nBuf = MAXSTRLEN+1;
            sal_Unicode pBuf[nBuf];
            for ( size_t j=0; j<nBuf;  j++ )
            {
                pBuf[j] = '0';
            }
            sal_Unicode* p = pBuf + nBuf - 1;
            *p = 0;
            if ( fVal <= (ULONG)(~0) )
            {
                ULONG nVal = (ULONG) fVal;
                ULONG nBase = (ULONG) fBase;
                while ( nVal && p > pBuf )
                {
                    *--p = pDigits[ nVal % nBase ];
                    nVal /= nBase;
                }
                fVal = (double) nVal;
            }
            else
            {
                BOOL bDirt = FALSE;
                while ( fVal && p > pBuf )
                {
//! mit fmod Rundungsfehler ab 2**48
//                  double fDig = SolarMath::ApproxFloor( fmod( fVal, fBase ) );
// so ist es etwas besser
                    double fInt = SolarMath::ApproxFloor( fVal / fBase );
                    double fMult = fInt * fBase;
#ifdef DEBUG
                    // #53943# =BASIS(1e308;36) => GPF mit
                    // nDig = (size_t) SolarMath::ApproxFloor( fVal - fMult );
                    // trotz vorheriger Pruefung ob fVal >= fMult
                    double fDebug1 = fVal - fMult;
                    // fVal    := 7,5975311883090e+290
                    // fMult   := 7,5975311883090e+290
                    // fDebug1 := 1,3848924157003e+275  <- RoundOff-Error
                    // fVal != fMult, aber: SolarMath::ApproxEqual( fVal, fMult ) == TRUE
                    double fDebug2 = SolarMath::ApproxSub( fVal, fMult );
                    // und SolarMath::ApproxSub( fVal, fMult ) == 0
                    double fDebug3 = ( fInt ? fVal / fInt : 0.0 );
                    // Nach dem strange fDebug1 und fVal < fMult  ist eigentlich
                    // fDebug2 == fBase, trotzdem wird das mit einem Vergleich
                    // nicht erkannt, dann schlaegt bDirt zu und alles wird wieder gut..
#endif
                    size_t nDig;
                    if ( fVal < fMult )
                    {   // da ist was gekippt
                        bDirt = TRUE;
                        nDig = 0;
                    }
                    else
                    {
                        double fDig = SolarMath::ApproxFloor( SolarMath::ApproxSub( fVal, fMult ) );
                        if ( bDirt )
                        {
                            bDirt = FALSE;
                            --fDig;
                        }
                        if ( fDig <= 0.0 )
                            nDig = 0;
                        else if ( fDig >= fBase )
                            nDig = ((size_t) fBase) - 1;
                        else
                            nDig = (size_t) fDig;
                    }
                    *--p = pDigits[ nDig ];
                    fVal = fInt;
                }
            }
            if ( fVal )
            {
                SetError( errStringOverflow );
                PushInt(0);
            }
            else
            {
                if ( nBuf - (p - pBuf) <= nMinLen )
                    p = pBuf + nBuf - 1 - nMinLen;
                PushString( p );
            }
        }
        else
            SetIllegalArgument();
    }
}


void ScInterpreter::ScDecimal()
{   // Text, Base
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fBase = SolarMath::ApproxFloor( GetDouble() );
        String aStr( GetString() );
        if ( !nGlobalError && 2 <= fBase && fBase <= 36 )
        {
            double fVal = 0.0;
            int nBase = (int) fBase;
            register const sal_Unicode* p = aStr.GetBuffer();
            while ( *p == ' ' || *p == '\t' )
                p++;        // strip leading white space
            if ( nBase == 16 )
            {   // evtl. hex-prefix strippen
                if ( *p == 'x' || *p == 'X' )
                    p++;
                else if ( *p == '0' && (*(p+1) == 'x' || *(p+1) == 'X') )
                    p += 2;
            }
            while ( *p )
            {
                int n;
                if ( '0' <= *p && *p <= '9' )
                    n = *p - '0';
                else if ( 'A' <= *p && *p <= 'Z' )
                    n = 10 + (*p - 'A');
                else if ( 'a' <= *p && *p <= 'z' )
                    n = 10 + (*p - 'a');
                else
                    n = nBase;
                if ( nBase <= n )
                {
                    if ( *(p+1) == 0 &&
                            ( (nBase ==  2 && (*p == 'b' || *p == 'B'))
                            ||(nBase == 16 && (*p == 'h' || *p == 'H')) )
                        )
                        ;       // 101b und F00Dh sind ok
                    else
                    {
                        SetIllegalArgument();
                        return ;
                    }
                }
                else
                    fVal = fVal * fBase + n;
                p++;

            }
            PushDouble( fVal );
        }
        else
            SetIllegalArgument();
    }
}


void ScInterpreter::ScConvert()
{   // Value, FromUnit, ToUnit
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        String aToUnit( GetString() );
        String aFromUnit( GetString() );
        double fVal = GetDouble();
        if ( nGlobalError )
            SetIllegalArgument();
        else
        {   // erst die angegebene Reihenfolge suchen, wenn nicht gefunden den Kehrwert
            double fConv;
            if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aFromUnit, aToUnit ) )
                PushDouble( fVal * fConv );
            else if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aToUnit, aFromUnit ) )
                PushDouble( fVal / fConv );
            else
                SetNV();
        }
    }
}




