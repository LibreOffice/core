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

#ifndef INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX
#define INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/xml/sax/XFastTokenHandler.hpp"
#include "sax/fastattribs.hxx"

namespace writerfilter {
namespace ooxml
{

class OOXMLFastTokenHandler:
    public ::cppu::WeakImplHelper1<
        css::xml::sax::XFastTokenHandler>,
    public sax_fastparser::FastTokenHandlerBase
{
public:
    explicit OOXMLFastTokenHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::xml::sax::XFastTokenHandler:
    virtual ::sal_Int32 SAL_CALL getToken(const OUString & Identifier) throw (css::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getIdentifier(::sal_Int32 Token) throw (css::uno::RuntimeException, std::exception);
    virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getUTF8Identifier(::sal_Int32 Token) throw (css::uno::RuntimeException, std::exception);
    virtual ::sal_Int32 SAL_CALL getTokenFromUTF8(const css::uno::Sequence< ::sal_Int8 > & Identifier) throw (css::uno::RuntimeException, std::exception);

    // Much faster direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect( const char *pToken, sal_Int32 nLength ) const;

private:
    OOXMLFastTokenHandler(OOXMLFastTokenHandler &); // not defined
    void operator =(OOXMLFastTokenHandler &); // not defined

    virtual ~OOXMLFastTokenHandler() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

}}
#endif // INCLUDED_OOXML_FAST_TOKEN_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
