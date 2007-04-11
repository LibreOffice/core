/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stack.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:17:55 $
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

#ifndef _STACK_HXX
#define _STACK_HXX

#ifndef _CONTNR_HXX
#include <tools/contnr.hxx>
#endif

// ---------
// - Stack -
// ---------

#define STACK_ENTRY_NOTFOUND   CONTAINER_ENTRY_NOTFOUND

class Stack : private Container
{
public:
            using Container::Clear;
            using Container::Count;
            using Container::GetObject;
            using Container::GetPos;

            Stack( USHORT _nInitSize = 16, USHORT _nReSize = 16 ) :
                Container( CONTAINER_MAXBLOCKSIZE, _nInitSize, _nReSize ) {}
            Stack( const Stack& rStack ) : Container( rStack ) {}

    void    Push( void* p ) { Container::Insert( p, CONTAINER_APPEND ); }
    void*   Pop()           { return Container::Remove( Count()-1 ); }
    void*   Top() const     { return Container::GetObject( Count()-1 ); }

    Stack&  operator =( const Stack& rStack )
                { Container::operator =( rStack ); return *this; }

    BOOL    operator ==( const Stack& rStack ) const
                { return Container::operator ==( rStack ); }
    BOOL    operator !=( const Stack& rStack ) const
                { return Container::operator !=( rStack ); }
};

// -----------------
// - DECLARE_STACK -
// -----------------

#define DECLARE_STACK( ClassName, Type )                            \
class ClassName : private Stack                                     \
{                                                                   \
public:                                                             \
                using Stack::Clear;                                 \
                using Stack::Count;                                 \
                                                                    \
                ClassName( USHORT _nInitSize = 16,                  \
                       USHORT _nReSize = 16 ) :                     \
                    Stack( _nInitSize, _nReSize ) {}                \
                ClassName( const ClassName& rClassName ) :          \
                    Stack( rClassName ) {}                          \
                                                                    \
    void        Push( Type p ) { Stack::Push( (void*)p ); }         \
    Type        Pop()          { return (Type)Stack::Pop(); }       \
    Type        Top() const    { return (Type)Stack::Top(); }       \
                                                                    \
    Type        GetObject( ULONG nIndex ) const                     \
                    { return (Type)Stack::GetObject( nIndex ); }    \
    ULONG       GetPos( const Type p ) const                        \
                    { return Stack::GetPos( (const void*)p ); }     \
    ULONG       GetPos( const Type p, ULONG nStartIndex,            \
                        BOOL bForward = TRUE ) const                \
                    { return Stack::GetPos( (const void*)p,         \
                                            nStartIndex,            \
                                            bForward ); }           \
                                                                    \
    ClassName&  operator =( const ClassName& rClassName )           \
                    { Stack::operator =( rClassName );              \
                      return *this; }                               \
                                                                    \
    BOOL        operator ==( const ClassName& rStack ) const        \
                    { return Stack::operator ==( rStack ); }        \
    BOOL        operator !=( const ClassName& rStack ) const        \
                    { return Stack::operator !=( rStack ); }        \
};

#endif  // _STACK_HXX
