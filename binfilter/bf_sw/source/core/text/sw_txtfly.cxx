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


#include <horiornt.hxx>

#include "doc.hxx"
#include "viewsh.hxx"
#include "rootfrm.hxx"
#include "pam.hxx"			// SwPosition
#include "dcontact.hxx"		// SwContact
#include "dflyobj.hxx"		// SdrObject
#include "frmtool.hxx"	  // ::DrawGraphic
#include <pormulti.hxx> 	// SwMultiPortion

#ifdef VERT_DISTANCE
#include <math.h>
#endif


#include <bf_svx/obj3d.hxx>

#include <bf_svx/txtrange.hxx>

#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <txtflcnt.hxx>
#include <fmtsrnd.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>




#include <pagefrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>

// #102344#
#include <bf_svx/svdoedge.hxx>

#include "itrform2.hxx"   // SwTxtFormatter
#include "porfly.hxx"	  // NewFlyCntPortion
#include "porfld.hxx"	  // SwGrfNumPortion


#include "flyfrms.hxx"
#include "fmtcnct.hxx"  // SwFmtChain

#ifdef DBG_UTIL
#include "viewopt.hxx"	// SwViewOptions, nur zum Testen (Test2)
#endif
namespace binfilter {

/*****************************************************************************
 * Beschreibung:
 * Die Klasse SwTxtFly soll die Universalschnittstelle zwischen der
 * Formatierung/Textausgabe und den u.U. ueberlappenden freifliegenden
 * Frames sein.
 * Waehrend der Formatierung erkundigt sich der Formatierer beim SwTxtFly,
 * ob ein bestimmter Bereich durch die Attribute eines ueberlappenden
 * Frames vorliegt. Solche Bereiche werden in Form von Dummy-Portions
 * abgebildet.
 * Die gesamte Textausgabe und Retusche wird ebenfalls an ein SwTxtFly
 * weitergeleitet. Dieser entscheidet, ob Textteile geclippt werden muessen
 * und zerteilt z.B. die Bereiche bei einem DrawRect.
 * Zu beachten ist, dass alle freifliegenden Frames in einem nach TopLeft
 * sortiertem PtrArray an der Seite zu finden sind. Intern wird immer nur
 * in dokumentglobalen Werten gerechnet. Die IN- und OUT-Parameter sind
 * jedoch in den meisten Faellen an die Beduerfnisse des LineIters
 * zugeschnitten, d.h. sie werden in frame- oder windowlokalen Koordinaten
 * konvertiert.
 * Wenn mehrere Frames mit Umlaufattributen in einer Zeile liegen,
 * ergeben sich unterschiedliche Auswirkungen fuer den Textfluss:
 *
 *		L/R    P	 L	   R	 K
 *		 P	 -P-P- -P-L  -P R- -P K
 *		 L	 -L P- -L L  -L R- -L K
 *		 R	  R-P-	R-L   R R-	R K
 *		 K	  K P-	K L   K R-	K K
 *
 * (P=parallel, L=links, R=rechts, K=kein Umlauf)
 *
 * Das Verhalten so beschreiben:
 * Jeder Rahmen kann Text verdraengen, wobei der Einfluss allerdings nur
 * bis zum naechsten Rahmen reicht.
 *****************************************************************************/

/*****************************************************************************
 *
 * lcl_TheAnchor liefert den SwFrm, an dem das Objekt verankert ist.
 *
 *****************************************************************************/

// OD 03.07.2003 #108784# - change return type from <pointer> to <reference>
/*N*/ const SwFrm& lcl_TheAnchor( const SdrObject* pObj )
/*N*/ {
/*N*/     // OD 03.07.2003 #108784# - adjustments for support of drawing objects in
/*N*/     // header/footer.
/*N*/     const SwFrm* pRet = 0L;
/*N*/     if ( pObj->IsWriterFlyFrame() )
/*N*/     {
/*N*/         pRet = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm()->GetAnchor();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         SwDrawContact* pDrawContact =
/*N*/                 static_cast<SwDrawContact*>(GetUserCall( pObj ));
/*N*/         if ( pObj->ISA(SwDrawVirtObj) )
/*N*/         {
/*N*/             const SwDrawVirtObj* pDrawVirtObj = static_cast<const SwDrawVirtObj*>(pObj);
/*N*/             pRet = pDrawVirtObj->GetAnchorFrm();
/*N*/ 
/*N*/             // error handling, if no anchor frame is found.
/*N*/             if ( !pRet )
/*N*/             {
/*N*/                 // assert, if no anchor frame found at 'virtual' drawing object
/*N*/                 // and return anchor frame of 'master' drawing object.
/*N*/                 ASSERT( false, "<lcl_TheAnchor(..)> - virtual drawing object with no anchor frame!" );
/*N*/                 pRet = pDrawContact->GetAnchor();
/*N*/             }
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             pRet = pDrawContact->GetAnchor();
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     ASSERT( pRet, "<lcl_TheAnchor(..)> - no anchor frame found!" );
/*N*/ 
/*N*/     return *pRet;
/*N*/ }

/*****************************************************************************
 * lcl_MaxAscDescent liefert die max. Ascents und Descents in der Zeile ohne
 * FlyPortions (abs. und seitengeb. Objekte), einmal mit und einmal ohne
 * Beruecksichtigung der zeichengeb. Objekte.
 * Diese Werte sind fuer das SetBase der zeichengebundenen Objekte wichtig,
 * wenn diese an den Zeichen oder an der Zeile ausgerichtet werden sollen.
 *****************************************************************************/

/*N*/ void lcl_MaxAscDescent( SwLinePortion* pPos, long &rAscent, long &rDescent,
/*N*/ 	 long &rFlyAscent, long &rFlyDescent, SwLinePortion* pNot = NULL )
/*N*/ {
/*N*/ 	rAscent = 0;
/*N*/ 	rDescent = 0;
/*N*/ 	rFlyAscent = 0;
/*N*/ 	rFlyDescent = 0;
/*N*/ 
/*N*/ 	if( !pPos->GetLen() && ( pPos->IsParaPortion() || pPos->IsLayPortion() ) )
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 
/*N*/ 	while ( pPos )
/*N*/ 	{
/*N*/ 		if( !pPos->IsBreakPortion() && !pPos->IsFlyPortion() )
/*N*/ 		{
/*N*/ 			sal_Bool bFlyCmp = pPos->IsFlyCntPortion() ?
/*N*/ 						   ((SwFlyCntPortion*)pPos)->IsMax() :	pPos != pNot;
/*N*/ 			if( bFlyCmp )
/*N*/ 			{
/*N*/ 				rFlyAscent = Max( rFlyAscent, (long)pPos->GetAscent() );
/*N*/ 				rFlyDescent = Max( rFlyDescent,
/*N*/ 						(long)( pPos->Height() - pPos->GetAscent() ) );
/*N*/ 			}
/*N*/ 			if( !pPos->IsFlyCntPortion() && !pPos->IsGrfNumPortion() )
/*N*/ 			{
/*N*/ 				rAscent = Max( rAscent, (long)pPos->GetAscent() );
/*N*/ 				rDescent = Max( rDescent,
/*N*/ 					(long)( pPos->Height() - pPos->GetAscent() ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ }

/*N*/ void SwTxtFormatter::CalcUnclipped( SwTwips& rTop, SwTwips& rBottom )
/*N*/ {
/*N*/     ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
/*N*/             "SwTxtFormatter::CalcUnclipped with unswapped frame" )
/*N*/ 
/*N*/ 	long nFlyAsc, nFlyDesc;
/*N*/ 	lcl_MaxAscDescent( pCurr, rTop, rBottom, nFlyAsc, nFlyDesc );
/*N*/ 	rTop = Y() + GetCurr()->GetAscent();
/*N*/ 	rBottom = rTop + nFlyDesc;
/*N*/ 	rTop -= nFlyAsc;
/*N*/ }

/*************************************************************************
 * SwTxtFormatter::UpdatePos() aktualisiert die Referenzpunkte der zeichengeb.
 * Objekte, z. B. nach Adjustierung ( rechtsbuendig, Blocksatz etc. )
 * ( hauptsaechlich Korrrektur der X-Position )
 *************************************************************************/

/*N*/ void SwTxtFormatter::UpdatePos( SwLineLayout *pCurr, Point aStart,
/*N*/ 	xub_StrLen nStartIdx, sal_Bool bAllWays ) const
/*N*/ {
/*N*/     ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
/*N*/             "SwTxtFormatter::UpdatePos with unswapped frame" )
/*N*/ 
/*N*/     if( GetInfo().IsTest() )
/*N*/ 		return;
/*N*/ 	SwLinePortion *pFirst = pCurr->GetFirstPortion();
/*N*/ 	SwLinePortion *pPos = pFirst;
/*N*/ 	SwTxtPaintInfo aTmpInf( GetInfo() );
/*N*/ 	aTmpInf.SetSpaceAdd( pCurr->GetpSpaceAdd() );
/*N*/ 	aTmpInf.ResetSpaceIdx();
/*N*/     aTmpInf.SetKanaComp( pCurr->GetpKanaComp() );
/*N*/     aTmpInf.ResetKanaIdx();
/*N*/ 
/*N*/ 	// Die Groesse des Frames
/*N*/ 	aTmpInf.SetIdx( nStartIdx );
/*N*/ 	aTmpInf.SetPos( aStart );
/*N*/ 
/*N*/ 	long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
/*N*/ 	lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
/*N*/ 	KSHORT nTmpHeight = pCurr->GetRealHeight();
/*N*/ 	KSHORT nAscent = pCurr->GetAscent() + nTmpHeight - pCurr->Height();
/*N*/ 	sal_uInt8 nFlags = SETBASE_ULSPACE;
/*N*/ 	if( GetMulti() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 	{
/*N*/ 	else
/*N*/ 		aTmpInf.Y( aTmpInf.Y() + nAscent );
/*N*/ 
/*N*/ 	while( pPos )
/*N*/ 	{
/*N*/ 		// bislang ist mir nur ein Fall bekannt, wo die Positionsaenderung
/*N*/ 		// (verursacht durch das Adjustment) fuer eine Portion wichtig
/*N*/ 		// sein koennte: Bei FlyCntPortions muss ein SetRefPoint erfolgen.
/*N*/ 		if( ( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
/*N*/ 			&& ( bAllWays || !IsQuick() ) )
/*N*/ 		{
/*N*/ 			lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
/*N*/ 							   nFlyAsc, nFlyDesc, pPos );
/*N*/ 			if( pPos->IsGrfNumPortion() )
/*N*/ 			{
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 				if( !nFlyAsc && !nFlyDesc )
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/                 Point aBase( aTmpInf.GetPos() );
/*N*/                 if ( GetInfo().GetTxtFrm()->IsVertical() )
/*N*/                     GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aBase );
/*N*/ 
/*N*/                 ((SwFlyCntPortion*)pPos)->SetBase( *aTmpInf.GetTxtFrm(),
/*N*/                     aBase, nTmpAscent, nTmpDescent, nFlyAsc,
/*N*/                     nFlyDesc, nFlags );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasFlyInCntnt() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 		{
/*N*/ 		pPos->Move( aTmpInf );
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ }

/*************************************************************************
 * SwTxtFormatter::AlignFlyInCntBase()
 * richtet die zeichengeb. Objekte in Y-Richtung ggf. neu aus.
 *************************************************************************/

/*N*/ void SwTxtFormatter::AlignFlyInCntBase( long nBaseLine ) const
/*N*/ {
/*N*/     ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
/*N*/             "SwTxtFormatter::AlignFlyInCntBase with unswapped frame" )
/*N*/ 
/*N*/ 	if( GetInfo().IsTest() )
/*N*/ 		return;
/*N*/ 	SwLinePortion *pFirst = pCurr->GetFirstPortion();
/*N*/ 	SwLinePortion *pPos = pFirst;
/*N*/ 	sal_uInt8 nFlags = SETBASE_NOFLAG;
/*N*/ 	if( GetMulti() && GetMulti()->HasRotation() )
/*N*/ 	{
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		nFlags |= SETBASE_ROTATE;
/*N*/ 	}
/*N*/ 
/*N*/ 	long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
/*N*/ 
/*N*/ 	while( pPos )
/*N*/ 	{
/*N*/ 		if( pPos->IsFlyCntPortion() || pPos->IsGrfNumPortion() )
/*N*/ 		{
/*N*/ 			lcl_MaxAscDescent( pFirst, nTmpAscent, nTmpDescent,
/*N*/ 							   nFlyAsc, nFlyDesc, pPos );
/*N*/ 			if( pPos->IsGrfNumPortion() )
/*N*/ 				((SwGrfNumPortion*)pPos)->SetBase( nTmpAscent, nTmpDescent,
/*N*/ 												   nFlyAsc, nFlyDesc );
/*N*/ 			else
/*N*/ 			{
/*N*/                 Point aBase;
/*N*/                 if ( GetInfo().GetTxtFrm()->IsVertical() )
/*N*/                 {
                        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                     nBaseLine = GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( nBaseLine );
/*N*/                 }
/*N*/                 else
/*N*/                     aBase = Point( ((SwFlyCntPortion*)pPos)->GetRefPoint().X(), nBaseLine );
/*N*/ 
/*N*/                 ((SwFlyCntPortion*)pPos)->SetBase( *GetInfo().GetTxtFrm(), aBase, nTmpAscent, nTmpDescent,
/*N*/ 					nFlyAsc, nFlyDesc, nFlags );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *                      SwTxtFly::ChkFlyUnderflow()
 * This is called after the real height of the line has been calculated
 * Therefore it is possible, that more flys from below intersect with the
 * line, or that flys from above do not intersect with the line anymore
 * We check this and return true if so, meaning that the line has to be
 * formatted again
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatter::ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/     ASSERT( rInf.GetTxtFly()->IsOn(), "SwTxtFormatter::ChkFlyUnderflow: why?" );
/*N*/ 	if( GetCurr() )
/*N*/ 	{
/*N*/ 		// Erst pruefen wir, ob ueberhaupt ein Fly mit der Zeile ueberlappt.
/*N*/         // = GetLineHeight()
/*N*/         const long nHeight = GetCurr()->GetRealHeight();
/*N*/ 		SwRect aLine( GetLeftMargin(), Y(), rInf.RealWidth(), nHeight );
/*N*/ 
/*N*/         SwRect aLineVert( aLine );
/*N*/         if ( pFrm->IsVertical() )
/*?*/             pFrm->SwitchHorizontalToVertical( aLineVert );
/*N*/         SwRect aInter( rInf.GetTxtFly()->GetFrm( aLineVert ) );
/*N*/         if ( pFrm->IsVertical() )
                //STRIP001 /*?*/             pFrm->SwitchVerticalToHorizontal( aInter );
/*N*/ 
/*N*/ 		if( !aInter.HasArea() )
/*N*/ 			return sal_False;
/*N*/ 
/*N*/ 		// Nun ueberpruefen wir jede Portion, die sich haette senken koennen,
/*N*/ 		// ob sie mit dem Fly ueberlappt.
/*N*/ 		const SwLinePortion *pPos = GetCurr()->GetFirstPortion();
/*N*/         aLine.Pos().Y() = Y() + GetCurr()->GetRealHeight() - GetCurr()->Height();
/*N*/         aLine.Height( GetCurr()->Height() );
/*N*/ 
/*N*/ 		while( pPos )
/*N*/ 		{
/*N*/             aLine.Width( pPos->Width() );
/*N*/ 
/*N*/             aLineVert = aLine;
/*N*/             if ( pFrm->IsVertical() )
/*?*/                 pFrm->SwitchHorizontalToVertical( aLineVert );
/*N*/             aInter = rInf.GetTxtFly()->GetFrm( aLineVert );
/*N*/             if ( pFrm->IsVertical() )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 pFrm->SwitchVerticalToHorizontal( aInter );
/*N*/ 
/*N*/             // new flys from below?
/*N*/ 			if( !pPos->IsFlyPortion() )
/*N*/ 			{
/*N*/ 				if( aInter.IsOver( aLine ) )
/*N*/ 				{
/*N*/ 					aInter._Intersection( aLine );
/*N*/ 					if( aInter.HasArea() )
/*N*/ 					{
/*N*/                         // to be evaluated during reformat of this line:
/*N*/                         // RealHeight including spacing
/*N*/ 						rInf.SetLineHeight( KSHORT(nHeight) );
/*N*/                         // Height without extra spacing
/*N*/                         rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
/*N*/ 						return sal_True;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/             else
/*N*/             {
/*N*/                 // the fly portion is not anylonger intersected by a fly
/*N*/                 if ( ! aInter.IsOver( aLine ) )
/*N*/                 {
/*?*/                     rInf.SetLineHeight( KSHORT(nHeight) );
/*?*/                     rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
/*?*/                     return sal_True;
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/ 					aInter._Intersection( aLine );
/*N*/ 
/*N*/                     // no area means a fly has become invalid because of
/*N*/                     // lowering the line => reformat the line
/*N*/                     // we also have to reformat the line, if the fly size
/*N*/                     // differs from the intersection intervals size
/*N*/                     if( ! aInter.HasArea() ||
/*N*/                         ((SwFlyPortion*)pPos)->GetFixWidth() != aInter.Width() )
/*N*/ 					{
/*N*/ 						rInf.SetLineHeight( KSHORT(nHeight) );
/*N*/                         rInf.SetLineNettoHeight( KSHORT( pCurr->Height() ) );
/*N*/ 						return sal_True;
/*N*/ 					}
/*N*/ 				}
/*N*/             }
/*N*/ 
/*N*/ 			aLine.Left( aLine.Left() + pPos->Width() );
/*N*/ 			pPos = pPos->GetPortion();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 * SwTxtFormatter::CalcFlyWidth()
 * ermittelt das naechste Objekt, das in die restliche Zeile ragt und
 * konstruiert die zugehoerige FlyPortion.
 * Dazu wird SwTxtFly.GetFrm(..) benutzt.
 *************************************************************************/

// Durch Flys kann sich der rechte Rand verkuerzen.

/*N*/ void SwTxtFormatter::CalcFlyWidth( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/     if( GetMulti() || rInf.GetFly() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	SwTxtFly *pTxtFly = rInf.GetTxtFly();
/*N*/ 	if( !pTxtFly->IsOn() || rInf.IsIgnoreFly() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	register const SwLinePortion *pLast = rInf.GetLast();
/*N*/ 
/*N*/ 	long nAscent;
/*N*/     long nTop = Y();
/*N*/     long nHeight;
/*N*/ 
/*N*/     if( rInf.GetLineHeight() )
/*N*/     {
/*N*/         // real line height has already been calculated, we only have to
/*N*/         // search for intersections in the lower part of the strip
/*N*/         nAscent = pCurr->GetAscent();
/*N*/         nHeight = rInf.GetLineNettoHeight();
/*N*/         nTop += rInf.GetLineHeight() - nHeight;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         nAscent = pLast->GetAscent();
/*N*/         nHeight = pLast->Height();
/*N*/ 
/*N*/         // we make a first guess for the lines real height
/*N*/         if ( ! pCurr->GetRealHeight() )
/*N*/             CalcRealHeight();
/*N*/ 
/*N*/         if ( pCurr->GetRealHeight() > nHeight )
/*N*/             nTop += pCurr->GetRealHeight() - nHeight;
/*N*/         else
/*N*/             // important for fixed space between lines
/*N*/             nHeight = pCurr->GetRealHeight();
/*N*/ 	}
/*N*/ 
/*N*/     const long nLeftMar = GetLeftMargin();
/*N*/ 	const long nLeftMin = (rInf.X() || GetDropLeft()) ? nLeftMar : GetLeftMin();
/*N*/ 
/*N*/     SwRect aLine( rInf.X() + nLeftMin, nTop, rInf.RealWidth() - rInf.X()
/*N*/ 				  + nLeftMar - nLeftMin	, nHeight );
/*N*/ 
/*N*/     SwRect aLineVert( aLine );
/*N*/     if ( pFrm->IsRightToLeft() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         pFrm->SwitchLTRtoRTL( aLineVert );
/*N*/ 
/*N*/     if ( pFrm->IsVertical() )
/*N*/         pFrm->SwitchHorizontalToVertical( aLineVert );
/*N*/     SwRect aInter( pTxtFly->GetFrm( aLineVert ) );
/*N*/ 
/*N*/     if ( pFrm->IsRightToLeft() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         pFrm->SwitchRTLtoLTR( aInter );
/*N*/ 
/*N*/     if ( pFrm->IsVertical() )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         pFrm->SwitchVerticalToHorizontal( aInter );
/*N*/ 
/*N*/     if( aInter.IsOver( aLine ) )
/*N*/ 	{
/*N*/         aLine.Left( rInf.X() + nLeftMar );
/*N*/ 		sal_Bool bForced = sal_False;
/*N*/ 		if( aInter.Left() <= nLeftMin )
/*N*/ 		{
/*N*/ 			SwTwips nFrmLeft = GetTxtFrm()->Frm().Left();
/*N*/ 			if( GetTxtFrm()->Prt().Left() < 0 )
/*?*/ 				nFrmLeft += GetTxtFrm()->Prt().Left();
/*N*/ 			if( aInter.Left() < nFrmLeft )
/*N*/ 				aInter.Left( nFrmLeft );
/*N*/ 			aInter.Width( aInter.Width() + nLeftMar - nFrmLeft );
/*N*/ 			// Bei negativem Erstzeileneinzug setzen wir das Flag,
/*N*/ 			// um anzuzeigen, dass der Einzug/Rand verschoben wurde
/*N*/ 			// Dies muss beim DefaultTab an der Nullposition beruecksichtigt
/*N*/ 			// werden.
/*N*/ 			if( IsFirstTxtLine() && HasNegFirst() )
/*N*/ 				bForced = sal_True;
/*N*/ 		}
/*N*/ 		aInter.Intersection( aLine );
/*N*/ 		if( !aInter.HasArea() )
/*N*/ 			return;
/*N*/ 
/*N*/ 		const sal_Bool bFullLine =	aLine.Left()  == aInter.Left() &&
/*N*/ 								aLine.Right() == aInter.Right();
/*N*/ 
/*N*/ 		// Obwohl kein Text mehr da ist, muss eine weitere Zeile
/*N*/ 		// formatiert werden, weil auch leere Zeilen einem Fly
/*N*/ 		// ohne Umlauf ausweichen muessen.
/*N*/ 		if( bFullLine && rInf.GetIdx() == rInf.GetTxt().Len() )
/*N*/ 		{
/*N*/ 			rInf.SetNewLine( sal_True );
/*N*/ 			// 8221: Dummies erkennt man an Ascent == Height
/*N*/             pCurr->SetDummy(sal_True);
/*N*/ 		}
/*N*/ 
/*N*/ 		// aInter wird framelokal
/*N*/ 		aInter.Pos().X() -= nLeftMar;
/*N*/ 		SwFlyPortion *pFly = new SwFlyPortion( aInter );
/*N*/ 		if( bForced )
/*N*/ 		{
/*N*/ 			pCurr->SetForcedLeftMargin( sal_True );
/*N*/           rInf.ForcedLeftMargin( (USHORT)aInter.Width() );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bFullLine )
/*N*/ 		{
/*N*/ 			// 8110: wir muessen um Einheiten von Zeilenhoehen anwachsen,
/*N*/ 			// um nebeneinanderliegende Flys mit unterschiedlichen
/*N*/ 			// Umlaufattributen angemessen zu umfliessen.
/*N*/ 			// Die letzte ausweichende Zeile, sollte in der Hoehe angepasst
/*N*/ 			// sein, damit nicht der Eindruck von "Rahmenabstaenden" aufkommt.
/*N*/ 			// 8221: Wichtig ist, dass Ascent == Height ist, weil die FlyPortionWerte
/*N*/ 			// im CalcLine in pCurr uebertragen werden und IsDummy() darauf
/*N*/ 			// angewiesen ist.
/*N*/ 			// Es gibt meines Wissens nur zwei Stellen, in denen DummyLines
/*N*/ 			// entstehen koennen: hier und in MakeFlyDummies.
/*N*/ 			// Ausgewertet wird IsDummy() in IsFirstTxtLine() und
/*N*/ 			// beim Zeilenwandern und im Zusammenhang mit DropCaps.
/*N*/ 			pFly->Height( KSHORT(aInter.Height()) );
/*N*/ 
/*N*/ 			// In nNextTop steckt jetzt die Unterkante des Rahmens, dem wir
/*N*/ 			// ausweichen oder die Oberkante des naechsten Rahmens, den wir
/*N*/ 			// beachten muessen. Wir koennen also jetzt getrost bis zu diesem
/*N*/ 			// Wert anwachsen, so sparen wir einige Leerzeilen.
/*N*/             SWRECTFN( pFrm )
/*N*/             long nNextTop = pTxtFly->GetNextTop();
/*N*/             if ( bVert )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 nNextTop = pFrm->SwitchVerticalToHorizontal( nNextTop );
/*N*/             if( nNextTop > aInter.Bottom() )
/*N*/ 			{
/*N*/                 SwTwips nH = nNextTop - aInter.Top();
/*N*/ 				if( nH < KSHRT_MAX )
/*N*/ 					pFly->Height( KSHORT( nH ) );
/*N*/ 			}
/*N*/ 			if( nAscent < pFly->Height() )
/*N*/ 				pFly->SetAscent( KSHORT(nAscent) );
/*N*/ 			else
/*N*/ 				pFly->SetAscent( pFly->Height() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( rInf.GetIdx() == rInf.GetTxt().Len() )
/*N*/ 			{
/*N*/ 				// Nicht nHeight nehmen, sonst haben wir einen Riesendescent
/*N*/ 				pFly->Height( pLast->Height() );
/*N*/ 				pFly->SetAscent( pLast->GetAscent() );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pFly->Height( KSHORT(aInter.Height()) );
/*N*/ 				if( nAscent < pFly->Height() )
/*N*/ 					pFly->SetAscent( KSHORT(nAscent) );
/*N*/ 				else
/*N*/ 					pFly->SetAscent( pFly->Height() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		rInf.SetFly( pFly );
/*N*/ 
/*N*/         if( pFly->Fix() < rInf.Width() )
/*N*/ 			rInf.Width( pFly->Fix() );
/*N*/ 
/*N*/         GETGRID( pFrm->FindPageFrm() )
/*N*/         if ( pGrid )
/*N*/         {
/*?*/             const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
/*?*/             const SwLayoutFrm* pBody = pPageFrm->FindBodyCont();
/*?*/ 
/*?*/             SWRECTFN( pPageFrm )
/*?*/ 
/*?*/             const long nGridOrigin = pBody ?
/*?*/                                     (pBody->*fnRect->fnGetPrtLeft)() :
/*?*/                                     (pPageFrm->*fnRect->fnGetPrtLeft)();
/*?*/ 
/*?*/             const USHORT nGridWidth = pGrid->GetBaseHeight();
/*?*/ 
/*?*/             SwTwips nStartX = GetLeftMargin();
/*?*/             if ( bVert )
/*?*/             {
/*?*/                 Point aPoint( nStartX, 0 );
/*?*/                 pFrm->SwitchHorizontalToVertical( aPoint );
/*?*/                 nStartX = aPoint.Y();
/*?*/             }
/*?*/ 
/*?*/             const SwTwips nOfst = nStartX - nGridOrigin;
/*?*/             const SwTwips nTmpWidth = rInf.Width() + nOfst;
/*?*/ 
/*?*/             const ULONG i = nTmpWidth / nGridWidth + 1;
/*?*/ 
/*?*/             const long nNewWidth = ( i - 1 ) * nGridWidth - nOfst;
/*?*/             if ( nNewWidth > 0 )
/*?*/                 rInf.Width( (USHORT)nNewWidth );
/*?*/             else
/*?*/                 rInf.Width( 0 );
/*N*/         }
/*N*/ 	}
/*N*/ }

/*****************************************************************************
 * SwTxtFormatter::NewFlyCntPortion
 * legt eine neue Portion fuer ein zeichengebundenes Objekt an.
 *****************************************************************************/

/*N*/ SwFlyCntPortion *SwTxtFormatter::NewFlyCntPortion( SwTxtFormatInfo &rInf,
/*N*/ 												   SwTxtAttr *pHint ) const
/*N*/ {
/*N*/ 	SwFlyCntPortion *pRet = 0;
/*N*/ 	const SwFrm *pFrame = (SwFrm*)pFrm;
/*N*/ 
/*N*/ 	SwFlyInCntFrm *pFly;
/*N*/ 	SwFrmFmt* pFrmFmt = ((SwTxtFlyCnt*)pHint)->GetFlyCnt().GetFrmFmt();
/*N*/ 	if( RES_FLYFRMFMT == pFrmFmt->Which() )
/*N*/ 		pFly = ((SwTxtFlyCnt*)pHint)->GetFlyFrm(pFrame);
/*N*/ 	else
/*N*/ 		pFly = NULL;
/*N*/ 	// aBase bezeichnet die dokumentglobale Position,
/*N*/ 	// ab der die neue Extraportion plaziert wird.
/*N*/ 	// aBase.X() = Offset in der Zeile,
/*N*/ 	//			   hinter der aktuellen Portion
/*N*/ 	// aBase.Y() = LineIter.Y() + Ascent der aktuellen Portion
/*N*/ 
/*N*/ 	SwLinePortion *pPos = pCurr->GetFirstPortion();
/*N*/ 
/*N*/ 	long nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc;
/*N*/ 	lcl_MaxAscDescent( pPos, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc );
/*N*/ 
/*N*/ 	// Wenn der Ascent des Rahmens groesser als der Ascent der akt. Portion
/*N*/ 	// ist, wird dieser bei der Base-Berechnung verwendet, sonst wuerde
/*N*/ 	// der Rahmen zunaechst zu weit nach oben gesetzt, um dann doch wieder
/*N*/ 	// nach unten zu rutschen und dabei ein Repaint in einem Bereich ausloesen,
/*N*/ 	// indem er niemals wirklich war.
/*N*/     KSHORT nAscent;
/*N*/     if ( IsQuick() || !pFly || !pFly->GetValidPosFlag() ||
/*N*/         ( GetInfo().GetTxtFrm()->IsVertical() ?
/*N*/           ( ! pFly->GetRefPoint().X() ||
/*N*/             ( nAscent = Abs( int( pFly->GetRelPos().X() ) ) ) ) :
/*N*/           ( ! pFly->GetRefPoint().Y() ||
/*N*/             ( nAscent = Abs( int( pFly->GetRelPos().Y() ) ) ) ) ) )
/*N*/         nAscent = rInf.GetLast()->GetAscent();
/*N*/ 	else if( nAscent > nFlyAsc )
/*N*/ 		nFlyAsc = nAscent;
/*N*/ 
/*N*/ 	Point aBase( GetLeftMargin() + rInf.X(), Y() + nAscent );
/*N*/ 	sal_uInt8 nMode = IsQuick() ? SETBASE_QUICK : 0;
/*N*/ 	if( GetMulti() && GetMulti()->HasRotation() )
/*N*/ 	{
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 		nMode |= SETBASE_ROTATE;
/*N*/ 	}
/*N*/ 
/*N*/     Point aTmpBase( aBase );
/*N*/     if ( GetInfo().GetTxtFrm()->IsVertical() )
/*?*/         GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );
/*N*/ 
/*N*/ 	if( pFly )
/*N*/ 	{
/*N*/         pRet = new SwFlyCntPortion( *GetInfo().GetTxtFrm(), pFly, aTmpBase,
/*N*/                                     nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
/*N*/ 		// Wir muessen sicherstellen, dass unser Font wieder im OutputDevice
/*N*/ 		// steht. Es koennte sein, dass der FlyInCnt frisch eingefuegt wurde,
/*N*/ 		// dann hat GetFlyFrm dazu gefuehrt, dass er neu angelegt wird.
/*N*/ 		// Dessen Frames werden sofort formatiert, die verstellen den Font
/*N*/ 		// und schon haben wir den Salat (3322).
/*N*/ 		rInf.SelectFont();
/*N*/ 		if( pRet->GetAscent() > nAscent )
/*N*/ 		{
/*N*/ 			aBase.Y() = Y() + pRet->GetAscent();
/*N*/ 			nMode |= SETBASE_ULSPACE;
/*N*/ 			if( !rInf.IsTest() )
/*N*/                 aTmpBase = aBase;
/*N*/                 if ( GetInfo().GetTxtFrm()->IsVertical() )
/*?*/                     GetInfo().GetTxtFrm()->SwitchHorizontalToVertical( aTmpBase );
/*N*/ 
/*N*/                 pRet->SetBase( *rInf.GetTxtFrm(), aTmpBase, nTmpAscent,
/*N*/                                nTmpDescent, nFlyAsc, nFlyDesc, nMode );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         pRet = new SwFlyCntPortion( *rInf.GetTxtFrm(), (SwDrawContact*)pFrmFmt->FindContactObj(),
/*N*/            aTmpBase, nTmpAscent, nTmpDescent, nFlyAsc, nFlyDesc, nMode );
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }



/*************************************************************************
 *						SwTxtFly::SwTxtFly()
 *************************************************************************/

/*N*/ SwTxtFly::SwTxtFly( const SwTxtFly& rTxtFly )
/*N*/  {
/*N*/  	pPage = rTxtFly.pPage;
/*N*/  	pCurrFly = rTxtFly.pCurrFly;
/*N*/  	pCurrFrm = rTxtFly.pCurrFrm;
/*N*/  	pMaster = rTxtFly.pMaster;
/*N*/  	if( rTxtFly.pFlyList )
/*N*/  	{
/*N*/         pFlyList = new SwFlyList( (BYTE)rTxtFly.pFlyList->Count(), 10 );
/*N*/  		pFlyList->Insert( rTxtFly.pFlyList, 0 );
/*N*/  	}
/*N*/  	else
/*N*/  		pFlyList = NULL;
/*N*/  
/*N*/  	bOn = rTxtFly.bOn;
/*N*/  	bLeftSide = rTxtFly.bLeftSide;
/*N*/  	bTopRule = rTxtFly.bTopRule;
/*N*/  }

/*N*/ void SwTxtFly::CtorInit( const SwTxtFrm *pFrm )
/*N*/ {
/*N*/ 	mbIgnoreCurrentFrame = sal_False;
/*N*/   mbIgnoreContour = sal_False;
/*N*/ 	pPage = pFrm->FindPageFrm();
/*N*/ 	const SwFlyFrm* pTmp = pFrm->FindFlyFrm();
/*N*/ 	pCurrFly = pTmp ? pTmp->GetVirtDrawObj() : NULL;
/*N*/ 	pCurrFrm = pFrm;
/*N*/ 	pMaster = pCurrFrm->IsFollow() ? NULL : pCurrFrm;
/*N*/ 	pFlyList = NULL;
/*N*/     // Wenn wir nicht von einem Frame ueberlappt werden, oder wenn
/*N*/ 	// es gar keine FlyCollection gibt, dann schaltet wir uns fuer immer ab.
/*N*/ 	// Aber es koennte sein, dass waehrend der Formatierung eine Zeile
/*N*/ 	// hinzukommt, die in einen Frame hineinragt. Deswegen keine Optimierung
/*N*/ 	// per bOn = pSortedFlys && IsAnyFrm();
/*N*/ 	bOn = pPage->GetSortedObjs() != 0;
/*N*/ 	bTopRule = sal_True;
/*N*/ 	bLeftSide = sal_False;
/*N*/ 	nMinBottom = 0;
/*N*/ 	nIndex = ULONG_MAX;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::_GetFrm()
 *
 * IN:	dokumentglobal	(rRect)
 * OUT: framelokal		(return-Wert)
 * Diese Methode wird waehrend der Formatierung vom LineIter gerufen.
 * 1. um die naechste FlyPortion vorzubereiten
 * 2. um nach Aenderung der Zeilenhoehe neue Ueberlappungen festzustellen
 *************************************************************************/

/*N*/ SwRect SwTxtFly::_GetFrm( const SwRect &rRect, sal_Bool bTop ) const
/*N*/ {
/*N*/ 	SwRect aRet;
/*N*/ 	if( ForEach( rRect, &aRet, sal_True ) )
/*N*/ 	{
/*N*/         SWRECTFN( pCurrFrm )
/*N*/ 		if( bTop )
/*N*/             (aRet.*fnRect->fnSetTop)( (rRect.*fnRect->fnGetTop)() );
/*N*/ 
/*N*/ 		// 8110: Bottom nicht immer anpassen.
/*N*/         const SwTwips nRetBottom = (aRet.*fnRect->fnGetBottom)();
/*N*/         const SwTwips nRectBottom = (rRect.*fnRect->fnGetBottom)();
/*N*/         if ( (*fnRect->fnYDiff)( nRetBottom, nRectBottom ) > 0 ||
/*N*/              (aRet.*fnRect->fnGetHeight)() < 0 )
/*N*/             (aRet.*fnRect->fnSetBottom)( nRectBottom );
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::IsAnyFrm()
 *
 * IN: dokumentglobal
 * fuer die Printarea des aktuellen Frame
 *
 * dient zum Abschalten des SwTxtFly, wenn keine Objekte ueberlappen (Relax)
 *
 *************************************************************************/

/*N*/ sal_Bool SwTxtFly::IsAnyFrm() const
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( pCurrFrm )
/*N*/ 
/*N*/ 	ASSERT( bOn, "IsAnyFrm: Why?" );
/*N*/ 	SwRect aRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
/*N*/ 		pCurrFrm->Prt().SSize() );
/*N*/ 
/*N*/     const sal_Bool bRet = ForEach( aRect, NULL, sal_False );
/*N*/     UNDO_SWAP( pCurrFrm )
/*N*/     return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::IsAnyObj()
 *
 * IN: dokumentglobal
 * OUT: sal_True Wenn ein Rahmen oder DrawObj beruecksichtigt werden muss
 * Nur wenn IsAnyObj sal_False liefert, koennen Optimierungen benutzt werden
 * wie Paint/FormatEmpty fuer leere Absaetze
 * und auch das virtuelle Outputdevice.
 *************************************************************************/

/*N*/ sal_Bool SwTxtFly::IsAnyObj( const SwRect &rRect ) const
/*N*/ {
/*N*/ 	ASSERT ( bOn, "SwTxtFly::IsAnyObj: Who's knocking?" );
/*N*/ 
/*N*/ 	SwRect aRect( rRect );
/*N*/ 	if ( aRect.IsEmpty() )
/*N*/ 		aRect = SwRect( pCurrFrm->Frm().Pos() + pCurrFrm->Prt().Pos(),
/*N*/ 						pCurrFrm->Prt().SSize() );
/*N*/ 
/*N*/ 	const SwSortDrawObjs *pSorted = pPage->GetSortedObjs();
/*N*/ 	if( pSorted ) // Eigentlich ist durch bOn sichergestellt, dass es an der
/*N*/ 	// Seite Objekte gibt, aber wer weiss, wer inzwischen etwas geloescht hat.
/*N*/ 	{
/*N*/ 		for ( MSHORT i = 0; i < pSorted->Count(); ++i )
/*N*/ 		{
/*N*/ 			const SdrObject *pObj = (*pSorted)[i];
/*N*/ 
/*N*/ 			const SwRect aBound( GetBoundRect( pObj ) );
/*N*/ 
/*N*/ 			// Optimierung
/*N*/ 			if( pObj->GetBoundRect().Left() > aRect.Right() )
/*N*/ 				continue;
/*N*/ 
/*N*/ 			if( pCurrFly != pObj && aBound.IsOver( aRect ) )
/*N*/ 				return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*N*/ const SwCntntFrm* SwTxtFly::_GetMaster()
/*N*/ {
/*N*/ 	pMaster = pCurrFrm;
/*N*/ 	while( pMaster->IsFollow() )
/*N*/ 		pMaster = (SwCntntFrm*)pMaster->FindMaster();
/*N*/ 	return pMaster;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::DrawTextOpaque()
 *
 * IN: dokumentglobal
 * DrawTextOpaque() wird von DrawText() gerufen.
 * Die Clipregions werden so gesetzt, dass nur die Teile ausgegeben werden,
 * die nicht in den Bereichen von FlyFrms liegen, die undurchsichtig und
 * ueber dem aktuellen Frame liegen.
 * Die On-Optimierung uebernimmt DrawText()!
 *************************************************************************/

#define UINT32_MAX 0xFFFFFFFF


/*************************************************************************
 *						SwTxtFly::DrawFlyRect()
 *
 * IN: windowlokal
 * Zwei Feinheiten gilt es zu beachten:
 * 1) DrawRect() oberhalb des ClipRects sind erlaubt !
 * 2) FlyToRect() liefert groessere Werte als die Framedaten !
 *************************************************************************/


/*************************************************************************
 *						SwTxtFly::GetTop()
 *
 * GetTop() ueberprueft, ob pNew ueber pCurrFly liegt (Z-Order).
 * Es gilt, dass die unten liegenden die obenliegenden beachten nicht
 * umgekehrt !
 * Returnwert: pNew, wenn pNew ueber pCurrFly liegt, ansonsten 0.
 * wird nur von InitFlyList benutzt, um die in Frage kommenden Objekte
 * einzusammeln.
 *************************************************************************/

/*M*/ sal_Bool SwTxtFly::GetTop( const SdrObject *pNew, const sal_Bool bInFtn,
/*M*/                            const sal_Bool bInFooterOrHeader )
/*M*/ {
/*M*/ 	if( pNew != pCurrFly )
/*M*/ 	{
/*N*/ 		// #102344# Ignore connectors which have one or more connections
/*N*/ 		if(pNew && pNew->ISA(SdrEdgeObj))
/*N*/ 		{
/*N*/ 			if(((SdrEdgeObj*)pNew)->GetConnectedNode(TRUE)
/*N*/ 				|| ((SdrEdgeObj*)pNew)->GetConnectedNode(FALSE))
/*N*/ 			{
/*N*/ 				return sal_False;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*M*/         if( ( bInFtn || bInFooterOrHeader ) && bTopRule )
/*M*/         {
/*M*/             SwFrmFmt *pFmt = ((SwContact*)GetUserCall(pNew))->GetFmt();
/*M*/             const SwFmtAnchor& rNewA = pFmt->GetAnchor();
/*M*/ 
/*M*/             if ( FLY_PAGE == rNewA.GetAnchorId() )
/*M*/             {
/*M*/                 if ( bInFtn )
/*M*/                     return sal_False;
/*M*/ 
/*M*/                 if ( bInFooterOrHeader )
/*M*/                 {
/*M*/                     SwFmtVertOrient aVert( pFmt->GetVertOrient() );
/*M*/                     BOOL bVertPrt = aVert.GetRelationOrient() == PRTAREA ||
/*M*/                             aVert.GetRelationOrient() == REL_PG_PRTAREA;
/*M*/                     if( bVertPrt )
/*M*/                         return sal_False;
/*M*/                 }
/*M*/             }
/*M*/         }
/*M*/ 
/*M*/ 		sal_Bool bEvade = !pCurrFly  //Selbst nicht im Fly -> allen ausweichen.
/*M*/ 				 //Den Lowern ist auszuweichen.
/*M*/ 			|| Is_Lower_Of(((SwVirtFlyDrawObj*)pCurrFly)->GetFlyFrm(), pNew);
/*M*/ 		if ( !bEvade )
/*M*/ 		{
/*M*/ 			if ( !bTopRule )
/*M*/ 				bEvade = sal_True; // nur an der Paint-Ordnung interessiert
/*M*/ 			else
/*M*/ 			{
/*M*/ 				// innerhalb von verketteten Flys wird nur Lowern ausgewichen
/*M*/ 				const SwFmtChain &rChain = ((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetChain();
/*M*/ 				if ( !rChain.GetPrev() && !rChain.GetNext() )
/*M*/ 				{
/*M*/ 					// Ausweichregel fuer Text:
/*M*/ 					const SwFmtAnchor& rNewA =
/*M*/ 						((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
/*M*/ 					const SwFmtAnchor& rCurrA =
/*M*/ 						((SwContact*)GetUserCall(pCurrFly))->GetFmt()->GetAnchor();
/*M*/ 					if( FLY_IN_CNTNT == rCurrA.GetAnchorId() )
/*M*/ 						return sal_False; // Zeichengebundene weichen nur Lowern aus.
/*M*/ 					if( FLY_PAGE == rNewA.GetAnchorId() )
/*M*/ 					{   //Chg: Seitengebundenen wird nur noch von anderen
/*M*/ 						// seitengebundenen ausgewichen!
/*M*/ 						if( FLY_PAGE == rCurrA.GetAnchorId() )
/*M*/ 							bEvade = sal_True;
/*M*/ 						else
/*M*/ 							return sal_False;
/*M*/ 					}
/*M*/ 					else if( FLY_PAGE == rCurrA.GetAnchorId() )
/*M*/ 						return sal_False; // Seitengebundene weichen nur seitengeb. aus
/*M*/ 					else if( FLY_AT_FLY == rNewA.GetAnchorId() )
/*M*/ 						bEvade = sal_True; // Nicht seitengeb. weichen Rahmengeb. aus
/*M*/ 					else if( FLY_AT_FLY == rCurrA.GetAnchorId() )
/*M*/ 						return sal_False; // Rahmengebundene weichen abs.geb. nicht aus
/*M*/                     else if( bInFooterOrHeader )
/*M*/                         return sal_False;  // In header or footer no wrapping
/*M*/                                            // if both bounded at paragraph
/*M*/                     else // Zwei Flies mit (auto-)absatzgebunder Verankerung ...
/*M*/ 					// ... entscheiden nach der Reihenfolge ihrer Anker im Dok.
/*M*/ 						bEvade = rNewA.GetCntntAnchor()->nNode.GetIndex() <=
/*M*/ 								rCurrA.GetCntntAnchor()->nNode.GetIndex();
/*M*/ 				}
/*M*/ 			}
/*M*/ 			// aber: es wird niemals einem hierarchisch untergeordnetem
/*M*/ 			// ausgewichen und ausserdem braucht nur bei Ueberlappung
/*M*/ 			// ausgewichen werden.
/*M*/ 			bEvade &= ( pCurrFly->GetOrdNum() < pNew->GetOrdNum() );
/*M*/ 			if( bEvade )
/*M*/ 			{
/*M*/ 				SwRect aTmp( GetBoundRect( pNew ) );
/*M*/ 				if( !aTmp.IsOver( pCurrFly->GetBoundRect() ) )
/*M*/ 					bEvade = sal_False;
/*M*/ 			}
/*M*/ 		}
/*M*/ 		if ( bEvade )
/*M*/ 		{
/*M*/ 			const SwFmtAnchor& rNewA =
/*M*/ 				((SwContact*)GetUserCall(pNew))->GetFmt()->GetAnchor();
/*M*/ 			ASSERT( FLY_IN_CNTNT != rNewA.GetAnchorId(), "Don't call GetTop with a FlyInCntFrm" );
/*M*/ 			if( FLY_PAGE == rNewA.GetAnchorId() )
/*M*/ 				return sal_True;  // Seitengebundenen wird immer ausgewichen.
/*M*/ 
/*M*/ 			// Wenn absatzgebundene Flys in einem FlyCnt gefangen sind, so
/*M*/ 			// endet deren Einflussbereich an den Grenzen des FlyCnt!
/*M*/ 			// Wenn wir aber gerade den Text des FlyCnt formatieren, dann
/*M*/ 			// muss er natuerlich dem absatzgebundenen Frm ausweichen!
/*M*/ 			// pCurrFrm ist der Anker von pNew?
/*N*/           const SwFrm* pTmp = &lcl_TheAnchor( pNew );
/*M*/ 			if( pTmp == pCurrFrm )
/*M*/ 				return sal_True;
/*M*/ 			if( pTmp->IsTxtFrm() && ( pTmp->IsInFly() || pTmp->IsInFtn() ) )
/*M*/ 			{
/*M*/ 				Point aPos;
/*M*/ 				if( pNew->IsWriterFlyFrame() )
/*M*/ 					aPos = ( (SwVirtFlyDrawObj*)pNew )->GetFlyFrm()->Frm().Pos();
/*M*/ 				else
/*M*/ 					aPos = pNew->GetBoundRect().TopLeft();
/*M*/ 				pTmp = GetVirtualUpper( pTmp, aPos );
/*M*/ 			}
/*M*/ 			if( pCurrFrm->GetNext() != pTmp &&
/*M*/ 				IsFrmInSameKontext( pTmp, pCurrFrm ) )
/*M*/ 			{
/*M*/ 				if( FLY_AT_FLY == rNewA.GetAnchorId() ) // LAYER_IMPL
/*M*/ 					return sal_True;  // Rahmengebundenen ausweichen.
/*M*/ 				// Den Index des anderen erhalten wir immer ueber das Ankerattr.
/*M*/ 				ULONG nTmpIndex = rNewA.GetCntntAnchor()->nNode.GetIndex();
/*M*/ 				// Jetzt wird noch ueberprueft, ob der aktuelle Absatz vor dem
/*M*/ 				// Anker des verdraengenden Objekts im Text steht, dann wird
/*M*/ 				// nicht ausgewichen.
/*M*/ 				// Der Index wird moeglichst ueber einen SwFmtAnchor ermittelt,
/*M*/ 				// da sonst recht teuer.
/*M*/ 				if( ULONG_MAX == nIndex )
/*M*/ 					nIndex = pCurrFrm->GetNode()->GetIndex();
/*M*/ 
/*M*/ 				if( nIndex >= nTmpIndex )
/*M*/ 					return sal_True;
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/ 	return 0;
/*M*/ }

/*************************************************************************
 * SwTxtFly::InitFlyList()
 *
 * fuellt die FlyList mit den Objekten, denen ggf. ausgwichen werden muss
 *
 *************************************************************************/

/*N*/ SwFlyList *SwTxtFly::InitFlyList()
/*N*/ {
/*N*/ 	ASSERT( pCurrFrm, "InitFlyList: No Frame, no FlyList" );
/*N*/     ASSERT( !pFlyList, "InitFlyList: FlyList already initialized" );
/*N*/ 
/*N*/     SWAP_IF_SWAPPED( pCurrFrm )
/*N*/ 
/*N*/ 	const SwSortDrawObjs *pSorted = pPage->GetSortedObjs();
/*N*/ 	const MSHORT nCount = pSorted ? pSorted->Count() : 0;
/*N*/ 	bOn = sal_False;
/*N*/ 	if( nCount )
/*N*/ 	{
/*N*/ 		pFlyList = new SwFlyList( 10, 10 );
/*N*/ 
/*N*/ 		SwRect aRect( pCurrFrm->Prt() );
/*N*/ 		aRect += pCurrFrm->Frm().Pos();
/*N*/ 		// Wir machen uns etwas kleiner als wir sind,
/*N*/ 		// damit Ein-Twip-Ueberlappungen ignoriert werden. (#49532)
/*N*/         SWRECTFN( pCurrFrm )
/*N*/         const long nRight = (aRect.*fnRect->fnGetRight)() - 1;
/*N*/         const long nLeft = (aRect.*fnRect->fnGetLeft)() + 1;
/*N*/ 		const sal_Bool bFooter = pCurrFrm->IsInFtn();
/*N*/       const sal_Bool bR2L = pCurrFrm->IsRightToLeft();
/*N*/ 
/*N*/ 		for( MSHORT i = 0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*pSorted)[ i ];
/*N*/ 			const SwRect aBound( GetBoundRect( pO ) );
/*N*/ 
/*N*/             if( nRight < (aBound.*fnRect->fnGetLeft)() ||
/*N*/                 (*fnRect->fnYDiff)( (aRect.*fnRect->fnGetTop)(),
/*N*/                                     (aBound.*fnRect->fnGetBottom)() ) > 0 ||
/*N*/                 nLeft > (aBound.*fnRect->fnGetRight)() )
/*N*/ 				continue;
/*N*/ 
/*N*/             if( GetTop( pO, pCurrFrm->IsInFtn(),
/*N*/                         0 != pCurrFrm->FindFooterOrHeader() ) )
/*N*/ 			{
/*N*/                 // OD 11.03.2003 #107862# - adjust insert position:
/*N*/                 // overlapping objects should be sorted from left to right and
/*N*/                 // inside left to right sorting from top to bottom.
/*N*/                 // If objects on the same position are found, they are sorted
/*N*/                 // on its width.
/*N*/                 sal_uInt16 nPos = pFlyList->Count();
/*N*/                 while ( nPos )
/*N*/                 {
/*N*/                     SdrObject* pTmpObj = (*pFlyList)[ --nPos ];
/*N*/                     const SwRect aBoundRectOfTmpObj( GetBoundRect( pTmpObj ) );
/*N*/                     if ( ( bR2L &&
/*N*/                            ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ==
/*N*/                              (aBound.*fnRect->fnGetRight)() ) ) ||
/*N*/                          ( !bR2L &&
/*N*/                            ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ==
/*N*/                              (aBound.*fnRect->fnGetLeft)() ) ) )
/*N*/                     {
/*N*/                         SwTwips nTopDiff =
/*N*/                             (*fnRect->fnYDiff)( (aBound.*fnRect->fnGetTop)(),
/*N*/                                                 (aBoundRectOfTmpObj.*fnRect->fnGetTop)() );
/*N*/                         if ( nTopDiff == 0 &&
/*N*/                              ( ( bR2L &&
/*N*/                                  ( (aBound.*fnRect->fnGetLeft)() >
/*N*/                                    (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() ) ) ||
/*N*/                                ( !bR2L &&
/*N*/                                  ( (aBound.*fnRect->fnGetRight)() <
/*N*/                                    (aBoundRectOfTmpObj.*fnRect->fnGetRight)() ) ) ) )
/*N*/                         {
/*N*/                             ++nPos;
/*N*/                             break;
/*N*/                         }
/*N*/                         else if ( nTopDiff > 0 )
/*N*/                         {
/*N*/                             ++nPos;
/*N*/                             break;
/*N*/                         }
/*N*/                     }
/*N*/                     else if ( ( bR2L &&
/*N*/                                 ( (aBoundRectOfTmpObj.*fnRect->fnGetRight)() >
/*N*/                                   (aBound.*fnRect->fnGetRight)() ) ) ||
/*N*/                               ( !bR2L &&
/*N*/                                 ( (aBoundRectOfTmpObj.*fnRect->fnGetLeft)() <
/*N*/                                   (aBound.*fnRect->fnGetLeft)() ) ) )
/*N*/                     {
/*N*/                         ++nPos;
/*N*/                         break;
/*N*/                     }
/*N*/                 }
/*N*/ 				pFlyList->C40_INSERT( SdrObject, pO, nPos );
/*N*/ 
/*N*/ 				SwContact *pContact = (SwContact*)GetUserCall(pO);
/*N*/ 				const SwFmtSurround &rFlyFmt = pContact->GetFmt()->GetSurround();
/*N*/                 if( rFlyFmt.IsAnchorOnly() && &lcl_TheAnchor( pO ) == GetMaster() )
/*N*/ 				{
/*N*/ 					const SwFmtVertOrient &rTmpFmt = pContact->GetFmt()->GetVertOrient();
/*N*/ 					if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
/*N*/                         nMinBottom = ( bVert && nMinBottom ) ?
/*N*/                                      Min( nMinBottom, aBound.Left() ) :
/*N*/                                      Max( nMinBottom, (aBound.*fnRect->fnGetBottom)() );
/*N*/ 				}
/*N*/ 
/*N*/ 				bOn = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( nMinBottom )
/*N*/ 		{
/*N*/             SwTwips nMax = (pCurrFrm->GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/             if( (*fnRect->fnYDiff)( nMinBottom, nMax ) > 0 )
/*N*/ 				nMinBottom = nMax;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pFlyList = new SwFlyList( 0, 10 );
/*N*/ 
/*N*/     UNDO_SWAP( pCurrFrm )
/*N*/ 
/*N*/ 	return pFlyList;
/*N*/ }

/*N*/ SwTwips SwTxtFly::CalcMinBottom() const
/*N*/ {
/*N*/ 	SwTwips nRet = 0;
/*N*/ 	const SwDrawObjs *pDrawObj = GetMaster()->GetDrawObjs();
/*N*/ 	const MSHORT nCount = pDrawObj ? pDrawObj->Count() : 0;
/*N*/ 	if( nCount )
/*N*/ 	{
/*N*/ 		SwTwips nEndOfFrm = pCurrFrm->Frm().Bottom();
/*N*/ 		for( MSHORT i = 0; i < nCount; i++ )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*pDrawObj)[ i ];
/*N*/ 			SwContact *pContact = (SwContact*)GetUserCall(pO);
/*N*/ 			const SwFmtSurround &rFlyFmt = pContact->GetFmt()->GetSurround();
/*N*/ 			if( rFlyFmt.IsAnchorOnly() )
/*N*/ 			{
/*?*/ 				const SwFmtVertOrient &rTmpFmt = pContact->GetFmt()->GetVertOrient();
/*?*/ 				if( VERT_BOTTOM != rTmpFmt.GetVertOrient() )
/*?*/ 				{
/*?*/ 					const SwRect aBound( GetBoundRect( pO ) );
/*?*/ 					if( aBound.Top() < nEndOfFrm )
/*?*/ 						nRet = Max( nRet, aBound.Bottom() );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		SwTwips nMax = pCurrFrm->GetUpper()->Frm().Top() +
/*N*/ 					   pCurrFrm->GetUpper()->Prt().Bottom();
/*N*/ 		if( nRet > nMax )
/*N*/ 			nRet = nMax;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
 * Hier erfolgt die Berechnung der Kontur ...
 * CalcBoundRect(..) und andere
 *************************************************************************/

/*************************************************************************
 * class SwContourCache
 *************************************************************************/

/*N*/ SwContourCache::SwContourCache() :
/*N*/ 	nObjCnt( 0 ), nPntCnt( 0 )
/*N*/ {
/*N*/ 	memset( (SdrObject**)pSdrObj, 0, sizeof(pSdrObj) );
/*N*/ 	memset( pTextRanger, 0, sizeof(pTextRanger) );
/*N*/ }
/*N*/ 
/*N*/ SwContourCache::~SwContourCache()
/*N*/ {
/*N*/ 	for( MSHORT i = 0; i < nObjCnt; delete pTextRanger[ i++ ] )
/*N*/ 		;
/*N*/ }

/*?*/ void SwContourCache::ClrObject( MSHORT nPos )
/*?*/ {
/*?*/ 	ASSERT( pTextRanger[ nPos ], "ClrObject: Allready cleared. Good Bye!" );
/*?*/ 	nPntCnt -= pTextRanger[ nPos ]->GetPointCount();
/*?*/ 	delete pTextRanger[ nPos ];
/*?*/ 	--nObjCnt;
/*?*/ 	memmove( (SdrObject**)pSdrObj + nPos, pSdrObj + nPos + 1,
/*?*/ 			 ( nObjCnt - nPos ) * sizeof( SdrObject* ) );
/*?*/ 	memmove( pTextRanger + nPos, pTextRanger + nPos + 1,
/*?*/ 			 ( nObjCnt - nPos ) * sizeof( TextRanger* ) );
/*?*/ }

/*N*/ void ClrContourCache( const SdrObject *pObj )
/*N*/ {
/*N*/ 	if( pContourCache && pObj )
/*N*/ 		for( MSHORT i = 0; i < pContourCache->GetCount(); ++i )
/*N*/ 			if( pObj == pContourCache->GetObject( i ) )
/*N*/ 			{
/*?*/ 				pContourCache->ClrObject( i );
/*?*/ 				break;
/*N*/ 			}
/*N*/ }

/*N*/ void ClrContourCache()
/*N*/ {
/*N*/ 	if( pContourCache )
/*N*/ 	{
/*N*/ 		for( MSHORT i = 0; i < pContourCache->GetCount();
/*N*/ 			 delete pContourCache->pTextRanger[ i++ ] )
/*N*/ 			 ;
/*N*/ 		pContourCache->nObjCnt = 0;
/*N*/ 		pContourCache->nPntCnt = 0;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 * SwContourCache::CalcBoundRect
 * berechnet das Rechteck, welches vom Objekt in der angegebenen Zeile
 * ueberdeckt wird.
 * Bei _nicht_ konturumflossenen Objekten ist dies einfach die Ueber-
 * lappung von BoundRect (inkl. Abstand!) und Zeile,
 * bei Konturumfluss wird das Polypolygon des Objekts abgeklappert
 *************************************************************************/

/*N*/ const SwRect SwContourCache::CalcBoundRect( const SdrObject* pObj,
/*N*/         const SwRect &rLine, const SwTxtFrm* pFrm, const long nXPos,
/*N*/         const sal_Bool bRight )
/*N*/ {
/*N*/     SWRECTFN( pFrm )
/*N*/ 
/*N*/     SwRect aRet;
/*N*/ 	const SwFmt *pFmt =
/*N*/ 		((SwContact*)GetUserCall(pObj))->GetFmt();
/*N*/ 	if( pFmt->GetSurround().IsContour() &&
/*N*/ 		( !pObj->IsWriterFlyFrame() ||
/*N*/ 		  ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower() &&
/*N*/ 		  ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm()->Lower()->IsNoTxtFrm() ) )
/*N*/ 	{
/*N*/         aRet = GetBoundRect( pObj );
/*N*/ 		if( aRet.IsOver( rLine ) )
/*N*/ 		{
/*N*/ 			if( !pContourCache )
/*N*/ 				pContourCache = new SwContourCache;
/*N*/ 
/*N*/             aRet = pContourCache->ContourRect(
/*N*/                     pFmt, pObj, pFrm, rLine, nXPos, bRight );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aRet.Width( 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         aRet = GetBoundRect( pObj );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aRet;
/*N*/ }

/*N*/ const SwRect SwContourCache::ContourRect( const SwFmt* pFmt,
/*N*/     const SdrObject* pObj, const SwTxtFrm* pFrm, const SwRect &rLine,
/*N*/     const long nXPos, const sal_Bool bRight )
/*N*/ {
/*N*/ 	SwRect aRet;
/*N*/ 	MSHORT nPos = 0; // Suche im Cache ...
/*N*/ 	while( nPos < GetCount() && pObj != pSdrObj[ nPos ] )
/*N*/ 		++nPos;
/*N*/ 	if( GetCount() == nPos ) // nicht gefunden
/*N*/ 	{
/*N*/ 		if( nObjCnt == POLY_CNT )
/*N*/ 		{
/*?*/ 			nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
/*?*/ 			delete pTextRanger[ nObjCnt ];
/*N*/ 		}
/*N*/ 		XPolyPolygon aXPoly;
/*N*/ 		XPolyPolygon *pXPoly = NULL;
/*N*/ 		if ( pObj->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			// Vorsicht #37347: Das GetContour() fuehrt zum Laden der Grafik,
/*N*/ 			// diese aendert dadurch ggf. ihre Groesse, ruft deshalb ein
/*N*/ 			// ClrObject() auf.
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 PolyPolygon aPoly;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             if( !pObj->ISA( E3dObject ) )
/*N*/ 				pObj->TakeXorPoly( aXPoly, sal_True );
/*N*/ 			pXPoly = new XPolyPolygon();
/*N*/ 			pObj->TakeContour( *pXPoly );
/*N*/ 		}
/*N*/ 		const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
/*N*/ 		const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
/*N*/ 		memmove( pTextRanger + 1, pTextRanger, nObjCnt * sizeof( TextRanger* ) );
/*N*/ 		memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nObjCnt++ * sizeof( SdrObject* ) );
/*N*/ 		pSdrObj[ 0 ] = pObj; // Wg. #37347 darf das Object erst nach dem
/*N*/ 							 // GetContour() eingetragen werden.
/*N*/         pTextRanger[ 0 ] = new TextRanger( aXPoly, pXPoly, 20,
/*N*/             (USHORT)rLRSpace.GetLeft(), (USHORT)rLRSpace.GetRight(),
/*N*/             pFmt->GetSurround().IsOutside(), sal_False, pFrm->IsVertical() );
/*N*/         pTextRanger[ 0 ]->SetUpper( rULSpace.GetUpper() );
/*N*/ 		pTextRanger[ 0 ]->SetLower( rULSpace.GetLower() );
/*N*/ 
/*N*/ 		delete pXPoly;
/*N*/ 		// UPPER_LOWER_TEST
/*N*/ #ifdef DBG_UTIL
/*N*/ 		if(	pFmt->GetDoc()->GetRootFrm()->GetCurrShell() )
/*N*/ 		{
/*N*/ 			sal_Bool bT2 =	pFmt->GetDoc()->GetRootFrm()->GetCurrShell()->GetViewOptions()->IsTest2();
/*N*/ 			sal_Bool bT6 = pFmt->GetDoc()->GetRootFrm()->GetCurrShell()->GetViewOptions()->IsTest6();
/*N*/ 			if( bT2 || bT6 )
/*N*/ 			{
/*?*/ 				if( bT2 )
/*?*/ 					pTextRanger[ 0 ]->SetFlag7( sal_True );
/*?*/ 				else
/*?*/ 					pTextRanger[ 0 ]->SetFlag6( sal_True );
/*N*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/ 		nPntCnt += pTextRanger[ 0 ]->GetPointCount();
/*N*/ 		while( nPntCnt > POLY_MAX && nObjCnt > POLY_MIN )
/*N*/ 		{
/*?*/ 			nPntCnt -= pTextRanger[ --nObjCnt ]->GetPointCount();
/*?*/ 			delete pTextRanger[ nObjCnt ];
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( nPos )
/*N*/ 	{
/*N*/ 		const SdrObject* pTmpObj = pSdrObj[ nPos ];
/*N*/ 		TextRanger* pTmpRanger = pTextRanger[ nPos ];
/*N*/ 		memmove( (SdrObject**)pSdrObj + 1, pSdrObj, nPos * sizeof( SdrObject* ) );
/*N*/ 		memmove( pTextRanger + 1, pTextRanger, nPos * sizeof( TextRanger* ) );
/*N*/ 		pSdrObj[ 0 ] = pTmpObj;
/*N*/ 		pTextRanger[ 0 ] = pTmpRanger;
/*N*/ 	}
/*N*/     SWRECTFN( pFrm )
/*N*/     long nTmpTop = (rLine.*fnRect->fnGetTop)();
/*N*/     // fnGetBottom is top + height
/*N*/     long nTmpBottom = (rLine.*fnRect->fnGetBottom)();
/*N*/ 
/*N*/     Range aRange( Min( nTmpTop, nTmpBottom ), Max( nTmpTop, nTmpBottom ) );
/*N*/ 
/*N*/ 	SvLongs *pTmp = pTextRanger[ 0 ]->GetTextRanges( aRange );
/*N*/ 
/*N*/ 	MSHORT nCount;
/*N*/ 	if( 0 != ( nCount = pTmp->Count() ) )
/*N*/ 	{
/*N*/ 		MSHORT nIdx = 0;
/*N*/         while( nIdx < nCount && (*pTmp)[ nIdx ] < nXPos )
/*N*/ 			++nIdx;
/*N*/ 		sal_Bool bOdd = nIdx % 2 ? sal_True : sal_False;
/*N*/ 		sal_Bool bSet = sal_True;
/*N*/ 		if( bOdd )
/*N*/ 			--nIdx; // innerhalb eines Intervalls
/*N*/         else if( ! bRight && ( nIdx >= nCount || (*pTmp)[ nIdx ] != nXPos ) )
/*N*/ 		{
/*N*/ 			if( nIdx )
/*N*/ 				nIdx -= 2; // ein Intervall nach links gehen
/*N*/ 			else
/*N*/ 				bSet = sal_False; // vor dem erstem Intervall
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bSet && nIdx < nCount )
/*N*/ 		{
/*N*/             (aRet.*fnRect->fnSetTopAndHeight)( (rLine.*fnRect->fnGetTop)(),
/*N*/                                                (rLine.*fnRect->fnGetHeight)() );
/*N*/             (aRet.*fnRect->fnSetLeft)( (*pTmp)[ nIdx ] );
/*N*/             (aRet.*fnRect->fnSetRight)( (*pTmp)[ nIdx + 1 ] + 1 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aRet;
/*N*/ }

/*************************************************************************
 *						SwContourCache::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#ifdef DBG_UTIL

#endif

/*************************************************************************
 *						SwTxtFly::ShowContour()
 * zeichnet die PolyPolygone des Caches zu Debugzwecken.
 *************************************************************************/
#ifdef DBG_UTIL

#endif

/*************************************************************************
 *						SwTxtFly::ForEach()
 *
 * sucht nach dem ersten Objekt, welches mit dem Rechteck ueberlappt
 *
 *************************************************************************/

/*M*/ sal_Bool SwTxtFly::ForEach( const SwRect &rRect, SwRect* pRect, sal_Bool bAvoid ) const
/*M*/ {
/*M*/     SWAP_IF_SWAPPED( pCurrFrm )
/*M*/ 
/*M*/ 	sal_Bool bRet = sal_False;
/*M*/ 	MSHORT nCount;
/*M*/ 	if( bOn && ( 0 != ( nCount = GetFlyList()->Count() ) ) )
/*M*/ 	{
/*M*/ 		for( MSHORT i = 0; i < nCount; ++i )
/*M*/ 		{
/*M*/ 			const SdrObject *pObj = (*pFlyList)[ i ];
/*M*/ 
/*M*/ 			SwRect aRect( GetBoundRect( pObj ) );
/*M*/ 
/*M*/             // Optimierung
/*M*/             SWRECTFN( pCurrFrm )
/*M*/             if( (aRect.*fnRect->fnGetLeft)() > (rRect.*fnRect->fnGetRight)() )
/*M*/ 				break;
/*M*/ 			if( pCurrFly != pObj && aRect.IsOver( rRect ) )
/*M*/ 			{
/*M*/ 				const SwFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
/*M*/ 				const SwFmtSurround &rSur = pFmt->GetSurround();
/*M*/ 				if( bAvoid )
/*M*/ 				{
/*M*/ 					// Wenn der Text drunter durchlaeuft, bleibt die
/*M*/ 					// Formatierung unbeeinflusst. Im LineIter::DrawText()
/*M*/ 					// muessen "nur" geschickt die ClippingRegions gesetzt werden ...
/*M*/ 					const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*M*/ 					if( ( SURROUND_THROUGHT == rSur.GetSurround() &&
/*M*/ 						  ( !rSur.IsAnchorOnly() ||
/*N*/                             GetMaster() == &lcl_TheAnchor( pObj ) ||
/*N*/ 							( FLY_AT_CNTNT != rAnchor.GetAnchorId() &&
/*N*/ 							  FLY_AUTO_CNTNT != rAnchor.GetAnchorId() ) ) )
/*N*/ 						|| aRect.Top() == WEIT_WECH )
/*N*/ 						continue;
/*N*/ 				}
/*N*/ 
/*N*/                 if ( mbIgnoreCurrentFrame && pCurrFrm == &lcl_TheAnchor( pObj ) )
/*N*/                     continue;
/*N*/ 
/*N*/ 				if( pRect )
/*N*/ 				{
/*N*/                     SwRect aFly = FlyToRect( pObj, rRect );
/*N*/ 					if( aFly.IsEmpty() || !aFly.IsOver( rRect ) )
/*N*/ 						continue;
/*N*/                     if( !bRet ||
/*N*/                         ( !pCurrFrm->IsRightToLeft() &&
/*N*/                           ( (aFly.*fnRect->fnGetLeft)() <
/*N*/                             (pRect->*fnRect->fnGetLeft)() ) ||
/*N*/                         ( pCurrFrm->IsRightToLeft() &&
/*N*/                           ( (aFly.*fnRect->fnGetRight)() >
/*N*/                             (pRect->*fnRect->fnGetRight)() ) ) ) )
/*N*/ 						*pRect = aFly;
/*N*/ 					if( rSur.IsContour() )
/*N*/ 					{
/*N*/ 						bRet = sal_True;
/*N*/ 						continue;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				bRet = sal_True;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( pCurrFrm )
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::GetPos()
 *
 * liefert die Position im sorted Array zurueck
 *************************************************************************/

/*N*/ MSHORT SwTxtFly::GetPos( const SdrObject *pObj ) const
/*N*/ {
/*N*/ 	MSHORT nCount = GetFlyList()->Count();
/*N*/ 	MSHORT nRet = 0;
/*N*/ 	while( nRet < nCount && pObj != (*pFlyList)[ nRet ] )
/*N*/ 		++nRet;
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::CalcRightMargin()
 *
 * pObj ist das Object, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der rechte Rand ist der rechte Rand oder
 * er wird durch das naechste Object, welches in die Zeile ragt, bestimmt.
 *************************************************************************/

/*N*/ void SwTxtFly::CalcRightMargin( SwRect &rFly, MSHORT nFlyPos,
/*N*/ 								  const SwRect &rLine ) const
/*N*/ {
/*N*/ 	// Normalerweise ist der rechte Rand der rechte Rand der Printarea.
/*N*/     ASSERT( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
/*N*/             "SwTxtFly::CalcRightMargin with swapped frame" )
/*N*/     SWRECTFN( pCurrFrm )
/*N*/     SwTwips nRight = (pCurrFrm->Frm().*fnRect->fnGetLeft)() +
/*N*/                      (pCurrFrm->Prt().*fnRect->fnGetRight)() + 1;
/*N*/     SwTwips nFlyRight = (rFly.*fnRect->fnGetRight)();
/*N*/ 	SwRect aLine( rLine );
/*N*/     (aLine.*fnRect->fnSetRight)( nRight );
/*N*/     (aLine.*fnRect->fnSetLeft)( (rFly.*fnRect->fnGetLeft)() );
/*N*/ 
/*N*/ 	// Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
/*N*/ 	// Object hineinragt, welches _ueber_ uns liegt.
/*N*/ 	// Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
/*N*/ 	// unsichtbar, das heisst, dass sie bei der Berechnung der Raender
/*N*/ 	// anderer Flys ebenfalls nicht auffallen.
/*N*/ 	// 3301: pNext->Frm().IsOver( rLine ) ist noetig
/*N*/ 	_FlyCntnt eOrder;
/*N*/ 
/*N*/ 	sal_Bool bStop = sal_False;
/*N*/ 	MSHORT nPos = 0;
/*N*/ 
/*N*/ 	while( nPos < pFlyList->Count() && !bStop )
/*N*/ 	{
/*N*/ 		if( nPos == nFlyPos )
/*N*/ 		{
/*N*/ 			++nPos;
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 		const SdrObject *pNext = (*pFlyList)[ nPos++ ];
/*N*/ 		if( pNext == pCurrFly )
/*N*/ 			continue;
/*N*/ 		eOrder = GetOrder( pNext );
/*N*/ 		if( SURROUND_THROUGHT == eOrder )
/*N*/ 			continue;
/*N*/ 
/*N*/         const SwRect aTmp( SwContourCache::CalcBoundRect
/*N*/                 ( pNext, aLine, pCurrFrm, nFlyRight, sal_True ) );
/*N*/         SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
/*N*/ 
/*N*/ 		// Optimierung:
/*N*/ 		// In nNextTop wird notiert, an welcher Y-Positon mit Aenderung der
/*N*/ 		// Rahmenverhaeltnisse gerechnet werden muss. Dies dient dazu, dass,
/*N*/ 		// obwohl nur die Rahmen in der aktuellen Zeilenhoehe betrachtet werden,
/*N*/ 		// bei Rahmen ohne Umlauf die Zeilenhoehe so erhoeht wird, dass mit einer
/*N*/ 		// einzigen Zeile die Unterkante das Rahmens oder ggf. die Oberkante des
/*N*/ 		// naechsten Rahmen erreicht wird.
/*N*/ 		// Insbesondere bei HTML-Dokumenten kommen oft (Dummy-)Absaetze in einer
/*N*/ 		// 2-Pt.-Schrift vor, bis diese einem groesseren Rahmen ausgewichen sind,
/*N*/ 		// erforderte es frueher Unmengen von Leerzeilen.
/*N*/         const long nTmpTop = (aTmp.*fnRect->fnGetTop)();
/*N*/         if( (*fnRect->fnYDiff)( nTmpTop, (aLine.*fnRect->fnGetTop)() ) > 0 )
/*N*/ 		{
/*N*/             if( (*fnRect->fnYDiff)( nNextTop, nTmpTop ) > 0 )
/*N*/                 SetNextTop( nTmpTop ); // Die Oberkante des "naechsten" Rahmens
/*N*/ 		}
/*N*/         else if( ! (aTmp.*fnRect->fnGetWidth)() ) // Typisch fuer Objekte mit Konturumlauf
/*N*/ 		{   // Bei Objekten mit Konturumlauf, die vor der aktuellen Zeile beginnen
/*N*/ 			// und hinter ihr enden, trotzdem aber nicht mit ihr ueberlappen,
/*N*/ 			// muss die Optimierung ausgeschaltet werden, denn bereits in der
/*N*/ 			// naechsten Zeile kann sich dies aendern.
/*N*/             if( ! (aTmp.*fnRect->fnGetHeight)() ||
/*N*/                 (*fnRect->fnYDiff)( (aTmp.*fnRect->fnGetBottom)(),
/*N*/                                     (aLine.*fnRect->fnGetTop)() ) > 0 )
/*N*/ 				SetNextTop( 0 );
/*N*/ 		}
/*N*/ 		if( aTmp.IsOver( aLine ) && nTmpRight > nFlyRight )
/*N*/ 		{
/*N*/ 			nFlyRight = nTmpRight;
/*N*/ 			switch( eOrder )
/*N*/ 			{
/*N*/ 				case SURROUND_RIGHT :
/*N*/ 				case SURROUND_PARALLEL :
/*N*/ 				{
/*N*/ 					// der FlyFrm wird ueberstimmt.
/*N*/ 					if( nRight > nFlyRight )
/*N*/ 						nRight = nFlyRight;
/*N*/ 					bStop = sal_True;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/     (rFly.*fnRect->fnSetRight)( nRight );
/*N*/ }

/*************************************************************************
 *						SwTxtFly::CalcLeftMargin()
 *
 * pFly ist der FlyFrm, der uns gerade ueberlappt.
 * pCurrFrm ist der aktuelle Textframe, der ueberlappt wird.
 * Der linke Rand ist der linke Rand der aktuellen PrintArea oder
 * er wird durch den vorigen FlyFrm, der in die Zeile ragt, bestimmt.
 *************************************************************************/

/*N*/ void SwTxtFly::CalcLeftMargin( SwRect &rFly, MSHORT nFlyPos,
/*N*/ 								  const SwRect &rLine ) const
/*N*/ {
/*N*/     ASSERT( ! pCurrFrm->IsVertical() || ! pCurrFrm->IsSwapped(),
/*N*/             "SwTxtFly::CalcLeftMargin with swapped frame" )
/*N*/     SWRECTFN( pCurrFrm )
/*N*/     SwTwips nLeft = (pCurrFrm->Frm().*fnRect->fnGetLeft)() +
/*N*/                     (pCurrFrm->Prt().*fnRect->fnGetLeft)();
/*N*/     const SwTwips nFlyLeft = (rFly.*fnRect->fnGetLeft)();
/*N*/ 
/*N*/     if( nLeft > nFlyLeft )
/*N*/ 		nLeft = rFly.Left();
/*N*/ 
/*N*/     SwRect aLine( rLine );
/*N*/     (aLine.*fnRect->fnSetLeft)( nLeft );
/*N*/ 
/*N*/ 	// Es koennte aber sein, dass in die gleiche Zeile noch ein anderes
/*N*/ 	// Object hineinragt, welches _ueber_ uns liegt.
/*N*/ 	// Wunder der Technik: Flys mit Durchlauf sind fuer die darunterliegenden
/*N*/ 	// unsichtbar, das heisst, dass sie bei der Berechnung der Raender
/*N*/ 	// anderer Flys ebenfalls nicht auffallen.
/*N*/ 	// 3301: pNext->Frm().IsOver( rLine ) ist noetig
/*N*/ 
/*N*/ 	MSHORT nMyPos = nFlyPos;
/*N*/ 	while( ++nFlyPos < pFlyList->Count() )
/*N*/ 	{
/*N*/ 		const SdrObject *pNext = (*pFlyList)[ nFlyPos ];
/*N*/ 		const SwRect aTmp( GetBoundRect( pNext ) );
/*N*/         if( (aTmp.*fnRect->fnGetLeft)() >= nFlyLeft )
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	while( nFlyPos )
/*N*/ 	{
/*N*/ 		if( --nFlyPos == nMyPos )
/*N*/ 			continue;
/*N*/ 		const SdrObject *pNext = (*pFlyList)[ nFlyPos ];
/*N*/ 		if( pNext == pCurrFly )
/*N*/ 			continue;
/*N*/ 		_FlyCntnt eOrder = GetOrder( pNext );
/*N*/ 		if( SURROUND_THROUGHT == eOrder )
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwRect aTmp( SwContourCache::CalcBoundRect
/*N*/                 ( pNext, aLine, pCurrFrm, nFlyLeft, sal_False ) );
/*N*/ 
/*N*/         if( (aTmp.*fnRect->fnGetLeft)() < nFlyLeft && aTmp.IsOver( aLine ) )
/*N*/ 		{
/*N*/             SwTwips nTmpRight = (aTmp.*fnRect->fnGetRight)();
/*N*/ 			if( nLeft <= nTmpRight )
/*N*/ 				nLeft = nTmpRight + 1;
/*N*/ 
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/     (rFly.*fnRect->fnSetLeft)( nLeft );
/*N*/ }

/*************************************************************************
 *						SwTxtFly::FlyToRect()
 *
 * IN:	dokumentglobal	(rRect)
 * OUT: dokumentglobal	(return-Wert)
 * Liefert zu einem SwFlyFrm das von ihm in Anspruch genommene Rechteck
 * unter Beruecksichtigung der eingestellten Attribute fuer den Abstand
 * zum Text zurueck.
 *************************************************************************/

/*N*/ SwRect SwTxtFly::FlyToRect( const SdrObject *pObj, const SwRect &rLine ) const
/*N*/ {
/*N*/     SWRECTFN( pCurrFrm )
/*N*/ 
/*N*/     const long nXPos = pCurrFrm->IsRightToLeft() ?
/*N*/                        rLine.Right() :
/*N*/                        (rLine.*fnRect->fnGetLeft)();
/*N*/ 
/*N*/     SwRect aFly = mbIgnoreContour ?
/*N*/                   GetBoundRect( pObj ) :
/*N*/                   SwContourCache::CalcBoundRect( pObj, rLine, pCurrFrm,
/*N*/                                                  nXPos, ! pCurrFrm->IsRightToLeft() );
/*N*/ 
/*N*/     if( !aFly.Width() )
/*N*/ 		return aFly;
/*N*/ 
/*N*/     SetNextTop( (aFly.*fnRect->fnGetBottom)() ); // Damit die Zeile ggf. bis zur Unterkante
/*N*/ 								 // des Rahmens waechst.
/*N*/ 	MSHORT nFlyPos = GetPos( pObj );
/*N*/ 
/*N*/ 	// Bei LEFT und RIGHT vergroessern wir das Rechteck.
/*N*/ 	// Hier gibt es einige Probleme, wenn mehrere Frames zu sehen sind.
/*N*/ 	// Zur Zeit wird nur der einfachste Fall angenommen:
/*N*/ 	// LEFT bedeutet, dass der Text links vom Frame fliessen soll,
/*N*/ 	// d.h. der Frame blaeht sich bis zum rechten Rand der Printarea
/*N*/ 	// oder bis zum naechsten Frame auf.
/*N*/ 	// Bei RIGHT ist es umgekehrt.
/*N*/ 	// Ansonsten wird immer der eingestellte Abstand zwischen Text
/*N*/ 	// und Frame aufaddiert.
/*N*/ 	switch( GetOrder( pObj ) )
/*N*/ 	{
/*N*/ 		case SURROUND_LEFT :
/*N*/ 		{
/*N*/ 			CalcRightMargin( aFly, nFlyPos, rLine );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SURROUND_RIGHT :
/*N*/ 		{
/*N*/ 			CalcLeftMargin( aFly, nFlyPos, rLine );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SURROUND_NONE :
/*N*/ 		{
/*N*/ 			CalcRightMargin( aFly, nFlyPos, rLine );
/*N*/ 			CalcLeftMargin( aFly, nFlyPos, rLine );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aFly;
/*N*/ }


/*************************************************************************
 *						SwTxtFly::CalcSmart()
 *
 * CalcSmart() liefert die Umlaufform zurueck.
 *
 * Auf beiden Seiten ist weniger als 2 cm Platz fuer den Text
 * 	 => kein Umlauf ( SURROUND_NONE )
 * Auf genau einer Seite ist mehr als 2 cm Platz
 *   => Umlauf auf dieser Seite ( SURROUND_LEFT / SURROUND_RIGHT )
 * Auf beiden Seiten ist mehr als 2 cm Platz, das Objekt ist breiter als 1,5 cm
 * 	 => Umlauf auf der breiteren Seite ( SURROUND_LEFT / SURROUND_RIGHT )
 * Auf beiden Seiten ist mehr als 2 cm Platz, das Objekt ist schmaler als 1,5 cm
 * 	 => beidseitiger Umlauf ( SURROUND_PARALLEL	)
 *
 *************************************************************************/

// Umfluss nur auf Seiten mit mindestens 2 cm Platz fuer den Text
#define TEXT_MIN 1134
// Beidseitiger Umfluss bis zu einer Rahmenbreite von maximal 1,5 cm
#define FRAME_MAX 850

/*N*/ _FlyCntnt SwTxtFly::CalcSmart( const SdrObject *pObj ) const
/*N*/ {
/*N*/ 	_FlyCntnt eOrder;
/*N*/ 
/*N*/ 	// 11839: Nur die X-Positionen sind interessant, die Y-Positionen des
/*N*/ 	// CurrentFrames koennen sich noch aendern (wachsen).
/*N*/ 
/*N*/     SWRECTFN( pCurrFrm )
/*N*/     const long nCurrLeft = (pCurrFrm->*fnRect->fnGetPrtLeft)();
/*N*/     const long nCurrRight = (pCurrFrm->*fnRect->fnGetPrtRight)();
/*N*/ 	const SwRect aRect( GetBoundRect( pObj ) );
/*N*/     long nFlyLeft = (aRect.*fnRect->fnGetLeft)();
/*N*/     long nFlyRight = (aRect.*fnRect->fnGetRight)();
/*N*/ 
/*N*/ 	if ( nFlyRight < nCurrLeft || nFlyLeft > nCurrRight )
/*N*/ 		eOrder = SURROUND_PARALLEL;
/*N*/ 	else
/*N*/ 	{
/*N*/ #ifndef USED
/*N*/ 		long nLeft = nFlyLeft - nCurrLeft;
/*N*/ 		long nRight = nCurrRight - nFlyRight;
/*N*/ 		if( nFlyRight - nFlyLeft > FRAME_MAX )
/*N*/ 		{
/*N*/ 			if( nLeft < nRight )
/*N*/ 				nLeft = 0;
/*N*/ 			else
/*N*/ 				nRight = 0;
/*N*/ 		}
/*N*/ 		if( nLeft < TEXT_MIN )
/*N*/ 			nLeft = 0;
/*N*/ 		if( nRight < TEXT_MIN )
/*N*/ 			nRight = 0;
/*N*/ 		if( nLeft )
/*N*/ 			eOrder = nRight ? SURROUND_PARALLEL : SURROUND_LEFT;
/*N*/ 		else
/*N*/ 			eOrder = nRight ? SURROUND_RIGHT: SURROUND_NONE;
/*N*/ #else
/*N*/ 		if ( nFlyRight > nCurrRight )
/*N*/ 			nFlyRight = nCurrRight;
/*N*/ 		if ( nFlyLeft < nCurrLeft )
/*N*/ 			nFlyLeft = nCurrLeft;
/*N*/ 		const long nCurrPart = ( nCurrRight - nCurrLeft )/3;
/*N*/ 		const long nFlyWidth = nFlyRight - nFlyLeft;
/*N*/ 
/*N*/ 		if( nFlyWidth < nCurrPart )
/*N*/ 			eOrder = SURROUND_PARALLEL;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nFlyWidth > (nCurrPart * 2) )
/*N*/ 				eOrder = SURROUND_NONE;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				const long nHalfCurr = ( nCurrRight + nCurrLeft ) / 2;
/*N*/ 				const long nHalfFly  = ( nFlyRight + nFlyLeft ) / 2 ;
/*N*/ 				if ( nHalfFly == nHalfCurr )
/*N*/ 					eOrder = SURROUND_COLUMN;
/*N*/ 				else
/*N*/ 					eOrder = nHalfFly < nHalfCurr ?
/*N*/ 							 SURROUND_RIGHT : SURROUND_LEFT;
/*N*/ 			}
/*N*/ 		}
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	return eOrder;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::GetOrder()
 *************************************************************************/

/*N*/ _FlyCntnt SwTxtFly::GetOrder( const SdrObject *pObj ) const
/*N*/ {
/*N*/ 	const SwFrmFmt *pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
/*N*/ 	const SwFmtSurround &rFlyFmt = pFmt->GetSurround();
/*N*/ 	_FlyCntnt eOrder = rFlyFmt.GetSurround();
/*N*/ 
/*N*/   if( rFlyFmt.IsAnchorOnly() && &lcl_TheAnchor( pObj ) != GetMaster() )
/*N*/ 	{
/*?*/ 		const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*?*/ 		if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
/*?*/ 			FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
/*?*/ 			return SURROUND_NONE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Beim Durchlauf und Nowrap wird smart ignoriert.
/*N*/ 	if( SURROUND_THROUGHT == eOrder || SURROUND_NONE == eOrder )
/*N*/ 		return eOrder;
/*N*/ 
/*N*/     // left is left and right is right
/*N*/     if ( pCurrFrm->IsRightToLeft() )
/*N*/     {
/*?*/         if ( SURROUND_LEFT == eOrder )
/*?*/             eOrder = SURROUND_RIGHT;
/*?*/         else if ( SURROUND_RIGHT == eOrder )
/*?*/             eOrder = SURROUND_LEFT;
/*N*/     }
/*N*/ 
/*N*/ 	// "idealer Seitenumlauf":
/*N*/ 	if( SURROUND_IDEAL == eOrder )
/*N*/ 		eOrder = CalcSmart( pObj );	//Bei SMART wird die Order automatisch berechnet:
/*N*/ 
/*N*/ 	return eOrder;
/*N*/ }

/*************************************************************************
 *						SwTxtFly::IsAnyFrm( SwRect )
 *
 * IN: dokumentglobal
 *
 * dient zum Abschalten des SwTxtFly, wenn keine Objekte ueberlappen (Relax)
 *
 *************************************************************************/

/*N*/ sal_Bool SwTxtFly::IsAnyFrm( const SwRect &rLine ) const
/*N*/ {
/*N*/ 
/*N*/     SWAP_IF_SWAPPED( pCurrFrm )
/*N*/ 
/*N*/ 	ASSERT( bOn, "IsAnyFrm: Why?" );
/*N*/ 
/*N*/     const sal_Bool bRet = ForEach( rLine, NULL, sal_False );
/*N*/     UNDO_SWAP( pCurrFrm )
/*N*/     return bRet;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
