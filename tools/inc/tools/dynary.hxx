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

#ifndef _DYNARY_HXX
#define _DYNARY_HXX

#include <tools/solar.h>
#include <tools/contnr.hxx>

// ------------
// - DynArray -
// ------------

#define DYNARRAY_ENTRY_NOTFOUND     CONTAINER_ENTRY_NOTFOUND

class DynArray : private Container
{
public:
                using Container::SetSize;
                using Container::GetSize;
                using Container::Clear;

                DynArray( ULONG nSize = 16 ) : Container( nSize ) {}
                DynArray( const DynArray& rAry ) : Container( rAry ) {}

    void*       Put( ULONG nIndex, void* p )
                    { return Container::Replace( p, nIndex ); }
    void*       Get( ULONG nIndex ) const
                    { return Container::GetObject( nIndex ); }

    ULONG       GetIndex( const void* p ) const
                    { return Container::GetPos( p ); }
    ULONG       GetIndex( const void* p, ULONG nStartIndex,
                          BOOL bForward = TRUE ) const
                    { return Container::GetPos( p, nStartIndex, bForward ); }

    DynArray&   operator =( const DynArray& rAry )
                    { Container::operator =( rAry ); return *this; }

    BOOL        operator ==( const DynArray& rAry ) const
                    { return Container::operator ==( rAry ); }
    BOOL        operator !=( const DynArray& rAry ) const
                    { return Container::operator !=( rAry ); }
};

// --------------------
// - DECLARE_DYNARRAY -
// --------------------

#define DECLARE_DYNARRAY( ClassName, Type )                             \
class ClassName : private DynArray                                      \
{                                                                       \
public:                                                                 \
                using DynArray::SetSize;                                \
                using DynArray::GetSize;                                \
                using DynArray::Clear;                                  \
                                                                        \
                ClassName( ULONG nSize = 16 ) :                         \
                    DynArray( nSize ) {}                                \
                ClassName( const ClassName& rClassName ) :              \
                    DynArray( rClassName ) {}                           \
                                                                        \
    Type        Put( ULONG nIndex, Type p )                             \
                    { return (Type)DynArray::Put( nIndex, (void*)p ); } \
    Type        Get( ULONG nIndex ) const                               \
                    { return (Type)DynArray::Get( nIndex ); }           \
                                                                        \
    ULONG       GetIndex( const Type p ) const                          \
                    { return DynArray::GetIndex( (const void*)p ); }    \
    ULONG       GetIndex( const Type p, ULONG nStartIndex,              \
                          BOOL bForward = TRUE ) const                  \
                    { return DynArray::GetIndex( (const void*)p,        \
                                                 nStartIndex,           \
                                                 bForward ); }          \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { DynArray::operator =( rClassName );               \
                      return *this; }                                   \
                                                                        \
    BOOL        operator ==( const ClassName& rAry ) const              \
                    { return DynArray::operator ==( rAry ); }           \
    BOOL        operator !=( const ClassName& rAry ) const              \
                    { return DynArray::operator !=( rAry ); }           \
};

#endif // _DYNARY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
