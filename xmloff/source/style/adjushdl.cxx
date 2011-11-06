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
#include <adjushdl.hxx>
#include <tools/solar.h>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;

SvXMLEnumMapEntry __READONLY_DATA pXML_Para_Adjust_Enum[] =
{
    { XML_START,        style::ParagraphAdjust_LEFT },
    { XML_END,          style::ParagraphAdjust_RIGHT },
    { XML_CENTER,       style::ParagraphAdjust_CENTER },
    { XML_JUSTIFY,      style::ParagraphAdjust_BLOCK },
    { XML_JUSTIFIED,    style::ParagraphAdjust_BLOCK }, // obsolete
    { XML_LEFT,         style::ParagraphAdjust_LEFT },
    { XML_RIGHT,        style::ParagraphAdjust_RIGHT },
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry __READONLY_DATA pXML_Para_Align_Last_Enum[] =
{
    { XML_START,        style::ParagraphAdjust_LEFT },
    { XML_CENTER,       style::ParagraphAdjust_CENTER },
    { XML_JUSTIFY,      style::ParagraphAdjust_BLOCK },
    { XML_JUSTIFIED,    style::ParagraphAdjust_BLOCK }, // obsolete
    { XML_TOKEN_INVALID, 0 }
};

///////////////////////////////////////////////////////////////////////////////
//
// class XMLParaAdjustPropHdl
//

XMLParaAdjustPropHdl::~XMLParaAdjustPropHdl()
{
    // nothing to do
}

sal_Bool XMLParaAdjustPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eAdjust;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( eAdjust, rStrImpValue, pXML_Para_Adjust_Enum );
    if( bRet )
        rValue <<= (sal_Int16)eAdjust;

    return bRet;
}

sal_Bool XMLParaAdjustPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    if(!rValue.hasValue())
        return sal_False;     //added by BerryJia for fixing Bug102407 2002-11-5
    OUStringBuffer aOut;
    sal_Int16 nVal = 0;

    rValue >>= nVal;

    sal_Bool bRet = SvXMLUnitConverter::convertEnum( aOut, nVal, pXML_Para_Adjust_Enum, XML_START );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLLastLineAdjustPropHdl
//

XMLLastLineAdjustPropHdl::~XMLLastLineAdjustPropHdl()
{
    // nothing to do
}

sal_Bool XMLLastLineAdjustPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 eAdjust;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( eAdjust, rStrImpValue, pXML_Para_Align_Last_Enum );
    if( bRet )
        rValue <<= (sal_Int16)eAdjust;

    return bRet;
}

sal_Bool XMLLastLineAdjustPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    OUStringBuffer aOut;
    sal_Int16 nVal = 0;
    sal_Bool bRet = sal_False;

    rValue >>= nVal;

    if( nVal != style::ParagraphAdjust_LEFT )
        bRet = SvXMLUnitConverter::convertEnum( aOut, nVal, pXML_Para_Align_Last_Enum, XML_START );

    rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

