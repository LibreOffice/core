/*************************************************************************
 *
 *  $RCSfile: PageMasterPropHdl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-18 11:30:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERPROPHDL_HXX_
#include "PageMasterPropHdl.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLNUMI_HXX
#include "xmlnumi.hxx"
#endif
#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_PAGESTYLELAYOUT_HPP_
#include <com/sun/star/style/PageStyleLayout.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;


//______________________________________________________________________________

#define DEFAULT_PAPERTRAY   (sal_Int32(-1))


//______________________________________________________________________________
// property handler for style::PageStyleLayout

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

    if( rStrImpValue.compareToAscii( sXML_all ) == 0 )
        rValue <<= PageStyleLayout_ALL;
    else if( rStrImpValue.compareToAscii( sXML_left ) == 0 )
        rValue <<= PageStyleLayout_LEFT;
    else if( rStrImpValue.compareToAscii( sXML_right ) == 0 )
        rValue <<= PageStyleLayout_RIGHT;
    else if( rStrImpValue.compareToAscii( sXML_mirrored ) == 0 )
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
                rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_all ) );
            break;
            case PageStyleLayout_LEFT:
                rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_left ) );
            break;
            case PageStyleLayout_RIGHT:
                rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_right ) );
            break;
            case PageStyleLayout_MIRRORED:
                rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_mirrored ) );
            break;
            default:
                bRet = sal_False;
        }
    }

    return bRet;
}


//______________________________________________________________________________
// property handler for style::NumberingType (num-format)

XMLPMPropHdl_NumFormat::~XMLPMPropHdl_NumFormat()
{
}

sal_Bool XMLPMPropHdl_NumFormat::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nSync;
    sal_Int16 nNumType = SvxXMLListStyleContext::GetNumType(
                          rStrImpValue, OUString(), NumberingType::NUMBER_NONE, sal_True );

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
        const char* pAsciiNumType = SvxXMLNumRuleExport::GetNumFormatValue( nNumType );
        if( pAsciiNumType )
        {
            rStrExpValue = OUString::createFromAscii( pAsciiNumType );
            bRet = sal_True;
        }
    }
    return bRet;
}


//______________________________________________________________________________
// property handler for style::NumberingType (num-letter-sync)

XMLPMPropHdl_NumLetterSync::~XMLPMPropHdl_NumLetterSync()
{
}

sal_Bool XMLPMPropHdl_NumLetterSync::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Int16 nNumType;
    sal_Int16 nSync = SvxXMLListStyleContext::GetNumType(
                           OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_a ) ), rStrImpValue );

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
        const char* pAsciiNumType = SvxXMLNumRuleExport::GetNumLetterSync( nNumType );
        if( pAsciiNumType )
        {
            rStrExpValue = OUString::createFromAscii( pAsciiNumType );
            bRet = sal_True;
        }
    }
    return bRet;
}


//______________________________________________________________________________
// property handler for paper-tray-number

XMLPMPropHdl_PaperTrayNumber::~XMLPMPropHdl_PaperTrayNumber()
{
}

sal_Bool XMLPMPropHdl_PaperTrayNumber::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    if( rStrImpValue.compareToAscii( sXML_default ) == 0 )
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
            rStrExpValue = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_default ) );
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
// property handler for print-orientation

XMLPMPropHdl_PrintOrientation::~XMLPMPropHdl_PrintOrientation()
{
}

sal_Bool XMLPMPropHdl_PrintOrientation::importXML(
        const OUString& rStrImpValue,
        Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_True;

    if( rStrImpValue.compareToAscii( sXML_landscape ) == 0 )
        rValue <<= sal_True;
    else if( rStrImpValue.compareToAscii( sXML_portrait ) == 0 )
        rValue <<= sal_False;
    else
        bRet = sal_False;

    return bRet;
}

sal_Bool XMLPMPropHdl_PrintOrientation::exportXML(
        OUString& rStrExpValue,
        const Any& rValue,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;
    sal_Bool bOrientation;

    if( rValue >>= bOrientation )
    {
        rStrExpValue = bOrientation ?
            OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_landscape ) ) :
            OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_portrait ) );
        bRet = sal_True;
    }
    return bRet;
}

