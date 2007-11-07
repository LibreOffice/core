/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificatecontainer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:05:41 $
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

#include "precompiled_xmlsecurity.hxx"
#include <certificatecontainer.hxx>

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

using namespace ::com::sun::star::uno;


sal_Bool
CertificateContainer::searchMap( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, Map &_certMap )
{
    Map::iterator p = _certMap.find(url);

    ::sal_Bool ret = sal_False;

    while( p != _certMap.end() )
    {
        ret = (sal_Bool) (*p).second.equals(certificate_name);
        if( ret )
                    break;
        p++;
    }

    return ret;
}
// -------------------------------------------------------------------

sal_Bool
CertificateContainer::isTemporaryCertificate ( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name )
    throw(::com::sun::star::uno::RuntimeException)
{
    return searchMap( url, certificate_name, certMap);
}

// -------------------------------------------------------------------

sal_Bool
CertificateContainer::isCertificateTrust ( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name )
    throw(::com::sun::star::uno::RuntimeException)
{
    return searchMap( url, certificate_name, certTrustMap);
}

// -------------------------------------------------------------------
sal_Bool
CertificateContainer::addCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, ::sal_Bool trust )
    throw(::com::sun::star::uno::RuntimeException)
{
    certMap.insert( Map::value_type( url, certificate_name ) );

        //remember that the cert is trusted
        if (trust)
            certTrustMap.insert( Map::value_type( url, certificate_name ) );

        return true;
}

//-------------------------------------------------------------------------
::security::CertificateContainerStatus
CertificateContainer::hasCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( isTemporaryCertificate( url, certificate_name ) )
    {
        if ( isCertificateTrust( url, certificate_name ) )
            return security::CertificateContainerStatus( security::CertificateContainerStatus_TRUSTED );
        else
            return security::CertificateContainerStatus_UNTRUSTED;
    } else
    {
        return security::CertificateContainerStatus_NOCERT;
    }
}
//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL
CertificateContainer::getImplementationName( )
    throw(::com::sun::star::uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------

sal_Bool SAL_CALL
CertificateContainer::supportsService( const ::rtl::OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.security.CertificateContainer") == 0 )
        return sal_True;
    else
        return sal_False;
}

//-------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL
CertificateContainer::getSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

//-------------------------------------------------------------------------

Sequence< ::rtl::OUString > SAL_CALL
CertificateContainer::impl_getStaticSupportedServiceNames(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(1);
    *aRet.getArray() = ::rtl::OUString::createFromAscii("com.sun.star.security.CertificateContainer");
    return aRet;
}

//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL
CertificateContainer::impl_getStaticImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.security.CertificateContainer");
}

//-------------------------------------------------------------------------

Reference< XInterface > SAL_CALL CertificateContainer::impl_createInstance( const Reference< XMultiServiceFactory >& xServiceManager )
    throw( RuntimeException )
{
    return Reference< XInterface >( *new CertificateContainer( xServiceManager ) );
}

//-------------------------------------------------------------------------

Reference< XSingleServiceFactory > SAL_CALL
CertificateContainer::impl_createFactory( const Reference< XMultiServiceFactory >& ServiceManager )
    throw(RuntimeException)
{
    Reference< XSingleServiceFactory > xReturn( ::cppu::createOneInstanceFactory( ServiceManager,
        CertificateContainer::impl_getStaticImplementationName(),
        CertificateContainer::impl_createInstance,
        CertificateContainer::impl_getStaticSupportedServiceNames()));

    return xReturn;
}

