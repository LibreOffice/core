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

SvXMLEnumMapEntry const pXML_Para_Adjust_Enum[] =
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

SvXMLEnumMapEntry const pXML_Para_Align_Last_Enum[] =
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
        return sal_False;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
