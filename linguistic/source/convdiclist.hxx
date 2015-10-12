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
        ::com::sun::star::linguistic2::XConversionDictionaryList,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{

    class MyAppExitListener : public linguistic::AppExitListener
    {
        ConvDicList & rMyDicList;

    public:
        explicit MyAppExitListener( ConvDicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit() SAL_OVERRIDE;
    };


    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    ConvDicNameContainer                   *pNameContainer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameContainer >   xNameContainer;

    MyAppExitListener                      *pExitListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::
                XTerminateListener >        xExitListener;

    bool                                    bDisposing;

    ConvDicList( const ConvDicList & ) = delete;
    ConvDicList & operator = (const ConvDicList &) = delete;

    ConvDicNameContainer &  GetNameContainer();

public:
    ConvDicList();
    virtual ~ConvDicList();

    // XConversionDictionaryList
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getDictionaryContainer(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > SAL_CALL addNewDictionary( const OUString& aName, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL queryConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL queryMaxCharCount( const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    static inline OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< OUString >
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
