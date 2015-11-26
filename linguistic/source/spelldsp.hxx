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

#ifndef INCLUDED_LINGUISTIC_SOURCE_SPELLDSP_HXX
#define INCLUDED_LINGUISTIC_SOURCE_SPELLDSP_HXX

#include "lngopt.hxx"
#include "linguistic/misc.hxx"
#include "iprcache.hxx"

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

#include <map>
#include <memory>
#include <unotools/charclass.hxx>

class LngSvcMgr;


class SpellCheckerDispatcher :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XSpellChecker1,
        css::linguistic2::XSpellChecker
    >,
    public LinguDispatcher
{
    typedef std::shared_ptr< LangSvcEntries_Spell >               LangSvcEntries_Spell_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Spell_Ptr_t >    SpellSvcByLangMap_t;
    SpellSvcByLangMap_t     aSvcMap;

    css::uno::Reference< css::linguistic2::XLinguProperties >           xPropSet;
    css::uno::Reference< css::linguistic2::XSearchableDictionaryList >  xDicList;

    LngSvcMgr                   &rMgr;
    mutable linguistic::SpellCache      *pCache; // Spell Cache (holds known words)
    CharClass                   * pCharClass;

    SpellCheckerDispatcher(const SpellCheckerDispatcher &) = delete;
    SpellCheckerDispatcher & operator = (const SpellCheckerDispatcher &) = delete;

    inline linguistic::SpellCache &  GetCache() const;

    inline css::uno::Reference< css::linguistic2::XLinguProperties >
            GetPropSet();
    inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    bool    isValid_Impl(const OUString& aWord, LanguageType nLanguage,
                    const css::beans::PropertyValues& aProperties,
                    bool bCheckDics)
                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception );

    css::uno::Reference<
        css::linguistic2::XSpellAlternatives >
            spell_Impl(const OUString& aWord, LanguageType nLanguage,
                    const css::beans::PropertyValues& aProperties,
                    bool bCheckDics)
                throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception );

public:
    explicit SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~SpellCheckerDispatcher();

    // XSupportedLocales (for XSpellChecker)
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasLocale( const css::lang::Locale& aLocale ) throw(css::uno::RuntimeException, std::exception) override;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, const css::lang::Locale& aLocale, const css::beans::PropertyValues& aProperties ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, const css::lang::Locale& aLocale, const css::beans::PropertyValues& aProperties ) throw(css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // XSupportedLanguages
    virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getLanguages(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasLanguage( ::sal_Int16 nLanguage ) throw (css::uno::RuntimeException, std::exception) override;

    // XSpellChecker1
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, ::sal_Int16 nLanguage, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, ::sal_Int16 nLanguage, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

    // LinguDispatcher
    virtual void SetServiceList( const css::lang::Locale &rLocale, const css::uno::Sequence< OUString > &rSvcImplNames ) override;
    virtual css::uno::Sequence< OUString > GetServiceList( const css::lang::Locale &rLocale ) const override;

    void    FlushSpellCache();

private:
    void setCharClass(const LanguageTag& rLanguageTag);
    static OUString SAL_CALL makeLowerCase(const OUString&, CharClass *);

};

inline linguistic::SpellCache & SpellCheckerDispatcher::GetCache() const
{
    if (!pCache)
        pCache = new linguistic::SpellCache();
    return *pCache;
}


inline css::uno::Reference< css::linguistic2::XLinguProperties >
        SpellCheckerDispatcher::GetPropSet()
{
    return xPropSet.is() ?
        xPropSet : xPropSet = linguistic::GetLinguProperties();
}


inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
        SpellCheckerDispatcher::GetDicList()
{
    return xDicList.is() ?
        xDicList : xDicList = linguistic::GetDictionaryList();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
