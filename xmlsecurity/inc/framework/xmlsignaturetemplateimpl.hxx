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

#pragma once

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSignatureTemplate.hpp>

#include <vector>

namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::xml::wrapper { class XXMLElementWrapper; }

class XMLSignatureTemplateImpl final : public ::cppu::WeakImplHelper<
    css::xml::crypto::XXMLSignatureTemplate ,
    css::lang::XServiceInfo >
{
    private:
        css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > m_xTemplate ;
        std::vector< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > > targets;
        css::uno::Reference< css::xml::crypto::XUriBinding > m_xUriBinding;
        css::xml::crypto::SecurityOperationStatus m_nStatus;

    public:
        explicit XMLSignatureTemplateImpl();
        virtual ~XMLSignatureTemplateImpl() override;

        //Methods from XXMLSignatureTemplate
        virtual void SAL_CALL setTemplate(
            const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aXmlElement
            ) override;

        virtual css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > SAL_CALL getTemplate(
        ) override;

        virtual void SAL_CALL setTarget(
            const css::uno::Reference< css::xml::wrapper::XXMLElementWrapper >& aXmlElement
        ) override;

        virtual css::uno::Sequence< css::uno::Reference< css::xml::wrapper::XXMLElementWrapper > > SAL_CALL getTargets(
        ) override;

        virtual void SAL_CALL setBinding(
            const css::uno::Reference< css::xml::crypto::XUriBinding >& aUriBinding ) override;
        virtual css::uno::Reference< css::xml::crypto::XUriBinding >
            SAL_CALL getBinding(  ) override;

        virtual void SAL_CALL setStatus(
            css::xml::crypto::SecurityOperationStatus status ) override;
        virtual css::xml::crypto::SecurityOperationStatus
            SAL_CALL getStatus(  ) override;

        //Methods from XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override ;

        virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName
        ) override ;

        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override ;

        //Helper for XServiceInfo
        static css::uno::Sequence< OUString > impl_getSupportedServiceNames() ;

        /// @throws css::uno::RuntimeException
        static OUString impl_getImplementationName() ;

        //Helper for registry
        /// @throws css::uno::RuntimeException
        static css::uno::Reference< css::uno::XInterface > impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& aServiceManager ) ;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
