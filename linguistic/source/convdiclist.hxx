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

#ifndef INCLUDED_LINGUISTIC_SOURCE_CONVDICLIST_HXX
#define INCLUDED_LINGUISTIC_SOURCE_CONVDICLIST_HXX

#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <rtl/ref.hxx>

#include <linguistic/misc.hxx>


class ConvDicNameContainer;


class ConvDicList :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XConversionDictionaryList,
        css::lang::XComponent,
        css::lang::XServiceInfo
    >
{

    class MyAppExitListener : public linguistic::AppExitListener
    {
        ConvDicList & rMyDicList;

    public:
        explicit MyAppExitListener( ConvDicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit() override;
    };

    ::comphelper::OInterfaceContainerHelper2  aEvtListeners;
    rtl::Reference<ConvDicNameContainer>      mxNameContainer;
    rtl::Reference<MyAppExitListener>         mxExitListener;
    bool                                      bDisposing;

    ConvDicList( const ConvDicList & ) = delete;
    ConvDicList & operator = (const ConvDicList &) = delete;

    ConvDicNameContainer &  GetNameContainer();

public:
    ConvDicList();
    virtual ~ConvDicList() override;

    // XConversionDictionaryList
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getDictionaryContainer(  ) override;
    virtual css::uno::Reference< css::linguistic2::XConversionDictionary > SAL_CALL addNewDictionary( const OUString& aName, const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL queryConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, css::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) override;
    virtual sal_Int16 SAL_CALL queryMaxCharCount( const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, css::linguistic2::ConversionDirection eDirection ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // non UNO-specific
    void                    FlushDics();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
