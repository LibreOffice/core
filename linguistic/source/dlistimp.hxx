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


    void                CreateDicList();
    DictionaryVec_t &   GetOrCreateDicList()
                        {
                            if ( !bInCreation && aDicList.empty() )
                                CreateDicList();
                            return aDicList;
                        }

    void                SearchForDictionaries( DictionaryVec_t &rDicList,
                                            const OUString &rDicDir, bool bIsWritePath );
    sal_Int32           GetDicPos(const css::uno::Reference<
                            css::linguistic2::XDictionary > &xDic);

public:
    DicList();
    virtual ~DicList() override;

    // XDictionaryList
    virtual ::sal_Int16 SAL_CALL getCount(  ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::linguistic2::XDictionary > > SAL_CALL getDictionaries(  ) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > SAL_CALL getDictionaryByName( const OUString& aDictionaryName ) override;
    virtual sal_Bool SAL_CALL addDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) override;
    virtual sal_Bool SAL_CALL removeDictionary( const css::uno::Reference< css::linguistic2::XDictionary >& xDictionary ) override;
    virtual sal_Bool SAL_CALL addDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener, sal_Bool bReceiveVerbose ) override;
    virtual sal_Bool SAL_CALL removeDictionaryListEventListener( const css::uno::Reference< css::linguistic2::XDictionaryListEventListener >& xListener ) override;
    virtual ::sal_Int16 SAL_CALL beginCollectEvents(  ) override;
    virtual ::sal_Int16 SAL_CALL endCollectEvents(  ) override;
    virtual ::sal_Int16 SAL_CALL flushEvents(  ) override;
    virtual css::uno::Reference< css::linguistic2::XDictionary > SAL_CALL createDictionary( const OUString& aName, const css::lang::Locale& aLocale, css::linguistic2::DictionaryType eDicType, const OUString& aURL ) override;

    // XSearchableDictionaryList
    virtual css::uno::Reference< css::linguistic2::XDictionaryEntry > SAL_CALL queryDictionaryEntry( const OUString& aWord, const css::lang::Locale& aLocale, sal_Bool bSearchPosDics, sal_Bool bSpellEntry ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XInitialization
    virtual void SAL_CALL
    initialize(const css::uno::Sequence<css::uno::Any>& /*rArguments*/) override;

    // non UNO-specific
    void    SaveDics();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
