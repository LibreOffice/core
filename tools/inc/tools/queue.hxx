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



#ifndef _QUEUE_HXX
#define _QUEUE_HXX

#include <tools/solar.h>
#include <tools/contnr.hxx>

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

            Queue( sal_uInt16 _nInitSize = 16, sal_uInt16 _nReSize = 16 ) :
                Container( _nReSize, _nInitSize, _nReSize ) {}
            Queue( const Queue& rQueue ) : Container( rQueue ) {}

    void    Put( void* p ) { Container::Insert( p, CONTAINER_APPEND ); }
    void*   Get()          { return Container::Remove( (sal_uIntPtr)0 ); }

    Queue&  operator =( const Queue& rQueue )
                { Container::operator =( rQueue ); return *this; }

    sal_Bool    operator ==( const Queue& rQueue ) const
                { return Container::operator ==( rQueue ); }
    sal_Bool    operator !=( const Queue& rQueue ) const
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
                ClassName( sal_uInt16 _nInitSize = 16,                  \
                           sal_uInt16 _nReSize = 16 ) :                 \
                    Queue( _nInitSize, _nReSize ) {}                \
                ClassName( const ClassName& rClassName ) :          \
                    Queue( rClassName ) {}                          \
                                                                    \
    void        Put( Type p ) { Queue::Put( (void*)p ); }           \
    Type        Get()         { return (Type)Queue::Get(); }        \
                                                                    \
    Type        GetObject( sal_uIntPtr nIndex ) const                     \
                    { return (Type)Queue::GetObject( nIndex ); }    \
    sal_uIntPtr       GetPos( const Type p ) const                        \
                    { return Queue::GetPos( (const void*)p ); }     \
    sal_uIntPtr       GetPos( const Type p, sal_uIntPtr nStartIndex,            \
                        sal_Bool bForward = sal_True ) const                \
                    { return Queue::GetPos( (const void*)p,         \
                                            nStartIndex,            \
                                            bForward ); }           \
                                                                    \
    ClassName&  operator =( const ClassName& rClassName )           \
                    { Queue::operator =( rClassName );              \
                      return *this; }                               \
                                                                    \
    sal_Bool        operator ==( const Queue& rQueue ) const            \
                    { return Queue::operator ==( rQueue ); }        \
    sal_Bool        operator !=( const Queue& rQueue ) const            \
                    { return Queue::operator !=( rQueue ); }        \
};

#endif // _QUEUE_HXX
