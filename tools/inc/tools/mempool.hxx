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


#ifndef _SVMEMPOOL_HXX
#define _SVMEMPOOL_HXX

#include "tools/toolsdllapi.h"
#include "tools/solar.h"

// ----------------
// - FixedMemPool -
// ----------------

struct FixedMemPool_Impl;

class TOOLS_DLLPUBLIC FixedMemPool
{
    FixedMemPool_Impl * m_pImpl;
    char const *        m_pTypeName;

public:
                    FixedMemPool( char const * pTypeName,
                                  sal_uInt16 nTypeSize,
                                  sal_uInt16 nInitSize = 512,
                                  sal_uInt16 nGrowSize = 256 );
                    ~FixedMemPool();

    void*           Alloc();
    void            Free( void* p );
};

// ----------------------------
// - DECL_FIXEDMEMPOOL_NEWDEL -
// ----------------------------

#define DECL_FIXEDMEMPOOL_NEW_DECL() \
static void * operator new( size_t n )

#define DECL_FIXEDMEMPOOL_NEW_IMPL( Class ) \
void * Class::operator new( size_t n )

#define IMPL_FIXEDMEMPOOL_NEW_BODY( Class, aPool ) \
{ \
    if ( n == sizeof( Class ) ) \
        return (aPool).Alloc(); \
    else \
        return ::operator new(n); \
}

#define DECL_FIXEDMEMPOOL_NEW_INLINE( Class, aPool ) \
DECL_FIXEDMEMPOOL_NEW_DECL() \
IMPL_FIXEDMEMPOOL_NEW_BODY( Class, aPool )

#define DECL_FIXEDMEMPOOL_DEL_DECL() \
static void operator delete( void * p, size_t n )

#define DECL_FIXEDMEMPOOL_DEL_IMPL( Class ) \
void Class::operator delete( void * p, size_t n )

#define IMPL_FIXEDMEMPOOL_DEL_BODY( Class, aPool ) \
{ \
    if ( n == sizeof( Class ) ) \
        (aPool).Free(p); \
    else \
        ::operator delete(p); \
}

#define DECL_FIXEDMEMPOOL_DEL_INLINE( Class, aPool ) \
DECL_FIXEDMEMPOOL_DEL_DECL() \
IMPL_FIXEDMEMPOOL_DEL_BODY( Class, aPool )

#define DECL_FIXEDMEMPOOL_NEWDEL( Class ) \
    private: \
        static FixedMemPool aPool; \
    public: \
        DECL_FIXEDMEMPOOL_NEW_INLINE( Class, aPool ) \
        DECL_FIXEDMEMPOOL_DEL_INLINE( Class, aPool )

#define IMPL_FIXEDMEMPOOL_STRING(x) IMPL_FIXEDMEMPOOL_MAKESTRING(x)
#define IMPL_FIXEDMEMPOOL_MAKESTRING(x) #x

#define IMPL_FIXEDMEMPOOL_NEWDEL( Class, InitSize, GrowSize) \
    FixedMemPool Class::aPool( IMPL_FIXEDMEMPOOL_STRING( Class ), sizeof( Class ), (InitSize), (GrowSize) );

#define DECL_FIXEDMEMPOOL_NEWDEL_DLL( Class ) \
    private: \
        static FixedMemPool aPool; \
    public: \
        DECL_FIXEDMEMPOOL_NEW_DECL(); \
        DECL_FIXEDMEMPOOL_DEL_DECL();

#define IMPL_FIXEDMEMPOOL_NEWDEL_DLL( Class, InitSize, GrowSize) \
    FixedMemPool Class::aPool( IMPL_FIXEDMEMPOOL_STRING( Class ), sizeof( Class ), (InitSize), (GrowSize) ); \
    DECL_FIXEDMEMPOOL_NEW_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_NEW_BODY( Class, aPool ) \
    DECL_FIXEDMEMPOOL_DEL_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_DEL_BODY( Class, aPool )

#define INIT_FIXEDMEMPOOL_NEWDEL_DLL( Class, aPool, InitSize, GrowSize ) \
    aPool( IMPL_FIXEDMEMPOOL_STRING( Class ), sizeof( Class ), (InitSize), (GrowSize) )

#endif // _SVMEMPOOL_HXX
