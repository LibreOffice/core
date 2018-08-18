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
#ifndef INCLUDED_CPPUHELPER_ACCESS_CONTROL_HXX
#define INCLUDED_CPPUHELPER_ACCESS_CONTROL_HXX

#include "cppuhelper/cppuhelperdllapi.h"
#include "com/sun/star/uno/Reference.hxx"

namespace com { namespace sun { namespace star { namespace security { class XAccessController; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace cppu
{

/** Helper class for retrieving access controller singleton from component context.
*/
class SAL_WARN_UNUSED CPPUHELPER_DLLPUBLIC AccessControl
{
    css::uno::Reference< css::security::XAccessController > m_xController;

public:
    /** Ctor.

        @param xContext component context to retrieve access controller singleton
    */
    AccessControl(
        css::uno::Reference< css::uno::XComponentContext > const & xContext );
    /** Ctor.

        @param xController access controller
    */
    AccessControl(
        css::uno::Reference< css::security::XAccessController > const & xController );
    /** Copy ctor.

        @param ac another object
    */
    AccessControl( ::cppu::AccessControl const & ac );

    /** Clears the access controller reference being used.
    */
    void SAL_CALL clear()
        { m_xController.clear(); }

    /** Returns access to the access controller reference being used.

        @return access controller
    */
    css::uno::Reference< css::security::XAccessController > const & SAL_CALL get() const
        { return m_xController; }

    /** Returns access to the access controller reference being used.

        @return access controller
    */
    css::security::XAccessController * SAL_CALL operator -> () const
        { return m_xController.get(); }


    /** A com.sun.star.security.RuntimePermission is for runtime permissions.
        A RuntimePermission contains a name (also referred to as a "target name") but no
        actions list; you either have the named permission or you don't.

        @param name name of permission
    */
    void SAL_CALL checkRuntimePermission(
        ::rtl::OUString const & name );

    /** A com.sun.star.io.FilePermission represents access to a file or directory.
        A FilePermission consists of a file url and a set of actions valid for that pathname.

        @param url file url
        @param actions actions list
    */
    void SAL_CALL checkFilePermission(
        ::rtl::OUString const & url,
        ::rtl::OUString const & actions );

    /** A com.sun.star.connection.SocketPermission represents access to a network via sockets.
        A SocketPermission consists of a host specification and a set of "actions"
        specifying ways to connect to that host.

        @param host host and optional portrange
        @param actions actions list
    */
    void SAL_CALL checkSocketPermission(
        ::rtl::OUString const & host,
        ::rtl::OUString const & actions );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
