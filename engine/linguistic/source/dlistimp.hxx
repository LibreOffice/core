/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <comphelper/interfacecontainer3.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <vector>

#include <linguistic/misc.hxx>
#include "lngopt.hxx"

class DicEvtListenerHelper;

class DicList : public cppu::WeakImplHelper<css::linguistic2::XSearchableDictionaryList,
                                            css::lang::XComponent, css::lang::XServiceInfo,
                                            css::lang::XInitialization>
{
    class MyAppExitListener : public linguistic::AppExitListener
    {
        DicList & rMyDicList;

    public:
        explicit MyAppExitListener( DicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit() override;
    };

    LinguOptions    aOpt;

    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> aEvtListeners;

    typedef std::vector< css::uno::Reference< css::linguistic2::XDictionary > >   DictionaryVec_t;
    DictionaryVec_t                             aDicList;

    rtl::Reference<DicEvtListenerHelper>        mxDicEvtLstnrHelper;
    rtl::Reference<MyAppExitListener>           mxExitListener;

    bool    bDisposing;
    bool    bInCreation;

    DicList( const DicList & ) = delete;
    DicList & operator = (const DicList &) = delete;


    void                CreateDicList(bool bDispatchEvents);
    DictionaryVec_t &   GetOrCreateDicList()
                        {
                            if ( !bInCreation && aDicList.empty() )
                                CreateDicList(/*bDispatchEvents=*/false);
                            return aDicList;
                        }

    void                SearchForDictionaries( DictionaryVec_t &rDicList,
                                            const OUString &rDicDir, bool bIsWritePath, bool bDispatchEvents );
    sal_Int32           GetDicPos(const css::uno::Reference<
                            css::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList() override;

    // XDictionaryList
    virtual ::sal_Int16 getCount(  ) override;
    virtual cpo::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionary > > getDictionaries(  ) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > getDictionaryByName( const OUString& aDictionaryName ) override;
    virtual bool addDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) override;
    virtual bool removeDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) override;
    virtual bool addDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener, bool bReceiveVerbose ) override;
    virtual bool removeDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener ) override;
    virtual ::sal_Int16 beginCollectEvents(  ) override;
    virtual ::sal_Int16 endCollectEvents(  ) override;
    virtual ::sal_Int16 flushEvents(  ) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > createDictionary( const OUString& aName, const css::lang::Locale& aLocale, css::linguistic2::DictionaryType eDicType, const OUString& aURL ) override;

    // XSearchableDictionaryList
    virtual css::uno::Reference< css::linguistic2::XDictionaryEntry > queryDictionaryEntry( const OUString& aWord, const css::lang::Locale& aLocale, bool bSearchPosDics, bool bSpellEntry ) override;

    // XComponent
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    // XInitialization
    virtual void
    initialize(const cpo::uno::Sequence<cpo::uno::Any>& /*rArguments*/) override;

    // non UNO-specific
    void    SaveDics();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
