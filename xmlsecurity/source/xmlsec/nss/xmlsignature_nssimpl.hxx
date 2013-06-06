/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _XMLSIGNATURE_NSSIMPL_HXX_
#define _XMLSIGNATURE_NSSIMPL_HXX_

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSignature.hpp>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

class XMLSignature_NssImpl : public ::cppu::WeakImplHelper2<
    ::com::sun::star::xml::crypto::XXMLSignature ,
    ::com::sun::star::lang::XServiceInfo >
{
    private :
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager ;

    public :
        XMLSignature_NssImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aFactory ) ;
        virtual ~XMLSignature_NssImpl() ;

        //Methods from XXMLSignature
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSignatureTemplate > SAL_CALL generate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aEnvironment
        ) throw( com::sun::star::xml::crypto::XMLSignatureException,
                 com::sun::star::uno::SecurityException) ;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSignatureTemplate > SAL_CALL validate(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSignatureTemplate >& aTemplate ,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XXMLSecurityContext >& aContext
        ) throw( com::sun::star::uno::RuntimeException,
                 com::sun::star::uno::SecurityException,
                 com::sun::star::xml::crypto::XMLSignatureException);

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( ::com::sun::star::uno::RuntimeException ) ;

        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for XServiceInfo
        static ::com::sun::star::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Helper for registry
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) throw( ::com::sun::star::uno::RuntimeException ) ;

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager ) ;
} ;

#endif  // _XMLSIGNATURE_NSSIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
