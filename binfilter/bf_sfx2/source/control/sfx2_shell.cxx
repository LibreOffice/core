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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "app.hxx"

namespace binfilter {

//====================================================================

/*N*/ TYPEINIT0(SfxShell);

//====================================================================
/*N*/ SV_IMPL_PTRARR( SfxItemPtrArray, SfxPoolItemPtr);

/*N*/ struct SfxShell_Impl: public SfxBroadcaster
/*N*/ {
/*N*/ 	String                      aObjectName;// Name des Sbx-Objects
/*N*/ 	SfxItemArray_Impl           aItems;     // Datenaustausch auf Item-Basis
/*N*/ };

/*N*/ SfxShell::SfxShell()
/*N*/ :   pPool(0),
/*N*/ 	pImp(0)
/*N*/ {
/*N*/ 	pImp = new SfxShell_Impl;
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ SfxShell::~SfxShell()
/*N*/ {
/*N*/         delete pImp;
/*N*/ }
//--------------------------------------------------------------------

/*N*/ const SfxPoolItem* SfxShell::GetItem
/*N*/ (
/*N*/ 	USHORT  nSlotId         // Slot-Id des zu erfragenden <SfxPoolItem>s
/*N*/ )   const
/*N*/ {
/*N*/ 	for ( USHORT nPos = 0; nPos < pImp->aItems.Count(); ++nPos )
/*N*/ 		if ( pImp->aItems.GetObject(nPos)->Which() == nSlotId )
/*N*/ 			return pImp->aItems.GetObject(nPos);
/*N*/ 	return 0;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxShell::RemoveItem
/*N*/ (
/*N*/ 	USHORT  nSlotId         // Slot-Id des zu l"oschenden <SfxPoolItem>s
/*N*/ )
/*N*/ {
/*N*/ 	for ( USHORT nPos = 0; nPos < pImp->aItems.Count(); ++nPos )
/*?*/ 		if ( pImp->aItems.GetObject(nPos)->Which() == nSlotId )
/*?*/ 		{
/*?*/ 			// Item entfernen und l"oschen
/*?*/ 			SfxPoolItem *pItem = pImp->aItems.GetObject(nPos);
/*?*/ 			delete pItem;
/*?*/ 			pImp->aItems.Remove(nPos);
/*?*/
/*N*/ 		}
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxShell::PutItem
/*N*/ (
/*N*/ 	const SfxPoolItem&  rItem   /*  Instanz, von der eine Kopie erstellt wird,
                                    die in der SfxShell in einer Liste
                                    gespeichert wird. */
/*N*/ )
/*N*/ {
/*N*/ 	DBG_ASSERT( !rItem.ISA(SfxSetItem), "SetItems aren't allowed here" );
/*N*/ 	DBG_ASSERT( SfxItemPool::IsSlot( rItem.Which() ),
/*N*/ 				"items with Which-Ids aren't allowed here" );
/*N*/
/*N*/ 	// MSC auf WNT/W95 machte hier Mist, Vorsicht bei Umstellungen
/*N*/ 	const SfxPoolItem *pItem = rItem.Clone();
/*N*/ 	SfxPoolItemHint aItemHint( (SfxPoolItem*) pItem );
/*N*/ 	const USHORT nWhich = rItem.Which();
/*N*/ 	SfxPoolItem **ppLoopItem = (SfxPoolItem**) pImp->aItems.GetData();
/*N*/ 	USHORT nPos;
/*N*/ 	for ( nPos = 0; nPos < pImp->aItems.Count(); ++nPos, ++ppLoopItem )
/*N*/ 	{
/*N*/ 		if ( (*ppLoopItem)->Which() == nWhich )
/*N*/ 		{
/*N*/ 			// Item austauschen
/*N*/ 			delete *ppLoopItem;
/*N*/ 			pImp->aItems.Remove(nPos);
/*N*/ 			pImp->aItems.Insert( (SfxPoolItemPtr) pItem, nPos );
/*N*/
/*N*/ 			return;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	Broadcast( aItemHint );
/*N*/ 	pImp->aItems.Insert((SfxPoolItemPtr)pItem, nPos );
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
