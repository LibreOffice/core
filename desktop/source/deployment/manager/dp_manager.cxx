/*************************************************************************
 *
 *  $RCSfile: dp_manager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:06:35 $
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
#include "dp_props.h"
#include "dp_persmap.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/diagnose.h"
#include "osl/thread.h"
#include "cppuhelper/weakref.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/compbase2.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/container/ElementExistException.hpp"
#include "com/sun/star/deployment/XPackageManagerFactory.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "com/sun/star/ucb/NameClash.hpp"
#include "com/sun/star/ucb/NameClashResolveRequest.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include <vector>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_manager
{

//==============================================================================
class CommandEnvironmentImpl
    : public ::cppu::WeakImplHelper2< XCommandEnvironment, XProgressHandler >
{
    Reference< XProgressHandler > m_xLogFile;
    Reference< XProgressHandler > m_xUserProgress;
    Reference< task::XInteractionHandler > m_xUserInteractionHandler;

public:
    virtual ~CommandEnvironmentImpl();
    inline CommandEnvironmentImpl(
        Reference< XCommandEnvironment > const & xUserCmdEnv,
        Reference< XProgressHandler > const & xLogFile )
        : m_xLogFile( xLogFile ),
          m_xUserProgress( xUserCmdEnv->getProgressHandler() ),
          m_xUserInteractionHandler( xUserCmdEnv->getInteractionHandler() )
        {}

    // XCommandEnvironment
    virtual Reference< task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (RuntimeException);
    virtual Reference< XProgressHandler > SAL_CALL getProgressHandler()
        throw (RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( Any const & Status ) throw (RuntimeException);
    virtual void SAL_CALL update( Any const & Status ) throw (RuntimeException);
    virtual void SAL_CALL pop() throw (RuntimeException);
};

//______________________________________________________________________________
CommandEnvironmentImpl::~CommandEnvironmentImpl()
{
}

// XCommandEnvironment
//______________________________________________________________________________
Reference< task::XInteractionHandler >
CommandEnvironmentImpl::getInteractionHandler() throw (RuntimeException)
{
    return m_xUserInteractionHandler;
}

//______________________________________________________________________________
Reference< XProgressHandler > CommandEnvironmentImpl::getProgressHandler()
    throw (RuntimeException)
{
    return this;
}

// XProgressHandler
//______________________________________________________________________________
void CommandEnvironmentImpl::push( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->push( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->push( Status );
}

//______________________________________________________________________________
void CommandEnvironmentImpl::update( Any const & Status )
    throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->update( Status );
    if (m_xUserProgress.is())
        m_xUserProgress->update( Status );
}

//______________________________________________________________________________
void CommandEnvironmentImpl::pop() throw (RuntimeException)
{
    if (m_xLogFile.is())
        m_xLogFile->pop();
    if (m_xUserProgress.is())
        m_xUserProgress->pop();
}

//##############################################################################

typedef ::cppu::ImplInheritanceHelper2<
    PropertyComponentBase,
    lang::XServiceInfo, deployment::XPackageManager > t_pm_helper;

//==============================================================================
class PackageManagerImpl : public t_pm_helper
{
    enum { DEPLOYED_PACKAGES, REGISTRY };
    ::std::auto_ptr< ::cppu::IPropertyArrayHelper > m_property_array_helper;

    Reference< XComponentContext > m_xComponentContext;
    OUString m_context;
    OUString m_packages_dir;
    bool m_register_packages;
    Reference< deployment::XPackageRegistry > m_xPackageRegistry;
    Reference< XProgressHandler > m_xLogFile;
    ::std::auto_ptr< PersistentMap > m_persMediaTypes;

    inline OUString getDeployPath( OUString const & name ) const;
    inline OUString errmsg( OUString const & package, bool adding ) const;

    bool bindPackage_(
        Reference< deployment::XPackage > & xPackage,
        OUString const & name,
        Reference< XCommandEnvironment > const & xCmdEnv =
        Reference< XCommandEnvironment >() ) const;
    Sequence< Reference< deployment::XPackage > > getDeployedPackages(
        Reference< XCommandEnvironment > const & xCmdEnv );

protected:
    inline void check() const { check_undisposed(); }
    virtual void SAL_CALL disposing();

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        Any & rConvertedValue, Any & rOldValue,
        sal_Int32 nHandle, Any const & rValue )
        throw (lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle, Any const & rValue ) throw (Exception);
    virtual void SAL_CALL getFastPropertyValue(
        Any & rValue, sal_Int32 nHandle ) const;

    virtual ~PackageManagerImpl();
public:
    PackageManagerImpl(
        Reference< XComponentContext > const & xComponentContext,
        OUString const & context );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XPackageManager
    virtual sal_Bool SAL_CALL addPackage(
        Reference< deployment::XPackage > & xPackage,
        OUString const & url,
        OUString const & mediaType,
        Reference< XCommandEnvironment > const & xCmdEnv )
        throw (deployment::DeploymentException, RuntimeException);
    virtual sal_Bool SAL_CALL removePackage(
        OUString const & name,
        Reference< XCommandEnvironment > const & xCmdEnv )
        throw (deployment::DeploymentException, RuntimeException);
    virtual Reference< deployment::XPackage > SAL_CALL getDeployedPackage(
        OUString const & name )
        throw (container::NoSuchElementException, RuntimeException);
    virtual sal_Bool SAL_CALL repair(
        Reference< XCommandEnvironment > const & xCmdEnv )
        throw (deployment::DeploymentException, RuntimeException);
};

//______________________________________________________________________________
inline OUString PackageManagerImpl::getDeployPath( OUString const & name ) const
{
    return make_url( m_packages_dir, ::rtl::Uri::encode(
                         name, rtl_UriCharClassPchar,
                         rtl_UriEncodeIgnoreEscapes,
                         RTL_TEXTENCODING_UTF8 ) );
}

//______________________________________________________________________________
inline OUString PackageManagerImpl::errmsg(
    OUString const & package, bool adding ) const
{
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[context=\"") );
    buf.append( m_context );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"] error while ") );
    if (adding)
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("adding package \"") );
    else
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("removing package \"") );
    buf.append( package );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
bool PackageManagerImpl::bindPackage_(
    Reference< deployment::XPackage > & xPackage,
    OUString const & name,
    Reference< XCommandEnvironment > const & xCmdEnv ) const
{
    OUString mediaType;
    // xxx todo: guarding
    if (m_persMediaTypes.get() != 0)
    {
        OSL_ASSERT( m_persMediaTypes->has( name ) );
        m_persMediaTypes->get( &mediaType, name );
    }
    return (m_xPackageRegistry->bindPackage(
                xPackage, getDeployPath(name), mediaType, xCmdEnv )
            != sal_False);
}

// OPropertySetHelper
//______________________________________________________________________________
::cppu::IPropertyArrayHelper & PackageManagerImpl::getInfoHelper()
{
    ::osl::MutexGuard guard( getMutex() );
    check();
    if (m_property_array_helper.get() == 0)
    {
        beans::Property props [] = {
            beans::Property(
                OUSTR("DeployedPackages") /* name */,
                DEPLOYED_PACKAGES /* handle */,
                ::getCppuType(
                    reinterpret_cast< Reference<
                    deployment::XPackageRegistry > const * >(0) ),
                beans::PropertyAttribute::READONLY |
                beans::PropertyAttribute::BOUND ),
            beans::Property(
                OUSTR("Registry") /* name */,
                REGISTRY /* handle */,
                ::getCppuType(
                    reinterpret_cast< Reference<
                    deployment::XPackageRegistry > const * >(0) ),
                beans::PropertyAttribute::READONLY )
        };
        m_property_array_helper.reset(
            new ::cppu::OPropertyArrayHelper( props, ARLEN(props) ) );
    }
    return *m_property_array_helper.get();
}

//______________________________________________________________________________
sal_Bool PackageManagerImpl::convertFastPropertyValue(
    Any & rConvertedValue, Any & rOldValue,
    sal_Int32 nHandle, Any const & rValue )
    throw (lang::IllegalArgumentException)
{
    check();
    OSL_ENSURE( 0, "unexpected!" );
    return false;
}

//______________________________________________________________________________
void PackageManagerImpl::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, Any const & rValue )
    throw (Exception)
{
    check();
    throw beans::PropertyVetoException(
        OUSTR("unexpected: only readonly properties!"),
        static_cast< OWeakObject * >(this) );
}

//______________________________________________________________________________
void PackageManagerImpl::getFastPropertyValue(
    Any & rValue, sal_Int32 nHandle ) const
{
    check();
    switch (nHandle)
    {
    case DEPLOYED_PACKAGES:
        try
        {
            rValue <<= const_cast< PackageManagerImpl * >(this)
                ->getDeployedPackages( Reference< XCommandEnvironment >() );
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw lang::WrappedTargetException(
                OUSTR("unexpected exception occured!"),
                static_cast< OWeakObject * >(
                    const_cast< PackageManagerImpl * >(this) ), exc );
        }
        break;
    case REGISTRY:
        rValue <<= m_xPackageRegistry;
        break;
    default:
       throw beans::UnknownPropertyException(
            OUSTR("unexpected property handle!"),
            static_cast< OWeakObject * >(
                const_cast< PackageManagerImpl * >(this) ) );
    }
}

//______________________________________________________________________________
PackageManagerImpl::PackageManagerImpl(
    Reference< XComponentContext > const & xComponentContext,
    OUString const & context )
    : m_xComponentContext( xComponentContext ),
      m_context( context ),
      m_register_packages( true )
{
    bool use_registry = true;
    OUString log_file, registry_cache, media_types;
    if (m_context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
    {
        m_packages_dir = OUSTR("vnd.sun.star.expand:$UNO_"
                               "USER_PACKAGES_CACHE/uno_packages");
        registry_cache = OUSTR("vnd.sun.star.expand:$UNO_"
                               "USER_PACKAGES_CACHE/registry");
        media_types = OUSTR("vnd.sun.star.expand:$UNO_"
                            "USER_PACKAGES_CACHE/media_types.db");
        log_file = OUSTR("vnd.sun.star.expand:$UNO_"
                         "USER_PACKAGES_CACHE/log.txt");
    }
    else if (m_context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
    {
        m_packages_dir = OUSTR("vnd.sun.star.expand:$UNO_"
                               "SHARED_PACKAGES_CACHE/uno_packages");
        registry_cache = OUSTR("vnd.sun.star.expand:$UNO_"
                               "SHARED_PACKAGES_CACHE/registry");
        media_types = OUSTR("vnd.sun.star.expand:$UNO_"
                            "SHARED_PACKAGES_CACHE/media_types.db");
        log_file = OUSTR("vnd.sun.star.expand:$UNO_"
                         "SHARED_PACKAGES_CACHE/log.txt");
    }
    else if (m_context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.pkg:///") ))
    {
        // deploy into document, but no registration:
        m_packages_dir = m_context;
        m_register_packages = false;
        // no log file?: xxx todo
    }
    else if (m_context.matchIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("vnd.sun.star.tdoc:/") ))
    {
        // deploy into transient document, including registration:
        m_packages_dir = m_context;
        // no log file?: xxx todo
    }
    else
    {
        throw lang::IllegalArgumentException(
            OUSTR("invalid context given: ") + m_context,
            Reference< XInterface >(), static_cast< sal_Int16 >(0) );
    }

    try
    {
        // assure packages folder is existing:
        create_folder( 0, m_packages_dir );

        if (log_file.getLength() > 0)
        {
            Any logfile( makeAny( log_file ) );
            m_xLogFile.set(
                m_xComponentContext->getServiceManager()
                ->createInstanceWithArgumentsAndContext(
                    OUSTR("com.sun.star.comp.deployment.ProgressLog"),
                    Sequence< Any >( &logfile, 1 ),
                    m_xComponentContext ),
                UNO_QUERY_THROW );
        }

        if (media_types.getLength() > 0)
        {
            // open media_types.db:
            m_persMediaTypes.reset(
                new PersistentMap( media_types, m_xComponentContext ) );
        }

        Sequence< Any > args;
        if (registry_cache.getLength() > 0)
        {
            create_folder( 0, registry_cache );
            args.realloc( 2 );
            args[ 0 ] <<= registry_cache;
            args[ 1 ] <<= m_context;
        }

        // xxx todo: create an own registry (with subsequent backends
        //           for any manager(context)?
        //           what happens if an embedded document is opened
        //           and modified?  Then a different manager with
        //           different registry will be instantiated...
        //           the other way round: instantiating via /singl/reg
        //           will globally register all scripts of open documents
        //           which might be not wanted, too.
        m_xPackageRegistry.set(
            m_xComponentContext->getServiceManager()
            ->createInstanceWithArgumentsAndContext(
                OUSTR("com.sun.star.deployment.PackageRegistry"),
                args, m_xComponentContext ), UNO_QUERY_THROW );
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
        buf.append( m_context );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             "\"] caught unexpected exception!") );
        throw lang::WrappedTargetRuntimeException(
            buf.makeStringAndClear(), Reference< XInterface >(), exc );
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
            static_cast< OWeakObject * >(this), exc );
    }
}

// XServiceInfo
//______________________________________________________________________________
OUString PackageManagerImpl::getImplementationName()
    throw (RuntimeException)
{
//     check();
    return OUSTR("com.sun.star.comp.deployment.PackageManager");
}

//______________________________________________________________________________
sal_Bool PackageManagerImpl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
//     check();
    Sequence< OUString > supported_services( getSupportedServiceNames() );
    OUString const * ar = supported_services.getConstArray();
    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return true;
    }
    return false;
}

//______________________________________________________________________________
Sequence< OUString > PackageManagerImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
//     check();
    OUString str_name = OUSTR("com.sun.star.comp.deployment.PackageManager");
    return Sequence< OUString >( &str_name, 1 );
}

// XPackageManager
//______________________________________________________________________________
sal_Bool PackageManagerImpl::addPackage(
    Reference< deployment::XPackage > & xPackage,
    OUString const & url, OUString const & mediaType_,
    Reference< XCommandEnvironment > const & xCmdEnv_ )
    throw (deployment::DeploymentException, RuntimeException)
{
    check();
    Reference< XCommandEnvironment > xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CommandEnvironmentImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    bool success = true;
    ProgressLevel progress( xCmdEnv );
    try
    {
        OUString mediaType( mediaType_ );
        if (mediaType.getLength() == 0)
        {
            // auto-detect media-type:
            mediaType = m_xPackageRegistry->detectMediaType( url );
            if (mediaType.getLength() == 0)
                throw Exception /* xxx todo: what exception */(
                    OUSTR("Cannot detect media-type of ") + url, 0 );
        }

        ::ucb::Content source_content;
        create_ucb_content( &source_content, url, xCmdEnv ); // throws exc
        OUString title( extract_throw< OUString >(
                            source_content.getPropertyValue(OUSTR("Title")) ) );
        OUString deploy_path( getDeployPath( title ) );

        {
        ::osl::MutexGuard guard( getMutex() );

        if (create_ucb_content( 0, deploy_path, xCmdEnv, false /* no throw */ ))
        {
            // interact whether pkgchk -f:
            handle_error(
                deployment::DeploymentException(
                    errmsg( source_content.getURL(), true /* adding */ ),
                    static_cast< OWeakObject * >(this),
                    makeAny( NameClashResolveRequest(
                                 OUSTR("package already added!"),
                                 static_cast< OWeakObject * >(this),
                                 task::InteractionClassification_ERROR,
                                 deploy_path, title, OUString() ) ) ),
                xCmdEnv, false /* no logging */ );
            // remove clashing package:
            if (! removePackage( title, xCmdEnv_ /* unwrapped cmd env */ ))
                return false;
        }

        // copy file:
        progress.update( OUSTR("copying package ") + title );
        ::ucb::Content dest_content( m_packages_dir, xCmdEnv );
        if (! dest_content.transferContent(
                source_content, ::ucb::InsertOperation_COPY,
                OUString(), NameClash::ASK ))
            throw RuntimeException( OUSTR("UCB transferContent() failed!"), 0 );

        if (m_persMediaTypes.get() != 0)
        {
            OSL_ASSERT( ! m_persMediaTypes->has( title ) );
            OUString inserted( mediaType );
            m_persMediaTypes->put( title, inserted );
            OSL_ASSERT( inserted.equals( mediaType ) );
        }
        else // document
        {
            dest_content.setPropertyValue(
                OUSTR("MediaType"), makeAny(mediaType) );
        }
        } // guard

        // xxx todo: fire before bind()?
        fireChange( DEPLOYED_PACKAGES, Any(), Any() /* performance resaons */ );

        // bind destination package:
        success &= (m_xPackageRegistry->bindPackage(
                        xPackage, deploy_path, mediaType, xCmdEnv )
                    != sal_False);
        // register package:
        if (success && m_register_packages && xPackage.is())
            success &= (xPackage->registerPackage( xCmdEnv ) != sal_False);
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (deployment::DeploymentException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        handle_error( deployment::DeploymentException(
                          errmsg( url, true /* adding */ ),
                          static_cast< OWeakObject * >(this), exc ),
                      xCmdEnv );
        success = false;
    }
    return success;
}

//______________________________________________________________________________
sal_Bool PackageManagerImpl::removePackage(
    OUString const & name,
    Reference< XCommandEnvironment > const & xCmdEnv_ )
    throw (deployment::DeploymentException, RuntimeException)
{
    check();
    Reference< XCommandEnvironment > xCmdEnv;
    if (m_xLogFile.is())
        xCmdEnv.set( new CommandEnvironmentImpl( xCmdEnv_, m_xLogFile ) );
    else
        xCmdEnv.set( xCmdEnv_ );

    bool success;
    try
    {
        OUString url( getDeployPath(name) );
        ProgressLevel progress( xCmdEnv );
        Reference< deployment::XPackage > xPackage;
        success = bindPackage_( xPackage, name, xCmdEnv );
        if (success && m_register_packages && xPackage.is())
        {
            // revoke package from registry:
            success &= (xPackage->revokePackage( xCmdEnv ) != sal_False);
            OSL_ASSERT(
                extract_throw< OUString >(
                    Reference< beans::XPropertySet >(
                        xPackage, UNO_QUERY_THROW )->getPropertyValue(
                            OUSTR("URL") ) ).equals( url ) );
            try_dispose( xPackage );
            if (m_persMediaTypes.get() != 0)
                m_persMediaTypes->erase( name );
        }
        // remove manager's copy:
        erase_path( url, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (deployment::DeploymentException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        handle_error( deployment::DeploymentException(
                          errmsg( name, false /* removing */ ),
                          static_cast< OWeakObject * >(this), exc ),
                      xCmdEnv );
        success = false;
    }
    return success;
}

//______________________________________________________________________________
Reference< deployment::XPackage > PackageManagerImpl::getDeployedPackage(
    OUString const & name )
    throw (container::NoSuchElementException, RuntimeException)
{
    OUString deployPath( getDeployPath( name ) );
    ::osl::MutexGuard guard( getMutex() );
    check();
    if (create_ucb_content(
            0, deployPath,
            Reference< XCommandEnvironment >(), false /* no throw */ ))
    {
        try
        {
            Reference< deployment::XPackage > xPackage;
            if (bindPackage_( xPackage, name ))
                return xPackage;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception &)
        {
            Any exc( ::cppu::getCaughtException() );
            throw lang::WrappedTargetRuntimeException(
                OUSTR("unexpected exception occured!"),
                static_cast< OWeakObject * >(this), exc );
        }
    }
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("[context=\"") );
    buf.append( m_context );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                         "\"] there is no such package deployed: \"") );
    buf.append( name );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
    throw container::NoSuchElementException(
        buf.makeStringAndClear(), static_cast< OWeakObject * >(this) );
}

//______________________________________________________________________________
Sequence< Reference< deployment::XPackage > >
PackageManagerImpl::getDeployedPackages(
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::std::vector< Reference< deployment::XPackage > > packages;
    // scan for all entries in m_packages_dir:
    ::ucb::Content ucb_content( m_packages_dir, xCmdEnv );
    OUString str_title = OUSTR("Title");
    Reference< sdbc::XResultSet > xResultSet(
        ucb_content.createCursor(
            Sequence< OUString >( &str_title, 1 ),
            ::ucb::INCLUDE_FOLDERS_AND_DOCUMENTS ) );
    while (xResultSet->next())
    {
        Reference< sdbc::XRow > xRow( xResultSet, UNO_QUERY_THROW );
        Reference< deployment::XPackage > xPackage;
        if (bindPackage_( xPackage, xRow->getString( 1 /* Title */ ) ) &&
            xPackage.is())
            packages.push_back( xPackage );
    }
    return Sequence< Reference< deployment::XPackage > >(
        &packages[ 0 ], packages.size() );
}

//______________________________________________________________________________
sal_Bool PackageManagerImpl::repair(
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    // repair registry:
    bool success = m_xPackageRegistry->repair( xCmdEnv );
    // verify added packages:
    Sequence< Reference< deployment::XPackage > > deployedPackages(
        getDeployedPackages( xCmdEnv ) );
    Reference< deployment::XPackage > const * pdeployedPackages =
        deployedPackages.getConstArray();
    for ( sal_Int32 pos = deployedPackages.getLength(); pos--; )
    {
        Reference< beans::XPropertySet > xProps(
            pdeployedPackages[ pos ], UNO_QUERY_THROW );
        if (! xProps->getPropertyValue( OUSTR("IsRegistered") ).hasValue())
        {
            // state unknown => try to register:
            success &= (pdeployedPackages[ pos ]->registerPackage( xCmdEnv )
                        != sal_False);
        }
    }
    return success;
}

//##############################################################################

namespace factory
{

typedef ::cppu::WeakComponentImplHelper2<
    lang::XServiceInfo, deployment::XPackageManagerFactory > t_pmfac_helper;

//==============================================================================
class PackageManagerFactoryImpl : private MutexHolder, public t_pmfac_helper
{
    Reference< XComponentContext > m_xComponentContext;

    Reference< deployment::XPackageManager > m_xUserMgr;
    Reference< deployment::XPackageManager > m_xSharedMgr;
    typedef ::std::hash_map<
        OUString, WeakReference< deployment::XPackageManager >,
        ::rtl::OUStringHash > t_string2weakref;
    t_string2weakref m_managers;

protected:
    inline void check() const;
    virtual void SAL_CALL disposing();

    virtual ~PackageManagerFactoryImpl();
public:
    PackageManagerFactoryImpl(
        Reference< XComponentContext > const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XPackageManagerFactory
    virtual Reference< deployment::XPackageManager > SAL_CALL getPackageManager(
        OUString const & context ) throw (RuntimeException);
};

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.PackageManagerFactory");
}

//==============================================================================
Sequence< OUString > SAL_CALL getSupportedServiceNames()
{
    OUString str_name = OUSTR("com.sun.star.deployment.PackageManagerFactory");
    return Sequence< OUString >( &str_name, 1 );
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new PackageManagerFactoryImpl( xComponentContext ) );
}

//==============================================================================
bool singleton_entries(
    Reference< registry::XRegistryKey > const & xRegistryKey )
{
    try
    {
        Reference< registry::XRegistryKey > xKey(
            xRegistryKey->createKey(
                getImplementationName() +
                OUSTR("/UNO/SINGLETONS/"
                      "com.sun.star.deployment.thePackageManagerFactory") ) );
        xKey->setStringValue(
            OUSTR("com.sun.star.deployment.PackageManagerFactory") );
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
    Reference< XComponentContext > const & xComponentContext )
    : t_pmfac_helper( getMutex() ),
      m_xComponentContext( xComponentContext )
{
}

//______________________________________________________________________________
PackageManagerFactoryImpl::~PackageManagerFactoryImpl()
{
}

//______________________________________________________________________________
inline void PackageManagerFactoryImpl::check() const
{
    // xxx todo guard?
//     ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("PackageManagerFactory instance has already been disposed!"),
            static_cast< OWeakObject * >(
                const_cast< PackageManagerFactoryImpl * >(this) ) );
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

// XServiceInfo
//______________________________________________________________________________
OUString PackageManagerFactoryImpl::getImplementationName()
    throw (RuntimeException)
{
//     check();
    return ::dp_manager::factory::getImplementationName();
}

//______________________________________________________________________________
sal_Bool PackageManagerFactoryImpl::supportsService(
    OUString const & serviceName )
    throw (RuntimeException)
{
//     check();
    Sequence< OUString > supported_services( getSupportedServiceNames() );
    OUString const * ar = supported_services.getConstArray();
    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (ar[ pos ].equals( serviceName ))
            return true;
    }
    return false;
}

//______________________________________________________________________________
Sequence< OUString > PackageManagerFactoryImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
//     check();
    return ::dp_manager::factory::getSupportedServiceNames();
}

// XPackageManagerFactory
//______________________________________________________________________________
Reference< deployment::XPackageManager >
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
    xRet.set( new PackageManagerImpl( m_xComponentContext, context ) );
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

