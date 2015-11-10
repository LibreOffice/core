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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_TEXTSEARCH_HXX
#define INCLUDED_UNOTOOLS_TEXTSEARCH_HXX
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/util/SearchOptions.hpp>

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

namespace utl
{

// Utility class for searching
class UNOTOOLS_DLLPUBLIC SearchParam
{
public:
    enum SearchType{ SRCH_NORMAL, SRCH_REGEXP, SRCH_LEVDIST };

private:
    OUString sSrchStr;            // the search string
    OUString sReplaceStr;         // the replace string

    SearchType m_eSrchType;       // search normal/regular/LevDist

    bool m_bWordOnly   : 1;        // used by normal search
    bool m_bSrchInSel  : 1;        // search only in the selection
    bool m_bCaseSense  : 1;

    // values for the "weight Levenshtein-Distance"
    bool bLEV_Relaxed : 1;
    int nLEV_OtherX;
    int nLEV_ShorterY;
    int nLEV_LongerZ;

    // asian flags - used for the transliteration
    long nTransliterationFlags;

public:
    SearchParam( const OUString &rText,
                    SearchType eSrchType = SearchParam::SRCH_NORMAL,
                    bool bCaseSensitive = true,
                    bool bWordOnly = false,
                    bool bSearchInSelection = false );

    SearchParam( const SearchParam& );

    ~SearchParam();

    const OUString& GetSrchStr() const          { return sSrchStr; }
    const OUString& GetReplaceStr() const       { return sReplaceStr; }
    SearchType      GetSrchType() const         { return m_eSrchType; }

    bool            IsCaseSensitive() const     { return m_bCaseSense; }
    bool            IsSrchInSelection() const   { return m_bSrchInSel; }
    bool            IsSrchWordOnly() const      { return m_bWordOnly; }

    bool            IsSrchRelaxed() const       { return bLEV_Relaxed; }
    int             GetLEVOther() const         { return nLEV_OtherX; }
    int             GetLEVShorter() const       { return nLEV_ShorterY; }
    int             GetLEVLonger() const        { return nLEV_LongerZ; }

    long GetTransliterationFlags() const        { return nTransliterationFlags; }
};

//  Utility class for searching a substring in a string.
//  The following metrics are supported
//      - ordinary text (Bayer/Moore)
//      - regular expressions
//      - weighted Levenshtein distance

//  This class allows forward and backward searching!

class UNOTOOLS_DLLPUBLIC TextSearch
{
    static css::uno::Reference< css::util::XTextSearch >
        getXTextSearch( const css::util::SearchOptions& rPara );

    css::uno::Reference < css::util::XTextSearch >
            xTextSearch;

    void Init( const SearchParam & rParam,
               const css::lang::Locale& rLocale );

public:
    // rText is the string being searched for
    // this first two CTORs are deprecated!
    TextSearch( const SearchParam & rPara, LanguageType nLanguage );
    TextSearch( const SearchParam & rPara, const CharClass& rCClass );

    TextSearch( const css::util::SearchOptions& rPara );
    ~TextSearch();

    /* search in the (selected) text the search string:
        rScrTxt - the text, in which we search
        pStart  - start position for the search
        pEnde   - end position for the search

        RETURN values   ==  true: something is found
                        - pStart start pos of the found text,
                        - pStart end pos of the found text,
                        - pSrchResult - the search result with all found
                             positions. Is only filled with more positions
                             if the regular expression handles groups.

                        == false: nothing found, pStart,pEnde unchanged.

        Definitions: start pos always inclusive, end pos always exclusive!
                     The position must always in the right direction!
                    search forward: start <= end
                    search backward: end <= start
    */
    bool SearchForward( const OUString &rStr,
                        sal_Int32* pStart, sal_Int32* pEnd,
                        css::util::SearchResult* pRes = nullptr );
    bool SearchBackward( const OUString &rStr,
                        sal_Int32* pStart, sal_Int32* pEnd,
                        css::util::SearchResult* pRes = nullptr );

    void SetLocale( const css::util::SearchOptions& rOpt,
                    const css::lang::Locale& rLocale );

    /* replace back references in the replace string by the sub expressions from the search result */
    void ReplaceBackReferences( OUString& rReplaceStr, const OUString &rStr, const css::util::SearchResult& rResult );

};

}   // namespace utl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
