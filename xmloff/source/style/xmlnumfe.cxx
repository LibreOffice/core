/*************************************************************************
 *
 *  $RCSfile: xmlnumfe.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-19 18:07:10 $
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

#define _SVSTDARR_ULONGS
#define _ZFORLIST_DECLARE_TABLE

#include <svtools/svstdarr.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <svtools/numuno.hxx>
#include <tools/isolang.hxx>
#include <tools/debug.hxx>
#include <tools/solmath.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>

#include "xmlnumfe.hxx"
#include "xmlnmspe.hxx"
#include "xmlkywd.hxx"
#include "xmluconv.hxx"
#include "attrlist.hxx"
#include "nmspmap.hxx"
#include "families.hxx"
#include "xmlnumfi.hxx"     // SvXMLNumFmtDefaults

using namespace ::rtl;
using namespace ::com::sun::star;

//-------------------------------------------------------------------------

//! enum Sc_SymbolType is in source/numbers/zforscan.hxx

#define XMLNUM_SYMBOLTYPE_STRING     (-1)
#define XMLNUM_SYMBOLTYPE_DEL        (-2)
#define XMLNUM_SYMBOLTYPE_BLANK      (-3)
#define XMLNUM_SYMBOLTYPE_STAR       (-4)
#define XMLNUM_SYMBOLTYPE_DIGIT      (-5)
#define XMLNUM_SYMBOLTYPE_DECSEP     (-6)
#define XMLNUM_SYMBOLTYPE_THSEP      (-7)
#define XMLNUM_SYMBOLTYPE_EXP        (-8)
#define XMLNUM_SYMBOLTYPE_FRAC       (-9)
#define XMLNUM_SYMBOLTYPE_EMPTY      (-10)
#define XMLNUM_SYMBOLTYPE_FRACBLANK  (-11)
#define XMLNUM_SYMBOLTYPE_COMMENT    (-12)
#define XMLNUM_SYMBOLTYPE_CURRENCY   (-13)
#define XMLNUM_SYMBOLTYPE_CURRDEL    (-14)
#define XMLNUM_SYMBOLTYPE_CURREXT    (-15)
#define XMLNUM_SYMBOLTYPE_CALENDAR   (-16)
#define XMLNUM_SYMBOLTYPE_CALDEL     (-17)

//-------------------------------------------------------------------------

//  4th condition for text formats doesn't work
//#define XMLNUM_MAX_PARTS  4
#define XMLNUM_MAX_PARTS    3

//-------------------------------------------------------------------------

class SvXMLNumUsedList_Impl
{
    SvULongs    aUsed;
    SvULongs    aWasUsed;

public:
            SvXMLNumUsedList_Impl();
            ~SvXMLNumUsedList_Impl();

    void        SetUsed( sal_uInt32 nKey );
    sal_Bool    IsUsed( sal_uInt32 nKey ) const;
    sal_Bool    IsWasUsed( sal_uInt32 nKey ) const;
    void        Export();
};

//-------------------------------------------------------------------------

//
//! SvXMLNumUsedList_Impl should be optimized!
//

SvXMLNumUsedList_Impl::SvXMLNumUsedList_Impl()
{
}

SvXMLNumUsedList_Impl::~SvXMLNumUsedList_Impl()
{
}

void SvXMLNumUsedList_Impl::SetUsed( sal_uInt32 nKey )
{
    if ( !IsUsed( nKey ) && !IsWasUsed(nKey) )
        aUsed.Insert( nKey, aUsed.Count() );
}

sal_Bool SvXMLNumUsedList_Impl::IsUsed( sal_uInt32 nKey ) const
{
    sal_uInt16 nCount = aUsed.Count();
    for ( sal_uInt16 i = 0; i < nCount; i++ )
        if ( aUsed[i] == nKey )
            return sal_True;

    return sal_False;
}

sal_Bool SvXMLNumUsedList_Impl::IsWasUsed( sal_uInt32 nKey ) const
{
    sal_uInt16 nCount = aWasUsed.Count();
    for ( sal_uInt16 i = 0; i < nCount; i++ )
        if ( aWasUsed[i] == nKey )
            return sal_True;

    return sal_False;
}

void SvXMLNumUsedList_Impl::Export()
{
    sal_uInt16 nCount = aUsed.Count();
    for (sal_uInt16 i = 0; i < nCount; i++)
        aWasUsed.Insert( aUsed[i], aWasUsed.Count());
    aUsed.Remove(0, nCount);
}

//-------------------------------------------------------------------------

SvXMLNumFmtExport::SvXMLNumFmtExport(
            const uno::Reference< xml::sax::XDocumentHandler >& rHdl,
            const uno::Reference< util::XNumberFormatsSupplier >& rSupp ) :
    xHandler( rHdl ),
    pFormatter( NULL ),
    pNamespaceMap( NULL ),
    sCDATA( OUString::createFromAscii( sXML_CDATA ) ),
    sWS( OUString::createFromAscii( sXML_WS ) ),
    pCharClass( NULL ),
    pLocaleData( NULL )
{
    //  supplier must be SvNumberFormatsSupplierObj
    SvNumberFormatsSupplierObj* pObj =
                    SvNumberFormatsSupplierObj::getImplementation( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    if ( pFormatter )
    {
        pCharClass = new CharClass( pFormatter->GetServiceManager(),
            pFormatter->GetLocale() );
        pLocaleData = new LocaleDataWrapper( pFormatter->GetServiceManager(),
            pFormatter->GetLocale() );
    }
    else
    {
        lang::Locale aLocale( SvNumberFormatter::ConvertLanguageToLocale( ::GetSystemLanguage() ) );
        pCharClass = new CharClass( ::comphelper::getProcessServiceFactory(), aLocale );
        pLocaleData = new LocaleDataWrapper( ::comphelper::getProcessServiceFactory(), aLocale );
    }

    pAttrList = new SvXMLAttributeList;
    xAttrList = pAttrList;

    pUsedList = new SvXMLNumUsedList_Impl;
}

SvXMLNumFmtExport::~SvXMLNumFmtExport()
{
    delete pUsedList;
    delete pLocaleData;
    delete pCharClass;
}

//-------------------------------------------------------------------------

//
//  helper methods
//

OUString lcl_CreateStyleName( sal_Int32 nKey, sal_Int32 nPart, sal_Bool bDefPart )
{
    OUStringBuffer aFmtName( 10L );
    aFmtName.append( (sal_Unicode)'N' );
    aFmtName.append( nKey );
    if (!bDefPart)
    {
        aFmtName.append( (sal_Unicode)'P' );
        aFmtName.append( nPart );
    }
    return aFmtName.makeStringAndClear();
}

void SvXMLNumFmtExport::AddCalendarAttr_Impl( const OUString& rCalendar )
{
    if ( rCalendar.getLength() )
    {
        OUString sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                                OUString::createFromAscii( sXML_calendar ) );
        OUString sAttrValue = rCalendar;
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }
}

void SvXMLNumFmtExport::AddTextualAttr_Impl( sal_Bool bText )
{
    if ( bText )            // non-textual
    {
        OUString sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                                OUString::createFromAscii( sXML_textual ) );
        OUString sAttrValue = OUString::createFromAscii( sXML_true );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }
}

void SvXMLNumFmtExport::AddStyleAttr_Impl( sal_Bool bLong )
{
    if ( bLong )            // short is default
    {
        OUString sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                                OUString::createFromAscii( sXML_style ) );
        OUString sAttrValue = OUString::createFromAscii( sXML_long );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }
}

void SvXMLNumFmtExport::AddLanguageAttr_Impl( sal_Int32 nLang )
{
    if ( nLang != LANGUAGE_SYSTEM )
    {
        String aLangStr, aCountryStr;
        ConvertLanguageToIsoNames( (LanguageType)nLang, aLangStr, aCountryStr );

        OUString sAttrName, sAttrValue;
        if (aLangStr.Len())
        {
            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER, OUString::createFromAscii(sXML_language) );
            sAttrValue = aLangStr;
            pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
        }
        if (aCountryStr.Len())
        {
            sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER, OUString::createFromAscii(sXML_country) );
            sAttrValue = aCountryStr;
            pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
        }
    }
}

//-------------------------------------------------------------------------

//
//  methods to write individual elements within a format
//

void SvXMLNumFmtExport::AddToTextElement_Impl( const OUString& rString )
{
    //  append to sTextContent, write element in FinishTextElement_Impl
    //  to avoid several text elements following each other

    sTextContent.append( rString );
}

void SvXMLNumFmtExport::FinishTextElement_Impl()
{
    DBG_ASSERT( !xAttrList->getLength(), "FinishTextElement_Impl: AttrList not empty" );

    if ( sTextContent.getLength() )
    {
        OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                            OUString::createFromAscii( sXML_text ) );
        OUString sContentStr = sTextContent.makeStringAndClear();

        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->characters( sContentStr );
        xHandler->endElement( sElem );
    }
}

void SvXMLNumFmtExport::WriteColorElement_Impl( const Color& rColor )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE,
                        OUString::createFromAscii( sXML_properties ) );

    OUString sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_FO,
                            OUString::createFromAscii(sXML_color) );
    OUStringBuffer aColStr( 7 );
    SvXMLUnitConverter::convertColor( aColStr, rColor );
    OUString sAttrValue = aColStr.makeStringAndClear();
    pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteCurrencyElement_Impl( const OUString& rString,
                                                    const OUString& rExt )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_currency_symbol ) );

    if ( rExt.getLength() )
    {
        sal_Int32 nLang = rExt.toInt32(16);     // hex
        if ( nLang < 0 )                        // extension string may contain "-" separator
            nLang = -nLang;
        AddLanguageAttr_Impl( nLang );          // adds to pAttrList
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->characters( rString );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteBooleanElement_Impl()
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_boolean ) );
    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
}

void SvXMLNumFmtExport::WriteTextContentElement_Impl()
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_text_content ) );
    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
}

//  date elements

void SvXMLNumFmtExport::WriteDayElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_day ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteMonthElement_Impl( const OUString& rCalendar, sal_Bool bLong, sal_Bool bText )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_month ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    AddTextualAttr_Impl( bText );   // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteYearElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_year ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteEraElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_era ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteDayOfWeekElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_day_of_week ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteWeekElement_Impl( const OUString& rCalendar )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_week_of_year ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
}

void SvXMLNumFmtExport::WriteQuarterElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_quarter ) );

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

//  time elements

void SvXMLNumFmtExport::WriteHoursElement_Impl( sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_hours ) );

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteMinutesElement_Impl( sal_Bool bLong )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_minutes ) );

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteSecondsElement_Impl( sal_Bool bLong, sal_uInt16 nDecimals )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_seconds ) );

    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    if ( nDecimals > 0 )
    {
        OUString sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                                OUString::createFromAscii( sXML_decimal_places ) );
        OUString sAttrValue = OUString::valueOf( (sal_Int32) nDecimals );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );

    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteAMPMElement_Impl()
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_am_pm ) );
    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
}

//  numbers

void SvXMLNumFmtExport::WriteNumberElement_Impl(
                            sal_Int32 nDecimals, sal_Int32 nInteger,
                            const OUString& rDashStr, sal_Bool bGrouping )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_number ) );
    OUString sAttrName, sAttrValue;

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_decimal_places ) );
        sAttrValue = OUString::valueOf( nDecimals );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_integer_digits ) );
        sAttrValue = OUString::valueOf( nInteger );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  decimal replacement (dashes)
    if ( rDashStr.getLength() )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_decimal_replacement ) );
        sAttrValue = rDashStr;
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_grouping ) );
        sAttrValue = OUString::createFromAscii(sXML_true);
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteScientificElement_Impl(
                            sal_Int32 nDecimals, sal_Int32 nInteger,
                            sal_Bool bGrouping, sal_Int32 nExp )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_scientific_number ) );
    OUString sAttrName, sAttrValue;

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_decimal_places ) );
        sAttrValue = OUString::valueOf( nDecimals );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_integer_digits ) );
        sAttrValue = OUString::valueOf( nInteger );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_grouping ) );
        sAttrValue = OUString::createFromAscii(sXML_true);
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  exponent digits
    if ( nExp >= 0 )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_exponent_digits ) );
        sAttrValue = OUString::valueOf( nExp );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
    pAttrList->Clear();
}

void SvXMLNumFmtExport::WriteFractionElement_Impl(
                            sal_Int32 nInteger, sal_Bool bGrouping,
                            sal_Int32 nNumerator, sal_Int32 nDenominator )
{
    FinishTextElement_Impl();

    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_fraction ) );
    OUString sAttrName, sAttrValue;

    //  integer digits
    if ( nInteger >= 0 )        // negative = default (no integer part)
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_integer_digits ) );
        sAttrValue = OUString::valueOf( nInteger );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_grouping ) );
        sAttrValue = OUString::createFromAscii(sXML_true);
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  numerator digits
    if ( nNumerator >= 0 )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_numerator_digits ) );
        sAttrValue = OUString::valueOf( nNumerator );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  denominator digits
    if ( nDenominator >= 0 )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                       OUString::createFromAscii( sXML_min_denominator_digits ) );
        sAttrValue = OUString::valueOf( nDenominator );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    xHandler->endElement( sElem );
    pAttrList->Clear();
}

//  mapping (condition)

void SvXMLNumFmtExport::WriteMapElement_Impl( sal_Int32 nOp, double fLimit,
                                                sal_Int32 nKey, sal_Int32 nPart )
{
    FinishTextElement_Impl();

    if ( nOp != NUMBERFORMAT_OP_NO )
    {
        // style namespace

        OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE,
                            OUString::createFromAscii( sXML_map ) );
        OUString sAttrName, sAttrValue;

        OUStringBuffer aCondStr( 20L );
        aCondStr.appendAscii( "value()" );          //! define constant
        switch ( nOp )
        {
            case NUMBERFORMAT_OP_EQ: aCondStr.append( (sal_Unicode) '=' );  break;
            case NUMBERFORMAT_OP_NE: aCondStr.appendAscii( "<>" );          break;
            case NUMBERFORMAT_OP_LT: aCondStr.append( (sal_Unicode) '<' );  break;
            case NUMBERFORMAT_OP_LE: aCondStr.appendAscii( "<=" );          break;
            case NUMBERFORMAT_OP_GT: aCondStr.append( (sal_Unicode) '>' );  break;
            case NUMBERFORMAT_OP_GE: aCondStr.appendAscii( ">=" );          break;
            default:
                DBG_ERROR("unknown operator");
        }
        String aValStr;
        SolarMath::DoubleToString( aValStr, fLimit, 'A', INT_MAX, '.', TRUE );
        aCondStr.append( aValStr );

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE,
                       OUString::createFromAscii( sXML_condition ) );
        sAttrValue = aCondStr.makeStringAndClear();
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE,
                       OUString::createFromAscii( sXML_apply_style_name ) );
        sAttrValue = lcl_CreateStyleName( nKey, nPart, sal_False );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );

        xHandler->ignorableWhitespace( sWS );
        xHandler->startElement( sElem, xAttrList );
        xHandler->endElement( sElem );
        pAttrList->Clear();
    }
}

//-------------------------------------------------------------------------
//  for old (automatic) currency formats: parse currency symbol from text

xub_StrLen lcl_FindSymbol( const String& sUpperStr, const String& sCurString )
{
    //  search for currency symbol
    //  Quoting as in ImpSvNumberformatScan::Symbol_Division

    xub_StrLen nCPos = 0;
    while (nCPos != STRING_NOTFOUND)
    {
        nCPos = sUpperStr.Search( sCurString, nCPos );
        if (nCPos != STRING_NOTFOUND)
        {
            // in Quotes?
            xub_StrLen nQ = SvNumberformat::GetQuoteEnd( sUpperStr, nCPos );
            if ( nQ == STRING_NOTFOUND )
            {
                //  dm can be escaped as "dm or \d
                sal_Unicode c;
                if ( nCPos == 0 ||
                    ((c = sUpperStr.GetChar(xub_StrLen(nCPos-1))) != '"'
                            && c != '\\') )
                {
                    return nCPos;                   // found
                }
                else
                    nCPos++;                        // continue
            }
            else
                nCPos = nQ + 1;                     // continue after quote end
        }
    }
    return STRING_NOTFOUND;                         // not found
}

sal_Bool SvXMLNumFmtExport::WriteTextWithCurrency_Impl( const OUString& rString,
                            const ::com::sun::star::lang::Locale& rLocale )
{
    //  returns TRUE if currency element was written

    sal_Bool bRet = sal_False;
    pLocaleData->setLocale( rLocale );
    pCharClass->setLocale( rLocale );
    String sCurString = pLocaleData->getCurrSymbol();
    String sUpperStr = pCharClass->upper(rString);
    xub_StrLen nPos = lcl_FindSymbol( sUpperStr, sCurString );
    if ( nPos != STRING_NOTFOUND )
    {
        sal_Int32 nLength = rString.getLength();
        sal_Int32 nCurLen = sCurString.Len();
        sal_Int32 nCont = nPos + nCurLen;

        //  text before currency symbol
        if ( nPos > 0 )
            AddToTextElement_Impl( rString.copy( 0, nPos ) );

        //  currency symbol (empty string -> default)
        OUString sEmpty;
        WriteCurrencyElement_Impl( sEmpty, sEmpty );
        bRet = sal_True;

        //  text after currency symbol
        if ( nCont < nLength )
            AddToTextElement_Impl( rString.copy( nCont, nLength-nCont ) );
    }
    else
        AddToTextElement_Impl( rString );       // simple text

    return bRet;        // TRUE: currency element written
}

//-------------------------------------------------------------------------

//  test if all date elements match the system settings

sal_Bool lcl_MatchesSystemDate( SvNumberformat& rFormat, sal_uInt16 nPart, sal_Bool bLongSysDate )
{
    sal_Bool bMatch = sal_True;
    International aIntl( rFormat.GetLanguage() );

    //  loop through elements (only look for date elements that depend on style attribute)

    sal_uInt16 nPos = 0;
    sal_Bool bEnd = sal_False;
    while ( bMatch && !bEnd )
    {
        short nElemType = rFormat.GetNumForType( nPart, nPos, sal_False );
        switch ( nElemType )
        {
            case 0:
                bEnd = sal_True;                // end of format reached
                break;

            case NF_KEY_D:                      // short day
                if ( SvXMLNumFmtDefaults::IsSystemLongDay( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_DD:                     // long day
                if ( !SvXMLNumFmtDefaults::IsSystemLongDay( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_DDD:                    // short day of week
            case NF_KEY_NN:
            case NF_KEY_AAA:
                if ( SvXMLNumFmtDefaults::IsSystemLongDayOfWeek( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_DDDD:                   // long day of week
            case NF_KEY_NNN:
            case NF_KEY_NNNN:
            case NF_KEY_AAAA:
                if ( !SvXMLNumFmtDefaults::IsSystemLongDayOfWeek( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_M:                      // short numerical month
                if ( SvXMLNumFmtDefaults::IsSystemLongMonth( aIntl, bLongSysDate ) ||
                     SvXMLNumFmtDefaults::IsSystemTextualMonth( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_MM:                     // long numerical month
                if ( !SvXMLNumFmtDefaults::IsSystemLongMonth( aIntl, bLongSysDate ) ||
                     SvXMLNumFmtDefaults::IsSystemTextualMonth( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_MMM:                    // short textual month
            case NF_KEY_MMMMM:                  //! (first letter)
                if ( SvXMLNumFmtDefaults::IsSystemLongMonth( aIntl, bLongSysDate ) ||
                     !SvXMLNumFmtDefaults::IsSystemTextualMonth( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_MMMM:                   // long textual month
                if ( !SvXMLNumFmtDefaults::IsSystemLongMonth( aIntl, bLongSysDate ) ||
                     !SvXMLNumFmtDefaults::IsSystemTextualMonth( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_YY:                     // short year
            case NF_KEY_EEC:
                if ( SvXMLNumFmtDefaults::IsSystemLongYear( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_YYYY:                   // long year
            case NF_KEY_EC:
            case NF_KEY_R:
                if ( !SvXMLNumFmtDefaults::IsSystemLongYear( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_G:                      // short era
            case NF_KEY_GG:
                if ( SvXMLNumFmtDefaults::IsSystemLongEra( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            case NF_KEY_GGG:                    // long era
            case NF_KEY_RR:
                if ( !SvXMLNumFmtDefaults::IsSystemLongEra( aIntl, bLongSysDate ) )
                    bMatch = sal_False;
                break;

            // quarter isn't changed by format-source
        }
        ++nPos;
    }

    return bMatch;
}

//-------------------------------------------------------------------------

OUString lcl_GetDefaultCalendar( SvNumberFormatter* pFormatter, LanguageType nLang )
{
    //  get name of first non-gregorian calendar for the language

    OUString aCalendar;
    CalendarWrapper* pCalendar = pFormatter->GetCalendar();
    if (pCalendar)
    {
        String sLangStr, sCountry;
        ConvertLanguageToIsoNames( nLang, sLangStr, sCountry );
        lang::Locale aLocale( sLangStr, sCountry, OUString() );

        uno::Sequence<OUString> aCals = pCalendar->getAllCalendars( aLocale );
        sal_Int32 nCnt = aCals.getLength();
        sal_Bool bFound = sal_False;
        for ( sal_Int32 j=0; j < nCnt && !bFound; j++ )
        {
            if ( !aCals[j].equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("gregorian") ) )
            {
                aCalendar = aCals[j];
                bFound = sal_True;
            }
        }
    }
    return aCalendar;
}

//-------------------------------------------------------------------------

//
//  export one part (condition)
//

void SvXMLNumFmtExport::ExportPart_Impl( SvNumberformat& rFormat, sal_uInt32 nKey,
                                            sal_uInt16 nPart, sal_Bool bDefPart )
{
    //! for the default part, pass the coditions from the other parts!

    //
    //  element name
    //

    NfIndexTableOffset eBuiltIn = pFormatter->GetIndexTableOffset( nKey );
    short nFmtType = rFormat.GetType() & ~NUMBERFORMAT_DEFINED;
    OUString sType;
    switch ( nFmtType )
    {
        // type is 0 if a format contains no recognized elements
        // (like text only) - this is handled as a number-style.
        case 0:
        case NUMBERFORMAT_NUMBER:
        case NUMBERFORMAT_SCIENTIFIC:
        case NUMBERFORMAT_FRACTION:
            sType = OUString::createFromAscii(sXML_number_style);
            break;
        case NUMBERFORMAT_PERCENT:
            sType = OUString::createFromAscii(sXML_percentage_style);
            break;
        case NUMBERFORMAT_CURRENCY:
            sType = OUString::createFromAscii(sXML_currency_style);
            break;
        case NUMBERFORMAT_DATE:
        case NUMBERFORMAT_DATETIME:
            sType = OUString::createFromAscii(sXML_date_style);
            break;
        case NUMBERFORMAT_TIME:
            sType = OUString::createFromAscii(sXML_time_style);
            break;
        case NUMBERFORMAT_TEXT:
            sType = OUString::createFromAscii(sXML_text_style);
            break;
        case NUMBERFORMAT_LOGICAL:
            sType = OUString::createFromAscii(sXML_boolean_style);
            break;
    }
    if (!sType.getLength())
    {
        DBG_ERROR("unknown format type");
        return;
    }
    OUString sElem = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER, sType );
    OUString sAttrName, sAttrValue;
    sal_Bool bUserDef = ( ( rFormat.GetType() & NUMBERFORMAT_DEFINED ) != 0 );

    //! get FormatInfo from each part separately

    sal_Bool bThousand = sal_False;
    sal_Bool bRed = sal_False;
    sal_uInt16 nPrecision = 0;
    sal_uInt16 nLeading = 0;
    rFormat.GetFormatSpecialInfo( bThousand, bRed, nPrecision, nLeading);

    //
    //  common attributes for format
    //

    //  format name (generated from key) - style namespace
    sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE, OUString::createFromAscii(sXML_name) );
    sAttrValue = lcl_CreateStyleName( nKey, nPart, bDefPart );
    pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_STYLE, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_family)));
    sAttrValue = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_DATA_STYLE_NAME));
    pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );

    //  language / country
    LanguageType nLang = rFormat.GetLanguage();
    AddLanguageAttr_Impl( nLang );                  // adds to pAttrList

    //  title (comment)
    //  titles for builtin formats are not written
    sAttrValue = rFormat.GetComment();
    if ( sAttrValue.getLength() && bUserDef && bDefPart )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER, OUString::createFromAscii(sXML_title) );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  automatic ordering for currency and date formats
    //  only used for some built-in formats
    BOOL bAutoOrder = ( eBuiltIn == NF_CURRENCY_1000INT     || eBuiltIn == NF_CURRENCY_1000DEC2 ||
                        eBuiltIn == NF_CURRENCY_1000INT_RED || eBuiltIn == NF_CURRENCY_1000DEC2_RED ||
                        eBuiltIn == NF_CURRENCY_1000DEC2_DASHED ||
                        eBuiltIn == NF_DATE_SYSTEM_SHORT    || eBuiltIn == NF_DATE_SYSTEM_LONG ||
                        eBuiltIn == NF_DATE_SYS_MMYY        || eBuiltIn == NF_DATE_SYS_DDMMM ||
                        eBuiltIn == NF_DATE_SYS_DDMMYYYY    || eBuiltIn == NF_DATE_SYS_DDMMYY ||
                        eBuiltIn == NF_DATE_SYS_DMMMYY      || eBuiltIn == NF_DATE_SYS_DMMMYYYY ||
                        eBuiltIn == NF_DATE_SYS_DMMMMYYYY   || eBuiltIn == NF_DATE_SYS_NNDMMMYY ||
                        eBuiltIn == NF_DATE_SYS_NNDMMMMYYYY || eBuiltIn == NF_DATE_SYS_NNNNDMMMMYYYY );
    if ( bAutoOrder &&
        ( nFmtType == NUMBERFORMAT_CURRENCY || nFmtType == NUMBERFORMAT_DATE || nFmtType == NUMBERFORMAT_DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_automatic_order ) );
        sAttrValue = OUString::createFromAscii( sXML_true );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  format source (for date and time formats)
    //  only used for some built-in formats
    BOOL bSystemDate = ( eBuiltIn == NF_DATE_SYSTEM_SHORT ||
                         eBuiltIn == NF_DATE_SYSTEM_LONG  ||
                         eBuiltIn == NF_DATETIME_SYSTEM_SHORT_HHMM ) && rFormat.GetComment().Len();
    BOOL bLongSysDate = ( eBuiltIn == NF_DATE_SYSTEM_LONG ) && rFormat.GetComment().Len();
    //  test if all date elements match the system settings
    if ( bSystemDate && !lcl_MatchesSystemDate( rFormat, nPart, bLongSysDate ) )
        bSystemDate = sal_False;
    if ( bSystemDate &&
        ( nFmtType == NUMBERFORMAT_DATE || nFmtType == NUMBERFORMAT_DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_format_source ) );
        sAttrValue = OUString::createFromAscii( sXML_language );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    //  overflow for time formats as in [hh]:mm
    //  controlled by bThousand from number format info
    //  default for truncate-on-overflow is true
    if ( nFmtType == NUMBERFORMAT_TIME && bThousand )
    {
        sAttrName = pNamespaceMap->GetQNameByKey( XML_NAMESPACE_NUMBER,
                        OUString::createFromAscii( sXML_truncate_on_overflow ) );
        sAttrValue = OUString::createFromAscii( sXML_false );
        pAttrList->AddAttribute( sAttrName, sCDATA, sAttrValue );
    }

    xHandler->ignorableWhitespace( sWS );
    xHandler->startElement( sElem, xAttrList );
    pAttrList->Clear();

    //
    //  color (properties element)
    //

    const Color* pCol = rFormat.GetColor( nPart );
    if (pCol)
        WriteColorElement_Impl(*pCol);

    //
    //  format elements
    //

    if ( eBuiltIn == NF_NUMBER_STANDARD )
    {
        //  default number format contains just one number element
        WriteNumberElement_Impl( -1, 1, OUString(), sal_False );
    }
    else if ( eBuiltIn == NF_BOOLEAN )
    {
        //  boolean format contains just one boolean element
        WriteBooleanElement_Impl();
    }
    else
    {
        //  first loop to collect attributes

        sal_Bool bDecDashes  = sal_False;
        sal_Bool bExpFound   = sal_False;
        sal_Bool bCurrFound  = sal_False;
        sal_Int32 nExpDigits = 0;
        OUString sCurrExt;
        OUString aCalendar;
        sal_uInt16 nPos = 0;
        sal_Bool bEnd = sal_False;
        while (!bEnd)
        {
            short nElemType = rFormat.GetNumForType( nPart, nPos, sal_False );
            const XubString* pElemStr = rFormat.GetNumForString( nPart, nPos, sal_False );

            switch ( nElemType )
            {
                case 0:
                    bEnd = sal_True;                // end of format reached
                    break;
                case XMLNUM_SYMBOLTYPE_DIGIT:
                    if ( bExpFound && pElemStr )
                        nExpDigits += pElemStr->Len();
                    else if ( !bDecDashes && pElemStr && pElemStr->GetChar(0) == '-' )
                        bDecDashes = TRUE;
                    break;
                case XMLNUM_SYMBOLTYPE_EXP:
                    bExpFound = TRUE;               // following digits are exponent digits
                    break;
                case XMLNUM_SYMBOLTYPE_CURRENCY:
                    bCurrFound = TRUE;
                    break;
                case XMLNUM_SYMBOLTYPE_CURREXT:
                    if (pElemStr)
                        sCurrExt = *pElemStr;
                    break;

                // E, EE, R, RR: select non-gregorian calendar
                // AAA, AAAA: calendar is switched at the position of the element
                case NF_KEY_EC:
                case NF_KEY_EEC:
                case NF_KEY_R:
                case NF_KEY_RR:
                    if (!aCalendar.getLength())
                        aCalendar = lcl_GetDefaultCalendar( pFormatter, nLang );
                    break;
            }
            ++nPos;
        }

        //  second loop to write elements

        sal_Bool bNumWritten = sal_False;
        sal_Bool bCurrencyWritten = sal_False;
        short nPrevType = 0;
        nPos = 0;
        bEnd = sal_False;
        while (!bEnd)
        {
            short nElemType = rFormat.GetNumForType( nPart, nPos, sal_False );
            const XubString* pElemStr = rFormat.GetNumForString( nPart, nPos, sal_False );

            switch ( nElemType )
            {
                case 0:
                    bEnd = sal_True;                // end of format reached
                    break;
                case XMLNUM_SYMBOLTYPE_STRING:
                    if (pElemStr)
                    {
                        if ( ( nPrevType == NF_KEY_S || nPrevType == NF_KEY_SS ) &&
                             ( pElemStr->EqualsAscii( "," ) || pElemStr->EqualsAscii( "." ) ) &&
                             nPrecision > 0 )
                        {
                            //  decimal separator after seconds is implied by
                            //  "decimal-places" attribute and must not be written
                            //  as text element
                            //! difference between '.' and ',' is lost here
                        }
                        else if ( nFmtType == NUMBERFORMAT_CURRENCY && !bCurrFound && !bCurrencyWritten )
                        {
                            //  automatic currency symbol is implemented as part of
                            //  normal text -> search for the symbol
                            bCurrencyWritten = WriteTextWithCurrency_Impl( *pElemStr,
                                SvNumberFormatter::ConvertLanguageToLocale( nLang ) );
                        }
                        else
                            AddToTextElement_Impl( *pElemStr );
                    }
                    break;
                case NF_KEY_CCC:
                    if (pElemStr)
                    {
                        if ( bCurrencyWritten )
                            AddToTextElement_Impl( *pElemStr );     // never more than one currency element
                        else
                        {
                            //! must be different from short automatic format
                            //! but should still be empty (meaning automatic)
                            //  pElemStr is "CCC"

                            WriteCurrencyElement_Impl( *pElemStr, OUString() );
                            bCurrencyWritten = sal_True;
                        }
                    }
                    break;
                case XMLNUM_SYMBOLTYPE_CURRENCY:
                    if (pElemStr)
                    {
                        if ( bCurrencyWritten )
                            AddToTextElement_Impl( *pElemStr );     // never more than one currency element
                        else
                        {
                            WriteCurrencyElement_Impl( *pElemStr, sCurrExt );
                            bCurrencyWritten = sal_True;
                        }
                    }
                    break;
                case XMLNUM_SYMBOLTYPE_DIGIT:
                    if (!bNumWritten)           // write number part
                    {
                        switch ( nFmtType )
                        {
                            // for type 0 (not recognized as a special type),
                            // write a "normal" number
                            case 0:
                            case NUMBERFORMAT_NUMBER:
                            case NUMBERFORMAT_CURRENCY:
                            case NUMBERFORMAT_PERCENT:
                                {
                                    //  decimals
                                    //  only some built-in formats have automatic decimals
                                    sal_Int32 nDecimals = nPrecision;   // from GetFormatSpecialInfo
                                    if ( eBuiltIn == NF_NUMBER_STANDARD ||
                                         eBuiltIn == NF_CURRENCY_1000DEC2 ||
                                         eBuiltIn == NF_CURRENCY_1000DEC2_RED ||
                                         eBuiltIn == NF_CURRENCY_1000DEC2_CCC ||
                                         eBuiltIn == NF_CURRENCY_1000DEC2_DASHED )
                                        nDecimals = -1;

                                    //  integer digits
                                    //  only one built-in format has automatic integer digits
                                    sal_Int32 nInteger = nLeading;
                                    if ( eBuiltIn == NF_NUMBER_SYSTEM )
                                        nInteger = -1;

                                    //  string for decimal replacement
                                    //  has to be taken from nPrecision
                                    //  (positive number even for automatic decimals)
                                    String sDashStr;
                                    if ( bDecDashes && nPrecision > 0 )
                                        sDashStr.Fill( nPrecision, '-' );

                                    WriteNumberElement_Impl( nDecimals, nInteger, sDashStr, bThousand );
                                }
                                break;
                            case NUMBERFORMAT_SCIENTIFIC:
                                WriteScientificElement_Impl( nPrecision, nLeading, bThousand, nExpDigits );
                                break;
                            case NUMBERFORMAT_FRACTION:
                                WriteFractionElement_Impl( nLeading, bThousand, nPrecision, nPrecision );
                                break;
                        }

                        bNumWritten = sal_True;
                    }
                    break;
                case XMLNUM_SYMBOLTYPE_DEL:
                    if ( pElemStr && *pElemStr == XubString('@') )
                        WriteTextContentElement_Impl();
                    break;

                case XMLNUM_SYMBOLTYPE_CALENDAR:
                    if ( pElemStr )
                        aCalendar = *pElemStr;
                    break;

                // date elements:

                case NF_KEY_D:
                case NF_KEY_DD:
                    {
                        sal_Bool bLong = ( nElemType == NF_KEY_DD );
                        WriteDayElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                    }
                    break;
                case NF_KEY_DDD:
                case NF_KEY_DDDD:
                case NF_KEY_NN:
                case NF_KEY_NNN:
                case NF_KEY_NNNN:
                case NF_KEY_AAA:
                case NF_KEY_AAAA:
                    {
                        OUString aCalAttr = aCalendar;
                        if ( nElemType == NF_KEY_AAA || nElemType == NF_KEY_AAAA )
                        {
                            //  calendar attribute for AAA and AAAA is switched only for this element
                            if (!aCalAttr.getLength())
                                aCalAttr = lcl_GetDefaultCalendar( pFormatter, nLang );
                        }

                        sal_Bool bLong = ( nElemType == NF_KEY_NNN || nElemType == NF_KEY_NNNN ||
                                           nElemType == NF_KEY_DDDD || nElemType == NF_KEY_AAAA );
                        WriteDayOfWeekElement_Impl( aCalAttr, ( bSystemDate ? bLongSysDate : bLong ) );
                        if ( nElemType == NF_KEY_NNNN )
                        {
                            //  write additional text element for separator
                            pLocaleData->setLocale( SvNumberFormatter::ConvertLanguageToLocale( nLang ) );
                            AddToTextElement_Impl( pLocaleData->getLongDateDayOfWeekSep() );
                        }
                    }
                    break;
                case NF_KEY_M:
                case NF_KEY_MM:
                case NF_KEY_MMM:
                case NF_KEY_MMMM:
                case NF_KEY_MMMMM:      //! first letter of month name, no attribute available
                    {
                        sal_Bool bLong = ( nElemType == NF_KEY_MM  || nElemType == NF_KEY_MMMM );
                        sal_Bool bText = ( nElemType == NF_KEY_MMM || nElemType == NF_KEY_MMMM ||
                                            nElemType == NF_KEY_MMMMM );
                        WriteMonthElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ), bText );
                    }
                    break;
                case NF_KEY_YY:
                case NF_KEY_YYYY:
                case NF_KEY_EC:
                case NF_KEY_EEC:
                case NF_KEY_R:      //! R acts as EE, no attribute available
                    {
                        //! distinguish EE and R
                        //  calendar attribute for E and EE and R is set in first loop
                        sal_Bool bLong = ( nElemType == NF_KEY_YYYY || nElemType == NF_KEY_EEC ||
                                            nElemType == NF_KEY_R );
                        WriteYearElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                    }
                    break;
                case NF_KEY_G:
                case NF_KEY_GG:
                case NF_KEY_GGG:
                case NF_KEY_RR:     //! RR acts as GGGEE, no attribute available
                    {
                        //! distinguish GG and GGG and RR
                        sal_Bool bLong = ( nElemType == NF_KEY_GGG || nElemType == NF_KEY_RR );
                        WriteEraElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        if ( nElemType == NF_KEY_RR )
                        {
                            //  calendar attribute for RR is set in first loop
                            WriteYearElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : sal_True ) );
                        }
                    }
                    break;
                case NF_KEY_Q:
                case NF_KEY_QQ:
                    {
                        sal_Bool bLong = ( nElemType == NF_KEY_QQ );
                        WriteQuarterElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                    }
                    break;
                case NF_KEY_WW:
                    WriteWeekElement_Impl( aCalendar );
                    break;

                // time elements (bSystemDate is not used):

                case NF_KEY_H:
                case NF_KEY_HH:
                    WriteHoursElement_Impl( nElemType == NF_KEY_HH );
                    break;
                case NF_KEY_MI:
                case NF_KEY_MMI:
                    WriteMinutesElement_Impl( nElemType == NF_KEY_MMI );
                    break;
                case NF_KEY_S:
                case NF_KEY_SS:
                    WriteSecondsElement_Impl( ( nElemType == NF_KEY_SS ), nPrecision );
                    break;
                case NF_KEY_AMPM:
                case NF_KEY_AP:
                    WriteAMPMElement_Impl();        // short/long?
                    break;
            }
            nPrevType = nElemType;
            ++nPos;
        }
    }

    //
    //  mapping (conditions) must be last elements
    //

    SvNumberformatLimitOps eOp1, eOp2;
    double fLimit1, fLimit2;
    rFormat.GetConditions( eOp1, fLimit1, eOp2, fLimit2 );
    if (bDefPart)
    {
        WriteMapElement_Impl( eOp1, fLimit1, nKey, 0 );
        WriteMapElement_Impl( eOp2, fLimit2, nKey, 1 );
    }

    FinishTextElement_Impl();       // final text element

    xHandler->ignorableWhitespace( sWS );
    xHandler->endElement( sElem );
}

//-------------------------------------------------------------------------

//
//  export one format
//

void SvXMLNumFmtExport::ExportFormat_Impl( SvNumberformat& rFormat, sal_uInt32 nKey )
{
    sal_uInt16 nUsedParts = 0;
    sal_uInt16 nPart;
    for (nPart=0; nPart<XMLNUM_MAX_PARTS; nPart++)
        if (rFormat.GetNumForType( nPart, 0, sal_False ) != 0)
            nUsedParts = nPart+1;

    for (nPart=0; nPart<nUsedParts; nPart++)
    {
        sal_Bool bDefault = ( nPart+1 == nUsedParts );          // last = default
        ExportPart_Impl( rFormat, nKey, nPart, bDefault );
    }
}

//-------------------------------------------------------------------------

//
//  export method called by application
//

void SvXMLNumFmtExport::Export( const SvXMLNamespaceMap& rNamespaceMap, sal_Bool bIsAutoStyle )
{
    if ( !pFormatter )
        return;                         // no formatter -> no entries

    pNamespaceMap = &rNamespaceMap;

    SvUShorts aLanguages;
    pFormatter->GetUsedLanguages( aLanguages );
    sal_uInt16 nLangCount = aLanguages.Count();
    for (sal_uInt16 nLangPos=0; nLangPos<nLangCount; nLangPos++)
    {
        LanguageType nLang = aLanguages[nLangPos];

        sal_uInt32 nStandard;
        SvNumberFormatTable& rTable = pFormatter->GetEntryTable(
                                        NUMBERFORMAT_ALL, nStandard, nLang );
        SvNumberformat* pFormat = rTable.First();
        while (pFormat)
        {
            sal_uInt32 nKey = rTable.GetCurKey();
            if ( (!bIsAutoStyle &&( pFormat->GetType() & NUMBERFORMAT_DEFINED )) ||
                 pUsedList->IsUsed( nKey ) )
            {
                //  user-defined and used formats are exported
                ExportFormat_Impl( *pFormat, nKey );
                // if it is a user-defined Format it will be added else nothing will hapen
                pUsedList->SetUsed(nKey);
            }

            pFormat = rTable.Next();
        }
    }
    pUsedList->Export();

    pNamespaceMap = NULL;
}

OUString SvXMLNumFmtExport::GetStyleName( sal_uInt32 nKey )
{
    DBG_ASSERT(pUsedList->IsUsed(nKey) || pUsedList->IsWasUsed(nKey), "There is no written Data-Style");
    return lcl_CreateStyleName( nKey, 0, sal_True );
}

void SvXMLNumFmtExport::SetUsed( sal_uInt32 nKey )
{
    pUsedList->SetUsed( nKey );
}


