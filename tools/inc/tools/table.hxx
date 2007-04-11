/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: table.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:18:57 $
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
#ifndef _TOOLS_TABLE_HXX
#define _TOOLS_TABLE_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

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
