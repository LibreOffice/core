/*************************************************************************
 *
 *  $RCSfile: dp_manager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:07:26 $
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

#include "dp_misc.h"
#include "dp_ucb.h"
#include "dp_resource.h"
#include "dp_interact.h"
#include "dp_persmap.h"
#include "dp_manager.hrc"
#include "rtl/ustrbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/uri.hxx"
#include "rtl/ref.hxx"
#include "osl/diagnose.h"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "ucbhelper/content.hxx"
#include "xmlscript/xml_helper.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/deployment/PackageRegistry.hpp"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/XInteractionReplaceExistingData.hpp"
#include <vector>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_manager
{

typedef ::cppu::WeakComponentImplHelper1<
    deployment::XPackageManager > t_pm_helper;

//==============================================================================
class PackageManagerImpl : private MutexHolder, public t_pm_helper
{
    Reference<XComponentContext> m_xComponentContext;
    OUString m_context;
    OUString m_packagesDir;
    OUString m_registryCache;
    sal_Bool m_readOnly;

    Reference<deployment::XPackageRegistry> m_xPackageRegistry;
    bool m_callRegistry;

    Reference<XProgressHandler> m_xLogFile;
    inline void logIntern( Any const & status );
    void fireModified();

    ::std::auto_ptr<PersistentMap> m_persMediaTypes;
    inline OUString getDeployPath( OUString const & name ) const;

    void checkAborted( ::rtl::Reference<AbortChannel> const & abortChannel );

    class CmdEnvWrapperImpl
        : public ::cppu::WeakImplHelper2< XCommandEnvironment,
                                          XProgressHandler >
    {
        Reference<XProgressHandler> m_xLogFile;
        Reference<XProgressHandler> m_xUserProgress;
        Reference<task::XInteractionHandler> m_xUserInteractionHandler;

    public:
        virtual ~CmdEnvWrapperImpl();
        CmdEnvWrapperImpl( Reference<XCommandEnvironment> const & xUserCmdEnv,
                           Reference<XProgressHandler> const & xLogFile );

        // XCommandEnvironment
        virtual Reference<task::XInteractionHandler> SAL_CALL
        getInteractionHandler() throw (RuntimeException);
        virtual Reference<XProgressHandler> SAL_CALL getProgressHandler()
            throw (RuntimeException);

        // XProgressHandler
        virtual void SAL_CALL push( Any const & Status )
            throw (RuntimeException);
        virtual void SAL_CALL update( Any const & Status )
            throw (RuntimeException);
        virtual void SAL_CALL pop() throw (RuntimeException);
    };

protected:
    inline void check();
    virtual void SAL_CALL disposing();

    virtual ~PackageManagerImpl();
    inline PackageManagerImpl(
        Reference<XComponentContext> const & xComponentContext,
        OUString const & context )
        : t_pm_helper( getMutex() ),
          m_xComponentContext( xComponentContext ),
          m_context( context ),
          m_readOnly( false ),
          m_callRegistry( true )
        {}

public:
    static Reference<deployment::XPackageManager> create(
        Reference<XComponentContext> const & xComponentContext,
        OUString const & context );

    // XComponent
    virtual void SAL_CALL dispose() throw (RuntimeException);
    virtual void SAL_CALL addEventListener(
        Reference<lang::XEventListener> const & xListener )
        throw (RuntimeException);
    virtual void SAL_CALL removeEventListener(
        Reference<lang::XEventListener> const & xListener )
        throw (RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        Reference<util::XModifyListener> const & xListener )
        throw (RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        Reference<util::XModifyListener> const & xListener )
        throw (RuntimeException);

    // XPackageManager
    virtual OUString SAL_CALL getContext() throw (RuntimeException);
    virtual Reference<deployment::XPackageRegistry> SAL_CALL
    getPackageRegistry() throw (RuntimeException);
    virtual Reference<task::XAbortChannel> SAL_CALL createAbortChannel()
        throw (RuntimeException);

    virtual Reference<deployment::XPackage> SAL_CALL addPackage(
        OUString const & url, OUString const & mediaType,
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               CommandAbortedException, lang::IllegalArgumentException,
               RuntimeException);

    void removePackage_(
        OUString const & name,
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );
    virtual void SAL_CALL removePackage(
        OUString const & name,
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               CommandAbortedException, lang::IllegalArgumentException,
               RuntimeException);

    Reference<deployment::XPackage> SAL_CALL getDeployedPackage_(
        OUString const & name, Reference<XCommandEnvironment> const & xCmdEnv,
        bool ignoreAlienPlatforms = false );
    virtual Reference<deployment::XPackage> SAL_CALL getDeployedPackage(
        OUString const & name, Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               lang::IllegalArgumentException, RuntimeException);

    Sequence< Reference<deployment::XPackage> > getDeployedPackages_(
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv );
    virtual Sequence< Reference<deployment::XPackage> > SAL_CALL
    getDeployedPackages( Reference<task::XAbortChannel> const & xAbortChannel,
                         Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               CommandAbortedException, lang::IllegalArgumentException,
               RuntimeException);

    virtual void SAL_CALL reinstallDeployedPackages(
        Reference<task::XAbortChannel> const & xAbortChannel,
        Reference<XCommandEnvironment> const & xCmdEnv )
        throw (deployment::DeploymentException, CommandFailedException,
               CommandAbortedException, lang::IllegalArgumentException,
               RuntimeException);
};

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
    if (xUserCmdEnv.is())
    {
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


//______________________________________________________________________________
inline void PackageManagerImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("PackageManager instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
inline OUString PackageManagerImpl::getDeployPath( OUString const & name ) const
{
    return make_url( m_packagesDir, ::rtl::Uri::encode(
                         name, rtl_UriCharClassPchar,
                         rtl_UriEncodeIgnoreEscapes,
                         RTL_TEXTENCODING_UTF8 ) );
}

//______________________________________________________________________________
inline void PackageManagerImpl::logIntern( Any const & status )
{
    if (m_xLogFile.is())
        m_xLogFile->update( status );
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
        for ( sal_Int32 pos = 0; pos < elements.getLength(); ++pos )
        {
            Reference<util::XModifyListener> xListener(
                elements[ pos ], UNO_QUERY );
            if (xListener.is())
                xListener->modified( evt );
        }
    }
}

//______________________________________________________________________________
Reference<deployment::XPackageManager> PackageManagerImpl::create(
    Reference<XComponentContext> const & xComponentContext,
    OUString const & context )
{
    PackageManagerImpl * that = new PackageManagerImpl(
        xComponentContext, context );
    Reference<deployment::XPackageManager> xPackageManager( that );

    OUString logFile, mediaTypes, stampURL;
    if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
    {
        that->m_packagesDir = OUSTR("vnd.sun.star.expand:$UNO_"
                                    "USER_PACKAGES_CACHE/uno_packages");
        that->m_registryCache = OUSTR("vnd.sun.star.expand:$UNO_"
                                      "USER_PACKAGES_CACHE/registry");
        mediaTypes = OUSTR("vnd.sun.star.expand:$UNO_"
                           "USER_PACKAGES_CACHE/media_types.db");
        logFile = OUSTR("vnd.sun.star.expand:$UNO_"
                        "USER_PACKAGES_CACHE/log.txt");
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "USER_PACKAGES_CACHE/stamp");
    }
    else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
    {
        that->m_packagesDir = OUSTR("vnd.sun.star.expand:$UNO_"
                                    "SHARED_PACKAGES_CACHE/uno_packages");
        that->m_registryCache = OUSTR("vnd.sun.star.expand:$UNO_"
                                      "SHARED_PACKAGES_CACHE/registry");
        mediaTypes = OUSTR("vnd.sun.star.expand:$UNO_"
                           "SHARED_PACKAGES_CACHE/media_types.db");
        logFile = OUSTR("vnd.sun.star.expand:$UNO_"
                        "SHARED_PACKAGES_CACHE/log.txt");
        stampURL = OUSTR("vnd.sun.star.expand:$UNO_"
                         "SHARED_PACKAGES_CACHE/stamp");
    }
    else if (context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg:///") ))
    {
        // deploy into document, but no registration:
        that->m_packagesDir = context;
        that->m_callRegistry = false;
        // no log file?: xxx todo
    }
    else if (context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:/") ))
    {
        // deploy into transient document, including registration:
        that->m_packagesDir = context;
        // no log file?: xxx todo
    }
    else
    {
        throw lang::IllegalArgumentException(
            OUSTR("invalid context given: ") + context,
            Reference<XInterface>(), static_cast<sal_Int16>(-1) );
    }

    Reference<XCommandEnvironment> xCmdEnv;

    try
    {
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

            try
            {
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
            catch (RuntimeException &)
            {
                throw;
            }
            catch (Exception &)
            {
                that->m_readOnly = true;
            }
        }

        // assure packages folder is existing:
        create_folder( 0, that->m_packagesDir, xCmdEnv, !that->m_readOnly );
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
        }

        if (mediaTypes.getLength() > 0) // open media_types.db
            that->m_persMediaTypes.reset(
                new PersistentMap( mediaTypes, that->m_readOnly ) );

        OSL_ENSURE( !that->m_readOnly || !renewal,
                    "### ought to reinstall all packages, but cannot write!" );
        if (!that->m_readOnly && renewal) // try to reinstall
            that->reinstallDeployedPackages(
                Reference<task::XAbortChannel>(), xCmdEnv );

        return xPackageManager;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
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
void PackageManagerImpl::disposing()
{
    try
    {
//     // xxx todo: guarding?
//     ::osl::MutexGuard guard( getMutex() );
        try_dispose( m_xLogFile );
        m_xLogFile.clear();
        try_dispose( m_xPackageRegistry );
        m_xPackageRegistry.clear();
        m_persMediaTypes.reset(0);

        t_pm_helper::disposing();
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
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
Reference<deployment::XPackageRegistry> PackageManagerImpl::getPackageRegistry()
    throw (RuntimeException)
{
    ::osl::MutexGuard guard( getMutex() );
    check();
    if (! m_xPackageRegistry.is())
    {
        try
        {
            // xxx todo: create an own registry (with subsequent backends
            //           for any manager(context)?
            //           what happens if an embedded document is opened
            //           and modified?  Then a different manager with
            //           different registry will be instantiated...
            //           the other way round: instantiating via /singl/reg
            //           will globally register all scripts of open documents
            //           which might be not wanted, too.
            if (m_registryCache.getLength() > 0)
            {
                create_folder( 0, m_registryCache,
                               Reference<XCommandEnvironment>(), !m_readOnly );
                m_xPackageRegistry.set(
                    deployment::PackageRegistry::createPersistent(
                        m_xComponentContext, m_context,
                        m_registryCache, m_readOnly ) );
            }
            else
            {
                m_xPackageRegistry.set(
                    deployment::PackageRegistry::createTransient(
                        m_xComponentContext, m_context ) );
            }
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException(
                OUSTR("caught unexpected exception while "
                      "installing PackageRegistry!"),
                static_cast<OWeakObject *>(this), exc );
        }
    }
    return m_xPackageRegistry;
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
    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    ProgressLevel progress( xCmdEnv );
    try
    {
        ::ucb::Content sourceContent;
        create_ucb_content( &sourceContent, url, xCmdEnv ); // throws exc
        OUString title( extract_throw< OUString >(
                            sourceContent.getPropertyValue(OUSTR("Title")) ) );
        OUString deployPath( getDeployPath(title) );

        // prebind package before any copying to assure a valid package:
        Reference<deployment::XPackage> xPackage(
            getPackageRegistry()->bindPackage( url, mediaType_, xCmdEnv ) );
        OUString mediaType( xPackage->getMediaType() );

        {
        ::osl::MutexGuard guard( getMutex() );

        if (create_ucb_content( 0, deployPath, xCmdEnv, false /* no throw */ ))
        {
            // package already deployed, interact whether pkgchk -f:
            interactThrow(
                deployment::DeploymentException(
                    getResourceString(RID_STR_ERROR_WHILE_ADDING) + title,
                    static_cast<OWeakObject *>(this),
                    makeAny( NameClashResolveRequest(
                                 getResourceString(
                                     RID_STR_PACKAGE_ALREADY_ADDED ) + title,
                                 static_cast<OWeakObject *>(this),
                                 task::InteractionClassification_QUERY,
                                 deployPath, title, OUString() ) ) ),
                ::getCppuType(
                    static_cast<Reference<XInteractionReplaceExistingData>
                    const *>(0) ),
                xCmdEnv );
            // remove clashing package before copying new version:
            removePackage_(
                title, xAbortChannel, xCmdEnv_ /* unwrapped cmd env */ );
        }

        // copy file:
        progress.update( getResourceString(RID_STR_COPYING_PACKAGE) + title );
        ::ucb::Content destContent( m_packagesDir, xCmdEnv );
        if (! destContent.transferContent(
                sourceContent, ::ucb::InsertOperation_COPY,
                OUString(), NameClash::ASK /* xxx todo: ASK not needed? */ ))
            throw RuntimeException( OUSTR("UCB transferContent() failed!"), 0 );

        if (m_persMediaTypes.get() == 0)
        {
            destContent.setPropertyValue(
                OUSTR("MediaType"), makeAny(mediaType) );
        }
        else
        {
            OSL_ASSERT( ! m_persMediaTypes->has( title ) );
            OUString inserted( mediaType );
            m_persMediaTypes->put( title, inserted );
            OSL_ASSERT( inserted.equals( mediaType ) );
        }
        } // guard

        // xxx todo: fire before bind(), registration?
        fireModified();

        // bind deployed package:
        xPackage = getPackageRegistry()->bindPackage(
            deployPath, mediaType, xCmdEnv );
        OSL_ASSERT( xPackage.is() );
        // register package:
        if (m_callRegistry && xPackage.is())
            xPackage->registerPackage( xAbortChannel, xCmdEnv );
        return xPackage;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (CommandFailedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &)
    {
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
    Reference<deployment::XPackage> xPackage( getDeployedPackage_(
                                                  name, xCmdEnv ) );
    OUString url( xPackage->getURL() );
    OSL_ASSERT( url.equals( getDeployPath(name) ) );
    if (m_callRegistry)
    {
        bool reg = true;
        try
        {
            reg = xPackage->isRegistered(
                // xxx todo: chain abort?
                Reference<task::XAbortChannel>(), xCmdEnv );
        }
        catch (beans::UnknownPropertyException &)
        {
        }
        if (reg) // revoke package from registry:
            xPackage->revokePackage( xAbortChannel, xCmdEnv );
        try_dispose( xPackage );
        if (m_persMediaTypes.get() != 0)
            m_persMediaTypes->erase( name );
    }
    // remove manager's copy:
    erase_path( url, xCmdEnv );
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
    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try
    {
        removePackage_( name, xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (lang::IllegalArgumentException &)
    {
        throw;
    }
    catch (CommandFailedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_REMOVING) + name,
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
Reference<deployment::XPackage> PackageManagerImpl::getDeployedPackage_(
    OUString const & name, Reference<XCommandEnvironment> const & xCmdEnv,
    bool ignoreAlienPlatforms )
{
    OUString deployPath( getDeployPath(name) );
    ::osl::MutexGuard guard( getMutex() );
    check();
    ::ucb::Content ucbContent;
    if (create_ucb_content(
            &ucbContent, deployPath, xCmdEnv, false /* no throw */ ))
    {
        OUString mediaType;
        if (m_persMediaTypes.get() == 0)
        {
            try
            {
                ucbContent.getPropertyValue( OUSTR("MediaType") ) >>= mediaType;
            }
            catch (beans::UnknownPropertyException &)
            {
                OSL_ENSURE(
                    0, "### no media-type entry for deployed package!" );
            }
        }
        else
        {
            OSL_ASSERT( m_persMediaTypes->has( name ) );
            m_persMediaTypes->get( &mediaType, name );
        }
        if (ignoreAlienPlatforms)
        {
            String type, subType;
            INetContentTypeParameterList params;
            if (INetContentTypes::parse( mediaType, type, subType, &params ))
            {
                INetContentTypeParameter const * param = params.find(
                    ByteString("platform") );
                if (param != 0 && !platform_fits( param->m_sValue ))
                    return Reference<deployment::XPackage>();
            }
        }
        return getPackageRegistry()->bindPackage(
            deployPath, mediaType, xCmdEnv );
    }
    throw lang::IllegalArgumentException(
        getResourceString(RID_STR_NO_SUCH_PACKAGE) + name,
        static_cast<OWeakObject *>(this), static_cast<sal_Int16>(-1) );
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

    try
    {
        return getDeployedPackage_( name, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (CommandFailedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (lang::IllegalArgumentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &)
    {
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
PackageManagerImpl::getDeployedPackages_(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
{
    ::std::vector< Reference<deployment::XPackage> > packages;
    // scan for all entries in m_packagesDir:
    ::ucb::Content ucbContent( m_packagesDir, xCmdEnv );
    OUString strTitle = OUSTR("Title");
    Reference<sdbc::XResultSet> xResultSet(
        ucbContent.createCursor( Sequence<OUString>( &strTitle, 1 ),
                                 ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    while (xResultSet->next())
    {
        Reference<sdbc::XRow> xRow( xResultSet, UNO_QUERY_THROW );
        // xxx todo: remove workaround for tdoc
        if (xRow->getString( 1 /* Title */ ).equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM(
                    "this_is_a_dummy_stream_just_there_"
                    "as_a_workaround_for_a_"
                    "temporary_limitation_of_the_"
                    "storage_api_implementation") ))
            continue;
        Reference<deployment::XPackage> xPackage(
            getDeployedPackage_(
                xRow->getString( 1 /* Title */ ), xCmdEnv,
                true /* ignore other platforms than the current one */ ) );
        if (xPackage.is())
            packages.push_back( xPackage );
    }
    return Sequence< Reference<deployment::XPackage> >(
        &packages[ 0 ], packages.size() );
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

    try
    {
        return getDeployedPackages_( xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (CommandFailedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &)
    {
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
    if (office_is_running())
        throw RuntimeException(
            OUSTR("You must close any running Office process before "
                  "reinstalling packages!"), static_cast<OWeakObject *>(this) );

    Reference<XCommandEnvironment> xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CmdEnvWrapperImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    try
    {
        ProgressLevel progress(
            xCmdEnv, OUSTR("Reinstalling all deployed packages...") );

        try_dispose( m_xPackageRegistry );
        m_xPackageRegistry.clear();
        if (m_registryCache.getLength() > 0)
        {
            erase_path( m_registryCache, xCmdEnv );
            create_folder( 0, m_registryCache, xCmdEnv );
        }

        Sequence< Reference<deployment::XPackage> > packages(
            getDeployedPackages_( xAbortChannel, xCmdEnv ) );
        Reference<deployment::XPackage> const * ppackages =
            packages.getConstArray();
        for ( sal_Int32 pos = 0; pos < packages.getLength(); ++pos )
            ppackages[ pos ]->registerPackage( xAbortChannel, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (CommandFailedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (CommandAbortedException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (deployment::DeploymentException & exc)
    {
        logIntern( makeAny(exc) );
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        logIntern( exc );
        throw deployment::DeploymentException(
            OUSTR("Error while reinstalling all previously deployed "
                  "packages of context ") + m_context,
            static_cast<OWeakObject *>(this), exc );
    }
}

//##############################################################################

namespace factory
{

typedef ::cppu::WeakComponentImplHelper2<
    deployment::XPackageManagerFactory, lang::XUnoTunnel > t_pmfac_helper;

//==============================================================================
class PackageManagerFactoryImpl : private MutexHolder, public t_pmfac_helper
{
    Reference<XComponentContext> m_xComponentContext;

    Reference<deployment::XPackageManager> m_xUserMgr;
    Reference<deployment::XPackageManager> m_xSharedMgr;
    typedef ::std::hash_map<
        OUString, WeakReference<deployment::XPackageManager>,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_managers;

protected:
    inline void check();
    virtual void SAL_CALL disposing();

public:
    virtual ~PackageManagerFactoryImpl();
    PackageManagerFactoryImpl(
        Reference<XComponentContext> const & xComponentContext );

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( Sequence<sal_Int8> const & id )
        throw (RuntimeException);

    // XPackageManagerFactory
    virtual Reference<deployment::XPackageManager> SAL_CALL getPackageManager(
        OUString const & context ) throw (RuntimeException);
};

// XUnoTunnel: hack to set application solar mutex from within gui
//______________________________________________________________________________
sal_Int64 PackageManagerFactoryImpl::getSomething(
    Sequence<sal_Int8> const & id ) throw (RuntimeException)
{
    ::rtl::OString str( reinterpret_cast<sal_Char const *>(id.getConstArray()),
                        id.getLength() );
    if (str.equals("ResMgrMutexPointer"))
        return reinterpret_cast<sal_Int64>(&g_pResMgrMmutex);
    return 0;
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.PackageManagerFactory");
}

//==============================================================================
Sequence<OUString> SAL_CALL getSupportedServiceNames()
{
    OUString strName( getImplementationName() );
    return Sequence<OUString>( &strName, 1 );
}

//==============================================================================
Reference<XInterface> SAL_CALL create(
    Reference<XComponentContext> const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new PackageManagerFactoryImpl( xComponentContext ) );
}

//==============================================================================
bool singleton_entries(
    Reference<registry::XRegistryKey> const & xRegistryKey )
{
    try
    {
        Reference<registry::XRegistryKey> xKey(
            xRegistryKey->createKey(
                getImplementationName() +
                // xxx todo: use future generated function to get singleton name
                OUSTR("/UNO/SINGLETONS/"
                      "com.sun.star.deployment.thePackageManagerFactory") ) );
        xKey->setStringValue( getImplementationName() );
        return true;
    }
    catch (registry::InvalidRegistryException & exc)
    {
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
        return false;
    }
}

//______________________________________________________________________________
PackageManagerFactoryImpl::PackageManagerFactoryImpl(
    Reference<XComponentContext> const & xComponentContext )
    : t_pmfac_helper( getMutex() ),
      m_xComponentContext( xComponentContext )
{
}

//______________________________________________________________________________
PackageManagerFactoryImpl::~PackageManagerFactoryImpl()
{
}

//______________________________________________________________________________
inline void PackageManagerFactoryImpl::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("PackageManagerFactory instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
    }
}

//______________________________________________________________________________
void PackageManagerFactoryImpl::disposing()
{
    // dispose all managers:
    ::osl::MutexGuard guard( getMutex() );
    t_string2weakref::const_iterator iPos( m_managers.begin() );
    t_string2weakref::const_iterator const iEnd( m_managers.end() );
    for ( ; iPos != iEnd; ++iPos )
        try_dispose( iPos->second );
    m_managers = t_string2weakref();
    // the below are already disposed:
    m_xUserMgr.clear();
    m_xSharedMgr.clear();
}

// XPackageManagerFactory
//______________________________________________________________________________
Reference<deployment::XPackageManager>
PackageManagerFactoryImpl::getPackageManager( OUString const & context )
    throw (RuntimeException)
{
    Reference< deployment::XPackageManager > xRet;
    ::osl::ResettableMutexGuard guard( getMutex() );
    check();
    t_string2weakref::const_iterator const iFind( m_managers.find( context ) );
    if (iFind != m_managers.end())
    {
        xRet = iFind->second;
        if (xRet.is())
            return xRet;
    }

    guard.clear();
    xRet.set( PackageManagerImpl::create( m_xComponentContext, context ) );
    guard.reset();
    ::std::pair< t_string2weakref::iterator, bool > insertion(
        m_managers.insert( t_string2weakref::value_type( context, xRet ) ) );
    if (insertion.second)
    {
        OSL_ASSERT( insertion.first->second.get() == xRet );
        // hold user, shared mgrs for whole process: live deployment
        if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
            m_xUserMgr = xRet;
        else if (context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
            m_xSharedMgr = xRet;
    }
    else
    {
        Reference< deployment::XPackageManager > xAlreadyIn(
            insertion.first->second );
        if (xAlreadyIn.is())
        {
            guard.clear();
            try_dispose( xRet );
            xRet = xAlreadyIn;
        }
        else
        {
            insertion.first->second = xRet;
        }
    }
    return xRet;
}

} // namespace factory

} // namespace dp_manager

