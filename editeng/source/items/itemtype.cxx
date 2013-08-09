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

#include <vcl/outdev.hxx>
#include <editeng/editrids.hrc>
#include <unotools/intlwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <rtl/ustrbuf.hxx>

// -----------------------------------------------------------------------

OUString GetMetricText( long nVal, SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit, const IntlWrapper* pIntl )
{
    sal_Bool bNeg = sal_False;
    sal_Int32 nRet = 0;

    if ( nVal < 0 )
    {
        bNeg = sal_True;
        nVal *= -1;
    }

    switch ( eDestUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
        case SFX_MAPUNIT_CM:
        {
            nRet = (long)OutputDevice::LogicToLogic(
                nVal, (MapUnit)eSrcUnit, (MapUnit)SFX_MAPUNIT_100TH_MM );

            switch ( eDestUnit )
            {
                case SFX_MAPUNIT_100TH_MM:  nRet *= 1000; break;
                case SFX_MAPUNIT_10TH_MM:   nRet *= 100; break;
                case SFX_MAPUNIT_MM:        nRet *= 10; break;
                default: ;//prevent warning
            }
            break;
        }

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
        {
            nRet = OutputDevice::LogicToLogic(
                nVal, (MapUnit)eSrcUnit, (MapUnit)SFX_MAPUNIT_1000TH_INCH );

            switch ( eDestUnit )
            {
                case SFX_MAPUNIT_1000TH_INCH:   nRet *= 1000; break;
                case SFX_MAPUNIT_100TH_INCH:    nRet *= 100; break;
                case SFX_MAPUNIT_10TH_INCH:     nRet *= 10; break;
                default: ;//prevent warning
            }
            break;
        }

        case SFX_MAPUNIT_POINT:
        case SFX_MAPUNIT_TWIP:
        case SFX_MAPUNIT_PIXEL:
            return OUString::number( (long)OutputDevice::LogicToLogic(
                        nVal, (MapUnit)eSrcUnit, (MapUnit)eDestUnit ));

        default:
            OSL_FAIL( "not supported mapunit" );
            return OUString();
    }

    if ( SFX_MAPUNIT_CM == eDestUnit || SFX_MAPUNIT_INCH == eDestUnit )
    {
        sal_Int32 nMod = nRet % 10;

        if ( nMod > 4 )
            nRet += 10 - nMod;
        else if ( nMod > 0 )
            nRet -= nMod;
    }

    OUStringBuffer sRet;

    if ( bNeg )
        sRet.append('-');

    long nDiff = 1000;
    for( int nDigits = 4; nDigits; --nDigits, nDiff /= 10 )
    {
        if ( nRet < nDiff )
            sRet.append('0');
        else
            sRet.append(nRet / nDiff);
        nRet %= nDiff;
        if( 4 == nDigits )
        {
            if(pIntl)
                sRet.append(pIntl->getLocaleData()->getNumDecimalSep());
            else
                sRet.append(',');
            if( !nRet )
            {
                sRet.append('0');
                break;
            }
        }
        else if( !nRet )
            break;
    }
    return sRet.makeStringAndClear();
}

// -----------------------------------------------------------------------

OUString GetSvxString( sal_uInt16 nId )
{
    return EE_RESSTR( nId );
}

// -----------------------------------------------------------------------

OUString GetColorString( const Color& rCol )
{
    OUString sStr;

    ColorData nColData =
        RGB_COLORDATA( rCol.GetRed(), rCol.GetGreen(), rCol.GetBlue() );
    sal_uInt16 nColor = 0;

    static const ColorData aColAry[] = {
        COL_BLACK, COL_BLUE, COL_GREEN, COL_CYAN,
        COL_RED, COL_MAGENTA, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTGREEN, COL_LIGHTCYAN,
        COL_LIGHTRED, COL_LIGHTMAGENTA, COL_YELLOW, COL_WHITE };

    while ( nColor < SAL_N_ELEMENTS(aColAry) &&
            aColAry[nColor] != nColData )
    {
        nColor += 1;
    }

    if ( nColor < SAL_N_ELEMENTS(aColAry) )
        sStr = EE_RESSTR( RID_SVXITEMS_COLOR_BEGIN + nColor + 1 );

    if ( sStr.isEmpty() )
    {
        sStr += "RGB(" +
                OUString::number( rCol.GetRed() )   + OUString(cpDelim) +
                OUString::number( rCol.GetGreen() ) + OUString(cpDelim) +
                OUString::number( rCol.GetBlue() )  + ")";
    }
    return sStr;
}

// -----------------------------------------------------------------------

sal_uInt16 GetMetricId( SfxMapUnit eUnit )
{
    sal_uInt16 nId = RID_SVXITEMS_METRIC_MM;

    switch ( eUnit )
    {
        case SFX_MAPUNIT_100TH_MM:
        case SFX_MAPUNIT_10TH_MM:
        case SFX_MAPUNIT_MM:
            nId = RID_SVXITEMS_METRIC_MM;
            break;

        case SFX_MAPUNIT_CM:
            nId = RID_SVXITEMS_METRIC_CM;
            break;

        case SFX_MAPUNIT_1000TH_INCH:
        case SFX_MAPUNIT_100TH_INCH:
        case SFX_MAPUNIT_10TH_INCH:
        case SFX_MAPUNIT_INCH:
            nId = RID_SVXITEMS_METRIC_INCH;
            break;

        case SFX_MAPUNIT_POINT:
            nId = RID_SVXITEMS_METRIC_POINT;
            break;

        case SFX_MAPUNIT_TWIP:
            nId = RID_SVXITEMS_METRIC_TWIP;
            break;

        case SFX_MAPUNIT_PIXEL:
            nId = RID_SVXITEMS_METRIC_PIXEL;
            break;

        default:
            OSL_FAIL( "not supported mapunit" );
    }
    return nId;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
