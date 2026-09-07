/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

namespace cpo::uno { class XComponentContext; }

namespace unotools::misc {

class ServiceDocumenter : public ::cppu::WeakImplHelper<
    css::script::XServiceDocumenter, css::lang::XServiceInfo>
{
    public:
        ServiceDocumenter(css::uno::Reference< cpo::uno::XComponentContext> xContext)
            : m_xContext(std::move(xContext))
            , m_sCoreBaseUrl(u"http://example.com"_ustr)
            , m_sServiceBaseUrl(u"https://api.libreoffice.org/docs/idl/ref"_ustr)
            {};

        //  XServiceInfo
        virtual bool supportsService(const OUString& sServiceName) override;
        virtual OUString getImplementationName() override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

        // XServiceDocumenter
        virtual OUString getCoreBaseUrl() override
            { return m_sCoreBaseUrl; };
        virtual void setCoreBaseUrl( const OUString& sCoreBaseUrl ) override
            { m_sCoreBaseUrl = sCoreBaseUrl; };
        virtual OUString getServiceBaseUrl() override
            { return m_sServiceBaseUrl; };
        virtual void setServiceBaseUrl( const OUString& sServiceBaseUrl ) override
            { m_sServiceBaseUrl = sServiceBaseUrl; };
        virtual void showServiceDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) override;
        virtual void showInterfaceDocs( const ::css::uno::Reference< ::css::lang::XTypeProvider >& xTypeProvider ) override;
        virtual void showCoreDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) override;
    protected:
        virtual ~ServiceDocumenter() override
            {};
    private:
        css::uno::Reference< cpo::uno::XComponentContext> m_xContext;
        OUString m_sCoreBaseUrl;
        OUString m_sServiceBaseUrl;
};

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
