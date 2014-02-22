/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>

#include "dp_ucb.h"
#include "dp_resource.h"
#include "dp_platform.hxx"
#include "dp_manager.h"
#include "dp_identifier.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/uri.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/security.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/sequence.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svl/inettype.hxx"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/util/XUpdatable.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/deployment/Prerequisites.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "boost/bind.hpp"
#include "unotools/tempfile.hxx"

#include <vector>
#include <list>
#include "dp_descriptioninfoset.hxx"
#include "dp_commandenvironments.hxx"
#include "dp_properties.hxx"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_log {
extern comphelper::service_decl::ServiceDecl const serviceDecl;
}

namespace dp_registry {
Reference<deployment::XPackageRegistry> create(
    OUString const & context,
    OUString const & cachePath, bool readOnly,
    Reference<XComponentContext> const & xComponentContext );
}

namespace dp_manager {

struct MatchTempDir
{
    OUString m_str;
    MatchTempDir( OUString const & str ) : m_str( str ) {}
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
    while (xResultSet->next())
    {
        title = Reference<sdbc::XRow>(
            xResultSet, UNO_QUERY_THROW )->getString(1 /* Title */ ) ;
        break;
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
        
        m_activePackagesDB.reset( new ActivePackages );
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, m_context, xCmdEnv,
                                false /* no throw */ ))
        {
            
            Reference<sdbc::XResultSet> xResultSet(
                        StrTitle::createCursor (ucbContent, ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS ) );

            while (xResultSet->next())
            {
                Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
                OUString title( xRow->getString( 1 /* Title */ ) );
                
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
                        
                        
                        
                        
                }
            }
        }
    }
    else
    {
        
        OSL_ASSERT( !m_activePackages.isEmpty() );
        m_activePackages_expanded = expandUnoRcUrl( m_activePackages );
        m_registrationData_expanded = expandUnoRcUrl(m_registrationData);
        if (!m_readOnly)
            create_folder( 0, m_activePackages_expanded, xCmdEnv, true);

        OUString dbName;
        if (m_context == "user")
            dbName = m_activePackages_expanded + ".pmap";
        else
        {
            
            create_folder( 0, m_registrationData_expanded, xCmdEnv, true);
            dbName = m_registrationData_expanded + "/extensions.pmap";
        }
        
        m_activePackagesDB.reset( new ActivePackages( dbName, false ) );

        if (! m_readOnly && ! (m_context == "bundled"))
        {
            
            ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

            ::ucbhelper::Content tempFolder( m_activePackages_expanded, xCmdEnv, m_xComponentContext );
            Reference<sdbc::XResultSet> xResultSet(
                StrTitle::createCursor (tempFolder,
                                         ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );

            
            ::std::vector<OUString> tempEntries;
            ::std::vector<OUString> removedEntries;
            while (xResultSet->next())
            {
                OUString title(
                    Reference<sdbc::XRow>(
                        xResultSet, UNO_QUERY_THROW )->getString(
                            1 /* Title */ ) );

                const char extensionRemoved[] = "removed";
                if (title.endsWith( extensionRemoved ))
                {
                    
                    sal_Int32 index = title.lastIndexOfAsciiL(
                        extensionRemoved, sizeof(extensionRemoved) - 1);
                    OUString remFile = title.copy(0, index);
                    removedEntries.push_back(::rtl::Uri::encode(
                                                remFile, rtl_UriCharClassPchar,
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

            bool bShared = (m_context == "shared") ? true : false;
            for ( ::std::size_t pos = 0; pos < tempEntries.size(); ++pos )
            {
                OUString const & tempEntry = tempEntries[ pos ];
                const MatchTempDir match( tempEntry );
                if (::std::find_if( id2temp.begin(), id2temp.end(), match ) ==
                    id2temp.end())
                {
                    const OUString url(
                        makeURL(m_activePackages_expanded, tempEntry ) );

                    
                    
                    if (bShared)
                    {
                        if (::std::find(removedEntries.begin(), removedEntries.end(), tempEntry) ==
                            removedEntries.end())
                        {
                            continue;
                        }
                        else
                        {
                            
                            
                            
                            
                            
                            
                            
                            
                            
                            OUString aUserName;
                            ::osl::Security aSecurity;
                            aSecurity.getUserName( aUserName );
                            ucbhelper::Content remFileContent(
                                url + "removed", Reference<XCommandEnvironment>(), m_xComponentContext);
                            ::rtl::ByteSequence data = dp_misc::readFile(remFileContent);
                            OString osData(reinterpret_cast<const sal_Char*>(data.getConstArray()),
                                                  data.getLength());
                            OUString sData = OStringToOUString(
                                osData, RTL_TEXTENCODING_UTF8);
                            if (!sData.equals(aUserName))
                                continue;
                        }
                    }
                    
                    erase_path( url + "_",
                                Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    erase_path( url, Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    
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
        create_folder( 0, m_registryCache,
                       Reference<XCommandEnvironment>(), false);
    m_xRegistry.set( ::dp_registry::create(
                         m_context, m_registryCache, false,
                         m_xComponentContext ) );
}



static bool isMacroURLReadOnly( const OUString &rMacro )
{
    OUString aDirURL( rMacro );
    ::rtl::Bootstrap::expandMacros( aDirURL );

    ::osl::FileBase::RC aErr = ::osl::Directory::create( aDirURL );
    if ( aErr == ::osl::FileBase::E_None )
        return false; 
    if ( aErr != ::osl::FileBase::E_EXIST )
        return true; 

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
        logFile = "vnd.sun.star.expand:$UNO_USER_PACKAGES_CACHE/log.txt";
        
        
        
        
        
        
        
        
        
        
        
        stamp = "$UNO_USER_PACKAGES_CACHE";
    }
    else if ( context == "shared" ) {
        that->m_activePackages = "vnd.sun.star.expand:$UNO_SHARED_PACKAGES_CACHE/uno_packages";
        that->m_registrationData = "vnd.sun.star.expand:$SHARED_EXTENSIONS_USER";
        that->m_registryCache = "vnd.sun.star.expand:$SHARED_EXTENSIONS_USER/registry";
        logFile = "vnd.sun.star.expand:$SHARED_EXTENSIONS_USER/log.txt";
#if !HAVE_FEATURE_READONLY_INSTALLSET
        
        
        stamp = "$UNO_SHARED_PACKAGES_CACHE";
#endif
    }
    else if ( context == "bundled" ) {
        that->m_activePackages = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS";
        that->m_registrationData = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS_USER";
        that->m_registryCache = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS_USER/registry";
        logFile = "vnd.sun.star.expand:$BUNDLED_EXTENSIONS_USER/log.txt";
        
        
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
        
        if (!stamp.isEmpty())
            that->m_readOnly = isMacroURLReadOnly( stamp );

        if (!that->m_readOnly && !logFile.isEmpty())
        {
            const Any any_logFile(logFile);
            that->m_xLogFile.set(
                that->m_xComponentContext->getServiceManager()
                ->createInstanceWithArgumentsAndContext(
                    dp_log::serviceDecl.getSupportedServiceNames()[0],
                    Sequence<Any>( &any_logFile, 1 ),
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
    if (pContainer != 0) {
        pContainer->forEach<util::XModifyListener>(
            boost::bind(&util::XModifyListener::modified, _1,
                        lang::EventObject(static_cast<OWeakObject *>(this))) );
    }
}


void PackageManagerImpl::disposing()
{
    try {


        try_dispose( m_xLogFile );
        m_xLogFile.clear();
        try_dispose( m_xRegistry );
        m_xRegistry.clear();
        m_activePackagesDB.reset(0);
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



void PackageManagerImpl::dispose() throw (RuntimeException)
{
    
    
    WeakComponentImplHelperBase::dispose();
}


void PackageManagerImpl::addEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    
    
    WeakComponentImplHelperBase::addEventListener( xListener );
}


void PackageManagerImpl::removeEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    
    
    WeakComponentImplHelperBase::removeEventListener( xListener );
}



OUString PackageManagerImpl::getContext() throw (RuntimeException)
{
    check();
    return m_context;
}


Sequence< Reference<deployment::XPackageTypeInfo> >
PackageManagerImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    OSL_ASSERT( m_xRegistry.is() );
    return m_xRegistry->getSupportedPackageTypes();
}


Reference<task::XAbortChannel> PackageManagerImpl::createAbortChannel()
    throw (RuntimeException)
{
    check();
    return new AbortChannel;
}



void PackageManagerImpl::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.addListener( ::getCppuType( &xListener ), xListener );
}


void PackageManagerImpl::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
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
        catch (const lang::IllegalArgumentException & exc) {
            if (throw_exc)
                throw;
            (void) exc;
            OSL_FAIL( OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
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

    
    ::ucbhelper::Content destFolderContent;
    create_folder( &destFolderContent, destFolder, xCmdEnv );

    
    if (mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.package-bundle") ||
        
        mediaType.matchIgnoreAsciiCase("application/vnd.sun.star.legacy-package-bundle"))
    {
        
        OUStringBuffer buf;
        if (!sourceContent.isFolder())
        {
            buf.appendAscii( "vnd.sun.star.zip:
            buf.append( ::rtl::Uri::encode( sourceContent.getURL(),
                                            rtl_UriCharClassRegName,
                                            rtl_UriEncodeIgnoreEscapes,
                                            RTL_TEXTENCODING_UTF8 ) );
        }
        else
        {
            
            buf.append(sourceContent.getURL());
        }
        buf.append( '/' );
        sourceContent = ::ucbhelper::Content(
            buf.makeStringAndClear(), xCmdEnv, m_xComponentContext );
    }
    if (! destFolderContent.transferContent(
            sourceContent, ::ucbhelper::InsertOperation_COPY,
            title, NameClash::OVERWRITE ))
        throw RuntimeException( "UCB transferContent() failed!", 0 );


    
    
    
    OSL_ASSERT(!(m_context == "bundled"));
    OUString sFolderUrl = makeURLAppendSysPathSegment(destFolderContent.getURL(), title);
    DescriptionInfoset info =
        dp_misc::getDescriptionInfoset(sFolderUrl);
    dbData->temporaryName = tempEntry;
    dbData->fileName = title;
    dbData->mediaType = mediaType;
    dbData->version = info.getVersion();

    
    ExtensionProperties props(sFolderUrl, properties, xCmdEnv, m_xComponentContext);
    props.write();
    return destFolder;
}


void PackageManagerImpl::insertToActivationLayerDB(
    OUString const & id, ActivePackages::Data const & dbData )
{
    
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



Reference<deployment::XPackage> PackageManagerImpl::importExtension(
    Reference<deployment::XPackage> const & extension,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
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
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
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
        create_ucb_content( &sourceContent, url, xCmdEnv ); 
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
        
        progressUpdate(
            getResourceString(RID_STR_COPYING_PACKAGE) + title, xCmdEnv );
        if (m_activePackages.isEmpty())
        {
            ::ucbhelper::Content docFolderContent;
            create_folder( &docFolderContent, m_context, xCmdEnv );
            
            if (! docFolderContent.transferContent(
                    sourceContent, ::ucbhelper::InsertOperation_COPY,
                    OUString(),
                    NameClash::ASK /* xxx todo: ASK not needed? */))
                throw RuntimeException("UCB transferContent() failed!", 0 );
            
            ::ucbhelper::Content docContent(
                makeURL( m_context, title_enc ), xCmdEnv, m_xComponentContext );
                
                
                
                
            docContent.setPropertyValue("MediaType", Any(mediaType) );

            
            try {
                docFolderContent.executeCommand( "flush", Any() );
            }
            catch (const UnsupportedCommandException &) {
            }
        }
        ActivePackages::Data dbData;
        destFolder = insertToActivationLayer(
            properties, mediaType, sourceContent, title, &dbData );


        
        
        
        
        
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
            getResourceString(RID_STR_ERROR_WHILE_ADDING) + url,
            static_cast<OWeakObject *>(this), exc );
    }
}
void PackageManagerImpl::deletePackageFromCache(
    Reference<deployment::XPackage> const & xPackage,
    OUString const & destFolder)
{
    try_dispose( xPackage );

    
    
    erase_path( destFolder, Reference<XCommandEnvironment>(),
        false /* no throw: ignore errors */ );
    
    OUString url = destFolder.copy(0, destFolder.getLength() - 1);
    erase_path( url, Reference<XCommandEnvironment>(),
        false /* no throw: ignore errors */ );

}

void PackageManagerImpl::removePackage(
    OUString const & id, OUString const & fileName,
    Reference<task::XAbortChannel> const & /*xAbortChannel*/,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
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
            
            
            
            
            
            xPackage = getDeployedPackage_(id, fileName, xCmdEnv );


            
            
            
            
            
            
            
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
                        ::rtl::ByteSequence(
                            reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                            stamp.getLength() ) ) );
                contentRemoved.writeStream( xData, true /* replace existing */ );
            }
            m_activePackagesDB->erase( id, fileName ); 
            
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
            getResourceString(RID_STR_ERROR_WHILE_REMOVING) + id,
            static_cast<OWeakObject *>(this), exc );
    }
}


OUString PackageManagerImpl::getDeployPath( ActivePackages::Data const & data )
{
    OUStringBuffer buf;
    buf.append( data.temporaryName );
    
    
    
    if (!(m_context == "bundled"))
    {
        buf.appendAscii( "_/" );
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
        return getDeployedPackage_( id, val, xCmdEnv, false );
    }
    throw lang::IllegalArgumentException(
        getResourceString(RID_STR_NO_SUCH_PACKAGE) + id,
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
            INetContentTypeParameter const * param = params.find(
                OString("platform") );
            if (param != 0 && !platform_fits( param->m_sValue ))
                throw lang::IllegalArgumentException(
                    getResourceString(RID_STR_NO_SUCH_PACKAGE) + id,
                    static_cast<OWeakObject *>(this),
                    static_cast<sal_Int16>(-1) );
        }
    }
    Reference<deployment::XPackage> xExtension;
    try
    {
        
        
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
    ::std::vector< Reference<deployment::XPackage> > packages;
    ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );
    ActivePackages::Entries::const_iterator iPos( id2temp.begin() );
    ActivePackages::Entries::const_iterator const iEnd( id2temp.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        if (! (iPos->second.failedPrerequisites == "0"))
            continue;
        try {
            packages.push_back(
                getDeployedPackage_(
                    iPos->first, iPos->second, xCmdEnv,
                    true /* xxx todo: think of GUI:
                            ignore other platforms than the current one */ ) );
        }
        catch (const lang::IllegalArgumentException & exc) {
            
            (void) exc; 
            OSL_FAIL( OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
        catch (const deployment::DeploymentException& exc) {
            
            (void) exc; 
            OSL_FAIL( OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return comphelper::containerToSequence(packages);
}


Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage(
    OUString const & id, OUString const & fileName,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           lang::IllegalArgumentException, RuntimeException)
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
            
            "error while accessing deployed package: " + id,
            static_cast<OWeakObject *>(this), exc );
    }
}


Sequence< Reference<deployment::XPackage> >
PackageManagerImpl::getDeployedPackages(
    Reference<task::XAbortChannel> const &,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
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
            
            "error while getting all deployed packages: " + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}






void PackageManagerImpl::reinstallDeployedPackages(
    sal_Bool force, Reference<task::XAbortChannel> const &  /*xAbortChannel*/,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
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


::sal_Bool SAL_CALL PackageManagerImpl::isReadOnly(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return m_readOnly;
}
bool PackageManagerImpl::synchronizeRemovedExtensions(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
{

    
    
    OSL_ASSERT(!(m_context == "user"));
    bool bModified = false;
    ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

    typedef ActivePackages::Entries::const_iterator ITActive;
    bool bShared = (m_context == "shared");

    for (ITActive i = id2temp.begin(); i != id2temp.end(); ++i)
    {
        try
        {
            
            
            OUString url = makeURL(m_activePackages, i->second.temporaryName);
            if (bShared)
                url = makeURLAppendSysPathSegment( url + "_", i->second.fileName);

            bool bRemoved = false;
            
            ::ucbhelper::Content contentExtension;

            if (!create_ucb_content(
                    &contentExtension, url,
                    Reference<XCommandEnvironment>(), false))
            {
                bRemoved = true;
            }

            
            
            
            
            if (!bRemoved && bShared)
            {
                ::ucbhelper::Content contentRemoved;

                if (create_ucb_content(
                        &contentRemoved,
                        m_activePackages_expanded + "/" +
                        i->second.temporaryName + "removed",
                        Reference<XCommandEnvironment>(), false))
                {
                    bRemoved = true;
                }
            }

            if (!bRemoved)
            {
                
                dp_misc::DescriptionInfoset infoset =
                    dp_misc::getDescriptionInfoset(url);
                OSL_ENSURE(infoset.hasDescription() && infoset.getIdentifier(),
                           "Extension Manager: bundled and shared extensions "
                           "must have an identifer and a version");
                if (infoset.hasDescription() &&
                    infoset.getIdentifier() &&
                    (! i->first.equals(*(infoset.getIdentifier()))
                     || ! i->second.version.equals(infoset.getVersion())))
                {
                    bRemoved = true;
                }

            }
            if (bRemoved)
            {
                Reference<deployment::XPackage> xPackage = m_xRegistry->bindPackage(
                    url, i->second.mediaType, true, i->first, xCmdEnv );
                OSL_ASSERT(xPackage.is()); 
                xPackage->revokePackage(true, xAbortChannel, xCmdEnv);
                removePackage(xPackage->getIdentifier().Value, xPackage->getName(),
                              xAbortChannel, xCmdEnv);
                bModified |= true;
            }
        }
        catch( const uno::Exception & e )
        {
            SAL_WARN("desktop.deployment", e.Message);
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
    
    
    if (!create_ucb_content(
            NULL, m_activePackages_expanded, Reference<css::ucb::XCommandEnvironment>(), false))
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

            
            
            
            const MatchTempDir match(titleEncoded);
            if (::std::find_if( id2temp.begin(), id2temp.end(), match ) ==
                id2temp.end())
            {

                
                
                OUString url(m_activePackages_expanded + "/" + titleEncoded);
                OUString sExtFolder;
                if (bShared) 
                {
                    
                    
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

                    
                    
                    
                    
                    OSL_ASSERT(!(m_context == "user"));

                    
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
                    
                    
                    
                    
                    dbData.failedPrerequisites = OUString::number(failedPrereq);
                    insertToActivationLayerDB(id, dbData);
                    bModified |= true;
                }
            }
        }
        catch (const uno::Exception & e)
        {
            
            SAL_WARN("desktop.deployment", e.Message);
        }
    }
    return bModified;
}

sal_Bool PackageManagerImpl::synchronize(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<css::ucb::XCommandEnvironment> const & xCmdEnv)
    throw (css::deployment::DeploymentException,
           css::ucb::CommandFailedException,
           css::ucb::CommandAbortedException,
           css::uno::RuntimeException)
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
    throw (deployment::DeploymentException, RuntimeException)
{
    ::std::vector<Reference<deployment::XPackage> > vec;

    try
    {
        const ::osl::MutexGuard guard( getMutex() );
        
        ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

        ActivePackages::Entries::const_iterator i = id2temp.begin();
        bool bShared = (m_context == "shared");

        for (; i != id2temp.end(); ++i )
        {
            
            ActivePackages::Data const & dbData = i->second;
            sal_Int32 failedPrereq = dbData.failedPrerequisites.toInt32();
            
            
            if (failedPrereq ^= deployment::Prerequisites::LICENSE)
                continue;

            
            OUString url = makeURL(m_activePackages, i->second.temporaryName);
            if (bShared)
                url = makeURLAppendSysPathSegment( url + "_", i->second.fileName);

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
    throw (css::deployment::DeploymentException,
           css::ucb::CommandFailedException,
           css::ucb::CommandAbortedException,
           css::lang::IllegalArgumentException,
           css::uno::RuntimeException)
{
    try
    {
        if (!extension.is())
            return 0;
        if (!m_context.equals(extension->getRepositoryName()))
            throw lang::IllegalArgumentException(
                "PackageManagerImpl::checkPrerequisites: extension is not from this repository.",
                0, 0);

        ActivePackages::Data dbData;
        OUString id = dp_misc::getIdentifier(extension);
        if (m_activePackagesDB->get( &dbData, id, OUString()))
        {
            
            Reference<ucb::XCommandEnvironment> _xCmdEnv = xCmdEnv;
            sal_Int32 prereq = dbData.failedPrerequisites.toInt32();
            if ( !(prereq & deployment::Prerequisites::LICENSE))
                _xCmdEnv = new NoLicenseCommandEnv(xCmdEnv->getInteractionHandler());

            sal_Int32 failedPrereq = extension->checkPrerequisites(
                xAbortChannel, _xCmdEnv, false);
            dbData.failedPrerequisites = OUString::number(failedPrereq);
            insertToActivationLayerDB(id, dbData);
        }
        else
        {
            throw lang::IllegalArgumentException(
                "PackageManagerImpl::checkPrerequisites: unknown extension",
                0, 0);

        }
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



Reference<task::XInteractionHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getInteractionHandler()
    throw (RuntimeException)
{
    return m_xUserInteractionHandler;
}


Reference<XProgressHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getProgressHandler()
    throw (RuntimeException)
{
    return this;
}



void PackageManagerImpl::CmdEnvWrapperImpl::push( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->push( Status );
}


void PackageManagerImpl::CmdEnvWrapperImpl::update( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->update( Status );
}


void PackageManagerImpl::CmdEnvWrapperImpl::pop() throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->pop();
    if (m_xUserProgress.is())
        m_xUserProgress->pop();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
