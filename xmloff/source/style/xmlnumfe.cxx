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

#include <comphelper/string.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/numuno.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#include <rtl/math.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustrbuf.hxx>
#include <tools/color.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/i18n/NativeNumberXmlAttributes.hpp>

#include <xmloff/xmlnumfe.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>      // SvXMLNumFmtDefaults

#include <svl/nfsymbol.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>

#include <set>
#include <boost/ptr_container/ptr_vector.hpp>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::svt;

//-------------------------------------------------------------------------

#define XMLNUM_MAX_PARTS    3

//-------------------------------------------------------------------------

struct LessuInt32
{
    sal_Bool operator() (const sal_uInt32 rValue1, const sal_uInt32 rValue2) const
    {
        return rValue1 < rValue2;
    }
};

typedef std::set< sal_uInt32, LessuInt32 >  SvXMLuInt32Set;

class SvXMLNumUsedList_Impl
{
    SvXMLuInt32Set              aUsed;
    SvXMLuInt32Set              aWasUsed;
    SvXMLuInt32Set::iterator    aCurrentUsedPos;
    sal_uInt32                  nUsedCount;
    sal_uInt32                  nWasUsedCount;

public:
            SvXMLNumUsedList_Impl();
            ~SvXMLNumUsedList_Impl();

    void        SetUsed( sal_uInt32 nKey );
    sal_Bool    IsUsed( sal_uInt32 nKey ) const;
    sal_Bool    IsWasUsed( sal_uInt32 nKey ) const;
    void        Export();

    sal_Bool    GetFirstUsed(sal_uInt32& nKey);
    sal_Bool    GetNextUsed(sal_uInt32& nKey);

    void GetWasUsed(uno::Sequence<sal_Int32>& rWasUsed);
    void SetWasUsed(const uno::Sequence<sal_Int32>& rWasUsed);
};

//-------------------------------------------------------------------------

struct SvXMLEmbeddedTextEntry
{
    sal_uInt16      nSourcePos;     // position in NumberFormat (to skip later)
    sal_Int32       nFormatPos;     // resulting position in embedded-text element
    rtl::OUString   aText;

    SvXMLEmbeddedTextEntry( sal_uInt16 nSP, sal_Int32 nFP, const rtl::OUString& rT ) :
        nSourcePos(nSP), nFormatPos(nFP), aText(rT) {}
};

class SvXMLEmbeddedTextEntryArr : public boost::ptr_vector<SvXMLEmbeddedTextEntry> {};

//-------------------------------------------------------------------------

//
//! SvXMLNumUsedList_Impl should be optimized!
//

SvXMLNumUsedList_Impl::SvXMLNumUsedList_Impl() :
    nUsedCount(0),
    nWasUsedCount(0)
{
}

SvXMLNumUsedList_Impl::~SvXMLNumUsedList_Impl()
{
}

void SvXMLNumUsedList_Impl::SetUsed( sal_uInt32 nKey )
{
    if ( !IsWasUsed(nKey) )
    {
        std::pair<SvXMLuInt32Set::iterator, bool> aPair = aUsed.insert( nKey );
        if (aPair.second)
            nUsedCount++;
    }
}

sal_Bool SvXMLNumUsedList_Impl::IsUsed( sal_uInt32 nKey ) const
{
    SvXMLuInt32Set::const_iterator aItr = aUsed.find(nKey);
    return (aItr != aUsed.end());
}

sal_Bool SvXMLNumUsedList_Impl::IsWasUsed( sal_uInt32 nKey ) const
{
    SvXMLuInt32Set::const_iterator aItr = aWasUsed.find(nKey);
    return (aItr != aWasUsed.end());
}

void SvXMLNumUsedList_Impl::Export()
{
    SvXMLuInt32Set::const_iterator aItr = aUsed.begin();
    while (aItr != aUsed.end())
    {
        std::pair<SvXMLuInt32Set::const_iterator, bool> aPair = aWasUsed.insert( *aItr );
        if (aPair.second)
            nWasUsedCount++;
        ++aItr;
    }
    aUsed.clear();
    nUsedCount = 0;
}

sal_Bool SvXMLNumUsedList_Impl::GetFirstUsed(sal_uInt32& nKey)
{
    sal_Bool bRet(sal_False);
    aCurrentUsedPos = aUsed.begin();
    if(nUsedCount)
    {
        DBG_ASSERT(aCurrentUsedPos != aUsed.end(), "something went wrong");
        nKey = *aCurrentUsedPos;
        bRet = sal_True;
    }
    return bRet;
}

sal_Bool SvXMLNumUsedList_Impl::GetNextUsed(sal_uInt32& nKey)
{
    sal_Bool bRet(sal_False);
    if (aCurrentUsedPos != aUsed.end())
    {
        ++aCurrentUsedPos;
        if (aCurrentUsedPos != aUsed.end())
        {
            nKey = *aCurrentUsedPos;
            bRet = sal_True;
        }
    }
    return bRet;
}

void SvXMLNumUsedList_Impl::GetWasUsed(uno::Sequence<sal_Int32>& rWasUsed)
{
    rWasUsed.realloc(nWasUsedCount);
    sal_Int32* pWasUsed = rWasUsed.getArray();
    if (pWasUsed)
    {
        SvXMLuInt32Set::const_iterator aItr = aWasUsed.begin();
        while (aItr != aWasUsed.end())
        {
            *pWasUsed = *aItr;
            ++aItr;
            ++pWasUsed;
        }
    }
}

void SvXMLNumUsedList_Impl::SetWasUsed(const uno::Sequence<sal_Int32>& rWasUsed)
{
    DBG_ASSERT(nWasUsedCount == 0, "WasUsed should be empty");
    sal_Int32 nCount(rWasUsed.getLength());
    const sal_Int32* pWasUsed = rWasUsed.getConstArray();
    for (sal_uInt16 i = 0; i < nCount; i++, pWasUsed++)
    {
        std::pair<SvXMLuInt32Set::const_iterator, bool> aPair = aWasUsed.insert( *pWasUsed );
        if (aPair.second)
            nWasUsedCount++;
    }
}

//-------------------------------------------------------------------------

SvXMLNumFmtExport::SvXMLNumFmtExport(
            SvXMLExport& rExp,
            const uno::Reference< util::XNumberFormatsSupplier >& rSupp ) :
    rExport( rExp ),
    sPrefix( OUString("N") ),
    pFormatter( NULL ),
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
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( MsLangId::getSystemLanguage() ) );

        pCharClass = new CharClass( rExport.getServiceFactory(), aLocale );
        pLocaleData = new LocaleDataWrapper( rExport.getServiceFactory(), aLocale );
    }

    pUsedList = new SvXMLNumUsedList_Impl;
}

SvXMLNumFmtExport::SvXMLNumFmtExport(
                       SvXMLExport& rExp,
                       const ::com::sun::star::uno::Reference<
                        ::com::sun::star::util::XNumberFormatsSupplier >& rSupp,
                       const rtl::OUString& rPrefix ) :
    rExport( rExp ),
    sPrefix( rPrefix ),
    pFormatter( NULL ),
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
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( MsLangId::getSystemLanguage() ) );

        pCharClass = new CharClass( rExport.getServiceFactory(), aLocale );
        pLocaleData = new LocaleDataWrapper( rExport.getServiceFactory(), aLocale );
    }

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

OUString lcl_CreateStyleName( sal_Int32 nKey, sal_Int32 nPart, sal_Bool bDefPart, const rtl::OUString& rPrefix )
{
    OUStringBuffer aFmtName( 10L );
    aFmtName.append( rPrefix );
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
    if ( !rCalendar.isEmpty() )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_CALENDAR, rCalendar );
    }
}

void SvXMLNumFmtExport::AddTextualAttr_Impl( sal_Bool bText )
{
    if ( bText )            // non-textual
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TEXTUAL, XML_TRUE );
    }
}

void SvXMLNumFmtExport::AddStyleAttr_Impl( sal_Bool bLong )
{
    if ( bLong )            // short is default
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_STYLE, XML_LONG );
    }
}

void SvXMLNumFmtExport::AddLanguageAttr_Impl( sal_Int32 nLang )
{
    if ( nLang != LANGUAGE_SYSTEM )
    {
        OUString aLangStr, aCountryStr;
        MsLangId::convertLanguageToIsoNames( (LanguageType)nLang, aLangStr, aCountryStr );

        if (!aLangStr.isEmpty())
            rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_LANGUAGE, aLangStr );
        if (!aCountryStr.isEmpty())
            rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_COUNTRY, aCountryStr );
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
    if ( sTextContent.getLength() )
    {
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_TEXT,
                                  sal_True, sal_False );
        rExport.Characters( sTextContent.makeStringAndClear() );
    }
}

void SvXMLNumFmtExport::WriteColorElement_Impl( const Color& rColor )
{
    FinishTextElement_Impl();

    OUStringBuffer aColStr( 7 );
    ::sax::Converter::convertColor( aColStr, rColor.GetColor() );
    rExport.AddAttribute( XML_NAMESPACE_FO, XML_COLOR,
                          aColStr.makeStringAndClear() );

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_TEXT_PROPERTIES,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteCurrencyElement_Impl( const OUString& rString,
                                                    const OUString& rExt )
{
    FinishTextElement_Impl();

    if ( !rExt.isEmpty() )
    {
        sal_Int32 nLang = rExt.toInt32(16);     // hex
        if ( nLang < 0 )                        // extension string may contain "-" separator
            nLang = -nLang;
        AddLanguageAttr_Impl( nLang );          // adds to pAttrList
    }

    SvXMLElementExport aElem( rExport,
                              XML_NAMESPACE_NUMBER, XML_CURRENCY_SYMBOL,
                              sal_True, sal_False );
    rExport.Characters( rString );
}

void SvXMLNumFmtExport::WriteBooleanElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_BOOLEAN,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteTextContentElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_TEXT_CONTENT,
                              sal_True, sal_False );
}

//  date elements

void SvXMLNumFmtExport::WriteDayElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_DAY,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteMonthElement_Impl( const OUString& rCalendar, sal_Bool bLong, sal_Bool bText )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    AddTextualAttr_Impl( bText );   // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_MONTH,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteYearElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_YEAR,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteEraElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_ERA,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteDayOfWeekElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_DAY_OF_WEEK,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteWeekElement_Impl( const OUString& rCalendar )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_WEEK_OF_YEAR,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteQuarterElement_Impl( const OUString& rCalendar, sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_QUARTER,
                              sal_True, sal_False );
}

//  time elements

void SvXMLNumFmtExport::WriteHoursElement_Impl( sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_HOURS,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteMinutesElement_Impl( sal_Bool bLong )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_MINUTES,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteRepeatedElement_Impl( sal_Unicode nChar )
{
    FinishTextElement_Impl();
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_FILL_CHARACTER,
                                  sal_True, sal_False );
    rExport.Characters( OUString::valueOf( nChar ) );
}

void SvXMLNumFmtExport::WriteSecondsElement_Impl( sal_Bool bLong, sal_uInt16 nDecimals )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    if ( nDecimals > 0 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::valueOf( (sal_Int32) nDecimals ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_SECONDS,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteAMPMElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_AM_PM,
                              sal_True, sal_False );
}

//  numbers

void SvXMLNumFmtExport::WriteNumberElement_Impl(
                            sal_Int32 nDecimals, sal_Int32 nInteger,
                            const OUString& rDashStr, sal_Bool bVarDecimals,
                            sal_Bool bGrouping, sal_Int32 nTrailingThousands,
                            const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries )
{
    FinishTextElement_Impl();

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::valueOf( nDecimals ) );
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::valueOf( nInteger ) );
    }

    //  decimal replacement (dashes) or variable decimals (#)
    if ( !rDashStr.isEmpty() || bVarDecimals )
    {
        //  variable decimals means an empty replacement string
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_REPLACEMENT,
                              rDashStr );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_GROUPING, XML_TRUE );
    }

    //  display-factor if there are trailing thousands separators
    if ( nTrailingThousands )
    {
        //  each separator character removes three digits
        double fFactor = ::rtl::math::pow10Exp( 1.0, 3 * nTrailingThousands );

        OUStringBuffer aFactStr;
        ::sax::Converter::convertDouble( aFactStr, fFactor );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DISPLAY_FACTOR, aFactStr.makeStringAndClear() );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_NUMBER,
                              sal_True, sal_True );

    //  number:embedded-text as child elements

    sal_uInt16 nEntryCount = rEmbeddedEntries.size();
    for (sal_uInt16 nEntry=0; nEntry<nEntryCount; nEntry++)
    {
        const SvXMLEmbeddedTextEntry* pObj = &rEmbeddedEntries[nEntry];

        //  position attribute
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_POSITION,
                                OUString::valueOf( pObj->nFormatPos ) );
        SvXMLElementExport aChildElem( rExport, XML_NAMESPACE_NUMBER, XML_EMBEDDED_TEXT,
                                          sal_True, sal_False );

        //  text as element content
        rtl::OUString aContent( pObj->aText );
        while ( nEntry+1 < nEntryCount && rEmbeddedEntries[nEntry+1].nFormatPos == pObj->nFormatPos )
        {
            // The array can contain several elements for the same position in the number
            // (for example, literal text and space from underscores). They must be merged
            // into a single embedded-text element.
            aContent += rEmbeddedEntries[nEntry+1].aText;
            ++nEntry;
        }
        rExport.Characters( aContent );
    }
}

void SvXMLNumFmtExport::WriteScientificElement_Impl(
                            sal_Int32 nDecimals, sal_Int32 nInteger,
                            sal_Bool bGrouping, sal_Int32 nExp )
{
    FinishTextElement_Impl();

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::valueOf( nDecimals ) );
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::valueOf( nInteger ) );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_GROUPING, XML_TRUE );
    }

    //  exponent digits
    if ( nExp >= 0 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_EXPONENT_DIGITS,
                              OUString::valueOf( nExp ) );
    }

    SvXMLElementExport aElem( rExport,
                              XML_NAMESPACE_NUMBER, XML_SCIENTIFIC_NUMBER,
                              sal_True, sal_False );
}

void SvXMLNumFmtExport::WriteFractionElement_Impl(
                            sal_Int32 nInteger, sal_Bool bGrouping,
                            sal_Int32 nNumerator, sal_Int32 nDenominator )
{
    FinishTextElement_Impl();

    //  integer digits
    if ( nInteger >= 0 )        // negative = default (no integer part)
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::valueOf( nInteger ) );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_GROUPING, XML_TRUE );
    }

    //  numerator digits
    if ( nNumerator >= 0 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_NUMERATOR_DIGITS,
                                 OUString::valueOf( nNumerator ) );
    }

    //  denominator digits
    if ( nDenominator >= 0 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_DENOMINATOR_DIGITS,
                              OUString::valueOf( nDenominator ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_FRACTION,
                              sal_True, sal_False );
}

//  mapping (condition)

void SvXMLNumFmtExport::WriteMapElement_Impl( sal_Int32 nOp, double fLimit,
                                                sal_Int32 nKey, sal_Int32 nPart )
{
    FinishTextElement_Impl();

    if ( nOp != NUMBERFORMAT_OP_NO )
    {
        // style namespace

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
                OSL_FAIL("unknown operator");
        }
        ::rtl::math::doubleToUStringBuffer( aCondStr, fLimit,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                '.', true );

        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_CONDITION,
                              aCondStr.makeStringAndClear() );

        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME,
                              rExport.EncodeStyleName( lcl_CreateStyleName( nKey, nPart, sal_False,
                                                   sPrefix ) ) );

        SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_MAP,
                                  sal_True, sal_False );
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
    //  returns sal_True if currency element was written

    sal_Bool bRet = sal_False;

    LanguageType nLang = MsLangId::convertLocaleToLanguage( rLocale );
    pFormatter->ChangeIntl( nLang );
    String sCurString, sDummy;
    pFormatter->GetCompatibilityCurrency( sCurString, sDummy );

    pCharClass->setLocale( rLocale );
    String sUpperStr = pCharClass->uppercase(rString);
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

    return bRet;        // sal_True: currency element written
}

//-------------------------------------------------------------------------

OUString lcl_GetDefaultCalendar( SvNumberFormatter* pFormatter, LanguageType nLang )
{
    //  get name of first non-gregorian calendar for the language

    OUString aCalendar;
    CalendarWrapper* pCalendar = pFormatter->GetCalendar();
    if (pCalendar)
    {
        lang::Locale aLocale( MsLangId::convertLanguageToLocale( nLang ) );

        uno::Sequence<OUString> aCals = pCalendar->getAllCalendars( aLocale );
        sal_Int32 nCnt = aCals.getLength();
        sal_Bool bFound = sal_False;
        for ( sal_Int32 j=0; j < nCnt && !bFound; j++ )
        {
            if ( aCals[j] != "gregorian" )
            {
                aCalendar = aCals[j];
                bFound = sal_True;
            }
        }
    }
    return aCalendar;
}

//-------------------------------------------------------------------------

sal_Bool lcl_IsInEmbedded( const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries, sal_uInt16 nPos )
{
    sal_uInt16 nCount = rEmbeddedEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( rEmbeddedEntries[i].nSourcePos == nPos )
            return sal_True;

    return sal_False;       // not found
}

sal_Bool lcl_IsDefaultDateFormat( const SvNumberformat& rFormat, sal_Bool bSystemDate, NfIndexTableOffset eBuiltIn )
{
    //  make an extra loop to collect date elements, to check if it is a default format
    //  before adding the automatic-order attribute

    SvXMLDateElementAttributes eDateDOW = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateDay = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateMonth = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateYear = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateHours = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateMins = XML_DEA_NONE;
    SvXMLDateElementAttributes eDateSecs = XML_DEA_NONE;
    sal_Bool bDateNoDefault = sal_False;

    sal_uInt16 nPos = 0;
    sal_Bool bEnd = sal_False;
    short nLastType = 0;
    while (!bEnd)
    {
        short nElemType = rFormat.GetNumForType( 0, nPos, sal_False );
        switch ( nElemType )
        {
            case 0:
                if ( nLastType == NF_SYMBOLTYPE_STRING )
                    bDateNoDefault = sal_True;  // text at the end -> no default date format
                bEnd = sal_True;                // end of format reached
                break;
            case NF_SYMBOLTYPE_STRING:
            case NF_SYMBOLTYPE_DATESEP:
            case NF_SYMBOLTYPE_TIMESEP:
            case NF_SYMBOLTYPE_TIME100SECSEP:
                // text is ignored, except at the end
                break;
            // same mapping as in SvXMLNumFormatContext::AddNfKeyword:
            case NF_KEY_NN:     eDateDOW = XML_DEA_SHORT;       break;
            case NF_KEY_NNN:
            case NF_KEY_NNNN:   eDateDOW = XML_DEA_LONG;        break;
            case NF_KEY_D:      eDateDay = XML_DEA_SHORT;       break;
            case NF_KEY_DD:     eDateDay = XML_DEA_LONG;        break;
            case NF_KEY_M:      eDateMonth = XML_DEA_SHORT;     break;
            case NF_KEY_MM:     eDateMonth = XML_DEA_LONG;      break;
            case NF_KEY_MMM:    eDateMonth = XML_DEA_TEXTSHORT; break;
            case NF_KEY_MMMM:   eDateMonth = XML_DEA_TEXTLONG;  break;
            case NF_KEY_YY:     eDateYear = XML_DEA_SHORT;      break;
            case NF_KEY_YYYY:   eDateYear = XML_DEA_LONG;       break;
            case NF_KEY_H:      eDateHours = XML_DEA_SHORT;     break;
            case NF_KEY_HH:     eDateHours = XML_DEA_LONG;      break;
            case NF_KEY_MI:     eDateMins = XML_DEA_SHORT;      break;
            case NF_KEY_MMI:    eDateMins = XML_DEA_LONG;       break;
            case NF_KEY_S:      eDateSecs = XML_DEA_SHORT;      break;
            case NF_KEY_SS:     eDateSecs = XML_DEA_LONG;       break;
            case NF_KEY_AP:
            case NF_KEY_AMPM:   break;          // AM/PM may or may not be in date/time formats -> ignore by itself
            default:
                bDateNoDefault = sal_True;      // any other element -> no default format
        }
        nLastType = nElemType;
        ++nPos;
    }

    if ( bDateNoDefault )
        return sal_False;                       // additional elements
    else
    {
        NfIndexTableOffset eFound = (NfIndexTableOffset) SvXMLNumFmtDefaults::GetDefaultDateFormat(
                eDateDOW, eDateDay, eDateMonth, eDateYear, eDateHours, eDateMins, eDateSecs, bSystemDate );

        return ( eFound == eBuiltIn );
    }
}

//
//  export one part (condition)
//

void SvXMLNumFmtExport::ExportPart_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey,
                                            sal_uInt16 nPart, sal_Bool bDefPart )
{
    //! for the default part, pass the coditions from the other parts!

    //
    //  element name
    //

    NfIndexTableOffset eBuiltIn = pFormatter->GetIndexTableOffset( nKey );

    short nFmtType = 0;
    bool bThousand = false;
    sal_uInt16 nPrecision = 0;
    sal_uInt16 nLeading = 0;
    rFormat.GetNumForInfo( nPart, nFmtType, bThousand, nPrecision, nLeading);
    nFmtType &= ~NUMBERFORMAT_DEFINED;

    //  special treatment of builtin formats that aren't detected by normal parsing
    //  (the same formats that get the type set in SvNumberFormatter::ImpGenerateFormats)
    if ( eBuiltIn == NF_NUMBER_STANDARD )
        nFmtType = NUMBERFORMAT_NUMBER;
    else if ( eBuiltIn == NF_BOOLEAN )
        nFmtType = NUMBERFORMAT_LOGICAL;
    else if ( eBuiltIn == NF_TEXT )
        nFmtType = NUMBERFORMAT_TEXT;

    // #101606# An empty subformat is a valid number-style resulting in an
    // empty display string for the condition of the subformat.
    if ( nFmtType == NUMBERFORMAT_UNDEFINED && rFormat.GetNumForType( nPart,
                0, sal_False ) == 0 )
        nFmtType = 0;

    XMLTokenEnum eType = XML_TOKEN_INVALID;
    switch ( nFmtType )
    {
        // type is 0 if a format contains no recognized elements
        // (like text only) - this is handled as a number-style.
        case 0:
        case NUMBERFORMAT_NUMBER:
        case NUMBERFORMAT_SCIENTIFIC:
        case NUMBERFORMAT_FRACTION:
            eType = XML_NUMBER_STYLE;
            break;
        case NUMBERFORMAT_PERCENT:
            eType = XML_PERCENTAGE_STYLE;
            break;
        case NUMBERFORMAT_CURRENCY:
            eType = XML_CURRENCY_STYLE;
            break;
        case NUMBERFORMAT_DATE:
        case NUMBERFORMAT_DATETIME:
            eType = XML_DATE_STYLE;
            break;
        case NUMBERFORMAT_TIME:
            eType = XML_TIME_STYLE;
            break;
        case NUMBERFORMAT_TEXT:
            eType = XML_TEXT_STYLE;
            break;
        case NUMBERFORMAT_LOGICAL:
            eType = XML_BOOLEAN_STYLE;
            break;
    }
    DBG_ASSERT( eType != XML_TOKEN_INVALID, "unknown format type" );

    OUString sAttrValue;
    sal_Bool bUserDef = ( ( rFormat.GetType() & NUMBERFORMAT_DEFINED ) != 0 );

    //
    //  common attributes for format
    //

    //  format name (generated from key) - style namespace
    rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                        lcl_CreateStyleName( nKey, nPart, bDefPart, sPrefix ) );

    //  "volatile" attribute for styles used only in maps
    if ( !bDefPart )
        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_VOLATILE, XML_TRUE );

    //  language / country
    LanguageType nLang = rFormat.GetLanguage();
    AddLanguageAttr_Impl( nLang );                  // adds to pAttrList

    //  title (comment)
    //  titles for builtin formats are not written
    sAttrValue = rFormat.GetComment();
    if ( !sAttrValue.isEmpty() && bUserDef && bDefPart )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TITLE, sAttrValue );
    }

    //  automatic ordering for currency and date formats
    //  only used for some built-in formats
    sal_Bool bAutoOrder = ( eBuiltIn == NF_CURRENCY_1000INT     || eBuiltIn == NF_CURRENCY_1000DEC2 ||
                        eBuiltIn == NF_CURRENCY_1000INT_RED || eBuiltIn == NF_CURRENCY_1000DEC2_RED ||
                        eBuiltIn == NF_CURRENCY_1000DEC2_DASHED ||
                        eBuiltIn == NF_DATE_SYSTEM_SHORT    || eBuiltIn == NF_DATE_SYSTEM_LONG ||
                        eBuiltIn == NF_DATE_SYS_MMYY        || eBuiltIn == NF_DATE_SYS_DDMMM ||
                        eBuiltIn == NF_DATE_SYS_DDMMYYYY    || eBuiltIn == NF_DATE_SYS_DDMMYY ||
                        eBuiltIn == NF_DATE_SYS_DMMMYY      || eBuiltIn == NF_DATE_SYS_DMMMYYYY ||
                        eBuiltIn == NF_DATE_SYS_DMMMMYYYY   || eBuiltIn == NF_DATE_SYS_NNDMMMYY ||
                        eBuiltIn == NF_DATE_SYS_NNDMMMMYYYY || eBuiltIn == NF_DATE_SYS_NNNNDMMMMYYYY ||
                        eBuiltIn == NF_DATETIME_SYSTEM_SHORT_HHMM || eBuiltIn == NF_DATETIME_SYS_DDMMYYYY_HHMMSS );

    //  format source (for date and time formats)
    //  only used for some built-in formats
    sal_Bool bSystemDate = ( eBuiltIn == NF_DATE_SYSTEM_SHORT ||
                         eBuiltIn == NF_DATE_SYSTEM_LONG  ||
                         eBuiltIn == NF_DATETIME_SYSTEM_SHORT_HHMM );
    sal_Bool bLongSysDate = ( eBuiltIn == NF_DATE_SYSTEM_LONG );

    // check if the format definition matches the key
    if ( bAutoOrder && ( nFmtType == NUMBERFORMAT_DATE || nFmtType == NUMBERFORMAT_DATETIME ) &&
            !lcl_IsDefaultDateFormat( rFormat, bSystemDate, eBuiltIn ) )
    {
        bAutoOrder = bSystemDate = bLongSysDate = sal_False;        // don't write automatic-order attribute then
    }

    if ( bAutoOrder &&
        ( nFmtType == NUMBERFORMAT_CURRENCY || nFmtType == NUMBERFORMAT_DATE || nFmtType == NUMBERFORMAT_DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER,
                              XML_TRUE );
    }

    if ( bSystemDate && bAutoOrder &&
        ( nFmtType == NUMBERFORMAT_DATE || nFmtType == NUMBERFORMAT_DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_FORMAT_SOURCE,
                              XML_LANGUAGE );
    }

    //  overflow for time formats as in [hh]:mm
    //  controlled by bThousand from number format info
    //  default for truncate-on-overflow is true
    if ( nFmtType == NUMBERFORMAT_TIME && bThousand )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRUNCATE_ON_OVERFLOW,
                              XML_FALSE );
    }

    //
    // Native number transliteration
    //
    ::com::sun::star::i18n::NativeNumberXmlAttributes aAttr;
    rFormat.GetNatNumXml( aAttr, nPart );
    if ( !aAttr.Format.isEmpty() )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_FORMAT,
                              aAttr.Format );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_LANGUAGE,
                              aAttr.Locale.Language );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_COUNTRY,
                              aAttr.Locale.Country );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_STYLE,
                              aAttr.Style );
    }

    //
    // The element
    //
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, eType,
                              sal_True, sal_True );

    //
    //  color (properties element)
    //

    const Color* pCol = rFormat.GetColor( nPart );
    if (pCol)
        WriteColorElement_Impl(*pCol);


    //  detect if there is "real" content, excluding color and maps
    //! move to implementation of Write... methods?
    sal_Bool bAnyContent = sal_False;

    //
    //  format elements
    //

    SvXMLEmbeddedTextEntryArr aEmbeddedEntries;
    if ( eBuiltIn == NF_NUMBER_STANDARD )
    {
        //  default number format contains just one number element
        WriteNumberElement_Impl( -1, 1, OUString(), sal_False, sal_False, 0, aEmbeddedEntries );
        bAnyContent = sal_True;
    }
    else if ( eBuiltIn == NF_BOOLEAN )
    {
        //  boolean format contains just one boolean element
        WriteBooleanElement_Impl();
        bAnyContent = sal_True;
    }
    else
    {
        //  first loop to collect attributes

        sal_Bool bDecDashes  = sal_False;
        sal_Bool bVarDecimals = sal_False;
        sal_Bool bExpFound   = sal_False;
        sal_Bool bCurrFound  = sal_False;
        sal_Bool bInInteger  = sal_True;
        sal_Int32 nExpDigits = 0;
        sal_Int32 nIntegerSymbols = 0;          // for embedded-text, including "#"
        sal_Int32 nTrailingThousands = 0;       // thousands-separators after all digits
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
                case NF_SYMBOLTYPE_DIGIT:
                    if ( bExpFound && pElemStr )
                        nExpDigits += pElemStr->Len();
                    else if ( !bDecDashes && pElemStr && pElemStr->GetChar(0) == '-' )
                        bDecDashes = sal_True;
                    else if ( !bVarDecimals && !bInInteger && pElemStr && pElemStr->GetChar(0) == '#' )
                    {
                        //  If the decimal digits string starts with a '#', variable
                        //  decimals is assumed (for 0.###, but not 0.0##).
                        bVarDecimals = sal_True;
                    }
                    if ( bInInteger && pElemStr )
                        nIntegerSymbols += pElemStr->Len();
                    nTrailingThousands = 0;
                    break;
                case NF_SYMBOLTYPE_DECSEP:
                    bInInteger = sal_False;
                    break;
                case NF_SYMBOLTYPE_THSEP:
                    if (pElemStr)
                        nTrailingThousands += pElemStr->Len();      // is reset to 0 if digits follow
                    break;
                case NF_SYMBOLTYPE_EXP:
                    bExpFound = sal_True;           // following digits are exponent digits
                    bInInteger = sal_False;
                    break;
                case NF_SYMBOLTYPE_CURRENCY:
                    bCurrFound = sal_True;
                    break;
                case NF_SYMBOLTYPE_CURREXT:
                    if (pElemStr)
                        sCurrExt = *pElemStr;
                    break;

                // E, EE, R, RR: select non-gregorian calendar
                // AAA, AAAA: calendar is switched at the position of the element
                case NF_KEY_EC:
                case NF_KEY_EEC:
                case NF_KEY_R:
                case NF_KEY_RR:
                    if (aCalendar.isEmpty())
                        aCalendar = lcl_GetDefaultCalendar( pFormatter, nLang );
                    break;
            }
            ++nPos;
        }

        //  collect strings for embedded-text (must be known before number element is written)

        sal_Bool bAllowEmbedded = ( nFmtType == 0 || nFmtType == NUMBERFORMAT_NUMBER ||
                                        nFmtType == NUMBERFORMAT_CURRENCY ||
                                        nFmtType == NUMBERFORMAT_PERCENT );
        if ( bAllowEmbedded )
        {
            sal_Int32 nDigitsPassed = 0;
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
                    case NF_SYMBOLTYPE_DIGIT:
                        if ( pElemStr )
                            nDigitsPassed += pElemStr->Len();
                        break;
                    case NF_SYMBOLTYPE_STRING:
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_PERCENT:
                        if ( nDigitsPassed > 0 && nDigitsPassed < nIntegerSymbols && pElemStr )
                        {
                            //  text (literal or underscore) within the integer part of a number:number element

                            String aEmbeddedStr;
                            if ( nElemType == NF_SYMBOLTYPE_STRING || nElemType == NF_SYMBOLTYPE_PERCENT )
                                aEmbeddedStr = *pElemStr;
                            else
                                SvNumberformat::InsertBlanks( aEmbeddedStr, 0, pElemStr->GetChar(1) );

                            sal_Int32 nEmbedPos = nIntegerSymbols - nDigitsPassed;

                            SvXMLEmbeddedTextEntry* pObj = new SvXMLEmbeddedTextEntry( nPos, nEmbedPos, aEmbeddedStr );
                            aEmbeddedEntries.push_back( pObj );
                        }
                        break;
                }
                ++nPos;
            }
        }

        //  final loop to write elements

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
                case NF_SYMBOLTYPE_STRING:
                case NF_SYMBOLTYPE_DATESEP:
                case NF_SYMBOLTYPE_TIMESEP:
                case NF_SYMBOLTYPE_TIME100SECSEP:
                case NF_SYMBOLTYPE_PERCENT:
                    if (pElemStr)
                    {
                        if ( ( nPrevType == NF_KEY_S || nPrevType == NF_KEY_SS ) &&
                             ( nElemType == NF_SYMBOLTYPE_TIME100SECSEP ) &&
                             nPrecision > 0 )
                        {
                            //  decimal separator after seconds is implied by
                            //  "decimal-places" attribute and must not be written
                            //  as text element
                            //! difference between '.' and ',' is lost here
                        }
                        else if ( lcl_IsInEmbedded( aEmbeddedEntries, nPos ) )
                        {
                            //  text is written as embedded-text child of the number,
                            //  don't create a text element
                        }
                        else if ( nFmtType == NUMBERFORMAT_CURRENCY && !bCurrFound && !bCurrencyWritten )
                        {
                            //  automatic currency symbol is implemented as part of
                            //  normal text -> search for the symbol
                            bCurrencyWritten = WriteTextWithCurrency_Impl( *pElemStr,
                                MsLangId::convertLanguageToLocale( nLang ) );
                            bAnyContent = sal_True;
                        }
                        else
                            AddToTextElement_Impl( *pElemStr );
                    }
                    break;
                case NF_SYMBOLTYPE_BLANK:
                    if ( pElemStr && !lcl_IsInEmbedded( aEmbeddedEntries, nPos ) )
                    {
                        //  turn "_x" into the number of spaces used for x in InsertBlanks in the NumberFormat
                        //  (#i20396# the spaces may also be in embedded-text elements)

                        String aBlanks;
                        SvNumberformat::InsertBlanks( aBlanks, 0, pElemStr->GetChar(1) );
                        AddToTextElement_Impl( aBlanks );
                    }
                    break;
                case NF_KEY_GENERAL :
                        WriteNumberElement_Impl( -1, 1, OUString(), sal_False, sal_False, 0, aEmbeddedEntries );
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
                            bAnyContent = sal_True;
                            bCurrencyWritten = sal_True;
                        }
                    }
                    break;
                case NF_SYMBOLTYPE_CURRENCY:
                    if (pElemStr)
                    {
                        if ( bCurrencyWritten )
                            AddToTextElement_Impl( *pElemStr );     // never more than one currency element
                        else
                        {
                            WriteCurrencyElement_Impl( *pElemStr, sCurrExt );
                            bAnyContent = sal_True;
                            bCurrencyWritten = sal_True;
                        }
                    }
                    break;
                case NF_SYMBOLTYPE_DIGIT:
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

                                    WriteNumberElement_Impl( nDecimals, nInteger, sDashStr, bVarDecimals,
                                                        bThousand, nTrailingThousands, aEmbeddedEntries );
                                    bAnyContent = sal_True;
                                }
                                break;
                            case NUMBERFORMAT_SCIENTIFIC:
                                // #i43959# for scientific numbers, count all integer symbols ("0" and "#")
                                // as integer digits: use nIntegerSymbols instead of nLeading
                                // (use of '#' to select multiples in exponent might be added later)
                                WriteScientificElement_Impl( nPrecision, nIntegerSymbols, bThousand, nExpDigits );
                                bAnyContent = sal_True;
                                break;
                            case NUMBERFORMAT_FRACTION:
                                {
                                    sal_Int32 nInteger = nLeading;
                                    if ( pElemStr && pElemStr->GetChar(0) == '?' )
                                    {
                                        //  If the first digit character is a question mark,
                                        //  the fraction doesn't have an integer part, and no
                                        //  min-integer-digits attribute must be written.
                                        nInteger = -1;
                                    }
                                    WriteFractionElement_Impl( nInteger, bThousand, nPrecision, nPrecision );
                                    bAnyContent = sal_True;
                                }
                                break;
                        }

                        bNumWritten = sal_True;
                    }
                    break;
                case NF_SYMBOLTYPE_DECSEP:
                    if ( pElemStr && nPrecision == 0 )
                    {
                        //  A decimal separator after the number, without following decimal digits,
                        //  isn't modelled as part of the number element, so it's written as text
                        //  (the distinction between a quoted and non-quoted, locale-dependent
                        //  character is lost here).

                        AddToTextElement_Impl( *pElemStr );
                    }
                    break;
                case NF_SYMBOLTYPE_DEL:
                    if ( pElemStr && comphelper::string::equals(*pElemStr, '@') )
                    {
                        WriteTextContentElement_Impl();
                        bAnyContent = sal_True;
                    }
                    break;

                case NF_SYMBOLTYPE_CALENDAR:
                    if ( pElemStr )
                        aCalendar = *pElemStr;
                    break;

                // date elements:

                case NF_KEY_D:
                case NF_KEY_DD:
                    {
                        sal_Bool bLong = ( nElemType == NF_KEY_DD );
                        WriteDayElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = sal_True;
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
                            if (aCalAttr.isEmpty())
                                aCalAttr = lcl_GetDefaultCalendar( pFormatter, nLang );
                        }

                        sal_Bool bLong = ( nElemType == NF_KEY_NNN || nElemType == NF_KEY_NNNN ||
                                           nElemType == NF_KEY_DDDD || nElemType == NF_KEY_AAAA );
                        WriteDayOfWeekElement_Impl( aCalAttr, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = sal_True;
                        if ( nElemType == NF_KEY_NNNN )
                        {
                            //  write additional text element for separator
                            pLocaleData->setLocale( MsLangId::convertLanguageToLocale( nLang ) );
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
                        bAnyContent = sal_True;
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
                        bAnyContent = sal_True;
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
                        bAnyContent = sal_True;
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
                        bAnyContent = sal_True;
                    }
                    break;
                case NF_KEY_WW:
                    WriteWeekElement_Impl( aCalendar );
                    bAnyContent = sal_True;
                    break;

                // time elements (bSystemDate is not used):

                case NF_KEY_H:
                case NF_KEY_HH:
                    WriteHoursElement_Impl( nElemType == NF_KEY_HH );
                    bAnyContent = sal_True;
                    break;
                case NF_KEY_MI:
                case NF_KEY_MMI:
                    WriteMinutesElement_Impl( nElemType == NF_KEY_MMI );
                    bAnyContent = sal_True;
                    break;
                case NF_KEY_S:
                case NF_KEY_SS:
                    WriteSecondsElement_Impl( ( nElemType == NF_KEY_SS ), nPrecision );
                    bAnyContent = sal_True;
                    break;
                case NF_KEY_AMPM:
                case NF_KEY_AP:
                    WriteAMPMElement_Impl();        // short/long?
                    bAnyContent = sal_True;
                    break;
                case NF_SYMBOLTYPE_STAR :
                    // export only if ODF 1.2 extensions are enabled
                    if( SvtSaveOptions().GetODFDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                    {
                        if ( pElemStr && pElemStr->Len() > 1 )
                            WriteRepeatedElement_Impl( pElemStr->GetChar( 1 ) );
                    }
                    break;
            }
            nPrevType = nElemType;
            ++nPos;
        }
    }

    if ( sTextContent.getLength() )
        bAnyContent = sal_True;     // element written in FinishTextElement_Impl

    FinishTextElement_Impl();       // final text element - before maps

    if ( !bAnyContent )
    {
        //  for an empty format, write an empty text element
        SvXMLElementExport aTElem( rExport, XML_NAMESPACE_NUMBER, XML_TEXT,
                                   sal_True, sal_False );
    }

    //
    //  mapping (conditions) must be last elements
    //

    if (bDefPart)
    {
        SvNumberformatLimitOps eOp1, eOp2;
        double fLimit1, fLimit2;
        rFormat.GetConditions( eOp1, fLimit1, eOp2, fLimit2 );

        WriteMapElement_Impl( eOp1, fLimit1, nKey, 0 );
        WriteMapElement_Impl( eOp2, fLimit2, nKey, 1 );

        if ( rFormat.HasTextFormat() )
        {
            //  4th part is for text -> make an "all other numbers" condition for the 3rd part
            //  by reversing the 2nd condition

            SvNumberformatLimitOps eOp3 = NUMBERFORMAT_OP_NO;
            double fLimit3 = fLimit2;
            switch ( eOp2 )
            {
                case NUMBERFORMAT_OP_EQ: eOp3 = NUMBERFORMAT_OP_NE; break;
                case NUMBERFORMAT_OP_NE: eOp3 = NUMBERFORMAT_OP_EQ; break;
                case NUMBERFORMAT_OP_LT: eOp3 = NUMBERFORMAT_OP_GE; break;
                case NUMBERFORMAT_OP_LE: eOp3 = NUMBERFORMAT_OP_GT; break;
                case NUMBERFORMAT_OP_GT: eOp3 = NUMBERFORMAT_OP_LE; break;
                case NUMBERFORMAT_OP_GE: eOp3 = NUMBERFORMAT_OP_LT; break;
                default:
                    break;
            }

            if ( fLimit1 == fLimit2 &&
                    ( ( eOp1 == NUMBERFORMAT_OP_LT && eOp2 == NUMBERFORMAT_OP_GT ) ||
                      ( eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_LT ) ) )
            {
                //  For <x and >x, add =x as last condition
                //  (just for readability, <=x would be valid, too)

                eOp3 = NUMBERFORMAT_OP_EQ;
            }

            WriteMapElement_Impl( eOp3, fLimit3, nKey, 2 );
        }
    }
}

//-------------------------------------------------------------------------

//
//  export one format
//

void SvXMLNumFmtExport::ExportFormat_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey )
{
    sal_uInt16 nUsedParts = 0;
    sal_uInt16 nPart;
    for (nPart=0; nPart<XMLNUM_MAX_PARTS; nPart++)
        if (rFormat.GetNumForType( nPart, 0, sal_False ) != 0)
            nUsedParts = nPart+1;

    SvNumberformatLimitOps eOp1, eOp2;
    double fLimit1, fLimit2;
    rFormat.GetConditions( eOp1, fLimit1, eOp2, fLimit2 );

    //  if conditions are set, even empty formats must be written

    if ( eOp1 != NUMBERFORMAT_OP_NO && nUsedParts < 2 )
        nUsedParts = 2;
    if ( eOp2 != NUMBERFORMAT_OP_NO && nUsedParts < 3 )
        nUsedParts = 3;
    if ( rFormat.HasTextFormat() && nUsedParts < 4 )
        nUsedParts = 4;

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

void SvXMLNumFmtExport::Export( sal_Bool bIsAutoStyle )
{
    if ( !pFormatter )
        return;                         // no formatter -> no entries

    sal_uInt32 nKey;
    const SvNumberformat* pFormat = NULL;
    sal_Bool bNext(pUsedList->GetFirstUsed(nKey));
    while(bNext)
    {
        pFormat = pFormatter->GetEntry(nKey);
        if(pFormat)
            ExportFormat_Impl( *pFormat, nKey );
        bNext = pUsedList->GetNextUsed(nKey);
    }
    if (!bIsAutoStyle)
    {
        std::vector<sal_uInt16> aLanguages;
        pFormatter->GetUsedLanguages( aLanguages );
        for (std::vector<sal_uInt16>::const_iterator it(aLanguages.begin()); it != aLanguages.end(); ++it)
        {
            LanguageType nLang = *it;

            sal_uInt32 nDefaultIndex = 0;
            SvNumberFormatTable& rTable = pFormatter->GetEntryTable(
                                            NUMBERFORMAT_DEFINED, nDefaultIndex, nLang );
            SvNumberFormatTable::iterator it2 = rTable.begin();
            while (it2 != rTable.end())
            {
                nKey = it2->first;
                pFormat = it2->second;
                if (!pUsedList->IsUsed(nKey))
                {
                    DBG_ASSERT((pFormat->GetType() & NUMBERFORMAT_DEFINED) != 0, "a not user defined numberformat found");
                    //  user-defined and used formats are exported
                    ExportFormat_Impl( *pFormat, nKey );
                    // if it is a user-defined Format it will be added else nothing will hapen
                    pUsedList->SetUsed(nKey);
                }

                ++it2;
            }
        }
    }
    pUsedList->Export();
}

OUString SvXMLNumFmtExport::GetStyleName( sal_uInt32 nKey )
{
    if(pUsedList->IsUsed(nKey) || pUsedList->IsWasUsed(nKey))
        return lcl_CreateStyleName( nKey, 0, sal_True, sPrefix );
    else
    {
        OSL_FAIL("There is no written Data-Style");
        return rtl::OUString();
    }
}

void SvXMLNumFmtExport::SetUsed( sal_uInt32 nKey )
{
    DBG_ASSERT( pFormatter != NULL, "missing formatter" );
    if( !pFormatter )
        return;

    if (pFormatter->GetEntry(nKey))
        pUsedList->SetUsed( nKey );
    else {
        OSL_FAIL("no existing Numberformat found with this key");
    }
}

void SvXMLNumFmtExport::GetWasUsed(uno::Sequence<sal_Int32>& rWasUsed)
{
    if (pUsedList)
        pUsedList->GetWasUsed(rWasUsed);
}

void SvXMLNumFmtExport::SetWasUsed(const uno::Sequence<sal_Int32>& rWasUsed)
{
    if (pUsedList)
        pUsedList->SetWasUsed(rWasUsed);
}



const SvNumberformat* lcl_GetFormat( SvNumberFormatter* pFormatter,
                           sal_uInt32 nKey )
{
    return ( pFormatter != NULL ) ? pFormatter->GetEntry( nKey ) : NULL;
}

sal_uInt32 SvXMLNumFmtExport::ForceSystemLanguage( sal_uInt32 nKey )
{
    sal_uInt32 nRet = nKey;

    const SvNumberformat* pFormat = lcl_GetFormat( pFormatter, nKey );
    if( pFormat != NULL )
    {
        DBG_ASSERT( pFormatter != NULL, "format without formatter?" );

        xub_StrLen nErrorPos;
        short nType = pFormat->GetType();

        sal_uInt32 nNewKey = pFormatter->GetFormatForLanguageIfBuiltIn(
                       nKey, LANGUAGE_SYSTEM );

        if( nNewKey != nKey )
        {
            nRet = nNewKey;
        }
        else
        {
            String aFormatString( pFormat->GetFormatstring() );
            pFormatter->PutandConvertEntry(
                            aFormatString,
                            nErrorPos, nType, nNewKey,
                            pFormat->GetLanguage(), LANGUAGE_SYSTEM );

            // success? Then use new key.
            if( nErrorPos == 0 )
                nRet = nNewKey;
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
