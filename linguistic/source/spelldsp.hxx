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

#ifndef _LINGUISTIC_SPELLDSP_HXX_
#define _LINGUISTIC_SPELLDSP_HXX_

#include "lngopt.hxx"
#include "linguistic/misc.hxx"
#include "iprcache.hxx"

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase7.hxx>
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

#include <boost/shared_ptr.hpp>
#include <map>

class LngSvcMgr;


class SpellCheckerDispatcher :
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XSpellChecker1,
        ::com::sun::star::linguistic2::XSpellChecker
    >,
    public LinguDispatcher
{
    typedef boost::shared_ptr< LangSvcEntries_Spell >               LangSvcEntries_Spell_Ptr_t;
    typedef std::map< LanguageType, LangSvcEntries_Spell_Ptr_t >    SpellSvcByLangMap_t;
    SpellSvcByLangMap_t     aSvcMap;
    LinguOptions            aOpt;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >                     xPropSet;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >  xDicList;

    LngSvcMgr                   &rMgr;
    mutable linguistic::SpellCache      *pCache; // Spell Cache (holds known words)

    // disallow copy-constructor and assignment-operator for now
    SpellCheckerDispatcher(const SpellCheckerDispatcher &);
    SpellCheckerDispatcher & operator = (const SpellCheckerDispatcher &);

    inline linguistic::SpellCache &  GetCache() const;

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
            GetPropSet();
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSearchableDictionaryList >
            GetDicList();

    void    ClearSvcList();

    sal_Bool    isValid_Impl(const ::rtl::OUString& aWord, LanguageType nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    sal_Bool bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellAlternatives >
            spell_Impl(const ::rtl::OUString& aWord, LanguageType nLanguage,
                    const ::com::sun::star::beans::PropertyValues& aProperties,
                    sal_Bool bCheckDics)
                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

public:
    SpellCheckerDispatcher( LngSvcMgr &rLngSvcMgr );
    virtual ~SpellCheckerDispatcher();

    // XSupportedLocales (for XSpellChecker)
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale > SAL_CALL getLocales() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasLocale( const ::com::sun::star::lang::Locale& aLocale ) throw(::com::sun::star::uno::RuntimeException);

    // XSpellChecker
    virtual sal_Bool SAL_CALL isValid( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL spell( const ::rtl::OUString& aWord, const ::com::sun::star::lang::Locale& aLocale, const ::com::sun::star::beans::PropertyValues& aProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XSupportedLanguages
    virtual ::com::sun::star::uno::Sequence< ::sal_Int16 > SAL_CALL getLanguages(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasLanguage( ::sal_Int16 nLanguage ) throw (::com::sun::star::uno::RuntimeException);

    // XSpellChecker1
    virtual ::sal_Bool SAL_CALL isValid( const ::rtl::OUString& aWord, ::sal_Int16 nLanguage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellAlternatives > SAL_CALL spell( const ::rtl::OUString& aWord, ::sal_Int16 nLanguage, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // LinguDispatcher
    virtual void SetServiceList( const ::com::sun::star::lang::Locale &rLocale, const ::com::sun::star::uno::Sequence< rtl::OUString > &rSvcImplNames );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > GetServiceList( const ::com::sun::star::lang::Locale &rLocale ) const;
    virtual DspType GetDspType() const;

    void    FlushSpellCache();
};


inline linguistic::SpellCache & SpellCheckerDispatcher::GetCache() const
{
    if (!pCache)
        pCache = new linguistic::SpellCache();
    return *pCache;
}


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
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
        xDicList : xDicList = linguistic::GetSearchableDictionaryList();
}



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
