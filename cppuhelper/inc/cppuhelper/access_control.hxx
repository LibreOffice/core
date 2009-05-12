/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: access_control.hxx,v $
 * $Revision: 1.10 $
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
#ifndef _CPPUHELPER_ACCESS_CONTROL_HXX_
#define _CPPUHELPER_ACCESS_CONTROL_HXX_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/security/XAccessController.hpp>


namespace cppu
{

/** Helper class retriving access controller singleton from component context.
*/
class AccessControl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::security::XAccessController > m_xController;

public:
    /** Ctor.

        @param xContext component context to retrieve access controller singleton
    */
    AccessControl(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
    /** Ctor.

        @param xController access controller
    */
    AccessControl(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAccessController > const & xController )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
    /** Copy ctor.

        @param another object
    */
    AccessControl( ::cppu::AccessControl const & ac )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** Clears the access controller reference being used.
    */
    inline void SAL_CALL clear() SAL_THROW( () )
        { m_xController.clear(); }

    /** Returns access to the access controller reference being used.

        @return access controller
    */
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessController > const & SAL_CALL get() const SAL_THROW( () )
        { return m_xController; }

    /** Returns access to the access controller reference being used.

        @return access controller
    */
    inline ::com::sun::star::security::XAccessController * SAL_CALL operator -> () const SAL_THROW( () )
        { return m_xController.get(); }


    /** A com.sun.star.security.RuntimePermission is for runtime permissions.
        A RuntimePermission contains a name (also referred to as a "target name") but no
        actions list; you either have the named permission or you don't.

        @param name name of permission
    */
    void SAL_CALL checkRuntimePermission(
        ::rtl::OUString const & name )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** A com.sun.star.io.FilePermission represents access to a file or directory.
        A FilePermission consists of a file url and a set of actions valid for that pathname.

        @param url file url
        @param actions actions list
    */
    void SAL_CALL checkFilePermission(
        ::rtl::OUString const & url,
        ::rtl::OUString const & actions )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** A com.sun.star.connection.SocketPermission represents access to a network via sockets.
        A SocketPermission consists of a host specification and a set of "actions"
        specifying ways to connect to that host.

        @param host host and optional portrange
        @param actions actions list
    */
    void SAL_CALL checkSocketPermission(
        ::rtl::OUString const & host,
        ::rtl::OUString const & actions )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
};

}

#endif
