/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_svx/linkmgr.hxx>
#include <bf_svtools/zforlist.hxx>
#include <string.h>
#include <math.h>
#include <sal/macros.h>

#include "interpre.hxx"
#include "bf_sc.hrc"
#include "ddelink.hxx"
#include "scmatrix.hxx"
#include "dociter.hxx"
#include "unitconv.hxx"
#include "globstr.hrc"
#include "hints.hxx"

namespace binfilter {

// STATIC DATA -----------------------------------------------------------

#define D_TIMEFACTOR              86400.0
#define SCdEpsilon                1.0E-7

//-----------------------------------------------------------------------------
// Datum und Zeit
//-----------------------------------------------------------------------------

/*N*/ double ScInterpreter::GetDate(INT16 nYear, INT16 nMonth, INT16 nDay)
/*N*/ {
/*N*/ 	if ( nYear < 100 )
/*?*/ 		nYear = pFormatter->ExpandTwoDigitYear( nYear );
/*N*/ 	INT16 nY, nM;
/*N*/ 	if (nMonth > 0)
/*N*/ 	{
/*N*/ 		nY = nYear + (nMonth-1) / 12;
/*N*/ 		nM = ((nMonth-1) % 12) + 1;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		nY = nYear + (nMonth-12) / 12;
/*?*/ 		nM = 12 - (-nMonth) % 12;
/*N*/ 	}
/*N*/ 	Date aDate(1, nM, nY);
/*N*/ 	aDate += nDay - 1;
/*N*/ 	if (aDate.IsValid())
/*N*/ 		return (double) (aDate - *(pFormatter->GetNullDate()));
/*N*/ 	else
/*N*/ 	{
/*?*/ 		SetError(errNoValue);
/*?*/ 		return 0;
/*N*/ 	}
/*N*/ }

//-----------------------------------------------------------------------------
// Funktionen
//-----------------------------------------------------------------------------

/*N*/ void ScInterpreter::ScGetActDate()
/*N*/ {
/*N*/ 	nFuncFmtType = NUMBERFORMAT_DATE;
/*N*/ 	Date aActDate;
/*N*/ 	long nDiff = aActDate - *(pFormatter->GetNullDate());
/*N*/ 	PushDouble((double) nDiff);
/*N*/ }

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
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetYear() );
}

void ScInterpreter::ScGetMonth()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetMonth() );
}

/*N*/ void ScInterpreter::ScGetDay()
/*N*/ {
/*N*/ 	Date aDate = *(pFormatter->GetNullDate());
/*N*/ 	aDate += (long)::rtl::math::approxFloor(GetDouble());
/*N*/ 	PushDouble((double) aDate.GetDay());
/*N*/ }

void ScInterpreter::ScGetMin()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);		// Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) % 3600;
    PushDouble( (double) (nVal/60) );
}

void ScInterpreter::ScGetSec()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);		// Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) % 60;
    PushDouble( (double) nVal );
}

void ScInterpreter::ScGetHour()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);		// Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) / 3600;
    PushDouble((double) nVal);
}

void ScInterpreter::ScGetDateValue()
{
    String aInputString = GetString();
    sal_uInt32 nFIndex = 0;					// damit default Land/Spr.
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

/*N*/ void ScInterpreter::ScGetDayOfWeek()
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	if ( MustHaveParamCount( nParamCount, 1, 2 ) )
/*N*/ 	{
/*N*/ 		short nFlag;
/*N*/ 		if (nParamCount == 2)
/*?*/ 			nFlag = (short) ::rtl::math::approxFloor(GetDouble());
/*N*/ 		else
/*N*/ 			nFlag = 1;
/*N*/
/*N*/ 		Date aDate = *(pFormatter->GetNullDate());
/*N*/ 		aDate += (long)::rtl::math::approxFloor(GetDouble());
/*N*/ 		int nVal = (int) aDate.GetDayOfWeek();
/*N*/ 		if (nFlag == 1)
/*N*/ 		{
/*N*/ 			if (nVal == 6)
/*N*/ 				nVal = 1;
/*N*/ 			else
/*N*/ 				nVal += 2;
/*N*/ 		}
/*N*/ 		else if (nFlag == 2)
/*N*/ 			nVal += 1;
/*N*/ 		PushInt( nVal );
/*N*/ 	}
/*N*/ }

/*N*/ void ScInterpreter::ScGetWeekOfYear()
/*N*/ {
/*N*/ 	if ( MustHaveParamCount( GetByte(), 2 ) )
/*N*/ 	{
/*N*/ 		short nFlag = (short) ::rtl::math::approxFloor(GetDouble());
/*N*/
/*N*/ 		Date aDate = *(pFormatter->GetNullDate());
/*N*/ 		aDate += (long)::rtl::math::approxFloor(GetDouble());
/*N*/ 		PushInt( (int) aDate.GetWeekOfYear( nFlag == 1 ? SUNDAY : MONDAY ));
/*N*/ 	}
/*N*/ }

/*N*/ void ScInterpreter::ScEasterSunday()
/*N*/ {
/*N*/ 	nFuncFmtType = NUMBERFORMAT_DATE;
/*N*/ 	if ( MustHaveParamCount( GetByte(), 1 ) )
/*N*/ 	{
/*N*/ 		INT16 nDay, nMonth, nYear;
/*N*/ 		nYear = (INT16) ::rtl::math::approxFloor( GetDouble() );
/*N*/ 		if ( nYear < 100 )
/*?*/ 			nYear = pFormatter->ExpandTwoDigitYear( nYear );
/*N*/ 		// don't worry, be happy :)
/*N*/ 		int B,C,D,E,F,G,H,I,K,L,M,N,O;
/*N*/ 		N = nYear % 19;
/*N*/ 		B = int(nYear / 100);
/*N*/ 		C = nYear % 100;
/*N*/ 		D = int(B / 4);
/*N*/ 		E = B % 4;
/*N*/ 		F = int((B + 8) / 25);
/*N*/ 		G = int((B - F + 1) / 3);
/*N*/ 		H = (19 * N + B - D - G + 15) % 30;
/*N*/ 		I = int(C / 4);
/*N*/ 		K = C % 4;
/*N*/ 		L = (32 + 2 * E + 2 * I - H - K) % 7;
/*N*/ 		M = int((N + 11 * H + 22 * L) / 451);
/*N*/ 		O = H + L - 7 * M + 114;
/*N*/ 		nDay = O % 31 + 1;
/*N*/ 		nMonth = int(O / 31);
/*N*/ 		PushDouble( GetDate( nYear, nMonth, nDay ) );
/*N*/ 	}
/*N*/ }

void ScInterpreter::ScGetDate()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        INT16 nDay   = (INT16) ::rtl::math::approxFloor(GetDouble());
        INT16 nMonth = (INT16) ::rtl::math::approxFloor(GetDouble());
        INT16 nYear  = (INT16) ::rtl::math::approxFloor(GetDouble());
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
            aDate1 += (long) ::rtl::math::approxFloor(nDate1);
            Date aDate2 = *(pFormatter->GetNullDate());
            aDate2 += (long) ::rtl::math::approxFloor(nDate2);
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
                        aDate2 += (ULONG) 1;			// -> 1.
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
    sal_uInt32 nFIndex = 0;					// damit default Land/Spr.
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

/*N*/ void ScInterpreter::ScPlusMinus()
/*N*/ {
/*N*/ 	double nVal = GetDouble();
/*N*/ 	short n = 0;
/*N*/ 	if (nVal < 0.0)
/*N*/ 		n = -1;
/*N*/ 	else if (nVal > 0.0)
/*N*/ 		n = 1;
/*N*/ 	PushInt( n );
/*N*/ }

void ScInterpreter::ScAbs()
{
    PushDouble(fabs(GetDouble()));
}

/*N*/ void ScInterpreter::ScInt()
/*N*/ {
/*N*/ 	PushDouble(::rtl::math::approxFloor(GetDouble()));
/*N*/ }


/*N*/ void ScInterpreter::RoundNumber( rtl_math_RoundingMode eMode )
/*N*/ {
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	if ( MustHaveParamCount( nParamCount, 1, 2 ) )
/*N*/ 	{
/*N*/ 		double fVal = 0.0;
/*N*/ 		if (nParamCount == 1)
/*N*/ 			fVal = ::rtl::math::round( GetDouble(), 0, eMode );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			INT32 nDec = (INT32) ::rtl::math::approxFloor(GetDouble());
/*N*/ 			if( nDec < -20 || nDec > 20 )
/*N*/ 				SetIllegalArgument();
/*N*/ 			else
/*N*/ 				fVal = ::rtl::math::round( GetDouble(), (short)nDec, eMode );
/*N*/ 		}
/*N*/ 		PushDouble(fVal);
/*N*/ 	}
/*N*/ }

void ScInterpreter::ScRound()
{
    RoundNumber( rtl_math_RoundingMode_Corrected );
}

void ScInterpreter::ScRoundDown()
{
    RoundNumber( rtl_math_RoundingMode_Down );
}

/*N*/ void ScInterpreter::ScRoundUp()
/*N*/ {
/*N*/ 	RoundNumber( rtl_math_RoundingMode_Up );
/*N*/ }

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
                PushDouble(::rtl::math::approxFloor(fVal/fDec) * fDec);
            else
                PushDouble(::rtl::math::approxCeil(fVal/fDec) * fDec);
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
                PushDouble(::rtl::math::approxCeil(fVal/fDec) * fDec);
            else
                PushDouble(::rtl::math::approxFloor(fVal/fDec) * fDec);
        }
    }
}

void ScInterpreter::ScEven()
{
    double fVal = GetDouble();
    if (fVal < 0.0)
        PushDouble(::rtl::math::approxFloor(fVal/2.0) * 2.0);
    else
        PushDouble(::rtl::math::approxCeil(fVal/2.0) * 2.0);
}

void ScInterpreter::ScOdd()
{
    double fVal = GetDouble();
    if (fVal >= 0.0)
    {
        fVal = ::rtl::math::approxCeil(fVal);
        if (fmod(fVal, 2.0) == 0.0)
            fVal += 1.0;
    }
    else
    {
        fVal = ::rtl::math::approxFloor(fVal);
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
            double	nCount = 1.0;
            double	nZins = GetDouble();
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
    USHORT sPos = sp;						// Stack-Position merken
    double fEps = 1.0;
    double x, xNeu, fWert, fZaehler, fNenner, nCount;
    if (fSchaetzwert == -1.0)
        x = 0.1;							// default gegen Nulldivisionen
    else
        x = fSchaetzwert;					// Startwert
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
    {										// Newton-Verfahren:
        sp = sPos;							// Stack zuruecksetzen
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
        xNeu = x - fZaehler / fNenner;	// x(i+1) = x(i)-f(x(i))/f'(x(i))
        nItCount++;
        fEps = fabs(xNeu - x);
        x = xNeu;
    }
    if (fSchaetzwert == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;						// auf Null normieren
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


void ScInterpreter::ScMIRR()
{	// range_of_values ; rate_invest ; rate_reinvest
    nFuncFmtType = NUMBERFORMAT_PERCENT;
    if( MustHaveParamCount( GetByte(), 3 ) )
    {
        double fRate1_reinvest = GetDouble() + 1;
        double fNPV_reinvest = 0.0;
        double fPow_reinvest = 1.0;

        double fRate1_invest = GetDouble() + 1;
        double fNPV_invest = 0.0;
        double fPow_invest = 1.0;

        ScRange aRange;
        PopDoubleRef( aRange );

        if( nGlobalError )
            SetIllegalParameter();
        else
        {
            ScValueIterator aValIter( pDok, aRange, glSubTotal );
            double fCellValue;
            ULONG nCount = 0;
            USHORT nIterError = 0;

            BOOL bLoop = aValIter.GetFirst( fCellValue, nIterError );
            while( bLoop )
            {
                if( fCellValue > 0.0 )			// reinvestments
                    fNPV_reinvest += fCellValue * fPow_reinvest;
                else if( fCellValue < 0.0 )		// investments
                    fNPV_invest += fCellValue * fPow_invest;
                fPow_reinvest /= fRate1_reinvest;
                fPow_invest /= fRate1_invest;
                nCount++;

                bLoop = aValIter.GetNext( fCellValue, nIterError );
            }
            if( nIterError )
                SetError( nIterError );
            else
            {
                double fResult = -fNPV_reinvest / fNPV_invest;
                fResult *= pow( fRate1_reinvest, double(nCount - 1) );
                fResult = pow( fResult, 1.0 / (nCount - 1) );
                PushDouble( fResult - 1.0 );
            }
        }
    }
}


void ScInterpreter::ScISPMT()
{	// rate ; period ; total_periods ; invest
    if( MustHaveParamCount( GetByte(), 4 ) )
    {
        double fInvest = GetDouble();
        double fTotal = GetDouble();
        double fPeriod = GetDouble();
        double fRate = GetDouble();

        if( nGlobalError )
            SetIllegalParameter();
        else
            PushDouble( fInvest * fRate * (fPeriod / fTotal - 1.0) );
    }
}


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
        nMonate = ::rtl::math::approxFloor(GetDouble());
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
    nAbRate = ::rtl::math::approxFloor((nAbRate * 1000.0) + 0.5) / 1000.0;
    double nErsteAbRate = nWert * nAbRate * nMonate / 12.0;
    double nGda2;
    if (::rtl::math::approxFloor(nPeriode) == 1)
        nGda2 = nErsteAbRate;
    else
    {
        double nSummAbRate = nErsteAbRate;
        double nMin = nDauer;
        if (nMin > nPeriode) nMin = nPeriode;
        USHORT iMax = (USHORT)::rtl::math::approxFloor(nMin);
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
    double fIntEnd   = ::rtl::math::approxCeil(fPeriode);
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
        double fWert, fRest, fDauer, fAnfang, fEnde, fFaktor, fVdb = 0.0;
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
            double fIntStart = ::rtl::math::approxFloor(fAnfang);
            double fIntEnd   = ::rtl::math::approxCeil(fEnde);
            ULONG nLoopStart = (ULONG) fIntStart;
            ULONG nLoopEnd   = (ULONG) fIntEnd;

            fVdb = 0.0;
            if (bFlag)
            {
                for (ULONG i = nLoopStart + 1; i <= nLoopEnd; i++)
                {
                    double fTerm = ScGetGDA(fWert, fRest, fDauer, (double) i, fFaktor);

                    //	Teilperioden am Anfang / Ende beruecksichtigen:
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
                if(!::rtl::math::approxEqual(fAnfang,::rtl::math::approxFloor(fAnfang)))
                {
                    if(fFaktor>1)
                    {
                        if(fAnfang>fDauer/2 || ::rtl::math::approxEqual(fAnfang,fDauer/2))
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
        x = 0.1;								// default gegen Nulldivisionen
    else
        x = fSchaetzwert;						// Startwert
    const USHORT nIterationsMax = 150;
    USHORT nCount = 0;
    while (fEps > SCdEpsilon && nCount < nIterationsMax)
    {											// Newton-Verfahren:
        if (x == 0.0)
            xNeu = x -
                    (fBw + fRmz*fZzr + fZw) /
                    (fBw*fZzr + fRmz*(fZzr*(fZzr-1.0) + 2*fF*fZzr)/2.0);
        else
        {
            fTerm1 = pow(1.0+x, fZzr-1);
            fTerm2 = fTerm1*(1.0+x);
            xNeu = x*(1.0 -						// x(i+1) = x(i) - f(x(i)) / f'(x(i))
                 (x*fBw*fTerm2 + fRmz*(1.0+x*fF)*(fTerm2-1.0) + x*fZw) /
                 (x*x*fZzr*fBw*fTerm1 - fRmz*(fTerm2-1.0)
                                      + x*fRmz*(1.0+x*fF)*fZzr*fTerm1) );
        }
        nCount++;
        fEps = fabs(xNeu - x);
        x = xNeu;
    }
    if (fSchaetzwert == 0.0 && fabs(x) < SCdEpsilon)
        x = 0.0;								// auf Null normieren
    fSchaetzwert = x;							//n Rueckgabe
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
    fRmz = ScGetRmz(fZins, fZzr, fBw, fZw, fF);		// fuer kapz auch bei fZr == 1
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
        fEnde   = ::rtl::math::approxFloor(GetDouble());
        fAnfang = ::rtl::math::approxFloor(GetDouble());
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
        fEnde   = ::rtl::math::approxFloor(GetDouble());
        fAnfang = ::rtl::math::approxFloor(GetDouble());
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
            fPerioden = ::rtl::math::approxFloor(fPerioden);
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
            fPerioden = ::rtl::math::approxFloor(fPerioden);
            PushDouble( (pow(fEffektiv + 1.0, 1.0 / fPerioden) - 1.0) * fPerioden );
        }
    }
}

/*N*/ void ScInterpreter::ScMod()
/*N*/ {
/*N*/ 	if ( MustHaveParamCount( GetByte(), 2 ) )
/*N*/ 	{
/*N*/ 		double nVal2 = GetDouble();
/*N*/ 		double nVal1 = GetDouble();
/*N*/ 		PushDouble(nVal1 - (::rtl::math::approxFloor(nVal1 / nVal2) * nVal2));
/*N*/ 	}
/*N*/ }

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
                ScRange aVRange( aVAdr, aVAdr );	// fuer SetDirty
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
                BOOL bError = ( pFormula->GetErrCode() != 0 );
                // bError always corresponds with fn
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
                    bError = ( pFormula->GetErrCode() != 0 );
                    fn = pFormula->GetValue();
                    fn -= nVal;
                    if ( bError )
                    {
                        // move closer to last valid value (xn1), keep xn1/fn1
                        double fDiff = ( xn1 - xn ) / 2;
                        if (fabs(fDiff) < nEps)
                            fDiff = (fDiff < 0.0) ? -nEps : nEps;
                        xn += fDiff;
                    }
                    else if (fabs(fn) < nDelta)
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
                double nX = ::rtl::math::approxFloor((nBestX / nDelta) + 0.5) * nDelta;
                if ( bRet )
                {
                    pValue->SetValue( nX );
                    pDok->SetDirty( aVRange );
                    pFormula->Interpret();
                    if ( fabs( pFormula->GetValue() - nVal ) > fabs( fn ) )
                        nX = nBestX;
                }
                else if ( bError )
                {
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
                PushInt(0);			// falsche Zelltypen
            }
        }
        else
        {
            if (!bRet)
                SetError(NOVALUE);
            PushInt(0);				// nGlobalError
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
            const String& rStr = PopString();
            PushString( rStr );
            PushString( rStr );
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
                    PushString( aStr );
                    PushString( aStr );
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
        String aStyle2;								// Vorlage nach Timer
        if (nParamCount >= 3)
            aStyle2 = GetString();
        long nTimeOut = 0;							// Timeout
        if (nParamCount >= 2)
            nTimeOut = (long)(GetDouble()*1000.0);
        String aStyle1 = GetString();				// Vorlage fuer sofort

        if (nTimeOut < 0)
            nTimeOut = 0;

        //
        //	Request ausfuehren, um Vorlage anzuwenden
        //

        if ( !pDok->IsClipOrUndo() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
            {
                //!	notify object shell directly

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

ScDdeLink* lcl_GetDdeLink( SvxLinkManager* pLinkMgr,
                                const String& rA, const String& rT, const String& rI, BYTE nM )
{
    USHORT nCount = pLinkMgr->GetLinks().Count();
    for (USHORT i=0; i<nCount; i++ )
    {
        ::binfilter::SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
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

/*N*/  void ScInterpreter::ScDde()
/*N*/  {
/*N*/  	//	Applikation, Datei, Bereich
/*N*/  	//	Application, Topic, Item
/*N*/  
/*N*/  	BYTE nParamCount = GetByte();
/*N*/  	if ( MustHaveParamCount( nParamCount, 3, 4 ) )
/*N*/  	{
/*N*/  		BYTE nMode = SC_DDE_DEFAULT;
/*N*/  		if (nParamCount == 4)
/*N*/  			nMode = (BYTE) ::rtl::math::approxFloor(GetDouble());
/*N*/  		String aItem  = GetString();
/*N*/  		String aTopic = GetString();
/*N*/  		String aAppl  = GetString();
/*N*/  
/*N*/  		if (nMode < SC_DDE_DEFAULT || nMode > SC_DDE_TEXT)
/*N*/  			nMode = SC_DDE_DEFAULT;
/*N*/  
/*N*/  		//	temporary documents (ScFunctionAccess) have no DocShell
/*N*/  		//	and no LinkManager -> abort
/*N*/  
/*N*/  		SvxLinkManager* pLinkMgr = pDok->GetLinkManager();
/*N*/  		if (!pLinkMgr)
/*N*/  		{
/*N*/  			SetNoValue();
/*N*/  			return;
/*N*/  		}
/*N*/  
/*N*/  			//	Nach dem Laden muss neu interpretiert werden (Verknuepfungen aufbauen)
/*N*/  
/*N*/  		if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
/*N*/  			pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();
/*N*/  
/*N*/  			//	solange der Link nicht ausgewertet ist, Idle abklemmen
/*N*/  			//	(um zirkulaere Referenzen zu vermeiden)
/*N*/  
/*N*/  		BOOL bOldDis = pDok->IsIdleDisabled();
/*N*/  		pDok->DisableIdle( TRUE );
/*N*/  
/*N*/  			//	Link-Objekt holen / anlegen
/*N*/  
/*N*/  		ScDdeLink* pLink = lcl_GetDdeLink( pLinkMgr, aAppl, aTopic, aItem, nMode );
/*N*/  
/*N*/  		//!	Dde-Links (zusaetzlich) effizienter am Dokument speichern !!!!!
/*N*/  		//		ScDdeLink* pLink = pDok->GetDdeLink( aAppl, aTopic, aItem );
/*N*/  
/*N*/  		BOOL bWasError = ( pMyFormulaCell->GetCode()->GetError() != 0 );
/*N*/  
/*N*/  		if (!pLink)
/*N*/  		{
/*N*/  			pLink = new ScDdeLink( pDok, aAppl, aTopic, aItem, nMode );
/*N*/  			pLinkMgr->InsertDDELink( pLink, aAppl, aTopic, aItem );
/*N*/  
/*N*/  									//! asynchron auswerten ???
/*N*/  			pLink->TryUpdate();		//	TryUpdate ruft Update nicht mehrfach auf
/*N*/  
/*N*/  			// StartListening erst nach dem Update, sonst circular reference
/*N*/  			pMyFormulaCell->StartListening( *pLink, TRUE );
/*N*/  		}
/*N*/  		else
/*N*/  		{
/*N*/  			if ( !pMyFormulaCell->IsListening( *pLink ) )
/*N*/  				pMyFormulaCell->StartListening( *pLink, TRUE );
/*N*/  		}
/*N*/  
/*N*/  		//	Wenn aus dem Reschedule beim Ausfuehren des Links ein Fehler
/*N*/  		//	(z.B. zirkulaere Referenz) entstanden ist, der vorher nicht da war,
/*N*/  		//	das Fehler-Flag zuruecksetzen:
/*N*/  
/*N*/  		if ( pMyFormulaCell->GetCode()->GetError() && !bWasError )
/*N*/  			pMyFormulaCell->GetCode()->SetError(0);
/*N*/  
/*N*/  			//	Wert abfragen
/*N*/  
/*N*/  		const ScMatrix*	pLinkMat = pLink->GetResult();
/*N*/  		if (pLinkMat)
/*N*/  		{
/*N*/  			USHORT nC, nR, nMatInd;
/*N*/  			pLinkMat->GetDimensions(nC, nR);
/*N*/  			ScMatrix* pNewMat = GetNewMat( nC, nR, nMatInd );
/*N*/  			if (pNewMat)
/*N*/  			{
/*N*/  				pLinkMat->MatCopy(*pNewMat);		// kopieren
/*N*/  				PushMatrix( pNewMat );
/*N*/  				nRetMat = nMatInd;
/*N*/  			}
/*N*/  			// sonst Fehler schon in GetNewMat gesetzt
/*N*/  		}
/*N*/  		else
/*N*/  			SetNV();
/*N*/  
/*N*/  		pDok->DisableIdle( bOldDis );
/*N*/  	}
/*N*/  }

void ScInterpreter::ScBase()
{	// Value, Base [, MinLen]
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
        xub_StrLen nMinLen;
        if ( nParamCount == 3 )
        {
            double fLen = ::rtl::math::approxFloor( GetDouble() );
            if ( 1.0 <= fLen && fLen < STRING_MAXLEN )
                nMinLen = (xub_StrLen) fLen;
            else if ( fLen == 0.0 )
                nMinLen = 1;
            else
                nMinLen = 0;	// Error
        }
        else
            nMinLen = 1;
        double fBase = ::rtl::math::approxFloor( GetDouble() );
        double fVal = ::rtl::math::approxFloor( GetDouble() );
        double fChars = ((fVal > 0.0 && fBase > 0.0) ?
            (ceil( log( fVal ) / log( fBase ) ) + 2.0) :
            2.0);
        if ( fChars >= STRING_MAXLEN )
            nMinLen = 0;	// Error

        if ( !nGlobalError && nMinLen && 2 <= fBase && fBase <= nDigits && 0 <= fVal )
        {
            const xub_StrLen nConstBuf = 128;
            sal_Unicode aBuf[nConstBuf];
            xub_StrLen nBuf = Max( (xub_StrLen) fChars, (xub_StrLen) (nMinLen+1) );
            sal_Unicode* pBuf = (nBuf <= nConstBuf ? aBuf : new sal_Unicode[nBuf]);
            for ( xub_StrLen j = 0; j < nBuf; ++j )
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
//					double fDig = ::rtl::math::approxFloor( fmod( fVal, fBase ) );
// so ist es etwas besser
                    double fInt = ::rtl::math::approxFloor( fVal / fBase );
                    double fMult = fInt * fBase;
#if OSL_DEBUG_LEVEL > 1
                    // #53943# =BASIS(1e308;36) => GPF mit
                    // nDig = (size_t) ::rtl::math::approxFloor( fVal - fMult );
                    // trotz vorheriger Pruefung ob fVal >= fMult
                    double fDebug1 = fVal - fMult;
                    // fVal    := 7,5975311883090e+290
                    // fMult   := 7,5975311883090e+290
                    // fDebug1 := 1,3848924157003e+275	<- RoundOff-Error
                    // fVal != fMult, aber: ::rtl::math::approxEqual( fVal, fMult ) == TRUE
                    double fDebug2 = ::rtl::math::approxSub( fVal, fMult );
                    // und ::rtl::math::approxSub( fVal, fMult ) == 0
                    double fDebug3 = ( fInt ? fVal / fInt : 0.0 );
                    // Nach dem strange fDebug1 und fVal < fMult  ist eigentlich
                    // fDebug2 == fBase, trotzdem wird das mit einem Vergleich
                    // nicht erkannt, dann schlaegt bDirt zu und alles wird wieder gut..
#endif
                    size_t nDig;
                    if ( fVal < fMult )
                    {	// da ist was gekippt
                        bDirt = TRUE;
                        nDig = 0;
                    }
                    else
                    {
                        double fDig = ::rtl::math::approxFloor( ::rtl::math::approxSub( fVal, fMult ) );
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
                PushStringBuffer( p );
            }
            if ( pBuf != aBuf )
                delete [] pBuf;
        }
        else
            SetIllegalArgument();
    }
}


/*N*/ void ScInterpreter::ScDecimal()
/*N*/ {	// Text, Base
/*N*/ 	if ( MustHaveParamCount( GetByte(), 2 ) )
/*N*/ 	{
/*N*/ 		double fBase = ::rtl::math::approxFloor( GetDouble() );
/*N*/ 		String aStr( GetString() );
/*N*/ 		if ( !nGlobalError && 2 <= fBase && fBase <= 36 )
/*N*/ 		{
/*N*/ 			double fVal = 0.0;
/*N*/ 			int nBase = (int) fBase;
/*N*/ 			register const sal_Unicode* p = aStr.GetBuffer();
/*N*/ 			while ( *p == ' ' || *p == '\t' )
/*N*/ 				p++;		// strip leading white space
/*N*/ 			if ( nBase == 16 )
/*N*/ 			{	// evtl. hex-prefix strippen
/*N*/ 				if ( *p == 'x' || *p == 'X' )
/*N*/ 					p++;
/*N*/ 				else if ( *p == '0' && (*(p+1) == 'x' || *(p+1) == 'X') )
/*N*/ 					p += 2;
/*N*/ 			}
/*N*/ 			while ( *p )
/*N*/ 			{
/*N*/ 				int n;
/*N*/ 				if ( '0' <= *p && *p <= '9' )
/*N*/ 					n = *p - '0';
/*N*/ 				else if ( 'A' <= *p && *p <= 'Z' )
/*N*/ 					n = 10 + (*p - 'A');
/*N*/ 				else if ( 'a' <= *p && *p <= 'z' )
/*N*/ 					n = 10 + (*p - 'a');
/*N*/ 				else
/*N*/ 					n = nBase;
/*N*/ 				if ( nBase <= n )
/*N*/ 				{
/*N*/ 					if ( *(p+1) == 0 &&
/*N*/ 							( (nBase ==  2 && (*p == 'b' || *p == 'B'))
/*N*/ 							||(nBase == 16 && (*p == 'h' || *p == 'H')) )
/*N*/ 						)
/*N*/ 						;		// 101b und F00Dh sind ok
/*N*/ 					else
/*N*/ 					{
/*?*/ 						SetIllegalArgument();
/*?*/ 						return ;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					fVal = fVal * fBase + n;
/*N*/ 				p++;
/*N*/ 
/*N*/ 			}
/*N*/ 			PushDouble( fVal );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			SetIllegalArgument();
/*N*/ 	}
/*N*/ }


/*N*/ void ScInterpreter::ScConvert()
/*N*/ {	// Value, FromUnit, ToUnit
/*N*/ 	if ( MustHaveParamCount( GetByte(), 3 ) )
/*N*/ 	{
/*N*/ 		String aToUnit( GetString() );
/*N*/ 		String aFromUnit( GetString() );
/*N*/ 		double fVal = GetDouble();
/*N*/ 		if ( nGlobalError )
/*?*/ 			SetIllegalArgument();
/*N*/ 		else
/*N*/ 		{	// erst die angegebene Reihenfolge suchen, wenn nicht gefunden den Kehrwert
/*N*/ 			double fConv;
/*N*/ 			if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aFromUnit, aToUnit ) )
/*?*/ 				PushDouble( fVal * fConv );
/*N*/ 			else if ( ScGlobal::GetUnitConverter()->GetValue( fConv, aToUnit, aFromUnit ) )
/*N*/ 				PushDouble( fVal / fConv );
/*N*/ 			else
/*?*/ 				SetNV();
/*N*/ 		}
/*N*/ 	}
/*N*/ }


void ScInterpreter::ScRoman()
{	// Value [Mode]
    BYTE nParamCount = GetByte();
    if( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fMode = (nParamCount == 2) ? ::rtl::math::approxFloor( GetDouble() ) : 0.0;
        double fVal = ::rtl::math::approxFloor( GetDouble() );
        if( nGlobalError )
            SetIllegalParameter();
        else if( (fMode >= 0.0) && (fMode < 5.0) && (fVal >= 0.0) && (fVal < 4000.0) )
        {
            static const sal_Unicode pChars[] = { 'M', 'D', 'C', 'L', 'X', 'V', 'I' };
            static const USHORT pValues[] = { 1000, 500, 100, 50, 10, 5, 1 };
            static const USHORT nMaxIndex = (USHORT)(SAL_N_ELEMENTS(pValues) - 1);

            String aRoman;
            USHORT nVal = (USHORT) fVal;
            USHORT nMode = (USHORT) fMode;

            for( UINT16 i = 0; i <= nMaxIndex / 2; i++ )
            {
                USHORT nIndex = 2 * i;
                USHORT nDigit = nVal / pValues[ nIndex ];

                if( (nDigit % 5) == 4 )
                {
                    USHORT nIndex2 = (nDigit == 4) ? nIndex - 1 : nIndex - 2;
                    USHORT nSteps = 0;
                    while( (nSteps < nMode) && (nIndex < nMaxIndex) )
                    {
                        nSteps++;
                        if( pValues[ nIndex2 ] - pValues[ nIndex + 1 ] <= nVal )
                            nIndex++;
                        else
                            nSteps = nMode;
                    }
                    aRoman += pChars[ nIndex ];
                    aRoman += pChars[ nIndex2 ];
                    nVal += pValues[ nIndex ];
                    nVal -= pValues[ nIndex2 ];
                }
                else
                {
                    if( nDigit > 4 )
                        aRoman += pChars[ nIndex - 1 ];
                    aRoman.Expand( aRoman.Len() + (nDigit % 5), pChars[ nIndex ] );
                    nVal %= pValues[ nIndex ];
                }
            }

            PushString( aRoman );
        }
        else
            SetIllegalArgument();
    }
}


BOOL lcl_GetArabicValue( sal_Unicode cChar, USHORT& rnValue, BOOL& rbIsDec )
{
    switch( cChar )
    {
        case 'M':	rnValue = 1000;	rbIsDec = TRUE;		break;
        case 'D':	rnValue = 500;	rbIsDec = FALSE;	break;
        case 'C':	rnValue = 100;	rbIsDec = TRUE;		break;
        case 'L':	rnValue = 50;	rbIsDec = FALSE;	break;
        case 'X':	rnValue = 10;	rbIsDec = TRUE;		break;
        case 'V':	rnValue = 5;	rbIsDec = FALSE;	break;
        case 'I':	rnValue = 1;	rbIsDec = TRUE;		break;
        default:	return FALSE;
    }
    return TRUE;
}


void ScInterpreter::ScArabic()
{
    String aRoman( GetString() );
    if( nGlobalError )
        SetIllegalParameter();
    else
    {
        aRoman.ToUpperAscii();

        USHORT nValue = 0;
        USHORT nValidRest = 3999;
        USHORT nCharIndex = 0;
        USHORT nCharCount = aRoman.Len();
        BOOL bValid = TRUE;

        while( bValid && (nCharIndex < nCharCount) )
        {
            USHORT nDigit1 = 0;
            USHORT nDigit2 = 0;
            BOOL bIsDec1 = FALSE;
            BOOL bIsDec2 = FALSE;
            bValid = lcl_GetArabicValue( aRoman.GetChar( nCharIndex ), nDigit1, bIsDec1 );
            if( bValid && (nCharIndex + 1 < nCharCount) )
                bValid = lcl_GetArabicValue( aRoman.GetChar( nCharIndex + 1 ), nDigit2, bIsDec2 );
            if( bValid )
            {
                if( nDigit1 >= nDigit2 )
                {
                    nValue += nDigit1;
                    nValidRest %= (nDigit1 * (bIsDec1 ? 5 : 2));
                    bValid = (nValidRest >= nDigit1);
                    if( bValid )
                        nValidRest -= nDigit1;
                    nCharIndex++;
                }
                else if( nDigit1 * 2 != nDigit2 )
                {
                    USHORT nDiff = nDigit2 - nDigit1;
                    nValue += nDiff;
                    bValid = (nValidRest >= nDiff);
                    if( bValid )
                        nValidRest = nDigit1 - 1;
                    nCharIndex += 2;
                }
                else
                    bValid = FALSE;
            }
        }
        if( bValid )
            PushInt( nValue );
        else
            SetIllegalArgument();
    }
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
