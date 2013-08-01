/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "hintids.hxx"
#include "ndtxt.hxx"
#include "frmfmt.hxx"
#include "paratr.hxx"
#include "flyfrm.hxx"
#include "pam.hxx"
#include "swselectionlist.hxx"
#include <sortedobjs.hxx>
#include <editeng/protitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/borderline.hxx>
#include <frmatr.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagefrm.hxx>

#include "itrtxt.hxx"
#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include "porglue.hxx"      // SwFlyCnt
#include "porfld.hxx"       // SwFldPortion::IsFollow()
#include "porfly.hxx"       // GetFlyCrsrOfst()
#include "pordrop.hxx"
#include "crstate.hxx"      // SwCrsrMoveState
#include <pormulti.hxx>     // SwMultiPortion
// #i111284#
#include <numrule.hxx>

// Not reentrant !!!
// is set in GetCharRect and is interpreted in UnitUp/Down.
sal_Bool SwTxtCursor::bRightMargin = sal_False;


/*************************************************************************
 *                    lcl_GetCharRectInsideField
 *
 * After calculating the position of a character during GetCharRect
 * this function allows to find the coordinates of a position (defined
 * in pCMS->pSpecialPos) inside a special portion (e.g., a field)
 *************************************************************************/
static void lcl_GetCharRectInsideField( SwTxtSizeInfo& rInf, SwRect& rOrig,
                                 const SwCrsrMoveState& rCMS,
                                 const SwLinePortion& rPor )
{
    OSL_ENSURE( rCMS.pSpecialPos, "Information about special pos missing" );

    if ( rPor.InFldGrp() && !((SwFldPortion&)rPor).GetExp().isEmpty() )
    {
        const sal_uInt16 nCharOfst = rCMS.pSpecialPos->nCharOfst;
        sal_Int32 nFldIdx = 0;
        sal_Int32 nFldLen = 0;

        const OUString* pString = 0;
        const SwLinePortion* pPor = &rPor;
        do
        {
            if ( pPor->InFldGrp() )
            {
                pString = &((SwFldPortion*)pPor)->GetExp();
                nFldLen = pString->getLength();
            }
            else
            {
                pString = 0;
                nFldLen = 0;
            }

            if ( ! pPor->GetPortion() || nFldIdx + nFldLen > nCharOfst )
                break;

            nFldIdx = nFldIdx + nFldLen;
            rOrig.Pos().X() += pPor->Width();
            pPor = pPor->GetPortion();

        } while ( true );

        OSL_ENSURE( nCharOfst >= nFldIdx, "Request of position inside field failed" );
        sal_Int32 nLen = nCharOfst - nFldIdx + 1;

        if ( pString )
        {
            // get script for field portion
            rInf.GetFont()->SetActual( SwScriptInfo::WhichFont( 0, pString, 0 ) );

            sal_Int32 nOldLen = pPor->GetLen();
            ((SwLinePortion*)pPor)->SetLen( nLen - 1 );
            const SwTwips nX1 = pPor->GetLen() ?
                                pPor->GetTxtSize( rInf ).Width() :
                                0;

            SwTwips nX2 = 0;
            if ( rCMS.bRealWidth )
            {
                ((SwLinePortion*)pPor)->SetLen( nLen );
                nX2 = pPor->GetTxtSize( rInf ).Width();
            }

            ((SwLinePortion*)pPor)->SetLen( nOldLen );

            rOrig.Pos().X() += nX1;
            rOrig.Width( ( nX2 > nX1 ) ?
                         ( nX2 - nX1 ) :
                           1 );
        }
    }
    else
    {
        // special cases: no common fields, e.g., graphic number portion,
        // FlyInCntPortions, Notes
        rOrig.Width( rCMS.bRealWidth && rPor.Width() ? rPor.Width() : 1 );
    }
}

// #i111284#
namespace {
    bool AreListLevelIndentsApplicableAndLabelAlignmentActive( const SwTxtNode& rTxtNode )
    {
        bool bRet( false );

        if ( rTxtNode.AreListLevelIndentsApplicable() )
        {
            const SwNumFmt& rNumFmt =
                    rTxtNode.GetNumRule()->Get( static_cast<sal_uInt16>(rTxtNode.GetActualListLevel()) );
            if ( rNumFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                bRet = true;
            }
        }

        return bRet;
    }
} // end of anonymous namespace

/*************************************************************************
 *                SwTxtMargin::CtorInitTxtMargin()
 *************************************************************************/
void SwTxtMargin::CtorInitTxtMargin( SwTxtFrm *pNewFrm, SwTxtSizeInfo *pNewInf )
{
    CtorInitTxtIter( pNewFrm, pNewInf );

    pInf = pNewInf;
    GetInfo().SetFont( GetFnt() );
    const SwTxtNode *pNode = pFrm->GetTxtNode();

    const SvxLRSpaceItem &rSpace = pFrm->GetTxtNode()->GetSwAttrSet().GetLRSpace();
    // #i95907#
    // #i111284#
    const bool bListLevelIndentsApplicableAndLabelAlignmentActive(
        AreListLevelIndentsApplicableAndLabelAlignmentActive( *(pFrm->GetTxtNode()) ) );

    //
    // Carefully adjust the text formatting ranges.
    //
    // This whole area desperately needs some rework. There are
    // quite a couple of values that need to be considered:
    // 1. paragraph indent
    // 2. paragraph first line indent
    // 3. numbering indent
    // 4. numbering spacing to text
    // 5. paragraph border
    // Note: These values have already been used during calculation
    // of the printing area of the paragraph.
    const int nLMWithNum = pNode->GetLeftMarginWithNum( sal_True );
    if ( pFrm->IsRightToLeft() )
    {
        // this calculation is identical this the calculation for L2R layout - see below
        nLeft = pFrm->Frm().Left() +
                pFrm->Prt().Left() +
                nLMWithNum -
                pNode->GetLeftMarginWithNum( sal_False ) -
                // #i95907#
                // #i111284#
//                rSpace.GetLeft() +
//                rSpace.GetTxtLeft();
                ( bListLevelIndentsApplicableAndLabelAlignmentActive
                  ? 0
                  : ( rSpace.GetLeft() - rSpace.GetTxtLeft() ) );
    }
    else
    {
        // #i95907#
        // #i111284#
        if ( bListLevelIndentsApplicableAndLabelAlignmentActive ||
             !pNode->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) )
        {
            // this calculation is identical this the calculation for R2L layout - see above
            nLeft = pFrm->Frm().Left() +
                    pFrm->Prt().Left() +
                    nLMWithNum -
                    pNode->GetLeftMarginWithNum( sal_False ) -
                    // #i95907#
                    // #i111284#
                    ( bListLevelIndentsApplicableAndLabelAlignmentActive
                      ? 0
                      : ( rSpace.GetLeft() - rSpace.GetTxtLeft() ) );
        }
        else
        {
            nLeft = pFrm->Frm().Left() +
                    std::max( long( rSpace.GetTxtLeft() + nLMWithNum ),
                         pFrm->Prt().Left() );
        }
    }

    nRight = pFrm->Frm().Left() + pFrm->Prt().Left() + pFrm->Prt().Width();

    if( nLeft >= nRight &&
         // #i53066# Omit adjustment of nLeft for numbered
         // paras inside cells inside new documents:
        ( pNode->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) ||
          !pFrm->IsInTab() ||
          !nLMWithNum ) )
    {
        nLeft = pFrm->Prt().Left() + pFrm->Frm().Left();
        if( nLeft >= nRight )   // e.g. with large paragraph indentations in slim table columns
            nRight = nLeft + 1; // einen goennen wir uns immer
    }

    if( pFrm->IsFollow() && pFrm->GetOfst() )
        nFirst = nLeft;
    else
    {
        short nFLOfst = 0;
        long nFirstLineOfs = 0;
        if( !pNode->GetFirstLineOfsWithNum( nFLOfst ) &&
            rSpace.IsAutoFirst() )
        {
            nFirstLineOfs = GetFnt()->GetSize( GetFnt()->GetActual() ).Height();
            const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
            if( pSpace )
            {
                switch( pSpace->GetLineSpaceRule() )
                {
                    case SVX_LINE_SPACE_AUTO:
                    break;
                    case SVX_LINE_SPACE_MIN:
                    {
                        if( nFirstLineOfs < KSHORT( pSpace->GetLineHeight() ) )
                            nFirstLineOfs = pSpace->GetLineHeight();
                        break;
                    }
                    case SVX_LINE_SPACE_FIX:
                        nFirstLineOfs = pSpace->GetLineHeight();
                    break;
                    default: OSL_FAIL( ": unknown LineSpaceRule" );
                }
                switch( pSpace->GetInterLineSpaceRule() )
                {
                    case SVX_INTER_LINE_SPACE_OFF:
                    break;
                    case SVX_INTER_LINE_SPACE_PROP:
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        // 50% is the minimumm, at 0% we switch to
                        // the default value 100% ...
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;

                        nTmp *= nFirstLineOfs;
                        nTmp /= 100;
                        if( !nTmp )
                            ++nTmp;
                        nFirstLineOfs = (KSHORT)nTmp;
                        break;
                    }
                    case SVX_INTER_LINE_SPACE_FIX:
                    {
                        nFirstLineOfs += pSpace->GetInterLineSpace();
                        break;
                    }
                    default: OSL_FAIL( ": unknown InterLineSpaceRule" );
                }
            }
        }
        else
            nFirstLineOfs = nFLOfst;

        // #i95907#
        // #i111284#
        if ( pFrm->IsRightToLeft() ||
             bListLevelIndentsApplicableAndLabelAlignmentActive ||
             !pNode->getIDocumentSettingAccess()->get(IDocumentSettingAccess::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) )
        {
            nFirst = nLeft + nFirstLineOfs;
        }
        else
        {
              nFirst = pFrm->Frm().Left() +
                     std::max( rSpace.GetTxtLeft() + nLMWithNum+ nFirstLineOfs,
                          pFrm->Prt().Left() );
        }

        // Note: <SwTxtFrm::GetAdditionalFirstLineOffset()> returns a negative
        //       value for the new list label postion and space mode LABEL_ALIGNMENT
        //       and label alignment CENTER and RIGHT in L2R layout respectively
        //       label alignment LEFT and CENTER in R2L layout
        nFirst += pFrm->GetAdditionalFirstLineOffset();

        if( nFirst >= nRight )
            nFirst = nRight - 1;
    }
    const SvxAdjustItem& rAdjust = pFrm->GetTxtNode()->GetSwAttrSet().GetAdjust();
    nAdjust = static_cast<sal_uInt16>(rAdjust.GetAdjust());

    // left is left and right is right
    if ( pFrm->IsRightToLeft() )
    {
        if ( SVX_ADJUST_LEFT == nAdjust )
            nAdjust = SVX_ADJUST_RIGHT;
        else if ( SVX_ADJUST_RIGHT == nAdjust )
            nAdjust = SVX_ADJUST_LEFT;
    }

    bOneBlock = rAdjust.GetOneWord() == SVX_ADJUST_BLOCK;
    bLastBlock = rAdjust.GetLastBlock() == SVX_ADJUST_BLOCK;
    bLastCenter = rAdjust.GetLastBlock() == SVX_ADJUST_CENTER;

    // #i91133#
    mnTabLeft = pNode->GetLeftMarginForTabCalculation();

#if OSL_DEBUG_LEVEL > 1
    static sal_Bool bOne = sal_False;
    static sal_Bool bLast = sal_False;
    static sal_Bool bCenter = sal_False;
    bOneBlock |= bOne;
    bLastBlock |= bLast;
    bLastCenter |= bCenter;
#endif
    DropInit();
}

/*************************************************************************
 *                SwTxtMargin::DropInit()
 *************************************************************************/
void SwTxtMargin::DropInit()
{
    nDropLeft = nDropLines = nDropHeight = nDropDescent = 0;
    const SwParaPortion *pPara = GetInfo().GetParaPortion();
    if( pPara )
    {
        const SwDropPortion *pPorDrop = pPara->FindDropPortion();
        if ( pPorDrop )
        {
            nDropLeft = pPorDrop->GetDropLeft();
            nDropLines = pPorDrop->GetLines();
            nDropHeight = pPorDrop->GetDropHeight();
            nDropDescent = pPorDrop->GetDropDescent();
        }
    }
}

/*************************************************************************
 *                SwTxtMargin::GetLineStart()
 *************************************************************************/

// The function is interpreting / observing / evaluating / keeping / respecting the first line indention and the specified width.
SwTwips SwTxtMargin::GetLineStart() const
{
    SwTwips nRet = GetLeftMargin();
    if( GetAdjust() != SVX_ADJUST_LEFT &&
        !pCurr->GetFirstPortion()->IsMarginPortion() )
    {
        // If the first portion is a Margin, then the
        // adjustment is expressed by the portions.
        if( GetAdjust() == SVX_ADJUST_RIGHT )
            nRet = Right() - CurrWidth();
        else if( GetAdjust() == SVX_ADJUST_CENTER )
            nRet += (GetLineWidth() - CurrWidth()) / 2;
    }
    return nRet;
}

/*************************************************************************
 *                      SwTxtCursor::CtorInitTxtCursor()
 *************************************************************************/
void SwTxtCursor::CtorInitTxtCursor( SwTxtFrm *pNewFrm, SwTxtSizeInfo *pNewInf )
{
    CtorInitTxtMargin( pNewFrm, pNewInf );
    // 6096: Attention, the iterators are derived!
    // GetInfo().SetOut( GetInfo().GetWin() );
}

/*************************************************************************
 *                      SwTxtCursor::GetEndCharRect()
 *************************************************************************/

// 1170: Ancient bug: Shift-End forgets the last character ...

sal_Bool SwTxtCursor::GetEndCharRect( SwRect* pOrig, const xub_StrLen nOfst,
                                  SwCrsrMoveState* pCMS, const long nMax )
{
    // 1170: Ambiguity of document positions
    bRightMargin = sal_True;
    CharCrsrToLine(nOfst);

    // Somehow twisted: nOfst names the position behind the last
    // character of the last line == This is the position in front of the first character
    // of the line, in which we are situated:
    if( nOfst != GetStart() || !pCurr->GetLen() )
    {
        // 8810: Master line RightMargin, after that LeftMargin
        const sal_Bool bRet = GetCharRect( pOrig, nOfst, pCMS, nMax );
        bRightMargin = nOfst >= GetEnd() && nOfst < GetInfo().GetTxt().getLength();
        return bRet;
    }

    if( !GetPrev() || !GetPrev()->GetLen() || !PrevLine() )
        return GetCharRect( pOrig, nOfst, pCMS, nMax );

    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    KSHORT nX = 0;
    KSHORT nLast = 0;
    SwLinePortion *pPor = pCurr->GetFirstPortion();

    KSHORT nTmpHeight, nTmpAscent;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    KSHORT nPorHeight = nTmpHeight;
    KSHORT nPorAscent = nTmpAscent;

    // Search for the last Text/EndPortion of the line
    while( pPor )
    {
        nX = nX + pPor->Width();
        if( pPor->InTxtGrp() || ( pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsHolePortion() ) || pPor->IsBreakPortion() )
        {
            nLast = nX;
            nPorHeight = pPor->Height();
            nPorAscent = pPor->GetAscent();
        }
        pPor = pPor->GetPortion();
    }

    const Size aCharSize( 1, nTmpHeight );
    pOrig->Pos( GetTopLeft() );
    pOrig->SSize( aCharSize );
    pOrig->Pos().X() += nLast;
    const SwTwips nTmpRight = Right() - 1;
    if( pOrig->Left() > nTmpRight )
        pOrig->Pos().X() = nTmpRight;

    if ( pCMS && pCMS->bRealHeight )
    {
        if ( nTmpAscent > nPorAscent )
            pCMS->aRealHeight.X() = nTmpAscent - nPorAscent;
        else
            pCMS->aRealHeight.X() = 0;
        OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
        pCMS->aRealHeight.Y() = nPorHeight;
    }

    return sal_True;
}

/*************************************************************************
 * void SwTxtCursor::_GetCharRect(..)
 * internal function, called by SwTxtCursor::GetCharRect() to calculate
 * the relative character position in the current line.
 * pOrig referes to x and y coordinates, width and height of the cursor
 * pCMS is used for restricting the cursor, if there are different font
 * heights in one line ( first value = offset to y of pOrig, second
 * value = real height of (shortened) cursor
 *************************************************************************/

void SwTxtCursor::_GetCharRect( SwRect* pOrig, const xub_StrLen nOfst,
    SwCrsrMoveState* pCMS )
{
    const OUString aText = GetInfo().GetTxt();
    SwTxtSizeInfo aInf( GetInfo(), &aText, nStart );
    if( GetPropFont() )
        aInf.GetFont()->SetProportion( GetPropFont() );
    KSHORT nTmpAscent, nTmpHeight;  // Line height
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    const Size  aCharSize( 1, nTmpHeight );
    const Point aCharPos;
    pOrig->Pos( aCharPos );
    pOrig->SSize( aCharSize );

    // If we are looking for a position inside a field which covers
    // more than one line we may not skip any "empty portions" at the
    // beginning of a line
    const sal_Bool bInsideFirstField = pCMS && pCMS->pSpecialPos &&
                                       ( pCMS->pSpecialPos->nLineOfst ||
                                         SP_EXTEND_RANGE_BEFORE ==
                                         pCMS->pSpecialPos->nExtendRange );

    sal_Bool bWidth = pCMS && pCMS->bRealWidth;
    if( !pCurr->GetLen() && !pCurr->Width() )
    {
        if ( pCMS && pCMS->bRealHeight )
        {
            pCMS->aRealHeight.X() = 0;
            pCMS->aRealHeight.Y() = nTmpHeight;
        }
    }
    else
    {
        KSHORT nPorHeight = nTmpHeight;
        KSHORT nPorAscent = nTmpAscent;
        SwTwips nX = 0;
        SwTwips nTmpFirst = 0;
        SwLinePortion *pPor = pCurr->GetFirstPortion();
        SwBidiPortion* pLastBidiPor = 0;
        SwTwips nLastBidiPorWidth = 0;
        std::deque<sal_uInt16>* pKanaComp = pCurr->GetpKanaComp();
        MSHORT nSpaceIdx = 0;
        size_t nKanaIdx = 0;
        long nSpaceAdd = pCurr->IsSpaceAdd() ? pCurr->GetLLSpaceAdd( 0 ) : 0;

        sal_Bool bNoTxt = sal_True;

        // First all portions without Len at beginning of line are skipped.
        // Exceptions are the mean special portions from WhichFirstPortion:
        // Num, ErgoSum, FtnNum, FeldReste
        // 8477: but also the only Textportion of an empty line with
        // Right/Center-Adjustment! So not just pPor->GetExpandPortion() ...
        while( pPor && !pPor->GetLen() && ! bInsideFirstField )
        {
            nX += pPor->Width();
            if ( pPor->InSpaceGrp() && nSpaceAdd )
                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
            if( bNoTxt )
                nTmpFirst = nX;
            // 8670: EndPortions count once as TxtPortions.
//            if( pPor->InTxtGrp() || pPor->IsBreakPortion() )
            if( pPor->InTxtGrp() || pPor->IsBreakPortion() || pPor->InTabGrp() )
            {
                bNoTxt = sal_False;
                nTmpFirst = nX;
            }
            if( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
            {
                if ( pCurr->IsSpaceAdd() )
                {
                    if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                        nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                    else
                        nSpaceAdd = 0;
                }

                if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                    ++nKanaIdx;
            }
            if( pPor->InFixMargGrp() )
            {
                if( pPor->IsMarginPortion() )
                    bNoTxt = sal_False;
                else
                {
                    // fix margin portion => next SpaceAdd, KanaComp value
                    if ( pCurr->IsSpaceAdd() )
                    {
                        if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                            nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                        else
                            nSpaceAdd = 0;
                    }

                    if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                        ++nKanaIdx;
                }
            }
            pPor = pPor->GetPortion();
        }

        if( !pPor )
        {
            // There's just Spezialportions.
            nX = nTmpFirst;
        }
        else
        {
            if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                (!pPor->InFldGrp() || pPor->GetAscent() ) )
            {
                nPorHeight = pPor->Height();
                nPorAscent = pPor->GetAscent();
            }
            while( pPor && !pPor->IsBreakPortion() && ( aInf.GetIdx() < nOfst ||
                   ( bWidth && ( pPor->IsKernPortion() || pPor->IsMultiPortion() ) ) ) )
            {
                if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                    (!pPor->InFldGrp() || pPor->GetAscent() ) )
                {
                    nPorHeight = pPor->Height();
                    nPorAscent = pPor->GetAscent();
                }

                // If we are behind the portion, we add the portion width to
                // nX. Special case: nOfst = aInf.GetIdx() + pPor->GetLen().
                // For common portions (including BidiPortions) we want to add
                // the portion width to nX. For MultiPortions, nExtra = 0,
                // therefore we go to the 'else' branch and start a recursion.
                const sal_uInt8 nExtra = pPor->IsMultiPortion() &&
                                    ! ((SwMultiPortion*)pPor)->IsBidi() &&
                                    ! bWidth ? 0 : 1;
                if ( aInf.GetIdx() + pPor->GetLen() < nOfst + nExtra )
                {
                    if ( pPor->InSpaceGrp() && nSpaceAdd )
                        nX += pPor->PrtWidth() +
                              pPor->CalcSpacing( nSpaceAdd, aInf );
                    else
                    {
                        if( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() )
                        {
                            // update to current SpaceAdd, KanaComp values
                            if ( pCurr->IsSpaceAdd() )
                            {
                                if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                                    nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                                else
                                    nSpaceAdd = 0;
                            }

                            if ( pKanaComp &&
                                ( nKanaIdx + 1 ) < pKanaComp->size()
                                )
                                ++nKanaIdx;
                        }
                        if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                                !pPor->GetPortion()->IsMarginPortion() ) )
                            nX += pPor->PrtWidth();
                    }
                    if( pPor->IsMultiPortion() )
                    {
                        if ( ((SwMultiPortion*)pPor)->HasTabulator() )
                        {
                            if ( pCurr->IsSpaceAdd() )
                            {
                                if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                                    nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                                else
                                    nSpaceAdd = 0;
                            }

                            if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                                ++nKanaIdx;
                        }

                        // if we are right behind a BidiPortion, we have to
                        // hold a pointer to the BidiPortion in order to
                        // find the correct cursor position, depending on the
                        // cursor level
                        if ( ((SwMultiPortion*)pPor)->IsBidi() &&
                             aInf.GetIdx() + pPor->GetLen() == nOfst )
                        {
                             pLastBidiPor = (SwBidiPortion*)pPor;
                             nLastBidiPorWidth = pLastBidiPor->Width() +
                                                 pLastBidiPor->CalcSpacing( nSpaceAdd, aInf );;
                        }
                    }

                    aInf.SetIdx( aInf.GetIdx() + pPor->GetLen() );
                    pPor = pPor->GetPortion();
                }
                else
                {
                    if( pPor->IsMultiPortion() )
                    {
                        nTmpAscent = AdjustBaseLine( *pCurr, pPor );
                        GetInfo().SetMulti( sal_True );
                        pOrig->Pos().Y() += nTmpAscent - nPorAscent;

                        if( pCMS && pCMS->b2Lines )
                        {
                            sal_Bool bRecursion = sal_True;
                            if ( ! pCMS->p2Lines )
                            {
                                pCMS->p2Lines = new Sw2LinesPos;
                                pCMS->p2Lines->aLine = SwRect(aCharPos, aCharSize);
                                bRecursion = sal_False;
                            }

                            if( ((SwMultiPortion*)pPor)->HasRotation() )
                            {
                                if( ((SwMultiPortion*)pPor)->IsRevers() )
                                    pCMS->p2Lines->nMultiType = MT_ROT_270;
                                else
                                    pCMS->p2Lines->nMultiType = MT_ROT_90;
                            }
                            else if( ((SwMultiPortion*)pPor)->IsDouble() )
                                pCMS->p2Lines->nMultiType = MT_TWOLINE;
                            else if( ((SwMultiPortion*)pPor)->IsBidi() )
                                pCMS->p2Lines->nMultiType = MT_BIDI;
                            else
                                pCMS->p2Lines->nMultiType = MT_RUBY;

                            SwTwips nTmpWidth = pPor->Width();
                            if( nSpaceAdd )
                                nTmpWidth += pPor->CalcSpacing(nSpaceAdd, aInf);

                            SwRect aRect( Point(aCharPos.X() + nX, pOrig->Top() ),
                                          Size( nTmpWidth, pPor->Height() ) );

                            if ( ! bRecursion )
                                pCMS->p2Lines->aPortion = aRect;
                            else
                                pCMS->p2Lines->aPortion2 = aRect;
                        }

                        // In a multi-portion we use GetCharRect()-function
                        // recursively and must add the x-position
                        // of the multi-portion.
                        xub_StrLen nOldStart = nStart;
                        SwTwips nOldY = nY;
                        sal_uInt8 nOldProp = GetPropFont();
                        nStart = aInf.GetIdx();
                        SwLineLayout* pOldCurr = pCurr;
                        pCurr = &((SwMultiPortion*)pPor)->GetRoot();
                        if( ((SwMultiPortion*)pPor)->IsDouble() )
                            SetPropFont( 50 );

                        GETGRID( GetTxtFrm()->FindPageFrm() )
                        const sal_Bool bHasGrid = pGrid && GetInfo().SnapToGrid();
                        const sal_uInt16 nRubyHeight = bHasGrid ?
                                                   pGrid->GetRubyHeight() : 0;

                        if( nStart + pCurr->GetLen() <= nOfst && GetNext() &&
                            ( ! ((SwMultiPortion*)pPor)->IsRuby() ||
                                ((SwMultiPortion*)pPor)->OnTop() ) )
                        {
                            sal_uInt16 nOffset;
                            // in grid mode we may only add the height of the
                            // ruby line if ruby line is on top
                            if ( bHasGrid &&
                                ((SwMultiPortion*)pPor)->IsRuby() &&
                                ((SwMultiPortion*)pPor)->OnTop() )
                                nOffset = nRubyHeight;
                            else
                                nOffset = GetLineHeight();

                            pOrig->Pos().Y() += nOffset;
                            Next();
                        }

                        sal_Bool bSpaceChg = ((SwMultiPortion*)pPor)->
                                                ChgSpaceAdd( pCurr, nSpaceAdd );
                        Point aOldPos = pOrig->Pos();

                        // Ok, for ruby portions in grid mode we have to
                        // temporarily set the inner line height to the
                        // outer line height because that value is needed
                        // for the adjustment inside the recursion
                        const sal_uInt16 nOldRubyHeight = pCurr->Height();
                        const sal_uInt16 nOldRubyRealHeight = pCurr->GetRealHeight();
                        const sal_Bool bChgHeight =
                                ((SwMultiPortion*)pPor)->IsRuby() && bHasGrid;

                        if ( bChgHeight )
                        {
                            pCurr->Height( pOldCurr->Height() - nRubyHeight );
                            pCurr->SetRealHeight( pOldCurr->GetRealHeight() -
                                                  nRubyHeight );
                        }

                        SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
                        if ( ((SwMultiPortion*)pPor)->IsBidi() )
                        {
                            aLayoutModeModifier.Modify(
                                ((SwBidiPortion*)pPor)->GetLevel() % 2 );
                        }

                        _GetCharRect( pOrig, nOfst, pCMS );

                        if ( bChgHeight )
                        {
                            pCurr->Height( nOldRubyHeight );
                            pCurr->SetRealHeight( nOldRubyRealHeight );
                        }

                        // if we are still in the first row of
                        // our 2 line multiportion, we use the FirstMulti flag
                        // to indicate this
                        if ( ((SwMultiPortion*)pPor)->IsDouble() )
                        {
                            // the recursion may have damaged our font size
                            SetPropFont( nOldProp );
                            if ( !nOldProp )
                                nOldProp = 100;
                            GetInfo().GetFont()->SetProportion( 100 );

                            if ( pCurr == &((SwMultiPortion*)pPor)->GetRoot() )
                            {
                                GetInfo().SetFirstMulti( true );

                                // we want to treat a double line portion like a
                                // single line portion, if there is no text in
                                // the second line
                                if ( !pCurr->GetNext() ||
                                     !pCurr->GetNext()->GetLen() )
                                    GetInfo().SetMulti( sal_False );
                            }
                        }
                        // ruby portions are treated like single line portions
                        else if( ((SwMultiPortion*)pPor)->IsRuby() ||
                                 ((SwMultiPortion*)pPor)->IsBidi() )
                            GetInfo().SetMulti( sal_False );

                        // calculate cursor values
                        if( ((SwMultiPortion*)pPor)->HasRotation() )
                        {
                            GetInfo().SetMulti( sal_False );
                            long nTmp = pOrig->Width();
                            pOrig->Width( pOrig->Height() );
                            pOrig->Height( nTmp );
                            nTmp = pOrig->Left() - aOldPos.X();

                            // if we travel into our rotated portion from
                            // a line below, we have to take care, that the
                            // y coord in pOrig is less than line height:
                            if ( nTmp )
                                nTmp--;

                            pOrig->Pos().X() = nX + aOldPos.X();
                            if( ((SwMultiPortion*)pPor)->IsRevers() )
                                pOrig->Pos().Y() = aOldPos.Y() + nTmp;
                            else
                                pOrig->Pos().Y() = aOldPos.Y()
                                    + pPor->Height() - nTmp - pOrig->Height();
                            if ( pCMS && pCMS->bRealHeight )
                            {
                                pCMS->aRealHeight.Y() = -pCMS->aRealHeight.Y();
                                // result for rotated multi portion is not
                                // correct for reverse (270 degree) portions
                                if( ((SwMultiPortion*)pPor)->IsRevers() )
                                {
                                    if ( SvxParaVertAlignItem::AUTOMATIC ==
                                         GetLineInfo().GetVertAlign() )
                                        // if vertical alignment is set to auto,
                                        // we switch from base line alignment
                                        // to centered alignment
                                        pCMS->aRealHeight.X() =
                                            ( pOrig->Width() +
                                              pCMS->aRealHeight.Y() ) / 2;
                                    else
                                        pCMS->aRealHeight.X() =
                                            ( pOrig->Width() -
                                              pCMS->aRealHeight.X() +
                                              pCMS->aRealHeight.Y() );
                                }
                            }
                        }
                        else
                        {
                            pOrig->Pos().Y() += aOldPos.Y();
                            if ( ((SwMultiPortion*)pPor)->IsBidi() )
                            {
                                const SwTwips nPorWidth = pPor->Width() +
                                                         pPor->CalcSpacing( nSpaceAdd, aInf );
                                const SwTwips nInsideOfst = pOrig->Pos().X();
                                pOrig->Pos().X() = nX + nPorWidth -
                                                   nInsideOfst - pOrig->Width();
                            }
                            else
                                pOrig->Pos().X() += nX;

                            if( ((SwMultiPortion*)pPor)->HasBrackets() )
                                pOrig->Pos().X() +=
                                    ((SwDoubleLinePortion*)pPor)->PreWidth();
                        }

                        if( bSpaceChg )
                            SwDoubleLinePortion::ResetSpaceAdd( pCurr );

                        pCurr = pOldCurr;
                        nStart = nOldStart;
                        nY = nOldY;
                        bPrev = sal_False;

                        return;
                    }
                    if ( pPor->PrtWidth() )
                    {
                        xub_StrLen nOldLen = pPor->GetLen();
                        pPor->SetLen( nOfst - aInf.GetIdx() );
                        aInf.SetLen( pPor->GetLen() );
                        if( nX || !pPor->InNumberGrp() )
                        {
                            SeekAndChg( aInf );
                            const bool bOldOnWin = aInf.OnWin();
                            aInf.SetOnWin( false ); // keine BULLETs!
                            SwTwips nTmp = nX;
                            aInf.SetKanaComp( pKanaComp );
                            aInf.SetKanaIdx( nKanaIdx );
                            nX += pPor->GetTxtSize( aInf ).Width();
                            aInf.SetOnWin( bOldOnWin );
                            if ( pPor->InSpaceGrp() && nSpaceAdd )
                                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
                            if( bWidth )
                            {
                                pPor->SetLen( pPor->GetLen() + 1 );
                                aInf.SetLen( pPor->GetLen() );
                                aInf.SetOnWin( false ); // keine BULLETs!
                                nTmp += pPor->GetTxtSize( aInf ).Width();
                                aInf.SetOnWin( bOldOnWin );
                                if ( pPor->InSpaceGrp() && nSpaceAdd )
                                    nTmp += pPor->CalcSpacing(nSpaceAdd, aInf);
                                pOrig->Width( nTmp - nX );
                            }
                        }
                        pPor->SetLen( nOldLen );

                        // Shift the cursor with the right border width
                        // Note: nX remains positive because GetTxtSize() also include the width of the right border
                        if( aInf.GetIdx() < nOfst && nOfst < aInf.GetIdx() + pPor->GetLen() )
                        {
                            // Find the current drop portion part and use its right border
                            if( pPor->IsDropPortion() )
                            {
                                SwDropPortion* pDrop = static_cast<SwDropPortion*>(pPor);
                                const SwDropPortionPart* pCurrPart = pDrop->GetPart();
                                sal_Int16 nSumLength = 0;
                                while( pCurrPart && (nSumLength += pCurrPart->GetLen()) < nOfst - aInf.GetIdx() )
                                {
                                    pCurrPart = pCurrPart->GetFollow();
                                }
                                if( pCurrPart && nSumLength != nOfst - aInf.GetIdx() && pCurrPart->GetFont().GetRightBorder() )
                                {
                                    nX -= pCurrPart->GetFont().GetRightBorder().get().GetScaledWidth();
                                }
                            }
                            else if(GetInfo().GetFont()->GetRightBorder())
                            {
                                nX -= GetInfo().GetFont()->GetRightBorder().get().GetScaledWidth();
                            }
                         }
                    }
                    bWidth = sal_False;
                    break;
                }
            }
        }

        if( pPor )
        {
            OSL_ENSURE( !pPor->InNumberGrp() || bInsideFirstField, "Number surprise" );
            sal_Bool bEmptyFld = sal_False;
            if( pPor->InFldGrp() && pPor->GetLen() )
            {
                SwFldPortion *pTmp = (SwFldPortion*)pPor;
                while( pTmp->HasFollow() && pTmp->GetExp().isEmpty() )
                {
                    KSHORT nAddX = pTmp->Width();
                    SwLinePortion *pNext = pTmp->GetPortion();
                    while( pNext && !pNext->InFldGrp() )
                    {
                        OSL_ENSURE( !pNext->GetLen(), "Where's my field follow?" );
                        nAddX = nAddX + pNext->Width();
                        pNext = pNext->GetPortion();
                    }
                    if( !pNext )
                        break;
                    pTmp = (SwFldPortion*)pNext;
                    nPorHeight = pTmp->Height();
                    nPorAscent = pTmp->GetAscent();
                    nX += nAddX;
                    bEmptyFld = sal_True;
                }
            }
            // 8513: Fields in justified text, skipped
            while( pPor && !pPor->GetLen() && ! bInsideFirstField &&
                   ( pPor->IsFlyPortion() || pPor->IsKernPortion() ||
                     pPor->IsBlankPortion() || pPor->InTabGrp() ||
                     ( !bEmptyFld && pPor->InFldGrp() ) ) )
            {
                if ( pPor->InSpaceGrp() && nSpaceAdd )
                    nX += pPor->PrtWidth() +
                          pPor->CalcSpacing( nSpaceAdd, aInf );
                else
                {
                    if( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() )
                    {
                        if ( pCurr->IsSpaceAdd() )
                        {
                            if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                                nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                            else
                                nSpaceAdd = 0;
                        }

                        if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                            ++nKanaIdx;
                    }
                    if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                            !pPor->GetPortion()->IsMarginPortion() ) )
                        nX += pPor->PrtWidth();
                }
                if( pPor->IsMultiPortion() &&
                    ((SwMultiPortion*)pPor)->HasTabulator() )
                {
                    if ( pCurr->IsSpaceAdd() )
                    {
                        if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                            nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                        else
                            nSpaceAdd = 0;
                    }

                    if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                        ++nKanaIdx;
                }
                if( !pPor->IsFlyPortion() )
                {
                    nPorHeight = pPor->Height();
                    nPorAscent = pPor->GetAscent();
                }
                pPor = pPor->GetPortion();
            }

            if( aInf.GetIdx() == nOfst && pPor && pPor->InHyphGrp() &&
                pPor->GetPortion() && pPor->GetPortion()->InFixGrp() )
            {
                // All special portions have to be skipped
                // Taking the German word "zusammen" as example: zu-[FLY]sammen, 'u' == 19, 's' == 20; Right()
                // Without the adjustment we end up in front of '-', with the
                // adjustment in front of the 's'.
                while( pPor && !pPor->GetLen() )
                {
                    nX += pPor->Width();
                    if( !pPor->IsMarginPortion() )
                    {
                        nPorHeight = pPor->Height();
                        nPorAscent = pPor->GetAscent();
                    }
                    pPor = pPor->GetPortion();
                }
            }
            if( pPor && pCMS )
            {
                if( pCMS->bFieldInfo && pPor->InFldGrp() && pPor->Width() )
                    pOrig->Width( pPor->Width() );
                if( pPor->IsDropPortion() )
                {
                    nPorAscent = ((SwDropPortion*)pPor)->GetDropHeight();
                    // The drop height is only calculated, if we have more than
                    // one line. Otherwise it is 0.
                    if ( ! nPorAscent)
                        nPorAscent = pPor->Height();
                    nPorHeight = nPorAscent;
                    pOrig->Height( nPorHeight +
                        ((SwDropPortion*)pPor)->GetDropDescent() );
                    if( nTmpHeight < pOrig->Height() )
                    {
                        nTmpAscent = nPorAscent;
                        nTmpHeight = sal_uInt16( pOrig->Height() );
                    }
                }
                if( bWidth && pPor->PrtWidth() && pPor->GetLen() &&
                    aInf.GetIdx() == nOfst )
                {
                    if( !pPor->IsFlyPortion() && pPor->Height() &&
                        pPor->GetAscent() )
                    {
                        nPorHeight = pPor->Height();
                        nPorAscent = pPor->GetAscent();
                    }
                    SwTwips nTmp;
                    if( 2 > pPor->GetLen() )
                    {
                        nTmp = pPor->Width();
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                    }
                    else
                    {
                        const bool bOldOnWin = aInf.OnWin();
                        xub_StrLen nOldLen = pPor->GetLen();
                        pPor->SetLen( 1 );
                        aInf.SetLen( pPor->GetLen() );
                        SeekAndChg( aInf );
                        aInf.SetOnWin( false ); // keine BULLETs!
                        aInf.SetKanaComp( pKanaComp );
                        aInf.SetKanaIdx( nKanaIdx );
                        nTmp = pPor->GetTxtSize( aInf ).Width();
                        aInf.SetOnWin( bOldOnWin );
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                        pPor->SetLen( nOldLen );
                    }
                    pOrig->Width( nTmp );
                }

                // travel inside field portion?
                if ( pCMS->pSpecialPos )
                {
                    // apply attributes to font
                    SeekAndChgAttrIter( nOfst, aInf.GetOut() );
                    lcl_GetCharRectInsideField( aInf, *pOrig, *pCMS, *pPor );
                }
            }
        }

        // special case: We are at the beginning of a BidiPortion or
        // directly behind a BidiPortion
        if ( pCMS &&
                ( pLastBidiPor ||
                ( pPor &&
                  pPor->IsMultiPortion() &&
                  ((SwMultiPortion*)pPor)->IsBidi() ) ) )
        {
            // we determine if the cursor has to blink before or behind
            // the bidi portion
            if ( pLastBidiPor )
            {
                const sal_uInt8 nPortionLevel = pLastBidiPor->GetLevel();

                if ( pCMS->nCursorBidiLevel >= nPortionLevel )
                {
                    // we came from inside the bidi portion, we want to blink
                    // behind the portion
                    pOrig->Pos().X() -= nLastBidiPorWidth;

                    // Again, there is a special case: logically behind
                    // the portion can actually mean that the cursor is inside
                    // the portion. This can happen is the last portion
                    // inside the bidi portion is a nested bidi portion
                    SwLineLayout& rLineLayout =
                            ((SwMultiPortion*)pLastBidiPor)->GetRoot();

                    const SwLinePortion *pLast = rLineLayout.FindLastPortion();
                    if ( pLast->IsMultiPortion() )
                    {
                        OSL_ENSURE( ((SwMultiPortion*)pLast)->IsBidi(),
                                 "Non-BidiPortion inside BidiPortion" );
                        pOrig->Pos().X() += pLast->Width() +
                                            pLast->CalcSpacing( nSpaceAdd, aInf );
                    }
                }
            }
            else
            {
                const sal_uInt8 nPortionLevel = ((SwBidiPortion*)pPor)->GetLevel();

                if ( pCMS->nCursorBidiLevel >= nPortionLevel )
                {
                    // we came from inside the bidi portion, we want to blink
                    // behind the portion
                    pOrig->Pos().X() += pPor->Width() +
                                        pPor->CalcSpacing( nSpaceAdd, aInf );
                }
            }
        }

        pOrig->Pos().X() += nX;

        if ( pCMS && pCMS->bRealHeight )
        {
            nTmpAscent = AdjustBaseLine( *pCurr, 0, nPorHeight, nPorAscent );
            if ( nTmpAscent > nPorAscent )
                pCMS->aRealHeight.X() = nTmpAscent - nPorAscent;
            else
                pCMS->aRealHeight.X() = 0;
            OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
            if ( nTmpHeight > nPorHeight )
                pCMS->aRealHeight.Y() = nPorHeight;
            else
                pCMS->aRealHeight.Y() = nTmpHeight;
        }
    }
}

/*************************************************************************
 *                      SwTxtCursor::GetCharRect()
 *************************************************************************/

sal_Bool SwTxtCursor::GetCharRect( SwRect* pOrig, const xub_StrLen nOfst,
                               SwCrsrMoveState* pCMS, const long nMax )
{
    CharCrsrToLine(nOfst);

    // Indicates that a position inside a special portion (field, number portion)
    // is requested.
    const sal_Bool bSpecialPos = pCMS && pCMS->pSpecialPos;
    xub_StrLen nFindOfst = nOfst;

    if ( bSpecialPos )
    {
        const sal_uInt8 nExtendRange = pCMS->pSpecialPos->nExtendRange;

        OSL_ENSURE( ! pCMS->pSpecialPos->nLineOfst || SP_EXTEND_RANGE_BEFORE != nExtendRange,
                "LineOffset AND Number Portion?" );

        // portions which are behind the string
        if ( SP_EXTEND_RANGE_BEHIND == nExtendRange )
            ++nFindOfst;

        // skip lines for fields which cover more than one line
        for ( sal_uInt16 i = 0; i < pCMS->pSpecialPos->nLineOfst; i++ )
            Next();
    }

    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    const Point aCharPos( GetTopLeft() );
    sal_Bool bRet = sal_True;

    _GetCharRect( pOrig, nFindOfst, pCMS );

    // This actually would have to be "-1 LogicToPixel", but that seems too
    // expensive, so it's a value (-12), that should hopefully be OK.
    const SwTwips nTmpRight = Right() - 12;

    pOrig->Pos().X() += aCharPos.X();
    pOrig->Pos().Y() += aCharPos.Y();

    if( pCMS && pCMS->b2Lines && pCMS->p2Lines )
    {
        pCMS->p2Lines->aLine.Pos().X() += aCharPos.X();
        pCMS->p2Lines->aLine.Pos().Y() += aCharPos.Y();
        pCMS->p2Lines->aPortion.Pos().X() += aCharPos.X();
        pCMS->p2Lines->aPortion.Pos().Y() += aCharPos.Y();
    }

    const bool bTabOverMargin = GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_OVER_MARGIN);
    // Make sure the cursor respects the right margin, unless in compat mode, where the tab size has priority over the margin size.
    if( pOrig->Left() > nTmpRight && !bTabOverMargin)
        pOrig->Pos().X() = nTmpRight;

    if( nMax )
    {
        if( pOrig->Top() + pOrig->Height() > nMax )
        {
            if( pOrig->Top() > nMax )
                pOrig->Top( nMax );
            pOrig->Height( nMax - pOrig->Top() );
        }
        if ( pCMS && pCMS->bRealHeight && pCMS->aRealHeight.Y() >= 0 )
        {
            long nTmp = pCMS->aRealHeight.X() + pOrig->Top();
            if( nTmp >= nMax )
            {
                pCMS->aRealHeight.X() = nMax - pOrig->Top();
                pCMS->aRealHeight.Y() = 0;
            }
            else if( nTmp + pCMS->aRealHeight.Y() > nMax )
                pCMS->aRealHeight.Y() = nMax - nTmp;
        }
    }
    long nOut = pOrig->Right() - GetTxtFrm()->Frm().Right();
    if( nOut > 0 )
    {
        if( GetTxtFrm()->Frm().Width() < GetTxtFrm()->Prt().Left()
                                   + GetTxtFrm()->Prt().Width() )
            nOut += GetTxtFrm()->Frm().Width() - GetTxtFrm()->Prt().Left()
                    - GetTxtFrm()->Prt().Width();
        if( nOut > 0 )
            pOrig->Pos().X() -= nOut + 10;
    }

    return bRet;
}

/*************************************************************************
 *                      SwTxtCursor::GetCrsrOfst()
 *
 * Return: Offset in String
 *************************************************************************/
xub_StrLen SwTxtCursor::GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
                     const MSHORT nChgNode, SwCrsrMoveState* pCMS ) const
{
    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    const OUString &rText = GetInfo().GetTxt();
    xub_StrLen nOffset = 0;

    // x is the horizontal offset within the line.
    SwTwips x = rPoint.X();
    const SwTwips nLeftMargin  = GetLineStart();
    SwTwips nRightMargin = GetLineEnd();
    if( nRightMargin == nLeftMargin )
        nRightMargin += 30;

    const sal_Bool bLeftOver = x < nLeftMargin;
    if( bLeftOver )
        x = nLeftMargin;
    const sal_Bool bRightOver = x > nRightMargin;
    if( bRightOver )
        x = nRightMargin;

    sal_Bool bRightAllowed = pCMS && ( pCMS->eState == MV_NONE );

    // Until here everything in document coordinates.
    x -= nLeftMargin;

    KSHORT nX = KSHORT( x );

    // If there are attribut changes in the line, search for the paragraph,
    // in which nX is situated.
    SwLinePortion *pPor = pCurr->GetFirstPortion();
    xub_StrLen nCurrStart  = nStart;
    sal_Bool bHolePortion = sal_False;
    sal_Bool bLastHyph = sal_False;

    std::deque<sal_uInt16> *pKanaComp = pCurr->GetpKanaComp();
    xub_StrLen nOldIdx = GetInfo().GetIdx();
    MSHORT nSpaceIdx = 0;
    size_t nKanaIdx = 0;
    long nSpaceAdd = pCurr->IsSpaceAdd() ? pCurr->GetLLSpaceAdd( 0 ) : 0;
    short nKanaComp = pKanaComp ? (*pKanaComp)[0] : 0;

    // nWidth is the width of the line, or the width of
    // the paragraph with the font change, in which nX is situated.

    KSHORT nWidth = pPor->Width();
    if ( pCurr->IsSpaceAdd() || pKanaComp )
    {
        if ( pPor->InSpaceGrp() && nSpaceAdd )
        {
            ((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
            nWidth = nWidth + sal_uInt16( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
        }
        if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
            ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
          )
        {
            if ( pCurr->IsSpaceAdd() )
            {
                if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                    nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                else
                    nSpaceAdd = 0;
            }

            if( pKanaComp )
            {
                if ( nKanaIdx + 1 < pKanaComp->size() )
                    nKanaComp = (*pKanaComp)[++nKanaIdx];
                else
                    nKanaComp = 0;
            }
        }
    }

    KSHORT nWidth30;
    if ( pPor->IsPostItsPortion() )
        nWidth30 = 30 + pPor->GetViewWidth( GetInfo() ) / 2;
    else
        nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFldGrp() ?
                     30 :
                     nWidth;

    while( pPor->GetPortion() && nWidth30 < nX && !pPor->IsBreakPortion() )
    {
        nX = nX - nWidth;
        nCurrStart = nCurrStart + pPor->GetLen();
        bHolePortion = pPor->IsHolePortion();
        pPor = pPor->GetPortion();
        nWidth = pPor->Width();
        if ( pCurr->IsSpaceAdd() || pKanaComp )
        {
            if ( pPor->InSpaceGrp() && nSpaceAdd )
            {
                ((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
                nWidth = nWidth + sal_uInt16( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
            }

            if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
                ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
              )
            {
                if ( pCurr->IsSpaceAdd() )
                {
                    if ( ++nSpaceIdx < pCurr->GetLLSpaceAddCount() )
                        nSpaceAdd = pCurr->GetLLSpaceAdd( nSpaceIdx );
                    else
                        nSpaceAdd = 0;
                }

                if ( pKanaComp )
                {
                    if( nKanaIdx + 1 < pKanaComp->size() )
                        nKanaComp = (*pKanaComp)[++nKanaIdx];
                    else
                        nKanaComp = 0;
                }
            }
        }

        if ( pPor->IsPostItsPortion() )
            nWidth30 = 30 +  pPor->GetViewWidth( GetInfo() ) / 2;
        else
            nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFldGrp() ?
                         30 :
                         nWidth;
        if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
            bLastHyph = pPor->InHyphGrp();
    }

    const sal_Bool bLastPortion = (0 == pPor->GetPortion());

    if( nX==nWidth )
    {
        SwLinePortion *pNextPor = pPor->GetPortion();
        while( pNextPor && pNextPor->InFldGrp() && !pNextPor->Width() )
        {
            nCurrStart = nCurrStart + pPor->GetLen();
            pPor = pNextPor;
            if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
                bLastHyph = pPor->InHyphGrp();
            pNextPor = pPor->GetPortion();
        }
    }

    ((SwTxtSizeInfo&)GetInfo()).SetIdx( nOldIdx );

    xub_StrLen nLength = pPor->GetLen();

    sal_Bool bFieldInfo = pCMS && pCMS->bFieldInfo;

    if( bFieldInfo && ( nWidth30 < nX || bRightOver || bLeftOver ||
        ( pPor->InNumberGrp() && !pPor->IsFtnNumPortion() ) ||
        ( pPor->IsMarginPortion() && nWidth > nX + 30 ) ) )
        ((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;


    // #i27615#
    if (pCMS)
    {
        if( pCMS->bInFrontOfLabel)
        {
            if (! (2 * nX < nWidth && pPor->InNumberGrp() &&
                   !pPor->IsFtnNumPortion()))
                pCMS->bInFrontOfLabel = sal_False;
        }
    }

    // 7684: We are exactly ended up at ther HyphPortion. It is our task to
    // provide, that we end up in the String.
    // 7993: If length = 0, then we must exit...
    if( !nLength )
    {
        if( pCMS )
        {
            if( pPor->IsFlyPortion() && bFieldInfo )
                ((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;

            if (!bRightOver && nX)
            {
                if( pPor->IsFtnNumPortion())
                    ((SwCrsrMoveState*)pCMS)->bFtnNoInfo = sal_True;
                else if (pPor->InNumberGrp() ) // #i23726#
                {
                    ((SwCrsrMoveState*)pCMS)->nInNumPostionOffset = nX;
                    ((SwCrsrMoveState*)pCMS)->bInNumPortion = sal_True;
                }
            }
        }
        if( !nCurrStart )
            return 0;

        // 7849, 7816: pPor->GetHyphPortion is mandatory!
        if( bHolePortion || ( !bRightAllowed && bLastHyph ) ||
            ( pPor->IsMarginPortion() && !pPor->GetPortion() &&
              // 46598: Consider the situation: We might end up behind the last character,
              // in the last line of a centered paragraph
              nCurrStart < rText.getLength() ) )
            --nCurrStart;
        else if( pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()
                 && nWidth > nX )
        {
            if( bFieldInfo )
                --nCurrStart;
            else
            {
                KSHORT nHeight = pPor->Height();
                if ( !nHeight || nHeight > nWidth )
                    nHeight = nWidth;
                if( nChgNode && nWidth - nHeight/2 > nX )
                    --nCurrStart;
            }
        }
        return nCurrStart;
    }
    if ( 1 == nLength )
    {
        if ( nWidth )
        {
            // Else we may not enter the character-supplying frame...
            if( !( nChgNode && pPos && pPor->IsFlyCntPortion() ) )
            {
                if ( pPor->InFldGrp() ||
                     ( pPor->IsMultiPortion() &&
                       ((SwMultiPortion*)pPor)->IsBidi()  ) )
                {
                    KSHORT nHeight = 0;
                    if( !bFieldInfo )
                    {
                        nHeight = pPor->Height();
                        if ( !nHeight || nHeight > nWidth )
                            nHeight = nWidth;
                    }

                    if( nWidth - nHeight/2 <= nX &&
                        ( ! pPor->InFldGrp() ||
                          !((SwFldPortion*)pPor)->HasFollow() ) )
                        ++nCurrStart;
                }
                else if ( ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                    !pPor->GetPortion()->IsMarginPortion() &&
                    !pPor->GetPortion()->IsHolePortion() ) )
                         && ( nWidth/2 < nX ) &&
                         ( !bFieldInfo ||
                            ( pPor->GetPortion() &&
                              pPor->GetPortion()->IsPostItsPortion() ) )
                         && ( bRightAllowed || !bLastHyph ))
                    ++nCurrStart;

                // if we want to get the position inside the field, we should not return
                if ( !pCMS || !pCMS->pSpecialPos )
                    return nCurrStart;
            }
        }
        else
        {
            if ( pPor->IsPostItsPortion() || pPor->IsBreakPortion() ||
                 pPor->InToxRefGrp() )
                return nCurrStart;
            if ( pPor->InFldGrp() )
            {
                if( bRightOver && !((SwFldPortion*)pPor)->HasFollow() )
                    ++nCurrStart;
                return nCurrStart;
            }
        }
    }

    // Skip space at the end of the line
    if( bLastPortion && (pCurr->GetNext() || pFrm->GetFollow() )
        && rText[nCurrStart + nLength - 1] == ' ' )
        --nLength;

    if( nWidth > nX ||
      ( nWidth == nX && pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsDouble() ) )
    {
        if( pPor->IsMultiPortion() )
        {
            // In a multi-portion we use GetCrsrOfst()-function recursively
            SwTwips nTmpY = rPoint.Y() - pCurr->GetAscent() + pPor->GetAscent();
            // if we are in the first line of a double line portion, we have
            // to add a value to nTmpY for not staying in this line
            // we also want to skip the first line, if we are inside ruby
            if ( ( ((SwTxtSizeInfo*)pInf)->IsMulti() &&
                   ((SwTxtSizeInfo*)pInf)->IsFirstMulti() ) ||
                 ( ((SwMultiPortion*)pPor)->IsRuby() &&
                   ((SwMultiPortion*)pPor)->OnTop() ) )
                nTmpY += ((SwMultiPortion*)pPor)->Height();

            // Important for cursor traveling in ruby portions:
            // We have to set nTmpY to 0 in order to stay in the first row
            // if the phonetic line is the second row
            if (   ((SwMultiPortion*)pPor)->IsRuby() &&
                 ! ((SwMultiPortion*)pPor)->OnTop() )
                nTmpY = 0;

            SwTxtCursorSave aSave( (SwTxtCursor*)this, (SwMultiPortion*)pPor,
                 nTmpY, nX, nCurrStart, nSpaceAdd );

            SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
            if ( ((SwMultiPortion*)pPor)->IsBidi() )
            {
                const sal_uInt8 nBidiLevel = ((SwBidiPortion*)pPor)->GetLevel();
                aLayoutModeModifier.Modify( nBidiLevel % 2 );
            }

            if( ((SwMultiPortion*)pPor)->HasRotation() )
            {
                nTmpY -= nY;
                if( !((SwMultiPortion*)pPor)->IsRevers() )
                    nTmpY = pPor->Height() - nTmpY;
                if( nTmpY < 0 )
                    nTmpY = 0;
                nX = (KSHORT)nTmpY;
            }

            if( ((SwMultiPortion*)pPor)->HasBrackets() )
            {
                sal_uInt16 nPreWidth = ((SwDoubleLinePortion*)pPor)->PreWidth();
                if ( nX > nPreWidth )
                    nX = nX - nPreWidth;
                else
                    nX = 0;
            }

            return GetCrsrOfst( pPos, Point( GetLineStart() + nX, rPoint.Y() ),
                                nChgNode, pCMS );
        }
        if( pPor->InTxtGrp() )
        {
            sal_uInt8 nOldProp;
            if( GetPropFont() )
            {
                ((SwFont*)GetFnt())->SetProportion( GetPropFont() );
                nOldProp = GetFnt()->GetPropr();
            }
            else
                nOldProp = 0;
            {
                SwTxtSizeInfo aSizeInf( GetInfo(), &rText, nCurrStart );
                ((SwTxtCursor*)this)->SeekAndChg( aSizeInf );
                SwTxtSlot aDiffTxt( &aSizeInf, ((SwTxtPortion*)pPor), false, false );
                SwFontSave aSave( aSizeInf, pPor->IsDropPortion() ?
                        ((SwDropPortion*)pPor)->GetFnt() : NULL );

                SwParaPortion* pPara = (SwParaPortion*)GetInfo().GetParaPortion();
                OSL_ENSURE( pPara, "No paragraph!" );

                SwDrawTextInfo aDrawInf( aSizeInf.GetVsh(),
                                         *aSizeInf.GetOut(),
                                         &pPara->GetScriptInfo(),
                                         aSizeInf.GetTxt(),
                                         aSizeInf.GetIdx(),
                                         pPor->GetLen() );

                // Drop portion works like a multi portion, just its parts are not portions
                if( pPor->IsDropPortion() )
                {
                    SwDropPortion* pDrop = static_cast<SwDropPortion*>(pPor);
                    const SwDropPortionPart* pCurrPart = pDrop->GetPart();
                    sal_uInt16 nSumWidth = 0;
                    sal_uInt16 nSumBorderWidth = 0;
                    // Shift offset with the right and left border of previous parts and left border of actual one
                    while( pCurrPart && nSumWidth <= nX - nCurrStart )
                    {
                        nSumWidth += pCurrPart->GetWidth();
                        if( pCurrPart->GetFont().GetLeftBorder() )
                        {
                            nSumBorderWidth += pCurrPart->GetFont().GetLeftBorder().get().GetScaledWidth();
                        }
                        if( nSumWidth <= nX - nCurrStart && pCurrPart->GetFont().GetRightBorder() )
                        {
                            nSumBorderWidth += pCurrPart->GetFont().GetRightBorder().get().GetScaledWidth();
                        }
                        pCurrPart = pCurrPart->GetFollow();
                    }
                    nX = std::max(0, nX - nSumBorderWidth);
                }
                // Shift the offset with the left border width
                else if (GetInfo().GetFont()->GetLeftBorder() )
                    nX = std::max(0, nX - GetInfo().GetFont()->GetLeftBorder().get().GetScaledWidth());


                aDrawInf.SetOfst( nX );

                if ( nSpaceAdd )
                {
                    sal_Int32 nCharCnt = 0;
                    // #i41860# Thai justified alignemt needs some
                    // additional information:
                    aDrawInf.SetNumberOfBlanks( pPor->InTxtGrp() ?
                                                static_cast<const SwTxtPortion*>(pPor)->GetSpaceCnt( aSizeInf, nCharCnt ) :
                                                0 );
                }

                if ( pPor->InFldGrp() && pCMS && pCMS->pSpecialPos )
                    aDrawInf.SetLen( STRING_LEN ); // SMARTTAGS

                aDrawInf.SetSpace( nSpaceAdd );
                aDrawInf.SetFont( aSizeInf.GetFont() );
                aDrawInf.SetFrm( pFrm );
                aDrawInf.SetSnapToGrid( aSizeInf.SnapToGrid() );
                aDrawInf.SetPosMatchesBounds( pCMS && pCMS->bPosMatchesBounds );

                if ( SW_CJK == aSizeInf.GetFont()->GetActual() &&
                     pPara->GetScriptInfo().CountCompChg() &&
                    ! pPor->InFldGrp() )
                    aDrawInf.SetKanaComp( nKanaComp );

                nLength = aSizeInf.GetFont()->_GetCrsrOfst( aDrawInf );

                // get position inside field portion?
                if ( pPor->InFldGrp() && pCMS && pCMS->pSpecialPos )
                {
                    pCMS->pSpecialPos->nCharOfst = nLength;
                    nLength = 0; // SMARTTAGS
                }

                // set cursor bidi level
                if ( pCMS )
                    ((SwCrsrMoveState*)pCMS)->nCursorBidiLevel =
                        aDrawInf.GetCursorBidiLevel();

                if( bFieldInfo && nLength == pPor->GetLen() &&
                    ( ! pPor->GetPortion() ||
                      ! pPor->GetPortion()->IsPostItsPortion() ) )
                    --nLength;
            }
            if( nOldProp )
                ((SwFont*)GetFnt())->SetProportion( nOldProp );
        }
        else
        {
            if( nChgNode && pPos && pPor->IsFlyCntPortion()
                && !( (SwFlyCntPortion*)pPor )->IsDraw() )
            {
                // JP 24.11.94: if the Position is not in Fly, then
                //              we many not return with STRING_LEN as value!
                //              (BugId: 9692 + Change in feshview)
                SwFlyInCntFrm *pTmp = ( (SwFlyCntPortion*)pPor )->GetFlyFrm();
                sal_Bool bChgNode = 1 < nChgNode;
                if( !bChgNode )
                {
                    SwFrm* pLower = pTmp->GetLower();
                    if( pLower && (pLower->IsTxtFrm() || pLower->IsLayoutFrm()) )
                        bChgNode = sal_True;
                }
                Point aTmpPoint( rPoint );

                if ( pFrm->IsRightToLeft() )
                    pFrm->SwitchLTRtoRTL( aTmpPoint );

                if ( pFrm->IsVertical() )
                    pFrm->SwitchHorizontalToVertical( aTmpPoint );

                if( bChgNode && pTmp->Frm().IsInside( aTmpPoint ) &&
                    !( pTmp->IsProtected() ) )
                {
                    nLength = ((SwFlyCntPortion*)pPor)->
                              GetFlyCrsrOfst( nX, aTmpPoint, pPos, pCMS );
                    // After a change of the frame, our font must be still
                    // available for/in the OutputDevice.
                    // For comparison: Paint and new SwFlyCntPortion !
                    ((SwTxtSizeInfo*)pInf)->SelectFont();

                    // 6776: The pIter->GetCrsrOfst is returning here
                    // from a nesting with STRING_LEN.
                    return STRING_LEN;
                }
            }
            else
                nLength = pPor->GetCrsrOfst( nX );
        }
    }
    nOffset = nCurrStart + nLength;

    // 7684: We end up in front of the HyphPortion. We must assure
    // that we end up in the string.
    // If we are at end of line in front of FlyFrms, we must proceed the same way.
    if( nOffset && pPor->GetLen() == nLength && pPor->GetPortion() &&
        !pPor->GetPortion()->GetLen() && pPor->GetPortion()->InHyphGrp() )
        --nOffset;

    return nOffset;
}

/** Looks for text portions which are inside the given rectangle

    For a rectangular text selection every text portions which is inside the given
    rectangle has to be put into the SwSelectionList as SwPaM
    From these SwPaM the SwCursors will be created.

    @param rSelList
    The container for the overlapped text portions

    @param rRect
    A rectangle in document coordinates, text inside this rectangle has to be
    selected.

    @return [ true, false ]
    true if any overlapping text portion has been found and put into list
    false if no portion overlaps, the list has been unchanged
*/
bool SwTxtFrm::FillSelection( SwSelectionList& rSelList, const SwRect& rRect ) const
{
    bool bRet = false;
    // PaintArea() instead Frm() for negative indents
    SwRect aTmpFrm( PaintArea() );
    if( !rRect.IsOver( aTmpFrm ) )
        return false;
    if( rSelList.checkContext( this ) )
    {
        SwRect aRect( aTmpFrm );
        aRect.Intersection( rRect );
        // rNode without const to create SwPaMs
        SwCntntNode &rNode = const_cast<SwCntntNode&>( *GetNode() );
        SwNodeIndex aIdx( rNode );
        SwPosition aPosL( aIdx, SwIndex( &rNode, 0 ) );
        if( IsEmpty() )
        {
            SwPaM *pPam = new SwPaM( aPosL, aPosL );
            rSelList.insertPaM( pPam );
        }
        else if( aRect.HasArea() )
        {
            xub_StrLen nOld = STRING_LEN;
            SwPosition aPosR( aPosL );
            Point aPoint;
            SwTxtInfo aInf( const_cast<SwTxtFrm*>(this) );
            SwTxtIter aLine( const_cast<SwTxtFrm*>(this), &aInf );
            // We have to care for top-to-bottom layout, where right becomes top etc.
            SWRECTFN( this )
            SwTwips nTop = (aRect.*fnRect->fnGetTop)();
            SwTwips nBottom = (aRect.*fnRect->fnGetBottom)();
            SwTwips nLeft = (aRect.*fnRect->fnGetLeft)();
            SwTwips nRight = (aRect.*fnRect->fnGetRight)();
            SwTwips nY = aLine.Y(); // Top position of the first line
            SwTwips nLastY = nY;
            while( nY < nTop && aLine.Next() ) // line above rectangle
            {
                nLastY = nY;
                nY = aLine.Y();
            }
            bool bLastLine = false;
            if( nY < nTop && !aLine.GetNext() )
            {
                bLastLine = true;
                nY += aLine.GetLineHeight();
            }
            do // check the lines for overlapping
            {
                if( nLastY < nTop ) // if the last line was above rectangle
                    nLastY = nTop;
                if( nY > nBottom ) // if the current line leaves the rectangle
                    nY = nBottom;
                if( nY >= nLastY ) // gotcha: overlapping
                {
                    nLastY += nY;
                    nLastY /= 2;
                    if( bVert )
                    {
                        aPoint.X() = nLastY;
                        aPoint.Y() = nLeft;
                    }
                    else
                    {
                        aPoint.X() = nLeft;
                        aPoint.Y() = nLastY;
                    }
                    // Looking for the position of the left border of the rectangle
                    // in this text line
                    SwCrsrMoveState aState( MV_UPDOWN );
                    if( GetCrsrOfst( &aPosL, aPoint, &aState ) )
                    {
                        if( bVert )
                        {
                            aPoint.X() = nLastY;
                            aPoint.Y() = nRight;
                        }
                        else
                        {
                            aPoint.X() = nRight;
                            aPoint.Y() = nLastY;
                        }
                        // If we get a right position and if the left position
                        // is not the same like the left position of the line before
                        // which cound happen e.g. for field portions or fly frames
                        // a SwPaM will be inserted with these positions
                        if( GetCrsrOfst( &aPosR, aPoint, &aState ) &&
                            nOld != aPosL.nContent.GetIndex() )
                        {
                            SwPaM *pPam = new SwPaM( aPosL, aPosR );
                            rSelList.insertPaM( pPam );
                            nOld = aPosL.nContent.GetIndex();
                        }
                    }
                }
                if( aLine.Next() )
                {
                    nLastY = nY;
                    nY = aLine.Y();
                }
                else if( !bLastLine )
                {
                    bLastLine = true;
                    nLastY = nY;
                    nY += aLine.GetLineHeight();
                }
                else
                    break;
            }while( nLastY < nBottom );
        }
    }
    if( GetDrawObjs() )
    {
        const SwSortedObjs &rObjs = *GetDrawObjs();
        for ( sal_uInt16 i = 0; i < rObjs.Count(); ++i )
        {
            const SwAnchoredObject* pAnchoredObj = rObjs[i];
            if( !pAnchoredObj->ISA(SwFlyFrm) )
                continue;
            const SwFlyFrm* pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
            if( pFly->IsFlyInCntFrm() && pFly->FillSelection( rSelList, rRect ) )
                bRet = true;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
