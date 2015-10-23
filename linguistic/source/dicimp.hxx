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

#ifndef INCLUDED_LINGUISTIC_SOURCE_DICIMP_HXX
#define INCLUDED_LINGUISTIC_SOURCE_DICIMP_HXX

#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/stream.hxx>

#include "defs.hxx"
#include "linguistic/misc.hxx"

#define DIC_MAX_ENTRIES     30000

sal_Int16 ReadDicVersion( SvStreamPtr &rpStream, sal_uInt16 &nLng, bool &bNeg );

class DictionaryNeo :
    public ::cppu::WeakImplHelper
    <
        css::linguistic2::XDictionary,
        css::frame::XStorable
    >
{

    ::cppu::OInterfaceContainerHelper                           aDicEvtListeners;
    css::uno::Sequence< css::uno::Reference<  css::linguistic2::XDictionaryEntry > >
                                                                aEntries;
    OUString                                                    aDicName;
    OUString                                                    aMainURL;
    css::linguistic2::DictionaryType                            eDicType;
    sal_Int16                                                   nCount;
    sal_Int16                                                   nLanguage;
    sal_Int16                                                   nDicVersion;
    bool                                                        bNeedEntries;
    bool                                                        bIsModified;
    bool                                                        bIsActive;
    bool                                                        bIsReadonly;

    DictionaryNeo(const DictionaryNeo &) = delete;
    DictionaryNeo & operator = (const DictionaryNeo &) = delete;

    void                    launchEvent(sal_Int16 nEvent,
                                        css::uno::Reference< css::linguistic2::XDictionaryEntry > xEntry);

    sal_uLong                   loadEntries(const OUString &rMainURL);
    sal_uLong                   saveEntries(const OUString &rMainURL);
    static int                  cmpDicEntry(const OUString &rWord1,
                                        const OUString &rWord2,
                                        bool bSimilarOnly = false);
    bool                        seekEntry(const OUString &rWord, sal_Int32 *pPos,
                                        bool bSimilarOnly = false);
    bool                        isSorted();

    bool                        addEntry_Impl(const css::uno::Reference< css::linguistic2::XDictionaryEntry >& rDicEntry,
                                          bool bIsLoadEntries = false);

public:
    DictionaryNeo(const OUString &rName, sal_Int16 nLang,
                    css::linguistic2::DictionaryType eType,
                    const OUString &rMainURL,
                    bool bWriteable );
    virtual ~DictionaryNeo();

    // XNamed
    virtual OUString SAL_CALL
        getName()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        setName( const OUString& aName )
            throw(css::uno::RuntimeException, std::exception) override;

    // XDictionary
    virtual css::linguistic2::DictionaryType SAL_CALL
        getDictionaryType()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        setActive( sal_Bool bActivate )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        isActive()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL
        getCount()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL
        getLocale()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        setLocale( const css::lang::Locale& aLocale )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
            css::linguistic2::XDictionaryEntry > SAL_CALL
        getEntry( const OUString& aWord )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        addEntry( const css::uno::Reference<
                css::linguistic2::XDictionaryEntry >& xDicEntry )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        add( const OUString& aWord, sal_Bool bIsNegative,
                const OUString& aRplcText )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        remove( const OUString& aWord )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        isFull()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionaryEntry > > SAL_CALL
        getEntries()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        clear()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        addDictionaryEventListener( const css::uno::Reference< css::linguistic2::XDictionaryEventListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        removeDictionaryEventListener( const css::uno::Reference< css::linguistic2::XDictionaryEventListener >& xListener )
            throw(css::uno::RuntimeException, std::exception) override;

    // XStorable
    virtual sal_Bool SAL_CALL
        hasLocation()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL
        getLocation()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        isReadonly()
            throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        store()
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        storeAsURL( const OUString& aURL,
                const css::uno::Sequence< css::beans::PropertyValue >& aArgs )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        storeToURL( const OUString& aURL,
                const css::uno::Sequence< css::beans::PropertyValue >& aArgs )
            throw(css::io::IOException,
                  css::uno::RuntimeException, std::exception) override;
};



class DicEntry :
    public cppu::WeakImplHelper< css::linguistic2::XDictionaryEntry >
{
    OUString aDicWord,       // including hyphen positions represented by "="
                    aReplacement;   // including hyphen positions represented by "="
    bool            bIsNegativ;

    DicEntry(const DicEntry &) = delete;
    DicEntry & operator = (const DicEntry &) = delete;

    static void            splitDicFileWord(const OUString &rDicFileWord,
                                     OUString &rDicWord,
                                     OUString &rReplacement);

public:
    DicEntry(const OUString &rDicFileWord, bool bIsNegativ);
    DicEntry(const OUString &rDicWord, bool bIsNegativ,
             const OUString &rRplcText);
    virtual ~DicEntry();

    // XDictionaryEntry
    virtual OUString SAL_CALL
        getDictionaryWord() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        isNegative() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL
        getReplacementText() throw(css::uno::RuntimeException, std::exception) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
