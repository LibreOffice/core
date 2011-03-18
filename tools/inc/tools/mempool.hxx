/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVMEMPOOL_HXX
#define _SVMEMPOOL_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

// ----------------
// - FixedMemPool -
// ----------------

struct FixedMemPool_Impl;

class TOOLS_DLLPUBLIC FixedMemPool
{
    FixedMemPool_Impl * m_pImpl;

public:
                    FixedMemPool( sal_uInt16 nTypeSize,
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

#define IMPL_FIXEDMEMPOOL_NEWDEL( Class, InitSize, GrowSize) \
    FixedMemPool Class::aPool( sizeof( Class ), (InitSize), (GrowSize) );

#define DECL_FIXEDMEMPOOL_NEWDEL_DLL( Class ) \
    private: \
        static FixedMemPool aPool; \
    public: \
        DECL_FIXEDMEMPOOL_NEW_DECL(); \
        DECL_FIXEDMEMPOOL_DEL_DECL();

#define IMPL_FIXEDMEMPOOL_NEWDEL_DLL( Class, InitSize, GrowSize) \
    FixedMemPool Class::aPool( sizeof( Class ), (InitSize), (GrowSize) ); \
    DECL_FIXEDMEMPOOL_NEW_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_NEW_BODY( Class, aPool ) \
    DECL_FIXEDMEMPOOL_DEL_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_DEL_BODY( Class, aPool )

#define INIT_FIXEDMEMPOOL_NEWDEL_DLL( class, aPool, InitSize, GrowSize ) \
    aPool( sizeof( class ), InitSize, GrowSize )

#endif // _SVMEMPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
