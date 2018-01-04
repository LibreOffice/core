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
#include <comphelper/interfacecontainer2.hxx>
#include <tools/solar.h>
#include <vcl/errcode.hxx>

#include "defs.hxx"
#include <linguistic/misc.hxx>

#define DIC_MAX_ENTRIES     30000

sal_Int16 ReadDicVersion( SvStreamPtr const &rpStream, LanguageType &nLng, bool &bNeg, OUString &aDicName );

class DictionaryNeo :
    public ::cppu::WeakImplHelper
    <
        css::linguistic2::XDictionary,
        css::frame::XStorable
    >
{

    ::comphelper::OInterfaceContainerHelper2                    aDicEvtListeners;
    std::vector< css::uno::Reference< css::linguistic2::XDictionaryEntry > >
                                                                aEntries;
    OUString                                                    aDicName;
    OUString                                                    aMainURL;
    css::linguistic2::DictionaryType                            eDicType;
    LanguageType                                                nLanguage;
    sal_Int16                                                   nDicVersion;
    bool                                                        bNeedEntries;
    bool                                                        bIsModified;
    bool                                                        bIsActive;
    bool                                                        bIsReadonly;

    DictionaryNeo(const DictionaryNeo &) = delete;
    DictionaryNeo & operator = (const DictionaryNeo &) = delete;

    void                    launchEvent(sal_Int16 nEvent,
                                        const css::uno::Reference< css::linguistic2::XDictionaryEntry >& xEntry);

    ErrCode                     loadEntries(const OUString &rMainURL);
    ErrCode                     saveEntries(const OUString &rMainURL);
    static int                  cmpDicEntry(const OUString &rWord1,
                                        const OUString &rWord2,
                                        bool bSimilarOnly = false);
    bool                        seekEntry(const OUString &rWord, sal_Int32 *pPos,
                                        bool bSimilarOnly = false);
    bool                        isSorted();

    bool                        addEntry_Impl(const css::uno::Reference< css::linguistic2::XDictionaryEntry >& rDicEntry,
                                          bool bIsLoadEntries = false);

public:
    DictionaryNeo(const OUString &rName, LanguageType nLang,
                    css::linguistic2::DictionaryType eType,
                    const OUString &rMainURL,
                    bool bWriteable );
    virtual ~DictionaryNeo() override;

    // XNamed
    virtual OUString SAL_CALL
        getName() override;
    virtual void SAL_CALL
        setName( const OUString& aName ) override;

    // XDictionary
    virtual css::linguistic2::DictionaryType SAL_CALL
        getDictionaryType() override;
    virtual void SAL_CALL
        setActive( sal_Bool bActivate ) override;
    virtual sal_Bool SAL_CALL
        isActive() override;
    virtual sal_Int32 SAL_CALL
        getCount() override;
    virtual css::lang::Locale SAL_CALL
        getLocale() override;
    virtual void SAL_CALL
        setLocale( const css::lang::Locale& aLocale ) override;
    virtual css::uno::Reference<
            css::linguistic2::XDictionaryEntry > SAL_CALL
        getEntry( const OUString& aWord ) override;
    virtual sal_Bool SAL_CALL
        addEntry( const css::uno::Reference<
                css::linguistic2::XDictionaryEntry >& xDicEntry ) override;
    virtual sal_Bool SAL_CALL
        add( const OUString& aWord, sal_Bool bIsNegative,
                const OUString& aRplcText ) override;
    virtual sal_Bool SAL_CALL
        remove( const OUString& aWord ) override;
    virtual sal_Bool SAL_CALL
        isFull() override;
    virtual css::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionaryEntry > > SAL_CALL
        getEntries() override;
    virtual void SAL_CALL
        clear() override;
    virtual sal_Bool SAL_CALL
        addDictionaryEventListener( const css::uno::Reference< css::linguistic2::XDictionaryEventListener >& xListener ) override;
    virtual sal_Bool SAL_CALL
        removeDictionaryEventListener( const css::uno::Reference< css::linguistic2::XDictionaryEventListener >& xListener ) override;

    // XStorable
    virtual sal_Bool SAL_CALL
        hasLocation() override;
    virtual OUString SAL_CALL
        getLocation() override;
    virtual sal_Bool SAL_CALL
        isReadonly() override;
    virtual void SAL_CALL
        store() override;
    virtual void SAL_CALL
        storeAsURL( const OUString& aURL,
                const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
    virtual void SAL_CALL
        storeToURL( const OUString& aURL,
                const css::uno::Sequence< css::beans::PropertyValue >& aArgs ) override;
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
    virtual ~DicEntry() override;

    // XDictionaryEntry
    virtual OUString SAL_CALL
        getDictionaryWord() override;
    virtual sal_Bool SAL_CALL
        isNegative() override;
    virtual OUString SAL_CALL
        getReplacementText() override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
