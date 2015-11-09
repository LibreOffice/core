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

#ifndef INCLUDED_UCB_SOURCE_UCP_FILE_PROV_HXX
#define INCLUDED_UCB_SOURCE_UCP_FILE_PROV_HXX

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
#include <cppuhelper/implbase.hxx>

// FileProvider



namespace fileaccess {

    // Forward declaration

    class BaseContent;
    class shell;

    class FileProvider: public cppu::WeakImplHelper <
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::ucb::XContentProvider,
        css::ucb::XContentIdentifierFactory,
        css::beans::XPropertySet,
        css::ucb::XFileIdentifierConverter >
    {
        friend class BaseContent;
    public:

        explicit FileProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
        virtual ~FileProvider();

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName(
            void )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual sal_Bool SAL_CALL
        supportsService(
            const OUString& ServiceName )
            throw(css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames(
            void )
            throw( css::uno::RuntimeException, std::exception ) override;


        static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL
        createServiceFactory(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr );

        static css::uno::Reference< css::uno::XInterface > SAL_CALL
        CreateInstance(
            const css::uno::Reference< css::lang::XMultiServiceFactory >& xMultiServiceFactory );

        // XInitialization
        virtual void SAL_CALL
        initialize(
            const css::uno::Sequence< css::uno::Any >& aArguments )
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;


        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        queryContent(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
            throw( css::ucb::IllegalIdentifierException,
                   css::uno::RuntimeException, std::exception ) override;

        // XContentIdentifierFactory

        virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
        createContentIdentifier(
            const OUString& ContentId )
            throw( css::uno::RuntimeException, std::exception ) override;


        virtual sal_Int32 SAL_CALL
        compareContentIds(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Id1,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Id2 )
            throw( css::uno::RuntimeException, std::exception ) override;

        // XProperySet

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(  )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        setPropertyValue(
            const OUString& aPropertyName,
            const css::uno::Any& aValue )
            throw( css::beans::UnknownPropertyException,
                   css::beans::PropertyVetoException,
                   css::lang::IllegalArgumentException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual css::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL
        removeVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
            throw( css::beans::UnknownPropertyException,
                   css::lang::WrappedTargetException,
                   css::uno::RuntimeException, std::exception) override;


        // XFileIdentifierConverter

        virtual sal_Int32 SAL_CALL
        getFileProviderLocality( const OUString& BaseURL )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual OUString SAL_CALL getFileURLFromSystemPath( const OUString& BaseURL,
                                                            const OUString& SystemPath )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual OUString SAL_CALL getSystemPathFromFileURL( const OUString& URL )
            throw( css::uno::RuntimeException, std::exception ) override;


    private:
        // methods
        void SAL_CALL init();

        // Members
        css::uno::Reference< css::uno::XComponentContext >      m_xContext;

        void SAL_CALL initProperties();
        osl::Mutex   m_aMutex;
        OUString m_HostName;
        OUString m_HomeDirectory;
        sal_Int32     m_FileSystemNotation;

        css::uno::Reference< css::beans::XPropertySetInfo >     m_xPropertySetInfo;

        shell*                                                  m_pMyShell;
    };

}       // end namespace fileaccess

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
