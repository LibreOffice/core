/*************************************************************************
 *
 *  $RCSfile: dp_registry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-04-13 12:08:09 $
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
#include "osl/diagnose.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/compbase3.hxx"
#include "ucbhelper/content.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/container/XContentEnumerationAccess.hpp"
#include "com/sun/star/deployment/XPackageRegistry.hpp"

#include <vector>
#include <hash_map>
#include <memory>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry
{

typedef ::cppu::WeakComponentImplHelper3<
    lang::XServiceInfo, lang::XInitialization,
    deployment::XPackageRegistry > t_helper;

//==============================================================================
class PackageRegistryImpl : private MutexHolder, public t_helper
{
    Reference< XComponentContext > m_xComponentContext;

    struct ci_string_hash
    {
        inline ::std::size_t operator () ( OUString const & str ) const
            { return str.toAsciiLowerCase().hashCode(); }
    };
    struct ci_string_equals
    {
        inline bool operator () (
            OUString const & str1, OUString const & str2 ) const
            { return str1.equalsIgnoreAsciiCase( str2 ); }
    };
    typedef ::std::hash_map<
        OUString,
        Reference< deployment::XPackageRegistry >,
        ci_string_hash, ci_string_equals > t_string2registry;
    t_string2registry m_media2backend;

    typedef ::std::vector<
        Reference< deployment::XPackageRegistry > > t_registryvec;
    t_registryvec m_backends;

protected:
    inline void check() const;
    virtual void SAL_CALL disposing();

    virtual ~PackageRegistryImpl();
public:
    inline PackageRegistryImpl(
        Reference< XComponentContext > const & xComponentContext )
        : t_helper( getMutex() ),
          m_xComponentContext( xComponentContext )
        {}

    // XInitialization
    void SAL_CALL initialize( Sequence< Any > const & args )
        throw (Exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

    // XPackageRegistry
    virtual sal_Bool SAL_CALL bindPackage(
        Reference< deployment::XPackage > & xPackage,
        OUString const & url,
        OUString const & mediaType,
        Reference< XCommandEnvironment > const & xCmdEnv )
        throw (deployment::DeploymentException, RuntimeException);
    virtual OUString SAL_CALL detectMediaType( OUString const & url )
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL repair(
        Reference< XCommandEnvironment > const & xCmdEnv )
        throw (deployment::DeploymentException, RuntimeException);
};

//______________________________________________________________________________
inline void PackageRegistryImpl::check() const
{
    // xxx todo guard?
//     ::osl::MutexGuard guard( getMutex() );
    if (rBHelper.bInDispose || rBHelper.bDisposed)
    {
        throw lang::DisposedException(
            OUSTR("PackageRegistry instance has already been disposed!"),
            static_cast< OWeakObject * >(
                const_cast< PackageRegistryImpl * >(this) ) );
    }
}

//______________________________________________________________________________
void PackageRegistryImpl::disposing()
{
    // dispose all registries:
    t_registryvec::const_iterator iPos( m_backends.begin() );
    t_registryvec::const_iterator const iEnd( m_backends.end() );
    for ( ; iPos != iEnd; ++iPos )
        try_dispose( *iPos );
    m_media2backend = t_string2registry();
    m_backends = t_registryvec();

    t_helper::disposing();
}

//______________________________________________________________________________
PackageRegistryImpl::~PackageRegistryImpl()
{
}

//______________________________________________________________________________
static OUString normalizeMediaType( OUString const & mediaType )
{
    ::rtl::OUStringBuffer buf;
    sal_Int32 index = 0;
    for (;;)
    {
        buf.append( mediaType.getToken( 0, '/', index ).trim() );
        if (index < 0)
            break;
        buf.append( static_cast< sal_Unicode >('/') );
    }
    return buf.makeStringAndClear();
}

// XInitialization
//______________________________________________________________________________
void PackageRegistryImpl::initialize( Sequence< Any > const & args )
    throw (Exception)
{
    check();
    OUString cache_path;
    if (args.getLength() > 0)
        cache_path = extract_throw< OUString >( args[ 0 ] );

    try
    {
        // auto-detect all registered package registries:
        Reference< container::XEnumeration > xEnum(
            Reference< container::XContentEnumerationAccess >(
                m_xComponentContext->getServiceManager(),
                UNO_QUERY_THROW )->createContentEnumeration(
                    OUSTR("com.sun.star.deployment.PackageRegistryBackend") ) );
        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                Any element( xEnum->nextElement() );
                Sequence< Any > registry_args( 1 );
                registry_args[ 0 ] <<=
                    Reference< deployment::XPackageRegistry >(this);
                if (cache_path.getLength() > 0)
                {
                    Reference< lang::XServiceInfo > xServiceInfo(
                        element, UNO_QUERY_THROW );
                    OUString registry_cache_path(
                        make_url( cache_path,
                                  ::rtl::Uri::encode(
                                      xServiceInfo->getImplementationName(),
                                      rtl_UriCharClassPchar,
                                      rtl_UriEncodeIgnoreEscapes,
                                      RTL_TEXTENCODING_UTF8 ) ) );
                    create_folder( 0, registry_cache_path );
                    registry_args.realloc( 1 + args.getLength() );
                    Any * pregistry_args = registry_args.getArray();
                    pregistry_args[ 1 ] <<= registry_cache_path;
                    Any const * pargs = args.getConstArray();
                    // append all args:
                    for ( sal_Int32 pos = 1; pos < args.getLength(); ++pos )
                        pregistry_args[ 1 + pos ] = pargs[ pos ];
                }

                Reference< deployment::XPackageRegistry > xBackend;
                Reference< lang::XSingleComponentFactory > xFac(
                    element, UNO_QUERY );
                if (xFac.is())
                {
                    xBackend.set(
                        xFac->createInstanceWithArgumentsAndContext(
                            registry_args, m_xComponentContext ), UNO_QUERY );
                }
                else
                {
                    Reference< lang::XSingleServiceFactory >
                        xSingleServiceFactory( element, UNO_QUERY_THROW );
                    xBackend.set(
                        xSingleServiceFactory->createInstanceWithArguments(
                            registry_args ), UNO_QUERY );
                }

                if (! xBackend.is())
                {
                    throw DeploymentException(
                        OUSTR("cannot instantiate PackageRegistryBackend "
                              "service: ") +
                        Reference< lang::XServiceInfo >(
                            element, UNO_QUERY_THROW )->getImplementationName(),
                        static_cast< OWeakObject * >(this) );
                }
                m_backends.push_back( xBackend );

                Sequence< OUString > mtypes(
                    extract_throw< Sequence< OUString > >(
                        Reference< beans::XPropertySet >(
                            xBackend, UNO_QUERY_THROW )->getPropertyValue(
                                OUSTR("SupportedMediaTypes") ) ) );

                OUString const * pmtypes = mtypes.getConstArray();
                sal_Int32 len = mtypes.getLength();
                for ( sal_Int32 pos = 0; pos < len; ++pos )
                {
                    OUString mediaType( normalizeMediaType( pmtypes[ pos ] ) );

                    ::std::pair< t_string2registry::iterator, bool >
                          mr_insertion(
                              m_media2backend.insert(
                                  t_string2registry::value_type(
                                      mediaType, xBackend ) ) );
#if OSL_DEBUG_LEVEL > 0
                    if (! mr_insertion.second)
                    {
                        ::rtl::OUStringBuffer buf;
                        buf.appendAscii(
                            RTL_CONSTASCII_STRINGPARAM(
                                "more than one PackageRegistryBackend for "
                                "media-type=\"") );
                        buf.append( mediaType );
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" => ") );
                        buf.append( Reference< lang::XServiceInfo >(
                                        xBackend, UNO_QUERY_THROW )->
                                    getImplementationName() );
                        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
                        OSL_ENSURE( 0, ::rtl::OUStringToOString(
                                        buf.makeStringAndClear(),
                                        RTL_TEXTENCODING_UTF8 ) );
                    }
#endif
                }
            }
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
            OUSTR("caught unexpected exception!"),
            Reference< XInterface >(), exc );
    }
}

//==============================================================================
OUString SAL_CALL getImplementationName()
{
    return OUSTR("com.sun.star.comp.deployment.PackageRegistry");
}

//==============================================================================
Sequence< OUString > SAL_CALL getSupportedServiceNames()
{
    OUString str_name = OUSTR("com.sun.star.deployment.PackageRegistry");
    return Sequence< OUString >( &str_name, 1 );
}

//==============================================================================
Reference< XInterface > SAL_CALL create(
    Reference< XComponentContext > const & xComponentContext )
    SAL_THROW( (Exception) )
{
    return static_cast< ::cppu::OWeakObject * >(
        new PackageRegistryImpl( xComponentContext ) );
}

// XServiceInfo
//______________________________________________________________________________
OUString PackageRegistryImpl::getImplementationName()
    throw (RuntimeException)
{
//     check();
    return ::dp_registry::getImplementationName();
}

//______________________________________________________________________________
sal_Bool PackageRegistryImpl::supportsService(
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
Sequence< OUString > PackageRegistryImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
//     check();
    return ::dp_registry::getSupportedServiceNames();
}

// XPackageRegistry
//______________________________________________________________________________
sal_Bool PackageRegistryImpl::bindPackage(
    Reference< deployment::XPackage > & xPackage,
    OUString const & url,
    OUString const & mediaType_,
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    try
    {
        OUString mediaType( mediaType_ );
        if (mediaType.getLength() == 0)
        {
            // auto-detect media-type:
            mediaType = detectMediaType( url );
            if (mediaType.getLength() == 0)
                throw Exception /* xxx todo: what exception */(
                    OUSTR("Cannot detect media-type of ") + url, 0 );
        }

        // get backend:
        t_string2registry::const_iterator iFind(
            m_media2backend.find( normalizeMediaType(mediaType) ) );
        if (iFind == m_media2backend.end())
        {
            // xxx todo: more sophisticated media-type argument parsing...
            sal_Int32 q = mediaType.indexOf( ';' );
            if (q >= 0)
            {
                iFind = m_media2backend.find(
                    normalizeMediaType(
                        /* cut parameters: */ mediaType.copy( 0, q ) ) );
            }
            if (iFind == m_media2backend.end())
            {
                throw container::NoSuchElementException(
                    OUSTR("No PackageRegistryBackend for media-type ") +
                    mediaType, static_cast< OWeakObject * >(this) );
            }
        }
        return (iFind->second->bindPackage(
                    xPackage, url, mediaType, xCmdEnv ) != sal_False);
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
        ::rtl::OUStringBuffer buf;
        buf.append( static_cast< sal_Unicode >('[') );
        buf.append( getImplementationName() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("] cannot bind package ") );
        buf.append( url );
        buf.append( static_cast< sal_Unicode >('!') );
        handle_error( deployment::DeploymentException(
                          buf.makeStringAndClear(),
                          static_cast< OWeakObject * >(this), exc ),
                      xCmdEnv );
        return false;
    }
}

//______________________________________________________________________________
OUString PackageRegistryImpl::detectMediaType( OUString const & url )
    throw (RuntimeException)
{
    try
    {
        ::ucb::Content ucb_content(
            url, Reference< XCommandEnvironment >() );

        OUString mediaType;
        try
        {
            ucb_content.getPropertyValue(
                OUSTR("MediaType") ) >>= mediaType;
        }
        catch (beans::UnknownPropertyException &)
        {
        }
        if (mediaType.getLength() == 0)
        {
            // iterator all backends:
            t_registryvec::const_iterator iPos( m_backends.begin() );
            t_registryvec::const_iterator const iEnd( m_backends.end() );
            for ( ; iPos != iEnd; ++iPos )
            {
                mediaType = (*iPos)->detectMediaType( url );
                if (mediaType.getLength() > 0)
                    break;
            }
        }
        return mediaType;
    }
    catch (RuntimeException &)
    {
        throw;
    }
    catch (Exception &)
    {
        Any exc( ::cppu::getCaughtException() );
        throw lang::WrappedTargetRuntimeException(
            OUSTR("caught exception while detecting media-type of ") + url,
            static_cast< OWeakObject * >(this), exc );
    }
}

//______________________________________________________________________________
sal_Bool PackageRegistryImpl::repair(
    Reference< XCommandEnvironment > const & xCmdEnv )
    throw (deployment::DeploymentException, RuntimeException)
{
    bool success = true;
    t_registryvec::const_iterator iPos( m_backends.begin() );
    t_registryvec::const_iterator const iEnd( m_backends.end() );
    for ( ; iPos != iEnd; ++iPos )
    {
        try
        {
            success &= ((*iPos)->repair( xCmdEnv ) != sal_False);
        }
        catch (deployment::DeploymentException & exc)
        {
            ::rtl::OUStringBuffer buf;
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("ERROR: ") );
            buf.append( exc.Message );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\nCause: ") );
            buf.append( ::comphelper::anyToString(exc.Cause) );
            ProgressLevel progress( xCmdEnv, buf.makeStringAndClear() );
            success = false;
        }
    }
    return success;
}

}

