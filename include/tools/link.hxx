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
#ifndef _LINK_HXX
#define _LINK_HXX

#include "tools/toolsdllapi.h"
#include "sal/config.h"
#include "sal/types.h"
#include <tools/solar.h>

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

#define IMPL_STUB(Class, Method, ArgType) \
    long Class::LinkStub##Method( void* pThis, void* pCaller) \
    { \
        return ((Class*)pThis )->Method( (ArgType)pCaller ); \
    }

#define IMPL_STATIC_LINK( Class, Method, ArgType, ArgName ) \
    long Class::Method( Class* pThis, ArgType ArgName )

#define IMPL_STATIC_LINK_NOINSTANCE( Class, Method, ArgType, ArgName ) \
    long Class::Method( SAL_UNUSED_PARAMETER Class*, ArgType ArgName )

#define LINK( Inst, Class, Member ) \
    Link( (Class*)Inst, (PSTUB)&Class::LinkStub##Member )

#define STATIC_LINK( Inst, Class, Member ) \
    Link( (Class*)Inst, (PSTUB)&Class::Member )

#define IMPL_LINK( Class, Method, ArgType, ArgName ) \
    IMPL_STUB( Class, Method, ArgType ) \
    long Class::Method( ArgType ArgName )

#define IMPL_LINK_NOARG( Class, Method ) \
    IMPL_STUB( Class, Method, void* ) \
    long Class::Method( SAL_UNUSED_PARAMETER void* )

#define IMPL_LINK_INLINE_START( Class, Method, ArgType, ArgName ) \
    inline long Class::Method( ArgType ArgName )

#define IMPL_LINK_INLINE_END( Class, Method, ArgType, ArgName ) \
    IMPL_STUB( Class, Method, ArgType )

#define IMPL_LINK_NOARG_INLINE_START( Class, Method ) \
    inline long Class::Method( SAL_UNUSED_PARAMETER void* )

#define IMPL_LINK_NOARG_INLINE_END( Class, Method ) \
    IMPL_STUB( Class, Method, void* )

#define IMPL_LINK_INLINE( Class, Method, ArgType, ArgName, Body ) \
    long Class::Method( ArgType ArgName ) \
    Body \
    IMPL_STUB( Class, Method, ArgType )

#define EMPTYARG

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
