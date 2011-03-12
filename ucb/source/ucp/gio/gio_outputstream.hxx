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

#ifndef GIO_OUTPUTSTREAM_HXX
#define GIO_OUTPUTSTREAM_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include "gio_seekable.hxx"

namespace gio
{

class OutputStream :
    public ::com::sun::star::io::XOutputStream,
    public Seekable
{
private:
    GFileOutputStream *mpStream;

public:
    OutputStream ( GFileOutputStream *pStream );
    virtual ~OutputStream();

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface(const ::com::sun::star::uno::Type & type )
            throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL acquire( void ) throw () { OWeakObject::acquire(); }
    virtual void SAL_CALL release( void ) throw() { OWeakObject::release(); }

    // XOutputStream
   virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::BufferSizeExceededException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException);

   virtual void SAL_CALL flush( void )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::BufferSizeExceededException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException);


    virtual void SAL_CALL closeOutput( void )
           throw( com::sun::star::io::NotConnectedException,
                  com::sun::star::io::IOException,
                  com::sun::star::uno::RuntimeException );
};

} // namespace gio
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
