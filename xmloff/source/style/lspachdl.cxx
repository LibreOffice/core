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

#include <lspachdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

using namespace ::com::sun::star;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_CASEMAP_NORMAL;

//
// class XMLEscapementPropHdl
//

XMLLineHeightHdl::~XMLLineHeightHdl()
{
    // nothing to do
}

bool XMLLineHeightHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    style::LineSpacing aLSp;
    sal_Int32 nTemp = 0;

    if( -1 != rStrImpValue.indexOf( sal_Unicode( '%' ) ) )
    {
        aLSp.Mode = style::LineSpacingMode::PROP;
        if (!::sax::Converter::convertPercent( nTemp, rStrImpValue ))
            return false;
        aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);
    }
    else if( IsXMLToken( rStrImpValue, XML_CASEMAP_NORMAL) )
    {
        aLSp.Mode = style::LineSpacingMode::PROP;
        aLSp.Height = 100;
    }
    else
    {
        aLSp.Mode = style::LineSpacingMode::FIX;
        if (!rUnitConverter.convertMeasureToCore(
                    nTemp, rStrImpValue, 0x0000, 0xffff))
            return false;
        aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);
    }

    rValue <<= aLSp;
    return true;
}

bool XMLLineHeightHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return false;

    if( style::LineSpacingMode::PROP != aLSp.Mode && style::LineSpacingMode::FIX  != aLSp.Mode )
        return false;

    if( style::LineSpacingMode::PROP == aLSp.Mode )
    {
        ::sax::Converter::convertPercent( aOut, aLSp.Height );
    }
    else
    {
        rUnitConverter.convertMeasureToXML( aOut, aLSp.Height );
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

//
// class XMLLineHeightAtLeastHdl
//

XMLLineHeightAtLeastHdl::~XMLLineHeightAtLeastHdl()
{
    // nothing to do
}

bool XMLLineHeightAtLeastHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    style::LineSpacing aLSp;

    sal_Int32 nTemp;
    aLSp.Mode = style::LineSpacingMode::MINIMUM;
    if (!rUnitConverter.convertMeasureToCore( nTemp, rStrImpValue, 0, 0xffff))
        return false;
    aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);

    rValue <<= aLSp;
    return true;
}

bool XMLLineHeightAtLeastHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return false;

    if( style::LineSpacingMode::MINIMUM != aLSp.Mode )
        return false;

    rUnitConverter.convertMeasureToXML( aOut, aLSp.Height );

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

//
// class XMLLineSpacingHdl
//

XMLLineSpacingHdl::~XMLLineSpacingHdl()
{
    // nothing to do
}

bool XMLLineSpacingHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    style::LineSpacing aLSp;
    sal_Int32 nTemp;

    aLSp.Mode = style::LineSpacingMode::LEADING;
    if (!rUnitConverter.convertMeasureToCore( nTemp, rStrImpValue, 0, 0xffff))
        return false;
    aLSp.Height = sal::static_int_cast< sal_Int16 >(nTemp);

    rValue <<= aLSp;
    return true;
}

bool XMLLineSpacingHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    OUStringBuffer aOut;

    style::LineSpacing aLSp;
    if(!(rValue >>= aLSp))
        return false;

    if( style::LineSpacingMode::LEADING != aLSp.Mode )
        return false;

    rUnitConverter.convertMeasureToXML( aOut, aLSp.Height );

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
