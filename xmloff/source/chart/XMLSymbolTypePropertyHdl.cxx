/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
