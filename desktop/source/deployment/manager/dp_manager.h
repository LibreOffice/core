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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_MANAGER_H
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_MANAGER_H

#include "dp_manager.hrc"
#include "dp_misc.h"
#include "dp_interact.h"
#include "dp_activepackages.hxx"
#include <rtl/ref.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/deployment/XPackageRegistry.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>
#include <memory>


namespace dp_manager {

typedef ::cppu::WeakComponentImplHelper<
    css::deployment::XPackageManager > t_pm_helper;


class PackageManagerImpl : private ::dp_misc::MutexHolder, public t_pm_helper
{
    css::uno::Reference<css::uno::XComponentContext> m_xComponentContext;
    OUString m_context;
    OUString m_registrationData;
    OUString m_registrationData_expanded;
    OUString m_registryCache;
    bool m_readOnly;

    OUString m_activePackages;
    OUString m_activePackages_expanded;
    ::std::unique_ptr< ActivePackages > m_activePackagesDB;
    //This mutex is only used for synchronization in addPackage
    ::osl::Mutex m_addMutex;
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
        virtual ~CmdEnvWrapperImpl();
        CmdEnvWrapperImpl(
            css::uno::Reference<css::ucb::XCommandEnvironment>
            const & xUserCmdEnv,
            css::uno::Reference<css::ucb::XProgressHandler> const & xLogFile );

        // XCommandEnvironment
        virtual css::uno::Reference<css::task::XInteractionHandler> SAL_CALL
        getInteractionHandler() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference<css::ucb::XProgressHandler> SAL_CALL
        getProgressHandler() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XProgressHandler
        virtual void SAL_CALL push( css::uno::Any const & Status )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL update( css::uno::Any const & Status )
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL pop() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

protected:
    inline void check();
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    virtual ~PackageManagerImpl();
    inline PackageManagerImpl(
        css::uno::Reference<css::uno::XComponentContext>
        const & xComponentContext, OUString const & context )
        : t_pm_helper( getMutex() ),
          m_xComponentContext( xComponentContext ),
          m_context( context ),
          m_readOnly( true )
        {}

public:
    static css::uno::Reference<css::deployment::XPackageManager> create(
        css::uno::Reference<css::uno::XComponentContext>
        const & xComponentContext, OUString const & context );

    // XComponent
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeEventListener(
        css::uno::Reference<css::lang::XEventListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeModifyListener(
        css::uno::Reference<css::util::XModifyListener> const & xListener )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPackageManager
    virtual OUString SAL_CALL getContext()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Sequence<
        css::uno::Reference<css::deployment::XPackageTypeInfo> > SAL_CALL
    getSupportedPackageTypes() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::task::XAbortChannel> SAL_CALL
    createAbortChannel() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL addPackage(
        OUString const & url,
        css::uno::Sequence<css::beans::NamedValue> const & properties,
        OUString const & mediaType,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL importExtension(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::ucb::CommandAbortedException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removePackage(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    OUString getDeployPath( ActivePackages::Data const & data );
    css::uno::Reference<css::deployment::XPackage> SAL_CALL getDeployedPackage_(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    css::uno::Reference<css::deployment::XPackage> getDeployedPackage_(
        OUString const & id, ActivePackages::Data const & data,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        bool ignoreAlienPlatforms = false );
    virtual css::uno::Reference<css::deployment::XPackage> SAL_CALL
    getDeployedPackage(
        OUString const & id, OUString const & fileName,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    getDeployedPackages_(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    virtual css::uno::Sequence< css::uno::Reference<css::deployment::XPackage> >
    SAL_CALL getDeployedPackages(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL reinstallDeployedPackages(
        sal_Bool force,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::sal_Bool SAL_CALL isReadOnly(  )
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::sal_Bool SAL_CALL synchronize(
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::ContentCreationException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence<css::uno::Reference<css::deployment::XPackage> > SAL_CALL
    getExtensionsWithUnacceptedLicenses(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
        throw (css::deployment::DeploymentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL checkPrerequisites(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        throw (css::deployment::DeploymentException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        };


inline void PackageManagerImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
        throw css::lang::DisposedException(
            "PackageManager instance has already been disposed!",
            static_cast< ::cppu::OWeakObject * >(this) );
}


inline void PackageManagerImpl::logIntern( css::uno::Any const & status )
{
    if (m_xLogFile.is())
        m_xLogFile->update( status );
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
