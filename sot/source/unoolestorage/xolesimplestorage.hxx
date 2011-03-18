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

#ifndef __XOLESIMPLESTORAGE_HXX_
#define __XOLESIMPLESTORAGE_HXX_

#include <com/sun/star/embed/XOLESimpleStorage.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>


#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <osl/mutex.hxx>

#include <sot/stg.hxx>


class OLESimpleStorage  : public ::cppu::WeakImplHelper3
    < ::com::sun::star::embed::XOLESimpleStorage
                , ::com::sun::star::lang::XInitialization
                , ::com::sun::star::lang::XServiceInfo >
{
    ::osl::Mutex m_aMutex;

    sal_Bool m_bDisposed;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > m_xTempStream;
    SvStream* m_pStream;
    BaseStorage* m_pStorage;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    sal_Bool m_bNoTemporaryCopy;

    void UpdateOriginal_Impl();

    static void InsertInputStreamToStorage_Impl( BaseStorage* pStorage, ::rtl::OUString aName, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInputStream )
    throw ( ::com::sun::star::uno::Exception );

    static void InsertNameAccessToStorage_Impl( BaseStorage* pStorage, ::rtl::OUString aName, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xNameAccess )
    throw ( ::com::sun::star::uno::Exception );

public:

    OLESimpleStorage( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory );

    virtual ~OLESimpleStorage();

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL impl_staticGetSupportedServiceNames();
    static ::rtl::OUString SAL_CALL impl_staticGetImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        impl_staticCreateSelfInstance(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );


    //____________________________________________________________________________________________________
    //  XInitialization
    //____________________________________________________________________________________________________

    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
    //  XNameContainer
    //____________________________________________________________________________________________________

    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::ElementExistException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeByName( const ::rtl::OUString& Name )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasElements()
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XComponent
    //____________________________________________________________________________________________________

    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XTransactedObject
    //____________________________________________________________________________________________________

    virtual void SAL_CALL commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XClassifiedObject
    //____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getClassID()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getClassName()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClassInfo( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aClassID,
                                        const ::rtl::OUString& sClassName )
        throw ( ::com::sun::star::lang::NoSupportException,
                ::com::sun::star::uno::RuntimeException );

    //____________________________________________________________________________________________________
    //  XServiceInfo
    //____________________________________________________________________________________________________

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw ( ::com::sun::star::uno::RuntimeException );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
