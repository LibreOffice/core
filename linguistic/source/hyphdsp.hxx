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

#ifndef INCLUDED_LINGUISTIC_SOURCE_HYPHDSP_HXX
#define INCLUDED_LINGUISTIC_SOURCE_HYPHDSP_HXX


#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>

#include <cppuhelper/implbase.hxx>

#include <map>
#include <memory>

#include "lngopt.hxx"
#include <linguistic/misc.hxx>
#include "defs.hxx"

class LngSvcMgr;


class HyphenatorDispatcher :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XHyphenator
    >,
    public LinguDispatcher
{
    typedef std::shared_ptr< LangSvcEntries_Hyph >                LangSvcEntries_Hyph_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Hyph_Ptr_t >     HyphSvcByLangMap_t;
    HyphSvcByLangMap_t      aSvcMap;

    css::uno::Reference< css::linguistic2::XLinguProperties >          xPropSet;
    css::uno::Reference< css::linguistic2::XSearchableDictionaryList > xDicList;

    LngSvcMgr      &rMgr;

    HyphenatorDispatcher(const HyphenatorDispatcher &) = delete;
    HyphenatorDispatcher & operator = (const HyphenatorDispatcher &) = delete;

    inline css::uno::Reference< css::linguistic2::XLinguProperties >
            GetPropSet();
    inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    static css::uno::Reference< css::linguistic2::XHyphenatedWord>
            buildHyphWord( const OUString& rOrigWord,
                const css::uno::Reference< css::linguistic2::XDictionaryEntry> &xEntry,
                LanguageType nLang, sal_Int16 nMaxLeading );

    static css::uno::Reference< css::linguistic2::XPossibleHyphens >
            buildPossHyphens( const css::uno::Reference< css::linguistic2::XDictionaryEntry > &xEntry,
                    LanguageType nLanguage );

public:
    explicit HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~HyphenatorDispatcher() override;

    // XSupportedLocales
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL
        getLocales() override;
    virtual sal_Bool SAL_CALL
        hasLocale( const css::lang::Locale& aLocale ) override;

    // XHyphenator
    virtual css::uno::Reference< css::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const OUString& aWord,
                const css::lang::Locale& aLocale,
                sal_Int16 nMaxLeading,
                const css::uno::Sequence< ::css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const OUString& aWord,
                const css::lang::Locale& aLocale,
                sal_Int16 nIndex,
                const css::uno::Sequence< ::css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference<
            css::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const OUString& aWord,
                const css::lang::Locale& aLocale,
                const css::uno::Sequence< ::css::beans::PropertyValue >& aProperties ) override;

    // LinguDispatcher
    virtual void
        SetServiceList( const css::lang::Locale &rLocale,
                const css::uno::Sequence< OUString > &rSvcImplNames ) override;
    virtual css::uno::Sequence< OUString >
        GetServiceList( const css::lang::Locale &rLocale ) const override;
};


inline css::uno::Reference< css::linguistic2::XLinguProperties >
        HyphenatorDispatcher::GetPropSet()
{
    return xPropSet.is() ?
                xPropSet : xPropSet = ::linguistic::GetLinguProperties();
}


inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
        HyphenatorDispatcher::GetDicList()
{
    return xDicList.is() ?
                xDicList : xDicList = ::linguistic::GetDictionaryList();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
