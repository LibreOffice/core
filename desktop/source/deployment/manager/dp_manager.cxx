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

#include <config_features.h>

#include <dp_registry.hxx>
#include <dp_shared.hxx>
#include <strings.hrc>
#include <dp_ucb.h>
#include <dp_resource.h>
#include <dp_platform.hxx>
#include "dp_manager.h"
#include <dp_identifier.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/logging.hxx>
#include <comphelper/servicedecl.hxx>
#include <comphelper/sequence.hxx>
#include <xmlscript/xml_helper.hxx>
#include <svl/inettype.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/logging/LogLevel.hpp>
#include <com/sun/star/logging/FileHandler.hpp>
#include <com/sun/star/logging/SimpleTextFormatter.hpp>
#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/InvalidRemovedParameterException.hpp>
#include <com/sun/star/deployment/VersionException.hpp>
#include <com/sun/star/deployment/InstallException.hpp>
#include <com/sun/star/deployment/Prerequisites.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <unotools/tempfile.hxx>

#include <dp_descriptioninfoset.hxx>
#include "dp_commandenvironments.hxx"
#include "dp_properties.hxx"

#include <vector>
#include <list>
#include <algorithm>

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::logging;

namespace dp_log {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace dp_manager {

struct MatchTempDir
{
    OUString m_str;
    explicit MatchTempDir( OUString const & str ) : m_str( str ) {}
    bool operator () ( ActivePackages::Entries::value_type const & v ) const {
        return v.second.temporaryName.equalsIgnoreAsciiCase( m_str );
    }
};


namespace {
OUString getExtensionFolder(OUString const &  parentFolder,
                            Reference<ucb::XCommandEnvironment> const & xCmdEnv,
                            Reference<uno::XComponentContext> const & xContext)
{
    ::ucbhelper::Content tempFolder( parentFolder, xCmdEnv, xContext );
    Reference<sdbc::XResultSet> xResultSet(
                StrTitle::createCursor (tempFolder, ::ucbhelper::INCLUDE_FOLDERS_ONLY ) );

    OUString title;
    if (xResultSet->next())
    {
        title = Reference<sdbc::XRow>(
            xResultSet, UNO_QUERY_THROW )->getString(1 /* Title */ ) ;
    }
    return title;
}
}

void PackageManagerImpl::initActivationLayer(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_activePackages.isEmpty())
    {
        OSL_ASSERT( m_registryCache.isEmpty() );
        // documents temp activation:
        m_activePackagesDB.reset( new ActivePackages );
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, m_context, xCmdEnv,
                                false /* no throw */ ))
        {
            // scan for all entries in m_packagesDir:
            Reference<sdbc::XResultSet> xResultSet(
                        StrTitle::createCursor (ucbContent, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS ) );

            while (xResultSet->next())
            {
                Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
                OUString title( xRow->getString( 1 /* Title */ ) );
                // xxx todo: remove workaround for tdoc
                if ( title == "this_is_a_dummy_stream_just_there_as_a_workaround_for_a_temporary_limitation_of_the_storage_api_implementation" )
                    continue;
                if ( title == "META-INF" )
                    continue;

                ::ucbhelper::Content sourceContent(
                    Reference<XContentAccess>(
                        xResultSet, UNO_QUERY_THROW )->queryContent(),
                    xCmdEnv, m_xComponentContext );

                OUString mediaType( detectMediaType( sourceContent,
                                                     false /* no throw */) );
                if (!mediaType.isEmpty())
                {
                    ActivePackages::Data dbData;
                    insertToActivationLayer(
                        Sequence<css::beans::NamedValue>(),mediaType, sourceContent,
                        title, &dbData );

                    insertToActivationLayerDB( title, dbData );
                        //TODO #i73136#: insertToActivationLayerDB needs id not
                        // title, but the whole m_activePackages.getLength()==0
                        // case (i.e., document-relative deployment) currently
                        // does not work, anyway.
                }
            }
        }
    }
    else
    {
        // user|share:
        OSL_ASSERT( !m_activePackages.isEmpty() );
        m_activePackages_expanded = expandUnoRcUrl( m_activePackages );
        m_registrationData_expanded = expandUnoRcUrl(m_registrationData);
        if (!m_readOnly)
            create_folder( nullptr, m_activePackages_expanded, xCmdEnv);

        OUString dbName;
        if (m_context == "user")
            dbName = m_activePackages_expanded + ".pmap";
        else
        {
            // Create the extension data base in the user installation
            create_folder( nullptr, m_registrationData_expanded, xCmdEnv);
            dbName = m_registrationData_expanded + "/extensions.pmap";
        }
        // The data base can always be written because it is always in the user installation
        m_activePackagesDB.reset( new ActivePackages( dbName ) );

        if (! m_readOnly && m_context != "bundled")
        {
            // clean up activation layer, scan for zombie temp dirs:
            ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

            ::ucbhelper::Content tempFolder( m_activePackages_expanded, xCmdEnv, m_xComponentContext );
            Reference<sdbc::XResultSet> xResultSet(
                StrTitle::createCursor (tempFolder,
                                         ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

            // get all temp directories:
            std::vector<OUString> tempEntries;
            std::vector<OUString> removedEntries;
            while (xResultSet->next())
            {
                OUString title(
                    Reference<sdbc::XRow>(
                        xResultSet, UNO_QUERY_THROW )->getString(
                            1 /* Title */ ) );
                if (title.endsWith("removed", &title))
                {
                    //save the file name withouth the "removed" part
                    removedEntries.push_back(::rtl::Uri::encode(
                                                title, rtl_UriCharClassPchar,
                                                rtl_UriEncodeIgnoreEscapes,
                                                RTL_TEXTENCODING_UTF8 ) );
                }
                else
                {
                    tempEntries.push_back( ::rtl::Uri::encode(
                                               title, rtl_UriCharClassPchar,
                                               rtl_UriEncodeIgnoreEscapes,
                                               RTL_TEXTENCODING_UTF8 ) );
                }
            }

            bool bShared = (m_context == "shared");
            for (OUString & tempEntry : tempEntries)
            {
                const MatchTempDir match( tempEntry );
                if (std::none_of( id2temp.begin(), id2temp.end(), match ))
                {
                    const OUString url(
                        makeURL(m_activePackages_expanded, tempEntry ) );

                    //In case of shared extensions, new entries are regarded as
                    //added extensions if there is no xxx.tmpremoved file.
                    if (bShared)
                    {
                        if (std::find(removedEntries.begin(), removedEntries.end(), tempEntry) ==
                            removedEntries.end())
                        {
                            continue;
                        }
                        else
                        {
                            //Make sure only the same user removes the extension, who
                            //previously unregistered it. This is avoid races if multiple instances
                            //of OOo are running which all have write access to the shared installation.
                            //For example, a user removes the extension, but keeps OOo
                            //running. Parts of the extension may still be loaded and used by OOo.
                            //Therefore the extension is only deleted the next time the extension manager is
                            //run after restarting OOo. While OOo is still running, another user starts OOo
                            //which would deleted the extension files. If the same user starts another
                            //instance of OOo then the lock file will prevent this.
                            OUString aUserName;
                            ::osl::Security aSecurity;
                            aSecurity.getUserName( aUserName );
                            ucbhelper::Content remFileContent(
                                url + "removed", Reference<XCommandEnvironment>(), m_xComponentContext);
                            std::vector<sal_Int8> data = dp_misc::readFile(remFileContent);
                            OString osData(reinterpret_cast<const sal_Char*>(data.data()),
                                                  data.size());
                            OUString sData = OStringToOUString(
                                osData, RTL_TEXTENCODING_UTF8);
                            if (sData != aUserName)
                                continue;
                        }
                    }
                    // temp entry not needed anymore:
                    erase_path( url + "_",
                                Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    erase_path( url, Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    //delete the xxx.tmpremoved file
                    erase_path(url + "removed",
                               Reference<XCommandEnvironment>(), false);
                }
            }
        }
    }
}


void PackageManagerImpl::initRegistryBackends()
{
    if (!m_registryCache.isEmpty())
        create_folder( nullptr, m_registryCache,
                       Reference<XCommandEnvironment>(), false);
    m_xRegistry.set( ::dp_registry::create(
                         m_context, m_registryCache,
                         m_xComponentContext ) );
}

namespace {

osl::FileBase::RC createDirectory(OUString const & url) {
    auto e = osl::Directory::create(url);
    if (e != osl::FileBase::E_NOENT) {
        return e;
    }
    INetURLObject o(url);
    if (!o.removeSegment()) {
        return osl::FileBase::E_INVAL; // anything but E_None/E_EXIST
    }
    e = createDirectory(o.GetMainURL(INetURLObject::DecodeMechanism::NONE));
    if (e != osl::FileBase::E_None && e != osl::FileBase::E_EXIST) {
        return e;
    }
    return osl::Directory::create(url);
}

bool isMacroURLReadOnly( const OUString &rMacro )
{
    OUString aDirURL( rMacro );
    ::rtl::Bootstrap::expandMacros( aDirURL );

    ::osl::FileBase::RC aErr = createDirectory( aDirURL );
    if ( aErr == ::osl::FileBase::E_None )
        return false; // it will be writeable
    if ( aErr != ::osl::FileBase::E_EXIST )
        return true; // some serious problem creating it

    bool bError;
    sal_uInt64 nWritten = 0;
    OUString aFileURL( aDirURL + "/stamp.sys" );
    ::osl::File aFile( aFileURL );

    bError = aFile.open( osl_File_OpenFlag_Read |
                         osl_File_OpenFlag_Write |
                         osl_File_OpenFlag_Create ) != ::osl::FileBase::E_None;
    if (!bError)
        bError = aFile.write( "1", 1, nWritten ) != ::osl::FileBase::E_None;
    if (aFile.close() != ::osl::FileBase::E_None)
        bError = true;
    if (osl::File::remove( aFileURL ) != ::osl::FileBase::E_None)
        bError = true;

    SAL_INFO(
        "desktop.deployment",
        "local url '" << rMacro << "' -> '" << aFileURL << "' "
            << (bError ? "is" : "is not") << " readonly\n");
    return bError;
}

}

Reference<deployment::XPackageManager> PackageManagerImpl::create(
    Reference<XComponentContext> const & xComponentContext,
    OUString const & context )
{
    PackageManagerImpl * that = new PackageManagerImpl(
        xComponentContext, context );
    Reference<deployment::XPackageManager> xPackageManager( that );

    OUString logFile, stamp;
    if ( context == "user" ) {
        that->m_activePackages = "vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE/uno_packages";
        that->m_registrationData = "vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE";
        that->m_registryCache = "vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE/registry";
        logFile = "$UNO_USER_PACKAGES_CACHE/log.txt";
        //We use the extension .sys for the file because on Windows Vista a sys
        //(as well as exe and dll) file
        //will not be written in the VirtualStore. For example if the process has no
        //admin right once cannot write to the %programfiles% folder. However, when
        //virtualization is used, the file will be written into the VirtualStore and
        //it appears as if one could write to %programfiles%. When we test for write
        //access to the office/shared folder for shared extensions then this typically
        //fails because a normal user typically cannot write to this folder. However,
        //using virtualization it appears that he/she can. Then a shared extension can
        //be installed but is only visible for the user (because the extension is in
        //the virtual store).
        stamp = "$UNO_USER_PACKAGES_CACHE";
    }
    else if ( context == "shared" ) {
        that->m_activePackages = "vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE/uno_packages";
        that->m_registrationData = "vnd.sun.star.expand:$SHARED_EXTENSIONS_USER";
        that->m_registryCache = "vnd.sun.star.expand:$SHARED_EXTENSIONS_USER/registry";
        logFile = "$SHARED_EXTENSIONS_USER/log.txt";
#if !HAVE_FEATURE_READONLY_INSTALLSET
        // The "shared" extensions are read-only when we have a
        // read-only installset.
        stamp = "$UNO_SHARED_PACKAGES_CACHE";
#endif
    }
    else if ( context == "bundled" ) {
        that->m_activePackages = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS";
        that->m_registrationData = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS_USER";
        that->m_registryCache = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS_USER/registry";
        logFile = "$BUNDLED_EXTENSIONS_USER/log.txt";
        //No stamp file. We assume that bundled is always readonly. It must not be
        //modified from ExtensionManager but only by the installer
    }
    else if ( context == "tmp" ) {
        that->m_activePackages = "vnd.sun.star.expand:$TMP_EXTENSIONS/extensions";
        that->m_registrationData = "vnd.sun.star.expand:$TMP_EXTENSIONS";
        that->m_registryCache = "vnd.sun.star.expand:$TMP_EXTENSIONS/registry";
        stamp = "$TMP_EXTENSIONS";
    }
    else if (context == "bak") {
        that->m_activePackages = "vnd.sun.star.expand:$BAK_EXTENSIONS/extensions";
        that->m_registrationData = "vnd.sun.star.expand:$BAK_EXTENSIONS";
        that->m_registryCache = "vnd.sun.star.expand:$BAK_EXTENSIONS/registry";
        stamp = "$BAK_EXTENSIONS";
    }

    else if (! context.match("vnd.sun.star.tdoc:/")) {
        throw lang::IllegalArgumentException(
            "invalid context given: " + context,
            Reference<XInterface>(), static_cast<sal_Int16>(-1) );
    }

    Reference<XCommandEnvironment> xCmdEnv;

    try {
        // There is no stamp for the bundled folder:
        if (!stamp.isEmpty())
            that->m_readOnly = isMacroURLReadOnly( stamp );

        if (!that->m_readOnly && !logFile.isEmpty())
        {
            // Initialize logger which will be used in ProgressLogImpl (created below)
            rtl::Bootstrap::expandMacros(logFile);
            comphelper::EventLogger logger(xComponentContext, "unopkg");
            const Reference<XLogger> xLogger(logger.getLogger());
            Reference<XLogFormatter> xLogFormatter(SimpleTextFormatter::create(xComponentContext));
            Sequence < beans::NamedValue > aSeq2 { { "Formatter", Any(xLogFormatter) }, {"FileURL", Any(logFile)} };
            Reference<XLogHandler> xFileHandler(css::logging::FileHandler::createWithSettings(xComponentContext, aSeq2));
            xFileHandler->setLevel(LogLevel::WARNING);
            xLogger->addLogHandler(xFileHandler);

            that->m_xLogFile.set(
                that->m_xComponentContext->getServiceManager()
                ->createInstanceWithArgumentsAndContext(
                    dp_log::serviceDecl.getSupportedServiceNames()[0],
                    Sequence<Any>(),
                    that->m_xComponentContext ),
                UNO_QUERY_THROW );
            xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv, that->m_xLogFile ) );
        }

        that->initRegistryBackends();
        that->initActivationLayer( xCmdEnv );

        return xPackageManager;

    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception & e) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            ("[context=\"" + context + "\"] caught unexpected "
             + exc.getValueType().getTypeName() + ": " + e.Message),
            Reference<XInterface>(), exc );
    }
}


PackageManagerImpl::~PackageManagerImpl()
{
}


void PackageManagerImpl::fireModified()
{
    ::cppu::OInterfaceContainerHelper * pContainer = rBHelper.getContainer(
        cppu::UnoType<util::XModifyListener>::get() );
    if (pContainer != nullptr) {
        pContainer->forEach<util::XModifyListener>(
            [this] (uno::Reference<util::XModifyListener> const& xListener)
                { return xListener->modified(lang::EventObject(static_cast<OWeakObject *>(this))); });
    }
}


void PackageManagerImpl::disposing()
{
    try {
//     // xxx todo: guarding?
//     ::osl::MutexGuard guard( getMutex() );
        try_dispose( m_xLogFile );
        m_xLogFile.clear();
        try_dispose( m_xRegistry );
        m_xRegistry.clear();
        m_activePackagesDB.reset(nullptr);
        m_xComponentContext.clear();

        t_pm_helper::disposing();

    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            "caught unexpected exception while disposing...",
            static_cast<OWeakObject *>(this), exc );
    }
}

// XComponent

void PackageManagerImpl::dispose()
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::dispose();
}


void PackageManagerImpl::addEventListener(
    Reference<lang::XEventListener> const & xListener )
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::addEventListener( xListener );
}


void PackageManagerImpl::removeEventListener(
    Reference<lang::XEventListener> const & xListener )
{
    //Do not call check here. We must not throw an exception here if the object
    //is being disposed or is already disposed. See com.sun.star.lang.XComponent
    WeakComponentImplHelperBase::removeEventListener( xListener );
}

// XPackageManager

OUString PackageManagerImpl::getContext()
{
    check();
    return m_context;
}


Sequence< Reference<deployment::XPackageTypeInfo> >
PackageManagerImpl::getSupportedPackageTypes()
{
    OSL_ASSERT( m_xRegistry.is() );
    return m_xRegistry->getSupportedPackageTypes();
}


Reference<task::XAbortChannel> PackageManagerImpl::createAbortChannel()
{
    check();
    return new AbortChannel;
}

// XModifyBroadcaster

void PackageManagerImpl::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
    check();
    rBHelper.addListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}


void PackageManagerImpl::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
{
    check();
    rBHelper.removeListener( cppu::UnoType<decltype(xListener)>::get(), xListener );
}


OUString PackageManagerImpl::detectMediaType(
    ::ucbhelper::Content const & ucbContent_, bool throw_exc )
{
    ::ucbhelper::Content ucbContent(ucbContent_);
    OUString url( ucbContent.getURL() );
    OUString mediaType;
    if (url.match( "vnd.sun.star.tdoc:" ) || url.match( "vnd.sun.star.pkg:" ))
    {
        try {
            ucbContent.getPropertyValue( "MediaType" ) >>= mediaType;
        }
        catch (const beans::UnknownPropertyException &) {
        }
        OSL_ENSURE( !mediaType.isEmpty(), "### no media-type?!" );
    }
    if (mediaType.isEmpty())
    {
        try {
            Reference<deployment::XPackage> xPackage(
                m_xRegistry->bindPackage(
                    url, OUString(), false, OUString(), ucbContent.getCommandEnvironment() ) );
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
                mediaType = xPackageType->getMediaType();
        }
        catch (const lang::IllegalArgumentException &) {
            if (throw_exc)
                throw;
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "desktop", exceptionToString(ex) );
        }
    }
    return mediaType;
}


OUString PackageManagerImpl::insertToActivationLayer(
    Sequence<beans::NamedValue> const & properties,
    OUString const & mediaType, ::ucbhelper::Content const & sourceContent_,
    OUString const & title, ActivePackages::Data * dbData )
{
    ::ucbhelper::Content sourceContent(sourceContent_);
    Reference<XCommandEnvironment> xCmdEnv(
        sourceContent.getCommandEnvironment() );

    OUString baseDir(m_activePackages_expanded);
    ::utl::TempFile aTemp(&baseDir, false);
    OUString tempEntry = aTemp.GetURL();
    tempEntry = tempEntry.copy(tempEntry.lastIndexOf('/') + 1);
    OUString destFolder = makeURL( m_activePackages, tempEntry);
    destFolder += "_";

    // prepare activation folder:
    ::ucbhelper::Content destFolderContent;
    create_folder( &destFolderContent, destFolder, xCmdEnv );

    // copy content into activation temp dir:
    if (mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.package-bundle") ||
        // xxx todo: more sophisticated parsing
        mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.legacy-package-bundle"))
    {
        // inflate content:
        OUStringBuffer buf;
        if (!sourceContent.isFolder())
        {
            buf.append( "vnd.sun.star.zip://" );
            buf.append( ::rtl::Uri::encode( sourceContent.getURL(),
                                            rtl_UriCharClassRegName,
                                            rtl_UriEncodeIgnoreEscapes,
                                            RTL_TEXTENCODING_UTF8 ) );
        }
        else
        {
            //Folder. No need to unzip, just copy
            buf.append(sourceContent.getURL());
        }
        buf.append( '/' );
        sourceContent = ::ucbhelper::Content(
            buf.makeStringAndClear(), xCmdEnv, m_xComponentContext );
    }
    destFolderContent.transferContent(
            sourceContent, ::ucbhelper::InsertOperation::Copy,
            title, NameClash::OVERWRITE );


    // write to DB:
    //bundled extensions should only be added by the synchronizeAddedExtensions
    //functions. Moreover, there is no "temporary folder" for bundled extensions.
    OSL_ASSERT(!(m_context == "bundled"));
    OUString sFolderUrl = makeURLAppendSysPathSegment(destFolderContent.getURL(), title);
    DescriptionInfoset info =
        dp_misc::getDescriptionInfoset(sFolderUrl);
    dbData->temporaryName = tempEntry;
    dbData->fileName = title;
    dbData->mediaType = mediaType;
    dbData->version = info.getVersion();

    //No write the properties file next to the extension
    ExtensionProperties props(sFolderUrl, properties, xCmdEnv, m_xComponentContext);
    props.write();
    return destFolder;
}


void PackageManagerImpl::insertToActivationLayerDB(
    OUString const & id, ActivePackages::Data const & dbData )
{
    //access to the database must be guarded. See removePackage
    const ::osl::MutexGuard guard( getMutex() );
    m_activePackagesDB->put( id, dbData );
}


/* The function returns true if there is an extension with the same id already
    installed which needs to be uninstalled, before the new extension can be installed.
*/
bool PackageManagerImpl::isInstalled(
    Reference<deployment::XPackage> const & package)
{
    OUString id(dp_misc::getIdentifier(package));
    OUString fn(package->getName());
    bool bInstalled = false;
    if (m_activePackagesDB->has( id, fn ))
    {
        bInstalled = true;
    }
    return bInstalled;
}

// XPackageManager

Reference<deployment::XPackage> PackageManagerImpl::importExtension(
    Reference<deployment::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    return addPackage(extension->getURL(), Sequence<beans::NamedValue>(),
                      OUString(), xAbortChannel, xCmdEnv_);
}

/* The function adds an extension but does not register it!!!
    It may not do any user interaction. This is done in XExtensionManager::addExtension
*/
Reference<deployment::XPackage> PackageManagerImpl::addPackage(
    OUString const & url,
    css::uno::Sequence<css::beans::NamedValue> const & properties,
    OUString const & mediaType_,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    check();
    if (m_readOnly)
    {
        OUString message;
        if (m_context == "shared")
            message = "You need write permissions to install a shared extension!";
        else
            message = "You need write permissions to install this extension!";
        throw deployment::DeploymentException(
            message, static_cast<OWeakObject *>(this), Any() );
    }
    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        ::ucbhelper::Content sourceContent;
        (void)create_ucb_content( &sourceContent, url, xCmdEnv ); // throws exc
        const OUString title( StrTitle::getTitle( sourceContent ) );
        const OUString title_enc( ::rtl::Uri::encode(
                                      title, rtl_UriCharClassPchar,
                                      rtl_UriEncodeIgnoreEscapes,
                                      RTL_TEXTENCODING_UTF8 ) );
        OUString destFolder;

        OUString mediaType(mediaType_);
        if (mediaType.isEmpty())
            mediaType = detectMediaType( sourceContent );

        Reference<deployment::XPackage> xPackage;
        // copy file:
        progressUpdate(
            DpResId(RID_STR_COPYING_PACKAGE) + title, xCmdEnv );
        if (m_activePackages.isEmpty())
        {
            ::ucbhelper::Content docFolderContent;
            create_folder( &docFolderContent, m_context, xCmdEnv );
            // copy into document, first:
            docFolderContent.transferContent(
                    sourceContent, ::ucbhelper::InsertOperation::Copy,
                    OUString(),
                    NameClash::ASK /* xxx todo: ASK not needed? */);
            // set media-type:
            ::ucbhelper::Content docContent(
                makeURL( m_context, title_enc ), xCmdEnv, m_xComponentContext );
                //TODO #i73136#: using title instead of id can lead to
                // clashes, but the whole m_activePackages.getLength()==0
                // case (i.e., document-relative deployment) currently does
                // not work, anyway.
            docContent.setPropertyValue("MediaType", Any(mediaType) );

            // xxx todo: obsolete in the future
            try {
                docFolderContent.executeCommand( "flush", Any() );
            }
            catch (const UnsupportedCommandException &) {
            }
        }
        ActivePackages::Data dbData;
        destFolder = insertToActivationLayer(
            properties, mediaType, sourceContent, title, &dbData );


        // bind activation package:
        //Because every shared/user extension will be unpacked in a folder,
        //which was created with a unique name we will always have two different
        //XPackage objects, even if the second extension is the same.
        //Therefore bindPackage does not need a guard here.
        xPackage = m_xRegistry->bindPackage(
            makeURL( destFolder, title_enc ), mediaType, false, OUString(), xCmdEnv );

        OSL_ASSERT( xPackage.is() );
        if (xPackage.is())
        {
            bool install = false;
            try
            {
                OUString const id = dp_misc::getIdentifier( xPackage );

                ::osl::MutexGuard g(m_addMutex);
                if (isInstalled(xPackage))
                {
                    //Do not guard the complete function with the getMutex
                    removePackage(id, xPackage->getName(), xAbortChannel,
                                  xCmdEnv);
                }
                install = true;
                insertToActivationLayerDB(id, dbData);
            }
            catch (...)
            {
                deletePackageFromCache( xPackage, destFolder );
                throw;
            }
            if (!install)
            {
                deletePackageFromCache( xPackage, destFolder );
            }
            //ToDo: We should notify only if the extension is registered
            fireModified();
        }
        return xPackage;
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            DpResId(RID_STR_ERROR_WHILE_ADDING) + url,
            static_cast<OWeakObject *>(this), exc );
    }
}
void PackageManagerImpl::deletePackageFromCache(
    Reference<deployment::XPackage> const & xPackage,
    OUString const & destFolder)
{
    try_dispose( xPackage );

    //we remove the package from the uno cache
    //no service from the package may be loaded at this time!!!
    erase_path( destFolder, Reference<XCommandEnvironment>(),
        false /* no throw: ignore errors */ );
    //rm last character '_'
    OUString url = destFolder.copy(0, destFolder.getLength() - 1);
    erase_path( url, Reference<XCommandEnvironment>(),
        false /* no throw: ignore errors */ );

}

void PackageManagerImpl::removePackage(
    OUString const & id, OUString const & fileName,
    Reference<task::XAbortChannel> const & /*xAbortChannel*/,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    check();

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        Reference<deployment::XPackage> xPackage;
        {
            const ::osl::MutexGuard guard(getMutex());
            //Check if this extension exist and throw an IllegalArgumentException
            //if it does not
            //If the files of the extension are already removed, or there is a
            //different extension at the same place, for example after updating the
            //extension, then the returned object is that which uses the database data.
            xPackage = getDeployedPackage_(id, fileName, xCmdEnv );


            //Because the extension is only removed the next time the extension
            //manager runs after restarting OOo, we need to indicate that a
            //shared extension was "deleted". When a user starts OOo, then it
            //will check if something changed in the shared repository. Based on
            //the flag file it will then recognize, that the extension was
            //deleted and can then update the extension database of the shared
            //extensions in the user installation.
            if ( xPackage.is() && !m_readOnly && !xPackage->isRemoved() && (m_context == "shared"))
            {
                ActivePackages::Data val;
                m_activePackagesDB->get( & val, id, fileName);
                OSL_ASSERT(!val.temporaryName.isEmpty());
                OUString url(makeURL(m_activePackages_expanded,
                                     val.temporaryName + "removed"));
                ::ucbhelper::Content contentRemoved(url, xCmdEnv, m_xComponentContext);
                OUString aUserName;
                ::osl::Security aSecurity;
                aSecurity.getUserName( aUserName );

                OString stamp = OUStringToOString(aUserName, RTL_TEXTENCODING_UTF8);
                Reference<css::io::XInputStream> xData(
                    ::xmlscript::createInputStream(
                            reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                            stamp.getLength() ) );
                contentRemoved.writeStream( xData, true /* replace existing */ );
            }
            m_activePackagesDB->erase( id, fileName ); // to be removed upon next start
            //remove any cached data hold by the backend
            m_xRegistry->packageRemoved(xPackage->getURL(), xPackage->getPackageType()->getMediaType());
        }
        try_dispose( xPackage );

        fireModified();
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            DpResId(RID_STR_ERROR_WHILE_REMOVING) + id,
            static_cast<OWeakObject *>(this), exc );
    }
}


OUString PackageManagerImpl::getDeployPath( ActivePackages::Data const & data )
{
    OUStringBuffer buf;
    buf.append( data.temporaryName );
    //The bundled extensions are not contained in an additional folder
    //with a unique name. data.temporaryName contains already the
    //UTF8 encoded folder name. See PackageManagerImpl::synchronize
    if (m_context != "bundled")
    {
        buf.append( "_/" );
        buf.append( ::rtl::Uri::encode( data.fileName, rtl_UriCharClassPchar,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) );
    }
    return makeURL( m_activePackages, buf.makeStringAndClear() );
}


Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage_(
    OUString const & id, OUString const & fileName,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ActivePackages::Data val;
    if (m_activePackagesDB->get( &val, id, fileName ))
    {
        return getDeployedPackage_( id, val, xCmdEnv );
    }
    throw lang::IllegalArgumentException(
        DpResId(RID_STR_NO_SUCH_PACKAGE) + id,
        static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
}


Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage_(
    OUString const & id, ActivePackages::Data const & data,
    Reference<XCommandEnvironment> const & xCmdEnv, bool ignoreAlienPlatforms )
{
    if (ignoreAlienPlatforms)
    {
        OUString type, subType;
        INetContentTypeParameterList params;
        if (INetContentTypes::parse( data.mediaType, type, subType, &params ))
        {
            auto const iter = params.find(OString("platform"));
            if (iter != params.end() && !platform_fits(iter->second.m_sValue))
                throw lang::IllegalArgumentException(
                    DpResId(RID_STR_NO_SUCH_PACKAGE) + id,
                    static_cast<OWeakObject *>(this),
                    static_cast<sal_Int16>(-1) );
        }
    }
    Reference<deployment::XPackage> xExtension;
    try
    {
        //Ignore extensions where XPackage::checkPrerequisites failed.
        //They must not be usable for this user.
        if (data.failedPrerequisites == "0")
        {
            xExtension = m_xRegistry->bindPackage(
                getDeployPath( data ), data.mediaType, false, OUString(), xCmdEnv );
        }
    }
    catch (const deployment::InvalidRemovedParameterException& e)
    {
        xExtension = e.Extension;
    }
    return xExtension;
}


Sequence< Reference<deployment::XPackage> >
PackageManagerImpl::getDeployedPackages_(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    std::vector< Reference<deployment::XPackage> > packages;
    ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );
    for (auto const& elem : id2temp)
    {
        if (elem.second.failedPrerequisites != "0")
            continue;
        try {
            packages.push_back(
                getDeployedPackage_(
                    elem.first, elem.second, xCmdEnv,
                    true /* xxx todo: think of GUI:
                            ignore other platforms than the current one */ ) );
        }
        catch (const lang::IllegalArgumentException &) {
            css::uno::Any ex( cppu::getCaughtException() );
            // ignore
            SAL_WARN( "desktop", exceptionToString(ex) );
        }
        catch (const deployment::DeploymentException&) {
            css::uno::Any ex( cppu::getCaughtException() );
            // ignore
            SAL_WARN( "desktop", exceptionToString(ex) );
        }
    }
    return comphelper::containerToSequence(packages);
}


Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage(
    OUString const & id, OUString const & fileName,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    check();
    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        const ::osl::MutexGuard guard( getMutex() );
        return getDeployedPackage_( id, fileName, xCmdEnv );
    }
    catch (const lang::IllegalArgumentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            // ought never occur...
            "error while accessing deployed package: " + id,
            static_cast<OWeakObject *>(this), exc );
    }
}


Sequence< Reference<deployment::XPackage> >
PackageManagerImpl::getDeployedPackages(
    Reference<task::XAbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    check();
    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        const ::osl::MutexGuard guard( getMutex() );
        return getDeployedPackages_( xCmdEnv );
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            // ought never occur...
            "error while getting all deployed packages: " + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}


//ToDo: the function must not call registerPackage, do this in
//XExtensionManager.reinstallDeployedExtensions
void PackageManagerImpl::reinstallDeployedPackages(
    sal_Bool force, Reference<task::XAbortChannel> const &  /*xAbortChannel*/,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
{
    check();
    if (!force && office_is_running())
        throw RuntimeException(
            "You must close any running Office process before reinstalling packages!",
            static_cast<OWeakObject *>(this) );

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        ProgressLevel progress(
            xCmdEnv, "Reinstalling all deployed packages..." );

        try_dispose( m_xRegistry );
        m_xRegistry.clear();
        if (!m_registryCache.isEmpty())
            erase_path( m_registryCache, xCmdEnv );
        initRegistryBackends();
        Reference<util::XUpdatable> xUpdatable( m_xRegistry, UNO_QUERY );
        if (xUpdatable.is())
            xUpdatable->update();

        //registering is done by the ExtensionManager service.
    }
    catch (const RuntimeException &) {
        throw;
    }
    catch (const CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (const Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            "Error while reinstalling all previously deployed packages of context " + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}


sal_Bool SAL_CALL PackageManagerImpl::isReadOnly(  )
{
    return m_readOnly;
}
bool PackageManagerImpl::synchronizeRemovedExtensions(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
{

    //find all which are in the extension data base but which
    //are removed already.
    OSL_ASSERT(!(m_context == "user"));
    bool bModified = false;
    ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

    bool bShared = (m_context == "shared");

    for (auto const& elem : id2temp)
    {
        try
        {
            //Get the URL to the extensions folder, first make the url for the
            //shared repository including the temporary name
            OUString url = makeURL(m_activePackages, elem.second.temporaryName);
            if (bShared)
                url = makeURLAppendSysPathSegment( url + "_", elem.second.fileName);

            bool bRemoved = false;
            //Check if the URL to the extension is still the same
            ::ucbhelper::Content contentExtension;

            if (!create_ucb_content(
                    &contentExtension, url,
                    Reference<XCommandEnvironment>(), false))
            {
                bRemoved = true;
            }

            //The folder is in the extension database, but it can still be deleted.
            //look for the xxx.tmpremoved file
            //There can also be the case that a different extension was installed
            //in a "temp" folder with name that is already used.
            if (!bRemoved && bShared)
            {
                ::ucbhelper::Content contentRemoved;

                if (create_ucb_content(
                        &contentRemoved,
                        m_activePackages_expanded + "/" +
                        elem.second.temporaryName + "removed",
                        Reference<XCommandEnvironment>(), false))
                {
                    bRemoved = true;
                }
            }

            if (!bRemoved)
            {
                //There may be another extensions at the same place
                dp_misc::DescriptionInfoset infoset =
                    dp_misc::getDescriptionInfoset(url);
                OSL_ENSURE(infoset.hasDescription() && infoset.getIdentifier(),
                           "Extension Manager: bundled and shared extensions "
                           "must have an identifier and a version");
                if (infoset.hasDescription() &&
                    infoset.getIdentifier() &&
                    ( elem.first != *(infoset.getIdentifier())
                      || elem.second.version != infoset.getVersion()))
                {
                    bRemoved = true;
                }

            }
            if (bRemoved)
            {
                Reference<deployment::XPackage> xPackage = m_xRegistry->bindPackage(
                    url, elem.second.mediaType, true, elem.first, xCmdEnv );
                OSL_ASSERT(xPackage.is()); //Even if the files are removed, we must get the object.
                xPackage->revokePackage(true, xAbortChannel, xCmdEnv);
                removePackage(xPackage->getIdentifier().Value, xPackage->getName(),
                              xAbortChannel, xCmdEnv);
                bModified = true;
            }
        }
        catch( const uno::Exception & e )
        {
            SAL_WARN("desktop.deployment", e);
        }
    }
    return bModified;
}


bool PackageManagerImpl::synchronizeAddedExtensions(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
{
    bool bModified = false;
    OSL_ASSERT(!(m_context == "user"));

    ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );
    //check if the folder exist at all. The shared extension folder
    //may not exist for a normal user.
    bool bOk=true;
    try
    {
        bOk = create_ucb_content(
                nullptr, m_activePackages_expanded, Reference<css::ucb::XCommandEnvironment>(), false);
    }
    catch (const css::ucb::ContentCreationException&)
    {
        bOk = false;
    }

    if (!bOk)
        return bModified;

    ::ucbhelper::Content tempFolder( m_activePackages_expanded, xCmdEnv, m_xComponentContext );
    Reference<sdbc::XResultSet> xResultSet(
        StrTitle::createCursor( tempFolder,
                                ::ucbhelper::INCLUDE_FOLDERS_ONLY ) );

    while (xResultSet->next())
    {
        try
        {
            OUString title(
                Reference<sdbc::XRow>(
                    xResultSet, UNO_QUERY_THROW )->getString(
                        1 /* Title */ ) );
            //The temporary folders of user and shared have an '_' at then end.
            //But the name in ActivePackages.temporaryName is saved without.
            OUString title2 = title;
            bool bShared = (m_context == "shared");
            if (bShared)
            {
                OSL_ASSERT(title2.endsWith("_"));
                title2 = title2.copy(0, title2.getLength() -1);
            }
            OUString titleEncoded =  ::rtl::Uri::encode(
                title2, rtl_UriCharClassPchar,
                rtl_UriEncodeIgnoreEscapes,
                RTL_TEXTENCODING_UTF8);

            //It is sufficient to check for the folder name, because when the administrator
            //installed the extension it was already checked if there is one with the
            //same identifier.
            const MatchTempDir match(titleEncoded);
            if (std::none_of( id2temp.begin(), id2temp.end(), match ))
            {

                // The folder was not found in the data base, so it must be
                // an added extension
                OUString url(m_activePackages_expanded + "/" + titleEncoded);
                OUString sExtFolder;
                if (bShared) //that is, shared
                {
                    //Check if the extension was not "deleted" already which is indicated
                    //by a xxx.tmpremoved file
                    ::ucbhelper::Content contentRemoved;
                    if (create_ucb_content(&contentRemoved, url + "removed",
                                           Reference<XCommandEnvironment>(), false))
                        continue;
                    sExtFolder = getExtensionFolder(
                        m_activePackages_expanded + "/" + titleEncoded + "_",
                        xCmdEnv, m_xComponentContext);
                    url = makeURLAppendSysPathSegment(m_activePackages_expanded, title);
                    url = makeURLAppendSysPathSegment(url, sExtFolder);
                }
                Reference<deployment::XPackage> xPackage = m_xRegistry->bindPackage(
                    url, OUString(), false, OUString(), xCmdEnv );
                if (xPackage.is())
                {
                    OUString id = dp_misc::getIdentifier( xPackage );

                    //Prepare the database entry
                    ActivePackages::Data dbData;

                    dbData.temporaryName = titleEncoded;
                    if (bShared)
                        dbData.fileName = sExtFolder;
                    else
                        dbData.fileName = title;
                    dbData.mediaType = xPackage->getPackageType()->getMediaType();
                    dbData.version = xPackage->getVersion();
                    SAL_WARN_IF(
                        dbData.version.isEmpty(), "desktop.deployment",
                        "bundled/shared extension " << id << " at <" << url
                            << "> has no explicit version");

                    //We provide a special command environment that will prevent
                    //showing a license if simple-license/@accept-by = "admin"
                    //It will also prevent showing the license for bundled extensions
                    //which is not supported.
                    OSL_ASSERT(!(m_context == "user"));

                    // shall the license be suppressed?
                    DescriptionInfoset info =
                        dp_misc::getDescriptionInfoset(url);
                    ::boost::optional<dp_misc::SimpleLicenseAttributes>
                          attr = info.getSimpleLicenseAttributes();
                    ExtensionProperties props(url, xCmdEnv, m_xComponentContext);
                    bool bNoLicense = false;
                    if (attr && attr->suppressIfRequired && props.isSuppressedLicense())
                        bNoLicense = true;

                    Reference<ucb::XCommandEnvironment> licCmdEnv(
                        new LicenseCommandEnv(xCmdEnv->getInteractionHandler(),
                                              bNoLicense, m_context));
                    sal_Int32 failedPrereq = xPackage->checkPrerequisites(
                        xAbortChannel, licCmdEnv, false);
                    //Remember that this failed. For example, the user
                    //could have declined the license. Then the next time the
                    //extension folder is investigated we do not want to
                    //try to install the extension again.
                    dbData.failedPrerequisites = OUString::number(failedPrereq);
                    insertToActivationLayerDB(id, dbData);
                    bModified = true;
                }
            }
        }
        catch (const uno::Exception & e)
        {
            // Looks like exceptions being caught here is not an uncommon case.
            SAL_WARN("desktop.deployment", e);
        }
    }
    return bModified;
}

sal_Bool PackageManagerImpl::synchronize(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
{
    check();
    bool bModified = false;
    if (m_context == "user")
        return bModified;
    bModified |=
        synchronizeRemovedExtensions(xAbortChannel, xCmdEnv);
    bModified |= synchronizeAddedExtensions(xAbortChannel, xCmdEnv);

    return bModified;
}

Sequence< Reference<deployment::XPackage> > PackageManagerImpl::getExtensionsWithUnacceptedLicenses(
    Reference<ucb::XCommandEnvironment> const & xCmdEnv)
{
    std::vector<Reference<deployment::XPackage> > vec;

    try
    {
        const ::osl::MutexGuard guard( getMutex() );
        // clean up activation layer, scan for zombie temp dirs:
        ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

        bool bShared = (m_context == "shared");

        for (auto const& elem : id2temp)
        {
            //Get the database entry
            ActivePackages::Data const & dbData = elem.second;
            sal_Int32 failedPrereq = dbData.failedPrerequisites.toInt32();
            //If the installation failed for other reason then the license then we
            //ignore it.
            if (failedPrereq ^= deployment::Prerequisites::LICENSE)
                continue;

            //Prepare the URL to the extension
            OUString url = makeURL(m_activePackages, elem.second.temporaryName);
            if (bShared)
                url = makeURLAppendSysPathSegment( url + "_", elem.second.fileName);

            Reference<deployment::XPackage> p = m_xRegistry->bindPackage(
                url, OUString(), false, OUString(), xCmdEnv );

            if (p.is())
                vec.push_back(p);

        }
        return ::comphelper::containerToSequence(vec);
    }
    catch (const deployment::DeploymentException &)
    {
        throw;
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (...)
    {
        Any exc = ::cppu::getCaughtException();
        deployment::DeploymentException de(
            "PackageManagerImpl::getExtensionsWithUnacceptedLicenses",
            static_cast<OWeakObject*>(this), exc);
        exc <<= de;
        ::cppu::throwException(exc);
    }

    return ::comphelper::containerToSequence(vec);
}

sal_Int32 PackageManagerImpl::checkPrerequisites(
    css::uno::Reference<css::deployment::XPackage> const & extension,
    css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
{
    try
    {
        if (!extension.is())
            return 0;
        if (m_context != extension->getRepositoryName())
            throw lang::IllegalArgumentException(
                "PackageManagerImpl::checkPrerequisites: extension is not from this repository.",
                nullptr, 0);

        ActivePackages::Data dbData;
        OUString id = dp_misc::getIdentifier(extension);
        if (!m_activePackagesDB->get( &dbData, id, OUString()))
        {
            throw lang::IllegalArgumentException(
                "PackageManagerImpl::checkPrerequisites: unknown extension",
                nullptr, 0);

        }
        //If the license was already displayed, then do not show it again
        Reference<ucb::XCommandEnvironment> _xCmdEnv = xCmdEnv;
        sal_Int32 prereq = dbData.failedPrerequisites.toInt32();
        if ( !(prereq & deployment::Prerequisites::LICENSE))
            _xCmdEnv = new NoLicenseCommandEnv(xCmdEnv->getInteractionHandler());

        sal_Int32 failedPrereq = extension->checkPrerequisites(
            xAbortChannel, _xCmdEnv, false);
        dbData.failedPrerequisites = OUString::number(failedPrereq);
        insertToActivationLayerDB(id, dbData);
        return 0;
    }
    catch ( const deployment::DeploymentException& ) {
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
        deployment::DeploymentException exc(
            "PackageManagerImpl::checkPrerequisites: exception ",
            static_cast<OWeakObject*>(this), excOccurred);
        throw exc;
    }
}


PackageManagerImpl::CmdEnvWrapperImpl::~CmdEnvWrapperImpl()
{
}


PackageManagerImpl::CmdEnvWrapperImpl::CmdEnvWrapperImpl(
    Reference<XCommandEnvironment> const & xUserCmdEnv,
    Reference<XProgressHandler> const & xLogFile )
    : m_xLogFile( xLogFile )
{
    if (xUserCmdEnv.is()) {
        m_xUserProgress.set( xUserCmdEnv->getProgressHandler() );
        m_xUserInteractionHandler.set( xUserCmdEnv->getInteractionHandler() );
    }
}

// XCommandEnvironment

Reference<task::XInteractionHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getInteractionHandler()
{
    return m_xUserInteractionHandler;
}


Reference<XProgressHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getProgressHandler()
{
    return this;
}

// XProgressHandler

void PackageManagerImpl::CmdEnvWrapperImpl::push( Any const & Status )
{
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->push( Status );
}


void PackageManagerImpl::CmdEnvWrapperImpl::update( Any const & Status )
{
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->update( Status );
}


void PackageManagerImpl::CmdEnvWrapperImpl::pop()
{
    if (m_xLogFile.is())
        m_xLogFile->pop();
    if (m_xUserProgress.is())
        m_xUserProgress->pop();
}

} // namespace dp_manager

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
