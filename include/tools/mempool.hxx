/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SVMEMPOOL_HXX
#define _SVMEMPOOL_HXX

#include "sal/config.h"

#include "sal/macros.h"
#include "tools/toolsdllapi.h"
#include "tools/solar.h"

struct FixedMemPool_Impl;

class TOOLS_DLLPUBLIC FixedMemPool
{
    FixedMemPool_Impl * m_pImpl;
    char const *        m_pTypeName;

public:
                    FixedMemPool( char const * pTypeName,
                                  sal_uInt16 nTypeSize );
                    ~FixedMemPool();

    void*           Alloc();
    void            Free( void* p );
};

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

#define IMPL_FIXEDMEMPOOL_NEWDEL( Class ) \
    FixedMemPool Class::aPool( SAL_STRINGIFY( Class ), sizeof( Class ) );

#define DECL_FIXEDMEMPOOL_NEWDEL_DLL( Class ) \
    private: \
        static FixedMemPool aPool; \
    public: \
        DECL_FIXEDMEMPOOL_NEW_DECL(); \
        DECL_FIXEDMEMPOOL_DEL_DECL();

#define IMPL_FIXEDMEMPOOL_NEWDEL_DLL( Class ) \
    FixedMemPool Class::aPool( SAL_STRINGIFY( Class ), sizeof( Class ) ); \
    DECL_FIXEDMEMPOOL_NEW_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_NEW_BODY( Class, aPool ) \
    DECL_FIXEDMEMPOOL_DEL_IMPL( Class ) \
    IMPL_FIXEDMEMPOOL_DEL_BODY( Class, aPool )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
