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

#include <rtl/ustrbuf.hxx>
#include "propimp0.hxx"
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlimp.hxx>

#include <tools/time.hxx>

using namespace ::com::sun::star;

// implementation of graphic property Stroke

// implementation of presentation page property Change

// implementation of an effect duration property handler

XMLDurationPropertyHdl::~XMLDurationPropertyHdl()
{
}

bool XMLDurationPropertyHdl::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    util::Duration aDuration;

    if (::sax::Converter::convertDuration(aDuration,  rStrImpValue))
    {
        const double fSeconds = ((aDuration.Days * 24 + aDuration.Hours) * 60
                                 + aDuration.Minutes) * 60
                                 + aDuration.Seconds
                                 + aDuration.NanoSeconds / static_cast<double>(::tools::Time::nanoSecPerSec);
        rValue <<= fSeconds;

        return true;
    }

    SAL_WARN_IF(!rStrImpValue.isEmpty(), "xmloff", "Invalid duration: " << rStrImpValue);

    return false;
}

bool XMLDurationPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    double nVal = 0;

    if(rValue >>= nVal)
    {
        util::Duration aDuration;
        aDuration.Seconds = static_cast<sal_uInt16>(nVal);
        aDuration.NanoSeconds = static_cast<sal_uInt32>((nVal - aDuration.Seconds) * ::tools::Time::nanoSecPerSec);

        OUStringBuffer aOut;
        ::sax::Converter::convertDuration(aOut, aDuration);
        rStrExpValue = aOut.makeStringAndClear();
        return true;
    }

    return false;
}

// implementation of an opacity property handler

XMLOpacityPropertyHdl::XMLOpacityPropertyHdl( SvXMLImport* pImport )
: mpImport( pImport )
{
}

XMLOpacityPropertyHdl::~XMLOpacityPropertyHdl()
{
}

bool XMLOpacityPropertyHdl::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Int32 nValue = 0;

    if( rStrImpValue.indexOf( '%' ) != -1 )
    {
        if (::sax::Converter::convertPercent( nValue, rStrImpValue ))
            bRet = true;
    }
    else
    {
        nValue = sal_Int32( rStrImpValue.toDouble() * 100.0 );
        bRet = true;
    }

    if( bRet )
    {
        // check ranges
        if( nValue < 0 )
            nValue = 0;
        if( nValue > 100 )
            nValue = 100;

        // convert xml opacity to api transparency
        nValue = 100 - nValue;

        // #i42959#
        if( mpImport )
        {
            sal_Int32 nUPD, nBuild;
            if( mpImport->getBuildIds( nUPD, nBuild ) )
            {
                // correct import of documents written prior to StarOffice 8/OOO 2.0 final
                if( (nUPD == 680) && (nBuild < 8951) )
                    nValue = 100 - nValue;
            }
        }

        rValue <<= sal_uInt16(nValue);
    }

    return bRet;
}

bool XMLOpacityPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_uInt16 nVal = sal_uInt16();

    if( rValue >>= nVal )
    {
        OUStringBuffer aOut;

        nVal = 100 - nVal;
        ::sax::Converter::convertPercent( aOut, nVal );
        rStrExpValue = aOut.makeStringAndClear();
        bRet = true;
    }

    return bRet;
}

// implementation of an text animation step amount

XMLTextAnimationStepPropertyHdl::~XMLTextAnimationStepPropertyHdl()
{
}

bool XMLTextAnimationStepPropertyHdl::importXML(
    const OUString& rStrImpValue,
    css::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    sal_Int32 nValue = 0;

    const OUString aPX( "px" );
    sal_Int32 nPos = rStrImpValue.indexOf( aPX );
    if( nPos != -1 )
    {
        if (::sax::Converter::convertNumber(nValue, rStrImpValue.copy(0, nPos)))
        {
            rValue <<= sal_Int16( -nValue );
            bRet = true;
        }
    }
    else
    {
        if (rUnitConverter.convertMeasureToCore( nValue, rStrImpValue ))
        {
            rValue <<= sal_Int16( nValue );
            bRet = true;
        }
    }

    return bRet;
}

bool XMLTextAnimationStepPropertyHdl::exportXML(
    OUString& rStrExpValue,
    const css::uno::Any& rValue,
    const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    sal_Int16 nVal = sal_Int16();

    if( rValue >>= nVal )
    {
        OUStringBuffer aOut;

        if( nVal < 0 )
        {
            const OUString aPX( "px" );
            ::sax::Converter::convertNumber( aOut, (sal_Int32)-nVal );
            aOut.append( aPX );
        }
        else
        {
            rUnitConverter.convertMeasureToXML( aOut, nVal );
        }

        rStrExpValue = aOut.makeStringAndClear();
        bRet = true;
    }

    return bRet;
}

#include "sdxmlexp_impl.hxx"

XMLDateTimeFormatHdl::XMLDateTimeFormatHdl( SvXMLExport* pExport )
: mpExport( pExport )
{
}

XMLDateTimeFormatHdl::~XMLDateTimeFormatHdl()
{
}

bool XMLDateTimeFormatHdl::importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    rValue <<= rStrImpValue;
    return true;
}

bool XMLDateTimeFormatHdl::exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Int32 nNumberFormat = 0;
    if( mpExport && (rValue >>= nNumberFormat) )
    {
        mpExport->addDataStyle( nNumberFormat );
        rStrExpValue = mpExport->getDataStyleName( nNumberFormat );
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
