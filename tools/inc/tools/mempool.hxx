/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mempool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:14:09 $
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
#ifndef _SVMEMPOOL_HXX
#define _SVMEMPOOL_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// ----------------
// - FixedMemPool -
// ----------------

struct FixedMemPool_Impl;

class TOOLS_DLLPUBLIC FixedMemPool
{
    FixedMemPool_Impl * m_pImpl;

public:
                    FixedMemPool( USHORT nTypeSize,
                                  USHORT nInitSize = 512,
                                  USHORT nGrowSize = 256 );
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
