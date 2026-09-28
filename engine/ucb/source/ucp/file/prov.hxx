/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XFileIdentifierConverter.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <memory>
#include <mutex>

// FileProvider


namespace fileaccess {

    // Forward declaration

    class TaskManager;
    class XPropertySetInfoImpl2;

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

        explicit FileProvider( const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext );
        virtual ~FileProvider() override;

        // XServiceInfo
        virtual OUString
        getImplementationName() override;

        virtual bool
        supportsService( const OUString& ServiceName ) override;

        virtual cpo::uno::Sequence< OUString >
        getSupportedServiceNames() override;


        // XInitialization
        virtual void
        initialize(
            const cpo::uno::Sequence< cpo::uno::Any >& aArguments ) override;


        // XContentProvider
        virtual cpo::uno::Reference< css::ucb::XContent >
        queryContent(
            const cpo::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

        // XContentIdentifierFactory

        virtual cpo::uno::Reference< css::ucb::XContentIdentifier >
        createContentIdentifier(
            const OUString& ContentId ) override;


        virtual sal_Int32
        compareContentIds(
            const cpo::uno::Reference< css::ucb::XContentIdentifier >& Id1,
            const cpo::uno::Reference< css::ucb::XContentIdentifier >& Id2 ) override;

        // XPropertySet

        virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
        getPropertySetInfo(  ) override;

        virtual void
        setPropertyValue(
            const OUString& aPropertyName,
            const cpo::uno::Any& aValue ) override;

        virtual cpo::uno::Any
        getPropertyValue(
            const OUString& PropertyName ) override;

        virtual void
        addPropertyChangeListener(
            const OUString& aPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;

        virtual void
        removePropertyChangeListener(
            const OUString& aPropertyName,
            const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;

        virtual void
        addVetoableChangeListener(
            const OUString& PropertyName,
            const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        virtual void
        removeVetoableChangeListener(
            const OUString& PropertyName,
            const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;


        // XFileIdentifierConverter

        virtual sal_Int32
        getFileProviderLocality( const OUString& BaseURL ) override;

        virtual OUString getFileURLFromSystemPath( const OUString& BaseURL,
                                                            const OUString& SystemPath ) override;

        virtual OUString getSystemPathFromFileURL( const OUString& URL ) override;


    private:
        // methods
        void init();

        // Members
        cpo::uno::Reference< cpo::uno::XComponentContext >      m_xContext;

        void initProperties(std::unique_lock<std::mutex>& rGuard);
        std::mutex   m_aMutex;
        OUString m_HostName;
        OUString m_HomeDirectory;
        sal_Int32     m_FileSystemNotation;

        rtl::Reference< XPropertySetInfoImpl2 >                 m_xPropertySetInfo;

        std::unique_ptr<TaskManager>                            m_pMyShell;
    };

}       // end namespace fileaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
