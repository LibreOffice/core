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

#pragma once
#if 1

#include <cppuhelper/implbase2.hxx>
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

class TextSearch: public cppu::WeakImplHelper2
<
    ::com::sun::star::util::XTextSearch,
    ::com::sun::star::lang::XServiceInfo
>
{
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext > m_xContext;

    ::com::sun::star::util::SearchOptions aSrchPara;
    ::rtl::OUString sSrchStr;
    ::rtl::OUString sSrchStr2;

    mutable com::sun::star::uno::Reference<
        com::sun::star::i18n::XCharacterClassification > xCharClass;

    com::sun::star::uno::Reference<
        com::sun::star::i18n::XExtendedTransliteration > xTranslit;
    com::sun::star::uno::Reference<
        com::sun::star::i18n::XExtendedTransliteration > xTranslit2;

    // define a function pointer for the different search nethods
    typedef ::com::sun::star::util::SearchResult
        (SAL_CALL TextSearch:: *FnSrch)( const ::rtl::OUString& searchStr,
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
    ::com::sun::star::util::SearchResult SAL_CALL
        NSrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        NSrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    // Members and methods for the regular expression search
    RegexMatcher* pRegexMatcher;
    ::com::sun::star::util::SearchResult SAL_CALL
        RESrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        RESrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    void RESrchPrepare( const ::com::sun::star::util::SearchOptions&);

    // Members and methods for the "Weight Levenshtein-Distance" search
    int nLimit;
    WLevDistance* pWLD;
    com::sun::star::uno::Reference < com::sun::star::i18n::XBreakIterator > xBreak;
    ::com::sun::star::util::SearchResult SAL_CALL
        ApproxSrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        ApproxSrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    bool IsDelimiter( const ::rtl::OUString& rStr, sal_Int32 nPos ) const;

    sal_Bool checkCTLStart, checkCTLEnd;
    sal_Bool SAL_CALL isCellStart(const ::rtl::OUString& searchStr, sal_Int32 nPos)
                            throw(::com::sun::star::uno::RuntimeException);

public:
    TextSearch(
        const ::com::sun::star::uno::Reference < ::com::sun::star::uno::XComponentContext >& rxContext );

    virtual ~TextSearch();

    // Methods
    virtual void SAL_CALL
        setOptions( const ::com::sun::star::util::SearchOptions& options )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::SearchResult SAL_CALL
        searchForward( const ::rtl::OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::SearchResult SAL_CALL
        searchBackward( const ::rtl::OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
