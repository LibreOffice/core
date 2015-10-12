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
    < ::com::sun::star::embed::XOLESimpleStorage
                , ::com::sun::star::lang::XInitialization
                , ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;

    bool m_bDisposed;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xTempStream;
    SvStream* m_pStream;
    BaseStorage* m_pStorage;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    bool m_bNoTemporaryCopy;

    void UpdateOriginal_Impl();

    static void InsertInputStreamToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream )
    throw ( ::com::sun::star::uno::Exception );

    static void InsertNameAccessToStorage_Impl( BaseStorage* pStorage, const OUString & aName, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xNameAccess )
    throw ( ::com::sun::star::uno::Exception );

public:

    explicit OLESimpleStorage( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    virtual ~OLESimpleStorage();

    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL impl_staticGetSupportedServiceNames();
    static OUString SAL_CALL impl_staticGetImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );



    //  XInitialization


    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException, std::exception) override;


    //  XNameContainer


    virtual void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeByName( const OUString& Name )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    //  XComponent


    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;


    //  XTransactedObject


    virtual void SAL_CALL commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;


    //  XClassifiedObject


    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getClassID()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual OUString SAL_CALL getClassName()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setClassInfo( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aClassID,
                                        const OUString& sClassName )
        throw ( ::com::sun::star::lang::NoSupportException,
                ::com::sun::star::uno::RuntimeException, std::exception ) override;


    //  XServiceInfo


    virtual OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
