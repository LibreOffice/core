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
#include <bf_svtools/itemset.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "request.hxx"

namespace binfilter {

/*N*/ const SfxPoolItem* SfxRequest::GetItem
/*N*/ (
/*N*/ 	const SfxItemSet* pArgs,
/*N*/ 	USHORT 			nSlotId, 	// Slot-Id oder Which-Id des Parameters
/*N*/ 	FASTBOOL 		bDeep,	 	// FALSE: nicht in Parent-ItemSets suchen
/*N*/ 	TypeId			aType		// != 0:  RTTI Pruefung mit Assertion
/*N*/ )
/*N*/ {
/*N*/ 	if ( pArgs )
/*N*/ 	{
/*N*/ 		// ggf. in Which-Id umrechnen
/*N*/ 		USHORT nWhich = pArgs->GetPool()->GetWhich(nSlotId);
/*N*/
/*N*/ 		// ist das Item gesetzt oder bei bDeep==TRUE verf"ugbar?
/*N*/ 		const SfxPoolItem *pItem = 0;
/*N*/ 		if ( ( bDeep ? SFX_ITEM_AVAILABLE : SFX_ITEM_SET )
/*N*/ 			 <= pArgs->GetItemState( nWhich, bDeep, &pItem ) )
/*N*/ 		{
/*N*/ 			// stimmt der Typ "uberein?
/*N*/ 			if ( !pItem || pItem->IsA(aType) )
/*N*/ 				return pItem;
/*N*/
/*N*/ 			// Item da aber falsch => Programmierfehler
/*N*/ 			DBG_ERROR(  "invalid argument type" );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// keine Parameter, nicht gefunden oder falschen Typ gefunden
/*N*/ 	return 0;
/*N*/ }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
