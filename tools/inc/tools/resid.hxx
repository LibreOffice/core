/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resid.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:47:17 $
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

#ifndef _TOOLS_RESID_HXX
#define _TOOLS_RESID_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

struct RSHEADER_TYPE;
typedef sal_uInt32 RESOURCE_TYPE;
#define RSC_NOTYPE              0x100
#define RSC_DONTRELEASE         (sal_uInt32(1 << 31))

class ResMgr;

//---------
//- ResId -
//---------

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
    ResId( USHORT nId, ResMgr& rMgr )
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

    const ResId &  SetAutoRelease(BOOL bRelease) const
    {
        if( bRelease )
            m_nResId &= ~RSC_DONTRELEASE;
        else
            m_nResId |= RSC_DONTRELEASE;
        return *this;
    }

    BOOL           IsAutoRelease()  const
    { return !(m_nResId & RSC_DONTRELEASE); }

    sal_uInt32     GetId()          const { return m_nResId & ~RSC_DONTRELEASE; }
    RSHEADER_TYPE* GetpResource()   const { return m_pResource; }
};

#endif // _RESID_HXX
