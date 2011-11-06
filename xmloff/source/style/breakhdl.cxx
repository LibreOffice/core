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
#include <breakhdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/uno/Any.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry pXML_BreakTypes[] =
{
    { XML_AUTO,         0 },
    { XML_COLUMN,       1 },
    { XML_PAGE,         2 },
    { XML_EVEN_PAGE,    2 },
    { XML_ODD_PAGE,     2 },
    { XML_TOKEN_INVALID, 0}
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtBreakBeforePropHdl
//

XMLFmtBreakBeforePropHdl::~XMLFmtBreakBeforePropHdl()
{
    // Nothing to do
}

sal_Bool XMLFmtBreakBeforePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_BEFORE;
            break;
        default:
            eBreak = style::BreakType_PAGE_BEFORE;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

sal_Bool XMLFmtBreakBeforePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return sal_False;

        eBreak = (style::BreakType) nValue;
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_BEFORE:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_BEFORE:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return sal_False;
    }

    OUStringBuffer aOut;
    /* sal_Bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLFmtBreakBeforePropHdl
//

XMLFmtBreakAfterPropHdl::~XMLFmtBreakAfterPropHdl()
{
    // Nothing to do
}

sal_Bool XMLFmtBreakAfterPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_AFTER;
            break;
        default:
            eBreak = style::BreakType_PAGE_AFTER;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

sal_Bool XMLFmtBreakAfterPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return sal_False;

        eBreak = (style::BreakType) nValue;
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_AFTER:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_AFTER:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return sal_False;
    }

    OUStringBuffer aOut;
    /* sal_Bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return sal_True;
}
