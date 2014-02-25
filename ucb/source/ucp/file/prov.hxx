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

#ifndef _PROV_HXX_
#define _PROV_HXX_

#include <cppuhelper/weak.hxx>

#include "osl/mutex.hxx"
#include <ucbhelper/macros.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

// FileProvider



namespace fileaccess {

    // Forward declaration

    class BaseContent;
    class shell;

    class FileProvider:
        public cppu::OWeakObject,
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::lang::XInitialization,
        public com::sun::star::lang::XTypeProvider,
        public com::sun::star::ucb::XContentProvider,
        public com::sun::star::ucb::XContentIdentifierFactory,
        public com::sun::star::beans::XPropertySet,
        public com::sun::star::ucb::XFileIdentifierConverter
    {
        friend class BaseContent;
    public:

        FileProvider( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
        ~FileProvider();

        // XInterface
        virtual com::sun::star::uno::Any SAL_CALL
        queryInterface(
            const com::sun::star::uno::Type& aType )
            throw( com::sun::star::uno::RuntimeException, std::exception);

        virtual void SAL_CALL
        acquire(
            void )
            throw();

        virtual void SAL_CALL
        release(
            void )
            throw();

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception );

        virtual sal_Bool SAL_CALL
        supportsService(
            const OUString& ServiceName )
            throw(com::sun::star::uno::RuntimeException, std::exception );

        virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( com::sun::star::uno::RuntimeException, std::exception );


        static com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

        static com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
        CreateInstance(
            const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMultiServiceFactory );

        // XTypeProvider

        XTYPEPROVIDER_DECL()

        // XInitialization
        virtual void SAL_CALL
        initialize(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception);


        // XContentProvider
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( com::sun::star::ucb::IllegalIdentifierException,
                   com::sun::star::uno::RuntimeException, std::exception );

        // XContentIdentifierFactory

        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier > SAL_CALL
        createContentIdentifier(
            const OUString& ContentId )
            throw( com::sun::star::uno::RuntimeException, std::exception );


        virtual sal_Int32 SAL_CALL
        compareContentIds(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id1,
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Id2 )
            throw( com::sun::star::uno::RuntimeException, std::exception );

        // XProperySet

        virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(  )
            throw( com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL
        setPropertyValue(
            const OUString& aPropertyName,
            const com::sun::star::uno::Any& aValue )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::beans::PropertyVetoException,
                   com::sun::star::lang::IllegalArgumentException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception );

        virtual com::sun::star::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception);

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL
        removeVetoableChangeListener(
            const OUString& PropertyName,
            const com::sun::star::uno::Reference< com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw( com::sun::star::beans::UnknownPropertyException,
                   com::sun::star::lang::WrappedTargetException,
                   com::sun::star::uno::RuntimeException, std::exception);


        // XFileIdentifierConverter

        virtual sal_Int32 SAL_CALL
        getFileProviderLocality( const OUString& BaseURL )
            throw( com::sun::star::uno::RuntimeException, std::exception );

        virtual OUString SAL_CALL getFileURLFromSystemPath( const OUString& BaseURL,
                                                                 const OUString& SystemPath )
            throw( com::sun::star::uno::RuntimeException, std::exception );

        virtual OUString SAL_CALL getSystemPathFromFileURL( const OUString& URL )
            throw( com::sun::star::uno::RuntimeException, std::exception );


    private:
        // methods
        void SAL_CALL init();

        // Members
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >      m_xContext;

        void SAL_CALL initProperties( void );
        osl::Mutex   m_aMutex;
        OUString m_HostName;
        OUString m_HomeDirectory;
        sal_Int32     m_FileSystemNotation;

        com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >     m_xPropertySetInfo;

        shell*                                                                        m_pMyShell;
    };

}       // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
