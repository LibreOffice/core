/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dynary.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:10:26 $
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

#ifndef _DYNARY_HXX
#define _DYNARY_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

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
