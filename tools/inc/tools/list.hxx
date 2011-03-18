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

#ifndef _LIST_HXX
#define _LIST_HXX

#include <tools/solar.h>
#include <tools/contnr.hxx>

// --------
// - List -
// --------

#define LIST_APPEND           CONTAINER_APPEND
#define LIST_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class List : private Container
{
public:
            using Container::Insert;
            using Container::Remove;
            using Container::Replace;
            using Container::Clear;
            using Container::Count;
            using Container::GetCurObject;
            using Container::GetCurPos;
            using Container::GetObject;
            using Container::GetPos;
            using Container::Seek;
            using Container::First;
            using Container::Last;
            using Container::Next;
            using Container::Prev;

            List( sal_uInt16 _nInitSize = 16, sal_uInt16 _nReSize = 16 ) :
                Container( 1024, _nInitSize, _nReSize ) {}
            List( sal_uInt16 _nBlockSize, sal_uInt16 _nInitSize, sal_uInt16 _nReSize ) :
                Container( _nBlockSize, _nInitSize, _nReSize ) {}
            List( const List& rList ) : Container( rList ) {}

    List&   operator =( const List& rList )
                { Container::operator =( rList ); return *this; }

    sal_Bool    operator ==( const List& rList ) const
                { return Container::operator ==( rList ); }
    sal_Bool    operator !=( const List& rList ) const
                { return Container::operator !=( rList ); }
};

// ----------------
// - DECLARE_LIST -
// ----------------

#define DECLARE_LIST( ClassName, Type )                                 \
class ClassName : private List                                          \
{                                                                       \
public:                                                                 \
                using List::Clear;                                      \
                using List::Count;                                      \
                using List::GetCurPos;                                  \
                                                                        \
                ClassName( sal_uInt16 _nInitSize = 16,                      \
                           sal_uInt16 _nReSize = 16 ) :                     \
                    List( _nInitSize, _nReSize ) {}                     \
                ClassName( sal_uInt16 _nBlockSize, sal_uInt16 _nInitSize,       \
                           sal_uInt16 _nReSize ) :                          \
                    List( _nBlockSize, _nInitSize, _nReSize ) {}        \
                ClassName( const ClassName& rClassName ) :              \
                    List( rClassName ) {}                               \
                                                                        \
    void        Insert( Type p, sal_uIntPtr nIndex )                          \
                    { List::Insert( (void*)p, nIndex ); }               \
    void        Insert( Type p )                                        \
                    { List::Insert( (void*)p ); }                       \
    void        Insert( Type pNew, Type pOld )                          \
                    { List::Insert( (void*)pNew, (void*)pOld ); }       \
    Type        Remove()                                                \
                    { return (Type)List::Remove(); }                    \
    Type        Remove( sal_uIntPtr nIndex )                                  \
                    { return (Type)List::Remove( nIndex ); }            \
    Type        Remove( Type p )                                        \
                    { return (Type)List::Remove( (void*)p ); }          \
    Type        Replace( Type p )                                       \
                    { return (Type)List::Replace( (void*)p ); }         \
    Type        Replace( Type p, sal_uIntPtr nIndex )                         \
                    { return (Type)List::Replace( (void*)p, nIndex ); } \
    Type        Replace( Type pNew, Type pOld )                         \
                    { return (Type)List::Replace( (void*)pNew,          \
                                                  (void*)pOld ); }      \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)List::GetCurObject(); }              \
    Type        GetObject( sal_uIntPtr nIndex ) const                         \
                    { return (Type)List::GetObject( nIndex ); }         \
    sal_uIntPtr       GetPos( const Type p ) const                            \
                    { return List::GetPos( (const void*)p ); }          \
    sal_uIntPtr       GetPos( const Type p, sal_uIntPtr nStartIndex,                \
                        sal_Bool bForward = sal_True ) const                    \
                    { return List::GetPos( (const void*)p, nStartIndex, \
                                           bForward ); }                \
                                                                        \
    Type        Seek( sal_uIntPtr nIndex )                                    \
                    { return (Type)List::Seek( nIndex ); }              \
    Type        Seek( void* p ) { return (Type)List::Seek( p ); }       \
    Type        First()         { return (Type)List::First(); }         \
    Type        Last()          { return (Type)List::Last(); }          \
    Type        Next()          { return (Type)List::Next(); }          \
    Type        Prev()          { return (Type)List::Prev(); }          \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { List::operator =( rClassName ); return *this; }   \
                                                                        \
    sal_Bool        operator ==( const ClassName& rList ) const             \
                    { return List::operator ==( rList ); }              \
    sal_Bool        operator !=( const ClassName& rList ) const             \
                    { return List::operator !=( rList ); }              \
};

#endif // _LIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
