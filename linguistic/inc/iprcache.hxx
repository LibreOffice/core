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

#ifndef _LINGUISTIC_IPRCACHE_HXX_
#define _LINGUISTIC_IPRCACHE_HXX_


#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>

#include <rtl/string.hxx>
#include <i18npool/lang.h>

#include <set>
#include <map>

namespace linguistic
{


class Flushable
{
public:
    virtual void    Flush() = 0;

protected:
    ~Flushable() {}
};


class FlushListener :
    public cppu::WeakImplHelper2
    <
        ::com::sun::star::linguistic2::XDictionaryListEventListener,
        ::com::sun::star::beans::XPropertyChangeListener
    >
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >    xDicList;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >             xPropSet;
    Flushable                                              *pFlushObj;

    // don't allow to use copy-constructor and assignment-operator
    FlushListener(const FlushListener &);
    FlushListener & operator = (const FlushListener &);

public:
    FlushListener( Flushable *pFO );
    virtual ~FlushListener();

    inline void SetFlushObj( Flushable *pFO)    { pFlushObj = pFO; }

    void        SetDicList( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryList > &rDL );
    void        SetPropSet( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > &rPS );

    //XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& rSource ) throw(::com::sun::star::uno::RuntimeException);

    // XDictionaryListEventListener
    virtual void SAL_CALL processDictionaryListEvent( const ::com::sun::star::linguistic2::DictionaryListEvent& rDicListEvent ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt ) throw(::com::sun::star::uno::RuntimeException);
};


class SpellCache :
    public Flushable
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryListEventListener >
                        xFlushLstnr;
    FlushListener      *pFlushLstnr;

    typedef std::set< ::rtl::OUString >             WordList_t;
    typedef std::map< LanguageType, WordList_t >    LangWordList_t;
    LangWordList_t  aWordLists;

    // don't allow to use copy-constructor and assignment-operator
    SpellCache(const SpellCache &);
    SpellCache & operator = (const SpellCache &);

public:
    SpellCache();
    virtual ~SpellCache();

    // Flushable
    virtual void    Flush();

    void    AddWord( const ::rtl::OUString& rWord, LanguageType nLang );
    bool    CheckWord( const ::rtl::OUString& rWord, LanguageType nLang );
};


}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
