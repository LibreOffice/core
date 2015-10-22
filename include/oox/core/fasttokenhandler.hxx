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

#ifndef INCLUDED_OOX_CORE_FASTTOKENHANDLER_HXX
#define INCLUDED_OOX_CORE_FASTTOKENHANDLER_HXX

#include <oox/dllapi.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <cppuhelper/implbase.hxx>
#include <sax/fastattribs.hxx>

namespace oox { class TokenMap; }

namespace oox {
namespace core {



/** Wrapper implementing the com.sun.star.xml.sax.XFastTokenHandler API interface
    that provides access to the tokens generated from the internal token name list.
 */
class OOX_DLLPUBLIC FastTokenHandler :
    public ::cppu::WeakImplHelper< css::lang::XServiceInfo,
                                   css::xml::sax::XFastTokenHandler >,
    public sax_fastparser::FastTokenHandlerBase
{
public:
    explicit            FastTokenHandler();
    virtual             ~FastTokenHandler();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XFastTokenHandler
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 nToken ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier ) throw (css::uno::RuntimeException, std::exception) override;

    // Much faster direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect( const char *pToken, sal_Int32 nLength ) const override;

private:
    const TokenMap&     mrTokenMap;     ///< Reference to global token map singleton.
};



} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
