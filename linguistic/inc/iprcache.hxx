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

#ifndef INCLUDED_LINGUISTIC_INC_IPRCACHE_HXX
#define INCLUDED_LINGUISTIC_INC_IPRCACHE_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>

#include <rtl/string.hxx>
#include <i18nlangtag/lang.h>

#include <set>
#include <map>

namespace linguistic
{

class SpellCache;

class FlushListener :
    public cppu::WeakImplHelper
    <
        ::com::sun::star::linguistic2::XDictionaryListEventListener,
        ::com::sun::star::beans::XPropertyChangeListener
    >
{
    css::uno::Reference< css::linguistic2::XSearchableDictionaryList >    xDicList;
    css::uno::Reference< css::linguistic2::XLinguProperties >             xPropSet;
    SpellCache&                                                           mrSpellCache;

    FlushListener(const FlushListener &) = delete;
    FlushListener & operator = (const FlushListener &) = delete;

public:
    FlushListener( SpellCache& rFO ) : mrSpellCache(rFO) {}
    virtual ~FlushListener() {}

    void        SetDicList( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSearchableDictionaryList > &rDL );
    void        SetPropSet( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLinguProperties > &rPS );

    //XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XDictionaryListEventListener
    virtual void SAL_CALL processDictionaryListEvent( const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


class SpellCache
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryListEventListener >
                        xFlushLstnr;
    FlushListener      *pFlushLstnr;

    typedef std::set< OUString >             WordList_t;
    typedef std::map< LanguageType, WordList_t >    LangWordList_t;
    LangWordList_t  aWordLists;

    SpellCache(const SpellCache &) = delete;
    SpellCache & operator = (const SpellCache &) = delete;

public:
    SpellCache();
    virtual ~SpellCache();

    // called from FlushListener
    void    Flush();

    void    AddWord( const OUString& rWord, LanguageType nLang );
    bool    CheckWord( const OUString& rWord, LanguageType nLang );
};


}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
