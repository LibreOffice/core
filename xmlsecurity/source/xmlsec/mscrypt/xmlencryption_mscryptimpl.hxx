/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlencryption_mscryptimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:31:48 $
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

#ifndef _XMLENCRYPTION_MSCRYPTIMPL_HXX_
#define _XMLENCRYPTION_MSCRYPTIMPL_HXX_

#ifndef _SAL_CONFIG_H_
#include <sal/config.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HPP_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSECVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLENCRYPTION_HPP_
#include <com/sun/star/xml/crypto/XXMLEncryption.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLENCRYPTIONTEMPLATE_HPP_
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLSECURITYCONTEXT_HPP_
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif

class XMLEncryption_MSCryptImpl : public ::cppu::WeakImplHelper3<
    ::com::sun::star::xml::crypto::XXMLEncryption ,
    ::com::sun::star::lang::XInitialization ,
    ::com::sun::star::lang::XServiceInfo >
{
    private :
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;

    public :
        XMLEncryption_MSCryptImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~XMLEncryption_MSCryptImpl() ;

        //Methods from XXMLEncryption
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLEncryptionTemplate > SAL_CALL encrypt(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLEncryptionTemplate >& aTemplate ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aEnvironment)
            // ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;
            throw ( com::sun::star::xml::crypto::XMLEncryptionException ,
                    com::sun::star::uno::SecurityException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLEncryptionTemplate > SAL_CALL decrypt(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLEncryptionTemplate >& aTemplate ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSecurityContext >& aContext
        ) throw( com::sun::star::xml::crypto::XMLEncryptionException ,
                    com::sun::star::uno::SecurityException) ;

        //Methods from XInitialization
        virtual void SAL_CALL initialize(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments
        ) throw( ::com::sun::star::uno::Exception , ::com::sun::star::uno::RuntimeException ) ;

        //Methods from XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual sal_Bool SAL_CALL supportsService(
            const ::rtl::OUString& ServiceName
        ) throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for XServiceInfo
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames() ;

        static ::rtl::OUString impl_getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for registry
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) throw( ::com::sun::star::uno::RuntimeException ) ;

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) ;
} ;

#endif  // _XMLENCRYPTION_MSCRYPTIMPL_HXX_

