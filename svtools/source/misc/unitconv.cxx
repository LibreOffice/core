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
    sal_Int64 nFirst    = rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) );
    sal_Int64 nLast = rField.Denormalize( rField.GetLast( FUNIT_TWIP ) );
    sal_Int64 nMin = rField.Denormalize( rField.GetMin( FUNIT_TWIP ) );
    sal_Int64 nMax = rField.Denormalize( rField.GetMax( FUNIT_TWIP ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eUnit = FUNIT_CM;
                break;

            case FUNIT_FOOT:
            case FUNIT_MILE:
                eUnit = FUNIT_INCH;
                break;
            default: ;//prevent warning
        }
    }
    rField.SetUnit( eUnit );
    switch( eUnit )
    {
        // _CHAR and _LINE sets the step of "char" and "line" unit, they are same as FUNIT_MM
        case FUNIT_CHAR:
        case FUNIT_LINE:
        case FUNIT_MM:
            rField.SetSpinSize( 50 );
            break;

        case FUNIT_INCH:
            rField.SetSpinSize( 2 );
            break;

        default:
            rField.SetSpinSize( 10 );
    }

    if ( FUNIT_POINT == eUnit )
    {
        if( rField.GetDecimalDigits() > 1 )
            rField.SetDecimalDigits( 1 );
    }
    else
        rField.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
        rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
        rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
        rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
    }
}



void SetFieldUnit( MetricBox& rBox, FieldUnit eUnit, bool bAll )
{
    sal_Int64 nMin = rBox.Denormalize( rBox.GetMin( FUNIT_TWIP ) );
    sal_Int64 nMax = rBox.Denormalize( rBox.GetMax( FUNIT_TWIP ) );

    if ( !bAll )
    {
        switch ( eUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eUnit = FUNIT_CM;
                break;

            case FUNIT_FOOT:
            case FUNIT_MILE:
                eUnit = FUNIT_INCH;
                break;
            default: ;//prevent warning
        }
    }
    rBox.SetUnit( eUnit );

    if ( FUNIT_POINT == eUnit && rBox.GetDecimalDigits() > 1 )
        rBox.SetDecimalDigits( 1 );
    else
        rBox.SetDecimalDigits( 2 );

    if ( !bAll )
    {
        rBox.SetMin( rBox.Normalize( nMin ), FUNIT_TWIP );
        rBox.SetMax( rBox.Normalize( nMax ), FUNIT_TWIP );
    }
}


void SetMetricValue( MetricField& rField, long nCoreValue, SfxMapUnit eUnit )
{
    sal_Int64 nVal = OutputDevice::LogicToLogic( nCoreValue, (MapUnit)eUnit, MAP_100TH_MM );
    nVal = rField.Normalize( nVal );
    rField.SetValue( nVal, FUNIT_100TH_MM );

}



long GetCoreValue( const MetricField& rField, SfxMapUnit eUnit )
{
    sal_Int64 nVal = rField.GetValue( FUNIT_100TH_MM );
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
    sal_Int64 nUnitVal = OutputDevice::LogicToLogic( static_cast<long>(nVal), MAP_100TH_MM, (MapUnit)eUnit );
    if( ! bRoundBefore )
        nUnitVal = rField.Denormalize( nUnitVal );
    return static_cast<long>(nUnitVal);
}



long CalcToUnit( float nIn, SfxMapUnit eUnit )
{
    // nIn ist in Points

    DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP       ||
                eUnit == SFX_MAPUNIT_100TH_MM   ||
                eUnit == SFX_MAPUNIT_10TH_MM    ||
                eUnit == SFX_MAPUNIT_MM         ||
                eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );

    float nTmp = nIn;

    if ( SFX_MAPUNIT_TWIP != eUnit )
        nTmp = nIn * 10 / 567;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:  nTmp *= 100; break;
        case SFX_MAPUNIT_10TH_MM:   nTmp *= 10;  break;
        case SFX_MAPUNIT_MM:                     break;
        case SFX_MAPUNIT_CM:        nTmp /= 10;  break;
        default: ;//prevent warning
    }

    nTmp *= 20;
    long nRet = (long)nTmp;
    return nRet;
//! return (long)(nTmp * 20);
}



long ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl )
{
    long nOut = 0;

    switch ( eItem )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
        {
            if ( eItem == SFX_MAPUNIT_10TH_MM )
                nIn /= 10;
            else if ( eItem == SFX_MAPUNIT_100TH_MM )
                nIn /= 100;
            nOut = TransformMetric( nIn, FUNIT_MM, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_CM:
        {
            nOut = TransformMetric( nIn, FUNIT_CM, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
        {
            if ( eItem == SFX_MAPUNIT_10TH_INCH )
                nIn /= 10;
            else if ( eItem == SFX_MAPUNIT_100TH_INCH )
                nIn /= 100;
            else if ( eItem == SFX_MAPUNIT_1000TH_INCH )
                nIn /= 1000;
            nOut = TransformMetric( nIn, FUNIT_INCH, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_POINT:
        {
            nOut = TransformMetric( nIn, FUNIT_POINT, (FieldUnit)eCtrl );
        }
        break;

        case SFX_MAPUNIT_TWIP:
        {
            nOut = TransformMetric( nIn, FUNIT_TWIP, (FieldUnit)eCtrl );
        }
        break;
        default: ;//prevent warning
    }
    return nOut;
}



long ControlToItem( long nIn, SfxFieldUnit eCtrl, SfxMapUnit eItem )
{
    return ItemToControl( nIn, eItem, eCtrl );
}



FieldUnit MapToFieldUnit( const SfxMapUnit eUnit )
{
    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
            return FUNIT_MM;

        case SFX_MAPUNIT_CM:
            return FUNIT_CM;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
            return FUNIT_INCH;

        case SFX_MAPUNIT_POINT:
            return FUNIT_POINT;

        case SFX_MAPUNIT_TWIP:
            return FUNIT_TWIP;
        default: ;//prevent warning
    }
    return FUNIT_NONE;
}



long CalcToPoint( long nIn, SfxMapUnit eUnit, sal_uInt16 nFactor )
{
    DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP       ||
                eUnit == SFX_MAPUNIT_100TH_MM   ||
                eUnit == SFX_MAPUNIT_10TH_MM    ||
                eUnit == SFX_MAPUNIT_MM         ||
                eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );

    long nRet = 0;

    if ( SFX_MAPUNIT_TWIP == eUnit )
        nRet = nIn;
    else
        nRet = nIn * 567;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:  nRet /= 100; break;
        case SFX_MAPUNIT_10TH_MM:   nRet /= 10;  break;
        case SFX_MAPUNIT_MM:                     break;
        case SFX_MAPUNIT_CM:        nRet *= 10;  break;
        default: ;//prevent warning
    }

    // ggf. aufrunden
    if ( SFX_MAPUNIT_TWIP != eUnit )
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
    if ( aOld == FUNIT_NONE   || aNew == FUNIT_NONE ||
         aOld == FUNIT_CUSTOM || aNew == FUNIT_CUSTOM )
    {
        return nVal;
    }

    sal_uInt16 nOld = 0;
    sal_uInt16 nNew = 0;

    switch ( aOld )
    {
        case FUNIT_CM:
            nOld = 0; break;
        case FUNIT_MM:
            nOld = 1; break;
        case FUNIT_INCH:
            nOld = 2; break;
        case FUNIT_POINT:
            nOld = 3; break;
        case FUNIT_PICA:
            nOld = 4; break;
        case FUNIT_TWIP:
            nOld = 5; break;
        default: ;//prevent warning
    }

    switch ( aNew )
    {
        case FUNIT_CM:
            nNew = 0; break;
        case FUNIT_MM:
            nNew = 1; break;
        case FUNIT_INCH:
            nNew = 2; break;
        case FUNIT_POINT:
            nNew = 3; break;
        case FUNIT_PICA:
            nNew = 4; break;
        case FUNIT_TWIP:
            nNew = 5; break;
        default: ;//prevent warning
    }
    return ConvertTable[nOld][nNew]( nVal );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
