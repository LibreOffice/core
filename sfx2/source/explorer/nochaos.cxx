/*************************************************************************
 *
 *  $RCSfile: nochaos.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    SetFileFormatVersion( SOFFICE_FILEFORMAT_NOW );

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
