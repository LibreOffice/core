/*************************************************************************
 *
 *  $RCSfile: itrtxt.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:54:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "ndtxt.hxx"
#include "flyfrm.hxx"
#include "paratr.hxx"
#include "errhdl.hxx"

#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _SVX_PARAVERTALIGNITEM_HXX //autogen
#include <svx/paravertalignitem.hxx>
#endif

#ifdef VERTICAL_LAYOUT
#include "pormulti.hxx"
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
#include "txtfrm.hxx"
#include "porfly.hxx"

#if OSL_DEBUG_LEVEL > 1
# include "txtfrm.hxx"      // GetFrmID,
#endif

/*************************************************************************
 *                      SwTxtIter::CtorInit()
 *************************************************************************/

void SwTxtIter::CtorInit( SwTxtFrm *pNewFrm, SwTxtInfo *pNewInf )
{
#ifdef DBGTXT
    // nStopAt laesst sich vom CV bearbeiten.
    static MSHORT nStopAt = 0;
    if( nStopAt == pNewFrm->GetFrmId() )
    {
        int i = pNewFrm->GetFrmId();
    }
#endif

    SwTxtNode *pNode = pNewFrm->GetTxtNode();

    ASSERT( pNewFrm->GetPara(), "No paragraph" );

#ifdef VERTICAL_LAYOUT
    SwAttrIter::CtorInit( *pNode, pNewFrm->GetPara()->GetScriptInfo(), pNewFrm );
#else
    SwAttrIter::CtorInit( *pNode, pNewFrm->GetPara()->GetScriptInfo() );
#endif

    pFrm = pNewFrm;
    pInf = pNewInf;
    aLineInf.CtorInit( pNode->GetSwAttrSet() );
#ifdef VERTICAL_LAYOUT
    nFrameStart = pFrm->Frm().Pos().Y() + pFrm->Prt().Pos().Y();
#else
    aTopLeft = pFrm->Frm().Pos() + pFrm->Prt().Pos();
#endif
    SwTxtIter::Init();
    if( pNode->GetSwAttrSet().GetRegister().GetValue() )
        bRegisterOn = pFrm->FillRegister( nRegStart, nRegDiff );
    else
        bRegisterOn = sal_False;
}

/*************************************************************************
 *                      SwTxtIter::Init()
 *************************************************************************/

void SwTxtIter::Init()
{
    pCurr = pInf->GetParaPortion();
    nStart = pInf->GetTxtStart();
#ifdef VERTICAL_LAYOUT
    nY = nFrameStart;
#else
    nY = aTopLeft.Y();
#endif
    bPrev = sal_True;
    pPrev = 0;
    nLineNr = 1;
}

/*************************************************************************
 *                 SwTxtIter::_GetHeightAndAscent()
 *************************************************************************/

void SwTxtIter::CalcAscentAndHeight( KSHORT &rAscent, KSHORT &rHeight ) const
{
    rHeight = GetLineHeight();
    rAscent = pCurr->GetAscent() + rHeight - pCurr->Height();
}

/*************************************************************************
 *                    SwTxtIter::_GetPrev()
 *************************************************************************/

SwLineLayout *SwTxtIter::_GetPrev()
{
    pPrev = 0;
    bPrev = sal_True;
    SwLineLayout *pLay = pInf->GetParaPortion();
    if( pCurr == pLay )
        return 0;
    while( pLay->GetNext() != pCurr )
        pLay = pLay->GetNext();
    return pPrev = pLay;
}

/*************************************************************************
 *                    SwTxtIter::GetPrev()
 *************************************************************************/

const SwLineLayout *SwTxtIter::GetPrev()
{
    if(! bPrev)
        _GetPrev();
    return pPrev;
}

/*************************************************************************
 *                    SwTxtIter::Prev()
 *************************************************************************/

const SwLineLayout *SwTxtIter::Prev()
{
    if( !bPrev )
        _GetPrev();
    if( pPrev )
    {
        bPrev = sal_False;
        pCurr = pPrev;
        nStart -= pCurr->GetLen();
        nY -= GetLineHeight();
        if( !pCurr->IsDummy() && !(--nLineNr) )
            ++nLineNr;
        return pCurr;
    }
    else
        return 0;
}

/*************************************************************************
 *                      SwTxtIter::Next()
 *************************************************************************/

const SwLineLayout *SwTxtIter::Next()
{
    if(pCurr->GetNext())
    {
        pPrev = pCurr;
        bPrev = sal_True;
        nStart += pCurr->GetLen();
        nY += GetLineHeight();
        if( pCurr->GetLen() || ( nLineNr>1 && !pCurr->IsDummy() ) )
            ++nLineNr;
        return pCurr = pCurr->GetNext();
    }
    else
        return 0;
}

/*************************************************************************
 *                      SwTxtIter::NextLine()
 *************************************************************************/

const SwLineLayout *SwTxtIter::NextLine()
{
    const SwLineLayout *pNext = Next();
    while( pNext && pNext->IsDummy() && pNext->GetNext() )
    {
        DBG_LOOP;
        pNext = Next();
    }
    return pNext;
}

/*************************************************************************
 *                      SwTxtIter::GetNextLine()
 *************************************************************************/

const SwLineLayout *SwTxtIter::GetNextLine() const
{
    const SwLineLayout *pNext = pCurr->GetNext();
    while( pNext && pNext->IsDummy() && pNext->GetNext() )
    {
        DBG_LOOP;
        pNext = pNext->GetNext();
    }
    return (SwLineLayout*)pNext;
}

/*************************************************************************
 *                      SwTxtIter::GetPrevLine()
 *************************************************************************/

const SwLineLayout *SwTxtIter::GetPrevLine()
{
    const SwLineLayout *pRoot = pInf->GetParaPortion();
    if( pRoot == pCurr )
        return 0;
    const SwLineLayout *pLay = pRoot;

    while( pLay->GetNext() != pCurr )
        pLay = pLay->GetNext();

    if( pLay->IsDummy() )
    {
        const SwLineLayout *pTmp = pRoot;
        pLay = pRoot->IsDummy() ? 0 : pRoot;
        while( pTmp->GetNext() != pCurr )
        {
            if( !pTmp->IsDummy() )
                pLay = pTmp;
            pTmp = pTmp->GetNext();
        }
    }

    // Wenn sich nichts getan hat, dann gibt es nur noch Dummys
    return (SwLineLayout*)pLay;
}

/*************************************************************************
 *                      SwTxtIter::PrevLine()
 *************************************************************************/

const SwLineLayout *SwTxtIter::PrevLine()
{
    const SwLineLayout *pPrev = Prev();
    if( !pPrev )
        return 0;

    const SwLineLayout *pLast = pPrev;
    while( pPrev && pPrev->IsDummy() )
    {
        DBG_LOOP;
        pLast = pPrev;
        pPrev = Prev();
    }
    return (SwLineLayout*)(pPrev ? pPrev : pLast);
}

/*************************************************************************
 *                      SwTxtIter::Bottom()
 *************************************************************************/

void SwTxtIter::Bottom()
{
    while( Next() )
    {
        DBG_LOOP;
    }
}

/*************************************************************************
 *                      SwTxtIter::CharToLine()
 *************************************************************************/

void SwTxtIter::CharToLine(const xub_StrLen nChar)
{
    while( nStart + pCurr->GetLen() <= nChar && Next() )
        ;
    while( nStart > nChar && Prev() )
        ;
}

/*************************************************************************
 *                      SwTxtIter::CharCrsrToLine()
 *************************************************************************/

// 1170: beruecksichtigt Mehrdeutigkeiten:
const SwLineLayout *SwTxtCursor::CharCrsrToLine( const xub_StrLen nPos )
{
    CharToLine( nPos );
    if( nPos != nStart )
        bRightMargin = sal_False;
    sal_Bool bPrev = bRightMargin && pCurr->GetLen() && GetPrev() &&
        GetPrev()->GetLen();
    if( bPrev && nPos && CH_BREAK == GetInfo().GetChar( nPos-1 ) )
        bPrev = sal_False;
    return bPrev ? PrevLine() : pCurr;
}

/*************************************************************************
 *                      SwTxtCrsr::AdjustBaseLine()
 *************************************************************************/

#ifdef VERTICAL_LAYOUT
USHORT SwTxtCursor::AdjustBaseLine( const SwLineLayout& rLine,
                                    const SwLinePortion* pPor,
                                    USHORT nPorHeight, USHORT nPorAscent,
                                    const sal_Bool bAutoToCentered ) const
{
    if ( pPor )
    {
        nPorHeight = pPor->Height();
        nPorAscent = pPor->GetAscent();
    }

    USHORT nOfst = rLine.GetRealHeight() - rLine.Height();

    GETGRID( pFrm->FindPageFrm() )
    const sal_Bool bHasGrid = pGrid && GetInfo().SnapToGrid();

    if ( bHasGrid )
    {
        const USHORT nGridWidth = pGrid->GetBaseHeight();
        const USHORT nRubyHeight = pGrid->GetRubyHeight();
        const sal_Bool bRubyTop = ! pGrid->GetRubyTextBelow();

        if ( GetInfo().IsMulti() )
            // we are inside the GetCharRect recursion for multi portions
            // we center the portion in its surrounding line
            nOfst = ( pCurr->Height() - nPorHeight ) / 2 + nPorAscent;
        else
        {
            // We have to take care for ruby portions.
            // The ruby portion is NOT centered
            nOfst += nPorAscent;

            if ( ! pPor || ! pPor->IsMultiPortion() ||
                 ! ((SwMultiPortion*)pPor)->IsRuby() )
            {
                // Portions which are bigger than on grid distance are
                // centered inside the whole line.
                const USHORT nLineNetto = ( nPorHeight > nGridWidth ) ?
                                            rLine.Height() - nRubyHeight :
                                            nGridWidth;
                nOfst += ( nLineNetto - nPorHeight ) / 2;
                if ( bRubyTop )
                    nOfst += nRubyHeight;
            }
        }
    }
    else
    {
        switch ( GetLineInfo().GetVertAlign() ) {
            case SvxParaVertAlignItem::TOP :
                nOfst += nPorAscent;
                break;
            case SvxParaVertAlignItem::CENTER :
                ASSERT( rLine.Height() >= nPorHeight, "Portion height > Line height");
                nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                break;
            case SvxParaVertAlignItem::BOTTOM :
                nOfst += rLine.Height() - nPorHeight + nPorAscent;
                break;
            case SvxParaVertAlignItem::AUTOMATIC :
                if ( bAutoToCentered || GetInfo().GetTxtFrm()->IsVertical() )
                {
                    nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                    break;
                }
            case SvxParaVertAlignItem::BASELINE :
                // base line
                nOfst += rLine.GetAscent();
                break;
        }
    }

    return nOfst;
}
#else
USHORT SwTxtCursor::AdjustBaseLine( const SwLineLayout& rLine,
                                    const USHORT nPorHeight,
                                    const USHORT nPorAscent,
                                    const sal_Bool bAutoToCentered ) const
{
    USHORT nOfst = rLine.GetRealHeight() - rLine.Height();

    switch ( GetLineInfo().GetVertAlign() ) {
        case SvxParaVertAlignItem::TOP :
            nOfst += nPorAscent;
            break;
        case SvxParaVertAlignItem::CENTER :
            ASSERT( rLine.Height() >= nPorHeight, "Portion height > Line height");
            nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
            break;
        case SvxParaVertAlignItem::BOTTOM :
            nOfst += rLine.Height() - nPorHeight + nPorAscent;
            break;
        case SvxParaVertAlignItem::AUTOMATIC :
            if ( bAutoToCentered )
            {
                nOfst += ( rLine.Height() - nPorHeight ) / 2 + nPorAscent;
                break;
            }
        case SvxParaVertAlignItem::BASELINE :
            // base line
            nOfst += rLine.GetAscent();
            break;
    }

    return nOfst;
}
#endif

/*************************************************************************
 *                      SwTxtIter::TwipsToLine()
 *************************************************************************/

const SwLineLayout *SwTxtIter::TwipsToLine( const SwTwips y)
{
    while( nY + GetLineHeight() <= y && Next() )
        ;
    while( nY > y && Prev() )
        ;
    return pCurr;
}

/*************************************************************************
 *                      SwTxtIter::TruncLines()
 *************************************************************************/

void SwTxtIter::TruncLines( sal_Bool bNoteFollow )
{
    SwLineLayout *pDel = pCurr->GetNext();
    const xub_StrLen nEnd = nStart + pCurr->GetLen();

    if( pDel )
    {
        pCurr->SetNext( 0 );
        if( GetHints() && bNoteFollow )
        {
            GetInfo().GetParaPortion()->SetFollowField( pDel->IsRest() );

            // bug 88534: wrong positioning of flys
            SwTxtFrm* pFollow = GetTxtFrm()->GetFollow();
            if ( pFollow && ! pFollow->IsLocked() &&
                 nEnd == pFollow->GetOfst() )
            {
                xub_StrLen nRangeEnd = nEnd;
                SwLineLayout* pLine = pDel;

                // determine range to be searched for flys anchored as characters
                while ( pLine )
                {
                    nRangeEnd += pLine->GetLen();
                    pLine = pLine->GetNext();
                }

                SwpHints* pHints = GetTxtFrm()->GetTxtNode()->GetpSwpHints();

                // examine hints in range nEnd - (nEnd + nRangeChar)
                for( USHORT i = 0; i < pHints->Count(); i++ )
                {
                    const SwTxtAttr* pHt = pHints->GetHt( i );
                    if( RES_TXTATR_FLYCNT == pHt->Which() )
                    {
                        // check, if hint is in our range
                        const USHORT nPos = *pHt->GetStart();
                        if ( nEnd <= nPos && nPos < nRangeEnd )
                            pFollow->_InvalidateRange(
                                SwCharRange( nPos, nPos ), 0 );
                    }
                }
            }
        }
        delete pDel;
    }
    if( pCurr->IsDummy() &&
        !pCurr->GetLen() &&
         nStart < GetTxtFrm()->GetTxt().Len() )
        pCurr->SetRealHeight( 1 );
    if( GetHints() )
        pFrm->RemoveFtn( nEnd );
}

/*************************************************************************
 *                      SwTxtIter::CntHyphens()
 *************************************************************************/

void SwTxtIter::CntHyphens( sal_uInt8 &nEndCnt, sal_uInt8 &nMidCnt) const
{
    nEndCnt = 0;
    nMidCnt = 0;
    if ( bPrev && pPrev && !pPrev->IsEndHyph() && !pPrev->IsMidHyph() )
         return;
    SwLineLayout *pLay = pInf->GetParaPortion();
    if( pCurr == pLay )
        return;
    while( pLay != pCurr )
    {
        DBG_LOOP;
        if ( pLay->IsEndHyph() )
            nEndCnt++;
        else
            nEndCnt = 0;
        if ( pLay->IsMidHyph() )
            nMidCnt++;
        else
            nMidCnt = 0;
        pLay = pLay->GetNext();
    }
}

/*************************************************************************
 *                          SwHookOut
 *
 * Change current output device to formatting device, this has to be done before
 * formatting.
 *************************************************************************/

SwHookOut::SwHookOut( SwTxtSizeInfo& rInfo ) :
     pInf( &rInfo ),
     pOut( rInfo.GetOut() ),
     bOnWin( rInfo.OnWin() )
{
    ASSERT( rInfo.GetRefDev(), "No reference device for text formatting" )

    // set new values
    rInfo.SetOut( rInfo.GetRefDev() );
    rInfo.SetOnWin( sal_False );
}

SwHookOut::~SwHookOut()
{
    pInf->SetOut( pOut );
    pInf->SetOnWin( bOnWin );
}
