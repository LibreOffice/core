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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#ifndef _INTN_HXX //autogen
//#include <tools/intn.hxx>
#endif
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HDL_
#include <com/sun/star/util/SearchFlags.hdl>
#endif
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

    // Werte fuer "Gewichtete Levenshtein-Distanz"
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
        Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        rCache.xTextSearch.set( xMSF->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.util.TextSearch" ) ) ), UNO_QUERY_THROW );
        rCache.xTextSearch->setOptions( rPara );
        rCache.Options = rPara;
    }
    catch ( Exception& )
    {
        DBG_ERRORFILE( "TextSearch ctor: Exception caught!" );
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
 * Die allgemeinen Methoden zu Suchen. Diese rufen dann die entpsrecheden
 * Methoden fuer die normale Suche oder der Suche nach Regular-Expressions
 * ueber die MethodenPointer auf.
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
        DBG_ERRORFILE( "SearchForward: Exception caught!" );
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
        DBG_ERRORFILE( "SearchBackward: Exception caught!" );
    }
    return nRet;
}

void TextSearch::ReplaceBackReferences( String& rReplaceStr, const String &rStr, const SearchResult& rResult )
{
    if( rResult.subRegExpressions > 0 )
    {
        String sTab( '\t' );
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

