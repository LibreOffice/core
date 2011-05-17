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
    sal_uIntPtr           nReSize;
    sal_uIntPtr           nStartIndex;
    sal_uIntPtr           nUniqIndex;
    sal_uIntPtr           nCount;

public:
                    using Container::GetCurObject;

                    UniqueIndex( sal_uIntPtr nStartIndex = 0,
                                 sal_uIntPtr nInitSize = 16,
                                 sal_uIntPtr nReSize = 16 );
                    UniqueIndex( const UniqueIndex& rIdx );

    sal_uIntPtr           Insert( sal_uIntPtr nIndex, void* p );
    sal_uIntPtr           Insert( void* p );
    void*           Remove( sal_uIntPtr nIndex );
    void*           Replace( sal_uIntPtr nIndex, void* p );
    void*           Get( sal_uIntPtr nIndex ) const;

    void            Clear();
    sal_uIntPtr           Count() const { return nCount; }

    sal_uIntPtr           GetCurIndex() const;
    sal_uIntPtr           GetIndex( const void* p ) const;
    sal_Bool            IsIndexValid( sal_uIntPtr nIndex ) const;

    void*           Seek( sal_uIntPtr nIndex );
    void*           Seek( void* p );
    void*           First();
    void*           Last();
    void*           Next();
    void*           Prev();

    sal_uIntPtr           GetStartIndex() const { return nStartIndex; }
    sal_uIntPtr           GetCurMaxIndex() const
                        { return (nStartIndex + Container::GetSize()); }

    UniqueIndex&    operator =( const UniqueIndex& rIdx );

    sal_Bool            operator ==( const UniqueIndex& rIdx ) const;
    sal_Bool            operator !=( const UniqueIndex& rIdx ) const
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
                ClassName( sal_uIntPtr _nStartIndex = 0,                      \
                           sal_uIntPtr _nInitSize = 16, sal_uIntPtr _nReSize = 16 ):\
                    UniqueIndex( _nStartIndex, _nInitSize, _nReSize ) {}\
                ClassName( const ClassName& rClassName ) :              \
                    UniqueIndex( rClassName ) {}                        \
                                                                        \
    sal_uIntPtr       Insert( sal_uIntPtr nIndex, Type p )                          \
                    { return UniqueIndex::Insert( nIndex, (void*)p ); } \
    sal_uIntPtr       Insert( Type p )                                        \
                    { return UniqueIndex::Insert( (void*)p ); }         \
    Type        Remove( sal_uIntPtr nIndex )                                  \
                    { return (Type)UniqueIndex::Remove( nIndex ); }     \
    Type        Replace( sal_uIntPtr nIndex, Type p )                         \
                    { return (Type)UniqueIndex::Replace( nIndex,        \
                                                         (void*)p ); }  \
    Type        Get( sal_uIntPtr nIndex ) const                               \
                    { return (Type)UniqueIndex::Get( nIndex ); }        \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)UniqueIndex::GetCurObject(); }       \
    sal_uIntPtr       GetIndex( const Type p ) const                          \
                    { return UniqueIndex::GetIndex( (const void*)p ); } \
                                                                        \
    Type        Seek( sal_uIntPtr nKey )                                      \
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
    sal_Bool        operator ==( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator ==( rIdx ); }        \
    sal_Bool        operator !=( const ClassName& rIdx ) const              \
                    { return UniqueIndex::operator !=( rIdx ); }        \
};

#endif // _UNQIDX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
