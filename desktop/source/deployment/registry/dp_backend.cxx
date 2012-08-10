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


#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/uri.hxx"
#include "rtl/bootstrap.hxx"
#include "sal/log.hxx"
#include "osl/file.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/deployment/InvalidRemovedParameterException.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"
#include "com/sun/star/ucb/IOErrorCode.hpp"
#include "com/sun/star/beans/StringPair.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "unotools/tempfile.hxx"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {

//______________________________________________________________________________
PackageRegistryBackend::~PackageRegistryBackend()
{
}

//______________________________________________________________________________
void PackageRegistryBackend::disposing( lang::EventObject const & event )
    throw (RuntimeException)
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

//______________________________________________________________________________
PackageRegistryBackend::PackageRegistryBackend(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext )
    : t_BackendBase( getMutex() ),
      m_xComponentContext( xContext ),
      m_eContext( CONTEXT_UNKNOWN ),
      m_readOnly( false )
{
    boost::optional<OUString> cachePath;
    boost::optional<bool> readOnly;
    comphelper::unwrapArgs( args, m_context, cachePath, readOnly );
    if (cachePath)
        m_cachePath = *cachePath;
    if (readOnly)
        m_readOnly = *readOnly;

    if ( m_context == "user" )
        m_eContext = CONTEXT_USER;
    else if ( m_context == "shared" )
        m_eContext = CONTEXT_SHARED;
    else if ( m_context == "bundled" )
        m_eContext = CONTEXT_BUNDLED;
    else if ( m_context == "tmp" )
        m_eContext = CONTEXT_TMP;
    else if (m_context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:/") ))
        m_eContext = CONTEXT_DOCUMENT;
    else
        m_eContext = CONTEXT_UNKNOWN;
}

//______________________________________________________________________________
void PackageRegistryBackend::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("PackageRegistryBackend instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
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
            OUSTR("caught unexpected exception while disposing!"),
            static_cast<OWeakObject *>(this), exc );
    }
}

// XPackageRegistry
//______________________________________________________________________________
Reference<deployment::XPackage> PackageRegistryBackend::bindPackage(
    OUString const & url, OUString const & mediaType, sal_Bool  bRemoved,
    OUString const & identifier, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           deployment::InvalidRemovedParameterException,
           ucb::CommandFailedException,
           lang::IllegalArgumentException, RuntimeException)
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
                    (OUSTR("XPackageRegistry::bindPackage: media type does not match"),
                     static_cast<OWeakObject*>(this), 1);
            if (xPackage->isRemoved() != bRemoved)
                throw deployment::InvalidRemovedParameterException(
                    OUSTR("XPackageRegistry::bindPackage: bRemoved parameter does not match"),
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
    catch (const lang::IllegalArgumentException &) {
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
            OUSTR("Error binding package: ") + url,
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

    const String baseDir(sDataFolder);
    const ::utl::TempFile aTemp(&baseDir, sal_True);
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

        if (folderUrl[folderUrl.getLength() - 1] == '_')
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
            sDataFolder, Reference<ucb::XCommandEnvironment>());

        Reference<sdbc::XResultSet> xResultSet(
                 StrTitle::createCursor( tempFolder, ::ucbhelper::INCLUDE_FOLDERS_ONLY ) );

        // get all temp directories:
        ::std::vector<OUString> tempEntries;

        const char tmp[] = ".tmp";

        while (xResultSet->next())
        {
            OUString title(
                Reference<sdbc::XRow>(
                    xResultSet, UNO_QUERY_THROW )->getString(
                        1 /* Title */ ) );

            if (title.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(tmp)))
                tempEntries.push_back(
                    makeURLAppendSysPathSegment(sDataFolder, title));
        }

        for ( ::std::size_t pos = 0; pos < tempEntries.size(); ++pos )
        {
            if (::std::find( usedFolders.begin(), usedFolders.end(), tempEntries[pos] ) ==
                usedFolders.end())
            {
                deleteTempFolder(tempEntries[pos]);
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


//______________________________________________________________________________
Package::~Package()
{
}

//______________________________________________________________________________
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

//______________________________________________________________________________
void Package::disposing()
{
    m_myBackend.clear();
    WeakComponentImplHelperBase::disposing();
}

//______________________________________________________________________________
void Package::check() const
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("Package instance has already been disposed!"),
            static_cast<OWeakObject *>(const_cast<Package *>(this)));
    }
}

// XComponent
//______________________________________________________________________________
void Package::dispose() throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::dispose();
}

//______________________________________________________________________________
void Package::addEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::addEventListener( xListener );
}

//______________________________________________________________________________
void Package::removeEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::removeEventListener( xListener );
}

// XModifyBroadcaster
//______________________________________________________________________________
void Package::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.addListener( ::getCppuType( &xListener ), xListener );
}

//______________________________________________________________________________
void Package::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

//______________________________________________________________________________
void Package::checkAborted(
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    if (abortChannel.is() && abortChannel->isAborted()) {
        throw CommandAbortedException(
            OUSTR("abort!"), static_cast<OWeakObject *>(this) );
    }
}

// XPackage
//______________________________________________________________________________
Reference<task::XAbortChannel> Package::createAbortChannel()
    throw (RuntimeException)
{
    check();
    return new AbortChannel;
}

//______________________________________________________________________________
sal_Bool Package::isBundle() throw (RuntimeException)
{
    return false; // default
}

//______________________________________________________________________________
::sal_Int32 Package::checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >&,
        const css::uno::Reference< css::ucb::XCommandEnvironment >&,
        sal_Bool)
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::ucb::CommandAbortedException,
               css::uno::RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return 0;
}

//______________________________________________________________________________
::sal_Bool Package::checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& )
        throw (css::deployment::DeploymentException,
               css::deployment::ExtensionRemovedException,
               css::ucb::CommandFailedException,
               css::uno::RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return true;
}


//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> > Package::getBundle(
    Reference<task::XAbortChannel> const &,
    Reference<XCommandEnvironment> const & )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    return Sequence< Reference<deployment::XPackage> >();
}

//______________________________________________________________________________
OUString Package::getName() throw (RuntimeException)
{
    return m_name;
}

beans::Optional<OUString> Package::getIdentifier() throw (RuntimeException)
{
    if (m_bRemoved)
        return beans::Optional<OUString>(true, m_identifier);

    return beans::Optional<OUString>();
}

//______________________________________________________________________________
OUString Package::getVersion() throw (
    deployment::ExtensionRemovedException,
    RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}

//______________________________________________________________________________
OUString Package::getURL() throw (RuntimeException)
{
    return m_url;
}

//______________________________________________________________________________
OUString Package::getDisplayName() throw (
    deployment::ExtensionRemovedException, RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return m_displayName;
}

//______________________________________________________________________________
OUString Package::getDescription() throw (
    deployment::ExtensionRemovedException,RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}

//______________________________________________________________________________
OUString Package::getLicenseText() throw (
    deployment::ExtensionRemovedException,RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return OUString();
}

//______________________________________________________________________________
Sequence<OUString> Package::getUpdateInformationURLs() throw (
    deployment::ExtensionRemovedException, RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return Sequence<OUString>();
}

//______________________________________________________________________________
css::beans::StringPair Package::getPublisherInfo() throw (
    deployment::ExtensionRemovedException, RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    css::beans::StringPair aEmptyPair;
    return aEmptyPair;
}

//______________________________________________________________________________
uno::Reference< css::graphic::XGraphic > Package::getIcon( sal_Bool /*bHighContrast*/ )
    throw (deployment::ExtensionRemovedException, RuntimeException )
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    uno::Reference< css::graphic::XGraphic > aEmpty;
    return aEmpty;
}

//______________________________________________________________________________
Reference<deployment::XPackageTypeInfo> Package::getPackageType()
    throw (RuntimeException)
{
    return m_xPackageType;
}

//______________________________________________________________________________
void Package::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::ExtensionRemovedException,
           CommandFailedException, CommandAbortedException, RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();

    ::ucbhelper::Content destFolder( destFolderURL, xCmdEnv );
    ::ucbhelper::Content sourceContent( getURL(), xCmdEnv );
    if (! destFolder.transferContent(
            sourceContent, ::ucbhelper::InsertOperation_COPY,
            newTitle, nameClashAction ))
        throw RuntimeException( OUSTR("UCB transferContent() failed!"), 0 );
}

//______________________________________________________________________________
void Package::fireModified()
{
    ::cppu::OInterfaceContainerHelper * container = rBHelper.getContainer(
        ::getCppuType( static_cast<Reference<
                       util::XModifyListener> const *>(0) ) );
    if (container != 0) {
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
//______________________________________________________________________________
beans::Optional< beans::Ambiguous<sal_Bool> > Package::isRegistered(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException, RuntimeException)
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
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw deployment::DeploymentException(
            OUSTR("unexpected exception occurred!"),
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
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

//______________________________________________________________________________
void Package::registerPackage(
    sal_Bool startup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           deployment::ExtensionRemovedException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    processPackage_impl( true /* register */, startup, xAbortChannel, xCmdEnv );
}

//______________________________________________________________________________
void Package::revokePackage(
    sal_Bool startup,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    processPackage_impl( false /* revoke */, startup, xAbortChannel, xCmdEnv );

}

PackageRegistryBackend * Package::getMyBackend() const
{
    PackageRegistryBackend * pBackend = m_myBackend.get();
    if (NULL == pBackend)
    {
        //May throw a DisposedException
        check();
        //We should never get here...
        throw RuntimeException(
            OUSTR("Failed to get the BackendImpl"),
            static_cast<OWeakObject*>(const_cast<Package *>(this)));
    }
    return pBackend;
}
OUString Package::getRepositoryName()
    throw (RuntimeException)
{
    PackageRegistryBackend * backEnd = getMyBackend();
    return backEnd->getContext();
}

beans::Optional< OUString > Package::getRegistrationDataURL()
        throw (deployment::ExtensionRemovedException,
               css::uno::RuntimeException)
{
    if (m_bRemoved)
        throw deployment::ExtensionRemovedException();
    return beans::Optional<OUString>();
}

sal_Bool Package::isRemoved()
    throw (RuntimeException)
{
    return m_bRemoved;
}


//______________________________________________________________________________
Package::TypeInfo::~TypeInfo()
{
}

// XPackageTypeInfo
//______________________________________________________________________________
OUString Package::TypeInfo::getMediaType() throw (RuntimeException)
{
    return m_mediaType;
}

//______________________________________________________________________________
OUString Package::TypeInfo::getDescription()
    throw (deployment::ExtensionRemovedException, RuntimeException)
{
    return getShortDescription();
}

//______________________________________________________________________________
OUString Package::TypeInfo::getShortDescription()
    throw (deployment::ExtensionRemovedException, RuntimeException)
{
    return m_shortDescr;
}

//______________________________________________________________________________
OUString Package::TypeInfo::getFileFilter() throw (RuntimeException)
{
    return m_fileFilter;
}

//______________________________________________________________________________
/**************************
 * Get Icon
 *
 * @param highContrast NOTE: disabled the returning of high contrast icons.
 *                     This bool is a noop now.
 * @param smallIcon    Return the small version of the icon
 */
Any Package::TypeInfo::getIcon( sal_Bool /*highContrast*/, sal_Bool smallIcon )
    throw (RuntimeException)
{
    if (! smallIcon)
        return Any();
    const sal_uInt16 nIconId = m_smallIcon;
    return Any( &nIconId, getCppuType( static_cast<sal_uInt16 const *>(0) ) );
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
