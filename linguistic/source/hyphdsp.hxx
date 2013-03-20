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


#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations

#include <boost/shared_ptr.hpp>
#include <map>

#include "lngopt.hxx"
#include "linguistic/misc.hxx"
#include "defs.hxx"

class LngSvcMgr;


class HyphenatorDispatcher :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::linguistic2::XHyphenator
    >,
    public LinguDispatcher
{
    typedef boost::shared_ptr< LangSvcEntries_Hyph >                LangSvcEntries_Hyph_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Hyph_Ptr_t >     HyphSvcByLangMap_t;
    HyphSvcByLangMap_t      aSvcMap;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >                     xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList > xDicList;

    LngSvcMgr      &rMgr;

    // disallow copy-constructor and assignment-operator for now
    HyphenatorDispatcher(const HyphenatorDispatcher &);
    HyphenatorDispatcher & operator = (const HyphenatorDispatcher &);

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord>
            buildHyphWord( const rtl::OUString rOrigWord,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry> &xEntry,
                sal_Int16 nLang, sal_Int16 nMaxLeading );

    com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XPossibleHyphens >
            buildPossHyphens( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XDictionaryEntry > &xEntry,
                    sal_Int16 nLanguage );

public:
    HyphenatorDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~HyphenatorDispatcher();

    // XSupportedLocales
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::lang::Locale > SAL_CALL
        getLocales()
            throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
        hasLocale( const ::com::sun::star::lang::Locale& aLocale )
            throw(::com::sun::star::uno::RuntimeException);

    // XHyphenator
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        hyphenate( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nMaxLeading,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > SAL_CALL
        queryAlternativeSpelling( const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                sal_Int16 nIndex,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XPossibleHyphens > SAL_CALL
        createPossibleHyphens(
                const ::rtl::OUString& aWord,
                const ::com::sun::star::lang::Locale& aLocale,
                const ::com::sun::star::beans::PropertyValues& aProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);

    // LinguDispatcher
    virtual void
        SetServiceList( const ::com::sun::star::lang::Locale &rLocale,
                const ::com::sun::star::uno::Sequence<
                    rtl::OUString > &rSvcImplNames );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString >
        GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const;
    virtual DspType
        GetDspType() const;
};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
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
