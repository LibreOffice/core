/*************************************************************************
 *
 *  $RCSfile: dp_manager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:06:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "dp_ucb.h"
#include "dp_resource.h"
#include "dp_manager.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/uri.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/XInteractionReplaceExistingData.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include <vector>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

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
    bool operator () ( t_string2string_map::value_type const & v ) const {
        return v.second.copy( 0, v.second.indexOf(';') ).equalsIgnoreAsciiCase(
            m_str );
    }
};

//______________________________________________________________________________
void PackageManagerImpl::initActivationLayer(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_activePackages.getLength() == 0)
    {
        OSL_ASSERT( m_registryCache.getLength() == 0 );
        // documents temp activation:
        m_activePackagesDB.reset( new PersistentMap );
        ::ucb::Content ucbContent;
        if (create_ucb_content( &ucbContent, m_context, xCmdEnv,
                                false /* no throw */ ))
        {
            // scan for all entries in m_packagesDir:
            OUString strTitle = OUSTR("Title");
            Reference<sdbc::XResultSet> xResultSet(
                ucbContent.createCursor(
                    Sequence<OUString>( &strTitle, 1 ),
                    ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
            while (xResultSet->next())
            {
                Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
                OUString title( xRow->getString( 1 /* Title */ ) );
                // xxx todo: remove workaround for tdoc
                if (title.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                            "this_is_a_dummy_stream_just_there_"
                                            "as_a_workaround_for_a_"
                                            "temporary_limitation_of_the_"
                                            "storage_api_implementation") ))
                    continue;
                if (title.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(
                                            "META-INF") ))
                    continue;

                ::ucb::Content sourceContent(
                    Reference<XContentAccess>(
                        xResultSet, UNO_QUERY_THROW )->queryContent(),
                    xCmdEnv );

                OUString mediaType( detectMediaType( title, sourceContent,
                                                     false /* no throw */) );
                if (mediaType.getLength() > 0)
                    insertToActivationLayer( title, mediaType, sourceContent );
            }
        }
    }
    else
    {
        // user|share
        OSL_ASSERT( m_activePackages.getLength() > 0 );
        m_activePackages_expanded = expand_url( m_activePackages );
        create_folder( 0, m_activePackages_expanded, xCmdEnv );
        m_activePackagesDB.reset(
            new PersistentMap(
                m_activePackages_expanded + OUSTR(".db"), m_readOnly ) );

        if (! m_readOnly)
        {
            // clean up activation layer, scan for zombie temp dirs:
            t_string2string_map title2temp( m_activePackagesDB->getEntries() );

            OUString strTitle = OUSTR("Title");
            ::ucb::Content tempFolder( m_activePackages_expanded, xCmdEnv );
            Reference<sdbc::XResultSet> xResultSet(
                tempFolder.createCursor( Sequence<OUString>( &strTitle, 1 ),
                                         ::ucb::INCLUDE_DOCUMENTS_ONLY ) );
            // get all temp directories:
            ::std::vector<OUString> tempEntries;
            while (xResultSet->next()) {
                OUString title(
                    Reference<sdbc::XRow>(
                        xResultSet, UNO_QUERY_THROW )->getString(
                            1 /* Title */ ) );
                tempEntries.push_back( ::rtl::Uri::encode(
                                           title, rtl_UriCharClassPchar,
                                           rtl_UriEncodeIgnoreEscapes,
                                           RTL_TEXTENCODING_UTF8 ) );
            }

            for ( ::std::size_t pos = 0; pos < tempEntries.size(); ++pos )
            {
                OUString const & tempEntry = tempEntries[ pos ];
                if (::std::find_if( title2temp.begin(), title2temp.end(),
                                    MatchTempDir( tempEntry ) ) ==
                    title2temp.end())
                {
                    // temp entry not needed anymore:
                    OUString url(
                        make_url( m_activePackages_expanded, tempEntry ) );
                    erase_path( url + OUSTR("_"),
                                Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                    erase_path( url, Reference<XCommandEnvironment>(),
                                false /* no throw: ignore errors */ );
                }
            }
        }
    }
}

//______________________________________________________________________________
void PackageManagerImpl::initRegistryBackends(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_registryCache.getLength() > 0)
        create_folder( 0, m_registryCache,
                       Reference<XCommandEnvironment>(), !m_readOnly );
    m_xRegistry.set( ::dp_registry::create(
                         m_context, m_registryCache, m_readOnly,
                         m_xComponentContext ) );
}

//______________________________________________________________________________
Reference<deployment::XPackageManager> PackageManagerImpl::create(
    Reference<XComponentContext> const & xComponentContext,
    OUString const & context )
{
    PackageManagerImpl * that = new PackageManagerImpl(
        xComponentContext, context );
    Reference<deployment::XPackageManager> xPackageManager( that );

    OUString packages, logFile, stampURL;
    if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") )) {
        that->m_activePackages = OUSTR("vnd.sun.star.expand:$UNO_"
                                       "USER_PACKAGES_CACHE/uno_packages");
        that->m_registryCache = OUSTR("vnd.sun.star.expand:$UNO_"
                                      "USER_PACKAGES_CACHE/registry");
        logFile = OUSTR("vnd.sun.star.expand:$UNO_"
                        "USER_PACKAGES_CACHE/log.txt");
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "USER_PACKAGES_CACHE/stamp");
    }
    else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") )) {
        that->m_activePackages = OUSTR("vnd.sun.star.expand:$UNO_"
                                       "SHARED_PACKAGES_CACHE/uno_packages");
        that->m_registryCache = OUSTR("vnd.sun.star.expand:$UNO_"
                                      "SHARED_PACKAGES_CACHE/registry");
        logFile = OUSTR("vnd.sun.star.expand:$UNO_"
                        "SHARED_PACKAGES_CACHE/log.txt");
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "SHARED_PACKAGES_CACHE/stamp");
    }
    else if (! context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:/") )) {
        throw lang::IllegalArgumentException(
            OUSTR("invalid context given: ") + context,
            Reference<XInterface>(), static_cast<sal_Int16>(-1) );
    }

    Reference<XCommandEnvironment> xCmdEnv;

    try {
        bool renewal = false;

        if (stampURL.getLength() > 0)
        {
            // currently no automatic renewal possible, because quickstarter
            // already hinders from deleting registry directory...

#define CURRENT_STAMP "1"
//             renewal = true;
//             {
//             ::ucb::Content ucbStamp;
//             if (create_ucb_content(
//                     &ucbStamp, stampURL, xCmdEnv, false /* no throw */ ))
//             {
//                 OUString line;
//                 renewal = !readLine( &line, OUSTR(CURRENT_STAMP), ucbStamp,
//                                      RTL_TEXTENCODING_ASCII_US );
//             }
//             }

            try {
                // probe writing:
                erase_path( stampURL, xCmdEnv );
                ::ucb::Content ucbStamp( stampURL, xCmdEnv );
                ::rtl::OString stamp(
                    RTL_CONSTASCII_STRINGPARAM(CURRENT_STAMP) );
                Reference<io::XInputStream> xData(
                    ::xmlscript::createInputStream(
                        ::rtl::ByteSequence(
                            reinterpret_cast<sal_Int8 const *>(stamp.getStr()),
                            stamp.getLength() ) ) );
                ucbStamp.writeStream( xData, true /* replace existing */ );
            }
            catch (RuntimeException &) {
                throw;
            }
            catch (Exception &) {
                that->m_readOnly = true;
            }
        }

        if (!that->m_readOnly && logFile.getLength() > 0)
        {
            Any any_logFile( makeAny(logFile) );
            that->m_xLogFile.set(
                that->m_xComponentContext->getServiceManager()
                ->createInstanceWithArgumentsAndContext(
                    OUSTR("com.sun.star.comp.deployment.ProgressLog"),
                    Sequence<Any>( &any_logFile, 1 ),
                    that->m_xComponentContext ),
                UNO_QUERY_THROW );
            xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv, that->m_xLogFile ) );
        }

        OSL_ENSURE( !that->m_readOnly || !renewal,
                    "### ought to reinstall all packages, but cannot write!" );
        if (!that->m_readOnly && renewal) // try to reinstall
            that->reinstallDeployedPackages(
                Reference<task::XAbortChannel>(), xCmdEnv );

        that->initRegistryBackends( xCmdEnv );
        that->initActivationLayer( xCmdEnv );

        return xPackageManager;

    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[context=\"") );
        buf.append( context );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             "\"] caught unexpected exception!") );
        throw lang::WrappedTargetRuntimeException(
            buf.makeStringAndClear(), Reference<XInterface>(), exc );
    }
}

//______________________________________________________________________________
PackageManagerImpl::~PackageManagerImpl()
{
}

//______________________________________________________________________________
void PackageManagerImpl::fireModified()
{
    ::cppu::OInterfaceContainerHelper * container = rBHelper.getContainer(
        ::getCppuType( static_cast<Reference<
                       util::XModifyListener> const *>(0) ) );
    if (container != 0)
    {
        Sequence< Reference<XInterface> > elements(
            container->getElements() );
        lang::EventObject evt( static_cast<OWeakObject *>(this) );
        for ( sal_Int32 pos = 0; pos < elements.getLength(); ++pos ) {
            Reference<util::XModifyListener> xListener(
                elements[ pos ], UNO_QUERY );
            if (xListener.is())
                xListener->modified( evt );
        }
    }
}

//______________________________________________________________________________
void PackageManagerImpl::disposing()
{
    try {
//     // xxx todo: guarding?
//     ::osl::MutexGuard guard( getMutex() );
        try_dispose( m_xLogFile );
        m_xLogFile.clear();
        try_dispose( m_xRegistry );
        m_xRegistry.clear();
        m_activePackagesDB.reset(0);
        m_xComponentContext.clear();

        t_pm_helper::disposing();

    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing..."),
            static_cast<OWeakObject *>(this), exc );
    }
}

// XComponent
//______________________________________________________________________________
void PackageManagerImpl::dispose() throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::dispose();
}

//______________________________________________________________________________
void PackageManagerImpl::addEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::addEventListener( xListener );
}

//______________________________________________________________________________
void PackageManagerImpl::removeEventListener(
    Reference<lang::XEventListener> const & xListener ) throw (RuntimeException)
{
    check();
    WeakComponentImplHelperBase::removeEventListener( xListener );
}

// XPackageManager
//______________________________________________________________________________
OUString PackageManagerImpl::getContext() throw (RuntimeException)
{
    check();
    return m_context;
}

//______________________________________________________________________________
void PackageManagerImpl::checkAborted(
    ::rtl::Reference<AbortChannel> const & abortChannel )
{
    if (abortChannel.is() && abortChannel->isAborted())
        throw CommandAbortedException(
            OUSTR("abort!"), static_cast<OWeakObject *>(this) );
}

//______________________________________________________________________________
Reference<task::XAbortChannel> PackageManagerImpl::createAbortChannel()
    throw (RuntimeException)
{
    check();
    return new AbortChannel;
}

// XModifyBroadcaster
//______________________________________________________________________________
void PackageManagerImpl::addModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.addListener( ::getCppuType( &xListener ), xListener );
}

//______________________________________________________________________________
void PackageManagerImpl::removeModifyListener(
    Reference<util::XModifyListener> const & xListener )
    throw (RuntimeException)
{
    check();
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

//______________________________________________________________________________
OUString PackageManagerImpl::detectMediaType(
    OUString const & title, ::ucb::Content const & ucbContent_, bool throw_exc )
{
    ::ucb::Content ucbContent(ucbContent_);
    OUString url( ucbContent.getURL() );
    OUString mediaType;
    if (url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:") ) ||
        url.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg:") ))
    {
        try {
            ucbContent.getPropertyValue( OUSTR("MediaType") ) >>= mediaType;
        }
        catch (beans::UnknownPropertyException &) {
        }
        OSL_ENSURE( mediaType.getLength() > 0, "### no media-type?!" );
    }
    if (mediaType.getLength() == 0)
    {
        if (title.endsWithIgnoreAsciiCaseAsciiL(
                RTL_CONSTASCII_STRINGPARAM(".uno.pkg") ))
            mediaType = OUSTR("application/vnd.sun.star.package-bundle");
        else if (title.endsWithIgnoreAsciiCaseAsciiL(
                     RTL_CONSTASCII_STRINGPARAM(".zip") ))
            mediaType = OUSTR("application/vnd.sun.star.legacy-package-bundle");
        else
        {
            try {
                Reference<deployment::XPackage> xPackage(
                    m_xRegistry->bindPackage(
                        url, OUString(), ucbContent.getCommandEnvironment() ) );
                mediaType = xPackage->getMediaType();
            }
            catch (lang::IllegalArgumentException & exc) {
                if (throw_exc)
                    throw;
                OSL_ENSURE( 0,
                            ::rtl::OUStringToOString(
                                exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }
    }
    return mediaType;
}

//______________________________________________________________________________
OUString PackageManagerImpl::insertToActivationLayer(
    OUString const & title, OUString const & mediaType,
    ::ucb::Content const & sourceContent_ )
{
    ::ucb::Content sourceContent(sourceContent_);
    Reference<XCommandEnvironment> xCmdEnv(
        sourceContent.getCommandEnvironment() );
    OUString destFolder, tempEntry;
    if (::osl::File::createTempFile(
            m_activePackages_expanded.getLength() == 0
            ? 0 : &m_activePackages_expanded,
            0, &tempEntry ) != ::osl::File::E_None)
        throw RuntimeException(
            OUSTR("::osl::File::createTempFile() failed!"), 0 );
    if (m_activePackages_expanded.getLength() == 0) {
        destFolder = tempEntry;
    }
    else {
        tempEntry = tempEntry.copy( tempEntry.lastIndexOf( '/' ) + 1 );
        // tweak user|share to macrofied url:
        destFolder = make_url( m_activePackages, tempEntry );
    }
    destFolder += OUSTR("_");

    // prepare activation folder:
    ::ucb::Content destFolderContent;
    create_folder( &destFolderContent, destFolder, xCmdEnv );

    // copy content into activation temp dir:
    if (mediaType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(
                "application/vnd.sun.star.package-bundle") ) ||
        // xxx todo: more sophisticated parsing
        mediaType.matchIgnoreAsciiCaseAsciiL(
            RTL_CONSTASCII_STRINGPARAM(
                "application/vnd.sun.star.legacy-package-bundle") ))
    {
        // inflate content:
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.zip://") );
        buf.append( ::rtl::Uri::encode( sourceContent.getURL(),
                                        rtl_UriCharClassRegName,
                                        rtl_UriEncodeIgnoreEscapes,
                                        RTL_TEXTENCODING_UTF8 ) );
        buf.append( static_cast<sal_Unicode>('/') );
        sourceContent = ::ucb::Content( buf.makeStringAndClear(), xCmdEnv );
    }
    if (! destFolderContent.transferContent(
            sourceContent, ::ucb::InsertOperation_COPY,
            OUString(), NameClash::OVERWRITE ))
        throw RuntimeException( OUSTR("UCB transferContent() failed!"), 0 );

    // write to DB:
    ::rtl::OUStringBuffer buf;
    buf.append( tempEntry );
    buf.append( static_cast<sal_Unicode>(';') );
    buf.append( mediaType );
    OUString inserted( buf.makeStringAndClear() );
    OSL_ASSERT( ! m_activePackagesDB->has( title ) );
    m_activePackagesDB->put( title, inserted, false /* ! overwrite */ );
    return destFolder;
}

// XPackageManager
//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::addPackage(
    OUString const & url, OUString const & mediaType_,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
{
    check();
    if (m_readOnly)
        throw deployment::DeploymentException(
            OUSTR("operating on read-only context!"),
            static_cast<OWeakObject *>(this), Any() );

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    ProgressLevel progress( xCmdEnv );
    try {
        ::ucb::Content sourceContent;
        create_ucb_content( &sourceContent, url, xCmdEnv ); // throws exc
        OUString title( extract_throw<OUString>(
                            sourceContent.getPropertyValue(OUSTR("Title")) ) );
        OUString title_enc( ::rtl::Uri::encode(
                                title, rtl_UriCharClassPchar,
                                rtl_UriEncodeIgnoreEscapes,
                                RTL_TEXTENCODING_UTF8 ) );
        OUString destFolder;

        OUString mediaType(mediaType_);
        if (mediaType.getLength() == 0)
            mediaType = detectMediaType( title, sourceContent );

        {
            ::osl::MutexGuard guard( getMutex() );

            if (m_activePackagesDB->has( title ))
            {
                // package already deployed, interact --force:
                interactContinuation_throw(
                    deployment::DeploymentException(
                        getResourceString(RID_STR_ERROR_WHILE_ADDING) + title,
                        static_cast<OWeakObject *>(this),
                        makeAny(
                            NameClashResolveRequest(
                                getResourceString(
                                    RID_STR_PACKAGE_ALREADY_ADDED ) + title,
                                static_cast<OWeakObject *>(this),
                                task::InteractionClassification_QUERY,
                                getDeployPath(title), title, OUString() ) ) ),
                    XInteractionReplaceExistingData::static_type(), xCmdEnv );
                // remove clashing package before copying new version:
                removePackage_(
                    title, xAbortChannel, xCmdEnv_ /* unwrapped cmd env */ );
            }
            OSL_ASSERT( ! m_activePackagesDB->has( title ) );

            // copy file:
            progress.update(
                getResourceString(RID_STR_COPYING_PACKAGE) + title );
            if (m_activePackages.getLength() == 0)
            {
                ::ucb::Content docFolderContent;
                create_folder( &docFolderContent, m_context, xCmdEnv );
                // copy into document, first:
                if (! docFolderContent.transferContent(
                        sourceContent, ::ucb::InsertOperation_COPY,
                        OUString(),
                        NameClash::ASK /* xxx todo: ASK not needed? */))
                    throw RuntimeException(
                        OUSTR("UCB transferContent() failed!"), 0 );
                // set media-type:
                ::ucb::Content docContent(
                    make_url( m_context, title_enc ), xCmdEnv );
                docContent.setPropertyValue(
                    OUSTR("MediaType"), makeAny(mediaType) );

                // xxx todo: obsolete in the future
                try {
                    docFolderContent.executeCommand( OUSTR("flush"), Any() );
                }
                catch (UnsupportedCommandException &) {
                }
            }
            destFolder = insertToActivationLayer(
                title, mediaType, sourceContent );
        } // guard

        // xxx todo: fire before bind(), registration?
        fireModified();

        // bind activation package:
        Reference<deployment::XPackage> xPackage(
            m_xRegistry->bindPackage( make_url( destFolder, title_enc ),
                                      mediaType, xCmdEnv ) );
        OSL_ASSERT( xPackage.is() );
        // register package:
        if (xPackage.is())
            xPackage->registerPackage( xAbortChannel, xCmdEnv );
        return xPackage;
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_ADDING) + url,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
void PackageManagerImpl::removePackage_(
    OUString const & name,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ProgressLevel progress( xCmdEnv );
    ::osl::MutexGuard guard( getMutex() );
    Reference<deployment::XPackage> xPackage( getDeployedPackage_(
                                                  name, xCmdEnv ) );
    OSL_ASSERT( xPackage->getURL().equals( getDeployPath(name) ) );
    beans::Optional< beans::Ambiguous<sal_Bool> > option(
        xPackage->isRegistered( Reference<task::XAbortChannel>(),
                                xCmdEnv ) );
    if (!option.IsPresent || option.Value.IsAmbiguous || option.Value.Value)
        xPackage->revokePackage( xAbortChannel, xCmdEnv );
    try_dispose( xPackage );
    m_activePackagesDB->erase( name ); // to be removed upon next start
}

//______________________________________________________________________________
void PackageManagerImpl::removePackage(
    OUString const & name,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException, CommandFailedException,
           CommandAbortedException, lang::IllegalArgumentException,
           RuntimeException)
{
    check();
    if (m_readOnly)
        throw deployment::DeploymentException(
            OUSTR("operating on read-only context!"),
            static_cast<OWeakObject *>(this), Any() );

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        removePackage_( name, xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (lang::IllegalArgumentException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_REMOVING) + name,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
OUString PackageManagerImpl::getDeployPath( OUString const & name,
                                            OUString * pMediaType,
                                            bool ignoreAlienPlatforms )
{
    OUString val;
    if (m_activePackagesDB->get( &val, name ))
    {
        sal_Int32 semi = val.indexOf(';');
        OSL_ASSERT( semi > 0 );
        if (pMediaType != 0)
            *pMediaType = val.copy( semi + 1 );
        if (ignoreAlienPlatforms)
        {
            String type, subType;
            INetContentTypeParameterList params;
            if (INetContentTypes::parse(
                    pMediaType == 0 ? val.copy( semi + 1 ) : *pMediaType,
                    type, subType, &params ))
            {
                INetContentTypeParameter const * param = params.find(
                    ByteString("platform") );
                if (param != 0 && !platform_fits( param->m_sValue ))
                    throw lang::IllegalArgumentException(
                        getResourceString(RID_STR_NO_SUCH_PACKAGE) + name,
                        static_cast<OWeakObject *>(this),
                        static_cast<sal_Int16>(-1) );
            }
        }

        ::rtl::OUStringBuffer buf;
        if (m_activePackages.getLength() > 0) {
            buf.append( m_activePackages );
            buf.append( static_cast<sal_Unicode>('/') );
        }
        buf.append( val.copy( 0, semi ) );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("_/") );
        buf.append( ::rtl::Uri::encode( name, rtl_UriCharClassPchar,
                                        rtl_UriEncodeIgnoreEscapes,
                                        RTL_TEXTENCODING_UTF8 ) );
        return buf.makeStringAndClear();
    }
    throw lang::IllegalArgumentException(
        getResourceString(RID_STR_NO_SUCH_PACKAGE) + name,
        static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
}

//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage_(
    OUString const & name, Reference<XCommandEnvironment> const & xCmdEnv,
    bool ignoreAlienPlatforms )
{
    OUString mediaType;
    OUString deployPath( getDeployPath(
                             name, &mediaType, ignoreAlienPlatforms ) );
    return m_xRegistry->bindPackage( deployPath, mediaType, xCmdEnv );
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> >
PackageManagerImpl::getDeployedPackages_(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::std::vector< Reference<deployment::XPackage> > packages;
    t_string2string_map title2temp( m_activePackagesDB->getEntries() );
    t_string2string_map::const_iterator iPos( title2temp.begin() );
    t_string2string_map::const_iterator const iEnd( title2temp.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        try {
            packages.push_back(
                getDeployedPackage_(
                    iPos->first, xCmdEnv,
                    true /* xxx todo: think of GUI:
                            ignore other platforms than the current one */ ) );
        }
        catch (lang::IllegalArgumentException & exc) {
            // ignore
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return Sequence< Reference<deployment::XPackage> >(
        &packages[ 0 ], packages.size() );
}

//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage(
    OUString const & name, Reference<XCommandEnvironment> const & xCmdEnv_ )
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
        ::osl::MutexGuard guard( getMutex() );
        return getDeployedPackage_( name, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (lang::IllegalArgumentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            // ought never occur...
            OUSTR("error while accessing deployed package: ") + name,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
Sequence< Reference<deployment::XPackage> >
PackageManagerImpl::getDeployedPackages(
    Reference<task::XAbortChannel> const & xAbortChannel,
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
        ::osl::MutexGuard guard( getMutex() );
        return getDeployedPackages_( xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            // ought never occur...
            OUSTR("error while getting all deployed packages: ") + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
void PackageManagerImpl::reinstallDeployedPackages(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv_ )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    check();
    if (m_readOnly)
        throw deployment::DeploymentException(
            OUSTR("operating on read-only context!"),
            static_cast<OWeakObject *>(this), Any() );

    if (office_is_running())
        throw RuntimeException(
            OUSTR("You must close any running Office process before "
                  "reinstalling packages!"), static_cast<OWeakObject *>(this) );

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try {
        ProgressLevel progress(
            xCmdEnv, OUSTR("Reinstalling all deployed packages...") );

        try_dispose( m_xRegistry );
        m_xRegistry.clear();
        if (m_registryCache.getLength() > 0)
            erase_path( m_registryCache, xCmdEnv );
        initRegistryBackends( xCmdEnv );

        // reregister all:
        ::osl::MutexGuard guard( getMutex() );
        Sequence< Reference<deployment::XPackage> > packages(
            getDeployedPackages_( xAbortChannel, xCmdEnv ) );
        Reference<deployment::XPackage> const * ppackages =
            packages.getConstArray();
        for ( sal_Int32 pos = 0; pos < packages.getLength(); ++pos )
            ppackages[ pos ]->registerPackage( xAbortChannel, xCmdEnv );

    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            OUSTR("Error while reinstalling all previously deployed "
                  "packages of context ") + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}

//##############################################################################

//______________________________________________________________________________
PackageManagerImpl::CmdEnvWrapperImpl::~CmdEnvWrapperImpl()
{
}

//______________________________________________________________________________
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
//______________________________________________________________________________
Reference<task::XInteractionHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getInteractionHandler()
    throw (RuntimeException)
{
    return m_xUserInteractionHandler;
}

//______________________________________________________________________________
Reference<XProgressHandler>
PackageManagerImpl::CmdEnvWrapperImpl::getProgressHandler()
    throw (RuntimeException)
{
    return this;
}

// XProgressHandler
//______________________________________________________________________________
void PackageManagerImpl::CmdEnvWrapperImpl::push( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->push( Status );
}

//______________________________________________________________________________
void PackageManagerImpl::CmdEnvWrapperImpl::update( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->update( Status );
}

//______________________________________________________________________________
void PackageManagerImpl::CmdEnvWrapperImpl::pop() throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->pop();
    if (m_xUserProgress.is())
        m_xUserProgress->pop();
}

} // namespace dp_manager

