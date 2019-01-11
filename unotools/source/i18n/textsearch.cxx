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

#include <sal/config.h>

#include <cstdlib>
#include <string_view>

#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>
#include <i18nutil/transliteration.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/TextSearch2.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <sal/log.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/textsearch.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

namespace utl
{

SearchParam::SearchParam( const OUString &rText,
                                SearchType eType,
                                bool bCaseSensitive,
                                sal_uInt32 cWildEscChar,
                                bool bWildMatchSel )
{
    sSrchStr        = rText;
    m_eSrchType     = eType;

    m_cWildEscChar  = cWildEscChar;

    m_bCaseSense    = bCaseSensitive;
    m_bWildMatchSel = bWildMatchSel;
}

SearchParam::SearchParam( const SearchParam& rParam )
{
    sSrchStr        = rParam.sSrchStr;
    m_eSrchType     = rParam.m_eSrchType;

    m_cWildEscChar  = rParam.m_cWildEscChar;

    m_bCaseSense    = rParam.m_bCaseSense;
    m_bWildMatchSel = rParam.m_bWildMatchSel;
}

SearchParam::~SearchParam() {}

static bool lcl_Equals( const i18nutil::SearchOptions2& rSO1, const i18nutil::SearchOptions2& rSO2 )
{
    return
        rSO1.AlgorithmType2 == rSO2.AlgorithmType2 &&
        rSO1.WildcardEscapeCharacter == rSO2.WildcardEscapeCharacter &&
        rSO1.algorithmType == rSO2.algorithmType &&
        rSO1.searchFlag == rSO2.searchFlag &&
        rSO1.searchString == rSO2.searchString &&
        rSO1.replaceString == rSO2.replaceString &&
        rSO1.changedChars == rSO2.changedChars &&
        rSO1.deletedChars == rSO2.deletedChars &&
        rSO1.insertedChars == rSO2.insertedChars &&
        rSO1.Locale.Language == rSO2.Locale.Language &&
        rSO1.Locale.Country == rSO2.Locale.Country &&
        rSO1.Locale.Variant == rSO2.Locale.Variant &&
        rSO1.transliterateFlags == rSO2.transliterateFlags;
}

namespace
{
    struct CachedTextSearch
    {
        ::osl::Mutex mutex;
        i18nutil::SearchOptions2 Options;
        css::uno::Reference< css::util::XTextSearch2 > xTextSearch;
    };

    struct theCachedTextSearch
        : public rtl::Static< CachedTextSearch, theCachedTextSearch > {};
}

Reference<XTextSearch2> TextSearch::getXTextSearch( const i18nutil::SearchOptions2& rPara )
{
    CachedTextSearch &rCache = theCachedTextSearch::get();

    osl::MutexGuard aGuard(rCache.mutex);

    if ( lcl_Equals(rCache.Options, rPara) )
        return rCache.xTextSearch;

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    rCache.xTextSearch.set( ::TextSearch2::create(xContext) );
    rCache.xTextSearch->setOptions2( rPara.toUnoSearchOptions2() );
    rCache.Options = rPara;

    return rCache.xTextSearch;
}

TextSearch::TextSearch(const SearchParam & rParam, LanguageType eLang )
{
    if( LANGUAGE_NONE == eLang )
        eLang = LANGUAGE_SYSTEM;
    css::lang::Locale aLocale( LanguageTag::convertToLocale( eLang ) );

    Init( rParam, aLocale);
}

TextSearch::TextSearch(const SearchParam & rParam, const CharClass& rCClass )
{
    Init( rParam, rCClass.getLanguageTag().getLocale() );
}

TextSearch::TextSearch( const i18nutil::SearchOptions2& rPara )
{
    xTextSearch = getXTextSearch( rPara );
}

i18nutil::SearchOptions2 TextSearch::UpgradeToSearchOptions2( const i18nutil::SearchOptions& rOptions )
{
    sal_Int16 nAlgorithmType2;
    switch (rOptions.algorithmType)
    {
        case SearchAlgorithms_REGEXP:
            nAlgorithmType2 = SearchAlgorithms2::REGEXP;
            break;
        case SearchAlgorithms_APPROXIMATE:
            nAlgorithmType2 = SearchAlgorithms2::APPROXIMATE;
            break;
        case SearchAlgorithms_ABSOLUTE:
            nAlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
            break;
        default:
            for (;;) std::abort();
    }
    // It would be nice if an inherited struct had a ctor that takes an
    // instance of the object the struct derived from..
    i18nutil::SearchOptions2 aOptions2(
            rOptions.algorithmType,
            rOptions.searchFlag,
            rOptions.searchString,
            rOptions.replaceString,
            rOptions.Locale,
            rOptions.changedChars,
            rOptions.deletedChars,
            rOptions.insertedChars,
            rOptions.transliterateFlags,
            nAlgorithmType2,
            0       // no wildcard search, no escape character..
            );
    return aOptions2;
}

void TextSearch::Init( const SearchParam & rParam,
                        const css::lang::Locale& rLocale )
{
    // convert SearchParam to the UNO SearchOptions2
    i18nutil::SearchOptions2 aSOpt;

    switch( rParam.GetSrchType() )
    {
    case SearchParam::SearchType::Wildcard:
        aSOpt.AlgorithmType2 = SearchAlgorithms2::WILDCARD;
        aSOpt.algorithmType = SearchAlgorithms::SearchAlgorithms_MAKE_FIXED_SIZE;    // no old enum for that
        aSOpt.WildcardEscapeCharacter = rParam.GetWildEscChar();
        if (rParam.IsWildMatchSel())
            aSOpt.searchFlag |= SearchFlags::WILD_MATCH_SELECTION;
        break;

    case SearchParam::SearchType::Regexp:
        aSOpt.AlgorithmType2 = SearchAlgorithms2::REGEXP;
        aSOpt.algorithmType = SearchAlgorithms_REGEXP;
        break;

    case SearchParam::SearchType::Normal:
        aSOpt.AlgorithmType2 = SearchAlgorithms2::ABSOLUTE;
        aSOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
        break;

    default:
        for (;;) std::abort();
    }
    aSOpt.searchString = rParam.GetSrchStr();
    aSOpt.replaceString = "";
    aSOpt.Locale = rLocale;
    aSOpt.transliterateFlags = TransliterationFlags::NONE;
    if( !rParam.IsCaseSensitive() )
    {
        aSOpt.searchFlag |= SearchFlags::ALL_IGNORE_CASE;
        aSOpt.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    }

    xTextSearch = getXTextSearch( aSOpt );
}

void TextSearch::SetLocale( const i18nutil::SearchOptions2& rOptions,
                            const css::lang::Locale& rLocale )
{
    i18nutil::SearchOptions2 aSOpt( rOptions );
    aSOpt.Locale = rLocale;

    xTextSearch = getXTextSearch( aSOpt );
}

TextSearch::~TextSearch()
{
}

/*
 * General search methods. These methods will call the respective
 * methods, such as ordinary string searching or regular expression
 * matching, using the method pointer.
 */
bool TextSearch::SearchForward( const OUString &rStr,
                    sal_Int32* pStart, sal_Int32* pEnd,
                    css::util::SearchResult* pRes)
{
    bool bRet = false;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchForward( rStr, *pStart, *pEnd ));
            if( aRet.subRegExpressions > 0 )
            {
                bRet = true;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is always exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pStart = aRet.startOffset[ 0 ];
                *pEnd = aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "unotools.i18n", "SearchForward: Exception caught!" );
    }
    return bRet;
}

bool TextSearch::searchForward( const OUString &rStr )
{
    sal_Int32 pStart = 0;
    sal_Int32 pEnd = rStr.getLength();

    bool bResult = SearchForward(rStr, &pStart, &pEnd);

    return bResult;
}

bool TextSearch::SearchBackward( const OUString & rStr, sal_Int32* pStart,
                                sal_Int32* pEnde, SearchResult* pRes )
{
    bool bRet = false;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchBackward( rStr, *pStart, *pEnde ));
            if( aRet.subRegExpressions )
            {
                bRet = true;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is always exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pEnde = aRet.startOffset[ 0 ];
                *pStart = aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "unotools.i18n", "SearchBackward: Exception caught!" );
    }
    return bRet;
}

void TextSearch::ReplaceBackReferences( OUString& rReplaceStr, const OUString &rStr, const SearchResult& rResult ) const
{
    if( rResult.subRegExpressions > 0 )
    {
        sal_Unicode sFndChar;
        sal_Int32 i;
        OUStringBuffer sBuff(rReplaceStr.getLength()*4);
        for(i = 0; i < rReplaceStr.getLength(); i++)
        {
            if( rReplaceStr[i] == '&')
            {
                sal_Int32 nStart = rResult.startOffset[0];
                sal_Int32 nLength = rResult.endOffset[0] - rResult.startOffset[0];
                sBuff.append(std::u16string_view(rStr).substr(nStart, nLength));
            }
            else if((i < rReplaceStr.getLength() - 1) && rReplaceStr[i] == '$')
            {
                sFndChar = rReplaceStr[ i + 1 ];
                switch(sFndChar)
                {   // placeholder for a backward reference?
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    {
                        int j = sFndChar - '0'; // index
                        if(j < rResult.subRegExpressions)
                        {
                            sal_Int32 nSttReg = rResult.startOffset[j];
                            sal_Int32 nRegLen = rResult.endOffset[j];
                            if (nSttReg < 0 || nRegLen < 0) // A "not found" optional capture
                            {
                                nSttReg = nRegLen = 0; // Copy empty string
                            }
                            else if (nRegLen >= nSttReg)
                            {
                                nRegLen = nRegLen - nSttReg;
                            }
                            else
                            {
                                nRegLen = nSttReg - nRegLen;
                                nSttReg = rResult.endOffset[j];
                            }
                            // Copy reference from found string
                            sBuff.append(std::u16string_view(rStr).substr(nSttReg, nRegLen));
                        }
                        i += 1;
                    }
                    break;
                default:
                    sBuff.append(rReplaceStr[i]);
                    sBuff.append(rReplaceStr[i+1]);
                    i += 1;
                    break;
                }
            }
            else if((i < rReplaceStr.getLength() - 1) && rReplaceStr[i] == '\\')
            {
                sFndChar = rReplaceStr[ i+1 ];
                switch(sFndChar)
                {
                case '\\':
                case '&':
                case '$':
                    sBuff.append(sFndChar);
                    i+=1;
                    break;
                case 't':
                    sBuff.append('\t');
                    i += 1;
                    break;
                default:
                    sBuff.append(rReplaceStr[i]);
                    sBuff.append(rReplaceStr[i+1]);
                    i += 1;
                    break;
                }
            }
            else
            {
                sBuff.append(rReplaceStr[i]);
            }
        }
        rReplaceStr = sBuff.makeStringAndClear();
    }
}

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
