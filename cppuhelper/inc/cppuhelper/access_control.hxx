/*************************************************************************
 *
 *  $RCSfile: access_control.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-12-17 14:37:52 $
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
#ifndef _CPPUHELPER_SECURITY_HXX_
#define _CPPUHELPER_SECURITY_HXX_

#include <osl/diagnose.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/security/XAccessController.hpp>

#define AC_SINGLETON "/singletons/com.sun.star.security.theAccessController"


namespace cppu
{

//==================================================================================================
class AccessControl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::security::XAccessController > m_xController;

#ifdef SAL_W32
#pragma pack(push, 8)
#endif
    // binary comp. version of Permission
    struct __permission
    {
        rtl_uString * m_permType;
        rtl_uString * m_permTarget;
        rtl_uString * m_permActions;
    };
#ifdef SAL_W32
#pragma pack(pop)
#endif

public:
    inline AccessControl(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessController > const & xController )
        SAL_THROW( () );
    inline AccessControl(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    // checkPermission()
    inline void SAL_CALL checkPermission(
        ::com::sun::star::security::Permission const & perm )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
    inline void SAL_CALL checkPermission(
        ::rtl::OUString const & permType,
        ::rtl::OUString const & permTarget,
        ::rtl::OUString const & permActions )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    inline void SAL_CALL checkFilePermission(
        ::rtl::OUString const & permTarget,
        ::rtl::OUString const & permActions )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    // doRestricted()
    inline ::com::sun::star::uno::Any SAL_CALL doRestricted(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAction > const & xAction,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAccessControlContext > const & xRestriction )
        SAL_THROW( (::com::sun::star::uno::Exception) );
    // doPrivileged()
    inline ::com::sun::star::uno::Any SAL_CALL doPrivileged(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAction > const & xAction,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAccessControlContext > const & xRestriction )
        SAL_THROW( (::com::sun::star::uno::Exception) );
    // getContext()
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessControlContext > SAL_CALL getContext()
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
};
//__________________________________________________________________________________________________
inline AccessControl::AccessControl(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessController > const & xController )
    SAL_THROW( () )
    : m_xController( xController )
{
}
//__________________________________________________________________________________________________
inline AccessControl::AccessControl(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > const & xContext )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) )
{
    xContext->getValueByName(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(AC_SINGLETON) ) ) >>= m_xController;
}
//__________________________________________________________________________________________________
inline void AccessControl::checkPermission(
    ::com::sun::star::security::Permission const & perm )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) )
{
    if (m_xController.is())
    {
        m_xController->checkPermission( perm );
    }
}
//__________________________________________________________________________________________________
inline void AccessControl::checkPermission(
    ::rtl::OUString const & permType,
    ::rtl::OUString const & permTarget,
    ::rtl::OUString const & permActions )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) )
{
    __permission perm;
    perm.m_permType = permType.pData;
    perm.m_permTarget = permTarget.pData;
    perm.m_permActions = permActions.pData;
    checkPermission(
        * reinterpret_cast< ::com::sun::star::security::Permission const * >( &perm ) );
}
//__________________________________________________________________________________________________
inline void AccessControl::checkFilePermission(
    ::rtl::OUString const & permTarget,
    ::rtl::OUString const & permActions )
    SAL_THROW( (::com::sun::star::uno::RuntimeException) )
{
    ::rtl::OUString permType( RTL_CONSTASCII_USTRINGPARAM("java.io.FilePermission") );
    checkPermission( permType, permTarget, permActions );
}

//__________________________________________________________________________________________________
inline ::com::sun::star::uno::Any AccessControl::doRestricted(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAction > const & xAction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessControlContext > const & xRestriction )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    if (m_xController.is())
    {
        return m_xController->doRestricted( xAction, xRestriction );
    }
    else
    {
        return xAction->run();
    }
}
//__________________________________________________________________________________________________
inline ::com::sun::star::uno::Any AccessControl::doPrivileged(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAction > const & xAction,
    ::com::sun::star::uno::Reference<
        ::com::sun::star::security::XAccessControlContext > const & xRestriction )
    SAL_THROW( (::com::sun::star::uno::Exception) )
{
    if (m_xController.is())
    {
        return m_xController->doPrivileged( xAction, xRestriction );
    }
    else
    {
        return xAction->run();
    }
}
//__________________________________________________________________________________________________
inline ::com::sun::star::uno::Reference<
    ::com::sun::star::security::XAccessControlContext > AccessControl::getContext()
    SAL_THROW( (::com::sun::star::uno::RuntimeException) )
{
    if (m_xController.is())
    {
        return m_xController->getContext();
    }
    else
    {
        return ::com::sun::star::uno::Reference<
            ::com::sun::star::security::XAccessControlContext >();
    }
}

}

#endif
