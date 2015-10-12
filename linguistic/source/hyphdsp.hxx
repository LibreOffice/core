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
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <cppuhelper/implbase.hxx>

#include <map>
#include <memory>

#include "lngopt.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

class LngSvcMgr;


class HyphenatorDispatcher :
    public cppu::WeakImplHelper
    <
        ::com::sun::star::linguistic2::XHyphenator
    >,
    public LinguDispatcher
{
    typedef std::shared_ptr< LangSvcEntries_Hyph >                LangSvcEntries_Hyph_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Hyph_Ptr_t >     HyphSvcByLangMap_t;
    HyphSvcByLangMap_t      aSvcMap;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguProperties >          xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList > xDicList;

    LngSvcMgr      &rMgr;

    HyphenatorDispatcher(const HyphenatorDispatcher &) = delete;
    HyphenatorDispatcher & operator = (const HyphenatorDispatcher &) = delete;

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguProperties >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    static com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord>
            buildHyphWord( const OUString& rOrigWord,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry> &xEntry,
                sal_Int16 nLang, sal_Int16 nMaxLeading );

    static com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XPossibleHyphens >
            buildPossHyphens( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry > &xEntry,
                    sal_Int16 nLanguage );

public:
    explicit HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~HyphenatorDispatcher();

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XHyphenator
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nMaxLeading,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nIndex,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException, std::exception) override;

    // LinguDispatcher
    virtual void
        SetServiceList( const ::com::sun::star::lang::Locale &rLocale,
                const ::com::sun::star::uno::Sequence<
                    OUString > &rSvcImplNames ) override;
    virtual ::com::sun::star::uno::Sequence< OUString >
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const override;
};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XLinguProperties >
        HyphenatorDispatcher::GetPropSet()
{
    return xPropSet.is() ?
                xPropSet : xPropSet = ::linguistic::GetLinguProperties();
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSearchableDictionaryList >
        HyphenatorDispatcher::GetDicList()
{
    return xDicList.is() ?
                xDicList : xDicList = ::linguistic::GetDictionaryList();
}




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
