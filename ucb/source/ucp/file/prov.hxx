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

#pragma once

#include <osl/mutex.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

// FileProvider


namespace fileaccess {

    // Forward declaration

    class BaseContent;
    class TaskManager;

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
        virtual ~FileProvider() override;

        // XServiceInfo
        virtual OUString SAL_CALL
        getImplementationName() override;

        virtual sal_Bool SAL_CALL
        supportsService( const OUString& ServiceName ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;


        // XInitialization
        virtual void SAL_CALL
        initialize(
            const css::uno::Sequence< css::uno::Any >& aArguments ) override;


        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        queryContent(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

        // XContentIdentifierFactory

        virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
        createContentIdentifier(
            const OUString& ContentId ) override;


        virtual sal_Int32 SAL_CALL
        compareContentIds(
            const css::uno::Reference< css::ucb::XContentIdentifier >& Id1,
            const css::uno::Reference< css::ucb::XContentIdentifier >& Id2 ) override;

        // XPropertySet

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo(  ) override;

        virtual void SAL_CALL
        setPropertyValue(
            const OUString& aPropertyName,
            const css::uno::Any& aValue ) override;

        virtual css::uno::Any SAL_CALL
        getPropertyValue(
            const OUString& PropertyName ) override;

        virtual void SAL_CALL
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

        virtual void SAL_CALL
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

        virtual void SAL_CALL
        addVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        virtual void SAL_CALL
        removeVetoableChangeListener(
            const OUString& PropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;


        // XFileIdentifierConverter

        virtual sal_Int32 SAL_CALL
        getFileProviderLocality( const OUString& BaseURL ) override;

        virtual OUString SAL_CALL getFileURLFromSystemPath( const OUString& BaseURL,
                                                            const OUString& SystemPath ) override;

        virtual OUString SAL_CALL getSystemPathFromFileURL( const OUString& URL ) override;


    private:
        // methods
        void init();

        // Members
        css::uno::Reference< css::uno::XComponentContext >      m_xContext;

        void initProperties();
        osl::Mutex   m_aMutex;
        OUString m_HostName;
        OUString m_HomeDirectory;
        sal_Int32     m_FileSystemNotation;

        css::uno::Reference< css::beans::XPropertySetInfo >     m_xPropertySetInfo;

        std::unique_ptr<TaskManager>                            m_pMyShell;
    };

}       // end namespace fileaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
