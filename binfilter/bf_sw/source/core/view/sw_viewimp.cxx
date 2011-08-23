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

#include "scrrect.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"
#include "crsrsh.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "layact.hxx"
#include "swregion.hxx"
#include "dview.hxx"
#ifndef _SVDPAGE_HXX //autogen
#include <bf_svx/svdpage.hxx>
#endif
#ifdef ACCESSIBLE_LAYOUT
#endif

// OD 12.12.2002 #103492#
namespace binfilter {

/*************************************************************************
|*
|*	SwViewImp::Init()
|*
|*	Ersterstellung		MA 25. Jul. 94
|*	Letzte Aenderung	MA 03. Nov. 95
|*
|*************************************************************************/

/*N*/ void SwViewImp::Init( const SwViewOption *pNewOpt )
/*N*/ {
        DBG_ERROR("Strip!");
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::~SwViewImp()
|*
|*	Ersterstellung		MA 25. Jul. 94
|*	Letzte Aenderung	MA 16. Dec. 94
|*
******************************************************************************/

/*N*/ SwViewImp::~SwViewImp()
/*N*/ {
#if 0
/*N*/ 
/*N*/     // OD 12.12.2002 #103492#
/*N*/ 
/*N*/ 	//JP 29.03.96: nach ShowPage muss auch HidePage gemacht werden!!!
/*N*/ 	if( pDrawView )
/*N*/  		pDrawView->HidePage( pSdrPageView );
/*N*/ 
/*N*/ 	delete pDrawView;
/*N*/ 
/*N*/ 	DelRegions();
/*N*/ 	delete pScrolledArea;
/*N*/ 
/*N*/ 	ASSERT( !pLayAct, "Have action for the rest of your life." );
/*N*/ 	ASSERT( !pIdleAct,"Be idle for the rest of your life." );
#endif
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::DelRegions()
|*
|*	Ersterstellung		MA 14. Apr. 94
|*	Letzte Aenderung	MA 14. Apr. 94
|*
******************************************************************************/

/*N*/ void SwViewImp::DelRegions()
/*N*/ {
/*N*/ 	DELETEZ(pRegion);
/*N*/ 	DELETEZ(pScrollRects);
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::AddPaintRect()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 27. Jul. 94
|*
******************************************************************************/

/*N*/ BOOL SwViewImp::AddPaintRect( const SwRect &rRect )
/*N*/ {
/*N*/ 	if ( rRect.IsOver( pSh->VisArea() ) )
/*N*/ 	{
/*?*/ 		if ( !pRegion )
/*?*/ 			pRegion = new SwRegionRects( pSh->VisArea() );
/*?*/ 		(*pRegion) -= rRect;
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }


/******************************************************************************
|*
|*	ViewImp::CheckWaitCrsr()
|*
|*	Ersterstellung		MA 10. Aug. 94
|*	Letzte Aenderung	MA 10. Aug. 94
|*
******************************************************************************/

/*N*/ void SwViewImp::CheckWaitCrsr()
/*N*/ {
/*N*/ 	if ( pLayAct )
/*N*/ 		pLayAct->CheckWaitCrsr();
/*N*/ }

/******************************************************************************
|*
|*	ViewImp::IsCalcLayoutProgress()
|*
|*	Ersterstellung		MA 12. Aug. 94
|*	Letzte Aenderung	MA 12. Aug. 94
|*
******************************************************************************/

/*N*/ BOOL SwViewImp::IsCalcLayoutProgress() const
/*N*/ {
/*N*/ 	if ( pLayAct )
/*N*/ 		return pLayAct->IsCalcLayout();
/*N*/ 	return FALSE;
/*N*/ }

/******************************************************************************
|*
|*	ViewImp::IsUpdateExpFlds()
|*
|*	Ersterstellung		MA 28. Mar. 96
|*	Letzte Aenderung	MA 28. Mar. 96
|*
******************************************************************************/

/*N*/ BOOL SwViewImp::IsUpdateExpFlds()
/*N*/ {
/*N*/ 	if ( pLayAct && pLayAct->IsCalcLayout() )
/*N*/ 	{
/*?*/ 		pLayAct->SetUpdateExpFlds();
/*?*/ 		return TRUE;
/*N*/ 	}
/*N*/  	return FALSE;
/*N*/ }


/******************************************************************************
|*
|*	SwViewImp::SetFirstVisPage(), ImplGetFirstVisPage();
|*
|*	Ersterstellung		MA 21. Sep. 93
|*	Letzte Aenderung	MA 08. Mar. 94
|*
******************************************************************************/

/*N*/ void SwViewImp::SetFirstVisPage()
/*N*/ {
/*N*/ 	if ( pSh->bDocSizeChgd && pSh->VisArea().Top() > pSh->GetLayout()->Frm().Height() )
/*N*/ 	{
/*N*/ 		//Wir stecken in einer Action und die VisArea sitzt wegen
/*N*/ 		//Loeschoperationen hinter der erste sichtbaren Seite.
/*N*/ 		//Damit nicht zu heftig Formatiert wird, liefern wir die letzte Seite
/*N*/ 		//zurueck.
/*?*/ 		pFirstVisPage = (SwPageFrm*)pSh->GetLayout()->Lower();
/*?*/ 		while ( pFirstVisPage && pFirstVisPage->GetNext() )
/*?*/ 			pFirstVisPage = (SwPageFrm*)pFirstVisPage->GetNext();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = (SwPageFrm*)pSh->GetLayout()->Lower();
/*N*/ 		while ( pPage && !pPage->Frm().IsOver( pSh->VisArea() ) )
/*N*/ 			pPage = (SwPageFrm*)pPage->GetNext();
/*N*/ 		pFirstVisPage = pPage ? pPage : (SwPageFrm*)pSh->GetLayout()->Lower();
/*N*/ 	}
/*N*/ 	bFirstPageInvalid = FALSE;
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::MakeDrawView();
|*
|*	Ersterstellung		AMA 01. Nov. 95
|*	Letzte Aenderung	AMA 01. Nov. 95
|*
******************************************************************************/

/*N*/ void SwViewImp::MakeDrawView()
/*N*/ {
#if 0
/*N*/ 	if( !GetShell()->GetDoc()->GetDrawModel() )
/*?*/ 		GetShell()->GetDoc()->_MakeDrawModel();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( !pDrawView )
/*N*/ 		{
/*N*/ 			pDrawView =	new SwDrawView( *this,
/*N*/ 						GetShell()->GetDoc()->GetDrawModel(),
/*N*/    						GetShell()->GetWin() ?
/*N*/ 							GetShell()->GetWin() :
/*N*/ 							(OutputDevice*)GetShell()->GetDoc()->GetPrt() );
/*N*/ 		}
/*N*/ 		GetDrawView()->SetActiveLayer( XubString::CreateFromAscii(
/*N*/ 							RTL_CONSTASCII_STRINGPARAM( "Heaven" ) ) );
/*N*/ 		Init( GetShell()->GetViewOptions() );
/*N*/ 	}
#endif
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::GetRetoucheColor()
|*
|*	Ersterstellung		MA 24. Jun. 98
|*	Letzte Aenderung	MA 24. Jun. 98
|*
******************************************************************************/


/** create page preview layout

    OD 12.12.2002 #103492#

    @author OD
*/


/*N*/ void SwViewImp::DisposeAccessible( const SwFrm *pFrm,
/*N*/ 								   const SdrObject *pObj,
/*N*/ 								   sal_Bool bRecursive )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void SwViewImp::MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
/*N*/ 								const SwRect& rOldFrm )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void SwViewImp::InvalidateAccessibleFrmContent( const SwFrm *pFrm )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

// OD 15.01.2003 #103492# - method signature change due to new page preview functionality
}
