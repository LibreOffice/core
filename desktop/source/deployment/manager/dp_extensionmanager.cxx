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


#include <cppuhelper/compbase.hxx>

#include <comphelper/servicedecl.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/bootstrap.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/deployment/thePackageManagerFactory.hpp>
#include <com/sun/star/deployment/XPackageManager.hpp>
#include <com/sun/star/deployment/XPackageManagerFactory.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/deployment/InstallException.hpp>
#include <com/sun/star/deployment/VersionException.hpp>
#include <com/sun/star/deployment/LicenseException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/beans/Ambiguous.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <xmlscript/xml_helper.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>
#include "dp_interact.h"
#include "dp_resource.h"
#include "dp_services.hxx"
#include "dp_ucb.h"
#include "dp_identifier.hxx"
#include "dp_descriptioninfoset.hxx"
#include "dp_extensionmanager.hxx"
#include "dp_commandenvironments.hxx"
#include "dp_properties.hxx"

#include <list>
#include <algorithm>
#include <set>

namespace lang  = com::sun::star::lang;
namespace task = com::sun::star::task;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;
namespace beans = com::sun::star::beans;
namespace util = com::sun::star::util;

using ::com::sun::star::uno::Reference;

namespace {

struct CompIdentifiers
{
    bool operator() (::std::vector<Reference<css::deployment::XPackage> > const & a,
                     ::std::vector<Reference<css::deployment::XPackage> > const & b)
        {
            if (getName(a).compareTo(getName(b)) < 0)
                return true;
            return false;
        }

    static OUString getName(::std::vector<Reference<css::deployment::XPackage> > const & a);
};

OUString CompIdentifiers::getName(::std::vector<Reference<css::deployment::XPackage> > const & a)
{
    OSL_ASSERT(a.size() == 3);
    //get the first non-null reference
    Reference<css::deployment::XPackage>  extension;
    ::std::vector<Reference<css::deployment::XPackage> >::const_iterator it = a.begin();
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
        OString stamp("1" );
        Reference<css::io::XInputStream> xData(
            ::xmlscript::createInputStream(
                    reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                    stamp.getLength() ) );
        ucbStamp.writeStream( xData, true /* replace existing */ );
    }
    catch(...)
    {
        uno::Any exc(::cppu::getCaughtException());
        throw css::deployment::DeploymentException("Failed to update" + url, nullptr, exc);
    }
}

class ExtensionRemoveGuard
{
    css::uno::Reference<css::deployment::XPackage> m_extension;
    css::uno::Reference<css::deployment::XPackageManager> m_xPackageManager;

public:
    ExtensionRemoveGuard(){};
    ExtensionRemoveGuard(
        css::uno::Reference<css::deployment::XPackage> const & extension,
        css::uno::Reference<css::deployment::XPackageManager> const & xPackageManager):
        m_extension(extension), m_xPackageManager(xPackageManager) {}
    ~ExtensionRemoveGuard();

    void set(css::uno::Reference<css::deployment::XPackage> const & extension,
             css::uno::Reference<css::deployment::XPackageManager> const & xPackageManager) {
        m_extension = extension;
        m_xPackageManager = xPackageManager;
    }
};

ExtensionRemoveGuard::~ExtensionRemoveGuard()
{
    try {
        OSL_ASSERT(!(m_extension.is() && !m_xPackageManager.is()));
        if (m_xPackageManager.is() && m_extension.is())
            m_xPackageManager->removePackage(
                dp_misc::getIdentifier(m_extension), OUString(),
                css::uno::Reference<css::task::XAbortChannel>(),
                css::uno::Reference<css::ucb::XCommandEnvironment>());
    } catch (...) {
        OSL_ASSERT(false);
    }
}

}

namespace dp_manager {

//ToDo: bundled extension
ExtensionManager::ExtensionManager( Reference< uno::XComponentContext > const& xContext) :
    ::cppu::WeakComponentImplHelper< css::deployment::XExtensionManager >(getMutex())
    , m_xContext(xContext)
{
    m_xPackageManagerFactory = css::deployment::thePackageManagerFactory::get(m_xContext);
    OSL_ASSERT(m_xPackageManagerFactory.is());

    m_repositoryNames.push_back("user");
    m_repositoryNames.push_back("shared");
    m_repositoryNames.push_back("bundled");
}

ExtensionManager::~ExtensionManager()
{
}

Reference<css::deployment::XPackageManager> ExtensionManager::getUserRepository()
{
    return m_xPackageManagerFactory->getPackageManager("user");
}
Reference<css::deployment::XPackageManager>  ExtensionManager::getSharedRepository()
{
    return m_xPackageManagerFactory->getPackageManager("shared");
}
Reference<css::deployment::XPackageManager>  ExtensionManager::getBundledRepository()
{
    return m_xPackageManagerFactory->getPackageManager("bundled");
}
Reference<css::deployment::XPackageManager>  ExtensionManager::getTmpRepository()
{
    return m_xPackageManagerFactory->getPackageManager("tmp");
}
Reference<css::deployment::XPackageManager>  ExtensionManager::getBakRepository()
{
    return m_xPackageManagerFactory->getPackageManager("bak");
}

Reference<task::XAbortChannel> ExtensionManager::createAbortChannel()
{
    return new dp_misc::AbortChannel;
}

css::uno::Reference<css::deployment::XPackageManager>
ExtensionManager::getPackageManager(OUString const & repository)
{
    Reference<css::deployment::XPackageManager> xPackageManager;
    if (repository == "user")
        xPackageManager = getUserRepository();
    else if (repository == "shared")
        xPackageManager = getSharedRepository();
    else if (repository == "bundled")
        xPackageManager = getBundledRepository();
    else if (repository == "tmp")
        xPackageManager = getTmpRepository();
    else if (repository == "bak")
        xPackageManager = getBakRepository();
    else
        throw lang::IllegalArgumentException(
            "No valid repository name provided.",
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
    uno::Sequence<Reference<css::deployment::XPackage> > const & seqExt,
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
        Reference<css::deployment::XPackage> const & xExtension = seqExt[i];
        OUString id = dp_misc::getIdentifier(xExtension);
        id2extensions::iterator ivec =  mapExt.find(id);
        if (ivec == mapExt.end())
        {
            ::std::vector<Reference<css::deployment::XPackage> > vec(3);
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
::std::list<Reference<css::deployment::XPackage> >
    ExtensionManager::getExtensionsWithSameId(
        OUString const & identifier, OUString const & fileName,
        Reference< ucb::XCommandEnvironment> const & /*xCmdEnv*/)

{
    ::std::list<Reference<css::deployment::XPackage> > extensionList;
    Reference<css::deployment::XPackageManager> lRepos[] = {
          getUserRepository(), getSharedRepository(), getBundledRepository() };
    for (int i(0); i != SAL_N_ELEMENTS(lRepos); ++i)
    {
        Reference<css::deployment::XPackage> xPackage;
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

uno::Sequence<Reference<css::deployment::XPackage> >
ExtensionManager::getExtensionsWithSameIdentifier(
        OUString const & identifier,
        OUString const & fileName,
        Reference< ucb::XCommandEnvironment> const & xCmdEnv )
{
    try
    {
        ::std::list<Reference<css::deployment::XPackage> > listExtensions =
            getExtensionsWithSameId(
                identifier, fileName, xCmdEnv);
        bool bHasExtension = false;

        //throw an IllegalArgumentException if there is no extension at all.
        typedef  ::std::list<Reference<css::deployment::XPackage> >::const_iterator CIT;
        for (CIT i = listExtensions.begin(); i != listExtensions.end(); ++i)
            bHasExtension |= i->is();
        if (!bHasExtension)
            throw lang::IllegalArgumentException(
                "Could not find extension: " + identifier + ", " + fileName,
                static_cast<cppu::OWeakObject*>(this), -1);

        return comphelper::containerToSequence(listExtensions);
    }
    catch ( const css::deployment::DeploymentException & )
    {
        throw;
    }
    catch ( const ucb::CommandFailedException & )
    {
        throw;
    }
    catch (css::uno::RuntimeException &)
    {
        throw;
    }
    catch (...)
    {
        uno::Any exc = ::cppu::getCaughtException();
        throw css::deployment::DeploymentException(
            "Extension Manager: exception during getExtensionsWithSameIdentifier",
            static_cast<OWeakObject*>(this), exc);
    }
}

bool ExtensionManager::isUserDisabled(
    OUString const & identifier, OUString const & fileName)
{
    ::std::list<Reference<css::deployment::XPackage> > listExtensions;

    try {
        listExtensions = getExtensionsWithSameId(identifier, fileName);
    } catch ( const lang::IllegalArgumentException & ) {
    }
    OSL_ASSERT(listExtensions.size() == 3);

    return isUserDisabled( ::comphelper::containerToSequence(listExtensions) );
}

bool ExtensionManager::isUserDisabled(
    uno::Sequence<Reference<css::deployment::XPackage> > const & seqExtSameId)
{
    OSL_ASSERT(seqExtSameId.getLength() == 3);
    Reference<css::deployment::XPackage> const & userExtension = seqExtSameId[0];
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
    ::std::list<Reference<css::deployment::XPackage> > listExtensions;
    try {
        listExtensions = getExtensionsWithSameId(identifier, fileName);
    } catch (const lang::IllegalArgumentException &) {
    }
    OSL_ASSERT(listExtensions.size() == 3);

    activateExtension(
        ::comphelper::containerToSequence(listExtensions),
        bUserDisabled, bStartup, xAbortChannel, xCmdEnv);

    fireModified();
}

void ExtensionManager::activateExtension(
    uno::Sequence<Reference<css::deployment::XPackage> > const & seqExt,
    bool bUserDisabled,
    bool bStartup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    bool bActive = false;
    sal_Int32 len = seqExt.getLength();
    for (sal_Int32 i = 0; i < len; i++)
    {
        Reference<css::deployment::XPackage> const & aExt =  seqExt[i];
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

Reference<css::deployment::XPackage> ExtensionManager::backupExtension(
    OUString const & identifier, OUString const & fileName,
    Reference<css::deployment::XPackageManager> const & xPackageManager,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    Reference<css::deployment::XPackage> xBackup;
    Reference<ucb::XCommandEnvironment> tmpCmdEnv(
        new TmpRepositoryCommandEnv(xCmdEnv->getInteractionHandler()));
    Reference<css::deployment::XPackage> xOldExtension;
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
uno::Sequence< Reference<css::deployment::XPackageTypeInfo> >
ExtensionManager::getSupportedPackageTypes()
{
    return getUserRepository()->getSupportedPackageTypes();
}
//Do some necessary checks and user interaction. This function does not
//acquire the extension manager mutex and that mutex must not be acquired
//when this function is called. doChecksForAddExtension does  synchronous
//user interactions which may require acquiring the solar mutex.
//Returns true if the extension can be installed.
bool ExtensionManager::doChecksForAddExtension(
    Reference<css::deployment::XPackageManager> const & xPackageMgr,
    uno::Sequence<beans::NamedValue> const & properties,
    css::uno::Reference<css::deployment::XPackage> const & xTmpExtension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv,
    Reference<css::deployment::XPackage> & out_existingExtension )
{
    try
    {
        Reference<css::deployment::XPackage> xOldExtension;
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
            _xCmdEnv.set(new NoLicenseCommandEnv(xCmdEnv->getInteractionHandler()));

        bCanInstall = xTmpExtension->checkPrerequisites(
            xAbortChannel, _xCmdEnv, xOldExtension.is() || props.isExtensionUpdate()) == 0;

        return bCanInstall;
    }
    catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception in doChecksForAddExtension",
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    } catch (...) {
        throw uno::RuntimeException(
            "Extension Manager: unexpected exception in doChecksForAddExtension",
            static_cast<OWeakObject*>(this));
    }
}

// Only add to shared and user repository
Reference<css::deployment::XPackage> ExtensionManager::addExtension(
    OUString const & url, uno::Sequence<beans::NamedValue> const & properties,
    OUString const & repository,
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    Reference<css::deployment::XPackage> xNewExtension;
    //Determine the repository to use
    Reference<css::deployment::XPackageManager> xPackageManager;
    if (repository == "user")
        xPackageManager = getUserRepository();
    else if (repository == "shared")
        xPackageManager = getSharedRepository();
    else
        throw lang::IllegalArgumentException(
            "No valid repository name provided.",
            static_cast<cppu::OWeakObject*>(this), 0);
    //We must make sure that the xTmpExtension is not create twice, because this
    //would remove the first one.
    ::osl::MutexGuard addGuard(m_addMutex);

    Reference<css::deployment::XPackageManager> xTmpRepository(getTmpRepository());
        // make sure xTmpRepository is alive as long as xTmpExtension is; as
        // the "tmp" manager is only held weakly by m_xPackageManagerFactory, it
        // could otherwise be disposed early, which would in turn dispose
        // xTmpExtension's PackageRegistryBackend behind its back
    Reference<css::deployment::XPackage> xTmpExtension(
        xTmpRepository->addPackage(
            url, uno::Sequence<beans::NamedValue>(), OUString(), xAbortChannel,
            new TmpRepositoryCommandEnv()));
    if (!xTmpExtension.is()) {
        throw css::deployment::DeploymentException(
            ("Extension Manager: Failed to create temporary XPackage for url: "
             + url),
            static_cast<OWeakObject*>(this), uno::Any());
    }

    //Make sure the extension is removed from the tmp repository in case
    //of an exception
    ExtensionRemoveGuard tmpExtensionRemoveGuard(xTmpExtension, getTmpRepository());
    ExtensionRemoveGuard bakExtensionRemoveGuard;
    const OUString sIdentifier = dp_misc::getIdentifier(xTmpExtension);
    const OUString sFileName = xTmpExtension->getName();
    Reference<css::deployment::XPackage> xOldExtension;
    Reference<css::deployment::XPackage> xExtensionBackup;

    uno::Any excOccurred2;
    bool bCanInstall = doChecksForAddExtension(
        xPackageManager,
        properties,
        xTmpExtension,
        xAbortChannel,
        xCmdEnv,
        xOldExtension );

    {
        bool bUserDisabled = false;
        // In this guarded section (getMutex) we must not use the argument xCmdEnv
        // because it may bring up dialogs (XInteractionHandler::handle) this
        // may potentially deadlock. See issue
        // http://qa.openoffice.org/issues/show_bug.cgi?id=114933
        // By not providing xCmdEnv the underlying APIs will throw an exception if
        // the XInteractionRequest cannot be handled.
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
                        xExtensionBackup = getBakRepository()->importExtension(
                            xOldExtension, Reference<task::XAbortChannel>(),
                            Reference<ucb::XCommandEnvironment>());
                        bakExtensionRemoveGuard.set(xExtensionBackup, getBakRepository());
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
                    if (repository == "user")
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
                        throw css::deployment::DeploymentException (
                            "Extension Manager: exception during addExtension, ckeckPrerequisites failed",
                            static_cast<OWeakObject*>(this), uno::Any());
                }
            }
            catch ( const css::deployment::DeploymentException& ) {
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
                css::deployment::DeploymentException exc(
                    "Extension Manager: exception during addExtension, url: "
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
                    Reference<css::deployment::XPackage> xRestored =
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
    } // leaving the guarded section (getMutex())

    try
    {
        fireModified();

    }catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception in doChecksForAddExtension",
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    } catch (...) {
        throw uno::RuntimeException(
            "Extension Manager: unexpected exception in doChecksForAddExtension",
            static_cast<OWeakObject*>(this));
    }

    return xNewExtension;
}

void ExtensionManager::removeExtension(
    OUString const & identifier, OUString const & fileName,
    OUString const & repository,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    uno::Any excOccurred1;
    Reference<css::deployment::XPackage> xExtensionBackup;
    Reference<css::deployment::XPackageManager> xPackageManager;
    bool bUserDisabled = false;
    ::osl::MutexGuard guard(getMutex());
    try
    {
//Determine the repository to use
        if (repository == "user")
            xPackageManager = getUserRepository();
        else if (repository == "shared")
            xPackageManager = getSharedRepository();
        else
            throw lang::IllegalArgumentException(
                "No valid repository name provided.",
                static_cast<cppu::OWeakObject*>(this), 0);

        bUserDisabled = isUserDisabled(identifier, fileName);
        //Backup the extension, in case the user cancels the action
        xExtensionBackup = backupExtension(
            identifier, fileName, xPackageManager, xCmdEnv);

        //revoke the extension if it is active
        Reference<css::deployment::XPackage> xOldExtension =
            xPackageManager->getDeployedPackage(
                identifier, fileName, xCmdEnv);
        xOldExtension->revokePackage(false, xAbortChannel, xCmdEnv);

        xPackageManager->removePackage(
            identifier, fileName, xAbortChannel, xCmdEnv);
        activateExtension(identifier, fileName, bUserDisabled, false,
                          xAbortChannel, xCmdEnv);
        fireModified();
    }
    catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception during removeEtension",
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
                Reference<css::deployment::XPackage> xRestored =
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
    Reference<css::deployment::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    ::osl::MutexGuard guard(getMutex());
    bool bUserDisabled = false;
    uno::Any excOccurred;
    try
    {
        if (!extension.is())
            return;
        OUString repository = extension->getRepositoryName();
        if (!(repository == "user"))
            throw lang::IllegalArgumentException(
                "No valid repository name provided.",
                static_cast<cppu::OWeakObject*>(this), 0);

        bUserDisabled = isUserDisabled(dp_misc::getIdentifier(extension),
                                       extension->getName());

        activateExtension(dp_misc::getIdentifier(extension),
                          extension->getName(), false, false,
                          xAbortChannel, xCmdEnv);
    }
    catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception during enableExtension",
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
    Reference<css::deployment::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    try
    {
        if (!extension.is())
            return 0;
        ::osl::MutexGuard guard(getMutex());
        sal_Int32 ret = 0;
        Reference<css::deployment::XPackageManager> mgr =
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
    catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception during disableExtension",
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    }
}

void ExtensionManager::disableExtension(
    Reference<css::deployment::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    ::osl::MutexGuard guard(getMutex());
    uno::Any excOccurred;
    bool bUserDisabled = false;
    try
    {
        if (!extension.is())
            return;
        const OUString repository( extension->getRepositoryName());
        if (! (repository == "user"))
            throw lang::IllegalArgumentException(
                "No valid repository name provided.",
                static_cast<cppu::OWeakObject*>(this), 0);

        const OUString id(dp_misc::getIdentifier(extension));
        bUserDisabled = isUserDisabled(id, extension->getName());

        activateExtension(id, extension->getName(), true, false,
                          xAbortChannel, xCmdEnv);
    }
    catch ( const css::deployment::DeploymentException& ) {
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
        css::deployment::DeploymentException exc(
            "Extension Manager: exception during disableExtension",
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

uno::Sequence< Reference<css::deployment::XPackage> >
    ExtensionManager::getDeployedExtensions(
    OUString const & repository,
    Reference<task::XAbortChannel> const &xAbort,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    return getPackageManager(repository)->getDeployedPackages(
        xAbort, xCmdEnv);
}

Reference<css::deployment::XPackage>
    ExtensionManager::getDeployedExtension(
    OUString const & repository,
    OUString const & identifier,
    OUString const & filename,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    return getPackageManager(repository)->getDeployedPackage(
        identifier, filename, xCmdEnv);
}

uno::Sequence< uno::Sequence<Reference<css::deployment::XPackage> > >
    ExtensionManager::getAllExtensions(
    Reference<task::XAbortChannel> const & xAbort,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    try
    {
        id2extensions mapExt;

        uno::Sequence<Reference<css::deployment::XPackage> > userExt =
            getUserRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, userExt, "user");
        uno::Sequence<Reference<css::deployment::XPackage> > sharedExt =
            getSharedRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, sharedExt, "shared");
        uno::Sequence<Reference<css::deployment::XPackage> > bundledExt =
            getBundledRepository()->getDeployedPackages(xAbort, xCmdEnv);
        addExtensionsToMap(mapExt, bundledExt, "bundled");

        // Create the tmp repository to trigger its clean up (deletion
        // of old temporary data.)
        getTmpRepository();

        //copy the values of the map to a vector for sorting
        ::std::vector< ::std::vector<Reference<css::deployment::XPackage> > >
              vecExtensions;
        id2extensions::const_iterator mapIt = mapExt.begin();
        for (;mapIt != mapExt.end(); ++mapIt)
            vecExtensions.push_back(mapIt->second);

        //sort the element according to the identifier
        ::std::sort(vecExtensions.begin(), vecExtensions.end(), CompIdentifiers());

        ::std::vector< ::std::vector<Reference<css::deployment::XPackage> > >::const_iterator
              citVecVec = vecExtensions.begin();
        sal_Int32 j = 0;
        uno::Sequence< uno::Sequence<Reference<css::deployment::XPackage> > > seqSeq(vecExtensions.size());
        for (;citVecVec != vecExtensions.end(); ++citVecVec, j++)
        {
            seqSeq[j] = comphelper::containerToSequence(*citVecVec);
        }
        return seqSeq;

    } catch ( const css::deployment::DeploymentException& ) {
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
        throw css::deployment::DeploymentException(
            "Extension Manager: exception during enableExtension",
            static_cast<OWeakObject*>(this), exc);
   }
}

// Only to be called from unopkg or soffice bootstrap (with force=true in the
// latter case):
void ExtensionManager::reinstallDeployedExtensions(
    sal_Bool force, OUString const & repository,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    try
    {
        Reference<css::deployment::XPackageManager>
            xPackageManager = getPackageManager(repository);

        std::set< OUString > disabledExts;
        {
            const uno::Sequence< Reference<css::deployment::XPackage> > extensions(
                xPackageManager->getDeployedPackages(xAbortChannel, xCmdEnv));
            for ( sal_Int32 pos = 0; pos < extensions.getLength(); ++pos )
            {
                try
                {
                    beans::Optional< beans::Ambiguous< sal_Bool > > registered(
                        extensions[pos]->isRegistered(xAbortChannel, xCmdEnv));
                    if (registered.IsPresent &&
                        !(registered.Value.IsAmbiguous ||
                          registered.Value.Value))
                    {
                        const OUString id = dp_misc::getIdentifier(extensions[ pos ]);
                        OSL_ASSERT(!id.isEmpty());
                        disabledExts.insert(id);
                    }
                }
                catch (const lang::DisposedException &)
                {
                }
            }
        }

        ::osl::MutexGuard guard(getMutex());
        xPackageManager->reinstallDeployedPackages(
            force, xAbortChannel, xCmdEnv);
        //We must sync here, otherwise we will get exceptions when extensions
        //are removed.
        dp_misc::syncRepositories(force, xCmdEnv);
        const uno::Sequence< Reference<css::deployment::XPackage> > extensions(
            xPackageManager->getDeployedPackages(xAbortChannel, xCmdEnv));

        for ( sal_Int32 pos = 0; pos < extensions.getLength(); ++pos )
        {
            try
            {
                const OUString id =  dp_misc::getIdentifier(extensions[ pos ]);
                const OUString fileName = extensions[ pos ]->getName();
                OSL_ASSERT(!id.isEmpty());
                activateExtension(
                    id, fileName, disabledExts.find(id) != disabledExts.end(),
                    true, xAbortChannel, xCmdEnv );
            }
            catch (const lang::DisposedException &)
            {
            }
        }
    } catch ( const css::deployment::DeploymentException& ) {
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
        throw css::deployment::DeploymentException(
            "Extension Manager: exception during enableExtension",
            static_cast<OWeakObject*>(this), exc);
    }
}

sal_Bool ExtensionManager::synchronize(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    try
    {
        ::osl::MutexGuard guard(getMutex());
        OUString sSynchronizingShared(StrSyncRepository::get());
        sSynchronizingShared = sSynchronizingShared.replaceAll("%NAME", "shared");
        dp_misc::ProgressLevel progressShared(xCmdEnv, sSynchronizingShared);
        bool bModified = getSharedRepository()->synchronize(xAbortChannel, xCmdEnv);
        progressShared.update("\n\n");

        OUString sSynchronizingBundled(StrSyncRepository::get());
        sSynchronizingBundled = sSynchronizingBundled.replaceAll("%NAME", "bundled");
        dp_misc::ProgressLevel progressBundled(xCmdEnv, sSynchronizingBundled);
        bModified |= (bool)getBundledRepository()->synchronize(xAbortChannel, xCmdEnv);
        progressBundled.update("\n\n");

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
            const uno::Sequence<uno::Sequence<Reference<css::deployment::XPackage> > >
                seqSeqExt = getAllExtensions(xAbortChannel, xCmdEnv);
            for (sal_Int32 i = 0; i < seqSeqExt.getLength(); i++)
            {
                uno::Sequence<Reference<css::deployment::XPackage> > const & seqExt =
                    seqSeqExt[i];
                activateExtension(seqExt, isUserDisabled(seqExt), true,
                                  xAbortChannel, xCmdEnv);
            }
        }
        catch (...)
        {
            //We catch the exception, so we can write the lastmodified file
            //so we will no repeat this every time OOo starts.
            OSL_FAIL("Extensions Manager: synchronize");
        }
        OUString lastSyncBundled("$BUNDLED_EXTENSIONS_USER/lastsynchronized");
        writeLastModified(lastSyncBundled, xCmdEnv, m_xContext);
        OUString lastSyncShared("$SHARED_EXTENSIONS_USER/lastsynchronized");
        writeLastModified(lastSyncShared, xCmdEnv, m_xContext);
        return bModified;
    } catch ( const css::deployment::DeploymentException& ) {
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
        throw css::deployment::DeploymentException(
            "Extension Manager: exception in synchronize",
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
            css::deployment::InstallException(
                "Extension " + displayName +
                " is about to be installed.",
                static_cast<OWeakObject *>(this), displayName));
        bool approve = false, abort = false;
        if (! dp_misc::interactContinuation(
                request, cppu::UnoType<task::XInteractionApprove>::get(),
                cmdEnv, &approve, &abort ))
        {
            OSL_ASSERT( !approve && !abort );
            throw css::deployment::DeploymentException(
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
    Reference<css::deployment::XPackage> const & oldExtension,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv )
{
    // package already deployed, interact --force:
    uno::Any request(
        (css::deployment::VersionException(
            dp_misc::getResourceString(
                RID_STR_PACKAGE_ALREADY_ADDED ) + newDisplayName,
            static_cast<OWeakObject *>(this), newVersion, newDisplayName,
            oldExtension ) ) );
    bool replace = false, abort = false;
    if (! dp_misc::interactContinuation(
            request, cppu::UnoType<task::XInteractionApprove>::get(),
            xCmdEnv, &replace, &abort )) {
        OSL_ASSERT( !replace && !abort );
        throw css::deployment::DeploymentException(
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

uno::Sequence<Reference<css::deployment::XPackage> > SAL_CALL
ExtensionManager::getExtensionsWithUnacceptedLicenses(
        OUString const & repository,
        Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    Reference<css::deployment::XPackageManager>
        xPackageManager = getPackageManager(repository);
    ::osl::MutexGuard guard(getMutex());
    return xPackageManager->getExtensionsWithUnacceptedLicenses(xCmdEnv);
}

sal_Bool ExtensionManager::isReadOnlyRepository(OUString const & repository)
{
    return getPackageManager(repository)->isReadOnly();
}


namespace sdecl = comphelper::service_decl;
sdecl::class_<ExtensionManager> const servicePIP;
sdecl::ServiceDecl const serviceDecl(
    servicePIP,
    // a private one:
    "com.sun.star.comp.deployment.ExtensionManager",
    "com.sun.star.comp.deployment.ExtensionManager");

// XModifyBroadcaster

void ExtensionManager::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
     check();
     rBHelper.addListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}


void ExtensionManager::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
    check();
    rBHelper.removeListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}

void ExtensionManager::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            "ExtensionManager instance has already been disposed!",
            static_cast<OWeakObject *>(this) );
    }
}

void ExtensionManager::fireModified()
{
    ::cppu::OInterfaceContainerHelper * pContainer = rBHelper.getContainer(
        cppu::UnoType<util::XModifyListener>::get() );
    if (pContainer != nullptr) {
        pContainer->forEach<util::XModifyListener>(
            [this] (uno::Reference<util::XModifyListener> const& xListener)
                { return xListener->modified(lang::EventObject(static_cast<OWeakObject *>(this))); });
    }
}

} // namespace dp_manager

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
