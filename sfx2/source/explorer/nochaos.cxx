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

#include <sal/config.h>

#include <tools/solar.h>
#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <nochaos.hxx>
#include <memory>


#define WID_CHAOS_START 500


namespace {

class CntItemPool;

class CntStaticPoolDefaults_Impl
{
    static const sal_uInt32  m_nItems = 1;
    std::vector<SfxPoolItem*> mvDefaults;
    std::unique_ptr<SfxItemInfo[]>  m_pItemInfos;

private:
    inline void Insert( SfxPoolItem* pItem );

public:
    explicit CntStaticPoolDefaults_Impl();
    ~CntStaticPoolDefaults_Impl();
    CntStaticPoolDefaults_Impl(const CntStaticPoolDefaults_Impl&) = delete;
    CntStaticPoolDefaults_Impl& operator=(const CntStaticPoolDefaults_Impl&) = delete;

    std::vector<SfxPoolItem*>*  GetDefaults() { return &mvDefaults; }
    const SfxItemInfo*          GetItemInfos() const { return m_pItemInfos.get(); }
};


class CntItemPool: public SfxItemPool
{
    static CntItemPool* _pThePool;
    sal_uInt16              _nRefs;

protected:
             CntItemPool();
    virtual ~CntItemPool() override;

public:
    static CntItemPool* Acquire();
    static sal_uInt16       Release();
};

}

// static
SfxItemPool* NoChaos::GetItemPool()
{
    // Get and hold CHAOS item pool.
    return CntItemPool::Acquire();
}


// static
sal_uInt16 NoChaos::ReleaseItemPool()
{
    // Release CHAOS item pool.
    return CntItemPool::Release();
}


//  CntItemPool implementation


static CntStaticPoolDefaults_Impl* pPoolDefs_Impl = nullptr;

// static member!
CntItemPool* CntItemPool::_pThePool = nullptr;


CntItemPool::CntItemPool()
: SfxItemPool( "chaos", WID_CHAOS_START, WID_CHAOS_START, nullptr ),
  _nRefs( 0 )
{
    FreezeIdRanges();

    // Create static defaults.
    pPoolDefs_Impl = new CntStaticPoolDefaults_Impl;

    // Set item infos.
    SetItemInfos( pPoolDefs_Impl->GetItemInfos() );

    // Set static pool default items.
    SetDefaults( pPoolDefs_Impl->GetDefaults() );
}


//virtual
CntItemPool::~CntItemPool()
{
    // Release static pool default items.
    ReleaseDefaults();
}


// static
CntItemPool* CntItemPool::Acquire()
{
    if ( !_pThePool )
        _pThePool = new CntItemPool;

    _pThePool->_nRefs++;

    return _pThePool;
}


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
        delete _pThePool;
        _pThePool = nullptr;
        delete pPoolDefs_Impl;
        pPoolDefs_Impl = nullptr;
        return 0;
    }

    return nRefs;
}


// CntStaticPoolDefaults_Impl implementation.


inline void CntStaticPoolDefaults_Impl::Insert(
         SfxPoolItem* pItem        /* Static Pool Default Item */ )
{
    sal_uInt16 nPos = pItem->Which() - WID_CHAOS_START;

    mvDefaults[ nPos ]         = pItem;
    m_pItemInfos[ nPos ]._nSID   = 0;
    m_pItemInfos[ nPos ]._bPoolable = true;
}


CntStaticPoolDefaults_Impl::~CntStaticPoolDefaults_Impl()
{
    for ( sal_uInt32 n = 0; n < m_nItems; ++n )
        delete mvDefaults[ n ];
}


CntStaticPoolDefaults_Impl::CntStaticPoolDefaults_Impl()
: mvDefaults( m_nItems, nullptr ),
  m_pItemInfos( new SfxItemInfo  [ m_nItems ] )
{
    memset( m_pItemInfos.get(), 0, sizeof( SfxItemInfo ) * m_nItems );
    Insert( new SfxStringItem( WID_CHAOS_START, OUString() ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
