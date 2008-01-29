/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interpr2.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:22:44 $
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
#include "dpobject.hxx"
#include "postit.hxx"

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
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetYear() );
}

void ScInterpreter::ScGetMonth()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long) ::rtl::math::approxFloor(GetDouble());
    PushDouble( (double) aDate.GetMonth() );
}

void ScInterpreter::ScGetDay()
{
    Date aDate = *(pFormatter->GetNullDate());
    aDate += (long)::rtl::math::approxFloor(GetDouble());
    PushDouble((double) aDate.GetDay());
}

void ScInterpreter::ScGetMin()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) % 3600;
    PushDouble( (double) (nVal/60) );
}

void ScInterpreter::ScGetSec()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) % 60;
    PushDouble( (double) nVal );
}

void ScInterpreter::ScGetHour()
{
    double fTime = GetDouble();
    fTime -= ::rtl::math::approxFloor(fTime);       // Datumsanteil weg
    long nVal = (long)::rtl::math::approxFloor(fTime*D_TIMEFACTOR+0.5) / 3600;
    PushDouble((double) nVal);
}

void ScInterpreter::ScGetDateValue()
{
    String aInputString = GetString();
    sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
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
            nFlag = (short) ::rtl::math::approxFloor(GetDouble());
        else
            nFlag = 1;

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor(GetDouble());
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
        short nFlag = (short) ::rtl::math::approxFloor(GetDouble());

        Date aDate = *(pFormatter->GetNullDate());
        aDate += (long)::rtl::math::approxFloor(GetDouble());
        PushInt( (int) aDate.GetWeekOfYear( nFlag == 1 ? SUNDAY : MONDAY ));
    }
}

void ScInterpreter::ScEasterSunday()
{
    nFuncFmtType = NUMBERFORMAT_DATE;
    if ( MustHaveParamCount( GetByte(), 1 ) )
    {
        INT16 nDay, nMonth, nYear;
        nYear = (INT16) ::rtl::math::approxFloor( GetDouble() );
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
        nDay = sal::static_int_cast<INT16>( O % 31 + 1 );
        nMonth = sal::static_int_cast<INT16>( int(O / 31) );
        PushDouble( GetDate( nYear, nMonth, nDay ) );
    }
}

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
    /* Implementation follows
     * http://www.bondmarkets.com/eCommerce/SMD_Fields_030802.pdf
     * Appendix B: Day-Count Bases, there are 7 different ways to calculate the
     * 30-days count. That document also claims that Excel implements the "PSA
     * 30" or "NASD 30" method (funny enough they also state that Excel is the
     * only tool that does so).
     *
     * Note that the definiton given in
     * http://msdn.microsoft.com/library/en-us/office97/html/SEB7C.asp
     * is _not_ the way how it is actually calculated by Excel (that would not
     * even match any of the 7 methods mentioned above) and would result in the
     * following test cases producing wrong results according to that appendix B:
     *
     * 28-Feb-95  31-Aug-95  181 instead of 180
     * 29-Feb-96  31-Aug-96  181 instead of 180
     * 30-Jan-96  31-Mar-96   61 instead of  60
     * 31-Jan-96  31-Mar-96   61 instead of  60
     *
     * Still, there is a difference between OOoCalc and Excel:
     * In Excel:
     * 02-Feb-99 31-Mar-00 results in  419
     * 31-Mar-00 02-Feb-99 results in -418
     * In Calc the result is 419 respectively -419. I consider the -418 a bug in Excel.
     */

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
                if (!bFlag )
                {
                    if (aDate1.GetDay() == 30)
                        aDate2 -= (ULONG) 1;
                }
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
    sal_uInt32 nFIndex = 0;                 // damit default Land/Spr.
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
    PushDouble(::rtl::math::approxFloor(GetDouble()));
}


void ScInterpreter::RoundNumber( rtl_math_RoundingMode eMode )
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double fVal = 0.0;
        if (nParamCount == 1)
            fVal = ::rtl::math::round( GetDouble(), 0, eMode );
        else
        {
            INT32 nDec = (INT32) ::rtl::math::approxFloor(GetDouble());
            if( nDec < -20 || nDec > 20 )
                SetIllegalArgument();
            else
                fVal = ::rtl::math::round( GetDouble(), (short)nDec, eMode );
        }
        PushDouble(fVal);
    }
}

void ScInterpreter::ScRound()
{
    RoundNumber( rtl_math_RoundingMode_Corrected );
}

void ScInterpreter::ScRoundDown()
{
    RoundNumber( rtl_math_RoundingMode_Down );
}

void ScInterpreter::ScRoundUp()
{
    RoundNumber( rtl_math_RoundingMode_Up );
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
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        double nBase;
        if (nParamCount == 2)
            nBase = GetDouble();
        else
            nBase = 10.0;
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
                        nVal += (GetDouble() / pow(1.0 + nZins, (double)nCount));
                        nCount++;
                    }
                    break;
                    case svSingleRef :
                    {
                        nVal += (GetDouble() / pow(1.0 + nZins, (double)nCount));
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
                            nVal += (nCellVal / pow(1.0 + nZins, (double)nCount));
                            nCount++;
                            while ((nErr == 0) && aValIter.GetNext(nCellVal, nErr))
                            {
                                nVal += (nCellVal / pow(1.0 + nZins, (double)nCount));
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
            fZaehler +=           fWert / pow(1.0+x,(double)nCount);
            fNenner  += -nCount * fWert / pow(1.0+x,nCount+1.0);
            nCount++;
            while ((nErr == 0) && aValIter.GetNext(fWert, nErr))
            {
                fZaehler +=           fWert / pow(1.0+x,(double)nCount);
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


void ScInterpreter::ScMIRR()
{   // range_of_values ; rate_invest ; rate_reinvest
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
                if( fCellValue > 0.0 )          // reinvestments
                    fNPV_reinvest += fCellValue * fPow_reinvest;
                else if( fCellValue < 0.0 )     // investments
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
                fResult *= pow( fRate1_reinvest, (double) nCount - 1 );
                fResult = pow( fResult, 1.0 / (nCount - 1) );
                PushDouble( fResult - 1.0 );
            }
        }
    }
}


void ScInterpreter::ScISPMT()
{   // rate ; period ; total_periods ; invest
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
    double nGda2 = 0.0;
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
    BOOL bNowLia = FALSE;

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

void ScInterpreter::ScMod()
{
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double nVal2 = GetDouble();
        double nVal1 = GetDouble();
        PushDouble( ::rtl::math::approxSub( nVal1,
                    ::rtl::math::approxFloor(nVal1 / nVal2) * nVal2));
    }
}

/** (Goal Seek) Find a value of x that is a root of f(x)

    This function is used internally for the goal seek operation.  It uses the
    Regula Falsi (aka false position) algorithm to find a root of f(x).  The
    start value and the target value are to be given by the user in the
    goal seek dialog.  The f(x) in this case is defined as the formula in the
    formula cell minus target value.  This function may also perform additional
    search in the horizontal directions when the f(x) is discrete in order to
    ensure a non-zero slope necessary for deriving a subsequent x that is
    reasonably close to the root of interest.

    @change 24.10.2004 by Kohei Yoshida (kohei@openoffice.org)

    @see #i28955#
*/
void ScInterpreter::ScBackSolver()
{
    if ( MustHaveParamCount( GetByte(), 3 ) )
    {
        BOOL bDoneIteration = FALSE;
        ScAddress aValueAdr, aFormulaAdr;
        double fTargetVal = GetDouble();
        PopSingleRef( aFormulaAdr );
        PopSingleRef( aValueAdr );

        if (nGlobalError == 0)
        {
            ScBaseCell* pVCell = GetCell( aValueAdr );
            // CELLTYPE_NOTE: kein Value aber von Formel referiert
            BOOL bTempCell = (!pVCell || pVCell->GetCellType() == CELLTYPE_NOTE);
            ScBaseCell* pFCell = GetCell( aFormulaAdr );

            if ( ((pVCell && pVCell->GetCellType() == CELLTYPE_VALUE) || bTempCell)
                && pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA )
            {
                ScRange aVRange( aValueAdr, aValueAdr );    // fuer SetDirty
                double fSaveVal; // Original value to be restored later if necessary
                ScPostIt aNote(pDok);
                BOOL bHasNote = FALSE;

                if ( bTempCell )
                {
                    if ( ( bHasNote = (pVCell != NULL) ) != FALSE )
                        bHasNote = pVCell->GetNote( aNote );
                    fSaveVal = 0.0;
                    pVCell = new ScValueCell( fSaveVal );
                    pDok->PutCell( aValueAdr, pVCell );
                }
                else
                    fSaveVal = GetCellValue( aValueAdr, pVCell );

                const USHORT nMaxIter = 100;
                const double fEps = 1E-10;
                const double fDelta = 1E-3;

                double fBestX, fXPrev;
                double fBestF, fFPrev;
                fBestX = fXPrev = fSaveVal;

                ScFormulaCell* pFormula = (ScFormulaCell*) pFCell;
                ScValueCell* pValue = (ScValueCell*) pVCell;

                pFormula->Interpret();
                BOOL bError = ( pFormula->GetErrCode() != 0 );
                // bError always corresponds with fF

                fFPrev = pFormula->GetValue() - fTargetVal;

                fBestF = fabs( fFPrev );
                if ( fBestF < fDelta )
                    bDoneIteration = TRUE;

                double fX = fXPrev + fEps;
                double fF = fFPrev;
                double fSlope;

                USHORT nIter = 0;

                BOOL bHorMoveError = FALSE;
                                                // Nach der Regula Falsi Methode
                while ( !bDoneIteration && ( nIter++ < nMaxIter ) )
                {
                    pValue->SetValue( fX );
                    pDok->SetDirty( aVRange );
                    pFormula->Interpret();
                    bError = ( pFormula->GetErrCode() != 0 );
                    fF = pFormula->GetValue() - fTargetVal;

                    if ( fF == fFPrev && !bError )
                    {
                        // HORIZONTAL SEARCH: Keep moving x in both directions until the f(x)
                        // becomes different from the previous f(x).  This routine is needed
                        // when a given function is discrete, in which case the resulting slope
                        // may become zero which ultimately causes the goal seek operation
                        // to fail. #i28955#

                        USHORT nHorIter = 0;
                        const double fHorStepAngle = 5.0;
                        const double fHorMaxAngle = 80.0;
                        int nHorMaxIter = static_cast<int>( fHorMaxAngle / fHorStepAngle );
                        BOOL bDoneHorMove = FALSE;

                        while ( !bDoneHorMove && !bHorMoveError && nHorIter++ < nHorMaxIter )
                        {
                            double fHorAngle = fHorStepAngle * static_cast<double>( nHorIter );
                            double fHorTangent = ::rtl::math::tan( fHorAngle * F_PI / 180 );

                            USHORT nIdx = 0;
                            while( nIdx++ < 2 && !bDoneHorMove )
                            {
                                double fHorX;
                                if ( nIdx == 1 )
                                    fHorX = fX + fabs(fF)*fHorTangent;
                                else
                                    fHorX = fX - fabs(fF)*fHorTangent;

                                pValue->SetValue( fHorX );
                                pDok->SetDirty( aVRange );
                                pFormula->Interpret();
                                bHorMoveError = ( pFormula->GetErrCode() != 0 );
                                if ( bHorMoveError )
                                    break;

                                fF = pFormula->GetValue() - fTargetVal;
                                if ( fF != fFPrev )
                                {
                                    fX = fHorX;
                                    bDoneHorMove = TRUE;
                                }
                            }
                        }
                        if ( !bDoneHorMove )
                            bHorMoveError = TRUE;
                    }

                    if ( bError )
                    {
                        // move closer to last valid value (fXPrev), keep fXPrev & fFPrev
                        double fDiff = ( fXPrev - fX ) / 2;
                        if (fabs(fDiff) < fEps)
                            fDiff = (fDiff < 0.0) ? - fEps : fEps;
                        fX += fDiff;
                    }
                    else if ( bHorMoveError )
                        break;
                    else if ( fabs(fF) < fDelta )
                    {
                        // converged to root
                        fBestX = fX;
                        bDoneIteration = TRUE;
                    }
                    else
                    {
                        if ( fabs(fF) + fDelta < fBestF )
                        {
                            fBestX = fX;
                            fBestF = fabs(fF);
                        }

                        if ( ( fXPrev - fX ) != 0 )
                        {
                            fSlope = ( fFPrev - fF ) / ( fXPrev - fX );
                            if ( fabs( fSlope ) < fEps )
                                fSlope = fSlope < 0.0 ? -fEps : fEps;
                        }
                        else
                            fSlope = fEps;

                        fXPrev = fX;
                        fFPrev = fF;
                        fX = fX - ( fF / fSlope );
                    }
                }

                double nX = ::rtl::math::approxFloor((fBestX / fDelta) + 0.5) * fDelta;

                if ( bDoneIteration )
                {
                    pValue->SetValue( nX );
                    pDok->SetDirty( aVRange );
                    pFormula->Interpret();
                    if ( fabs( pFormula->GetValue() - fTargetVal ) > fabs( fF ) )
                        nX = fBestX;
                }
                else if ( bError || bHorMoveError )
                {
                    nX = fBestX;
                }
                if ( bTempCell )
                {
                    if ( bHasNote )
                        pVCell = new ScNoteCell( aNote, pDok );
                    else
                        pVCell = NULL;
                    pDok->PutCell( aValueAdr, pVCell );
                }
                else
                    pValue->SetValue( fSaveVal );
                pDok->SetDirty( aVRange );
                pFormula->Interpret();
                if ( !bDoneIteration )
                    SetError(NOTAVAILABLE);
                PushDouble(nX);
            }
            else
            {
                if ( !bDoneIteration )
                    SetError(NOTAVAILABLE);
                PushInt(0);         // falsche Zelltypen
            }
        }
        else
        {
            if ( !bDoneIteration )
                SetError(NOTAVAILABLE);
            PushInt(0);             // nGlobalError
        }
    }
}

void ScInterpreter::ScIntersect()
{
    SCCOL nCol11;
    SCROW nRow11;
    SCTAB nTab11;
    SCCOL nCol21;
    SCROW nRow21;
    SCTAB nTab21;
    SCCOL nCol12;
    SCROW nRow12;
    SCTAB nTab12;
    SCCOL nCol22;
    SCROW nRow22;
    SCTAB nTab22;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    BYTE eStackVar = sal::static_int_cast<BYTE>( GetStackType() );
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
    eStackVar = sal::static_int_cast<BYTE>( GetStackType() );
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

ScDdeLink* lcl_GetDdeLink( SvxLinkManager* pLinkMgr,
                                const String& rA, const String& rT, const String& rI, BYTE nM )
{
    USHORT nCount = pLinkMgr->GetLinks().Count();
    for (USHORT i=0; i<nCount; i++ )
    {
        ::sfx2::SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
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
            nMode = (BYTE) ::rtl::math::approxFloor(GetDouble());
        String aItem  = GetString();
        String aTopic = GetString();
        String aAppl  = GetString();

        if (nMode > SC_DDE_TEXT)
            nMode = SC_DDE_DEFAULT;

        //  temporary documents (ScFunctionAccess) have no DocShell
        //  and no LinkManager -> abort

        SvxLinkManager* pLinkMgr = pDok->GetLinkManager();
        if (!pLinkMgr)
        {
            SetNoValue();
            return;
        }

            //  Nach dem Laden muss neu interpretiert werden (Verknuepfungen aufbauen)

        if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
            pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();

            //  solange der Link nicht ausgewertet ist, Idle abklemmen
            //  (um zirkulaere Referenzen zu vermeiden)

        BOOL bOldDis = pDok->IsIdleDisabled();
        pDok->DisableIdle( TRUE );

            //  Link-Objekt holen / anlegen

        ScDdeLink* pLink = lcl_GetDdeLink( pLinkMgr, aAppl, aTopic, aItem, nMode );

        //! Dde-Links (zusaetzlich) effizienter am Dokument speichern !!!!!
        //      ScDdeLink* pLink = pDok->GetDdeLink( aAppl, aTopic, aItem );

        BOOL bWasError = ( pMyFormulaCell->GetCode()->GetError() != 0 );

        if (!pLink)
        {
            pLink = new ScDdeLink( pDok, aAppl, aTopic, aItem, nMode );
            pLinkMgr->InsertDDELink( pLink, aAppl, aTopic, aItem );
            if ( pLinkMgr->GetLinks().Count() == 1 )                    // erster ?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enablen
            }

                                    //! asynchron auswerten ???
            pLink->TryUpdate();     //  TryUpdate ruft Update nicht mehrfach auf

            // StartListening erst nach dem Update, sonst circular reference
            pMyFormulaCell->StartListening( *pLink );
        }
        else
        {
            pMyFormulaCell->StartListening( *pLink );
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
            SCSIZE nC, nR;
            pLinkMat->GetDimensions(nC, nR);
            ScMatrixRef pNewMat = GetNewMat( nC, nR);
            if (pNewMat)
            {
                pLinkMat->MatCopy(*pNewMat);        // kopieren
                PushMatrix( pNewMat );
            }
            else
                PushError();
        }
        else
            SetNA();

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
        xub_StrLen nMinLen;
        if ( nParamCount == 3 )
        {
            double fLen = ::rtl::math::approxFloor( GetDouble() );
            if ( 1.0 <= fLen && fLen < STRING_MAXLEN )
                nMinLen = (xub_StrLen) fLen;
            else if ( fLen == 0.0 )
                nMinLen = 1;
            else
                nMinLen = 0;    // Error
        }
        else
            nMinLen = 1;
        double fBase = ::rtl::math::approxFloor( GetDouble() );
        double fVal = ::rtl::math::approxFloor( GetDouble() );
        double fChars = ((fVal > 0.0 && fBase > 0.0) ?
            (ceil( log( fVal ) / log( fBase ) ) + 2.0) :
            2.0);
        if ( fChars >= STRING_MAXLEN )
            nMinLen = 0;    // Error

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
//                  double fDig = ::rtl::math::approxFloor( fmod( fVal, fBase ) );
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
                    // fDebug1 := 1,3848924157003e+275  <- RoundOff-Error
                    // fVal != fMult, aber: ::rtl::math::approxEqual( fVal, fMult ) == TRUE
                    double fDebug2 = ::rtl::math::approxSub( fVal, fMult );
                    // und ::rtl::math::approxSub( fVal, fMult ) == 0
                    double fDebug3 = ( fInt ? fVal / fInt : 0.0 );
                    // Nach dem strange fDebug1 und fVal < fMult  ist eigentlich
                    // fDebug2 == fBase, trotzdem wird das mit einem Vergleich
                    // nicht erkannt, dann schlaegt bDirt zu und alles wird wieder gut..

                    // prevent compiler warnings
                    (void)fDebug1; (void)fDebug2; (void)fDebug3;
#endif
                    size_t nDig;
                    if ( fVal < fMult )
                    {   // da ist was gekippt
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


void ScInterpreter::ScDecimal()
{   // Text, Base
    if ( MustHaveParamCount( GetByte(), 2 ) )
    {
        double fBase = ::rtl::math::approxFloor( GetDouble() );
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
                SetNA();
        }
    }
}


void ScInterpreter::ScRoman()
{   // Value [Mode]
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
            static const USHORT nMaxIndex = (USHORT)(sizeof(pValues) / sizeof(pValues[0]) - 1);

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
                    nVal = sal::static_int_cast<USHORT>( nVal + pValues[ nIndex ] );
                    nVal = sal::static_int_cast<USHORT>( nVal - pValues[ nIndex2 ] );
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
        case 'M':   rnValue = 1000; rbIsDec = TRUE;     break;
        case 'D':   rnValue = 500;  rbIsDec = FALSE;    break;
        case 'C':   rnValue = 100;  rbIsDec = TRUE;     break;
        case 'L':   rnValue = 50;   rbIsDec = FALSE;    break;
        case 'X':   rnValue = 10;   rbIsDec = TRUE;     break;
        case 'V':   rnValue = 5;    rbIsDec = FALSE;    break;
        case 'I':   rnValue = 1;    rbIsDec = TRUE;     break;
        default:    return FALSE;
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
                    nValue = sal::static_int_cast<USHORT>( nValue + nDigit1 );
                    nValidRest %= (nDigit1 * (bIsDec1 ? 5 : 2));
                    bValid = (nValidRest >= nDigit1);
                    if( bValid )
                        nValidRest = sal::static_int_cast<USHORT>( nValidRest - nDigit1 );
                    nCharIndex++;
                }
                else if( nDigit1 * 2 != nDigit2 )
                {
                    USHORT nDiff = nDigit2 - nDigit1;
                    nValue = sal::static_int_cast<USHORT>( nValue + nDiff );
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


void ScInterpreter::ScHyperLink()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1, 2 ) )
    {
        String aCellText = GetString();
        ScMatrixRef pResMat = GetNewMat(1,2);
        pResMat->PutString(aCellText,0);
        pResMat->PutString((nParamCount == 2) ? GetString() : aCellText, 1);
        bMatrixFormula = true;
        PushMatrix(pResMat);
    }
}

// BAHTTEXT ===================================================================

#define UTF8_TH_0       "\340\270\250\340\270\271\340\270\231\340\270\242\340\271\214"
#define UTF8_TH_1       "\340\270\253\340\270\231\340\270\266\340\271\210\340\270\207"
#define UTF8_TH_2       "\340\270\252\340\270\255\340\270\207"
#define UTF8_TH_3       "\340\270\252\340\270\262\340\270\241"
#define UTF8_TH_4       "\340\270\252\340\270\265\340\271\210"
#define UTF8_TH_5       "\340\270\253\340\271\211\340\270\262"
#define UTF8_TH_6       "\340\270\253\340\270\201"
#define UTF8_TH_7       "\340\271\200\340\270\210\340\271\207\340\270\224"
#define UTF8_TH_8       "\340\271\201\340\270\233\340\270\224"
#define UTF8_TH_9       "\340\271\200\340\270\201\340\271\211\340\270\262"
#define UTF8_TH_10      "\340\270\252\340\270\264\340\270\232"
#define UTF8_TH_11      "\340\271\200\340\270\255\340\271\207\340\270\224"
#define UTF8_TH_20      "\340\270\242\340\270\265\340\271\210"
#define UTF8_TH_1E2     "\340\270\243\340\271\211\340\270\255\340\270\242"
#define UTF8_TH_1E3     "\340\270\236\340\270\261\340\270\231"
#define UTF8_TH_1E4     "\340\270\253\340\270\241\340\270\267\340\271\210\340\270\231"
#define UTF8_TH_1E5     "\340\271\201\340\270\252\340\270\231"
#define UTF8_TH_1E6     "\340\270\245\340\271\211\340\270\262\340\270\231"
#define UTF8_TH_DOT0    "\340\270\226\340\271\211\340\270\247\340\270\231"
#define UTF8_TH_BAHT    "\340\270\232\340\270\262\340\270\227"
#define UTF8_TH_SATANG  "\340\270\252\340\270\225\340\270\262\340\270\207\340\270\204\340\271\214"
#define UTF8_TH_MINUS   "\340\270\245\340\270\232"

#define UTF8_STRINGPARAM( ascii )   ascii, static_cast< xub_StrLen >( sizeof( ascii ) - 1 )
#define UTF8_CREATE( ascii )        ByteString( UTF8_STRINGPARAM( ascii ) )
#define UTF8_APPEND( ascii )        Append( UTF8_STRINGPARAM( ascii ) )
#define UTF8_PREPEND( ascii )       Insert( UTF8_CREATE( ascii ), 0 )

// local functions ------------------------------------------------------------

namespace {

inline void lclSplitBlock( double& rfInt, sal_Int32& rnBlock, double fValue, double fSize )
{
    rnBlock = static_cast< sal_Int32 >( modf( (fValue + 0.1) / fSize, &rfInt ) * fSize + 0.1 );
}

/** Appends a digit (0 to 9) to the passed string. */
void lclAppendDigit( ByteString& rText, sal_Int32 nDigit )
{
    switch( nDigit )
    {
        case 0: rText.UTF8_APPEND( UTF8_TH_0 ); break;
        case 1: rText.UTF8_APPEND( UTF8_TH_1 ); break;
        case 2: rText.UTF8_APPEND( UTF8_TH_2 ); break;
        case 3: rText.UTF8_APPEND( UTF8_TH_3 ); break;
        case 4: rText.UTF8_APPEND( UTF8_TH_4 ); break;
        case 5: rText.UTF8_APPEND( UTF8_TH_5 ); break;
        case 6: rText.UTF8_APPEND( UTF8_TH_6 ); break;
        case 7: rText.UTF8_APPEND( UTF8_TH_7 ); break;
        case 8: rText.UTF8_APPEND( UTF8_TH_8 ); break;
        case 9: rText.UTF8_APPEND( UTF8_TH_9 ); break;
        default:    DBG_ERRORFILE( "lclAppendDigit - illegal digit" );
    }
}

/** Appends a value raised to a power of 10: nDigit*10^nPow10.
    @param nDigit  A digit in the range from 1 to 9.
    @param nPow10  A value in the range from 2 to 5.
 */
void lclAppendPow10( ByteString& rText, sal_Int32 nDigit, sal_Int32 nPow10 )
{
    DBG_ASSERT( (1 <= nDigit) && (nDigit <= 9), "lclAppendPow10 - illegal digit" );
    lclAppendDigit( rText, nDigit );
    switch( nPow10 )
    {
        case 2: rText.UTF8_APPEND( UTF8_TH_1E2 );   break;
        case 3: rText.UTF8_APPEND( UTF8_TH_1E3 );   break;
        case 4: rText.UTF8_APPEND( UTF8_TH_1E4 );   break;
        case 5: rText.UTF8_APPEND( UTF8_TH_1E5 );   break;
        default:    DBG_ERRORFILE( "lclAppendPow10 - illegal power" );
    }
}

/** Appends a block of 6 digits (value from 1 to 999,999) to the passed string. */
void lclAppendBlock( ByteString& rText, sal_Int32 nValue )
{
    DBG_ASSERT( (1 <= nValue) && (nValue <= 999999), "lclAppendBlock - illegal value" );
    if( nValue >= 100000 )
    {
        lclAppendPow10( rText, nValue / 100000, 5 );
        nValue %= 100000;
    }
    if( nValue >= 10000 )
    {
        lclAppendPow10( rText, nValue / 10000, 4 );
        nValue %= 10000;
    }
    if( nValue >= 1000 )
    {
        lclAppendPow10( rText, nValue / 1000, 3 );
        nValue %= 1000;
    }
    if( nValue >= 100 )
    {
        lclAppendPow10( rText, nValue / 100, 2 );
        nValue %= 100;
    }
    if( nValue > 0 )
    {
        sal_Int32 nTen = nValue / 10;
        sal_Int32 nOne = nValue % 10;
        if( nTen >= 1 )
        {
            if( nTen >= 3 )
                lclAppendDigit( rText, nTen );
            else if( nTen == 2 )
                rText.UTF8_APPEND( UTF8_TH_20 );
            rText.UTF8_APPEND( UTF8_TH_10 );
        }
        if( (nTen > 0) && (nOne == 1) )
            rText.UTF8_APPEND( UTF8_TH_11 );
        else if( nOne > 0 )
            lclAppendDigit( rText, nOne );
    }
}

} // namespace

// ----------------------------------------------------------------------------

void ScInterpreter::ScBahtText()
{
    BYTE nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1 ) )
    {
        double fValue = GetDouble();
        if( nGlobalError )
        {
            SetIllegalParameter();
            return;
        }

        // sign
        bool bMinus = fValue < 0.0;
        fValue = fabs( fValue );

        // round to 2 digits after decimal point, fValue contains Satang as integer
        fValue = ::rtl::math::approxFloor( fValue * 100.0 + 0.5 );

        // split Baht and Satang
        double fBaht = 0.0;
        sal_Int32 nSatang = 0;
        lclSplitBlock( fBaht, nSatang, fValue, 100.0 );

        ByteString aText;

        // generate text for Baht value
        if( fBaht == 0.0 )
        {
            if( nSatang == 0 )
                aText.UTF8_APPEND( UTF8_TH_0 );
        }
        else while( fBaht > 0.0 )
        {
            ByteString aBlock;
            sal_Int32 nBlock = 0;
            lclSplitBlock( fBaht, nBlock, fBaht, 1.0e6 );
            if( nBlock > 0 )
                lclAppendBlock( aBlock, nBlock );
            // add leading "million", if there will come more blocks
            if( fBaht > 0.0 )
                aBlock.UTF8_PREPEND( UTF8_TH_1E6 );
            aText.Insert( aBlock, 0 );
        }
        if( aText.Len() > 0 )
            aText.UTF8_APPEND( UTF8_TH_BAHT );

        // generate text for Satang value
        if( nSatang == 0 )
        {
            aText.UTF8_APPEND( UTF8_TH_DOT0 );
        }
        else
        {
            lclAppendBlock( aText, nSatang );
            aText.UTF8_APPEND( UTF8_TH_SATANG );
        }

        // add the minus sign
        if( bMinus )
            aText.UTF8_PREPEND( UTF8_TH_MINUS );

        PushString( String( aText, RTL_TEXTENCODING_UTF8 ) );
    }
}

// ============================================================================

void ScInterpreter::ScGetPivotData()
{
    BYTE nParamCount = GetByte();

    if ( MustHaveParamCount( nParamCount, 2, 30 ) )
    {
        // there must be an even number of args
        //      target, ref, then field/item pairs
        if( (nParamCount % 2) == 1)
            goto failed;

        bool bOldSyntax = false;
        if ( nParamCount == 2 )
        {
            // if the first parameter is a ref, assume old syntax
            StackVar eFirstType = GetStackType( 2 );
            if ( eFirstType == svSingleRef || eFirstType == svDoubleRef )
                bOldSyntax = true;
        }

        ScDPGetPivotDataField aTarget;                  // target field, and returns result
        std::vector< ScDPGetPivotDataField > aFilters;
        String aFilterList;
        if ( bOldSyntax )
            aFilterList = GetString();      // old syntax: second parameter is list of constraints
        else
        {
            // new syntax: separate name/value pairs

            USHORT nFilterCount = nParamCount / 2 - 1;
            aFilters.resize( nFilterCount );

            USHORT i = nFilterCount;
            while( i-- > 0 )
            {
                //! should allow numeric constraint values
                aFilters[i].mbValIsStr = TRUE;
                aFilters[i].maValStr = GetString();

                aFilters[i].maFieldName = GetString();
            }
        }

        // common to both syntaxes: a reference to the data pilot table

        ScRange aBlock;
        switch ( GetStackType() )
        {
            case svDoubleRef :
                PopDoubleRef( aBlock );
                break;

            case svSingleRef :
                {
                    ScAddress aAddr;
                    PopSingleRef( aAddr );
                    aBlock = aAddr;
                    break;
                }
            default:
                goto failed;
        }
        // NOTE : MS Excel docs claim to use the 'most recent' which is not
        // exactly the same as what we do in ScDocument::GetDPAtBlock
        // However we do need to use GetDPABlock
        ScDPObject* pDPObj = pDok->GetDPAtBlock ( aBlock );
        if( NULL == pDPObj)
            goto failed;

        if ( bOldSyntax )
        {
            // fill aFilters / aTarget from aFilterList string
            if ( !pDPObj->ParseFilters( aTarget, aFilters, aFilterList ) )
                goto failed;
        }
        else
            aTarget.maFieldName = GetString();      // new syntax: first parameter is data field name

        if( pDPObj->GetPivotData( aTarget, aFilters ) )
        {
            if( aTarget.mbValIsStr )
                PushString( aTarget.maValStr );
            else
                PushDouble( aTarget.mnValNum );
            return;
        }
    }

failed :
    SetError( errNoRef );
}

