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

#ifndef INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
#define INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastTokenHandler.hpp"

namespace css = ::com::sun::star;

namespace writerfilter {
namespace ooxml
{

class OOXMLFastTokenHandler:
    public ::cppu::WeakImplHelper1<
        css::xml::sax::XFastTokenHandler>
{
public:
    explicit OOXMLFastTokenHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const ::rtl::OUString & Identifier) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier(::sal_Int32 Token) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException);

private:
    OOXMLFastTokenHandler(OOXMLFastTokenHandler &); // not defined
    void operator =(OOXMLFastTokenHandler &); // not defined

    virtual ~OOXMLFastTokenHandler() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

}}
#endif // INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
