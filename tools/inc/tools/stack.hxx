/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
