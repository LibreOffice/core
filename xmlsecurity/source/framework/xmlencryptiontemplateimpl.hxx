/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlencryptiontemplateimpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:20:47 $
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

#ifndef _XMLENCRYPTIONTEMPLATEIMPL_HXX_
#define _XMLENCRYPTIONTEMPLATEIMPL_HXX_

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

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_CRYPTO_XXMLENCRYPTIONTEMPLATE_HPP_
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_WRAPPER_XXMLELEMENTWRAPPER_HPP_
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>
#endif

class XMLEncryptionTemplateImpl : public ::cppu::WeakImplHelper3<
    ::com::sun::star::xml::crypto::XXMLEncryptionTemplate ,
    ::com::sun::star::lang::XInitialization ,
    ::com::sun::star::lang::XServiceInfo >
{
    private :
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > m_xTemplate ;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > m_xTarget ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;
        ::com::sun::star::xml::crypto::SecurityOperationStatus m_nStatus;

    public :
        XMLEncryptionTemplateImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~XMLEncryptionTemplateImpl() ;

        //Methods from XXMLEncryptionTemplate
        virtual void SAL_CALL setTemplate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >& aXmlElement
            )
            throw (com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL getTemplate(
        ) throw (com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setTarget(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >& aXmlElement
            ) throw( com::sun::star::lang::IllegalArgumentException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL getTarget(
        ) throw (com::sun::star::uno::RuntimeException) ;

        virtual void SAL_CALL setStatus(
            ::com::sun::star::xml::crypto::SecurityOperationStatus status )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::xml::crypto::SecurityOperationStatus
            SAL_CALL getStatus(  )
            throw (::com::sun::star::uno::RuntimeException);

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

#endif  // _XMLENCRYPTIONTEMPLATE_XMLSECIMPL_HXX_
