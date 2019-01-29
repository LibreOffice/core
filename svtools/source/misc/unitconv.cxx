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

#include <svtools/unitconv.hxx>
#include <tools/debug.hxx>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>

void SetFieldUnit(weld::MetricSpinButton& rField, FieldUnit eUnit, bool bAll)
{
    int nMin, nMax;
    rField.get_range(nMin, nMax, FieldUnit::TWIP);
    nMin = rField.denormalize(nMin);
    nMax = rField.denormalize(nMax);

    if (!bAll)
    {
        switch (eUnit)
        {
            case FieldUnit::M:
            case FieldUnit::KM:
                eUnit = FieldUnit::CM;
                break;
            case FieldUnit::FOOT:
            case FieldUnit::MILE:
                eUnit = FieldUnit::INCH;
                break;
            default: //prevent warning
                break;
        }
    }

    rField.set_unit(eUnit);

    if (FieldUnit::POINT == eUnit && rField.get_digits() > 1)
        rField.set_digits(1);
    else
        rField.set_digits(2);

    switch (eUnit)
    {
        // _CHAR and _LINE sets the step of "char" and "line" unit, they are same as FieldUnit::MM
        case FieldUnit::CHAR:
        case FieldUnit::LINE:
        case FieldUnit::MM:
            rField.set_increments(50, 500, eUnit);
            break;
        case FieldUnit::INCH:
            rField.set_increments(2, 20, eUnit);
            break;
        default:
            rField.set_increments(10, 100, eUnit);
            break;
    }

    if (!bAll)
    {
        nMin = rField.normalize(nMin);
        nMax = rField.normalize(nMax);
        rField.set_range(nMin, nMax, FieldUnit::TWIP);
    }
}

void SetFieldUnit( MetricField& rField, FieldUnit eUnit, bool bAll )
{
    sal_Int64 nFirst    = rField.Denormalize( rField.GetFirst( FieldUnit::TWIP ) );
    sal_Int64 nLast = rField.Denormalize( rField.GetLast( FieldUnit::TWIP ) );
    sal_Int64 nMin = rField.Denormalize( rField.GetMin( FieldUnit::TWIP ) );
    sal_Int64 nMax = rField.Denormalize( rField.GetMax( FieldUnit::TWIP ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FieldUnit::M:
            case FieldUnit::KM:
                eUnit = FieldUnit::CM;
                break;

            case FieldUnit::FOOT:
            case FieldUnit::MILE:
                eUnit = FieldUnit::INCH;
                break;
            default: ;//prevent warning
        }
    }
    rField.SetUnit( eUnit );
    switch( eUnit )
    {
        // _CHAR and _LINE sets the step of "char" and "line" unit, they are same as FieldUnit::MM
        case FieldUnit::CHAR:
        case FieldUnit::LINE:
        case FieldUnit::MM:
            rField.SetSpinSize( 50 );
            break;

        case FieldUnit::INCH:
            rField.SetSpinSize( 2 );
            break;

        default:
            rField.SetSpinSize( 10 );
    }

    if ( FieldUnit::POINT == eUnit )
    {
        if( rField.GetDecimalDigits() > 1 )
            rField.SetDecimalDigits( 1 );
    }
    else
        rField.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rField.SetFirst( rField.Normalize( nFirst ), FieldUnit::TWIP );
        rField.SetLast( rField.Normalize( nLast ), FieldUnit::TWIP );
        rField.SetMin( rField.Normalize( nMin ), FieldUnit::TWIP );
        rField.SetMax( rField.Normalize( nMax ), FieldUnit::TWIP );
    }
}

void SetMetricValue(weld::MetricSpinButton& rField, int nCoreValue, MapUnit eUnit)
{
    auto nVal = OutputDevice::LogicToLogic(nCoreValue, eUnit, MapUnit::Map100thMM);
    nVal = rField.normalize(nVal);
    rField.set_value(nVal, FieldUnit::MM_100TH);
}

void SetMetricValue( MetricField& rField, long nCoreValue, MapUnit eUnit )
{
    sal_Int64 nVal = OutputDevice::LogicToLogic( nCoreValue, eUnit, MapUnit::Map100thMM );
    nVal = rField.Normalize( nVal );
    rField.SetValue(nVal, FieldUnit::MM_100TH);

}

int GetCoreValue(const weld::MetricSpinButton& rField, MapUnit eUnit)
{
    int nVal = rField.get_value(FieldUnit::MM_100TH);
    // avoid rounding issues
    const int nSizeMask = 0xff000000;
    bool bRoundBefore = true;
    if( nVal >= 0 )
    {
        if( (nVal & nSizeMask) == 0 )
            bRoundBefore = false;
    }
    else
    {
        if( ((-nVal) & nSizeMask ) == 0 )
            bRoundBefore = false;
    }
    if( bRoundBefore )
        nVal = rField.denormalize( nVal );
    auto nUnitVal = OutputDevice::LogicToLogic(nVal, MapUnit::Map100thMM, eUnit);
    if (!bRoundBefore)
        nUnitVal = rField.denormalize(nUnitVal);
    return nUnitVal;
}

long GetCoreValue( const MetricField& rField, MapUnit eUnit )
{
    sal_Int64 nVal = rField.GetValue(FieldUnit::MM_100TH);
    // avoid rounding issues
    const sal_Int64 nSizeMask = 0xffffffffff000000LL;
    bool bRoundBefore = true;
    if( nVal >= 0 )
    {
        if( (nVal & nSizeMask) == 0 )
            bRoundBefore = false;
    }
    else
    {
        if( ((-nVal) & nSizeMask ) == 0 )
            bRoundBefore = false;
    }
    if( bRoundBefore )
        nVal = rField.Denormalize( nVal );
    sal_Int64 nUnitVal = OutputDevice::LogicToLogic( static_cast<long>(nVal), MapUnit::Map100thMM, eUnit );
    if( ! bRoundBefore )
        nUnitVal = rField.Denormalize( nUnitVal );
    return static_cast<long>(nUnitVal);
}

long CalcToUnit( float nIn, MapUnit eUnit )
{
    // nIn is in Points

    DBG_ASSERT( eUnit == MapUnit::MapTwip       ||
                eUnit == MapUnit::Map100thMM   ||
                eUnit == MapUnit::Map10thMM    ||
                eUnit == MapUnit::MapMM         ||
                eUnit == MapUnit::MapCM, "this unit is not implemented" );

    float nTmp = nIn;

    if ( MapUnit::MapTwip != eUnit )
        nTmp = nIn * 10 / 567;

    switch ( eUnit )
    {
        case MapUnit::Map100thMM:  nTmp *= 100; break;
        case MapUnit::Map10thMM:   nTmp *= 10;  break;
        case MapUnit::MapMM:                     break;
        case MapUnit::MapCM:        nTmp /= 10;  break;
        default: ;//prevent warning
    }

    nTmp *= 20;
    long nRet = static_cast<long>(nTmp);
    return nRet;
//! return (long)(nTmp * 20);
}


long ItemToControl( long nIn, MapUnit eItem, FieldUnit eCtrl )
{
    long nOut = 0;

    switch ( eItem )
    {
        case MapUnit::Map100thMM:
        case MapUnit::Map10thMM:
        case MapUnit::MapMM:
        {
            if ( eItem == MapUnit::Map10thMM )
                nIn /= 10;
            else if ( eItem == MapUnit::Map100thMM )
                nIn /= 100;
            nOut = TransformMetric( nIn, FieldUnit::MM, eCtrl );
        }
        break;

        case MapUnit::MapCM:
        {
            nOut = TransformMetric( nIn, FieldUnit::CM, eCtrl );
        }
        break;

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
        {
            if ( eItem == MapUnit::Map10thInch )
                nIn /= 10;
            else if ( eItem == MapUnit::Map100thInch )
                nIn /= 100;
            else if ( eItem == MapUnit::Map1000thInch )
                nIn /= 1000;
            nOut = TransformMetric( nIn, FieldUnit::INCH, eCtrl );
        }
        break;

        case MapUnit::MapPoint:
        {
            nOut = TransformMetric( nIn, FieldUnit::POINT, eCtrl );
        }
        break;

        case MapUnit::MapTwip:
        {
            nOut = TransformMetric( nIn, FieldUnit::TWIP, eCtrl );
        }
        break;
        default: ;//prevent warning
    }
    return nOut;
}


long ControlToItem( long nIn, FieldUnit eCtrl, MapUnit eItem )
{
    return ItemToControl( nIn, eItem, eCtrl );
}


FieldUnit MapToFieldUnit( const MapUnit eUnit )
{
    switch ( eUnit )
    {
        case MapUnit::Map100thMM:
        case MapUnit::Map10thMM:
        case MapUnit::MapMM:
            return FieldUnit::MM;

        case MapUnit::MapCM:
            return FieldUnit::CM;

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
            return FieldUnit::INCH;

        case MapUnit::MapPoint:
            return FieldUnit::POINT;

        case MapUnit::MapTwip:
            return FieldUnit::TWIP;
        default: ;//prevent warning
    }
    return FieldUnit::NONE;
}


long CalcToPoint( long nIn, MapUnit eUnit, sal_uInt16 nFactor )
{
    DBG_ASSERT( eUnit == MapUnit::MapTwip       ||
                eUnit == MapUnit::Map100thMM   ||
                eUnit == MapUnit::Map10thMM    ||
                eUnit == MapUnit::MapMM         ||
                eUnit == MapUnit::MapCM, "this unit is not implemented" );

    long nRet = 0;

    if ( MapUnit::MapTwip == eUnit )
        nRet = nIn;
    else
        nRet = nIn * 567;

    switch ( eUnit )
    {
        case MapUnit::Map100thMM:  nRet /= 100; break;
        case MapUnit::Map10thMM:   nRet /= 10;  break;
        case MapUnit::MapMM:                     break;
        case MapUnit::MapCM:        nRet *= 10;  break;
        default: ;//prevent warning
    }

    // round up if necessary
    if ( MapUnit::MapTwip != eUnit )
    {
        long nTmp = nRet % 10;

        if ( nTmp >= 4 )
            nRet += 10 - nTmp;
        nRet /= 10;
    }
    return nRet * nFactor / 20;
}


static long CMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567;
    return nRet;
}


static long MMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 10;
    return nRet;
}


static long InchToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 1440 ) && nIn >= ( LONG_MIN / 1440 ) )
        nRet = nIn * 1440;
    return nRet;
}


long PointToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 20 ) && nIn >= ( LONG_MIN / 20 ) )
        nRet = nIn * 20;
    return nRet;
}


static long PicaToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 240 ) && nIn >= ( LONG_MIN / 240 ) )
        nRet = nIn * 240;
    return nRet;
}


static long TwipsToCM( long nIn )
{
    long nRet = nIn / 567;
    return nRet;
}


static long InchToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 100;
    return nRet;
}


static long MMToCM( long nIn )
{
    long nRet = nIn / 10;
    return nRet;
}


static long PointToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 20 ) && nIn >= ( LONG_MIN / 20 ) )
        nRet = nIn * 20 / 567;
    return nRet;
}


static long PicaToCM( long nIn)
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 20 ) && nIn >= ( LONG_MIN / 12 / 20 ) )
        nRet = nIn * 12 * 20 / 567;
    return nRet;
}


static long TwipsToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 566;
    return nRet;
}


static long CMToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10;
    return nRet;
}


static long InchToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 10;
    return nRet;
}


static long PointToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 200 ) && nIn >= ( LONG_MIN / 200 ) )
        nRet = nIn * 200 / 567;
    return nRet;
}


static long PicaToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 200 ) && nIn >= ( LONG_MIN / 12 / 200 ) )
        nRet = nIn * 12 * 200 / 567;
    return nRet;
}


static long TwipsToInch( long nIn )
{
    long nRet = nIn / 1440;
    return nRet;
}


static long CMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 100 ) && nIn >= ( LONG_MIN / 100 ) )
        nRet = nIn * 100 / 254;
    return nRet;
}


static long MMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 254;
    return nRet;
}


static long PointToInch( long nIn )
{
    long nRet = nIn / 72;
    return nRet;
}


static long PicaToInch( long nIn )
{
    long nRet = nIn / 6;
    return nRet;
}


static long TwipsToPoint( long nIn )
{
    long nRet = nIn / 20;
    return nRet;
}


static long InchToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 72 ) && nIn >= ( LONG_MIN / 72 ) )
        nRet = nIn * 72;
    return nRet;
}


static long CMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20;
    return nRet;
}


static long MMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200;
    return nRet;
}


static long PicaToPoint( long nIn )
{
    long nRet = nIn / 12;
    return nRet;
}


static long TwipsToPica( long nIn )
{
    long nRet = nIn / 240;
    return nRet;
}


static long InchToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 6 ) && nIn >= ( LONG_MIN / 6 ) )
        nRet = nIn * 6;
    return nRet;
}


static long PointToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 ) && nIn >= ( LONG_MIN / 12 ) )
        nRet = nIn * 12;
    return nRet;
}


static long CMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20 / 12;
    return nRet;
}


static long MMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200 / 12;
    return nRet;
}


static long Nothing( long nIn )
{
    long nRet = nIn;
    return nRet;
}

FUNC_CONVERT const ConvertTable[6][6] =
{
//  CM,         MM          INCH         POINT        PICAS=32     TWIPS
    { Nothing,  CMToMM,     CMToInch,    CMToPoint,   CMToPica,    CMToTwips },
    { MMToCM,       Nothing,    MMToInch,    MMToPoint,   MMToPica,    MMToTwips },
    { InchToCM, InchToMM,   Nothing,     InchToPoint, InchToPica,  InchToTwips },
    { PointToCM,    PointToMM,  PointToInch, Nothing,     PointToPica, PointToTwips },
    { PicaToCM, PicaToMM,   PicaToInch,  PicaToPoint, Nothing,     PicaToTwips },
    { TwipsToCM,    TwipsToMM,  TwipsToInch, TwipsToPoint,TwipsToPica, Nothing }
};


long TransformMetric( long nVal, FieldUnit aOld, FieldUnit aNew )
{
    if ( aOld == FieldUnit::NONE   || aNew == FieldUnit::NONE ||
         aOld == FieldUnit::CUSTOM || aNew == FieldUnit::CUSTOM )
    {
        return nVal;
    }

    sal_uInt16 nOld = 0;
    sal_uInt16 nNew = 0;

    switch ( aOld )
    {
        case FieldUnit::CM:
            nOld = 0; break;
        case FieldUnit::MM:
            nOld = 1; break;
        case FieldUnit::INCH:
            nOld = 2; break;
        case FieldUnit::POINT:
            nOld = 3; break;
        case FieldUnit::PICA:
            nOld = 4; break;
        case FieldUnit::TWIP:
            nOld = 5; break;
        default: ;//prevent warning
    }

    switch ( aNew )
    {
        case FieldUnit::CM:
            nNew = 0; break;
        case FieldUnit::MM:
            nNew = 1; break;
        case FieldUnit::INCH:
            nNew = 2; break;
        case FieldUnit::POINT:
            nNew = 3; break;
        case FieldUnit::PICA:
            nNew = 4; break;
        case FieldUnit::TWIP:
            nNew = 5; break;
        default: ;//prevent warning
    }
    return ConvertTable[nOld][nNew]( nVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
