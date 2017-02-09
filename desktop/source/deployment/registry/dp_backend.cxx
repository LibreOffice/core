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

#include <sal/config.h>

#include <cassert>

#include "dp_backend.h"
#include "dp_ucb.h"
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/unwrapargs.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionRemovedException.hpp>
#include <com/sun/star/deployment/InvalidRemovedParameterException.hpp>
#include <com/sun/star/deployment/thePackageManagerFactory.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <unotools/tempfile.hxx>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_registry {
namespace backend {


PackageRegistryBackend::~PackageRegistryBackend()
{
}


void PackageRegistryBackend::disposing( lang::EventObject const & event )
{
    Reference<deployment::XPackage> xPackage(
        event.Source, UNO_QUERY_THROW );
    OUString url( xPackage->getURL() );
    ::osl::MutexGuard guard( getMutex() );
    if ( m_bound.erase( url ) != 1 )
    {
        SAL_WARN("desktop.deployment", "erase(" << url << ") != 1");
    }
}


PackageRegistryBackend::PackageRegistryBackend(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
    : t_BackendBase( getMutex() ),
      m_xComponentContext( xContext ),
      m_eContext( Context::Unknown ),
      m_readOnly( false )
{
    assert(xContext.is());
    boost::optional<OUString> cachePath;
    boost::optional<bool> readOnly;
    comphelper::unwrapArgs( args, m_context, cachePath, readOnly );
    if (cachePath)
        m_cachePath = *cachePath;
    if (readOnly)
        m_readOnly = *readOnly;

    if ( m_context == "user" )
        m_eContext = Context::User;
    else if ( m_context == "shared" )
        m_eContext = Context::Shared;
    else if ( m_context == "bundled" )
        m_eContext = Context::Bundled;
    else if ( m_context == "tmp" )
        m_eContext = Context::Tmp;
    else if (m_context.matchIgnoreAsciiCase("vnd.sun.star.tdoc:/"))
        m_eContext = Context::Document;
    else
        m_eContext = Context::Unknown;
}


void PackageRegistryBackend::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            "PackageRegistryBackend instance has already been disposed!",
            static_cast<OWeakObject *>(this) );
    }
}


void PackageRegistryBackend::disposing()
{
    try {
        for ( t_string2ref::const_iterator i = m_bound.begin(); i != m_bound.end(); ++i)
            i->second->removeEventListener(this);
        m_bound.clear();
        m_xComponentContext.clear();
        WeakComponentImplHelperBase::disposing();
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            "caught unexpected exception while disposing!",
            static_cast<OWeakObject *>(this), exc );
    }
}

// XPackageRegistry

Reference<deployment::XPackage> PackageRegistryBackend::bindPackage(
    OUString const & url, OUString const & mediaType, sal_Bool  bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::osl::ResettableMutexGuard guard( getMutex() );
    check();

    t_string2ref::const_iterator const iFind( m_bound.find( url ) );
    if (iFind != m_bound.end())
    {
        Reference<deployment::XPackage> xPackage( iFind->second );
        if (xPackage.is())
        {
            if (!mediaType.isEmpty() &&
                mediaType != xPackage->getPackageType()->getMediaType())
                throw lang::IllegalArgumentException
                    ("XPackageRegistry::bindPackage: media type does not match",
                     static_cast<OWeakObject*>(this), 1);
            if (xPackage->isRemoved() != bRemoved)
                throw deployment::InvalidRemovedParameterException(
                    "XPackageRegistry::bindPackage: bRemoved parameter does not match",
                    static_cast<OWeakObject*>(this), xPackage->isRemoved(), xPackage);
            return xPackage;
        }
    }

    guard.clear();

    Reference<deployment::XPackage> xNewPackage;
    try {
        xNewPackage = bindPackage_( url, mediaType, bRemoved,
            identifier, xCmdEnv );
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException &) {
        throw;
    }
    catch (const deployment::DeploymentException &) {
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw deployment::DeploymentException(
            "Error binding package: " + url,
            static_cast<OWeakObject *>(this), exc );
    }

    guard.reset();

    ::std::pair< t_string2ref::iterator, bool > insertion(
        m_bound.insert( t_string2ref::value_type( url, xNewPackage ) ) );
    if (insertion.second)
    { // first insertion
        SAL_WARN_IF(
            Reference<XInterface>(insertion.first->second) != xNewPackage,
            "desktop.deployment", "mismatch");
    }
    else
    { // found existing entry
        Reference<deployment::XPackage> xPackage( insertion.first->second );
        if (xPackage.is())
            return xPackage;
        insertion.first->second = xNewPackage;
    }

    guard.clear();
    xNewPackage->addEventListener( this ); // listen for disposing events
    return xNewPackage;
}

OUString PackageRegistryBackend::createFolder(
    OUString const & relUrl,
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    const OUString sDataFolder = makeURL(getCachePath(), relUrl);
    //make sure the folder exist
    ucbhelper::Content dataContent;
    ::dp_misc::create_folder(&dataContent, sDataFolder, xCmdEnv);

    const OUString baseDir(sDataFolder);
    ::utl::TempFile aTemp(&baseDir, true);
    const OUString url = aTemp.GetURL();
    return sDataFolder + url.copy(url.lastIndexOf('/'));
}

//folderURL can have the extension .tmp or .tmp_
//Before OOo 3.4 the created a tmp file with osl_createTempFile and
//then created a Folder with a same name and a trailing '_'
//If the folderURL has no '_' then there is no corresponding tmp file.
void PackageRegistryBackend::deleteTempFolder(
    OUString const & folderUrl)
{
    if (!folderUrl.isEmpty())
    {
        erase_path( folderUrl, Reference<XCommandEnvironment>(),
                    false /* no throw: ignore errors */ );

        if (folderUrl.endsWith("_"))
        {
            const OUString  tempFile = folderUrl.copy(0, folderUrl.getLength() - 1);
            erase_path( tempFile, Reference<XCommandEnvironment>(),
                        false /* no throw: ignore errors */ );
        }
    }
}

//usedFolders can contain folder names which have the extension .tmp or .tmp_
//Before OOo 3.4 we created a tmp file with osl_createTempFile and
//then created a Folder with a same name and a trailing '_'
//If the folderURL has no '_' then there is no corresponding tmp file.
void PackageRegistryBackend::deleteUnusedFolders(
    OUString const & relUrl,
    ::std::list< OUString> const & usedFolders)
{
    try
    {
        const OUString sDataFolder = makeURL(getCachePath(), relUrl);
        ::ucbhelper::Content tempFolder(
            sDataFolder, Reference<ucb::XCommandEnvironment>(), m_xComponentContext);

        Reference<sdbc::XResultSet> xResultSet(
                 StrTitle::createCursor( tempFolder, ::ucbhelper::INCLUDE_FOLDERS_ONLY ) );

        // get all temp directories:
        ::std::vector<OUString> tempEntries;

        while (xResultSet->next())
        {
            OUString title(
                Reference<sdbc::XRow>(
                    xResultSet, UNO_QUERY_THROW )->getString(
                        1 /* Title */ ) );

            if (title.endsWith(".tmp"))
                tempEntries.push_back(
                    makeURLAppendSysPathSegment(sDataFolder, title));
        }

        for (OUString & tempEntrie : tempEntries)
        {
            if (::std::find( usedFolders.begin(), usedFolders.end(), tempEntrie ) ==
                usedFolders.end())
            {
                deleteTempFolder(tempEntrie);
            }
        }
    }
    catch (const ucb::InteractiveAugmentedIOException& e)
    {
        //In case the folder containing all the data folder does not
        //exist yet, we ignore the exception
        if (e.Code != ucb::IOErrorCode_NOT_EXISTING)
            throw;
    }

}


Package::~Package()
{
}


Package::Package( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
                  OUString const & url,
                  OUString const & rName,
                  OUString const & displayName,
                  Reference<deployment::XPackageTypeInfo> const & xPackageType,
                  bool bRemoved,
                  OUString const & identifier)
    : t_PackageBase( getMutex() ),
      m_myBackend( myBackend ),
      m_url( url ),
      m_name( rName ),
      m_displayName( displayName ),
      m_xPackageType( xPackageType ),
      m_bRemoved(bRemoved),
      m_identifier(identifier)
{
    if (m_bRemoved)
    {
        //We use the last segment of the URL
        SAL_WARN_IF(
            !m_name.isEmpty(), "desktop.deployment", "non-empty m_name");
        OUString name = m_url;
        rtl::Bootstrap::expandMacros(name);
        sal_Int32 index = name.lastIndexOf('/');
        if (index != -1 && index < name.getLength())
            m_name = name.copy(index + 1);
    }
}


void Package::disposing()
{
    m_myBackend.clear();
    WeakComponentImplHelperBase::disposing();
}


void Package::check() const
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            "Package instance has already been disposed!",
            static_cast<OWeakObject *>(const_cast<Package *>(this)));
    }
}

// XComponent

void Package::dispose()
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::dispose();
}


void Package::addEventListener(
    Reference<lang::XEventListener> const & xListener )
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::addEventListener( xListener );
}


void Package::removeEventListener(
    Reference<lang::XEventListener> const & xListener )
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::removeEventListener( xListener );
}

// XModifyBroadcaster

void Package::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
    check();
    rBHelper.addListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}


void Package::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
    check();
    rBHelper.removeListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}


void Package::checkAborted(
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    if (abortChannel.is() && abortChannel->isAborted()) {
        throw CommandAbortedException(
            "abort!", static_cast<OWeakObject *>(this) );
    }
}

// XPackage

Reference<task::XAbortChannel> Package::createAbortChannel()
{
    check();
    return new AbortChannel;
}


sal_Bool Package::isBundle()
{
    return false; // default
}


::sal_Int32 Package::checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >&,
        const css::uno::Reference< css::ucb::XCommandEnvironment >&,
        sal_Bool)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return 0;
}


sal_Bool Package::checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return true;
}


Sequence< Reference<deployment::XPackage> > Package::getBundle(
    Reference<task::XAbortChannel> const &,
    Reference<XCommandEnvironment> const & )
{
    return Sequence< Reference<deployment::XPackage> >();
}


OUString Package::getName()
{
    return m_name;
}

beans::Optional<OUString> Package::getIdentifier()
{
    if (m_bRemoved)
        return beans::Optional<OUString>(true, m_identifier);

    return beans::Optional<OUString>();
}


OUString Package::getVersion()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}


OUString Package::getURL()
{
    return m_url;
}


OUString Package::getDisplayName()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return m_displayName;
}


OUString Package::getDescription()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}


OUString Package::getLicenseText()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}


Sequence<OUString> Package::getUpdateInformationURLs()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return Sequence<OUString>();
}


css::beans::StringPair Package::getPublisherInfo()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    css::beans::StringPair aEmptyPair;
    return aEmptyPair;
}


uno::Reference< css::graphic::XGraphic > Package::getIcon( sal_Bool /*bHighContrast*/ )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    uno::Reference< css::graphic::XGraphic > aEmpty;
    return aEmpty;
}


Reference<deployment::XPackageTypeInfo> Package::getPackageType()
{
    return m_xPackageType;
}

void Package::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    ::ucbhelper::Content destFolder( destFolderURL, xCmdEnv, getMyBackend()->getComponentContext() );
    ::ucbhelper::Content sourceContent( getURL(), xCmdEnv, getMyBackend()->getComponentContext() );
    bool bOk=true;
    try
    {
        bOk = destFolder.transferContent(
            sourceContent, ::ucbhelper::InsertOperation::Copy,
            newTitle, nameClashAction);
    }
    catch (const css::ucb::ContentCreationException&)
    {
        bOk = false;
    }

    if (!bOk)
        throw RuntimeException( "UCB transferContent() failed!", nullptr );
}

void Package::fireModified()
{
    ::cppu::OInterfaceContainerHelper * container = rBHelper.getContainer(
        cppu::UnoType<util::XModifyListener>::get() );
    if (container != nullptr) {
        Sequence< Reference<XInterface> > elements(
            container->getElements() );
        lang::EventObject evt( static_cast<OWeakObject *>(this) );
        for ( sal_Int32 pos = 0; pos < elements.getLength(); ++pos )
        {
            Reference<util::XModifyListener> xListener(
                elements[ pos ], UNO_QUERY );
            if (xListener.is())
                xListener->modified( evt );
        }
    }
}

// XPackage

beans::Optional< beans::Ambiguous<sal_Bool> > Package::isRegistered(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    try {
        ::osl::ResettableMutexGuard guard( getMutex() );
        return isRegistered_( guard,
                              AbortChannel::get(xAbortChannel),
                              xCmdEnv );
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException &) {
        throw;
    }
    catch (const CommandAbortedException &) {
        throw;
    }
    catch (const deployment::DeploymentException &) {
        throw;
    }
    catch (const Exception & e) {
        Any exc( ::cppu::getCaughtException() );
        throw deployment::DeploymentException(
            "unexpected " + exc.getValueTypeName() + ": " + e.Message,
            static_cast<OWeakObject *>(this), exc );
    }
}


void Package::processPackage_impl(
    bool doRegisterPackage,
    bool startup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    check();
    bool action = false;

    try {
        try {
            ::osl::ResettableMutexGuard guard( getMutex() );
            beans::Optional< beans::Ambiguous<sal_Bool> > option(
                isRegistered_( guard, AbortChannel::get(xAbortChannel),
                               xCmdEnv ) );
            action = (option.IsPresent &&
                      (option.Value.IsAmbiguous ||
                       (doRegisterPackage ? !option.Value.Value
                                        : option.Value.Value)));
            if (action) {

                OUString displayName = isRemoved() ? getName() : getDisplayName();
                ProgressLevel progress(
                    xCmdEnv,
                    (doRegisterPackage
                     ? PackageRegistryBackend::StrRegisteringPackage::get()
                     : PackageRegistryBackend::StrRevokingPackage::get())
                    + displayName );
                processPackage_( guard,
                                 doRegisterPackage,
                                 startup,
                                 AbortChannel::get(xAbortChannel),
                                 xCmdEnv );
            }
        }
        catch (lang::IllegalArgumentException &) {
            Any e(cppu::getCaughtException());
            throw deployment::DeploymentException(
                ((doRegisterPackage
                  ? getResourceString(RID_STR_ERROR_WHILE_REGISTERING)
                  : getResourceString(RID_STR_ERROR_WHILE_REVOKING))
                 + getDisplayName()),
                static_cast< OWeakObject * >(this), e);
        }
        catch (const RuntimeException &e) {
            SAL_WARN(
                "desktop.deployment",
                "unexpected RuntimeException \"" << e.Message << '"');
            throw;
        }
        catch (const CommandFailedException &) {
            throw;
        }
        catch (const CommandAbortedException &) {
            throw;
        }
        catch (const deployment::DeploymentException &) {
            throw;
        }
        catch (const Exception &) {
            Any exc( ::cppu::getCaughtException() );
            throw deployment::DeploymentException(
                (doRegisterPackage
                 ? getResourceString(RID_STR_ERROR_WHILE_REGISTERING)
                 : getResourceString(RID_STR_ERROR_WHILE_REVOKING))
                + getDisplayName(), static_cast<OWeakObject *>(this), exc );
        }
    }
    catch (...) {
        if (action)
            fireModified();
        throw;
    }
    if (action)
        fireModified();
}


void Package::registerPackage(
    sal_Bool startup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    processPackage_impl( true /* register */, startup, xAbortChannel, xCmdEnv );
}


void Package::revokePackage(
    sal_Bool startup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    processPackage_impl( false /* revoke */, startup, xAbortChannel, xCmdEnv );

}

PackageRegistryBackend * Package::getMyBackend() const
{
    PackageRegistryBackend * pBackend = m_myBackend.get();
    if (nullptr == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            "Failed to get the BackendImpl",
            static_cast<OWeakObject*>(const_cast<Package *>(this)));
    }
    return pBackend;
}

OUString Package::getRepositoryName()
{
    PackageRegistryBackend * backEnd = getMyBackend();
    return backEnd->getContext();
}

beans::Optional< OUString > Package::getRegistrationDataURL()
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return beans::Optional<OUString>();
}

sal_Bool Package::isRemoved()
{
    return m_bRemoved;
}

Package::TypeInfo::~TypeInfo()
{
}

// XPackageTypeInfo

OUString Package::TypeInfo::getMediaType()
{
    return m_mediaType;
}


OUString Package::TypeInfo::getDescription()
{
    return getShortDescription();
}


OUString Package::TypeInfo::getShortDescription()
{
    return m_shortDescr;
}

OUString Package::TypeInfo::getFileFilter()
{
    return m_fileFilter;
}

Any Package::TypeInfo::getIcon( sal_Bool /*highContrast*/, sal_Bool /*smallIcon*/ )
{
    return Any();
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
