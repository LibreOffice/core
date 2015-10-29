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

#ifndef INCLUDED_SOT_SOURCE_UNOOLESTORAGE_XOLESIMPLESTORAGE_HXX
#define INCLUDED_SOT_SOURCE_UNOOLESTORAGE_XOLESIMPLESTORAGE_HXX

#include <com/sun/star/embed/XOLESimpleStorage.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>


#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <osl/mutex.hxx>

#include <sot/stg.hxx>


class OLESimpleStorage  : public ::cppu::WeakImplHelper
    < css::embed::XOLESimpleStorage
    , css::lang::XInitialization
    , css::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;

    bool m_bDisposed;

    css::uno::Reference< css::io::XStream > m_xStream;
    css::uno::Reference< css::io::XStream > m_xTempStream;
    SvStream* m_pStream;
    BaseStorage* m_pStorage;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

    bool m_bNoTemporaryCopy;

    void UpdateOriginal_Impl();

    static void InsertInputStreamToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const css::uno::Reference< css::io::XInputStream >& xInputStream )
    throw ( css::uno::Exception );

    static void InsertNameAccessToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const css::uno::Reference< css::container::XNameAccess >& xNameAccess )
    throw ( css::uno::Exception );

public:

    explicit OLESimpleStorage( css::uno::Reference< css::lang::XMultiServiceFactory > xFactory );

    virtual ~OLESimpleStorage();

    static css::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();
    static OUString SAL_CALL impl_staticGetImplementationName();
    static css::uno::Reference< css::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager );



    //  XInitialization


    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception,
                css::uno::RuntimeException, std::exception) override;


    //  XNameContainer


    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement )
        throw ( css::lang::IllegalArgumentException,
                css::container::ElementExistException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeByName( const OUString& Name )
        throw ( css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw ( css::lang::IllegalArgumentException,
                css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( css::container::NoSuchElementException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Type SAL_CALL getElementType()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw ( css::uno::RuntimeException, std::exception ) override;


    //  XComponent


    virtual void SAL_CALL dispose()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener >& xListener )
        throw ( css::uno::RuntimeException, std::exception ) override;


    //  XTransactedObject


    virtual void SAL_CALL commit()
        throw ( css::io::IOException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL revert()
        throw ( css::io::IOException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception ) override;


    //  XClassifiedObject


    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getClassID()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL getClassName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setClassInfo( const css::uno::Sequence< ::sal_Int8 >& aClassID,
                                        const OUString& sClassName )
        throw ( css::lang::NoSupportException,
                css::uno::RuntimeException, std::exception ) override;


    //  XServiceInfo


    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
