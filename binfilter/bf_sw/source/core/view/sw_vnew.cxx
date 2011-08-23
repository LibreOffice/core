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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>		// Zugriff auf TxtCache
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _PTQUEUE_HXX
#include <ptqueue.hxx>
#endif
#ifndef _DVIEW_HXX
#include <dview.hxx>		// SdrView
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _ACCESSIBILITYOPTIONS_HXX
#include <accessibilityoptions.hxx>
#endif
namespace binfilter {

/*************************************************************************
|*
|*	ViewShell::Init()
|*
|*	Letzte Aenderung	MA 14. Jun. 96
|*
|*************************************************************************/

/*N*/ void ViewShell::Init( const SwViewOption *pNewOpt )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "ViewShell::Init" );
/*N*/ 
/*N*/ 	bDocSizeChgd = FALSE;
/*N*/ 
/*N*/ 	// Wir gehen auf Nummer sicher:
/*N*/ 	// Wir muessen die alten Fontinformationen wegschmeissen,
/*N*/ 	// wenn die Druckeraufloesung oder der Zoomfaktor sich aendert.
/*N*/ 	// Init() und Reformat() sind die sichersten Stellen.
/*N*/ 	pFntCache->Flush( );
/*N*/ 
/*N*/ 	// ViewOptions werden dynamisch angelegt
/*N*/ 	if( !pOpt )
/*N*/ 	{
/*N*/ 		pOpt = new SwViewOption;
/*N*/ 
/*N*/ 		// Ein ApplyViewOptions braucht nicht gerufen zu werden
/*N*/ 		if( pNewOpt )
/*N*/ 		{
/*N*/ 			*pOpt = *pNewOpt;
/*N*/ 			// Der Zoomfaktor muss eingestellt werden, weil in der CTOR-
/*N*/ 			// phase aus Performancegruenden kein ApplyViewOptions gerufen wird.
/*N*/ 			if( GetWin() && 100 != pOpt->GetZoom() )
/*N*/ 			{
/*N*/ 				MapMode aMode( pWin->GetMapMode() );
/*N*/ 				const Fraction aNewFactor( pOpt->GetZoom(), 100 );
/*N*/ 				aMode.SetScaleX( aNewFactor );
/*N*/ 				aMode.SetScaleY( aNewFactor );
/*N*/ 				pWin->SetMapMode( aMode );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDocShell* pDShell = pDoc->GetDocShell();
/*N*/ 	pDoc->SetHTMLMode( 0 != ::binfilter::GetHtmlMode( pDShell ) );
/*N*/ 
/*N*/ 	// JP 02.02.99: Bug 61335 - Readonly-Flag an den ViewOptions setzen,
/*N*/ 	//				bevor das Layout angelegt wird. Ansonsten muesste man
/*N*/ 	//				nochmals durchformatieren!!
/*N*/ 	if( pDShell && pDShell->IsReadOnly() )
/*N*/ 		pOpt->SetReadonly( TRUE );
/*N*/ 
/*N*/ 	RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - before InitPrt" );

    // Setup the printer. This has to be done even if we use the virtual
    // device for formatting, because the default page format is obtained
    // from the printer
/*N*/     if( GetPrt( ! pDoc->IsBrowseMode() ) )
/*N*/ 		InitPrt( GetPrt() );
/*N*/ 
/*N*/ 	RTL_LOGFILE_CONTEXT_TRACE( aLog, "View::Init - after InitPrt" );
/*N*/ 
/*N*/ 	if( GetWin() )
/*N*/ 	{
/*N*/ 		pOpt->Init( GetWin() );
/*N*/ 		GetWin()->SetFillColor();
/*N*/ 		GetWin()->SetBackground();
/*N*/ 		GetWin()->SetLineColor();
/*N*/ 	}
/*N*/ 
/*N*/ 	//Layout erzeugen wenn es noch nicht vorhanden ist.
/*N*/ 	SwRootFrm* pRoot = GetDoc()->GetRootFrm();
/*N*/ 	if( !pRoot )
/*N*/ 		GetDoc()->SetRootFrm( pRoot = new SwRootFrm( pDoc->GetDfltFrmFmt(), this ) );
/*N*/ 
/*N*/ 	SizeChgNotify( pRoot->Frm().SSize() );
/*N*/ }

/******************************************************************************
|*
|*	ViewShell::~ViewShell()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 10. May. 95
|*
******************************************************************************/

/*N*/ ViewShell::~ViewShell()
/*N*/ {
/*N*/ 	{
/*N*/ 		SET_CURR_SHELL( this );
/*N*/ 		bPaintWorks = FALSE;
/*N*/ 
/*N*/ 		//Die Animierten Grafiken abschalten!
/*N*/ 		if( pDoc )
/*N*/ 		{
/*N*/ 			SwNodes& rNds = pDoc->GetNodes();
/*N*/ 			SwGrfNode *pGNd;
/*N*/ 
/*N*/ 			SwStartNode *pStNd;
/*N*/ 			SwNodeIndex aIdx( *rNds.GetEndOfAutotext().StartOfSectionNode(), 1 );
/*N*/ 			while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
/*N*/ 			{
/*N*/ 				aIdx++;
/*N*/ 				if ( 0 != ( pGNd = aIdx.GetNode().GetGrfNode() ) )
/*N*/ 				{
/*N*/ 					if( pGNd->IsAnimated() )
/*N*/ 					{
/*N*/ 						SwClientIter aIter( *pGNd );
/*N*/ 						for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
/*N*/ 							pFrm; pFrm = (SwFrm*)aIter.Next() )
/*N*/ 						{
/*N*/ 							ASSERT( pFrm->IsNoTxtFrm(), "GraphicNode with Text?" );
/*N*/ 							((SwNoTxtFrm*)pFrm)->StopAnimation( pOut );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				aIdx.Assign( *pStNd->EndOfSectionNode(), +1 );
/*N*/ 			}
/*N*/ 
/*N*/ 			GetDoc()->StopNumRuleAnimations( pOut );
/*N*/ 		}
/*N*/ 
/*N*/ 		delete pImp; //Erst loeschen, damit die LayoutViews vernichtet werden.
/*N*/ 		pImp = 0;	// Set to zero, because ~SwFrm relies on it.
/*N*/ 
/*N*/ 		if ( pDoc )
/*N*/ 		{
/*N*/ 			if( !pDoc->RemoveLink() )
/*?*/ 				delete pDoc, pDoc = 0;
/*N*/ 			else
/*N*/ 				pDoc->GetRootFrm()->ResetNewLayout();
/*N*/ 		}
/*N*/ 
/*N*/ 		delete pOpt;
/*N*/ 
/*N*/ 		//Format-Cache zurueckschrauben.
/*N*/ 		if ( SwTxtFrm::GetTxtCache()->GetCurMax() > 250 )
/*N*/ 			SwTxtFrm::GetTxtCache()->DecreaseMax( 100 );
/*N*/ 
/*N*/ 		//Ggf. aus der PaintQueue entfernen lassen
/*N*/ 		SwPaintQueue::Remove( this );
/*N*/ 
/*N*/ 		ASSERT( !nStartAction, "EndAction() pending." );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pDoc )
/*N*/ 		GetLayout()->DeRegisterShell( this );
/*N*/ 
/*N*/     delete mpTmpRef;
/*N*/     delete pAccOptions;
/*N*/ }

/*N*/ BOOL ViewShell::HasDrawView() const
/*N*/ {
/*N*/ 	return Imp()->HasDrawView();
/*N*/ }

/*N*/ void ViewShell::MakeDrawView()
/*N*/ {
/*N*/ 	Imp()->MakeDrawView( );
/*N*/ }

/*N*/ SdrView* ViewShell::GetDrawView()
/*N*/ {
/*N*/ 	return Imp()->GetDrawView();
/*N*/ }

}
