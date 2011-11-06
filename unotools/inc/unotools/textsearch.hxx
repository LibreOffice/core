/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

