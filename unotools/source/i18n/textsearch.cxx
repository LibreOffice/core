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

#include <i18npool/mslangid.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/TextSearch.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/textsearch.hxx>
#include <rtl/instance.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// ............................................................................
namespace utl
{
// ............................................................................

SearchParam::SearchParam( const String &rText,
                                SearchType eType,
                                sal_Bool bCaseSensitive,
                                sal_Bool bWrdOnly,
                                sal_Bool bSearchInSel )
{
    sSrchStr        = rText;
    eSrchType       = eType;

    bWordOnly       = bWrdOnly;
    bSrchInSel      = bSearchInSel;
    bCaseSense      = bCaseSensitive;

    nTransliterationFlags = 0;

    // Parameters for weighted Levenshtein distance
    bLEV_Relaxed    = sal_True;
    nLEV_OtherX     = 2;
    nLEV_ShorterY   = 1;
    nLEV_LongerZ    = 3;
}

SearchParam::SearchParam( const SearchParam& rParam )
{
    sSrchStr        = rParam.sSrchStr;
    sReplaceStr     = rParam.sReplaceStr;
    eSrchType       = rParam.eSrchType;

    bWordOnly       = rParam.bWordOnly;
    bSrchInSel      = rParam.bSrchInSel;
    bCaseSense      = rParam.bCaseSense;

    bLEV_Relaxed    = rParam.bLEV_Relaxed;
    nLEV_OtherX     = rParam.nLEV_OtherX;
    nLEV_ShorterY   = rParam.nLEV_ShorterY;
    nLEV_LongerZ    = rParam.nLEV_LongerZ;

    nTransliterationFlags = rParam.nTransliterationFlags;
}

static bool lcl_Equals( const SearchOptions& rSO1, const SearchOptions& rSO2 )
{
    return rSO1.algorithmType == rSO2.algorithmType &&
        rSO1.searchFlag == rSO2.searchFlag &&
        rSO1.searchString.equals(rSO2.searchString) &&
        rSO1.replaceString.equals(rSO2.replaceString) &&
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
        ::com::sun::star::util::SearchOptions Options;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XTextSearch > xTextSearch;
    };

    struct theCachedTextSearch
        : public rtl::Static< CachedTextSearch, theCachedTextSearch > {};
}

Reference<XTextSearch> TextSearch::getXTextSearch( const SearchOptions& rPara )
{
    CachedTextSearch &rCache = theCachedTextSearch::get();

    osl::MutexGuard aGuard(rCache.mutex);

    if ( lcl_Equals(rCache.Options, rPara) )
        return rCache.xTextSearch;

    try
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        rCache.xTextSearch.set( ::TextSearch::create(xContext) );
        rCache.xTextSearch->setOptions( rPara );
        rCache.Options = rPara;
    }
    catch ( Exception& e )
    {
        SAL_WARN( "unotools.i18n", "caught " << e.Message );
    }
    return rCache.xTextSearch;
}

TextSearch::TextSearch(const SearchParam & rParam, LanguageType eLang )
{
    if( LANGUAGE_NONE == eLang )
        eLang = LANGUAGE_SYSTEM;
    ::com::sun::star::lang::Locale aLocale(
            MsLangId::convertLanguageToLocale( LanguageType(eLang)));

    Init( rParam, aLocale);
}

TextSearch::TextSearch(const SearchParam & rParam, const CharClass& rCClass )
{
    Init( rParam, rCClass.getLocale() );
}

TextSearch::TextSearch( const SearchOptions& rPara )
{
    xTextSearch = getXTextSearch( rPara );
}

void TextSearch::Init( const SearchParam & rParam,
                        const ::com::sun::star::lang::Locale& rLocale )
{
    // convert SearchParam to the UNO SearchOptions
    SearchOptions aSOpt;

    switch( rParam.GetSrchType() )
    {
    case SearchParam::SRCH_REGEXP:
        aSOpt.algorithmType = SearchAlgorithms_REGEXP;
        if( rParam.IsSrchInSelection() )
            aSOpt.searchFlag |= SearchFlags::REG_NOT_BEGINOFLINE |
                                SearchFlags::REG_NOT_ENDOFLINE;
        break;

    case SearchParam::SRCH_LEVDIST:
        aSOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
        aSOpt.changedChars = rParam.GetLEVOther();
        aSOpt.deletedChars = rParam.GetLEVLonger();
        aSOpt.insertedChars = rParam.GetLEVShorter();
        if( rParam.IsSrchRelaxed() )
            aSOpt.searchFlag |= SearchFlags::LEV_RELAXED;
        break;

//  case SearchParam::SRCH_NORMAL:
    default:
        aSOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
        if( rParam.IsSrchWordOnly() )
            aSOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;
        break;
    }
    aSOpt.searchString = rParam.GetSrchStr();
    aSOpt.replaceString = rParam.GetReplaceStr();
    aSOpt.Locale = rLocale;
    aSOpt.transliterateFlags = rParam.GetTransliterationFlags();
    if( !rParam.IsCaseSensitive() )
    {
        aSOpt.searchFlag |= SearchFlags::ALL_IGNORE_CASE;
        aSOpt.transliterateFlags |= ::com::sun::star::i18n::TransliterationModules_IGNORE_CASE;
    }

    xTextSearch = getXTextSearch( aSOpt );
}

void TextSearch::SetLocale( const ::com::sun::star::util::SearchOptions& rOptions,
                            const ::com::sun::star::lang::Locale& rLocale )
{
    // convert SearchParam to the UNO SearchOptions
    SearchOptions aSOpt( rOptions );
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
#if defined _MSC_VER
#pragma optimize("", off)
#pragma warning(push)
#pragma warning(disable: 4748)
#endif
int TextSearch::SearchFrwrd( const String & rStr, xub_StrLen* pStart,
                            xub_StrLen* pEnde, SearchResult* pRes )
{
    int nRet = 0;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchForward(
                                                    rStr, *pStart, *pEnde ));
            if( aRet.subRegExpressions > 0 )
            {
                nRet = 1;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is allways exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pStart = (xub_StrLen)aRet.startOffset[ 0 ];
                *pEnde = (xub_StrLen)aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "unotools.i18n", "SearchForward: Exception caught!" );
    }
    return nRet;
}

sal_Bool TextSearch::SearchForward( const ::rtl::OUString &rStr,
                    sal_Int32* pStart, sal_Int32* pEnd,
                    ::com::sun::star::util::SearchResult* pRes)
{
    sal_Bool nRet = sal_False;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchForward(
                                                    rStr, *pStart, *pEnd ));
            if( aRet.subRegExpressions > 0 )
            {
                nRet = sal_True;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is allways exclusive.
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
    return nRet;
}


int TextSearch::SearchBkwrd( const String & rStr, xub_StrLen* pStart,
                            xub_StrLen* pEnde, SearchResult* pRes )
{
    int nRet = 0;
    try
    {
        if( xTextSearch.is() )
        {
            SearchResult aRet( xTextSearch->searchBackward(
                                                    rStr, *pStart, *pEnde ));
            if( aRet.subRegExpressions )
            {
                nRet = 1;
                // the XTextsearch returns in startOffset the higher position
                // and the endposition is allways exclusive.
                // The caller of this function will have in startPos the
                // lower pos. and end
                *pEnde = (xub_StrLen)aRet.startOffset[ 0 ];
                *pStart = (xub_StrLen)aRet.endOffset[ 0 ];
                if( pRes )
                    *pRes = aRet;
            }
        }
    }
    catch ( Exception& )
    {
        SAL_WARN( "unotools.i18n", "SearchBackward: Exception caught!" );
    }
    return nRet;
}

void TextSearch::ReplaceBackReferences( String& rReplaceStr, const String &rStr, const SearchResult& rResult )
{
    if( rResult.subRegExpressions > 0 )
    {
        rtl::OUString sTab( '\t' );
        sal_Unicode sSrchChrs[] = {'\\', '&', '$', 0};
        String sTmp;
        xub_StrLen nPos = 0;
        sal_Unicode sFndChar;
        while( STRING_NOTFOUND != ( nPos = rReplaceStr.SearchChar( sSrchChrs, nPos )) )
        {
            if( rReplaceStr.GetChar( nPos ) == '&')
            {
                sal_uInt16 nStart = (sal_uInt16)(rResult.startOffset[0]);
                sal_uInt16 nLength = (sal_uInt16)(rResult.endOffset[0] - rResult.startOffset[0]);
                rReplaceStr.Erase( nPos, 1 );   // delete ampersand
                // replace by found string
                rReplaceStr.Insert( rStr, nStart, nLength, nPos );
                // jump over
                nPos = nPos + nLength;
            }
            else if( rReplaceStr.GetChar( nPos ) == '$')
            {
                if( nPos + 1 < rReplaceStr.Len())
                {
                    sFndChar = rReplaceStr.GetChar( nPos + 1 );
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
                            rReplaceStr.Erase( nPos, 2 );   // delete both
                            int i = sFndChar - '0'; // index
                            if(i < rResult.subRegExpressions)
                            {
                                sal_uInt16 nSttReg = (sal_uInt16)(rResult.startOffset[i]);
                                sal_uInt16 nRegLen = (sal_uInt16)(rResult.endOffset[i]);
                                if( nRegLen > nSttReg )
                                    nRegLen = nRegLen - nSttReg;
                                else
                                {
                                    nRegLen = nSttReg - nRegLen;
                                    nSttReg = (sal_uInt16)(rResult.endOffset[i]);
                                }
                                // Copy reference from found string
                                sTmp = rStr.Copy((sal_uInt16)nSttReg, (sal_uInt16)nRegLen);
                                // insert
                                rReplaceStr.Insert( sTmp, nPos );
                                // and step over
                                nPos = nPos + sTmp.Len();
                            }
                        }
                        break;
                        default:
                            nPos += 2; // leave both chars unchanged
                            break;
                    }
                }
                else
                    ++nPos;
            }
            else
            {
                // at least another character?
                if( nPos + 1 < rReplaceStr.Len())
                {
                    sFndChar = rReplaceStr.GetChar( nPos + 1 );
                    switch(sFndChar)
                    {
                        case '\\':
                        case '&':
                        case '$':
                            rReplaceStr.Erase( nPos, 1 );
                            nPos++;
                        break;
                        case 't':
                            rReplaceStr.Erase( nPos, 2 ); // delete both
                            rReplaceStr.Insert( sTab, nPos ); // insert tabulator
                            nPos++; // step over
                        break;
                        default:
                            nPos += 2; // ignore both characters
                        break;
                    }
                }
                else
                    ++nPos;
            }
        }
    }
}


#if defined _MSC_VER
#pragma optimize("", on)
#pragma warning(pop)
#endif

// ............................................................................
}   // namespace utl
// ............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
