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

#include <ostream>

class CharClass;

namespace com::sun::star::lang { struct Locale; }
namespace com::sun::star::util { class XTextSearch2; }
namespace com::sun::star::util { struct SearchResult; }
namespace i18nutil {
    struct SearchOptions;
    struct SearchOptions2;
}
enum class TransliterationFlags;

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
            default:
                rbWildcard = false;
                rbRegExp = false;
                break;
        }
    }

private:
    OUString sSrchStr;            // the search string

    SearchType m_eSrchType;       // search normal/regular/LevDist

    sal_uInt32 m_cWildEscChar;      // wildcard escape character

    bool m_bCaseSense  : 1;
    bool m_bWildMatchSel : 1;       // wildcard pattern must match entire selection

public:
    SearchParam( const OUString &rText,
                    SearchType eSrchType,
                    bool bCaseSensitive = true,
                    sal_uInt32 cWildEscChar = '\\',
                    bool bWildMatchSel = false );

    SearchParam( const SearchParam& );

    ~SearchParam();

    const OUString& GetSrchStr() const          { return sSrchStr; }
    SearchType      GetSrchType() const         { return m_eSrchType; }

    bool            IsCaseSensitive() const     { return m_bCaseSense; }
    bool            IsWildMatchSel() const      { return m_bWildMatchSel; }

    // signed return for API use
    sal_Int32       GetWildEscChar() const      { return static_cast<sal_Int32>(m_cWildEscChar); }
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const SearchParam::SearchType& eType)
{
    switch (eType)
    {
    case SearchParam::SearchType::Normal:
        stream << "N";
        break;
    case SearchParam::SearchType::Regexp:
        stream << "RE";
        break;
    case SearchParam::SearchType::Wildcard:
        stream << "WC";
        break;
    case SearchParam::SearchType::Unknown:
        stream << "UNK";
        break;
    default:
        stream << static_cast<int>(eType) << '?';
        break;
    }

    return stream;
}

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
        getXTextSearch( const i18nutil::SearchOptions2& rPara );

    css::uno::Reference < css::util::XTextSearch2 >
            xTextSearch;

    void Init( const SearchParam & rParam,
               const css::lang::Locale& rLocale );

public:
    // rText is the string being searched for
    // this first two CTORs are deprecated!
    TextSearch( const SearchParam & rPara, LanguageType nLanguage );
    TextSearch( const SearchParam & rPara, const CharClass& rCClass );

    TextSearch( const i18nutil::SearchOptions2& rPara );
    ~TextSearch();

    /* search in the (selected) text the search string:
        rScrTxt - the text, in which we search
        pStart  - start position for the search
        pEnd    - end position for the search

        RETURN values   ==  true: something is found
                        - pStart start pos of the found text,
                        - pEnd end pos of the found text,
                        - pSrchResult - the search result with all found
                             positions. Is only filled with more positions
                             if the regular expression handles groups.

                        == false: nothing found, pStart, pEnd unchanged.

        Definitions: start pos always inclusive, end pos always exclusive!
                     The position must always in the right direction!
                    search forward: start <= end
                    search backward: end <= start
    */
    bool SearchForward( const OUString &rStr,
                        sal_Int32* pStart, sal_Int32* pEnd,
                        css::util::SearchResult* pRes = nullptr );
    /**
     * @brief searchForward Search forward beginning from the start to the end
     *        of the given text
     * @param rStr The text in which we search
     * @return True if the search term is found in the text
     */
    bool searchForward( const OUString &rStr );
    bool SearchBackward( const OUString &rStr,
                        sal_Int32* pStart, sal_Int32* pEnd,
                        css::util::SearchResult* pRes = nullptr );

    void SetLocale( const i18nutil::SearchOptions2& rOpt,
                    const css::lang::Locale& rLocale );

    /* replace back references in the replace string by the sub expressions from the search result */
    static void ReplaceBackReferences( OUString& rReplaceStr, std::u16string_view rStr, const css::util::SearchResult& rResult );
};

}   // namespace utl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
