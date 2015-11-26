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

#ifndef INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_XMLENCRYPTIONTEMPLATEIMPL_HXX
#define INCLUDED_XMLSECURITY_SOURCE_FRAMEWORK_XMLENCRYPTIONTEMPLATEIMPL_HXX

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/crypto/XXMLEncryptionTemplate.hpp>
#include <com/sun/star/xml/wrapper/XXMLElementWrapper.hpp>

class XMLEncryptionTemplateImpl : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLEncryptionTemplate ,
    css::lang::XServiceInfo >
{
    private:
        css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > m_xTemplate;
        css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > m_xTarget;
        css::xml::crypto::SecurityOperationStatus m_nStatus;

    public:
        explicit XMLEncryptionTemplateImpl();
        virtual ~XMLEncryptionTemplateImpl();

        //Methods from XXMLEncryptionTemplate
        virtual void SAL_CALL setTemplate(
            const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aXmlElement
            )
            throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;

        virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL getTemplate(
        ) throw (css::uno::RuntimeException, std::exception) override;

        virtual void SAL_CALL setTarget(
            const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aXmlElement
            ) throw( css::lang::IllegalArgumentException, std::exception) override;

        virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL getTarget(
        ) throw (css::uno::RuntimeException, std::exception) override ;

        virtual void SAL_CALL setStatus(
            css::xml::crypto::SecurityOperationStatus status )
            throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::xml::crypto::SecurityOperationStatus
            SAL_CALL getStatus(  )
            throw (css::uno::RuntimeException, std::exception) override;

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) throw( css::uno::RuntimeException, std::exception ) override ;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override ;

        //Helper for XServiceInfo
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        static OUString impl_getImplementationName() throw( css::uno::RuntimeException ) ;

        //Helper for registry
        static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) throw( css::uno::RuntimeException ) ;

        static css::uno::Reference< css::lang::XSingleServiceFactory > impl_createFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;
} ;

#endif  // _XMLENCRYPTIONTEMPLATE_XMLSECIMPL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
