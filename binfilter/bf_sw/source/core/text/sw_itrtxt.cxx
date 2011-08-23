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

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "paratr.hxx"
#include "errhdl.hxx"


#ifdef VERTICAL_LAYOUT
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#endif

#include "txtcfg.hxx"
#include "itrtxt.hxx"
namespace binfilter {

#if OSL_DEBUG_LEVEL > 1
# include "txtfrm.hxx"      // GetFrmID,
#endif

/*************************************************************************
 *						SwTxtIter::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtIter::CtorInit( SwTxtFrm *pNewFrm, SwTxtInfo *pNewInf )
/*N*/ {
/*N*/ #ifdef DBGTXT
/*N*/ 	// nStopAt laesst sich vom CV bearbeiten.
/*N*/ 	static MSHORT nStopAt = 0;
/*N*/ 	if( nStopAt == pNewFrm->GetFrmId() )
/*N*/ 	{
/*N*/ 		int i = pNewFrm->GetFrmId();
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	SwTxtNode *pNode = pNewFrm->GetTxtNode();
/*N*/ 
/*N*/ 	ASSERT( pNewFrm->GetPara(), "No paragraph" );
/*N*/ 
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/     SwAttrIter::CtorInit( *pNode, pNewFrm->GetPara()->GetScriptInfo(), pNewFrm );
/*N*/ #else
/*N*/     SwAttrIter::CtorInit( *pNode, pNewFrm->GetPara()->GetScriptInfo() );
/*N*/ #endif
/*N*/ 
/*N*/ 	pFrm = pNewFrm;
/*N*/     pInf = pNewInf;
/*N*/ 	aLineInf.CtorInit( pNode->GetSwAttrSet() );
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/     nFrameStart = pFrm->Frm().Pos().Y() + pFrm->Prt().Pos().Y();
/*N*/ #else
/*N*/ 	aTopLeft = pFrm->Frm().Pos() + pFrm->Prt().Pos();
/*N*/ #endif
/*N*/ 	SwTxtIter::Init();
/*N*/ 	if( pNode->GetSwAttrSet().GetRegister().GetValue() )
/*N*/ 		bRegisterOn = pFrm->FillRegister( nRegStart, nRegDiff );
/*N*/ 	else
/*N*/ 		bRegisterOn = sal_False;
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::Init()
 *************************************************************************/

/*N*/ void SwTxtIter::Init()
/*N*/ {
/*N*/ 	pCurr = pInf->GetParaPortion();
/*N*/ 	nStart = pInf->GetTxtStart();
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/     nY = nFrameStart;
/*N*/ #else
/*N*/ 	nY = aTopLeft.Y();
/*N*/ #endif
/*N*/ 	bPrev = sal_True;
/*N*/ 	pPrev = 0;
/*N*/ 	nLineNr = 1;
/*N*/ }

/*************************************************************************
 *				   SwTxtIter::_GetHeightAndAscent()
 *************************************************************************/

/*N*/ void SwTxtIter::CalcAscentAndHeight( KSHORT &rAscent, KSHORT &rHeight ) const
/*N*/ {
/*N*/ 	rHeight = GetLineHeight();
/*N*/     rAscent = pCurr->GetAscent() + rHeight - pCurr->Height();
/*N*/ }

/*************************************************************************
 *					  SwTxtIter::_GetPrev()
 *************************************************************************/

/*N*/ SwLineLayout *SwTxtIter::_GetPrev()
/*N*/ {
/*N*/ 	pPrev = 0;
/*N*/ 	bPrev = sal_True;
/*N*/ 	SwLineLayout *pLay = pInf->GetParaPortion();
/*N*/ 	if( pCurr == pLay )
/*N*/ 		return 0;
/*N*/ 	while( pLay->GetNext() != pCurr )
/*N*/ 		pLay = pLay->GetNext();
/*N*/ 	return pPrev = pLay;
/*N*/ }

/*************************************************************************
 *                    SwTxtIter::GetPrev()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::GetPrev()
/*N*/ {
/*N*/ 	if(! bPrev)
/*?*/ 		_GetPrev();
/*N*/ 	return pPrev;
/*N*/ }

/*************************************************************************
 *                    SwTxtIter::Prev()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::Prev()
/*N*/ {
/*N*/ 	if( !bPrev )
/*N*/ 		_GetPrev();
/*N*/ 	if( pPrev )
/*N*/ 	{
/*N*/ 		bPrev = sal_False;
/*N*/ 		pCurr = pPrev;
/*N*/ 		nStart -= pCurr->GetLen();
/*N*/ 		nY -= GetLineHeight();
/*N*/ 		if( !pCurr->IsDummy() && !(--nLineNr) )
/*N*/ 			++nLineNr;
/*N*/ 		return pCurr;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::Next()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::Next()
/*N*/ {
/*N*/ 	if(pCurr->GetNext())
/*N*/ 	{
/*N*/ 		pPrev = pCurr;
/*N*/ 		bPrev = sal_True;
/*N*/ 		nStart += pCurr->GetLen();
/*N*/ 		nY += GetLineHeight();
/*N*/ 		if( pCurr->GetLen() || ( nLineNr>1 && !pCurr->IsDummy() ) )
/*N*/ 			++nLineNr;
/*N*/ 		return pCurr = pCurr->GetNext();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::NextLine()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::NextLine()
/*N*/ {
/*N*/ 	const SwLineLayout *pNext = Next();
/*N*/ 	while( pNext && pNext->IsDummy() && pNext->GetNext() )
/*N*/ 	{
///*?*/ 		DBG_LOOP;
/*?*/ 		pNext = Next();
/*N*/ 	}
/*N*/ 	return pNext;
/*N*/ }

/*************************************************************************
 *						SwTxtIter::GetNextLine()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::GetNextLine() const
/*N*/ {
/*N*/ 	const SwLineLayout *pNext = pCurr->GetNext();
/*N*/ 	while( pNext && pNext->IsDummy() && pNext->GetNext() )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		pNext = pNext->GetNext();
/*N*/ 	}
/*N*/ 	return (SwLineLayout*)pNext;
/*N*/ }

/*************************************************************************
 *						SwTxtIter::GetPrevLine()
 *************************************************************************/


/*************************************************************************
 *                      SwTxtIter::PrevLine()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::PrevLine()
/*N*/ {
/*N*/ 	const SwLineLayout *pPrev = Prev();
/*N*/ 	if( !pPrev )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	const SwLineLayout *pLast = pPrev;
/*N*/ 	while( pPrev && pPrev->IsDummy() )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		pLast = pPrev;
/*N*/ 		pPrev = Prev();
/*N*/ 	}
/*N*/ 	return (SwLineLayout*)(pPrev ? pPrev : pLast);
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::Bottom()
 *************************************************************************/

/*N*/ void SwTxtIter::Bottom()
/*N*/ {
/*N*/ 	while( Next() )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::CharToLine()
 *************************************************************************/
/*N*/ 
/*N*/ void SwTxtIter::CharToLine(const xub_StrLen nChar)
/*N*/ {
/*N*/ 	while( nStart + pCurr->GetLen() <= nChar && Next() )
/*N*/ 		;
/*N*/ 	while( nStart > nChar && Prev() )
/*N*/ 		;
/*N*/ }

/*************************************************************************
 *                      SwTxtIter::CharCrsrToLine()
 *************************************************************************/

// 1170: beruecksichtigt Mehrdeutigkeiten:
/*N*/ const SwLineLayout *SwTxtCursor::CharCrsrToLine( const xub_StrLen nPos )
/*N*/ {
/*N*/ 	CharToLine( nPos );
/*N*/ 	if( nPos != nStart )
/*N*/ 		bRightMargin = sal_False;
/*N*/ 	sal_Bool bPrev = bRightMargin && pCurr->GetLen() && GetPrev() &&
/*N*/ 		GetPrev()->GetLen();
/*N*/ 	if( bPrev && nPos && CH_BREAK == GetInfo().GetChar( nPos-1 ) )
/*N*/ 		bPrev = sal_False;
/*N*/ 	return bPrev ? PrevLine() : pCurr;
/*N*/ }

/*************************************************************************
 *                      SwTxtCrsr::AdjustBaseLine()
 *************************************************************************/

/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/ USHORT SwTxtCursor::AdjustBaseLine( const SwLineLayout& rLine,
/*N*/                                     const SwLinePortion* pPor,
/*N*/                                     USHORT nPorHeight, USHORT nPorAscent,
/*N*/                                     const sal_Bool bAutoToCentered ) const
/*N*/ {
/*N*/     if ( pPor )
/*N*/     {
/*?*/         nPorHeight = pPor->Height();
/*?*/         nPorAscent = pPor->GetAscent();
/*N*/     }
/*N*/ 
/*N*/     USHORT nOfst = rLine.GetRealHeight() - rLine.Height();
/*N*/ 
/*N*/     GETGRID( pFrm->FindPageFrm() )
/*N*/     const sal_Bool bHasGrid = pGrid && GetInfo().SnapToGrid();
/*N*/ 
/*N*/     if ( bHasGrid )
/*N*/     {
/*?*/         const USHORT nGridWidth = pGrid->GetBaseHeight();
/*?*/         const USHORT nRubyHeight = pGrid->GetRubyHeight();
/*?*/         const sal_Bool bRubyTop = ! pGrid->GetRubyTextBelow();
/*?*/ 
/*?*/         if ( GetInfo().IsMulti() )
/*?*/             // we are inside the GetCharRect recursion for multi portions
/*?*/             // we center the portion in its surrounding line
/*?*/             nOfst = ( pCurr->Height() - nPorHeight ) / 2 + nPorAscent;
/*?*/         else
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         {
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         switch ( GetLineInfo().GetVertAlign() ) {
/*?*/             case SvxParaVertAlignItem::TOP :
/*?*/                 nOfst += nPorAscent;
/*?*/                 break;
/*?*/             case SvxParaVertAlignItem::CENTER :
/*?*/                 ASSERT( rLine.Height() >= nPorHeight, "Portion height > Line height");
/*?*/                 nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
/*?*/                 break;
/*?*/             case SvxParaVertAlignItem::BOTTOM :
/*?*/                 nOfst += rLine.Height() - nPorHeight + nPorAscent;
/*?*/                 break;
/*N*/             case SvxParaVertAlignItem::AUTOMATIC :
/*N*/                 if ( bAutoToCentered || GetInfo().GetTxtFrm()->IsVertical() )
/*N*/                 {
/*?*/                     nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
/*?*/                     break;
/*N*/                 }
/*N*/             case SvxParaVertAlignItem::BASELINE :
/*N*/                 // base line
/*N*/                 nOfst += rLine.GetAscent();
/*N*/                 break;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return nOfst;
/*N*/ }
/*N*/ #else
/*N*/ USHORT SwTxtCursor::AdjustBaseLine( const SwLineLayout& rLine,
/*N*/                                     const USHORT nPorHeight,
/*N*/                                     const USHORT nPorAscent,
/*N*/                                     const sal_Bool bAutoToCentered ) const
/*N*/ {
/*N*/     USHORT nOfst = rLine.GetRealHeight() - rLine.Height();
/*N*/ 
/*N*/     switch ( GetLineInfo().GetVertAlign() ) {
/*N*/         case SvxParaVertAlignItem::TOP :
/*N*/             nOfst += nPorAscent;
/*N*/             break;
/*N*/         case SvxParaVertAlignItem::CENTER :
/*N*/             ASSERT( rLine.Height() >= nPorHeight, "Portion height > Line height");
/*N*/             nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
/*N*/             break;
/*N*/         case SvxParaVertAlignItem::BOTTOM :
/*N*/             nOfst += rLine.Height() - nPorHeight + nPorAscent;
/*N*/             break;
/*N*/         case SvxParaVertAlignItem::AUTOMATIC :
/*N*/             if ( bAutoToCentered )
/*N*/             {
/*N*/                 nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
/*N*/                 break;
/*N*/             }
/*N*/         case SvxParaVertAlignItem::BASELINE :
/*N*/             // base line
/*N*/             nOfst += rLine.GetAscent();
/*N*/             break;
/*N*/     }
/*N*/ 
/*N*/     return nOfst;
/*N*/ }
/*N*/ #endif

/*************************************************************************
 *                      SwTxtIter::TwipsToLine()
 *************************************************************************/

/*N*/ const SwLineLayout *SwTxtIter::TwipsToLine( const SwTwips y)
/*N*/ {
/*N*/ 	while( nY + GetLineHeight() <= y && Next() )
/*N*/ 		;
/*N*/ 	while( nY > y && Prev() )
/*N*/ 		;
/*N*/ 	return pCurr;
/*N*/ }

/*************************************************************************
 *						SwTxtIter::TruncLines()
 *************************************************************************/

/*N*/ void SwTxtIter::TruncLines( sal_Bool bNoteFollow )
/*N*/ {
/*N*/ 	SwLineLayout *pDel = pCurr->GetNext();
/*N*/     const xub_StrLen nEnd = nStart + pCurr->GetLen();
/*N*/ 
/*N*/ 	if( pDel )
/*N*/ 	{
/*N*/ 		pCurr->SetNext( 0 );
/*N*/ 		if( GetHints() && bNoteFollow )
/*N*/         {
/*N*/ 			GetInfo().GetParaPortion()->SetFollowField( pDel->IsRest() );
/*N*/ 
/*N*/             // bug 88534: wrong positioning of flys
/*N*/             SwTxtFrm* pFollow = GetTxtFrm()->GetFollow();
/*N*/             if ( pFollow && ! pFollow->IsLocked() &&
/*N*/                  nEnd == pFollow->GetOfst() )
/*N*/             {
/*N*/                 xub_StrLen nRangeEnd = nEnd;
/*N*/                 SwLineLayout* pLine = pDel;
/*N*/ 
/*N*/                 // determine range to be searched for flys anchored as characters
/*N*/                 while ( pLine )
/*N*/                 {
/*N*/                     nRangeEnd += pLine->GetLen();
/*N*/                     pLine = pLine->GetNext();
/*N*/                 }
/*N*/ 
/*N*/                 SwpHints* pHints = GetTxtFrm()->GetTxtNode()->GetpSwpHints();
/*N*/ 
/*N*/                 // examine hints in range nEnd - (nEnd + nRangeChar)
/*N*/                 for( USHORT i = 0; i < pHints->Count(); i++ )
/*N*/                 {
/*N*/                     const SwTxtAttr* pHt = pHints->GetHt( i );
/*N*/                     if( RES_TXTATR_FLYCNT == pHt->Which() )
/*N*/                     {
/*N*/                         // check, if hint is in our range
/*?*/                         const USHORT nPos = *pHt->GetStart();
/*?*/                         if ( nEnd <= nPos && nPos < nRangeEnd )
/*?*/                             pFollow->_InvalidateRange(
/*?*/                                 SwCharRange( nPos, nPos ), 0 );
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 		delete pDel;
/*N*/ 	}
/*N*/     if( pCurr->IsDummy() &&
/*N*/         !pCurr->GetLen() &&
/*N*/          nStart < GetTxtFrm()->GetTxt().Len() )
/*N*/         pCurr->SetRealHeight( 1 );
/*N*/ 	if( GetHints() )
/*N*/         pFrm->RemoveFtn( nEnd );
/*N*/ }

/*************************************************************************
 *						SwTxtIter::CntHyphens()
 *************************************************************************/

/*N*/ void SwTxtIter::CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const
/*N*/ {
/*N*/ 	nEndCnt = 0;
/*N*/ 	nMidCnt = 0;
/*N*/ 	if ( bPrev && pPrev && !pPrev->IsEndHyph() && !pPrev->IsMidHyph() )
/*N*/ 		 return;
/*N*/ 	SwLineLayout *pLay = pInf->GetParaPortion();
/*N*/ 	if( pCurr == pLay )
/*N*/ 		return;
/*N*/ 	while( pLay != pCurr )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		if ( pLay->IsEndHyph() )
/*N*/ 			nEndCnt++;
/*N*/ 		else
/*N*/ 			nEndCnt = 0;
/*N*/ 		if ( pLay->IsMidHyph() )
/*N*/ 			nMidCnt++;
/*N*/ 		else
/*N*/ 			nMidCnt = 0;
/*N*/ 		pLay = pLay->GetNext();
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *                          SwHookOut
 *
 * Change current output device to formatting device, this has to be done before
 * formatting.
 *************************************************************************/

/*N*/ SwHookOut::SwHookOut( SwTxtSizeInfo& rInfo ) :
/*N*/      pInf( &rInfo ),
/*N*/      pOut( rInfo.GetOut() ),
/*N*/      bOnWin( rInfo.OnWin() )
/*N*/ {
/*N*/     ASSERT( rInfo.GetRefDev(), "No reference device for text formatting" )
/*N*/ 
/*N*/     // set new values
/*N*/     rInfo.SetOut( rInfo.GetRefDev() );
/*N*/     rInfo.SetOnWin( sal_False );
/*N*/ }

/*N*/ SwHookOut::~SwHookOut()
/*N*/ {
/*N*/     pInf->SetOut( pOut );
/*N*/     pInf->SetOnWin( bOnWin );
/*N*/ }


}
