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

#include <svl/zforlist.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <svl/numuno.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <utility>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/languagetagodf.hxx>

#include <memory>
#include <string_view>
#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

struct SvXMLNumFmtEntry
{
    OUString   aName;
    sal_uInt32  nKey;
    bool        bRemoveAfterUse;

    SvXMLNumFmtEntry( OUString aN, sal_uInt32 nK, bool bR ) :
        aName(std::move(aN)), nKey(nK), bRemoveAfterUse(bR) {}
};

}

class SvXMLNumImpData
{
    SvNumberFormatter*  pFormatter;
    std::unique_ptr<LocaleDataWrapper>  pLocaleData;
    std::vector<SvXMLNumFmtEntry> m_NameEntries;

    uno::Reference< uno::XComponentContext > m_xContext;

public:
    SvXMLNumImpData(
        SvNumberFormatter* pFmt,
        const uno::Reference<uno::XComponentContext>& rxContext );

    SvNumberFormatter*      GetNumberFormatter() const  { return pFormatter; }
    const LocaleDataWrapper&    GetLocaleData( LanguageType nLang );
    sal_uInt32              GetKeyForName( std::u16string_view rName );
    void                    AddKey( sal_uInt32 nKey, const OUString& rName, bool bRemoveAfterUse );
    void                    SetUsed( sal_uInt32 nKey );
    void                    RemoveVolatileFormats();
};

struct SvXMLNumberInfo
{
    sal_Int32   nDecimals           = -1;
    sal_Int32   nInteger            = -1;       /// Total min number of digits in integer part ('0' + '?')
    sal_Int32   nBlankInteger       = -1;       /// Number of '?' in integer part
    sal_Int32   nExpDigits          = -1;       /// Number of '0' and '?' in exponent
    sal_Int32   nBlankExp           = -1;       /// Number of '?' in exponent
    sal_Int32   nExpInterval        = -1;
    sal_Int32   nMinNumerDigits     = -1;
    sal_Int32   nMinDenomDigits     = -1;
    sal_Int32   nMaxNumerDigits     = -1;
    sal_Int32   nMaxDenomDigits     = -1;
    sal_Int32   nFracDenominator    = -1;
    sal_Int32   nMinDecimalDigits   = -1;
    sal_Int32   nZerosNumerDigits   = -1;
    sal_Int32   nZerosDenomDigits   = -1;
    bool        bGrouping           = false;
    bool        bDecReplace         = false;
    bool        bExpSign            = true;
    bool        bExponentLowercase  = false;     /// Exponent is 'e' instead of 'E'
    bool        bDecAlign           = false;
    double      fDisplayFactor      = 1.0;
    OUString    aIntegerFractionDelimiter;
    std::map<sal_Int32, OUString> m_EmbeddedElements;
};

namespace {

enum class SvXMLStyleTokens;

class SvXMLNumFmtElementContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    SvXMLStyleTokens        nType;
    OUStringBuffer          aContent;
    SvXMLNumberInfo         aNumInfo;
    LanguageType            nElementLang;
    bool                    bLong;
    bool                    bTextual;
    OUString                sCalendar;
    OUString                sBlankWidthString;

public:
                SvXMLNumFmtElementContext( SvXMLImport& rImport, sal_Int32 nElement,
                                    SvXMLNumFormatContext& rParentContext, SvXMLStyleTokens nNewType,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL characters( const OUString& rChars ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void    AddEmbeddedElement( sal_Int32 nFormatPos, std::u16string_view rContent, std::u16string_view rBlankWidthString );
};

class SvXMLNumFmtEmbeddedTextContext : public SvXMLImportContext
{
    SvXMLNumFmtElementContext&  rParent;
    OUStringBuffer         aContent;
    sal_Int32                   nTextPosition;
    OUString                    aBlankWidthString;

public:
                SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport, sal_Int32 nElement,
                                    SvXMLNumFmtElementContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList );

    virtual void SAL_CALL characters( const OUString& rChars ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SvXMLNumFmtMapContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    OUString           sCondition;
    OUString           sName;

public:
                SvXMLNumFmtMapContext( SvXMLImport& rImport, sal_Int32 nElement,
                                    SvXMLNumFormatContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SvXMLNumFmtPropContext : public SvXMLImportContext
{
    SvXMLNumFormatContext&  rParent;
    Color                   m_nColor;
    bool                    bColSet;

public:
                SvXMLNumFmtPropContext( SvXMLImport& rImport, sal_Int32 nElement,
                                    SvXMLNumFormatContext& rParentContext,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList );

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

enum class SvXMLStyleTokens
{
    Text,
    FillCharacter,
    Number,
    ScientificNumber,
    Fraction,
    CurrencySymbol,
    Day,
    Month,
    Year,
    Era,
    DayOfWeek,
    WeekOfYear,
    Quarter,
    Hours,
    AmPm,
    Minutes,
    Seconds,
    Boolean,
    TextContent
};

}

//  standard colors


#define XML_NUMF_COLORCOUNT     10

const Color aNumFmtStdColors[XML_NUMF_COLORCOUNT] =
{
    COL_BLACK,
    COL_LIGHTBLUE,
    COL_LIGHTGREEN,
    COL_LIGHTCYAN,
    COL_LIGHTRED,
    COL_LIGHTMAGENTA,
    COL_BROWN,
    COL_GRAY,
    COL_YELLOW,
    COL_WHITE
};


//  token maps


// maps for SvXMLUnitConverter::convertEnum

const SvXMLEnumMapEntry<bool> aStyleValueMap[] =
{
    { XML_SHORT,            false },
    { XML_LONG,             true },
    { XML_TOKEN_INVALID,    false }
};

const SvXMLEnumMapEntry<bool> aFormatSourceMap[] =
{
    { XML_FIXED,            false },
    { XML_LANGUAGE,         true },
    { XML_TOKEN_INVALID,    false }
};

namespace {

struct SvXMLDefaultDateFormat
{
    NfIndexTableOffset          eFormat;
    SvXMLDateElementAttributes  eDOW;
    SvXMLDateElementAttributes  eDay;
    SvXMLDateElementAttributes  eMonth;
    SvXMLDateElementAttributes  eYear;
    SvXMLDateElementAttributes  eHours;
    SvXMLDateElementAttributes  eMins;
    SvXMLDateElementAttributes  eSecs;
    bool                        bSystem;
};

}

const SvXMLDefaultDateFormat aDefaultDateFormats[] =
{
    // format                           day-of-week     day             month               year            hours           minutes         seconds         format-source

    { NF_DATE_SYSTEM_SHORT,             XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   true },
    { NF_DATE_SYSTEM_LONG,              XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   true },
    { NF_DATE_SYS_MMYY,                 XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMM,                XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_TEXTSHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMYYYY,             XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DDMMYY,               XML_DEA_NONE,   XML_DEA_LONG,   XML_DEA_LONG,       XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMYY,               XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMYYYY,             XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_DMMMMYYYY,            XML_DEA_NONE,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNDMMMYY,             XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTSHORT,  XML_DEA_SHORT,  XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNDMMMMYYYY,          XML_DEA_SHORT,  XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATE_SYS_NNNNDMMMMYYYY,        XML_DEA_LONG,   XML_DEA_SHORT,  XML_DEA_TEXTLONG,   XML_DEA_LONG,   XML_DEA_NONE,   XML_DEA_NONE,   XML_DEA_NONE,   false },
    { NF_DATETIME_SYS_DDMMYYYY_HHMM,    XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_LONG,   XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_NONE,   false },
    { NF_DATETIME_SYSTEM_SHORT_HHMM,    XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_NONE,   true },
    { NF_DATETIME_SYS_DDMMYYYY_HHMMSS,  XML_DEA_NONE,   XML_DEA_ANY,    XML_DEA_ANY,        XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    XML_DEA_ANY,    false }
};


//  SvXMLNumImpData


SvXMLNumImpData::SvXMLNumImpData(
    SvNumberFormatter* pFmt,
    const uno::Reference<uno::XComponentContext>& rxContext )
:   pFormatter(pFmt),
    m_xContext(rxContext)
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );
}

sal_uInt32 SvXMLNumImpData::GetKeyForName( std::u16string_view rName )
{
    for (const auto& rObj : m_NameEntries)
    {
        if (rObj.aName == rName)
            return rObj.nKey;              // found
    }
    return NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void SvXMLNumImpData::AddKey( sal_uInt32 nKey, const OUString& rName, bool bRemoveAfterUse )
{
    if ( bRemoveAfterUse )
    {
        //  if there is already an entry for this key without the bRemoveAfterUse flag,
        //  clear the flag for this entry, too

        for (const auto& rObj : m_NameEntries)
        {
            if (rObj.nKey == nKey && !rObj.bRemoveAfterUse)
            {
                bRemoveAfterUse = false;        // clear flag for new entry
                break;
            }
        }
    }
    else
    {
        //  call SetUsed to clear the bRemoveAfterUse flag for other entries for this key
        SetUsed( nKey );
    }

    m_NameEntries.emplace_back(rName, nKey, bRemoveAfterUse);
}

void SvXMLNumImpData::SetUsed( sal_uInt32 nKey )
{
    for (auto& rObj : m_NameEntries)
    {
        if (rObj.nKey == nKey)
        {
            rObj.bRemoveAfterUse = false;      // used -> don't remove

            //  continue searching - there may be several entries for the same key
            //  (with different names), the format must not be deleted if any one of
            //  them is used
        }
    }
}

void SvXMLNumImpData::RemoveVolatileFormats()
{
    //  remove temporary (volatile) formats from NumberFormatter
    //  called at the end of each import (styles and content), so volatile formats
    //  from styles can't be used in content

    if ( !pFormatter )
        return;

    for (const auto& rObj : m_NameEntries)
    {
        if (rObj.bRemoveAfterUse )
        {
            const SvNumberformat* pFormat = pFormatter->GetEntry(rObj.nKey);
            if (pFormat && (pFormat->GetType() & SvNumFormatType::DEFINED))
                pFormatter->DeleteEntry(rObj.nKey);
        }
    }
}

const LocaleDataWrapper& SvXMLNumImpData::GetLocaleData( LanguageType nLang )
{
    if ( !pLocaleData || pLocaleData->getLanguageTag() != LanguageTag(nLang) )
        pLocaleData = std::make_unique<LocaleDataWrapper>(
               pFormatter ? pFormatter->GetComponentContext() : m_xContext,
            LanguageTag( nLang ) );
    return *pLocaleData;
}


//  SvXMLNumFmtMapContext


SvXMLNumFmtMapContext::SvXMLNumFmtMapContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    SvXMLNumFormatContext& rParentContext,
                                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport ),
    rParent( rParentContext )
{
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_CONDITION):
                sCondition = sValue;
                break;
            case XML_ELEMENT(STYLE, XML_APPLY_STYLE_NAME):
                sName = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SvXMLNumFmtMapContext::endFastElement(sal_Int32 )
{
    rParent.AddCondition( sCondition, sName );
}


//  SvXMLNumFmtPropContext


SvXMLNumFmtPropContext::SvXMLNumFmtPropContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    SvXMLNumFormatContext& rParentContext,
                                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport ),
    rParent( rParentContext ),
    m_nColor( 0 ),
    bColSet( false )
{
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        switch ( aIter.getToken())
        {
            case XML_ELEMENT(FO, XML_COLOR):
            case XML_ELEMENT(FO_COMPAT, XML_COLOR):
                bColSet = ::sax::Converter::convertColor( m_nColor, aIter.toView() );
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

void SvXMLNumFmtPropContext::endFastElement(sal_Int32 )
{
    if (bColSet)
        rParent.AddColor( m_nColor );
}


//  SvXMLNumFmtEmbeddedTextContext


SvXMLNumFmtEmbeddedTextContext::SvXMLNumFmtEmbeddedTextContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    SvXMLNumFmtElementContext& rParentContext,
                                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport ),
    rParent( rParentContext ),
    nTextPosition( 0 )
{
    sal_Int32 nAttrVal;

    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        if ( aIter.getToken() == XML_ELEMENT(NUMBER, XML_POSITION) )
        {
            if (::sax::Converter::convertNumber( nAttrVal, aIter.toView() ))
                nTextPosition = nAttrVal;
        }
        else if ( aIter.getToken() == XML_ELEMENT(LO_EXT, XML_BLANK_WIDTH_CHAR)
                || aIter.getToken() == XML_ELEMENT(NUMBER, XML_BLANK_WIDTH_CHAR) )
        {
            aBlankWidthString = aIter.toString();
        }
        else
            XMLOFF_WARN_UNKNOWN("xmloff", aIter);
    }
}

void SvXMLNumFmtEmbeddedTextContext::characters( const OUString& rChars )
{
    aContent.append( rChars );
}

void SvXMLNumFmtEmbeddedTextContext::endFastElement(sal_Int32 )
{
    rParent.AddEmbeddedElement( nTextPosition, aContent.makeStringAndClear(), aBlankWidthString );
}

static bool lcl_ValidChar( sal_Unicode cChar, const SvXMLNumFormatContext& rParent )
{
    SvXMLStylesTokens nFormatType = rParent.GetType();

    // Treat space equal to non-breaking space separator.
    const sal_Unicode cNBSP = 0x00A0;
    sal_Unicode cTS;
    if ( ( nFormatType == SvXMLStylesTokens::NUMBER_STYLE ||
           nFormatType == SvXMLStylesTokens::CURRENCY_STYLE ||
           nFormatType == SvXMLStylesTokens::PERCENTAGE_STYLE ) &&
            (cChar == (cTS = rParent.GetLocaleData().getNumThousandSep()[0]) ||
             (cChar == ' ' && cTS == cNBSP)) )
    {
        //  #i22394# Extra occurrences of thousands separator must be quoted, so they
        //  aren't mis-interpreted as display-factor.
        //  This must be limited to the format types that can contain a number element,
        //  because the same character can be a date separator that should not be quoted
        //  in date formats.

        return false;   // force quotes
    }

    //  see ImpSvNumberformatScan::Next_Symbol

    // All format types except BOOLEAN may contain minus sign or delimiter.
    if ( cChar == '-' )
        return nFormatType != SvXMLStylesTokens::BOOLEAN_STYLE;

    if ( ( cChar == ' ' ||
           cChar == '/' ||
           cChar == '.' ||
           cChar == ',' ||
           cChar == ':' ||
           cChar == '\''   ) &&
         ( nFormatType == SvXMLStylesTokens::CURRENCY_STYLE ||
           nFormatType == SvXMLStylesTokens::DATE_STYLE ||
           nFormatType == SvXMLStylesTokens::TIME_STYLE ) ) // other formats do not require delimiter tdf#97837
        return true;

    //  percent sign must be used without quotes for percentage styles only
    if ( nFormatType == SvXMLStylesTokens::PERCENTAGE_STYLE && cChar == '%' )
        return true;

    //  don't put quotes around single parentheses (often used for negative numbers)
    if ( ( nFormatType == SvXMLStylesTokens::NUMBER_STYLE ||
           nFormatType == SvXMLStylesTokens::CURRENCY_STYLE ||
           nFormatType == SvXMLStylesTokens::PERCENTAGE_STYLE ) &&
         ( cChar == '(' || cChar == ')' ) )
        return true;

    return false;
}

static void lcl_EnquoteIfNecessary( OUStringBuffer& rContent, const SvXMLNumFormatContext& rParent )
{
    bool bQuote = true;
    sal_Int32 nLength = rContent.getLength();
    const SvXMLStylesTokens nFormatType = rParent.GetType();

    if (nFormatType != SvXMLStylesTokens::BOOLEAN_STYLE &&
            ((nLength == 1 && lcl_ValidChar( rContent[0], rParent)) ||
             (nLength == 2 &&
              ((rContent[0] == ' ' && rContent[1] == '-') ||
               (rContent[1] == ' ' && lcl_ValidChar( rContent[0], rParent))))))
    {
        //  Don't quote single separator characters like space or percent,
        //  or separator characters followed by space (used in date formats).
        //  Or space followed by minus (used in currency formats) that would
        //  lead to almost duplicated formats with built-in formats just with
        //  the difference of quotes.
        bQuote = false;
    }
    else if ( nFormatType == SvXMLStylesTokens::PERCENTAGE_STYLE && nLength > 1 )
    {
        //  the percent character in percentage styles must be left out of quoting
        //  (one occurrence is enough even if there are several percent characters in the string)

        sal_Int32 nPos = rContent.indexOf( '%' );
        if ( nPos >= 0 )
        {
            if ( nPos + 1 < nLength )
            {
                if ( nPos + 2 == nLength && lcl_ValidChar( rContent[nPos + 1], rParent ) )
                {
                    //  single character that doesn't need quoting
                }
                else
                {
                    //  quote text behind percent character
                    rContent.insert( nPos + 1, '"' );
                    rContent.append( '"' );
                }
            }
            if ( nPos > 0 )
            {
                if ( nPos == 1 && lcl_ValidChar( rContent[0], rParent ) )
                {
                    //  single character that doesn't need quoting
                }
                else
                {
                    //  quote text before percent character
                    rContent.insert( nPos, '"' );
                    rContent.insert( 0, '"' );
                }
            }
            bQuote = false;
        }
        // else: normal quoting (below)
    }

    if ( !bQuote )
        return;

    // #i55469# quotes in the string itself have to be escaped
    bool bEscape = ( rContent.indexOf( '"' ) >= 0 );
    if ( bEscape )
    {
        // A quote is turned into "\"" - a quote to end quoted text, an escaped quote,
        // and a quote to resume quoting.
        OUString aInsert(  "\"\\\""  );

        sal_Int32 nPos = 0;
        while ( nPos < rContent.getLength() )
        {
            if ( rContent[nPos] == '"' )
            {
                rContent.insert( nPos, aInsert );
                nPos += aInsert.getLength();
            }
            ++nPos;
        }
    }

    //  quote string literals
    rContent.insert( 0, '"' );
    rContent.append( '"' );

    // remove redundant double quotes at start or end
    if ( !bEscape )
        return;

    if ( rContent.getLength() > 2 &&
         rContent[0] == '"' &&
         rContent[1] == '"' )
    {
        rContent.remove(0, 2);
    }

    sal_Int32 nLen = rContent.getLength();
    if ( nLen > 2 &&
         rContent[nLen - 1] == '"' &&
         rContent[nLen - 2] == '"' )
    {
        rContent.truncate(nLen - 2);
    }
}


//  SvXMLNumFmtElementContext


SvXMLNumFmtElementContext::SvXMLNumFmtElementContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    SvXMLNumFormatContext& rParentContext, SvXMLStyleTokens nNewType,
                                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport ),
    rParent( rParentContext ),
    nType( nNewType ),
    nElementLang( LANGUAGE_SYSTEM ),
    bLong( false ),
    bTextual( false )
{
    LanguageTagODF aLanguageTagODF;
    sal_Int32 nAttrVal;
    bool bAttrBool(false);
    bool bVarDecimals = false;
    bool bIsMaxDenominator = false;
    double fAttrDouble;

    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(NUMBER, XML_DECIMAL_PLACES):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                {
                    // fdo#58539 & gnome#627420: limit number of digits during import
                    aNumInfo.nDecimals = std::min<sal_Int32>(nAttrVal, NF_MAX_FORMAT_SYMBOLS);
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_MIN_DECIMAL_PLACES):
            case XML_ELEMENT(NUMBER, XML_MIN_DECIMAL_PLACES):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nMinDecimalDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_INTEGER_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nInteger = nAttrVal;
                break;
            case XML_ELEMENT(LO_EXT, XML_MAX_BLANK_INTEGER_DIGITS):
            case XML_ELEMENT(NUMBER, XML_MAX_BLANK_INTEGER_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nBlankInteger = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_GROUPING):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    aNumInfo.bGrouping = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_DISPLAY_FACTOR):
                if (::sax::Converter::convertDouble( fAttrDouble, aIter.toView() ))
                    aNumInfo.fDisplayFactor = fAttrDouble;
                break;
            case XML_ELEMENT(NUMBER, XML_DECIMAL_REPLACEMENT):
                if ( aIter.toView() == " " )
                {
                    aNumInfo.bDecAlign = true; // space replacement for "?"
                    bVarDecimals = true;
                }
                else
                    if ( aIter.isEmpty() )
                        bVarDecimals = true;   // empty replacement string: variable decimals
                    else                                // all other strings
                        aNumInfo.bDecReplace = true;    // decimal replacement with dashes
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_EXPONENT_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nExpDigits = std::min<sal_Int32>(nAttrVal, NF_MAX_FORMAT_SYMBOLS);
                break;
            case XML_ELEMENT(NUMBER, XML_BLANK_EXPONENT_DIGITS):
            case XML_ELEMENT(LO_EXT, XML_BLANK_EXPONENT_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nBlankExp = std::min<sal_Int32>(nAttrVal, NF_MAX_FORMAT_SYMBOLS);
                break;
            case XML_ELEMENT(NUMBER, XML_EXPONENT_INTERVAL):
            case XML_ELEMENT(LO_EXT, XML_EXPONENT_INTERVAL):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nExpInterval = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_FORCED_EXPONENT_SIGN):
            case XML_ELEMENT(LO_EXT, XML_FORCED_EXPONENT_SIGN):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    aNumInfo.bExpSign = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_EXPONENT_LOWERCASE):
            case XML_ELEMENT(LO_EXT, XML_EXPONENT_LOWERCASE):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    aNumInfo.bExponentLowercase = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nMinNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_DENOMINATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nMinDenomDigits = nAttrVal;
                break;
            case XML_ELEMENT(LO_EXT, XML_MAX_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 1 ))  // at least one '#'
                    aNumInfo.nMaxNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_DENOMINATOR_VALUE):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 1 )) // 0 is not valid
                {
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = false;
                }
                break;
            case XML_ELEMENT(NUMBER, XML_MAX_DENOMINATOR_VALUE):  // part of ODF 1.3
            case XML_ELEMENT(LO_EXT, XML_MAX_DENOMINATOR_VALUE):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 1 ) && aNumInfo.nFracDenominator <= 0)
                {   // if denominator value not yet defined
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = true;
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_ZEROS_NUMERATOR_DIGITS):
            case XML_ELEMENT(NUMBER, XML_ZEROS_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nZerosNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_ZEROS_DENOMINATOR_DIGITS):
            case XML_ELEMENT(LO_EXT, XML_ZEROS_DENOMINATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, aIter.toView(), 0 ))
                    aNumInfo.nZerosDenomDigits = nAttrVal;
                 break;
            case XML_ELEMENT(NUMBER, XML_INTEGER_FRACTION_DELIMITER):
            case XML_ELEMENT(LO_EXT, XML_INTEGER_FRACTION_DELIMITER):
                aNumInfo.aIntegerFractionDelimiter = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_RFC_LANGUAGE_TAG):
                aLanguageTagODF.maRfcLanguageTag = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_LANGUAGE):
                aLanguageTagODF.maLanguage = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_SCRIPT):
                aLanguageTagODF.maScript = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_COUNTRY):
                aLanguageTagODF.maCountry = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_STYLE):
                SvXMLUnitConverter::convertEnum( bLong, aIter.toView(), aStyleValueMap );
                break;
            case XML_ELEMENT(NUMBER, XML_TEXTUAL):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    bTextual = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_CALENDAR):
                sCalendar = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_BLANK_WIDTH_CHAR):
            case XML_ELEMENT(LO_EXT, XML_BLANK_WIDTH_CHAR):
                sBlankWidthString = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    if ( aNumInfo.nBlankInteger > aNumInfo.nInteger )
        aNumInfo.nInteger = aNumInfo.nBlankInteger;
    if ( aNumInfo.nMinDecimalDigits == -1)
    {
        if ( bVarDecimals || aNumInfo.bDecReplace )
            aNumInfo.nMinDecimalDigits = 0;
        else
            aNumInfo.nMinDecimalDigits = aNumInfo.nDecimals;
    }
    if ( aNumInfo.nExpDigits > 0 && aNumInfo.nBlankExp >= aNumInfo.nExpDigits )
        aNumInfo.nBlankExp = aNumInfo.nExpDigits - 1; // at least one '0' in exponent

    if ( aNumInfo.nZerosDenomDigits > 0 )
    {   // nMin = count of '0' and '?'
        if ( aNumInfo.nMinDenomDigits < aNumInfo.nZerosDenomDigits )
            aNumInfo.nMinDenomDigits = aNumInfo.nZerosDenomDigits;
    }
    else
        aNumInfo.nZerosDenomDigits = 0;
    if ( aNumInfo.nMinDenomDigits >= 0 )
        if ( aNumInfo.nMaxDenomDigits < aNumInfo.nMinDenomDigits )
            aNumInfo.nMaxDenomDigits = ( aNumInfo.nMinDenomDigits ? aNumInfo.nMinDenomDigits : 1 );
    if ( aNumInfo.nZerosNumerDigits > 0 )
    {
        if ( aNumInfo.nMinNumerDigits < aNumInfo.nZerosNumerDigits )
            aNumInfo.nMinNumerDigits = aNumInfo.nZerosNumerDigits;
    }
    else
        aNumInfo.nZerosNumerDigits = 0;
    if ( aNumInfo.nMinNumerDigits >= 0 )
        if ( aNumInfo.nMaxNumerDigits < aNumInfo.nMinNumerDigits )
            aNumInfo.nMaxNumerDigits = ( aNumInfo.nMinNumerDigits ? aNumInfo.nMinNumerDigits : 1 );
    if ( bIsMaxDenominator && aNumInfo.nFracDenominator > 0 )
    {
        aNumInfo.nMaxDenomDigits = floor( log10( aNumInfo.nFracDenominator ) ) + 1;
        aNumInfo.nFracDenominator = -1;  // Max denominator value only gives number of digits at denominator
    }
    if ( aNumInfo.nMaxDenomDigits > 0 )
    {
        if ( aNumInfo.nMinDenomDigits < 0 )
            aNumInfo.nMinDenomDigits = 0;
        else if ( aNumInfo.nMinDenomDigits > aNumInfo.nMaxDenomDigits )
            aNumInfo.nMinDenomDigits = aNumInfo.nMaxDenomDigits;
    }

    if ( !aLanguageTagODF.isEmpty() )
    {
        nElementLang = aLanguageTagODF.getLanguageTag().getLanguageType( false);
        if ( nElementLang == LANGUAGE_DONTKNOW )
            nElementLang = LANGUAGE_SYSTEM;         //! error handling for unknown locales?
    }

    if ( aNumInfo.aIntegerFractionDelimiter.isEmpty() )
        aNumInfo.aIntegerFractionDelimiter = " ";
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvXMLNumFmtElementContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    //  only number:number and number:scientific-number supports number:embedded-text child element

    if ( ( nType == SvXMLStyleTokens::Number || nType == SvXMLStyleTokens::ScientificNumber ) &&
         nElement == XML_ELEMENT(NUMBER, XML_EMBEDDED_TEXT) )
    {
        return new SvXMLNumFmtEmbeddedTextContext( GetImport(), nElement, *this, xAttrList );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void SvXMLNumFmtElementContext::characters( const OUString& rChars )
{
    aContent.append( rChars );
}

namespace {
void lcl_InsertBlankWidthChars( std::u16string_view rBlankWidthString, OUStringBuffer& rContent )
{
    sal_Int32 nShiftPosition = 1; // rContent starts with a quote
    const size_t nLenBlank = rBlankWidthString.size();
    for ( size_t i = 0 ; i < nLenBlank ; i++ )
    {
        sal_Unicode nChar = rBlankWidthString[ i ];
        OUString aBlanks;
        SvNumberformat::InsertBlanks( aBlanks, 0, nChar );
        sal_Int32 nPositionContent = 0;
        if ( ++i < nLenBlank )
        {
            sal_Int32 nNext = rBlankWidthString.find( '_', i );
            if ( static_cast<sal_Int32>( i ) < nNext )
            {
                nPositionContent = o3tl::toInt32( rBlankWidthString.substr( i, nNext - i ) );
                i = nNext;
            }
            else
                nPositionContent = o3tl::toInt32( rBlankWidthString.substr( i ) );
        }
        nPositionContent += nShiftPosition;
        if ( nPositionContent >= 0 )
        {
            rContent.remove( nPositionContent, aBlanks.getLength() );
            if ( nPositionContent >= 1 && rContent[ nPositionContent-1 ] == '\"' )
            {
                nPositionContent--;
                rContent.insert( nPositionContent, nChar );
                rContent.insert( nPositionContent, '_' );
            }
            else
            {
                rContent.insert( nPositionContent, '\"' );
                rContent.insert( nPositionContent, nChar );
                rContent.insert( nPositionContent, "\"_" );
                nShiftPosition += 2;
            }
            // rContent length was modified: remove blanks, add "_x"
            nShiftPosition += 2 - aBlanks.getLength();
        }
    }
    // remove empty string at the end of rContent
    if ( std::u16string_view( rContent ).substr( rContent.getLength() - 2 ) == u"\"\"" )
    {
        sal_Int32 nLen = rContent.getLength();
        if ( nLen >= 3 && rContent[ nLen-3 ] != '\\' )
            rContent.truncate( nLen - 2 );
    }
}
}

void SvXMLNumFmtElementContext::AddEmbeddedElement( sal_Int32 nFormatPos, std::u16string_view rContentEmbedded, std::u16string_view rBlankWidthString )
{
    if ( rContentEmbedded.empty() )
        return;
    OUStringBuffer aContentEmbedded( rContentEmbedded );
    //  #107805# always quote embedded strings - even space would otherwise
    //  be recognized as thousands separator in French.
    aContentEmbedded.insert( 0, '"' );
    aContentEmbedded.append( '"' );
    if ( !rBlankWidthString.empty() )
        lcl_InsertBlankWidthChars( rBlankWidthString, aContentEmbedded );

    auto iterPair = aNumInfo.m_EmbeddedElements.emplace( nFormatPos, aContentEmbedded.toString() );
    if (!iterPair.second)
    {
        // there's already an element at this position - append text to existing element
        if ( iterPair.first->second.endsWith( "\"" ) && aContentEmbedded[ 0 ] == '"' )
        {   // remove double quote
            iterPair.first->second = OUString::Concat( iterPair.first->second.subView( 0, iterPair.first->second.getLength() - 1 ) )
                                + aContentEmbedded.subView( 1, aContentEmbedded.getLength() - 1 );
        }
        else
            iterPair.first->second += aContentEmbedded;
    }
}

void SvXMLNumFmtElementContext::endFastElement(sal_Int32 )
{
    bool bEffLong = bLong;
    switch (nType)
    {
        case SvXMLStyleTokens::Text:
            if ( rParent.HasLongDoW() &&
                 std::u16string_view(aContent) == rParent.GetLocaleData().getLongDateDayOfWeekSep() )
            {
                //  skip separator constant after long day of week
                //  (NF_KEY_NNNN contains the separator)

                if ( rParent.ReplaceNfKeyword( NF_KEY_NNN, NF_KEY_NNNN ) )
                {
                    aContent.truncate();
                }

                rParent.SetHasLongDoW( false );     // only once
            }
            if ( !aContent.isEmpty() )
            {
                lcl_EnquoteIfNecessary( aContent, rParent );
                if ( !sBlankWidthString.isEmpty() )
                {
                    lcl_InsertBlankWidthChars( sBlankWidthString, aContent );
                    sBlankWidthString = "";
                }
                rParent.AddToCode( aContent );
                aContent.setLength(0);
            }
            else
            {
                // Quoted empty text may be significant to separate.
                aContent.append("\"\"");
                rParent.AddToCode( aContent );
                aContent.setLength(0);
                rParent.SetHasTrailingEmptyText(true);  // *after* AddToCode()
            }
            break;

        case SvXMLStyleTokens::Number:
            rParent.AddNumber( aNumInfo );
            break;

        case SvXMLStyleTokens::CurrencySymbol:
            rParent.AddCurrency( aContent.makeStringAndClear(), nElementLang );
            break;

        case SvXMLStyleTokens::TextContent:
            rParent.AddToCode( '@');
            break;
        case SvXMLStyleTokens::FillCharacter:
            if ( !aContent.isEmpty() )
            {
                rParent.AddToCode( '*' );
                rParent.AddToCode( aContent[0] );
            }
            break;
        case SvXMLStyleTokens::Boolean:
            rParent.AddNfKeyword( NF_KEY_BOOLEAN );
            break;

        case SvXMLStyleTokens::Day:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet

            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_DD : NF_KEY_D ) );
            break;
        case SvXMLStyleTokens::Month:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet

            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bTextual
                    ? ( bEffLong ? NF_KEY_MMMM : NF_KEY_MMM )
                    : ( bEffLong ? NF_KEY_MM : NF_KEY_M ) ) );
            break;
        case SvXMLStyleTokens::Year:
//! I18N doesn't provide SYSTEM or extended date information yet
            {
                // Y after G (era) is replaced by E for a secondary calendar.
                // Do not replace for default calendar.
                // Also replace Y by E if we're switching to the secondary
                // calendar of a locale if it is known to implicitly use E.
                rParent.UpdateCalendar( sCalendar);
                const SvXMLNumFormatContext::ImplicitCalendar eCal = rParent.GetImplicitCalendarState();
                if (eCal == SvXMLNumFormatContext::ImplicitCalendar::SECONDARY
                        || eCal == SvXMLNumFormatContext::ImplicitCalendar::SECONDARY_FROM_OTHER)
                {
                    rParent.AddNfKeyword(
                            sal::static_int_cast< sal_uInt16 >(
                                bEffLong ? NF_KEY_EEC : NF_KEY_EC ) );
                }
                else
                {
                    rParent.AddNfKeyword(
                            sal::static_int_cast< sal_uInt16 >(
                                bEffLong ? NF_KEY_YYYY : NF_KEY_YY ) );
                }
            }
            break;
        case SvXMLStyleTokens::Era:
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_GGG : NF_KEY_G ) );
            //  HasEra flag is set
            break;
        case SvXMLStyleTokens::DayOfWeek:
//! I18N doesn't provide SYSTEM or extended date information yet
            {
                // Implicit secondary calendar uses A keyword, default and
                // explicit calendar N keyword.
                rParent.UpdateCalendar( sCalendar);
                const SvXMLNumFormatContext::ImplicitCalendar eCal = rParent.GetImplicitCalendarState();
                if (eCal == SvXMLNumFormatContext::ImplicitCalendar::SECONDARY
                        || eCal == SvXMLNumFormatContext::ImplicitCalendar::SECONDARY_FROM_OTHER)
                {
                    rParent.AddNfKeyword(
                            sal::static_int_cast< sal_uInt16 >(
                                bEffLong ? NF_KEY_AAAA : NF_KEY_AAA ) );
                }
                else
                {
                    rParent.AddNfKeyword(
                            sal::static_int_cast< sal_uInt16 >(
                                bEffLong ? NF_KEY_NNNN : NF_KEY_NN ) );
                }
            }
            break;
        case SvXMLStyleTokens::WeekOfYear:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword( NF_KEY_WW );
            break;
        case SvXMLStyleTokens::Quarter:
            rParent.UpdateCalendar( sCalendar );
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_QQ : NF_KEY_Q ) );
            break;
        case SvXMLStyleTokens::Hours:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_HH : NF_KEY_H ) );
            break;
        case SvXMLStyleTokens::AmPm:
            //! short/long?
            rParent.AddNfKeyword( NF_KEY_AMPM );
            break;
        case SvXMLStyleTokens::Minutes:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_MMI : NF_KEY_MI ) );
            break;
        case SvXMLStyleTokens::Seconds:
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_SS : NF_KEY_S ) );
            if ( aNumInfo.nDecimals > 0 )
            {
                //  manually add the decimal places
                rParent.AddToCode(rParent.GetLocaleData().getNumDecimalSep());
                for (sal_Int32 i=0; i<aNumInfo.nDecimals; i++)
                {
                    rParent.AddToCode( '0');
                }
            }
            break;

        case SvXMLStyleTokens::Fraction:
            {
                if ( aNumInfo.nInteger >= 0 )
                {
                    // add integer part only if min-integer-digits attribute is there
                    aNumInfo.nDecimals = 0;
                    rParent.AddNumber( aNumInfo );      // number without decimals
                    OUStringBuffer sIntegerFractionDelimiter(aNumInfo.aIntegerFractionDelimiter);
                    lcl_EnquoteIfNecessary( sIntegerFractionDelimiter, rParent );
                    rParent.AddToCode( sIntegerFractionDelimiter ); // default is ' '
                }

                //! build string and add at once

                sal_Int32 i;
                for (i=aNumInfo.nMaxNumerDigits; i > 0; i--)
                {
                    if ( i > aNumInfo.nMinNumerDigits )
                        rParent.AddToCode( '#' );
                    else if ( i > aNumInfo.nZerosNumerDigits )
                        rParent.AddToCode( '?' );
                    else
                        rParent.AddToCode( '0' );
                }
                rParent.AddToCode( '/' );
                if ( aNumInfo.nFracDenominator > 0 )
                {
                    rParent.AddToCode(  OUString::number( aNumInfo.nFracDenominator ) );
                }
                else
                {
                    for (i=aNumInfo.nMaxDenomDigits; i > 0 ; i--)
                    {
                        if ( i > aNumInfo.nMinDenomDigits )
                            rParent.AddToCode( '#' );
                        else if ( i > aNumInfo.nZerosDenomDigits )
                            rParent.AddToCode( '?' );
                        else
                            rParent.AddToCode( '0' );
                    }
                }
            }
            break;

        case SvXMLStyleTokens::ScientificNumber:
            {
                // exponential interval for engineering notation
                if( !aNumInfo.bGrouping && aNumInfo.nExpInterval > aNumInfo.nInteger )
                {
                    for (sal_Int32 i=aNumInfo.nInteger; i<aNumInfo.nExpInterval; i++)
                    {
                        rParent.AddToCode( '#' );
                    }
                }
                rParent.AddNumber( aNumInfo );      //  number and exponent
            }
            break;

        default:
            assert(false && "invalid element ID");
    }
}

sal_uInt16 SvXMLNumFmtDefaults::GetDefaultDateFormat( SvXMLDateElementAttributes eDOW,
                SvXMLDateElementAttributes eDay, SvXMLDateElementAttributes eMonth,
                SvXMLDateElementAttributes eYear, SvXMLDateElementAttributes eHours,
                SvXMLDateElementAttributes eMins, SvXMLDateElementAttributes eSecs,
                bool bSystem )
{
    for (const auto & rEntry : aDefaultDateFormats)
    {
        if ( bSystem == rEntry.bSystem &&
            ( eDOW   == rEntry.eDOW   || ( rEntry.eDOW   == XML_DEA_ANY && eDOW   != XML_DEA_NONE ) ) &&
            ( eDay   == rEntry.eDay   || ( rEntry.eDay   == XML_DEA_ANY && eDay   != XML_DEA_NONE ) ) &&
            ( eMonth == rEntry.eMonth || ( rEntry.eMonth == XML_DEA_ANY && eMonth != XML_DEA_NONE ) ) &&
            ( eYear  == rEntry.eYear  || ( rEntry.eYear  == XML_DEA_ANY && eYear  != XML_DEA_NONE ) ) &&
            ( eHours == rEntry.eHours || ( rEntry.eHours == XML_DEA_ANY && eHours != XML_DEA_NONE ) ) &&
            ( eMins  == rEntry.eMins  || ( rEntry.eMins  == XML_DEA_ANY && eMins  != XML_DEA_NONE ) ) &&
            ( eSecs  == rEntry.eSecs  || ( rEntry.eSecs  == XML_DEA_ANY && eSecs  != XML_DEA_NONE ) ) )
        {
            return sal::static_int_cast< sal_uInt16 >(rEntry.eFormat);
        }
    }

    return NF_INDEX_TABLE_ENTRIES;  // invalid
}


//  SvXMLNumFormatContext

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    sal_Int32 /*nElement*/,
                                    SvXMLNumImpData* pNewData, SvXMLStylesTokens nNewType,
                                    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport ),
    m_pData( pNewData ),
    m_pStyles( &rStyles ),
    m_nType( nNewType ),
    m_nKey(-1),
    m_eImplicitCalendar(ImplicitCalendar::DEFAULT),
    m_nFormatLang( LANGUAGE_SYSTEM ),
    m_bAutoOrder( false ),
    m_bFromSystem( false ),
    m_bTruncate( true ),
    m_bAutoDec( false ),
    m_bAutoInt( false ),
    m_bHasExtraText( false ),
    m_bHasTrailingEmptyText( false ),
    m_bHasLongDoW( false ),
    m_bHasDateTime( false ),
    m_bRemoveAfterUse( false ),
    m_eDateDOW( XML_DEA_NONE ),
    m_eDateDay( XML_DEA_NONE ),
    m_eDateMonth( XML_DEA_NONE ),
    m_eDateYear( XML_DEA_NONE ),
    m_eDateHours( XML_DEA_NONE ),
    m_eDateMins( XML_DEA_NONE ),
    m_eDateSecs( XML_DEA_NONE ),
    m_bDateNoDefault( false )
{
    LanguageTagODF aLanguageTagODF;
    css::i18n::NativeNumberXmlAttributes aNatNumAttr;
    OUString aSpellout;
    bool bAttrBool(false);

    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        switch (aIter.getToken())
        {
        //  attributes for a style
            case XML_ELEMENT(STYLE, XML_NAME):
                break;
            case XML_ELEMENT(NUMBER, XML_RFC_LANGUAGE_TAG):
                aLanguageTagODF.maRfcLanguageTag = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_LANGUAGE):
                aLanguageTagODF.maLanguage = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_SCRIPT):
                aLanguageTagODF.maScript = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_COUNTRY):
                aLanguageTagODF.maCountry = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_TITLE):
                m_sFormatTitle = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_AUTOMATIC_ORDER):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    m_bAutoOrder = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_FORMAT_SOURCE):
                SvXMLUnitConverter::convertEnum( m_bFromSystem, aIter.toView(), aFormatSourceMap );
                break;
            case XML_ELEMENT(NUMBER, XML_TRUNCATE_ON_OVERFLOW):
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    m_bTruncate = bAttrBool;
                break;
            case XML_ELEMENT(STYLE, XML_VOLATILE):
                //  volatile formats can be removed after importing
                //  if not used in other styles
                if (::sax::Converter::convertBool( bAttrBool, aIter.toView() ))
                    m_bRemoveAfterUse = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_FORMAT):
                aNatNumAttr.Format = aIter.toString();
                break;
            case XML_ELEMENT(LO_EXT, XML_TRANSLITERATION_SPELLOUT):
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_SPELLOUT):
                aSpellout = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_LANGUAGE):
                aNatNumAttr.Locale.Language = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_COUNTRY):
                aNatNumAttr.Locale.Country = aIter.toString();
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_STYLE):
                aNatNumAttr.Style = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if (!aLanguageTagODF.isEmpty())
    {
        m_nFormatLang = aLanguageTagODF.getLanguageTag().getLanguageType( false);
        if ( m_nFormatLang == LANGUAGE_DONTKNOW )
            m_nFormatLang = LANGUAGE_SYSTEM;          //! error handling for unknown locales?
    }

    if (aNatNumAttr.Format.isEmpty() && aSpellout.isEmpty())
        return;

    LanguageTag aLanguageTag( OUString(), aNatNumAttr.Locale.Language,
                std::u16string_view(), aNatNumAttr.Locale.Country);
    aNatNumAttr.Locale = aLanguageTag.getLocale( false);

    // NatNum12 spell out formula (cardinal, ordinal, ordinal-feminine etc.)
    if ( !aSpellout.isEmpty() )
    {
        m_aFormatCode.append( "[NatNum12 " );
        m_aFormatCode.append( aSpellout );
    } else {
        SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
        if ( !pFormatter ) return;

        sal_Int32 nNatNum = pFormatter->GetNatNum()->convertFromXmlAttributes( aNatNumAttr );
        m_aFormatCode.append( "[NatNum" );
        m_aFormatCode.append( nNatNum );
    }

    LanguageType eLang = aLanguageTag.getLanguageType( false );
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;            //! error handling for unknown locales?
    if ( eLang != m_nFormatLang && eLang != LANGUAGE_SYSTEM )
    {
        m_aFormatCode.append( "][$-" );
        // language code in upper hex:
        m_aFormatCode.append(OUString::number(static_cast<sal_uInt16>(eLang), 16).toAsciiUpperCase());
    }
    m_aFormatCode.append( ']' );
}

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    const OUString& rName,
                                    const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/,
                                    const sal_Int32 nTempKey, LanguageType nLang,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, XmlStyleFamily::DATA_STYLE ),
    m_pData( nullptr ),
    m_pStyles( &rStyles ),
    m_nType( SvXMLStylesTokens::NUMBER_STYLE ),
    m_nKey(nTempKey),
    m_eImplicitCalendar(ImplicitCalendar::DEFAULT),
    m_nFormatLang( nLang ),
    m_bAutoOrder( false ),
    m_bFromSystem( false ),
    m_bTruncate( true ),
    m_bAutoDec( false ),
    m_bAutoInt( false ),
    m_bHasExtraText( false ),
    m_bHasTrailingEmptyText( false ),
    m_bHasLongDoW( false ),
    m_bHasDateTime( false ),
    m_bRemoveAfterUse( false ),
    m_eDateDOW( XML_DEA_NONE ),
    m_eDateDay( XML_DEA_NONE ),
    m_eDateMonth( XML_DEA_NONE ),
    m_eDateYear( XML_DEA_NONE ),
    m_eDateHours( XML_DEA_NONE ),
    m_eDateMins( XML_DEA_NONE ),
    m_eDateSecs( XML_DEA_NONE ),
    m_bDateNoDefault( false )
{
    SetAttribute(XML_ELEMENT(STYLE, XML_NAME), rName);
}

SvXMLNumFormatContext::~SvXMLNumFormatContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvXMLNumFormatContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT(LO_EXT, XML_TEXT):
        case XML_ELEMENT(NUMBER, XML_TEXT):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Text, xAttrList );
            break;
        case XML_ELEMENT(LO_EXT, XML_FILL_CHARACTER):
        case XML_ELEMENT(NUMBER, XML_FILL_CHARACTER):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::FillCharacter, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_NUMBER):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Number, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_SCIENTIFIC_NUMBER):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::ScientificNumber, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_FRACTION):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Fraction, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_CURRENCY_SYMBOL):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::CurrencySymbol, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_DAY):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Day, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_MONTH):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Month, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_YEAR):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Year, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_ERA):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Era, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_DAY_OF_WEEK):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::DayOfWeek, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_WEEK_OF_YEAR):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::WeekOfYear, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_QUARTER):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Quarter, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_HOURS):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Hours, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_AM_PM):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::AmPm, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_MINUTES):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Minutes, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_SECONDS):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Seconds, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_BOOLEAN):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::Boolean, xAttrList );
            break;
        case XML_ELEMENT(NUMBER, XML_TEXT_CONTENT):
            pContext = new SvXMLNumFmtElementContext( GetImport(), nElement,
                                                        *this, SvXMLStyleTokens::TextContent, xAttrList );
            break;

        case XML_ELEMENT(STYLE, XML_TEXT_PROPERTIES):
            pContext = new SvXMLNumFmtPropContext( GetImport(), nElement,
                                                        *this, xAttrList );
            break;
        case XML_ELEMENT(STYLE, XML_MAP):
            {
                //  SvXMLNumFmtMapContext::EndElement adds to aMyConditions,
                //  so there's no need for an extra flag
                pContext = new SvXMLNumFmtMapContext( GetImport(), nElement,
                                                            *this, xAttrList );
            }
            break;
    }

    if( !pContext )
    {
        SAL_WARN("xmloff.core", "No context for unknown-element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
        pContext = new SvXMLImportContext(GetImport());
    }

    return pContext;
}

sal_Int32 SvXMLNumFormatContext::GetKey()
{
    if (m_nKey > -1)
    {
        if (m_bRemoveAfterUse)
        {
            //  format is used -> don't remove
            m_bRemoveAfterUse = false;
            if (m_pData)
                m_pData->SetUsed(m_nKey);

            //  Add to import's list of keys now - CreateAndInsert didn't add
            //  the style if bRemoveAfterUse was set.
            GetImport().AddNumberStyle( m_nKey, GetName() );
        }
        return m_nKey;
    }
    else
    {
        // reset bRemoveAfterUse before CreateAndInsert, so AddKey is called without bRemoveAfterUse set
        m_bRemoveAfterUse = false;
        CreateAndInsert(true);
        return m_nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::PrivateGetKey()
{
    //  used for map elements in CreateAndInsert - don't reset bRemoveAfterUse flag

    if (m_nKey > -1)
        return m_nKey;
    else
    {
        CreateAndInsert(true);
        return m_nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert( css::uno::Reference< css::util::XNumberFormatsSupplier > const & xFormatsSupplier )
{
    if (m_nKey <= -1)
    {
        SvNumberFormatter* pFormatter = nullptr;
        SvNumberFormatsSupplierObj* pObj =
                        comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>( xFormatsSupplier );
        if (pObj)
            pFormatter = pObj->GetNumberFormatter();

        if ( pFormatter )
            return CreateAndInsert( pFormatter );
        else
            return -1;
    }
    else
        return m_nKey;
}

void SvXMLNumFormatContext::CreateAndInsert(bool /*bOverwrite*/)
{
    if (m_nKey <= -1)
        CreateAndInsert(m_pData->GetNumberFormatter());
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert(SvNumberFormatter* pFormatter)
{
    if (!pFormatter)
    {
        OSL_FAIL("no number formatter");
        return -1;
    }

    sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

    for (size_t i = 0; i < m_aMyConditions.size(); i++)
    {
        SvXMLNumFormatContext* pStyle = const_cast<SvXMLNumFormatContext*>( static_cast<const SvXMLNumFormatContext *>(m_pStyles->FindStyleChildContext(
            XmlStyleFamily::DATA_STYLE, m_aMyConditions[i].sMapName)));
        if (this == pStyle)
        {
            SAL_INFO("xmloff.style", "invalid style:map references containing style");
            pStyle = nullptr;
        }
        if (pStyle)
        {
            if (pStyle->PrivateGetKey() > -1)     // don't reset pStyle's bRemoveAfterUse flag
                AddCondition(i);
        }
    }

    sal_Int32 nBufLen;
    if ( m_aFormatCode.isEmpty() )
    {
        //  insert empty format as empty string (with quotes)
        //  #93901# this check has to be done before inserting the conditions
        m_aFormatCode.append("\"\"");    // ""
    }
    else if (m_bHasTrailingEmptyText && (nBufLen = m_aFormatCode.getLength()) >= 3)
    {
        // Remove a trailing empty text. Earlier this may had been written to
        // file, like in "General;General" written with elements for
        // 'General"";General""' (whyever); when reading, empty text was
        // ignored, which it isn't anymore, so get rid of those.
        if (m_aFormatCode[nBufLen-1] == '"' && m_aFormatCode[nBufLen-2] == '"')
            m_aFormatCode.truncate( nBufLen - 2);
    }

    m_aFormatCode.insert( 0, m_aConditions );
    m_aConditions.setLength(0);
    OUString sFormat = m_aFormatCode.makeStringAndClear();

    //  test special cases

    if ( m_bAutoDec )         // automatic decimal places
    {
        //  #99391# adjust only if the format contains no text elements, no conditions
        //  and no color definition (detected by the '[' at the start)

        if ( m_nType == SvXMLStylesTokens::NUMBER_STYLE && !m_bHasExtraText &&
                m_aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetStandardIndex( m_nFormatLang );
    }
    if ( m_bAutoInt )         // automatic integer digits
    {
        //! only if two decimal places was set?

        if ( m_nType == SvXMLStylesTokens::NUMBER_STYLE && !m_bHasExtraText &&
                m_aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, m_nFormatLang );
    }

    if ( m_nType == SvXMLStylesTokens::BOOLEAN_STYLE && !m_bHasExtraText &&
            m_aMyConditions.empty() && sFormat.toChar() != '[' )
        nIndex = pFormatter->GetFormatIndex( NF_BOOLEAN, m_nFormatLang );

    //  check for default date formats
    if ( m_nType == SvXMLStylesTokens::DATE_STYLE && m_bAutoOrder && !m_bDateNoDefault )
    {
        NfIndexTableOffset eFormat = static_cast<NfIndexTableOffset>(SvXMLNumFmtDefaults::GetDefaultDateFormat(
            m_eDateDOW, m_eDateDay, m_eDateMonth, m_eDateYear,
            m_eDateHours, m_eDateMins, m_eDateSecs, m_bFromSystem ));
        if ( eFormat < NF_INDEX_TABLE_RESERVED_START )
        {
            //  #109651# if a date format has the automatic-order attribute and
            //  contains exactly the elements of one of the default date formats,
            //  use that default format, with the element order and separators
            //  from the current locale settings

            nIndex = pFormatter->GetFormatIndex( eFormat, m_nFormatLang );
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND && !sFormat.isEmpty() )
    {
        //  insert by format string

        OUString aFormatStr( sFormat );
        nIndex = pFormatter->GetEntryKey( aFormatStr, m_nFormatLang );
        if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32  nErrPos = 0;
            SvNumFormatType l_nType = SvNumFormatType::ALL;
            bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, l_nType, nIndex, m_nFormatLang );
            if ( !bOk && nErrPos == 0 && aFormatStr != sFormat )
            {
                //  if the string was modified by PutEntry, look for an existing format
                //  with the modified string
                nIndex = pFormatter->GetEntryKey( aFormatStr, m_nFormatLang );
                if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
                    bOk = true;
            }
            if (!bOk)
                nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }

//! I18N doesn't provide SYSTEM or extended date information yet
    if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !m_bAutoOrder )
    {
        //  use fixed-order formats instead of SYS... if bAutoOrder is false
        //  (only if the format strings are equal for the locale)

        NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
        if ( eOffset == NF_DATE_SYS_DMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMYYYY, m_nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
        else if ( eOffset == NF_DATE_SYS_DMMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMMYYYY, m_nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
    }

    if ((nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND) && !m_sFormatTitle.isEmpty())
    {
        SvNumberformat* pFormat = const_cast<SvNumberformat*>(pFormatter->GetEntry( nIndex ));
        if (pFormat)
        {
            pFormat->SetComment(m_sFormatTitle);
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        OSL_FAIL("invalid number format");
        nIndex = pFormatter->GetStandardIndex( m_nFormatLang );
    }

    m_pData->AddKey( nIndex, GetName(), m_bRemoveAfterUse );
    m_nKey = nIndex;

    //  Add to import's list of keys (shared between styles and content import)
    //  only if not volatile - formats are removed from NumberFormatter at the
    //  end of each import (in SvXMLNumFmtHelper dtor).
    //  If bRemoveAfterUse is reset later in GetKey, AddNumberStyle is called there.

    if (!m_bRemoveAfterUse)
        GetImport().AddNumberStyle( m_nKey, GetName() );

    return m_nKey;
}

const LocaleDataWrapper& SvXMLNumFormatContext::GetLocaleData() const
{
    return m_pData->GetLocaleData( m_nFormatLang );
}

void SvXMLNumFormatContext::AddToCode( sal_Unicode c )
{
    m_aFormatCode.append( c );
    m_bHasExtraText = true;
}

void SvXMLNumFormatContext::AddToCode( std::u16string_view rString )
{
    m_aFormatCode.append( rString );
    m_bHasExtraText = true;
    m_bHasTrailingEmptyText = false;  // is set by caller again if so
}

void SvXMLNumFormatContext::AddNumber( const SvXMLNumberInfo& rInfo )
{
    SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    //  store special conditions
    m_bAutoDec = ( rInfo.nDecimals < 0 );
    m_bAutoInt = ( rInfo.nInteger < 0 );

    sal_uInt16 nPrec = 0;
    sal_uInt16 nLeading = 0;
    if ( rInfo.nDecimals >= 0 )                     //  < 0 : Default
        nPrec = static_cast<sal_uInt16>(rInfo.nDecimals);
    if ( rInfo.nInteger >= 0 )                      //  < 0 : Default
        nLeading = static_cast<sal_uInt16>(rInfo.nInteger);

    if ( m_bAutoDec )
    {
        if ( m_nType == SvXMLStylesTokens::CURRENCY_STYLE )
        {
            //  for currency formats, "automatic decimals" is used for the automatic
            //  currency format with (fixed) decimals from the locale settings

            const LocaleDataWrapper& rLoc = m_pData->GetLocaleData( m_nFormatLang );
            nPrec = rLoc.getCurrDigits();
        }
        else
        {
            //  for other types, "automatic decimals" means dynamic determination of
            //  decimals, as achieved with the "general" keyword

            m_aFormatCode.append( pFormatter->GetStandardName( m_nFormatLang ) );
            return;
        }
    }
    if ( m_bAutoInt )
    {
        //!...
    }

    sal_uInt16 nGenPrec = nPrec;
    if ( rInfo.nMinDecimalDigits >= 0 )
        nGenPrec = rInfo.nMinDecimalDigits;
    if ( rInfo.bDecReplace )
        nGenPrec = 0;               // generate format without decimals...

    bool bGrouping = rInfo.bGrouping;
    size_t const nEmbeddedCount = rInfo.m_EmbeddedElements.size();
    if ( nEmbeddedCount && rInfo.m_EmbeddedElements.rbegin()->first > 0 )
        bGrouping = false;      // grouping and embedded characters in integer part can't be used together

    sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( m_nFormatLang );
    OUStringBuffer aNumStr(pFormatter->GenerateFormat( nStdIndex, m_nFormatLang,
                                                         bGrouping, false, nGenPrec, nLeading ));

    if ( rInfo.nExpDigits >= 0 && nLeading == 0 && !bGrouping && nEmbeddedCount == 0 )
    {
        // #i43959# For scientific numbers, "#" in the integer part forces a digit,
        // so it has to be removed if nLeading is 0 (".00E+0", not "#.00E+0").

        aNumStr.stripStart('#');
    }

    if ( rInfo.nBlankInteger > 0 )
    {
        // Replace nBlankInteger '0' by '?'
        sal_Int32 nIndex = 0;
        sal_Int32 nBlanks = rInfo.nBlankInteger;
        sal_Int32 nIntegerEnd = aNumStr.indexOf( pFormatter->GetNumDecimalSep() );
        if ( nIntegerEnd < 0 )
            nIntegerEnd = aNumStr.getLength();
        while ( nIndex < nIntegerEnd && nBlanks > 0 )
        {
            if ( aNumStr[nIndex] == '0' )
            {
                aNumStr[nIndex] = '?';
                nBlanks--;
            }
            nIndex++;
        }
    }

    if ( bGrouping && rInfo.nExpInterval > rInfo.nInteger )
    {
        sal_Int32 nIndex = 0;
        sal_Int32 nDigits = rInfo.nInteger;
        sal_Int32 nIntegerEnd = aNumStr.indexOf( pFormatter->GetNumDecimalSep() );
        if ( nIntegerEnd < 0 )
            nIntegerEnd = aNumStr.getLength();
        while ( nIndex >= 0 && nIndex < nIntegerEnd )
        {
            if ( ( nIndex = aNumStr.indexOf( '#', nIndex ) ) >= 0 )
            {
                nDigits ++;
                nIndex ++;
            }
            else
                nIndex = -1;
        }
        while ( rInfo.nExpInterval > nDigits )
        {
            nDigits++;
            aNumStr.insert( 0, '#' );
        }
    }

    if ( ( rInfo.bDecReplace || rInfo.nMinDecimalDigits < rInfo.nDecimals ) && nPrec )     // add decimal replacement (dashes)
    {
        //  add dashes for explicit decimal replacement, # or ? for variable decimals
        sal_Unicode cAdd = rInfo.bDecReplace ? '-' : ( rInfo.bDecAlign ? '?': '#' );

        if ( rInfo.nMinDecimalDigits == 0 )
            aNumStr.append( m_pData->GetLocaleData( m_nFormatLang ).getNumDecimalSep() );
        for ( sal_uInt16 i=rInfo.nMinDecimalDigits; i<nPrec; i++)
            aNumStr.append( cAdd );
    }

    // Scientific number
    sal_Int32 nExpPos = -1;
    if ( rInfo.nExpDigits > 0 )
    {
        nExpPos = aNumStr.getLength();
        aNumStr.append( rInfo.bExponentLowercase ? u"e" : u"E" );
                                // exponent sign is required with embedded text in exponent
        if ( rInfo.bExpSign || ( nEmbeddedCount && ( rInfo.nDecimals + 1 < -rInfo.m_EmbeddedElements.begin()->first ) ) )
        {
            aNumStr.append( u"+" );
        }
        for (sal_Int32 i=0; i<rInfo.nExpDigits; i++)
        {
            if ( i < rInfo.nBlankExp )
                aNumStr.append( '?' );
            else
                aNumStr.append( '0' );
        }
    }

    if ( nEmbeddedCount )
    {
        //  insert embedded strings into number string
        //  support integer (position >=0) and decimal (position <0) part
        //  nZeroPos is the string position where format position 0 is inserted

        sal_Int32 nZeroPos = aNumStr.indexOf( m_pData->GetLocaleData( m_nFormatLang ).getNumDecimalSep() );
        if ( nZeroPos < 0 )
        {
            nZeroPos = aNumStr.getLength();
        }

        // m_EmbeddedElements is sorted - last entry has the largest position (leftmost)
        sal_Int32 const nLastFormatPos = rInfo.m_EmbeddedElements.rbegin()->first;
        if ( nLastFormatPos >= nZeroPos )
        {
            //  add '#' characters so all embedded texts are really embedded in digits
            //  (there always has to be a digit before the leftmost embedded text)

            sal_Int32 nAddCount = nLastFormatPos + 1 - nZeroPos;
            for(sal_Int32 index = 0; index < nAddCount; ++index)
            {
                aNumStr.insert(0, '#');
            }
            nZeroPos = nZeroPos + nAddCount;
            if ( nExpPos > 0 )
                nExpPos = nExpPos + nAddCount;
        }

        // m_EmbeddedElements is sorted with ascending positions - loop is from right to left
        for (auto const& it : rInfo.m_EmbeddedElements)
        {
            sal_Int32 const nFormatPos = it.first;
            sal_Int32 nInsertPos = nZeroPos - nFormatPos;
            if ( nExpPos > 0 && nInsertPos > nExpPos )
                nInsertPos ++;
            if ( 0 <= nInsertPos && nInsertPos <= aNumStr.getLength() )
            {
                aNumStr.insert( nInsertPos, it.second );
            }
        }
    }

    m_aFormatCode.append( aNumStr );

    //  add extra thousands separators for display factor

    if (rInfo.fDisplayFactor == 1.0 || rInfo.fDisplayFactor <= 0.0)
        return;

    //  test for 1.0 is just for optimization - nSepCount would be 0

    //  one separator for each factor of 1000
    sal_Int32 nSepCount = static_cast<sal_Int32>(::rtl::math::round( log10(rInfo.fDisplayFactor) / 3.0 ));
    if ( nSepCount > 0 )
    {
        OUString aSep = m_pData->GetLocaleData( m_nFormatLang ).getNumThousandSep();
        for ( sal_Int32 i=0; i<nSepCount; i++ )
            m_aFormatCode.append( aSep );
    }
}

void SvXMLNumFormatContext::AddCurrency( const OUString& rContent, LanguageType nLang )
{
    bool bAutomatic = false;
    OUString aSymbol = rContent;
    if ( aSymbol.isEmpty())
    {
        SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
        if ( pFormatter )
        {
            pFormatter->ChangeIntl( m_nFormatLang );
            OUString sCurString, sDummy;
            pFormatter->GetCompatibilityCurrency( sCurString, sDummy );
            aSymbol = sCurString;

            bAutomatic = true;
        }
    }
    else if ( nLang == LANGUAGE_SYSTEM && aSymbol == "CCC" )
    {
        //  "CCC" is used for automatic long symbol
        bAutomatic = true;
    }

    if ( bAutomatic )
    {
        //  remove unnecessary quotes before automatic symbol (formats like "-(0DM)")
        //  otherwise the currency symbol isn't recognized (#94048#)

        sal_Int32 nLength = m_aFormatCode.getLength();
        if ( nLength > 1 && m_aFormatCode[nLength - 1] == '"' )
        {
            //  find start of quoted string
            //  When SvXMLNumFmtElementContext::EndElement creates escaped quotes,
            //  they must be handled here, too.

            sal_Int32 nFirst = nLength - 2;
            while ( nFirst >= 0 && m_aFormatCode[nFirst] != '"' )
                --nFirst;
            if ( nFirst >= 0 )
            {
                //  remove both quotes from aFormatCode
                OUString aOld = m_aFormatCode.makeStringAndClear();
                if ( nFirst > 0 )
                    m_aFormatCode.append( aOld.subView( 0, nFirst ) );
                if ( nLength > nFirst + 2 )
                    m_aFormatCode.append( aOld.subView( nFirst + 1, nLength - nFirst - 2 ) );
            }
        }
    }

    if (!bAutomatic)
        m_aFormatCode.append( "[$" );            // intro for "new" currency symbols

    m_aFormatCode.append( aSymbol );

    if (!bAutomatic)
    {
        if ( nLang != LANGUAGE_SYSTEM )
        {
            //  '-' sign and language code in hex:
            m_aFormatCode.append("-" + OUString(OUString::number(sal_uInt16(nLang), 16)).toAsciiUpperCase());
        }

        m_aFormatCode.append( ']' );    // end of "new" currency symbol
    }
}

void SvXMLNumFormatContext::AddNfKeyword( sal_uInt16 nIndex )
{
    SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    if ( nIndex == NF_KEY_NNNN )
    {
        nIndex = NF_KEY_NNN;
        m_bHasLongDoW = true;         // to remove string constant with separator
    }

    OUString sKeyword = pFormatter->GetKeyword( m_nFormatLang, nIndex );

    if ( nIndex == NF_KEY_H  || nIndex == NF_KEY_HH  ||
         nIndex == NF_KEY_MI || nIndex == NF_KEY_MMI ||
         nIndex == NF_KEY_S  || nIndex == NF_KEY_SS )
    {
        if ( !m_bTruncate && !m_bHasDateTime )
        {
            //  with truncate-on-overflow = false, add "[]" to first time part
            m_aFormatCode.append("[" + sKeyword + "]");
        }
        else
        {
            m_aFormatCode.append( sKeyword );
        }
        m_bHasDateTime = true;
    }
    else
    {
        m_aFormatCode.append( sKeyword );
    }
    //  collect the date elements that the format contains, to recognize default date formats
    switch ( nIndex )
    {
        case NF_KEY_NN:     m_eDateDOW = XML_DEA_SHORT;       break;
        case NF_KEY_NNN:
        case NF_KEY_NNNN:   m_eDateDOW = XML_DEA_LONG;        break;
        case NF_KEY_D:      m_eDateDay = XML_DEA_SHORT;       break;
        case NF_KEY_DD:     m_eDateDay = XML_DEA_LONG;        break;
        case NF_KEY_M:      m_eDateMonth = XML_DEA_SHORT;     break;
        case NF_KEY_MM:     m_eDateMonth = XML_DEA_LONG;      break;
        case NF_KEY_MMM:    m_eDateMonth = XML_DEA_TEXTSHORT; break;
        case NF_KEY_MMMM:   m_eDateMonth = XML_DEA_TEXTLONG;  break;
        case NF_KEY_YY:     m_eDateYear = XML_DEA_SHORT;      break;
        case NF_KEY_YYYY:   m_eDateYear = XML_DEA_LONG;       break;
        case NF_KEY_H:      m_eDateHours = XML_DEA_SHORT;     break;
        case NF_KEY_HH:     m_eDateHours = XML_DEA_LONG;      break;
        case NF_KEY_MI:     m_eDateMins = XML_DEA_SHORT;      break;
        case NF_KEY_MMI:    m_eDateMins = XML_DEA_LONG;       break;
        case NF_KEY_S:      m_eDateSecs = XML_DEA_SHORT;      break;
        case NF_KEY_SS:     m_eDateSecs = XML_DEA_LONG;       break;
        case NF_KEY_AP:
        case NF_KEY_AMPM:   break;          // AM/PM may or may not be in date/time formats -> ignore by itself
        default:
            m_bDateNoDefault = true;      // any other element -> no default format
    }
}

static bool lcl_IsAtEnd( OUStringBuffer& rBuffer, std::u16string_view rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    sal_Int32 nTokLen = rToken.size();

    if ( nTokLen > nBufLen )
        return false;

    sal_Int32 nStartPos = nBufLen - nTokLen;
    for ( sal_Int32 nTokPos = 0; nTokPos < nTokLen; nTokPos++ )
        if ( rToken[ nTokPos ] != rBuffer[nStartPos + nTokPos] )
            return false;

    return true;
}

bool SvXMLNumFormatContext::ReplaceNfKeyword( sal_uInt16 nOld, sal_uInt16 nNew )
{
    //  replaces one keyword with another if it is found at the end of the code

    SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
    if (!pFormatter)
        return false;

    OUString sOldStr = pFormatter->GetKeyword( m_nFormatLang, nOld );
    if ( lcl_IsAtEnd( m_aFormatCode, sOldStr ) )
    {
        // remove old keyword
        m_aFormatCode.setLength( m_aFormatCode.getLength() - sOldStr.getLength() );

        // add new keyword
        OUString sNewStr = pFormatter->GetKeyword( m_nFormatLang, nNew );
        m_aFormatCode.append( sNewStr );

        return true;    // changed
    }
    return false;       // not found
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex )
{
    OUString rApplyName = m_aMyConditions[nIndex].sMapName;
    OUString rCondition = m_aMyConditions[nIndex].sCondition;
    SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
    sal_uInt32 l_nKey = m_pData->GetKeyForName( rApplyName );

    OUString sRealCond;
    if ( !(pFormatter && l_nKey != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            rCondition.startsWith("value()", &sRealCond)) )
        return;

    //! test for valid conditions
    //! test for default conditions

    bool bDefaultCond = false;

    //! collect all conditions first and adjust default to >=0, >0 or <0 depending on count
    //! allow blanks in conditions
    if ( m_aConditions.isEmpty() && m_aMyConditions.size() == 1 && sRealCond == ">=0" )
        bDefaultCond = true;

    if ( m_nType == SvXMLStylesTokens::TEXT_STYLE && static_cast<size_t>(nIndex) == m_aMyConditions.size() - 1 )
    {
        //  The last condition in a number format with a text part can only
        //  be "all other numbers", the condition string must be empty.
        bDefaultCond = true;
    }

    if (!bDefaultCond)
    {
        // Convert != to <>
        sal_Int32 nPos = sRealCond.indexOf( "!=" );
        if ( nPos >= 0 )
        {
            sRealCond = sRealCond.replaceAt( nPos, 2, u"<>" );
        }

        nPos = sRealCond.indexOf( '.' );
        if ( nPos >= 0 )
        {
            // #i8026# #103991# localize decimal separator
            const OUString& rDecSep = GetLocaleData().getNumDecimalSep();
            if ( rDecSep.getLength() > 1 || rDecSep[0] != '.' )
            {
                sRealCond = sRealCond.replaceAt( nPos, 1, rDecSep );
            }
        }
        m_aConditions.append("[" + sRealCond + "]");
    }

    const SvNumberformat* pFormat = pFormatter->GetEntry(l_nKey);
    if ( pFormat )
        m_aConditions.append( pFormat->GetFormatstring() );

    m_aConditions.append( ';' );
}

void SvXMLNumFormatContext::AddCondition( const OUString& rCondition, const OUString& rApplyName )
{
    MyCondition aCondition;
    aCondition.sCondition = rCondition;
    aCondition.sMapName = rApplyName;
    m_aMyConditions.push_back(aCondition);
}

void SvXMLNumFormatContext::AddColor( Color const nColor )
{
    SvNumberFormatter* pFormatter = m_pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    OUStringBuffer aColName;
    for ( sal_uInt16 i=0; i<XML_NUMF_COLORCOUNT; i++ )
        if (nColor == aNumFmtStdColors[i])
        {
            aColName = pFormatter->GetKeyword( m_nFormatLang, sal::static_int_cast< sal_uInt16 >(NF_KEY_FIRSTCOLOR + i) );
            break;
        }

    if ( !aColName.isEmpty() )
    {
        aColName.insert( 0, '[' );
        aColName.append( ']' );
        m_aFormatCode.insert( 0, aColName );
    }
}

void SvXMLNumFormatContext::UpdateCalendar( const OUString& rNewCalendar )
{
    if ( rNewCalendar == m_sCalendar )
        return;

    if (rNewCalendar.isEmpty() || rNewCalendar == m_aImplicitCalendar[0])
    {
        m_eImplicitCalendar = (m_eImplicitCalendar == ImplicitCalendar::OTHER ?
                ImplicitCalendar::DEFAULT_FROM_OTHER : ImplicitCalendar::DEFAULT);
    }
    else if (m_aImplicitCalendar[0].isEmpty() && rNewCalendar == GetLocaleData().getDefaultCalendar()->Name)
    {
        m_eImplicitCalendar = (m_eImplicitCalendar == ImplicitCalendar::OTHER ?
                ImplicitCalendar::DEFAULT_FROM_OTHER : ImplicitCalendar::DEFAULT);
        m_aImplicitCalendar[0] = rNewCalendar;
    }
    else if (rNewCalendar == m_aImplicitCalendar[1])
    {
        m_eImplicitCalendar = (m_eImplicitCalendar == ImplicitCalendar::OTHER ?
                ImplicitCalendar::SECONDARY_FROM_OTHER : ImplicitCalendar::SECONDARY);
    }
    else if (m_aImplicitCalendar[1].isEmpty() && GetLocaleData().doesSecondaryCalendarUseEC( rNewCalendar))
    {
        m_eImplicitCalendar = (m_eImplicitCalendar == ImplicitCalendar::OTHER ?
                ImplicitCalendar::SECONDARY_FROM_OTHER : ImplicitCalendar::SECONDARY);
        m_aImplicitCalendar[1] = rNewCalendar;
    }
    else
    {
        m_eImplicitCalendar = ImplicitCalendar::OTHER;
    }

    if (m_eImplicitCalendar != ImplicitCalendar::DEFAULT && m_eImplicitCalendar != ImplicitCalendar::SECONDARY)
    {
        // A switch from empty default calendar to named default calendar or
        // vice versa is not a switch.
        bool bSameDefault = false;
        if (m_sCalendar.isEmpty() || rNewCalendar.isEmpty())
        {
            // As both are not equal, only one can be empty here, the other
            // can not.
            const OUString& rDefaultCalendar = GetLocaleData().getDefaultCalendar()->Name;
            // So if one is the named default calendar the other is the
            // empty default calendar.
            bSameDefault = (rNewCalendar == rDefaultCalendar || m_sCalendar == rDefaultCalendar);
        }
        if (!bSameDefault)
        {
            m_aFormatCode.append( "[~" );   // intro for calendar code
            if (rNewCalendar.isEmpty())
            {
                // Empty calendar name here means switching to default calendar
                // from a different calendar. Needs to be explicitly stated in
                // format code.
                m_aFormatCode.append( GetLocaleData().getDefaultCalendar()->Name );
            }
            else
            {
                m_aFormatCode.append( rNewCalendar );
            }
            m_aFormatCode.append( ']' );    // end of calendar code
        }
    }
    m_sCalendar = rNewCalendar;
}

bool SvXMLNumFormatContext::IsSystemLanguage() const
{
    return m_nFormatLang == LANGUAGE_SYSTEM;
}


//  SvXMLNumFmtHelper


SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    const uno::Reference<util::XNumberFormatsSupplier>& rSupp,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    SvNumberFormatter* pFormatter = nullptr;
    SvNumberFormatsSupplierObj* pObj =
                    comphelper::getFromUnoTunnel<SvNumberFormatsSupplierObj>( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    m_pData = std::make_unique<SvXMLNumImpData>( pFormatter, rxContext );
}

SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    SvNumberFormatter* pNumberFormatter,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    m_pData = std::make_unique<SvXMLNumImpData>( pNumberFormatter, rxContext );
}

SvXMLNumFmtHelper::~SvXMLNumFmtHelper()
{
    //  remove temporary (volatile) formats from NumberFormatter
    m_pData->RemoveVolatileFormats();
}


SvXMLStyleContext*  SvXMLNumFmtHelper::CreateChildContext( SvXMLImport& rImport,
                sal_Int32 nElement,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList,
                SvXMLStylesContext& rStyles )
{
    SvXMLStylesTokens nStyleToken;
    switch (nElement)
    {
        case XML_ELEMENT(NUMBER, XML_NUMBER_STYLE):
            nStyleToken = SvXMLStylesTokens::NUMBER_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_CURRENCY_STYLE):
            nStyleToken = SvXMLStylesTokens::CURRENCY_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_PERCENTAGE_STYLE):
            nStyleToken = SvXMLStylesTokens::PERCENTAGE_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_DATE_STYLE):
            nStyleToken = SvXMLStylesTokens::DATE_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_TIME_STYLE):
            nStyleToken = SvXMLStylesTokens::TIME_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_BOOLEAN_STYLE):
            nStyleToken = SvXMLStylesTokens::BOOLEAN_STYLE;
            break;
        case XML_ELEMENT(NUMBER, XML_TEXT_STYLE):
            nStyleToken = SvXMLStylesTokens::TEXT_STYLE;
            break;
        default:
            // return NULL if not a data style, caller must handle other elements
            return nullptr;
    }
    return new SvXMLNumFormatContext( rImport, nElement,
                                      m_pData.get(), nStyleToken, xAttrList, rStyles );
}

LanguageType SvXMLNumFmtHelper::GetLanguageForKey(sal_Int32 nKey) const
{
    if (m_pData->GetNumberFormatter())
    {
        const SvNumberformat* pEntry = m_pData->GetNumberFormatter()->GetEntry(nKey);
        if (pEntry)
            return pEntry->GetLanguage();
    }

    return LANGUAGE_SYSTEM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
