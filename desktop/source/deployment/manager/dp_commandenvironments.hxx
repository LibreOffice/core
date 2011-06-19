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

#if ! defined INCLUDED_DP_COMMANDENVIRONMENTS_HXX
#define INCLUDED_DP_COMMANDENVIRONMENTS_HXX

#include "cppuhelper/compbase3.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/uno/Type.hxx"

namespace css = ::com::sun::star;

namespace dp_manager {

/**
   This command environment is to be used when an extension is temporarily
   stored in the "tmp" repository. It prevents all kind of user interaction.
 */
class BaseCommandEnv
    : public ::cppu::WeakImplHelper3< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler,
                                      css::ucb::XProgressHandler >
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::task::XInteractionHandler> m_forwardHandler;

    void handle_(bool approve, bool abort,
                 css::uno::Reference< css::task::XInteractionRequest> const & xRequest );
public:
    virtual ~BaseCommandEnv();
    BaseCommandEnv();
    BaseCommandEnv(
        css::uno::Reference< css::task::XInteractionHandler> const & handler);

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (css::uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL update( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL pop() throw (css::uno::RuntimeException);
};

class TmpRepositoryCommandEnv : public BaseCommandEnv
{
public:
    TmpRepositoryCommandEnv();
    TmpRepositoryCommandEnv(css::uno::Reference< css::task::XInteractionHandler> const & handler);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

};

/** this class is for use in XPackageManager::synchronize.

    It handles particular license cases.
 */
class LicenseCommandEnv : public BaseCommandEnv
{
private:
    ::rtl::OUString m_repository;
    bool m_bSuppressLicense;
public:
    LicenseCommandEnv() : m_bSuppressLicense(false) {};
    LicenseCommandEnv(
        css::uno::Reference< css::task::XInteractionHandler> const & handler,
        bool bSuppressLicense,
        ::rtl::OUString const & repository);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

};

/** this class is for use in XPackageManager::checkPrerequisites

    It always prohibits a license interaction
 */
class NoLicenseCommandEnv : public BaseCommandEnv
{

public:
    NoLicenseCommandEnv(){};
    NoLicenseCommandEnv(css::uno::Reference< css::task::XInteractionHandler> const & handler);

// XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

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
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

    // Set to true if a PlatformException or a DependencyException were handled.
    css::uno::Any m_Exception;
    // Set to true if an unknown exception was handled.
    css::uno::Any m_UnknownException;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
