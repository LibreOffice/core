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

#ifndef _STACK_HXX
#define _STACK_HXX

#include <tools/contnr.hxx>

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

            Stack( sal_uInt16 _nInitSize = 16, sal_uInt16 _nReSize = 16 ) :
                Container( CONTAINER_MAXBLOCKSIZE, _nInitSize, _nReSize ) {}
            Stack( const Stack& rStack ) : Container( rStack ) {}

    void    Push( void* p ) { Container::Insert( p, CONTAINER_APPEND ); }
    void*   Pop()           { return Container::Remove( Count()-1 ); }
    void*   Top() const     { return Container::GetObject( Count()-1 ); }

    Stack&  operator =( const Stack& rStack )
                { Container::operator =( rStack ); return *this; }

    sal_Bool    operator ==( const Stack& rStack ) const
                { return Container::operator ==( rStack ); }
    sal_Bool    operator !=( const Stack& rStack ) const
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
                ClassName( sal_uInt16 _nInitSize = 16,                  \
                       sal_uInt16 _nReSize = 16 ) :                     \
                    Stack( _nInitSize, _nReSize ) {}                \
                ClassName( const ClassName& rClassName ) :          \
                    Stack( rClassName ) {}                          \
                                                                    \
    void        Push( Type p ) { Stack::Push( (void*)p ); }         \
    Type        Pop()          { return (Type)Stack::Pop(); }       \
    Type        Top() const    { return (Type)Stack::Top(); }       \
                                                                    \
    Type        GetObject( sal_uIntPtr nIndex ) const                     \
                    { return (Type)Stack::GetObject( nIndex ); }    \
    sal_uIntPtr       GetPos( const Type p ) const                        \
                    { return Stack::GetPos( (const void*)p ); }     \
    sal_uIntPtr       GetPos( const Type p, sal_uIntPtr nStartIndex,            \
                        sal_Bool bForward = sal_True ) const                \
                    { return Stack::GetPos( (const void*)p,         \
                                            nStartIndex,            \
                                            bForward ); }           \
                                                                    \
    ClassName&  operator =( const ClassName& rClassName )           \
                    { Stack::operator =( rClassName );              \
                      return *this; }                               \
                                                                    \
    sal_Bool        operator ==( const ClassName& rStack ) const        \
                    { return Stack::operator ==( rStack ); }        \
    sal_Bool        operator !=( const ClassName& rStack ) const        \
                    { return Stack::operator !=( rStack ); }        \
};

#endif  // _STACK_HXX
