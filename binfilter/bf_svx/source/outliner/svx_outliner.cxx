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

#include <outl_pch.hxx>

#include <bf_svtools/style.hxx>
#include <vcl/wrkwin.hxx>

#define _OUTLINER_CXX
#include <paralist.hxx>
#include <outlobj.hxx>
#include <outleeng.hxx>
#include <editstat.hxx>
#include <scripttypeitem.hxx>

#include <vcl/metric.hxx>

#include <numitem.hxx>

#include <bf_goodies/graphicobject.hxx>

#include <brshitem.hxx>

// #101498# calculate if it's RTL or not
#include <unicode/ubidi.h>

#include "lrspitem.hxx"

#include <bf_svtools/undo.hxx>

namespace binfilter {

/*N*/ #define DEFAULT_SCALE	75

/*N*/ static const USHORT nDefStyles = 3;	// Sonderbehandlung fuer die ersten 3 Ebenen
/*N*/ static const USHORT nDefBulletIndent = 800;
/*N*/ static const USHORT nDefBulletWidth = 700;
/*N*/ static const USHORT	pDefBulletIndents[nDefStyles]= 	{ 1400, 800, 800 };
/*N*/ static const USHORT	pDefBulletWidths[nDefStyles] = 	{ 1000, 850, 700 };

/*N*/ USHORT lcl_ImplGetDefBulletWidth( USHORT nDepth )
/*N*/ {
/*N*/ 	return ( nDepth < nDefStyles ) ? pDefBulletWidths[nDepth] : nDefBulletWidth;
/*N*/ }

/*N*/ USHORT lcl_ImplGetDefBulletIndent( USHORT nDepth )
/*N*/ {
/*N*/ 	USHORT nI = 0;
/*N*/ 	for ( USHORT n = 0; n <= nDepth; n++ )
/*N*/ 		nI += ( n < nDefStyles ) ? pDefBulletIndents[n] : nDefBulletIndent;
/*N*/ 	return nI;
/*N*/ }

/*N*/ SvxLRSpaceItem lcl_ImplGetDefLRSpaceItem( USHORT nDepth, MapUnit eMapUnit )
/*N*/ {
/*N*/ 	SvxLRSpaceItem aLRSpace( EE_PARA_OUTLLRSPACE );
/*N*/ 
/*N*/ 	long nWidth = OutputDevice::LogicToLogic( (long) lcl_ImplGetDefBulletWidth( nDepth ), MAP_100TH_MM, eMapUnit );
/*N*/ 	long nIndent = OutputDevice::LogicToLogic( (long) lcl_ImplGetDefBulletIndent( nDepth ), MAP_100TH_MM, eMapUnit );
/*N*/ 	aLRSpace.SetTxtLeft( (USHORT) nIndent );
/*N*/ 	aLRSpace.SetTxtFirstLineOfst( - (short)nWidth );
/*N*/ 
/*N*/ 	return aLRSpace;
/*N*/ }

// ----------------------------------------------------------------------
// Outliner
// ----------------------------------------------------------------------
/*N*/ DBG_NAME(Outliner)

/*N*/ void Outliner::ImplCheckDepth( USHORT& rnDepth ) const
/*N*/ {
/*N*/ 	if( rnDepth < nMinDepth )
/*N*/ 		rnDepth = nMinDepth;
/*N*/ 	else if( rnDepth > nMaxDepth )
/*N*/ 		rnDepth = nMaxDepth;
/*N*/ }

/*N*/ Paragraph* Outliner::Insert(const XubString& rText, ULONG nAbsPos, USHORT nDepth)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return NULL;//STRIP001 
/*N*/ }


/*N*/ void Outliner::ParagraphInserted( USHORT nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	if ( bBlockInsCallback )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if( bPasting || pEditEngine->IsInUndo() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Paragraph* pPara = new Paragraph( 0xffff );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		USHORT nDepth = 0;
/*N*/ 		if ( nPara )
/*N*/ 			nDepth = pParaList->GetParagraph( nPara-1 )->GetDepth();
/*N*/ 
/*N*/ 		Paragraph* pPara = new Paragraph( nDepth );
/*N*/ 		pParaList->Insert( pPara, nPara );
/*N*/ 
/*N*/ 		if( !pEditEngine->IsInUndo() )
/*N*/ 		{
/*N*/ 			ImplCalcBulletText( nPara, TRUE, FALSE );
/*N*/ 			pHdlParagraph = pPara;
/*N*/ 			ParagraphInsertedHdl();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::ParagraphDeleted( USHORT nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	if ( bBlockInsCallback || ( nPara == EE_PARA_ALL ) )
/*N*/ 		return;
/*N*/ 
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	USHORT nDepth = pPara->GetDepth();
/*N*/ 
/*N*/ 	if( !pEditEngine->IsInUndo() )
/*N*/ 	{
/*N*/ 		pHdlParagraph = pPara;
/*N*/ 		ParagraphRemovingHdl();
/*N*/ 	}
/*N*/ 
/*N*/ 	pParaList->Remove( nPara );
/*N*/ 	delete pPara;
/*N*/ 
/*N*/ 	if( !pEditEngine->IsInUndo() && !bPasting )
/*N*/ 	{
/*N*/ 		pPara = pParaList->GetParagraph( nPara );
/*N*/ 		if ( pPara && ( pPara->GetDepth() > nDepth ) )
/*N*/ 		{
/*?*/ 			ImplCalcBulletText( nPara, TRUE, FALSE );
/*?*/ 			// naechsten auf gleicher Ebene suchen...
/*?*/ 			while ( pPara && pPara->GetDepth() > nDepth )
/*?*/ 				pPara = pParaList->GetParagraph( ++nPara );
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pPara && ( pPara->GetDepth() == nDepth ) )
/*?*/ 			ImplCalcBulletText( nPara, TRUE, FALSE );
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::Init( USHORT nMode )
/*N*/ {
/*N*/ 	nOutlinerMode = nMode;
/*N*/ 
/*N*/ 	Clear();
/*N*/ 
/*N*/ 	ULONG nCtrl = pEditEngine->GetControlWord();
/*N*/ 	nCtrl &= ~(EE_CNTRL_OUTLINER|EE_CNTRL_OUTLINER2);
/*N*/ 
/*N*/ 	switch ( ImplGetOutlinerMode() )
/*N*/ 	{
/*N*/ 		case OUTLINERMODE_TEXTOBJECT:
/*N*/ 		{
/*N*/ 			SetMinDepth( 0 );
/*N*/ 			SetMaxDepth( 9 );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case OUTLINERMODE_TITLEOBJECT:
/*N*/ 		{
/*N*/ 			SetMinDepth( 0 );
/*N*/ 			SetMaxDepth( 9 );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case OUTLINERMODE_OUTLINEOBJECT:
/*N*/ 		{
/*N*/ 			SetMinDepth( 1 );
/*N*/ 			SetMaxDepth( 9 );
/*N*/ 			nCtrl |= EE_CNTRL_OUTLINER2;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case OUTLINERMODE_OUTLINEVIEW:
/*N*/ 		{
/*?*/ 			SetMinDepth( 0 );
/*?*/ 			SetMaxDepth( 9 );
/*?*/ 			nCtrl |= EE_CNTRL_OUTLINER;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		default: DBG_ERROR( "Outliner::Init - Invalid Mode!" );
/*N*/ 	}
/*N*/ 
/*N*/ 	pEditEngine->SetControlWord( nCtrl );
/*N*/ 
/*N*/ 	ImplInitDepth( 0, GetMinDepth(), FALSE );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	GetUndoManager().Clear();
/*N*/ #endif
/*N*/ }

/*N*/ void Outliner::SetMinDepth( USHORT nDepth, BOOL bCheckParagraphs )
/*N*/ {
/*N*/ 	if( nMinDepth != nDepth )
/*N*/ 	{
/*N*/ 		nMinDepth = nDepth;
/*N*/ 
/*N*/ 		if( bCheckParagraphs && nMinDepth )
/*N*/ 		{
/*?*/ 			// Prueft nur dass kein Absatz kleiner ist,
/*?*/ 			// es werden keine Ebenen proportional verschoben!
/*?*/ 			USHORT nParagraphs = (USHORT)pParaList->GetParagraphCount();
/*?*/ 			for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
/*?*/ 			{
/*?*/ 				Paragraph* pPara = pParaList->GetParagraph( nPara );
/*?*/ 				if( pPara->GetDepth() < nMinDepth )
/*?*/ 				{
/*?*/ 					SetDepth( pPara, nMinDepth );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::SetMaxDepth( USHORT nDepth, BOOL bCheckParagraphs )
/*N*/ {
/*N*/ 	if( nMaxDepth != nDepth )
/*N*/ 	{
/*?*/ 		nMaxDepth = Min( nDepth, (USHORT)(SVX_MAX_NUM-1) );
/*?*/ 
/*?*/ 		if( bCheckParagraphs )
/*?*/ 		{
/*?*/ 			USHORT nParagraphs = (USHORT)pParaList->GetParagraphCount();
/*?*/ 			for ( USHORT nPara = 0; nPara < nParagraphs; nPara++ )
/*?*/ 			{
/*?*/ 				Paragraph* pPara = pParaList->GetParagraph( nPara );
/*?*/ 				if( pPara->GetDepth() > nMaxDepth )
/*?*/ 				{
/*?*/ 					SetDepth( pPara, nMaxDepth );
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ USHORT Outliner::GetDepth( USHORT nPara ) const
/*N*/ {
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	DBG_ASSERT( pPara, "Outliner::GetDepth - Paragraph not found!" );
/*N*/ 	return pPara ? pPara->GetDepth() : 0;
/*N*/ }

/*N*/ void Outliner::SetDepth( Paragraph* pPara, USHORT nNewDepth )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/     ImplCheckDepth( nNewDepth );
/*N*/ 
/*N*/ 	if ( nNewDepth != pPara->GetDepth() )
/*N*/ 	{
/*N*/ 		nDepthChangedHdlPrevDepth = pPara->GetDepth();
/*N*/ 		pHdlParagraph = pPara;
/*N*/ 
/*N*/ 		ImplInitDepth( (USHORT) GetAbsPos( pPara ), nNewDepth, TRUE );
/*N*/ 
/*N*/ 		DepthChangedHdl();
/*N*/ 	}
/*N*/ }

/*N*/ OutlinerParaObject* Outliner::CreateParaObject( USHORT nStartPara, USHORT nCount ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	if ( ( nStartPara + nCount ) > pParaList->GetParagraphCount() )
/*N*/ 		nCount = pParaList->GetParagraphCount() - nStartPara;
/*N*/ 
/*N*/ 	if( !nCount )
/*N*/ 		return NULL;
/*N*/ 
/*N*/ 	OutlinerParaObject* pPObj = new OutlinerParaObject( nCount );
/*N*/ 	pPObj->pText = pEditEngine->CreateTextObject( nStartPara, nCount );
/*N*/ 	pPObj->SetOutlinerMode( GetMode() );
/*N*/ 	pPObj->bIsEditDoc = ( ImplGetOutlinerMode() == OUTLINERMODE_TEXTOBJECT ) ? TRUE : FALSE;
/*N*/ 
/*N*/ 	USHORT nLastPara = nStartPara + nCount - 1;
/*N*/ 	for ( USHORT nPara = nStartPara; nPara <= nLastPara; nPara++ )
/*N*/ 		pPObj->pDepthArr[ nPara-nStartPara] = GetDepth( nPara );
/*N*/ 
/*N*/ 	return pPObj;
/*N*/ }

/*N*/ void Outliner::SetText( const XubString& rText, Paragraph* pPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	DBG_ASSERT(pPara,"SetText:No Para");
/*N*/ 
/*N*/ 	BOOL bUpdate = pEditEngine->GetUpdateMode();
/*N*/ 	pEditEngine->SetUpdateMode( FALSE );
/*N*/ 	ImplBlockInsertionCallbacks( TRUE );
/*N*/ 
/*N*/ 	USHORT nPara = (USHORT)pParaList->GetAbsPos( pPara );
/*N*/ 
/*N*/ 	if( !rText.Len() )
/*N*/ 	{
/*N*/ 		pEditEngine->SetText( nPara, rText );
/*N*/ 		ImplInitDepth( nPara, pPara->GetDepth(), FALSE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		XubString aText( rText );
/*N*/ 		aText.ConvertLineEnd( LINEEND_LF );
/*N*/ 
/*N*/ 		if( aText.GetChar( aText.Len()-1 ) == '\x0A' )
/*?*/ 			aText.Erase( aText.Len()-1, 1 ); // letzten Umbruch loeschen
/*N*/ 
/*N*/ 		USHORT nCount = aText.GetTokenCount( '\x0A' );
/*N*/ 		USHORT nPos = 0;
/*N*/ 		USHORT nInsPos = nPara+1;
/*N*/ 		while( nCount > nPos )
/*N*/ 		{
/*N*/ 			XubString aStr = aText.GetToken( nPos, '\x0A' );
/*N*/ 
/*N*/ 			USHORT nCurDepth;
/*N*/ 			if( nPos )
/*N*/ 			{
/*N*/ 				pPara = new Paragraph( 0 );
/*N*/ 				nCurDepth = 0;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nCurDepth = pPara->GetDepth();
/*N*/ 
/*N*/ 			// Im Outliner-Modus die Tabulatoren filtern und die
/*N*/ 			// Einrueckung ueber ein LRSpaceItem einstellen
/*N*/ 			// Im EditEngine-Modus ueber Maltes Tabulatoren einruecken
/*N*/ 			if( ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT ) ||
/*N*/ 				( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW ) )
/*N*/ 			{
/*N*/ 				// Tabs raus
/*N*/ 				USHORT nTabs = 0;
/*N*/ 				while ( ( nTabs < aStr.Len() ) && ( aStr.GetChar( nTabs ) == '\t' ) )
/*N*/ 					nTabs++;
/*N*/ 				if ( nTabs )
/*N*/ 					aStr.Erase( 0, nTabs );
/*N*/ 
/*N*/ 				// Tiefe beibehalten ?  (siehe Outliner::Insert)
/*N*/ 				if( !(pPara->nFlags & PARAFLAG_HOLDDEPTH) )
/*N*/ 				{
/*N*/ 					nCurDepth = nTabs;
/*N*/ 					ImplCheckDepth( nCurDepth );
/*N*/ 					pPara->SetDepth( nCurDepth );
/*N*/ 					pPara->nFlags &= (~PARAFLAG_HOLDDEPTH);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( nPos ) // nicht mit dem ersten Absatz
/*N*/ 			{
/*N*/ 				pParaList->Insert( pPara, nInsPos );
/*N*/ 				pEditEngine->InsertParagraph( nInsPos, aStr );
/*N*/ 				pHdlParagraph = pPara;
/*N*/ 				ParagraphInsertedHdl();
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				nInsPos--;
/*N*/ 				pEditEngine->SetText( nInsPos, aStr );
/*N*/ 			}
/*N*/ 			ImplInitDepth( nInsPos, nCurDepth, FALSE );
/*N*/ 			nInsPos++;
/*N*/ 			nPos++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	DBG_ASSERT(pParaList->GetParagraphCount()==pEditEngine->GetParagraphCount(),"SetText failed!");
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 	ImplBlockInsertionCallbacks( FALSE );
/*N*/ 	pEditEngine->SetUpdateMode( bUpdate );
/*N*/ }

// pView == 0 -> Tabulatoren nicht beachten


/*N*/ void Outliner::SetText( const OutlinerParaObject& rPObj )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	BOOL bUpdate = pEditEngine->GetUpdateMode();
/*N*/ 	pEditEngine->SetUpdateMode( FALSE );
/*N*/ 
/*N*/ 	BOOL bUndo = pEditEngine->IsUndoEnabled();
/*N*/ 	EnableUndo( FALSE );
/*N*/ 
/*N*/ 	Init( rPObj.GetOutlinerMode() );
/*N*/ 
/*N*/ 	ImplBlockInsertionCallbacks( TRUE );
/*N*/ 	pEditEngine->SetText( *(rPObj.pText) );
/*N*/ 	bFirstParaIsEmpty = FALSE;
/*N*/ 
/*N*/ 	pParaList->Clear( TRUE );
/*N*/ 	for( USHORT nCurPara = 0; nCurPara < rPObj.nCount; nCurPara++ )
/*N*/ 	{
/*N*/         USHORT nDepth = rPObj.pDepthArr[ nCurPara ];
/*N*/         ImplCheckDepth( nDepth );
/*N*/ 		Paragraph* pPara = new Paragraph( nDepth );
/*N*/ 		pParaList->Insert( pPara, LIST_APPEND );
/*N*/ 		ImplCheckNumBulletItem( nCurPara );
/*N*/ 	}
/*N*/ 
/*N*/ 	ImplCheckParagraphs( 0, (USHORT) (pParaList->GetParagraphCount()-1) );
/*N*/ 
/*N*/ 	EnableUndo( bUndo );
/*N*/ 	ImplBlockInsertionCallbacks( FALSE );
/*N*/ 	pEditEngine->SetUpdateMode( bUpdate );
/*N*/ 
/*N*/ 	DBG_ASSERT( pParaList->GetParagraphCount()==rPObj.Count(),"SetText failed");
/*N*/ 	DBG_ASSERT( pEditEngine->GetParagraphCount()==rPObj.Count(),"SetText failed");
/*N*/ }

/*N*/ XubString __EXPORT Outliner::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	if ( !aCalcFieldValueHdl.IsSet() )
/*N*/ 		return String( ' ' );
/*N*/ 
/*N*/ 	EditFieldInfo aFldInfo( this, rField, nPara, nPos );
/*N*/ 	// Die FldColor ist mit COL_LIGHTGRAY voreingestellt.
/*N*/ 	if ( rpFldColor )
/*N*/ 		aFldInfo.SetFldColor( *rpFldColor );
/*N*/ 
/*N*/ 	aCalcFieldValueHdl.Call( &aFldInfo );
/*N*/ 	if ( aFldInfo.GetTxtColor() )
/*N*/ 	{
/*N*/ 		delete rpTxtColor;
/*N*/ 		rpTxtColor = new Color( *aFldInfo.GetTxtColor() );
/*N*/ 	}
/*N*/ 
/*N*/ 	delete rpFldColor;
/*N*/ 	rpFldColor = aFldInfo.GetFldColor() ? new Color( *aFldInfo.GetFldColor() ) : 0;
/*N*/ 
/*N*/ 	return aFldInfo.GetRepresentation();
/*N*/ }

/*N*/ void Outliner::SetStyleSheet( ULONG nPara, SfxStyleSheet* pStyle )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetStyleSheet( (USHORT)nPara, pStyle );
/*N*/ 
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	pPara->nFlags |= PARAFLAG_SETBULLETTEXT;
/*N*/ 
/*N*/ 	ImplCheckNumBulletItem( (USHORT) nPara );
/*N*/ }


/*N*/ void Outliner::ImplCheckNumBulletItem( USHORT nPara )
/*N*/ {
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 
/*N*/ 	// Wenn es ein SvxNumBulletItem gibt, ueberschreibt dieses die
/*N*/ 	// Einstellungen von BulletItem und LRSpaceItem.
/*N*/     const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 	if ( pFmt )
/*N*/ 	{
/*N*/ 		SvxLRSpaceItem aNewLRSpace( EE_PARA_LRSPACE );
/*N*/ 		aNewLRSpace.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
/*N*/ 		aNewLRSpace.SetTxtLeft( pFmt->GetAbsLSpace() );
/*N*/ 		if ( pEditEngine->HasParaAttrib( nPara, EE_PARA_LRSPACE ) )
/*N*/ 		{
/*N*/ 			const SvxLRSpaceItem& rOldLRSpace = (const SvxLRSpaceItem&)pEditEngine->GetParaAttrib( nPara, EE_PARA_LRSPACE );
/*N*/ 			aNewLRSpace.SetRight( rOldLRSpace.GetRight() );
/*N*/ 		}
/*N*/ 		SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
/*N*/ 		aAttrs.Put( aNewLRSpace);
/*N*/ 		pPara->aBulSize.Width() = -1;
/*N*/ 		pEditEngine->SetParaAttribs( nPara, aAttrs );
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::ImplSetLevelDependendStyleSheet( USHORT nPara, SfxStyleSheet* pLevelStyle )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	DBG_ASSERT( ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEOBJECT ) || ( ImplGetOutlinerMode() == OUTLINERMODE_OUTLINEVIEW ), "SetLevelDependendStyleSheet: Wrong Mode!" );
/*N*/ 
/*N*/     SfxStyleSheet* pStyle = pLevelStyle;
/*N*/     if ( !pStyle )
/*N*/         pStyle = GetStyleSheet( nPara );
/*N*/ 
/*N*/ 	if ( pStyle )
/*N*/ 	{
/*N*/ 		String aNewStyleSheetName( pStyle->GetName() );
/*N*/ 		aNewStyleSheetName.Erase( aNewStyleSheetName.Len()-1, 1 );
/*N*/ 		aNewStyleSheetName += String::CreateFromInt32( GetDepth( nPara ) );
/*N*/ 		SfxStyleSheet* pNewStyle = (SfxStyleSheet*)GetStyleSheetPool()->Find( aNewStyleSheetName, pStyle->GetFamily() );
/*N*/ 		DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );
/*N*/ 		if ( pNewStyle && ( pNewStyle != GetStyleSheet( nPara ) ) )
/*N*/ 		{
/*N*/  			SfxItemSet aOldAttrs( GetParaAttribs( nPara ) );
/*N*/ 			SetStyleSheet( nPara, pNewStyle );
/*N*/ 			if ( aOldAttrs.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
/*N*/ 			{
/*N*/ 				SfxItemSet aAttrs( GetParaAttribs( nPara ) );
/*N*/ 				aAttrs.Put( aOldAttrs.Get( EE_PARA_NUMBULLET ) );
/*N*/ 				SetParaAttribs( nPara, aAttrs );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::ImplInitDepth( USHORT nPara, USHORT nDepth, BOOL bCreateUndo, BOOL bUndoAction )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/     DBG_ASSERT( ( nDepth >= nMinDepth ) && ( nDepth <= nMaxDepth ), "ImplInitDepth - Depth is invalid!" );
/*N*/ 
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	USHORT nOldDepth = pPara->GetDepth();
/*N*/ 	pPara->SetDepth( nDepth );
/*N*/ 
/*N*/ 	// Bei IsInUndo brauchen Attribute und Style nicht eingestellt werden,
/*N*/ 	// dort werden die alten Werte durch die EditEngine restauriert.
/*N*/ 
/*N*/ 	if( !IsInUndo() )
/*N*/ 	{
/*N*/ 		BOOL bUpdate = pEditEngine->GetUpdateMode();
/*N*/ 		pEditEngine->SetUpdateMode( FALSE );
/*N*/ 
/*N*/ 		BOOL bUndo = bCreateUndo && IsUndoEnabled();
/*N*/ 		if ( bUndo && bUndoAction )
/*?*/ 			UndoActionStart( OLUNDO_DEPTH );
/*N*/ 
/*N*/ 		SfxItemSet aAttrs( pEditEngine->GetParaAttribs( nPara ) );
/*N*/ 		aAttrs.Put( SfxUInt16Item( EE_PARA_OUTLLEVEL, nDepth ) );
/*N*/ 		// MT: OUTLLRSPACE immer default, sollte mal irgendwie eingestellt werden...
/*N*/ 		aAttrs.Put( lcl_ImplGetDefLRSpaceItem( nDepth, GetRefMapMode().GetMapUnit() ) );
/*N*/ 		pEditEngine->SetParaAttribs( nPara, aAttrs );
/*N*/ 		ImplCheckNumBulletItem( nPara );
/*N*/ 		ImplCalcBulletText( nPara, FALSE, FALSE );
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		if ( bUndo )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 InsertUndo( new OutlinerUndoChangeDepth( this, nPara, nOldDepth, nDepth ) );
/*N*/ 		}
/*N*/ #endif
/*N*/ 
/*N*/ 		pEditEngine->SetUpdateMode( bUpdate );
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::SetParaAttribs( ULONG nPara, const SfxItemSet& rSet, bool bApiCall /* = false */ )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	if( pPara )
/*N*/ 	{
/*N*/ 		if ( !pEditEngine->IsInUndo() && pEditEngine->IsUndoEnabled() )
/*N*/ 			UndoActionStart( OLUNDO_ATTR );
/*N*/ 
/*N*/ 		BOOL bLRSpaceChanged =
/*N*/ 			( rSet.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON  ) &&
/*N*/ 			( !(rSet.Get( EE_PARA_LRSPACE ) == pEditEngine->GetParaAttrib( (USHORT)nPara, EE_PARA_LRSPACE ) ) );
/*N*/ 
/*N*/ 		pEditEngine->SetParaAttribs( (USHORT)nPara, rSet );
/*N*/ 
/*N*/ 		if( bLRSpaceChanged )
/*N*/ 		{
/*N*/ 			const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&)pEditEngine->GetParaAttrib( (USHORT)nPara, EE_PARA_NUMBULLET );
/*N*/ 			Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 			const USHORT nDepth = pPara->GetDepth();
/*N*/ 			if ( rNumBullet.GetNumRule()->GetLevelCount() > nDepth )
/*N*/ 			{
/*N*/ 				SvxNumBulletItem* pNewNumBullet = (SvxNumBulletItem*) rNumBullet.Clone();
/*N*/ 				EditEngine::ImportBulletItem( *pNewNumBullet, nDepth, NULL, (SvxLRSpaceItem*)&rSet.Get( EE_PARA_LRSPACE ) );
/*N*/ 				SfxItemSet aAttribs( rSet );
/*N*/ 				aAttribs.Put( *pNewNumBullet );
/*N*/ 				pEditEngine->SetParaAttribs( (USHORT)nPara, aAttribs );
/*N*/ 				delete pNewNumBullet;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		ImplCheckNumBulletItem( (USHORT)nPara );
/*N*/ 		ImplCheckParagraphs( (USHORT)nPara, (USHORT)nPara );
/*N*/ 
/*N*/ 		if ( !pEditEngine->IsInUndo() && pEditEngine->IsUndoEnabled() )
/*N*/ 			UndoActionEnd( OLUNDO_ATTR );
/*N*/ 	}
/*N*/ }


/*N*/ Font Outliner::ImpCalcBulletFont( USHORT nPara ) const
/*N*/ {
/*N*/ 	const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 	DBG_ASSERT( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ), "ImpCalcBulletFont: Missing or BitmapBullet!" );
/*N*/ 
/*N*/     Font aStdFont;  //#107508#
/*N*/     if ( !pEditEngine->IsFlatMode() )
/*N*/     {
/*N*/         ESelection aSel( nPara, 0, nPara, 0 );
/*N*/         aStdFont = EditEngine::CreateFontFromItemSet( pEditEngine->GetAttribs( aSel ), GetScriptType( aSel ) );
/*N*/     }
/*N*/     else
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP");//STRIP001 aStdFont = pEditEngine->GetStandardFont( nPara );
/*N*/     }
/*N*/ 
/*N*/ 	Font aBulletFont;
/*N*/ 	if ( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
/*N*/     {
/*N*/ 		aBulletFont = *pFmt->GetBulletFont();
/*N*/     }
/*N*/ 	else
/*N*/     {
/*?*/ 		aBulletFont = aStdFont;
/*?*/         aBulletFont.SetUnderline( UNDERLINE_NONE );
/*?*/         aBulletFont.SetStrikeout( STRIKEOUT_NONE );
/*?*/         aBulletFont.SetEmphasisMark( EMPHASISMARK_NONE );
/*?*/         aBulletFont.SetRelief( RELIEF_NONE );
/*N*/     }
/*N*/ 
/*N*/     // #107508# Use original scale...
/*N*/ 	USHORT nScale = /* pEditEngine->IsFlatMode() ? DEFAULT_SCALE : */ pFmt->GetBulletRelSize();
/*N*/ 	ULONG nScaledLineHeight = aStdFont.GetSize().Height();
/*N*/ 	nScaledLineHeight *= nScale*10;
/*N*/ 	nScaledLineHeight /= 1000;
/*N*/ 
/*N*/     aBulletFont.SetAlign( ALIGN_BOTTOM );
/*N*/ 	aBulletFont.SetSize( Size( 0, nScaledLineHeight ) );
/*N*/ 	BOOL bVertical = IsVertical();
/*N*/     aBulletFont.SetVertical( bVertical );
/*N*/     aBulletFont.SetOrientation( bVertical ? 2700 : 0 );
/*N*/ 
/*N*/ 	Color aColor( COL_AUTO );
/*N*/ 	if( !pEditEngine->IsFlatMode() && !( pEditEngine->GetControlWord() & EE_CNTRL_NOCOLORS ) )
/*N*/     {
/*N*/ 		aColor = pFmt->GetBulletColor();
/*N*/     }
/*N*/ 
/*N*/     if ( ( aColor == COL_AUTO ) || ( IsForceAutoColor() ) )
/*N*/         aColor = pEditEngine->GetAutoColor();
/*N*/ 
/*N*/     aBulletFont.SetColor( aColor );
/*N*/ 	return aBulletFont;
/*N*/ }

/*N*/ void Outliner::PaintBullet( USHORT nPara, const Point& rStartPos,
/*N*/ 	const Point& rOrigin, short nOrientation, OutputDevice* pOutDev )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	if ( ImplHasBullet( nPara ) )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BOOL bVertical = IsVertical();
/*N*/ 	}
/*N*/ }

/*N*/ #ifndef SVX_LIGHT
/*N*/ SfxUndoManager&	Outliner::GetUndoManager()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetUndoManager();
/*N*/ }
/*N*/ #endif


/*N*/ Outliner::Outliner( SfxItemPool* pPool, USHORT nMode )
/*N*/ {
/*N*/ 	DBG_CTOR( Outliner, 0 );
/*N*/ 
/*N*/ 	bStrippingPortions 	= FALSE;
/*N*/ 	bPasting			= FALSE;
/*N*/ 
/*N*/ 	nFirstPage			= 1;
/*N*/ 	bBlockInsCallback	= FALSE;
/*N*/ 
/*N*/ 	nMinDepth			= 0;
/*N*/ 	nMaxDepth			= 9;
/*N*/ 
/*N*/     pOverwriteLevel0Bullet = NULL;
/*N*/ 
/*N*/ 	pParaList = new ParagraphList;
/*N*/ 	pParaList->SetVisibleStateChangedHdl( LINK( this, Outliner, ParaVisibleStateChangedHdl ) );
/*N*/ 	Paragraph* pPara = new Paragraph( 0 );
/*N*/ 	pParaList->Insert( pPara, LIST_APPEND );
/*N*/ 	bFirstParaIsEmpty = TRUE;
/*N*/ 
/*N*/ 	pEditEngine = new OutlinerEditEng( this, pPool );
/*N*/     pEditEngine->SetBeginMovingParagraphsHdl( LINK( this, Outliner, BeginMovingParagraphsHdl ) );
/*N*/     pEditEngine->SetEndMovingParagraphsHdl( LINK( this, Outliner, EndMovingParagraphsHdl ) );
/*N*/     pEditEngine->SetBeginPasteOrDropHdl( LINK( this, Outliner, BeginPasteOrDropHdl ) );
/*N*/     pEditEngine->SetEndPasteOrDropHdl( LINK( this, Outliner, EndPasteOrDropHdl ) );
/*N*/ 
/*N*/ 	Init( nMode );
/*N*/ }

/*N*/ Outliner::~Outliner()
/*N*/ {
/*N*/ 	DBG_DTOR(Outliner,0);
/*N*/ 
/*N*/ 	pParaList->Clear( TRUE );
/*N*/ 	delete pParaList;
/*N*/     delete pOverwriteLevel0Bullet;
/*N*/ 	delete pEditEngine;
/*N*/ }

/*N*/ void Outliner::ParagraphInsertedHdl()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	aParaInsertedHdl.Call( this );
/*N*/ }


/*N*/ void Outliner::ParagraphRemovingHdl()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	aParaRemovingHdl.Call( this );
/*N*/ }


/*N*/ void Outliner::DepthChangedHdl()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	aDepthChangedHdl.Call( this );
/*N*/ }


/*N*/ ULONG Outliner::GetAbsPos( Paragraph* pPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	DBG_ASSERT(pPara,"GetAbsPos:No Para");
/*N*/ 	return pParaList->GetAbsPos( pPara );
/*N*/ }

/*N*/ void Outliner::ParagraphHeightChanged( USHORT )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	// MT: Kann wohl weg...
/*N*/ }

/*N*/ ULONG Outliner::GetParagraphCount() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pParaList->GetParagraphCount();
/*N*/ }

/*N*/ Paragraph* Outliner::GetParagraph( ULONG nAbsPos ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pParaList->GetParagraph( nAbsPos );
/*N*/ }


/*N*/ BOOL Outliner::ImplHasBullet( USHORT nPara ) const
/*N*/ {
/*N*/ 	const SfxUInt16Item& rBulletState = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_BULLETSTATE );
/*N*/ 	BOOL bBullet = rBulletState.GetValue() ? TRUE : FALSE;
/*N*/ 	if ( !pParaList->GetParagraph( nPara )->GetDepth() && ( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) )
/*N*/ 	{
/*N*/ 		// Im Gliederungsmodus immer ein Bullet auf Ebene 0!
/*N*/ 		bBullet = TRUE;
/*N*/ 	}
/*N*/ 	else if ( bBullet )
/*N*/ 	{
/*N*/ 		const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 		if ( !pFmt || ( pFmt->GetNumberingType() == SVX_NUM_NUMBER_NONE ) )
/*N*/ 			bBullet = FALSE;
/*N*/ 	}
/*N*/ 	return bBullet;
/*N*/ }

/*N*/ const SvxNumberFormat* Outliner::ImplGetBullet( USHORT nPara ) const
/*N*/ {
/*N*/     const SvxNumberFormat* pFmt = NULL;
/*N*/ 
/*N*/     USHORT nDepth = pParaList->GetParagraph( nPara )->GetDepth();
/*N*/ 
/*N*/     if ( !nDepth && pOverwriteLevel0Bullet )
/*N*/     {
/*N*/         pFmt = pOverwriteLevel0Bullet;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( nPara, EE_PARA_NUMBULLET );
/*N*/         if ( rNumBullet.GetNumRule()->GetLevelCount() > nDepth )
/*N*/             pFmt = rNumBullet.GetNumRule()->Get( nDepth );
/*N*/     }
/*N*/ 
/*N*/     return pFmt;
/*N*/ }

/*N*/ Size Outliner::ImplGetBulletSize( USHORT nPara )
/*N*/ {
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 
/*N*/ 	if( pPara->aBulSize.Width() == -1 )
/*N*/ 	{
/*N*/ 		const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 		DBG_ASSERT( pFmt, "ImplGetBulletSize - no Bullet!" );
/*N*/ 
/*N*/ 		if ( pFmt->GetNumberingType() == SVX_NUM_NUMBER_NONE )
/*N*/ 		{
/*?*/ 			pPara->aBulSize = Size( 0, 0 );
/*N*/ 		}
/*N*/ 		else if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
/*N*/ 		{
/*N*/ 			String aBulletText = ImplGetBulletText( nPara );
/*N*/ 			OutputDevice* pRefDev = pEditEngine->GetRefDevice();
/*N*/ 			Font aBulletFont( ImpCalcBulletFont( nPara ) );
/*N*/ 			Font aRefFont( pRefDev->GetFont());
/*N*/ 			pRefDev->SetFont( aBulletFont );
/*N*/ 			pPara->aBulSize.Width() = pRefDev->GetTextWidth( aBulletText );
/*N*/ 			pPara->aBulSize.Height() = pRefDev->GetTextHeight();
/*N*/ 			pRefDev->SetFont( aRefFont );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			pPara->aBulSize = OutputDevice::LogicToLogic( pFmt->GetGraphicSize(), MAP_100TH_MM, pEditEngine->GetRefDevice()->GetMapMode() );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return pPara->aBulSize;
/*N*/ }

/*N*/ void Outliner::ImplCheckParagraphs( USHORT nStart, USHORT nEnd )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( Outliner, 0 );
/*N*/ 
/*N*/ 	// Sicherstellen dass es ein Bullet und ein LR-Space gibt.
/*N*/ 
/*N*/ 	for ( USHORT n = nStart; n <= nEnd; n++ )
/*N*/ 	{
/*N*/ 		Paragraph* pPara = pParaList->GetParagraph( n );
/*N*/ 		BOOL bHasLRSpace = pEditEngine->HasParaAttrib( n, EE_PARA_OUTLLRSPACE );
/*N*/ 		BOOL bHasLevel = pEditEngine->HasParaAttrib( n, EE_PARA_OUTLLEVEL );
/*N*/ 		if ( !bHasLRSpace || !bHasLevel )
/*N*/ 		{
/*N*/ 			SfxItemSet aAttrs( pEditEngine->GetParaAttribs( n ) );
/*N*/ 
/*N*/ 			// MT 05/00: Default-Item muss erstmal richtig sein => Dann koennen diese ganzen komischen Defaults weg!!!
/*N*/ //			const SvxNumBulletItem& rNumBullet = (const SvxNumBulletItem&) pEditEngine->GetParaAttrib( n, EE_PARA_NUMBULLET );
/*N*/ //			const SvxNumberFormat* pFmt = NULL;
/*N*/ //			if ( ( rNumBullet.GetNumRule()->GetLevelCount() > pPara->GetDepth() ) &&
/*N*/ //				 ( ( pFtm = rNumBullet.GetNumRule()->Get( pPara->GetDepth() ) != NULL ) )
/*N*/ //			{
/*N*/ //			}
/*N*/ 			if ( !bHasLRSpace )
/*N*/ 			{
/*N*/ 				SvxLRSpaceItem aLRSpaceItem = lcl_ImplGetDefLRSpaceItem( pPara->GetDepth(), GetRefMapMode().GetMapUnit() );
/*N*/ 				aAttrs.Put( aLRSpaceItem );
/*N*/ 			}
/*N*/ 			if ( !bHasLevel )
/*N*/ 			{
/*N*/ 				SfxUInt16Item aLevelItem( EE_PARA_OUTLLEVEL, pPara->GetDepth() );
/*N*/ 				aAttrs.Put( aLevelItem );
/*N*/ 			}
/*N*/ 			pEditEngine->SetParaAttribs( n, aAttrs );
/*N*/ 		}
/*N*/ 
/*N*/ 		pPara->Invalidate();
/*N*/ 		ImplCalcBulletText( n, FALSE, FALSE );
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::SetRefDevice( OutputDevice* pRefDev )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	pEditEngine->SetRefDevice( pRefDev );
/*N*/ 	for ( USHORT n = (USHORT) pParaList->GetParagraphCount(); n; )
/*N*/ 	{
/*N*/ 		Paragraph* pPara = pParaList->GetParagraph( --n );
/*N*/ 		pPara->Invalidate();
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::ParaAttribsChanged( USHORT nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	// Der Outliner hat kein eigenes Undo, wenn Absaetz getrennt/verschmolzen werden.
/*N*/ 	// Beim ParagraphInserted ist das Attribut EE_PARA_OUTLLEVEL
/*N*/ 	// ggf. noch nicht eingestellt, dies wird aber benoetigt um die Tiefe
/*N*/ 	// des Absatzes zu bestimmen.
/*N*/ 
/*N*/ 	if( pEditEngine->IsInUndo() )
/*N*/ 	{
/*?*/ 		if ( pParaList->GetParagraphCount() == pEditEngine->GetParagraphCount() )
/*?*/ 		{
/*?*/ 			Paragraph* pPara = pParaList->GetParagraph( nPara );
/*?*/ 			const SfxUInt16Item& rLevel = (const SfxUInt16Item&) pEditEngine->GetParaAttrib( nPara, EE_PARA_OUTLLEVEL );
/*?*/ 			if ( pPara->GetDepth() != rLevel.GetValue() )
/*?*/ 			{
/*?*/ 				USHORT nMin = Min( pPara->GetDepth(), (USHORT)rLevel.GetValue() );
/*?*/ 				pPara->SetDepth( rLevel.GetValue() );
/*?*/ 				ImplCalcBulletText( nPara, TRUE, TRUE );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ Rectangle Outliner::ImpCalcBulletArea( USHORT nPara, BOOL bAdjust, BOOL bReturnPaperPos )
/*N*/ {
/*N*/ 	// Bullet-Bereich innerhalb des Absatzes...
/*N*/ 	Rectangle aBulletArea;
/*N*/ 
/*N*/ 	const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 	if ( pFmt )
/*N*/ 	{
/*N*/ 		Point aTopLeft;
/*N*/ 		Size aBulletSize( ImplGetBulletSize( nPara ) );
/*N*/ 
/*N*/         BOOL bOutlineMode = ( pEditEngine->GetControlWord() & EE_CNTRL_OUTLINER ) != 0;
/*N*/ 
/*N*/         const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&) pEditEngine->GetParaAttrib( nPara, bOutlineMode ? EE_PARA_OUTLLRSPACE : EE_PARA_LRSPACE );
/*N*/ 		aTopLeft.X() = rLR.GetTxtLeft() + rLR.GetTxtFirstLineOfst();
/*N*/ 
/*N*/ 
/*N*/ 		long nBulletWidth = Max( (long) -rLR.GetTxtFirstLineOfst(), (long) ((-pFmt->GetFirstLineOffset()) + pFmt->GetCharTextDistance()) );
/*N*/ 		if ( nBulletWidth < aBulletSize.Width() ) 	// Bullet macht sich Platz
/*N*/ 			nBulletWidth = aBulletSize.Width();
/*N*/ 
/*N*/ 		if ( bAdjust && !bOutlineMode )
/*N*/ 		{{DBG_BF_ASSERT(0, "STRIP");}//STRIP001 
/*N*/ 		}
/*N*/ 
/*N*/ 		// Vertikal:
/*N*/ 		ParagraphInfos aInfos = pEditEngine->GetParagraphInfos( nPara );
/*N*/ 		if ( aInfos.bValid )
/*N*/ 		{
/*?*/ 			aTopLeft.Y() = /* aInfos.nFirstLineOffset + */ // #91076# nFirstLineOffset is already added to the StartPos (PaintBullet) from the EditEngine
/*?*/ 							aInfos.nFirstLineHeight - aInfos.nFirstLineTextHeight
/*?*/ 							+ aInfos.nFirstLineTextHeight / 2
/*?*/ 							- aBulletSize.Height() / 2;
/*?*/ 			// ggf. lieber auf der Baseline ausgeben...
/*?*/ 			if( ( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE ) && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) && ( pFmt->GetNumberingType() != SVX_NUM_CHAR_SPECIAL ) )
/*?*/ 			{
/*?*/ 				Font aBulletFont( ImpCalcBulletFont( nPara ) );
/*?*/ 				if ( aBulletFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )
/*?*/ 				{
/*?*/ 					OutputDevice* pRefDev = pEditEngine->GetRefDevice();
/*?*/ 					Font aOldFont = pRefDev->GetFont();
/*?*/ 					pRefDev->SetFont( aBulletFont );
/*?*/ 					FontMetric aMetric( pRefDev->GetFontMetric() );
/*?*/ 					// Leading der ersten Zeile...
/*?*/ 					aTopLeft.Y() = /* aInfos.nFirstLineOffset + */ aInfos.nFirstLineMaxAscent;
/*?*/ 					aTopLeft.Y() -= aMetric.GetAscent();
/*?*/ 					pRefDev->SetFont( aOldFont );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Horizontal:
/*N*/ 		if( pFmt->GetNumAdjust() == SVX_ADJUST_RIGHT )
/*N*/ 		{
/*?*/ 			aTopLeft.X() += nBulletWidth - aBulletSize.Width();
/*N*/ 		}
/*N*/ 		else if( pFmt->GetNumAdjust() == SVX_ADJUST_CENTER )
/*N*/ 		{
/*?*/ 			aTopLeft.X() += ( nBulletWidth - aBulletSize.Width() ) / 2;
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( aTopLeft.X() < 0 ) 	// dann draengeln
/*?*/ 			aTopLeft.X() = 0;
/*N*/ 
/*N*/ 		aBulletArea = Rectangle( aTopLeft, aBulletSize );
/*N*/ 	}
/*N*/     if ( bReturnPaperPos )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 Size aBulletSize( aBulletArea.GetSize() );
/*N*/     }
/*N*/ 	return aBulletArea;
/*N*/ }



/*N*/ EBulletInfo Outliner::GetBulletInfo( USHORT nPara )
/*N*/ {
/*N*/     EBulletInfo aInfo;
/*N*/ 
/*N*/     aInfo.nParagraph = nPara;
/*N*/     aInfo.bVisible = ImplHasBullet( nPara );
/*N*/ 
/*N*/     const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/     aInfo.nType = pFmt ? pFmt->GetNumberingType() : 0;
/*N*/ 
/*N*/     if( pFmt )
/*N*/     {        
/*N*/         if( pFmt->GetNumberingType() != SVX_NUM_BITMAP )
/*N*/         {
/*N*/             aInfo.aText = ImplGetBulletText( nPara );
/*N*/             
/*N*/             if( pFmt->GetBulletFont() )
/*N*/                 aInfo.aFont = *pFmt->GetBulletFont();
/*N*/         }
/*N*/         else if ( pFmt->GetBrush()->GetGraphicObject() )
/*N*/         {
/*N*/             aInfo.aGraphic = pFmt->GetBrush()->GetGraphicObject()->GetGraphic();
/*N*/         }
/*N*/     }
/*N*/     
/*N*/     if ( aInfo.bVisible )
/*N*/     {
/*N*/         aInfo.aBounds = ImpCalcBulletArea( nPara, TRUE, TRUE );
/*N*/     }
/*N*/ 
/*N*/     return aInfo;
/*N*/ }

/*N*/ XubString Outliner::GetText( Paragraph* pParagraph, ULONG nCount ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	XubString aText;
/*N*/ 	USHORT nStartPara = (USHORT) pParaList->GetAbsPos( pParagraph );
/*N*/ 	for ( USHORT n = 0; n < nCount; n++ )
/*N*/ 	{
/*N*/ 		aText += pEditEngine->GetText( nStartPara + n );
/*N*/ 		if ( (n+1) < (USHORT)nCount )
/*N*/ 			aText += '\n';
/*N*/ 	}
/*N*/ 	return aText;
/*N*/ }


/*N*/ void Outliner::StripPortions()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	bStrippingPortions = TRUE;
/*N*/ 	pEditEngine->StripPortions();
/*N*/ 	bStrippingPortions = FALSE;
/*N*/ }

// #101498#
/*N*/ void Outliner::DrawingText( const Point& rStartPos, const XubString& rText, USHORT nTextStart, USHORT nTextLen, const sal_Int32* pDXArray,const SvxFont& rFont, USHORT nPara, USHORT nIndex, BYTE nRightToLeft)
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	
/*N*/ 	// #101498#
/*N*/ 	DrawPortionInfo aInfo( rStartPos, rText, nTextStart, nTextLen, rFont, nPara, nIndex, pDXArray, nRightToLeft);
/*N*/ 	
/*N*/ 	aDrawPortionHdl.Call( &aInfo );
/*N*/ }



/*N*/ SfxItemSet Outliner::GetParaAttribs( ULONG nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 	return pEditEngine->GetParaAttribs( (USHORT)nPara );
/*N*/ }

/*N*/ IMPL_LINK( Outliner, ParaVisibleStateChangedHdl, Paragraph*, pPara )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( Outliner, BeginMovingParagraphsHdl, MoveParagraphsInfo*, pInfos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( Outliner, BeginPasteOrDropHdl, PasteOrDropInfos*, pInfos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( Outliner, EndPasteOrDropHdl, PasteOrDropInfos*, pInfos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return 0;
/*N*/ }

/*N*/ IMPL_LINK( Outliner, EndMovingParagraphsHdl, MoveParagraphsInfo*, pInfos )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     return 0;
/*N*/ }

/*N*/ void Outliner::ImplCalcBulletText( USHORT nPara, BOOL bRecalcLevel, BOOL bRecalcChilds )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	USHORT nRelPos = 0xFFFF;
/*N*/ 
/*N*/ 	while ( pPara )
/*N*/ 	{
/*N*/ 		XubString aBulletText;
/*N*/ 		const SvxNumberFormat* pFmt = ImplGetBullet( nPara );
/*N*/ 		if( pFmt && ( pFmt->GetNumberingType() != SVX_NUM_BITMAP ) )
/*N*/ 		{
/*N*/ 			aBulletText += pFmt->GetPrefix();
/*N*/ 			if( pFmt->GetNumberingType() == SVX_NUM_CHAR_SPECIAL )
/*N*/ 			{
/*N*/ 				aBulletText += pFmt->GetBulletChar();
/*N*/ 			}
/*N*/ 			else if( pFmt->GetNumberingType() != SVX_NUM_NUMBER_NONE )
/*N*/ 			{
/*?*/ 				if ( nRelPos == 0xFFFF )
/*NBFF*/				pParaList->GetParent( pPara, nRelPos );
/*?*/ 				USHORT nNumber = nRelPos + pFmt->GetStart();
/*?*/ 				aBulletText += pFmt->GetNumStr( nNumber );
/*N*/ 			}
/*N*/ 			aBulletText += pFmt->GetSuffix();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aBulletText != pPara->GetText() )
/*N*/ 			pPara->SetText( aBulletText );
/*N*/ 
/*N*/ 		pPara->nFlags &= (~PARAFLAG_SETBULLETTEXT);
/*N*/ 
/*N*/ 		if ( bRecalcLevel )
/*N*/ 		{
/*N*/ 			if ( nRelPos != 0xFFFF )
/*N*/ 				nRelPos++;
/*N*/ 
/*N*/ 			USHORT nDepth = pPara->GetDepth();
/*N*/ 			pPara = pParaList->GetParagraph( ++nPara );
/*N*/ 			if ( !bRecalcChilds )
/*N*/ 			{
/*N*/ 				while ( pPara && ( pPara->GetDepth() > nDepth ) )
/*?*/ 					pPara = pParaList->GetParagraph( ++nPara );
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pPara && ( pPara->GetDepth() < nDepth ) )
/*?*/ 				pPara = NULL;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pPara = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void Outliner::Clear()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(Outliner,0);
/*N*/ 
/*N*/ 	if( !bFirstParaIsEmpty )
/*N*/ 	{
/*N*/ 		ImplBlockInsertionCallbacks( TRUE );
/*N*/ 		pEditEngine->Clear();
/*N*/ 		pParaList->Clear( TRUE );
/*N*/ 		pParaList->Insert( new Paragraph( nMinDepth ), LIST_APPEND );
/*N*/ 		bFirstParaIsEmpty = TRUE;
/*N*/ 		ImplBlockInsertionCallbacks( FALSE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pParaList->GetParagraph( 0 )->SetDepth( nMinDepth );
/*N*/ 	}
/*N*/ }


/*N*/ String Outliner::ImplGetBulletText( USHORT nPara )
/*N*/ {
/*N*/ 	Paragraph* pPara = pParaList->GetParagraph( nPara );
/*N*/ 	// MT: Optimierung mal wieder aktivieren...
/*N*/ //	if( pPara->nFlags & PARAFLAG_SETBULLETTEXT )
/*N*/ 		ImplCalcBulletText( nPara, FALSE, FALSE );
/*N*/ 	return pPara->GetText();
/*N*/ }

// this is needed for StarOffice Api
/*N*/ void Outliner::SetLevelDependendStyleSheet( USHORT nPara )
/*N*/ {
/*N*/ 	SfxItemSet aOldAttrs( pEditEngine->GetParaAttribs( nPara ) );
/*N*/ 	ImplSetLevelDependendStyleSheet( nPara );
/*N*/ 	pEditEngine->SetParaAttribs( nPara, aOldAttrs );
/*N*/ }

/*N*/ SV_IMPL_PTRARR( NotifyList, EENotifyPtr );

/*N*/ void Outliner::ImplBlockInsertionCallbacks( BOOL b )
/*N*/ {
/*N*/     if ( b )
/*N*/     {
/*N*/         bBlockInsCallback++;
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         DBG_ASSERT( bBlockInsCallback, "ImplBlockInsertionCallbacks ?!" );
/*N*/         bBlockInsCallback--;
/*N*/         if ( !bBlockInsCallback )
/*N*/         {
/*N*/             // Call blocked notify events...
/*N*/             while ( pEditEngine->aNotifyCache.Count() )
/*N*/             {
/*?*/                 EENotify* pNotify = pEditEngine->aNotifyCache[0];
/*?*/                 // Remove from list before calling, maybe we enter LeaveBlockNotifications while calling the handler...
/*?*/                 pEditEngine->aNotifyCache.Remove( 0 );
/*?*/                 pEditEngine->aOutlinerNotifyHdl.Call( pNotify );
/*?*/                 delete pNotify;
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }


/*N*/ IMPL_LINK( Outliner, EditEngineNotifyHdl, EENotify*, pNotify )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     return 0;
/*N*/ }

/*N*/ sal_Bool DrawPortionInfo::IsRTL() const 
/*N*/ { 
/*N*/ 	if(0xFF == mnBiDiLevel)
/*N*/ 	{
/*?*/         // Use Bidi functions from icu 2.0 to calculate if this portion
/*?*/ 		// is RTL or not.
/*?*/         UErrorCode nError(U_ZERO_ERROR);
/*?*/         UBiDi* pBidi = ubidi_openSized(rText.Len(), 0, &nError);
/*?*/         nError = U_ZERO_ERROR;
/*?*/ 
/*?*/ 		// I do not have this info here. Is it necessary? I'll have to ask MT.
/*?*/ 	    const BYTE nDefaultDir = UBIDI_LTR; //IsRightToLeft( nPara ) ? UBIDI_RTL : UBIDI_LTR;
/*?*/     
/*?*/ 		ubidi_setPara(pBidi, reinterpret_cast<const UChar *>(rText.GetBuffer()), rText.Len(), nDefaultDir, NULL, &nError);	// UChar != sal_Unicode in MinGW
/*?*/         nError = U_ZERO_ERROR;
/*?*/ 
/*?*/         sal_Int32 nCount(ubidi_countRuns(pBidi, &nError));
/*?*/ 
/*?*/         int32_t nStart(0);
/*?*/         int32_t nEnd;
/*?*/         UBiDiLevel nCurrDir;
/*?*/ 
/*?*/ 		ubidi_getLogicalRun(pBidi, nStart, &nEnd, &nCurrDir);
/*?*/ 
/*?*/         ubidi_close(pBidi);
/*?*/ 
/*?*/ 		// remember on-demand calculated state
/*?*/ 		((DrawPortionInfo*)this)->mnBiDiLevel = nCurrDir;
/*N*/ 	}
/*N*/ 
/*N*/ 	return (1 == (mnBiDiLevel % 2)); 
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
