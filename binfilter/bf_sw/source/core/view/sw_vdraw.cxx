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

#ifndef _SVDPAGE_HXX //autogen
#include <bf_svx/svdpage.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <bf_svx/svdpagv.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif

/// OD 29.08.2002 #102450#
/// include <bf_svx/svdoutl.hxx>

#ifdef DBG_UTIL
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "fesh.hxx"
#include "doc.hxx"
#include "viewimp.hxx"
#include "dcontact.hxx"
#include "dview.hxx"
#include "flyfrm.hxx"
namespace binfilter {


/*************************************************************************
|*
|*	SwSaveHdl
|*
|*	Ersterstellung		MA 14. Feb. 95
|*	Letzte Aenderung	MA 02. Jun. 98
|*
|*************************************************************************/




/*************************************************************************
|*
|*	SwViewImp::StartAction(), EndAction()
|*
|*	Ersterstellung		MA 14. Feb. 95
|*	Letzte Aenderung	MA 14. Sep. 98
|*
|*************************************************************************/


/*N*/ void SwViewImp::StartAction()
/*N*/ {
/*N*/ 	if ( HasDrawView() )
/*N*/ 	{
/*N*/ 		SET_CURR_SHELL( GetShell() );
/*N*/ 		if ( pSh->ISA(SwFEShell) )
/*N*/ 			((SwFEShell*)pSh)->HideChainMarker();	//Kann sich geaendert haben
/*N*/ 		bResetXorVisibility = GetDrawView()->IsShownXorVisible( GetShell()->GetOut());
/*N*/ 		GetDrawView()->HideShownXor( GetShell()->GetOut() );
/*N*/ 	}
/*N*/ }



/*N*/ void SwViewImp::EndAction()
/*N*/ {
/*N*/ 	if ( HasDrawView() )
/*N*/ 	{
/*N*/ 		SET_CURR_SHELL( GetShell() );
/*N*/ 		if ( bResetXorVisibility )
/*?*/ 			GetDrawView()->ShowShownXor( GetShell()->GetOut() );
/*N*/ 		if ( pSh->ISA(SwFEShell) )
/*N*/ 			((SwFEShell*)pSh)->SetChainMarker();	//Kann sich geaendert haben
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwViewImp::LockPaint(), UnlockPaint()
|*
|*	Ersterstellung		MA 11. Jun. 96
|*	Letzte Aenderung	MA 11. Jun. 96
|*
|*************************************************************************/







/*************************************************************************
|*
|*	SwViewImp::PaintLayer(), PaintDispatcher()
|*
|*	Ersterstellung		MA 20. Dec. 94
|*	Letzte Aenderung	AMA 04. Jun. 98
|*
|*************************************************************************/
// OD 29.08.2002 #102450#
// add 3rd paramter <const Color* pPageBackgrdColor> for setting this
// color as the background color at the outliner of the draw view.
// OD 09.12.2002 #103045# - add 4th parameter for the horizontal text direction
// of the page in order to set the default horizontal text direction at the
// outliner of the draw view for painting layers <hell> and <heaven>.
// OD 25.06.2003 #108784# - correct type of 1st parameter



/*************************************************************************
|*
|*	SwViewImp::PaintFlyChilds()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	MA 02. Aug. 95
|*
|*************************************************************************/



/*************************************************************************
|*
|*	SwViewImp::IsDragPossible()
|*
|*	Ersterstellung		MA 19. Jan. 93
|*	Letzte Aenderung	MA 16. Jan. 95
|*
|*************************************************************************/



#define WIEDUWILLST 400


/*************************************************************************
|*
|*	SwViewImp::NotifySizeChg()
|*
|*	Ersterstellung		MA 23. Jun. 93
|*	Letzte Aenderung	MA 05. Oct. 98
|*
|*************************************************************************/

/*N*/ void SwViewImp::NotifySizeChg( const Size &rNewSz )
/*N*/ {
/*N*/ 	if ( !HasDrawView() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	if ( GetPageView() )
/*N*/ 		GetPageView()->GetPage()->SetSize( rNewSz );
/*N*/ 
/*N*/ 	//Begrenzung des Arbeitsbereiches.
/*N*/ 	Rectangle aRect( Point( DOCUMENTBORDER, DOCUMENTBORDER ), rNewSz );
/*N*/ 	const Rectangle &rOldWork = GetDrawView()->GetWorkArea();
/*N*/ 	BOOL bCheckDrawObjs = FALSE;
/*N*/ 	if ( aRect != rOldWork )
/*N*/ 	{
/*N*/ 		if ( rOldWork.Bottom() > aRect.Bottom() || rOldWork.Right() > aRect.Right())
/*N*/ 			bCheckDrawObjs = TRUE;
/*N*/ 		GetDrawView()->SetWorkArea( aRect );
/*N*/ 	}
/*N*/ 	if ( !bCheckDrawObjs )
/*N*/ 		return;
/*N*/ 
/*N*/ 	ASSERT( pSh->GetDoc()->GetDrawModel(), "NotifySizeChg without DrawModel" );
/*N*/ 	SdrPage* pPage = pSh->GetDoc()->GetDrawModel()->GetPage( 0 );
/*N*/ 	const ULONG nObjs = pPage->GetObjCount();
/*N*/ 	for( ULONG nObj = 0; nObj < nObjs; ++nObj )
/*N*/ 	{
/*N*/ 		SdrObject *pObj = pPage->GetObj( nObj );
/*N*/ 		if( !pObj->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			//Teilfix(26793): Objekte, die in Rahmen verankert sind, brauchen
/*N*/ 			//nicht angepasst werden.
/*N*/             const SwContact *pCont = (SwContact*)GetUserCall(pObj);
/*N*/ 			//JP - 16.3.00 Bug 73920: this function might be called by the
/*N*/ 			//				InsertDocument, when a PageDesc-Attribute is
/*N*/ 			//				set on a node. Then the SdrObject must not have
/*N*/ 			//				an UserCall.
/*N*/             if( !pCont || !pCont->ISA(SwDrawContact) )
/*N*/ 				continue;
/*N*/ 
/*N*/             const SwFrm *pAnchor = ((SwDrawContact*)pCont)->GetAnchor();
/*N*/ 			if ( !pAnchor || pAnchor->IsInFly() || !pAnchor->IsValid() ||
/*N*/                  !pAnchor->GetUpper() || !pAnchor->FindPageFrm() ||
/*N*/ 				 FLY_IN_CNTNT == pCont->GetFmt()->GetAnchor().GetAnchorId() )
/*N*/ 				continue;
/*N*/ 
/*N*/             // OD 19.06.2003 #108784# - no move for drawing objects in header/footer
/*N*/             if ( pAnchor->FindFooterOrHeader() )
/*N*/             {
/*N*/                 continue;
/*N*/             }
/*N*/ 
/*N*/ 			const Rectangle aBound( pObj->GetBoundRect() );
/*N*/ 			if ( !aRect.IsInside( aBound ) )
/*N*/ 			{
/*N*/ 				Size aSz;
/*N*/ 				if ( aBound.Left() > aRect.Right() )
/*N*/ 					aSz.Width() = (aRect.Right() - aBound.Left()) - MINFLY;
/*N*/ 				if ( aBound.Top() > aRect.Bottom() )
/*?*/ 					aSz.Height() = (aRect.Bottom() - aBound.Top()) - MINFLY;
/*N*/ 				if ( aSz.Width() || aSz.Height() )
/*N*/ 					pObj->Move( aSz );
/*N*/ 
/*N*/ 				//Notanker: Grosse Objekte nicht nach oben verschwinden lassen.
/*N*/ 				aSz.Width() = aSz.Height() = 0;
/*N*/ 				if ( aBound.Bottom() < aRect.Top() )
/*N*/ 					aSz.Width() = (aBound.Bottom() - aRect.Top()) - MINFLY;
/*N*/ 				if ( aBound.Right() < aRect.Left() )
/*N*/ 					aSz.Height() = (aBound.Right() - aRect.Left()) - MINFLY;
/*N*/ 				if ( aSz.Width() || aSz.Height() )
/*N*/ 					pObj->Move( aSz );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }



}
