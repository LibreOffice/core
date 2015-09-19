/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UNOTOOLS_INC_SERVICEDOCUMENTER_HXX
#define INCLUDED_UNOTOOLS_INC_SERVICEDOCUMENTER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/script/XServiceDocumenter.hpp>

namespace unotools { namespace misc {

class ServiceDocumenter : public ::cppu::WeakImplHelper<
    ::com::sun::star::script::XServiceDocumenter>
{
    public:
        ServiceDocumenter(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> const& xContext)
            : m_xContext(xContext)
            , m_sCoreBaseUrl("http://example.com")
            , m_sServiceBaseUrl("http://api.libreoffice.org/docs/idl/ref")
            {};
        // XServiceDocumenter
        virtual ::rtl::OUString SAL_CALL getCoreBaseUrl() /* throw (::css::uno::RuntimeException, ::std::exception) */ SAL_OVERRIDE
            { return m_sCoreBaseUrl; };
        virtual void SAL_CALL setCoreBaseUrl( const ::rtl::OUString& _corebaseurl ) /* throw (::css::uno::RuntimeException, ::std::exception) */ SAL_OVERRIDE
            { m_sCoreBaseUrl = sCoreBaseUrl; };
        virtual ::rtl::OUString SAL_CALL getServiceBaseUrl() /* throw (::css::uno::RuntimeException, ::std::exception) */ SAL_OVERRIDE
            { return m_sServiceBaseUrl; };
        virtual void SAL_CALL setServiceBaseUrl( const ::rtl::OUString& _servicebaseurl ) /* throw (::css::uno::RuntimeException, ::std::exception) */ SAL_OVERRIDE
            { m_sServiceBaseUrl = sServiceBaseUrl; };
        virtual void showServiceDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) SAL_OVERRIDE;
        virtual void showInterfaceDocs( const ::css::uno::Reference< ::css::lang::XTypeProvider >& xTypeProvider ) SAL_OVERRIDE;
        virtual void showCoreDocs( const ::css::uno::Reference< ::css::lang::XServiceInfo >& xService) SAL_OVERRIDE;
    protected:
        virtual ~ServiceDocumenter()
            {};
    private:
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
        OUString m_sCoreBaseUrl;
        OUString m_sServiceBaseUrl;
};

}}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
