/*************************************************************************
 *
 *  $RCSfile: itrtxt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ama $ $Date: 2001-02-20 10:23:15 $
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

#include "hintids.hxx"
#include "ndtxt.hxx"
#include "flyfrm.hxx"
#include "paratr.hxx"
#include "errhdl.hxx"

#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#include "txtcfg.hxx"
#include "itrtxt.hxx"
#include "txtfrm.hxx"
#include "porfly.hxx"

#ifdef DEBUG
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
    SwAttrIter::CtorInit( *pNode, pNewFrm->GetPara()->GetScriptInfo() );

    pFrm = pNewFrm;
    pInf = pNewInf;
    aLineInf.CtorInit( pNode->GetSwAttrSet() );
    aTopLeft = pFrm->Frm().Pos() + pFrm->Prt().Pos();
    SwTxtIter::Init();
    if( pNode->GetSwAttrSet().GetRegister().GetValue() )
        bRegisterOn = pFrm->FillRegister( nRegStart, nRegDiff );
    else
        bRegisterOn = sal_False;
    bUnderPara = UNDERLINE_NONE != pFnt->GetUnderline();
    bEscPara = 0 != pFnt->GetEscapement();
}

/*************************************************************************
 *                      SwTxtIter::Init()
 *************************************************************************/

void SwTxtIter::Init()
{
    pCurr = pInf->GetParaPortion();
    nStart = pInf->GetTxtStart();
    nY = aTopLeft.Y();
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
 *                    SwTxtIter::CalcRealHeight( sal_Bool bNewLine = sal_False )
 *************************************************************************/

void SwTxtIter::CalcRealHeight( sal_Bool bNewLine )
{
    KSHORT nLineHeight = pCurr->Height();
    pCurr->SetClipping( sal_False );
    // Das Dummyflag besitzen Zeilen, die nur Flyportions enthalten, diese
    // sollten kein Register etc. beachten. Dummerweise hat kann es eine leere
    // Zeile am Absatzende geben (bei leeren Abs„tzen oder nach einem
    // Shift-Return), die das Register durchaus beachten soll.
    if( !pCurr->IsDummy() || ( !pCurr->GetNext() &&
        GetStart() >= GetTxtFrm()->GetTxt().Len() && !bNewLine ) )
    {
        const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
        if( pSpace )
        {
            switch( pSpace->GetLineSpaceRule() )
            {
                case SVX_LINE_SPACE_AUTO:
                break;
                case SVX_LINE_SPACE_MIN:
                {
                    if( nLineHeight < KSHORT( pSpace->GetLineHeight() ) )
                        nLineHeight = pSpace->GetLineHeight();
                    break;
                }
                case SVX_LINE_SPACE_FIX:
                {
                    nLineHeight = pSpace->GetLineHeight();
                    KSHORT nAsc = ( 4 * nLineHeight ) / 5;  // 80%
                    if( nAsc < pCurr->GetAscent() ||
                        nLineHeight - nAsc < pCurr->Height() - pCurr->GetAscent() )
                        pCurr->SetClipping( sal_True );
                    pCurr->Height( nLineHeight );
                    pCurr->SetAscent( nAsc );
                    pInf->GetParaPortion()->SetFixLineHeight();
                }
                break;
                default: ASSERT( sal_False, ": unknown LineSpaceRule" );
            }
            if( !IsParaLine() )
                switch( pSpace->GetInterLineSpaceRule() )
                {
                    case SVX_INTER_LINE_SPACE_OFF:
                    break;
                    case SVX_INTER_LINE_SPACE_PROP:
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        // 50% ist das Minimum, bei 0% schalten wir auf
                        // den Defaultwert 100% um ...
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;

                        nTmp *= nLineHeight;
                        nTmp /= 100;
                        if( !nTmp )
                            ++nTmp;
                        nLineHeight = (KSHORT)nTmp;
                        break;
                    }
                    case SVX_INTER_LINE_SPACE_FIX:
                    {
                        nLineHeight += pSpace->GetInterLineSpace();
                        break;
                    }
                    default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
                }
        }
#ifdef DEBUG
        KSHORT nDummy = nLineHeight + 1;
#endif
        if( IsRegisterOn() )
        {
            SwTwips nTmpY = Y() + pCurr->GetAscent()
                            + nLineHeight - pCurr->Height() - RegStart();
            KSHORT nDiff = KSHORT( nTmpY % RegDiff() );
            if( nDiff )
                nLineHeight += RegDiff() - nDiff;
        }
    }
    pCurr->SetRealHeight( nLineHeight );
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
 *                      SwTxtIter::RecalcRealHeight()
 *************************************************************************/

void SwTxtIter::RecalcRealHeight()
{
    sal_Bool bMore = sal_True;
    while(bMore)
    {
        DBG_LOOP;
        CalcRealHeight();
        bMore = Next() != 0;
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
    if( pDel )
    {
        pCurr->SetNext( 0 );
        if( GetHints() && bNoteFollow )
            GetInfo().GetParaPortion()->SetFollowField( pDel->IsRest() );
        delete pDel;
    }
    if( GetHints() )
        pFrm->RemoveFtn( nStart + pCurr->GetLen() );
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


