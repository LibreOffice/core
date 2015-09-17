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
        ::com::sun::star::linguistic2::XSpellChecker1,
        ::com::sun::star::linguistic2::XSpellChecker
    >,
    public LinguDispatcher
{
    typedef std::shared_ptr< LangSvcEntries_Spell >               LangSvcEntries_Spell_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Spell_Ptr_t >    SpellSvcByLangMap_t;
    SpellSvcByLangMap_t     aSvcMap;
    LinguOptions            aOpt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguProperties >           xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >  xDicList;

    LngSvcMgr                   &rMgr;
    mutable linguistic::SpellCache      *pCache; // Spell Cache (holds known words)
    CharClass                   * pCharClass;

    SpellCheckerDispatcher(const SpellCheckerDispatcher &) SAL_DELETED_FUNCTION;
    SpellCheckerDispatcher & operator = (const SpellCheckerDispatcher &) SAL_DELETED_FUNCTION;

    inline linguistic::SpellCache &  GetCache() const;

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguProperties >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    bool    isValid_Impl(const OUString& aWord, LanguageType nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    bool bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            spell_Impl(const OUString& aWord, LanguageType nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    bool bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, std::exception );

public:
    explicit SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~SpellCheckerDispatcher();

    // XSupportedLocales (for XSpellChecker)
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale& aLocale ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSupportedLanguages
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getLanguages(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasLanguage( ::sal_Int16 nLanguage ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XSpellChecker1
    virtual sal_Bool SAL_CALL isValid( const OUString& aWord, ::sal_Int16 nLanguage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL spell( const OUString& aWord, ::sal_Int16 nLanguage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // LinguDispatcher
    virtual void SetServiceList( const ::com::sun::star::lang::Locale &rLocale, const ::com::sun::star::uno::Sequence< OUString > &rSvcImplNames ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const SAL_OVERRIDE;

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


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XLinguProperties >
        SpellCheckerDispatcher::GetPropSet()
{
    return xPropSet.is() ?
        xPropSet : xPropSet = linguistic::GetLinguProperties();
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSearchableDictionaryList >
        SpellCheckerDispatcher::GetDicList()
{
    return xDicList.is() ?
        xDicList : xDicList = linguistic::GetDictionaryList();
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
