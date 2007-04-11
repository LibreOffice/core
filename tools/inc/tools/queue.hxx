/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: queue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:15:32 $
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

#ifndef _QUEUE_HXX
#define _QUEUE_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

// ---------
// - Queue -
// ---------

#define QUEUE_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class Queue : private Container
{
public:
            using Container::Clear;
            using Container::Count;
            using Container::GetObject;
            using Container::GetPos;

            Queue( USHORT _nInitSize = 16, USHORT _nReSize = 16 ) :
                Container( _nReSize, _nInitSize, _nReSize ) {}
            Queue( const Queue& rQueue ) : Container( rQueue ) {}

    void    Put( void* p ) { Container::Insert( p, CONTAINER_APPEND ); }
    void*   Get()          { return Container::Remove( (ULONG)0 ); }

    Queue&  operator =( const Queue& rQueue )
                { Container::operator =( rQueue ); return *this; }

    BOOL    operator ==( const Queue& rQueue ) const
                { return Container::operator ==( rQueue ); }
    BOOL    operator !=( const Queue& rQueue ) const
                { return Container::operator !=( rQueue ); }
};

// -----------------
// - DECLARE_QUEUE -
// -----------------

#define DECLARE_QUEUE( ClassName, Type )                            \
class ClassName : private Queue                                     \
{                                                                   \
public:                                                             \
                using Queue::Clear;                                 \
                using Queue::Count;                                 \
                                                                    \
                ClassName( USHORT _nInitSize = 16,                  \
                           USHORT _nReSize = 16 ) :                 \
                    Queue( _nInitSize, _nReSize ) {}                \
                ClassName( const ClassName& rClassName ) :          \
                    Queue( rClassName ) {}                          \
                                                                    \
    void        Put( Type p ) { Queue::Put( (void*)p ); }           \
    Type        Get()         { return (Type)Queue::Get(); }        \
                                                                    \
    Type        GetObject( ULONG nIndex ) const                     \
                    { return (Type)Queue::GetObject( nIndex ); }    \
    ULONG       GetPos( const Type p ) const                        \
                    { return Queue::GetPos( (const void*)p ); }     \
    ULONG       GetPos( const Type p, ULONG nStartIndex,            \
                        BOOL bForward = TRUE ) const                \
                    { return Queue::GetPos( (const void*)p,         \
                                            nStartIndex,            \
                                            bForward ); }           \
                                                                    \
    ClassName&  operator =( const ClassName& rClassName )           \
                    { Queue::operator =( rClassName );              \
                      return *this; }                               \
                                                                    \
    BOOL        operator ==( const Queue& rQueue ) const            \
                    { return Queue::operator ==( rQueue ); }        \
    BOOL        operator !=( const Queue& rQueue ) const            \
                    { return Queue::operator !=( rQueue ); }        \
};

#endif // _QUEUE_HXX
