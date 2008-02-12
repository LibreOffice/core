/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_manager.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:18:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_ucb.h"
#include "dp_resource.h"
#include "dp_manager.h"
#include "dp_identifier.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/uri.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/sequence.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
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
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/ucb/UnsupportedCommandException.hpp"
#include "boost/bind.hpp"
#include <vector>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

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

//______________________________________________________________________________
void PackageManagerImpl::initActivationLayer(
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    if (m_activePackages.getLength() == 0)
    {
        OSL_ASSERT( m_registryCache.getLength() == 0 );
        // documents temp activation:
        m_activePackagesDB.reset( new ActivePackages );
        ::ucbhelper::Content ucbContent;
        if (create_ucb_content( &ucbContent, m_context, xCmdEnv,
                                false /* no throw */ ))
        {
            // scan for all entries in m_packagesDir:
            Reference<sdbc::XResultSet> xResultSet(
                ucbContent.createCursor(
                    Sequence<OUString>( &StrTitle::get(), 1 ),
                    ::ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
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
                                             "META-INF") ) )
                    continue;

                ::ucbhelper::Content sourceContent(
                    Reference<XContentAccess>(
                        xResultSet, UNO_QUERY_THROW )->queryContent(),
                    xCmdEnv );

                OUString mediaType( detectMediaType( sourceContent,
                                                     false /* no throw */) );
                if (mediaType.getLength() >0)
                {
                    ActivePackages::Data dbData;
                    insertToActivationLayer(
                        mediaType, sourceContent, title, &dbData );

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
        OSL_ASSERT( m_activePackages.getLength() > 0 );
        m_activePackages_expanded = expandUnoRcUrl( m_activePackages );
        create_folder( 0, m_activePackages_expanded, xCmdEnv, !m_readOnly );
        m_activePackagesDB.reset(
            new ActivePackages(
                m_activePackages_expanded + OUSTR(".db"), m_readOnly ) );

        if (! m_readOnly)
        {
            // clean up activation layer, scan for zombie temp dirs:
            ActivePackages::Entries id2temp( m_activePackagesDB->getEntries() );

            ::ucbhelper::Content tempFolder(
                m_activePackages_expanded, xCmdEnv );
            Reference<sdbc::XResultSet> xResultSet(
                tempFolder.createCursor(
                    Sequence<OUString>( &StrTitle::get(), 1 ),
                    ::ucbhelper::INCLUDE_DOCUMENTS_ONLY ) );
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
                const MatchTempDir match( tempEntry );
                if (::std::find_if( id2temp.begin(), id2temp.end(), match ) ==
                    id2temp.end())
                {
                    // temp entry not needed anymore:
                    const OUString url( makeURL( m_activePackages_expanded,
                                                 tempEntry ) );
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
void PackageManagerImpl::initRegistryBackends()
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
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "USER_PACKAGES_CACHE/stamp.sys");
    }
    else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") )) {
        that->m_activePackages = OUSTR("vnd.sun.star.expand:$UNO_"
                                       "SHARED_PACKAGES_CACHE/uno_packages");
        that->m_registryCache = OUSTR("vnd.sun.star.expand:$UNO_"
                                      "SHARED_PACKAGES_CACHE/registry");
        logFile = OUSTR("vnd.sun.star.expand:$UNO_"
                        "SHARED_PACKAGES_CACHE/log.txt");
        //See description for stampURL for user packages.
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "SHARED_PACKAGES_CACHE/stamp.sys");
    }
    else if (! context.matchAsciiL(
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
//             ::ucbhelper::Content ucbStamp;
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
                ::ucbhelper::Content ucbStamp( stampURL, xCmdEnv );
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

        OSL_ENSURE( !that->m_readOnly || !renewal,
                    "### ought to reinstall all packages, but cannot write!" );
        if (!that->m_readOnly && renewal) // try to reinstall
            that->reinstallDeployedPackages(
                Reference<task::XAbortChannel>(), xCmdEnv );

        that->initRegistryBackends();
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
    ::cppu::OInterfaceContainerHelper * pContainer = rBHelper.getContainer(
        util::XModifyListener::static_type() );
    if (pContainer != 0) {
        pContainer->forEach<util::XModifyListener>(
            boost::bind(&util::XModifyListener::modified, _1,
                        lang::EventObject(static_cast<OWeakObject *>(this))) );
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
Sequence< Reference<deployment::XPackageTypeInfo> >
PackageManagerImpl::getSupportedPackageTypes() throw (RuntimeException)
{
    OSL_ASSERT( m_xRegistry.is() );
    return m_xRegistry->getSupportedPackageTypes();
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
    ::ucbhelper::Content const & ucbContent_, bool throw_exc )
{
    ::ucbhelper::Content ucbContent(ucbContent_);
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
        try {
            Reference<deployment::XPackage> xPackage(
                m_xRegistry->bindPackage(
                    url, OUString(), ucbContent.getCommandEnvironment() ) );
            const Reference<deployment::XPackageTypeInfo> xPackageType(
                xPackage->getPackageType() );
            OSL_ASSERT( xPackageType.is() );
            if (xPackageType.is())
                mediaType = xPackageType->getMediaType();
        }
        catch (lang::IllegalArgumentException & exc) {
            if (throw_exc)
                throw;
            (void) exc;
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return mediaType;
}

//______________________________________________________________________________
OUString PackageManagerImpl::insertToActivationLayer(
    OUString const & mediaType, ::ucbhelper::Content const & sourceContent_,
    OUString const & title, ActivePackages::Data * dbData )
{
    ::ucbhelper::Content sourceContent(sourceContent_);
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
        destFolder = makeURL( m_activePackages, tempEntry );
    }
    destFolder += OUSTR("_");

    // prepare activation folder:
    ::ucbhelper::Content destFolderContent;
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
        sourceContent = ::ucbhelper::Content(
            buf.makeStringAndClear(), xCmdEnv );
    }
    if (! destFolderContent.transferContent(
            sourceContent, ::ucbhelper::InsertOperation_COPY,
            title, NameClash::OVERWRITE ))
        throw RuntimeException( OUSTR("UCB transferContent() failed!"), 0 );

    // write to DB:
    dbData->temporaryName = tempEntry;
    dbData->fileName = title;
    dbData->mediaType = mediaType;
    return destFolder;
}

//______________________________________________________________________________
void PackageManagerImpl::insertToActivationLayerDB(
    OUString const & id, ActivePackages::Data const & dbData )
{
    m_activePackagesDB->put( id, dbData );
}

//______________________________________________________________________________
/* The function returns true if there is an extension with the same id already
    installed which needs to be uninstalled, before the new extension can be installed.
*/
bool PackageManagerImpl::checkUpdate(
    Reference<deployment::XPackage> const & package,
    Reference<XCommandEnvironment> const & origCmdEnv,
    Reference<XCommandEnvironment> const & wrappedCmdEnv )
{
    OUString id(dp_misc::getIdentifier(package));
    OUString fn(package->getName());
    bool removeExisting = false;
    if (m_activePackagesDB->has( id, fn ))
    {
        // package already deployed, interact --force:
        Any request(
            (deployment::VersionException(
                getResourceString( RID_STR_PACKAGE_ALREADY_ADDED ) + id,
                static_cast<OWeakObject *>(this), package,
                getDeployedPackage_( id, fn, origCmdEnv ) ) ) );
        bool replace = false, abort = false;
        if (! interactContinuation(
                request, task::XInteractionApprove::static_type(),
                wrappedCmdEnv, &replace, &abort )) {
            OSL_ASSERT( !replace && !abort );
            throw deployment::DeploymentException(
                getResourceString(RID_STR_ERROR_WHILE_ADDING) + id,
                static_cast<OWeakObject *>(this), request );
        }
        if (abort || !replace)
            throw CommandFailedException(
                getResourceString(RID_STR_ERROR_WHILE_ADDING) + id,
                static_cast<OWeakObject *>(this), request );

        // remove clashing package before registering new version:
        removeExisting = true;
    }
    return removeExisting;
}
//______________________________________________________________________________
// Notify the user when a new extension is to be installed. This is only the case
//when unopkg gui extension1 is used (used by system integration (double click on .oxt
// file etc.)). In case there is already this extension then the function returns
//true.
//ToDo: Function always returns true or throws an exception
bool PackageManagerImpl::checkInstall(
    Reference<deployment::XPackage> const & package,
    Reference<XCommandEnvironment> const & cmdEnv)
{
    OUString id(dp_misc::getIdentifier(package));
    if ( ! m_activePackagesDB->has( id, package->getName() ))
    {
        Any request(
            deployment::InstallException(
                OUSTR("Extension ") + id + OUSTR("is about to be installed."),
                static_cast<OWeakObject *>(this), package));
        bool approve = false, abort = false;
        if (! interactContinuation(
                request, task::XInteractionApprove::static_type(),
                cmdEnv, &approve, &abort ))
        {
            OSL_ASSERT( !approve && !abort );
            throw deployment::DeploymentException(
                getResourceString(RID_STR_ERROR_WHILE_ADDING) + id,
                static_cast<OWeakObject *>(this), request );
        }
        if (abort || !approve)
            throw CommandFailedException(
                getResourceString(RID_STR_ERROR_WHILE_ADDING) + id,
                static_cast<OWeakObject *>(this), request );

    }
    return true;
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

    try {
        ::ucbhelper::Content sourceContent;
        create_ucb_content( &sourceContent, url, xCmdEnv ); // throws exc
        const OUString title(sourceContent.getPropertyValue(
                             StrTitle::get() ).get<OUString>() );
        const OUString title_enc( ::rtl::Uri::encode(
                                      title, rtl_UriCharClassPchar,
                                      rtl_UriEncodeIgnoreEscapes,
                                      RTL_TEXTENCODING_UTF8 ) );
        OUString destFolder;

        OUString mediaType(mediaType_);
        if (mediaType.getLength() == 0)
            mediaType = detectMediaType( sourceContent );

        Reference<deployment::XPackage> xPackage;
        // copy file:
        progressUpdate(
            getResourceString(RID_STR_COPYING_PACKAGE) + title, xCmdEnv );
        if (m_activePackages.getLength() == 0)
        {
            ::ucbhelper::Content docFolderContent;
            create_folder( &docFolderContent, m_context, xCmdEnv );
            // copy into document, first:
            if (! docFolderContent.transferContent(
                    sourceContent, ::ucbhelper::InsertOperation_COPY,
                    OUString(),
                    NameClash::ASK /* xxx todo: ASK not needed? */))
                throw RuntimeException(
                    OUSTR("UCB transferContent() failed!"), 0 );
            // set media-type:
            ::ucbhelper::Content docContent(
                makeURL( m_context, title_enc ), xCmdEnv );
                //TODO #i73136#: using title instead of id can lead to
                // clashes, but the whole m_activePackages.getLength()==0
                // case (i.e., document-relative deployment) currently does
                // not work, anyway.
            docContent.setPropertyValue(
                OUSTR("MediaType"), Any(mediaType) );

            // xxx todo: obsolete in the future
            try {
                docFolderContent.executeCommand( OUSTR("flush"), Any() );
            }
            catch (UnsupportedCommandException &) {
            }
        }
        ActivePackages::Data dbData;
        destFolder = insertToActivationLayer(
            mediaType, sourceContent, title, &dbData );


        // bind activation package:
        //Because every extension will be unpacked in a folder, which was created with a unique name
        //we will always have two different XPackage objects, even if the second extension is the same.
        //Therefore bindPackage does not need a guard here.
        xPackage = m_xRegistry->bindPackage(
            makeURL( destFolder, title_enc ), mediaType, xCmdEnv );

        OSL_ASSERT( xPackage.is() );
        if (xPackage.is())
        {
            bool install = false;
            OUString id;

            try
            {
                id = dp_misc::getIdentifier( xPackage );
                //checkInstall throws an exception if the user denies the installation
                checkInstall(xPackage, xCmdEnv);
                //checkUpdate throws an exception if the user cancels the interaction.
                //For example, he may be asked if he wants to replace the older version
                //with the new version.
                //checkUpdates must be called before checkPrerequisites
                bool removeExisting = checkUpdate(
                    xPackage, xCmdEnv_, xCmdEnv );

                if (xPackage->checkPrerequisites(xAbortChannel, xCmdEnv, removeExisting, m_context))
                {
                    //This guard is used to prevent that an extension is installed twice. Do not use it in other
                    //functions.
                    //Imagine addPackage is called two times by different threads for the same extension quickly
                    //after each other.
                    //The second call would calculate "removeExisting = false" if the first thread has not yet reached
                    //insertToActivationLayerDB.
                    ::osl::MutexGuard g(m_addMutex);

                    if (removeExisting)
                        // remove extension which is already installed
                        //do not guard the complete function with the getMutex
                        removePackage_(id, xPackage->getName(), xAbortChannel, xCmdEnv_ /* unwrapped cmd env */ );
                    install = true;
                    const ::osl::MutexGuard guard( getMutex() );
                    //throws CommandAbortedException if the user cancelled the installation.
                    xPackage->registerPackage(xAbortChannel, xCmdEnv);
                    //install new version of extension
                    //access to the database must be guarded. See removePackage_
                    insertToActivationLayerDB(id, dbData);
                }
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
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
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
//______________________________________________________________________________
void PackageManagerImpl::removePackage_(
    OUString const & id, OUString const & fileName,
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    Reference<deployment::XPackage> xPackage;
    {
        const ::osl::MutexGuard guard(getMutex());
        xPackage =  getDeployedPackage_(id, fileName, xCmdEnv );
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            xPackage->isRegistered( Reference<task::XAbortChannel>(),
                                    xCmdEnv ) );
        if (!option.IsPresent || option.Value.IsAmbiguous || option.Value.Value)
            xPackage->revokePackage( xAbortChannel, xCmdEnv );
        m_activePackagesDB->erase( id, fileName ); // to be removed upon next start
    }
    try_dispose( xPackage );
}

//______________________________________________________________________________
void PackageManagerImpl::removePackage(
    OUString const & id, ::rtl::OUString const & fileName,
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
        removePackage_( id, fileName, xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (lang::IllegalArgumentException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_REMOVING) + id,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
OUString PackageManagerImpl::getDeployPath( ActivePackages::Data const & data )
{
    ::rtl::OUStringBuffer buf;
    buf.append( data.temporaryName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("_/") );
    buf.append( ::rtl::Uri::encode( data.fileName, rtl_UriCharClassPchar,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8 ) );
    return makeURL( m_activePackages, buf.makeStringAndClear() );
}

//______________________________________________________________________________
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

//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage_(
    OUString const & id, ActivePackages::Data const & data,
    Reference<XCommandEnvironment> const & xCmdEnv, bool ignoreAlienPlatforms )
{
    if (ignoreAlienPlatforms)
    {
        String type, subType;
        INetContentTypeParameterList params;
        if (INetContentTypes::parse( data.mediaType, type, subType, &params ))
        {
            INetContentTypeParameter const * param = params.find(
                ByteString("platform") );
            if (param != 0 && !platform_fits( param->m_sValue ))
                throw lang::IllegalArgumentException(
                    getResourceString(RID_STR_NO_SUCH_PACKAGE) + id,
                    static_cast<OWeakObject *>(this),
                    static_cast<sal_Int16>(-1) );
        }
    }
    return m_xRegistry->bindPackage(
        getDeployPath( data ), data.mediaType, xCmdEnv );
}

//______________________________________________________________________________
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
        try {
            packages.push_back(
                getDeployedPackage_(
                    iPos->first, iPos->second, xCmdEnv,
                    true /* xxx todo: think of GUI:
                            ignore other platforms than the current one */ ) );
        }
        catch (lang::IllegalArgumentException & exc) {
            // ignore
            (void) exc; // avoid warnings
            OSL_ENSURE( 0, ::rtl::OUStringToOString(
                            exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    return comphelper::containerToSequence(packages);
}

//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage(
    OUString const & id, ::rtl::OUString const & fileName,
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
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (lang::IllegalArgumentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            // ought never occur...
            OUSTR("error while accessing deployed package: ") + id,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
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
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
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
        initRegistryBackends();
        Reference<util::XUpdatable> xUpdatable( m_xRegistry, UNO_QUERY );
        if (xUpdatable.is())
            xUpdatable->update();

        // reregister all:
        const ::osl::MutexGuard guard( getMutex() );
        const Sequence< Reference<deployment::XPackage> > packages(
            getDeployedPackages_( xCmdEnv ) );
        for ( sal_Int32 pos = 0; pos < packages.getLength(); ++pos )
            packages[ pos ]->registerPackage( xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (CommandAbortedException & exc) {
        logIntern( Any(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc) {
        logIntern( Any(exc) );
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


::sal_Bool SAL_CALL PackageManagerImpl::isReadOnly(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return m_readOnly;
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

