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



#ifndef _DYNARY_HXX
#define _DYNARY_HXX

#include <tools/solar.h>
#include <tools/contnr.hxx>

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

                DynArray( sal_uIntPtr nSize = 16 ) : Container( nSize ) {}
                DynArray( const DynArray& rAry ) : Container( rAry ) {}

    void*       Put( sal_uIntPtr nIndex, void* p )
                    { return Container::Replace( p, nIndex ); }
    void*       Get( sal_uIntPtr nIndex ) const
                    { return Container::GetObject( nIndex ); }

    sal_uIntPtr       GetIndex( const void* p ) const
                    { return Container::GetPos( p ); }
    sal_uIntPtr       GetIndex( const void* p, sal_uIntPtr nStartIndex,
                          sal_Bool bForward = sal_True ) const
                    { return Container::GetPos( p, nStartIndex, bForward ); }

    DynArray&   operator =( const DynArray& rAry )
                    { Container::operator =( rAry ); return *this; }

    sal_Bool        operator ==( const DynArray& rAry ) const
                    { return Container::operator ==( rAry ); }
    sal_Bool        operator !=( const DynArray& rAry ) const
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
                ClassName( sal_uIntPtr nSize = 16 ) :                         \
                    DynArray( nSize ) {}                                \
                ClassName( const ClassName& rClassName ) :              \
                    DynArray( rClassName ) {}                           \
                                                                        \
    Type        Put( sal_uIntPtr nIndex, Type p )                             \
                    { return (Type)DynArray::Put( nIndex, (void*)p ); } \
    Type        Get( sal_uIntPtr nIndex ) const                               \
                    { return (Type)DynArray::Get( nIndex ); }           \
                                                                        \
    sal_uIntPtr       GetIndex( const Type p ) const                          \
                    { return DynArray::GetIndex( (const void*)p ); }    \
    sal_uIntPtr       GetIndex( const Type p, sal_uIntPtr nStartIndex,              \
                          sal_Bool bForward = sal_True ) const                  \
                    { return DynArray::GetIndex( (const void*)p,        \
                                                 nStartIndex,           \
                                                 bForward ); }          \
                                                                        \
    ClassName&  operator =( const ClassName& rClassName )               \
                    { DynArray::operator =( rClassName );               \
                      return *this; }                                   \
                                                                        \
    sal_Bool        operator ==( const ClassName& rAry ) const              \
                    { return DynArray::operator ==( rAry ); }           \
    sal_Bool        operator !=( const ClassName& rAry ) const              \
                    { return DynArray::operator !=( rAry ); }           \
};

#endif // _DYNARY_HXX
