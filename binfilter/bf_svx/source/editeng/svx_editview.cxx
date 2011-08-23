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

#define _SOLAR__PRIVATE 1




#include <vcl/window.hxx>

#include <impedit.hxx>
#include <editeng.hxx>
#include <editview.hxx>



#include "itemdata.hxx"



#include <editeng.hrc>
#include <helpid.hrc>

#include <com/sun/star/beans/PropertyValues.hdl>
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;


/*N*/ DBG_NAME( EditView )

// From SW => Create common method


/*N*/ EditView::~EditView()
/*N*/ {
/*N*/ 	DBG_DTOR( EditView, 0 );
/*N*/ 	delete pImpEditView;
/*N*/ }


/*N*/ EditEngine* EditView::GetEditEngine() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/ 	return pImpEditView->pEditEngine;
/*N*/ }

/*N*/ void EditView::SetSelection( const ESelection& rESel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/ 
/*N*/ 	// Falls jemand gerade ein leeres Attribut hinterlassen hat,
/*N*/ 	// und dann der Outliner die Selektion manipulitert:
/*N*/ 	if ( !pImpEditView->GetEditSelection().HasRange() )
/*N*/ 	{
/*N*/ 		ContentNode* pNode = pImpEditView->GetEditSelection().Max().GetNode();
/*N*/ 		PIMPEE->CursorMoved( pNode );
/*N*/ 	}
/*N*/ 	EditSelection aNewSelection( PIMPEE->ConvertSelection( rESel.nStartPara, rESel.nStartPos, rESel.nEndPara, rESel.nEndPos ) );
/*N*/ 
/*N*/ 	// Selektion darf nicht bei einem unsichtbaren Absatz Starten/Enden:
/*N*/ 	ParaPortion* pPortion = PIMPEE->FindParaPortion( aNewSelection.Min().GetNode() );
/*N*/ 	if ( !pPortion->IsVisible() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pPortion = PIMPEE->GetPrevVisPortion( pPortion );
/*N*/ 	}
/*N*/ 	pPortion = PIMPEE->FindParaPortion( aNewSelection.Max().GetNode() );
/*N*/ 	if ( !pPortion->IsVisible() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pPortion = PIMPEE->GetPrevVisPortion( pPortion );
/*N*/ 	}
/*N*/ 
/*N*/ 	pImpEditView->DrawSelection();	// alte Selektion 'weg-zeichnen'
/*N*/ 	pImpEditView->SetEditSelection( aNewSelection );
/*N*/ 	pImpEditView->DrawSelection();
/*N*/ 	sal_Bool bGotoCursor = pImpEditView->DoAutoScroll();
/*N*/ 	ShowCursor( bGotoCursor );
/*N*/ }

/*N*/ ESelection EditView::GetSelection() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/ 
/*N*/ 	ESelection aSelection;
/*N*/ 
/*N*/ 	aSelection.nStartPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Min().GetNode() );
/*N*/ 	aSelection.nEndPara = PIMPEE->GetEditDoc().GetPos( pImpEditView->GetEditSelection().Max().GetNode() );
/*N*/ 
/*N*/ 	aSelection.nStartPos = pImpEditView->GetEditSelection().Min().GetIndex();
/*N*/ 	aSelection.nEndPos = pImpEditView->GetEditSelection().Max().GetIndex();
/*N*/ 
/*N*/ 	return aSelection;
/*N*/ }

/*N*/ sal_Bool EditView::HasSelection() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	return pImpEditView->HasSelection();
/*N*/ }






/*N*/ Window*	EditView::GetWindow() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	return pImpEditView->pOutWin;
/*N*/ }

/*N*/ const Rectangle& EditView::GetVisArea() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	// Change return value to Rectangle in next incompatible build !!!
/*N*/ 	static Rectangle aRect;
/*N*/ 	aRect = pImpEditView->GetVisDocArea();
/*N*/ 	return aRect;
/*N*/ }

/*N*/ const Rectangle& EditView::GetOutputArea() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	return pImpEditView->GetOutputArea();
/*N*/ }


/*N*/ void EditView::ShowCursor( sal_Bool bGotoCursor, sal_Bool bForceVisCursor )
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	DBG_CHKOBJ( pImpEditView->pEditEngine, EditEngine, 0 );
/*N*/ 
/*N*/ // Draw vertraegt die Assertion nicht, spaeter mal aktivieren
/*N*/ //	DBG_ASSERT( pImpEditView->pEditEngine->HasView( this ), "ShowCursor - View nicht angemeldet!" );
/*N*/ //	DBG_ASSERT( !GetWindow()->IsInPaint(), "ShowCursor - Why in Paint ?!" );
/*N*/ 
/*N*/ 	if ( pImpEditView->pEditEngine->HasView( this ) )
/*N*/ 	{
/*N*/ 		// Das ControlWord hat mehr Gewicht:
/*N*/ 		if ( !pImpEditView->DoAutoScroll() )
/*N*/ 			bGotoCursor = sal_False;
/*N*/ 		pImpEditView->ShowCursor( bGotoCursor, bForceVisCursor );
/*N*/ 	}
/*N*/ }

/*N*/ void EditView::HideCursor()
/*N*/ {
/*N*/ 	DBG_CHKTHIS( EditView, 0 );
/*N*/ 	pImpEditView->GetCursor()->Hide();
/*N*/ }











#ifndef SVX_LIGHT
#endif

/*N*/ void EditView::Cut()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void EditView::Copy()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void EditView::Paste()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
