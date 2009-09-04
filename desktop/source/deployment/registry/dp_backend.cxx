/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_backend.cxx,v $
 * $Revision: 1.20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_backend.h"
#include "dp_ucb.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/servicedecl.hxx"
#include "comphelper/unwrapargs.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/beans/StringPair.hpp"


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
        OSL_ASSERT( false );
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

//##############################################################################

//______________________________________________________________________________
Package::~Package()
{
}

//______________________________________________________________________________
Package::Package( ::rtl::Reference<PackageRegistryBackend> const & myBackend,
                  OUString const & url,
                  OUString const & name,
                  OUString const & displayName,
                  Reference<deployment::XPackageTypeInfo> const & xPackageType )
    : t_PackageBase( getMutex() ),
      m_myBackend( myBackend ),
      m_url( url ),
      m_name( name ),
      m_displayName( displayName ),
      m_xPackageType( xPackageType )
{
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
::sal_Bool Package::checkPrerequisites(
        const css::uno::Reference< css::task::XAbortChannel >&,
        const css::uno::Reference< css::ucb::XCommandEnvironment >&,
        sal_Bool, ::rtl::OUString const &)
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::ucb::CommandAbortedException,
            css::uno::RuntimeException)
{
    return true;
}

//______________________________________________________________________________
::sal_Bool Package::checkDependencies(
        const css::uno::Reference< css::ucb::XCommandEnvironment >& )
        throw (css::deployment::DeploymentException,
            css::ucb::CommandFailedException,
            css::uno::RuntimeException)
{
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
    return beans::Optional<OUString>();
}

//______________________________________________________________________________
OUString Package::getVersion() throw (RuntimeException)
{
    return OUString();
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
    return OUString();
}

//______________________________________________________________________________
Sequence<OUString> Package::getUpdateInformationURLs() throw (RuntimeException)
{
    return Sequence<OUString>();
}

//______________________________________________________________________________
css::beans::StringPair Package::getPublisherInfo() throw (RuntimeException)
{
    css::beans::StringPair aEmptyPair;
    return aEmptyPair;
}

//______________________________________________________________________________
uno::Reference< css::graphic::XGraphic > Package::getIcon( sal_Bool /*bHighContrast*/ ) throw ( RuntimeException )
{
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
    throw (CommandFailedException, CommandAbortedException, RuntimeException)
{
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
void Package::processPackage_impl(
    bool doRegisterPackage,
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
                OUString displayName( getDisplayName() );
                ProgressLevel progress(
                    xCmdEnv,
                    (doRegisterPackage
                     ? PackageRegistryBackend::StrRegisteringPackage::get()
                     : PackageRegistryBackend::StrRevokingPackage::get())
                    + displayName );
                processPackage_( guard,
                                 doRegisterPackage,
                                 AbortChannel::get(xAbortChannel),
                                 xCmdEnv );
            }
        }
        catch (RuntimeException &) {
            OSL_ENSURE( 0, "### unexpected RuntimeException!" );
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
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    processPackage_impl( true /* register */, xAbortChannel, xCmdEnv );
}

//______________________________________________________________________________
void Package::revokePackage(
    Reference<task::XAbortChannel> const & xAbortChannel,
    Reference<XCommandEnvironment> const & xCmdEnv )
    throw (deployment::DeploymentException,
           CommandFailedException, CommandAbortedException,
           lang::IllegalArgumentException, RuntimeException)
{
    processPackage_impl( false /* revoke */, xAbortChannel, xCmdEnv );
}

//##############################################################################

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
OUString Package::TypeInfo::getDescription() throw (RuntimeException)
{
    return getShortDescription();
}

//______________________________________________________________________________
OUString Package::TypeInfo::getShortDescription() throw (RuntimeException)
{
    return m_shortDescr;
}

//______________________________________________________________________________
OUString Package::TypeInfo::getFileFilter() throw (RuntimeException)
{
    return m_fileFilter;
}

//______________________________________________________________________________
Any Package::TypeInfo::getIcon( sal_Bool highContrast, sal_Bool smallIcon )
    throw (RuntimeException)
{
    if (! smallIcon)
        return Any();
    const sal_uInt16 nIconId = (highContrast ? m_smallIcon_HC : m_smallIcon);
    return Any( &nIconId, getCppuType( static_cast<sal_uInt16 const *>(0) ) );
}

}
}

