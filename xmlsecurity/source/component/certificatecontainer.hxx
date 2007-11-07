/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: certificatecontainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:05:57 $
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

#ifndef _XCERTIFICATECONTAINER_HXX_
#define _XCERTIFICATECONTAINER_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _XCERTIFICATECONTAINER_HPP_
#include <com/sun/star/security/XCertificateContainer.hpp>
#endif

#ifndef _CERTIFICATECONTAINERSTATUS_HPP_
#include <com/sun/star/security/CertificateContainerStatus.hpp>
#endif


#include <vector>
#include <map>

using namespace com::sun::star;
using namespace cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class CertificateContainer : public ::cppu::WeakImplHelper2< ::com::sun::star::lang::XServiceInfo, ::com::sun::star::security::XCertificateContainer >
{
    private:
        typedef std::map< ::rtl::OUString, ::rtl::OUString > Map;
        Map certMap;
        Map certTrustMap;

        ::sal_Bool SAL_CALL searchMap( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, Map &_certMap  );
        virtual ::sal_Bool SAL_CALL isTemporaryCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL isCertificateTrust( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);

    public:

        CertificateContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) {};
        virtual ~CertificateContainer(){};

        virtual ::sal_Bool SAL_CALL addCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name, ::sal_Bool trust ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::security::CertificateContainerStatus SAL_CALL hasCertificate( const ::rtl::OUString & url, const ::rtl::OUString & certificate_name ) throw(::com::sun::star::uno::RuntimeException);
        // provide factory
    static ::rtl::OUString SAL_CALL
                                        impl_getStaticImplementationName( ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                    impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
                    impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
                                        getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL
                                        supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

};



#endif

