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

#include "dcontact.hxx"	// SwDrawContact
#include "dflyobj.hxx"	// SwVirtFlyDrawObj
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#include "pam.hxx"		// SwPosition

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "frmfmt.hxx"	// SwFrmFmt

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "flyfrms.hxx"
#include "txatbase.hxx"	// SwTxtAttr
#include "porfly.hxx"
#include "inftxt.hxx"	// SwTxtPaintInfo
namespace binfilter {

/*************************************************************************
 *				  class SwFlyPortion
 *
 * Wir erwarten ein framelokales SwRect !
 *************************************************************************/


/*************************************************************************
 *				   virtual SwFlyPortion::Format()
 *************************************************************************/
/*N*/ sal_Bool SwFlyPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	ASSERT( Fix() >= rInf.X(), "SwFlyPortion::Format: rush hour" );
/*N*/ 	// 8537: Tabs muessen expandiert werden.
/*N*/ 	if( rInf.GetLastTab() )
/*?*/ 		((SwLinePortion*)rInf.GetLastTab())->FormatEOL( rInf );
/*N*/ 
/*N*/ 	// Der Glue wird aufgespannt.
/*N*/ 	rInf.GetLast()->FormatEOL( rInf );
/*N*/ #ifdef USED
/*N*/ 	long nFirstDiff;
/*N*/ 
/*N*/ 	if( !Fix() )
/*N*/ 	{
/*N*/ 		nFirstDiff = rInf.Left() - long( rInf.First() );
/*N*/ 		if( rInf.GetLineStart() )
/*N*/ 		{
/*N*/ 			if( nFirstDiff < 0 )
/*N*/ 				nFirstDiff = 0;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nFirstDiff > 0 )
/*N*/ 				nFirstDiff = 0;
/*N*/ 			else
/*N*/ 				nFirstDiff = -nFirstDiff;
/*N*/ 		}
/*N*/ 		nFirstDiff += rInf.GetTxtFrm()->Prt().Left();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nFirstDiff = 0;
/*N*/ 	PrtWidth( (Fix() - rInf.X()) + PrtWidth() + nFirstDiff );
/*N*/ #else
/*N*/ 	PrtWidth( (Fix() - rInf.X()) + PrtWidth() );
/*N*/ #endif
/*N*/ 	if( !Width() )
/*N*/ 	{
/*N*/ 		ASSERT( Width(), "+SwFlyPortion::Format: a fly is a fly is a fly" );
/*N*/ 		Width(1);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Restaurierung
/*N*/ 	rInf.SetFly( 0 );
/*N*/ 	rInf.Width( rInf.RealWidth() );
/*N*/ 	rInf.GetParaPortion()->SetFly( sal_True );
/*N*/ 
/*N*/ 	// trailing blank:
/*N*/ 	if( rInf.GetIdx() < rInf.GetTxt().Len() && 	1 < rInf.GetIdx()
/*N*/ 		&& !rInf.GetRest()
/*N*/ 		&& ' ' == rInf.GetChar( rInf.GetIdx() )
/*N*/ 		&& ' ' != rInf.GetChar( rInf.GetIdx() - 1 )
/*N*/ 		&& ( !rInf.GetLast() || !rInf.GetLast()->IsBreakPortion() ) )
/*N*/ 	{
/*N*/ 		SetBlankWidth( rInf.GetTxtSize( ' ' ).Width() );
/*N*/ 		SetLen( 1 );
/*N*/ 	}
/*N*/ 
/*N*/ 	const KSHORT nNewWidth = rInf.X() + PrtWidth();
/*N*/ 	if( rInf.Width() <= nNewWidth )
/*N*/ 	{
/*N*/ 		Truncate();
/*N*/ 		if( nNewWidth > rInf.Width() )
/*N*/ 		{
/*N*/ 			PrtWidth( nNewWidth - rInf.Width() );
/*N*/ 			SetFixWidth( PrtWidth() );
/*N*/ 		}
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 *				   virtual SwFlyCntPortion::Format()
 *************************************************************************/
/*M*/ sal_Bool SwFlyCntPortion::Format( SwTxtFormatInfo &rInf )
/*M*/ {
/*M*/ 	sal_Bool bFull = rInf.Width() < rInf.X() + PrtWidth();
/*M*/ 
/*M*/ 	if( bFull )
/*M*/ 	{
/*M*/ 		// 3924: wenn die Zeile voll ist und der zeichengebundene Frame am
/*M*/ 		// Anfang der Zeile steht.
/*M*/ 		// 5157: nicht wenn einem Fly ausgewichen werden kann!
/*M*/         // "Begin of line" criteria ( ! rInf.X() ) has to be extended.
/*M*/         // KerningPortions at beginning of line, e.g., for grid layout
/*M*/         // must be considered.
/*M*/         const SwLinePortion* pLastPor = rInf.GetLast();
/*M*/         const USHORT nLeft = ( pLastPor &&
/*M*/                                     ( pLastPor->IsKernPortion() ||
/*M*/                                       pLastPor->IsErgoSumPortion() ) ) ?
/*M*/                                pLastPor->Width() :
/*M*/                                0;
/*M*/ 
/*M*/         if( nLeft == rInf.X() && ! rInf.GetFly() )
/*M*/ 		{
/*M*/ 			Width( rInf.Width() );
/*M*/ 			bFull = sal_False; // Damit Notizen noch in dieser Zeile landen
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			if( !rInf.GetFly() )
/*M*/ 				rInf.SetNewLine( sal_True );
/*M*/ 			Width(0);
/*M*/ 			SetAscent(0);
/*M*/ 			SetLen(0);
/*M*/             if( rInf.GetLast() )
/*M*/                 rInf.GetLast()->FormatEOL( rInf );
/*M*/ 
/*M*/ 			return bFull;
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	rInf.GetParaPortion()->SetFly( sal_True );
/*M*/ 	return bFull;
/*M*/ }

/*************************************************************************
 *	SwTxtFrm::MoveFlyInCnt() haengt jetzt die zeichengebundenen Objekte
 *  innerhalb des angegebenen Bereichs um, damit koennen diese vom Master
 *  zum Follow oder umgekehrt wandern.
 *************************************************************************/
/*N*/ void SwTxtFrm::MoveFlyInCnt( SwTxtFrm *pNew, xub_StrLen nStart, xub_StrLen nEnd )
/*N*/ {
/*N*/ 	SwDrawObjs *pObjs;
/*N*/ 	if ( 0 != (pObjs = GetDrawObjs()) )
/*N*/ 	{
/*N*/ 		for ( int i = 0; GetDrawObjs() && i < int(pObjs->Count()); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pO = (*pObjs)[MSHORT(i)];
/*N*/ 			if ( pO->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 				if( pFly->IsFlyInCntFrm() )
/*N*/ 				{
/*?*/ 					const SwFmtAnchor &rAnch = pFly->GetFmt()->GetAnchor();
/*?*/ 					const SwPosition *pPos = rAnch.GetCntntAnchor();
/*?*/ 					xub_StrLen nIdx = pPos->nContent.GetIndex();
/*?*/ 					if ( nIdx >= nStart && nEnd > nIdx )
/*?*/ 					{
/*?*/ 						RemoveFly( pFly );
/*?*/ 						pNew->AppendFly( pFly );
/*?*/ 						--i;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwDrawContact *pContact = (SwDrawContact*)GetUserCall(pO);
/*N*/ 				const SwFmtAnchor &rAnch = pContact->GetFmt()->GetAnchor();
/*N*/ 				if ( FLY_IN_CNTNT == rAnch.GetAnchorId() )
/*N*/ 				{
/*?*/ 					const SwPosition *pPos = rAnch.GetCntntAnchor();
/*?*/ 					xub_StrLen nIdx = pPos->nContent.GetIndex();
/*?*/ 					if ( nIdx >= nStart && nEnd > nIdx )
/*?*/ 					{
/*?*/ 						RemoveDrawObj( pContact );
/*?*/ 						pNew->AppendDrawObj( pContact );
/*?*/ 						--i;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *				  SwTxtFrm::CalcFlyPos()
 *************************************************************************/
/*N*/ xub_StrLen SwTxtFrm::CalcFlyPos( SwFrmFmt* pSearch )
/*N*/ {
/*N*/ 	SwpHints* pHints = GetTxtNode()->GetpSwpHints();
/*N*/ 	ASSERT( pHints, "CalcFlyPos: Why me?" );
/*N*/ 	if( !pHints )
/*N*/ 		return STRING_LEN;
/*N*/ 	SwTxtAttr* pFound = NULL;
/*N*/ 	for( MSHORT i = 0; i < pHints->Count(); i++)
/*N*/ 	{
/*N*/ 		SwTxtAttr *pHt = pHints->GetHt( i );
/*N*/ 		if( RES_TXTATR_FLYCNT == pHt->Which() )
/*N*/ 		{
/*N*/ 			SwFrmFmt* pFrmFmt = pHt->GetFlyCnt().GetFrmFmt();
/*N*/ 			if( pFrmFmt == pSearch )
/*N*/ 				pFound = pHt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	ASSERT( pHints, "CalcFlyPos: Not Found!" );
/*N*/ 	if( !pFound )
/*N*/ 		return STRING_LEN;
/*N*/ 	return *pFound->GetStart();
/*N*/ }

/*************************************************************************
 *				   virtual SwFlyCntPortion::Paint()
 *************************************************************************/

/*************************************************************************
 *					SwFlyCntPortion::SwFlyCntPortion()
 *
 * Es werden die Masze vom pFly->OutRect() eingestellt.
 * Es erfolgt ein SetBase() !
 *************************************************************************/
/*N*/ SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
/*N*/                                   SwFlyInCntFrm *pFly, const Point &rBase,
/*N*/                                   long nLnAscent, long nLnDescent,
/*N*/                                   long nFlyAsc, long nFlyDesc, sal_uInt8 nFlags ) :
/*N*/ 	pContact( pFly ),
/*N*/ 	bDraw( sal_False ),
/*N*/ 	bMax( sal_False ),
/*N*/ 	nAlign( 0 )
/*N*/ {
/*N*/ 	ASSERT( pFly, "SwFlyCntPortion::SwFlyCntPortion: no SwFlyInCntFrm!" );
/*N*/ 	nLineLength = 1;
/*N*/ 	nFlags |= SETBASE_ULSPACE | SETBASE_INIT;
/*N*/     SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
/*N*/ 	SetWhichPor( POR_FLYCNT );
/*N*/ }

/*N*/ SwFlyCntPortion::SwFlyCntPortion( const SwTxtFrm& rFrm,
/*N*/                                   SwDrawContact *pDrawContact, const Point &rBase,
/*N*/                                   long nLnAscent, long nLnDescent, long nFlyAsc,
/*N*/                                   long nFlyDesc, sal_uInt8 nFlags ) :
/*N*/ 	pContact( pDrawContact ),
/*N*/ 	bDraw( sal_True ),
/*N*/ 	bMax( sal_False ),
/*N*/ 	nAlign( 0 )
/*N*/ {
/*N*/ 	ASSERT( pDrawContact, "SwFlyCntPortion::SwFlyCntPortion: no SwDrawContact!" );
/*N*/ 	if( !pDrawContact->GetAnchor() )
/*N*/ 	{
/*N*/ 		if( nFlags & SETBASE_QUICK )
/*N*/ 		{
/*N*/ 			Point aAnchorPos = pDrawContact->GetMaster()->GetAnchorPos();
/*N*/ 			pDrawContact->ConnectToLayout();
/*N*/ 			pDrawContact->GetMaster()->SetAnchorPos( aAnchorPos );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pDrawContact->ConnectToLayout();
/*N*/ 	}
/*N*/ 	nLineLength = 1;
/*N*/ 	nFlags |= SETBASE_ULSPACE | SETBASE_INIT;
/*N*/ 
/*N*/     SetBase( rFrm, rBase, nLnAscent, nLnDescent, nFlyAsc, nFlyDesc, nFlags );
/*N*/ 
/*N*/     SetWhichPor( POR_FLYCNT );
/*N*/ }

/*N*/ const SwFrmFmt *SwFlyCntPortion::GetFrmFmt() const
/*N*/ {
/*N*/ 	if( bDraw )
/*N*/ 		return GetDrawContact()->GetFmt();
/*N*/ 	else
/*N*/ 		return GetFlyFrm()->GetFmt();
/*N*/ }

/*************************************************************************
 *					SwFlyCntPortion::SetBase()
 *
 * Nach dem Setzen des RefPoints muss der Ascent neu berechnet werden,
 * da er von der RelPos abhaengt.
 * pFly->GetRelPos().Y() bezeichnet die relative Position zur Baseline.
 * Bei 0 liegt der obere Rand des FlyCnt auf der Baseline der Zeile.
 *************************************************************************/

/*N*/ void SwFlyCntPortion::SetBase( const SwTxtFrm& rFrm, const Point &rBase,
/*N*/                                long nLnAscent, long nLnDescent, long nFlyAsc,
/*N*/                                long nFlyDesc, sal_uInt8 nFlags )
/*N*/ {
/*N*/     // Note: rBase is an absolute value
/*N*/     SWAP_IF_SWAPPED( (&rFrm ) )
/*N*/     SWRECTFN( (&rFrm ) )
/*N*/ 	Point aBase( rBase );
/*N*/ 	const SwFrmFmt* pFmt = GetFrmFmt();
/*N*/ 	const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
/*N*/ 	const SwVertOrient eOri = rVert.GetVertOrient();
/*N*/ 	const SvxLRSpaceItem &rLRSpace = pFmt->GetLRSpace();
/*N*/ 	const SvxULSpaceItem &rULSpace = pFmt->GetULSpace();
/*N*/ 
/*N*/ 	//Die vertikale Position wird berechnet, die relative horizontale
/*N*/ 	//Position ist stets 0.
/*N*/ 
/*N*/ 	SdrObject *pSdrObj;
/*N*/ 	SwRect aBoundRect;
/*N*/ 	long nOldWidth;
/*N*/ 	if( bDraw )
/*N*/ 	{
/*N*/         // OD 20.06.2003 #108784# - determine drawing object ('master' or 'virtual')
/*N*/         // by frame.
/*N*/         pSdrObj = GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm );
/*N*/         if ( !pSdrObj )
/*N*/         {
/*N*/             ASSERT( false, "SwFlyCntPortion::SetBase(..) - No drawing object found by <GetDrawContact()->GetDrawObjectByAnchorFrm( rFrm )>" );
/*N*/             pSdrObj = GetDrawContact()->GetMaster();
/*N*/         }
/*N*/ 		aBoundRect = pSdrObj->GetBoundRect();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aBoundRect = GetFlyFrm()->Frm();
/*N*/ 		nOldWidth = aBoundRect.Width();
/*N*/ 	}
/*N*/ 
/*N*/     nOldWidth = (aBoundRect.*fnRect->fnGetWidth)();
/*N*/ 
/*N*/     long nLRSpaceLeft, nLRSpaceRight, nULSpaceUpper, nULSpaceLower;
/*N*/     if ( rFrm.IsVertical() )
/*N*/     {
/*N*/         // Seems to be easier to do it all the horizontal way
/*N*/         // So, from now on think horizontal.
/*N*/         rFrm.SwitchVerticalToHorizontal( aBoundRect );
/*N*/         rFrm.SwitchVerticalToHorizontal( aBase );
/*N*/ 
/*N*/         // convert the spacing values
/*N*/         nLRSpaceLeft = rULSpace.GetUpper();
/*N*/         nLRSpaceRight = rULSpace.GetLower();
/*N*/         nULSpaceUpper = rLRSpace.GetRight();
/*N*/         nULSpaceLower = rLRSpace.GetLeft();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         if ( rFrm.IsRightToLeft() )
/*N*/         {
/*N*/             nLRSpaceLeft = rLRSpace.GetRight();
/*N*/             nLRSpaceRight = rLRSpace.GetLeft();
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             nLRSpaceLeft = rLRSpace.GetLeft();
/*N*/             nLRSpaceRight = rLRSpace.GetRight();
/*N*/         }
/*N*/ 
/*N*/         nULSpaceUpper = rULSpace.GetUpper();
/*N*/         nULSpaceLower = rULSpace.GetLower();
/*N*/     }
/*N*/ 
/*N*/     if( nFlags & SETBASE_ULSPACE )
/*N*/         aBase.X() += nLRSpaceLeft;
/*N*/     aBase.Y() += nULSpaceUpper;
/*N*/ 
/*N*/     if( bDraw )
/*N*/ 	{
/*N*/         SwRect aSnapRect = pSdrObj->GetSnapRect();
/*N*/         if ( rFrm.IsVertical() )
/*N*/             rFrm.SwitchVerticalToHorizontal( aSnapRect );
/*N*/ 
/*N*/         if( nFlags & SETBASE_ULSPACE )
/*N*/             aBase.X() += aSnapRect.Left() - aBoundRect.Left();
/*N*/         aBase.Y() += aSnapRect.Top() - aBoundRect.Top();
/*N*/     }
/*N*/ 
/*N*/     aBoundRect.Left( aBoundRect.Left() - nLRSpaceLeft );
/*N*/     aBoundRect.Width( aBoundRect.Width() + nLRSpaceRight );
/*N*/     aBoundRect.Top( aBoundRect.Top() - nULSpaceUpper );
/*N*/     aBoundRect.Height( aBoundRect.Height() + nULSpaceLower );
/*N*/ 
/*N*/ 	SwTwips nBoundHeight = ( nFlags & SETBASE_ROTATE ) ?
/*N*/ 							aBoundRect.Width() : aBoundRect.Height();
/*N*/ 	SwTwips nRelPos = 0;
/*N*/ 	if ( eOri == VERT_NONE )
/*N*/ 		nRelPos = rVert.GetPos();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nRelPos = 0;
/*N*/ 		if ( eOri == VERT_CENTER )
/*N*/ 			nRelPos -= nBoundHeight /  2;
/*N*/ 		else if ( eOri == VERT_TOP )
/*N*/ 			nRelPos -= nBoundHeight;
/*N*/ 		else if ( eOri == VERT_BOTTOM )
/*N*/ 			;
/*N*/ 		else if ( eOri == VERT_CHAR_CENTER )
/*N*/ 			nRelPos -= ( nBoundHeight + nLnAscent - nLnDescent ) / 2;
/*N*/ 		else if ( eOri == VERT_CHAR_TOP )
/*N*/ 			nRelPos -= nLnAscent;
/*N*/ 		else if ( eOri == VERT_CHAR_BOTTOM )
/*N*/ 			nRelPos += nLnDescent - nBoundHeight;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nBoundHeight >= nFlyAsc + nFlyDesc )
/*N*/ 			{
/*N*/ 				// wenn ich genauso gross bin wie die Zeile, brauche ich mich
/*N*/ 				// nicht an der Zeile nicht weiter ausrichten, ich lasse
/*N*/ 				// dann auch den max. Ascent der Zeile zunaechst unveraendert
/*N*/ 				nRelPos -= nFlyAsc;
/*N*/ 				if ( eOri == VERT_LINE_CENTER )
/*N*/ 					SetAlign( 2 );
/*N*/ 				else if ( eOri == VERT_LINE_TOP )
/*N*/ 					SetAlign( 1 );
/*N*/ 				else if ( eOri == VERT_LINE_BOTTOM )
/*N*/ 					SetAlign( 3 );
/*N*/ 			}
/*N*/ 			else if ( eOri == VERT_LINE_CENTER )
/*N*/ 			{
/*N*/ 				nRelPos -= ( nBoundHeight +nFlyAsc -nFlyDesc ) / 2;
/*N*/ 				SetAlign( 2 );
/*N*/ 			}
/*N*/ 			else if ( eOri == VERT_LINE_TOP )
/*N*/ 			{
/*N*/ 				nRelPos -= nFlyAsc;
/*N*/ 				SetAlign( 1 );
/*N*/ 			}
/*N*/ 			else if ( eOri == VERT_LINE_BOTTOM )
/*N*/ 			{
/*N*/ 				nRelPos += nFlyDesc - nBoundHeight;
/*N*/ 				SetAlign( 3 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nFlags & SETBASE_INIT && nRelPos < 0 && nFlyAsc < -nRelPos )
/*N*/ 	{
/*N*/ 		if( nFlags & SETBASE_ROTATE )
/*N*/ 			aBase.X() -= nFlyAsc + nRelPos;
/*N*/ 		else
/*N*/ 			aBase.Y() -= nFlyAsc + nRelPos;
/*N*/ 	}
/*N*/ 
/*N*/     if( nFlags & SETBASE_BIDI )
/*N*/         aBase.X() -= aBoundRect.Width();
/*N*/ 
/*N*/ 	Point aRelPos;
/*N*/ 	if( nFlags & SETBASE_ROTATE )
/*N*/ 	{
/*N*/ 		if( nFlags & SETBASE_REVERSE )
/*N*/             aRelPos.X() = -nRelPos - aBoundRect.Width();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aRelPos.X() = nRelPos;
/*N*/ 			aRelPos.Y() = -aBoundRect.Height();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aRelPos.Y() = nRelPos;
/*N*/ 	if( bDraw )
/*N*/ 	{
/*N*/ 		if( !( nFlags & SETBASE_QUICK ) )
/*N*/ 		{
/*N*/ 			if( rVert.GetPos() != nRelPos && eOri != VERT_NONE )
/*N*/ 			{
/*N*/ 				// Das aRelPos wird gepflegt, weil sonst SwDrawContact::_Changed
/*N*/ 				// auf die Idee kommen koennte, auf VERT_NONE umzuschalten.
/*N*/ 				SwFmtVertOrient aVert( rVert );
/*N*/ 				aVert.SetPos( nRelPos );
/*N*/ 				((SwFrmFmt*)pFmt)->LockModify();
/*N*/ 				((SwFrmFmt*)pFmt)->SetAttr( aVert );
/*N*/ 				((SwFrmFmt*)pFmt)->UnlockModify();
/*N*/ 			}
/*N*/             Point aAnchorBase( aBase );
/*N*/             if ( rFrm.IsRightToLeft() )
/*N*/             {
/*N*/                 rFrm.SwitchLTRtoRTL( aAnchorBase );
/*N*/                 aAnchorBase.X() -= nOldWidth;
/*N*/             }
/*N*/             if ( rFrm.IsVertical() )
/*N*/                 rFrm.SwitchHorizontalToVertical( aAnchorBase );
/*N*/ 
/*N*/             // OD 20.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/             if ( pSdrObj->ISA(SwDrawVirtObj) )
/*N*/             {
/*N*/                 SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pSdrObj);
/*N*/ 
/*N*/                 pDrawVirtObj->NbcSetAnchorPos( aAnchorBase );
/*N*/                 pDrawVirtObj->AdjustRelativePosToReference();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // There used to be a ImpSetAnchorPos here. Very dangerous
/*N*/                 // for group object.
/*N*/                 pSdrObj->NbcSetAnchorPos( aAnchorBase );
/*N*/                 // OD 20.06.2003 #108784# - correct movement of 'virtual' drawing
/*N*/                 // objects caused by the <SetAnchorPos(..)> of the 'master' drawing object.
/*N*/                 GetDrawContact()->CorrectRelativePosOfVirtObjs();
/*N*/             }
/*N*/ 
/*N*/             SwRect aSnapRect = pSdrObj->GetSnapRect();
/*N*/ 
/*N*/             if ( rFrm.IsVertical() )
/*N*/                 rFrm.SwitchVerticalToHorizontal( aSnapRect );
/*N*/ 
/*N*/             Point aDiff;
/*N*/             if ( rFrm.IsRightToLeft() )
/*N*/                 aDiff = aRelPos + aAnchorBase - aSnapRect.TopLeft();
/*N*/             else
/*N*/                 aDiff = aRelPos + aBase - aSnapRect.TopLeft();
/*N*/ 
/*N*/             if ( rFrm.IsVertical() )
/*N*/                 aDiff = Point( -aDiff.Y(), aDiff.X() );
/*N*/ 
/*N*/             // OD 20.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/             if ( !pSdrObj->ISA(SwDrawVirtObj) )
/*N*/             {
/*N*/                 // #80046# here a Move() is necessary, a NbcMove() is NOT ENOUGH(!)
/*N*/                 pSdrObj->Move( Size( aDiff.X(), aDiff.Y() ) );
/*N*/                 // OD 23.06.2003 #108784# - correct movement of 'virtual' drawing
/*N*/                 // objects caused by the <Move(..)> of the 'master' drawing object
/*N*/                 GetDrawContact()->MoveOffsetOfVirtObjs( Size( -aDiff.X(), -aDiff.Y() ) );
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         if ( rFrm.IsVertical() )
/*N*/             rFrm.SwitchHorizontalToVertical( aBase );
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         Point aRelAttr;
/*N*/         if ( rFrm.IsRightToLeft() )
/*N*/         {
/*N*/             rFrm.SwitchLTRtoRTL( aBase );
/*N*/             aBase.X() -= nOldWidth;
/*N*/         }
/*N*/         if ( rFrm.IsVertical() )
/*N*/         {
/*N*/             rFrm.SwitchHorizontalToVertical( aBase );
/*N*/             aRelAttr = Point( -nRelPos, 0 );
/*N*/             aRelPos = Point( -aRelPos.Y(), aRelPos.X() );
/*N*/         }
/*N*/         else
/*N*/             aRelAttr = Point( 0, nRelPos );
/*N*/ 
/*N*/ 		if ( !(nFlags & SETBASE_QUICK) && (aBase != GetFlyFrm()->GetRefPoint() ||
/*N*/ 						 aRelAttr != GetFlyFrm()->GetCurRelPos()) )
/*N*/ 		{
/*N*/ 			GetFlyFrm()->SetRefPoint( aBase, aRelAttr, aRelPos );
/*N*/             if( nOldWidth != (GetFlyFrm()->Frm().*fnRect->fnGetWidth)() )
/*N*/ 			{
/*N*/ 				aBoundRect = GetFlyFrm()->Frm();
/*N*/ 				aBoundRect.Left( aBoundRect.Left() - rLRSpace.GetLeft() );
/*N*/ 				aBoundRect.Width( aBoundRect.Width() + rLRSpace.GetRight() );
/*N*/ 				aBoundRect.Top( aBoundRect.Top() - rULSpace.GetUpper() );
/*N*/ 				aBoundRect.Height( aBoundRect.Height() + rULSpace.GetLower() );
/*N*/ 			}
/*N*/ 		}
/*N*/         ASSERT( (GetFlyFrm()->Frm().*fnRect->fnGetHeight)(),
/*N*/ 			"SwFlyCntPortion::SetBase: flyfrm has an invalid height" );
/*N*/ 	}
/*N*/ 	aRef = aBase;
/*N*/ 	if( nFlags & SETBASE_ROTATE )
/*N*/ 		SvXSize( aBoundRect.SSize() );
/*N*/ 	else
/*N*/ 		SvLSize( aBoundRect.SSize() );
/*N*/ 	if( Height() )
/*N*/ 	{
/*N*/ 		if ( nRelPos < 0 )
/*N*/ 		{
/*N*/ 			nAscent = Abs( int( nRelPos ) );
/*N*/ 			if( nAscent > Height() )
/*N*/ 				Height( nAscent );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nAscent = 0;
/*N*/ 			Height( Height() + int( nRelPos ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		Height( 1 );
/*N*/ 		nAscent = 0;
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( ( &rFrm ) )
/*N*/ }

/*************************************************************************
 *				virtual SwFlyCntPortion::GetFlyCrsrOfst()
 *************************************************************************/


/*************************************************************************
 *				virtual SwFlyCntPortion::GetCrsrOfst()
 *************************************************************************/


}
