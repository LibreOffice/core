/*************************************************************************
 *
 *  $RCSfile: x509certificate_mscryptimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mmi $ $Date: 2004-07-15 08:12:09 $
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

#ifndef _X509CERTIFICATE_MSCRYPTIMPL_HXX_
#define _X509CERTIFICATE_MSCRYPTIMPL_HXX_

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SECURITYEXCEPTION_HPP_
#include <com/sun/star/uno/SecurityException.hpp>
#endif

#ifndef _COM_SUN_STAR_SECURITY_XCERTIFICATE_HPP_
#include <com/sun/star/security/XCertificate.hpp>
#endif

#include "Windows.h"
#include "WinCrypt.h"

class X509Certificate_MSCryptImpl : public ::cppu::WeakImplHelper2<
    ::com::sun::star::security::XCertificate ,
    ::com::sun::star::lang::XUnoTunnel >
{
    private :
        const CERT_CONTEXT* m_pCertContext ;

    public :
        X509Certificate_MSCryptImpl() ;
        virtual ~X509Certificate_MSCryptImpl() ;

        //Methods from XCertificate
        virtual sal_Int16 SAL_CALL getVersion() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSerialNumber() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getIssuerName() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getSubjectName() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::util::DateTime SAL_CALL getNotBefore() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::util::DateTime SAL_CALL getNotAfter() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getIssuerUniqueID() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectUniqueID() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > > SAL_CALL getExtensions() throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificateExtension > SAL_CALL findCertExtension( const ::com::sun::star::uno::Sequence< sal_Int8 >& oid ) throw (::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getEncoded() throw ( ::com::sun::star::uno::RuntimeException) ;

        // MM : added by MM
        virtual ::rtl::OUString SAL_CALL getSubjectPublicKeyAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSubjectPublicKeyValue()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::rtl::OUString SAL_CALL getSignatureAlgorithm()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getSHA1Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException) ;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getMD5Thumbprint()
            throw ( ::com::sun::star::uno::RuntimeException) ;
        // MM : end

        //Methods from XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (com::sun::star::uno::RuntimeException);

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;
        static X509Certificate_MSCryptImpl* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xObj ) ;

        //Helper methods
        void setMswcryCert( const CERT_CONTEXT* cert ) ;
        const CERT_CONTEXT* getMswcryCert() const ;
        void setRawCert( ::com::sun::star::uno::Sequence< sal_Int8 > rawCert ) throw ( ::com::sun::star::uno::RuntimeException) ;
} ;

#endif  // _X509CERTIFICATE_MSCRYPTIMPL_HXX_

