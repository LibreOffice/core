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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/logfile.hxx>
#include "cppuhelper/compbase3.hxx"

#include <unotools/configmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

using rtl::OUString;
using namespace desktop;
using namespace com::sun::star;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace
{
//For use with XExtensionManager.synchronize
class SilentCommandEnv
    : public ::cppu::WeakImplHelper3< ucb::XCommandEnvironment,
                                      task::XInteractionHandler,
                                      ucb::XProgressHandler >
{
public:
    virtual ~SilentCommandEnv(){};
    SilentCommandEnv(){};

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
Reference<task::XInteractionHandler> SilentCommandEnv::getInteractionHandler()
throw (uno::RuntimeException)
{
    return this;
}

Reference<ucb::XProgressHandler> SilentCommandEnv::getProgressHandler()
throw (uno::RuntimeException)
{
    return this;
}

// XInteractionHandler
void SilentCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );

    // We approve everything here
    uno::Sequence< Reference< task::XInteractionContinuation > > conts(
        xRequest->getContinuations() );
    Reference< task::XInteractionContinuation > const * pConts =
        conts.getConstArray();
    sal_Int32 len = conts.getLength();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {

        Reference< task::XInteractionApprove > xInteractionApprove(
            pConts[ pos ], uno::UNO_QUERY );
        if (xInteractionApprove.is()) {
            xInteractionApprove->select();
        }
    }
}

// XProgressHandler
void SilentCommandEnv::push( uno::Any const & /*Status*/ )
throw (uno::RuntimeException)
{
}


void SilentCommandEnv::update( uno::Any const & /*Status */)
throw (uno::RuntimeException)
{
}

void SilentCommandEnv::pop() throw (uno::RuntimeException)
{
}

} // end namespace
static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) );
//------------------------------------------------------------------------------
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
static bool impl_checkDependencies( const uno::Reference< deployment::XPackageManager > &xPackageManager )
{
    uno::Sequence< uno::Reference< deployment::XPackage > > packages;

    try {
        packages = xPackageManager->getDeployedPackages( uno::Reference< task::XAbortChannel >(),
                                                         uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( deployment::DeploymentException & ) { /* handleGeneralError(e.Cause);*/ }
    catch ( ucb::CommandFailedException & ) { /* handleGeneralError(e.Reason);*/ }
    catch ( ucb::CommandAbortedException & ) {}
    catch ( lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    for ( sal_Int32 i = 0; i < packages.getLength(); ++i )
    {
        bool bRegistered = false;
        try {
            beans::Optional< beans::Ambiguous< sal_Bool > > option( packages[i]->isRegistered( uno::Reference< task::XAbortChannel >(),
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
        catch ( uno::RuntimeException & ) { throw; }
        catch ( uno::Exception & exc) {
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            bRegistered = false;
        }

        if ( bRegistered )
        {
            bool bDependenciesValid = false;
            try {
                bDependenciesValid = packages[i]->checkDependencies( uno::Reference< ucb::XCommandEnvironment >() );
            }
            catch ( deployment::DeploymentException & ) {}
            if ( ! bDependenciesValid )
            {
                return false;
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
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString::createFromAscii("NodePath"),
                      makeAny( OUString::createFromAscii("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY_THROW );

        Any value = makeAny( OUString::createFromAscii("never") );

        pset->setPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID"), value );
        Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
    }
    catch (const Exception&) {}
}

//------------------------------------------------------------------------------
static bool impl_check()
{
    uno::Reference< uno::XComponentContext > xContext = comphelper_getProcessComponentContext();
    uno::Reference< deployment::XPackageManager > xManager;
    bool bDependenciesValid = true;

    try {
        xManager = deployment::thePackageManagerFactory::get( xContext )->getPackageManager( UNISTRING("user") );
    }
    catch ( ucb::CommandFailedException & ){}
    catch ( uno::RuntimeException & ) {}

    if ( xManager.is() )
        bDependenciesValid = impl_checkDependencies( xManager );

    if ( bDependenciesValid )
    {
        try {
            xManager = deployment::thePackageManagerFactory::get( xContext )->getPackageManager( UNISTRING("shared") );
        }
        catch ( ucb::CommandFailedException & ){}
        catch ( uno::RuntimeException & ) {}

        if ( xManager.is() )
            bDependenciesValid = impl_checkDependencies( xManager );
    }

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

//------------------------------------------------------------------------------
// to check, if we need checking the dependencies of the extensions again, we compare
// the build id of the office with the one of the last check
//------------------------------------------------------------------------------
static bool impl_needsCompatCheck()
{
    bool bNeedsCheck = false;
    rtl::OUString aLastCheckBuildID;
    rtl::OUString aCurrentBuildID( UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":buildid}" ) );
    rtl::Bootstrap::expandMacros( aCurrentBuildID );

    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString::createFromAscii("NodePath"),
                      makeAny( OUString::createFromAscii("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY_THROW );

        Any result = pset->getPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID") );

        result >>= aLastCheckBuildID;
        if ( aLastCheckBuildID != aCurrentBuildID )
        {
            bNeedsCheck = true;
            result <<= aCurrentBuildID;
            pset->setPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID"), result );
            Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
        }
    }
    catch (const Exception&) {}

    return bNeedsCheck;
}

//------------------------------------------------------------------------------
// Do we need to check the dependencies of the extensions?
// When there are unresolved issues, we can't continue with startup
sal_Bool Desktop::CheckExtensionDependencies()
{
    sal_Bool bAbort = false;

    if ( impl_needsCompatCheck() )
        bAbort = impl_check();

    return bAbort;
}

//Returns true if the Folder was more recently modified then
//the lastsynchronized file. That is the repository needs to
//be synchronized.
static bool compareExtensionFolderWithLastSynchronizedFile(
    OUString const & folderURL, OUString const & fileURL)
{
    bool bNeedsSync = false;
    ::osl::DirectoryItem itemExtFolder;
    ::osl::File::RC err1 =
          ::osl::DirectoryItem::get(folderURL, itemExtFolder);
    //If it does not exist, then there is nothing to be done
    if (err1 == ::osl::File::E_NOENT)
    {
        return false;
    }
    else if (err1 != ::osl::File::E_None)
    {
        OSL_ENSURE(0, "Cannot access extension folder");
        return true; //sync just in case
    }

    //If last synchronized does not exist, then OOo is started for the first time
    ::osl::DirectoryItem itemFile;
    ::osl::File::RC err2 = ::osl::DirectoryItem::get(fileURL, itemFile);
    if (err2 == ::osl::File::E_NOENT)
    {
        return true;

    }
    else if (err2 != ::osl::File::E_None)
    {
        OSL_ENSURE(0, "Cannot access file lastsynchronized");
        return true; //sync just in case
    }

    //compare the modification time of the extension folder and the last
    //modified file
    ::osl::FileStatus statFolder(FileStatusMask_ModifyTime);
    ::osl::FileStatus statFile(FileStatusMask_ModifyTime);
    if (itemExtFolder.getFileStatus(statFolder) == ::osl::File::E_None)
    {
        if (itemFile.getFileStatus(statFile) == ::osl::File::E_None)
        {
            TimeValue timeFolder = statFolder.getModifyTime();
            TimeValue timeFile = statFile.getModifyTime();

            if (timeFile.Seconds < timeFolder.Seconds)
                bNeedsSync = true;
        }
        else
        {
            OSL_ASSERT(0);
            bNeedsSync = true;
        }
    }
    else
    {
        OSL_ASSERT(0);
        bNeedsSync = true;
    }
    return bNeedsSync;
}

static bool needToSyncRepostitory(OUString const & name)
{
    OUString folder;
    OUString file;
    if (name.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("bundled"))))
    {
        folder = OUString(
            RTL_CONSTASCII_USTRINGPARAM("$BUNDLED_EXTENSIONS"));
        file = OUString (
            RTL_CONSTASCII_USTRINGPARAM(
                "$BUNDLED_EXTENSIONS_USER/lastsynchronized"));
    }
    else if (name.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("shared"))))
    {
        folder = OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "$UNO_SHARED_PACKAGES_CACHE/uno_packages"));
        file = OUString (
            RTL_CONSTASCII_USTRINGPARAM(
                "$SHARED_EXTENSIONS_USER/lastsynchronized"));
    }
    else
    {
        OSL_ASSERT(0);
        return true;
    }
    ::rtl::Bootstrap::expandMacros(folder);
    ::rtl::Bootstrap::expandMacros(file);
    return compareExtensionFolderWithLastSynchronizedFile(
        folder, file);
}

void Desktop::SynchronizeExtensionRepositories()
{
    RTL_LOGFILE_CONTEXT(aLog,"desktop (jl97489) ::Desktop::SynchronizeExtensionRepositories");
    OUString sDisable;
    ::rtl::Bootstrap::get(
        OUString(RTL_CONSTASCII_USTRINGPARAM("DISABLE_SYNC_EXTENSIONS")),
        sDisable,
        OUString(RTL_CONSTASCII_USTRINGPARAM("")));
    if (sDisable.getLength() > 0)
        return;
    Reference<deployment::XExtensionManager> xExtensionManager;
    //synchronize shared before bundled otherewise there are
    //more revoke and registration calls.
    OUString sShared(RTL_CONSTASCII_USTRINGPARAM("shared"));
    if (needToSyncRepostitory(sShared))
    {
        xExtensionManager =
            deployment::ExtensionManager::get(
                comphelper_getProcessComponentContext());
        if (xExtensionManager.is())
        {
            Reference<ucb::XCommandEnvironment> cmdEnv(
                new SilentCommandEnv());
            xExtensionManager->synchronize(
                sShared, Reference<task::XAbortChannel>(), cmdEnv);
        }

    }

    OUString sBundled(RTL_CONSTASCII_USTRINGPARAM("bundled"));
    if (needToSyncRepostitory( sBundled))
    {
        if (!xExtensionManager.is())
        {
            xExtensionManager =
                deployment::ExtensionManager::get(
                    comphelper_getProcessComponentContext());
        }
        if (xExtensionManager.is())
        {
            Reference<ucb::XCommandEnvironment> cmdEnv(
                new SilentCommandEnv());
            xExtensionManager->synchronize(
                sBundled, Reference<task::XAbortChannel>(), cmdEnv);

        }
    }
}
