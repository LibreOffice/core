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
#ifndef _TOOLS_TABLE_HXX
#define _TOOLS_TABLE_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/contnr.hxx>

// ---------
// - Table -
// ---------

#define TABLE_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class TOOLS_DLLPUBLIC Table : private Container
{
private:
    ULONG   nCount;
//#if 0 // _SOLAR__PRIVATE
    TOOLS_DLLPRIVATE ULONG  ImplGetIndex( ULONG nKey, ULONG* pIndex = NULL ) const;
//#endif
public:
            Table( USHORT nInitSize = 16, USHORT nReSize = 16 );
            Table( const Table& rTable ) : Container( rTable )
                { nCount = rTable.nCount; }

    BOOL    Insert( ULONG nKey, void* p );
    void*   Remove( ULONG nKey );
    void*   Replace( ULONG nKey, void* p );
    void*   Get( ULONG nKey ) const;

    void    Clear() { Container::Clear(); nCount = 0; }
    ULONG   Count() const { return( nCount ); }

    void*   GetCurObject() const;
    ULONG   GetCurKey() const { return (ULONG)Container::GetCurObject(); }
    ULONG   GetKey( const void* p ) const;
    BOOL    IsKeyValid( ULONG nKey ) const;

    void*   GetObject( ULONG nPos ) const
                { return Container::GetObject( (nPos*2)+1 ); }
    ULONG   GetObjectKey( ULONG nPos ) const
                { return (ULONG)Container::GetObject( nPos*2 ); }
    ULONG   GetUniqueKey( ULONG nStartKey = 1 ) const;
    ULONG   SearchKey( ULONG nKey, ULONG* pPos = NULL ) const;

    void*   Seek( ULONG nKey );
    void*   Seek( void* p );
    void*   First();
    void*   Last();
    void*   Next();
    void*   Prev();

    Table&  operator =( const Table& rTable );

    BOOL    operator ==( const Table& rTable ) const
                { return Container::operator ==( rTable ); }
    BOOL    operator !=( const Table& rTable ) const
                { return Container::operator !=( rTable ); }
};

inline Table& Table::operator =( const Table& r )
{
    Container::operator =( r );
    nCount = r.nCount;
    return *this;
}

// -----------------
// - DECLARE_TABLE -
// -----------------

#define DECLARE_TABLE( ClassName, Type )                                \
class ClassName : private Table                                         \
{                                                                       \
public:                                                                 \
                using Table::Clear;                                     \
                using Table::Count;                                     \
                using Table::GetCurKey;                                 \
                using Table::GetObjectKey;                              \
                using Table::GetUniqueKey;                              \
                using Table::SearchKey;                                 \
                using Table::IsKeyValid;                                \
                                                                        \
                ClassName( USHORT _nInitSize = 16,                      \
                           USHORT _nReSize = 16 ) :                     \
                    Table( _nInitSize, _nReSize ) {}                    \
                ClassName( const ClassName& rClassName ) :              \
                    Table( rClassName ) {}                              \
                                                                        \
    BOOL        Insert( ULONG nKey, Type p )                            \
                    { return Table::Insert( nKey, (void*)p ); }         \
    Type        Remove( ULONG nKey )                                    \
                    { return (Type)Table::Remove( nKey ); }             \
    Type        Replace( ULONG nKey, Type p )                           \
                    { return (Type)Table::Replace( nKey, (void*)p ); }  \
    Type        Get( ULONG nKey ) const                                 \
                    { return (Type)Table::Get( nKey ); }                \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)Table::GetCurObject(); }             \
    ULONG       GetKey( const Type p ) const                            \
                    { return Table::GetKey( (const void*)p ); }         \
                                                                        \
    Type        GetObject( ULONG nPos ) const                           \
                    { return (Type)Table::GetObject( nPos ); }          \
                                                                        \
    Type        Seek( ULONG nKey )                                      \
                    { return (Type)Table::Seek( nKey ); }               \
    Type        Seek( Type p )                                          \
                    { return (Type)Table::Seek( (void*)p ); }           \
    Type        First() { return (Type)Table::First(); }                \
    Type        Last()  { return (Type)Table::Last(); }                 \
    Type        Next()  { return (Type)Table::Next(); }                 \
    Type        Prev()  { return (Type)Table::Prev(); }                 \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { Table::operator =( rClassName );                  \
                      return *this; }                                   \
                                                                        \
    BOOL        operator ==( const ClassName& rTable ) const            \
                    { return Table::operator ==( rTable ); }            \
    BOOL        operator !=( const ClassName& rTable ) const            \
                    { return Table::operator !=( rTable ); }            \
};

#endif // _TOOLS_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
