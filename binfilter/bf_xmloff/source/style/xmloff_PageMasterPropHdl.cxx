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

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#include "PageMasterPropHdl.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::comphelper;
using namespace ::binfilter::xmloff::token;


//______________________________________________________________________________

#define DEFAULT_PAPERTRAY   (sal_Int32(-1))


//______________________________________________________________________________
// property handler for style:page-usage (style::PageStyleLayout)

XMLPMPropHdl_PageStyleLayout::~XMLPMPropHdl_PageStyleLayout()
{
}

sal_Bool XMLPMPropHdl_PageStyleLayout::equals( const Any& rAny1, const Any& rAny2 ) const
{
    style::PageStyleLayout eLayout1, eLayout2;
    return ((rAny1 >>= eLayout1) && (rAny2 >>= eLayout2)) ? (eLayout1 == eLayout2) : sal_False;
}

sal_Bool XMLPMPropHdl_PageStyleLayout::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;

    if( IsXMLToken( rStrImpValue, XML_ALL ) )
        rValue <<= PageStyleLayout_ALL;
    else if( IsXMLToken( rStrImpValue, XML_LEFT ) )
        rValue <<= PageStyleLayout_LEFT;
    else if( IsXMLToken( rStrImpValue, XML_RIGHT ) )
        rValue <<= PageStyleLayout_RIGHT;
    else if( IsXMLToken( rStrImpValue, XML_MIRRORED ) )
        rValue <<= PageStyleLayout_MIRRORED;
    else
        bRet = sal_False;

    return bRet;
}

sal_Bool XMLPMPropHdl_PageStyleLayout::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool        bRet = sal_False;
    PageStyleLayout eLayout;

    if( rValue >>= eLayout )
    {
        bRet = sal_True;
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
                bRet = sal_False;
        }
    }

    return bRet;
}


//______________________________________________________________________________
// property handler for style:num-format (style::NumberingType)

XMLPMPropHdl_NumFormat::~XMLPMPropHdl_NumFormat()
{
}

sal_Bool XMLPMPropHdl_NumFormat::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nSync;
    sal_Int16 nNumType = NumberingType::NUMBER_NONE;
    rUnitConverter.convertNumFormat( nNumType, rStrImpValue, OUString(),
                                     sal_True );

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

    return sal_True;
}

sal_Bool XMLPMPropHdl_NumFormat::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool    bRet = sal_False;
    sal_Int16   nNumType;

    if( rValue >>= nNumType )
    {
        OUStringBuffer aBuffer( 10 );
        rUnitConverter.convertNumFormat( aBuffer, nNumType );
        rStrExpValue = aBuffer.makeStringAndClear();
        bRet = sal_True;
    }
    return bRet;
}


//______________________________________________________________________________
// property handler for style:num-letter-sync (style::NumberingType)

XMLPMPropHdl_NumLetterSync::~XMLPMPropHdl_NumLetterSync()
{
}

sal_Bool XMLPMPropHdl_NumLetterSync::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nNumType;
    sal_Int16 nSync = NumberingType::NUMBER_NONE;
    rUnitConverter.convertNumFormat( nSync, rStrImpValue, 
                                     GetXMLToken( XML_A ), sal_True );

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

    return sal_True;
}

sal_Bool XMLPMPropHdl_NumLetterSync::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool    bRet = sal_False;
    sal_Int16   nNumType;

    if( rValue >>= nNumType )
    {
        OUStringBuffer aBuffer( 5 );
        rUnitConverter.convertNumLetterSync( aBuffer, nNumType );
        rStrExpValue = aBuffer.makeStringAndClear();
        bRet = rStrExpValue.getLength() > 0;
    }
    return bRet;
}


//______________________________________________________________________________
// property handler for style:paper-tray-number

XMLPMPropHdl_PaperTrayNumber::~XMLPMPropHdl_PaperTrayNumber()
{
}

sal_Bool XMLPMPropHdl_PaperTrayNumber::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( IsXMLToken( rStrImpValue, XML_DEFAULT ) )
    {
        rValue <<= DEFAULT_PAPERTRAY;
        bRet = sal_True;
    }
    else
    {
        sal_Int32 nPaperTray;
        if( SvXMLUnitConverter::convertNumber( nPaperTray, rStrImpValue, 0 ) )
        {
            rValue <<= nPaperTray;
            bRet = sal_True;
        }
    }

    return bRet;
}

sal_Bool XMLPMPropHdl_PaperTrayNumber::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool    bRet = sal_False;
    sal_Int32   nPaperTray;

    if( rValue >>= nPaperTray )
    {
        if( nPaperTray == DEFAULT_PAPERTRAY )
            rStrExpValue = GetXMLToken( XML_DEFAULT );
        else
        {
            OUStringBuffer aBuffer;
            SvXMLUnitConverter::convertNumber( aBuffer, nPaperTray );
            rStrExpValue = aBuffer.makeStringAndClear();
        }
        bRet = sal_True;
    }
    return bRet;
}


//______________________________________________________________________________
// property handler for style:print

XMLPMPropHdl_Print::XMLPMPropHdl_Print( enum XMLTokenEnum eValue ) :
    sAttrValue( GetXMLToken( eValue ) )
{
}

XMLPMPropHdl_Print::~XMLPMPropHdl_Print()
{
}

sal_Bool XMLPMPropHdl_Print::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Unicode cToken  = ' ';
    sal_Int32   nTokenIndex = 0;
    sal_Bool    bFound  = sal_False;

    do
    {
        bFound = (sAttrValue == rStrImpValue.getToken( 0, cToken, nTokenIndex ));
    }
    while ( (nTokenIndex >= 0) && !bFound );

    setBOOL( rValue, bFound );
    return sal_True;
}

sal_Bool XMLPMPropHdl_Print::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    if( getBOOL( rValue ) )
    {
        if( rStrExpValue.getLength() )
            rStrExpValue += OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) );
        rStrExpValue += sAttrValue;
    }

    return sal_True;
}

//______________________________________________________________________________
// property handler for style:table-centering

XMLPMPropHdl_CenterHorizontal::~XMLPMPropHdl_CenterHorizontal()
{
}

sal_Bool XMLPMPropHdl_CenterHorizontal::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if (rStrImpValue.getLength())
        if (IsXMLToken( rStrImpValue, XML_BOTH) ||
            IsXMLToken( rStrImpValue, XML_HORIZONTAL))
        {
            rValue = ::cppu::bool2any(sal_True);
            bRet = sal_True;
        }


    return bRet;
}

sal_Bool XMLPMPropHdl_CenterHorizontal::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool    bRet = sal_False;

    if ( ::cppu::any2bool( rValue ) )
    {
        bRet = sal_True;
        if (rStrExpValue.getLength())
            rStrExpValue = GetXMLToken(XML_BOTH);
        else
            rStrExpValue = GetXMLToken(XML_HORIZONTAL);
    }

    return bRet;
}

XMLPMPropHdl_CenterVertical::~XMLPMPropHdl_CenterVertical()
{
}

sal_Bool XMLPMPropHdl_CenterVertical::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if (rStrImpValue.getLength())
        if (IsXMLToken(rStrImpValue, XML_BOTH) ||
            IsXMLToken(rStrImpValue, XML_VERTICAL) )
        {
            rValue = ::cppu::bool2any(sal_True);
            bRet = sal_True;
        }

    return bRet;
}

sal_Bool XMLPMPropHdl_CenterVertical::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool    bRet = sal_False;

    if ( ::cppu::any2bool( rValue ) )
    {
        bRet = sal_True;
        if (rStrExpValue.getLength())
            rStrExpValue = GetXMLToken(XML_BOTH);
        else
            rStrExpValue = GetXMLToken(XML_VERTICAL);
    }

    return bRet;
}

}//end of namespace binfilter
