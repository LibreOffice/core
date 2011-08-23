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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "cfgmgr.hxx"
#include "sfx.hrc"
#include "cfgimpl.hxx"
#include <tools/debug.hxx>
namespace binfilter {
// class SfxConfigItem ---------------------------------------------------

/*N*/ SfxConfigItem::SfxConfigItem( USHORT nType, SfxConfigManager* pCfgMgr )
/*N*/ 	: m_pCfgMgr( pCfgMgr )
/*N*/ 	, m_nType( nType )
/*N*/ 	, m_bDefault( TRUE )
/*N*/     , m_bModified( FALSE )
/*N*/     , m_bInitialized( FALSE )
/*N*/ {
/*N*/     if ( pCfgMgr )
/*N*/         pCfgMgr->AddConfigItem( *this );
/*N*/ }

/*N*/ SfxConfigItem::~SfxConfigItem()
/*N*/ {
/*N*/     if ( m_pCfgMgr )
/*N*/         m_pCfgMgr->RemoveConfigItem( *this );
/*N*/ }

// ----------------------------------------------------------------------
/*N*/ BOOL SfxConfigItem::Initialize()
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	if ( !m_pCfgMgr )
/*N*/ 		UseDefault();
/*N*/ 	else
/*N*/         bRet = m_pCfgMgr->LoadConfigItem( *this );
/*N*/     m_bModified = FALSE;
/*N*/     m_bInitialized = TRUE;
/*N*/ 	return bRet;
/*N*/ }

// ----------------------------------------------------------------------
/*N*/ BOOL SfxConfigItem::StoreConfig()
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/     if ( m_pCfgMgr && m_bModified )
/*?*/         bRet = m_pCfgMgr->StoreConfigItem( *this );
/*N*/ 	m_bModified = FALSE;
/*N*/ 	return bRet;
/*N*/ }

// ------------------------------------------------------------------------
/*N*/ void SfxConfigItem::SetDefault( BOOL bOn )
/*N*/ {
/*N*/     if ( m_bInitialized && (!bOn || !m_bDefault) )
/*?*/ 		SetModified( TRUE );
/*N*/ 	m_bDefault = bOn;
/*N*/ }

//-------------------------------------------------------------------------
/*N*/ void SfxConfigItem::SetModified( BOOL bMod )
/*N*/ {
/*N*/     if ( m_bInitialized )
/*N*/     {
/*N*/         m_bModified = bMod;
/*N*/         if ( bMod && m_pCfgMgr )
/*?*/             m_pCfgMgr->SetModified( bMod );
/*N*/     }
/*N*/ }

/*?*/ BOOL SfxConfigItem::ReInitialize()
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE; //STRIP001 
/*?*/ }

/*?*/ String SfxConfigItem::GetStreamName( USHORT nType )
/*?*/ {
/*?*/     return String();
/*?*/ }

/*N*/ void SfxConfigItem::Connect( SfxConfigManager* pMgr )
/*N*/ {
/*N*/ 	m_pCfgMgr = pMgr; pMgr->AddConfigItem(*this);
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
