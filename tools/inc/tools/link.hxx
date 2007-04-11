/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: link.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:13:49 $
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
#ifndef _LINK_HXX
#define _LINK_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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

    BOOL        IsSet() const;
    BOOL        operator !() const;

    BOOL        operator==( const Link& rLink ) const;
    BOOL        operator!=( const Link& rLink ) const
                    { return !(Link::operator==( rLink )); }
    BOOL        operator<( const Link& rLink ) const
                    { return ((ULONG)rLink.pFunc < (ULONG)pFunc); }
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

inline BOOL Link::IsSet() const
{
    if ( pFunc )
        return TRUE;
    else
        return FALSE;
}

inline BOOL Link::operator !() const
{
    if ( !pFunc )
        return TRUE;
    else
        return FALSE;
}

#endif  // _LINK_HXX
