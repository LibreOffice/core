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



#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include <impedit.hxx>
#include <editeng.hxx>



namespace binfilter {

/*N*/ void ImpEditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
/*N*/ {
/*N*/ 	if ( pStylePool != pSPool )
/*N*/ 	{
/*N*/ //		if ( pStylePool )
/*N*/ //			EndListening( *pStylePool, TRUE );
/*N*/ 
/*N*/ 		pStylePool = pSPool;
/*N*/ 
/*N*/ //		if ( pStylePool )
/*N*/ //			StartListening( *pStylePool, TRUE );
/*N*/ 	}
/*N*/ }

/*N*/ SfxStyleSheet* ImpEditEngine::GetStyleSheet( USHORT nPara ) const
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/ 	return pNode ? pNode->GetContentAttribs().GetStyleSheet() : NULL;
/*N*/ }

/*N*/ void ImpEditEngine::SetStyleSheet( USHORT nPara, SfxStyleSheet* pStyle )
/*N*/ {
/*N*/ 	DBG_ASSERT( GetStyleSheetPool() || !pStyle, "SetStyleSheet: No StyleSheetPool registered!" );
/*N*/ 	ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/ 	SfxStyleSheet* pCurStyle = pNode->GetStyleSheet();
/*N*/ 	if ( pStyle != pCurStyle )
/*N*/ 	{
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
/*N*/ 		{
/*N*/ 			XubString aPrevStyleName;
/*N*/ 			if ( pCurStyle )
/*N*/ 				aPrevStyleName = pCurStyle->GetName();
/*N*/ 
/*N*/             XubString aNewStyleName;
/*N*/ 			if ( pStyle )
/*N*/ 				aNewStyleName = pStyle->GetName();
/*N*/ 
/*N*/ 			InsertUndo(
/*N*/ 				new EditUndoSetStyleSheet( this, aEditDoc.GetPos( pNode ),
/*N*/ 						aPrevStyleName, pCurStyle ? pCurStyle->GetFamily() : SFX_STYLE_FAMILY_PARA,
/*N*/                         aNewStyleName, pStyle ? pStyle->GetFamily() : SFX_STYLE_FAMILY_PARA,
/*N*/ 						pNode->GetContentAttribs().GetItems() ) );
/*N*/ 		}
/*N*/ #endif
/*N*/ 		if ( pCurStyle )
/*N*/ 			EndListening( *pCurStyle, FALSE );
/*N*/ 		pNode->SetStyleSheet( pStyle, aStatus.UseCharAttribs() );
/*N*/ 		if ( pStyle )
/*N*/ 			StartListening( *pStyle, FALSE );
/*N*/ 		ParaAttribsChanged( pNode );
/*N*/ 	}
/*N*/ 	FormatAndUpdate();
/*N*/ }






/*N*/ void ImpEditEngine::UndoActionStart( USHORT nId )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( IsUndoEnabled() && !IsInUndo() )
/*N*/ 	{
/*N*/ 		GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), XubString(), nId );
/*N*/ 		DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*N*/ void ImpEditEngine::UndoActionEnd( USHORT nId )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( IsUndoEnabled() && !IsInUndo() )
/*N*/ 	{
/*N*/ 		GetUndoManager().LeaveListAction();
/*N*/ 		delete pUndoMarkSelection;
/*N*/ 		pUndoMarkSelection = NULL;
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*N*/ void ImpEditEngine::InsertUndo( EditUndo* pUndo, BOOL bTryMerge )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	DBG_ASSERT( !IsInUndo(), "InsertUndo im Undomodus!" );
/*N*/ 	if ( pUndoMarkSelection )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EditUndoMarkSelection* pU = new EditUndoMarkSelection( this, *pUndoMarkSelection );
/*N*/ 	}
/*N*/ 	GetUndoManager().AddUndoAction( pUndo, bTryMerge );
/*N*/ #endif
/*N*/ }

/*N*/ void ImpEditEngine::ResetUndoManager()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	if ( HasUndoManager() )
/*N*/ 		GetUndoManager().Clear();
/*N*/ #endif
/*N*/ }

/*N*/ void ImpEditEngine::EnableUndo( BOOL bEnable )
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	// Beim Umschalten des Modus Liste loeschen:
/*N*/ 	if ( bEnable != IsUndoEnabled() )
/*N*/ 		ResetUndoManager();
/*N*/ 
/*N*/ 	bUndoEnabled = bEnable;
/*N*/ #endif
/*N*/ }




/*N*/ SfxItemSet ImpEditEngine::GetAttribs( EditSelection aSel, BOOL bOnlyHardAttrib )
/*N*/ {
/*N*/ 	DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );
/*N*/ 
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ //    if ( ( aSel.Min().GetNode() == aSel.Max().GetNode() ) && ( bOnlyHardAttrib == EditEngineAttribs_All ) )
/*N*/ //        return GetAttribs( aEditDoc.GetPos( aSel.Min().GetNode() ), aSel.Min().GetIndex(), aSel.Max().GetIndex(), GETATTRIBS_ALL );
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/ 	SfxItemSet aCurSet( GetEmptyItemSet() );
/*N*/ 
/*N*/ 	USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
/*N*/ 	USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
/*N*/ 
/*N*/ 	// ueber die Absaetze iterieren...
/*N*/ 	for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++	)
/*N*/ 	{
/*N*/ 		ContentNode* pNode = aEditDoc.GetObject( nNode );
/*N*/ 		DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: GetAttrib" );
/*N*/ 
/*N*/ 		xub_StrLen nStartPos = 0;
/*N*/ 		xub_StrLen nEndPos = pNode->Len();
/*N*/ 		if ( nNode == nStartNode )
/*N*/ 			nStartPos = aSel.Min().GetIndex();
/*N*/ 		if ( nNode == nEndNode ) // kann auch == nStart sein!
/*N*/ 			nEndPos = aSel.Max().GetIndex();
/*N*/ 
/*N*/ 		// Problem: Vorlagen....
/*N*/ 		// => Andersrum:
/*N*/ 		// 1) Harte Zeichenattribute, wie gehabt...
/*N*/ 		// 2) Nur wenn OFF, Style and Absatzattr. pruefen...
/*N*/ 
/*N*/ 		// Erst die ganz harte Formatierung...
/*N*/ 		aEditDoc.FindAttribs( pNode, nStartPos, nEndPos, aCurSet );
/*N*/ 
/*N*/ 		if( bOnlyHardAttrib != EditEngineAttribs_OnlyHard )
/*N*/ 		{
/*N*/ 			// Und dann Absatzformatierung und Vorlage...
/*N*/ 			// SfxStyleSheet* pStyle = pNode->GetStyleSheet();
/*N*/ 			for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
/*N*/ 			{
/*N*/ 				if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
/*N*/ 				{
/*N*/ 					if ( bOnlyHardAttrib == EditEngineAttribs_All )
/*N*/ 					{
/*N*/ 						const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
/*N*/ 						aCurSet.Put( rItem );
/*N*/ 					}
/*N*/ 					else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 					{
/*?*/ 						const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItems().Get( nWhich );
/*?*/ 						aCurSet.Put( rItem );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 				{
/*N*/ 					const SfxPoolItem* pItem = NULL;
/*N*/ 					if ( bOnlyHardAttrib == EditEngineAttribs_All )
/*N*/ 					{
/*N*/ 						pItem = &pNode->GetContentAttribs().GetItem( nWhich );
/*N*/ 					}
/*N*/ 					else if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 					{
/*?*/ 						pItem = &pNode->GetContentAttribs().GetItems().Get( nWhich );
/*N*/ 					}
/*N*/ 					// pItem can only be NULL when bOnlyHardAttrib...
/*N*/ 					if ( !pItem || ( *pItem != aCurSet.Get( nWhich ) ) )
/*N*/ 					{
/*N*/ 						// Problem: Wenn Absatzvorlage mit z.B. Font,
/*N*/ 						// aber Font hart und anders und komplett in Selektion
/*N*/ 						// Falsch, wenn invalidiert....
/*N*/ 						// => Lieber nicht invalidieren, UMSTELLEN!
/*N*/ 						// Besser waere, Absatzweise ein ItemSet zu fuellen
/*N*/ 						// und dieses mit dem gesmten vergleichen.
/*N*/ 	//						aCurSet.InvalidateItem( nWhich );
/*N*/ 						if ( nWhich <= EE_PARA_END )
/*?*/ 							aCurSet.InvalidateItem( nWhich );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Leere Slots mit Defaults fuellen...
/*N*/     if ( bOnlyHardAttrib == EditEngineAttribs_All )
/*N*/     {
/*N*/ 	    for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++ )
/*N*/ 	    {
/*N*/ 		    if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
/*N*/ 		    {
/*?*/ 			    aCurSet.Put( aEditDoc.GetItemPool().GetDefaultItem( nWhich ) );
/*N*/ 		    }
/*N*/ 	    }
/*N*/     }
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ /*
/*N*/ #ifdef MAC
/*N*/ 		FILE* fp = fopen( "debug.log", "a" );
/*N*/ #elif defined UNX
/*N*/ 		FILE* fp = fopen( "/tmp/debug.log", "a" );
/*N*/ #else
/*N*/ 		FILE* fp = fopen( "d:\\debug.log", "a" );
/*N*/ #endif
/*N*/ 		if ( fp )
/*N*/ 		{
/*N*/ 			fprintf( fp, "\n\n<= Attribute: Absatz %i\n", nNode );
/*N*/ 			DbgOutItemSet( fp, aCurSet, TRUE, FALSE );
/*N*/ 			fclose( fp );
/*N*/ 		}
/*N*/ */
/*N*/ #endif
/*N*/ 
/*N*/ 	return aCurSet;
/*N*/ }


/*N*/ SfxItemSet ImpEditEngine::GetAttribs( USHORT nPara, USHORT nStart, USHORT nEnd, sal_uInt8 nFlags ) const
/*N*/ {
/*N*/     // MT: #94002# Optimized function with less Puts(), which cause unnecessary cloning from default items.
/*N*/     // If this works, change GetAttribs( EditSelection ) to use this for each paragraph and merge the results!
/*N*/ 
/*N*/     DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );
/*N*/ 
/*N*/     ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/     DBG_ASSERT( pNode, "GetAttribs - unknown paragraph!" );
/*N*/     DBG_ASSERT( nStart <= nEnd, "getAttribs: Start > End not supported!" );
/*N*/ 
/*N*/     SfxItemSet aAttribs( ((ImpEditEngine*)this)->GetEmptyItemSet() );
/*N*/ 
/*N*/     if ( pNode )
/*N*/     {
/*N*/         if ( nEnd > pNode->Len() )
/*N*/             nEnd = pNode->Len();
/*?*/ 
/*N*/         if ( nStart > nEnd )
/*?*/             nStart = nEnd;
/*N*/ 
/*N*/         // StyleSheet / Parattribs...
/*N*/ 
/*N*/         if ( pNode->GetStyleSheet() && ( nFlags & GETATTRIBS_STYLESHEET ) )
/*N*/             aAttribs.Set( pNode->GetStyleSheet()->GetItemSet(), TRUE );
/*N*/ 
/*N*/         if ( nFlags & GETATTRIBS_PARAATTRIBS )
/*N*/             aAttribs.Put( pNode->GetContentAttribs().GetItems() );
/*N*/ 
/*N*/         // CharAttribs...
/*N*/ 
/*N*/         if ( nFlags & GETATTRIBS_CHARATTRIBS )
/*N*/         {
/*N*/             // Make testing easier...
/*N*/             pNode->GetCharAttribs().OptimizeRanges( ((ImpEditEngine*)this)->GetEditDoc().GetItemPool() );
/*N*/ 
/*N*/             const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
/*N*/             for ( USHORT nAttr = 0; nAttr < rAttrs.Count(); nAttr++ )
/*N*/             {
/*N*/                 EditCharAttrib* pAttr = rAttrs.GetObject( nAttr );
/*N*/ 
/*N*/                 if ( nStart == nEnd )
/*N*/                 {
/*N*/                     USHORT nCursorPos = nStart;
/*N*/                     if ( ( pAttr->GetStart() <= nCursorPos ) && ( pAttr->GetEnd() >= nCursorPos ) )
/*N*/                     {
/*N*/                         // To be used the attribute has to start BEFORE the position, or it must be a
/*N*/                         // new empty attr AT the position, or we are on position 0.
/*N*/                         if ( ( pAttr->GetStart() < nCursorPos ) || pAttr->IsEmpty() || !nCursorPos )
/*N*/                         {
/*N*/                             // maybe this attrib ends here and a new attrib with 0 Len may follow and be valid here,
/*N*/                             // but that s no problem, the empty item will come later and win.
/*N*/                             aAttribs.Put( *pAttr->GetItem() );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     // Check every attribute covering the area, partial or full.
/*N*/                     if ( ( pAttr->GetStart() < nEnd ) && ( pAttr->GetEnd() > nStart ) )
/*N*/                     {
/*N*/                         if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
/*N*/                         {
/*N*/                             // full coverage
/*N*/                             aAttribs.Put( *pAttr->GetItem() );
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             // OptimizeRagnge() assures that not the same attr can follow for full coverage
/*N*/                             // only partial, check with current, when using para/styhe, otherwise invalid.
/*N*/                             if ( !( nFlags & (GETATTRIBS_PARAATTRIBS|GETATTRIBS_STYLESHEET) ) ||
/*N*/                                 ( *pAttr->GetItem() != aAttribs.Get( pAttr->Which() ) ) )
/*N*/                             {
/*N*/                                 aAttribs.InvalidateItem( pAttr->Which() );
/*N*/                             }
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/ 
/*N*/                 if ( pAttr->GetStart() > nEnd )
/*N*/                 {
/*N*/                     break;
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	return aAttribs;
/*N*/ }


/*N*/ void ImpEditEngine::SetAttribs( EditSelection aSel, const SfxItemSet& rSet, BYTE nSpecial )
/*N*/ {
/*N*/ 	aSel.Adjust( aEditDoc );
/*N*/ 
/*N*/ 	// Wenn keine Selektion => die Attribute aufs Wort anwenden.
/*N*/ 	// ( Der RTF-Perser sollte die Methode eigentlich nie ohne Range rufen )
/*N*/ 	if ( ( nSpecial == ATTRSPECIAL_WHOLEWORD ) && !aSel.HasRange() )
/*?*/ 		aSel = SelectWord( aSel, ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, FALSE );
/*N*/ 
/*N*/ 	USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
/*N*/ 	USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );
/*N*/ 
/*N*/ 	BOOL bCheckLanguage = FALSE;
/*N*/ 
/*N*/ 	// ueber die Absaetze iterieren...
/*N*/ 	for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++	)
/*N*/ 	{
/*N*/ 		BOOL bParaAttribFound = FALSE;
/*N*/ 		BOOL bCharAttribFound = FALSE;
/*N*/ 
/*N*/ 		ContentNode* pNode = aEditDoc.GetObject( nNode );
/*N*/ 		ParaPortion* pPortion = GetParaPortions().GetObject( nNode );
/*N*/ 
/*N*/ 		DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: SetAttribs" );
/*N*/ 		DBG_ASSERT( GetParaPortions().GetObject( nNode ), "Portion nicht gefunden: SetAttribs" );
/*N*/ 
/*N*/ 		xub_StrLen nStartPos = 0;
/*N*/ 		xub_StrLen nEndPos = pNode->Len();
/*N*/ 		if ( nNode == nStartNode )
/*N*/ 			nStartPos = aSel.Min().GetIndex();
/*N*/ 		if ( nNode == nEndNode ) // kann auch == nStart sein!
/*N*/ 			nEndPos = aSel.Max().GetIndex();
/*N*/ 
/*N*/ 		// ueber die Items iterieren...
/*N*/ #ifdef EDITDEBUG
/*N*/ //		FILE* fp = fopen( "d:\\debug.log", "a" );
/*N*/ //		if ( fp )
/*N*/ //		{
/*N*/ //			fprintf( fp, "\n\n=> Zeichen-Attribute: Absatz %i, %i-%i\n", nNode, nStartPos, nEndPos );
/*N*/ //			DbgOutItemSet( fp, rSet, TRUE, FALSE );
/*N*/ //			fclose( fp );
/*N*/ //		}
/*N*/ #endif
/*N*/ 
/*N*/ 		for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
/*N*/ 		{
/*N*/ 			if ( rSet.GetItemState( nWhich ) == SFX_ITEM_ON )
/*N*/ 			{
/*N*/ 				const SfxPoolItem& rItem = rSet.Get( nWhich );
/*N*/ 				if ( nWhich <= EE_PARA_END )
/*N*/ 				{
/*?*/ 					pNode->GetContentAttribs().GetItems().Put( rItem );
/*?*/ 					bParaAttribFound = TRUE;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aEditDoc.InsertAttrib( pNode, nStartPos, nEndPos, rItem );
/*N*/ 					bCharAttribFound = TRUE;
/*N*/ 					if ( nSpecial == ATTRSPECIAL_EDGE )
/*N*/ 					{
/*?*/ 						CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
/*?*/ 						USHORT nAttrs = rAttribs.Count();
/*?*/ 						for ( USHORT n = 0; n < nAttrs; n++ )
/*?*/ 						{
/*?*/ 							EditCharAttrib* pAttr = rAttribs.GetObject( n );
/*?*/ 							if ( pAttr->GetStart() > nEndPos )
/*?*/ 								break;
/*?*/ 
/*?*/ 							if ( ( pAttr->GetEnd() == nEndPos ) && ( pAttr->Which() == nWhich ) )
/*?*/ 							{
/*?*/ 								pAttr->SetEdge( TRUE );
/*?*/ 								break;
/*?*/ 							}
/*?*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( bParaAttribFound )
/*N*/ 		{
/*?*/ 			ParaAttribsChanged( pPortion->GetNode() );
/*N*/ 		}
/*N*/ 		else if ( bCharAttribFound )
/*N*/ 		{
/*N*/ 			bFormatted = FALSE;
/*N*/ 			if ( !pNode->Len() || ( nStartPos != nEndPos  ) )
/*N*/ 			{
/*N*/ 				pPortion->MarkSelectionInvalid( nStartPos, nEndPos-nStartPos );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ typedef EditCharAttrib* EditCharAttribPtr;

/*N*/ void ImpEditEngine::RemoveCharAttribs( USHORT nPara, USHORT nWhich, BOOL bRemoveFeatures )
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/ 	ParaPortion* pPortion = GetParaPortions().SaveGetObject( nPara );
/*N*/ 
/*N*/ 	DBG_ASSERT( pNode, "Node nicht gefunden: RemoveCharAttribs" );
/*N*/ 	DBG_ASSERT( pPortion, "Portion nicht gefunden: RemoveCharAttribs" );
/*N*/ 
/*N*/ 	if ( !pNode )
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT nAttr = 0;
/*N*/ 	EditCharAttribPtr pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	while ( pAttr )
/*N*/ 	{
/*N*/ 		if ( ( !pAttr->IsFeature() || bRemoveFeatures ) &&
/*N*/ 			 ( !nWhich || ( pAttr->GetItem()->Which() == nWhich ) ) )
/*N*/ 		{
/*?*/ 			pNode->GetCharAttribs().GetAttribs().Remove( nAttr );
/*?*/ 			delete pAttr;
/*?*/ 			nAttr--;
/*N*/ 		}
/*N*/ 		nAttr++;
/*N*/ 		pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
/*N*/ 	}
/*N*/ 
/*N*/ 	pPortion->MarkSelectionInvalid( 0, pNode->Len() );
/*N*/ }

/*N*/ void ImpEditEngine::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
/*N*/ 
/*N*/ 	if ( !pNode )
/*N*/ 		return;
/*N*/ 
/*N*/ #ifdef EDITDEBUG
/*N*/ //		FILE* fp = fopen( "d:\\debug.log", "a" );
/*N*/ //		if ( fp )
/*N*/ //		{
/*N*/ //			fprintf( fp, "\n\n=> Absatz-Attribute: Absatz %i\n", nPara );
/*N*/ //			DbgOutItemSet( fp, rSet, TRUE, FALSE );
/*N*/ //			fclose( fp );
/*N*/ //		}
/*N*/ #endif
/*N*/ 
/*N*/ 	if ( !( pNode->GetContentAttribs().GetItems() == rSet ) )
/*N*/ 	{
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
/*N*/ 		{
/*N*/ 			if ( rSet.GetPool() != &aEditDoc.GetItemPool() )
/*N*/ 			{
/*?*/ 				SfxItemSet aTmpSet( GetEmptyItemSet() );
/*?*/ 				aTmpSet.Put( rSet );
/*?*/ 				InsertUndo( new EditUndoSetParaAttribs( this, nPara, pNode->GetContentAttribs().GetItems(), aTmpSet ) );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				InsertUndo( new EditUndoSetParaAttribs( this, nPara, pNode->GetContentAttribs().GetItems(), rSet ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/ 		pNode->GetContentAttribs().GetItems().Set( rSet );
/*N*/ 		if ( aStatus.UseCharAttribs() )
/*N*/ 			pNode->CreateDefFont();
/*N*/ 
/*N*/ 		ParaAttribsChanged( pNode );
/*N*/ 	}
/*N*/ }

/*N*/ const SfxItemSet& ImpEditEngine::GetParaAttribs( USHORT nPara ) const
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.GetObject( nPara );
/*N*/ 	DBG_ASSERT( pNode, "Node nicht gefunden: GetParaAttribs" );
/*N*/ 	return pNode->GetContentAttribs().GetItems();
/*N*/ }

/*N*/ BOOL ImpEditEngine::HasParaAttrib( USHORT nPara, USHORT nWhich ) const
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.GetObject( nPara );
/*N*/ 	DBG_ASSERT( pNode, "Node nicht gefunden: HasParaAttrib" );
/*N*/ 
/*N*/ 	return pNode->GetContentAttribs().HasItem( nWhich );
/*N*/ }

/*N*/ const SfxPoolItem& ImpEditEngine::GetParaAttrib( USHORT nPara, USHORT nWhich ) const
/*N*/ {
/*N*/ 	ContentNode* pNode = aEditDoc.GetObject( nPara );
/*N*/ 	DBG_ASSERT( pNode, "Node nicht gefunden: GetParaAttrib" );
/*N*/ 
/*N*/     return pNode->GetContentAttribs().GetItem( nWhich );
/*N*/ }

}
