/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <nochaos.hxx>
#include <sfx2/sfxuno.hxx>


#define WID_CHAOS_START 500




class CntItemPool;

class CntStaticPoolDefaults_Impl
{
    sal_uInt32        m_nItems;
    SfxPoolItem** m_ppDefaults;
    SfxItemInfo*  m_pItemInfos;

private:
    
    CntStaticPoolDefaults_Impl( const CntStaticPoolDefaults_Impl& );
    CntStaticPoolDefaults_Impl& operator=( const CntStaticPoolDefaults_Impl& );

    inline void Insert( SfxPoolItem* pItem, sal_uInt16 nSID, sal_uInt16 nFlags );

public:
    CntStaticPoolDefaults_Impl( CntItemPool* pPool );
    ~CntStaticPoolDefaults_Impl();

    SfxPoolItem**      GetDefaults() const  { return m_ppDefaults; }
    const SfxItemInfo* GetItemInfos() const { return m_pItemInfos; }
};





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





SfxItemPool* NoChaos::GetItemPool()
{
    
    return CntItemPool::Acquire();
}



sal_uInt16 NoChaos::ReleaseItemPool()
{
    
    return CntItemPool::Release();
}





static CntStaticPoolDefaults_Impl* pPoolDefs_Impl = NULL;


CntItemPool* CntItemPool::_pThePool = NULL;


CntItemPool::CntItemPool()
: SfxItemPool( "chaos", WID_CHAOS_START, WID_CHAOS_START, NULL ),
  _nRefs( 0 )
{
    SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );

    FreezeIdRanges();

    
    pPoolDefs_Impl = new CntStaticPoolDefaults_Impl( this );

    
    SetItemInfos( pPoolDefs_Impl->GetItemInfos() );

    
    SetDefaults( pPoolDefs_Impl->GetDefaults() );
}



CntItemPool::~CntItemPool()
{
    
    ReleaseDefaults( false );
}



CntItemPool* CntItemPool::Acquire()
{
    if ( !_pThePool )
        _pThePool = new CntItemPool;

    _pThePool->_nRefs++;

    return _pThePool;
}



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





inline void CntStaticPoolDefaults_Impl::Insert(
         SfxPoolItem* pItem,        /* Static Pool Default Item */
         sal_uInt16 nSID, sal_uInt16 nFlags  /* Item Info */    )
{
    sal_uInt16 nPos = pItem->Which() - WID_CHAOS_START;

    m_ppDefaults[ nPos ]         = pItem;
    m_pItemInfos[ nPos ]._nSID   = nSID;
    m_pItemInfos[ nPos ]._nFlags = nFlags;
}


CntStaticPoolDefaults_Impl::~CntStaticPoolDefaults_Impl()
{
    for ( sal_uInt32 n = 0; n < m_nItems; ++n )
        delete m_ppDefaults[ n ];

    delete [] m_ppDefaults;
    delete [] m_pItemInfos;
}


CntStaticPoolDefaults_Impl::CntStaticPoolDefaults_Impl( CntItemPool* /*pPool*/ )
: m_nItems( 1 ),
  m_ppDefaults( new SfxPoolItem* [ m_nItems ] ),
  m_pItemInfos( new SfxItemInfo  [ m_nItems ] )
{
    memset( m_ppDefaults, 0, sizeof( SfxPoolItem* ) * m_nItems );
    memset( m_pItemInfos, 0, sizeof( SfxItemInfo ) * m_nItems );
    Insert(
        new SfxStringItem( WID_CHAOS_START, OUString() ),
        0,
        SFX_ITEM_POOLABLE );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
