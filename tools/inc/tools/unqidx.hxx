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
#ifndef _UNQIDX_HXX
#define _UNQIDX_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>
#include <tools/contnr.hxx>

// ---------------
// - UniqueIndex -
// ---------------

#define UNIQUEINDEX_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class TOOLS_DLLPUBLIC UniqueIndex : private Container
{
private:
    ULONG           nReSize;
    ULONG           nStartIndex;
    ULONG           nUniqIndex;
    ULONG           nCount;

public:
                    using Container::GetCurObject;

                    UniqueIndex( ULONG nStartIndex = 0,
                                 ULONG nInitSize = 16,
                                 ULONG nReSize = 16 );
                    UniqueIndex( const UniqueIndex& rIdx );

    ULONG           Insert( ULONG nIndex, void* p );
    ULONG           Insert( void* p );
    void*           Remove( ULONG nIndex );
    void*           Replace( ULONG nIndex, void* p );
    void*           Get( ULONG nIndex ) const;

    void            Clear();
    ULONG           Count() const { return nCount; }

    ULONG           GetCurIndex() const;
    ULONG           GetIndex( const void* p ) const;
    BOOL            IsIndexValid( ULONG nIndex ) const;

    void*           Seek( ULONG nIndex );
    void*           Seek( void* p );
    void*           First();
    void*           Last();
    void*           Next();
    void*           Prev();

    ULONG           GetStartIndex() const { return nStartIndex; }
    ULONG           GetCurMaxIndex() const
                        { return (nStartIndex + Container::GetSize()); }

    UniqueIndex&    operator =( const UniqueIndex& rIdx );

    BOOL            operator ==( const UniqueIndex& rIdx ) const;
    BOOL            operator !=( const UniqueIndex& rIdx ) const
                        { return !(UniqueIndex::operator==( rIdx )); }
};

inline void UniqueIndex::Clear()
{
    Container::Clear();
    nCount     = 0;
    nUniqIndex = 0;
}

// -----------------------
// - DECLARE_UNIQUEINDEX -
// -----------------------

#define DECLARE_UNIQUEINDEX( ClassName, Type )                          \
class ClassName : private UniqueIndex                                   \
{                                                                       \
public:                                                                 \
                using UniqueIndex::Clear;                                       \
                using UniqueIndex::Count;                                       \
                using UniqueIndex::GetCurIndex;                             \
                using UniqueIndex::IsIndexValid;                                \
                using UniqueIndex::GetStartIndex;                               \
                using UniqueIndex::GetCurMaxIndex;                          \
                                                                        \
                ClassName( ULONG _nStartIndex = 0,                      \
                           ULONG _nInitSize = 16, ULONG _nReSize = 16 ):\
                    UniqueIndex( _nStartIndex, _nInitSize, _nReSize ) {}\
                ClassName( const ClassName& rClassName ) :              \
                    UniqueIndex( rClassName ) {}                        \
                                                                        \
    ULONG       Insert( ULONG nIndex, Type p )                          \
                    { return UniqueIndex::Insert( nIndex, (void*)p ); } \
    ULONG       Insert( Type p )                                        \
                    { return UniqueIndex::Insert( (void*)p ); }         \
    Type        Remove( ULONG nIndex )                                  \
                    { return (Type)UniqueIndex::Remove( nIndex ); }     \
    Type        Replace( ULONG nIndex, Type p )                         \
                    { return (Type)UniqueIndex::Replace( nIndex,        \
                                                         (void*)p ); }  \
    Type        Get( ULONG nIndex ) const                               \
                    { return (Type)UniqueIndex::Get( nIndex ); }        \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)UniqueIndex::GetCurObject(); }       \
    ULONG       GetIndex( const Type p ) const                          \
                    { return UniqueIndex::GetIndex( (const void*)p ); } \
                                                                        \
    Type        Seek( ULONG nKey )                                      \
                    { return (Type)UniqueIndex::Seek( nKey ); }         \
    Type        Seek( Type p )                                          \
                    { return (Type)UniqueIndex::Seek( (void*)p ); }     \
    Type        First()  { return (Type)UniqueIndex::First(); }         \
    Type        Last()   { return (Type)UniqueIndex::Last(); }          \
    Type        Next()   { return (Type)UniqueIndex::Next(); }          \
    Type        Prev()   { return (Type)UniqueIndex::Prev(); }          \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { UniqueIndex::operator =( rClassName );            \
                      return *this; }                                   \
                                                                        \
    BOOL        operator ==( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator ==( rIdx ); }        \
    BOOL        operator !=( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator !=( rIdx ); }        \
};

#endif // _UNQIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
