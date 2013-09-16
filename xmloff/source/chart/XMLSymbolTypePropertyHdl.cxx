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

#include "XMLSymbolTypePropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::xmloff::token;

namespace
{
struct SvXMLSignedEnumMapEntry
{
    ::xmloff::token::XMLTokenEnum   eToken;
    sal_Int32                       nValue;
};

SvXMLSignedEnumMapEntry aXMLChartSymbolTypeEnumMap[] =
{
    { XML_NONE,                -3 },
    { XML_AUTOMATIC,           -2 },
    { XML_IMAGE,               -1 },
    { XML_TOKEN_INVALID,        0 }
};

SvXMLSignedEnumMapEntry aXMLChartSymbolNameMap[] =
{
    { XML_GRADIENTSTYLE_SQUARE, 0 },  // "square"
    { XML_DIAMOND,              1 },
    { XML_ARROW_DOWN,           2 },
    { XML_ARROW_UP,             3 },
    { XML_ARROW_RIGHT,          4 },
    { XML_ARROW_LEFT,           5 },
    { XML_BOW_TIE,              6 },
    { XML_HOURGLASS,            7 },
    { XML_CIRCLE,               8 },
    { XML_STAR,                 9 },
    { XML_X,                   10 },
    { XML_PLUS,                11 },
    { XML_ASTERISK,            12 },
    { XML_HORIZONTAL_BAR,      13 },
    { XML_VERTICAL_BAR,        14 },
    { XML_TOKEN_INVALID,        0 }
};

bool lcl_convertEnum(
    OUStringBuffer & rBuffer,
    sal_Int32 nValue,
    const SvXMLSignedEnumMapEntry *pMap )
{
    enum XMLTokenEnum eTok = XML_TOKEN_INVALID;

    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( pMap->nValue == nValue )
        {
            eTok = pMap->eToken;
            break;
        }
        pMap++;
    }

    if( eTok != XML_TOKEN_INVALID )
        rBuffer.append( GetXMLToken(eTok) );

    return (eTok != XML_TOKEN_INVALID);
}

bool lcl_convertEnum(
    sal_Int32 & rEnum,
    const OUString & rValue,
    const SvXMLSignedEnumMapEntry *pMap )
{
    while( pMap->eToken != XML_TOKEN_INVALID )
    {
        if( IsXMLToken( rValue, pMap->eToken ) )
        {
            rEnum = pMap->nValue;
            return sal_True;
        }
        pMap++;
    }
    return sal_False;
}

} // anonymous namespace

using namespace com::sun::star;

XMLSymbolTypePropertyHdl::XMLSymbolTypePropertyHdl( bool bIsNamedSymbol )
        : m_bIsNamedSymbol( bIsNamedSymbol )
{}

XMLSymbolTypePropertyHdl::~XMLSymbolTypePropertyHdl()
{}

bool XMLSymbolTypePropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool bResult = false;

    if( m_bIsNamedSymbol )
    {
        sal_Int32 nValue = -3; // NONE
        bResult = lcl_convertEnum( nValue, rStrImpValue, aXMLChartSymbolNameMap );
        rValue <<= nValue;
    }
    else
    {
        sal_Int32 nValue = -3; // NONE
        bResult = lcl_convertEnum( nValue, rStrImpValue, aXMLChartSymbolTypeEnumMap );
        rValue <<= nValue;
    }

    return bResult;
}

bool XMLSymbolTypePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool bResult = false;

    sal_Int32 nType = -3; // NONE
    rValue >>= nType;

    if( m_bIsNamedSymbol )
    {
        OUStringBuffer aBuf;
        bResult = lcl_convertEnum( aBuf, nType, aXMLChartSymbolNameMap );
        rStrExpValue = aBuf.makeStringAndClear();
    }
    else
    {
        if( nType < 0 )
        {
            OUStringBuffer aBuf;
            bResult = lcl_convertEnum( aBuf, nType, aXMLChartSymbolTypeEnumMap );
            rStrExpValue = aBuf.makeStringAndClear();
        }
        else
        {
            bResult = true;
            rStrExpValue = GetXMLToken( XML_NAMED_SYMBOL );
        }
    }

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
