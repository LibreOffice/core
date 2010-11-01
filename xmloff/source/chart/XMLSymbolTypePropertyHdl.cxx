/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLSymbolTypePropertyHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/chart/ChartErrorIndicatorType.hpp>
#include <rtl/ustrbuf.hxx>

using namespace ::xmloff::token;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
    { XML_TOKEN_INVALID,        0 }
};

sal_Bool lcl_convertEnum(
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

sal_Bool lcl_convertEnum(
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

sal_Bool XMLSymbolTypePropertyHdl::importXML( const OUString& rStrImpValue,
                                                  uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

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

sal_Bool XMLSymbolTypePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bResult = sal_False;

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
