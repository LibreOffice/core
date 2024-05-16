/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/XServiceDocumenter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <utility>

namespace com::sun::star::uno { class XComponentContext; }

namespace unotools::misc {

class ServiceDocumenter : public ::cppu::WeakImplHelper<
    css::script::XServiceDocumenter, css::lang::XServiceInfo>
{
    public:
        ServiceDocumenter(css::uno::Reference< css::uno::XComponentContext> xContext)
            : m_xContext(std::move(xContext))
            , m_sCoreBaseUrl(u"http://example.com"_ustr)
            , m_sServiceBaseUrl(u"https://api.libreoffice.org/docs/idl/ref"_ustr)
            {};

        //  XServiceInfo
        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XServiceDocumenter
        virtual OUString SAL_CALL getCoreBaseUrl() override
            { return m_sCoreBaseUrl; };
        virtual void SAL_CALL setCoreBaseUrl( const OUString& sCoreBaseUrl ) override
            { m_sCoreBaseUrl = sCoreBaseUrl; };
        virtual OUString SAL_CALL getServiceBaseUrl() override
            { return m_sServiceBaseUrl; };
        virtual void SAL_CALL setServiceBaseUrl( const OUString& sServiceBaseUrl ) override
            { m_sServiceBaseUrl = sServiceBaseUrl; };
        virtual void SAL_CALL showServiceDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) override;
        virtual void SAL_CALL showInterfaceDocs( const ::css::uno::Reference< ::css::lang::XTypeProvider >& xTypeProvider ) override;
        virtual void SAL_CALL showCoreDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) override;
    protected:
        virtual ~ServiceDocumenter() override
            {};
    private:
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
        OUString m_sCoreBaseUrl;
        OUString m_sServiceBaseUrl;
};

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
