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

#include "osl/file.hxx"
#include "osl/mutex.hxx"

#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/logfile.hxx>
#include "cppuhelper/compbase3.hxx"

#include "vcl/wrkwin.hxx"
#include "vcl/timer.hxx"

#include <unotools/configmgr.hxx>
#include "toolkit/helper/vclunohelper.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/ui/LicenseDialog.hpp"
#include <com/sun/star/task/OfficeRestartManager.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

#include "../deployment/inc/dp_misc.h"

using rtl::OUString;
using namespace desktop;
using namespace com::sun::star;

#define UNISTRING(s) OUString(s)

namespace
{
//For use with XExtensionManager.synchronize
class SilentCommandEnv
    : public ::cppu::WeakImplHelper3< ucb::XCommandEnvironment,
                                      task::XInteractionHandler,
                                      ucb::XProgressHandler >
{
    uno::Reference<uno::XComponentContext> mxContext;
    Desktop    *mpDesktop;
    sal_Int32   mnLevel;
    sal_Int32   mnProgress;

public:
    SilentCommandEnv(
        uno::Reference<uno::XComponentContext> const & xContext,
        Desktop* pDesktop );
    virtual ~SilentCommandEnv();

    // XCommandEnvironment
    virtual uno::Reference<task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (uno::RuntimeException);
    virtual uno::Reference<ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        uno::Reference<task::XInteractionRequest > const & xRequest )
        throw (uno::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( uno::Any const & Status )
        throw (uno::RuntimeException);
    virtual void SAL_CALL update( uno::Any const & Status )
        throw (uno::RuntimeException);
    virtual void SAL_CALL pop() throw (uno::RuntimeException);
};

//-----------------------------------------------------------------------------
SilentCommandEnv::SilentCommandEnv(
    uno::Reference<uno::XComponentContext> const & xContext,
    Desktop* pDesktop ):
    mxContext( xContext ),
    mpDesktop( pDesktop ),
    mnLevel( 0 ),
    mnProgress( 25 )
{}

//-----------------------------------------------------------------------------
SilentCommandEnv::~SilentCommandEnv()
{
    mpDesktop->SetSplashScreenText( OUString() );
}

//-----------------------------------------------------------------------------
Reference<task::XInteractionHandler> SilentCommandEnv::getInteractionHandler()
    throw (uno::RuntimeException)
{
    return this;
}

//-----------------------------------------------------------------------------
Reference<ucb::XProgressHandler> SilentCommandEnv::getProgressHandler()
    throw (uno::RuntimeException)
{
    return this;
}

//-----------------------------------------------------------------------------
// XInteractionHandler
void SilentCommandEnv::handle( Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    deployment::LicenseException licExc;

    uno::Any request( xRequest->getRequest() );
    bool bApprove = true;

    if ( request >>= licExc )
    {
        uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
            deployment::ui::LicenseDialog::create(
            mxContext, VCLUnoHelper::GetInterface( NULL ),
            licExc.ExtensionName, licExc.Text ) );
        sal_Int16 res = xDialog->execute();
        if ( res == ui::dialogs::ExecutableDialogResults::CANCEL )
            bApprove = false;
        else if ( res == ui::dialogs::ExecutableDialogResults::OK )
            bApprove = true;
        else
        {
            OSL_ASSERT(0);
        }
    }

    // We approve everything here
    uno::Sequence< Reference< task::XInteractionContinuation > > conts( xRequest->getContinuations() );
    Reference< task::XInteractionContinuation > const * pConts = conts.getConstArray();
    sal_Int32 len = conts.getLength();

    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        if ( bApprove )
        {
            uno::Reference< task::XInteractionApprove > xInteractionApprove( pConts[ pos ], uno::UNO_QUERY );
            if ( xInteractionApprove.is() )
                xInteractionApprove->select();
        }
        else
        {
            uno::Reference< task::XInteractionAbort > xInteractionAbort( pConts[ pos ], uno::UNO_QUERY );
            if ( xInteractionAbort.is() )
                xInteractionAbort->select();
        }
    }
}

//-----------------------------------------------------------------------------
// XProgressHandler
void SilentCommandEnv::push( uno::Any const & rStatus )
    throw (uno::RuntimeException)
{
    OUString sText;
    mnLevel += 1;

    if ( rStatus.hasValue() && ( rStatus >>= sText) )
    {
        if ( mnLevel <= 3 )
            mpDesktop->SetSplashScreenText( sText );
        else
            mpDesktop->SetSplashScreenProgress( ++mnProgress );
    }
}

//-----------------------------------------------------------------------------
void SilentCommandEnv::update( uno::Any const & rStatus )
    throw (uno::RuntimeException)
{
    OUString sText;
    if ( rStatus.hasValue() && ( rStatus >>= sText) )
    {
        mpDesktop->SetSplashScreenText( sText );
    }
}

//-----------------------------------------------------------------------------
void SilentCommandEnv::pop() throw (uno::RuntimeException)
{
    mnLevel -= 1;
}

} // end namespace


static const char aAccessSrvc[] = "com.sun.star.configuration.ConfigurationUpdateAccess";

static sal_Int16 impl_showExtensionDialog( uno::Reference< uno::XComponentContext > &xContext )
{
    rtl::OUString sServiceName = UNISTRING("com.sun.star.deployment.ui.UpdateRequiredDialog");
    uno::Reference< uno::XInterface > xService;
    sal_Int16 nRet = 0;

    uno::Reference< lang::XMultiComponentFactory > xServiceManager( xContext->getServiceManager() );
    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "impl_showExtensionDialog(): unable to obtain service manager from component context" ), uno::Reference< uno::XInterface > () );

    xService = xServiceManager->createInstanceWithContext( sServiceName, xContext );
    uno::Reference< ui::dialogs::XExecutableDialog > xExecuteable( xService, uno::UNO_QUERY );
    if ( xExecuteable.is() )
        nRet = xExecuteable->execute();

    return nRet;
}

//------------------------------------------------------------------------------
// Check dependencies of all packages
//------------------------------------------------------------------------------
static bool impl_checkDependencies( const uno::Reference< uno::XComponentContext > &xContext )
{
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;
    uno::Reference< deployment::XExtensionManager > xExtensionManager = deployment::ExtensionManager::get( xContext );

    if ( !xExtensionManager.is() )
    {
        OSL_FAIL( "Could not get the Extension Manager!" );
        return true;
    }

    try {
        xAllPackages = xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                            uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( const deployment::DeploymentException & ) { return true; }
    catch ( const ucb::CommandFailedException & ) { return true; }
    catch ( const ucb::CommandAbortedException & ) { return true; }
    catch ( const lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    sal_Int32 nMax = 2;
#ifdef DEBUG
    nMax = 3;
#endif

    for ( sal_Int32 i = 0; i < xAllPackages.getLength(); ++i )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > xPackageList = xAllPackages[i];

        for ( sal_Int32 j = 0; (j<nMax) && (j < xPackageList.getLength()); ++j )
        {
            uno::Reference< deployment::XPackage > xPackage = xPackageList[j];
            if ( xPackage.is() )
            {
                bool bRegistered = false;
                try {
                    beans::Optional< beans::Ambiguous< sal_Bool > > option( xPackage->isRegistered( uno::Reference< task::XAbortChannel >(),
                                                                                                    uno::Reference< ucb::XCommandEnvironment >() ) );
                    if ( option.IsPresent )
                    {
                        ::beans::Ambiguous< sal_Bool > const & reg = option.Value;
                        if ( reg.IsAmbiguous )
                            bRegistered = false;
                        else
                            bRegistered = reg.Value ? true : false;
                    }
                    else
                        bRegistered = false;
                }
                catch ( const uno::RuntimeException & ) { throw; }
                catch (const uno::Exception & exc) {
                    (void) exc;
                    OSL_FAIL( ::rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                }

                if ( bRegistered )
                {
                    bool bDependenciesValid = false;
                    try {
                        bDependenciesValid = xPackage->checkDependencies( uno::Reference< ucb::XCommandEnvironment >() );
                    }
                    catch ( const deployment::DeploymentException & ) {}
                    if ( ! bDependenciesValid )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
// resets the 'check needed' flag (needed, if aborted)
//------------------------------------------------------------------------------
static void impl_setNeedsCompatCheck()
{
    try {
        Reference< XMultiServiceFactory > theConfigProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString("nodepath"),
                      makeAny( OUString("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( rtl::OUString(aAccessSrvc), theArgs ), UNO_QUERY_THROW );

        Any value = makeAny( OUString("never") );

        pset->setPropertyValue( OUString("LastCompatibilityCheckID"), value );
        Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
    }
    catch (const Exception&) {}
}

//------------------------------------------------------------------------------
// to check if we need checking the dependencies of the extensions again, we compare
// the build id of the office with the one of the last check
//------------------------------------------------------------------------------
static bool impl_needsCompatCheck()
{
    bool bNeedsCheck = false;
    rtl::OUString aLastCheckBuildID;
    rtl::OUString aCurrentBuildID( UNISTRING( "${$BRAND_BASE_DIR/program/" SAL_CONFIGFILE("version") ":buildid}" ) );
    rtl::Bootstrap::expandMacros( aCurrentBuildID );

    try {
        Reference< XMultiServiceFactory > theConfigProvider(
            configuration::theDefaultProvider::get(
                comphelper::getProcessComponentContext() ) );

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString("nodepath"),
                      makeAny( OUString("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( rtl::OUString(aAccessSrvc), theArgs ), UNO_QUERY_THROW );

        Any result = pset->getPropertyValue( OUString("LastCompatibilityCheckID") );

        result >>= aLastCheckBuildID;
        if ( aLastCheckBuildID != aCurrentBuildID )
        {
            bNeedsCheck = true;
            result <<= aCurrentBuildID;
            pset->setPropertyValue( OUString("LastCompatibilityCheckID"), result );
            Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
        }
#ifdef DEBUG
        bNeedsCheck = true;
#endif
    }
    catch (const com::sun::star::uno::Exception&) {}

    return bNeedsCheck;
}

//------------------------------------------------------------------------------
// Do we need to check the dependencies of the extensions?
// When there are unresolved issues, we can't continue with startup
sal_Bool Desktop::CheckExtensionDependencies()
{
    if (!impl_needsCompatCheck())
    {
        return false;
    }

    uno::Reference< uno::XComponentContext > xContext = comphelper_getProcessComponentContext();

    bool bDependenciesValid = impl_checkDependencies( xContext );

    short nRet = 0;

    if ( !bDependenciesValid )
        nRet = impl_showExtensionDialog( xContext );

    if ( nRet == -1 )
    {
        impl_setNeedsCompatCheck();
        return true;
    }
    else
        return false;
}

void Desktop::SynchronizeExtensionRepositories()
{
    RTL_LOGFILE_CONTEXT(aLog,"desktop (jl) ::Desktop::SynchronizeExtensionRepositories");
    uno::Reference< uno::XComponentContext > context(
        comphelper_getProcessComponentContext());
    uno::Reference< ucb::XCommandEnvironment > silent(
        new SilentCommandEnv(context, this));
    if (m_bCleanedExtensionCache) {
        deployment::ExtensionManager::get(context)->reinstallDeployedExtensions(
            true, "user", Reference<task::XAbortChannel>(), silent);
        task::OfficeRestartManager::get(context)->requestRestart(
            silent->getInteractionHandler());
    } else {
        // reinstallDeployedExtensions above already calls syncRepositories
        // internally:
        dp_misc::syncRepositories(m_bCleanedExtensionCache, silent);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
