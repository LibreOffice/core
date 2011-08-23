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


// #define _SOLAR__PRIVATE 1
#ifdef _MSC_VER
#pragma hdrstop
#endif

#define USE_SVXFONT

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx> 


#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>
#include <eerdll2.hxx>
#include <eerdll.hxx>
#include <editeng.hrc>



#ifndef _SVX_ITEMDATA_HXX
#include "itemdata.hxx"
#endif



#include <flditem.hxx>

#include "lrspitem.hxx"


//#ifndef _SV_SYSTEM_HXX
//#include <vcl/system.hxx>
//#endif

#include <numitem.hxx>
#include <bulitem.hxx>





#if OSL_DEBUG_LEVEL > 1
#endif
namespace binfilter {

// Spaeter -> TOOLS\STRING.H (fuer Grep: WS_TARGET)

//using namespace ::rtl;
//using namespace ::utl;
using namespace ::com::sun::star;
//using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::lang;
//using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::linguistic2;



/*N*/ DBG_NAME( EditEngine )
/*N*/ DBG_NAMEEX( EditView )//STRIP008 ;

#if (OSL_DEBUG_LEVEL > 1) || defined ( DBG_UTIL )
/*N*/ static sal_Bool bDebugPaint = sal_False;
#endif

SV_IMPL_VARARR( EECharAttribArray, EECharAttrib );


// ----------------------------------------------------------------------
// EditEngine
// ----------------------------------------------------------------------
/*N*/ EditEngine::EditEngine( SfxItemPool* pItemPool )
/*N*/ {
/*N*/ 	DBG_CTOR( EditEngine, 0 );
/*N*/ 	pImpEditEngine = new ImpEditEngine( this, pItemPool );
/*N*/ }

/*N*/ EditEngine::~EditEngine()
/*N*/ {
/*N*/ 	DBG_DTOR( EditEngine, 0 );
/*N*/ 	delete pImpEditEngine;
/*N*/ }

/*N*/ void EditEngine::EnableUndo( sal_Bool bEnable )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->EnableUndo( bEnable );
/*N*/ }

/*N*/ sal_Bool EditEngine::IsUndoEnabled()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->IsUndoEnabled();
/*N*/ }

/*N*/ sal_Bool EditEngine::IsInUndo()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->IsInUndo();
/*N*/ }

#ifndef SVX_LIGHT
/*N*/ SfxUndoManager& EditEngine::GetUndoManager()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetUndoManager();
/*N*/ }
#endif

/*N*/ void EditEngine::UndoActionStart( sal_uInt16 nId )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Aufruf von UndoActionStart im Undomodus!" );
/*N*/ 	if ( !pImpEditEngine->IsInUndo() )
/*N*/ 		pImpEditEngine->UndoActionStart( nId );
/*N*/ }

/*N*/ void EditEngine::UndoActionEnd( sal_uInt16 nId )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	DBG_ASSERT( !pImpEditEngine->IsInUndo(), "Aufruf von UndoActionEnd im Undomodus!" );
/*N*/ 	if ( !pImpEditEngine->IsInUndo() )
/*N*/ 		pImpEditEngine->UndoActionEnd( nId );
/*N*/ }

/*N*/ void EditEngine::SetRefDevice( OutputDevice* pRefDev )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetRefDevice( pRefDev );
/*N*/ }

/*N*/ OutputDevice* EditEngine::GetRefDevice() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetRefDevice();
/*N*/ }

/*N*/ void EditEngine::SetRefMapMode( const MapMode& rMapMode )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetRefMapMode( rMapMode );
/*N*/ }

/*N*/ MapMode EditEngine::GetRefMapMode()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetRefMapMode();
/*N*/ }

/*N*/ void EditEngine::SetBackgroundColor( const Color& rColor )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetBackgroundColor( rColor );
/*N*/ }


/*N*/ Color EditEngine::GetAutoColor() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetAutoColor();
/*N*/ }

/*N*/ void EditEngine::EnableAutoColor( BOOL b )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->EnableAutoColor( b );
/*N*/ }



/*N*/ BOOL EditEngine::IsForceAutoColor() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->IsForceAutoColor();
/*N*/ }

/*N*/ const SfxItemSet& EditEngine::GetEmptyItemSet()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetEmptyItemSet();
/*N*/ }


/*N*/ EditView* EditEngine::RemoveView( EditView* pView )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	DBG_CHKOBJ( pView, EditView, 0 );
/*N*/ 
/*N*/ 	pView->HideCursor();
/*N*/ 	EditView* pRemoved = 0;
/*N*/ 	sal_uInt16 nPos = pImpEditEngine->GetEditViews().GetPos( pView );
/*N*/ 	DBG_ASSERT( nPos != USHRT_MAX, "RemoveView mit ungueltigem Index" );
/*N*/ 	if ( nPos != USHRT_MAX )
/*N*/ 	{
/*N*/ 		pRemoved = pImpEditEngine->GetEditViews().GetObject( nPos );
/*N*/ 		pImpEditEngine->GetEditViews().Remove( nPos );
/*N*/ 		if ( pImpEditEngine->GetActiveView() == pView )
/*N*/ 		{
/*N*/ 			pImpEditEngine->SetActiveView( 0 );
/*N*/ 			pImpEditEngine->GetSelEngine().SetCurView( 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRemoved;
/*N*/ }




/*N*/ sal_Bool EditEngine::HasView( EditView* pView ) const
/*N*/ {
/*N*/ 	return pImpEditEngine->GetEditViews().GetPos( pView ) != USHRT_MAX;
/*N*/ }



/*N*/ void EditEngine::SetDefTab( sal_uInt16 nDefTab )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->GetEditDoc().SetDefTab( nDefTab );
/*N*/ 	if ( pImpEditEngine->IsFormatted() )
/*N*/ 	{
/*N*/ 		pImpEditEngine->FormatFullDoc();
/*N*/ 		pImpEditEngine->UpdateViews( (EditView*) 0 );
/*N*/ 	}
/*N*/ }


/*N*/ void EditEngine::SetPaperSize( const Size& rNewSize )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	Size aOldSize( pImpEditEngine->GetPaperSize() );
/*N*/ 	pImpEditEngine->SetValidPaperSize( rNewSize );
/*N*/ 	Size aNewSize( pImpEditEngine->GetPaperSize() );
/*N*/ 
/*N*/ 	sal_Bool bAutoPageSize = pImpEditEngine->GetStatus().AutoPageSize();
/*N*/ 	if ( bAutoPageSize || ( aNewSize.Width() != aOldSize.Width() ) )
/*N*/ 	{
/*N*/ 		for ( sal_uInt16 nView = 0; nView < pImpEditEngine->aEditViews.Count(); nView++ )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EditView* pView = pImpEditEngine->aEditViews[nView];
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( bAutoPageSize || pImpEditEngine->IsFormatted() )
/*N*/ 		{
/*N*/ 			// Aendern der Breite hat bei AutoPageSize keine Wirkung, da durch
/*N*/ 			// Textbreite bestimmt.
/*N*/ 			// Optimierung erst nach Vobis-Auslieferung aktivieren...
/*N*/ //			if ( !bAutoPageSize )
/*N*/ 				pImpEditEngine->FormatFullDoc();
/*N*/ //			else
/*N*/ //			{
/*N*/ //				pImpEditEngine->FormatDoc();			// PageSize, falls Aenderung
/*N*/ //				pImpEditEngine->CheckAutoPageSize();	// Falls nichts formatiert wurde
/*N*/ //			}
/*N*/ 
/*N*/ 			pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
/*N*/ 
/*N*/ 			if ( pImpEditEngine->GetUpdateMode() && pImpEditEngine->GetActiveView() )
/*?*/ 				pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ const Size& EditEngine::GetPaperSize() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetPaperSize();
/*N*/ }

/*N*/ void EditEngine::SetVertical( BOOL bVertical )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetVertical( bVertical );
/*N*/ }

/*N*/ BOOL EditEngine::IsVertical() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->IsVertical();
/*N*/ }



/*N*/ USHORT EditEngine::GetScriptType( const ESelection& rSelection ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	EditSelection aSel( pImpEditEngine->CreateSel( rSelection ) );
/*N*/ 	return pImpEditEngine->GetScriptType( aSel );
/*N*/ }

/*N*/ LanguageType EditEngine::GetLanguage( USHORT nPara, USHORT nPos ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/     ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
/*N*/     DBG_ASSERT( pNode, "GetLanguage - nPara is invalid!" );
/*N*/     return pNode ? pImpEditEngine->GetLanguage( EditPaM( pNode, nPos ) ) : LANGUAGE_DONTKNOW;
/*N*/ }



/*N*/ void EditEngine::SetAsianCompressionMode( USHORT n )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	pImpEditEngine->SetAsianCompressionMode( n );
/*N*/ }


/*N*/ void EditEngine::SetKernAsianPunctuation( BOOL b )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	pImpEditEngine->SetKernAsianPunctuation( b );
/*N*/ }




/*N*/ void EditEngine::ClearPolygon()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetTextRanger( 0 );
/*N*/ }



/*N*/ void EditEngine::SetMinAutoPaperSize( const Size& rSz )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetMinAutoPaperSize( rSz );
/*N*/ }


/*N*/ void EditEngine::SetMaxAutoPaperSize( const Size& rSz )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetMaxAutoPaperSize( rSz );
/*N*/ }

/*N*/ XubString EditEngine::GetText( LineEnd eEnd ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetEditDoc().GetText( eEnd );
/*N*/ }

/*N*/ XubString EditEngine::GetText( const ESelection& rESelection, const LineEnd eEnd ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	EditSelection aSel( pImpEditEngine->CreateSel( rESelection ) );
/*N*/ 	return pImpEditEngine->GetSelected( aSel, eEnd );
/*N*/ }

/*N*/ sal_uInt16 EditEngine::GetParagraphCount() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->aEditDoc.Count();
/*N*/ }

/*N*/ sal_uInt16 EditEngine::GetLineCount( sal_uInt16 nParagraph ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetLineCount( nParagraph );
/*N*/ }

/*N*/ sal_uInt16 EditEngine::GetLineLen( sal_uInt16 nParagraph, sal_uInt16 nLine ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 
/*N*/ }

/*?*/ sal_uInt32 EditEngine::GetLineHeight( sal_uInt16 nParagraph, sal_uInt16 nLine )
/*?*/ {{DBG_BF_ASSERT(0, "STRIP");} return 0;//STRIP001 
/*?*/ }

/*N*/ sal_uInt16 EditEngine::GetFirstLineOffset( sal_uInt16 nParagraph )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*?*/ 		pImpEditEngine->FormatDoc();
/*N*/ 	ParaPortion* pPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nParagraph );
/*N*/ 	return ( pPortion ? pPortion->GetFirstLineOffset() : 0 );
/*N*/ }

/*N*/ sal_uInt32 EditEngine::GetTextHeight( sal_uInt16 nParagraph ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 
/*N*/ }


/*N*/ ESelection EditEngine::GetWord( const ESelection& rSelection, USHORT nWordType  ) const
/*N*/ {
/*N*/     // ImpEditEngine-Iteration-Methods should be const!
/*N*/     EditEngine* pE = (EditEngine*)this;
/*N*/ 
/*N*/ 	EditSelection aSel( pE->pImpEditEngine->CreateSel( rSelection ) );
/*N*/ 	aSel = pE->pImpEditEngine->SelectWord( aSel, nWordType );
/*N*/ 	return pE->pImpEditEngine->CreateESel( aSel );
/*N*/ }

/*N*/ sal_uInt32 EditEngine::GetTextHeight() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*?*/ 		pImpEditEngine->FormatDoc();
/*N*/ 
/*N*/ 	sal_uInt32 nHeight = !IsVertical() ? pImpEditEngine->GetTextHeight() : pImpEditEngine->CalcTextWidth( TRUE );
/*N*/ 	return nHeight;
/*N*/ }

/*N*/ sal_uInt32 EditEngine::CalcTextWidth()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*?*/ 		pImpEditEngine->FormatDoc();
/*N*/ 
/*N*/ 	sal_uInt32 nWidth = !IsVertical() ? pImpEditEngine->CalcTextWidth( TRUE ) : pImpEditEngine->GetTextHeight();
/*N*/  	return nWidth;
/*N*/ }

/*N*/ void EditEngine::SetUpdateMode( sal_Bool bUpdate )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetUpdateMode( bUpdate );
/*N*/ 	if ( pImpEditEngine->pActiveView )
/*?*/ 		pImpEditEngine->pActiveView->ShowCursor( sal_False, sal_False );
/*N*/ }

/*N*/ sal_Bool EditEngine::GetUpdateMode() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetUpdateMode();
/*N*/ }

/*N*/ void EditEngine::Clear()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->Clear();
/*N*/ }

/*N*/ void EditEngine::SetText( const XubString& rText )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetText( rText );
/*N*/ 	if ( rText.Len() )
/*N*/ 		pImpEditEngine->FormatAndUpdate();
/*N*/ }

/*N*/ EditTextObject*	EditEngine::CreateTextObject()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->CreateTextObject();
/*N*/ }


/*N*/ void EditEngine::SetText( const EditTextObject& rTextObject )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/     pImpEditEngine->EnterBlockNotifications();
/*N*/ 	pImpEditEngine->SetText( rTextObject );
/*N*/ 	pImpEditEngine->FormatAndUpdate();
/*N*/     pImpEditEngine->LeaveBlockNotifications();
/*N*/ }



/*N*/ void EditEngine::SetNotifyHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetNotifyHdl( rLink );
/*N*/ }

/*N*/ Link EditEngine::GetNotifyHdl() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetNotifyHdl();
/*N*/ }

/*N*/ void EditEngine::SetBeginMovingParagraphsHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->aBeginMovingParagraphsHdl = rLink;
/*N*/ }

/*N*/ void EditEngine::SetEndMovingParagraphsHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->aEndMovingParagraphsHdl = rLink;
/*N*/ }

/*N*/ void EditEngine::SetBeginPasteOrDropHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->aBeginPasteOrDropHdl = rLink;
/*N*/ }

/*N*/ void EditEngine::SetEndPasteOrDropHdl( const Link& rLink )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->aEndPasteOrDropHdl = rLink;
/*N*/ }

/*N*/ EditTextObject*	EditEngine::CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	DBG_ASSERT( nPara < pImpEditEngine->GetEditDoc().Count(), "CreateTextObject: Startpara out of Range" );
/*N*/ 	DBG_ASSERT( nPara+nParas-1 < pImpEditEngine->GetEditDoc().Count(), "CreateTextObject: Endpara out of Range" );
/*N*/ 
/*N*/ 	ContentNode* pStartNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
/*N*/ 	ContentNode* pEndNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara+nParas-1 );
/*N*/ 	DBG_ASSERT( pStartNode, "Start-Absatz existiert nicht: CreateTextObject" );
/*N*/ 	DBG_ASSERT( pEndNode, "End-Absatz existiert nicht: CreateTextObject" );
/*N*/ 
/*N*/ 	if ( pStartNode && pEndNode )
/*N*/ 	{
/*N*/ 		EditSelection aTmpSel;
/*N*/ 		aTmpSel.Min() = EditPaM( pStartNode, 0 );
/*N*/ 		aTmpSel.Max() = EditPaM( pEndNode, pEndNode->Len() );
/*N*/ 		return pImpEditEngine->CreateTextObject( aTmpSel );
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }


/*N*/ sal_uInt16 EditEngine::GetTextLen( sal_uInt16 nPara ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
/*N*/ 	DBG_ASSERT( pNode, "Absatz nicht gefunden: GetTextLen" );
/*N*/ 	if ( pNode )
/*N*/ 		return pNode->Len();
/*N*/ 	return 0;
/*N*/ }

/*N*/ XubString EditEngine::GetText( sal_uInt16 nPara ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	XubString aStr;
/*N*/ 	if ( nPara < pImpEditEngine->GetEditDoc().Count() )
/*N*/ 		aStr = pImpEditEngine->GetEditDoc().GetParaAsString( nPara );
/*N*/ 	return aStr;
/*N*/ }

/*N*/ sal_Bool EditEngine::IsModified() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->IsModified();
/*N*/ }




/*N*/ void EditEngine::InsertParagraph( sal_uInt16 nPara, const XubString& rTxt )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	if ( nPara > GetParagraphCount() )
/*N*/ 	{
/*?*/ 		DBG_ASSERTWARNING( nPara == USHRT_MAX, "AbsatzNr zu Gro? aber nicht LIST_APPEND! " );
/*?*/ 		nPara = GetParagraphCount();
/*N*/ 	}
/*N*/ 
/*N*/ 	pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
/*N*/ 	EditPaM aPaM( pImpEditEngine->InsertParagraph( nPara ) );
/*N*/ 	// Bei einem InsertParagraph von aussen sollen keine Harten
/*N*/ 	// Attribute uebernommen werden !
/*N*/ 	pImpEditEngine->RemoveCharAttribs( nPara );
/*N*/ 	pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
/*N*/ 	pImpEditEngine->ImpInsertText( EditSelection( aPaM, aPaM ), rTxt );
/*N*/ 	pImpEditEngine->FormatAndUpdate();
/*N*/ }


/*N*/ void EditEngine::SetText( sal_uInt16 nPara, const XubString& rTxt )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	EditSelection* pSel = pImpEditEngine->SelectParagraph( nPara );
/*N*/ 	if ( pSel )
/*N*/ 	{
/*N*/ 		pImpEditEngine->UndoActionStart( EDITUNDO_INSERT );
/*N*/ 		pImpEditEngine->ImpInsertText( *pSel, rTxt );
/*N*/ 		pImpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
/*N*/ 		pImpEditEngine->FormatAndUpdate();
/*N*/ 		delete pSel;
/*N*/ 	}
/*N*/ }

/*N*/ void EditEngine::SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	// Keine Undoklammerung noetig.
/*N*/ 	pImpEditEngine->SetParaAttribs( nPara, rSet );
/*N*/ 	pImpEditEngine->FormatAndUpdate();
/*N*/ }

/*N*/ const SfxItemSet& EditEngine::GetParaAttribs( sal_uInt16 nPara ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetParaAttribs( nPara );
/*N*/ }

/*N*/ sal_Bool EditEngine::HasParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->HasParaAttrib( nPara, nWhich );
/*N*/ }

/*N*/ const SfxPoolItem& EditEngine::GetParaAttrib( sal_uInt16 nPara, sal_uInt16 nWhich )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/     return pImpEditEngine->GetParaAttrib( nPara, nWhich );
/*N*/ }

/*N*/ void EditEngine::GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ SfxItemSet EditEngine::GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 	return pImpEditEngine->GetAttribs( aSel, bOnlyHardAttrib );
/*N*/ }

/*N*/ SfxItemSet EditEngine::GetAttribs( USHORT nPara, USHORT nStart, USHORT nEnd, sal_uInt8 nFlags ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetAttribs( nPara, nStart, nEnd, nFlags );
/*N*/ }


// MT: Can be removed after 6.x?


/*N*/ void EditEngine::StripPortions()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	VirtualDevice aTmpDev;
/*N*/ 	Rectangle aBigRec( Point( 0, 0 ), Size( 0x7FFFFFFF, 0x7FFFFFFF ) );
/*N*/ 	if ( IsVertical() )
/*N*/ 	{
/*?*/ 		aBigRec.Right() = 0;
/*?*/ 		aBigRec.Left() = -0x7FFFFFFF;
/*N*/ 	}
/*N*/ 	pImpEditEngine->Paint( &aTmpDev, aBigRec, Point(), sal_True );
/*N*/ }

/*N*/ void EditEngine::GetPortions( sal_uInt16 nPara, SvUShorts& rList )
/*N*/ {
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*N*/ 		pImpEditEngine->FormatFullDoc();
/*N*/ 
/*N*/ 	ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nPara );
/*N*/ 	if ( pParaPortion )
/*N*/ 	{
/*N*/ 		sal_uInt16 nEnd = 0;
/*N*/ 		sal_uInt16 nTextPortions = pParaPortion->GetTextPortions().Count();
/*N*/ 		for ( sal_uInt16 n = 0; n < nTextPortions; n++ )
/*N*/ 		{
/*N*/ 			nEnd += pParaPortion->GetTextPortions()[n]->GetLen();
/*N*/ 			rList.Insert( nEnd, rList.Count() );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ sal_Bool EditEngine::IsFlatMode() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return !( pImpEditEngine->aStatus.UseCharAttribs() );
/*N*/ }

/*N*/ void EditEngine::SetControlWord( sal_uInt32 nWord )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	if ( nWord != pImpEditEngine->aStatus.GetControlWord() )
/*N*/ 	{
/*N*/ 		sal_uInt32 nPrev = pImpEditEngine->aStatus.GetControlWord();
/*N*/ 		pImpEditEngine->aStatus.GetControlWord() = nWord;
/*N*/ 
/*N*/ 		sal_uInt32 nChanges = nPrev ^ nWord;
/*N*/ 		if ( pImpEditEngine->IsFormatted() )
/*N*/ 		{
/*N*/ 			// ggf. neu formatieren:
/*N*/ 			if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_USEPARAATTRIBS ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_ONECHARPERLINE ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_STRETCHING ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_OUTLINER ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_NOCOLORS ) ||
/*N*/ 				 ( nChanges & EE_CNTRL_OUTLINER2 ) )
/*N*/ 			{
/*N*/ 				if ( ( nChanges & EE_CNTRL_USECHARATTRIBS ) ||
/*N*/ 					 ( nChanges & EE_CNTRL_USEPARAATTRIBS ) )
/*N*/ 				{
/*?*/ 					sal_Bool bUseCharAttribs = ( nWord & EE_CNTRL_USECHARATTRIBS ) ? sal_True : sal_False;
/*?*/ 					pImpEditEngine->GetEditDoc().CreateDefFont( bUseCharAttribs );
/*N*/ 				}
/*N*/ 
/*N*/ 				pImpEditEngine->FormatFullDoc();
/*N*/ 				pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ sal_uInt32 EditEngine::GetControlWord() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->aStatus.GetControlWord();
/*N*/ }


/*N*/ Point EditEngine::GetDocPos( const Point& rPaperPos ) const
/*N*/ {
/*N*/ 	Point aDocPos( rPaperPos );
/*N*/ 	if ( IsVertical() )
/*N*/ 	{
/*?*/ 		aDocPos.X() = rPaperPos.Y();
/*?*/ 		aDocPos.Y() = GetPaperSize().Width() - rPaperPos.X();
/*N*/ 	}
/*N*/ 	return aDocPos;
/*N*/ }

/*N*/ Point EditEngine::GetDocPosTopLeft( sal_uInt16 nParagraph )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	ParaPortion* pPPortion = pImpEditEngine->GetParaPortions().SaveGetObject( nParagraph );
/*N*/ 	DBG_ASSERT( pPPortion, "Absatz nicht gefunden: GetWindowPosTopLeft" );
/*N*/ 	Point aPoint;
/*N*/ 	if ( pPPortion )
/*N*/ 	{
/*N*/ 		// Falls jemand mit einer leeren Engine ein GetLineHeight() macht.
/*N*/ 	    DBG_ASSERT( pImpEditEngine->IsFormatted() || !pImpEditEngine->IsFormatting(), "GetDocPosTopLeft: Doc not formatted - unable to format!" );
/*N*/ 		if ( !pImpEditEngine->IsFormatted() )
/*?*/ 			pImpEditEngine->FormatAndUpdate();
/*N*/ 		if ( pPPortion->GetLines().Count() )
/*N*/ 		{
/*N*/ 			// So richtiger, falls grosses Bullet.
/*N*/ 			EditLine* pFirstLine = pPPortion->GetLines()[0];
/*N*/ 			aPoint.X() = pFirstLine->GetStartPosX();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			const SvxLRSpaceItem& rLRItem = pImpEditEngine->GetLRSpaceItem( pPPortion->GetNode() );
/*?*/ 			aPoint.X() = pImpEditEngine->GetXValue( (short)(rLRItem.GetTxtLeft() + rLRItem.GetTxtFirstLineOfst()) );
/*N*/ 		}
/*N*/ 		aPoint.Y() = pImpEditEngine->GetParaPortions().GetYOffset( pPPortion );
/*N*/ 	}
/*N*/ 	return aPoint;
/*N*/ }


/*N*/ sal_Bool EditEngine::IsTextPos( const Point& rPaperPos, sal_uInt16 nBorder )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*?*/ 		pImpEditEngine->FormatDoc();
/*N*/ 
/*N*/ 	sal_Bool bTextPos = sal_False;
/*N*/ 	// #90780# take unrotated positions for calculation here
/*N*/ 	Point aDocPos = GetDocPos( rPaperPos );
/*N*/ 
/*N*/ 	if ( ( aDocPos.Y() > 0  ) && ( aDocPos.Y() < (long)pImpEditEngine->GetTextHeight() ) )
/*N*/ 	{
/*N*/ 		EditPaM aPaM = pImpEditEngine->GetPaM( aDocPos, sal_False );
/*N*/ 		if ( aPaM.GetNode() )
/*N*/ 		{
/*N*/ 			ParaPortion* pParaPortion = pImpEditEngine->FindParaPortion( aPaM.GetNode() );
/*N*/ 			DBG_ASSERT( pParaPortion, "ParaPortion?" );
/*N*/ 
/*N*/             sal_uInt16 nLine = pParaPortion->GetLineNumber( aPaM.GetIndex() );
/*N*/             EditLine* pLine = pParaPortion->GetLines().GetObject( nLine );
/*N*/             Range aLineXPosStartEnd = pImpEditEngine->GetLineXPosStartEnd( pParaPortion, pLine );
/*N*/ 			if ( ( aDocPos.X() >= aLineXPosStartEnd.Min() - nBorder ) &&
/*N*/ 				 ( aDocPos.X() <= aLineXPosStartEnd.Max() + nBorder ) )
/*N*/ 			{
/*N*/ 				 bTextPos = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bTextPos;
/*N*/ }

/*N*/ void EditEngine::SetEditTextObjectPool( SfxItemPool* pPool )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetEditTextObjectPool( pPool );
/*N*/ }

/*N*/ SfxItemPool* EditEngine::GetEditTextObjectPool() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetEditTextObjectPool();
/*N*/ }

/*N*/ void EditEngine::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 
/*N*/ 	pImpEditEngine->SetAttribs( aSel, rSet );
/*N*/ }


/*N*/ void EditEngine::QuickInsertText( const XubString& rText, const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 
/*N*/ 	pImpEditEngine->ImpInsertText( aSel, rText );
/*N*/ }

/*N*/ void EditEngine::QuickDelete( const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 
/*N*/ 	pImpEditEngine->ImpDeleteSelection( aSel );
/*N*/ }


/*N*/ void EditEngine::QuickInsertLineBreak( const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 
/*N*/ 	pImpEditEngine->InsertLineBreak( aSel );
/*N*/ }

/*N*/ void EditEngine::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	EditSelection aSel( pImpEditEngine->
/*N*/ 		ConvertSelection( rSel.nStartPara, rSel.nStartPos, rSel.nEndPara, rSel.nEndPos ) );
/*N*/ 
/*N*/ 	pImpEditEngine->ImpInsertFeature( aSel, rFld );
/*N*/ }

/*N*/ void EditEngine::QuickFormatDoc( sal_Bool bFull )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	if ( bFull )
/*N*/ 		pImpEditEngine->FormatFullDoc();
/*N*/ 	else
/*N*/ 		pImpEditEngine->FormatDoc();
/*N*/ 	pImpEditEngine->UpdateViews( pImpEditEngine->GetActiveView() );
/*N*/ }

/*N*/ void EditEngine::QuickRemoveCharAttribs( sal_uInt16 nPara, sal_uInt16 nWhich )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->RemoveCharAttribs( nPara, nWhich );
/*N*/ }

/*N*/ void EditEngine::SetStyleSheet( sal_uInt16 nPara, SfxStyleSheet* pStyle )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetStyleSheet( nPara, pStyle );
/*N*/ }

/*N*/ SfxStyleSheet* EditEngine::GetStyleSheet( sal_uInt16 nPara ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetStyleSheet( nPara );
/*N*/ }

/*N*/ void EditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetStyleSheetPool( pSPool );
/*N*/ }

/*N*/ SfxStyleSheetPool* EditEngine::GetStyleSheetPool()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->GetStyleSheetPool();
/*N*/ }

/*N*/ void EditEngine::SetWordDelimiters( const XubString& rDelimiters )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->aWordDelimiters = rDelimiters;
/*N*/ 	if ( pImpEditEngine->aWordDelimiters.Search( CH_FEATURE ) == STRING_NOTFOUND )
/*N*/ 		pImpEditEngine->aWordDelimiters.Insert( CH_FEATURE );
/*N*/ }

/*N*/ XubString EditEngine::GetWordDelimiters() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return pImpEditEngine->aWordDelimiters;
/*N*/ }







/*N*/ void EditEngine::EraseVirtualDevice()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->EraseVirtualDevice();
/*N*/ }

/*N*/ void EditEngine::SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetForbiddenCharsTable( xForbiddenChars );
/*N*/ }



/*N*/ void EditEngine::SetDefaultLanguage( LanguageType eLang )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetDefaultLanguage( eLang );
/*N*/ }






/*N*/ void EditEngine::SetGlobalCharStretching( sal_uInt16 nX, sal_uInt16 nY )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	pImpEditEngine->SetCharStretching( nX, nY );
/*N*/ }


/*N*/ sal_Bool EditEngine::ShouldCreateBigTextObject() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	sal_uInt16 nTextPortions = 0;
/*N*/ 	sal_uInt16 nParas = pImpEditEngine->GetEditDoc().Count();
/*N*/ 	for ( sal_uInt16 nPara = 0; nPara < nParas; nPara++  )
/*N*/ 	{
/*N*/ 		ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
/*N*/ 		nTextPortions += pParaPortion->GetTextPortions().Count();
/*N*/ 	}
/*N*/ 	return ( nTextPortions >= pImpEditEngine->GetBigTextObjectStart() ) ? sal_True : sal_False;
/*N*/ }

/*N*/ USHORT EditEngine::GetFieldCount( USHORT nPara ) const
/*N*/ {
/*N*/     USHORT nFields = 0;
/*N*/     ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
/*N*/     if ( pNode )
/*N*/     {
/*N*/ 	    const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
/*N*/ 	    for ( sal_uInt16 nAttr = 0; nAttr < rAttrs.Count(); nAttr++ )
/*N*/ 	    {
/*N*/ 		    EditCharAttrib* pAttr = rAttrs[nAttr];
/*N*/ 			if ( pAttr->Which() == EE_FEATURE_FIELD )
/*N*/                 nFields++;
/*N*/ 	    }
/*N*/     }
/*N*/ 
/*N*/     return nFields;
/*N*/ }

/*N*/ EFieldInfo EditEngine::GetFieldInfo( USHORT nPara, USHORT nField ) const
/*N*/ {
/*N*/     ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
/*N*/     if ( pNode )
/*N*/     {
/*N*/         USHORT nCurrentField = 0;
/*N*/ 	    const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
/*N*/ 	    for ( sal_uInt16 nAttr = 0; nAttr < rAttrs.Count(); nAttr++ )
/*N*/ 	    {
/*N*/ 		    EditCharAttrib* pAttr = rAttrs[nAttr];
/*N*/ 			if ( pAttr->Which() == EE_FEATURE_FIELD )
/*N*/             {
/*N*/                 if ( nCurrentField == nField )
/*N*/                 {
/*N*/                     EFieldInfo aInfo( *(const SvxFieldItem*)pAttr->GetItem(), nPara, pAttr->GetStart() );
/*N*/                     aInfo.aCurrentText = ((EditCharAttribField*)pAttr)->GetFieldValue();
/*N*/                     return aInfo;
/*N*/                 }
/*N*/ 
/*N*/                 nCurrentField++;
/*N*/             }
/*N*/ 	    }
/*N*/     }
/*N*/     return EFieldInfo();
/*N*/ }


/*N*/ sal_Bool EditEngine::UpdateFields()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	sal_Bool bChanges = pImpEditEngine->UpdateFields();
/*N*/ 	if ( bChanges )
/*N*/ 		pImpEditEngine->FormatAndUpdate();
/*N*/ 	return bChanges;
/*N*/ }

/*N*/ USHORT EditEngine::FindParagraph( long nDocPosY )
/*N*/ {
/*N*/ 	return pImpEditEngine->GetParaPortions().FindParagraph( nDocPosY );
/*N*/ }

/*N*/ EPosition EditEngine::FindDocPosition( const Point& rDocPos ) const
/*N*/ {
/*N*/     EPosition aPos;
/*N*/     // From the point of the API, this is const....
/*N*/     EditPaM aPaM = ((EditEngine*)this)->pImpEditEngine->GetPaM( rDocPos, FALSE );
/*N*/     if ( aPaM.GetNode() )
/*N*/     {
/*N*/         aPos.nPara = pImpEditEngine->aEditDoc.GetPos( aPaM.GetNode() );
/*N*/         aPos.nIndex = aPaM.GetIndex();
/*N*/     }
/*N*/     return aPos;
/*N*/ }

/*N*/ Rectangle EditEngine::GetCharacterBounds( const EPosition& rPos ) const
/*N*/ {
/*N*/     Rectangle aBounds;
/*N*/     ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( rPos.nPara );
/*N*/ 
/*N*/ 	// #109151# Check against index, not paragraph
/*N*/     if ( pNode && ( rPos.nIndex < pNode->Len() ) )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/     return aBounds;
/*N*/ }

/*N*/ ParagraphInfos EditEngine::GetParagraphInfos( sal_uInt16 nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/ 	// Funktioniert nur, wenn nicht bereits in der Formatierung...
/*N*/ 	if ( !pImpEditEngine->IsFormatted() )
/*N*/ 		pImpEditEngine->FormatDoc();
/*N*/ 
/*N*/ 	ParagraphInfos aInfos;
/*N*/ 	aInfos.bValid = pImpEditEngine->IsFormatted();
/*N*/ 	if ( pImpEditEngine->IsFormatted() )
/*N*/ 	{
/*?*/ 		ParaPortion* pParaPortion = pImpEditEngine->GetParaPortions()[nPara];
/*?*/ 		EditLine* pLine = pParaPortion ? pParaPortion->GetLines().GetObject( 0 ) : NULL;
/*?*/ 		DBG_ASSERT( pParaPortion && pLine, "GetParagraphInfos - Paragraph out of range" );
/*?*/ 		if ( pParaPortion && pLine )
/*?*/ 		{
/*?*/ 			aInfos.nParaHeight = (USHORT)pParaPortion->GetHeight();
/*?*/ 			aInfos.nLines = pParaPortion->GetLines().Count();
/*?*/ 			aInfos.nFirstLineStartX = pLine->GetStartPosX();
/*?*/ 			aInfos.nFirstLineOffset = pParaPortion->GetFirstLineOffset();
/*?*/ 			aInfos.nFirstLineHeight = pLine->GetHeight();
/*?*/ 			aInfos.nFirstLineTextHeight = pLine->GetTxtHeight();
/*?*/ 			aInfos.nFirstLineMaxAscent = pLine->GetMaxAscent();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return aInfos;
/*N*/ }


// =====================================================================
// ======================   Virtuelle Methoden   =======================
// =====================================================================
/*N*/ void __EXPORT EditEngine::DrawingText( const Point&, const XubString&, USHORT nTextStart, USHORT nTextLen, const sal_Int32*, const SvxFont&, sal_uInt16 nPara, sal_uInt16 nIndex, BYTE nRightToLeft )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ }

/*N*/ void __EXPORT EditEngine::PaintingFirstLine( sal_uInt16, const Point&, long, const Point&, short, OutputDevice* )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ }

/*N*/ void __EXPORT EditEngine::ParagraphInserted( sal_uInt16 nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/     if ( GetNotifyHdl().IsSet() )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EENotify aNotify( EE_NOTIFY_PARAGRAPHINSERTED );
/*N*/     }
/*N*/ }

/*N*/ void __EXPORT EditEngine::ParagraphDeleted( sal_uInt16 nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/     if ( GetNotifyHdl().IsSet() )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EENotify aNotify( EE_NOTIFY_PARAGRAPHREMOVED );
/*N*/     }
/*N*/ }

/*N*/ sal_Bool __EXPORT EditEngine::FormattingParagraph( sal_uInt16 )
/*N*/ {
/*N*/ 	// return sal_True, wenn die Attribute geaendert wurden...
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return sal_False;
/*N*/ }

/*N*/ void __EXPORT EditEngine::ParaAttribsChanged( sal_uInt16 /* nParagraph */ )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ }


/*N*/ void __EXPORT EditEngine::ParagraphHeightChanged( sal_uInt16 nPara )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 
/*N*/     if ( GetNotifyHdl().IsSet() )
/*N*/     {
/*?*/         DBG_BF_ASSERT(0, "STRIP"); //STRIP001 EENotify aNotify( EE_NOTIFY_TEXTHEIGHTCHANGED );
/*N*/     }
/*N*/ }

/*N*/ XubString __EXPORT EditEngine::GetUndoComment( sal_uInt16 nId ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	XubString aComment;
/*N*/ 	switch ( nId )
/*N*/ 	{
/*N*/ 		case EDITUNDO_REMOVECHARS:
/*N*/ 		case EDITUNDO_CONNECTPARAS:
/*N*/ 		case EDITUNDO_REMOVEFEATURE:
/*N*/ 		case EDITUNDO_DELCONTENT:
/*N*/ 		case EDITUNDO_DELETE:
/*N*/ 		case EDITUNDO_CUT:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_DEL ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_MOVEPARAGRAPHS:
/*N*/ 		case EDITUNDO_MOVEPARAS:
/*N*/ 		case EDITUNDO_DRAGANDDROP:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_MOVE ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_INSERTFEATURE:
/*N*/ 		case EDITUNDO_SPLITPARA:
/*N*/ 		case EDITUNDO_INSERTCHARS:
/*N*/ 		case EDITUNDO_PASTE:
/*N*/ 		case EDITUNDO_INSERT:
/*N*/ 		case EDITUNDO_READ:
/*N*/ 			aComment = XubString( EditResId( RID_EDITUNDO_INSERT ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_SRCHANDREPL:
/*N*/ 		case EDITUNDO_REPLACEALL:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_REPLACE ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_ATTRIBS:
/*N*/ 		case EDITUNDO_PARAATTRIBS:
/*N*/ 		case EDITUNDO_STRETCH:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_SETATTRIBS ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_RESETATTRIBS:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_RESETATTRIBS ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_STYLESHEET:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_SETSTYLE ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_TRANSLITERATE:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_TRANSLITERATE ) );
/*N*/ 		break;
/*N*/ 		case EDITUNDO_INDENTBLOCK:
/*N*/ 		case EDITUNDO_UNINDENTBLOCK:
/*?*/ 			aComment = XubString( EditResId( RID_EDITUNDO_INDENT ) );
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return aComment;
/*N*/ }

/*N*/ Rectangle EditEngine::GetBulletArea( sal_uInt16 nPara )
/*N*/ {
/*N*/ 	return Rectangle( Point(), Point() );
/*N*/ }

/*N*/ XubString __EXPORT EditEngine::CalcFieldValue( const SvxFieldItem& rField, sal_uInt16, sal_uInt16, Color*&, Color*& )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditEngine, 0 );
/*N*/ 	return ' ';
/*N*/ }



// =====================================================================
// ======================   Statische Methoden   =======================
// =====================================================================
/*N*/ SfxItemPool* EditEngine::CreatePool( sal_Bool bPersistentRefCounts )
/*N*/ {
/*N*/ 	SfxItemPool* pPool = new EditEngineItemPool( bPersistentRefCounts );
/*N*/ 	return pPool;
/*N*/ }


/*N*/ Font EditEngine::CreateFontFromItemSet( const SfxItemSet& rItemSet, USHORT nScriptType )
/*N*/ {
/*N*/ 	SvxFont aFont;
/*N*/ 	CreateFont( aFont, rItemSet, nScriptType );
/*N*/ 	return aFont;
/*N*/ }

// Maybe we can remove the next two methods, check after 6.x

/*N*/ SvxFont EditEngine::CreateSvxFontFromItemSet( const SfxItemSet& rItemSet )
/*N*/ {
/*N*/ 	SvxFont aFont;
/*N*/ 	CreateFont( aFont, rItemSet );
/*N*/ 	return aFont;
/*N*/ }


// Mal in den Outliner schieben...
/*N*/ void EditEngine::ImportBulletItem( SvxNumBulletItem& rNumBullet, sal_uInt16 nLevel,
/*N*/ 									const SvxBulletItem* pOldBullet, const SvxLRSpaceItem* pOldLRSpace )
/*N*/ {
/*N*/ 	if ( pOldBullet || pOldLRSpace )
/*N*/ 	{
/*N*/ 		// Numberformat dynamisch, weil Zuweisungsoperator nicht implementiert.
/*N*/ 
/*N*/ 		// Altes NumBulletItem nur uebernehmen, wenn kein altes BulletItem
/*N*/ 		const SvxNumberFormat* pFmt = ( !pOldBullet && ( rNumBullet.GetNumRule()->GetLevelCount() > nLevel ) ) ?
/*N*/ 										rNumBullet.GetNumRule()->Get( nLevel ) : NULL;
/*N*/ 		SvxNumberFormat* pNumberFormat = pFmt
/*N*/ 											? new SvxNumberFormat( *pFmt )
/*N*/ 											: new SvxNumberFormat( SVX_NUM_NUMBER_NONE );
/*N*/ 		if ( pOldBullet )
/*N*/ 		{
/*N*/ 			// Style
/*N*/ 			SvxExtNumType eNumType;
/*N*/ 			switch( pOldBullet->GetStyle() )
/*N*/ 			{
/*N*/ 				case BS_BMP:			eNumType = SVX_NUM_BITMAP;				break;
/*N*/ 				case BS_BULLET:			eNumType = SVX_NUM_CHAR_SPECIAL;		break;
/*N*/ 				case BS_ROMAN_BIG:		eNumType = SVX_NUM_ROMAN_UPPER;			break;
/*N*/ 				case BS_ROMAN_SMALL:	eNumType = SVX_NUM_ROMAN_LOWER;			break;
/*N*/ 				case BS_ABC_BIG:		eNumType = SVX_NUM_CHARS_UPPER_LETTER;	break;
/*N*/ 				case BS_ABC_SMALL:		eNumType = SVX_NUM_CHARS_LOWER_LETTER;	break;
/*N*/ 				case BS_123:			eNumType = SVX_NUM_ARABIC;				break;
/*N*/ 				default:				eNumType = SVX_NUM_NUMBER_NONE;			break;
/*N*/ 			}
/*N*/ 			pNumberFormat->SetNumberingType( eNumType );
/*N*/ 
/*N*/ 			// Justification
/*N*/ 			SvxAdjust eAdjust;
/*N*/ 			switch( pOldBullet->GetJustification() & (BJ_HRIGHT|BJ_HCENTER|BJ_HLEFT) )
/*N*/ 			{
/*?*/ 				case BJ_HRIGHT:		eAdjust = SVX_ADJUST_RIGHT;		break;
/*?*/ 				case BJ_HCENTER:	eAdjust = SVX_ADJUST_CENTER;	break;
/*N*/ 				default:			eAdjust = SVX_ADJUST_LEFT;		break;
/*N*/ 			}
/*N*/ 			pNumberFormat->SetNumAdjust(eAdjust);
/*N*/ 
/*N*/ 			// Prefix/Suffix
/*N*/ 			pNumberFormat->SetPrefix( pOldBullet->GetPrevText() );
/*N*/ 			pNumberFormat->SetSuffix( pOldBullet->GetFollowText() );
/*N*/ 
/*N*/ 			//Font
/*N*/ 			if ( eNumType != SVX_NUM_BITMAP )
/*N*/ 			{
/*N*/ 				Font aTmpFont = pOldBullet->GetFont();
/*N*/ 				pNumberFormat->SetBulletFont( &aTmpFont );
/*N*/ 			}
/*N*/ 
/*N*/ 			// Color
/*N*/ 			pNumberFormat->SetBulletColor( pOldBullet->GetFont().GetColor() );
/*N*/ 
/*N*/ 			// Start
/*N*/ 			pNumberFormat->SetStart( pOldBullet->GetStart() );
/*N*/ 
/*N*/ 			// Scale
/*N*/ 			pNumberFormat->SetBulletRelSize( pOldBullet->GetScale() );
/*N*/ 
/*N*/ 			// Bullet/Bitmap
/*N*/ 			if( eNumType == SVX_NUM_CHAR_SPECIAL )
/*N*/ 			{
/*N*/ 				pNumberFormat->SetBulletChar( pOldBullet->GetSymbol() );
/*N*/ 			}
/*N*/ 			else if( eNumType == SVX_NUM_BITMAP )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SvxBrushItem aBItem( Graphic( pOldBullet->GetBitmap() ), GPOS_NONE );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// Einzug und Erstzeileneinzug
/*N*/ 		if ( pOldLRSpace )
/*N*/ 		{
/*N*/ 			short nLSpace = (short)pOldLRSpace->GetTxtLeft();
/*N*/ 			pNumberFormat->SetLSpace( nLSpace );
/*N*/ 			pNumberFormat->SetAbsLSpace( nLSpace );
/*N*/ 			pNumberFormat->SetFirstLineOffset( pOldLRSpace->GetTxtFirstLineOfst() );
/*N*/ 		}
/*N*/ 
/*N*/ 		rNumBullet.GetNumRule()->SetLevel( nLevel, *pNumberFormat );
/*N*/ 		delete pNumberFormat;
/*N*/ 	}
/*N*/ }



// ---------------------------------------------------


/*N*/ EFieldInfo::EFieldInfo()
/*N*/ {
/*N*/     pFieldItem = NULL;
/*N*/ }


/*N*/ EFieldInfo::EFieldInfo( const SvxFieldItem& rFieldItem, USHORT nPara, USHORT nPos ) : aPosition( nPara, nPos )
/*N*/ {
/*N*/     pFieldItem = new SvxFieldItem( rFieldItem );
/*N*/ }

/*N*/ EFieldInfo::~EFieldInfo()
/*N*/ {
/*N*/     delete pFieldItem;
/*N*/ }

/*N*/ EFieldInfo::EFieldInfo( const EFieldInfo& rFldInfo )
/*N*/ {
/*N*/     *this = rFldInfo;
/*N*/ }

}
