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

#include <viewimp.hxx>
// OD 18.02.2003 #107562# - <SwAlignRect> for <ViewShell::Scroll()>

#include <horiornt.hxx>

#include "crsrsh.hxx"
#include "pagefrm.hxx"
// OD 12.11.2002 #96272# - include declaration for <SetMappingForVirtDev>
namespace binfilter {


/*****************************************************************************
|*
|*	ViewShell::AddScrollRect()
|*
|*	Creation			MA 07. Mar. 94
|*	Last change			AMA 20. July 00
|*
|*	Description
|*  ViewShell::AddScrollRect(..) passes a registration from a scrolling frame or
|*	rectangle to all ViewShells and SwViewImps respectively.
|*
******************************************************************************/

/*N*/ void ViewShell::AddScrollRect( const SwFrm *pFrm, const SwRect &rRect,
/*N*/ 	long nOfs )
/*N*/ {
/*N*/ 	ASSERT( pFrm, "Where is my friend, the frame?" );
/*N*/ 	BOOL bScrollOn = TRUE;
/*N*/ 
/*N*/ #ifdef NOSCROLL
/*N*/ 	//Auch im Product per speziellem Compilat abschaltbar.
/*N*/ 	bScrollOn = FALSE;
/*N*/ #endif
/*N*/ 
/*N*/ 	if( bScrollOn && Imp()->IsScroll() && nOfs <= SHRT_MAX && nOfs >= SHRT_MIN )
/*N*/ 	{
/*N*/ 		ViewShell *pSh = this;
/*N*/ 		do
/*N*/ 		{
/*N*/ 			pSh->Imp()->AddScrollRect( pFrm, rRect, nOfs );
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 		} while ( pSh != this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		AddPaintRect( rRect );
/*N*/ }

/******************************************************************************
|*
|*	ViewShell::Scroll()
|*
|*	Ersterstellung		MA 07. Mar. 94
|*	Last change			AMA 21. July 00
|*
|*  Description
|*  ViewShell::Scroll() scrolls all rectangles in the pScrollRects-list and
|*  transfers the critical lines by calling SwViewImp::MoveScrollArea(..).
|*
******************************************************************************/

/*N*/ void ViewShell::Scroll()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/******************************************************************************
|*
|*	ViewShell::SetNoNextScroll()
|*
|*	Ersterstellung		MA 08. Mar. 94
|*	Letzte Aenderung	MA 08. Mar. 94
|*
******************************************************************************/

/*N*/ void ViewShell::SetNoNextScroll()
/*N*/ {
/*N*/ 	ViewShell *pSh = this;
/*N*/ 	do
/*N*/ 	{	pSh->Imp()->ResetNextScroll();
/*N*/ 		pSh = (ViewShell*)pSh->GetNext();
/*N*/ 
/*N*/ 	} while ( pSh != this );
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::AddScrollRect()
|*
|*	Ersterstellung		MA 07. Mar. 94
|*	Last change			AMA 21. July 00
|*
|*	Adds a scrollable rectangle and his critical lines to the list.
|*
******************************************************************************/

/*N*/ void SwViewImp::AddScrollRect( const SwFrm *pFrm, const SwRect &rRect,
/*N*/ 	long nOffs )
/*N*/ {
/*N*/ 	ASSERT( nOffs != 0, "Scrollen ohne Ofst." );
/*N*/ 	SwRect aRect( rRect );
/*N*/     BOOL bVert = pFrm->IsVertical();
/*N*/     if( bVert )
/*?*/         aRect.Pos().X() += nOffs;
/*N*/     else
/*N*/ 	aRect.Pos().Y() -= nOffs;
/*N*/ 	if( aRect.IsOver( pSh->VisArea() ) )
/*N*/ 	{
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		ASSERT( pSh->GetWin(), "Scrolling without outputdevice" );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		AddPaintRect( rRect );
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::MoveScrollArea()
|*
|*	Creation			AMA 10. July 00
|*	Last change			AMA 21. July 00
|*
|*  Transfers the areas after scrolling to the scrolled list, but only those
|*	parts with critical lines.
|*
******************************************************************************/


/******************************************************************************
|*
|*	SwViewImp::FlushScrolledArea()
|*
|*	Creation			AMA 10. July 00
|*	Last change			AMA 21. July 00
|*
|*  Flushes the scrolled critical lines, that is transfer them to AddPaintRect()
|*  and remove them from the list.
|*
******************************************************************************/


/******************************************************************************
|*
|*	SwViewImp::_FlushScrolledArea(..)
|*
|*	Creation			AMA 10. July 00
|*	Last change			AMA 21. July 00
|*
|*  The critical lines, which overlaps with the given rectangle, will be united
|*  with the rectangle and removed from the list.
|*
******************************************************************************/


/******************************************************************************
|*
|*	SwViewImp::RefreshScrolledHdl(..)
|*
|*	Creation			MA 06. Oct. 94
|*	Last change			AMA 21. July 00
|*
|*  Every timerstop one of the critical lines will be painted.
|*
******************************************************************************/

/*N*/ IMPL_LINK( SwViewImp, RefreshScrolledHdl, Timer *, EMPTYARG )
/*N*/ {
        {DBG_BF_ASSERT(0, "STRIP");} return 0;//STRIP001 	DBG_PROFSTART( RefreshTimer );
/*N*/ }

/******************************************************************************
|*
|*	SwViewImp::_ScrolledRect(..)
|*
|*	Creation			AMA 20. July 00
|*	Last change			AMA 21. July 00
|*
|*  handles the problem of scrolled criticals lines, when they are a part of
|*  a scrolling area again. In this case, their rectangle has to move to the
|*  right position.
|*
******************************************************************************/


/******************************************************************************
|*
|*	SwViewImp::_RefreshScrolledArea()
|*
******************************************************************************/

//Berechnen der Hoehe fuer das virtuelle Device, Breite und maximaler
//Speicherbedarf sind vorgegeben.
#define MAXKB 180L



/******************************************************************************
|*
|*	SwViewImp::RefreshScrolledArea()
|*
|*	Ersterstellung		MA 06. Oct. 94
|*	Letzte Aenderung	MA 19. Apr. 95
|*
******************************************************************************/






/******************************************************************************
|*
|*	SwScrollAreas::Insert(..)
|*
******************************************************************************/




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
