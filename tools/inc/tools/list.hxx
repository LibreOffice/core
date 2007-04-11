/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: list.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:13:59 $
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

#ifndef _LIST_HXX
#define _LIST_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

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

            List( USHORT _nInitSize = 16, USHORT _nReSize = 16 ) :
                Container( 1024, _nInitSize, _nReSize ) {}
            List( USHORT _nBlockSize, USHORT _nInitSize, USHORT _nReSize ) :
                Container( _nBlockSize, _nInitSize, _nReSize ) {}
            List( const List& rList ) : Container( rList ) {}

    List&   operator =( const List& rList )
                { Container::operator =( rList ); return *this; }

    BOOL    operator ==( const List& rList ) const
                { return Container::operator ==( rList ); }
    BOOL    operator !=( const List& rList ) const
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
                ClassName( USHORT _nInitSize = 16,                      \
                           USHORT _nReSize = 16 ) :                     \
                    List( _nInitSize, _nReSize ) {}                     \
                ClassName( USHORT _nBlockSize, USHORT _nInitSize,       \
                           USHORT _nReSize ) :                          \
                    List( _nBlockSize, _nInitSize, _nReSize ) {}        \
                ClassName( const ClassName& rClassName ) :              \
                    List( rClassName ) {}                               \
                                                                        \
    void        Insert( Type p, ULONG nIndex )                          \
                    { List::Insert( (void*)p, nIndex ); }               \
    void        Insert( Type p )                                        \
                    { List::Insert( (void*)p ); }                       \
    void        Insert( Type pNew, Type pOld )                          \
                    { List::Insert( (void*)pNew, (void*)pOld ); }       \
    Type        Remove()                                                \
                    { return (Type)List::Remove(); }                    \
    Type        Remove( ULONG nIndex )                                  \
                    { return (Type)List::Remove( nIndex ); }            \
    Type        Remove( Type p )                                        \
                    { return (Type)List::Remove( (void*)p ); }          \
    Type        Replace( Type p )                                       \
                    { return (Type)List::Replace( (void*)p ); }         \
    Type        Replace( Type p, ULONG nIndex )                         \
                    { return (Type)List::Replace( (void*)p, nIndex ); } \
    Type        Replace( Type pNew, Type pOld )                         \
                    { return (Type)List::Replace( (void*)pNew,          \
                                                  (void*)pOld ); }      \
                                                                        \
    Type        GetCurObject() const                                    \
                    { return (Type)List::GetCurObject(); }              \
    Type        GetObject( ULONG nIndex ) const                         \
                    { return (Type)List::GetObject( nIndex ); }         \
    ULONG       GetPos( const Type p ) const                            \
                    { return List::GetPos( (const void*)p ); }          \
    ULONG       GetPos( const Type p, ULONG nStartIndex,                \
                        BOOL bForward = TRUE ) const                    \
                    { return List::GetPos( (const void*)p, nStartIndex, \
                                           bForward ); }                \
                                                                        \
    Type        Seek( ULONG nIndex )                                    \
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
    BOOL        operator ==( const ClassName& rList ) const             \
                    { return List::operator ==( rList ); }              \
    BOOL        operator !=( const ClassName& rList ) const             \
                    { return List::operator !=( rList ); }              \
};

#endif // _LIST_HXX
