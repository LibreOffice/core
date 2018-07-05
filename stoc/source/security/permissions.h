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
#ifndef INCLUDED_STOC_SOURCE_SECURITY_PERMISSIONS_H
#define INCLUDED_STOC_SOURCE_SECURITY_PERMISSIONS_H

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace stoc_sec
{

class Permission : public ::salhelper::SimpleReferenceObject
{
public:
    ::rtl::Reference< Permission > m_next;
    // mode
    enum t_type { ALL, RUNTIME, SOCKET, FILE } m_type;

    Permission(
        t_type type,
        ::rtl::Reference< Permission > const & next )
        : m_next( next )
        , m_type( type )
        {}

    virtual bool implies( Permission const & perm ) const = 0;
    virtual OUString toString() const = 0;
};

class AllPermission : public Permission
{
public:
    explicit AllPermission(
        ::rtl::Reference< Permission > const & next = ::rtl::Reference< Permission >() )
        : Permission( ALL, next )
        {}

    virtual bool implies( Permission const & ) const override;
    virtual OUString toString() const override;
};


class PermissionCollection
{
    ::rtl::Reference< Permission > m_head;
public:
    PermissionCollection()
        {}
    explicit PermissionCollection( ::rtl::Reference< Permission > const & single )
        : m_head( single )
        {}
    PermissionCollection(
        css::uno::Sequence< css::uno::Any > const & permissions,
        PermissionCollection const & addition = PermissionCollection() );
#ifdef __DIAGNOSE
    css::uno::Sequence< OUString > toStrings() const;
#endif
    void checkPermission( css::uno::Any const & perm ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
