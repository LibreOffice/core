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


#include <cppuhelper/implbase1.hxx>

#include "comphelper/servicedecl.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "rtl/bootstrap.hxx"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/XExtensionManager.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/deployment/XPackageManager.hpp"
#include "com/sun/star/deployment/XPackageManagerFactory.hpp"
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/beans/Ambiguous.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/util/XModifyBroadcaster.hpp"
#include "comphelper/sequence.hxx"
#include "xmlscript/xml_helper.hxx"
#include "osl/diagnose.h"
#include "dp_interact.h"
#include "dp_resource.h"
#include "dp_ucb.h"
#include "dp_identifier.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_extensionmanager.hxx"
#include "dp_commandenvironments.hxx"
#include "dp_properties.hxx"
#include "boost/bind.hpp"

#include <list>
#include <boost/unordered_map.hpp>
#include <algorithm>

namespace deploy = com::sun::star::deployment;
namespace lang  = com::sun::star::lang;
namespace registry = com::sun::star::registry;
namespace task = com::sun::star::task;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;
namespace beans = com::sun::star::beans;
namespace util = com::sun::star::util;
namespace css = com::sun::star;

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

namespace {

struct CompIdentifiers
{
    bool operator() (::std::vector<Reference<deploy::XPackage> > const & a,
                     ::std::vector<Reference<deploy::XPackage> > const & b)
        {
            if (getName(a).compareTo(getName(b)) < 0)
                return true;
            return false;
        }

    OUString getName(::std::vector<Reference<deploy::XPackage> > const & a);
};

OUString CompIdentifiers::getName(::std::vector<Reference<deploy::XPackage> > const & a)
{
    OSL_ASSERT(a.size() == 3);
    //get the first non-null reference
    Reference<deploy::XPackage>  extension;
    ::std::vector<Reference<deploy::XPackage> >::const_iterator it = a.begin();
    for (; it != a.end(); ++it)
    {
        if (it->is())
        {
            extension = *it;
            break;
        }
    }
    OSL_ASSERT(extension.is());
    return extension->getDisplayName();
}

void writeLastModified(OUString & url, Reference<ucb::XCommandEnvironment> const & xCmdEnv, Reference< uno::XComponentContext > const & xContext)
{
    //Write the lastmodified file
    try {
        ::rtl::Bootstrap::expandMacros(url);
        ::ucbhelper::Content ucbStamp(url, xCmdEnv, xContext);
        dp_misc::erase_path( url, xCmdEnv );
        ::rtl::OString stamp("1" );
        Reference<css::io::XInputStream> xData(
            ::xmlscript::createInputStream(
                ::rtl::ByteSequence(
                    reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                    stamp.getLength() ) ) );
        ucbStamp.writeStream( xData, true /* replace existing */ );
    }
    catch(...)
    {
        uno::Any exc(::cppu::getCaughtException());
        throw deploy::DeploymentException(
            OUSTR("Failed to update") + url, 0, exc);
    }
}

class ExtensionRemoveGuard
{
    css::uno::Reference<css::deployment::XPackage> m_extension;
    css::uno::Reference<css::deployment::XPackageManager> m_xPackageManager;

public:
    ExtensionRemoveGuard(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::deployment::XPackageManager> const & xPackageManager):
        m_extension(extension), m_xPackageManager(xPackageManager) {}
    ~ExtensionRemoveGuard();

    void reset(css::uno::Reference<css::deployment::XPackage> const & extension) {
        m_extension = extension;
    }
};

ExtensionRemoveGuard::~ExtensionRemoveGuard()
{
    try {
        if (m_xPackageManager.is() && m_extension.is())
            m_xPackageManager->removePackage(
                dp_misc::getIdentifier(m_extension), ::rtl::OUString(),
                css::uno::Reference<css::task::XAbortChannel>(),
                css::uno::Reference<css::ucb::XCommandEnvironment>());
    } catch (...) {
        OSL_ASSERT(0);
    }
}

}

namespace dp_manager {

//------------------------------------------------------------------------------

//ToDo: bundled extension
ExtensionManager::ExtensionManager( Reference< uno::XComponentContext > const& xContext) :
    ::cppu::WeakComponentImplHelper1< css::deployment::XExtensionManager >(getMutex()),
    m_xContext( xContext )
{
    m_xPackageManagerFactory = deploy::thePackageManagerFactory::get(m_xContext);
    OSL_ASSERT(m_xPackageManagerFactory.is());

    m_repositoryNames.push_back(OUSTR("user"));
    m_repositoryNames.push_back(OUSTR("shared"));
    m_repositoryNames.push_back(OUSTR("bundled"));
}

//------------------------------------------------------------------------------

ExtensionManager::~ExtensionManager()
{
}

Reference<deploy::XPackageManager> ExtensionManager::getUserRepository()
{
    return m_xPackageManagerFactory->getPackageManager(OUSTR("user"));
}
Reference<deploy::XPackageManager>  ExtensionManager::getSharedRepository()
{
    return m_xPackageManagerFactory->getPackageManager(OUSTR("shared"));
}
Reference<deploy::XPackageManager>  ExtensionManager::getBundledRepository()
{
    return m_xPackageManagerFactory->getPackageManager(OUSTR("bundled"));
}
Reference<deploy::XPackageManager>  ExtensionManager::getTmpRepository()
{
    return m_xPackageManagerFactory->getPackageManager(OUSTR("tmp"));
}

Reference<task::XAbortChannel> ExtensionManager::createAbortChannel()
    throw (uno::RuntimeException)
{
    return new dp_misc::AbortChannel;
}

css::uno::Reference<css::deployment::XPackageManager>
ExtensionManager::getPackageManager(::rtl::OUString const & repository)
    throw (css::lang::IllegalArgumentException)
{
    Reference<deploy::XPackageManager> xPackageManager;
    if (repository.equals(OUSTR("user")))
        xPackageManager = getUserRepository();
    else if (repository.equals(OUSTR("shared")))
        xPackageManager = getSharedRepository();
    else if (repository.equals(OUSTR("bundled")))
        xPackageManager = getBundledRepository();
    else
        throw lang::IllegalArgumentException(
            OUSTR("No valid repository name provided."),
            static_cast<cppu::OWeakObject*>(this), 0);
    return xPackageManager;
}

/*
  Enters the XPackage objects into a map. They must be all from the
  same repository. The value type of the map is a vector, where each vector
  represents an extension with a particular identifier. The first member
  represents the user extension, the second the shared extension and the
  third the bundled extension.
 */
void ExtensionManager::addExtensionsToMap(
    id2extensions & mapExt,
    uno::Sequence<Reference<deploy::XPackage> > const & seqExt,
    OUString const & repository)
{
    //Determine the index in the vector where these extensions are to be
    //added.
    ::std::list<OUString>::const_iterator citNames =
        m_repositoryNames.begin();
    int index = 0;
    for (;citNames != m_repositoryNames.end(); ++citNames, ++index)
    {
        if (citNames->equals(repository))
            break;
    }

    for (int i = 0; i < seqExt.getLength(); ++i)
    {
        Reference<deploy::XPackage> const & xExtension = seqExt[i];
        OUString id = dp_misc::getIdentifier(xExtension);
        id2extensions::iterator ivec =  mapExt.find(id);
        if (ivec == mapExt.end())
        {
            ::std::vector<Reference<deploy::XPackage> > vec(3);
            vec[index] = xExtension;
            mapExt[id] = vec;
        }
        else
        {
            ivec->second[index] = xExtension;
        }
    }
}

/*
   returns a list containing extensions with the same identifier from
   all repositories (user, shared, bundled). If one repository does not
   have this extension, then the list contains an empty Reference. The list
   is ordered according to the priority of the repostories:
   1. user
   2. shared
   3. bundled

   The number of elements is always three, unless the number of repository
   changes.
 */
::std::list<Reference<deploy::XPackage> >
    ExtensionManager::getExtensionsWithSameId(
        OUString const & identifier, OUString const & fileName,
        Reference< ucb::XCommandEnvironment> const & /*xCmdEnv*/)

{
    ::std::list<Reference<deploy::XPackage> > extensionList;
    Reference<deploy::XPackageManager> lRepos[] = {
          getUserRepository(), getSharedRepository(), getBundledRepository() };
    for (int i(0); i != SAL_N_ELEMENTS(lRepos); ++i)
    {
        Reference<deploy::XPackage> xPackage;
        try
        {
            xPackage = lRepos[i]->getDeployedPackage(
                identifier, fileName, Reference<ucb::XCommandEnvironment>());
        }
        catch(const lang::IllegalArgumentException &)
        {
            // thrown if the extension does not exist in this repository
        }
        extensionList.push_back(xPackage);
    }
    OSL_ASSERT(extensionList.size() == 3);
    return extensionList;
}

uno::Sequence<Reference<deploy::XPackage> >
ExtensionManager::getExtensionsWithSameIdentifier(
        OUString const & identifier,
        OUString const & fileName,
        Reference< ucb::XCommandEnvironment> const & xCmdEnv )
        throw (
            deploy::DeploymentException,
            ucb::CommandFailedException,
            lang::IllegalArgumentException,
            uno::RuntimeException)
{
    try
    {
        ::std::list<Reference<deploy::XPackage> > listExtensions =
            getExtensionsWithSameId(
                identifier, fileName, xCmdEnv);
        sal_Bool bHasExtension = false;

        //throw an IllegalArgumentException if there is no extension at all.
        typedef  ::std::list<Reference<deploy::XPackage> >::const_iterator CIT;
        for (CIT i = listExtensions.begin(); i != listExtensions.end(); ++i)
            bHasExtension |= i->is();
        if (!bHasExtension)
            throw lang::IllegalArgumentException(
                OUSTR("Could not find extension: ") + identifier + OUSTR(", ") + fileName,
                static_cast<cppu::OWeakObject*>(this), -1);

        return comphelper::containerToSequence<
            Reference<deploy::XPackage>,
            ::std::list<Reference<deploy::XPackage> >
            > (listExtensions);
    }
    catch ( const deploy::DeploymentException & )
    {
        throw;
    }
    catch ( const ucb::CommandFailedException & )
    {
        throw;
    }
    catch (const lang::IllegalArgumentException &)
    {
        throw;
    }
    catch (...)
    {
        uno::Any exc = ::cppu::getCaughtException();
        throw deploy::DeploymentException(
            OUSTR("Extension Manager: exception during getExtensionsWithSameIdentifier"),
            static_cast<OWeakObject*>(this), exc);
    }
}

bool ExtensionManager::isUserDisabled(
    OUString const & identifier, OUString const & fileName)
{
    ::std::list<Reference<deploy::XPackage> > listExtensions;

    try {
        listExtensions = getExtensionsWithSameId(identifier, fileName);
    } catch ( const lang::IllegalArgumentException & ) {
    }
    OSL_ASSERT(listExtensions.size() == 3);

    return isUserDisabled( ::comphelper::containerToSequence<
                           Reference<deploy::XPackage>,
                           ::std::list<Reference<deploy::XPackage> >
                           > (listExtensions));
}

bool ExtensionManager::isUserDisabled(
    uno::Sequence<Reference<deploy::XPackage> > const & seqExtSameId)
{
    OSL_ASSERT(seqExtSameId.getLength() == 3);
    Reference<deploy::XPackage> const & userExtension = seqExtSameId[0];
    if (userExtension.is())
    {
        beans::Optional<beans::Ambiguous<sal_Bool> > reg =
            userExtension->isRegistered(Reference<task::XAbortChannel>(),
                                        Reference<ucb::XCommandEnvironment>());
        //If the value is ambiguous is than we assume that the extension
        //is enabled, but something went wrong during enabling. We do not
        //automatically disable user extensions.
        if (reg.IsPresent &&
            ! reg.Value.IsAmbiguous && ! reg.Value.Value)
            return true;
    }
    return false;
}

/*
    This method determines the active extension (XPackage.registerPackage) with a
    particular identifier.

    The parameter bUserDisabled determines if the user extension is disabled.

    When the user repository contains an extension with the given identifier and
    it is not disabled by the user, then it is always registered.  Otherwise an
    extension is only registered when there is no registered extension in one of
    the repositories with a higher priority. That is, if the extension is from
    the shared repository and an active extension with the same identifer is in
    the user repository, then the extension is not registered. Similarly a
    bundled extension is not registered if there is an active extension with the
    same identifier in the shared or user repository.
*/
void ExtensionManager::activateExtension(
    OUString const & identifier, OUString const & fileName,
    bool bUserDisabled,
    bool bStartup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    ::std::list<Reference<deploy::XPackage> > listExtensions;
    try {
        listExtensions = getExtensionsWithSameId(identifier, fileName);
    } catch (const lang::IllegalArgumentException &) {
    }
    OSL_ASSERT(listExtensions.size() == 3);

    activateExtension(
        ::comphelper::containerToSequence<
        Reference<deploy::XPackage>,
        ::std::list<Reference<deploy::XPackage> >
        > (listExtensions),
        bUserDisabled, bStartup, xAbortChannel, xCmdEnv);

    fireModified();
}

void ExtensionManager::activateExtension(
    uno::Sequence<Reference<deploy::XPackage> > const & seqExt,
    bool bUserDisabled,
    bool bStartup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    bool bActive = false;
    sal_Int32 len = seqExt.getLength();
    for (sal_Int32 i = 0; i < len; i++)
    {
        Reference<deploy::XPackage> const & aExt =  seqExt[i];
        if (aExt.is())
        {
            //get the registration value of the current iteration
            beans::Optional<beans::Ambiguous<sal_Bool> > optReg =
                aExt->isRegistered(xAbortChannel, xCmdEnv);
            //If nothing can be registered then break
            if (!optReg.IsPresent)
                break;

            //Check if this is a disabled user extension,
            if (i == 0 && bUserDisabled)
            {
                   aExt->revokePackage(bStartup, xAbortChannel, xCmdEnv);
                   continue;
            }

            //If we have already determined an active extension then we must
            //make sure to unregister all extensions with the same id in
            //repositories with a lower priority
            if (bActive)
            {
                aExt->revokePackage(bStartup, xAbortChannel, xCmdEnv);
            }
            else
            {
                //This is the first extension in the ordered list, which becomes
                //the active extension
                bActive = true;
                //Register if not already done.
                //reregister if the value is ambiguous, which indicates that
                //something went wrong during last registration.
                aExt->registerPackage(bStartup, xAbortChannel, xCmdEnv);
            }
        }
    }
}

Reference<deploy::XPackage> ExtensionManager::backupExtension(
    OUString const & identifier, OUString const & fileName,
    Reference<deploy::XPackageManager> const & xPackageManager,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    Reference<deploy::XPackage> xBackup;
    Reference<ucb::XCommandEnvironment> tmpCmdEnv(
        new TmpRepositoryCommandEnv(xCmdEnv->getInteractionHandler()));
    Reference<deploy::XPackage> xOldExtension;
    xOldExtension = xPackageManager->getDeployedPackage(
            identifier, fileName, tmpCmdEnv);

    if (xOldExtension.is())
    {
        xBackup = getTmpRepository()->addPackage(
            xOldExtension->getURL(), uno::Sequence<beans::NamedValue>(),
            OUString(), Reference<task::XAbortChannel>(), tmpCmdEnv);

        OSL_ENSURE(xBackup.is(), "Failed to backup extension");
    }
    return xBackup;
}

//The supported package types are actually determined by the registry. However
//creating a registry
//(desktop/source/deployment/registry/dp_registry.cxx:PackageRegistryImpl) will
//create all the backends, so that the registry can obtain from them the package
//types. Creating the registry will also set up the registry folder containing
//all the subfolders for the respective backends.
//Because all repositories support the same backends, we can just delegate this
//call to one of the repositories.
uno::Sequence< Reference<deploy::XPackageTypeInfo> >
ExtensionManager::getSupportedPackageTypes()
    throw (uno::RuntimeException)
{
    return getUserRepository()->getSupportedPackageTypes();
}
//Do some necessary checks and user interaction. This function does not
//aquire the extension manager mutex and that mutex must not be aquired
//when this function is called. doChecksForAddExtension does  synchronous
//user interactions which may require aquiring the solar mutex.
//Returns true if the extension can be installed.
bool ExtensionManager::doChecksForAddExtension(
    Reference<deploy::XPackageManager> const & xPackageMgr,
    uno::Sequence<beans::NamedValue> const & properties,
    css::uno::Reference<css::deployment::XPackage> const & xTmpExtension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    Reference<deploy::XPackage> & out_existingExtension )
    throw (deploy::DeploymentException,
           ucb::CommandFailedException,
           ucb::CommandAbortedException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    try
    {
        Reference<deploy::XPackage> xOldExtension;
        const OUString sIdentifier = dp_misc::getIdentifier(xTmpExtension);
        const OUString sFileName = xTmpExtension->getName();
        const OUString sDisplayName = xTmpExtension->getDisplayName();
        const OUString sVersion = xTmpExtension->getVersion();

        try
        {
            xOldExtension = xPackageMgr->getDeployedPackage(
                sIdentifier, sFileName, xCmdEnv);
            out_existingExtension = xOldExtension;
        }
        catch (const lang::IllegalArgumentException &)
        {
        }
        bool bCanInstall = false;

        //This part is not guarded against other threads removing, adding, disabling ...
        //etc. the same extension.
        //checkInstall is safe because it notifies the user if the extension is not yet
        //installed in the same repository. Because addExtension has its own guard
        //(m_addMutex), another thread cannot add the extension in the meantime.
        //checkUpdate is called if the same extension exists in the same
        //repository. The user is asked if they want to replace it.  Another
        //thread
        //could already remove the extension. So asking the user was not
        //necessary. No harm is done. The other thread may also ask the user
        //if he wants to remove the extension. This depends on the
        //XCommandEnvironment which it passes to removeExtension.
        if (xOldExtension.is())
        {
            //throws a CommandFailedException if the user cancels
            //the action.
            checkUpdate(sVersion, sDisplayName,xOldExtension, xCmdEnv);
        }
        else
        {
            //throws a CommandFailedException if the user cancels
            //the action.
            checkInstall(sDisplayName, xCmdEnv);
        }
        //Prevent showing the license if requested.
        Reference<ucb::XCommandEnvironment> _xCmdEnv(xCmdEnv);
        ExtensionProperties props(OUString(), properties, Reference<ucb::XCommandEnvironment>(), m_xContext);

        dp_misc::DescriptionInfoset info(dp_misc::getDescriptionInfoset(xTmpExtension->getURL()));
        const ::boost::optional<dp_misc::SimpleLicenseAttributes> licenseAttributes =
            info.getSimpleLicenseAttributes();

        if (licenseAttributes && licenseAttributes->suppressIfRequired
            && props.isSuppressedLicense())
            _xCmdEnv = Reference<ucb::XCommandEnvironment>(
                new NoLicenseCommandEnv(xCmdEnv->getInteractionHandler()));

        bCanInstall = xTmpExtension->checkPrerequisites(
            xAbortChannel, _xCmdEnv, xOldExtension.is() || props.isExtensionUpdate()) == 0 ? true : false;

        return bCanInstall;
    }
    catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception &) {
        uno::Any excOccurred = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception in doChecksForAddExtension"),
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    } catch (...) {
        throw uno::RuntimeException(
            OUSTR("Extension Manager: unexpected exception in doChecksForAddExtension"),
            static_cast<OWeakObject*>(this));
    }
}

// Only add to shared and user repository
Reference<deploy::XPackage> ExtensionManager::addExtension(
    OUString const & url, uno::Sequence<beans::NamedValue> const & properties,
    OUString const & repository,
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<ucb::XCommandEnvironment> const & xCmdEnv )
        throw (deploy::DeploymentException,
               ucb::CommandFailedException,
               ucb::CommandAbortedException,
               lang::IllegalArgumentException,
               uno::RuntimeException)
{
    Reference<deploy::XPackage> xNewExtension;
    //Determine the repository to use
    Reference<deploy::XPackageManager> xPackageManager;
    if (repository.equals(OUSTR("user")))
        xPackageManager = getUserRepository();
    else if (repository.equals(OUSTR("shared")))
        xPackageManager = getSharedRepository();
    else
        throw lang::IllegalArgumentException(
            OUSTR("No valid repository name provided."),
            static_cast<cppu::OWeakObject*>(this), 0);
    //We must make sure that the xTmpExtension is not create twice, because this
    //would remove the first one.
    ::osl::MutexGuard addGuard(m_addMutex);

    Reference<deploy::XPackageManager> xTmpRepository(getTmpRepository());
        // make sure xTmpRepository is alive as long as xTmpExtension is; as
        // the "tmp" manager is only held weakly by m_xPackageManagerFactory, it
        // could otherwise be disposed early, which would in turn dispose
        // xTmpExtension's PackageRegistryBackend behind its back
    Reference<deploy::XPackage> xTmpExtension(
        xTmpRepository->addPackage(
            url, uno::Sequence<beans::NamedValue>(), OUString(), xAbortChannel,
            new TmpRepositoryCommandEnv()));
    if (!xTmpExtension.is()) {
        throw deploy::DeploymentException(
            ("Extension Manager: Failed to create temporary XPackage for url: "
             + url),
            static_cast<OWeakObject*>(this), uno::Any());
    }

    //Make sure the extension is removed from the tmp repository in case
    //of an exception
    ExtensionRemoveGuard tmpExtensionRemoveGuard(xTmpExtension, getTmpRepository());
    const OUString sIdentifier = dp_misc::getIdentifier(xTmpExtension);
    const OUString sFileName = xTmpExtension->getName();
    Reference<deploy::XPackage> xOldExtension;
    Reference<deploy::XPackage> xExtensionBackup;

    uno::Any excOccurred2;
    bool bUserDisabled = false;
    bool bCanInstall = doChecksForAddExtension(
        xPackageManager,
        properties,
        xTmpExtension,
        xAbortChannel,
        xCmdEnv,
        xOldExtension );

    {
        // In this garded section (getMutex) we must not use the argument xCmdEnv
        // because it may bring up dialogs (XInteractionHandler::handle) this
        //may potententially deadlock. See issue
        //http://qa.openoffice.org/issues/show_bug.cgi?id=114933
        //By not providing xCmdEnv the underlying APIs will throw an exception if
        //the XInteractionRequest cannot be handled
        ::osl::MutexGuard guard(getMutex());

        if (bCanInstall)
        {
            try
            {
                bUserDisabled = isUserDisabled(sIdentifier, sFileName);
                if (xOldExtension.is())
                {
                    try
                    {
                        xOldExtension->revokePackage(
                            false, xAbortChannel, Reference<ucb::XCommandEnvironment>());
                        //save the old user extension in case the user aborts
                        //store the extension in the tmp repository, this will overwrite
                        //xTmpPackage (same identifier). Do not let the user abort or
                        //interact
                        //importing the old extension in the tmp repository will remove
                        //the xTmpExtension
                        //no command environment supplied, only this class shall interact
                        //with the user!
                        xExtensionBackup = getTmpRepository()->importExtension(
                            xOldExtension, Reference<task::XAbortChannel>(),
                            Reference<ucb::XCommandEnvironment>());
                        tmpExtensionRemoveGuard.reset(xExtensionBackup);
                        //xTmpExtension will later be used to check the dependencies
                        //again. However, only xExtensionBackup will be later removed
                        //from the tmp repository
                        xTmpExtension = xExtensionBackup;
                        OSL_ASSERT(xTmpExtension.is());
                    }
                    catch (const lang::DisposedException &)
                    {
                        //Another thread might have removed the extension meanwhile
                    }
                }
                //check again dependencies but prevent user interaction,
                //We can disregard the license, because the user must have already
                //accepted it, when we called checkPrerequisites the first time
                SilentCheckPrerequisitesCommandEnv * pSilentCommandEnv =
                    new SilentCheckPrerequisitesCommandEnv();
                Reference<ucb::XCommandEnvironment> silentCommandEnv(pSilentCommandEnv);

                sal_Int32 failedPrereq = xTmpExtension->checkPrerequisites(
                    xAbortChannel, silentCommandEnv, true);
                if (failedPrereq == 0)
                {
                    xNewExtension = xPackageManager->addPackage(
                        url, properties, OUString(), xAbortChannel,
                        Reference<ucb::XCommandEnvironment>());
                    //If we add a user extension and there is already one which was
                    //disabled by a user, then the newly installed one is enabled. If we
                    //add to another repository then the user extension remains
                    //disabled.
                    bool bUserDisabled2 = bUserDisabled;
                    if (repository.equals(OUSTR("user")))
                        bUserDisabled2 = false;

                    // pass the two values via variables to workaround gcc-4.3.4 specific bug (bnc#655912)
                    OUString sNewExtensionIdentifier = dp_misc::getIdentifier(xNewExtension);
                    OUString sNewExtensionFileName = xNewExtension->getName();

                    activateExtension(
                        sNewExtensionIdentifier, sNewExtensionFileName,
                        bUserDisabled2, false, xAbortChannel,
                        Reference<ucb::XCommandEnvironment>());
                }
                else
                {
                    if (pSilentCommandEnv->m_Exception.hasValue())
                        ::cppu::throwException(pSilentCommandEnv->m_Exception);
                    else if ( pSilentCommandEnv->m_UnknownException.hasValue())
                        ::cppu::throwException(pSilentCommandEnv->m_UnknownException);
                    else
                        throw deploy::DeploymentException (
                            OUSTR("Extension Manager: exception during addExtension, ckeckPrerequisites failed"),
                            static_cast<OWeakObject*>(this), uno::Any());
                }
            }
            catch ( const deploy::DeploymentException& ) {
                excOccurred2 = ::cppu::getCaughtException();
            } catch ( const ucb::CommandFailedException & ) {
                excOccurred2 = ::cppu::getCaughtException();
            } catch ( const ucb::CommandAbortedException & ) {
                excOccurred2 = ::cppu::getCaughtException();
            } catch (const lang::IllegalArgumentException &) {
                excOccurred2 = ::cppu::getCaughtException();
            } catch (const uno::RuntimeException &) {
                excOccurred2 = ::cppu::getCaughtException();
            } catch (...) {
                excOccurred2 = ::cppu::getCaughtException();
                deploy::DeploymentException exc(
                    OUSTR("Extension Manager: exception during addExtension, url: ")
                    + url, static_cast<OWeakObject*>(this), excOccurred2);
                excOccurred2 <<= exc;
            }
        }

        if (excOccurred2.hasValue())
        {
            //It does not matter what exception is thrown. We try to
            //recover the original status.
            //If the user aborted installation then a ucb::CommandAbortedException
            //is thrown.
            //Use a private AbortChannel so the user cannot interrupt.
            try
            {
                if (xExtensionBackup.is())
                {
                    Reference<deploy::XPackage> xRestored =
                        xPackageManager->importExtension(
                            xExtensionBackup, Reference<task::XAbortChannel>(),
                            Reference<ucb::XCommandEnvironment>());
                }
                activateExtension(
                    sIdentifier, sFileName, bUserDisabled, false,
                    Reference<task::XAbortChannel>(), Reference<ucb::XCommandEnvironment>());
            }
            catch (...)
            {
            }
            ::cppu::throwException(excOccurred2);
        }
    } // leaving the garded section (getMutex())

    try
    {
        fireModified();

    }catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (const uno::Exception &) {
        uno::Any excOccurred = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception in doChecksForAddExtension"),
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    } catch (...) {
        throw uno::RuntimeException(
            OUSTR("Extension Manager: unexpected exception in doChecksForAddExtension"),
            static_cast<OWeakObject*>(this));
    }

    return xNewExtension;
}

void ExtensionManager::removeExtension(
    OUString const & identifier, OUString const & fileName,
    OUString const & repository,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
           ucb::CommandFailedException,
           ucb::CommandAbortedException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Any excOccurred1;
    Reference<deploy::XPackage> xExtensionBackup;
    Reference<deploy::XPackageManager> xPackageManager;
    bool bUserDisabled = false;
    ::osl::MutexGuard guard(getMutex());
    try
    {
//Determine the repository to use
        if (repository.equals(OUSTR("user")))
            xPackageManager = getUserRepository();
        else if (repository.equals(OUSTR("shared")))
            xPackageManager = getSharedRepository();
        else
            throw lang::IllegalArgumentException(
                OUSTR("No valid repository name provided."),
                static_cast<cppu::OWeakObject*>(this), 0);

        bUserDisabled = isUserDisabled(identifier, fileName);
        //Backup the extension, in case the user cancels the action
        xExtensionBackup = backupExtension(
            identifier, fileName, xPackageManager, xCmdEnv);

        //revoke the extension if it is active
        Reference<deploy::XPackage> xOldExtension =
            xPackageManager->getDeployedPackage(
                identifier, fileName, xCmdEnv);
        xOldExtension->revokePackage(false, xAbortChannel, xCmdEnv);

        xPackageManager->removePackage(
            identifier, fileName, xAbortChannel, xCmdEnv);
        activateExtension(identifier, fileName, bUserDisabled, false,
                          xAbortChannel, xCmdEnv);
        fireModified();
    }
    catch ( const deploy::DeploymentException& ) {
        excOccurred1 = ::cppu::getCaughtException();
    } catch ( const ucb::CommandFailedException & ) {
        excOccurred1 = ::cppu::getCaughtException();
    } catch ( const ucb::CommandAbortedException & ) {
        excOccurred1 = ::cppu::getCaughtException();
    } catch (const lang::IllegalArgumentException &) {
        excOccurred1 = ::cppu::getCaughtException();
    } catch (const uno::RuntimeException &) {
        excOccurred1 = ::cppu::getCaughtException();
    } catch (...) {
        excOccurred1 = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception during removeEtension"),
            static_cast<OWeakObject*>(this), excOccurred1);
        excOccurred1 <<= exc;
    }

    if (excOccurred1.hasValue())
    {
        //User aborted installation, restore the previous situation.
        //Use a private AbortChannel so the user cannot interrupt.
        try
        {
            Reference<ucb::XCommandEnvironment> tmpCmdEnv(
                new TmpRepositoryCommandEnv(xCmdEnv->getInteractionHandler()));
            if (xExtensionBackup.is())
            {
                Reference<deploy::XPackage> xRestored =
                    xPackageManager->importExtension(
                        xExtensionBackup, Reference<task::XAbortChannel>(),
                        tmpCmdEnv);
                activateExtension(
                    identifier, fileName, bUserDisabled, false,
                    Reference<task::XAbortChannel>(),
                    tmpCmdEnv);

                getTmpRepository()->removePackage(
                    dp_misc::getIdentifier(xExtensionBackup),
                    xExtensionBackup->getName(), xAbortChannel, xCmdEnv);
                fireModified();
            }
        }
        catch (...)
        {
        }
        ::cppu::throwException(excOccurred1);
    }

    if (xExtensionBackup.is())
        getTmpRepository()->removePackage(
            dp_misc::getIdentifier(xExtensionBackup),
            xExtensionBackup->getName(), xAbortChannel, xCmdEnv);
}

// Only enable extensions from shared and user repository
void ExtensionManager::enableExtension(
    Reference<deploy::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
    throw (deploy::DeploymentException,
        ucb::CommandFailedException,
        ucb::CommandAbortedException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    ::osl::MutexGuard guard(getMutex());
    bool bUserDisabled = false;
    uno::Any excOccurred;
    try
    {
        if (!extension.is())
            return;
        OUString repository = extension->getRepositoryName();
        if (!repository.equals(OUSTR("user")))
            throw lang::IllegalArgumentException(
                OUSTR("No valid repository name provided."),
                static_cast<cppu::OWeakObject*>(this), 0);

        bUserDisabled = isUserDisabled(dp_misc::getIdentifier(extension),
                                       extension->getName());

        activateExtension(dp_misc::getIdentifier(extension),
                          extension->getName(), false, false,
                          xAbortChannel, xCmdEnv);
    }
    catch ( const deploy::DeploymentException& ) {
        excOccurred = ::cppu::getCaughtException();
    } catch ( const ucb::CommandFailedException & ) {
        excOccurred = ::cppu::getCaughtException();
    } catch ( const ucb::CommandAbortedException & ) {
        excOccurred = ::cppu::getCaughtException();
    } catch (const lang::IllegalArgumentException &) {
        excOccurred = ::cppu::getCaughtException();
    } catch (const uno::RuntimeException &) {
        excOccurred = ::cppu::getCaughtException();
    } catch (...) {
        excOccurred = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception during enableExtension"),
            static_cast<OWeakObject*>(this), excOccurred);
        excOccurred <<= exc;
    }

    if (excOccurred.hasValue())
    {
        try
        {
            activateExtension(dp_misc::getIdentifier(extension),
                              extension->getName(), bUserDisabled, false,
                              xAbortChannel, xCmdEnv);
        }
        catch (...)
        {
        }
        ::cppu::throwException(excOccurred);
    }
}

/**
 */
sal_Int32 ExtensionManager::checkPrerequisitesAndEnable(
    Reference<deploy::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
    throw (deploy::DeploymentException,
        ucb::CommandFailedException,
        ucb::CommandAbortedException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    try
    {
        if (!extension.is())
            return 0;
        ::osl::MutexGuard guard(getMutex());
        sal_Int32 ret = 0;
        Reference<deploy::XPackageManager> mgr =
            getPackageManager(extension->getRepositoryName());
        ret = mgr->checkPrerequisites(extension, xAbortChannel, xCmdEnv);
        if (ret)
        {
            //There are some unfulfilled prerequisites, try to revoke
            extension->revokePackage(false, xAbortChannel, xCmdEnv);
        }
        const OUString id(dp_misc::getIdentifier(extension));
        activateExtension(id, extension->getName(),
                          isUserDisabled(id, extension->getName()), false,
                          xAbortChannel, xCmdEnv);
        return ret;
    }
    catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (...) {
        uno::Any excOccurred = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception during disableExtension"),
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    }
}

void ExtensionManager::disableExtension(
    Reference<deploy::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
           ucb::CommandFailedException,
           ucb::CommandAbortedException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    ::osl::MutexGuard guard(getMutex());
    uno::Any excOccurred;
    bool bUserDisabled = false;
    try
    {
        if (!extension.is())
            return;
        const OUString repository( extension->getRepositoryName());
        if (!repository.equals(OUSTR("user")))
            throw lang::IllegalArgumentException(
                OUSTR("No valid repository name provided."),
                static_cast<cppu::OWeakObject*>(this), 0);

        const OUString id(dp_misc::getIdentifier(extension));
        bUserDisabled = isUserDisabled(id, extension->getName());

        activateExtension(id, extension->getName(), true, false,
                          xAbortChannel, xCmdEnv);
    }
    catch ( const deploy::DeploymentException& ) {
        excOccurred = ::cppu::getCaughtException();
    } catch ( const ucb::CommandFailedException & ) {
        excOccurred = ::cppu::getCaughtException();
    } catch ( const ucb::CommandAbortedException & ) {
        excOccurred = ::cppu::getCaughtException();
    } catch (const lang::IllegalArgumentException &) {
        excOccurred = ::cppu::getCaughtException();
    } catch (const uno::RuntimeException &) {
        excOccurred = ::cppu::getCaughtException();
    } catch (...) {
        excOccurred = ::cppu::getCaughtException();
        deploy::DeploymentException exc(
            OUSTR("Extension Manager: exception during disableExtension"),
            static_cast<OWeakObject*>(this), excOccurred);
        excOccurred <<= exc;
    }

    if (excOccurred.hasValue())
    {
        try
        {
            activateExtension(dp_misc::getIdentifier(extension),
                              extension->getName(), bUserDisabled, false,
                              xAbortChannel, xCmdEnv);
        }
        catch (...)
        {
        }
        ::cppu::throwException(excOccurred);
    }
}

uno::Sequence< Reference<deploy::XPackage> >
    ExtensionManager::getDeployedExtensions(
    OUString const & repository,
    Reference<task::XAbortChannel> const &xAbort,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
        ucb::CommandFailedException,
        ucb::CommandAbortedException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    return getPackageManager(repository)->getDeployedPackages(
        xAbort, xCmdEnv);
}

Reference<deploy::XPackage>
    ExtensionManager::getDeployedExtension(
    OUString const & repository,
    OUString const & identifier,
    OUString const & filename,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
        ucb::CommandFailedException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    return getPackageManager(repository)->getDeployedPackage(
        identifier, filename, xCmdEnv);
}

uno::Sequence< uno::Sequence<Reference<deploy::XPackage> > >
    ExtensionManager::getAllExtensions(
    Reference<task::XAbortChannel> const & xAbort,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
        ucb::CommandFailedException,
        ucb::CommandAbortedException,
        lang::IllegalArgumentException,
        uno::RuntimeException)
{
    try
    {
        id2extensions mapExt;

        uno::Sequence<Reference<deploy::XPackage> > userExt =
            getUserRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, userExt, OUSTR("user"));
        uno::Sequence<Reference<deploy::XPackage> > sharedExt =
            getSharedRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, sharedExt, OUSTR("shared"));
        uno::Sequence<Reference<deploy::XPackage> > bundledExt =
            getBundledRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, bundledExt, OUSTR("bundled"));

        //copy the values of the map to a vector for sorting
        ::std::vector< ::std::vector<Reference<deploy::XPackage> > >
              vecExtensions;
        id2extensions::const_iterator mapIt = mapExt.begin();
        for (;mapIt != mapExt.end(); ++mapIt)
            vecExtensions.push_back(mapIt->second);

        //sort the element according to the identifier
        ::std::sort(vecExtensions.begin(), vecExtensions.end(), CompIdentifiers());

        ::std::vector< ::std::vector<Reference<deploy::XPackage> > >::const_iterator
              citVecVec = vecExtensions.begin();
        sal_Int32 j = 0;
        uno::Sequence< uno::Sequence<Reference<deploy::XPackage> > > seqSeq(vecExtensions.size());
        for (;citVecVec != vecExtensions.end(); ++citVecVec, j++)
        {
            seqSeq[j] = comphelper::containerToSequence(*citVecVec);
        }
        return seqSeq;

    } catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (...) {
        uno::Any exc = ::cppu::getCaughtException();
        throw deploy::DeploymentException(
            OUSTR("Extension Manager: exception during enableExtension"),
            static_cast<OWeakObject*>(this), exc);
   }
}

// Only to be called from unopkg or soffice bootstrap (with force=true in the
// latter case):
void ExtensionManager::reinstallDeployedExtensions(
    sal_Bool force, OUString const & repository,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
        ucb::CommandFailedException, ucb::CommandAbortedException,
        lang::IllegalArgumentException, uno::RuntimeException)
{
    try
    {
        Reference<deploy::XPackageManager>
            xPackageManager = getPackageManager(repository);

        ::osl::MutexGuard guard(getMutex());
        xPackageManager->reinstallDeployedPackages(
            force, xAbortChannel, xCmdEnv);
        //We must sync here, otherwise we will get exceptions when extensions
        //are removed.
        dp_misc::syncRepositories(force, xCmdEnv);
        const uno::Sequence< Reference<deploy::XPackage> > extensions(
            xPackageManager->getDeployedPackages(xAbortChannel, xCmdEnv));

        for ( sal_Int32 pos = 0; pos < extensions.getLength(); ++pos )
        {
            try
            {
                const OUString id =  dp_misc::getIdentifier(extensions[ pos ]);
                const OUString fileName = extensions[ pos ]->getName();
                OSL_ASSERT(!id.isEmpty());
                activateExtension(id, fileName, false, true, xAbortChannel, xCmdEnv );
            }
            catch (const lang::DisposedException &)
            {
            }
        }
    } catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (...) {
        uno::Any exc = ::cppu::getCaughtException();
        throw deploy::DeploymentException(
            OUSTR("Extension Manager: exception during enableExtension"),
            static_cast<OWeakObject*>(this), exc);
    }
}

sal_Bool ExtensionManager::synchronize(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
    throw (deploy::DeploymentException,
           ucb::CommandFailedException,
           ucb::CommandAbortedException,
           lang::IllegalArgumentException,
           uno::RuntimeException)
{
    try
    {
        sal_Bool bModified = sal_False;

        ::osl::MutexGuard guard(getMutex());
        String sSynchronizingShared(StrSyncRepository::get());
        sSynchronizingShared.SearchAndReplaceAllAscii( "%NAME", OUSTR("shared"));
        dp_misc::ProgressLevel progressShared(xCmdEnv, sSynchronizingShared);
        bModified = getSharedRepository()->synchronize(xAbortChannel, xCmdEnv);
        progressShared.update(OUSTR("\n\n"));

        String sSynchronizingBundled(StrSyncRepository::get());
        sSynchronizingBundled.SearchAndReplaceAllAscii( "%NAME", OUSTR("bundled"));
        dp_misc::ProgressLevel progressBundled(xCmdEnv, sSynchronizingBundled);
        bModified |= getBundledRepository()->synchronize(xAbortChannel, xCmdEnv);
        progressBundled.update(OUSTR("\n\n"));

        //Always determine the active extension.
        //TODO: Is this still necessary?  (It used to be necessary for the
        // first-start optimization:  The setup created the registration data
        // for the bundled extensions (share/prereg/bundled) which was copied to
        // the user installation when a user started OOo for the first time
        // after running setup.  All bundled extensions were registered at that
        // moment.  However, extensions with the same identifier could be in the
        // shared or user repository, in which case the respective bundled
        // extensions had to be revoked.)
        try
        {
            const uno::Sequence<uno::Sequence<Reference<deploy::XPackage> > >
                seqSeqExt = getAllExtensions(xAbortChannel, xCmdEnv);
            for (sal_Int32 i = 0; i < seqSeqExt.getLength(); i++)
            {
                uno::Sequence<Reference<deploy::XPackage> > const & seqExt =
                    seqSeqExt[i];
                activateExtension(seqExt, isUserDisabled(seqExt), true,
                                  xAbortChannel, xCmdEnv);
            }
        }
        catch (...)
        {
            //We catch the exception, so we can write the lastmodified file
            //so we will no repeat this everytime OOo starts.
            OSL_FAIL("Extensions Manager: synchronize");
        }
        OUString lastSyncBundled(RTL_CONSTASCII_USTRINGPARAM(
                                     "$BUNDLED_EXTENSIONS_USER/lastsynchronized"));
        writeLastModified(lastSyncBundled, xCmdEnv, m_xContext);
        OUString lastSyncShared(RTL_CONSTASCII_USTRINGPARAM(
                                    "$SHARED_EXTENSIONS_USER/lastsynchronized"));
        writeLastModified(lastSyncShared, xCmdEnv, m_xContext);
        return bModified;
    } catch ( const deploy::DeploymentException& ) {
        throw;
    } catch ( const ucb::CommandFailedException & ) {
        throw;
    } catch ( const ucb::CommandAbortedException & ) {
        throw;
    } catch (const lang::IllegalArgumentException &) {
        throw;
    } catch (const uno::RuntimeException &) {
        throw;
    } catch (...) {
        uno::Any exc = ::cppu::getCaughtException();
        throw deploy::DeploymentException(
            OUSTR("Extension Manager: exception in synchronize"),
            static_cast<OWeakObject*>(this), exc);
    }
}

// Notify the user when a new extension is to be installed. This is only the
// case when one uses the system integration to install an extension (double
// clicking on .oxt file etc.)). The function must only be called if there is no
// extension with the same identifier already deployed. Then the checkUpdate
// function will inform the user that the extension is about to be installed In
// case the user cancels the installation a CommandFailed exception is
// thrown.
void ExtensionManager::checkInstall(
    OUString const & displayName,
    Reference<ucb::XCommandEnvironment> const & cmdEnv)
{
        uno::Any request(
            deploy::InstallException(
                OUSTR("Extension ") + displayName +
                OUSTR(" is about to be installed."),
                static_cast<OWeakObject *>(this), displayName));
        bool approve = false, abort = false;
        if (! dp_misc::interactContinuation(
                request, task::XInteractionApprove::static_type(),
                cmdEnv, &approve, &abort ))
        {
            OSL_ASSERT( !approve && !abort );
            throw deploy::DeploymentException(
                dp_misc::getResourceString(RID_STR_ERROR_WHILE_ADDING) + displayName,
                static_cast<OWeakObject *>(this), request );
        }
        if (abort || !approve)
            throw ucb::CommandFailedException(
                dp_misc::getResourceString(RID_STR_ERROR_WHILE_ADDING) + displayName,
                static_cast<OWeakObject *>(this), request );
}

/* The function will make the user interaction in case there is an extension
installed with the same id. This function may only be called if there is already
an extension.
*/
void ExtensionManager::checkUpdate(
    OUString const & newVersion,
    OUString const & newDisplayName,
    Reference<deploy::XPackage> const & oldExtension,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    // package already deployed, interact --force:
    uno::Any request(
        (deploy::VersionException(
            dp_misc::getResourceString(
                RID_STR_PACKAGE_ALREADY_ADDED ) + newDisplayName,
            static_cast<OWeakObject *>(this), newVersion, newDisplayName,
            oldExtension ) ) );
    bool replace = false, abort = false;
    if (! dp_misc::interactContinuation(
            request, task::XInteractionApprove::static_type(),
            xCmdEnv, &replace, &abort )) {
        OSL_ASSERT( !replace && !abort );
        throw deploy::DeploymentException(
            dp_misc::getResourceString(
                RID_STR_ERROR_WHILE_ADDING) + newDisplayName,
            static_cast<OWeakObject *>(this), request );
    }
    if (abort || !replace)
        throw ucb::CommandFailedException(
            dp_misc::getResourceString(
                RID_STR_PACKAGE_ALREADY_ADDED) + newDisplayName,
            static_cast<OWeakObject *>(this), request );
}

uno::Sequence<Reference<deploy::XPackage> > SAL_CALL
ExtensionManager::getExtensionsWithUnacceptedLicenses(
        OUString const & repository,
        Reference<ucb::XCommandEnvironment> const & xCmdEnv)
        throw (deploy::DeploymentException,
               uno::RuntimeException)
{
    Reference<deploy::XPackageManager>
        xPackageManager = getPackageManager(repository);
    ::osl::MutexGuard guard(getMutex());
    return xPackageManager->getExtensionsWithUnacceptedLicenses(xCmdEnv);
}

sal_Bool ExtensionManager::isReadOnlyRepository(::rtl::OUString const & repository)
        throw (uno::RuntimeException)
{
    return getPackageManager(repository)->isReadOnly();
}
//------------------------------------------------------------------------------

namespace sdecl = comphelper::service_decl;
sdecl::class_<ExtensionManager> servicePIP;
extern sdecl::ServiceDecl const serviceDecl(
    servicePIP,
    // a private one:
    "com.sun.star.comp.deployment.ExtensionManager",
    "com.sun.star.comp.deployment.ExtensionManager");

// XModifyBroadcaster
//______________________________________________________________________________
void ExtensionManager::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (uno::RuntimeException)
{
     check();
     rBHelper.addListener( ::getCppuType( &xListener ), xListener );
}

//______________________________________________________________________________
void ExtensionManager::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (uno::RuntimeException)
{
    check();
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

void ExtensionManager::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("ExtensionManager instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

void ExtensionManager::fireModified()
{
    ::cppu::OInterfaceContainerHelper * pContainer = rBHelper.getContainer(
        util::XModifyListener::static_type() );
    if (pContainer != 0) {
        pContainer->forEach<util::XModifyListener>(
            boost::bind(&util::XModifyListener::modified, _1,
                        lang::EventObject(static_cast<OWeakObject *>(this))) );
    }
}

} // namespace dp_manager

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
