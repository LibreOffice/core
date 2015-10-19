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

#ifndef INCLUDED_I18NPOOL_SOURCE_SEARCH_TEXTSEARCH_HXX
#define INCLUDED_I18NPOOL_SOURCE_SEARCH_TEXTSEARCH_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <map>

#include <unicode/regex.h>
using namespace U_ICU_NAMESPACE;
typedef U_ICU_NAMESPACE::UnicodeString IcuUniString;

class WLevDistance;
typedef ::std::map< sal_Unicode, sal_Int32 > TextSearchJumpTable;

class TextSearch: public cppu::WeakImplHelper
<
    css::util::XTextSearch,
    css::lang::XServiceInfo
>
{
    css::uno::Reference < css::uno::XComponentContext > m_xContext;

    css::util::SearchOptions aSrchPara;
    OUString sSrchStr;
    OUString sSrchStr2;

    mutable css::uno::Reference< css::i18n::XCharacterClassification > xCharClass;

    css::uno::Reference< css::i18n::XExtendedTransliteration > xTranslit;
    css::uno::Reference< css::i18n::XExtendedTransliteration > xTranslit2;

    // define a function pointer for the different search methods
    typedef css::util::SearchResult
        (SAL_CALL TextSearch:: *FnSrch)( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos );

    FnSrch fnForward;
    FnSrch fnBackward;

    // Members and methods for the normal (Boyer-Moore) search
    TextSearchJumpTable* pJumpTable;
    TextSearchJumpTable* pJumpTable2;
    bool bIsForwardTab;
    bool bUsePrimarySrchStr;
    void MakeForwardTab();
    void MakeForwardTab2();
    void MakeBackwardTab();
    void MakeBackwardTab2();
    sal_Int32 GetDiff( const sal_Unicode ) const;
    css::util::SearchResult SAL_CALL
        NSrchFrwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);
    css::util::SearchResult SAL_CALL
        NSrchBkwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);

    // Members and methods for the regular expression search
    RegexMatcher* pRegexMatcher;
    css::util::SearchResult SAL_CALL
        RESrchFrwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);
    css::util::SearchResult SAL_CALL
        RESrchBkwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);
    void RESrchPrepare( const css::util::SearchOptions&);

    // Members and methods for the "Weight Levenshtein-Distance" search
    int nLimit;
    WLevDistance* pWLD;
    css::uno::Reference < css::i18n::XBreakIterator > xBreak;
    css::util::SearchResult SAL_CALL
        ApproxSrchFrwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);
    css::util::SearchResult SAL_CALL
        ApproxSrchBkwrd( const OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException);

    bool IsDelimiter( const OUString& rStr, sal_Int32 nPos ) const;

    bool checkCTLStart, checkCTLEnd;
    bool SAL_CALL isCellStart(const OUString& searchStr, sal_Int32 nPos)
                            throw(css::uno::RuntimeException);

public:
    TextSearch(
        const css::uno::Reference < css::uno::XComponentContext >& rxContext );

    virtual ~TextSearch();

    // Methods
    virtual void SAL_CALL
        setOptions( const css::util::SearchOptions& options )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::util::SearchResult SAL_CALL
        searchForward( const OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::util::SearchResult SAL_CALL
        searchBackward( const OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
                throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                throw( css::uno::RuntimeException, std::exception ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
