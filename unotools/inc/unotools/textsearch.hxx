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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_TEXTSEARCH_HXX
#define _UNOTOOLS_TEXTSEARCH_HXX
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/util/SearchOptions.hpp>

// Forward-Deklaration
class CharClass;

namespace com {
    namespace sun {
        namespace star {
            namespace util {
                struct SearchResult;
            }
        }
    }
}

// ............................................................................
namespace utl
{
// ............................................................................

// SS - Klasse fuers Suchen
class UNOTOOLS_DLLPUBLIC SearchParam
{
public:
    enum SearchType{ SRCH_NORMAL, SRCH_REGEXP, SRCH_LEVDIST };

private:
    String sSrchStr;            // the search string
    String sReplaceStr;         // the replace string

    SearchType eSrchType;       // search normal/regular/LevDist

    int bWordOnly   : 1;        // used by normal search
    int bSrchInSel  : 1;        // search only in the selection
    int bCaseSense  : 1;        //

    // values for the "weight Levenshtein-Distance"
    int bLEV_Relaxed : 1;
    int nLEV_OtherX;
    int nLEV_ShorterY;
    int nLEV_LongerZ;

    // asian flags - used for the transliteration
    long nTransliterationFlags;

public:
    SearchParam( const String &rText,
                    SearchType eSrchType = SearchParam::SRCH_NORMAL,
                    sal_Bool bCaseSens = sal_True,
                    sal_Bool bWrdOnly = sal_False,
                    sal_Bool bSrchInSel = sal_False );
    SearchParam( const SearchParam& );

    const String&   GetSrchStr() const          { return sSrchStr; }
    const String&   GetReplaceStr() const       { return sReplaceStr; }
    SearchType      GetSrchType() const         { return eSrchType; }

    int             IsCaseSensitive() const     { return bCaseSense; }
    int             IsSrchInSelection() const   { return bSrchInSel; }
    int             IsSrchWordOnly() const      { return bWordOnly; }


    void SetSrchStr( const String& rStr )       { sSrchStr = rStr; }
    void SetReplaceStr( const String& rStr )    { sReplaceStr = rStr; }
    void SetSrchType( SearchType eType )        { eSrchType = eType; }

    void SetCaseSensitive( int bFlag )          { bCaseSense = bFlag; }
    void SetSrchInSelection( int bFlag )        { bSrchInSel = bFlag; }
    void SetSrchWordOnly( int bFlag )           { bWordOnly = bFlag; }

    int             IsSrchRelaxed() const       { return bLEV_Relaxed; }
    int             GetLEVOther() const         { return nLEV_OtherX; }
    int             GetLEVShorter() const       { return nLEV_ShorterY; }
    int             GetLEVLonger() const        { return nLEV_LongerZ; }

    void SetSrchRelaxed( int bFlag )            { bLEV_Relaxed = bFlag; }
    void SetLEVOther( int nValue )              { nLEV_OtherX = nValue; }
    void SetLEVShorter( int nValue )            { nLEV_ShorterY = nValue; }
    void SetLEVLonger( int nValue )             { nLEV_LongerZ = nValue; }

    long GetTransliterationFlags() const        { return nTransliterationFlags; }
    void SetTransliterationFlags( long nValue ) { nTransliterationFlags = nValue; }
};

//  Klasse zum Suchen eines Strings in einem String.
//  Unterstuetzt werden folgende Verfahren:
//      - normalen Text (Bayer/Moore)
//      - regulaere Ausdruecke
//      - gewichtete Levenshtein Distanz
//
//  Es kann Vorwaerts und Rueckwaerts gesucht werden!

class UNOTOOLS_DLLPUBLIC TextSearch
{
    struct CachedTextSearch
    {
        ::osl::Mutex mutex;
        ::com::sun::star::util::SearchOptions Options;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XTextSearch > xTextSearch;
    };

    static CachedTextSearch maCache;

    static ::com::sun::star::uno::Reference< ::com::sun::star::util::XTextSearch >
        getXTextSearch( const ::com::sun::star::util::SearchOptions& rPara );

    com::sun::star::uno::Reference < com::sun::star::util::XTextSearch >
            xTextSearch;

    void Init( const SearchParam & rParam,
               const ::com::sun::star::lang::Locale& rLocale );

public:
    // rText ist der zusuchende String
    // this first two CTORs are deprecated!
    TextSearch( const SearchParam & rPara, LanguageType nLanguage );
    TextSearch( const SearchParam & rPara, const CharClass& rCClass );

    TextSearch( const ::com::sun::star::util::SearchOptions& rPara );
    ~TextSearch();

    /* search in the (selected) text the search string:
        rScrTxt - the text, in in which we search
        pStart  - start position for the search
        pEnde   - end position for the search

        RETURN values   ==  sal_True: something is found
                        - pStart start pos of the found text,
                        - pStart end pos of the found text,
                        - pSrchResult - the search result with all found
                             positions. Is only filled with more positions
                             if the regular expression handles groups.

                        == sal_False: nothing found, pStart,pEnde unchanged.

        Definitions: start pos always inclusive, end pos always exclusive!
                     The position must always in the right direction!
                    search forward: start <= end
                    search backward: end <= start
    */
    int SearchFrwrd( const String &rStr,
                    xub_StrLen* pStart, xub_StrLen* pEnde,
                    ::com::sun::star::util::SearchResult* pSrchResult = 0 );
    int SearchBkwrd( const String &rStr,
                    xub_StrLen* pStart, xub_StrLen* pEnde,
                    ::com::sun::star::util::SearchResult* pSrchResult = 0 );

    void SetLocale( const ::com::sun::star::util::SearchOptions& rOpt,
                    const ::com::sun::star::lang::Locale& rLocale );

    /* replace back references in the replace string by the sub expressions from the search result */
    void ReplaceBackReferences( String& rReplaceStr, const String &rStr, const ::com::sun::star::util::SearchResult& rResult );

};

// ............................................................................
}   // namespace utl
// ............................................................................

#endif

