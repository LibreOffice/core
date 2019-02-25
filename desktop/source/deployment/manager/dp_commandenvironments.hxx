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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_COMMANDENVIRONMENTS_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_MANAGER_DP_COMMANDENVIRONMENTS_HXX

#include <cppuhelper/implbase.hxx>
#include <ucbhelper/content.hxx>

namespace dp_manager {

/**
   This command environment is to be used when an extension is temporarily
   stored in the "tmp" repository. It prevents all kind of user interaction.
 */
class BaseCommandEnv
    : public ::cppu::WeakImplHelper< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler,
                                      css::ucb::XProgressHandler >
{
    css::uno::Reference< css::task::XInteractionHandler> m_forwardHandler;
protected:
    void handle_(bool approve,
                 css::uno::Reference< css::task::XInteractionRequest> const & xRequest );
public:
    virtual ~BaseCommandEnv() override;
    BaseCommandEnv();
    explicit BaseCommandEnv(
        css::uno::Reference< css::task::XInteractionHandler> const & handler);

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() override;
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;

    // XProgressHandler
    virtual void SAL_CALL push( css::uno::Any const & Status ) override;
    virtual void SAL_CALL update( css::uno::Any const & Status ) override;
    virtual void SAL_CALL pop() override;
};

class TmpRepositoryCommandEnv : public BaseCommandEnv
{
public:
    TmpRepositoryCommandEnv();
    explicit TmpRepositoryCommandEnv(css::uno::Reference< css::task::XInteractionHandler> const & handler);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;

};

/** this class is for use in XPackageManager::synchronize.

    It handles particular license cases.
 */
class LicenseCommandEnv : public BaseCommandEnv
{
private:
    OUString m_repository;
    bool m_bSuppressLicense;
public:
    LicenseCommandEnv(
        css::uno::Reference< css::task::XInteractionHandler> const & handler,
        bool bSuppressLicense,
        OUString const & repository);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;

};

/** this class is for use in XPackageManager::checkPrerequisites

    It always prohibits a license interaction
 */
class NoLicenseCommandEnv : public BaseCommandEnv
{

public:
    explicit NoLicenseCommandEnv(css::uno::Reference< css::task::XInteractionHandler> const & handler);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;

};

/* For use in XExtensionManager::addExtension in the call to
   XPackage::checkPrerequisites
   It prevents all user interactions. The license is always accepted.
   It remembers if there was a platform or a dependency exception in
   the member m_bException. if there was any other exception then m_bUnknownException
   is set.

 */
class SilentCheckPrerequisitesCommandEnv : public BaseCommandEnv
{
public:
    SilentCheckPrerequisitesCommandEnv();
    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest ) override;

    // Set to true if a PlatformException or a DependencyException were handled.
    css::uno::Any m_Exception;
    // Set to true if an unknown exception was handled.
    css::uno::Any m_UnknownException;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
