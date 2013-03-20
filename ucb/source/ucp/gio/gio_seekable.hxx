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

#pragma once
#if 1

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <gio/gio.h>

namespace gio
{

class Seekable : public ::com::sun::star::io::XTruncate,
    public ::com::sun::star::io::XSeekable,
    public ::cppu::OWeakObject
{
private:
    GSeekable *mpStream;
public:
    Seekable( GSeekable *pStream );
    virtual ~Seekable();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & type )
            throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire( void ) throw () { OWeakObject::acquire(); }
    virtual void SAL_CALL release( void ) throw() { OWeakObject::release(); }

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location )
            throw( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getPosition()
            throw( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getLength()
            throw( ::com::sun::star::io::IOException,
                ::com::sun::star::uno::RuntimeException );

    // XTruncate
    virtual void SAL_CALL truncate( void )
            throw( com::sun::star::io::IOException,
                com::sun::star::uno::RuntimeException );
};

} // namespace gio
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
