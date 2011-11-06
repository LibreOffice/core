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
    long __EXPORT Class::LinkStub##Method( void* pThis, void* pCaller) \
    { \
        return ((Class*)pThis )->Method( (ArgType)pCaller ); \
    }

#define IMPL_STATIC_LINK( Class, Method, ArgType, ArgName ) \
    long __EXPORT Class::Method( Class* pThis, ArgType ArgName )

#define IMPL_STATIC_LINK_NOINSTANCE( Class, Method, ArgType, ArgName ) \
    long __EXPORT Class::Method( Class*, ArgType ArgName )

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
