/*************************************************************************
 *
 *  $RCSfile: access_control.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2002-01-11 10:06:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CPPUHELPER_ACCESS_CONTROL_HXX_
#define _CPPUHELPER_ACCESS_CONTROL_HXX_

#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SECURITY_XACCESSCONTROLLER_HPP_
#include <com/sun/star/security/XAccessController.hpp>
#endif

#define AC_SINGLETON "/singletons/com.sun.star.security.theAccessController"


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

    /** Returns access to the access controller reference being used.

        @return access controller
    */
    inline ::com::sun::star::uno::Reference< ::com::sun::star::security::XAccessController > const &
    get() SAL_THROW( () )
        { return m_xController; }


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


/** Default implementation retieving the current access control restriction.

    @param xCurrentContext current context
    @return current access control restriction
*/
::com::sun::star::uno::Reference< ::com::sun::star::security::XAccessControlContext > SAL_CALL
ac_defimpl_getRestriction(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XCurrentContext > const & xCurrentContext )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) );

/** Default implementation performing a restricted action.

    @param xAction action to be performed
    @param xRestriction additional restriction to be in effect
    @param xCurrentContext current context
*/
::com::sun::star::uno::Any SAL_CALL
ac_defimpl_doRestricted(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAction > const & xAction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessControlContext > const & xRestriction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XCurrentContext > const & xCurrentContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

/** Default implementation performed a privileged action.

    @param xAction action to be performed
    @param xRestriction (the one and only) restriction to be in effect
    @param xCurrentContext current context
*/
::com::sun::star::uno::Any SAL_CALL
ac_defimpl_doPrivileged(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAction > const & xAction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessControlContext > const & xRestriction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XCurrentContext > const & xCurrentContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif
