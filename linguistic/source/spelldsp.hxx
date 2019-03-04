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
#include <linguistic/misc.hxx>
#include <iprcache.hxx>

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
    SpellSvcByLangMap_t     m_aSvcMap;

    css::uno::Reference< css::linguistic2::XLinguProperties >           m_xPropSet;
    css::uno::Reference< css::linguistic2::XSearchableDictionaryList >  m_xDicList;

    LngSvcMgr                       &m_rMgr;
    mutable std::unique_ptr<linguistic::SpellCache> m_pCache; // Spell Cache (holds known words)
    std::unique_ptr<CharClass>       m_pCharClass;

    SpellCheckerDispatcher(const SpellCheckerDispatcher &) = delete;
    SpellCheckerDispatcher & operator = (const SpellCheckerDispatcher &) = delete;

    inline linguistic::SpellCache &  GetCache() const;

    inline css::uno::Reference< css::linguistic2::XLinguProperties >
            GetPropSet();
    inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
            GetDicList();

    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    bool    isValid_Impl(const OUString& aWord, LanguageType nLanguage,
                    const css::beans::PropertyValues& aProperties);

    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    css::uno::Reference<
        css::linguistic2::XSpellAlternatives >
            spell_Impl(const OUString& aWord, LanguageType nLanguage,
                    const css::beans::PropertyValues& aProperties);

public:
    explicit SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~SpellCheckerDispatcher() override;

    // XSupportedLocales (for XSpellChecker)
    virtual css::uno::Sequence< css::lang::Locale > SAL_CALL getLocales() override;
    virtual sal_Bool SAL_CALL hasLocale( const css::lang::Locale& aLocale ) override;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, const css::lang::Locale& aLocale, const css::uno::Sequence< ::css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, const css::lang::Locale& aLocale, const css::uno::Sequence< ::css::beans::PropertyValue >& aProperties ) override;

    // XSupportedLanguages
    virtual css::uno::Sequence< ::sal_Int16 > SAL_CALL getLanguages(  ) override;
    virtual sal_Bool SAL_CALL hasLanguage( ::sal_Int16 nLanguage ) override;

    // XSpellChecker1
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, ::sal_Int16 nLanguage, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;
    virtual css::uno::Reference< css::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, ::sal_Int16 nLanguage, const css::uno::Sequence< css::beans::PropertyValue >& aProperties ) override;

    // LinguDispatcher
    virtual void SetServiceList( const css::lang::Locale &rLocale, const css::uno::Sequence< OUString > &rSvcImplNames ) override;
    virtual css::uno::Sequence< OUString > GetServiceList( const css::lang::Locale &rLocale ) const override;

    void    FlushSpellCache();

private:
    void setCharClass(const LanguageTag& rLanguageTag);
    static OUString makeLowerCase(const OUString&, CharClass const *);

};

inline linguistic::SpellCache & SpellCheckerDispatcher::GetCache() const
{
    if (!m_pCache)
        m_pCache.reset(new linguistic::SpellCache());
    return *m_pCache;
}


inline css::uno::Reference< css::linguistic2::XLinguProperties >
        SpellCheckerDispatcher::GetPropSet()
{
    return m_xPropSet.is() ?
        m_xPropSet : m_xPropSet = linguistic::GetLinguProperties();
}


inline css::uno::Reference< css::linguistic2::XSearchableDictionaryList >
        SpellCheckerDispatcher::GetDicList()
{
    return m_xDicList.is() ?
        m_xDicList : m_xDicList = linguistic::GetDictionaryList();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
