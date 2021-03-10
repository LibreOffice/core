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

#include <oox/core/fasttokenhandler.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <oox/token/tokenmap.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

namespace oox::core {

using namespace ::com::sun::star::uno;

FastTokenHandler::FastTokenHandler() :
    mrTokenMap( StaticTokenMap::get() )
{
}

FastTokenHandler::~FastTokenHandler()
{
}

// XServiceInfo
OUString SAL_CALL FastTokenHandler::getImplementationName()
{
    return "com.sun.star.comp.oox.core.FastTokenHandler";
}

sal_Bool SAL_CALL FastTokenHandler::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL FastTokenHandler::getSupportedServiceNames()
{
    Sequence<OUString> aServiceNames { "com.sun.star.xml.sax.FastTokenHandler" };
    return aServiceNames;
}

Sequence< sal_Int8 > FastTokenHandler::getUTF8Identifier( sal_Int32 nToken )
{
    return mrTokenMap.getUtf8TokenName( nToken );
}

sal_Int32 FastTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& rIdentifier )
{
    return mrTokenMap.getTokenFromUtf8( rIdentifier );
}

sal_Int32 FastTokenHandler::getTokenDirect( const char *pToken, sal_Int32 nLength ) const
{
    return mrTokenMap.getTokenFromUTF8( pToken, nLength );
}

} // namespace oox::core

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_core_FastTokenHandler_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::core::FastTokenHandler());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
