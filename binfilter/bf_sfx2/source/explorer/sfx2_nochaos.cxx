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

#include <bf_svtools/itempool.hxx>
#include <bf_svtools/stritem.hxx>
#include <nochaos.hxx>
#include "sfxuno.hxx"
namespace binfilter {


#define WID_CHAOS_START 500
//=========================================================================
//
// class CntStaticPoolDefaults_Impl
//
//=========================================================================

class CntItemPool;

class CntStaticPoolDefaults_Impl
{
    sal_uInt32		  m_nItems;
    SfxPoolItem** m_ppDefaults;
    SfxItemInfo*  m_pItemInfos;

private:
    // Forbidden and not implemented...
    CntStaticPoolDefaults_Impl( const CntStaticPoolDefaults_Impl& );
    CntStaticPoolDefaults_Impl&	operator=( const CntStaticPoolDefaults_Impl& );

    inline void Insert( SfxPoolItem* pItem, sal_uInt16 nSID, sal_uInt16 nFlags );

public:
    CntStaticPoolDefaults_Impl( CntItemPool* pPool );
    ~CntStaticPoolDefaults_Impl();

    SfxPoolItem** 	   GetDefaults() const  { return m_ppDefaults; }
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
    static USHORT 		Release();
};

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// static
/*N*/ SfxItemPool* NoChaos::GetItemPool()
/*N*/ {
/*N*/ 	// Get and hold CHAOS item pool.
/*N*/ 	return CntItemPool::Acquire();
/*N*/ }

//----------------------------------------------------------------------------
// static
/*N*/ USHORT NoChaos::ReleaseItemPool()
/*N*/ {
/*N*/ 	// Release CHAOS item pool.
/*N*/ 	return CntItemPool::Release();
/*N*/ }

//=========================================================================
//
//	CntItemPool implementation
//
//=========================================================================

static CntStaticPoolDefaults_Impl* pPoolDefs_Impl = NULL;

// static member!
CntItemPool* CntItemPool::_pThePool = NULL;

//-------------------------------------------------------------------------
/*N*/ CntItemPool::CntItemPool()
/*N*/ : SfxItemPool( DEFINE_CONST_UNICODE("chaos"), WID_CHAOS_START, WID_CHAOS_START, NULL ),
/*N*/   _nRefs( 0 )
/*N*/ {
/*N*/     SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
/*N*/ 
/*N*/ 	FreezeIdRanges();
/*N*/ 
/*N*/ 	// Create static defaults.
/*N*/ 	pPoolDefs_Impl = new CntStaticPoolDefaults_Impl( this );
/*N*/ 
/*N*/ 	// Set item infos.
/*N*/ 	SetItemInfos( pPoolDefs_Impl->GetItemInfos() );
/*N*/ 
/*N*/ 	// Set static pool default items.
/*N*/ 	SetDefaults( pPoolDefs_Impl->GetDefaults() );
/*N*/ }

//-------------------------------------------------------------------------
//virtual
/*N*/ CntItemPool::~CntItemPool()
/*N*/ {
/*N*/ 	// Release static pool default items.
/*N*/ 	ReleaseDefaults( sal_False );
/*N*/ }

//-------------------------------------------------------------------------
// static
/*N*/ CntItemPool* CntItemPool::Acquire()
/*N*/ {
/*N*/ 	if ( !_pThePool )
/*N*/ 		_pThePool = new CntItemPool;
/*N*/ 
/*N*/ 	_pThePool->_nRefs++;
/*N*/ 
/*N*/ 	return _pThePool;
/*N*/ }

//-------------------------------------------------------------------------
// static
/*N*/ sal_uInt16 CntItemPool::Release()
/*N*/ {
/*N*/ 	if ( !_pThePool )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	sal_uInt16& nRefs = _pThePool->_nRefs;
/*N*/ 
/*N*/ 	if ( nRefs )
/*N*/ 		--nRefs;
/*N*/ 
/*N*/ 	if ( !nRefs )
/*N*/ 	{
/*N*/ 		DELETEZ( _pThePool );
/*N*/ 		DELETEZ( pPoolDefs_Impl );
/*N*/ 		return 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	return nRefs;
/*N*/ }

//=========================================================================
//
// CntStaticPoolDefaults_Impl implementation.
//
//=========================================================================

/*?*/ inline void CntStaticPoolDefaults_Impl::Insert(
/*?*/ 	 	SfxPoolItem* pItem, 		/* Static Pool Default Item */
/*?*/      	sal_uInt16 nSID, sal_uInt16 nFlags  /* Item Info */	)
/*?*/ {
/*?*/ 	sal_uInt16 nPos = pItem->Which() - WID_CHAOS_START;
/*?*/ 
/*?*/ 	m_ppDefaults[ nPos ]         = pItem;
/*?*/ 	m_pItemInfos[ nPos ]._nSID   = nSID;
/*?*/ 	m_pItemInfos[ nPos ]._nFlags = nFlags;
/*?*/ }

//-------------------------------------------------------------------------
/*N*/ CntStaticPoolDefaults_Impl::~CntStaticPoolDefaults_Impl()
/*N*/ {
/*N*/ 	for ( sal_uInt32 n = 0; n < m_nItems; ++n )
/*N*/ 		delete m_ppDefaults[ n ];
/*N*/ 
/*N*/ 	delete [] m_ppDefaults;
/*N*/ 	delete [] m_pItemInfos;
/*N*/ }

//-------------------------------------------------------------------------
/*N*/ CntStaticPoolDefaults_Impl::CntStaticPoolDefaults_Impl( CntItemPool* pPool )
/*N*/ : m_nItems( 1 ),
/*N*/   m_ppDefaults( new SfxPoolItem* [ m_nItems ] ),
/*N*/   m_pItemInfos( new SfxItemInfo  [ m_nItems ] )
/*N*/ {
/*N*/ 	rtl_zeroMemory( m_ppDefaults, sizeof( SfxPoolItem* ) * m_nItems );
/*N*/ 	rtl_zeroMemory( m_pItemInfos, sizeof( SfxItemInfo ) * m_nItems );
/*N*/ 
/*N*/ 	Insert(
/*N*/ 		new	SfxStringItem( WID_CHAOS_START, String() ),
/*N*/ 		0,
/*N*/ 		SFX_ITEM_POOLABLE );
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
