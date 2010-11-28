/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "precompiled_xmlsecurity.hxx"
#include <certificatecontainer.hxx>

#include <sal/config.h>

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
    *aRet.getArray() = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.security.CertificateContainer"));
    return aRet;
}

//-------------------------------------------------------------------------

::rtl::OUString SAL_CALL
CertificateContainer::impl_getStaticImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.security.CertificateContainer"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
