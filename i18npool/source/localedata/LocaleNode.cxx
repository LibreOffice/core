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

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <o3tl/sorted_vector.hxx>
#include <o3tl/temporary.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>
#include <sal/types.h>

#include "LocaleNode.hxx"
#include <i18npool/reservedconstants.hxx>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

// NOTE: MUST match the Locale versionDTD attribute defined in data/locale.dtd
#define LOCALE_VERSION_DTD "2.0.3"

typedef ::o3tl::sorted_vector< OUString > NameSet;
typedef ::o3tl::sorted_vector< sal_Int16 > ValueSet;

namespace cssi = ::com::sun::star::i18n;

LocaleNode::LocaleNode (OUString name, const Reference< XAttributeList > & attr)
    : aName(std::move(name))
    , aAttribs(attr)
    , parent(nullptr)
    , nError(0)
{
}

int LocaleNode::getError() const
{
    int err = nError;
    for (size_t i=0;i<children.size();i++)
        err += children[i]->getError();
    return err;
}

void LocaleNode::addChild ( LocaleNode * node) {
    children.emplace_back(node);
    node->parent = this;
}

const LocaleNode* LocaleNode::getRoot() const
{
    const LocaleNode* pRoot = nullptr;
    const LocaleNode* pParent = this;
    while ( (pParent = pParent->parent) != nullptr )
        pRoot = pParent;
    return pRoot;
}

const LocaleNode * LocaleNode::findNode ( const char *name) const {
    if (aName.equalsAscii(name))
        return this;
    for (size_t i = 0; i< children.size(); i++)
    {
        const LocaleNode *n=children[i]->findNode(name);
        if (n)
            return n;
    }
    return nullptr;
}

LocaleNode::~LocaleNode()
{
}

LocaleNode* LocaleNode::createNode (const OUString& name, const Reference< XAttributeList > & attr)
{
    if ( name == "LC_INFO" )
        return new LCInfoNode (name,attr);
    if ( name == "LC_CTYPE" )
        return new LCCTYPENode (name,attr);
    if ( name == "LC_FORMAT" )
        return new LCFormatNode (name,attr);
    if ( name == "LC_FORMAT_1" )
        return new LCFormatNode (name,attr);
    if ( name == "LC_CALENDAR" )
        return new LCCalendarNode (name,attr);
    if ( name == "LC_CURRENCY" )
        return new LCCurrencyNode (name,attr);
    if ( name == "LC_TRANSLITERATION" )
        return new LCTransliterationNode (name,attr);
    if ( name == "LC_COLLATION" )
        return new LCCollationNode (name,attr);
    if ( name == "LC_INDEX" )
        return new LCIndexNode (name,attr);
    if ( name == "LC_SEARCH" )
        return new LCSearchNode (name,attr);
    if ( name == "LC_MISC" )
        return new LCMiscNode (name,attr);
    if ( name == "LC_NumberingLevel" )
        return new LCNumberingLevelNode (name, attr);
    if ( name == "LC_OutLineNumberingLevel" )
        return new LCOutlineNumberingLevelNode (name, attr);

    return new LocaleNode(name,attr);
}


//   printf(" name: '%s'\n", p->getName().pData->buffer );
//   printf("value: '%s'\n", p->getValue().pData->buffer );

#define OSTR(s) (OUStringToOString( (s), RTL_TEXTENCODING_UTF8).getStr())

void LocaleNode::generateCode (const OFileWriter &of) const
{
    OUString aDTD = getAttr().getValueByName("versionDTD");
    if ( aDTD != LOCALE_VERSION_DTD )
    {
        ++nError;
        fprintf( stderr, "Error: Locale versionDTD is not %s, see comment in locale.dtd\n", LOCALE_VERSION_DTD);
    }
    for (size_t i=0; i<children.size(); i++)
        children[i]->generateCode (of);
//      print_node( this );
}


OUString LocaleNode::writeOUStringLiteralParameterCheckLen( const OFileWriter &of,
        const char* pParameterName, const LocaleNode* pNode,
        sal_Int32 nMinLen, sal_Int32 nMaxLen ) const
{
    OUString aVal;
    if (pNode)
        aVal = pNode->getValue();
    else if (nMinLen >= 0)  // -1: optional => empty, 0: must be present, empty
    {
        ++nError;
        fprintf( stderr, "Error: node NULL pointer for parameter %s.\n",
                pParameterName);
    }
    // write empty data if error
    of.writeOUStringLiteralParameter( pParameterName, aVal);
    sal_Int32 nLen = aVal.getLength();
    if (nLen < nMinLen)
    {
        ++nError;
        fprintf( stderr, "Error: less than %" SAL_PRIdINT32 " character%s (%" SAL_PRIdINT32 ") in %s '%s'.\n",
                nMinLen, (nMinLen > 1 ? "s" : ""),
                nLen,
                (pNode ? OSTR( pNode->getName()) : ""),
                OSTR( aVal));
    }
    else if (nLen > nMaxLen && nMaxLen >= 0)
    {
        ++nError;
        fprintf( stderr,
                "Error: more than %" SAL_PRIdINT32 " character%s (%" SAL_PRIdINT32 ") in %s '%s' not supported by application.\n",
                nMaxLen, (nMaxLen > 1 ? "s" : ""),
                nLen,
                (pNode ? OSTR( pNode->getName()) : ""),
                OSTR( aVal));
    }
    return aVal;
}

OUString LocaleNode::writeOUStringLiteralParameterCheckLen( const OFileWriter &of,
        const char* pNodeName, const char* pParameterName,
        sal_Int32 nMinLen, sal_Int32 nMaxLen ) const
{
    OUString aVal;
    const LocaleNode * pNode = findNode( pNodeName);
    if (pNode || nMinLen < 0)
        aVal = writeOUStringLiteralParameterCheckLen( of, pParameterName, pNode, nMinLen, nMaxLen);
    else
    {
        ++nError;
        fprintf( stderr, "Error: node %s not found.\n", pNodeName);
        // write empty data if error
        of.writeOUStringLiteralParameter( pParameterName, aVal);
    }
    return aVal;
}

void LocaleNode::incError( const char* pStr ) const
{
    ++nError;
    fprintf( stderr, "Error: %s\n", pStr);
}

void LocaleNode::incError( std::u16string_view rStr ) const
{
    incError( OSTR( rStr));
}

void LocaleNode::incErrorInt( const char* pStr, int nVal ) const
{
    ++nError;
    fprintf( stderr, pStr, nVal);
}

void LocaleNode::incErrorStr( const char* pStr, std::u16string_view rVal ) const
{
    ++nError;
    fprintf( stderr, pStr, OSTR( rVal));
}

void LocaleNode::incErrorStrStr( const char* pStr, std::u16string_view rVal1, std::u16string_view rVal2 ) const
{
    ++nError;
    fprintf(stderr, pStr, OSTR(rVal1), OSTR(rVal2));
}

void LCInfoNode::generateCode (const OFileWriter &of) const
{

    const LocaleNode * languageNode = findNode("Language");
    const LocaleNode * countryNode = findNode("Country");
    const LocaleNode * variantNode = findNode("Variant");

    OUString aLanguage;

    if (languageNode)
    {
        aLanguage = languageNode->getChildAt(0)->getValue();
        if (aLanguage.getLength() != 2 && aLanguage.getLength() != 3)
            incErrorStr( "Error: langID '%s' not 2-3 characters\n", aLanguage);
        of.writeOUStringLiteralParameter("langID", aLanguage);
        of.writeOUStringLiteralParameter("langDefaultName", languageNode->getChildAt(1)->getValue());
    }
    else
        incError( "No Language node.");
    if (countryNode)
    {
        OUString aCountry( countryNode->getChildAt(0)->getValue());
        if (!(aCountry.isEmpty() || aCountry.getLength() == 2))
            incErrorStr( "Error: countryID '%s' not empty or more than 2 characters\n", aCountry);
        of.writeOUStringLiteralParameter("countryID", aCountry);
        of.writeOUStringLiteralParameter("countryDefaultName", countryNode->getChildAt(1)->getValue());
    }
    else
        incError( "No Country node.");
    if (variantNode)
    {
        // If given Variant must be at least ll-Ssss and language must be 'qlt'
        const OUString& aVariant( variantNode->getValue());
        if (!(aVariant.isEmpty() || (aVariant.getLength() >= 7 && aVariant.indexOf('-') >= 2)))
            incErrorStr( "Error: invalid Variant '%s'\n", aVariant);
        if (!(aVariant.isEmpty() || aLanguage == "qlt"))
            incErrorStrStr( "Error: Variant '%s' given but Language '%s' is not 'qlt'\n", aVariant, aLanguage);
        of.writeOUStringLiteralParameter("Variant", aVariant);
    }
    else
        of.writeOUStringLiteralParameter("Variant", std::u16string_view());
    of.writeAsciiString("\nstatic constexpr OUString LCInfoArray[] = {\n");
    of.writeAsciiString("\tlangID,\n");
    of.writeAsciiString("\tlangDefaultName,\n");
    of.writeAsciiString("\tcountryID,\n");
    of.writeAsciiString("\tcountryDefaultName,\n");
    of.writeAsciiString("\tVariant\n");
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getLCInfo_", "std::size(LCInfoArray)", "LCInfoArray");
}


static OUString aDateSep;
static OUString aDecSep;

void LCCTYPENode::generateCode (const OFileWriter &of) const
{
    const LocaleNode * sepNode = nullptr;
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getLocaleItem_", useLocale);
        return;
    }
    OUString str =   getAttr().getValueByName("unoid");
    of.writeAsciiString("\n\n");
    of.writeOUStringLiteralParameter("LC_CTYPE_Unoid", str);

    aDateSep =
        writeOUStringLiteralParameterCheckLen( of, "DateSeparator", "dateSeparator", 1, 1);
    OUString aThoSep =
        writeOUStringLiteralParameterCheckLen( of, "ThousandSeparator", "thousandSeparator", 1, 1);
    aDecSep =
        writeOUStringLiteralParameterCheckLen( of, "DecimalSeparator", "decimalSeparator", 1, 1);
    OUString aDecSepAlt =
        writeOUStringLiteralParameterCheckLen( of, "DecimalSeparatorAlternative", "decimalSeparatorAlternative", -1, 1);
    OUString aTimeSep =
        writeOUStringLiteralParameterCheckLen( of, "TimeSeparator", "timeSeparator", 1, 1);
    OUString aTime100Sep =
        writeOUStringLiteralParameterCheckLen( of, "Time100SecSeparator", "time100SecSeparator", 1, 1);
    OUString aListSep =
        writeOUStringLiteralParameterCheckLen( of, "ListSeparator", "listSeparator", 1, 1);

    OUString aLDS;

    sepNode = findNode("LongDateDayOfWeekSeparator");
    aLDS = sepNode->getValue();
    of.writeOUStringLiteralParameter("LongDateDayOfWeekSeparator", aLDS);
    if (aLDS == ",")
        fprintf( stderr, "Warning: %s\n",
                "LongDateDayOfWeekSeparator is only a comma not followed by a space. Usually this is not the case and may lead to concatenated display names like \"Wednesday,May 9, 2007\".");

    sepNode = findNode("LongDateDaySeparator");
    aLDS = sepNode->getValue();
    of.writeOUStringLiteralParameter("LongDateDaySeparator", aLDS);
    if (aLDS == "," || aLDS == ".")
        fprintf( stderr, "Warning: %s\n",
                "LongDateDaySeparator is only a comma or dot not followed by a space. Usually this is not the case and may lead to concatenated display names like \"Wednesday, May 9,2007\".");

    sepNode = findNode("LongDateMonthSeparator");
    aLDS = sepNode->getValue();
    of.writeOUStringLiteralParameter("LongDateMonthSeparator", aLDS);
    if (aLDS.isEmpty())
        fprintf( stderr, "Warning: %s\n",
                "LongDateMonthSeparator is empty. Usually this is not the case and may lead to concatenated display names like \"Wednesday, May9, 2007\".");

    sepNode = findNode("LongDateYearSeparator");
    aLDS = sepNode->getValue();
    of.writeOUStringLiteralParameter("LongDateYearSeparator", aLDS);
    if (aLDS.isEmpty())
        fprintf( stderr, "Warning: %s\n",
                "LongDateYearSeparator is empty. Usually this is not the case and may lead to concatenated display names like \"Wednesday, 2007May 9\".");

    int nSavErr = nError;
    int nWarn = 0;
    if (aDateSep == aTimeSep)
        incError( "DateSeparator equals TimeSeparator.");
    if (aDecSep == aThoSep)
        incError( "DecimalSeparator equals ThousandSeparator.");
    if (aDecSepAlt == aThoSep)
        incError( "DecimalSeparatorAlternative equals ThousandSeparator.");
    if (aDecSepAlt == aDecSep)
        incError( "DecimalSeparatorAlternative equals DecimalSeparator, it must not be specified then.");
    if ( aThoSep == " " )
        incError( "ThousandSeparator is an ' ' ordinary space, this should be a non-breaking space U+00A0 instead.");
    if (aListSep == aDecSep)
        fprintf( stderr, "Warning: %s\n",
                "ListSeparator equals DecimalSeparator.");
    if (aListSep == aThoSep)
        fprintf( stderr, "Warning: %s\n",
                "ListSeparator equals ThousandSeparator.");
    if (aListSep.getLength() != 1 || aListSep[0] != ';')
    {
        incError( "ListSeparator not ';' semicolon. Strongly recommended. Currently required.");
        ++nSavErr;  // format codes not affected
    }
    if (aTimeSep == aTime100Sep)
    {
        ++nWarn;
        fprintf( stderr, "Warning: %s\n",
                "Time100SecSeparator equals TimeSeparator, this is probably an error.");
    }
    if (aDecSep != aTime100Sep)
    {
        ++nWarn;
        fprintf( stderr, "Warning: %s\n",
                "Time100SecSeparator is different from DecimalSeparator, this may be correct or not. Intended?");
    }
    if (nSavErr != nError || nWarn)
        fprintf( stderr, "Warning: %s\n",
                "Don't forget to adapt corresponding FormatCode elements when changing separators.");

    OUString aQuoteStart =
        writeOUStringLiteralParameterCheckLen( of, "QuotationStart", "quotationStart", 1, 1);
    OUString aQuoteEnd =
        writeOUStringLiteralParameterCheckLen( of, "QuotationEnd", "quotationEnd", 1, 1);
    OUString aDoubleQuoteStart =
        writeOUStringLiteralParameterCheckLen( of, "DoubleQuotationStart", "doubleQuotationStart", 1, 1);
    OUString aDoubleQuoteEnd =
        writeOUStringLiteralParameterCheckLen( of, "DoubleQuotationEnd", "doubleQuotationEnd", 1, 1);

    if (aQuoteStart.toChar() <= 127 && aQuoteEnd.toChar() > 127)
        fprintf( stderr, "Warning: %s\n",
                "QuotationStart is an ASCII character but QuotationEnd is not.");
    if (aQuoteEnd.toChar() <= 127 && aQuoteStart.toChar() > 127)
        fprintf( stderr, "Warning: %s\n",
                "QuotationEnd is an ASCII character but QuotationStart is not.");
    if (aDoubleQuoteStart.toChar() <= 127 && aDoubleQuoteEnd.toChar() > 127)
        fprintf( stderr, "Warning: %s\n",
                "DoubleQuotationStart is an ASCII character but DoubleQuotationEnd is not.");
    if (aDoubleQuoteEnd.toChar() <= 127 && aDoubleQuoteStart.toChar() > 127)
        fprintf( stderr, "Warning: %s\n",
                "DoubleQuotationEnd is an ASCII character but DoubleQuotationStart is not.");
    if (aQuoteStart.toChar() <= 127 && aQuoteEnd.toChar() <= 127)
        fprintf( stderr, "Warning: %s\n",
                "QuotationStart and QuotationEnd are both ASCII characters. Not necessarily an issue, but unusual.");
    if (aDoubleQuoteStart.toChar() <= 127 && aDoubleQuoteEnd.toChar() <= 127)
        fprintf( stderr, "Warning: %s\n",
                "DoubleQuotationStart and DoubleQuotationEnd are both ASCII characters. Not necessarily an issue, but unusual.");
    if (aQuoteStart == aQuoteEnd)
        fprintf( stderr, "Warning: %s\n",
                "QuotationStart equals QuotationEnd. Not necessarily an issue, but unusual.");
    if (aDoubleQuoteStart == aDoubleQuoteEnd)
        fprintf( stderr, "Warning: %s\n",
                "DoubleQuotationStart equals DoubleQuotationEnd. Not necessarily an issue, but unusual.");
    /* TODO: should equalness of single and double quotes be an error? Would
     * need to adapt quite some locales' data. */
    if (aQuoteStart == aDoubleQuoteStart)
        fprintf( stderr, "Warning: %s\n",
                "QuotationStart equals DoubleQuotationStart. Not necessarily an issue, but unusual.");
    if (aQuoteEnd == aDoubleQuoteEnd)
        fprintf( stderr, "Warning: %s\n",
                "QuotationEnd equals DoubleQuotationEnd. Not necessarily an issue, but unusual.");
    // Known good values, exclude ASCII single (U+0027, ') and double (U+0022, ") quotes.
    switch (int ic = aQuoteStart.toChar())
    {
        case 0x2018:    // LEFT SINGLE QUOTATION MARK
        case 0x201a:    // SINGLE LOW-9 QUOTATION MARK
        case 0x201b:    // SINGLE HIGH-REVERSED-9 QUOTATION MARK
        case 0x2039:    // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        case 0x203a:    // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        case 0x300c:    // LEFT CORNER BRACKET (Chinese)
            ;
            break;
        default:
            fprintf( stderr, "Warning: %s U+%04X %s\n",
                    "QuotationStart may be wrong:", ic, OSTR( aQuoteStart));
    }
    switch (int ic = aQuoteEnd.toChar())
    {
        case 0x2019:    // RIGHT SINGLE QUOTATION MARK
        case 0x201a:    // SINGLE LOW-9 QUOTATION MARK
        case 0x201b:    // SINGLE HIGH-REVERSED-9 QUOTATION MARK
        case 0x2039:    // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        case 0x203a:    // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        case 0x300d:    // RIGHT CORNER BRACKET (Chinese)
            ;
            break;
        default:
            fprintf( stderr, "Warning: %s U+%04X %s\n",
                    "QuotationEnd may be wrong:", ic, OSTR( aQuoteEnd));
    }
    switch (int ic = aDoubleQuoteStart.toChar())
    {
        case 0x00ab:    // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        case 0x00bb:    // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        case 0x201c:    // LEFT DOUBLE QUOTATION MARK
        case 0x201e:    // DOUBLE LOW-9 QUOTATION MARK
        case 0x201f:    // DOUBLE HIGH-REVERSED-9 QUOTATION MARK
        case 0x300e:    // LEFT WHITE CORNER BRACKET (Chinese)
            ;
            break;
        default:
            fprintf( stderr, "Warning: %s U+%04X %s\n",
                    "DoubleQuotationStart may be wrong:", ic, OSTR( aDoubleQuoteStart));
    }
    switch (int ic = aDoubleQuoteEnd.toChar())
    {
        case 0x00ab:    // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        case 0x00bb:    // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        case 0x201d:    // RIGHT DOUBLE QUOTATION MARK
        case 0x201e:    // DOUBLE LOW-9 QUOTATION MARK
        case 0x201f:    // DOUBLE HIGH-REVERSED-9 QUOTATION MARK
        case 0x300f:    // RIGHT WHITE CORNER BRACKET (Chinese)
            ;
            break;
        default:
            fprintf( stderr, "Warning: %s U+%04X %s\n",
                    "DoubleQuotationEnd may be wrong:", ic, OSTR( aDoubleQuoteEnd));
    }

    writeOUStringLiteralParameterCheckLen( of, "TimeAM", "timeAM", 1, -1);
    writeOUStringLiteralParameterCheckLen( of, "TimePM", "timePM", 1, -1);
    sepNode = findNode("MeasurementSystem");
    of.writeOUStringLiteralParameter("measurementSystem", sepNode->getValue());

    of.writeAsciiString("\nstatic constexpr OUString LCType[] = {\n");
    of.writeAsciiString("\tLC_CTYPE_Unoid,\n");
    of.writeAsciiString("\tdateSeparator,\n");
    of.writeAsciiString("\tthousandSeparator,\n");
    of.writeAsciiString("\tdecimalSeparator,\n");
    of.writeAsciiString("\ttimeSeparator,\n");
    of.writeAsciiString("\ttime100SecSeparator,\n");
    of.writeAsciiString("\tlistSeparator,\n");
    of.writeAsciiString("\tquotationStart,\n");
    of.writeAsciiString("\tquotationEnd,\n");
    of.writeAsciiString("\tdoubleQuotationStart,\n");
    of.writeAsciiString("\tdoubleQuotationEnd,\n");
    of.writeAsciiString("\ttimeAM,\n");
    of.writeAsciiString("\ttimePM,\n");
    of.writeAsciiString("\tmeasurementSystem,\n");
    of.writeAsciiString("\tLongDateDayOfWeekSeparator,\n");
    of.writeAsciiString("\tLongDateDaySeparator,\n");
    of.writeAsciiString("\tLongDateMonthSeparator,\n");
    of.writeAsciiString("\tLongDateYearSeparator,\n");
    of.writeAsciiString("\tdecimalSeparatorAlternative\n");
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getLocaleItem_", "std::size(LCType)", "LCType");
}


static OUString sTheCurrencyReplaceTo;
static OUString sTheCompatibleCurrency;
static OUString sTheDateEditFormat;

sal_Int16 LCFormatNode::mnSection = 0;
sal_Int16 LCFormatNode::mnFormats = 0;

void LCFormatNode::generateCode (const OFileWriter &of) const
{
    if (mnSection >= 2)
        incError("more than 2 LC_FORMAT sections");

    ::std::vector< OUString > theDateAcceptancePatterns;

    OUString useLocale(getAttr().getValueByName("ref"));

    OUString str;
    OUString strFrom( getAttr().getValueByName("replaceFrom"));
    if (useLocale.isEmpty())
    {
        of.writeParameter("replaceFrom", strFrom, mnSection);
    }
    str = getAttr().getValueByName("replaceTo");
    if (!strFrom.isEmpty() && str.isEmpty())
        incErrorStr("replaceFrom=\"%s\" replaceTo=\"\" is empty replacement.\n", strFrom);
    // Locale data generator inserts FFFF for LangID, we need to adapt that.
    if (str.endsWithIgnoreAsciiCase( "-FFFF]"))
        incErrorStr("replaceTo=\"%s\" needs FFFF to be adapted to the real LangID value.\n", str);
    of.writeParameter("replaceTo", str, mnSection);
    // Remember the replaceTo value for "[CURRENCY]" to check format codes.
    if ( strFrom == "[CURRENCY]" )
        sTheCurrencyReplaceTo = str;
    // Remember the currency symbol if present.
    if (str.startsWith( "[$" ))
    {
        sal_Int32 nHyphen = str.indexOf( '-');
        if (nHyphen >= 3)
        {
            sTheCompatibleCurrency = str.copy( 2, nHyphen - 2);
        }
    }

    if (!useLocale.isEmpty())
    {
        if (!strFrom.isEmpty() && strFrom != "[CURRENCY]") //???
        {
            incErrorStrStr(
                "Error: non-empty replaceFrom=\"%s\" with non-empty ref=\"%s\".",
                strFrom, useLocale);
        }
        useLocale = useLocale.replace( '-', '_');
        switch (mnSection)
        {
            case 0:
                of.writeOUStringRefFunction("getAllFormats0_", useLocale, "replaceTo0");
                of.writeOUStringRefFunction("getDateAcceptancePatterns_", useLocale);
                break;
            case 1:
                of.writeOUStringRefFunction("getAllFormats1_", useLocale, "replaceTo1");
                break;
        }
        ++mnSection;
        return;
    }

    sal_Int16 formatCount = mnFormats;
    NameSet  aMsgIdSet;
    ValueSet aFormatIndexSet;
    NameSet  aDefaultsSet;
    bool bCtypeIsRef = false;
    bool bHaveEngineering = false;
    bool bShowNextFreeFormatIndex = false;

    for (sal_Int32 i = 0; i< getNumberOfChildren() ; i++, formatCount++)
    {
        LocaleNode * currNode = getChildAt (i);
        if ( currNode->getName() == "DateAcceptancePattern" )
        {
            if (mnSection > 0)
                incError( "DateAcceptancePattern only handled in LC_FORMAT, not LC_FORMAT_1");
            else
                theDateAcceptancePatterns.push_back( currNode->getValue());
            --formatCount;
            continue;   // for
        }
        if ( currNode->getName() != "FormatElement" )
        {
            incErrorStr( "Error: Undefined element '%s' in LC_FORMAT\n", currNode->getName());
            --formatCount;
            continue;   // for
        }

        OUString aUsage;
        OUString aType;
        OUString aFormatIndex;
        //      currNode -> print();
        const Attr &currNodeAttr = currNode->getAttr();
        //printf ("getLen() = %d\n", currNode->getAttr().getLength());

        str = currNodeAttr.getValueByName("msgid");
        if (!aMsgIdSet.insert( str).second)
            incErrorStr( "Error: Duplicated msgid=\"%s\" in FormatElement.\n", str);
        of.writeOUStringLiteralParameter("FormatKey", str, formatCount);

        str = currNodeAttr.getValueByName("default");
        bool bDefault = str == "true";
        of.writeOUStringLiteralDefaultParameter("FormatElement", str, formatCount);

        aType = currNodeAttr.getValueByName("type");
        of.writeOUStringLiteralParameter("FormatType", aType, formatCount);

        aUsage = currNodeAttr.getValueByName("usage");
        of.writeOUStringLiteralParameter("FormatUsage", aUsage, formatCount);

        aFormatIndex = currNodeAttr.getValueByName("formatindex");
        sal_Int16 formatindex = static_cast<sal_Int16>(aFormatIndex.toInt32());
        // Ensure the new reserved range is not used anymore, free usage start
        // was up'ed from 50 to 60 (and more later).
        if (i18npool::nStopPredefinedFormatIndex <= formatindex && formatindex < i18npool::nFirstFreeFormatIndex)
        {
            incErrorInt( "Error: Reserved formatindex=\"%d\" in FormatElement.\n", formatindex);
            bShowNextFreeFormatIndex = true;
        }
        if (!aFormatIndexSet.insert( formatindex).second)
        {
            incErrorInt( "Error: Duplicated formatindex=\"%d\" in FormatElement.\n", formatindex);
            bShowNextFreeFormatIndex = true;
        }
        of.writeOUStringLiteralIntParameter("Formatindex", formatCount, formatindex);

        // Ensure only one default per usage and type.
        if (bDefault)
        {
            OUString aKey( aUsage + "," + aType);
            if (!aDefaultsSet.insert( aKey).second)
            {
                OUString aStr = "Duplicated default for usage=\"" + aUsage + "\" type=\"" + aType + "\": formatindex=\"" + aFormatIndex + "\".";
                incError( aStr);
            }
        }

        const LocaleNode * n = currNode -> findNode("FormatCode");
        if (n)
        {
            of.writeOUStringLiteralParameter("FormatCode", n->getValue(), formatCount);
            // Check separator usage for some FormatCode elements.
            const LocaleNode* pCtype = nullptr;
            switch (formatindex)
            {
                case cssi::NumberFormatIndex::DATE_SYS_DDMMYYYY :
                    sTheDateEditFormat = n->getValue();
                    break;
                case cssi::NumberFormatIndex::NUMBER_1000DEC2 : // #,##0.00
                case cssi::NumberFormatIndex::TIME_MMSS00 :     // MM:SS.00
                case cssi::NumberFormatIndex::TIME_HH_MMSS00 :  // [HH]:MM:SS.00
                    {
                        const LocaleNode* pRoot = getRoot();
                        if (!pRoot)
                            incError( "No root for FormatCode.");
                        else
                        {
                            pCtype = pRoot->findNode( "LC_CTYPE");
                            if (!pCtype)
                                incError( "No LC_CTYPE found for FormatCode.");
                            else
                            {
                                OUString aRef( pCtype->getAttr().getValueByName("ref"));
                                if (!aRef.isEmpty())
                                {
                                    aRef = aRef.replace( '-', '_');
                                    if (!bCtypeIsRef)
                                        fprintf( stderr,
                                                "Warning: Can't check separators used in FormatCode due to LC_CTYPE ref=\"%s\".\n"
                                                "If these two locales use identical format codes, you should consider to use the ref= mechanism also for the LC_FORMAT element, together with replaceFrom= and replaceTo= for the currency.\n",
                                                OSTR( aRef));
                                    bCtypeIsRef = true;
                                    pCtype = nullptr;
                                }
                            }
                        }
                    }
                    break;
                case cssi::NumberFormatIndex::CURRENCY_1000DEC2 :
                    // Remember the currency symbol if present.
                    {
                        if (sTheCompatibleCurrency.isEmpty())
                        {
                            sal_Int32 nStart = n->getValue().indexOf("[$");
                            if (nStart >= 0)
                            {
                                const OUString& aCode( n->getValue());
                                sal_Int32 nHyphen = aCode.indexOf( '-', nStart);
                                if (nHyphen >= nStart + 3)
                                    sTheCompatibleCurrency = aCode.copy( nStart + 2, nHyphen - nStart - 2);
                            }
                        }
                    }
                    [[fallthrough]];
                case cssi::NumberFormatIndex::CURRENCY_1000INT :
                case cssi::NumberFormatIndex::CURRENCY_1000INT_RED :
                case cssi::NumberFormatIndex::CURRENCY_1000DEC2_RED :
                case cssi::NumberFormatIndex::CURRENCY_1000DEC2_CCC :
                case cssi::NumberFormatIndex::CURRENCY_1000DEC2_DASHED :
                    // Currency formats should be something like [C]###0;-[C]###0
                    // and not parenthesized [C]###0;([C]###0) if not en_US.
                    if (strcmp( of.getLocale(), "en_US") != 0)
                    {
                        const OUString& aCode( n->getValue());
                        if (aCode.indexOf( "0)" ) > 0 || aCode.indexOf( "-)" ) > 0 ||
                                aCode.indexOf( " )" ) > 0 || aCode.indexOf( "])" ) > 0)
                            fprintf( stderr, "Warning: FormatCode formatindex=\"%d\" for currency uses parentheses for negative amounts, which probably is not correct for locales not based on en_US.\n", formatindex);
                    }
                    // Check if we have replaceTo for "[CURRENCY]" placeholder.
                    if (sTheCurrencyReplaceTo.isEmpty())
                    {
                        const OUString& aCode( n->getValue());
                        if (aCode.indexOf( "[CURRENCY]" ) >= 0)
                            incErrorInt( "Error: [CURRENCY] replaceTo not found for formatindex=\"%d\".\n", formatindex);
                    }
                    break;
                default:
                    if (aUsage == "SCIENTIFIC_NUMBER")
                    {
                        // Check for presence of  ##0.00E+00
                        const OUString& aCode( n->getValue());
                        // Simple check without decimal separator (assumed to
                        // be one UTF-16 character). May be prefixed with
                        // [NatNum1] or other tags.
                        sal_Int32 nInt = aCode.indexOf("##0");
                        sal_Int32 nDec = (nInt < 0 ? -1 : aCode.indexOf("00E+00", nInt));
                        if (nInt >= 0 && nDec == nInt+4)
                            bHaveEngineering = true;
                    }
                    break;
            }
            if (pCtype)
            {
                int nSavErr = nError;
                const OUString& aCode( n->getValue());
                if (formatindex == cssi::NumberFormatIndex::NUMBER_1000DEC2)
                {
                    sal_Int32 nDec = -1;
                    sal_Int32 nGrp = -1;
                    const LocaleNode* pSep = pCtype->findNode( "DecimalSeparator");
                    if (!pSep)
                        incError( "No DecimalSeparator found for FormatCode.");
                    else
                    {
                        nDec = aCode.indexOf( pSep->getValue());
                        if (nDec < 0)
                            incErrorInt( "Error: DecimalSeparator not present in FormatCode formatindex=\"%d\".\n",
                                    formatindex);
                    }
                    pSep = pCtype->findNode( "ThousandSeparator");
                    if (!pSep)
                        incError( "No ThousandSeparator found for FormatCode.");
                    else
                    {
                        nGrp = aCode.indexOf( pSep->getValue());
                        if (nGrp < 0)
                            incErrorInt( "Error: ThousandSeparator not present in FormatCode formatindex=\"%d\".\n",
                                    formatindex);
                    }
                    if (nDec >= 0 && nGrp >= 0 && nDec <= nGrp)
                        incErrorInt( "Error: Ordering of ThousandSeparator and DecimalSeparator not correct in formatindex=\"%d\".\n",
                                formatindex);
                }
                if (formatindex == cssi::NumberFormatIndex::TIME_MMSS00 ||
                        formatindex == cssi::NumberFormatIndex::TIME_HH_MMSS00)
                {
                    sal_Int32 nTime = -1;
                    sal_Int32 n100s = -1;
                    const LocaleNode* pSep = pCtype->findNode( "TimeSeparator");
                    if (!pSep)
                        incError( "No TimeSeparator found for FormatCode.");
                    else
                    {
                        nTime = aCode.indexOf( pSep->getValue());
                        if (nTime < 0)
                            incErrorInt( "Error: TimeSeparator not present in FormatCode formatindex=\"%d\".\n",
                                    formatindex);
                    }
                    pSep = pCtype->findNode( "Time100SecSeparator");
                    if (!pSep)
                        incError( "No Time100SecSeparator found for FormatCode.");
                    else
                    {
                        n100s = aCode.indexOf( pSep->getValue());
                        if (n100s < 0)
                            incErrorInt( "Error: Time100SecSeparator not present in FormatCode formatindex=\"%d\".\n",
                                    formatindex);
                        n100s = aCode.indexOf( Concat2View(pSep->getValue() + "00"));
                        if (n100s < 0)
                            incErrorInt( "Error: Time100SecSeparator+00 not present in FormatCode formatindex=\"%d\".\n",
                                    formatindex);
                    }
                    if (n100s >= 0 && nTime >= 0 && n100s <= nTime)
                        incErrorInt( "Error: Ordering of Time100SecSeparator and TimeSeparator not correct in formatindex=\"%d\".\n",
                                formatindex);
                }
                if (nSavErr != nError)
                    fprintf( stderr,
                            "Warning: formatindex=\"%d\",\"%d\",\"%d\" are the only FormatCode elements checked for separator usage, there may be others that have errors.\n",
                            int(cssi::NumberFormatIndex::NUMBER_1000DEC2),
                            int(cssi::NumberFormatIndex::TIME_MMSS00),
                            int(cssi::NumberFormatIndex::TIME_HH_MMSS00));

            }
        }
        else
            incError( "No FormatCode in FormatElement.");
        n = currNode -> findNode("DefaultName");
        if (n)
            of.writeOUStringLiteralParameter("FormatDefaultName", n->getValue(), formatCount);
        else
            of.writeOUStringLiteralParameter("FormatDefaultName", std::u16string_view(), formatCount);

    }

    if (bShowNextFreeFormatIndex)
    {
        sal_Int16 nNext = i18npool::nFirstFreeFormatIndex;
        auto it = aFormatIndexSet.find( nNext);
        if (it != aFormatIndexSet.end())
        {
            // nFirstFreeFormatIndex already used, find next free including gaps.
            do
            {
                ++nNext;
            }
            while (++it != aFormatIndexSet.end() && *it == nNext);
        }
        fprintf( stderr, "Hint: Next free formatindex is %d.\n", static_cast<int>(nNext));
    }

    // Check presence of all required format codes only in first section
    // LC_FORMAT, not in optional LC_FORMAT_1
    if (mnSection == 0)
    {
        // At least one abbreviated date acceptance pattern must be present.
        if (theDateAcceptancePatterns.empty())
            incError( "No DateAcceptancePattern present.\n");
        else
        {
            bool bHaveAbbr = false;
            for (auto const& elem : theDateAcceptancePatterns)
            {
                if (elem.indexOf('D') > -1 && elem.indexOf('M') > -1 && elem.indexOf('Y') <= -1)
                {
                    bHaveAbbr = true;
                    break;
                }
            }
            if (!bHaveAbbr)
                incError( "No abbreviated DateAcceptancePattern present. For example M/D or D.M.\n");
        }

        // 0..9 MUST be present, 10,11 MUST NOT be present, 12..47 MUST be
        // present, 48,49 MUST NOT be present, 50 MUST be present.
        ValueSet::const_iterator aIter( aFormatIndexSet.begin());
        for (sal_Int16 nNext = cssi::NumberFormatIndex::NUMBER_START;
                nNext < i18npool::nStopPredefinedFormatIndex; ++nNext)
        {
            sal_Int16 nHere = ::std::min( (aIter != aFormatIndexSet.end() ? *aIter :
                    i18npool::nStopPredefinedFormatIndex),
                    i18npool::nStopPredefinedFormatIndex);
            if (aIter != aFormatIndexSet.end()) ++aIter;
            for ( ; nNext < nHere; ++nNext)
            {
                switch (nNext)
                {
                    case cssi::NumberFormatIndex::FRACTION_1 :
                    case cssi::NumberFormatIndex::FRACTION_2 :
                    case cssi::NumberFormatIndex::BOOLEAN :
                    case cssi::NumberFormatIndex::TEXT :
                        // generated internally
                        break;
                    default:
                        incErrorInt( "Error: FormatElement formatindex=\"%d\" not present.\n", nNext);
                }
            }
            switch (nHere)
            {
                case cssi::NumberFormatIndex::FRACTION_1 :
                    incErrorInt( "Error: FormatElement formatindex=\"%d\" reserved for internal ``# ?/?''.\n", nNext);
                    break;
                case cssi::NumberFormatIndex::FRACTION_2 :
                    incErrorInt( "Error: FormatElement formatindex=\"%d\" reserved for internal ``# ?\?/?\?''.\n", nNext);
                    break;
                case cssi::NumberFormatIndex::BOOLEAN :
                    incErrorInt( "Error: FormatElement formatindex=\"%d\" reserved for internal ``BOOLEAN''.\n", nNext);
                    break;
                case cssi::NumberFormatIndex::TEXT :
                    incErrorInt( "Error: FormatElement formatindex=\"%d\" reserved for internal ``@'' (TEXT).\n", nNext);
                    break;
                default:
                    ;   // nothing
            }
        }

        if (!bHaveEngineering)
            incError("Engineering notation format not present, e.g. ##0.00E+00 or ##0,00E+00 for usage=\"SCIENTIFIC_NUMBER\"\n");
    }

    of.writeAsciiString("\nstatic const sal_Int16 ");
    of.writeAsciiString("FormatElementsCount");
    of.writeInt(mnSection);
    of.writeAsciiString(" = ");
    of.writeInt( formatCount - mnFormats);
    of.writeAsciiString(";\n");
    of.writeAsciiString("static constexpr OUString ");
    of.writeAsciiString("FormatElementsArray");
    of.writeInt(mnSection);
    of.writeAsciiString("[] = {\n");
    for(sal_Int16 i = mnFormats; i < formatCount; i++) {

        of.writeAsciiString("\t");
        of.writeAsciiString("FormatCode");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\t");
        of.writeAsciiString("FormatDefaultName");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\t");
        of.writeAsciiString("FormatKey");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\t");
        of.writeAsciiString("FormatType");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\t");
        of.writeAsciiString("FormatUsage");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\t");
        of.writeAsciiString("Formatindex");
        of.writeInt(i);
        of.writeAsciiString(",\n");


        of.writeAsciiString("\tdefaultFormatElement");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

    switch (mnSection)
    {
        case 0:
            of.writeOUStringFunction("getAllFormats0_", "FormatElementsCount0", "FormatElementsArray0", "replaceFrom0", "replaceTo0");
            break;
        case 1:
            of.writeOUStringFunction("getAllFormats1_", "FormatElementsCount1", "FormatElementsArray1", "replaceFrom1", "replaceTo1");
            break;
    }

    mnFormats = mnFormats + formatCount;

    if (mnSection == 0)
    {
        // Extract and add date acceptance pattern for full date, so we provide
        // at least one valid pattern, even if the number parser doesn't need
        // that one.
        /* XXX NOTE: only simple [...] modifier and "..." quotes detected and
         * ignored, not nested, no fancy stuff. */
        // aDateSep can be empty if LC_CTYPE was a ref=..., determine from
        // FormatCode then.
        sal_uInt32 cDateSep = (aDateSep.isEmpty()
            ? 0 : aDateSep.iterateCodePoints( &o3tl::temporary(sal_Int32(0))));
        sal_uInt32 cDateSep2 = cDateSep;
        sal_Int32 nIndex = 0;
        OUStringBuffer aPatternBuf(5);
        OUStringBuffer aPatternBuf2(5);
        sal_uInt8 nDetected = 0;    // bits Y,M,D
        bool bInModifier = false;
        bool bQuoted = false;
        while (nIndex < sTheDateEditFormat.getLength() && nDetected < 7)
        {
            sal_uInt32 cChar = sTheDateEditFormat.iterateCodePoints( &nIndex);
            if (bInModifier)
            {
                if (cChar == ']')
                    bInModifier = false;
                continue;   // while
            }
            if (bQuoted)
            {
                if (cChar == '"')
                    bQuoted = false;
                continue;   // while
            }
            switch (cChar)
            {
                case 'Y':
                case 'y':
                    if (!(nDetected & 4))
                    {
                        aPatternBuf.append( 'Y');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'Y');
                        nDetected |= 4;
                    }
                    break;
                case 'M':
                case 'm':
                    if (!(nDetected & 2))
                    {
                        aPatternBuf.append( 'M');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'M');
                        nDetected |= 2;
                    }
                    break;
                case 'D':
                case 'd':
                    if (!(nDetected & 1))
                    {
                        aPatternBuf.append( 'D');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'D');
                        nDetected |= 1;
                    }
                    break;
                case '[':
                    bInModifier = true;
                    break;
                case '"':
                    bQuoted = true;
                    break;
                case '\\':
                    cChar = sTheDateEditFormat.iterateCodePoints( &nIndex);
                    goto handleDefault;
                case '-':
                case '.':
                case '/':
                    // There are locales that use an ISO 8601 edit format
                    // regardless of what the date separator or other formats
                    // say, for example hu-HU. Generalize this for all cases
                    // where the used separator differs and is one of the known
                    // separators and generate a second pattern with the
                    // format's separator at the current position.
                    cDateSep2 = cChar;
                    [[fallthrough]];
                default:
                handleDefault:
                    if (!cDateSep)
                        cDateSep = cChar;
                    if (!cDateSep2)
                        cDateSep2 = cChar;
                    if (cDateSep != cDateSep2 && aPatternBuf2.isEmpty())
                        aPatternBuf2 = aPatternBuf;
                    if (cChar == cDateSep || cChar == cDateSep2)
                        aPatternBuf.append( OUString( &cDateSep, 1));   // always the defined separator
                    if (cChar == cDateSep2 && !aPatternBuf2.isEmpty())
                        aPatternBuf2.append( OUString( &cDateSep2, 1)); // always the format's separator
                    break;
                // The localized legacy:
                case 'A':
                    if (((nDetected & 7) == 3) || ((nDetected & 7) == 0))
                    {
                        // es DD/MM/AAAA
                        // fr JJ.MM.AAAA
                        // it GG/MM/AAAA
                        // fr_CA AAAA-MM-JJ
                        aPatternBuf.append( 'Y');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'Y');
                        nDetected |= 4;
                    }
                    break;
                case 'J':
                    if (((nDetected & 7) == 0) || ((nDetected & 7) == 6))
                    {
                        // fr JJ.MM.AAAA
                        // fr_CA AAAA-MM-JJ
                        aPatternBuf.append( 'D');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'D');
                        nDetected |= 1;
                    }
                    else if ((nDetected & 7) == 3)
                    {
                        // nl DD-MM-JJJJ
                        // de TT.MM.JJJJ
                        aPatternBuf.append( 'Y');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'Y');
                        nDetected |= 4;
                    }
                    break;
                case 'T':
                    if ((nDetected & 7) == 0)
                    {
                        // de TT.MM.JJJJ
                        aPatternBuf.append( 'D');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'D');
                        nDetected |= 1;
                    }
                    break;
                case 'G':
                    if ((nDetected & 7) == 0)
                    {
                        // it GG/MM/AAAA
                        aPatternBuf.append( 'D');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'D');
                        nDetected |= 1;
                    }
                    break;
                case 'P':
                    if ((nDetected & 7) == 0)
                    {
                        // fi PP.KK.VVVV
                        aPatternBuf.append( 'D');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'D');
                        nDetected |= 1;
                    }
                    break;
                case 'K':
                    if ((nDetected & 7) == 1)
                    {
                        // fi PP.KK.VVVV
                        aPatternBuf.append( 'M');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'M');
                        nDetected |= 2;
                    }
                    break;
                case 'V':
                    if ((nDetected & 7) == 3)
                    {
                        // fi PP.KK.VVVV
                        aPatternBuf.append( 'Y');
                        if (!aPatternBuf2.isEmpty())
                            aPatternBuf2.append( 'Y');
                        nDetected |= 4;
                    }
                    break;
            }
        }
        OUString aPattern( aPatternBuf.makeStringAndClear());
        if (((nDetected & 7) != 7) || aPattern.getLength() < 5)
        {
            incErrorStr( "Error: failed to extract full date acceptance pattern: %s\n", aPattern);
            fprintf( stderr, "       with DateSeparator '%s' from FormatCode '%s' (formatindex=\"%d\")\n",
                    OSTR( OUString(&cDateSep, 1)), OSTR( sTheDateEditFormat),
                    int(cssi::NumberFormatIndex::DATE_SYS_DDMMYYYY));
        }
        else
        {
            fprintf( stderr, "Generated date acceptance pattern: '%s' from '%s' (formatindex=\"%d\" and defined DateSeparator '%s')\n",
                    OSTR( aPattern), OSTR( sTheDateEditFormat),
                    int(cssi::NumberFormatIndex::DATE_SYS_DDMMYYYY),
                    OSTR( OUString(&cDateSep, 1)));
            // Insert at front so full date pattern is first in checks.
            theDateAcceptancePatterns.insert( theDateAcceptancePatterns.begin(), aPattern);
        }
        if (!aPatternBuf2.isEmpty())
        {
            OUString aPattern2( aPatternBuf2.makeStringAndClear());
            if (aPattern2.getLength() < 5)
            {
                incErrorStr( "Error: failed to extract 2nd date acceptance pattern: %s\n", aPattern2);
                fprintf( stderr, "       with DateSeparator '%s' from FormatCode '%s' (formatindex=\"%d\")\n",
                        OSTR( OUString(&cDateSep2, 1)), OSTR( sTheDateEditFormat),
                        int(cssi::NumberFormatIndex::DATE_SYS_DDMMYYYY));
            }
            else
            {
                fprintf( stderr, "Generated  2nd acceptance pattern: '%s' from '%s' (formatindex=\"%d\")\n",
                        OSTR( aPattern2), OSTR( sTheDateEditFormat),
                        int(cssi::NumberFormatIndex::DATE_SYS_DDMMYYYY));
                theDateAcceptancePatterns.insert( theDateAcceptancePatterns.begin(), aPattern2);
            }
        }

        // Rudimentary check if a pattern interferes with decimal number.
        // But only if not inherited in which case we don't have aDecSep here.
        if (!aDecSep.isEmpty())
        {
            sal_uInt32 cDecSep = aDecSep.iterateCodePoints( &o3tl::temporary(sal_Int32(0)));
            for (auto const& elem : theDateAcceptancePatterns)
            {
                if (elem.getLength() == (cDecSep <= 0xffff ? 3 : 4))
                {
                    if (elem.iterateCodePoints( &o3tl::temporary(sal_Int32(1))) == cDecSep)
                    {
                        ++nError;
                        fprintf( stderr, "Error: Date acceptance pattern '%s' matches decimal number '#%s#'\n",
                                OSTR(elem), OSTR( aDecSep));
                    }
                }
            }
        }

        // Check for duplicates.
        for (std::vector<OUString>::const_iterator aIt = theDateAcceptancePatterns.begin();
                aIt != theDateAcceptancePatterns.end(); ++aIt)
        {
            for (std::vector<OUString>::iterator aComp = theDateAcceptancePatterns.begin();
                    aComp != theDateAcceptancePatterns.end(); /*nop*/)
            {
                if (aIt != aComp && *aIt == *aComp)
                {
                    incErrorStr( "Error: Duplicated DateAcceptancePattern: %s\n", *aComp);
                    aComp = theDateAcceptancePatterns.erase( aComp);
                }
                else
                    ++aComp;
            }
        }

        sal_Int16 nbOfDateAcceptancePatterns = static_cast<sal_Int16>(theDateAcceptancePatterns.size());

        for (sal_Int16 i = 0; i < nbOfDateAcceptancePatterns; ++i)
        {
            of.writeOUStringLiteralParameter("DateAcceptancePattern", theDateAcceptancePatterns[i], i);
        }

        of.writeAsciiString("static const sal_Int16 DateAcceptancePatternsCount = ");
        of.writeInt( nbOfDateAcceptancePatterns);
        of.writeAsciiString(";\n");

        of.writeAsciiString("static constexpr OUString DateAcceptancePatternsArray[] = {\n");
        for (sal_Int16 i = 0; i < nbOfDateAcceptancePatterns; ++i)
        {
            of.writeAsciiString("\t");
            of.writeAsciiString("DateAcceptancePattern");
            of.writeInt(i);
            of.writeAsciiString(",\n");
        }
        of.writeAsciiString("};\n\n");

        of.writeOUStringFunction("getDateAcceptancePatterns_", "DateAcceptancePatternsCount", "DateAcceptancePatternsArray");
    }

    ++mnSection;
}

void LCCollationNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getCollatorImplementation_", useLocale);
        of.writeOUStringRefFunction("getCollationOptions_", useLocale);
        return;
    }
    sal_Int16 nbOfCollations = 0;
    sal_Int16 nbOfCollationOptions = 0;

    for ( sal_Int32 j = 0; j < getNumberOfChildren(); j++ ) {
        LocaleNode * currNode = getChildAt (j);
        if( currNode->getName() == "Collator" )
        {
            OUString str;
            str = currNode->getAttr().getValueByName("unoid");
            of.writeOUStringLiteralParameter("CollatorID", str, j);
            str = currNode->getValue();
            of.writeOUStringLiteralParameter("CollatorRule", str, j);
            str = currNode -> getAttr().getValueByName("default");
            of.writeOUStringLiteralDefaultParameter("Collator", str, j);
            of.writeAsciiString("\n");

            nbOfCollations++;
        }
        if( currNode->getName() == "CollationOptions" )
        {
            LocaleNode* pCollationOptions = currNode;
            nbOfCollationOptions = sal::static_int_cast<sal_Int16>( pCollationOptions->getNumberOfChildren() );
            for( sal_Int16 i=0; i<nbOfCollationOptions; i++ )
            {
                of.writeOUStringLiteralParameter("collationOption", pCollationOptions->getChildAt( i )->getValue(), i );
            }

            of.writeAsciiString("static const sal_Int16 nbOfCollationOptions = ");
            of.writeInt( nbOfCollationOptions );
            of.writeAsciiString(";\n\n");
        }
    }
    of.writeAsciiString("static const sal_Int16 nbOfCollations = ");
    of.writeInt(nbOfCollations);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("\nstatic constexpr OUString LCCollatorArray[] = {\n");
    for(sal_Int16 j = 0; j < nbOfCollations; j++) {
        of.writeAsciiString("\tCollatorID");
        of.writeInt(j);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tdefaultCollator");
        of.writeInt(j);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tCollatorRule");
        of.writeInt(j);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

    of.writeAsciiString("static constexpr OUString collationOptions[] = {");
    for( sal_Int16 j=0; j<nbOfCollationOptions; j++ )
    {
        if (j)
            of.writeAsciiString( ", " );
        of.writeAsciiString( "collationOption" );
        of.writeInt( j );
    }
    of.writeAsciiString("};\n");
    of.writeOUStringFunction("getCollatorImplementation_", "nbOfCollations", "LCCollatorArray");
    of.writeOUStringFunction("getCollationOptions_", "nbOfCollationOptions", "collationOptions");
}

void LCSearchNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getSearchOptions_", useLocale);
        return;
    }

    if( getNumberOfChildren() != 1 )
    {
        ++nError;
        fprintf(
            stderr, "Error: LC_SEARCH: more than 1 child: %" SAL_PRIdINT32 "\n",
            getNumberOfChildren());
    }
    sal_Int32 i;
    LocaleNode* pSearchOptions = getChildAt( 0 );
    sal_Int32   nSearchOptions = pSearchOptions->getNumberOfChildren();
    for( i=0; i<nSearchOptions; i++ )
    {
        of.writeOUStringLiteralParameter("searchOption", pSearchOptions->getChildAt( i )->getValue(), sal::static_int_cast<sal_Int16>(i) );
    }

    of.writeAsciiString("static const sal_Int16 nbOfSearchOptions = ");
    of.writeInt( sal::static_int_cast<sal_Int16>( nSearchOptions ) );
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("static constexpr OUString searchOptions[] = {");
    for( i=0; i<nSearchOptions; i++ )
    {
        if (i)
            of.writeAsciiString( ", " );
        of.writeAsciiString( "searchOption" );
        of.writeInt( sal::static_int_cast<sal_Int16>(i) );
    }
    of.writeAsciiString(" };\n");
    of.writeOUStringFunction("getSearchOptions_", "nbOfSearchOptions", "searchOptions");
}

void LCIndexNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getIndexAlgorithm_", useLocale);
        of.writeOUStringRefFunction("getUnicodeScripts_", useLocale);
        of.writeOUStringRefFunction("getFollowPageWords_", useLocale);
        return;
    }
    sal_Int16 nbOfIndexs = 0;
    sal_Int16 nbOfUnicodeScripts = 0;
    sal_Int16 nbOfPageWords = 0;
    for (sal_Int32 i = 0; i< getNumberOfChildren();i++) {
        LocaleNode * currNode = getChildAt (i);
        if( currNode->getName() == "IndexKey" )
        {
            OUString str;
            str = currNode->getAttr().getValueByName("unoid");
            of.writeOUStringLiteralParameter("IndexID", str, nbOfIndexs);
            str = currNode->getAttr().getValueByName("module");
            of.writeOUStringLiteralParameter("IndexModule", str, nbOfIndexs);
            str = currNode->getValue();
            of.writeOUStringLiteralParameter("IndexKey", str, nbOfIndexs);
            str = currNode -> getAttr().getValueByName("default");
            of.writeOUStringLiteralDefaultParameter("Index", str, nbOfIndexs);
            str = currNode -> getAttr().getValueByName("phonetic");
            of.writeOUStringLiteralDefaultParameter("Phonetic", str, nbOfIndexs);
            of.writeAsciiString("\n");

            nbOfIndexs++;
        }
        if( currNode->getName() == "UnicodeScript" )
        {
            of.writeOUStringLiteralParameter("unicodeScript", currNode->getValue(), nbOfUnicodeScripts );
            nbOfUnicodeScripts++;

        }
        if( currNode->getName() == "FollowPageWord" )
        {
            of.writeOUStringLiteralParameter("followPageWord", currNode->getValue(), nbOfPageWords);
            nbOfPageWords++;
        }
    }
    of.writeAsciiString("static const sal_Int16 nbOfIndexs = ");
    of.writeInt(nbOfIndexs);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("\nstatic constexpr OUString IndexArray[] = {\n");
    for(sal_Int16 i = 0; i < nbOfIndexs; i++) {
        of.writeAsciiString("\tIndexID");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tIndexModule");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tIndexKey");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tdefaultIndex");
        of.writeInt(i);
        of.writeAsciiString(",\n");

        of.writeAsciiString("\tdefaultPhonetic");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");

    of.writeAsciiString("static const sal_Int16 nbOfUnicodeScripts = ");
    of.writeInt( nbOfUnicodeScripts );
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("static constexpr OUString UnicodeScriptArray[] = {");
    for( sal_Int16 i=0; i<nbOfUnicodeScripts; i++ )
    {
        if (i)
            of.writeAsciiString( ", " );
        of.writeAsciiString( "unicodeScript" );
        of.writeInt( i );
    }
    of.writeAsciiString(" };\n\n");

    of.writeAsciiString("static const sal_Int16 nbOfPageWords = ");
    of.writeInt(nbOfPageWords);
    of.writeAsciiString(";\n\n");

    // MSVC doesn't like zero sized arrays
    if (nbOfPageWords == 0)
    {
        // generate dummy array, reuse unicodeScript0 for dummy entry
        of.writeAsciiString("//dummy array, we have zero entries\n");
        of.writeAsciiString("static constexpr OUString FollowPageWordArray[] = { unicodeScript0 };\n\n");
    }
    else
    {
        of.writeAsciiString("static constexpr OUString FollowPageWordArray[] = {\n");
        for(sal_Int16 i = 0; i < nbOfPageWords; i++) {
            if (i)
                of.writeAsciiString(",\n");
            of.writeAsciiString("\tfollowPageWord");
            of.writeInt(i);
        }
        of.writeAsciiString("\t\n};\n\n");
    }
    of.writeOUStringFunction("getIndexAlgorithm_", "nbOfIndexs", "IndexArray");
    of.writeOUStringFunction("getUnicodeScripts_", "nbOfUnicodeScripts", "UnicodeScriptArray");
    of.writeOUStringFunction("getFollowPageWords_", "nbOfPageWords", "FollowPageWordArray");
}


static void lcl_writeAbbrFullNarrNames( const OFileWriter & of, const LocaleNode* currNode,
        const char* elementTag, sal_Int16 i, sal_Int16 j )
{
    OUString aAbbrName = currNode->getChildAt(1)->getValue();
    OUString aFullName = currNode->getChildAt(2)->getValue();
    OUString aNarrName;
    LocaleNode* p = (currNode->getNumberOfChildren() > 3 ? currNode->getChildAt(3) : nullptr);
    if ( p && p->getName() == "DefaultNarrowName" )
        aNarrName = p->getValue();
    else
    {
        sal_uInt32 nChar = aFullName.iterateCodePoints( &o3tl::temporary(sal_Int32(0)));
        aNarrName = OUString( &nChar, 1);
    }
    of.writeOUStringLiteralParameter( elementTag, "DefaultAbbrvName",  aAbbrName, i, j);
    of.writeOUStringLiteralParameter( elementTag, "DefaultFullName",   aFullName, i, j);
    of.writeOUStringLiteralParameter( elementTag, "DefaultNarrowName", aNarrName, i, j);
}

static void lcl_writeTabTagString( const OFileWriter & of, const char* pTag, const char* pStr )
{
    of.writeAsciiString("\t");
    of.writeAsciiString( pTag);
    of.writeAsciiString( pStr);
}

static void lcl_writeTabTagStringNums( const OFileWriter & of,
        const char* pTag, const char* pStr, sal_Int16 i, sal_Int16 j )
{
    lcl_writeTabTagString( of, pTag, pStr);
    of.writeInt(i); of.writeInt(j); of.writeAsciiString(",\n");
}

static void lcl_writeAbbrFullNarrArrays( const OFileWriter & of, sal_Int16 nCount,
        const char* elementTag, sal_Int16 i, bool bNarrow )
{
    if (nCount == 0)
    {
        lcl_writeTabTagString( of, elementTag, "Ref");
        of.writeInt(i); of.writeAsciiString(",\n");
        lcl_writeTabTagString( of, elementTag, "RefName");
        of.writeInt(i); of.writeAsciiString(",\n");
    }
    else
    {
        for (sal_Int16 j = 0; j < nCount; j++)
        {
            lcl_writeTabTagStringNums( of, elementTag, "ID", i, j);
            lcl_writeTabTagStringNums( of, elementTag, "DefaultAbbrvName",  i, j);
            lcl_writeTabTagStringNums( of, elementTag, "DefaultFullName",   i, j);
            if (bNarrow)
                lcl_writeTabTagStringNums( of, elementTag, "DefaultNarrowName", i, j);
        }
    }
}

bool LCCalendarNode::expectedCalendarElement( std::u16string_view rName,
        const LocaleNode* pNode, sal_Int16 nChild, std::u16string_view rCalendarID ) const
{
    bool bFound = true;
    if (nChild >= 0)
    {
        if (nChild >= pNode->getNumberOfChildren())
            bFound = false;
        else
            pNode = pNode->getChildAt(nChild);
    }
    if (bFound && (!pNode || pNode->getName() != rName))
        bFound = false;
    if (!bFound)
        incErrorStrStr( "Error: <%s> element expected in calendar '%s'\n", rName, rCalendarID);
    return bFound;
}

void LCCalendarNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getAllCalendars_", useLocale);
        return;
    }
    sal_Int16 nbOfCalendars = sal::static_int_cast<sal_Int16>( getNumberOfChildren() );
    OUString str;
    std::unique_ptr<sal_Int16[]> nbOfDays( new sal_Int16[nbOfCalendars] );
    std::unique_ptr<sal_Int16[]> nbOfMonths( new sal_Int16[nbOfCalendars] );
    std::unique_ptr<sal_Int16[]> nbOfGenitiveMonths( new sal_Int16[nbOfCalendars] );
    std::unique_ptr<sal_Int16[]> nbOfPartitiveMonths( new sal_Int16[nbOfCalendars] );
    std::unique_ptr<sal_Int16[]> nbOfEras( new sal_Int16[nbOfCalendars] );

    // Known allowed calendar identifiers (unoid) and whether used or not.
    // Of course there must be an implementation for new to be added
    // identifiers.. see data/locale.dtd
    std::map< OUString, bool > aCalendars;
    aCalendars[u"buddhist"_ustr]   = false;
    aCalendars[u"gengou"_ustr]     = false;
    aCalendars[u"gregorian"_ustr]  = false;
    aCalendars[u"hanja"_ustr]      = false;
    aCalendars[u"hanja_yoil"_ustr] = false;
    aCalendars[u"hijri"_ustr]      = false;
    aCalendars[u"jewish"_ustr]     = false;
    aCalendars[u"ROC"_ustr]        = false;
    // Not in ODF:
    aCalendars[u"dangi"_ustr]      = false;
    aCalendars[u"persian"_ustr]    = false;

    sal_Int16 j;
    sal_Int16 i;
    bool bHasGregorian = false;


    for ( i = 0; i < nbOfCalendars; i++) {
        LocaleNode * calNode = getChildAt (i);
        OUString calendarID = calNode -> getAttr().getValueByName("unoid");
        of.writeOUStringLiteralParameter( "calendarID", calendarID, i);
        bool bGregorian = calendarID == "gregorian";
        if (!bHasGregorian)
            bHasGregorian = bGregorian;
        auto calIt = aCalendars.find(calendarID);
        if (calIt == aCalendars.end())
            incErrorStr( "Error: unknown Calendar unoid: %s\n", calendarID);
        else if (calIt->second)
            incErrorStr( "Error: duplicate Calendar unoid: %s\n", calendarID);
        else
            calIt->second = true;
        str = calNode -> getAttr().getValueByName("default");
        of.writeOUStringLiteralDefaultParameter("Calendar", str, i);

        sal_Int16 nChild = 0;

        // Generate Days of Week
        const char *elementTag;
        LocaleNode * daysNode = nullptr;
        OUString ref_name = calNode->getChildAt(nChild)->getAttr().getValueByName("ref");
        ref_name = ref_name.replace( '-', '_');
        if (!ref_name.isEmpty() && i > 0) {
            for (j = 0; j < i; j++) {
                str = getChildAt(j)->getAttr().getValueByName("unoid");
                if (str == ref_name)
                    daysNode = getChildAt(j)->getChildAt(0);
            }
        }
        if (!ref_name.isEmpty() && daysNode == nullptr) {
            of.writeOUStringLiteralParameter("dayRef", u"ref", i);
            of.writeOUStringLiteralParameter("dayRefName", ref_name, i);
            nbOfDays[i] = 0;
        } else {
            if (daysNode == nullptr)
                daysNode = calNode -> getChildAt(nChild);
            nbOfDays[i] = sal::static_int_cast<sal_Int16>( daysNode->getNumberOfChildren() );
            if (bGregorian && nbOfDays[i] != 7)
                incErrorInt( "Error: A Gregorian calendar must have 7 days per week, this one has %d\n", nbOfDays[i]);
            elementTag = "day";
            for (j = 0; j < nbOfDays[i]; j++) {
                LocaleNode *currNode = daysNode -> getChildAt(j);
                OUString dayID( currNode->getChildAt(0)->getValue());
                of.writeOUStringLiteralParameter("dayID", dayID, i, j);
                if ( j == 0 && bGregorian && dayID != "sun" )
                    incError( "First day of a week of a Gregorian calendar must be <DayID>sun</DayID>");
                lcl_writeAbbrFullNarrNames( of, currNode, elementTag, i, j);
            }
        }
        ++nChild;

        // Generate Months of Year
        LocaleNode * monthsNode = nullptr;
        ref_name = calNode->getChildAt(nChild)->getAttr().getValueByName("ref");
        ref_name = ref_name.replace( '-', '_');
        if (!ref_name.isEmpty() && i > 0) {
            for (j = 0; j < i; j++) {
                str = getChildAt(j)->getAttr().getValueByName("unoid");
                if (str == ref_name)
                    monthsNode = getChildAt(j)->getChildAt(1);
            }
        }
        if (!ref_name.isEmpty() && monthsNode == nullptr) {
            of.writeOUStringLiteralParameter("monthRef", u"ref", i);
            of.writeOUStringLiteralParameter("monthRefName", ref_name, i);
            nbOfMonths[i] = 0;
        } else {
            if (monthsNode == nullptr)
                monthsNode = calNode -> getChildAt(nChild);
            nbOfMonths[i] = sal::static_int_cast<sal_Int16>( monthsNode->getNumberOfChildren() );
            if (bGregorian && nbOfMonths[i] != 12)
                incErrorInt( "Error: A Gregorian calendar must have 12 months, this one has %d\n", nbOfMonths[i]);
            elementTag = "month";
            for (j = 0; j < nbOfMonths[i]; j++) {
                LocaleNode *currNode = monthsNode -> getChildAt(j);
                OUString monthID( currNode->getChildAt(0)->getValue());
                of.writeOUStringLiteralParameter("monthID", monthID, i, j);
                if ( j == 0 && bGregorian && monthID != "jan" )
                    incError( "First month of a year of a Gregorian calendar must be <MonthID>jan</MonthID>");
                lcl_writeAbbrFullNarrNames( of, currNode, elementTag, i, j);
            }
        }
        ++nChild;

        // Generate genitive Months of Year
        // Optional, if not present fall back to month nouns.
        if ( calNode->getChildAt(nChild)->getName() != "GenitiveMonths" )
            --nChild;
        LocaleNode * genitiveMonthsNode = nullptr;
        ref_name = calNode->getChildAt(nChild)->getAttr().getValueByName("ref");
        ref_name = ref_name.replace( '-', '_');
        if (!ref_name.isEmpty() && i > 0) {
            for (j = 0; j < i; j++) {
                str = getChildAt(j)->getAttr().getValueByName("unoid");
                if (str == ref_name)
                    genitiveMonthsNode = getChildAt(j)->getChildAt(1);
            }
        }
        if (!ref_name.isEmpty() && genitiveMonthsNode == nullptr) {
            of.writeOUStringLiteralParameter("genitiveMonthRef", u"ref", i);
            of.writeOUStringLiteralParameter("genitiveMonthRefName", ref_name, i);
            nbOfGenitiveMonths[i] = 0;
        } else {
            if (genitiveMonthsNode == nullptr)
                genitiveMonthsNode = calNode -> getChildAt(nChild);
            nbOfGenitiveMonths[i] = sal::static_int_cast<sal_Int16>( genitiveMonthsNode->getNumberOfChildren() );
            if (bGregorian && nbOfGenitiveMonths[i] != 12)
                incErrorInt( "Error: A Gregorian calendar must have 12 genitive months, this one has %d\n", nbOfGenitiveMonths[i]);
            elementTag = "genitiveMonth";
            for (j = 0; j < nbOfGenitiveMonths[i]; j++) {
                LocaleNode *currNode = genitiveMonthsNode -> getChildAt(j);
                OUString genitiveMonthID( currNode->getChildAt(0)->getValue());
                of.writeOUStringLiteralParameter("genitiveMonthID", genitiveMonthID, i, j);
                if ( j == 0 && bGregorian && genitiveMonthID != "jan" )
                    incError( "First genitive month of a year of a Gregorian calendar must be <MonthID>jan</MonthID>");
                lcl_writeAbbrFullNarrNames( of, currNode, elementTag, i, j);
            }
        }
        ++nChild;

        // Generate partitive Months of Year
        // Optional, if not present fall back to genitive months, or nominative
        // months (nouns) if that isn't present either.
        if ( calNode->getChildAt(nChild)->getName() != "PartitiveMonths" )
            --nChild;
        LocaleNode * partitiveMonthsNode = nullptr;
        ref_name = calNode->getChildAt(nChild)->getAttr().getValueByName("ref");
        ref_name = ref_name.replace( '-', '_');
        if (!ref_name.isEmpty() && i > 0) {
            for (j = 0; j < i; j++) {
                str = getChildAt(j)->getAttr().getValueByName("unoid");
                if (str == ref_name)
                    partitiveMonthsNode = getChildAt(j)->getChildAt(1);
            }
        }
        if (!ref_name.isEmpty() && partitiveMonthsNode == nullptr) {
            of.writeOUStringLiteralParameter("partitiveMonthRef", u"ref", i);
            of.writeOUStringLiteralParameter("partitiveMonthRefName", ref_name, i);
            nbOfPartitiveMonths[i] = 0;
        } else {
            if (partitiveMonthsNode == nullptr)
                partitiveMonthsNode = calNode -> getChildAt(nChild);
            nbOfPartitiveMonths[i] = sal::static_int_cast<sal_Int16>( partitiveMonthsNode->getNumberOfChildren() );
            if (bGregorian && nbOfPartitiveMonths[i] != 12)
                incErrorInt( "Error: A Gregorian calendar must have 12 partitive months, this one has %d\n", nbOfPartitiveMonths[i]);
            elementTag = "partitiveMonth";
            for (j = 0; j < nbOfPartitiveMonths[i]; j++) {
                LocaleNode *currNode = partitiveMonthsNode -> getChildAt(j);
                OUString partitiveMonthID( currNode->getChildAt(0)->getValue());
                of.writeOUStringLiteralParameter("partitiveMonthID", partitiveMonthID, i, j);
                if ( j == 0 && bGregorian && partitiveMonthID != "jan" )
                    incError( "First partitive month of a year of a Gregorian calendar must be <MonthID>jan</MonthID>");
                lcl_writeAbbrFullNarrNames( of, currNode, elementTag, i, j);
            }
        }
        ++nChild;

        // Generate Era name
        LocaleNode * erasNode = nullptr;
        ref_name =   calNode -> getChildAt(nChild) ->getAttr().getValueByName("ref");
        ref_name = ref_name.replace( '-', '_');
        if (!ref_name.isEmpty() && i > 0) {
            for (j = 0; j < i; j++) {
                str = getChildAt(j)->getAttr().getValueByName("unoid");
                if (str == ref_name)
                    erasNode = getChildAt(j)->getChildAt(2);
            }
        }
        if (!ref_name.isEmpty() && erasNode == nullptr) {
            of.writeOUStringLiteralParameter("eraRef", u"ref", i);
            of.writeOUStringLiteralParameter("eraRefName", ref_name, i);
            nbOfEras[i] = 0;
        } else {
            if (erasNode == nullptr)
                erasNode = calNode -> getChildAt(nChild);
            if (!expectedCalendarElement(u"Eras", erasNode, -1, calendarID))
            {
                --nChild;
            }
            else
            {
                nbOfEras[i] = sal::static_int_cast<sal_Int16>( erasNode->getNumberOfChildren() );
                if (bGregorian && nbOfEras[i] != 2)
                    incErrorInt( "Error: A Gregorian calendar must have 2 eras, this one has %d\n", nbOfEras[i]);
                elementTag = "era";
                for (j = 0; j < nbOfEras[i]; j++) {
                    LocaleNode *currNode = erasNode -> getChildAt(j);
                    if (!expectedCalendarElement(u"Era", currNode, -1, calendarID))
                    {
                        continue;   // for
                    }
                    OUString eraID( currNode->getChildAt(0)->getValue());
                    of.writeOUStringLiteralParameter("eraID", eraID, i, j);
                    if ( j == 0 && bGregorian && eraID != "bc" )
                        incError( "First era of a Gregorian calendar must be <EraID>bc</EraID>");
                    if ( j == 1 && bGregorian && eraID != "ad" )
                        incError( "Second era of a Gregorian calendar must be <EraID>ad</EraID>");
                    of.writeAsciiString("\n");
                    of.writeOUStringLiteralParameter(elementTag, "DefaultAbbrvName",currNode->getChildAt(1)->getValue() ,i, j);
                    of.writeOUStringLiteralParameter(elementTag, "DefaultFullName",currNode->getChildAt(2)->getValue() , i, j);
                }
            }
        }
        ++nChild;

        if (expectedCalendarElement(u"StartDayOfWeek", calNode, nChild, calendarID))
        {
            str = calNode->getChildAt(nChild)->getChildAt(0)->getValue();
            if (nbOfDays[i])
            {
                for (j = 0; j < nbOfDays[i]; j++)
                {
                    LocaleNode *currNode = daysNode->getChildAt(j);
                    OUString dayID( currNode->getChildAt(0)->getValue());
                    if (str == dayID)
                        break;  // for
                }
                if (j >= nbOfDays[i])
                    incErrorStr( "Error: <StartDayOfWeek> <DayID> must be one of the <DaysOfWeek>, but is: %s\n", str);
            }
            of.writeOUStringLiteralParameter("startDayOfWeek", str, i);
            ++nChild;
        }

        if (expectedCalendarElement(u"MinimalDaysInFirstWeek", calNode, nChild, calendarID))
        {
            str = calNode ->getChildAt(nChild)-> getValue();
            sal_Int16 nDays = sal::static_int_cast<sal_Int16>( str.toInt32() );
            if (nDays < 1 || (0 < nbOfDays[i] && nbOfDays[i] < nDays))
                incErrorInt( "Error: Bad value of MinimalDaysInFirstWeek: %d, must be 1 <= value <= days_in_week\n",
                        nDays);
            of.writeOUStringLiteralIntParameter("minimalDaysInFirstWeek", i, nDays);
        }
    }
    if (!bHasGregorian)
        fprintf( stderr, "Warning: %s\n", "No Gregorian calendar defined, are you sure?");

    of.writeAsciiString("static const sal_Int16 calendarsCount = ");
    of.writeInt(nbOfCalendars);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("static constexpr OUStringLiteral nbOfDays = u\"");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\\x");
        of.writeHexInt(nbOfDays[i]);
    }
    of.writeAsciiString("\";\n");

    of.writeAsciiString("static constexpr OUStringLiteral nbOfMonths = u\"");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\\x");
        of.writeHexInt(nbOfMonths[i]);
    }
    of.writeAsciiString("\";\n");

    of.writeAsciiString("static constexpr OUStringLiteral nbOfGenitiveMonths = u\"");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\\x");
        of.writeHexInt(nbOfGenitiveMonths[i]);
    }
    of.writeAsciiString("\";\n");

    of.writeAsciiString("static constexpr OUStringLiteral nbOfPartitiveMonths = u\"");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\\x");
        of.writeHexInt(nbOfPartitiveMonths[i]);
    }
    of.writeAsciiString("\";\n");

    of.writeAsciiString("static constexpr OUStringLiteral nbOfEras = u\"");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\\x");
        of.writeHexInt(nbOfEras[i]);
    }
    of.writeAsciiString("\";\n");


    of.writeAsciiString("static constexpr OUString calendars[] = {\n");
    of.writeAsciiString("\tnbOfDays,\n");
    of.writeAsciiString("\tnbOfMonths,\n");
    of.writeAsciiString("\tnbOfGenitiveMonths,\n");
    of.writeAsciiString("\tnbOfPartitiveMonths,\n");
    of.writeAsciiString("\tnbOfEras,\n");
    for(i = 0; i < nbOfCalendars; i++) {
        of.writeAsciiString("\tcalendarID");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCalendar");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        lcl_writeAbbrFullNarrArrays( of, nbOfDays[i], "day", i, true);
        lcl_writeAbbrFullNarrArrays( of, nbOfMonths[i], "month", i, true);
        lcl_writeAbbrFullNarrArrays( of, nbOfGenitiveMonths[i], "genitiveMonth", i, true);
        lcl_writeAbbrFullNarrArrays( of, nbOfPartitiveMonths[i], "partitiveMonth", i, true);
        lcl_writeAbbrFullNarrArrays( of, nbOfEras[i], "era", i, false /*noNarrow*/);
        of.writeAsciiString("\tstartDayOfWeek");of.writeInt(i); of.writeAsciiString(",\n");
        of.writeAsciiString("\tminimalDaysInFirstWeek");of.writeInt(i); of.writeAsciiString(",\n");
    }

    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getAllCalendars_", "calendarsCount", "calendars");
}

static bool isIso4217( const OUString& rStr )
{
    const sal_Unicode* p = rStr.getStr();
    return rStr.getLength() == 3
        && 'A' <= p[0] && p[0] <= 'Z'
        && 'A' <= p[1] && p[1] <= 'Z'
        && 'A' <= p[2] && p[2] <= 'Z'
        ;
}

void LCCurrencyNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getAllCurrencies_", useLocale);
        return;
    }
    sal_Int16 nbOfCurrencies = 0;
    OUString str;

    bool bTheDefault= false;
    bool bTheCompatible = false;
    for ( sal_Int32 i = 0; i < getNumberOfChildren(); i++,nbOfCurrencies++) {
        LocaleNode * currencyNode = getChildAt (i);
        str = currencyNode->getAttr().getValueByName("default");
        bool bDefault = of.writeOUStringLiteralDefaultParameter("Currency", str, nbOfCurrencies);
        str = currencyNode->getAttr().getValueByName("usedInCompatibleFormatCodes");
        bool bCompatible = of.writeOUStringLiteralDefaultParameter("CurrencyUsedInCompatibleFormatCodes", str, nbOfCurrencies);
        str = currencyNode->getAttr().getValueByName("legacyOnly");
        bool bLegacy = of.writeOUStringLiteralDefaultParameter("CurrencyLegacyOnly", str, nbOfCurrencies);
        if (bLegacy && (bDefault || bCompatible))
            incError( "Currency: if legacyOnly==true, both 'default' and 'usedInCompatibleFormatCodes' must be false.");
        if (bDefault)
        {
            if (bTheDefault)
                incError( "Currency: more than one default currency.");
            bTheDefault = true;
        }
        if (bCompatible)
        {
            if (bTheCompatible)
                incError( "Currency: more than one currency flagged as usedInCompatibleFormatCodes.");
            bTheCompatible = true;
        }
        str = currencyNode -> findNode ("CurrencyID") -> getValue();
        of.writeOUStringLiteralParameter("currencyID", str, nbOfCurrencies);
        // CurrencyID MUST be ISO 4217.
        if (!bLegacy && !isIso4217(str))
            incError( "CurrencyID is not ISO 4217");
        str = currencyNode -> findNode ("CurrencySymbol") -> getValue();
        of.writeOUStringLiteralParameter("currencySymbol", str, nbOfCurrencies);
        // Check if this currency really is the one used in number format
        // codes. In case of ref=... mechanisms it may be that TheCurrency
        // couldn't had been determined from the current locale (i.e. is
        // empty), silently assume the referred locale has things right.
        if (bCompatible && !sTheCompatibleCurrency.isEmpty() && sTheCompatibleCurrency != str)
            incErrorStrStr( "Error: CurrencySymbol \"%s\" flagged as usedInCompatibleFormatCodes doesn't match \"%s\" determined from format codes.\n", str, sTheCompatibleCurrency);
        str = currencyNode -> findNode ("BankSymbol") -> getValue();
        of.writeOUStringLiteralParameter("bankSymbol", str, nbOfCurrencies);
        // BankSymbol currently must be ISO 4217. May change later if
        // application always uses CurrencyID instead of BankSymbol.
        if (!bLegacy && !isIso4217(str))
            incError( "BankSymbol is not ISO 4217");
        str = currencyNode -> findNode ("CurrencyName") -> getValue();
        of.writeOUStringLiteralParameter("currencyName", str, nbOfCurrencies);
        str = currencyNode -> findNode ("DecimalPlaces") -> getValue();
        sal_Int16 nDecimalPlaces = static_cast<sal_Int16>(str.toInt32());
        of.writeOUStringLiteralIntParameter("currencyDecimalPlaces", nbOfCurrencies, nDecimalPlaces);
        of.writeAsciiString("\n");
    };

    if (!bTheDefault)
        incError( "Currency: no default currency.");
    if (!bTheCompatible)
        incError( "Currency: no currency flagged as usedInCompatibleFormatCodes.");

    of.writeAsciiString("static const sal_Int16 currencyCount = ");
    of.writeInt(nbOfCurrencies);
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("static constexpr OUString currencies[] = {\n");
    for(sal_Int16 i = 0; i < nbOfCurrencies; i++) {
        of.writeAsciiString("\tcurrencyID");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencySymbol");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tbankSymbol");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencyName");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCurrency");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCurrencyUsedInCompatibleFormatCodes");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tcurrencyDecimalPlaces");
        of.writeInt(i);
        of.writeAsciiString(",\n");
        of.writeAsciiString("\tdefaultCurrencyLegacyOnly");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getAllCurrencies_", "currencyCount", "currencies");
}

void LCTransliterationNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getTransliterations_", useLocale);
        return;
    }
    sal_Int16 nbOfModules = 0;
    OUString str;

    for ( sal_Int32 i = 0; i < getNumberOfChildren(); i++,nbOfModules++) {
        LocaleNode * transNode = getChildAt (i);
        str = transNode->getAttr().getValueByIndex(0);
        of.writeOUStringLiteralParameter("Transliteration", str, nbOfModules);
    }
    of.writeAsciiString("static const sal_Int16 nbOfTransliterations = ");
    of.writeInt(nbOfModules);
    of.writeAsciiString(";\n\n");

    of.writeAsciiString("\nstatic constexpr OUString LCTransliterationsArray[] = {\n");
    for( sal_Int16 i = 0; i < nbOfModules; i++) {
        of.writeAsciiString("\tTransliteration");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getTransliterations_", "nbOfTransliterations", "LCTransliterationsArray");
}

namespace {

struct NameValuePair {
    const char *name;
    const char *value;
};

}

const NameValuePair ReserveWord[] = {
    { "trueWord", "true" },
    { "falseWord", "false" },
    { "quarter1Word", "1st quarter" },
    { "quarter2Word", "2nd quarter" },
    { "quarter3Word", "3rd quarter" },
    { "quarter4Word", "4th quarter" },
    { "aboveWord", "above" },
    { "belowWord", "below" },
    { "quarter1Abbreviation", "Q1" },
    { "quarter2Abbreviation", "Q2" },
    { "quarter3Abbreviation", "Q3" },
    { "quarter4Abbreviation", "Q4" }
};

void LCMiscNode::generateCode (const OFileWriter &of) const
{
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction("getForbiddenCharacters_", useLocale);
        of.writeOUStringRefFunction("getBreakIteratorRules_", useLocale);
        of.writeOUStringRefFunction("getReservedWords_", useLocale);
        return;
    }
    const LocaleNode * reserveNode = findNode("ReservedWords");
    if (!reserveNode)
        incError( "No ReservedWords element."); // should not happen if validated...
    const LocaleNode * forbidNode = findNode("ForbiddenCharacters");
    const LocaleNode * breakNode = findNode("BreakIteratorRules");

    bool bEnglishLocale = (strncmp( of.getLocale(), "en_", 3) == 0);

    sal_Int16 nbOfWords = 0;
    OUString str;
    sal_Int16 i;

    for ( i = 0; i < sal_Int16(SAL_N_ELEMENTS(ReserveWord)); i++,nbOfWords++) {
        const LocaleNode * curNode = (reserveNode ? reserveNode->findNode(
                    ReserveWord[i].name) : nullptr);
        if (!curNode)
            fprintf( stderr,
                    "Warning: No %s in ReservedWords, using en_US default: \"%s\".\n",
                    ReserveWord[i].name, ReserveWord[i].value);
        str = curNode ? curNode -> getValue() : OUString::createFromAscii(ReserveWord[i].value);
        if (str.isEmpty())
        {
            ++nError;
            fprintf( stderr, "Error: No content for ReservedWords %s.\n", ReserveWord[i].name);
        }
        of.writeOUStringLiteralParameter("ReservedWord", str, nbOfWords);
        // "true", ..., "below" trigger untranslated warning.
        if (!bEnglishLocale && curNode && i <= 7 &&
                str.equalsIgnoreAsciiCaseAscii( ReserveWord[i].value))
        {
            fprintf( stderr,
                    "Warning: ReservedWord %s seems to be untranslated \"%s\".\n",
                    ReserveWord[i].name, ReserveWord[i].value);
        }
    }
    of.writeAsciiString("static const sal_Int16 nbOfReservedWords = ");
    of.writeInt(nbOfWords);
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("\nstatic constexpr OUString LCReservedWordsArray[] = {\n");
    for( i = 0; i < nbOfWords; i++) {
        of.writeAsciiString("\tReservedWord");
        of.writeInt(i);
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getReservedWords_", "nbOfReservedWords", "LCReservedWordsArray");

    if (forbidNode)    {
         of.writeOUStringLiteralParameter( "forbiddenBegin", forbidNode -> getChildAt(0)->getValue());
         of.writeOUStringLiteralParameter( "forbiddenEnd", forbidNode -> getChildAt(1)->getValue());
         of.writeOUStringLiteralParameter( "hangingChars", forbidNode -> getChildAt(2)->getValue());
    } else {
         of.writeOUStringLiteralParameter( "forbiddenBegin", std::u16string_view());
         of.writeOUStringLiteralParameter( "forbiddenEnd", std::u16string_view());
         of.writeOUStringLiteralParameter( "hangingChars", std::u16string_view());
    }
    of.writeAsciiString("\nstatic constexpr OUString LCForbiddenCharactersArray[] = {\n");
    of.writeAsciiString("\tforbiddenBegin,\n");
    of.writeAsciiString("\tforbiddenEnd,\n");
    of.writeAsciiString("\thangingChars\n");
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getForbiddenCharacters_", "3", "LCForbiddenCharactersArray");

    if (breakNode) {
         of.writeOUStringLiteralParameter( "EditMode", breakNode -> getChildAt(0)->getValue());
         of.writeOUStringLiteralParameter( "DictionaryMode", breakNode -> getChildAt(1)->getValue());
         of.writeOUStringLiteralParameter( "WordCountMode", breakNode -> getChildAt(2)->getValue());
         of.writeOUStringLiteralParameter( "CharacterMode", breakNode -> getChildAt(3)->getValue());
         of.writeOUStringLiteralParameter( "LineMode", breakNode -> getChildAt(4)->getValue());
    } else {
         of.writeOUStringLiteralParameter( "EditMode", std::u16string_view());
         of.writeOUStringLiteralParameter( "DictionaryMode", std::u16string_view());
         of.writeOUStringLiteralParameter( "WordCountMode", std::u16string_view());
         of.writeOUStringLiteralParameter( "CharacterMode", std::u16string_view());
         of.writeOUStringLiteralParameter( "LineMode", std::u16string_view());
    }
    of.writeAsciiString("\nstatic constexpr OUString LCBreakIteratorRulesArray[] = {\n");
    of.writeAsciiString("\tEditMode,\n");
    of.writeAsciiString("\tDictionaryMode,\n");
    of.writeAsciiString("\tWordCountMode,\n");
    of.writeAsciiString("\tCharacterMode,\n");
    of.writeAsciiString("\tLineMode\n");
    of.writeAsciiString("};\n\n");
    of.writeOUStringFunction("getBreakIteratorRules_", "5", "LCBreakIteratorRulesArray");

}

void LCNumberingLevelNode::generateCode (const OFileWriter &of) const
{
    of.writeAsciiString("// ---> ContinuousNumbering\n");
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction2("getContinuousNumberingLevels_", useLocale);
        return;
    }

    // hard code number of attributes per style.
    const int   nAttributes = 5;
    const char* attr[ nAttributes ] = { "Prefix", "NumType", "Suffix", "Transliteration", "NatNum" };

    // record each attribute of each style in a static C++ variable.
    // determine number of styles on the fly.
    sal_Int32 nStyles = getNumberOfChildren();
    sal_Int32 i;

    for( i = 0; i < nStyles; ++i )
    {
        const Attr &q = getChildAt( i )->getAttr();
        for( sal_Int32 j=0; j<nAttributes; ++j )
        {
            const char* name = attr[j];
            OUString   value = q.getValueByName( name );
            of.writeOUStringLiteralParameter("continuous", name, value, sal::static_int_cast<sal_Int16>(i) );
        }
    }

    // record number of styles and attributes.
    of.writeAsciiString("static const sal_Int16 continuousNbOfStyles = ");
    of.writeInt( sal::static_int_cast<sal_Int16>( nStyles ) );
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("static const sal_Int16 continuousNbOfAttributesPerStyle = ");
    of.writeInt( nAttributes );
    of.writeAsciiString(";\n\n");

    // generate code. (intermediate arrays)
    for( i=0; i<nStyles; i++ )
    {
        of.writeAsciiString("\nstatic constexpr OUString continuousStyle" );
        of.writeInt( sal::static_int_cast<sal_Int16>(i) );
        of.writeAsciiString("[] = {\n");
        for( sal_Int32 j=0; j<nAttributes; j++)
        {
            of.writeAsciiString("\t");
            of.writeAsciiString( "continuous" );
            of.writeAsciiString( attr[j] );
            of.writeInt(sal::static_int_cast<sal_Int16>(i));
            of.writeAsciiString(",\n");
        }
        of.writeAsciiString("\t\n};\n\n");
    }

    // generate code. (top-level array)
    of.writeAsciiString("\n");
    of.writeAsciiString("static const OUString* LCContinuousNumberingLevelsArray[] = {\n" );
    for( i=0; i<nStyles; i++ )
    {
        of.writeAsciiString( "\t" );
        of.writeAsciiString( "continuousStyle" );
        of.writeInt( sal::static_int_cast<sal_Int16>(i) );
        of.writeAsciiString( ",\n");
    }
    of.writeAsciiString("\t0\n};\n\n");
    of.writeOUStringFunction2("getContinuousNumberingLevels_", "continuousNbOfStyles",
            "continuousNbOfAttributesPerStyle", "LCContinuousNumberingLevelsArray");
}


void LCOutlineNumberingLevelNode::generateCode (const OFileWriter &of) const
{
    of.writeAsciiString("// ---> OutlineNumbering\n");
    OUString useLocale =   getAttr().getValueByName("ref");
    if (!useLocale.isEmpty()) {
        useLocale = useLocale.replace( '-', '_');
        of.writeOUStringRefFunction3("getOutlineNumberingLevels_", useLocale);
        return;
    }

    // hardcode number of attributes per level
    const int nAttributes = 12;
    const char* attr[ nAttributes ] =
    {
        "Prefix",
        "NumType",
        "Suffix",
        "BulletChar",
        "BulletFontName",
        "ParentNumbering",
        "LeftMargin",
        "SymbolTextDistance",
        "FirstLineOffset",
        "Adjust",
        "Transliteration",
        "NatNum",
    };

    // record each attribute of each level of each style in a static C++ variable.
    // determine number of styles and number of levels per style on the fly.
    sal_Int32 nStyles = getNumberOfChildren();
    std::vector<sal_Int32> nLevels; // may be different for each style?
    for( sal_Int32 i = 0; i < nStyles; i++ )
    {
        LocaleNode* p = getChildAt( i );
        nLevels.push_back( p->getNumberOfChildren() );
        for( sal_Int32 j=0; j<nLevels.back(); j++ )
        {
            const Attr& q = p->getChildAt( j )->getAttr();
            for( sal_Int32 k=0; k<nAttributes; ++k )
            {
                const char* name = attr[k];
                OUString   value = q.getValueByName( name );
                of.writeOUStringLiteralParameter("outline", name, value,
                        sal::static_int_cast<sal_Int16>(i),
                        sal::static_int_cast<sal_Int16>(j) );
            }
        }
    }

    // verify that each style has the same number of levels.
    for( size_t i=0; i<nLevels.size(); i++ )
    {
        if( nLevels[0] != nLevels[i] )
        {
            incError( "Numbering levels don't match.");
        }
    }

    // record number of attributes, levels, and styles.
    of.writeAsciiString("static const sal_Int16 outlineNbOfStyles = ");
    of.writeInt( sal::static_int_cast<sal_Int16>( nStyles ) );
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("static const sal_Int16 outlineNbOfLevelsPerStyle = ");
    of.writeInt( sal::static_int_cast<sal_Int16>( nLevels.back() ) );
    of.writeAsciiString(";\n\n");
    of.writeAsciiString("static const sal_Int16 outlineNbOfAttributesPerLevel = ");
    of.writeInt( nAttributes );
    of.writeAsciiString(";\n\n");

    // too complicated for now...
    //     of.writeAsciiString("static const sal_Int16 nbOfOutlineNumberingLevels[] = { ");
    //     for( sal_Int32 j=0; j<nStyles; j++ )
    //     {
    //          of.writeInt( nLevels[j] );
    //          of.writeAsciiString(", ");
    //     }
    //     of.writeAsciiString("};\n\n");


    for( sal_Int32 i=0; i<nStyles; i++ )
    {
        for( sal_Int32 j=0; j<nLevels.back(); j++ )
        {
            of.writeAsciiString("static constexpr OUString outline");
            of.writeAsciiString("Style");
            of.writeInt( sal::static_int_cast<sal_Int16>(i) );
            of.writeAsciiString("Level");
            of.writeInt( sal::static_int_cast<sal_Int16>(j) );
            of.writeAsciiString("[] = { ");

            for( sal_Int32 k=0; k<nAttributes; k++ )
            {
                of.writeAsciiString( "outline" );
                of.writeAsciiString( attr[k] );
                of.writeInt( sal::static_int_cast<sal_Int16>(i) );
                of.writeInt( sal::static_int_cast<sal_Int16>(j) );
                of.writeAsciiString(", ");
            }
            of.writeAsciiString(" };\n");
        }
    }

    of.writeAsciiString("\n");


    for( sal_Int32 i=0; i<nStyles; i++ )
    {
        of.writeAsciiString("static constexpr OUString const * outlineStyle" );
        of.writeInt( sal::static_int_cast<sal_Int16>(i) );
        of.writeAsciiString("[] = { ");

        for( sal_Int32 j=0; j<nLevels.back(); j++ )
        {
            of.writeAsciiString("outlineStyle");
            of.writeInt( sal::static_int_cast<sal_Int16>(i) );
            of.writeAsciiString("Level");
            of.writeInt( sal::static_int_cast<sal_Int16>(j) );
            of.writeAsciiString(", ");
        }
        of.writeAsciiString("NULL };\n");
    }
    of.writeAsciiString("\n");

    of.writeAsciiString("static constexpr OUString const * const * LCOutlineNumberingLevelsArray[] = {\n" );
    for( sal_Int32 i=0; i<nStyles; i++ )
    {
        of.writeAsciiString( "\t" );
        of.writeAsciiString( "outlineStyle" );
        of.writeInt( sal::static_int_cast<sal_Int16>(i) );
        of.writeAsciiString(",\n");
    }
    of.writeAsciiString("\tNULL\n};\n\n");
    of.writeOUStringFunction3("getOutlineNumberingLevels_", "outlineNbOfStyles", "outlineNbOfLevelsPerStyle",
            "outlineNbOfAttributesPerLevel", "LCOutlineNumberingLevelsArray");
}

Attr::Attr (const Reference< XAttributeList > & attr) {
    sal_Int16 len = attr->getLength();
    name.realloc (len);
    auto pName = name.getArray();
    value.realloc (len);
    auto pValue = value.getArray();
    for (sal_Int16 i =0; i< len;i++) {
        pName[i] = attr->getNameByIndex(i);
        pValue[i] = attr -> getValueByIndex(i);
    }
}

OUString Attr::getValueByName (const char *str) const {
    auto pName = std::find_if(std::cbegin(name), std::cend(name),
        [&str](const OUString& rName) { return rName.equalsAscii(str); });
    if (pName != std::cend(name))
    {
        auto i = static_cast<sal_Int32>(std::distance(std::cbegin(name), pName));
        return value[i];
    }
    return OUString();
}

const OUString& Attr::getValueByIndex (sal_Int32 idx) const
{
    return value[idx];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
