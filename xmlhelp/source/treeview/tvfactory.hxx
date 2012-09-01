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
        virtual OUString SAL_CALL
        getImplementationName(
            void )
            throw( com::sun::star::uno::RuntimeException );

        virtual sal_Bool SAL_CALL
        supportsService(
            const OUString& ServiceName )
            throw(com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( com::sun::star::uno::RuntimeException );

        // XMultiServiceFactory

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstance(
            const OUString& aServiceSpecifier )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException );

        virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        createInstanceWithArguments(
            const OUString& ServiceSpecifier,
            const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& Arguments )
            throw( com::sun::star::uno::Exception,
                   com::sun::star::uno::RuntimeException);

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames( )
            throw( com::sun::star::uno::RuntimeException );

        // Other

        static OUString SAL_CALL getImplementationName_static();

        static com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();

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
