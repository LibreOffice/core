/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: permissions.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 13:01:35 $
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
#ifndef _STOC_SEC_PERMISSIONS_H_
#define _STOC_SEC_PERMISSIONS_H_

#include <rtl/unload.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

extern ::rtl_StandardModuleCount g_moduleCount;

namespace stoc_sec
{
//==================================================================================================
class Permission : public ::salhelper::SimpleReferenceObject
{
public:
    ::rtl::Reference< Permission > m_next;
    // mode
    enum t_type { ALL, RUNTIME, SOCKET, FILE } m_type;

    inline Permission(
        t_type type,
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        SAL_THROW( () )
        : m_next( next )
        , m_type( type )
        {}

    virtual bool implies( Permission const & perm ) const SAL_THROW( () ) = 0;
    virtual ::rtl::OUString toString() const SAL_THROW( () ) = 0;
};
//==================================================================================================
class AllPermission : public Permission
{
public:
    inline AllPermission(
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        SAL_THROW( () )
        : Permission( ALL, next )
        {}

    virtual bool implies( Permission const & ) const SAL_THROW( () );
    virtual ::rtl::OUString toString() const SAL_THROW( () );
};

//==================================================================================================
class PermissionCollection
{
    ::rtl::Reference< Permission > m_head;
public:
    inline PermissionCollection() SAL_THROW( () )
        {}
    inline PermissionCollection( PermissionCollection const & collection ) SAL_THROW( () )
        : m_head( collection.m_head )
        {}
    inline PermissionCollection( ::rtl::Reference< Permission > const & single ) SAL_THROW( () )
        : m_head( single )
        {}
    PermissionCollection(
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const & permissions,
        PermissionCollection const & addition = PermissionCollection() )
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    ::com::sun::star::uno::Sequence< ::rtl::OUString > toStrings() const SAL_THROW( () );

    bool implies( Permission const & ) const SAL_THROW( () );
    void checkPermission( ::com::sun::star::uno::Any const & perm ) const
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
};

}

#endif
