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

#ifndef _TOOLS_RESID_HXX
#define _TOOLS_RESID_HXX

#include <tools/solar.h>
#include <osl/diagnose.h>
#include "tools/toolsdllapi.h"

struct RSHEADER_TYPE;
typedef sal_uInt32 RESOURCE_TYPE;
#define RSC_NOTYPE              0x100
#define RSC_DONTRELEASE         (sal_uInt32(1 << 31))

class ResMgr;

//---------
//- ResId -
//---------

namespace rtl {
    class OUString;
}

class ResId
{
    /*
    consider two cases: either m_pResource is valid and points
    two a resource data buffer; then m_nResId and m_pResMgr are
    not used and may be 0 resp. NULL
    or m_pResource is NULL, the m_nResId and m_pResMgr must be valid.
    In this case the highest bit if set decides whether to
    not to release the Resource context after loading this id
    */
    RSHEADER_TYPE*          m_pResource;

    mutable sal_uInt32      m_nResId;      // Resource Identifier
    mutable RESOURCE_TYPE   m_nRT;         // type for loading (mutable to be set later)
    mutable ResMgr *        m_pResMgr;     // load from this ResMgr (mutable for setting on demand)
    mutable RESOURCE_TYPE   m_nRT2;        // type for loading (supercedes m_nRT)
    mutable sal_uInt32      m_nWinBits;    // container for original style bits on a window in a resource

    void ImplInit( sal_uInt32 nId, ResMgr& rMgr, RSHEADER_TYPE* pRes )
    {
        m_pResource = pRes; m_nResId = nId; m_nRT = RSC_NOTYPE; m_pResMgr = &rMgr; m_nRT2 = RSC_NOTYPE; m_nWinBits = 0;
        OSL_ENSURE( m_pResMgr != NULL, "ResId without ResMgr created" );
    }
    public:
    ResId( RSHEADER_TYPE * pRc, ResMgr& rMgr )
    {
        ImplInit( 0, rMgr, pRc );
    }
    ResId( sal_uInt32 nId, ResMgr& rMgr )
    {
        ImplInit( nId, rMgr, NULL );
    }
    // backwards compatibility; avoid ambiguities
    ResId( sal_uInt16 nId, ResMgr& rMgr )
    {
        ImplInit( sal_uInt32(nId), rMgr, NULL );
    }
    ResId( int nId, ResMgr& rMgr )
    {
        ImplInit( sal_uInt32(nId), rMgr, NULL );
    }
    ResId( long nId, ResMgr& rMgr )
    {
        ImplInit( sal_uInt32(nId), rMgr, NULL );
    }

    sal_uInt32 GetWinBits() const
    { return m_nWinBits; }
    void SetWinBits( sal_uInt32 nBits ) const
    { m_nWinBits = nBits; }

    RESOURCE_TYPE   GetRT() const { return( m_nRT ); }
    const ResId &   SetRT( RESOURCE_TYPE nType ) const
    /*
    Set the type if not already set. Ask for tye with GetRT()

    [Example]
    ResId aId( 1000 );
    aId.SetRT( RSC_WINDOW );    // settype window Window
    aId.SetRT( RSC_BUTTON );    // will not set type Button
    //aId.GetRT() == RSC_WINDOW is true

    @see
    ResId::GetRT2(), ResId::GetRT()
    */
    {
        if( RSC_NOTYPE == m_nRT )
            m_nRT = nType;
        return *this;
    }
    RESOURCE_TYPE   GetRT2() const
    /*
    Get the effective type (m_nRT2 or m_nRT1)

    A second resource type is used to supercede settings
    of the base class ( e.g. Window )
    */
    {
        return (RSC_NOTYPE == m_nRT2) ? m_nRT : m_nRT2;
    }
    const ResId &   SetRT2( RESOURCE_TYPE nTyp ) const
    /*
    Set the superceding type. Ask spcifically for it with GetRT2()

    SetRT2() may only be called if no derived class calls SetRT in its
    Resource constructor.
    */
    {
        if( RSC_NOTYPE == m_nRT2 )
            m_nRT2 = nTyp;
        return *this;
    }

    ResMgr *        GetResMgr() const { return m_pResMgr; }
    const ResId &   SetResMgr( ResMgr * pMgr ) const
    {
        m_pResMgr = pMgr;
        OSL_ENSURE( m_pResMgr != NULL, "invalid ResMgr set on ResId" );
        return *this;
    }

    const ResId &  SetAutoRelease(sal_Bool bRelease) const
    {
        if( bRelease )
            m_nResId &= ~RSC_DONTRELEASE;
        else
            m_nResId |= RSC_DONTRELEASE;
        return *this;
    }

    sal_Bool           IsAutoRelease()  const
    { return !(m_nResId & RSC_DONTRELEASE); }

    sal_uInt32     GetId()          const { return m_nResId & ~RSC_DONTRELEASE; }
    RSHEADER_TYPE* GetpResource()   const { return m_pResource; }

    static TOOLS_DLLPUBLIC rtl::OUString toString(const ResId& aId);
};

#endif // _RESID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
