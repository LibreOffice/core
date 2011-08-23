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

#include <errhdl.hxx>

#include "pam.hxx"			// SwPosition

#include <horiornt.hxx>

#include "pagefrm.hxx"






#include <unicode/ubidi.h>

#include "txtcfg.hxx"
#include "itrtxt.hxx"		// SwTxtCursor
#include "crstate.hxx"		// SwTxtCursor

#if OSL_DEBUG_LEVEL > 1
#endif
namespace binfilter {

#define MIN_OFFSET_STEP 10

/*
 * 1170-SurvivalKit: Wie gelangt man hinter das letzte Zeichen der Zeile.
 * - RightMargin verzichtet auf den Positionsausgleich mit -1
 * - GetCharRect liefert bei MV_RIGHTMARGIN ein GetEndCharRect
 * - GetEndCharRect setzt bRightMargin auf sal_True
 * - SwTxtCursor::bRightMargin wird per CharCrsrToLine auf sal_False gesetzt
 */

/*************************************************************************
 *						GetAdjFrmAtPos()
 *************************************************************************/

/*N*/ SwTxtFrm *GetAdjFrmAtPos( SwTxtFrm *pFrm, const SwPosition &rPos,
/*N*/                           const sal_Bool bRightMargin, const sal_Bool bNoScroll = TRUE )
/*N*/ {
/*N*/ 	// 8810: vgl. 1170, RightMargin in der letzten Masterzeile...
/*N*/ 	const xub_StrLen nOffset = rPos.nContent.GetIndex();
/*N*/     SwTxtFrm *pFrmAtPos = pFrm;
/*N*/     if( !bNoScroll || pFrm->GetFollow() )
/*N*/     {
/*N*/         pFrmAtPos = pFrm->GetFrmAtPos( rPos );
/*N*/         if( nOffset < pFrmAtPos->GetOfst() &&
/*N*/             !pFrmAtPos->IsFollow() )
/*N*/         {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             xub_StrLen nNew = nOffset;
/*N*/         }
/*N*/     }
/*N*/ 	while( pFrm != pFrmAtPos )
/*N*/ 	{
/*?*/ 		pFrm = pFrmAtPos;
/*?*/ 		pFrm->GetFormatted();
/*?*/ 		pFrmAtPos = (SwTxtFrm*)pFrm->GetFrmAtPos( rPos );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nOffset && bRightMargin )
/*N*/ 	{
/*N*/         while( pFrmAtPos && pFrmAtPos->GetOfst() == nOffset &&
/*N*/                pFrmAtPos->IsFollow() )
/*N*/ 		{
/*?*/ 			pFrmAtPos->GetFormatted();
/*?*/ 			pFrmAtPos = pFrmAtPos->FindMaster();
/*N*/ 		}
/*N*/ 		ASSERT( pFrmAtPos, "+GetCharRect: no frame with my rightmargin" );
/*N*/ 	}
/*N*/ 	return pFrmAtPos ? pFrmAtPos : pFrm;
/*N*/ }


/*************************************************************************
 *						GetFrmAtOfst(), GetFrmAtPos()
 *************************************************************************/

/*N*/ SwTxtFrm *SwTxtFrm::GetFrmAtOfst( const xub_StrLen nWhere )
/*N*/ {
/*N*/ 	SwTxtFrm *pRet = this;
/*N*/ 	while( pRet->HasFollow() && nWhere >= pRet->GetFollow()->GetOfst() )
/*?*/ 		pRet = pRet->GetFollow();
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SwTxtFrm *SwTxtFrm::GetFrmAtPos( const SwPosition &rPos )
/*N*/ {
/*N*/ 	SwTxtFrm *pFoll = (SwTxtFrm*)this;
/*N*/ 	while( pFoll->GetFollow() )
/*N*/ 	{
/*N*/ 		if( rPos.nContent.GetIndex() > pFoll->GetFollow()->GetOfst() )
/*N*/ 			pFoll = pFoll->GetFollow();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( rPos.nContent.GetIndex() == pFoll->GetFollow()->GetOfst()
/*N*/ 				 && !SwTxtCursor::IsRightMargin() )
/*?*/ 				 pFoll = pFoll->GetFollow();
/*N*/ 			else
/*N*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFoll;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::GetCharRect()
 *************************************************************************/

/*
 * GetCharRect() findet die Characterzelle des Characters, dass
 * durch aPos beschrieben wird. GetCrsrOfst() findet den
 * umgekehrten Weg: Von einer Dokumentkoordinate zu einem Pam.
 * Beide sind virtuell in der Framebasisklasse und werden deshalb
 * immer angezogen.
 */

/*N*/ sal_Bool SwTxtFrm::GetCharRect( SwRect& rOrig, const SwPosition &rPos,
/*N*/ 							SwCrsrMoveState *pCMS ) const
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::GetCharRect with swapped frame" );
/*N*/ 
/*N*/     if( IsLocked() || IsHiddenNow() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	//Erstmal den richtigen Frm finden, dabei muss beachtet werden, dass:
/*N*/ 	//- die gecachten Informationen verworfen sein koennen (GetPara() == 0)
/*N*/ 	//- das ein Follow gemeint sein kann
/*N*/ 	//- das die Kette der Follows dynamisch waechst; der in den wir
/*N*/ 	//	schliesslich gelangen muss aber Formatiert sein.
/*N*/ 
/*N*/ 	// opt: reading ahead erspart uns ein GetAdjFrmAtPos
/*N*/ 	const sal_Bool bRightMargin = pCMS && ( MV_RIGHTMARGIN == pCMS->eState );
/*N*/     const sal_Bool bNoScroll = pCMS && pCMS->bNoScroll;
/*N*/     SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, rPos, bRightMargin,
/*N*/                                      bNoScroll );
/*N*/ 	pFrm->GetFormatted();
/*N*/ 	const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();
/*N*/ 
/*N*/     SWRECTFN ( pFrm )
/*N*/     const SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();
/*N*/     const SwTwips nFrmMaxY = (pFrm->*fnRect->fnGetPrtBottom)();
/*N*/ 
/*N*/     // nMaxY is an absolute value
/*N*/     SwTwips nMaxY = bVert ?
/*N*/                     Max( nFrmMaxY, nUpperMaxY ) :
/*N*/                     Min( nFrmMaxY, nUpperMaxY );
/*N*/ 
/*N*/     sal_Bool bRet = sal_False;
/*N*/ 
/*N*/     if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
/*N*/ 	{
/*N*/ 		Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
/*N*/ 		SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
/*N*/ 		short nFirstOffset;
/*N*/ 		pTxtNd->GetFirstLineOfsWithNum( nFirstOffset );
/*N*/ 
/*N*/         Point aPnt2;
/*N*/         if ( bVert )
/*N*/         {
/*?*/             if( nFirstOffset > 0 )
/*?*/                 aPnt1.Y() += nFirstOffset;
/*?*/ 
/*?*/             if ( aPnt1.X() < nMaxY )
/*?*/                 aPnt1.X() = nMaxY;
/*?*/             aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
/*?*/             aPnt2.Y() = aPnt1.Y();
/*?*/             if( aPnt2.X() < nMaxY )
/*?*/                 aPnt2.X() = nMaxY;
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             if( nFirstOffset > 0 )
/*N*/                 aPnt1.X() += nFirstOffset;
/*N*/ 
/*N*/             if( aPnt1.Y() > nMaxY )
/*N*/                 aPnt1.Y() = nMaxY;
/*N*/             aPnt2.X() = aPnt1.X();
/*N*/             aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
/*N*/             if( aPnt2.Y() > nMaxY )
/*N*/                 aPnt2.Y() = nMaxY;
/*N*/         }
/*N*/ 
/*N*/         rOrig = SwRect( aPnt1, aPnt2 );
/*N*/ 
/*N*/         if ( pCMS )
/*N*/ 		{
/*N*/ 			pCMS->aRealHeight.X() = 0;
/*N*/             pCMS->aRealHeight.Y() = bVert ? -rOrig.Width() : rOrig.Height();
/*N*/ 		}
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/         if ( pFrm->IsRightToLeft() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/             pFrm->SwitchLTRtoRTL( rOrig );
/*N*/ #endif
/*N*/ 
/*N*/         bRet = sal_True;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( !pFrm->HasPara() )
/*N*/ 			return sal_False;
/*N*/ 
/*N*/         SwFrmSwapper aSwapper( pFrm, sal_True );
/*N*/         if ( bVert )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/             nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );
/*N*/ 
/*N*/         sal_Bool bGoOn = sal_True;
/*N*/ 		xub_StrLen nOffset = rPos.nContent.GetIndex();
/*N*/ 		xub_StrLen nNextOfst;
/*N*/ 
/*N*/ 		do
/*N*/ 		{
/*N*/ 			{
/*N*/ 				SwTxtSizeInfo aInf( pFrm );
/*N*/ 				SwTxtCursor  aLine( pFrm, &aInf );
/*N*/ 				nNextOfst = aLine.GetEnd();
/*N*/ 				// Siehe Kommentar in AdjustFrm
/*N*/ 				// 1170: das letzte Zeichen der Zeile mitnehmen?
/*N*/ 				bRet = bRightMargin ? aLine.GetEndCharRect( &rOrig, nOffset, pCMS, nMaxY )
/*N*/ 								: aLine.GetCharRect( &rOrig, nOffset, pCMS, nMaxY );
/*N*/ 			}
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/             if ( pFrm->IsRightToLeft() )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 pFrm->SwitchLTRtoRTL( rOrig );
/*N*/ #endif
/*N*/             if ( bVert )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 pFrm->SwitchHorizontalToVertical( rOrig );
/*N*/ 
/*N*/             if( pFrm->IsUndersized() && pCMS && !pFrm->GetNext() &&
/*N*/                 (rOrig.*fnRect->fnGetBottom)() == nUpperMaxY &&
/*N*/                 pFrm->GetOfst() < nOffset &&
/*N*/                 !pFrm->IsFollow() && !bNoScroll &&
/*N*/                 pFrm->GetTxtNode()->GetTxt().Len() != nNextOfst )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 				bGoOn = lcl_ChangeOffset( pFrm, nNextOfst );
/*N*/ 			else
/*N*/ 				bGoOn = sal_False;
/*N*/ 		} while ( bGoOn );
/*N*/ 
/*N*/         if ( pCMS )
/*N*/         {
/*N*/ #ifdef BIDI
/*N*/             if ( pFrm->IsRightToLeft() )
/*N*/             {
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                  if( pCMS->b2Lines && pCMS->p2Lines)
/*N*/             }
/*N*/ #endif
/*N*/ 
/*N*/             if ( bVert )
/*N*/             {
/*?*/                 if ( pCMS->bRealHeight )
/*?*/                 {
/*?*/                     pCMS->aRealHeight.Y() = -pCMS->aRealHeight.Y();
/*?*/                     if ( pCMS->aRealHeight.Y() < 0 )
/*?*/                     {
/*?*/                         // writing direction is from top to bottom
/*?*/                         pCMS->aRealHeight.X() =  ( rOrig.Width() -
/*?*/                                                     pCMS->aRealHeight.X() +
/*?*/                                                     pCMS->aRealHeight.Y() );
/*?*/                     }
/*?*/                 }
/*?*/                 if( pCMS->b2Lines && pCMS->p2Lines)
/*?*/                 {
                        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                     pFrm->SwitchHorizontalToVertical( pCMS->p2Lines->aLine );
/*?*/                 }
/*N*/             }
/*N*/ 
/*N*/         }
/*N*/     }
/*N*/     if( bRet )
/*N*/     {
/*N*/         SwPageFrm *pPage = pFrm->FindPageFrm();
/*N*/         ASSERT( pPage, "Text esaped from page?" );
/*N*/         const SwTwips nOrigTop = (rOrig.*fnRect->fnGetTop)();
/*N*/         const SwTwips nPageTop = (pPage->Frm().*fnRect->fnGetTop)();
/*N*/         const SwTwips nPageBott = (pPage->Frm().*fnRect->fnGetBottom)();
/*N*/ 
/*N*/         // Following situation: if the frame is in an invalid sectionframe,
/*N*/         // it's possible that the frame is outside the page. If we restrict
/*N*/         // the cursor position to the page area, we enforce the formatting
/*N*/         // of the page, of the section frame and the frame himself.
/*N*/         if( (*fnRect->fnYDiff)( nPageTop, nOrigTop ) > 0 )
/*N*/             (rOrig.*fnRect->fnSetTop)( nPageTop );
/*N*/ 
/*N*/         if ( (*fnRect->fnYDiff)( nOrigTop, nPageBott ) > 0 )
/*?*/             (rOrig.*fnRect->fnSetTop)( nPageBott );
/*N*/     }
/*N*/ 
/*N*/     return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::GetAutoPos()
 *************************************************************************/

/*
 * GetAutoPos() findet die Characterzelle des Characters, dass
 * durch aPos beschrieben wird und wird von autopositionierten Rahmen genutzt.
 */

/*N*/ sal_Bool SwTxtFrm::GetAutoPos( SwRect& rOrig, const SwPosition &rPos ) const
/*N*/ {
/*N*/ 	if( IsHiddenNow() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	xub_StrLen nOffset = rPos.nContent.GetIndex();
/*N*/ 	SwTxtFrm *pFrm = ((SwTxtFrm*)this)->GetFrmAtOfst( nOffset );
/*N*/ 
/*N*/ 	pFrm->GetFormatted();
/*N*/ 	const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();
/*N*/ 
/*N*/     SWRECTFN( pTmpFrm )
/*N*/     SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();
/*N*/ 
/*N*/     // nMaxY is in absolute value
/*N*/     SwTwips nMaxY = bVert ?
/*N*/                     Max( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY ) :
/*N*/                     Min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );
/*N*/ 
/*N*/     if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
/*N*/ 	{
/*N*/ 		Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
/*N*/         Point aPnt2;
/*N*/         if ( bVert )
/*N*/         {
/*?*/             if ( aPnt1.X() < nMaxY )
/*?*/                 aPnt1.X() = nMaxY;
/*?*/             aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
/*?*/             aPnt2.Y() = aPnt1.Y();
/*?*/             if( aPnt2.X() < nMaxY )
/*?*/                 aPnt2.X() = nMaxY;
/*?*/         }
/*?*/         else
/*?*/         {
/*N*/             if( aPnt1.Y() > nMaxY )
/*N*/                 aPnt1.Y() = nMaxY;
/*N*/             aPnt2.X() = aPnt1.X();
/*N*/             aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
/*N*/             if( aPnt2.Y() > nMaxY )
/*N*/                 aPnt2.Y() = nMaxY;
/*N*/         }
/*N*/ 		rOrig = SwRect( aPnt1, aPnt2 );
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
         return FALSE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::_GetCrsrOfst()
 *************************************************************************/

// Minimaler Abstand von nichtleeren Zeilen etwas weniger als 2 cm
#define FILL_MIN_DIST 1100

struct SwFillData
{
    SwRect aFrm;
    const SwCrsrMoveState *pCMS;
    SwPosition* pPos;
    const Point& rPoint;
    SwTwips nLineWidth;
    sal_Bool bFirstLine : 1;
    sal_Bool bInner		: 1;
    sal_Bool bColumn	: 1;
    sal_Bool bEmpty		: 1;
    SwFillData( const SwCrsrMoveState *pC, SwPosition* pP, const SwRect& rR,
        const Point& rPt ) : aFrm( rR ), pCMS( pC ), pPos( pP ), rPoint( rPt ),
        nLineWidth( 0 ), bFirstLine( sal_True ), bInner( sal_False ), bColumn( sal_False ),
        bEmpty( sal_True ){}
    SwFillMode Mode() const { return pCMS->pFill->eMode; }
    long X() const { return rPoint.X(); }
    long Y() const { return rPoint.Y(); }
    long Left() const { return aFrm.Left(); }
    long Right() const { return aFrm.Right(); }
    long Bottom() const { return aFrm.Bottom(); }
    SwRect& Frm() { return aFrm; }
    SwFillCrsrPos &Fill() const { return *pCMS->pFill; }
    void SetTab( MSHORT nNew ) { pCMS->pFill->nTabCnt = nNew; }
    void SetSpace( MSHORT nNew ) { pCMS->pFill->nSpaceCnt = nNew; }
    void SetOrient( const SwHoriOrient eNew ){ pCMS->pFill->eOrient = eNew; }
};

/*N*/ sal_Bool SwTxtFrm::_GetCrsrOfst(SwPosition* pPos, const Point& rPoint,
/*N*/ 					const sal_Bool bChgFrm, const SwCrsrMoveState* pCMS ) const
/*N*/ {
/*N*/ 	// 8804: _GetCrsrOfst wird vom GetCrsrOfst und GetKeyCrsrOfst gerufen.
/*N*/ 	// In keinem Fall nur ein return sal_False.
/*N*/ 
/*N*/ 	if( IsLocked() || IsHiddenNow() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	((SwTxtFrm*)this)->GetFormatted();
/*N*/ 
/*N*/     Point aOldPoint( rPoint );
/*N*/ 
/*N*/     if ( IsVertical() )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/     if ( IsRightToLeft() )
/*?*/         {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 SwitchRTLtoLTR( (Point&)rPoint );
/*N*/ #endif
/*N*/ 
/*N*/     SwFillData *pFillData = ( pCMS && pCMS->pFill ) ?
/*N*/                         new SwFillData( pCMS, pPos, Frm(), rPoint ) : NULL;
/*N*/ 
/*N*/     if ( IsEmpty() )
/*N*/ 	{
/*N*/ 		SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
/*N*/ 		pPos->nNode = *pTxtNd;
/*N*/ 		pPos->nContent.Assign( pTxtNd, 0 );
/*N*/ 		if( pCMS && pCMS->bFieldInfo )
/*N*/ 		{
/*?*/ 			SwTwips nDiff = rPoint.X() - Frm().Left() - Prt().Left();
/*?*/ 			if( nDiff > 50 || nDiff < 0 )
/*?*/ 				((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwTxtSizeInfo aInf( (SwTxtFrm*)this );
/*N*/ 		SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );
/*N*/ 
/*N*/ 		// Siehe Kommentar in AdjustFrm()
/*N*/ 		SwTwips nMaxY = Frm().Top() + Prt().Top() + Prt().Height();
/*N*/ 		aLine.TwipsToLine( rPoint.Y() );
/*N*/ 		while( aLine.Y() + aLine.GetLineHeight() > nMaxY )
/*N*/ 		{
///*?*/ 			DBG_LOOP;
/*?*/ 			if( !aLine.Prev() )
/*?*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aLine.GetDropLines() >= aLine.GetLineNr() && 1 != aLine.GetLineNr()
/*N*/ 			&& rPoint.X() < aLine.FirstLeft() + aLine.GetDropLeft() )
/*N*/ 			while( aLine.GetLineNr() > 1 )
/*N*/ 				aLine.Prev();
/*N*/ 
/*N*/         xub_StrLen nOffset = aLine.GetCrsrOfst( pPos, rPoint, bChgFrm, pCMS );
/*N*/ 
/*N*/         if( pCMS && pCMS->eState == MV_NONE && aLine.GetEnd() == nOffset )
/*?*/             ((SwCrsrMoveState*)pCMS)->eState = MV_RIGHTMARGIN;
/*N*/ 
/*N*/ 	// 6776: pPos ist ein reiner IN-Parameter, der nicht ausgewertet werden darf.
/*N*/ 	// Das pIter->GetCrsrOfst returnt aus einer Verschachtelung mit STRING_LEN.
/*N*/ 	// Wenn SwTxtIter::GetCrsrOfst von sich aus weitere GetCrsrOfst
/*N*/ 	// ruft, so aendert sich nNode der Position. In solchen Faellen
/*N*/ 	// darf pPos nicht berechnet werden.
/*N*/ 		if( STRING_LEN != nOffset )
/*N*/ 		{
/*N*/ #ifdef USED
/*N*/ 			// 8626: bei Up/Down darf diese Zeile nicht verlassen werden.
/*N*/ 			if( pCMS && MV_UPDOWN == pCMS->eState )
/*N*/ 			{
/*N*/ 				const xub_StrLen nEnd = aLine.GetEnd();
/*N*/ 				if( nOffset >= nEnd && nEnd )
/*N*/ 				{
/*N*/ 					// Man muss hinter das letzte Zeichen kommen duerfen?!
/*N*/ 					nOffset = nEnd - 1; 				// UnitUp-Korrektur
/*N*/ 				}
/*N*/ 				else
/*N*/ 					if( nOffset < aLine.GetStart() )
/*N*/ 						nOffset = aLine.GetStart();	// UnitDown-Korrektur
/*N*/ 			}
/*N*/ #endif
/*N*/ 			SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
/*N*/ 			pPos->nNode = *pTxtNd;
/*N*/ 			pPos->nContent.Assign( pTxtNd, nOffset );
/*N*/ 			if( pFillData )
/*N*/ 			{
/*N*/ 				if( pTxtNd->GetTxt().Len() > nOffset ||
/*N*/ 					rPoint.Y() < Frm().Top() )
/*N*/ 					pFillData->bInner = sal_True;
/*N*/ 				pFillData->bFirstLine = aLine.GetLineNr() < 2;
/*N*/ 				if( pTxtNd->GetTxt().Len() )
/*N*/ 				{
/*?*/ 					pFillData->bEmpty = sal_False;
/*?*/ 					pFillData->nLineWidth = aLine.GetCurr()->Width();
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/     sal_Bool bChgFillData = sal_False;
/*N*/     if( pFillData && FindPageFrm()->Frm().IsInside( aOldPoint ) )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ 
/*N*/     if ( IsVertical() )
/*N*/     {
/*N*/         if ( bChgFillData )
/*N*/             SwitchHorizontalToVertical( pFillData->Fill().aCrsr.Pos() );
/*N*/         ((SwTxtFrm*)this)->SwapWidthAndHeight();
/*N*/     }
/*N*/ 
/*N*/     if ( IsRightToLeft() && bChgFillData )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ 
/*N*/     (Point&)rPoint = aOldPoint;
/*N*/     delete pFillData;
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *				   virtual SwTxtFrm::GetCrsrOfst()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& rPoint,
/*N*/ 							const SwCrsrMoveState* pCMS ) const
/*N*/ {
/*N*/ 	MSHORT nChgFrm = 2;
/*N*/ 	if( pCMS )
/*N*/ 	{
/*N*/ 		if( MV_UPDOWN == pCMS->eState )
/*N*/ 			nChgFrm = 0;
/*N*/ 		else if( MV_SETONLYTEXT == pCMS->eState ||
/*N*/ 				 MV_TBLSEL == pCMS->eState )
/*N*/ 			nChgFrm = 1;
/*N*/ 	}
/*N*/     return _GetCrsrOfst( pPos, rPoint, nChgFrm != 0, pCMS );
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::LeftMargin()
 *************************************************************************/

/*
 * Layout-orientierte Cursorbewegungen
 */

/*
 * an den Zeilenanfang
 */


/*************************************************************************
 *						SwTxtFrm::RightMargin()
 *************************************************************************/

/*
 * An das Zeilenende:Das ist die Position vor dem letzten
 * Character in der Zeile. Ausnahme: In der letzten Zeile soll
 * der Cursor auch hinter dem letzten Character stehen koennen,
 * um Text anhaengen zu koennen.
 *
 */


/*************************************************************************
 *						SwTxtFrm::_UnitUp()
 *************************************************************************/

//Die beiden folgenden Methoden versuchen zunaechst den Crsr in die
//nachste/folgende Zeile zu setzen. Gibt es im Frame keine vorhergehende/
//folgende Zeile, so wird der Aufruf an die Basisklasse weitergeleitet.
//Die Horizontale Ausrichtung des Crsr wird hinterher von der CrsrShell
//vorgenommen.

class SwSetToRightMargin
{
    sal_Bool bRight;
public:
    inline SwSetToRightMargin() : bRight( sal_False ) { }
    inline ~SwSetToRightMargin() { SwTxtCursor::SetRightMargin( bRight ); }
    inline void SetRight( const sal_Bool bNew ) { bRight = bNew; }
};


//
// Used for Bidi. nPos is the logical position in the string, bLeft indicates
// if left arrow or right arrow was pressed. The return values are:
// nPos: the new visual position
// bLeft: whether the break iterator has to add or subtract from the
//          current position


/*************************************************************************
 *						SwTxtFrm::_UnitDown()
 *************************************************************************/


/*************************************************************************
 *					 virtual SwTxtFrm::UnitUp()
 *************************************************************************/


/*************************************************************************
 *					 virtual SwTxtFrm::UnitDown()
 *************************************************************************/




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
