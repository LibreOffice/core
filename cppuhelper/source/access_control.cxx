/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: access_control.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:39:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include <cppuhelper/access_control.hxx>

#include <com/sun/star/security/XAccessController.hpp>
#include <com/sun/star/security/RuntimePermission.hpp>
#include <com/sun/star/io/FilePermission.hpp>
#include <com/sun/star/connection/SocketPermission.hpp>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    inline OUString str_ac_singleton()
    {
        return OUSTR("/singletons/com.sun.star.security.theAccessController");
    }
}

namespace cppu
{
//__________________________________________________________________________________________________
AccessControl::AccessControl( Reference< XComponentContext > const & xContext )
    SAL_THROW( (RuntimeException) )
{
    if (! (xContext->getValueByName( str_ac_singleton() ) >>= m_xController))
    {
        throw SecurityException(
            OUSTR("no access controller!"), Reference< XInterface >() );
    }
}
//__________________________________________________________________________________________________
AccessControl::AccessControl(
    Reference< security::XAccessController > const & xController )
    SAL_THROW( (RuntimeException) )
    : m_xController( xController )
{
    if (! m_xController.is())
    {
        throw SecurityException(
            OUSTR("no access controller!"), Reference< XInterface >() );
    }
}
//__________________________________________________________________________________________________
AccessControl::AccessControl( AccessControl const & ac )
    SAL_THROW( (RuntimeException) )
    : m_xController( ac.m_xController )
{
    if (! m_xController.is())
    {
        throw SecurityException(
            OUSTR("no access controller!"), Reference< XInterface >() );
    }
}

#ifdef SAL_W32
#pragma pack(push, 8)
#endif
    // binary comp. to all Permission structs
    struct __permission
    {
        rtl_uString * m_str1;
        rtl_uString * m_str2;
    };
#ifdef SAL_W32
#pragma pack(pop)
#endif

//--------------------------------------------------------------------------------------------------
inline void __checkPermission(
    Reference< security::XAccessController > const & xController,
    Type const & type, rtl_uString * str1, rtl_uString * str2 )
    SAL_THROW( (RuntimeException) )
{
    __permission perm;
    perm.m_str1 = str1;
    perm.m_str2 = str2;

    uno_Any a;
    a.pType = type.getTypeLibType();
    a.pData = &perm;

    xController->checkPermission( * static_cast< Any * >( &a ) );
}
//__________________________________________________________________________________________________
void AccessControl::checkRuntimePermission(
    OUString const & name )
    SAL_THROW( (RuntimeException) )
{
    __checkPermission(
        m_xController,
        ::getCppuType( (security::RuntimePermission *)0 ), name.pData, 0 );
}
//__________________________________________________________________________________________________
void AccessControl::checkFilePermission(
    OUString const & url,
    OUString const & actions )
    SAL_THROW( (RuntimeException) )
{
    __checkPermission(
        m_xController,
        ::getCppuType( (io::FilePermission *)0 ), url.pData, actions.pData );
}
//__________________________________________________________________________________________________
void AccessControl::checkSocketPermission(
    OUString const & host,
    OUString const & actions )
    SAL_THROW( (RuntimeException) )
{
    __checkPermission(
        m_xController,
        ::getCppuType( (connection::SocketPermission *)0 ), host.pData, actions.pData );
}

}
