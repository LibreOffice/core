/*************************************************************************
 *
 *  $RCSfile: dp_backend.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:07:59 $
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
#include "dp_ucb.h"
#include "dp_platform.h"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "svtools/inettype.hxx"
#include "com/sun/star/beans/XMultiPropertySet.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry
{
namespace backend
{

//______________________________________________________________________________
PackageRegistryBackend::~PackageRegistryBackend()
{
}

//______________________________________________________________________________
OUString PackageRegistryBackend::toString() const
{
    return const_cast< PackageRegistryBackend * >(
        this)->getImplementationName();
}

// XInitialization
//______________________________________________________________________________
void PackageRegistryBackend::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    check();
    if (args.getLength() < 1)
    {
        throw lang::IllegalArgumentException(
            OUSTR("expected at least PackageRegistry argument!"),
            static_cast< OWeakObject * >(this), static_cast< sal_Int16 >(0) );
    }
    m_xPackageRegistry.set( args[ 0 ], UNO_QUERY_THROW );

    if (args.getLength() > 1) // optional cache path
        m_cache_path = extract_throw< OUString >( args[ 1 ] );
}

//______________________________________________________________________________
void PackageRegistryBackend::disposing()
{
    try
    {
        m_xPackageRegistry.clear();
        m_xComponentContext.clear();
        PropertyComponentBase::disposing();
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught unexpected exception while disposing!"),
            static_cast< OWeakObject * >(this), exc );
    }
}

// OPropertySetHelper
//______________________________________________________________________________
::cppu::IPropertyArrayHelper & PackageRegistryBackend::getInfoHelper()
{
    ::osl::MutexGuard guard( getMutex() );
    check();
    if (m_property_array_helper.get() == 0)
    {
        beans::Property props [] = {
            beans::Property(
                OUSTR("SupportedMediaTypes") /* name */,
                SUPPORTED_MEDIA_TYPES /* handle */,
                ::getCppuType(
                    reinterpret_cast< Sequence<
                    OUString > const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY )
        };
        m_property_array_helper.reset(
            new ::cppu::OPropertyArrayHelper( props, ARLEN(props) ) );
    }
    return *m_property_array_helper.get();
}

//______________________________________________________________________________
sal_Bool PackageRegistryBackend::convertFastPropertyValue(
    Any & rConvertedValue, Any & rOldValue,
    sal_Int32 nHandle, Any const & rValue )
    throw (lang::IllegalArgumentException)
{
    check();
    OSL_ENSURE( 0, "unexpected!" );
    return false;
}

//______________________________________________________________________________
void PackageRegistryBackend::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, Any const & rValue )
    throw (Exception)
{
    check();
    throw beans::PropertyVetoException(
        OUSTR("unexpected: only one readonly property!"),
        static_cast< OWeakObject * >(this) );
}

//______________________________________________________________________________
void PackageRegistryBackend::getFastPropertyValue(
    Any & rValue, sal_Int32 nHandle ) const
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    switch (nHandle)
    {
    case SUPPORTED_MEDIA_TYPES:
    {
        rValue <<= m_supported_media_types;
        break;
    }
    default:
        throw beans::UnknownPropertyException(
            OUSTR("unexpected property handle!"),
            static_cast< OWeakObject * >(
                const_cast< PackageRegistryBackend * >(this) ) );
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
    Sequence< OUString > supported_services( getSupportedServiceNames() );
    OUString const * psupported_services = supported_services.getConstArray();
    for ( sal_Int32 pos = supported_services.getLength(); pos--; )
    {
        if (serviceName.equals( psupported_services[ pos ] ))
            return true;
    }
    return false;
}

Sequence< OUString > SAL_CALL getSupportedServiceNames();

//______________________________________________________________________________
Sequence< OUString > PackageRegistryBackend::getSupportedServiceNames()
    throw (RuntimeException)
{
//     check();
    return ::dp_registry::backend::getSupportedServiceNames();
}

//______________________________________________________________________________
void PackageRegistryBackend::bind_error(
    OUString const & url, Any const & cause,
    Reference< XCommandEnvironment > const & xCmdEnv )
{
    ::rtl::OUStringBuffer buf;
    buf.append( static_cast< sal_Unicode >('[') );
    buf.append( toString() );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] cannot bind package ") );
    buf.append( url );
    buf.append( static_cast< sal_Unicode >('!') );
    handle_error( deployment::DeploymentException(
                      buf.makeStringAndClear(),
                      static_cast< OWeakObject * >(this), cause ),
                  xCmdEnv );
}

// XPackageRegistry
//______________________________________________________________________________
sal_Bool PackageRegistryBackend::bindPackage(
    Reference< deployment::XPackage > & xPackage,
    OUString const & url, OUString const & mediaType,
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    try
    {
        UniString type, subType;
        INetContentTypeParameterList params;
        if (INetContentTypes::parse( mediaType, type, subType, &params ))
        {
            if (type.CompareIgnoreCaseToAscii(
                    RTL_CONSTASCII_STRINGPARAM("application") ) == 0)
            {
                // every backend parses for platform parameter:
                INetContentTypeParameter const * param = params.find(
                    ByteString( RTL_CONSTASCII_STRINGPARAM("platform") ) );
                if (param != 0 && !platform_fits( param->m_sValue ))
                    return true; // break here, ignore package

                ::osl::ResettableMutexGuard guard( getMutex() );
                t_string2weakref::const_iterator const iFind(
                    m_bound.find( url ) );
                if (iFind != m_bound.end())
                {
                    xPackage.set( iFind->second );
                    if (xPackage.is())
                        return true;
                }
                guard.clear();

                Reference< deployment::XPackage > xNewPackage(
                    createPackage( url, mediaType, subType, params, xCmdEnv ) );

                guard.reset();
                ::std::pair< t_string2weakref::iterator, bool > insertion(
                    m_bound.insert( t_string2weakref::value_type(
                                        url, xNewPackage ) ) );
                xPackage.set( insertion.first->second );
                if (! xPackage.is())
                {
                    insertion.first->second = xNewPackage;
                    guard.clear();
                    // listen for disposing events:
                    Reference< lang::XComponent > xComp(
                        xNewPackage, UNO_QUERY );
                    if (xComp.is())
                        xComp->addEventListener( this );
                    xPackage.set( xNewPackage );
                }
                return true;
            }
        }
        throw lang::IllegalArgumentException(
            OUSTR("invalid media-type given: ") + mediaType,
            static_cast< OWeakObject * >(this),
            static_cast< sal_Int16 >(-1 /* not known */) );
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
        bind_error( url, exc, xCmdEnv );
        return false;
    }
}

//______________________________________________________________________________
void PackageRegistryBackend::disposing( lang::EventObject const & event )
    throw (RuntimeException)
{
    try
    {
        Reference< beans::XPropertySet > xProps(
            event.Source, UNO_QUERY_THROW );
        OUString url( extract_throw< OUString >(
                          xProps->getPropertyValue( OUSTR("URL") ) ) );
        ::osl::MutexGuard guard( getMutex() );
        ::std::size_t erased = m_bound.erase( url );
        OSL_ASSERT( erased == 1 );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception & exc)
    {
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
    }
}

//______________________________________________________________________________
void PackageRegistryBackend::ensure_no_running_office() const
{
    if (office_is_running( getComponentContext() ))
    {
        ::rtl::OUStringBuffer buf;
        buf.append( static_cast< sal_Unicode >('[') );
        buf.append( toString() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") );
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM(
                "Cannot register/revoke live into a running Office process!") );
        throw RuntimeException(
            buf.makeStringAndClear(), static_cast< OWeakObject * >(
                const_cast< PackageRegistryBackend * >(this) ) );
    }
}

//______________________________________________________________________________
void PackageRegistryBackend::ensure_persistentMode() const
{
    if (transientMode())
    {
        ::rtl::OUStringBuffer buf;
        buf.append( static_cast< sal_Unicode >('[') );
        buf.append( toString() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] ") );
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("Cannot register/revoke transiently!") );
        throw RuntimeException(
            buf.makeStringAndClear(), static_cast< OWeakObject * >(
                const_cast< PackageRegistryBackend * >(this) ) );
    }
}

//##############################################################################

//______________________________________________________________________________
Package::~Package()
{
}

//______________________________________________________________________________
OUString Package::toString() const
{
    ::rtl::OUStringBuffer buf;
    buf.append( m_description );
    buf.append( static_cast< sal_Unicode >(' ') );
    buf.append( m_url );
    return buf.makeStringAndClear();
}

//______________________________________________________________________________
void Package::disposing()
{
    m_myBackend.clear();
    PropertyComponentBase::disposing();
}

// OPropertySetHelper
//______________________________________________________________________________
::cppu::IPropertyArrayHelper & Package::getInfoHelper()
{
    ::osl::MutexGuard guard( getMutex() );
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    if (m_property_array_helper.get() == 0)
    {
        beans::Property props [] = {
            beans::Property(
                OUSTR("Composition") /* name */,
                COMPOSITION /* handle */,
                ::getCppuType( reinterpret_cast<
                               Sequence< Reference< deployment::XPackage > >
                               const * >(0) ) /* type */,
                beans::PropertyAttribute::OPTIONAL |
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("Description") /* name */,
                DESCRIPTION /* handle */,
                ::getCppuType(
                    reinterpret_cast< OUString const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("DisplayName") /* name */,
                DISPLAY_NAME /* handle */,
                ::getCppuType(
                    reinterpret_cast< OUString const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("IsRegistered") /* name */,
                IS_REGISTERED /* handle */,
                ::getCppuType(
                    reinterpret_cast< bool const * >(0) ) /* type */,
                beans::PropertyAttribute::BOUND |
                beans::PropertyAttribute::MAYBEVOID |
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("MediaType") /* name */,
                MEDIA_TYPE /* handle */,
                ::getCppuType(
                    reinterpret_cast< OUString const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("Name") /* name */,
                NAME /* handle */,
                ::getCppuType(
                    reinterpret_cast< OUString const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY ),
            beans::Property(
                OUSTR("URL") /* name */,
                URL /* handle */,
                ::getCppuType(
                    reinterpret_cast< OUString const * >(0) ) /* type */,
                beans::PropertyAttribute::READONLY )
        };
        if (m_package_composition)
        {
            m_property_array_helper.reset(
                new ::cppu::OPropertyArrayHelper( props, ARLEN(props) ) );
        }
        else
        {
            m_property_array_helper.reset(
                new ::cppu::OPropertyArrayHelper(
                    props + 1, ARLEN(props) - 1 ) );
        }
    }
    return *m_property_array_helper.get();
}

//______________________________________________________________________________
void Package::getFastPropertyValue( Any & rValue, sal_Int32 nHandle ) const
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    switch (nHandle)
    {
    case DESCRIPTION:
        rValue <<= m_description;
        break;
    case DISPLAY_NAME:
        rValue <<= m_displayName;
        break;
    case IS_REGISTERED:
    {
        ::osl::ResettableMutexGuard guard( getMutex() );
        t_Registered reg = const_cast< Package * >(this)->getRegStatus(
            guard, 0 );
        if (reg == REG_VOID)
            rValue.clear();
        else
            rValue <<= (reg == REG_REGISTERED);
        break;
    }
    case NAME:
        rValue <<= m_name;
        break;
    case MEDIA_TYPE:
        rValue <<= m_mediaType;
        break;
    case URL:
        rValue <<= m_url;
        break;
    default:
        throw beans::UnknownPropertyException(
            OUSTR("unexpected handle!"),
            static_cast< OWeakObject * >( const_cast< Package * >(this) ) );
        break;
    }
}

//______________________________________________________________________________
sal_Bool Package::convertFastPropertyValue(
    Any & rConvertedValue, Any & rOldValue,
    sal_Int32 nHandle, Any const & rValue )
    throw (lang::IllegalArgumentException)
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    OSL_ENSURE( 0, "unexpected!" );
    return false;
}

//______________________________________________________________________________
void Package::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle, Any const & rValue )
    throw (Exception)
{
    // xxx todo: upon disposing() getPropertyValue( URL )...
//     check();
    throw beans::PropertyVetoException(
        OUSTR("unexpected: only readonly properties!"),
        static_cast< OWeakObject * >(this) );
}

//______________________________________________________________________________
void Package::fireIsRegistered(
    t_Registered newStatus_, t_Registered oldStatus_ )
{
    if (newStatus_ != oldStatus_)
    {
        Any newStatus;
        if (newStatus_ != REG_VOID)
            newStatus <<= (newStatus_ == REG_REGISTERED);
        Any oldStatus;
        if (oldStatus_ != REG_VOID)
            oldStatus <<= (oldStatus_ == REG_REGISTERED);
        fireChange( IS_REGISTERED, newStatus, oldStatus );
    }
}

//______________________________________________________________________________
void Package::fireCurrentIsRegistered(
    t_Registered oldStatus, Reference< XCommandEnvironment > const & xCmdEnv )
{
    t_Registered currentStatus = REG_VOID;
    try
    {
        ::osl::ResettableMutexGuard guard( getMutex() );
        currentStatus = getRegStatus( guard, xCmdEnv );
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception & exc)
    {
        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_UTF8 ) );
    }

    fireIsRegistered( currentStatus, oldStatus );
}

// XPackage
//______________________________________________________________________________
sal_Bool Package::registerPackage(
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    check();
    ProgressLevel progress( xCmdEnv );
    t_Registered oldStatus = REG_VOID;
    try
    {
        ::osl::ResettableMutexGuard guard( getMutex() );
        oldStatus = getRegStatus( guard, xCmdEnv );
        if (oldStatus == REG_REGISTERED || oldStatus == REG_VOID)
        {
            progress.update( m_url + OUSTR(" already registered.") );
            return true;
        }
        else
        {
            ::rtl::OUStringBuffer buf;
            buf.append( static_cast< sal_Unicode >('[') );
            buf.append( m_myBackend->toString() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] registering ") );
            buf.append( toString() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("...") );
            OUString msg( buf.makeStringAndClear() );
            progress.update( msg );

            bool success = processPackage(
                true /* registerPackage() */, guard, xCmdEnv );

            if (success)
                msg += OUSTR("registration success.");
            else
                msg += OUSTR("registration errors occured!");
            progress.update( msg );

            t_Registered newStatus = getRegStatus( guard, xCmdEnv );
            guard.clear();
            fireIsRegistered( newStatus, oldStatus );
            return success;
        }
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (deployment::DeploymentException &)
    {
        fireCurrentIsRegistered( oldStatus, xCmdEnv );
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        fireCurrentIsRegistered( oldStatus, xCmdEnv );
        handle_error( deployment::DeploymentException(
                          OUSTR("error while registering ") + toString(),
                          static_cast< OWeakObject * >(this), exc ), xCmdEnv );
        return false;
    }
}

//______________________________________________________________________________
sal_Bool Package::revokePackage(
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    check();
    ProgressLevel progress( xCmdEnv );
    t_Registered oldStatus = REG_VOID;
    try
    {
        ::osl::ResettableMutexGuard guard( getMutex() );
        oldStatus = getRegStatus( guard, xCmdEnv );
        if (oldStatus == REG_REGISTERED || oldStatus == REG_VOID)
        {
            ::rtl::OUStringBuffer buf;
            buf.append( static_cast< sal_Unicode >('[') );
            buf.append( m_myBackend->toString() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] revoking ") );
            buf.append( toString() );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("...") );
            OUString msg( buf.makeStringAndClear() );
            progress.update( msg );

            bool success = processPackage(
                false /* revokePackage() */, guard, xCmdEnv );

            if (success)
                msg += OUSTR("revocation success.");
            else
                msg+= OUSTR("revocation errors occured!");
            progress.update( msg );

            t_Registered newStatus = getRegStatus( guard, xCmdEnv );
            guard.clear();
            fireIsRegistered( newStatus, oldStatus );
            return success;
        }
        else
        {
            progress.update( m_url + OUSTR(" is not registered.") );
            return true;
        }
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (deployment::DeploymentException &)
    {
        fireCurrentIsRegistered( oldStatus, xCmdEnv );
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        fireCurrentIsRegistered( oldStatus, xCmdEnv );
        handle_error( deployment::DeploymentException(
                          OUSTR("error while revoking ") + toString(),
                          static_cast< OWeakObject * >(this), exc ), xCmdEnv );
        return false;
    }
}

}
}

