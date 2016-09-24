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

#include <comphelper/string.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/numuno.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>

#include <svl/nfsymbol.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>

#include <set>
#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::svt;

struct LessuInt32
{
    bool operator() (const sal_uInt32 rValue1, const sal_uInt32 rValue2) const
    {
        return rValue1 < rValue2;
    }
};

typedef std::set< sal_uInt32, LessuInt32 >  SvXMLuInt32Set;

struct SvXMLEmbeddedTextEntry
{
    sal_uInt16      nSourcePos;     // position in NumberFormat (to skip later)
    sal_Int32       nFormatPos;     // resulting position in embedded-text element
    OUString   aText;

    SvXMLEmbeddedTextEntry( sal_uInt16 nSP, sal_Int32 nFP, const OUString& rT ) :
        nSourcePos(nSP), nFormatPos(nFP), aText(rT) {}
};

class SvXMLEmbeddedTextEntryArr
{
    typedef std::vector<SvXMLEmbeddedTextEntry> DataType;
    DataType maData;

public:

    void push_back( SvXMLEmbeddedTextEntry const& r )
    {
        maData.push_back(r);
    }

    const SvXMLEmbeddedTextEntry& operator[] ( size_t i ) const
    {
        return maData[i];
    }

    size_t size() const
    {
        return maData.size();
    }
};

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

    void    SetUsed( sal_uInt32 nKey );
    bool    IsUsed( sal_uInt32 nKey ) const;
    bool    IsWasUsed( sal_uInt32 nKey ) const;
    void    Export();

    bool    GetFirstUsed(sal_uInt32& nKey);
    bool    GetNextUsed(sal_uInt32& nKey);

    void GetWasUsed(uno::Sequence<sal_Int32>& rWasUsed);
    void SetWasUsed(const uno::Sequence<sal_Int32>& rWasUsed);
};

//! SvXMLNumUsedList_Impl should be optimized!

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

bool SvXMLNumUsedList_Impl::IsUsed( sal_uInt32 nKey ) const
{
    SvXMLuInt32Set::const_iterator aItr = aUsed.find(nKey);
    return (aItr != aUsed.end());
}

bool SvXMLNumUsedList_Impl::IsWasUsed( sal_uInt32 nKey ) const
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

bool SvXMLNumUsedList_Impl::GetFirstUsed(sal_uInt32& nKey)
{
    bool bRet(false);
    aCurrentUsedPos = aUsed.begin();
    if(nUsedCount)
    {
        DBG_ASSERT(aCurrentUsedPos != aUsed.end(), "something went wrong");
        nKey = *aCurrentUsedPos;
        bRet = true;
    }
    return bRet;
}

bool SvXMLNumUsedList_Impl::GetNextUsed(sal_uInt32& nKey)
{
    bool bRet(false);
    if (aCurrentUsedPos != aUsed.end())
    {
        ++aCurrentUsedPos;
        if (aCurrentUsedPos != aUsed.end())
        {
            nKey = *aCurrentUsedPos;
            bRet = true;
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
    for (sal_Int32 i = 0; i < nCount; i++, pWasUsed++)
    {
        std::pair<SvXMLuInt32Set::const_iterator, bool> aPair = aWasUsed.insert( *pWasUsed );
        if (aPair.second)
            nWasUsedCount++;
    }
}

SvXMLNumFmtExport::SvXMLNumFmtExport(
            SvXMLExport& rExp,
            const uno::Reference< util::XNumberFormatsSupplier >& rSupp ) :
    rExport( rExp ),
    sPrefix( OUString("N") ),
    pFormatter( nullptr ),
    pCharClass( nullptr ),
    pLocaleData( nullptr )
{
    //  supplier must be SvNumberFormatsSupplierObj
    SvNumberFormatsSupplierObj* pObj =
                    SvNumberFormatsSupplierObj::getImplementation( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    if ( pFormatter )
    {
        pCharClass = new CharClass( pFormatter->GetComponentContext(),
            pFormatter->GetLanguageTag() );
        pLocaleData = new LocaleDataWrapper( pFormatter->GetComponentContext(),
            pFormatter->GetLanguageTag() );
    }
    else
    {
        LanguageTag aLanguageTag( MsLangId::getSystemLanguage() );

        pCharClass = new CharClass( rExport.getComponentContext(), aLanguageTag );
        pLocaleData = new LocaleDataWrapper( rExport.getComponentContext(), aLanguageTag );
    }

    pUsedList.reset(new SvXMLNumUsedList_Impl);
}

SvXMLNumFmtExport::SvXMLNumFmtExport(
                       SvXMLExport& rExp,
                       const css::uno::Reference< css::util::XNumberFormatsSupplier >& rSupp,
                       const OUString& rPrefix ) :
    rExport( rExp ),
    sPrefix( rPrefix ),
    pFormatter( nullptr ),
    pCharClass( nullptr ),
    pLocaleData( nullptr )
{
    //  supplier must be SvNumberFormatsSupplierObj
    SvNumberFormatsSupplierObj* pObj =
                    SvNumberFormatsSupplierObj::getImplementation( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    if ( pFormatter )
    {
        pCharClass = new CharClass( pFormatter->GetComponentContext(),
            pFormatter->GetLanguageTag() );
        pLocaleData = new LocaleDataWrapper( pFormatter->GetComponentContext(),
            pFormatter->GetLanguageTag() );
    }
    else
    {
        LanguageTag aLanguageTag( MsLangId::getSystemLanguage() );

        pCharClass = new CharClass( rExport.getComponentContext(), aLanguageTag );
        pLocaleData = new LocaleDataWrapper( rExport.getComponentContext(), aLanguageTag );
    }

    pUsedList.reset(new SvXMLNumUsedList_Impl);
}

SvXMLNumFmtExport::~SvXMLNumFmtExport()
{
    delete pLocaleData;
    delete pCharClass;
}

//  helper methods

static OUString lcl_CreateStyleName( sal_Int32 nKey, sal_Int32 nPart, bool bDefPart, const OUString& rPrefix )
{
    OUStringBuffer aFmtName(10);
    aFmtName.append( rPrefix );
    aFmtName.append( nKey );
    if (!bDefPart)
    {
        aFmtName.append( 'P' );
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

void SvXMLNumFmtExport::AddTextualAttr_Impl( bool bText )
{
    if ( bText )            // non-textual
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TEXTUAL, XML_TRUE );
    }
}

void SvXMLNumFmtExport::AddStyleAttr_Impl( bool bLong )
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
        rExport.AddLanguageTagAttributes( XML_NAMESPACE_NUMBER, XML_NAMESPACE_NUMBER,
                LanguageTag( (LanguageType)nLang), false);
    }
}

//  methods to write individual elements within a format

void SvXMLNumFmtExport::AddToTextElement_Impl( const OUString& rString )
{
    //  append to sTextContent, write element in FinishTextElement_Impl
    //  to avoid several text elements following each other

    sTextContent.append( rString );
}

void SvXMLNumFmtExport::FinishTextElement_Impl(bool bUseExtensionNS)
{
    if ( !sTextContent.isEmpty() )
    {
        sal_uInt16 nNS = bUseExtensionNS ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER;
        SvXMLElementExport aElem( rExport, nNS, XML_TEXT,
                                  true, false );
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
                              true, false );
}

void SvXMLNumFmtExport::WriteCurrencyElement_Impl( const OUString& rString,
                                                    const OUString& rExt )
{
    FinishTextElement_Impl();

    if ( !rExt.isEmpty() )
    {
        // rExt should be a 16-bit hex value max FFFF which may contain a
        // leading "-" separator (that is not a minus sign, but toInt32 can be
        // used to parse it, with post-processing as necessary):
        sal_Int32 nLang = rExt.toInt32(16);
        if ( nLang < 0 )
            nLang = -nLang;
        AddLanguageAttr_Impl( nLang );          // adds to pAttrList
    }

    SvXMLElementExport aElem( rExport,
                              XML_NAMESPACE_NUMBER, XML_CURRENCY_SYMBOL,
                              true, false );
    rExport.Characters( rString );
}

void SvXMLNumFmtExport::WriteBooleanElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_BOOLEAN,
                              true, false );
}

void SvXMLNumFmtExport::WriteTextContentElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_TEXT_CONTENT,
                              true, false );
}

//  date elements

void SvXMLNumFmtExport::WriteDayElement_Impl( const OUString& rCalendar, bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_DAY,
                              true, false );
}

void SvXMLNumFmtExport::WriteMonthElement_Impl( const OUString& rCalendar, bool bLong, bool bText )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    AddTextualAttr_Impl( bText );   // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_MONTH,
                              true, false );
}

void SvXMLNumFmtExport::WriteYearElement_Impl( const OUString& rCalendar, bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_YEAR,
                              true, false );
}

void SvXMLNumFmtExport::WriteEraElement_Impl( const OUString& rCalendar, bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_ERA,
                              true, false );
}

void SvXMLNumFmtExport::WriteDayOfWeekElement_Impl( const OUString& rCalendar, bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_DAY_OF_WEEK,
                              true, false );
}

void SvXMLNumFmtExport::WriteWeekElement_Impl( const OUString& rCalendar )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_WEEK_OF_YEAR,
                              true, false );
}

void SvXMLNumFmtExport::WriteQuarterElement_Impl( const OUString& rCalendar, bool bLong )
{
    FinishTextElement_Impl();

    AddCalendarAttr_Impl( rCalendar ); // adds to pAttrList
    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_QUARTER,
                              true, false );
}

//  time elements

void SvXMLNumFmtExport::WriteHoursElement_Impl( bool bLong )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_HOURS,
                              true, false );
}

void SvXMLNumFmtExport::WriteMinutesElement_Impl( bool bLong )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_MINUTES,
                              true, false );
}

void SvXMLNumFmtExport::WriteRepeatedElement_Impl( sal_Unicode nChar )
{
    // Export only for 1.2 with extensions or 1.3 and later.
    SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
    if (eVersion > SvtSaveOptions::ODFSVER_012)
    {
        FinishTextElement_Impl(true);
        // For 1.2+ use loext namespace, for 1.3 use number namespace.
        SvXMLElementExport aElem( rExport,
                                  ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                                  XML_FILL_CHARACTER, true, false );
        rExport.Characters( OUString( nChar ) );
    }
}

void SvXMLNumFmtExport::WriteSecondsElement_Impl( bool bLong, sal_uInt16 nDecimals )
{
    FinishTextElement_Impl();

    AddStyleAttr_Impl( bLong );     // adds to pAttrList
    if ( nDecimals > 0 )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::number(  nDecimals ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_SECONDS,
                              true, false );
}

void SvXMLNumFmtExport::WriteAMPMElement_Impl()
{
    FinishTextElement_Impl();

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_AM_PM,
                              true, false );
}

//  numbers

void SvXMLNumFmtExport::WriteNumberElement_Impl(
                            sal_Int32 nDecimals, sal_Int32 nMinDecimals,
                            sal_Int32 nInteger, const OUString& rDashStr,
                            bool bGrouping, sal_Int32 nTrailingThousands,
                            const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries )
{
    FinishTextElement_Impl();

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::number( nDecimals ) );
    }

    if ( nMinDecimals >= 0 )   // negative = automatic
    {
        // Export only for 1.2 with extensions or 1.3 and later.
        SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
        if (eVersion > SvtSaveOptions::ODFSVER_012)
        {
            // For 1.2+ use loext namespace, for 1.3 use number namespace.
            rExport.AddAttribute(
                ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                                 XML_MIN_DECIMAL_PLACES,
                                 OUString::number( nMinDecimals ) );
        }
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::number( nInteger ) );
    }

    //  decimal replacement (dashes) or variable decimals (#)
    if ( !rDashStr.isEmpty() ||  nMinDecimals < nDecimals )
    {
        // full variable decimals means an empty replacement string
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
                              true, true );

    //  number:embedded-text as child elements

    sal_uInt16 nEntryCount = rEmbeddedEntries.size();
    for (sal_uInt16 nEntry=0; nEntry<nEntryCount; nEntry++)
    {
        const SvXMLEmbeddedTextEntry *const pObj = &rEmbeddedEntries[nEntry];

        //  position attribute
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_POSITION,
                                OUString::number( pObj->nFormatPos ) );
        SvXMLElementExport aChildElem( rExport, XML_NAMESPACE_NUMBER, XML_EMBEDDED_TEXT,
                                          true, false );

        //  text as element content
        OUString aContent( pObj->aText );
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
                            sal_Int32 nDecimals, sal_Int32 nMinDecimals, sal_Int32 nInteger,
                            bool bGrouping, sal_Int32 nExp, sal_Int32 nExpInterval, bool bExpSign )
{
    FinishTextElement_Impl();

    //  decimals
    if ( nDecimals >= 0 )   // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DECIMAL_PLACES,
                              OUString::number( nDecimals ) );
    }

    SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
    if ( nMinDecimals >= 0 )   // negative = automatic
    {
        // Export only for 1.2 with extensions or 1.3 and later.
        if (eVersion > SvtSaveOptions::ODFSVER_012)
        {
            // For 1.2+ use loext namespace, for 1.3 use number namespace.
            rExport.AddAttribute(
                ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                                 XML_MIN_DECIMAL_PLACES,
                                 OUString::number( nMinDecimals ) );
        }
    }

    //  integer digits
    if ( nInteger >= 0 )    // negative = automatic
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::number( nInteger ) );
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
                              OUString::number( nExp ) );
    }

    //  exponent interval for engineering notation
    if ( nExpInterval >= 0 )
    {
        // Export only for 1.2 with extensions or 1.3 and later.
        if (eVersion > SvtSaveOptions::ODFSVER_012)
        {
            // For 1.2+ use loext namespace, for 1.3 use number namespace.
            rExport.AddAttribute(
                    ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                    XML_EXPONENT_INTERVAL, OUString::number( nExpInterval ) );
        }
    }

    //  exponent sign
    // Export only for 1.2 with extensions or 1.3 and later.
    if (eVersion > SvtSaveOptions::ODFSVER_012)
    {
        // For 1.2+ use loext namespace, for 1.3 use number namespace.
        rExport.AddAttribute(
            ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                             XML_FORCED_EXPONENT_SIGN,
                             bExpSign? XML_TRUE : XML_FALSE );
    }

    SvXMLElementExport aElem( rExport,
                              XML_NAMESPACE_NUMBER, XML_SCIENTIFIC_NUMBER,
                              true, false );
}

void SvXMLNumFmtExport::WriteFractionElement_Impl(
                            sal_Int32 nInteger, bool bGrouping,
                            const SvNumberformat& rFormat, sal_uInt16 nPart )
{
    FinishTextElement_Impl();
    const OUString aNumeratorString = rFormat.GetNumeratorString( nPart );
    const OUString aDenominatorString = rFormat.GetDenominatorString( nPart );
    const OUString aIntegerFractionDelimiterString = rFormat.GetIntegerFractionDelimiterString( nPart );
    sal_Int32 nMaxNumeratorDigits = aNumeratorString.getLength();
    // Count '0' as '?'
    sal_Int32 nMinNumeratorDigits = aNumeratorString.replaceAll("0","?").indexOf('?');
    sal_Int32 nZerosNumeratorDigits = aNumeratorString.indexOf('0');
    if ( nMinNumeratorDigits >= 0 )
        nMinNumeratorDigits = nMaxNumeratorDigits - nMinNumeratorDigits;
    else
        nMinNumeratorDigits = 0;
    if ( nZerosNumeratorDigits >= 0 )
        nZerosNumeratorDigits = nMaxNumeratorDigits - nZerosNumeratorDigits;
    else
        nZerosNumeratorDigits = 0;
    sal_Int32 nMaxDenominatorDigits = aDenominatorString.getLength();
    sal_Int32 nMinDenominatorDigits = aDenominatorString.replaceAll("0","?").indexOf('?');
    sal_Int32 nZerosDenominatorDigits = aDenominatorString.indexOf('0');
    if ( nMinDenominatorDigits >= 0 )
        nMinDenominatorDigits = nMaxDenominatorDigits - nMinDenominatorDigits;
    else
        nMinDenominatorDigits = 0;
    if ( nZerosDenominatorDigits >= 0 )
        nZerosDenominatorDigits = nMaxDenominatorDigits - nZerosDenominatorDigits;
    else
        nZerosDenominatorDigits = 0;
    sal_Int32 nDenominator = aDenominatorString.toInt32();

    //  integer digits
    if ( nInteger >= 0 )        // negative = default (no integer part)
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_INTEGER_DIGITS,
                              OUString::number( nInteger ) );
    }

    //  (automatic) grouping separator
    if ( bGrouping )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_GROUPING, XML_TRUE );
    }

    // integer/fraction delimiter
    SvtSaveOptions::ODFSaneDefaultVersion eVersion = rExport.getSaneDefaultVersion();
    if ( !aIntegerFractionDelimiterString.isEmpty() && aIntegerFractionDelimiterString != " "
        && ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0) )
    {   // Export only for 1.2 with extensions or 1.3 and later.
        rExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_INTEGER_FRACTION_DELIMITER,
                              aIntegerFractionDelimiterString );
    }

    //  numerator digits
    if ( nMinNumeratorDigits == 0 ) // at least one digit to keep compatibility with previous versions
        nMinNumeratorDigits++;
    rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_NUMERATOR_DIGITS,
                          OUString::number( nMinNumeratorDigits ) );
    // Export only for 1.2 with extensions or 1.3 and later.
    if ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0)
    {
        // For extended ODF use loext namespace
        rExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_MAX_NUMERATOR_DIGITS,
                              OUString::number( nMaxNumeratorDigits ) );
    }
    if ( nZerosNumeratorDigits && ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0) )
        rExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_ZEROS_NUMERATOR_DIGITS,
                              OUString::number( nZerosNumeratorDigits ) );

    if ( nDenominator )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_DENOMINATOR_VALUE,
                              OUString::number( nDenominator) );
    }
    //  it's not necessary to export nDenominatorDigits
    //  if we have a forced denominator
    else
    {
        if ( nMinDenominatorDigits == 0 ) // at least one digit to keep compatibility with previous versions
            nMinDenominatorDigits++;
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_MIN_DENOMINATOR_DIGITS,
                              OUString::number( nMinDenominatorDigits ) );
        if (eVersion > SvtSaveOptions::ODFSVER_012)
        {
            // For 1.2+ use loext namespace, for 1.3 use number namespace.
            rExport.AddAttribute(
                ((eVersion < SvtSaveOptions::ODFSVER_013) ? XML_NAMESPACE_LO_EXT : XML_NAMESPACE_NUMBER),
                                 XML_MAX_DENOMINATOR_VALUE,
                                 OUString::number( pow ( 10.0, nMaxDenominatorDigits ) - 1 ) ); // 9, 99 or 999
        }
        if ( nZerosDenominatorDigits && ((eVersion & SvtSaveOptions::ODFSVER_EXTENDED) != 0) )
            rExport.AddAttribute( XML_NAMESPACE_LO_EXT, XML_ZEROS_DENOMINATOR_DIGITS,
                                  OUString::number( nZerosDenominatorDigits ) );
    }

    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, XML_FRACTION,
                              true, false );
}

//  mapping (condition)

void SvXMLNumFmtExport::WriteMapElement_Impl( sal_Int32 nOp, double fLimit,
                                                sal_Int32 nKey, sal_Int32 nPart )
{
    FinishTextElement_Impl();

    if ( nOp != NUMBERFORMAT_OP_NO )
    {
        // style namespace

        OUStringBuffer aCondStr(20);
        aCondStr.append( "value()" );          //! define constant
        switch ( nOp )
        {
            case NUMBERFORMAT_OP_EQ: aCondStr.append( '=' );  break;
            case NUMBERFORMAT_OP_NE: aCondStr.append( "!=" );          break;
            case NUMBERFORMAT_OP_LT: aCondStr.append( '<' );  break;
            case NUMBERFORMAT_OP_LE: aCondStr.append( "<=" );          break;
            case NUMBERFORMAT_OP_GT: aCondStr.append( '>' );  break;
            case NUMBERFORMAT_OP_GE: aCondStr.append( ">=" );          break;
            default:
                OSL_FAIL("unknown operator");
        }
        ::rtl::math::doubleToUStringBuffer( aCondStr, fLimit,
                rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                '.', true );

        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_CONDITION,
                              aCondStr.makeStringAndClear() );

        rExport.AddAttribute( XML_NAMESPACE_STYLE, XML_APPLY_STYLE_NAME,
                              rExport.EncodeStyleName( lcl_CreateStyleName( nKey, nPart, false,
                                                   sPrefix ) ) );

        SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_MAP,
                                  true, false );
    }
}

//  for old (automatic) currency formats: parse currency symbol from text

sal_Int32 lcl_FindSymbol( const OUString& sUpperStr, const OUString& sCurString )
{
    //  search for currency symbol
    //  Quoting as in ImpSvNumberformatScan::Symbol_Division

    sal_Int32 nCPos = 0;
    while (nCPos >= 0)
    {
        nCPos = sUpperStr.indexOf( sCurString, nCPos );
        if (nCPos >= 0)
        {
            // in Quotes?
            sal_Int32 nQ = SvNumberformat::GetQuoteEnd( sUpperStr, nCPos );
            if ( nQ < 0 )
            {
                //  dm can be escaped as "dm or \d
                sal_Unicode c;
                if ( nCPos == 0 ||
                    ((c = sUpperStr[nCPos-1]) != '"'
                     && c != '\\') )
                {
                    return nCPos;                   // found
                }
                else
                {
                    nCPos++;                        // continue
                }
            }
            else
            {
                nCPos = nQ + 1;                     // continue after quote end
            }
        }
    }
    return -1;
}

bool SvXMLNumFmtExport::WriteTextWithCurrency_Impl( const OUString& rString,
                            const css::lang::Locale& rLocale )
{
    //  returns true if currency element was written

    bool bRet = false;

    LanguageTag aLanguageTag( rLocale );
    pFormatter->ChangeIntl( aLanguageTag.getLanguageType( false) );
    OUString sCurString, sDummy;
    pFormatter->GetCompatibilityCurrency( sCurString, sDummy );

    pCharClass->setLanguageTag( aLanguageTag );
    OUString sUpperStr = pCharClass->uppercase(rString);
    sal_Int32 nPos = lcl_FindSymbol( sUpperStr, sCurString );
    if ( nPos >= 0 )
    {
        sal_Int32 nLength = rString.getLength();
        sal_Int32 nCurLen = sCurString.getLength();
        sal_Int32 nCont = nPos + nCurLen;

        //  text before currency symbol
        if ( nPos > 0 )
        {
            AddToTextElement_Impl( rString.copy( 0, nPos ) );
        }
        //  currency symbol (empty string -> default)
        WriteCurrencyElement_Impl( "", "" );
        bRet = true;

        //  text after currency symbol
        if ( nCont < nLength )
        {
            AddToTextElement_Impl( rString.copy( nCont, nLength-nCont ) );
        }
    }
    else
    {
        AddToTextElement_Impl( rString );       // simple text
    }

    return bRet;        // true: currency element written
}

static OUString lcl_GetDefaultCalendar( SvNumberFormatter* pFormatter, LanguageType nLang )
{
    //  get name of first non-gregorian calendar for the language

    OUString aCalendar;
    CalendarWrapper* pCalendar = pFormatter->GetCalendar();
    if (pCalendar)
    {
        lang::Locale aLocale( LanguageTag::convertToLocale( nLang ) );

        uno::Sequence<OUString> aCals = pCalendar->getAllCalendars( aLocale );
        sal_Int32 nCnt = aCals.getLength();
        bool bFound = false;
        for ( sal_Int32 j=0; j < nCnt && !bFound; j++ )
        {
            if ( aCals[j] != "gregorian" )
            {
                aCalendar = aCals[j];
                bFound = true;
            }
        }
    }
    return aCalendar;
}

static bool lcl_IsInEmbedded( const SvXMLEmbeddedTextEntryArr& rEmbeddedEntries, sal_uInt16 nPos )
{
    sal_uInt16 nCount = rEmbeddedEntries.size();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( rEmbeddedEntries[i].nSourcePos == nPos )
            return true;

    return false;       // not found
}

static bool lcl_IsDefaultDateFormat( const SvNumberformat& rFormat, bool bSystemDate, NfIndexTableOffset eBuiltIn )
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
    bool bDateNoDefault = false;

    sal_uInt16 nPos = 0;
    bool bEnd = false;
    short nLastType = 0;
    while (!bEnd)
    {
        short nElemType = rFormat.GetNumForType( 0, nPos );
        switch ( nElemType )
        {
            case 0:
                if ( nLastType == NF_SYMBOLTYPE_STRING )
                    bDateNoDefault = true;  // text at the end -> no default date format
                bEnd = true;                // end of format reached
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
                bDateNoDefault = true;      // any other element -> no default format
        }
        nLastType = nElemType;
        ++nPos;
    }

    if ( bDateNoDefault )
        return false;                       // additional elements
    else
    {
        NfIndexTableOffset eFound = (NfIndexTableOffset) SvXMLNumFmtDefaults::GetDefaultDateFormat(
                eDateDOW, eDateDay, eDateMonth, eDateYear, eDateHours, eDateMins, eDateSecs, bSystemDate );

        return ( eFound == eBuiltIn );
    }
}

//  export one part (condition)

void SvXMLNumFmtExport::ExportPart_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey,
                                            sal_uInt16 nPart, bool bDefPart )
{
    //! for the default part, pass the conditions from the other parts!

    //  element name

    NfIndexTableOffset eBuiltIn = pFormatter->GetIndexTableOffset( nKey );

    short nFmtType = 0;
    bool bThousand = false;
    sal_uInt16 nPrecision = 0;
    sal_uInt16 nLeading = 0;
    rFormat.GetNumForInfo( nPart, nFmtType, bThousand, nPrecision, nLeading);
    nFmtType &= ~css::util::NumberFormat::DEFINED;

    //  special treatment of builtin formats that aren't detected by normal parsing
    //  (the same formats that get the type set in SvNumberFormatter::ImpGenerateFormats)
    if ( eBuiltIn == NF_NUMBER_STANDARD )
        nFmtType = css::util::NumberFormat::NUMBER;
    else if ( eBuiltIn == NF_BOOLEAN )
        nFmtType = css::util::NumberFormat::LOGICAL;
    else if ( eBuiltIn == NF_TEXT )
        nFmtType = css::util::NumberFormat::TEXT;

    // #101606# An empty subformat is a valid number-style resulting in an
    // empty display string for the condition of the subformat.

    XMLTokenEnum eType = XML_TOKEN_INVALID;
    switch ( nFmtType )
    {
        // Type UNDEFINED likely is a crappy format string for that we could
        // not decide on any format type (and maybe could try harder?), but the
        // resulting XMLTokenEnum should be something valid, so make that
        // number-style.
        case css::util::NumberFormat::UNDEFINED:
            SAL_WARN("xmloff.style","UNDEFINED number format: '" << rFormat.GetFormatstring() << "'");
            SAL_FALLTHROUGH;
        // Type is 0 if a format contains no recognized elements
        // (like text only) - this is handled as a number-style.
        case 0:
        case css::util::NumberFormat::EMPTY:
        case css::util::NumberFormat::NUMBER:
        case css::util::NumberFormat::SCIENTIFIC:
        case css::util::NumberFormat::FRACTION:
            eType = XML_NUMBER_STYLE;
            break;
        case css::util::NumberFormat::PERCENT:
            eType = XML_PERCENTAGE_STYLE;
            break;
        case css::util::NumberFormat::CURRENCY:
            eType = XML_CURRENCY_STYLE;
            break;
        case css::util::NumberFormat::DATE:
        case css::util::NumberFormat::DATETIME:
            eType = XML_DATE_STYLE;
            break;
        case css::util::NumberFormat::TIME:
            eType = XML_TIME_STYLE;
            break;
        case css::util::NumberFormat::TEXT:
            eType = XML_TEXT_STYLE;
            break;
        case css::util::NumberFormat::LOGICAL:
            eType = XML_BOOLEAN_STYLE;
            break;
    }
    SAL_WARN_IF( eType == XML_TOKEN_INVALID, "xmloff", "unknown format type" );

    OUString sAttrValue;
    bool bUserDef = ( rFormat.GetType() & css::util::NumberFormat::DEFINED );

    //  common attributes for format

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
    bool bAutoOrder = ( eBuiltIn == NF_CURRENCY_1000INT     || eBuiltIn == NF_CURRENCY_1000DEC2 ||
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
    bool bSystemDate = ( eBuiltIn == NF_DATE_SYSTEM_SHORT ||
                         eBuiltIn == NF_DATE_SYSTEM_LONG  ||
                         eBuiltIn == NF_DATETIME_SYSTEM_SHORT_HHMM );
    bool bLongSysDate = ( eBuiltIn == NF_DATE_SYSTEM_LONG );

    // check if the format definition matches the key
    if ( bAutoOrder && ( nFmtType == css::util::NumberFormat::DATE || nFmtType == css::util::NumberFormat::DATETIME ) &&
            !lcl_IsDefaultDateFormat( rFormat, bSystemDate, eBuiltIn ) )
    {
        bAutoOrder = bSystemDate = bLongSysDate = false;        // don't write automatic-order attribute then
    }

    if ( bAutoOrder &&
        ( nFmtType == css::util::NumberFormat::CURRENCY || nFmtType == css::util::NumberFormat::DATE || nFmtType == css::util::NumberFormat::DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_AUTOMATIC_ORDER,
                              XML_TRUE );
    }

    if ( bSystemDate && bAutoOrder &&
        ( nFmtType == css::util::NumberFormat::DATE || nFmtType == css::util::NumberFormat::DATETIME ) )
    {
        //  #85109# format type must be checked to avoid dtd errors if
        //  locale data contains other format types at the built-in positions

        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_FORMAT_SOURCE,
                              XML_LANGUAGE );
    }

    //  overflow for time formats as in [hh]:mm
    //  controlled by bThousand from number format info
    //  default for truncate-on-overflow is true
    if ( nFmtType == css::util::NumberFormat::TIME && bThousand )
    {
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRUNCATE_ON_OVERFLOW,
                              XML_FALSE );
    }

    // Native number transliteration
    css::i18n::NativeNumberXmlAttributes aAttr;
    rFormat.GetNatNumXml( aAttr, nPart );
    if ( !aAttr.Format.isEmpty() )
    {
        /* FIXME-BCP47: ODF defines no transliteration-script or
         * transliteration-rfc-language-tag */
        LanguageTag aLanguageTag( aAttr.Locale);
        OUString aLanguage, aScript, aCountry;
        aLanguageTag.getIsoLanguageScriptCountry( aLanguage, aScript, aCountry);
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_FORMAT,
                              aAttr.Format );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_LANGUAGE,
                              aLanguage );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_COUNTRY,
                              aCountry );
        rExport.AddAttribute( XML_NAMESPACE_NUMBER, XML_TRANSLITERATION_STYLE,
                              aAttr.Style );
    }

    // The element
    SvXMLElementExport aElem( rExport, XML_NAMESPACE_NUMBER, eType,
                              true, true );

    //  color (properties element)

    const Color* pCol = rFormat.GetColor( nPart );
    if (pCol)
        WriteColorElement_Impl(*pCol);

    //  detect if there is "real" content, excluding color and maps
    //! move to implementation of Write... methods?
    bool bAnyContent = false;

    //  format elements

    SvXMLEmbeddedTextEntryArr aEmbeddedEntries;
    if ( eBuiltIn == NF_NUMBER_STANDARD )
    {
        //  default number format contains just one number element
        WriteNumberElement_Impl( -1, -1, 1, OUString(), false, 0, aEmbeddedEntries );
        bAnyContent = true;
    }
    else if ( eBuiltIn == NF_BOOLEAN )
    {
        //  boolean format contains just one boolean element
        WriteBooleanElement_Impl();
        bAnyContent = true;
    }
    else
    {
        //  first loop to collect attributes

        bool bDecDashes  = false;
        bool bExpFound   = false;
        bool bCurrFound  = false;
        bool bInInteger  = true;
        bool bExpSign = true;
        bool bDecAlign   = false;               // decimal alignment with "?"
        sal_Int32 nExpDigits = 0;
        sal_Int32 nIntegerSymbols = 0;          // for embedded-text, including "#"
        sal_Int32 nTrailingThousands = 0;       // thousands-separators after all digits
        sal_Int32 nMinDecimals = nPrecision;
        OUString sCurrExt;
        OUString aCalendar;
        sal_uInt16 nPos = 0;
        bool bEnd = false;
        while (!bEnd)
        {
            short nElemType = rFormat.GetNumForType( nPart, nPos );
            const OUString* pElemStr = rFormat.GetNumForString( nPart, nPos );

            switch ( nElemType )
            {
                case 0:
                    bEnd = true;                // end of format reached
                    break;
                case NF_SYMBOLTYPE_DIGIT:
                    if ( bExpFound && pElemStr )
                        nExpDigits += pElemStr->getLength();
                    else if ( !bDecDashes && pElemStr && (*pElemStr)[0] == '-' )
                    {
                        bDecDashes = true;
                        nMinDecimals = 0;
                    }
                    else if ( !bInInteger && pElemStr )
                    {
                        for ( sal_Int32 i = pElemStr->getLength()-1; i >= 0 ; i-- )
                        {
                            sal_Unicode aChar = (*pElemStr)[i];
                            if ( aChar == '#' || aChar == '?' )
                            {
                                nMinDecimals --;
                                if ( aChar == '?' )
                                    bDecAlign = true;
                            }
                            else
                                break;
                        }
                    }
                    if ( bInInteger && pElemStr )
                        nIntegerSymbols += pElemStr->getLength();
                    nTrailingThousands = 0;
                    break;
                case NF_SYMBOLTYPE_DECSEP:
                    bInInteger = false;
                    break;
                case NF_SYMBOLTYPE_THSEP:
                    if (pElemStr)
                        nTrailingThousands += pElemStr->getLength();      // is reset to 0 if digits follow
                    break;
                case NF_SYMBOLTYPE_EXP:
                    bExpFound = true;           // following digits are exponent digits
                    bInInteger = false;
                    if ( pElemStr && pElemStr->getLength() == 1 )
                        bExpSign = false;       // for 0.00E0
                    break;
                case NF_SYMBOLTYPE_CURRENCY:
                    bCurrFound = true;
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

        bool bAllowEmbedded = ( nFmtType == 0 || nFmtType == css::util::NumberFormat::NUMBER ||
                                        nFmtType == css::util::NumberFormat::CURRENCY ||
                                        nFmtType == css::util::NumberFormat::PERCENT );
        if ( bAllowEmbedded )
        {
            sal_Int32 nDigitsPassed = 0;
            nPos = 0;
            bEnd = false;
            while (!bEnd)
            {
                short nElemType = rFormat.GetNumForType( nPart, nPos );
                const OUString* pElemStr = rFormat.GetNumForString( nPart, nPos );

                switch ( nElemType )
                {
                    case 0:
                        bEnd = true;                // end of format reached
                        break;
                    case NF_SYMBOLTYPE_DIGIT:
                        if ( pElemStr )
                            nDigitsPassed += pElemStr->getLength();
                        break;
                    case NF_SYMBOLTYPE_STRING:
                    case NF_SYMBOLTYPE_BLANK:
                    case NF_SYMBOLTYPE_PERCENT:
                        if ( nDigitsPassed > 0 && nDigitsPassed < nIntegerSymbols && pElemStr )
                        {
                            //  text (literal or underscore) within the integer part of a number:number element

                            OUString aEmbeddedStr;
                            if ( nElemType == NF_SYMBOLTYPE_STRING || nElemType == NF_SYMBOLTYPE_PERCENT )
                            {
                                aEmbeddedStr = *pElemStr;
                            }
                            else if (pElemStr->getLength() >= 2)
                            {
                                SvNumberformat::InsertBlanks( aEmbeddedStr, 0, (*pElemStr)[1] );
                            }
                            sal_Int32 nEmbedPos = nIntegerSymbols - nDigitsPassed;

                            aEmbeddedEntries.push_back(
                                SvXMLEmbeddedTextEntry(nPos, nEmbedPos, aEmbeddedStr));
                        }
                        break;
                }
                ++nPos;
            }
        }

        //  final loop to write elements

        bool bNumWritten = false;
        bool bCurrencyWritten = false;
        short nPrevType = 0;
        nPos = 0;
        bEnd = false;
        while (!bEnd)
        {
            short nElemType = rFormat.GetNumForType( nPart, nPos );
            const OUString* pElemStr = rFormat.GetNumForString( nPart, nPos );

            switch ( nElemType )
            {
                case 0:
                    bEnd = true;                // end of format reached
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
                        else if ( nFmtType == css::util::NumberFormat::CURRENCY && !bCurrFound && !bCurrencyWritten )
                        {
                            //  automatic currency symbol is implemented as part of
                            //  normal text -> search for the symbol
                            bCurrencyWritten = WriteTextWithCurrency_Impl( *pElemStr,
                                LanguageTag::convertToLocale( nLang ) );
                            bAnyContent = true;
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

                        OUString aBlanks;
                        if (pElemStr->getLength() >= 2)
                            SvNumberformat::InsertBlanks( aBlanks, 0, (*pElemStr)[1] );
                        AddToTextElement_Impl( aBlanks );
                    }
                    break;
                case NF_KEY_GENERAL :
                        WriteNumberElement_Impl( -1, -1, 1, OUString(), false, 0, aEmbeddedEntries );
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
                            bAnyContent = true;
                            bCurrencyWritten = true;
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
                            bAnyContent = true;
                            bCurrencyWritten = true;
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
                            case css::util::NumberFormat::NUMBER:
                            case css::util::NumberFormat::CURRENCY:
                            case css::util::NumberFormat::PERCENT:
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
                                    OUStringBuffer sDashStr;
                                    if (bDecDashes && nPrecision > 0)
                                        comphelper::string::padToLength(sDashStr, nPrecision, '-');
                                    // "?" in decimal part are replaced by space character
                                    if (bDecAlign && nPrecision > 0)
                                        sDashStr = " ";

                                    WriteNumberElement_Impl(nDecimals, nMinDecimals, nInteger, sDashStr.makeStringAndClear(),
                                        bThousand, nTrailingThousands, aEmbeddedEntries);
                                    bAnyContent = true;
                                }
                                break;
                            case css::util::NumberFormat::SCIENTIFIC:
                                // #i43959# for scientific numbers, count all integer symbols ("0" and "#")
                                // as integer digits: use nIntegerSymbols instead of nLeading
                                // nIntegerSymbols represents exponent interval (for engineering notation)
                                WriteScientificElement_Impl( nPrecision, nMinDecimals, nLeading, bThousand, nExpDigits, nIntegerSymbols, bExpSign );
                                bAnyContent = true;
                                break;
                            case css::util::NumberFormat::FRACTION:
                                {
                                    sal_Int32 nInteger = nLeading;
                                    if ( rFormat.GetNumForNumberElementCount( nPart ) == 3 )
                                    {
                                        //  If there is only two numbers + fraction in format string
                                        //  the fraction doesn't have an integer part, and no
                                        //  min-integer-digits attribute must be written.
                                        nInteger = -1;
                                    }
                                    WriteFractionElement_Impl( nInteger, bThousand,  rFormat, nPart );
                                    bAnyContent = true;
                                }
                                break;
                        }

                        bNumWritten = true;
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
                    if ( pElemStr && *pElemStr == "@" )
                    {
                        WriteTextContentElement_Impl();
                        bAnyContent = true;
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
                        bool bLong = ( nElemType == NF_KEY_DD );
                        WriteDayElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = true;
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

                        bool bLong = ( nElemType == NF_KEY_NNN || nElemType == NF_KEY_NNNN ||
                                           nElemType == NF_KEY_DDDD || nElemType == NF_KEY_AAAA );
                        WriteDayOfWeekElement_Impl( aCalAttr, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = true;
                        if ( nElemType == NF_KEY_NNNN )
                        {
                            //  write additional text element for separator
                            pLocaleData->setLanguageTag( LanguageTag( nLang ) );
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
                        bool bLong = ( nElemType == NF_KEY_MM  || nElemType == NF_KEY_MMMM );
                        bool bText = ( nElemType == NF_KEY_MMM || nElemType == NF_KEY_MMMM ||
                                            nElemType == NF_KEY_MMMMM );
                        WriteMonthElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ), bText );
                        bAnyContent = true;
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
                        bool bLong = ( nElemType == NF_KEY_YYYY || nElemType == NF_KEY_EEC ||
                                            nElemType == NF_KEY_R );
                        WriteYearElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = true;
                    }
                    break;
                case NF_KEY_G:
                case NF_KEY_GG:
                case NF_KEY_GGG:
                case NF_KEY_RR:     //! RR acts as GGGEE, no attribute available
                    {
                        //! distinguish GG and GGG and RR
                        bool bLong = ( nElemType == NF_KEY_GGG || nElemType == NF_KEY_RR );
                        WriteEraElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = true;
                        if ( nElemType == NF_KEY_RR )
                        {
                            //  calendar attribute for RR is set in first loop
                            WriteYearElement_Impl( aCalendar, ( bSystemDate || bLongSysDate ) );
                        }
                    }
                    break;
                case NF_KEY_Q:
                case NF_KEY_QQ:
                    {
                        bool bLong = ( nElemType == NF_KEY_QQ );
                        WriteQuarterElement_Impl( aCalendar, ( bSystemDate ? bLongSysDate : bLong ) );
                        bAnyContent = true;
                    }
                    break;
                case NF_KEY_WW:
                    WriteWeekElement_Impl( aCalendar );
                    bAnyContent = true;
                    break;

                // time elements (bSystemDate is not used):

                case NF_KEY_H:
                case NF_KEY_HH:
                    WriteHoursElement_Impl( nElemType == NF_KEY_HH );
                    bAnyContent = true;
                    break;
                case NF_KEY_MI:
                case NF_KEY_MMI:
                    WriteMinutesElement_Impl( nElemType == NF_KEY_MMI );
                    bAnyContent = true;
                    break;
                case NF_KEY_S:
                case NF_KEY_SS:
                    WriteSecondsElement_Impl( ( nElemType == NF_KEY_SS ), nPrecision );
                    bAnyContent = true;
                    break;
                case NF_KEY_AMPM:
                case NF_KEY_AP:
                    WriteAMPMElement_Impl();        // short/long?
                    bAnyContent = true;
                    break;
                case NF_SYMBOLTYPE_STAR :
                    // export only if ODF 1.2 extensions are enabled
                    if( rExport.getDefaultVersion() > SvtSaveOptions::ODFVER_012 )
                    {
                        if ( pElemStr && pElemStr->getLength() > 1 )
                            WriteRepeatedElement_Impl( (*pElemStr)[1] );
                    }
                    break;
            }
            nPrevType = nElemType;
            ++nPos;
        }
    }

    if ( !sTextContent.isEmpty() )
        bAnyContent = true;     // element written in FinishTextElement_Impl

    FinishTextElement_Impl();       // final text element - before maps

    if ( !bAnyContent )
    {
        //  for an empty format, write an empty text element
        SvXMLElementExport aTElem( rExport, XML_NAMESPACE_NUMBER, XML_TEXT,
                                   true, false );
    }

    //  mapping (conditions) must be last elements

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
            //  by reversing the 2nd condition.
            //  For a trailing text format like  0;@  that has no conditions
            //  use a "less or equal than biggest" condition for the number
            //  part, ODF can't store subformats (style maps) without
            //  conditions.

            SvNumberformatLimitOps eOp3 = NUMBERFORMAT_OP_NO;
            double fLimit3 = fLimit2;
            sal_uInt16 nLastPart = 2;
            SvNumberformatLimitOps eOpLast = eOp2;
            if (eOp2 == NUMBERFORMAT_OP_NO)
            {
                eOpLast = eOp1;
                fLimit3 = fLimit1;
                nLastPart = (eOp1 == NUMBERFORMAT_OP_NO) ? 0 : 1;
            }
            switch ( eOpLast )
            {
                case NUMBERFORMAT_OP_EQ: eOp3 = NUMBERFORMAT_OP_NE; break;
                case NUMBERFORMAT_OP_NE: eOp3 = NUMBERFORMAT_OP_EQ; break;
                case NUMBERFORMAT_OP_LT: eOp3 = NUMBERFORMAT_OP_GE; break;
                case NUMBERFORMAT_OP_LE: eOp3 = NUMBERFORMAT_OP_GT; break;
                case NUMBERFORMAT_OP_GT: eOp3 = NUMBERFORMAT_OP_LE; break;
                case NUMBERFORMAT_OP_GE: eOp3 = NUMBERFORMAT_OP_LT; break;
                case NUMBERFORMAT_OP_NO: eOp3 = NUMBERFORMAT_OP_LE; fLimit3 = DBL_MAX; break;
            }

            if ( fLimit1 == fLimit2 &&
                    ( ( eOp1 == NUMBERFORMAT_OP_LT && eOp2 == NUMBERFORMAT_OP_GT ) ||
                      ( eOp1 == NUMBERFORMAT_OP_GT && eOp2 == NUMBERFORMAT_OP_LT ) ) )
            {
                //  For <x and >x, add =x as last condition
                //  (just for readability, <=x would be valid, too)

                eOp3 = NUMBERFORMAT_OP_EQ;
            }

            WriteMapElement_Impl( eOp3, fLimit3, nKey, nLastPart );
        }
    }
}

//  export one format

void SvXMLNumFmtExport::ExportFormat_Impl( const SvNumberformat& rFormat, sal_uInt32 nKey )
{
    const sal_uInt16 XMLNUM_MAX_PARTS = 4;
    bool bParts[XMLNUM_MAX_PARTS] = { false, false, false, false };
    sal_uInt16 nUsedParts = 0;
    for (sal_uInt16 nPart=0; nPart<XMLNUM_MAX_PARTS; ++nPart)
    {
        if (rFormat.GetNumForInfoScannedType( nPart) != css::util::NumberFormat::UNDEFINED)
        {
            bParts[nPart] = true;
            nUsedParts = nPart + 1;
        }
    }

    SvNumberformatLimitOps eOp1, eOp2;
    double fLimit1, fLimit2;
    rFormat.GetConditions( eOp1, fLimit1, eOp2, fLimit2 );

    //  if conditions are set, even empty formats must be written

    if ( eOp1 != NUMBERFORMAT_OP_NO )
    {
        bParts[1] = true;
        if (nUsedParts < 2)
            nUsedParts = 2;
    }
    if ( eOp2 != NUMBERFORMAT_OP_NO )
    {
        bParts[2] = true;
        if (nUsedParts < 3)
            nUsedParts = 3;
    }
    if ( rFormat.HasTextFormat() )
    {
        bParts[3] = true;
        if (nUsedParts < 4)
            nUsedParts = 4;
    }

    for (sal_uInt16 nPart=0; nPart<XMLNUM_MAX_PARTS; ++nPart)
    {
        if (bParts[nPart])
        {
            bool bDefault = ( nPart+1 == nUsedParts );          // last = default
            ExportPart_Impl( rFormat, nKey, nPart, bDefault );
        }
    }
}

//  export method called by application

void SvXMLNumFmtExport::Export( bool bIsAutoStyle )
{
    if ( !pFormatter )
        return;                         // no formatter -> no entries

    sal_uInt32 nKey;
    const SvNumberformat* pFormat = nullptr;
    bool bNext(pUsedList->GetFirstUsed(nKey));
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
                                            css::util::NumberFormat::DEFINED, nDefaultIndex, nLang );
            SvNumberFormatTable::iterator it2 = rTable.begin();
            while (it2 != rTable.end())
            {
                nKey = it2->first;
                pFormat = it2->second;
                if (!pUsedList->IsUsed(nKey))
                {
                    DBG_ASSERT((pFormat->GetType() & css::util::NumberFormat::DEFINED), "a not user defined numberformat found");
                    //  user-defined and used formats are exported
                    ExportFormat_Impl( *pFormat, nKey );
                    // if it is a user-defined Format it will be added else nothing will happen
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
        return lcl_CreateStyleName( nKey, 0, true, sPrefix );
    else
    {
        OSL_FAIL("There is no written Data-Style");
        return OUString();
    }
}

void SvXMLNumFmtExport::SetUsed( sal_uInt32 nKey )
{
    SAL_WARN_IF( pFormatter == nullptr, "xmloff", "missing formatter" );
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

static const SvNumberformat* lcl_GetFormat( SvNumberFormatter* pFormatter,
                           sal_uInt32 nKey )
{
    return ( pFormatter != nullptr ) ? pFormatter->GetEntry( nKey ) : nullptr;
}

sal_uInt32 SvXMLNumFmtExport::ForceSystemLanguage( sal_uInt32 nKey )
{
    sal_uInt32 nRet = nKey;

    const SvNumberformat* pFormat = lcl_GetFormat( pFormatter, nKey );
    if( pFormat != nullptr )
    {
        SAL_WARN_IF( pFormatter == nullptr, "xmloff", "format without formatter?" );

        sal_Int32 nErrorPos;
        short nType = pFormat->GetType();

        sal_uInt32 nNewKey = pFormatter->GetFormatForLanguageIfBuiltIn(
                       nKey, LANGUAGE_SYSTEM );

        if( nNewKey != nKey )
        {
            nRet = nNewKey;
        }
        else
        {
            OUString aFormatString( pFormat->GetFormatstring() );
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
