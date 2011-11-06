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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <nochaos.hxx>
#include <sfx2/sfxuno.hxx>


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
    sal_uInt16              _nRefs;

protected:
             CntItemPool();
    virtual ~CntItemPool();

public:
    static CntItemPool* Acquire();
    static sal_uInt16       Release();
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
sal_uInt16 NoChaos::ReleaseItemPool()
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
CntStaticPoolDefaults_Impl::CntStaticPoolDefaults_Impl( CntItemPool* /*pPool*/ )
: m_nItems( 1 ),
  m_ppDefaults( new SfxPoolItem* [ m_nItems ] ),
  m_pItemInfos( new SfxItemInfo  [ m_nItems ] )
{
    rtl_zeroMemory( m_ppDefaults, sizeof( SfxPoolItem* ) * m_nItems );
    rtl_zeroMemory( m_pItemInfos, sizeof( SfxItemInfo ) * m_nItems );
    Insert(
        new SfxStringItem( WID_CHAOS_START, String() ),
        0,
        SFX_ITEM_POOLABLE );
}
