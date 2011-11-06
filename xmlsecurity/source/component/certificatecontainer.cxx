/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

