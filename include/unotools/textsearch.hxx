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

#ifndef INCLUDED_UNOTOOLS_TEXTSEARCH_HXX
#define INCLUDED_UNOTOOLS_TEXTSEARCH_HXX

#include <unotools/unotoolsdllapi.h>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XTextSearch2.hpp>
#include <com/sun/star/util/SearchOptions2.hpp>

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
    enum class SearchType { Normal, Regexp, Wildcard, Unknown = -1 };

    /** Convert configuration and document boolean settings to SearchType.
        If bWildcard is true it takes precedence over rbRegExp.
        @param  rbRegExp
                If true and bWildcard is also true, rbRegExp is set to false to
                adapt the caller's settings.
     */
    static SearchType ConvertToSearchType( bool bWildcard, bool & rbRegExp )
    {
        if (bWildcard)
        {
            if (rbRegExp)
                rbRegExp = false;
            return SearchType::Wildcard;
        }
        return rbRegExp ? SearchType::Regexp : SearchType::Normal;
    }

    /** Convert SearchType to configuration and document boolean settings.
     */
    static void ConvertToBool( const SearchType eSearchType, bool& rbWildcard, bool& rbRegExp )
    {
        switch (eSearchType)
        {
            case SearchType::Wildcard:
                rbWildcard = true;
                rbRegExp = false;
                break;
            case SearchType::Regexp:
                rbWildcard = false;
                rbRegExp = true;
                break;
            case SearchType::Normal:
                rbWildcard = false;
                rbRegExp = false;
                break;
            default: assert(false);
        }
    }

private:
    OUString sSrchStr;            // the search string
    OUString sReplaceStr;         // the replace string

    SearchType m_eSrchType;       // search normal/regular/LevDist

    sal_uInt32 m_cWildEscChar;      // wildcard escape character

    bool m_bWordOnly   : 1;        // used by normal search
    bool m_bSrchInSel  : 1;        // search only in the selection
    bool m_bCaseSense  : 1;
    bool m_bWildMatchSel : 1;       // wildcard pattern must match entire selection

    // values for the "weight Levenshtein-Distance"
    bool bLEV_Relaxed : 1;
    int nLEV_OtherX;
    int nLEV_ShorterY;
    int nLEV_LongerZ;

    // asian flags - used for the transliteration
    long nTransliterationFlags;

public:
    SearchParam( const OUString &rText,
                    SearchType eSrchType,
                    bool bCaseSensitive = true,
                    sal_uInt32 cWildEscChar = '\\',
                    bool bWildMatchSel = false );

    SearchParam( const SearchParam& );

    ~SearchParam();

    const OUString& GetSrchStr() const          { return sSrchStr; }
    const OUString& GetReplaceStr() const       { return sReplaceStr; }
    SearchType      GetSrchType() const         { return m_eSrchType; }

    bool            IsCaseSensitive() const     { return m_bCaseSense; }
    bool            IsSrchInSelection() const   { return m_bSrchInSel; }
    bool            IsSrchWordOnly() const      { return m_bWordOnly; }
    bool            IsWildMatchSel() const      { return m_bWildMatchSel; }

    // signed return for API use
    sal_Int32       GetWildEscChar() const      { return static_cast<sal_Int32>(m_cWildEscChar); }

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
//      - wildcards '*' and '?'

//  This class allows forward and backward searching!

class UNOTOOLS_DLLPUBLIC TextSearch
{
    static css::uno::Reference< css::util::XTextSearch2 >
        getXTextSearch( const css::util::SearchOptions2& rPara );

    css::uno::Reference < css::util::XTextSearch2 >
            xTextSearch;

    void Init( const SearchParam & rParam,
               const css::lang::Locale& rLocale );

public:
    // rText is the string being searched for
    // this first two CTORs are deprecated!
    TextSearch( const SearchParam & rPara, LanguageType nLanguage );
    TextSearch( const SearchParam & rPara, const CharClass& rCClass );

    TextSearch( const css::util::SearchOptions2& rPara );
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

    void SetLocale( const css::util::SearchOptions2& rOpt,
                    const css::lang::Locale& rLocale );

    /* replace back references in the replace string by the sub expressions from the search result */
    void ReplaceBackReferences( OUString& rReplaceStr, const OUString &rStr, const css::util::SearchResult& rResult );

    /** Upgrade SearchOptions to SearchOptions2 for places that don't handle
        SearchOptions2 yet. Better fix your module if you want to support
        wildcard search.
     */
    static css::util::SearchOptions2 UpgradeToSearchOptions2( const css::util::SearchOptions& rOptions );

};

}   // namespace utl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
