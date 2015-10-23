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
#include <cppuhelper/interfacecontainer.h>

#include "linguistic/misc.hxx"
#include "lngopt.hxx"


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


    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    ConvDicNameContainer                   *pNameContainer;
    css::uno::Reference<
        css::container::XNameContainer >   xNameContainer;

    MyAppExitListener                      *pExitListener;
    css::uno::Reference< css::frame::
                XTerminateListener >        xExitListener;

    bool                                    bDisposing;

    ConvDicList( const ConvDicList & ) = delete;
    ConvDicList & operator = (const ConvDicList &) = delete;

    ConvDicNameContainer &  GetNameContainer();

public:
    ConvDicList();
    virtual ~ConvDicList();

    // XConversionDictionaryList
    virtual css::uno::Reference< css::container::XNameContainer > SAL_CALL getDictionaryContainer(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::linguistic2::XConversionDictionary > SAL_CALL addNewDictionary( const OUString& aName, const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType ) throw (css::lang::NoSupportException, css::container::ElementExistException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL queryConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, css::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL queryMaxCharCount( const css::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, css::linguistic2::ConversionDirection eDirection ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;


    static inline OUString
        getImplementationName_Static() throw();
    static css::uno::Sequence< OUString >
        getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void                    FlushDics();
};

inline OUString ConvDicList::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu2.ConvDicList" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
