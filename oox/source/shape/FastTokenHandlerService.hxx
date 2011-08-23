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

#ifndef OOX_SHAPE_FASTTOKENHANDLERSERVICE_HXX
#define OOX_SHAPE_FASTTOKENHANDLERSERVICE_HXX

#include <sal/config.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "oox/core/fasttokenhandler.hxx"

namespace css = ::com::sun::star;

namespace oox {
namespace shape {

class FastTokenHandlerService:
    public ::cppu::WeakImplHelper2<
        css::lang::XServiceInfo,
        css::xml::sax::XFastTokenHandler>
{
public:
    explicit FastTokenHandlerService(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier( ::sal_Int32 Token ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException);

private:
    FastTokenHandlerService(FastTokenHandlerService &); // not defined
    void operator =(FastTokenHandlerService &); // not defined

    virtual ~FastTokenHandlerService() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    ::oox::core::FastTokenHandler mFastTokenHandler;
};

::rtl::OUString SAL_CALL FastTokenHandlerService_getImplementationName();

css::uno::Sequence< ::rtl::OUString > SAL_CALL FastTokenHandlerService_getSupportedServiceNames();

css::uno::Reference< css::uno::XInterface > SAL_CALL _FastTokenHandlerService_create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
    SAL_THROW((css::uno::Exception));

}}
#endif // OOX_SHAPE_FAST_TOKEN_HANDLER_SERVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
