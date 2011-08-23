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

#ifndef _EEITEM_HXX //autogen
#include <bf_svx/eeitem.hxx>
#endif

#ifndef _XDEF_HXX //autogen
#include <bf_svx/xdef.hxx>
#endif


#ifndef _SFXITEMSET_HXX //autogen
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX //autogen
#include <bf_svtools/smplhint.hxx>
#endif

#include "stlsheet.hxx"
namespace binfilter {

/*N*/ TYPEINIT1(SchStyleSheet, SfxStyleSheet);


/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

/*N*/ SchStyleSheet::SchStyleSheet(const String& rName, SfxStyleSheetBasePool& rPool,
/*N*/ 							 SfxStyleFamily eFamily, USHORT nMask) :
/*N*/ 	SfxStyleSheet(rName, rPool, eFamily, nMask)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

/*N*/ SchStyleSheet::~SchStyleSheet()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Load
|*
\************************************************************************/

/*N*/ void SchStyleSheet::Load (SvStream& rIn, USHORT nVersion)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Store
|*
\************************************************************************/

/*N*/ void SchStyleSheet::Store(SvStream& rOut)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Parent setzen
|*
\************************************************************************/

/*N*/ BOOL SchStyleSheet::SetParent(const String& rParentName)
/*N*/ {
/*N*/ 	if (SfxStyleSheet::SetParent(rParentName))
/*N*/ 	{
/*N*/ 		SfxStyleSheetBase* pStyle = rPool.Find(rParentName, nFamily);
/*N*/ 
/*N*/ 		if (pStyle)
/*N*/ 		{
/*?*/ 			SfxItemSet& rParentSet = pStyle->GetItemSet();
/*?*/ 			GetItemSet().SetParent(&rParentSet);
/*?*/ 			Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
/*?*/ 
/*?*/ 			return TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|* ItemSet ggfs. erzeugen und herausreichen
|*
\************************************************************************/

/*N*/ SfxItemSet& SchStyleSheet::GetItemSet()
/*N*/ {
/*N*/ 	if (!pSet)
/*N*/ 	{
/*N*/ 		USHORT nWhichPairTable[] = { XATTR_LINE_FIRST, XATTR_LINE_LAST,
/*N*/ 									 XATTR_FILL_FIRST, XATTR_FILL_LAST,
/*N*/ 									 EE_PARA_START, EE_CHAR_END,
/*N*/ 									 (USHORT)0 };
/*N*/ 
/*N*/ 		pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
/*N*/ 		bMySet = TRUE; //Eigentum erklaeren, damit der DTor der Basisklasse den
/*N*/ 					   //Set wieder abraeumt.
/*N*/ 	}
/*N*/ 
/*N*/ 	return *pSet;
/*N*/ }

/*************************************************************************
|*
|* IsUsed(), wird an Listeners erkannt
|*
\************************************************************************/


/*************************************************************************
|*
|*
|*
\************************************************************************/


/*************************************************************************
|*
|*
|*
\************************************************************************/




}
