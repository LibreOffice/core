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

#include "PageMasterPropHdl.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlnume.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::comphelper;
using namespace ::xmloff::token;

#define DEFAULT_PAPERTRAY   (sal_Int32(-1))

// property handler for style:page-usage (style::PageStyleLayout)

XMLPMPropHdl_PageStyleLayout::~XMLPMPropHdl_PageStyleLayout()
{
}

bool XMLPMPropHdl_PageStyleLayout::equals( const Any& rAny1, const Any& rAny2 ) const
{
    style::PageStyleLayout eLayout1, eLayout2;
    return (rAny1 >>= eLayout1) && (rAny2 >>= eLayout2) && (eLayout1 == eLayout2);
}

bool XMLPMPropHdl_PageStyleLayout::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = true;

    if( IsXMLToken( rStrImpValue, XML_ALL ) )
        rValue <<= PageStyleLayout_ALL;
    else if( IsXMLToken( rStrImpValue, XML_LEFT ) )
        rValue <<= PageStyleLayout_LEFT;
    else if( IsXMLToken( rStrImpValue, XML_RIGHT ) )
        rValue <<= PageStyleLayout_RIGHT;
    else if( IsXMLToken( rStrImpValue, XML_MIRRORED ) )
        rValue <<= PageStyleLayout_MIRRORED;
    else
        bRet = false;

    return bRet;
}

bool XMLPMPropHdl_PageStyleLayout::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    PageStyleLayout eLayout;

    if( rValue >>= eLayout )
    {
        bRet = true;
        switch( eLayout )
        {
            case PageStyleLayout_ALL:
                rStrExpValue = GetXMLToken( XML_ALL );
            break;
            case PageStyleLayout_LEFT:
                rStrExpValue = GetXMLToken( XML_LEFT );
            break;
            case PageStyleLayout_RIGHT:
                rStrExpValue = GetXMLToken( XML_RIGHT );
            break;
            case PageStyleLayout_MIRRORED:
                rStrExpValue = GetXMLToken( XML_MIRRORED );
            break;
            default:
                bRet = false;
        }
    }

    return bRet;
}

// property handler for style:num-format (style::NumberingType)

XMLPMPropHdl_NumFormat::~XMLPMPropHdl_NumFormat()
{
}

bool XMLPMPropHdl_NumFormat::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nSync = sal_Int16();
    sal_Int16 nNumType = NumberingType::NUMBER_NONE;
    rUnitConverter.convertNumFormat( nNumType, rStrImpValue, OUString(), true );

    if( !(rValue >>= nSync) )
        nSync = NumberingType::NUMBER_NONE;

    // if num-letter-sync appears before num-format, the function
    // XMLPMPropHdl_NumLetterSync::importXML() sets the value
    // NumberingType::CHARS_LOWER_LETTER_N
    if( nSync == NumberingType::CHARS_LOWER_LETTER_N )
    {
        switch( nNumType )
        {
            case NumberingType::CHARS_LOWER_LETTER:
                nNumType = NumberingType::CHARS_LOWER_LETTER_N;
            break;
            case NumberingType::CHARS_UPPER_LETTER:
                nNumType = NumberingType::CHARS_UPPER_LETTER_N;
            break;
        }
    }
    rValue <<= nNumType;

    return true;
}

bool XMLPMPropHdl_NumFormat::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;
    sal_Int16   nNumType = sal_Int16();

    if( rValue >>= nNumType )
    {
        OUStringBuffer aBuffer( 10 );
        rUnitConverter.convertNumFormat( aBuffer, nNumType );
        rStrExpValue = aBuffer.makeStringAndClear();
        bRet = true;
    }
    return bRet;
}

// property handler for style:num-letter-sync (style::NumberingType)

XMLPMPropHdl_NumLetterSync::~XMLPMPropHdl_NumLetterSync()
{
}

bool XMLPMPropHdl_NumLetterSync::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nNumType;
    sal_Int16 nSync = NumberingType::NUMBER_NONE;
    rUnitConverter.convertNumFormat( nSync, rStrImpValue,
                                     GetXMLToken( XML_A ), true );

    if( !(rValue >>= nNumType) )
        nNumType = NumberingType::NUMBER_NONE;

    if( nSync == NumberingType::CHARS_LOWER_LETTER_N )
    {
        switch( nNumType )
        {
            case NumberingType::CHARS_LOWER_LETTER:
                nNumType = NumberingType::CHARS_LOWER_LETTER_N;
            break;
            case NumberingType::CHARS_UPPER_LETTER:
                nNumType = NumberingType::CHARS_UPPER_LETTER_N;
            break;
        }
    }
    rValue <<= nNumType;

    return true;
}

bool XMLPMPropHdl_NumLetterSync::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool        bRet = false;
    sal_Int16   nNumType = sal_Int16();

    if( rValue >>= nNumType )
    {
        OUStringBuffer aBuffer( 5 );
        SvXMLUnitConverter::convertNumLetterSync( aBuffer, nNumType );
        rStrExpValue = aBuffer.makeStringAndClear();
        bRet = !rStrExpValue.isEmpty();
    }
    return bRet;
}

// property handler for style:paper-tray-number

XMLPMPropHdl_PaperTrayNumber::~XMLPMPropHdl_PaperTrayNumber()
{
}

bool XMLPMPropHdl_PaperTrayNumber::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if( IsXMLToken( rStrImpValue, XML_DEFAULT ) )
    {
        rValue <<= DEFAULT_PAPERTRAY;
        bRet = true;
    }
    else
    {
        sal_Int32 nPaperTray;
        if (::sax::Converter::convertNumber( nPaperTray, rStrImpValue, 0 ))
        {
            rValue <<= nPaperTray;
            bRet = true;
        }
    }

    return bRet;
}

bool XMLPMPropHdl_PaperTrayNumber::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool        bRet = false;
    sal_Int32   nPaperTray = 0;

    if( rValue >>= nPaperTray )
    {
        if( nPaperTray == DEFAULT_PAPERTRAY )
            rStrExpValue = GetXMLToken( XML_DEFAULT );
        else
        {
            rStrExpValue = OUString::number( nPaperTray );
        }
        bRet = true;
    }
    return bRet;
}

// property handler for style:print

XMLPMPropHdl_Print::XMLPMPropHdl_Print( enum XMLTokenEnum eValue ) :
    sAttrValue( GetXMLToken( eValue ) )
{
}

XMLPMPropHdl_Print::~XMLPMPropHdl_Print()
{
}

bool XMLPMPropHdl_Print::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& ) const
{
    sal_Unicode cToken  = ' ';
    sal_Int32   nTokenIndex = 0;
    bool        bFound  = false;

    do
    {
        bFound = (sAttrValue == rStrImpValue.getToken( 0, cToken, nTokenIndex ));
    }
    while ( (nTokenIndex >= 0) && !bFound );

    rValue <<= bFound;
    return true;
}

bool XMLPMPropHdl_Print::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    if( getBOOL( rValue ) )
    {
        if( !rStrExpValue.isEmpty() )
            rStrExpValue += " ";
        rStrExpValue += sAttrValue;
    }

    return true;
}

// property handler for style:table-centering

XMLPMPropHdl_CenterHorizontal::~XMLPMPropHdl_CenterHorizontal()
{
}

bool XMLPMPropHdl_CenterHorizontal::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if (!rStrImpValue.isEmpty())
        if (IsXMLToken( rStrImpValue, XML_BOTH) ||
            IsXMLToken( rStrImpValue, XML_HORIZONTAL))
        {
            rValue <<= true;
            bRet = true;
        }

    return bRet;
}

bool XMLPMPropHdl_CenterHorizontal::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if ( ::cppu::any2bool( rValue ) )
    {
        bRet = true;
        if (!rStrExpValue.isEmpty())
            rStrExpValue = GetXMLToken(XML_BOTH);
        else
            rStrExpValue = GetXMLToken(XML_HORIZONTAL);
    }

    return bRet;
}

XMLPMPropHdl_CenterVertical::~XMLPMPropHdl_CenterVertical()
{
}

bool XMLPMPropHdl_CenterVertical::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if (!rStrImpValue.isEmpty())
        if (IsXMLToken(rStrImpValue, XML_BOTH) ||
            IsXMLToken(rStrImpValue, XML_VERTICAL) )
        {
            rValue <<= true;
            bRet = true;
        }

    return bRet;
}

bool XMLPMPropHdl_CenterVertical::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& ) const
{
    bool bRet = false;

    if ( ::cppu::any2bool( rValue ) )
    {
        bRet = true;
        if (!rStrExpValue.isEmpty())
            rStrExpValue = GetXMLToken(XML_BOTH);
        else
            rStrExpValue = GetXMLToken(XML_VERTICAL);
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
