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


void SetFieldUnit( MetricField& rField, FieldUnit eUnit, bool bAll )
{
    sal_Int64 nFirst    = rField.Denormalize( rField.GetFirst( FieldUnit::FldTwip ) );
    sal_Int64 nLast = rField.Denormalize( rField.GetLast( FieldUnit::FldTwip ) );
    sal_Int64 nMin = rField.Denormalize( rField.GetMin( FieldUnit::FldTwip ) );
    sal_Int64 nMax = rField.Denormalize( rField.GetMax( FieldUnit::FldTwip ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FieldUnit::FldM:
            case FieldUnit::FldKM:
                eUnit = FieldUnit::FldCM;
                break;

            case FieldUnit::FldFoot:
            case FieldUnit::FldMile:
                eUnit = FieldUnit::FldInch;
                break;
            default: ;//prevent warning
        }
    }
    rField.SetUnit( eUnit );
    switch( eUnit )
    {
        // _CHAR and _LINE sets the step of "char" and "line" unit, they are same as FieldUnit::FldMM
        case FieldUnit::FldChar:
        case FieldUnit::FldLine:
        case FieldUnit::FldMM:
            rField.SetSpinSize( 50 );
            break;

        case FieldUnit::FldInch:
            rField.SetSpinSize( 2 );
            break;

        default:
            rField.SetSpinSize( 10 );
    }

    if ( FieldUnit::FldPoint == eUnit )
    {
        if( rField.GetDecimalDigits() > 1 )
            rField.SetDecimalDigits( 1 );
    }
    else
        rField.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rField.SetFirst( rField.Normalize( nFirst ), FieldUnit::FldTwip );
        rField.SetLast( rField.Normalize( nLast ), FieldUnit::FldTwip );
        rField.SetMin( rField.Normalize( nMin ), FieldUnit::FldTwip );
        rField.SetMax( rField.Normalize( nMax ), FieldUnit::FldTwip );
    }
}


void SetFieldUnit( MetricBox& rBox, FieldUnit eUnit )
{
    sal_Int64 nMin = rBox.Denormalize( rBox.GetMin( FieldUnit::FldTwip ) );
    sal_Int64 nMax = rBox.Denormalize( rBox.GetMax( FieldUnit::FldTwip ) );

    switch ( eUnit )
    {
        case FieldUnit::FldM:
        case FieldUnit::FldKM:
            eUnit = FieldUnit::FldCM;
            break;

        case FieldUnit::FldFoot:
        case FieldUnit::FldMile:
            eUnit = FieldUnit::FldInch;
            break;
        default: ;//prevent warning
    }
    rBox.SetUnit( eUnit );

    if ( FieldUnit::FldPoint == eUnit && rBox.GetDecimalDigits() > 1 )
        rBox.SetDecimalDigits( 1 );
    else
        rBox.SetDecimalDigits( 2 );

    rBox.SetMin( rBox.Normalize( nMin ), FieldUnit::FldTwip );
    rBox.SetMax( rBox.Normalize( nMax ), FieldUnit::FldTwip );
}


void SetMetricValue( MetricField& rField, long nCoreValue, MapUnit eUnit )
{
    sal_Int64 nVal = OutputDevice::LogicToLogic( nCoreValue, (MapUnit)eUnit, MapUnit::Map100thMM );
    nVal = rField.Normalize( nVal );
    rField.SetValue( nVal, FieldUnit::Fld100thMM );

}


long GetCoreValue( const MetricField& rField, MapUnit eUnit )
{
    sal_Int64 nVal = rField.GetValue( FieldUnit::Fld100thMM );
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
    sal_Int64 nUnitVal = OutputDevice::LogicToLogic( static_cast<long>(nVal), MapUnit::Map100thMM, (MapUnit)eUnit );
    if( ! bRoundBefore )
        nUnitVal = rField.Denormalize( nUnitVal );
    return static_cast<long>(nUnitVal);
}


long CalcToUnit( float nIn, MapUnit eUnit )
{
    // nIn ist in Points

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
    long nRet = (long)nTmp;
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
            nOut = TransformMetric( nIn, FieldUnit::FldMM, eCtrl );
        }
        break;

        case MapUnit::MapCM:
        {
            nOut = TransformMetric( nIn, FieldUnit::FldCM, eCtrl );
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
            nOut = TransformMetric( nIn, FieldUnit::FldInch, eCtrl );
        }
        break;

        case MapUnit::MapPoint:
        {
            nOut = TransformMetric( nIn, FieldUnit::FldPoint, eCtrl );
        }
        break;

        case MapUnit::MapTwip:
        {
            nOut = TransformMetric( nIn, FieldUnit::FldTwip, eCtrl );
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
            return FieldUnit::FldMM;

        case MapUnit::MapCM:
            return FieldUnit::FldCM;

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
            return FieldUnit::FldInch;

        case MapUnit::MapPoint:
            return FieldUnit::FldPoint;

        case MapUnit::MapTwip:
            return FieldUnit::FldTwip;
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

    // ggf. aufrunden
    if ( MapUnit::MapTwip != eUnit )
    {
        long nMod = 10;
        long nTmp = nRet % nMod;

        if ( nTmp >= 4 )
            nRet += 10 - nTmp;
        nRet /= 10;
    }
    return nRet * nFactor / 20;
}


long CMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567;
    return nRet;
}


long MMToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 10;
    return nRet;
}


long InchToTwips( long nIn )
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


long PicaToTwips( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 240 ) && nIn >= ( LONG_MIN / 240 ) )
        nRet = nIn * 240;
    return nRet;
}


long TwipsToCM( long nIn )
{
    long nRet = nIn / 567;
    return nRet;
}


long InchToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 100;
    return nRet;
}


long MMToCM( long nIn )
{
    long nRet = nIn / 10;
    return nRet;
}


long PointToCM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 20 ) && nIn >= ( LONG_MIN / 20 ) )
        nRet = nIn * 20 / 567;
    return nRet;
}


long PicaToCM( long nIn)
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 20 ) && nIn >= ( LONG_MIN / 12 / 20 ) )
        nRet = nIn * 12 * 20 / 567;
    return nRet;
}


long TwipsToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 566;
    return nRet;
}


long CMToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10;
    return nRet;
}


long InchToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 254 ) && nIn >= ( LONG_MIN / 254 ) )
        nRet = nIn * 254 / 10;
    return nRet;
}


long PointToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 200 ) && nIn >= ( LONG_MIN / 200 ) )
        nRet = nIn * 200 / 567;
    return nRet;
}


long PicaToMM( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 / 200 ) && nIn >= ( LONG_MIN / 12 / 200 ) )
        nRet = nIn * 12 * 200 / 567;
    return nRet;
}


long TwipsToInch( long nIn )
{
    long nRet = nIn / 1440;
    return nRet;
}


long CMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 100 ) && nIn >= ( LONG_MIN / 100 ) )
        nRet = nIn * 100 / 254;
    return nRet;
}


long MMToInch( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 10 ) && nIn >= ( LONG_MIN / 10 ) )
        nRet = nIn * 10 / 254;
    return nRet;
}


long PointToInch( long nIn )
{
    long nRet = nIn / 72;
    return nRet;
}


long PicaToInch( long nIn )
{
    long nRet = nIn / 6;
    return nRet;
}


long TwipsToPoint( long nIn )
{
    long nRet = nIn / 20;
    return nRet;
}


long InchToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 72 ) && nIn >= ( LONG_MIN / 72 ) )
        nRet = nIn * 72;
    return nRet;
}


long CMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20;
    return nRet;
}


long MMToPoint( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200;
    return nRet;
}


long PicaToPoint( long nIn )
{
    long nRet = nIn / 12;
    return nRet;
}


long TwipsToPica( long nIn )
{
    long nRet = nIn / 240;
    return nRet;
}


long InchToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 6 ) && nIn >= ( LONG_MIN / 6 ) )
        nRet = nIn * 6;
    return nRet;
}


long PointToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 12 ) && nIn >= ( LONG_MIN / 12 ) )
        nRet = nIn * 12;
    return nRet;
}


long CMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 20 / 12;
    return nRet;
}


long MMToPica( long nIn )
{
    long nRet = 0;

    if ( nIn <= ( LONG_MAX / 567 ) && nIn >= ( LONG_MIN / 567 ) )
        nRet = nIn * 567 / 200 / 12;
    return nRet;
}


long Nothing( long nIn )
{
    long nRet = nIn;
    return nRet;
}

FUNC_CONVERT ConvertTable[6][6] =
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
         aOld == FieldUnit::FldCustom || aNew == FieldUnit::FldCustom )
    {
        return nVal;
    }

    sal_uInt16 nOld = 0;
    sal_uInt16 nNew = 0;

    switch ( aOld )
    {
        case FieldUnit::FldCM:
            nOld = 0; break;
        case FieldUnit::FldMM:
            nOld = 1; break;
        case FieldUnit::FldInch:
            nOld = 2; break;
        case FieldUnit::FldPoint:
            nOld = 3; break;
        case FieldUnit::FldPica:
            nOld = 4; break;
        case FieldUnit::FldTwip:
            nOld = 5; break;
        default: ;//prevent warning
    }

    switch ( aNew )
    {
        case FieldUnit::FldCM:
            nNew = 0; break;
        case FieldUnit::FldMM:
            nNew = 1; break;
        case FieldUnit::FldInch:
            nNew = 2; break;
        case FieldUnit::FldPoint:
            nNew = 3; break;
        case FieldUnit::FldPica:
            nNew = 4; break;
        case FieldUnit::FldTwip:
            nNew = 5; break;
        default: ;//prevent warning
    }
    return ConvertTable[nOld][nNew]( nVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
