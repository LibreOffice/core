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


#include <vcl/svapp.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "eeitem.hxx"
#include "eeitemid.hxx"

#include <vcl/window.hxx>

#include <impedit.hxx>

#include "lrspitem.hxx"
#include "langitem.hxx"

#include <rtl/tencinfo.h>

#include <unolingu.hxx>

namespace binfilter {
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;








#ifndef SVX_LIGHT
#endif





#ifndef SVX_LIGHT
#endif






/*N*/ EditTextObject*	ImpEditEngine::CreateTextObject()
/*N*/ {
/*N*/ 	EditSelection aCompleteSelection;
/*N*/ 	aCompleteSelection.Min() = aEditDoc.GetStartPaM();
/*N*/ 	aCompleteSelection.Max() = aEditDoc.GetEndPaM();
/*N*/ 
/*N*/ 	return CreateTextObject( aCompleteSelection );
/*N*/ }

/*N*/ EditTextObject*	ImpEditEngine::CreateTextObject( EditSelection aSel )
/*N*/ {
/*N*/ 	return CreateBinTextObject( aSel, GetEditTextObjectPool(), aStatus.AllowBigObjects(), nBigTextObjectStart );
/*N*/ }

/*N*/ EditTextObject*	ImpEditEngine::CreateBinTextObject( EditSelection aSel, SfxItemPool* pPool, sal_Bool bAllowBigObjects, sal_uInt16 nBigObjectStart ) const
/*N*/ {
/*N*/ 	BinTextObject* pTxtObj = new BinTextObject( pPool );
/*N*/ 	pTxtObj->SetVertical( IsVertical() );
/*N*/ 	MapUnit eMapUnit = (MapUnit)aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
/*N*/ 	pTxtObj->SetMetric( (sal_uInt16) eMapUnit );
/*N*/ 	if ( pTxtObj->IsOwnerOfPool() )
/*N*/ 		pTxtObj->GetPool()->SetDefaultMetric( (SfxMapUnit) eMapUnit );
/*N*/ 
/*N*/ 	sal_uInt16 nStartNode, nEndNode;
/*N*/ 	sal_uInt32 nTextPortions = 0;
/*N*/ 
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 	nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
/*N*/ 	nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
/*N*/ 
/*N*/ 	sal_Bool bOnlyFullParagraphs = ( aSel.Min().GetIndex() ||
/*N*/ 		( aSel.Max().GetIndex() < aSel.Max().GetNode()->Len() ) ) ?
/*N*/ 			sal_False : sal_True;
/*N*/ 
/*N*/ 	// Vorlagen werden nicht gespeichert!
/*N*/ 	// ( Nur Name und Familie, Vorlage selbst muss in App stehen! )
/*N*/ 
/*N*/ 	pTxtObj->SetScriptType( GetScriptType( aSel ) );
/*N*/ 
/*N*/ 	// ueber die Absaetze iterieren...
/*N*/ 	sal_uInt16 nNode;
/*N*/ 	for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
/*N*/ 	{
/*N*/ 		ContentNode* pNode = aEditDoc.SaveGetObject( nNode );
/*N*/ 		DBG_ASSERT( pNode, "Node nicht gefunden: Search&Replace" );
/*N*/ 
/*N*/ 		if ( bOnlyFullParagraphs )
/*N*/ 		{
/*N*/ 			ParaPortion* pParaPortion = GetParaPortions()[nNode];
/*N*/ 			nTextPortions += pParaPortion->GetTextPortions().Count();
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_uInt16 nStartPos = 0;
/*N*/ 		sal_uInt16 nEndPos = pNode->Len();
/*N*/ 
/*N*/ 		sal_Bool bEmptyPara = nEndPos ? sal_False : sal_True;
/*N*/ 
/*N*/ 		if ( ( nNode == nStartNode ) && !bOnlyFullParagraphs )
/*?*/ 			nStartPos = aSel.Min().GetIndex();
/*N*/ 		if ( ( nNode == nEndNode ) && !bOnlyFullParagraphs )
/*?*/ 			nEndPos = aSel.Max().GetIndex();
/*N*/ 
/*N*/ 
/*N*/ 		ContentInfo* pC = pTxtObj->CreateAndInsertContent();
/*N*/ 
/*N*/ 		// Die Absatzattribute...
/*N*/ 		pC->GetParaAttribs().Set( pNode->GetContentAttribs().GetItems() );
/*N*/ 
/*N*/ 		// Seit der 5.1 werden im Outliner die LRSpaceItems anders interpretiert.
/*N*/ 		// Damit sie in einer 5.0 und aelter richtig sind, muss das Flag
/*N*/ 		// richtig gesetzt sein...
/*N*/ 		sal_Bool bOutliner = aStatus.IsAnyOutliner();
/*N*/ 		for ( sal_uInt16 n = 0; n <=1; n++ )
/*N*/ 		{
/*N*/ 			sal_uInt16 nItemId = n ? EE_PARA_LRSPACE : EE_PARA_OUTLLRSPACE;
/*N*/ 			if ( pC->GetParaAttribs().GetItemState( nItemId ) == SFX_ITEM_ON )
/*N*/ 			{
/*N*/ 				const SvxLRSpaceItem& rItem = (const SvxLRSpaceItem&) pC->GetParaAttribs().Get( nItemId );
/*N*/ 				if ( rItem.IsBulletFI() != bOutliner )
/*N*/ 				{
/*N*/ 					SvxLRSpaceItem aNewItem( rItem );
/*N*/ 					aNewItem.SetBulletFI( bOutliner );
/*N*/ 					pC->GetParaAttribs().Put( aNewItem );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 
/*N*/ 		// Das StyleSheet...
/*N*/ 		if ( pNode->GetStyleSheet() )
/*N*/ 		{
/*N*/ 			pC->GetStyle() = pNode->GetStyleSheet()->GetName();
/*N*/ 			pC->GetFamily() = pNode->GetStyleSheet()->GetFamily();
/*N*/ 		}
/*N*/ 
/*N*/ 		// Der Text...
/*N*/ 		pC->GetText() = pNode->Copy( nStartPos, nEndPos-nStartPos );
/*N*/ 
/*N*/ 		// und die Attribute...
/*N*/ 		sal_uInt16 nAttr = 0;
/*N*/ 		EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 		while ( pAttr )
/*N*/ 		{
/*N*/ 			// In einem leeren Absatz die Attribute behalten!
/*N*/ 			if ( bEmptyPara ||
/*N*/ 				 ( ( pAttr->GetEnd() > nStartPos ) && ( pAttr->GetStart() < nEndPos ) ) )
/*N*/ 			{
/*N*/ 				XEditAttribute* pX = pTxtObj->CreateAttrib( *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
/*N*/ 				// Evtl. korrigieren...
/*N*/ 				if ( ( nNode == nStartNode ) && ( nStartPos != 0 ) )
/*N*/ 				{
/*?*/ 					pX->GetStart() = ( pX->GetStart() > nStartPos ) ? pX->GetStart()-nStartPos : 0;
/*?*/ 					pX->GetEnd() -= nStartPos;
/*N*/ 
/*N*/ 				}
/*N*/ 				if ( nNode == nEndNode )
/*N*/ 				{
/*N*/ 					if ( pX->GetEnd() > (nEndPos-nStartPos) )
/*?*/ 						pX->GetEnd() = nEndPos-nStartPos;
/*N*/ 				}
/*N*/ 				DBG_ASSERT( pX->GetEnd() <= (nEndPos-nStartPos), "CreateBinTextObject: Attribut zu lang!" );
/*N*/ 				if ( !pX->GetLen() && !bEmptyPara )
/*?*/ 					pTxtObj->DestroyAttrib( pX );
/*N*/ 				else
/*N*/ 					pC->GetAttribs().Insert( pX, pC->GetAttribs().Count() );
/*N*/ 			}
/*N*/ 			nAttr++;
/*N*/ 			pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Bei grossen Textobjekten die PortionInfos merken:
/*N*/ 	// Schwelle rauf setzen, wenn Olli die Absaetze nicht mehr zerhackt!
/*N*/ 	if ( bAllowBigObjects && bOnlyFullParagraphs && IsFormatted() && GetUpdateMode() && ( nTextPortions >= nBigObjectStart ) )
/*N*/ 	{
/*?*/ 		XParaPortionList* pXList = new XParaPortionList( GetRefDevice(), aPaperSize.Width() );
/*?*/ 		pTxtObj->SetPortionInfo( pXList );
/*?*/ 		for ( nNode = nStartNode; nNode <= nEndNode; nNode++  )
/*?*/ 		{
/*?*/ 			ParaPortion* pParaPortion = GetParaPortions()[nNode];
/*?*/ 			XParaPortion* pX = new XParaPortion;
/*?*/ 			pXList->Insert( pX, pXList->Count() );
/*?*/ 
/*?*/ 			pX->nHeight = pParaPortion->GetHeight();
/*?*/ 			pX->nFirstLineOffset = pParaPortion->GetFirstLineOffset();
/*?*/ 
/*?*/ 			// Die TextPortions
/*?*/ 			sal_uInt16 nCount = pParaPortion->GetTextPortions().Count();
/*?*/ 			sal_uInt16 n;
/*?*/ 			for ( n = 0; n < nCount; n++ )
/*?*/ 			{
/*?*/ 				TextPortion* pTextPortion = pParaPortion->GetTextPortions()[n];
/*?*/ 				TextPortion* pNew = new TextPortion( *pTextPortion );
/*?*/ 				pX->aTextPortions.Insert( pNew, pX->aTextPortions.Count() );
/*?*/ 			}
/*?*/ 
/*?*/ 			// Die Zeilen
/*?*/ 			nCount = pParaPortion->GetLines().Count();
/*?*/ 			for ( n = 0; n < nCount; n++ )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EditLine* pLine = pParaPortion->GetLines()[n];
/*?*/ 			}
/*?*/ #ifdef DBG_UTIL
/*?*/ 			USHORT nTest, nTPLen = 0, nTxtLen = 0;
/*?*/ 			for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
/*?*/ 				nTPLen += pParaPortion->GetTextPortions().GetObject( --nTest )->GetLen();
/*?*/ 			for ( nTest = pParaPortion->GetLines().Count(); nTest; )
/*?*/ 				nTxtLen += pParaPortion->GetLines().GetObject( --nTest )->GetLen();
/*?*/ 			DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "CreateBinTextObject: ParaPortion not completely formatted!" );
/*?*/ #endif
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return pTxtObj;
/*N*/ }

/*N*/ void ImpEditEngine::SetText( const EditTextObject& rTextObject )
/*N*/ {
/*N*/ 	// Da Setzen eines TextObject ist nicht Undo-faehig!
/*N*/ 	ResetUndoManager();
/*N*/ 	sal_Bool bUpdate = GetUpdateMode();
/*N*/ 	sal_Bool bUndo = IsUndoEnabled();
/*N*/ 
/*N*/ 	SetText( XubString() );
/*N*/ 	EditPaM aPaM = aEditDoc.GetStartPaM();
/*N*/ 
/*N*/ 	SetUpdateMode( sal_False );
/*N*/ 	EnableUndo( sal_False );
/*N*/ 
/*N*/ 	InsertText( rTextObject, EditSelection( aPaM, aPaM ) );
/*N*/ 	SetVertical( rTextObject.IsVertical() );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	DBG_ASSERT( !HasUndoManager() || !GetUndoManager().GetUndoActionCount(), "Woher kommt das Undo in SetText ?!" );
/*N*/ #endif
/*N*/ 	SetUpdateMode( bUpdate );
/*N*/ 	EnableUndo( bUndo );
/*N*/ }

/*N*/ EditSelection ImpEditEngine::InsertText( const EditTextObject& rTextObject, EditSelection aSel )
/*N*/ {
/*N*/     EnterBlockNotifications();
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 	if ( aSel.HasRange() )
/*?*/ 		aSel = ImpDeleteSelection( aSel );
/*N*/ 	EditSelection aNewSel = InsertBinTextObject( (BinTextObject&)rTextObject, aSel.Max() );
/*N*/     LeaveBlockNotifications();
/*N*/     return aNewSel;
/*N*/ 
/*N*/ 	// MT 05/00: InsertBinTextObject direkt hier machen...
/*N*/ }

/*N*/ EditSelection ImpEditEngine::InsertBinTextObject( BinTextObject& rTextObject, EditPaM aPaM )
/*N*/ {
/*N*/ 	// Optimieren:
/*N*/ 	// Kein GetPos undFindParaportion, sondern Index berechnen!
/*N*/ 	EditSelection aSel( aPaM, aPaM );
/*N*/ 
/*N*/ 	sal_Bool bUsePortionInfo = sal_False;
/*N*/ 	sal_Bool bFields = sal_False;
/*N*/ 	XParaPortionList* pPortionInfo = rTextObject.GetPortionInfo();
/*N*/ 
/*N*/ 	if ( pPortionInfo && ( (long)pPortionInfo->GetPaperWidth() == aPaperSize.Width() )
/*N*/ 			&& ( pPortionInfo->GetRefMapMode() == GetRefDevice()->GetMapMode() ) )
/*N*/ 	{
/*?*/ 		if ( ( pPortionInfo->GetRefDevPtr() == (sal_uIntPtr)GetRefDevice() ) ||
/*?*/ 			 ( ( pPortionInfo->GetRefDevType() == OUTDEV_VIRDEV ) &&
/*?*/ 			   ( GetRefDevice()->GetOutDevType() == OUTDEV_VIRDEV ) ) )
/*?*/ 		bUsePortionInfo = sal_True;
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_Bool bConvertItems = sal_False;
/*N*/ 	MapUnit eSourceUnit, eDestUnit;
/*N*/ 	if ( rTextObject.HasMetric() )
/*N*/ 	{
/*N*/ 		eSourceUnit = (MapUnit)rTextObject.GetMetric();
/*N*/ 		eDestUnit = (MapUnit)aEditDoc.GetItemPool().GetMetric( DEF_METRIC );
/*N*/ 		if ( eSourceUnit != eDestUnit )
/*N*/ 			bConvertItems = sal_True;
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_uInt16 nContents = rTextObject.GetContents().Count();
/*N*/ 	sal_uInt16 nPara = aEditDoc.GetPos( aPaM.GetNode() );
/*N*/ 
/*N*/ 	for ( sal_uInt16 n = 0; n < nContents; n++, nPara++ )
/*N*/ 	{
/*N*/ 		ContentInfo* pC = rTextObject.GetContents().GetObject( n );
/*N*/ 		sal_Bool bNewContent = aPaM.GetNode()->Len() ? sal_False: sal_True;
/*N*/ 		sal_uInt16 nStartPos = aPaM.GetIndex();
/*N*/ 
/*N*/ 		aPaM = ImpFastInsertText( aPaM, pC->GetText() );
/*N*/ 
/*N*/ 		ParaPortion* pPortion = FindParaPortion( aPaM.GetNode() );
/*N*/ 		DBG_ASSERT( pPortion, "Blinde Portion in FastInsertText" );
/*N*/ 		pPortion->MarkInvalid( nStartPos, pC->GetText().Len() );
/*N*/ 
/*N*/ 		// Zeicheattribute...
/*N*/ 		sal_Bool bAllreadyHasAttribs = aPaM.GetNode()->GetCharAttribs().Count() ? sal_True : sal_False;
/*N*/ 		sal_uInt16 nNewAttribs = pC->GetAttribs().Count();
/*N*/ 		if ( nNewAttribs )
/*N*/ 		{
/*N*/             BOOL bUpdateFields = FALSE;
/*N*/ 			for ( sal_uInt16 nAttr = 0; nAttr < nNewAttribs; nAttr++ )
/*N*/ 			{
/*N*/ 				XEditAttribute* pX = pC->GetAttribs().GetObject( nAttr );
/*N*/ 				// Kann passieren wenn Absaetze >16K entstehen, dann wird einfach umgebrochen.
/*N*/ 				if ( pX->GetEnd() <= aPaM.GetNode()->Len() )
/*N*/ 				{
/*N*/ 					if ( !bAllreadyHasAttribs || pX->IsFeature() )
/*N*/ 					{
/*N*/ 						// Normale Attribute gehen dann schneller...
/*N*/ 						// Features duerfen nicht ueber EditDoc::InsertAttrib
/*N*/ 						// eingefuegt werden, sie sind bei FastInsertText schon im TextFluss
/*N*/ 						DBG_ASSERT( pX->GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut zu gross!" );
/*N*/ 						EditCharAttrib* pAttr;
/*N*/ 						if ( !bConvertItems )
/*N*/ 							pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *(pX->GetItem()), pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos );
/*N*/ 						else
/*N*/ 						{
/*?*/ 							SfxPoolItem* pNew = pX->GetItem()->Clone();
/*?*/ 							ConvertItem( *pNew, eSourceUnit, eDestUnit );
/*?*/ 							pAttr = MakeCharAttrib( aEditDoc.GetItemPool(), *pNew, pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos );
/*?*/ 							delete pNew;
/*N*/ 						}
/*N*/ 						DBG_ASSERT( pAttr->GetEnd() <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut passt nicht! (1)" );
/*N*/ 						aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttr );
/*N*/ 						if ( pAttr->Which() == EE_FEATURE_FIELD )
/*N*/                             bUpdateFields = TRUE;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						DBG_ASSERT( pX->GetEnd()+nStartPos <= aPaM.GetNode()->Len(), "InsertBinTextObject: Attribut passt nicht! (2)" );
/*?*/ 						// Tabs und andere Features koennen nicht ueber InsertAttrib eingefuegt werden:
/*?*/ 						aEditDoc.InsertAttrib( aPaM.GetNode(), pX->GetStart()+nStartPos, pX->GetEnd()+nStartPos, *pX->GetItem() );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/             if ( bUpdateFields )
/*N*/                 UpdateFields();
/*N*/ 
/*N*/             // Sonst QuickFormat => Keine Attribute!
/*N*/ 			pPortion->MarkSelectionInvalid( nStartPos, pC->GetText().Len() );
/*N*/ 		}
/*N*/ 
/*N*/ 		DBG_ASSERT( CheckOrderedList( aPaM.GetNode()->GetCharAttribs().GetAttribs(), sal_True ), "InsertBinTextObject: Start-Liste verdreht" );
/*N*/ 
/*N*/ 		sal_Bool bParaAttribs = sal_False;
/*N*/ 		if ( bNewContent || ( ( n > 0 ) && ( n < (nContents-1) ) ) )
/*N*/ 		{
/*N*/             bParaAttribs = sal_False;
/*N*/             // #101512# Don't overwrite level/style from existing paragraph in OutlineView
/*N*/             // MT 10/2002: Removed because of #103874#, handled in Outliner::EndPasteOrDropHdl now.
/*N*/ //            if ( !aStatus.IsOutliner() || n )
/*N*/             {
/*N*/ 			    // nur dann Style und ParaAttribs, wenn neuer Absatz, oder
/*N*/ 			    // komplett inneliegender...
/*N*/ 			    bParaAttribs = pC->GetParaAttribs().Count() ? sal_True : sal_False;
/*N*/ 			    if ( GetStyleSheetPool() && pC->GetStyle().Len() )
/*N*/ 			    {
/*N*/ 				    SfxStyleSheet* pStyle = (SfxStyleSheet*)GetStyleSheetPool()->Find( pC->GetStyle(), pC->GetFamily() );
/*N*/ 				    DBG_ASSERT( pStyle, "InsertBinTextObject - Style not found!" );
/*N*/ 				    SetStyleSheet( nPara, pStyle );
/*N*/ 			    }
/*N*/ 			    if ( !bConvertItems )
/*N*/ 				    SetParaAttribs( aEditDoc.GetPos( aPaM.GetNode() ), pC->GetParaAttribs() );
/*N*/ 			    else
/*N*/ 			    {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 			    }
/*N*/             }
/*N*/ 			if ( bNewContent && bUsePortionInfo )
/*N*/ 			{
/*?*/ 				XParaPortion* pXP = pPortionInfo->GetObject( n );
/*?*/ 				DBG_ASSERT( pXP, "InsertBinTextObject: PortionInfo?" );
/*?*/ 				ParaPortion* pParaPortion = GetParaPortions()[ nPara ];
/*?*/ 				DBG_ASSERT( pParaPortion, "InsertBinTextObject: ParaPortion?" );
/*?*/ 				pParaPortion->nHeight = pXP->nHeight;
/*?*/ 				pParaPortion->nFirstLineOffset = pXP->nFirstLineOffset;
/*?*/ 				pParaPortion->bForceRepaint = sal_True;
/*?*/ 				pParaPortion->SetValid();	// Nicht formatieren
/*?*/ 
/*?*/ 				// Die TextPortions
/*?*/ 				pParaPortion->GetTextPortions().Reset();
/*?*/ 				sal_uInt16 nCount = pXP->aTextPortions.Count();
/*?*/ 				for ( sal_uInt16 n = 0; n < nCount; n++ )
/*?*/ 				{
/*?*/ 					TextPortion* pTextPortion = pXP->aTextPortions[n];
/*?*/ 					TextPortion* pNew = new TextPortion( *pTextPortion );
/*?*/ 					pParaPortion->GetTextPortions().Insert( pNew, n );
/*?*/ 				}
/*?*/ 
/*?*/ 				// Die Zeilen
/*?*/ 				pParaPortion->GetLines().Reset();
/*?*/ 				nCount = pXP->aLines.Count();
/*?*/ 				for ( sal_uInt16 m = 0; m < nCount; m++ )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EditLine* pLine = pXP->aLines[m];
/*?*/ 				}
/*?*/ #ifdef DBG_UTIL
/*?*/ 				USHORT nTest, nTPLen = 0, nTxtLen = 0;
/*?*/ 				for ( nTest = pParaPortion->GetTextPortions().Count(); nTest; )
/*?*/ 					nTPLen += pParaPortion->GetTextPortions().GetObject( --nTest )->GetLen();
/*?*/ 				for ( nTest = pParaPortion->GetLines().Count(); nTest; )
/*?*/ 					nTxtLen += pParaPortion->GetLines().GetObject( --nTest )->GetLen();
/*?*/ 				DBG_ASSERT( ( nTPLen == pParaPortion->GetNode()->Len() ) && ( nTxtLen == pParaPortion->GetNode()->Len() ), "InsertBinTextObject: ParaPortion not completely formatted!" );
/*?*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bParaAttribs ) // DefFont wird bei FastInsertParagraph nicht berechnet
/*N*/ 		{
/*N*/ 			aPaM.GetNode()->GetCharAttribs().GetDefFont() = aEditDoc.GetDefFont();
/*N*/ 			if ( aStatus.UseCharAttribs() )
/*N*/ 				aPaM.GetNode()->CreateDefFont();
/*N*/ 		}
/*N*/ 
/*N*/ 		// Zeilenumbruch, wenn weitere folgen...
/*N*/ 		if ( n < ( nContents-1) )
/*N*/ 		{
/*N*/ 			if ( bNewContent )
/*N*/ 				aPaM = ImpFastInsertParagraph( nPara+1 );
/*N*/ 			else
/*?*/ 				aPaM = ImpInsertParaBreak( aPaM, sal_False );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	aSel.Max() = aPaM;
/*N*/ 	return aSel;
/*N*/ }

/*N*/ LanguageType ImpEditEngine::GetLanguage( const EditPaM& rPaM, USHORT* pEndPos ) const
/*N*/ {
/*N*/ 	short nScriptType = GetScriptType( rPaM, pEndPos );	// pEndPos will be valid now, pointing to ScriptChange or NodeLen
/*N*/ 	USHORT nLangId = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
/*N*/ 	const SvxLanguageItem* pLangItem = &(const SvxLanguageItem&)rPaM.GetNode()->GetContentAttribs().GetItem( nLangId );
/*N*/ 	EditCharAttrib*	pAttr = rPaM.GetNode()->GetCharAttribs().FindAttrib( nLangId, rPaM.GetIndex() );
/*N*/ 	if ( pAttr )
/*?*/ 		pLangItem = (const SvxLanguageItem*)pAttr->GetItem();
/*N*/ 
/*N*/ 	if ( pEndPos && pAttr && ( pAttr->GetEnd() < *pEndPos ) )
/*?*/ 		*pEndPos = pAttr->GetEnd();
/*N*/ 
/*N*/ 	return pLangItem->GetLanguage();
/*N*/ }

/*N*/ ::com::sun::star::lang::Locale ImpEditEngine::GetLocale( const EditPaM& rPaM ) const
/*N*/ {
/*N*/ 	return SvxCreateLocale( GetLanguage( rPaM ) );
/*N*/ }














/*N*/ void ImpEditEngine::SetAsianCompressionMode( USHORT n )
/*N*/ {
/*N*/     if ( n != nAsianCompressionMode )
/*N*/     {
/*N*/         nAsianCompressionMode = n;
/*N*/         if ( ImplHasText() )
/*N*/         {
/*N*/             FormatFullDoc();
/*N*/             UpdateViews();
/*N*/         }
/*N*/     }
/*N*/ }

/*N*/ void ImpEditEngine::SetKernAsianPunctuation( BOOL b )
/*N*/ {
/*N*/     if ( b != bKernAsianPunctuation )
/*N*/     {
/*NBFF*/		bKernAsianPunctuation = b;
/*NBFF*/		if ( ImplHasText() )
/*NBFF*/		{
/*NBFF*/			FormatFullDoc();
/*NBFF*/			UpdateViews();
/*NBFF*/		}
/*N*/     }
/*N*/ }


/*N*/ BOOL ImpEditEngine::ImplHasText() const
/*N*/ {
/*N*/     return ( ( GetEditDoc().Count() > 1 ) || GetEditDoc().GetObject(0)->Len() );
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
