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


#include <tools/debug.hxx>
#include "swregion.hxx"
namespace binfilter {


/*N*/ SV_IMPL_VARARR( SwRects, SwRect );

/*************************************************************************
|*
|*	SwRegionRects::SwRegionRects()
|*
|*	Ersterstellung		MA 28. Oct. 92
|*	Letzte Aenderung	MA 01. Feb. 93
|*
|*************************************************************************/

/*N*/ SwRegionRects::SwRegionRects( const SwRect &rStartRect, USHORT nInit,
/*N*/ 														USHORT nGrow ) :
/*N*/ 	SwRects( (BYTE)nInit, (BYTE)nGrow ),
/*N*/ 	aOrigin( rStartRect )
/*N*/ {
/*N*/ 	Insert( aOrigin, 0 );
/*N*/ }

/*************************************************************************
 *						inline InsertRect()
 *
 * InsertRect() wird nur von operator-=() gerufen.
 * Wenn bDel == TRUE ist, dann wird das Rect an der Position nPos mit
 * rRect ueberschrieben, ansonsten wird rRect hinten angehaengt.
 *************************************************************************/

/*N*/  inline void SwRegionRects::InsertRect( const SwRect &rRect, const USHORT nPos,
/*N*/  									   FASTBOOL &rDel )
/*N*/  {
/*N*/  	if( rDel )
/*N*/  	{
/*N*/  		*(pData+nPos) = rRect;
/*N*/  		rDel = FALSE;
/*N*/  	}
/*N*/  	else
/*N*/  		Insert( rRect, Count() );
/*N*/  }

/*************************************************************************
|*
|*	SwRegionRects::operator-=()
|*
|*	Beschreibung		Alle Ueberschneidungen der Rechtecke, die sich
|*		gerade im Array befinden, mit dem uebergebenen Rechteck werden
|*		entfernt.
|*		Dazu muessen die vorhandenen Rechtecke entweder aufgeteilt oder
|*		geloescht werden.
|*	Ersterstellung		MA 28. Oct. 92
|*	Letzte Aenderung	MA 09. Sep. 93
|*
|*************************************************************************/

/*N*/ void SwRegionRects::operator-=( const SwRect &rRect )
/*N*/ {
/*N*/ 	USHORT nMax = Count();
/*N*/ 	for ( USHORT i = 0; i < nMax; ++i )
/*N*/ 	{
/*N*/ 		if ( rRect.IsOver( *(pData+i) ) )
/*N*/ 		{
/*N*/ 			SwRect aTmp( *(pData+i) );
/*N*/ 			SwRect aInter( aTmp );
/*N*/ 			aInter._Intersection( rRect );
/*N*/ 
/*N*/ 			// Das erste Rect, das wir inserten wollen, nimmt die
/*N*/ 			// Stelle von i ein. So ersparen wir uns das Delete().
/*N*/ 			FASTBOOL bDel = TRUE;
/*N*/ 
/*N*/ 			//Jetzt aufteilen das Teil: Es sollen diejenigen Rechtecke
/*N*/ 			//zurueckbleiben, die im alten aber nicht im neuen liegen.
/*N*/ 			//Sprich alle Rechtecke die im alten aber nicht in der Intersection
/*N*/ 			//liegen.
/*N*/ 			long nTmp;
/*N*/ 			if ( 0 < (nTmp = aInter.Top() - aTmp.Top()) )
/*N*/ 			{
/*N*/ 				const long nOldVal = aTmp.Height();
/*N*/ 				aTmp.SSize().Height() = nTmp;
/*N*/ 				InsertRect( aTmp, i, bDel );
/*N*/ 				aTmp.Height( nOldVal );
/*N*/ 			}
/*N*/ 
/*N*/ 			aTmp.Top( aInter.Top() + aInter.Height() );
/*N*/ 			if ( aTmp.Height() > 0 )
/*N*/ 				InsertRect( aTmp, i, bDel );
/*N*/ 
/*N*/ 			aTmp.Top( aInter.Top() );
/*N*/ 			aTmp.Bottom( aInter.Bottom() );
/*N*/ 			if ( 0 < (nTmp = aInter.Left() - aTmp.Left()) )
/*N*/ 			{
/*N*/ 				const long nOldVal = aTmp.Width();
/*N*/ 				aTmp.Width( nTmp );
/*N*/ 				InsertRect( aTmp, i, bDel );
/*N*/ 				aTmp.Width( nOldVal );
/*N*/ 			}
/*N*/ 
/*N*/ 			aTmp.Left( aInter.Left() + aInter.Width() ); //+1?
/*N*/ 			if ( aTmp.Width() > 0 )
/*N*/ 				InsertRect( aTmp, i, bDel );
/*N*/ 
/*N*/ 			if( bDel )
/*N*/ 			{
/*N*/ 				Remove( i );
/*N*/ 				--i;			  //Damit wir keinen uebergehen.
/*N*/ 				--nMax; 		  //Damit wir keinen zuviel verarbeiten.
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ }

/*************************************************************************
 *						SwRegionRects::Invert()
 *
 * Bezugspunkt ist aOrigin, das Original-SRectangle.
 * Aus Loechern werden Flaechen, aus Flaechen werden Loecher.
 * Ein Hinweis: Wenn keine Rects abgezogen wurden, so ist das enthaltene
 * Rechteck identisch mit aOrigin. Nach Invert() besteht die Region aus
 * einem Null-SRectangle.
 *************************************************************************/

/*************************************************************************
|*
|*	SwRegionRects::Compress()
|*
|*	Beschreibung		Zusammenfassen von benachbarten Rechtecken.
|*	Ersterstellung		MA 16. Apr. 93
|*	Letzte Aenderung	MA 21. Apr. 93
|*
|*************************************************************************/


#ifdef _MSC_VER
#pragma optimize("e",off)
#endif
#ifdef _MSC_VER
#pragma optimize("",on)
#endif


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
