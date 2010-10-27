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

#ifndef _TREEVIEW_TVFACTORY_HXX_
#define _TREEVIEW_TVFACTORY_HXX_

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>



namespace treeview {

    class TVFactory:
        public cppu::OWeakObject,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::lang::XMultiServiceFactory
    {
    public:

        TVFactory( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );

        ~TVFactory();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();


        // XTypeProvider

        XTYPEPROVIDER_DECL()


        // XServiceInfo
        virtual rtl::OUString SAL_CALL
        getImplementationName(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        supportsService(
            const rtl::OUString& ServiceName )
            throw(com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XMultiServiceFactory

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstance(
            const rtl::OUString& aServiceSpecifier )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const rtl::OUString& ServiceSpecifier,
            const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& Arguments )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        getAvailableServiceNames( )
            throw( com::sun::star::uno::RuntimeException );

        // Other

        static rtl::OUString SAL_CALL getImplementationName_static();

        static com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames_static();

        static com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        CreateInstance(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );


    private:

        // Members
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >  m_xMSF;
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface >             m_xHDS;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
