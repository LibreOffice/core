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


#include <cppuhelper/access_control.hxx>

#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

constexpr OUStringLiteral ACCESS_CONTROLLER_SINGLETON = u"/singletons/com.sun.star.security.theAccessController";

namespace cppu
{

AccessControl::AccessControl( Reference< XComponentContext > const & xContext )
{
    if (! (xContext->getValueByName( ACCESS_CONTROLLER_SINGLETON ) >>= m_xController))
    {
        throw SecurityException( u"no access controller!"_ustr );
    }
}

AccessControl::AccessControl(
    Reference< security::XAccessController > const & xController )
    : m_xController( xController )
{
    if (! m_xController.is())
    {
        throw SecurityException( u"no access controller!"_ustr );
    }
}

AccessControl::AccessControl( AccessControl const & ac )
    : m_xController( ac.m_xController )
{
    if (! m_xController.is())
    {
        throw SecurityException( u"no access controller!"_ustr );
    }
}

namespace {

#ifdef _WIN32
#pragma pack(push, 8)
#endif
    // binary comp. to all Permission structs
    struct permission
    {
        rtl_uString * m_str1;
        rtl_uString * m_str2;
    };
#ifdef _WIN32
#pragma pack(pop)
#endif

void checkPermission(
    Reference< security::XAccessController > const & xController,
    Type const & type, rtl_uString * str1, rtl_uString * str2 )
{
    permission perm;
    perm.m_str1 = str1;
    perm.m_str2 = str2;

    uno_Any a;
    a.pType = type.getTypeLibType();
    a.pData = &perm;

    xController->checkPermission( * static_cast< Any * >( &a ) );
}

}

void AccessControl::checkRuntimePermission(
    OUString const & name )
{
    checkPermission(
        m_xController,
        cppu::UnoType<security::RuntimePermission>::get(), name.pData, nullptr );
}

void AccessControl::checkFilePermission(
    OUString const & url,
    OUString const & actions )
{
    checkPermission(
        m_xController,
        cppu::UnoType<io::FilePermission>::get(), url.pData, actions.pData );
}

void AccessControl::checkSocketPermission(
    OUString const & host,
    OUString const & actions )
{
    checkPermission(
        m_xController,
        cppu::UnoType<connection::SocketPermission>::get(), host.pData, actions.pData );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
