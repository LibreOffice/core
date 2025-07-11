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

#include <sal/config.h>

#include <osl/diagnose.h>
#include <vcl/outdev.hxx>
#include <editeng/editrids.hrc>
#include <unotools/intlwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/itemtype.hxx>
#include <editeng/eerdll.hxx>
#include <rtl/ustrbuf.hxx>


OUString GetMetricText( tools::Long nVal, MapUnit eSrcUnit, MapUnit eDestUnit, const IntlWrapper* pIntl )
{
    bool bNeg = false;
    bool bShowAtLeastOneDecimalDigit = true;
    sal_Int32 nRet = 0;

    if ( nVal < 0 )
    {
        bNeg = true;
        nVal *= -1;
    }

    switch ( eDestUnit )
    {
        case MapUnit::Map100thMM:
        case MapUnit::Map10thMM:
        case MapUnit::MapMM:
        case MapUnit::MapCM:
        {
            nRet = OutputDevice::LogicToLogic( nVal, eSrcUnit, MapUnit::Map100thMM );

            switch ( eDestUnit )
            {
                case MapUnit::Map100thMM:  nRet *= 1000; break;
                case MapUnit::Map10thMM:   nRet *= 100; break;
                case MapUnit::MapMM:        nRet *= 10; break;
                default: ;//prevent warning
            }
            break;
        }

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
        {
            nRet = OutputDevice::LogicToLogic( nVal, eSrcUnit, MapUnit::Map1000thInch );

            switch ( eDestUnit )
            {
                case MapUnit::Map1000thInch:   nRet *= 1000; break;
                case MapUnit::Map100thInch:    nRet *= 100; break;
                case MapUnit::Map10thInch:     nRet *= 10; break;
                default: ;//prevent warning
            }
            break;
        }

        case MapUnit::MapPoint:
            // fractions of a point are used, e.g., for font size
            nRet = OutputDevice::LogicToLogic(nVal, eSrcUnit, MapUnit::MapTwip) * 50;
            bShowAtLeastOneDecimalDigit = false;
            break;

        case MapUnit::MapTwip:
        case MapUnit::MapPixel:
            return OUString::number( OutputDevice::LogicToLogic(
                        nVal, eSrcUnit, eDestUnit ));

        default:
            OSL_FAIL( "not supported mapunit" );
            return OUString();
    }

    if ( MapUnit::MapCM == eDestUnit || MapUnit::MapInch == eDestUnit )
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

    tools::Long nDiff = 1000;
    for( int nDigits = 4; nDigits; --nDigits, nDiff /= 10 )
    {
        if ( nRet < nDiff )
            sRet.append('0');
        else
            sRet.append(nRet / nDiff);
        nRet %= nDiff;
        if( 4 == nDigits && (bShowAtLeastOneDecimalDigit || nRet) )
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

OUString GetColorString( const Color& rCol )
{
    if (rCol == COL_AUTO)
        return EditResId(RID_SVXSTR_AUTOMATIC);

    static const Color aColAry[] = {
        COL_BLACK, COL_BLUE, COL_GREEN, COL_CYAN,
        COL_RED, COL_MAGENTA, COL_BROWN, COL_GRAY,
        COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTGREEN, COL_LIGHTCYAN,
        COL_LIGHTRED, COL_LIGHTMAGENTA, COL_YELLOW, COL_WHITE };

    sal_uInt16 nColor = 0;
    while ( nColor < std::size(aColAry) &&
            aColAry[nColor] != rCol.GetRGBColor() )
    {
        nColor += 1;
    }

    static const TranslateId RID_SVXITEMS_COLORS[] =
    {
        RID_SVXITEMS_COLOR_BLACK,
        RID_SVXITEMS_COLOR_BLUE,
        RID_SVXITEMS_COLOR_GREEN,
        RID_SVXITEMS_COLOR_CYAN,
        RID_SVXITEMS_COLOR_RED,
        RID_SVXITEMS_COLOR_MAGENTA,
        RID_SVXITEMS_COLOR_BROWN,
        RID_SVXITEMS_COLOR_GRAY,
        RID_SVXITEMS_COLOR_LIGHTGRAY,
        RID_SVXITEMS_COLOR_LIGHTBLUE,
        RID_SVXITEMS_COLOR_LIGHTGREEN,
        RID_SVXITEMS_COLOR_LIGHTCYAN,
        RID_SVXITEMS_COLOR_LIGHTRED,
        RID_SVXITEMS_COLOR_LIGHTMAGENTA,
        RID_SVXITEMS_COLOR_YELLOW,
        RID_SVXITEMS_COLOR_WHITE
    };

    static_assert(std::size(aColAry) == std::size(RID_SVXITEMS_COLORS), "must match");

    OUString sStr;
    if ( nColor < std::size(aColAry) )
        sStr = EditResId(RID_SVXITEMS_COLORS[nColor]);

    if ( sStr.isEmpty() )
    {
        sStr += "RGB(" +
                OUString::number( rCol.GetRed() )   + cpDelim +
                OUString::number( rCol.GetGreen() ) + cpDelim +
                OUString::number( rCol.GetBlue() )  + ")";
    }
    return sStr;
}

TranslateId GetMetricId( MapUnit eUnit )
{
    TranslateId pId = RID_SVXITEMS_METRIC_MM;

    switch ( eUnit )
    {
        case MapUnit::Map100thMM:
        case MapUnit::Map10thMM:
        case MapUnit::MapMM:
            pId = RID_SVXITEMS_METRIC_MM;
            break;

        case MapUnit::MapCM:
            pId = RID_SVXITEMS_METRIC_CM;
            break;

        case MapUnit::Map1000thInch:
        case MapUnit::Map100thInch:
        case MapUnit::Map10thInch:
        case MapUnit::MapInch:
            pId = RID_SVXITEMS_METRIC_INCH;
            break;

        case MapUnit::MapPoint:
            pId = RID_SVXITEMS_METRIC_POINT;
            break;

        case MapUnit::MapTwip:
            pId = RID_SVXITEMS_METRIC_TWIP;
            break;

        case MapUnit::MapPixel:
            pId = RID_SVXITEMS_METRIC_PIXEL;
            break;

        default:
            OSL_FAIL( "not supported mapunit" );
    }
    return pId;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
