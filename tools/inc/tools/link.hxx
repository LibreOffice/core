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
#ifndef _LINK_HXX
#define _LINK_HXX

#include "tools/toolsdllapi.h"
#include "sal/config.h"
#include "sal/types.h"
#include <tools/solar.h>

// ---------------
// - Link-Makros -
// ---------------

typedef long (*PSTUB)( void*, void* );

#define DECL_LINK( Method, ArgType ) \
    long Method( ArgType ); \
    static long LinkStub##Method( void* pThis, void* )

#define DECL_STATIC_LINK( Class, Method, ArgType ) \
    static long Method( Class*, ArgType )

#define DECL_DLLPRIVATE_LINK(Method, ArgType) \
    SAL_DLLPRIVATE long Method(ArgType); \
    SAL_DLLPRIVATE static long LinkStub##Method(void * pThis, void *)

#define DECL_DLLPRIVATE_STATIC_LINK(Class, Method, ArgType) \
    SAL_DLLPRIVATE static long Method(Class *, ArgType)

#define IMPL_METHOD( Class, Method, ArgType, ArgName ) \
    long Class::Method( ArgType ArgName )

#define IMPL_STUB(Class, Method, ArgType) \
    long Class::LinkStub##Method( void* pThis, void* pCaller) \
    { \
        return ((Class*)pThis )->Method( (ArgType)pCaller ); \
    }

#define IMPL_STATIC_LINK( Class, Method, ArgType, ArgName ) \
    long Class::Method( Class* pThis, ArgType ArgName )

#define IMPL_STATIC_LINK_NOINSTANCE( Class, Method, ArgType, ArgName ) \
    long Class::Method( Class*, ArgType ArgName )

#define LINK( Inst, Class, Member ) \
    Link( (Class*)Inst, (PSTUB)&Class::LinkStub##Member )

#define STATIC_LINK( Inst, Class, Member ) \
    Link( (Class*)Inst, (PSTUB)&Class::Member )

#define IMPL_LINK( Class, Method, ArgType, ArgName ) \
    IMPL_STUB( Class, Method, ArgType ) \
    long Class::Method( ArgType ArgName )

#if defined GCC && defined NO_OPTIMIZE
#define IMPL_LINK_INLINE_START( Class, Method, ArgType, ArgName ) \
    IMPL_LINK( Class, Method, ArgType, ArgName )

#define IMPL_LINK_INLINE_END( Class, Method, ArgType, ArgName )
#else
#define IMPL_LINK_INLINE_START( Class, Method, ArgType, ArgName ) \
    inline long Class::Method( ArgType ArgName )

#define IMPL_LINK_INLINE_END( Class, Method, ArgType, ArgName ) \
    IMPL_STUB( Class, Method, ArgType )
#endif

#define IMPL_LINK_INLINE( Class, Method, ArgType, ArgName, Body ) \
    long Class::Method( ArgType ArgName ) \
    Body \
    IMPL_STUB( Class, Method, ArgType )

#define EMPTYARG

// --------
// - Link -
// --------

class TOOLS_DLLPUBLIC Link
{
    void*       pInst;
    PSTUB       pFunc;

public:
                Link();
                Link( void* pLinkHdl, PSTUB pMemFunc );

    long        Call( void* pCaller ) const;

    sal_Bool        IsSet() const;
    sal_Bool        operator !() const;

    sal_Bool        operator==( const Link& rLink ) const;
    sal_Bool        operator!=( const Link& rLink ) const
                    { return !(Link::operator==( rLink )); }
    sal_Bool        operator<( const Link& rLink ) const
                    { return ((sal_uIntPtr)rLink.pFunc < (sal_uIntPtr)pFunc); }
};

inline Link::Link()
{
    pInst = 0;
    pFunc = 0;
}

inline Link::Link( void* pLinkHdl, PSTUB pMemFunc )
{
    pInst = pLinkHdl;
    pFunc = pMemFunc;
}

inline long Link::Call(void *pCaller) const
{
    return pFunc ? (*pFunc)(pInst, pCaller) : 0;
}

inline sal_Bool Link::IsSet() const
{
    if ( pFunc )
        return sal_True;
    else
        return sal_False;
}

inline sal_Bool Link::operator !() const
{
    if ( !pFunc )
        return sal_True;
    else
        return sal_False;
}

#endif  // _LINK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
