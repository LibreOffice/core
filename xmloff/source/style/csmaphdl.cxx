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

#include <csmaphdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

static SvXMLEnumMapEntry pXML_Casemap_Enum[] =
{
    { XML_NONE,                 style::CaseMap::NONE },
    { XML_CASEMAP_LOWERCASE,    style::CaseMap::LOWERCASE },
    { XML_CASEMAP_UPPERCASE,    style::CaseMap::UPPERCASE },
    { XML_CASEMAP_CAPITALIZE,   style::CaseMap::TITLE },
    { XML_TOKEN_INVALID,        0 }
};

// class XMLPosturePropHdl

XMLCaseMapPropHdl::~XMLCaseMapPropHdl()
{
    // nothing to do
}

bool XMLCaseMapPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nVal;
    bool bRet = SvXMLUnitConverter::convertEnum(
        nVal, rStrImpValue, pXML_Casemap_Enum );
    if( ( bRet ) )
        rValue <<= nVal;

    return bRet;
}

bool XMLCaseMapPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_uInt16 nValue = sal_uInt16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        bRet = SvXMLUnitConverter::convertEnum(
            aOut, nValue, pXML_Casemap_Enum );
        if( bRet )
            rStrExpValue = aOut.makeStringAndClear();
    }

    return bRet;
}

// class XMLCaseMapVariantHdl

XMLCaseMapVariantHdl::~XMLCaseMapVariantHdl()
{
    // nothing to do
}

bool XMLCaseMapVariantHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if( IsXMLToken( rStrImpValue, XML_CASEMAP_SMALL_CAPS ) )
    {
        rValue <<= (sal_Int16)style::CaseMap::SMALLCAPS;
        bRet = true;
    }
    else if( IsXMLToken( rStrImpValue, XML_CASEMAP_NORMAL ) )
    {
        rValue <<= (sal_Int16)style::CaseMap::NONE;
        bRet = true;
    }

    return bRet;
}

bool XMLCaseMapVariantHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nValue = sal_uInt16();
    OUStringBuffer aOut;

    if( rValue >>= nValue )
    {
        switch( nValue )
        {
        case style::CaseMap::NONE:
            aOut.append( GetXMLToken(XML_CASEMAP_NORMAL) );
            break;
        case style::CaseMap::SMALLCAPS:
            aOut.append( GetXMLToken(XML_CASEMAP_SMALL_CAPS) );
            break;
        }
    }

    rStrExpValue = aOut.makeStringAndClear();
    return !rStrExpValue.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
