/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifdef __DIAGNOSE
    ::com::sun::star::uno::Sequence< ::rtl::OUString > toStrings() const SAL_THROW( () );
#endif
    void checkPermission( ::com::sun::star::uno::Any const & perm ) const
        SAL_THROW( (::com::sun::star::uno::RuntimeException) );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
