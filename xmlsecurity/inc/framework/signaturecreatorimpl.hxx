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

#include <com/sun/star/xml/crypto/sax/XBlockerMonitor.hpp>
#include <com/sun/star/xml/crypto/sax/XSignatureCreationResultBroadcaster.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <framework/signatureengine.hxx>

namespace com::sun::star::xml::crypto { class XSecurityEnvironment; }
namespace com::sun::star::xml::crypto::sax { class XSignatureCreationResultListener;}

typedef cppu::ImplInheritanceHelper
<
    SignatureEngine,
    css::xml::crypto::sax::XBlockerMonitor,
    css::xml::crypto::sax::XSignatureCreationResultBroadcaster,
    css::lang::XInitialization,
    css::lang::XServiceInfo
> SignatureCreatorImpl_Base;

class SignatureCreatorImpl final : public SignatureCreatorImpl_Base
/****** SignatureCreatorImpl.hxx/CLASS SignatureCreatorImpl *******************
 *
 *   NAME
 *  SignatureCreatorImpl -- generates a signature
 *
 *   FUNCTION
 *  Collects all resources for a signature generation, then generates the
 *  signature by invoking a xmlsec-based signature bridge component.
 ******************************************************************************/
{
private:
    /*
     * the Id of template blocker.
     */
    sal_Int32 m_nIdOfBlocker;

    css::uno::Reference< css::xml::crypto::XSecurityEnvironment > m_xSecurityEnvironment;

    virtual void notifyResultListener() const override;
    virtual void clearUp( ) const override;
    virtual void startEngine( const rtl::Reference<XMLSignatureTemplateImpl>& xSignatureTemplate) override;

public:
    explicit SignatureCreatorImpl();
    virtual ~SignatureCreatorImpl() override;

    /* XBlockerMonitor */
    virtual void SAL_CALL setBlockerId( sal_Int32 id ) override;

    /* XSignatureCreationResultBroadcaster */
    void SAL_CALL addSignatureCreationResultListener(
        const css::uno::Reference< css::xml::crypto::sax::XSignatureCreationResultListener >& listener ) override;

    void SAL_CALL removeSignatureCreationResultListener(
        const css::uno::Reference< css::xml::crypto::sax::XSignatureCreationResultListener >& listener ) override;

    /* XInitialization */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    /* XServiceInfo */
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

/// @throws css::uno::RuntimeException
OUString SignatureCreatorImpl_getImplementationName();

/// @throws css::uno::RuntimeException
css::uno::Sequence< OUString > SignatureCreatorImpl_getSupportedServiceNames(  );


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
