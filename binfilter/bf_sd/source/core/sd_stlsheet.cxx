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

#ifndef _EEITEMID_HXX //autogen
#include <bf_svx/eeitemid.hxx>
#endif

#ifndef _EEITEM_HXX
#include <bf_svx/eeitem.hxx>
#endif

#include <bf_svtools/itemset.hxx>
#include <bf_svtools/smplhint.hxx>
#include <bf_svx/xdef.hxx>
#include <bf_svx/svddef.hxx>
#include <bf_svx/svdoattr.hxx>
#include "stlsheet.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "glob.hrc"
#include "glob.hxx"

namespace binfilter {

/*N*/ TYPEINIT1(SdStyleSheet, SfxStyleSheet);

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

/*N*/ SdStyleSheet::SdStyleSheet(const String& rName, SfxStyleSheetBasePool& rPool,
/*N*/ 						   SfxStyleFamily eFamily, USHORT nMask) :
/*N*/ 	SfxStyleSheet(rName, rPool, eFamily, nMask)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

/*N*/ SdStyleSheet::~SdStyleSheet()
/*N*/ {
/*N*/ 	delete pSet;
/*N*/ 	pSet = NULL;	// damit nachfolgende Destruktoren eine Chance haben
/*N*/ }

/*************************************************************************
|*
|* Load
|*
\************************************************************************/

/*N*/ void SdStyleSheet::Load (SvStream& rIn, USHORT nVersion)
/*N*/ {
/*N*/ 	SfxStyleSheetBase::Load(rIn, nVersion);
/*N*/ 
/*N*/ 	// Die Default-Maske war frueher 0xAFFE.
/*N*/ 	// Aus dieser Default-Maske wurden die benoetigten Flags ausmaskiert.
/*N*/ 	// Nun wurde das Flag SFXSTYLEBIT_READONLY eingefuehrt, was dazu
/*N*/ 	// das alle StyleSheets read-only waren.
/*N*/ 	// Da im Draw kein StyleSheet read-only sein soll, wird an dieser Stelle
/*N*/ 	// das Flag zurueckgesetzt.
/*N*/ 	nMask &= ~SFXSTYLEBIT_READONLY;
/*N*/ }

/*************************************************************************
|*
|* Store
|*
\************************************************************************/

/*N*/ void SdStyleSheet::Store(SvStream& rOut)
/*N*/ {
/*N*/ 	SfxStyleSheetBase::Store(rOut);
/*N*/ }

/*************************************************************************
|*
|* Parent setzen
|*
\************************************************************************/

/*N*/ BOOL SdStyleSheet::SetParent(const String& rParentName)
/*N*/ {
/*N*/ 	BOOL bResult = FALSE;
/*N*/ 
/*N*/ 	if (SfxStyleSheet::SetParent(rParentName))
/*N*/ 	{
/*N*/ 		// PseudoStyleSheets haben keine eigenen ItemSets
/*N*/ 		if (nFamily != SFX_STYLE_FAMILY_PSEUDO)
/*N*/ 		{
/*N*/ 			if( rParentName.Len() )
/*N*/ 			{
/*N*/ 				SfxStyleSheetBase* pStyle = rPool.Find(rParentName, nFamily);
/*N*/ 				if (pStyle)
/*N*/ 				{
/*N*/ 					bResult = TRUE;
/*N*/ 					SfxItemSet& rParentSet = pStyle->GetItemSet();
/*N*/ 					GetItemSet().SetParent(&rParentSet);
/*N*/ 					Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bResult = TRUE;
/*N*/ 				GetItemSet().SetParent(NULL);
/*N*/ 				Broadcast( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			bResult = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bResult;
/*N*/ }

/*************************************************************************
|*
|* ItemSet ggfs. erzeugen und herausreichen
|*
\************************************************************************/

/*N*/ SfxItemSet& SdStyleSheet::GetItemSet()
/*N*/ {
/*N*/ 	if (nFamily == SFX_STYLE_FAMILY_PARA || nFamily == SD_LT_FAMILY)
/*N*/ 	{
/*N*/ 		// ggfs. das ItemSet 'on demand' anlegen
/*N*/ 		if (!pSet)
/*N*/ 		{
/*N*/ 			USHORT nWhichPairTable[] = { XATTR_LINE_FIRST, 				XATTR_LINE_LAST,
/*N*/ 										 XATTR_FILL_FIRST, 				XATTR_FILL_LAST,
/*N*/ 
/*N*/ 										 SDRATTR_SHADOW_FIRST, 			SDRATTR_SHADOW_LAST,
/*N*/ 										 SDRATTR_TEXT_MINFRAMEHEIGHT,	SDRATTR_TEXT_HORZADJUST,
/*N*/ 										 SDRATTR_TEXT_ANIKIND,			SDRATTR_TEXT_CONTOURFRAME,
/*N*/ 										 SDRATTR_EDGE_FIRST,			SDRATTR_EDGE_LAST,
/*N*/ 										 SDRATTR_MEASURE_FIRST, 		SDRATTR_MEASURE_LAST,
/*N*/ 
/*N*/ 										 EE_PARA_START, 				EE_CHAR_END,
/*N*/ 
/*N*/ 										 SDRATTR_XMLATTRIBUTES,			SDRATTR_XMLATTRIBUTES,
/*N*/ 
/*N*/ 										 SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
/*N*/ 										 0, 0 };
/*N*/ 
/*N*/ 			pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
/*N*/ 		}
/*N*/ 
/*N*/ 		return *pSet;
/*N*/ 	}
/*N*/ 
/*N*/ 	// dies ist eine Stellvertretervorlage fuer die interne Vorlage des
/*N*/ 	// aktuellen Praesentationslayouts: dessen ItemSet returnieren
/*N*/ 	else
/*N*/ 	{
/*N*/ //        return (GetRealStyleSheet()->GetItemSet());
/*N*/ 
/*N*/ 		SdStyleSheet* pSdSheet = GetRealStyleSheet();
/*N*/ 
/*N*/ 		if (pSdSheet)
/*N*/ 		{
/*N*/ 			return(pSdSheet->GetItemSet());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if (!pSet)
/*N*/ 			{
/*N*/ 				USHORT nWhichPairTable[] = { XATTR_LINE_FIRST,              XATTR_LINE_LAST,
/*N*/ 											 XATTR_FILL_FIRST,              XATTR_FILL_LAST,
/*N*/ 
/*N*/ 											 SDRATTR_SHADOW_FIRST,          SDRATTR_SHADOW_LAST,
/*N*/ 											 SDRATTR_TEXT_MINFRAMEHEIGHT,   SDRATTR_TEXT_HORZADJUST,
/*N*/ 											 SDRATTR_TEXT_ANIKIND,          SDRATTR_TEXT_CONTOURFRAME,
/*N*/ 											 SDRATTR_EDGE_FIRST,            SDRATTR_EDGE_LAST,
/*N*/ 											 SDRATTR_MEASURE_FIRST,         SDRATTR_MEASURE_LAST,
/*N*/ 
/*N*/ 											 EE_PARA_START,                 EE_CHAR_END,
/*N*/ 
/*N*/ 											SDRATTR_XMLATTRIBUTES,			SDRATTR_XMLATTRIBUTES,
/*N*/ 
/*N*/ 											SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
/*N*/ 											 0, 0 };
/*N*/ 
/*N*/ 				pSet = new SfxItemSet(GetPool().GetPool(), nWhichPairTable);
/*N*/ 			}
/*N*/ 
/*N*/ 			return(*pSet);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* IsUsed(), eine Vorlage gilt als benutzt, wenn sie von eingefuegten Objekten
|*			 oder von benutzten Vorlagen referenziert wird
|*
\************************************************************************/

/*N*/ BOOL SdStyleSheet::IsUsed() const
/*N*/ {
/*N*/ 	BOOL bResult = FALSE;
/*N*/ 
/*N*/ 	USHORT nListenerCount = GetListenerCount();
/*N*/ 	if (nListenerCount > 0)
/*?*/ 	{
/*?*/ 		for (USHORT n = 0; n < nListenerCount; n++)
/*?*/ 		{
/*?*/ 			SfxListener* pListener = GetListener(n);
            // NULL-Pointer ist im Listener-Array erlaubt
/*?*/ 			if (pListener && pListener->ISA(SdrAttrObj))
/*?*/ 			{
/*?*/ 				bResult = ((SdrAttrObj*)pListener)->IsInserted();
/*?*/ 			}
/*?*/ 			else if (pListener && pListener->ISA(SfxStyleSheet))
/*?*/ 			{
/*?*/ 				bResult = ((SfxStyleSheet*)pListener)->IsUsed();
/*?*/ 			}
/*?*/ 			if (bResult)
/*?*/ 				break;
/*?*/ 		}
/*?*/ 	}
/*N*/ 	return bResult;
/*N*/ }

/*************************************************************************
|*
|* CreateChildList, erstellt eine Liste der unmittelbaren Kinder dieser
|* Vorlage; die Liste gehoert dem Caller!
|*
\************************************************************************/


/*************************************************************************
|*
|* das StyleSheet ermitteln, fuer das dieses StyleSheet steht
|*
\************************************************************************/

/*N*/ SdStyleSheet* SdStyleSheet::GetRealStyleSheet() const
/*N*/ {
/*N*/ 	String aRealStyle;
/*N*/ 	String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
/*N*/ 	SdStyleSheet* pRealStyle = NULL;
/*N*/ 	SdDrawDocument* pDoc = ((SdStyleSheetPool&) rPool).GetDoc();
/*N*/ 
/*N*/ 	if (aRealStyle.Len() == 0)
/*N*/ 	{
/*N*/ 		SdPage* pPage = pDoc->GetSdPage(0, PK_STANDARD);
/*N*/ 
/*N*/ 		if (pPage)
/*N*/ 		{
/*N*/ 			aRealStyle = pDoc->GetSdPage(0, PK_STANDARD)->GetLayoutName();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Noch keine Seite vorhanden
/*N*/ 			// Dieses kann beim Aktualisieren vonDokumentvorlagen vorkommen
/*?*/ 			SfxStyleSheetIterator aIter(&rPool, SD_LT_FAMILY);
            SfxStyleSheetBase* pSheet = aIter.First();
            if( pSheet )
                aRealStyle = pSheet->GetName();
        }

/*N*/ 		aRealStyle.Erase(aRealStyle.Search(aSep) + aSep.Len());
/*N*/ 	}
/*N*/ 
/*N*/ 	// jetzt vom Namen (landessprachlich angepasst) auf den internen
/*N*/ 	// Namen (unabhaengig von der Landessprache) mappen
/*N*/ 	String aInternalName;
/*N*/ 
/*N*/ 	if (aName == String(SdResId(STR_PSEUDOSHEET_TITLE)))
/*N*/ 	{
/*N*/ 		aInternalName = String(SdResId(STR_LAYOUT_TITLE));
/*N*/ 	}
/*N*/ 	else if (aName == String(SdResId(STR_PSEUDOSHEET_SUBTITLE)))
/*N*/ 	{
/*N*/ 		aInternalName = String(SdResId(STR_LAYOUT_SUBTITLE));
/*N*/ 	}
/*N*/ 	else if (aName == String(SdResId(STR_PSEUDOSHEET_BACKGROUND)))
/*N*/ 	{
/*N*/ 		aInternalName = String(SdResId(STR_LAYOUT_BACKGROUND));
/*N*/ 	}
/*N*/ 	else if (aName == String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS)))
/*N*/ 	{
/*N*/ 		aInternalName = String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
/*N*/ 	}
/*N*/ 	else if (aName == String(SdResId(STR_PSEUDOSHEET_NOTES)))
/*N*/ 	{
/*N*/ 		aInternalName = String(SdResId(STR_LAYOUT_NOTES));
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		String aOutlineStr(SdResId(STR_PSEUDOSHEET_OUTLINE));
/*N*/ 		USHORT nPos = aName.Search(aOutlineStr);
/*N*/ 		if (nPos != STRING_NOTFOUND)
/*N*/ 		{
/*N*/ 			String aNumStr(aName.Copy(aOutlineStr.Len()));
/*N*/ 			aInternalName = String(SdResId(STR_LAYOUT_OUTLINE));
/*N*/ 			aInternalName += aNumStr;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	aRealStyle += aInternalName;
/*N*/ 	pRealStyle = (SdStyleSheet*)rPool.Find(aRealStyle, SD_LT_FAMILY);
/*N*/ 	if( !pRealStyle )
/*N*/ 	{
/*N*/ 		SfxStyleSheetIterator aIter(&rPool, SD_LT_FAMILY);
/*N*/ 		if( aIter.Count() > 0 )
/*N*/ 			// StyleSheet not found, but pool already loaded
/*N*/ 			DBG_ASSERT(pRealStyle, "Internal StyleSheet not found");
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRealStyle;
/*N*/ }

/*************************************************************************
|*
|* das PseudoStyleSheet ermitteln, durch das dieses StyleSheet vertreten wird
|*
\************************************************************************/



/*************************************************************************
|*
|* Notify
|*
\************************************************************************/

/*N*/ void SdStyleSheet::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
/*N*/ 							  const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	// erstmal die Basisklassenfunktionalitaet
/*N*/ 	SfxStyleSheet::Notify(rBC, rHint);
/*N*/ 
/*N*/ 	// wenn der Stellvertreter ein Notify bezueglich geaenderter Attribute
/*N*/ 	// bekommt, sorgt er dafuer, dass das eigentlich gemeinte StyleSheet
/*N*/ 	// broadcastet
/*N*/ 	SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
/*N*/ 	ULONG nId = pSimple == NULL ? 0 : pSimple->GetId();
/*N*/ 	if (nId == SFX_HINT_DATACHANGED && nFamily == SFX_STYLE_FAMILY_PSEUDO)
/*N*/ 	{
/*?*/ 		SdStyleSheet* pRealStyle = GetRealStyleSheet();
/*?*/ 		if (pRealStyle)
/*?*/ 			pRealStyle->Broadcast(rHint);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|* AdjustToFontHeight passt die Bulletbreite und den linken Texteinzug
|* des uebergebenen ItemSets dessen Fonthoehe an. Die neuen Werte werden so
|* berechnet, dass das Verhaeltnis zur Fonthoehe so ist wie im StyleSheet.
|*
|* bOnlyMissingItems legt fest, ob lediglich nicht gesetzte Items ergaenzt
|* (TRUE) oder explizit gesetzte Items ueberschreiben werden sollen (FALSE)
|*
\************************************************************************/






}
