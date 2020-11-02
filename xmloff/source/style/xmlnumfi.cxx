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

#include <svl/zformat.hxx>
#include <svl/numuno.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/color.hxx>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <sax/tools/converter.hxx>

#include <xmloff/xmlement.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/languagetagodf.hxx>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace {

struct SvXMLNumFmtEntry
{
    OUString   aName;
    sal_uInt32  nKey;
    bool        bRemoveAfterUse;

    SvXMLNumFmtEntry( const OUString& rN, sal_uInt32 nK, bool bR ) :
        aName(rN), nKey(nK), bRemoveAfterUse(bR) {}
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
    sal_uInt32              GetKeyForName( const OUString& rName );
    void                    AddKey( sal_uInt32 nKey, const OUString& rName, bool bRemoveAfterUse );
    void                    SetUsed( sal_uInt32 nKey );
    void                    RemoveVolatileFormats();
};

struct SvXMLNumberInfo
{
    sal_Int32   nDecimals           = -1;
    sal_Int32   nInteger            = -1;
    sal_Int32   nExpDigits          = -1;
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

public:
                SvXMLNumFmtElementContext( SvXMLImport& rImport, sal_Int32 nElement,
                                    SvXMLNumFormatContext& rParentContext, SvXMLStyleTokens nNewType,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL characters( const OUString& rChars ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void    AddEmbeddedElement( sal_Int32 nFormatPos, const OUString& rContent );
};

class SvXMLNumFmtEmbeddedTextContext : public SvXMLImportContext
{
    SvXMLNumFmtElementContext&  rParent;
    OUStringBuffer         aContent;
    sal_Int32                   nTextPosition;

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

sal_uInt32 SvXMLNumImpData::GetKeyForName( const OUString& rName )
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
    if ( !pLocaleData )
        pLocaleData = std::make_unique<LocaleDataWrapper>(
               pFormatter ? pFormatter->GetComponentContext() : m_xContext,
            LanguageTag( nLang ) );
    else
        pLocaleData->setLanguageTag( LanguageTag( nLang ) );
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
        OUString sValue = aIter.toString();
        switch ( aIter.getToken())
        {
            case XML_ELEMENT(FO, XML_COLOR):
            case XML_ELEMENT(FO_COMPAT, XML_COLOR):
                bColSet = ::sax::Converter::convertColor( m_nColor, sValue );
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
        OUString sValue = aIter.toString();
        if ( aIter.getToken() == XML_ELEMENT(NUMBER, XML_POSITION) )
        {
            if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                nTextPosition = nAttrVal;
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
    rParent.AddEmbeddedElement( nTextPosition, aContent.makeStringAndClear() );
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
    if ( cChar == '-' )
        return true;   // all format types may content minus sign or delimiter
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

    if ((nLength == 1 && lcl_ValidChar( rContent[0], rParent)) ||
            (nLength == 2 &&
             ((rContent[0] == ' ' && rContent[1] == '-') ||
              (rContent[1] == ' ' && lcl_ValidChar( rContent[0], rParent)))))
    {
        //  Don't quote single separator characters like space or percent,
        //  or separator characters followed by space (used in date formats).
        //  Or space followed by minus (used in currency formats) that would
        //  lead to almost duplicated formats with built-in formats just with
        //  the difference of quotes.
        bQuote = false;
    }
    else if ( rParent.GetType() == SvXMLStylesTokens::PERCENTAGE_STYLE && nLength > 1 )
    {
        //  the percent character in percentage styles must be left out of quoting
        //  (one occurrence is enough even if there are several percent characters in the string)

        OUString aString( rContent.toString() );
        sal_Int32 nPos = aString.indexOf( '%' );
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
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(NUMBER, XML_DECIMAL_PLACES):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                {
                    // fdo#58539 & gnome#627420: limit number of digits during import
                    aNumInfo.nDecimals = std::min<sal_Int32>(nAttrVal, NF_MAX_FORMAT_SYMBOLS);
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_MIN_DECIMAL_PLACES):
            case XML_ELEMENT(NUMBER, XML_MIN_DECIMAL_PLACES):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinDecimalDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_INTEGER_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nInteger = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_GROUPING):
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    aNumInfo.bGrouping = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_DISPLAY_FACTOR):
                if (::sax::Converter::convertDouble( fAttrDouble, sValue ))
                    aNumInfo.fDisplayFactor = fAttrDouble;
                break;
            case XML_ELEMENT(NUMBER, XML_DECIMAL_REPLACEMENT):
                if ( sValue == " " )
                {
                    aNumInfo.bDecAlign = true; // space replacement for "?"
                    bVarDecimals = true;
                }
                else
                    if ( sValue.isEmpty() )
                        bVarDecimals = true;   // empty replacement string: variable decimals
                    else                                // all other strings
                        aNumInfo.bDecReplace = true;    // decimal replacement with dashes
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_EXPONENT_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nExpDigits = std::min<sal_Int32>(nAttrVal, NF_MAX_FORMAT_SYMBOLS);
                break;
            case XML_ELEMENT(NUMBER, XML_EXPONENT_INTERVAL):
            case XML_ELEMENT(LO_EXT, XML_EXPONENT_INTERVAL):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nExpInterval = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_FORCED_EXPONENT_SIGN):
            case XML_ELEMENT(LO_EXT, XML_FORCED_EXPONENT_SIGN):
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    aNumInfo.bExpSign = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_MIN_DENOMINATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nMinDenomDigits = nAttrVal;
                break;
            case XML_ELEMENT(LO_EXT, XML_MAX_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 ))  // at least one '#'
                    aNumInfo.nMaxNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_DENOMINATOR_VALUE):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 )) // 0 is not valid
                {
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = false;
                }
                break;
            case XML_ELEMENT(NUMBER, XML_MAX_DENOMINATOR_VALUE):  // part of ODF 1.3
            case XML_ELEMENT(LO_EXT, XML_MAX_DENOMINATOR_VALUE):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 1 ) && aNumInfo.nFracDenominator <= 0)
                {   // if denominator value not yet defined
                    aNumInfo.nFracDenominator = nAttrVal;
                    bIsMaxDenominator = true;
                }
                break;
            case XML_ELEMENT(LO_EXT, XML_ZEROS_NUMERATOR_DIGITS):
            case XML_ELEMENT(NUMBER, XML_ZEROS_NUMERATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nZerosNumerDigits = nAttrVal;
                break;
            case XML_ELEMENT(NUMBER, XML_ZEROS_DENOMINATOR_DIGITS):
            case XML_ELEMENT(LO_EXT, XML_ZEROS_DENOMINATOR_DIGITS):
                if (::sax::Converter::convertNumber( nAttrVal, sValue, 0 ))
                    aNumInfo.nZerosDenomDigits = nAttrVal;
                 break;
            case XML_ELEMENT(NUMBER, XML_INTEGER_FRACTION_DELIMITER):
            case XML_ELEMENT(LO_EXT, XML_INTEGER_FRACTION_DELIMITER):
                aNumInfo.aIntegerFractionDelimiter = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_RFC_LANGUAGE_TAG):
                aLanguageTagODF.maRfcLanguageTag = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_LANGUAGE):
                aLanguageTagODF.maLanguage = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_SCRIPT):
                aLanguageTagODF.maScript = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_COUNTRY):
                aLanguageTagODF.maCountry = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_STYLE):
                SvXMLUnitConverter::convertEnum( bLong, sValue, aStyleValueMap );
                break;
            case XML_ELEMENT(NUMBER, XML_TEXTUAL):
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bTextual = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_CALENDAR):
                sCalendar = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    if ( aNumInfo.nMinDecimalDigits == -1)
    {
        if ( bVarDecimals || aNumInfo.bDecReplace )
            aNumInfo.nMinDecimalDigits = 0;
        else
            aNumInfo.nMinDecimalDigits = aNumInfo.nDecimals;
    }
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
    //  only number:number supports number:embedded-text child element

    if ( nType == SvXMLStyleTokens::Number &&
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

void SvXMLNumFmtElementContext::AddEmbeddedElement( sal_Int32 nFormatPos, const OUString& rContent )
{
    if (rContent.isEmpty())
        return;

    auto iterPair = aNumInfo.m_EmbeddedElements.emplace(nFormatPos, rContent);
    if (!iterPair.second)
        // there's already an element at this position - append text to existing element
        iterPair.first->second += rContent;
}

void SvXMLNumFmtElementContext::endFastElement(sal_Int32 )
{
    bool bEffLong = bLong;
    switch (nType)
    {
        case SvXMLStyleTokens::Text:
            if ( rParent.HasLongDoW() &&
                 aContent.toString() == rParent.GetLocaleData().getLongDateDayOfWeekSep() )
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
                rParent.AddToCode( aContent.makeStringAndClear() );
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
            // ignored - only default boolean format is supported
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
                bool bImplicitEC = (!sCalendar.isEmpty() &&
                        rParent.GetLocaleData().doesSecondaryCalendarUseEC( sCalendar));
                if (bImplicitEC || (!sCalendar.isEmpty() && rParent.HasEra()))
                {
                    // If E or EE is the first format keyword, passing
                    // bImplicitEC=true suppresses the superfluous calendar
                    // modifier for this format. This does not help for
                    // something like [~cal]DD/MM/EE but so far only YMD order
                    // is used with such calendars. Live with the modifier if
                    // other keywords precede this.
                    rParent.UpdateCalendar( sCalendar, bImplicitEC);
                    rParent.AddNfKeyword(
                            sal::static_int_cast< sal_uInt16 >(
                                bEffLong ? NF_KEY_EEC : NF_KEY_EC ) );
                }
                else
                {
                    rParent.UpdateCalendar( sCalendar );
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
            rParent.UpdateCalendar( sCalendar );
//! I18N doesn't provide SYSTEM or extended date information yet
            rParent.AddNfKeyword(
                sal::static_int_cast< sal_uInt16 >(
                    bEffLong ? NF_KEY_NNNN : NF_KEY_NN ) );
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
                    OUStringBuffer sIntegerFractionDelimiter = aNumInfo.aIntegerFractionDelimiter;
                    lcl_EnquoteIfNecessary( sIntegerFractionDelimiter, rParent );
                    rParent.AddToCode( sIntegerFractionDelimiter.makeStringAndClear() ); // default is ' '
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
                rParent.AddNumber( aNumInfo );      // simple number

                if ( aNumInfo.bExpSign )
                    rParent.AddToCode( "E+" );
                else
                    rParent.AddToCode( "E" );
                for (sal_Int32 i=0; i<aNumInfo.nExpDigits; i++)
                {
                    rParent.AddToCode( '0' );
                }
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
    pData( pNewData ),
    pStyles( &rStyles ),
    aMyConditions(),
    nType( nNewType ),
    nKey(-1),
    nFormatLang( LANGUAGE_SYSTEM ),
    bAutoOrder( false ),
    bFromSystem( false ),
    bTruncate( true ),
    bAutoDec( false ),
    bAutoInt( false ),
    bHasExtraText( false ),
    bHasLongDoW( false ),
    bHasEra( false ),
    bHasDateTime( false ),
    bRemoveAfterUse( false ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( false )
{
    LanguageTagODF aLanguageTagODF;
    css::i18n::NativeNumberXmlAttributes aNatNumAttr;
    OUString aSpellout;
    bool bAttrBool(false);

    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
        //  attributes for a style
            case XML_ELEMENT(STYLE, XML_NAME):
                break;
            case XML_ELEMENT(NUMBER, XML_RFC_LANGUAGE_TAG):
                aLanguageTagODF.maRfcLanguageTag = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_LANGUAGE):
                aLanguageTagODF.maLanguage = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_SCRIPT):
                aLanguageTagODF.maScript = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_COUNTRY):
                aLanguageTagODF.maCountry = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_TITLE):
                sFormatTitle = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_AUTOMATIC_ORDER):
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bAutoOrder = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_FORMAT_SOURCE):
                SvXMLUnitConverter::convertEnum( bFromSystem, sValue, aFormatSourceMap );
                break;
            case XML_ELEMENT(NUMBER, XML_TRUNCATE_ON_OVERFLOW):
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bTruncate = bAttrBool;
                break;
            case XML_ELEMENT(STYLE, XML_VOLATILE):
                //  volatile formats can be removed after importing
                //  if not used in other styles
                if (::sax::Converter::convertBool( bAttrBool, sValue ))
                    bRemoveAfterUse = bAttrBool;
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_FORMAT):
                aNatNumAttr.Format = sValue;
                break;
            case XML_ELEMENT(LO_EXT, XML_TRANSLITERATION_SPELLOUT):
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_SPELLOUT):
                aSpellout = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_LANGUAGE):
                aNatNumAttr.Locale.Language = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_COUNTRY):
                aNatNumAttr.Locale.Country = sValue;
                break;
            case XML_ELEMENT(NUMBER, XML_TRANSLITERATION_STYLE):
                aNatNumAttr.Style = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if (!aLanguageTagODF.isEmpty())
    {
        nFormatLang = aLanguageTagODF.getLanguageTag().getLanguageType( false);
        if ( nFormatLang == LANGUAGE_DONTKNOW )
            nFormatLang = LANGUAGE_SYSTEM;          //! error handling for unknown locales?
    }

    if (aNatNumAttr.Format.isEmpty() && aSpellout.isEmpty())
        return;

    LanguageTag aLanguageTag( OUString(), aNatNumAttr.Locale.Language,
                OUString(), aNatNumAttr.Locale.Country);
    aNatNumAttr.Locale = aLanguageTag.getLocale( false);

    // NatNum12 spell out formula (cardinal, ordinal, ordinal-feminine etc.)
    if ( !aSpellout.isEmpty() )
    {
        aFormatCode.append( "[NatNum12 " );
        aFormatCode.append( aSpellout );
    } else {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if ( !pFormatter ) return;

        sal_Int32 nNatNum = pFormatter->GetNatNum()->convertFromXmlAttributes( aNatNumAttr );
        aFormatCode.append( "[NatNum" );
        aFormatCode.append( nNatNum );
    }

    LanguageType eLang = aLanguageTag.getLanguageType( false );
    if ( eLang == LANGUAGE_DONTKNOW )
        eLang = LANGUAGE_SYSTEM;            //! error handling for unknown locales?
    if ( eLang != nFormatLang && eLang != LANGUAGE_SYSTEM )
    {
        aFormatCode.append( "][$-" );
        // language code in upper hex:
        aFormatCode.append(OUString::number(static_cast<sal_uInt16>(eLang), 16).toAsciiUpperCase());
    }
    aFormatCode.append( ']' );
}

SvXMLNumFormatContext::SvXMLNumFormatContext( SvXMLImport& rImport,
                                    const OUString& rName,
                                    const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/,
                                    const sal_Int32 nTempKey, LanguageType nLang,
                                    SvXMLStylesContext& rStyles ) :
    SvXMLStyleContext( rImport, XmlStyleFamily::DATA_STYLE ),
    pData( nullptr ),
    pStyles( &rStyles ),
    aMyConditions(),
    nType( SvXMLStylesTokens::NUMBER_STYLE ),
    nKey(nTempKey),
    nFormatLang( nLang ),
    bAutoOrder( false ),
    bFromSystem( false ),
    bTruncate( true ),
    bAutoDec( false ),
    bAutoInt( false ),
    bHasExtraText( false ),
    bHasLongDoW( false ),
    bHasEra( false ),
    bHasDateTime( false ),
    bRemoveAfterUse( false ),
    eDateDOW( XML_DEA_NONE ),
    eDateDay( XML_DEA_NONE ),
    eDateMonth( XML_DEA_NONE ),
    eDateYear( XML_DEA_NONE ),
    eDateHours( XML_DEA_NONE ),
    eDateMins( XML_DEA_NONE ),
    eDateSecs( XML_DEA_NONE ),
    bDateNoDefault( false )
{
    SetAttribute(XML_NAMESPACE_STYLE, GetXMLToken(XML_NAME), rName);
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
    if (nKey > -1)
    {
        if (bRemoveAfterUse)
        {
            //  format is used -> don't remove
            bRemoveAfterUse = false;
            if (pData)
                pData->SetUsed(nKey);

            //  Add to import's list of keys now - CreateAndInsert didn't add
            //  the style if bRemoveAfterUse was set.
            GetImport().AddNumberStyle( nKey, GetName() );
        }
        return nKey;
    }
    else
    {
        // reset bRemoveAfterUse before CreateAndInsert, so AddKey is called without bRemoveAfterUse set
        bRemoveAfterUse = false;
        CreateAndInsert(true);
        return nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::PrivateGetKey()
{
    //  used for map elements in CreateAndInsert - don't reset bRemoveAfterUse flag

    if (nKey > -1)
        return nKey;
    else
    {
        CreateAndInsert(true);
        return nKey;
    }
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert( css::uno::Reference< css::util::XNumberFormatsSupplier > const & xFormatsSupplier )
{
    if (nKey <= -1)
    {
        SvNumberFormatter* pFormatter = nullptr;
        SvNumberFormatsSupplierObj* pObj =
                        comphelper::getUnoTunnelImplementation<SvNumberFormatsSupplierObj>( xFormatsSupplier );
        if (pObj)
            pFormatter = pObj->GetNumberFormatter();

        if ( pFormatter )
            return CreateAndInsert( pFormatter );
        else
            return -1;
    }
    else
        return nKey;
}

void SvXMLNumFormatContext::CreateAndInsert(bool /*bOverwrite*/)
{
    if (nKey <= -1)
        CreateAndInsert(pData->GetNumberFormatter());
}

sal_Int32 SvXMLNumFormatContext::CreateAndInsert(SvNumberFormatter* pFormatter)
{
    if (!pFormatter)
    {
        OSL_FAIL("no number formatter");
        return -1;
    }

    sal_uInt32 nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

    for (size_t i = 0; i < aMyConditions.size(); i++)
    {
        SvXMLNumFormatContext* pStyle = const_cast<SvXMLNumFormatContext*>( static_cast<const SvXMLNumFormatContext *>(pStyles->FindStyleChildContext(
            XmlStyleFamily::DATA_STYLE, aMyConditions[i].sMapName)));
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

    if ( aFormatCode.isEmpty() )
    {
        //  insert empty format as empty string (with quotes)
        //  #93901# this check has to be done before inserting the conditions
        aFormatCode.append("\"\"");    // ""
    }

    aFormatCode.insert( 0, aConditions.makeStringAndClear() );
    OUString sFormat = aFormatCode.makeStringAndClear();

    //  test special cases

    if ( bAutoDec )         // automatic decimal places
    {
        //  #99391# adjust only if the format contains no text elements, no conditions
        //  and no color definition (detected by the '[' at the start)

        if ( nType == SvXMLStylesTokens::NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetStandardIndex( nFormatLang );
    }
    if ( bAutoInt )         // automatic integer digits
    {
        //! only if two decimal places was set?

        if ( nType == SvXMLStylesTokens::NUMBER_STYLE && !bHasExtraText &&
                aMyConditions.empty() && sFormat.toChar() != '[' )
            nIndex = pFormatter->GetFormatIndex( NF_NUMBER_SYSTEM, nFormatLang );
    }

    //  boolean is always the builtin boolean format
    //  (no other boolean formats are implemented)
    if ( nType == SvXMLStylesTokens::BOOLEAN_STYLE )
        nIndex = pFormatter->GetFormatIndex( NF_BOOLEAN, nFormatLang );

    //  check for default date formats
    if ( nType == SvXMLStylesTokens::DATE_STYLE && bAutoOrder && !bDateNoDefault )
    {
        NfIndexTableOffset eFormat = static_cast<NfIndexTableOffset>(SvXMLNumFmtDefaults::GetDefaultDateFormat(
            eDateDOW, eDateDay, eDateMonth, eDateYear,
            eDateHours, eDateMins, eDateSecs, bFromSystem ));
        if ( eFormat < NF_INDEX_TABLE_RESERVED_START )
        {
            //  #109651# if a date format has the automatic-order attribute and
            //  contains exactly the elements of one of the default date formats,
            //  use that default format, with the element order and separators
            //  from the current locale settings

            nIndex = pFormatter->GetFormatIndex( eFormat, nFormatLang );
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND && !sFormat.isEmpty() )
    {
        //  insert by format string

        OUString aFormatStr( sFormat );
        nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
        if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
        {
            sal_Int32  nErrPos = 0;
            SvNumFormatType l_nType = SvNumFormatType::ALL;
            bool bOk = pFormatter->PutEntry( aFormatStr, nErrPos, l_nType, nIndex, nFormatLang );
            if ( !bOk && nErrPos == 0 && aFormatStr != sFormat )
            {
                //  if the string was modified by PutEntry, look for an existing format
                //  with the modified string
                nIndex = pFormatter->GetEntryKey( aFormatStr, nFormatLang );
                if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND )
                    bOk = true;
            }
            if (!bOk)
                nIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;
        }
    }

//! I18N doesn't provide SYSTEM or extended date information yet
    if ( nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND && !bAutoOrder )
    {
        //  use fixed-order formats instead of SYS... if bAutoOrder is false
        //  (only if the format strings are equal for the locale)

        NfIndexTableOffset eOffset = pFormatter->GetIndexTableOffset( nIndex );
        if ( eOffset == NF_DATE_SYS_DMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMYYYY, nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
        else if ( eOffset == NF_DATE_SYS_DMMMMYYYY )
        {
            sal_uInt32 nNewIndex = pFormatter->GetFormatIndex( NF_DATE_DIN_DMMMMYYYY, nFormatLang );
            const SvNumberformat* pOldEntry = pFormatter->GetEntry( nIndex );
            const SvNumberformat* pNewEntry = pFormatter->GetEntry( nNewIndex );
            if ( pOldEntry && pNewEntry && pOldEntry->GetFormatstring() == pNewEntry->GetFormatstring() )
                nIndex = nNewIndex;
        }
    }

    if ((nIndex != NUMBERFORMAT_ENTRY_NOT_FOUND) && !sFormatTitle.isEmpty())
    {
        SvNumberformat* pFormat = const_cast<SvNumberformat*>(pFormatter->GetEntry( nIndex ));
        if (pFormat)
        {
            pFormat->SetComment(sFormatTitle);
        }
    }

    if ( nIndex == NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        OSL_FAIL("invalid number format");
        nIndex = pFormatter->GetStandardIndex( nFormatLang );
    }

    pData->AddKey( nIndex, GetName(), bRemoveAfterUse );
    nKey = nIndex;

    //  Add to import's list of keys (shared between styles and content import)
    //  only if not volatile - formats are removed from NumberFormatter at the
    //  end of each import (in SvXMLNumFmtHelper dtor).
    //  If bRemoveAfterUse is reset later in GetKey, AddNumberStyle is called there.

    if (!bRemoveAfterUse)
        GetImport().AddNumberStyle( nKey, GetName() );

    return nKey;
}

const LocaleDataWrapper& SvXMLNumFormatContext::GetLocaleData() const
{
    return pData->GetLocaleData( nFormatLang );
}

void SvXMLNumFormatContext::AddToCode( sal_Unicode c )
{
    aFormatCode.append( c );
    bHasExtraText = true;
}

void SvXMLNumFormatContext::AddToCode( const OUString& rString )
{
    aFormatCode.append( rString );
    bHasExtraText = true;
}

void SvXMLNumFormatContext::AddNumber( const SvXMLNumberInfo& rInfo )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    //  store special conditions
    bAutoDec = ( rInfo.nDecimals < 0 );
    bAutoInt = ( rInfo.nInteger < 0 );

    sal_uInt16 nPrec = 0;
    sal_uInt16 nLeading = 0;
    if ( rInfo.nDecimals >= 0 )                     //  < 0 : Default
        nPrec = static_cast<sal_uInt16>(rInfo.nDecimals);
    if ( rInfo.nInteger >= 0 )                      //  < 0 : Default
        nLeading = static_cast<sal_uInt16>(rInfo.nInteger);

    if ( bAutoDec )
    {
        if ( nType == SvXMLStylesTokens::CURRENCY_STYLE )
        {
            //  for currency formats, "automatic decimals" is used for the automatic
            //  currency format with (fixed) decimals from the locale settings

            const LocaleDataWrapper& rLoc = pData->GetLocaleData( nFormatLang );
            nPrec = rLoc.getCurrDigits();
        }
        else
        {
            //  for other types, "automatic decimals" means dynamic determination of
            //  decimals, as achieved with the "general" keyword

            aFormatCode.append( pFormatter->GetStandardName( nFormatLang ) );
            return;
        }
    }
    if ( bAutoInt )
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
    if ( nEmbeddedCount )
        bGrouping = false;      // grouping and embedded characters can't be used together

    sal_uInt32 nStdIndex = pFormatter->GetStandardIndex( nFormatLang );
    OUStringBuffer aNumStr = pFormatter->GenerateFormat( nStdIndex, nFormatLang,
                                                         bGrouping, false, nGenPrec, nLeading );

    if ( rInfo.nExpDigits >= 0 && nLeading == 0 && !bGrouping && nEmbeddedCount == 0 )
    {
        // #i43959# For scientific numbers, "#" in the integer part forces a digit,
        // so it has to be removed if nLeading is 0 (".00E+0", not "#.00E+0").

        aNumStr.stripStart('#');
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

    if ( nEmbeddedCount )
    {
        //  insert embedded strings into number string
        //  only the integer part is supported
        //  nZeroPos is the string position where format position 0 is inserted

        sal_Int32 nZeroPos = aNumStr.indexOf( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
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
        }

        // m_EmbeddedElements is sorted with ascending positions - loop is from right to left
        for (auto const& it : rInfo.m_EmbeddedElements)
        {
            sal_Int32 const nFormatPos = it.first;
            sal_Int32 nInsertPos = nZeroPos - nFormatPos;
            if ( nFormatPos >= 0 && nInsertPos >= 0 )
            {
                //  #107805# always quote embedded strings - even space would otherwise
                //  be recognized as thousands separator in French.

                aNumStr.insert(nInsertPos, '"');
                aNumStr.insert(nInsertPos, it.second);
                aNumStr.insert(nInsertPos, '"');
            }
        }
    }

    aFormatCode.append( aNumStr.makeStringAndClear() );

    if ( ( rInfo.bDecReplace || rInfo.nMinDecimalDigits < rInfo.nDecimals ) && nPrec )     // add decimal replacement (dashes)
    {
        //  add dashes for explicit decimal replacement, # or ? for variable decimals
        sal_Unicode cAdd = rInfo.bDecReplace ? '-' : ( rInfo.bDecAlign ? '?': '#' );

        if ( rInfo.nMinDecimalDigits == 0 )
            aFormatCode.append( pData->GetLocaleData( nFormatLang ).getNumDecimalSep() );
        for ( sal_uInt16 i=rInfo.nMinDecimalDigits; i<nPrec; i++)
            aFormatCode.append( cAdd );
    }

    //  add extra thousands separators for display factor

    if (rInfo.fDisplayFactor == 1.0 || rInfo.fDisplayFactor <= 0.0)
        return;

    //  test for 1.0 is just for optimization - nSepCount would be 0

    //  one separator for each factor of 1000
    sal_Int32 nSepCount = static_cast<sal_Int32>(::rtl::math::round( log10(rInfo.fDisplayFactor) / 3.0 ));
    if ( nSepCount > 0 )
    {
        OUString aSep = pData->GetLocaleData( nFormatLang ).getNumThousandSep();
        for ( sal_Int32 i=0; i<nSepCount; i++ )
            aFormatCode.append( aSep );
    }
}

void SvXMLNumFormatContext::AddCurrency( const OUString& rContent, LanguageType nLang )
{
    bool bAutomatic = false;
    OUString aSymbol = rContent;
    if ( aSymbol.isEmpty())
    {
        SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
        if ( pFormatter )
        {
            pFormatter->ChangeIntl( nFormatLang );
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

        sal_Int32 nLength = aFormatCode.getLength();
        if ( nLength > 1 && aFormatCode[nLength - 1] == '"' )
        {
            //  find start of quoted string
            //  When SvXMLNumFmtElementContext::EndElement creates escaped quotes,
            //  they must be handled here, too.

            sal_Int32 nFirst = nLength - 2;
            while ( nFirst >= 0 && aFormatCode[nFirst] != '"' )
                --nFirst;
            if ( nFirst >= 0 )
            {
                //  remove both quotes from aFormatCode
                OUString aOld = aFormatCode.makeStringAndClear();
                if ( nFirst > 0 )
                    aFormatCode.append( aOld.copy( 0, nFirst ) );
                if ( nLength > nFirst + 2 )
                    aFormatCode.append( aOld.copy( nFirst + 1, nLength - nFirst - 2 ) );
            }
        }
    }

    if (!bAutomatic)
        aFormatCode.append( "[$" );            // intro for "new" currency symbols

    aFormatCode.append( aSymbol );

    if (!bAutomatic)
    {
        if ( nLang != LANGUAGE_SYSTEM )
        {
            //  '-' sign and language code in hex:
            aFormatCode.append("-").append(OUString(OUString::number(sal_uInt16(nLang), 16)).toAsciiUpperCase());
        }

        aFormatCode.append( ']' );    // end of "new" currency symbol
    }
}

void SvXMLNumFormatContext::AddNfKeyword( sal_uInt16 nIndex )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    if ( nIndex == NF_KEY_G || nIndex == NF_KEY_GG || nIndex == NF_KEY_GGG )
        bHasEra = true;

    if ( nIndex == NF_KEY_NNNN )
    {
        nIndex = NF_KEY_NNN;
        bHasLongDoW = true;         // to remove string constant with separator
    }

    OUString sKeyword = pFormatter->GetKeyword( nFormatLang, nIndex );

    if ( nIndex == NF_KEY_H  || nIndex == NF_KEY_HH  ||
         nIndex == NF_KEY_MI || nIndex == NF_KEY_MMI ||
         nIndex == NF_KEY_S  || nIndex == NF_KEY_SS )
    {
        if ( !bTruncate && !bHasDateTime )
        {
            //  with truncate-on-overflow = false, add "[]" to first time part
            aFormatCode.append("[").append(sKeyword).append("]");
        }
        else
        {
            aFormatCode.append( sKeyword );
        }
        bHasDateTime = true;
    }
    else
    {
        aFormatCode.append( sKeyword );
    }
    //  collect the date elements that the format contains, to recognize default date formats
    switch ( nIndex )
    {
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
}

static bool lcl_IsAtEnd( OUStringBuffer& rBuffer, const OUString& rToken )
{
    sal_Int32 nBufLen = rBuffer.getLength();
    sal_Int32 nTokLen = rToken.getLength();

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

    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return false;

    OUString sOldStr = pFormatter->GetKeyword( nFormatLang, nOld );
    if ( lcl_IsAtEnd( aFormatCode, sOldStr ) )
    {
        // remove old keyword
        aFormatCode.setLength( aFormatCode.getLength() - sOldStr.getLength() );

        // add new keyword
        OUString sNewStr = pFormatter->GetKeyword( nFormatLang, nNew );
        aFormatCode.append( sNewStr );

        return true;    // changed
    }
    return false;       // not found
}

void SvXMLNumFormatContext::AddCondition( const sal_Int32 nIndex )
{
    OUString rApplyName = aMyConditions[nIndex].sMapName;
    OUString rCondition = aMyConditions[nIndex].sCondition;
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    sal_uInt32 l_nKey = pData->GetKeyForName( rApplyName );

    OUString sRealCond;
    if ( !(pFormatter && l_nKey != NUMBERFORMAT_ENTRY_NOT_FOUND &&
            rCondition.startsWith("value()", &sRealCond)) )
        return;

    //! test for valid conditions
    //! test for default conditions

    bool bDefaultCond = false;

    //! collect all conditions first and adjust default to >=0, >0 or <0 depending on count
    //! allow blanks in conditions
    if ( aConditions.isEmpty() && aMyConditions.size() == 1 && sRealCond == ">=0" )
        bDefaultCond = true;

    if ( nType == SvXMLStylesTokens::TEXT_STYLE && static_cast<size_t>(nIndex) == aMyConditions.size() - 1 )
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
            sRealCond = sRealCond.replaceAt( nPos, 2, "<>" );
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
        aConditions.append("[").append(sRealCond).append("]");
    }

    const SvNumberformat* pFormat = pFormatter->GetEntry(l_nKey);
    if ( pFormat )
        aConditions.append( pFormat->GetFormatstring() );

    aConditions.append( ';' );
}

void SvXMLNumFormatContext::AddCondition( const OUString& rCondition, const OUString& rApplyName )
{
    MyCondition aCondition;
    aCondition.sCondition = rCondition;
    aCondition.sMapName = rApplyName;
    aMyConditions.push_back(aCondition);
}

void SvXMLNumFormatContext::AddColor( Color const nColor )
{
    SvNumberFormatter* pFormatter = pData->GetNumberFormatter();
    if (!pFormatter)
        return;

    OUStringBuffer aColName;
    for ( sal_uInt16 i=0; i<XML_NUMF_COLORCOUNT; i++ )
        if (nColor == aNumFmtStdColors[i])
        {
            aColName = pFormatter->GetKeyword( nFormatLang, sal::static_int_cast< sal_uInt16 >(NF_KEY_FIRSTCOLOR + i) );
            break;
        }

    if ( !aColName.isEmpty() )
    {
        aColName.insert( 0, '[' );
        aColName.append( ']' );
        aFormatCode.insert( 0, aColName.makeStringAndClear() );
    }
}

void SvXMLNumFormatContext::UpdateCalendar( const OUString& rNewCalendar, bool bImplicitSecondaryCalendarEC )
{
    if ( rNewCalendar != sCalendar )
    {
        sCalendar = rNewCalendar;
        if ( !sCalendar.isEmpty() && !bImplicitSecondaryCalendarEC )
        {
            aFormatCode.append( "[~" );            // intro for calendar code
            aFormatCode.append( sCalendar );
            aFormatCode.append( ']' );    // end of calendar code
        }
    }
}

bool SvXMLNumFormatContext::IsSystemLanguage() const
{
    return nFormatLang == LANGUAGE_SYSTEM;
}


//  SvXMLNumFmtHelper


SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    const uno::Reference<util::XNumberFormatsSupplier>& rSupp,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    SvNumberFormatter* pFormatter = nullptr;
    SvNumberFormatsSupplierObj* pObj =
                    comphelper::getUnoTunnelImplementation<SvNumberFormatsSupplierObj>( rSupp );
    if (pObj)
        pFormatter = pObj->GetNumberFormatter();

    pData = std::make_unique<SvXMLNumImpData>( pFormatter, rxContext );
}

SvXMLNumFmtHelper::SvXMLNumFmtHelper(
    SvNumberFormatter* pNumberFormatter,
    const uno::Reference<uno::XComponentContext>& rxContext )
{
    SAL_WARN_IF( !rxContext.is(), "xmloff", "got no service manager" );

    pData = std::make_unique<SvXMLNumImpData>( pNumberFormatter, rxContext );
}

SvXMLNumFmtHelper::~SvXMLNumFmtHelper()
{
    //  remove temporary (volatile) formats from NumberFormatter
    pData->RemoveVolatileFormats();
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
                                      pData.get(), nStyleToken, xAttrList, rStyles );
}

LanguageType SvXMLNumFmtHelper::GetLanguageForKey(sal_Int32 nKey)
{
    if (pData->GetNumberFormatter())
    {
        const SvNumberformat* pEntry = pData->GetNumberFormatter()->GetEntry(nKey);
        if (pEntry)
            return pEntry->GetLanguage();
    }

    return LANGUAGE_SYSTEM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
