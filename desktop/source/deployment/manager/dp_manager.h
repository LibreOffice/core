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

#include "dp_activepackages.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

namespace dp_manager {

typedef ::cppu::WeakComponentImplHelper<
    css::deployment::XPackageManager > t_pm_helper;


class PackageManagerImpl final : private cppu::BaseMutex, public t_pm_helper
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    OUString m_context;
    OUString m_registrationData;
    OUString m_registrationData_expanded;
    OUString m_registryCache;
    bool m_readOnly;

    OUString m_activePackages;
    OUString m_activePackages_expanded;
    std::unique_ptr< ActivePackages > m_activePackagesDB;
    //This mutex is only used for synchronization in addPackage
    std::mutex m_addMutex;
    css::uno::Reference<css::ucb::XProgressHandler> m_xLogFile;
    inline void logIntern( css::uno::Any const & status );
    void fireModified();

    css::uno::Reference<css::deployment::XPackageRegistry> m_xRegistry;

    void initRegistryBackends();
    void initActivationLayer(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    OUString detectMediaType(
        ::ucbhelper::Content const & ucbContent, bool throw_exc = true );
    OUString insertToActivationLayer(
        css::uno::Sequence<css::beans::NamedValue> const & properties,
        OUString const & mediaType,
        ::ucbhelper::Content const & sourceContent,
        OUString const & title, ActivePackages::Data * dbData );
    void insertToActivationLayerDB(
        OUString const & id, ActivePackages::Data const & dbData );

    static void deletePackageFromCache(
        css::uno::Reference<css::deployment::XPackage> const & xPackage,
        OUString const & destFolder );

    bool isInstalled(
        css::uno::Reference<css::deployment::XPackage> const & package);

    bool synchronizeRemovedExtensions(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

    bool synchronizeAddedExtensions(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv);

    class CmdEnvWrapperImpl
        : public ::cppu::WeakImplHelper< css::ucb::XCommandEnvironment,
                                          css::ucb::XProgressHandler >
    {
        css::uno::Reference<css::ucb::XProgressHandler> m_xLogFile;
        css::uno::Reference<css::ucb::XProgressHandler> m_xUserProgress;
        css::uno::Reference<css::task::XInteractionHandler>
        m_xUserInteractionHandler;

    public:
        virtual ~CmdEnvWrapperImpl() override;
        CmdEnvWrapperImpl(
            css::uno::Reference<css::ucb::XCommandEnvironment>
            const & xUserCmdEnv,
            css::uno::Reference<css::ucb::XProgressHandler> const & xLogFile );

        // XCommandEnvironment
        virtual css::uno::Reference<css::task::XInteractionHandler> SAL_CALL
        getInteractionHandler() override;
        virtual css::uno::Reference<css::ucb::XProgressHandler> SAL_CALL
        getProgressHandler() override;

        // XProgressHandler
        virtual void SAL_CALL push( css::uno::Any const & Status ) override;
        virtual void SAL_CALL update( css::uno::Any const & Status ) override;
        virtual void SAL_CALL pop() override;
    };

    inline void check();
    virtual void SAL_CALL disposing() override;

    virtual ~PackageManagerImpl() override;
    PackageManagerImpl(
        css::uno::Reference<css::uno::XComponentContext> xComponentContext, OUString context )
        : t_pm_helper( m_aMutex ),
          m_xComponentContext(std::move( xComponentContext )),
          m_context(std::move( context )),
          m_readOnly( true )
        {}

public:
    static css::uno::Reference<css::deployment::XPackageManager> create(
        css::uno::Reference<css::uno::XComponentContext>
        const & xComponentContext, OUString const & context );

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener ) override;

    // XPackageManager
    virtual OUString SAL_CALL getContext() override;
    virtual css::uno::Sequence<
        css::uno::Reference<css::deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() override;

    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() override;

    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL addPackage(
        OUString const & url,
        css::uno::Sequence<css::beans::NamedValue> const & properties,
        OUString const & mediaType,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL importExtension(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual void SAL_CALL removePackage(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    OUString getDeployPath( ActivePackages::Data const & data );
    css::uno::Reference<css::deployment::XPackage> getDeployedPackage_(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    css::uno::Reference<css::deployment::XPackage> getDeployedPackage_(
        std::u16string_view id, ActivePackages::Data const & data,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        bool ignoreAlienPlatforms = false );
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL
    getDeployedPackage(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    getDeployedPackages_(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getDeployedPackages(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual void SAL_CALL reinstallDeployedPackages(
        sal_Bool force,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual ::sal_Bool SAL_CALL isReadOnly(  ) override;

    virtual ::sal_Bool SAL_CALL synchronize(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;

    virtual css::uno::Sequence<css::uno::Reference<css::deployment::XPackage> > SAL_CALL
    getExtensionsWithUnacceptedLicenses(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv) override;

    virtual sal_Int32 SAL_CALL checkPrerequisites(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv ) override;
        };


inline void PackageManagerImpl::check()
{
    ::osl::MutexGuard guard( m_aMutex );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
        throw css::lang::DisposedException(
            u"PackageManager instance has already been disposed!"_ustr,
            static_cast< ::cppu::OWeakObject * >(this) );
}


inline void PackageManagerImpl::logIntern( css::uno::Any const & status )
{
    if (m_xLogFile.is())
        m_xLogFile->update( status );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
