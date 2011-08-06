/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
