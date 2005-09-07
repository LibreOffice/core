/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nochaos.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:52:22 $
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

#include <svtools/itempool.hxx>
#include <svtools/poolitem.hxx>
#include <svtools/stritem.hxx>
#include <nochaos.hxx>
#include "sfxuno.hxx"


#define WID_CHAOS_START 500
//=========================================================================
//
// class CntStaticPoolDefaults_Impl
//
//=========================================================================

class CntItemPool;

class CntStaticPoolDefaults_Impl
{
    sal_uInt32        m_nItems;
    SfxPoolItem** m_ppDefaults;
    SfxItemInfo*  m_pItemInfos;

private:
    // Forbidden and not implemented...
    CntStaticPoolDefaults_Impl( const CntStaticPoolDefaults_Impl& );
    CntStaticPoolDefaults_Impl& operator=( const CntStaticPoolDefaults_Impl& );

    inline void Insert( SfxPoolItem* pItem, sal_uInt16 nSID, sal_uInt16 nFlags );

public:
    CntStaticPoolDefaults_Impl( CntItemPool* pPool );
    ~CntStaticPoolDefaults_Impl();

    SfxPoolItem**      GetDefaults() const  { return m_ppDefaults; }
    const SfxItemInfo* GetItemInfos() const { return m_pItemInfos; }
};

//----------------------------------------------------------------------------

//=========================================================================

class CntItemPool: public SfxItemPool
{
    static CntItemPool* _pThePool;
    USHORT              _nRefs;

protected:
             CntItemPool();
    virtual ~CntItemPool();

public:
    static CntItemPool* Acquire();
    static USHORT       Release();
};

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// static
SfxItemPool* NoChaos::GetItemPool()
{
    // Get and hold CHAOS item pool.
    return CntItemPool::Acquire();
}

//----------------------------------------------------------------------------
// static
USHORT NoChaos::ReleaseItemPool()
{
    // Release CHAOS item pool.
    return CntItemPool::Release();
}

//=========================================================================
//
//  CntItemPool implementation
//
//=========================================================================

static CntStaticPoolDefaults_Impl* pPoolDefs_Impl = NULL;

// static member!
CntItemPool* CntItemPool::_pThePool = NULL;

//-------------------------------------------------------------------------
CntItemPool::CntItemPool()
: SfxItemPool( DEFINE_CONST_UNICODE("chaos"), WID_CHAOS_START, WID_CHAOS_START, NULL ),
  _nRefs( 0 )
{
    SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );

    FreezeIdRanges();

    // Create static defaults.
    pPoolDefs_Impl = new CntStaticPoolDefaults_Impl( this );

    // Set item infos.
    SetItemInfos( pPoolDefs_Impl->GetItemInfos() );

    // Set static pool default items.
    SetDefaults( pPoolDefs_Impl->GetDefaults() );
}

//-------------------------------------------------------------------------
//virtual
CntItemPool::~CntItemPool()
{
    // Release static pool default items.
    ReleaseDefaults( sal_False );
}

//-------------------------------------------------------------------------
// static
CntItemPool* CntItemPool::Acquire()
{
    if ( !_pThePool )
        _pThePool = new CntItemPool;

    _pThePool->_nRefs++;

    return _pThePool;
}

//-------------------------------------------------------------------------
// static
sal_uInt16 CntItemPool::Release()
{
    if ( !_pThePool )
        return 0;

    sal_uInt16& nRefs = _pThePool->_nRefs;

    if ( nRefs )
        --nRefs;

    if ( !nRefs )
    {
        DELETEZ( _pThePool );
        DELETEZ( pPoolDefs_Impl );
        return 0;
    }

    return nRefs;
}

//=========================================================================
//
// CntStaticPoolDefaults_Impl implementation.
//
//=========================================================================

inline void CntStaticPoolDefaults_Impl::Insert(
         SfxPoolItem* pItem,        /* Static Pool Default Item */
         sal_uInt16 nSID, sal_uInt16 nFlags  /* Item Info */    )
{
    sal_uInt16 nPos = pItem->Which() - WID_CHAOS_START;

    m_ppDefaults[ nPos ]         = pItem;
    m_pItemInfos[ nPos ]._nSID   = nSID;
    m_pItemInfos[ nPos ]._nFlags = nFlags;
}

//-------------------------------------------------------------------------
CntStaticPoolDefaults_Impl::~CntStaticPoolDefaults_Impl()
{
    for ( sal_uInt32 n = 0; n < m_nItems; ++n )
        delete m_ppDefaults[ n ];

    delete [] m_ppDefaults;
    delete [] m_pItemInfos;
}

//-------------------------------------------------------------------------
CntStaticPoolDefaults_Impl::CntStaticPoolDefaults_Impl( CntItemPool* pPool )
: m_nItems( 1 ),
  m_ppDefaults( new SfxPoolItem* [ m_nItems ] ),
  m_pItemInfos( new SfxItemInfo  [ m_nItems ] )
{
    rtl_zeroMemory( m_ppDefaults, sizeof( m_ppDefaults ) * m_nItems );
    rtl_zeroMemory( m_pItemInfos, sizeof( m_pItemInfos ) * m_nItems );

    Insert(
        new SfxStringItem( WID_CHAOS_START, String() ),
        0,
        SFX_ITEM_POOLABLE );
}
