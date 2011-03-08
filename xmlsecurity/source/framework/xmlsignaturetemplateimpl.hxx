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

#ifndef _XMLSIGNATURETEMPLATEIMPL_HXX_
#define _XMLSIGNATURETEMPLATEIMPL_HXX_

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

class XMLSignatureTemplateImpl : public ::cppu::WeakImplHelper3<
    ::com::sun::star::xml::crypto::XXMLSignatureTemplate ,
    ::com::sun::star::lang::XInitialization ,
    ::com::sun::star::lang::XServiceInfo >
{
    private :
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > m_xTemplate ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;
        std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > > targets;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XUriBinding > m_xUriBinding;
        ::com::sun::star::xml::crypto::SecurityOperationStatus m_nStatus;

    public :
        XMLSignatureTemplateImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~XMLSignatureTemplateImpl() ;

        //Methods from XXMLSignatureTemplate
        virtual void SAL_CALL setTemplate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >& aXmlElement
            ) throw( com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > SAL_CALL getTemplate(
        ) throw (com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setTarget(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper >& aXmlElement
        ) throw( com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException);

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::xml::wrapper::XXMLElementWrapper > > SAL_CALL getTargets(
        ) throw (com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setBinding(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::crypto::XUriBinding >& aUriBinding )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::crypto::XUriBinding >
            SAL_CALL getBinding(  )
            throw (::com::sun::star::uno::RuntimeException);

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

#endif  // _XMLSIGNATURETEMPLATE_XMLSECIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
