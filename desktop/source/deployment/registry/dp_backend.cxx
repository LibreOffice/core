/*************************************************************************
 *
 *  $RCSfile: dp_backend.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:09:42 $
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

#include "dp_backend.h"
#include "dp_registry.hrc"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"


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
    ::std::size_t erased = m_bound.erase( url );
    OSL_ASSERT( erased == 1 );
}

//______________________________________________________________________________
PackageRegistryBackend::PackageRegistryBackend(
    Sequence<Any> const & args,
    Reference<XComponentContext> const & xContext,
    OUString const & implName,
    Sequence<OUString> const & supportedMediaTypes )
    : t_BackendBase( getMutex() ),
      m_xComponentContext( xContext ),
      m_implName( implName ),
      m_supportedMediaTypes( supportedMediaTypes ),
      m_eContext( CONTEXT_UNKNOWN ),
      m_readOnly( false ),
      m_strCannotDetectMediaType(
          getResourceString(RID_STR_CANNOT_DETECT_MEDIA_TYPE) ),
      m_strUnsupportedMediaType(
          getResourceString(RID_STR_UNSUPPORTED_MEDIA_TYPE) ),
      m_strRegisteringPackage(
          getResourceString(RID_STR_REGISTERING_PACKAGE) ),
      m_strRevokingPackage(
          getResourceString(RID_STR_REVOKING_PACKAGE) )
{
    extract_throw( &m_context, args[ 0 ] );
    if (args.getLength() > 1) {
        extract_throw( &m_cachePath, args[ 1 ] );
        if (args.getLength() > 2)
            extract_throw( &m_readOnly, args[ 2 ] );
    }

    if (m_context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("user") ))
        m_eContext = CONTEXT_USER;
    else if (m_context.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("shared") ))
        m_eContext = CONTEXT_SHARED;
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
        m_xComponentContext.clear();
        WeakComponentImplHelperBase::disposing();
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing!"),
            static_cast<OWeakObject *>(this), exc );
    }
}

// XServiceInfo
//______________________________________________________________________________
OUString PackageRegistryBackend::getImplementationName()
    throw (RuntimeException)
{
//     check();
    return m_implName;
}

//______________________________________________________________________________
sal_Bool PackageRegistryBackend::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
//     check();
    Sequence<OUString> supported_services( getSupportedServiceNames() );
    OUString const * psupported_services = supported_services.getConstArray();
    for ( sal_Int32 pos = supported_services.getLength(); pos--; ) {
        if (serviceName.equals( psupported_services[ pos ] ))
            return true;
    }
    return false;
}

Sequence<OUString> SAL_CALL getSupportedServiceNames();

//______________________________________________________________________________
Sequence<OUString> PackageRegistryBackend::getSupportedServiceNames()
    throw (RuntimeException)
{
//     check();
    return ::dp_registry::backend::getSupportedServiceNames();
}

// XPackageRegistry
//______________________________________________________________________________
Reference<deployment::XPackage> PackageRegistryBackend::bindPackage(
    OUString const & url, OUString const & mediaType,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException, CommandFailedException,
           lang::IllegalArgumentException, RuntimeException)
{
    ::osl::ResettableMutexGuard guard( getMutex() );
    check();
    t_string2weakref::const_iterator const iFind( m_bound.find( url ) );
    if (iFind != m_bound.end()) {
        Reference<deployment::XPackage> xPackage( iFind->second );
        if (xPackage.is())
            return xPackage;
    }
    guard.clear();

    Reference<deployment::XPackage> xNewPackage;
    try {
        xNewPackage = bindPackage_( url, mediaType, xCmdEnv );
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (lang::IllegalArgumentException &) {
        throw;
    }
    catch (CommandFailedException &) {
        throw;
    }
    catch (deployment::DeploymentException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw deployment::DeploymentException(
            OUSTR("Error binding package: ") + url,
            static_cast<OWeakObject *>(this), exc );
    }

    guard.reset();
    ::std::pair< t_string2weakref::iterator, bool > insertion(
        m_bound.insert( t_string2weakref::value_type( url, xNewPackage ) ) );
    if (insertion.second)
    { // first insertion
        OSL_ASSERT( Reference<XInterface>(insertion.first->second)
                    == xNewPackage );
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

//______________________________________________________________________________
Sequence<OUString> PackageRegistryBackend::getSupportedMediaTypes()
    throw (RuntimeException)
{
    return m_supportedMediaTypes;
}

//##############################################################################

//______________________________________________________________________________
Package::~Package()
{
}

//______________________________________________________________________________
Package::Package( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
                  OUString const & url,
                  OUString const & mediaType,
                  OUString const & name,
                  OUString const & displayName,
                  OUString const & description )
    : t_PackageBase( getMutex() ),
      m_myBackend( myBackend ),
      m_url( url ),
      m_mediaType( mediaType ),
      m_name( name ),
      m_displayName( displayName ),
      m_description( description )
{
}

//______________________________________________________________________________
void Package::disposing()
{
    m_myBackend.clear();
    WeakComponentImplHelperBase::disposing();
}

//______________________________________________________________________________
void Package::check()
{
    ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed) {
        throw lang::DisposedException(
            OUSTR("Package instance has already been disposed!"),
            static_cast<OWeakObject *>(this) );
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
    if (abortChannel.is() && abortChannel->isAborted())
        throw CommandAbortedException(
            OUSTR("abort!"), static_cast<OWeakObject *>(this) );
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
Sequence< Reference<deployment::XPackage> > Package::getBundle(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
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

//______________________________________________________________________________
OUString Package::getMediaType() throw (RuntimeException)
{
    return m_mediaType;
}

//______________________________________________________________________________
OUString Package::getURL() throw (RuntimeException)
{
    return m_url;
}

//______________________________________________________________________________
OUString Package::getDisplayName() throw (RuntimeException)
{
    return m_displayName;
}

//______________________________________________________________________________
OUString Package::getDescription() throw (RuntimeException)
{
    return m_description;
}

//______________________________________________________________________________
Any Package::getIcon(
    sal_Bool highContrast, sal_Bool smallIcon ) throw (RuntimeException)
{
    return Any();
}

//______________________________________________________________________________
void Package::exportTo(
    OUString const & destFolderURL, OUString const & newTitle,
    sal_Int32 nameClashAction, Reference<XCommandEnvironment> const & xCmdEnv )
    throw (CommandFailedException, CommandAbortedException, RuntimeException)
{
    ::ucb::Content destFolder( destFolderURL, xCmdEnv );
    ::ucb::Content sourceContent( getURL(), xCmdEnv );
    if (! destFolder.transferContent(
            sourceContent, ::ucb::InsertOperation_COPY,
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
        for ( sal_Int32 pos = 0; pos < elements.getLength(); ++pos ) {
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
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        throw;
    }
    catch (CommandAbortedException &) {
        throw;
    }
    catch (deployment::DeploymentException &) {
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        throw deployment::DeploymentException(
            OUSTR("unexpected exception occured!"),
            static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
void Package::registerPackage(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    check();
    ProgressLevel progress( xCmdEnv );

    try {
        ::osl::ResettableMutexGuard guard( getMutex() );
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            isRegistered_( guard, AbortChannel::get(xAbortChannel), xCmdEnv ) );
        if (option.IsPresent &&
            (option.Value.IsAmbiguous || !option.Value.Value)) {
            OUString displayName( getDisplayName() );
            progress.update(
                m_myBackend->m_strRegisteringPackage + displayName );
            processPackage_( guard,
                             true /* registerPackage() */,
                             AbortChannel::get(xAbortChannel),
                             xCmdEnv );
            guard.clear();
            fireModified();
        }
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        fireModified();
        throw;
    }
    catch (CommandAbortedException &) {
        fireModified();
        throw;
    }
    catch (deployment::DeploymentException &) {
        fireModified();
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        fireModified();
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_REGISTERING) +
            getDisplayName(), static_cast<OWeakObject *>(this), exc );
    }
}

//______________________________________________________________________________
void Package::revokePackage(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    check();
    ProgressLevel progress( xCmdEnv );

    try {
        ::osl::ResettableMutexGuard guard( getMutex() );
        beans::Optional< beans::Ambiguous<sal_Bool> > option(
            isRegistered_( guard, AbortChannel::get(xAbortChannel), xCmdEnv ) );
        if (option.IsPresent &&
            (option.Value.IsAmbiguous || option.Value.Value))
        {
            OUString displayName( getDisplayName() );
            progress.update( m_myBackend->m_strRevokingPackage + displayName );
            processPackage_( guard,
                             false /* revokePackage() */,
                             AbortChannel::get(xAbortChannel),
                             xCmdEnv );
            guard.clear();
            fireModified();
        }
    }
    catch (RuntimeException &) {
        throw;
    }
    catch (CommandFailedException &) {
        fireModified();
        throw;
    }
    catch (CommandAbortedException &) {
        fireModified();
        throw;
    }
    catch (deployment::DeploymentException &) {
        fireModified();
        throw;
    }
    catch (Exception &) {
        Any exc( ::cppu::getCaughtException() );
        fireModified();
        throw deployment::DeploymentException(
            getResourceString(RID_STR_ERROR_WHILE_REVOKING) + getDisplayName(),
            static_cast<OWeakObject *>(this), exc );
    }
}

}
}

