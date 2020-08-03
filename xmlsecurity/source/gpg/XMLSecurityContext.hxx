/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>

#include <vector>

namespace com::sun::star::xml::crypto { class XSecurityEnvironment; }

class XMLSecurityContextGpg : public cppu::WeakImplHelper< css::xml::crypto::XXMLSecurityContext,
                                                           css::lang::XServiceInfo>
{
private:
    std::vector< css::uno::Reference< css::xml::crypto::XSecurityEnvironment > > m_vSecurityEnvironments;

    sal_Int32 m_nDefaultEnvIndex;

public:
    XMLSecurityContextGpg();
    virtual ~XMLSecurityContextGpg() override;

    // XXMLSecurityContext
    virtual sal_Int32 SAL_CALL addSecurityEnvironment(
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment) override;

    virtual ::sal_Int32 SAL_CALL getSecurityEnvironmentNumber() override;

    virtual css::uno::Reference<css::xml::crypto::XSecurityEnvironment > SAL_CALL getSecurityEnvironmentByIndex(sal_Int32 index) override;

    virtual css::uno::Reference<css::xml::crypto::XSecurityEnvironment > SAL_CALL getSecurityEnvironment() override;

    virtual sal_Int32 SAL_CALL getDefaultSecurityEnvironmentIndex() override;

    virtual void SAL_CALL setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override ;

    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override ;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override ;
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
