/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <ucbhelper/providerhelper.hxx>
#include <libcmis/libcmis.hxx>

namespace cmis
{

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
private:
    std::map< std::pair< OUString, OUString >, libcmis::Session* > m_aSessionCache;

public:
    explicit ContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ContentProvider() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XContentProvider
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
    queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

    libcmis::Session* getSession( const OUString& sBindingUrl, const OUString& sUsername );
    void registerSession( const OUString& sBindingUrl, const OUString& sUsername, libcmis::Session* pSession );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
