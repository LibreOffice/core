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

#include <hintids.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <frmfmt.hxx>
#include <paratr.hxx>
#include <flyfrm.hxx>
#include <pam.hxx>
#include <swselectionlist.hxx>
#include <sortedobjs.hxx>
#include <editeng/protitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/borderline.hxx>
#include <frmatr.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagefrm.hxx>

#include "itrtxt.hxx"
#include <txtfrm.hxx>
#include <flyfrms.hxx>
#include "porglue.hxx"
#include "porfld.hxx"
#include "porfly.hxx"
#include "pordrop.hxx"
#include <crstate.hxx>
#include "pormulti.hxx"
#include <numrule.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

// Not reentrant !!!
// is set in GetCharRect and is interpreted in UnitUp/Down.
bool SwTextCursor::bRightMargin = false;

// After calculating the position of a character during GetCharRect
// this function allows to find the coordinates of a position (defined
// in pCMS->pSpecialPos) inside a special portion (e.g., a field)
static void lcl_GetCharRectInsideField( SwTextSizeInfo& rInf, SwRect& rOrig,
                                 const SwCursorMoveState& rCMS,
                                 const SwLinePortion& rPor )
{
    OSL_ENSURE( rCMS.m_pSpecialPos, "Information about special pos missing" );

    if ( rPor.InFieldGrp() && !static_cast<const SwFieldPortion&>(rPor).GetExp().isEmpty() )
    {
        const sal_Int32 nCharOfst = rCMS.m_pSpecialPos->nCharOfst;
        sal_Int32 nFieldIdx = 0;
        sal_Int32 nFieldLen = 0;

        OUString sString;
        const OUString* pString = nullptr;
        const SwLinePortion* pPor = &rPor;
        do
        {
            if ( pPor->InFieldGrp() )
            {
                sString = static_cast<const SwFieldPortion*>(pPor)->GetExp();
                pString = &sString;
                nFieldLen = pString->getLength();
            }
            else
            {
                pString = nullptr;
                nFieldLen = 0;
            }

            if ( ! pPor->GetNextPortion() || nFieldIdx + nFieldLen > nCharOfst )
                break;

            nFieldIdx = nFieldIdx + nFieldLen;
            rOrig.Pos().AdjustX(pPor->Width() );
            pPor = pPor->GetNextPortion();

        } while ( true );

        OSL_ENSURE( nCharOfst >= nFieldIdx, "Request of position inside field failed" );
        sal_Int32 nLen = nCharOfst - nFieldIdx + 1;

        if ( pString )
        {
            // get script for field portion
            rInf.GetFont()->SetActual( SwScriptInfo::WhichFont(0, *pString) );

            TextFrameIndex const nOldLen = pPor->GetLen();
            const_cast<SwLinePortion*>(pPor)->SetLen(TextFrameIndex(nLen - 1));
            const SwTwips nX1 = pPor->GetLen() ?
                                pPor->GetTextSize( rInf ).Width() :
                                0;

            SwTwips nX2 = 0;
            if ( rCMS.m_bRealWidth )
            {
                const_cast<SwLinePortion*>(pPor)->SetLen(TextFrameIndex(nLen));
                nX2 = pPor->GetTextSize( rInf ).Width();
            }

            const_cast<SwLinePortion*>(pPor)->SetLen( nOldLen );

            rOrig.Pos().AdjustX(nX1 );
            rOrig.Width( ( nX2 > nX1 ) ?
                         ( nX2 - nX1 ) :
                           1 );
        }
    }
    else
    {
        // special cases: no common fields, e.g., graphic number portion,
        // FlyInCntPortions, Notes
        rOrig.Width( rCMS.m_bRealWidth && rPor.Width() ? rPor.Width() : 1 );
    }
}

// #i111284#
namespace {
    bool AreListLevelIndentsApplicableAndLabelAlignmentActive( const SwTextNode& rTextNode )
    {
        bool bRet( false );

        if ( rTextNode.GetNumRule() && rTextNode.AreListLevelIndentsApplicable() )
        {
            int nListLevel = rTextNode.GetActualListLevel();

            if (nListLevel < 0)
                nListLevel = 0;

            if (nListLevel >= MAXLEVEL)
                nListLevel = MAXLEVEL - 1;

            const SwNumFormat& rNumFormat =
                    rTextNode.GetNumRule()->Get( static_cast<sal_uInt16>(nListLevel) );
            if ( rNumFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                bRet = true;
            }
        }

        return bRet;
    }
} // end of anonymous namespace

void SwTextMargin::CtorInitTextMargin( SwTextFrame *pNewFrame, SwTextSizeInfo *pNewInf )
{
    CtorInitTextIter( pNewFrame, pNewInf );

    m_pInf = pNewInf;
    GetInfo().SetFont( GetFnt() );
    const SwTextNode *const pNode = m_pFrame->GetTextNodeForParaProps();

    const SvxLRSpaceItem &rSpace = pNode->GetSwAttrSet().GetLRSpace();
    // #i95907#
    // #i111284#
    const bool bListLevelIndentsApplicableAndLabelAlignmentActive(
        AreListLevelIndentsApplicableAndLabelAlignmentActive( *(m_pFrame->GetTextNodeForParaProps()) ) );

    // Carefully adjust the text formatting ranges.

    // This whole area desperately needs some rework. There are
    // quite a couple of values that need to be considered:
    // 1. paragraph indent
    // 2. paragraph first line indent
    // 3. numbering indent
    // 4. numbering spacing to text
    // 5. paragraph border
    // Note: These values have already been used during calculation
    // of the printing area of the paragraph.
    const int nLMWithNum = pNode->GetLeftMarginWithNum( true );
    if ( m_pFrame->IsRightToLeft() )
    {
        // this calculation is identical this the calculation for L2R layout - see below
        nLeft = m_pFrame->getFrameArea().Left() +
                m_pFrame->getFramePrintArea().Left() +
                nLMWithNum -
                pNode->GetLeftMarginWithNum() -
                // #i95907#
                // #i111284#
                // rSpace.GetLeft() + rSpace.GetTextLeft();
                ( bListLevelIndentsApplicableAndLabelAlignmentActive
                  ? 0
                  : ( rSpace.GetLeft() - rSpace.GetTextLeft() ) );
    }
    else
    {
        // #i95907#
        // #i111284#
        if ( bListLevelIndentsApplicableAndLabelAlignmentActive ||
             !pNode->getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) )
        {
            // this calculation is identical this the calculation for R2L layout - see above
            nLeft = m_pFrame->getFrameArea().Left() +
                    m_pFrame->getFramePrintArea().Left() +
                    nLMWithNum -
                    pNode->GetLeftMarginWithNum() -
                    // #i95907#
                    // #i111284#
                    ( bListLevelIndentsApplicableAndLabelAlignmentActive
                      ? 0
                      : ( rSpace.GetLeft() - rSpace.GetTextLeft() ) );
        }
        else
        {
            nLeft = m_pFrame->getFrameArea().Left() +
                    std::max( long( rSpace.GetTextLeft() + nLMWithNum ),
                         m_pFrame->getFramePrintArea().Left() );
        }
    }

    nRight = m_pFrame->getFrameArea().Left() + m_pFrame->getFramePrintArea().Left() + m_pFrame->getFramePrintArea().Width();

    if( nLeft >= nRight &&
         // #i53066# Omit adjustment of nLeft for numbered
         // paras inside cells inside new documents:
        ( pNode->getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) ||
          !m_pFrame->IsInTab() ||
          !nLMWithNum ) )
    {
        nLeft = m_pFrame->getFramePrintArea().Left() + m_pFrame->getFrameArea().Left();
        if( nLeft >= nRight )   // e.g. with large paragraph indentations in slim table columns
            nRight = nLeft + 1; // einen goennen wir uns immer
    }

    if( m_pFrame->IsFollow() && m_pFrame->GetOfst() )
        nFirst = nLeft;
    else
    {
        short nFLOfst = 0;
        long nFirstLineOfs = 0;
        if( !pNode->GetFirstLineOfsWithNum( nFLOfst ) &&
            rSpace.IsAutoFirst() )
        {
            nFirstLineOfs = GetFnt()->GetSize( GetFnt()->GetActual() ).Height();
            LanguageType const aLang = m_pFrame->GetLangOfChar(
                    TextFrameIndex(0), css::i18n::ScriptType::ASIAN);
            if (aLang != LANGUAGE_KOREAN && aLang != LANGUAGE_JAPANESE)
                nFirstLineOfs<<=1;

            const SvxLineSpacingItem *pSpace = m_aLineInf.GetLineSpacing();
            if( pSpace )
            {
                switch( pSpace->GetLineSpaceRule() )
                {
                    case SvxLineSpaceRule::Auto:
                    break;
                    case SvxLineSpaceRule::Min:
                    {
                        if( nFirstLineOfs < pSpace->GetLineHeight() )
                            nFirstLineOfs = pSpace->GetLineHeight();
                        break;
                    }
                    case SvxLineSpaceRule::Fix:
                        nFirstLineOfs = pSpace->GetLineHeight();
                    break;
                    default: OSL_FAIL( ": unknown LineSpaceRule" );
                }
                switch( pSpace->GetInterLineSpaceRule() )
                {
                    case SvxInterLineSpaceRule::Off:
                    break;
                    case SvxInterLineSpaceRule::Prop:
                    {
                        long nTmp = pSpace->GetPropLineSpace();
                        // 50% is the minimum, at 0% we switch to
                        // the default value 100%...
                        if( nTmp < 50 )
                            nTmp = nTmp ? 50 : 100;

                        nTmp *= nFirstLineOfs;
                        nTmp /= 100;
                        if( !nTmp )
                            ++nTmp;
                        nFirstLineOfs = nTmp;
                        break;
                    }
                    case SvxInterLineSpaceRule::Fix:
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
        if ( m_pFrame->IsRightToLeft() ||
             bListLevelIndentsApplicableAndLabelAlignmentActive ||
             !pNode->getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) )
        {
            nFirst = nLeft + nFirstLineOfs;
        }
        else
        {
              nFirst = m_pFrame->getFrameArea().Left() +
                     std::max( rSpace.GetTextLeft() + nLMWithNum+ nFirstLineOfs,
                          m_pFrame->getFramePrintArea().Left() );
        }

        // Note: <SwTextFrame::GetAdditionalFirstLineOffset()> returns a negative
        //       value for the new list label position and space mode LABEL_ALIGNMENT
        //       and label alignment CENTER and RIGHT in L2R layout respectively
        //       label alignment LEFT and CENTER in R2L layout
        nFirst += m_pFrame->GetAdditionalFirstLineOffset();

        if( nFirst >= nRight )
            nFirst = nRight - 1;
    }
    const SvxAdjustItem& rAdjust = m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetAdjust();
    nAdjust = rAdjust.GetAdjust();

    // left is left and right is right
    if ( m_pFrame->IsRightToLeft() )
    {
        if ( SvxAdjust::Left == nAdjust )
            nAdjust = SvxAdjust::Right;
        else if ( SvxAdjust::Right == nAdjust )
            nAdjust = SvxAdjust::Left;
    }

    m_bOneBlock = rAdjust.GetOneWord() == SvxAdjust::Block;
    m_bLastBlock = rAdjust.GetLastBlock() == SvxAdjust::Block;
    m_bLastCenter = rAdjust.GetLastBlock() == SvxAdjust::Center;

    // #i91133#
    mnTabLeft = pNode->GetLeftMarginForTabCalculation();

    DropInit();
}

void SwTextMargin::DropInit()
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

// The function is interpreting / observing / evaluating / keeping / respecting the first line indention and the specified width.
SwTwips SwTextMargin::GetLineStart() const
{
    SwTwips nRet = GetLeftMargin();
    if( GetAdjust() != SvxAdjust::Left &&
        !m_pCurr->GetFirstPortion()->IsMarginPortion() )
    {
        // If the first portion is a Margin, then the
        // adjustment is expressed by the portions.
        if( GetAdjust() == SvxAdjust::Right )
            nRet = Right() - CurrWidth();
        else if( GetAdjust() == SvxAdjust::Center )
            nRet += (GetLineWidth() - CurrWidth()) / 2;
    }
    return nRet;
}

void SwTextCursor::CtorInitTextCursor( SwTextFrame *pNewFrame, SwTextSizeInfo *pNewInf )
{
    CtorInitTextMargin( pNewFrame, pNewInf );
    // 6096: Attention, the iterators are derived!
    // GetInfo().SetOut( GetInfo().GetWin() );
}

// 1170: Ancient bug: Shift-End forgets the last character ...
void SwTextCursor::GetEndCharRect(SwRect* pOrig, const TextFrameIndex nOfst,
                                  SwCursorMoveState* pCMS, const long nMax )
{
    // 1170: Ambiguity of document positions
    bRightMargin = true;
    CharCursorToLine(nOfst);

    // Somehow twisted: nOfst names the position behind the last
    // character of the last line == This is the position in front of the first character
    // of the line, in which we are situated:
    if( nOfst != GetStart() || !m_pCurr->GetLen() )
    {
        // 8810: Master line RightMargin, after that LeftMargin
        GetCharRect( pOrig, nOfst, pCMS, nMax );
        bRightMargin = nOfst >= GetEnd() && nOfst < TextFrameIndex(GetInfo().GetText().getLength());
        return;
    }

    if( !GetPrev() || !GetPrev()->GetLen() || !PrevLine() )
    {
        GetCharRect( pOrig, nOfst, pCMS, nMax );
        return;
    }

    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    long nX = 0;
    long nLast = 0;
    SwLinePortion *pPor = m_pCurr->GetFirstPortion();

    sal_uInt16 nTmpHeight, nTmpAscent;
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    sal_uInt16 nPorHeight = nTmpHeight;
    sal_uInt16 nPorAscent = nTmpAscent;

    // Search for the last Text/EndPortion of the line
    while( pPor )
    {
        nX = nX + pPor->Width();
        if( pPor->InTextGrp() || ( pPor->GetLen() && !pPor->IsFlyPortion()
            && !pPor->IsHolePortion() ) || pPor->IsBreakPortion() )
        {
            nLast = nX;
            nPorHeight = pPor->Height();
            nPorAscent = pPor->GetAscent();
        }
        pPor = pPor->GetNextPortion();
    }

    const Size aCharSize( 1, nTmpHeight );
    pOrig->Pos( GetTopLeft() );
    pOrig->SSize( aCharSize );
    pOrig->Pos().AdjustX(nLast );
    const SwTwips nTmpRight = Right() - 1;
    if( pOrig->Left() > nTmpRight )
        pOrig->Pos().setX( nTmpRight );

    if ( pCMS && pCMS->m_bRealHeight )
    {
        if ( nTmpAscent > nPorAscent )
            pCMS->m_aRealHeight.setX( nTmpAscent - nPorAscent );
        else
            pCMS->m_aRealHeight.setX( 0 );
        OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
        pCMS->m_aRealHeight.setY( nPorHeight );
    }
}

// internal function, called by SwTextCursor::GetCharRect() to calculate
// the relative character position in the current line.
// pOrig referes to x and y coordinates, width and height of the cursor
// pCMS is used for restricting the cursor, if there are different font
// heights in one line ( first value = offset to y of pOrig, second
// value = real height of (shortened) cursor
void SwTextCursor::GetCharRect_( SwRect* pOrig, TextFrameIndex const nOfst,
    SwCursorMoveState* pCMS )
{
    const OUString aText = GetInfo().GetText();
    SwTextSizeInfo aInf( GetInfo(), &aText, m_nStart );
    if( GetPropFont() )
        aInf.GetFont()->SetProportion( GetPropFont() );
    sal_uInt16 nTmpAscent, nTmpHeight;  // Line height
    CalcAscentAndHeight( nTmpAscent, nTmpHeight );
    const Size  aCharSize( 1, nTmpHeight );
    const Point aCharPos;
    pOrig->Pos( aCharPos );
    pOrig->SSize( aCharSize );

    // If we are looking for a position inside a field which covers
    // more than one line we may not skip any "empty portions" at the
    // beginning of a line
    const bool bInsideFirstField = pCMS && pCMS->m_pSpecialPos &&
                                    ( pCMS->m_pSpecialPos->nLineOfst ||
                                      SwSPExtendRange::BEFORE ==
                                      pCMS->m_pSpecialPos->nExtendRange );

    bool bWidth = pCMS && pCMS->m_bRealWidth;
    if( !m_pCurr->GetLen() && !m_pCurr->Width() )
    {
        if ( pCMS && pCMS->m_bRealHeight )
        {
            pCMS->m_aRealHeight.setX( 0 );
            pCMS->m_aRealHeight.setY( nTmpHeight );
        }
    }
    else
    {
        sal_uInt16 nPorHeight = nTmpHeight;
        sal_uInt16 nPorAscent = nTmpAscent;
        SwTwips nX = 0;
        SwTwips nTmpFirst = 0;
        SwLinePortion *pPor = m_pCurr->GetFirstPortion();
        SwBidiPortion* pLastBidiPor = nullptr;
        TextFrameIndex nLastBidiIdx(-1);
        SwTwips nLastBidiPorWidth = 0;
        std::deque<sal_uInt16>* pKanaComp = m_pCurr->GetpKanaComp();
        sal_uInt16 nSpaceIdx = 0;
        size_t nKanaIdx = 0;
        long nSpaceAdd = m_pCurr->IsSpaceAdd() ? m_pCurr->GetLLSpaceAdd( 0 ) : 0;

        bool bNoText = true;

        // First all portions without Len at beginning of line are skipped.
        // Exceptions are the mean special portions from WhichFirstPortion:
        // Num, ErgoSum, FootnoteNum, FieldRests
        // 8477: but also the only Textportion of an empty line with
        // Right/Center-Adjustment! So not just pPor->GetExpandPortion() ...
        while( pPor && !pPor->GetLen() && ! bInsideFirstField )
        {
            nX += pPor->Width();
            if ( pPor->InSpaceGrp() && nSpaceAdd )
                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
            if( bNoText )
                nTmpFirst = nX;
            // 8670: EndPortions count once as TextPortions.
            // if( pPor->InTextGrp() || pPor->IsBreakPortion() )
            if( pPor->InTextGrp() || pPor->IsBreakPortion() || pPor->InTabGrp() )
            {
                bNoText = false;
                nTmpFirst = nX;
            }
            if( pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->HasTabulator() )
            {
                if ( m_pCurr->IsSpaceAdd() )
                {
                    if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                        nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
                    else
                        nSpaceAdd = 0;
                }

                if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                    ++nKanaIdx;
            }
            if( pPor->InFixMargGrp() )
            {
                if( pPor->IsMarginPortion() )
                    bNoText = false;
                else
                {
                    // fix margin portion => next SpaceAdd, KanaComp value
                    if ( m_pCurr->IsSpaceAdd() )
                    {
                        if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                            nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
                        else
                            nSpaceAdd = 0;
                    }

                    if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                        ++nKanaIdx;
                }
            }
            pPor = pPor->GetNextPortion();
        }

        if( !pPor )
        {
            // There's just Spezialportions.
            nX = nTmpFirst;
        }
        else
        {
            if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                (!pPor->InFieldGrp() || pPor->GetAscent() ) )
            {
                nPorHeight = pPor->Height();
                nPorAscent = pPor->GetAscent();
            }
            while( pPor && !pPor->IsBreakPortion() && ( aInf.GetIdx() < nOfst ||
                   ( bWidth && ( pPor->IsKernPortion() || pPor->IsMultiPortion() ) ) ) )
            {
                if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
                    (!pPor->InFieldGrp() || pPor->GetAscent() ) )
                {
                    nPorHeight = pPor->Height();
                    nPorAscent = pPor->GetAscent();
                }

                // If we are behind the portion, we add the portion width to
                // nX. Special case: nOfst = aInf.GetIdx() + pPor->GetLen().
                // For common portions (including BidiPortions) we want to add
                // the portion width to nX. For MultiPortions, nExtra = 0,
                // therefore we go to the 'else' branch and start a recursion.
                const TextFrameIndex nExtra( (pPor->IsMultiPortion()
                             && !static_cast<SwMultiPortion*>(pPor)->IsBidi()
                             && !bWidth)
                        ? 0 : 1 );
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
                            if ( m_pCurr->IsSpaceAdd() )
                            {
                                if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                                    nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
                                else
                                    nSpaceAdd = 0;
                            }

                            if ( pKanaComp &&
                                ( nKanaIdx + 1 ) < pKanaComp->size()
                                )
                                ++nKanaIdx;
                        }
                        if ( !pPor->IsFlyPortion() || ( pPor->GetNextPortion() &&
                                !pPor->GetNextPortion()->IsMarginPortion() ) )
                            nX += pPor->PrtWidth();
                    }
                    if( pPor->IsMultiPortion() )
                    {
                        if ( static_cast<SwMultiPortion*>(pPor)->HasTabulator() )
                        {
                            if ( m_pCurr->IsSpaceAdd() )
                            {
                                if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                                    nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
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
                        if ( static_cast<SwMultiPortion*>(pPor)->IsBidi() &&
                             aInf.GetIdx() + pPor->GetLen() == nOfst )
                        {
                             pLastBidiPor = static_cast<SwBidiPortion*>(pPor);
                             nLastBidiIdx = aInf.GetIdx();
                             nLastBidiPorWidth = pLastBidiPor->Width() +
                                                 pLastBidiPor->CalcSpacing( nSpaceAdd, aInf );
                        }
                    }

                    aInf.SetIdx( aInf.GetIdx() + pPor->GetLen() );
                    pPor = pPor->GetNextPortion();
                }
                else
                {
                    if( pPor->IsMultiPortion() )
                    {
                        nTmpAscent = AdjustBaseLine( *m_pCurr, pPor );
                        GetInfo().SetMulti( true );
                        pOrig->Pos().AdjustY(nTmpAscent - nPorAscent );

                        if( pCMS && pCMS->m_b2Lines )
                        {
                            const bool bRecursion (pCMS->m_p2Lines);
                            if ( !bRecursion )
                            {
                                pCMS->m_p2Lines.reset(new Sw2LinesPos);
                                pCMS->m_p2Lines->aLine = SwRect(aCharPos, aCharSize);
                            }

                            if( static_cast<SwMultiPortion*>(pPor)->HasRotation() )
                            {
                                if( static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                                    pCMS->m_p2Lines->nMultiType = MultiPortionType::ROT_270;
                                else
                                    pCMS->m_p2Lines->nMultiType = MultiPortionType::ROT_90;
                            }
                            else if( static_cast<SwMultiPortion*>(pPor)->IsDouble() )
                                pCMS->m_p2Lines->nMultiType = MultiPortionType::TWOLINE;
                            else if( static_cast<SwMultiPortion*>(pPor)->IsBidi() )
                                pCMS->m_p2Lines->nMultiType = MultiPortionType::BIDI;
                            else
                                pCMS->m_p2Lines->nMultiType = MultiPortionType::RUBY;

                            SwTwips nTmpWidth = pPor->Width();
                            if( nSpaceAdd )
                                nTmpWidth += pPor->CalcSpacing(nSpaceAdd, aInf);

                            SwRect aRect( Point(aCharPos.X() + nX, pOrig->Top() ),
                                          Size( nTmpWidth, pPor->Height() ) );

                            if ( ! bRecursion )
                                pCMS->m_p2Lines->aPortion = aRect;
                            else
                                pCMS->m_p2Lines->aPortion2 = aRect;
                        }

                        // In a multi-portion we use GetCharRect()-function
                        // recursively and must add the x-position
                        // of the multi-portion.
                        TextFrameIndex const nOldStart = m_nStart;
                        SwTwips nOldY = m_nY;
                        sal_uInt8 nOldProp = GetPropFont();
                        m_nStart = aInf.GetIdx();
                        SwLineLayout* pOldCurr = m_pCurr;
                        m_pCurr = &static_cast<SwMultiPortion*>(pPor)->GetRoot();
                        if( static_cast<SwMultiPortion*>(pPor)->IsDouble() )
                            SetPropFont( 50 );

                        SwTextGridItem const*const pGrid(
                                GetGridItem(GetTextFrame()->FindPageFrame()));
                        const bool bHasGrid = pGrid && GetInfo().SnapToGrid();
                        const sal_uInt16 nRubyHeight = bHasGrid ?
                                                   pGrid->GetRubyHeight() : 0;

                        if( m_nStart + m_pCurr->GetLen() <= nOfst && GetNext() &&
                            ( ! static_cast<SwMultiPortion*>(pPor)->IsRuby() ||
                                static_cast<SwMultiPortion*>(pPor)->OnTop() ) )
                        {
                            sal_uInt16 nOffset;
                            // in grid mode we may only add the height of the
                            // ruby line if ruby line is on top
                            if ( bHasGrid &&
                                static_cast<SwMultiPortion*>(pPor)->IsRuby() &&
                                static_cast<SwMultiPortion*>(pPor)->OnTop() )
                                nOffset = nRubyHeight;
                            else
                                nOffset = GetLineHeight();

                            pOrig->Pos().AdjustY(nOffset );
                            Next();
                        }

                        const bool bSpaceChg = static_cast<SwMultiPortion*>(pPor)->
                                                ChgSpaceAdd( m_pCurr, nSpaceAdd );
                        Point aOldPos = pOrig->Pos();

                        // Ok, for ruby portions in grid mode we have to
                        // temporarily set the inner line height to the
                        // outer line height because that value is needed
                        // for the adjustment inside the recursion
                        const sal_uInt16 nOldRubyHeight = m_pCurr->Height();
                        const sal_uInt16 nOldRubyRealHeight = m_pCurr->GetRealHeight();
                        const bool bChgHeight =
                                static_cast<SwMultiPortion*>(pPor)->IsRuby() && bHasGrid;

                        if ( bChgHeight )
                        {
                            m_pCurr->Height( pOldCurr->Height() - nRubyHeight );
                            m_pCurr->SetRealHeight( pOldCurr->GetRealHeight() -
                                                  nRubyHeight );
                        }

                        SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
                        if ( static_cast<SwMultiPortion*>(pPor)->IsBidi() )
                        {
                            aLayoutModeModifier.Modify(
                                static_cast<SwBidiPortion*>(pPor)->GetLevel() % 2 );
                        }

                        GetCharRect_( pOrig, nOfst, pCMS );

                        if ( bChgHeight )
                        {
                            m_pCurr->Height( nOldRubyHeight );
                            m_pCurr->SetRealHeight( nOldRubyRealHeight );
                        }

                        // if we are still in the first row of
                        // our 2 line multiportion, we use the FirstMulti flag
                        // to indicate this
                        if ( static_cast<SwMultiPortion*>(pPor)->IsDouble() )
                        {
                            // the recursion may have damaged our font size
                            SetPropFont( nOldProp );
                            GetInfo().GetFont()->SetProportion( 100 );

                            if ( m_pCurr == &static_cast<SwMultiPortion*>(pPor)->GetRoot() )
                            {
                                GetInfo().SetFirstMulti( true );

                                // we want to treat a double line portion like a
                                // single line portion, if there is no text in
                                // the second line
                                if ( !m_pCurr->GetNext() ||
                                     !m_pCurr->GetNext()->GetLen() )
                                    GetInfo().SetMulti( false );
                            }
                        }
                        // ruby portions are treated like single line portions
                        else if( static_cast<SwMultiPortion*>(pPor)->IsRuby() ||
                                 static_cast<SwMultiPortion*>(pPor)->IsBidi() )
                            GetInfo().SetMulti( false );

                        // calculate cursor values
                        if( static_cast<SwMultiPortion*>(pPor)->HasRotation() )
                        {
                            GetInfo().SetMulti( false );
                            long nTmp = pOrig->Width();
                            pOrig->Width( pOrig->Height() );
                            pOrig->Height( nTmp );
                            nTmp = pOrig->Left() - aOldPos.X();

                            // if we travel into our rotated portion from
                            // a line below, we have to take care, that the
                            // y coord in pOrig is less than line height:
                            if ( nTmp )
                                nTmp--;

                            pOrig->Pos().setX( nX + aOldPos.X() );
                            if( static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                                pOrig->Pos().setY( aOldPos.Y() + nTmp );
                            else
                                pOrig->Pos().setY( aOldPos.Y()
                                    + pPor->Height() - nTmp - pOrig->Height() );
                            if ( pCMS && pCMS->m_bRealHeight )
                            {
                                pCMS->m_aRealHeight.setY( -pCMS->m_aRealHeight.Y() );
                                // result for rotated multi portion is not
                                // correct for reverse (270 degree) portions
                                if( static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                                {
                                    if ( SvxParaVertAlignItem::Align::Automatic ==
                                         GetLineInfo().GetVertAlign() )
                                        // if vertical alignment is set to auto,
                                        // we switch from base line alignment
                                        // to centered alignment
                                        pCMS->m_aRealHeight.setX(
                                            ( pOrig->Width() +
                                              pCMS->m_aRealHeight.Y() ) / 2 );
                                    else
                                        pCMS->m_aRealHeight.setX(
                                            pOrig->Width() -
                                            pCMS->m_aRealHeight.X() +
                                            pCMS->m_aRealHeight.Y() );
                                }
                            }
                        }
                        else
                        {
                            pOrig->Pos().AdjustY(aOldPos.Y() );
                            if ( static_cast<SwMultiPortion*>(pPor)->IsBidi() )
                            {
                                const SwTwips nPorWidth = pPor->Width() +
                                                         pPor->CalcSpacing( nSpaceAdd, aInf );
                                const SwTwips nInsideOfst = pOrig->Pos().X();
                                pOrig->Pos().setX( nX + nPorWidth -
                                                   nInsideOfst - pOrig->Width() );
                            }
                            else
                                pOrig->Pos().AdjustX(nX );

                            if( static_cast<SwMultiPortion*>(pPor)->HasBrackets() )
                                pOrig->Pos().AdjustX(
                                    static_cast<SwDoubleLinePortion*>(pPor)->PreWidth() );
                        }

                        if( bSpaceChg )
                            SwDoubleLinePortion::ResetSpaceAdd( m_pCurr );

                        m_pCurr = pOldCurr;
                        m_nStart = nOldStart;
                        m_nY = nOldY;
                        m_bPrev = false;

                        return;
                    }
                    if ( pPor->PrtWidth() )
                    {
                        TextFrameIndex const nOldLen = pPor->GetLen();
                        pPor->SetLen( nOfst - aInf.GetIdx() );
                        aInf.SetLen( pPor->GetLen() );
                        if( nX || !pPor->InNumberGrp() )
                        {
                            SeekAndChg( aInf );
                            const bool bOldOnWin = aInf.OnWin();
                            aInf.SetOnWin( false ); // no BULLETs!
                            SwTwips nTmp = nX;
                            aInf.SetKanaComp( pKanaComp );
                            aInf.SetKanaIdx( nKanaIdx );
                            nX += pPor->GetTextSize( aInf ).Width();
                            aInf.SetOnWin( bOldOnWin );
                            if ( pPor->InSpaceGrp() && nSpaceAdd )
                                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
                            if( bWidth )
                            {
                                pPor->SetLen(pPor->GetLen() + TextFrameIndex(1));
                                aInf.SetLen( pPor->GetLen() );
                                aInf.SetOnWin( false ); // no BULLETs!
                                nTmp += pPor->GetTextSize( aInf ).Width();
                                aInf.SetOnWin( bOldOnWin );
                                if ( pPor->InSpaceGrp() && nSpaceAdd )
                                    nTmp += pPor->CalcSpacing(nSpaceAdd, aInf);
                                pOrig->Width( nTmp - nX );
                            }
                        }
                        pPor->SetLen( nOldLen );

                        // Shift the cursor with the right border width
                        // Note: nX remains positive because GetTextSize() also include the width of the right border
                        if( aInf.GetIdx() < nOfst && nOfst < aInf.GetIdx() + pPor->GetLen() )
                        {
                            // Find the current drop portion part and use its right border
                            if( pPor->IsDropPortion() && static_cast<SwDropPortion*>(pPor)->GetLines() > 1 )
                            {
                                SwDropPortion* pDrop = static_cast<SwDropPortion*>(pPor);
                                const SwDropPortionPart* pCurrPart = pDrop->GetPart();
                                TextFrameIndex nSumLength(0);
                                while( pCurrPart && (nSumLength += pCurrPart->GetLen()) < nOfst - aInf.GetIdx() )
                                {
                                    pCurrPart = pCurrPart->GetFollow();
                                }
                                if( pCurrPart && nSumLength != nOfst - aInf.GetIdx() &&
                                    pCurrPart->GetFont().GetRightBorder() && !pCurrPart->GetJoinBorderWithNext() )
                                {
                                    nX -= pCurrPart->GetFont().GetRightBorderSpace();
                                }
                            }
                            else if( GetInfo().GetFont()->GetRightBorder() && !pPor->GetJoinBorderWithNext())
                            {
                                nX -= GetInfo().GetFont()->GetRightBorderSpace();
                            }
                         }
                    }
                    bWidth = false;
                    break;
                }
            }
        }

        if( pPor )
        {
            OSL_ENSURE( !pPor->InNumberGrp() || bInsideFirstField, "Number surprise" );
            bool bEmptyField = false;
            if( pPor->InFieldGrp() && pPor->GetLen() )
            {
                SwFieldPortion *pTmp = static_cast<SwFieldPortion*>(pPor);
                while( pTmp->HasFollow() && pTmp->GetExp().isEmpty() )
                {
                    sal_uInt16 nAddX = pTmp->Width();
                    SwLinePortion *pNext = pTmp->GetNextPortion();
                    while( pNext && !pNext->InFieldGrp() )
                    {
                        OSL_ENSURE( !pNext->GetLen(), "Where's my field follow?" );
                        nAddX = nAddX + pNext->Width();
                        pNext = pNext->GetNextPortion();
                    }
                    if( !pNext )
                        break;
                    pTmp = static_cast<SwFieldPortion*>(pNext);
                    nPorHeight = pTmp->Height();
                    nPorAscent = pTmp->GetAscent();
                    nX += nAddX;
                    bEmptyField = true;
                }
            }
            // 8513: Fields in justified text, skipped
            while( pPor && !pPor->GetLen() && ! bInsideFirstField &&
                   ( pPor->IsFlyPortion() || pPor->IsKernPortion() ||
                     pPor->IsBlankPortion() || pPor->InTabGrp() ||
                     ( !bEmptyField && pPor->InFieldGrp() ) ) )
            {
                if ( pPor->InSpaceGrp() && nSpaceAdd )
                    nX += pPor->PrtWidth() +
                          pPor->CalcSpacing( nSpaceAdd, aInf );
                else
                {
                    if( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() )
                    {
                        if ( m_pCurr->IsSpaceAdd() )
                        {
                            if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                                nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
                            else
                                nSpaceAdd = 0;
                        }

                        if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->size() )
                            ++nKanaIdx;
                    }
                    if ( !pPor->IsFlyPortion() || ( pPor->GetNextPortion() &&
                            !pPor->GetNextPortion()->IsMarginPortion() ) )
                        nX += pPor->PrtWidth();
                }
                if( pPor->IsMultiPortion() &&
                    static_cast<SwMultiPortion*>(pPor)->HasTabulator() )
                {
                    if ( m_pCurr->IsSpaceAdd() )
                    {
                        if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                            nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
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
                pPor = pPor->GetNextPortion();
            }

            if( aInf.GetIdx() == nOfst && pPor && pPor->InHyphGrp() &&
                pPor->GetNextPortion() && pPor->GetNextPortion()->InFixGrp() )
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
                    pPor = pPor->GetNextPortion();
                }
            }
            if( pPor && pCMS )
            {
                if( pCMS->m_bFieldInfo && pPor->InFieldGrp() && pPor->Width() )
                    pOrig->Width( pPor->Width() );
                if( pPor->IsDropPortion() )
                {
                    nPorAscent = static_cast<SwDropPortion*>(pPor)->GetDropHeight();
                    // The drop height is only calculated, if we have more than
                    // one line. Otherwise it is 0.
                    if ( ! nPorAscent)
                        nPorAscent = pPor->Height();
                    nPorHeight = nPorAscent;
                    pOrig->Height( nPorHeight +
                        static_cast<SwDropPortion*>(pPor)->GetDropDescent() );
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
                    if (TextFrameIndex(2) > pPor->GetLen())
                    {
                        nTmp = pPor->Width();
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                    }
                    else
                    {
                        const bool bOldOnWin = aInf.OnWin();
                        TextFrameIndex const nOldLen = pPor->GetLen();
                        pPor->SetLen( TextFrameIndex(1) );
                        aInf.SetLen( pPor->GetLen() );
                        SeekAndChg( aInf );
                        aInf.SetOnWin( false ); // no BULLETs!
                        aInf.SetKanaComp( pKanaComp );
                        aInf.SetKanaIdx( nKanaIdx );
                        nTmp = pPor->GetTextSize( aInf ).Width();
                        aInf.SetOnWin( bOldOnWin );
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                        pPor->SetLen( nOldLen );
                    }
                    pOrig->Width( nTmp );
                }

                // travel inside field portion?
                if ( pCMS->m_pSpecialPos )
                {
                    // apply attributes to font
                    Seek( nOfst );
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
                  static_cast<SwMultiPortion*>(pPor)->IsBidi() ) ) )
        {
            // we determine if the cursor has to blink before or behind
            // the bidi portion
            if ( pLastBidiPor )
            {
                const sal_uInt8 nPortionLevel = pLastBidiPor->GetLevel();

                if ( pCMS->m_nCursorBidiLevel >= nPortionLevel )
                {
                    // we came from inside the bidi portion, we want to blink
                    // behind the portion
                    pOrig->Pos().AdjustX( -nLastBidiPorWidth );

                    // Again, there is a special case: logically behind
                    // the portion can actually mean that the cursor is inside
                    // the portion. This can happen is the last portion
                    // inside the bidi portion is a nested bidi portion
                    SwLineLayout& rLineLayout =
                            static_cast<SwMultiPortion*>(pLastBidiPor)->GetRoot();

                    const SwLinePortion *pLast = rLineLayout.FindLastPortion();
                    if ( pLast->IsMultiPortion() )
                    {
                        OSL_ENSURE( static_cast<const SwMultiPortion*>(pLast)->IsBidi(),
                                 "Non-BidiPortion inside BidiPortion" );
                        TextFrameIndex const nIdx = aInf.GetIdx();
                        // correct the index before using CalcSpacing.
                        aInf.SetIdx(nLastBidiIdx);
                        pOrig->Pos().AdjustX(pLast->Width() +
                                            pLast->CalcSpacing( nSpaceAdd, aInf ) );
                        aInf.SetIdx(nIdx);
                    }
                }
            }
            else
            {
                const sal_uInt8 nPortionLevel = static_cast<SwBidiPortion*>(pPor)->GetLevel();

                if ( pCMS->m_nCursorBidiLevel >= nPortionLevel )
                {
                    // we came from inside the bidi portion, we want to blink
                    // behind the portion
                    pOrig->Pos().AdjustX(pPor->Width() +
                                        pPor->CalcSpacing( nSpaceAdd, aInf ) );
                }
            }
        }

        pOrig->Pos().AdjustX(nX );

        if ( pCMS && pCMS->m_bRealHeight )
        {
            nTmpAscent = AdjustBaseLine( *m_pCurr, nullptr, nPorHeight, nPorAscent );
            if ( nTmpAscent > nPorAscent )
                pCMS->m_aRealHeight.setX( nTmpAscent - nPorAscent );
            else
                pCMS->m_aRealHeight.setX( 0 );
            OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
            if ( nTmpHeight > nPorHeight )
                pCMS->m_aRealHeight.setY( nPorHeight );
            else
                pCMS->m_aRealHeight.setY( nTmpHeight );
        }
    }
}

void SwTextCursor::GetCharRect( SwRect* pOrig, TextFrameIndex const nOfst,
                               SwCursorMoveState* pCMS, const long nMax )
{
    CharCursorToLine(nOfst);

    // Indicates that a position inside a special portion (field, number portion)
    // is requested.
    const bool bSpecialPos = pCMS && pCMS->m_pSpecialPos;
    TextFrameIndex nFindOfst = nOfst;

    if ( bSpecialPos )
    {
        const SwSPExtendRange nExtendRange = pCMS->m_pSpecialPos->nExtendRange;

        OSL_ENSURE( ! pCMS->m_pSpecialPos->nLineOfst || SwSPExtendRange::BEFORE != nExtendRange,
                "LineOffset AND Number Portion?" );

        // portions which are behind the string
        if ( SwSPExtendRange::BEHIND == nExtendRange )
            ++nFindOfst;

        // skip lines for fields which cover more than one line
        for ( sal_uInt16 i = 0; i < pCMS->m_pSpecialPos->nLineOfst; i++ )
            Next();
    }

    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    const Point aCharPos( GetTopLeft() );

    GetCharRect_( pOrig, nFindOfst, pCMS );

    // This actually would have to be "-1 LogicToPixel", but that seems too
    // expensive, so it's a value (-12), that should hopefully be OK.
    const SwTwips nTmpRight = Right() - 12;

    pOrig->Pos().AdjustX(aCharPos.X() );
    pOrig->Pos().AdjustY(aCharPos.Y() );

    if( pCMS && pCMS->m_b2Lines && pCMS->m_p2Lines )
    {
        pCMS->m_p2Lines->aLine.Pos().AdjustX(aCharPos.X() );
        pCMS->m_p2Lines->aLine.Pos().AdjustY(aCharPos.Y() );
        pCMS->m_p2Lines->aPortion.Pos().AdjustX(aCharPos.X() );
        pCMS->m_p2Lines->aPortion.Pos().AdjustY(aCharPos.Y() );
    }

    const bool bTabOverMargin = GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::TAB_OVER_MARGIN);
    // Make sure the cursor respects the right margin, unless in compat mode, where the tab size has priority over the margin size.
    if( pOrig->Left() > nTmpRight && !bTabOverMargin)
        pOrig->Pos().setX( nTmpRight );

    if( nMax )
    {
        if( pOrig->Top() + pOrig->Height() > nMax )
        {
            if( pOrig->Top() > nMax )
                pOrig->Top( nMax );
            pOrig->Height( nMax - pOrig->Top() );
        }
        if ( pCMS && pCMS->m_bRealHeight && pCMS->m_aRealHeight.Y() >= 0 )
        {
            long nTmp = pCMS->m_aRealHeight.X() + pOrig->Top();
            if( nTmp >= nMax )
            {
                pCMS->m_aRealHeight.setX( nMax - pOrig->Top() );
                pCMS->m_aRealHeight.setY( 0 );
            }
            else if( nTmp + pCMS->m_aRealHeight.Y() > nMax )
                pCMS->m_aRealHeight.setY( nMax - nTmp );
        }
    }
    long nOut = pOrig->Right() - GetTextFrame()->getFrameArea().Right();
    if( nOut > 0 )
    {
        if( GetTextFrame()->getFrameArea().Width() < GetTextFrame()->getFramePrintArea().Left()
                                   + GetTextFrame()->getFramePrintArea().Width() )
            nOut += GetTextFrame()->getFrameArea().Width() - GetTextFrame()->getFramePrintArea().Left()
                    - GetTextFrame()->getFramePrintArea().Width();
        if( nOut > 0 )
            pOrig->Pos().AdjustX( -(nOut + 10) );
    }
}

// Return: Offset in String
TextFrameIndex SwTextCursor::GetCursorOfst( SwPosition *pPos, const Point &rPoint,
                                    bool bChgNode, SwCursorMoveState* pCMS ) const
{
    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    const OUString &rText = GetInfo().GetText();
    TextFrameIndex nOffset(0);

    // x is the horizontal offset within the line.
    SwTwips x = rPoint.X();
    const SwTwips nLeftMargin  = GetLineStart();
    SwTwips nRightMargin = GetLineEnd() +
        ( GetCurr()->IsHanging() ? GetCurr()->GetHangingMargin() : 0 );
    if( nRightMargin == nLeftMargin )
        nRightMargin += 30;

    const bool bLeftOver = x < nLeftMargin;
    if( bLeftOver )
        x = nLeftMargin;
    const bool bRightOver = x > nRightMargin;
    if( bRightOver )
        x = nRightMargin;

    const bool bRightAllowed = pCMS && ( pCMS->m_eState == MV_NONE );

    // Until here everything in document coordinates.
    x -= nLeftMargin;

    sal_uInt16 nX = sal_uInt16( x );

    // If there are attribute changes in the line, search for the paragraph,
    // in which nX is situated.
    SwLinePortion *pPor = m_pCurr->GetFirstPortion();
    TextFrameIndex nCurrStart = m_nStart;
    bool bHolePortion = false;
    bool bLastHyph = false;

    std::deque<sal_uInt16> *pKanaComp = m_pCurr->GetpKanaComp();
    TextFrameIndex const nOldIdx = GetInfo().GetIdx();
    sal_uInt16 nSpaceIdx = 0;
    size_t nKanaIdx = 0;
    long nSpaceAdd = m_pCurr->IsSpaceAdd() ? m_pCurr->GetLLSpaceAdd( 0 ) : 0;
    short nKanaComp = pKanaComp ? (*pKanaComp)[0] : 0;

    // nWidth is the width of the line, or the width of
    // the paragraph with the font change, in which nX is situated.

    sal_uInt16 nWidth = pPor->Width();
    if ( m_pCurr->IsSpaceAdd() || pKanaComp )
    {
        if ( pPor->InSpaceGrp() && nSpaceAdd )
        {
            const_cast<SwTextSizeInfo&>(GetInfo()).SetIdx( nCurrStart );
            nWidth = nWidth + sal_uInt16( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
        }
        if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
            ( pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->HasTabulator() )
          )
        {
            if ( m_pCurr->IsSpaceAdd() )
            {
                if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                    nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
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

    sal_uInt16 nWidth30;
    if ( pPor->IsPostItsPortion() )
        nWidth30 = 30 + pPor->GetViewWidth( GetInfo() ) / 2;
    else
        nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFieldGrp() ?
                     30 :
                     nWidth;

    while( pPor->GetNextPortion() && nWidth30 < nX && !pPor->IsBreakPortion() )
    {
        nX = nX - nWidth;
        nCurrStart = nCurrStart + pPor->GetLen();
        bHolePortion = pPor->IsHolePortion();
        pPor = pPor->GetNextPortion();
        nWidth = pPor->Width();
        if ( m_pCurr->IsSpaceAdd() || pKanaComp )
        {
            if ( pPor->InSpaceGrp() && nSpaceAdd )
            {
                const_cast<SwTextSizeInfo&>(GetInfo()).SetIdx( nCurrStart );
                nWidth = nWidth + sal_uInt16( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
            }

            if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
                ( pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->HasTabulator() )
              )
            {
                if ( m_pCurr->IsSpaceAdd() )
                {
                    if ( ++nSpaceIdx < m_pCurr->GetLLSpaceAddCount() )
                        nSpaceAdd = m_pCurr->GetLLSpaceAdd( nSpaceIdx );
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
            nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFieldGrp() ?
                         30 :
                         nWidth;
        if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
            bLastHyph = pPor->InHyphGrp();
    }

    const bool bLastPortion = (nullptr == pPor->GetNextPortion());

    if( nX==nWidth )
    {
        SwLinePortion *pNextPor = pPor->GetNextPortion();
        while( pNextPor && pNextPor->InFieldGrp() && !pNextPor->Width() )
        {
            nCurrStart = nCurrStart + pPor->GetLen();
            pPor = pNextPor;
            if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
                bLastHyph = pPor->InHyphGrp();
            pNextPor = pPor->GetNextPortion();
        }
    }

    const_cast<SwTextSizeInfo&>(GetInfo()).SetIdx( nOldIdx );

    TextFrameIndex nLength = pPor->GetLen();

    const bool bFieldInfo = pCMS && pCMS->m_bFieldInfo;

    if( bFieldInfo && ( nWidth30 < nX || bRightOver || bLeftOver ||
        ( pPor->InNumberGrp() && !pPor->IsFootnoteNumPortion() ) ||
        ( pPor->IsMarginPortion() && nWidth > nX + 30 ) ) )
        pCMS->m_bPosCorr = true;

    // #i27615#
    if (pCMS && pCMS->m_bInFrontOfLabel)
    {
        if (! (2 * nX < nWidth && pPor->InNumberGrp() &&
               !pPor->IsFootnoteNumPortion()))
            pCMS->m_bInFrontOfLabel = false;
    }

    // 7684: We are exactly ended up at their HyphPortion. It is our task to
    // provide, that we end up in the String.
    // 7993: If length = 0, then we must exit...
    if( !nLength )
    {
        if( pCMS )
        {
            if( pPor->IsFlyPortion() && bFieldInfo )
                pCMS->m_bPosCorr = true;

            if (!bRightOver && nX)
            {
                if( pPor->IsFootnoteNumPortion())
                    pCMS->m_bFootnoteNoInfo = true;
                else if (pPor->InNumberGrp() ) // #i23726#
                {
                    pCMS->m_nInNumPortionOffset = nX;
                    pCMS->m_bInNumPortion = true;
                }
            }
        }
        if( !nCurrStart )
            return TextFrameIndex(0);

        // 7849, 7816: pPor->GetHyphPortion is mandatory!
        if( bHolePortion || ( !bRightAllowed && bLastHyph ) ||
            ( pPor->IsMarginPortion() && !pPor->GetNextPortion() &&
              // 46598: Consider the situation: We might end up behind the last character,
              // in the last line of a centered paragraph
              nCurrStart < TextFrameIndex(rText.getLength())))
            --nCurrStart;
        else if( pPor->InFieldGrp() && static_cast<SwFieldPortion*>(pPor)->IsFollow()
                 && nWidth > nX )
        {
            if( bFieldInfo )
                --nCurrStart;
            else
            {
                sal_uInt16 nHeight = pPor->Height();
                if ( !nHeight || nHeight > nWidth )
                    nHeight = nWidth;
                if( bChgNode && nWidth - nHeight/2 > nX )
                    --nCurrStart;
            }
        }
        return nCurrStart;
    }
    if (TextFrameIndex(1) == nLength)
    {
        if ( nWidth )
        {
            // no quick return for as-character frames, we want to peek inside
            if (!(bChgNode && pPos && pPor->IsFlyCntPortion())
            // if we want to get the position inside the field, we should not return
                && (!pCMS || !pCMS->m_pSpecialPos))
            {
                if ( pPor->InFieldGrp() ||
                     ( pPor->IsMultiPortion() &&
                       static_cast<SwMultiPortion*>(pPor)->IsBidi()  ) )
                {
                    sal_uInt16 nHeight = 0;
                    if( !bFieldInfo )
                    {
                        nHeight = pPor->Height();
                        if ( !nHeight || nHeight > nWidth )
                            nHeight = nWidth;
                    }

                    if( nWidth - nHeight/2 <= nX &&
                        ( ! pPor->InFieldGrp() ||
                          !static_cast<SwFieldPortion*>(pPor)->HasFollow() ) )
                        ++nCurrStart;
                }
                else if ( ( !pPor->IsFlyPortion() || ( pPor->GetNextPortion() &&
                    !pPor->GetNextPortion()->IsMarginPortion() &&
                    !pPor->GetNextPortion()->IsHolePortion() ) )
                         && ( nWidth/2 < nX ) &&
                         ( !bFieldInfo ||
                            ( pPor->GetNextPortion() &&
                              pPor->GetNextPortion()->IsPostItsPortion() ) )
                         && ( bRightAllowed || !bLastHyph ))
                    ++nCurrStart;

                return nCurrStart;
            }
        }
        else
        {
            if ( pPor->IsPostItsPortion() || pPor->IsBreakPortion() ||
                 pPor->InToxRefGrp() )
                return nCurrStart;
            if ( pPor->InFieldGrp() )
            {
                if( bRightOver && !static_cast<SwFieldPortion*>(pPor)->HasFollow() )
                    ++nCurrStart;
                return nCurrStart;
            }
        }
    }

    // Skip space at the end of the line
    if( bLastPortion && (m_pCurr->GetNext() || m_pFrame->GetFollow() )
        && rText[sal_Int32(nCurrStart + nLength) - 1] == ' ' )
        --nLength;

    if( nWidth > nX ||
      ( nWidth == nX && pPor->IsMultiPortion() && static_cast<SwMultiPortion*>(pPor)->IsDouble() ) )
    {
        if( pPor->IsMultiPortion() )
        {
            // In a multi-portion we use GetCursorOfst()-function recursively
            SwTwips nTmpY = rPoint.Y() - m_pCurr->GetAscent() + pPor->GetAscent();
            // if we are in the first line of a double line portion, we have
            // to add a value to nTmpY for not staying in this line
            // we also want to skip the first line, if we are inside ruby
            if ( ( static_cast<SwTextSizeInfo*>(m_pInf)->IsMulti() &&
                   static_cast<SwTextSizeInfo*>(m_pInf)->IsFirstMulti() ) ||
                 ( static_cast<SwMultiPortion*>(pPor)->IsRuby() &&
                   static_cast<SwMultiPortion*>(pPor)->OnTop() ) )
                nTmpY += static_cast<SwMultiPortion*>(pPor)->Height();

            // Important for cursor traveling in ruby portions:
            // We have to set nTmpY to 0 in order to stay in the first row
            // if the phonetic line is the second row
            if (   static_cast<SwMultiPortion*>(pPor)->IsRuby() &&
                 ! static_cast<SwMultiPortion*>(pPor)->OnTop() )
                nTmpY = 0;

            SwTextCursorSave aSave( const_cast<SwTextCursor*>(this), static_cast<SwMultiPortion*>(pPor),
                 nTmpY, nX, nCurrStart, nSpaceAdd );

            SwLayoutModeModifier aLayoutModeModifier( *GetInfo().GetOut() );
            if ( static_cast<SwMultiPortion*>(pPor)->IsBidi() )
            {
                const sal_uInt8 nBidiLevel = static_cast<SwBidiPortion*>(pPor)->GetLevel();
                aLayoutModeModifier.Modify( nBidiLevel % 2 );
            }

            if( static_cast<SwMultiPortion*>(pPor)->HasRotation() )
            {
                nTmpY -= m_nY;
                if( !static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                    nTmpY = pPor->Height() - nTmpY;
                if( nTmpY < 0 )
                    nTmpY = 0;
                nX = static_cast<sal_uInt16>(nTmpY);
            }

            if( static_cast<SwMultiPortion*>(pPor)->HasBrackets() )
            {
                const sal_uInt16 nPreWidth = static_cast<SwDoubleLinePortion*>(pPor)->PreWidth();
                if ( nX > nPreWidth )
                    nX = nX - nPreWidth;
                else
                    nX = 0;
            }

            return GetCursorOfst( pPos, Point( GetLineStart() + nX, rPoint.Y() ),
                                bChgNode, pCMS );
        }
        if( pPor->InTextGrp() )
        {
            sal_uInt8 nOldProp;
            if( GetPropFont() )
            {
                const_cast<SwFont*>(GetFnt())->SetProportion( GetPropFont() );
                nOldProp = GetFnt()->GetPropr();
            }
            else
                nOldProp = 0;
            {
                SwTextSizeInfo aSizeInf( GetInfo(), &rText, nCurrStart );
                const_cast<SwTextCursor*>(this)->SeekAndChg( aSizeInf );
                SwTextSlot aDiffText( &aSizeInf, static_cast<SwTextPortion*>(pPor), false, false );
                SwFontSave aSave( aSizeInf, pPor->IsDropPortion() ?
                        static_cast<SwDropPortion*>(pPor)->GetFnt() : nullptr );

                SwParaPortion* pPara = const_cast<SwParaPortion*>(GetInfo().GetParaPortion());
                OSL_ENSURE( pPara, "No paragraph!" );

                SwDrawTextInfo aDrawInf( aSizeInf.GetVsh(),
                                         *aSizeInf.GetOut(),
                                         &pPara->GetScriptInfo(),
                                         aSizeInf.GetText(),
                                         aSizeInf.GetIdx(),
                                         pPor->GetLen() );

                // Drop portion works like a multi portion, just its parts are not portions
                if( pPor->IsDropPortion() && static_cast<SwDropPortion*>(pPor)->GetLines() > 1 )
                {
                    SwDropPortion* pDrop = static_cast<SwDropPortion*>(pPor);
                    const SwDropPortionPart* pCurrPart = pDrop->GetPart();
                    sal_uInt16 nSumWidth = 0;
                    sal_uInt16 nSumBorderWidth = 0;
                    // Shift offset with the right and left border of previous parts and left border of actual one
                    while (pCurrPart && nSumWidth <= nX - sal_Int32(nCurrStart))
                    {
                        nSumWidth += pCurrPart->GetWidth();
                        if( pCurrPart->GetFont().GetLeftBorder() && !pCurrPart->GetJoinBorderWithPrev() )
                        {
                            nSumBorderWidth += pCurrPart->GetFont().GetLeftBorderSpace();
                        }
                        if (nSumWidth <= nX - sal_Int32(nCurrStart) && pCurrPart->GetFont().GetRightBorder() &&
                            !pCurrPart->GetJoinBorderWithNext() )
                        {
                            nSumBorderWidth += pCurrPart->GetFont().GetRightBorderSpace();
                        }
                        pCurrPart = pCurrPart->GetFollow();
                    }
                    nX = std::max(0, nX - nSumBorderWidth);
                }
                // Shift the offset with the left border width
                else if( GetInfo().GetFont()->GetLeftBorder() && !pPor->GetJoinBorderWithPrev() )
                {
                    nX = std::max(0, nX - GetInfo().GetFont()->GetLeftBorderSpace());
                }

                aDrawInf.SetOfst( nX );

                if ( nSpaceAdd )
                {
                    TextFrameIndex nCharCnt(0);
                    // #i41860# Thai justified alignment needs some
                    // additional information:
                    aDrawInf.SetNumberOfBlanks( pPor->InTextGrp() ?
                                                static_cast<const SwTextPortion*>(pPor)->GetSpaceCnt( aSizeInf, nCharCnt ) :
                                                TextFrameIndex(0) );
                }

                if ( pPor->InFieldGrp() && pCMS && pCMS->m_pSpecialPos )
                    aDrawInf.SetLen( TextFrameIndex(COMPLETE_STRING) );

                aDrawInf.SetSpace( nSpaceAdd );
                aDrawInf.SetFont( aSizeInf.GetFont() );
                aDrawInf.SetFrame( m_pFrame );
                aDrawInf.SetSnapToGrid( aSizeInf.SnapToGrid() );
                aDrawInf.SetPosMatchesBounds( pCMS && pCMS->m_bPosMatchesBounds );

                if ( SwFontScript::CJK == aSizeInf.GetFont()->GetActual() &&
                     pPara->GetScriptInfo().CountCompChg() &&
                    ! pPor->InFieldGrp() )
                    aDrawInf.SetKanaComp( nKanaComp );

                nLength = aSizeInf.GetFont()->GetCursorOfst_( aDrawInf );

                // get position inside field portion?
                if ( pPor->InFieldGrp() && pCMS && pCMS->m_pSpecialPos )
                {
                    pCMS->m_pSpecialPos->nCharOfst = sal_Int32(nLength);
                    nLength = TextFrameIndex(0);
                }

                // set cursor bidi level
                if ( pCMS )
                    pCMS->m_nCursorBidiLevel =
                        aDrawInf.GetCursorBidiLevel();

                if( bFieldInfo && nLength == pPor->GetLen() &&
                    ( ! pPor->GetNextPortion() ||
                      ! pPor->GetNextPortion()->IsPostItsPortion() ) )
                    --nLength;
            }
            if( nOldProp )
                const_cast<SwFont*>(GetFnt())->SetProportion( nOldProp );
        }
        else
        {
            sw::FlyContentPortion* pFlyPor(nullptr);
            if(bChgNode && pPos && (pFlyPor = dynamic_cast<sw::FlyContentPortion*>(pPor)))
            {
                // JP 24.11.94: if the Position is not in Fly, then
                //              we many not return with COMPLETE_STRING as value!
                //              (BugId: 9692 + Change in feshview)
                SwFlyInContentFrame *pTmp = pFlyPor->GetFlyFrame();
                SwFrame* pLower = pTmp->GetLower();
                bool bChgNodeInner = pLower
                    && (pLower->IsTextFrame() || pLower->IsLayoutFrame());
                Point aTmpPoint( rPoint );

                if ( m_pFrame->IsRightToLeft() )
                    m_pFrame->SwitchLTRtoRTL( aTmpPoint );

                if ( m_pFrame->IsVertical() )
                    m_pFrame->SwitchHorizontalToVertical( aTmpPoint );

                if( bChgNodeInner && pTmp->getFrameArea().IsInside( aTmpPoint ) &&
                    !( pTmp->IsProtected() ) )
                {
                    pFlyPor->GetFlyCursorOfst(aTmpPoint, *pPos, pCMS);
                    // After a change of the frame, our font must be still
                    // available for/in the OutputDevice.
                    // For comparison: Paint and new SwFlyCntPortion !
                    static_cast<SwTextSizeInfo*>(m_pInf)->SelectFont();

                    // 6776: The pIter->GetCursorOfst is returning here
                    // from a nesting with COMPLETE_STRING.
                    return TextFrameIndex(COMPLETE_STRING);
                }
            }
            else
                nLength = pPor->GetCursorOfst( nX );
        }
    }
    nOffset = nCurrStart + nLength;

    // 7684: We end up in front of the HyphPortion. We must assure
    // that we end up in the string.
    // If we are at end of line in front of FlyFrames, we must proceed the same way.
    if( nOffset && pPor->GetLen() == nLength && pPor->GetNextPortion() &&
        !pPor->GetNextPortion()->GetLen() && pPor->GetNextPortion()->InHyphGrp() )
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
bool SwTextFrame::FillSelection( SwSelectionList& rSelList, const SwRect& rRect ) const
{
    bool bRet = false;
    // GetPaintArea() instead getFrameArea() for negative indents
    SwRect aTmpFrame( GetPaintArea() );
    if( !rRect.IsOver( aTmpFrame ) )
        return false;
    if( rSelList.checkContext( this ) )
    {
        SwRect aRect( aTmpFrame );
        aRect.Intersection( rRect );
        SwPosition aPosL( MapViewToModelPos(TextFrameIndex(0)) );
        if( IsEmpty() )
        {
            SwPaM *pPam = new SwPaM( aPosL, aPosL );
            rSelList.insertPaM( pPam );
        }
        else if( aRect.HasArea() )
        {
            SwPosition aOld(aPosL.nNode.GetNodes().GetEndOfContent());
            SwPosition aPosR( aPosL );
            Point aPoint;
            SwTextInfo aInf( const_cast<SwTextFrame*>(this) );
            SwTextIter aLine( const_cast<SwTextFrame*>(this), &aInf );
            // We have to care for top-to-bottom layout, where right becomes top etc.
            SwRectFnSet aRectFnSet(this);
            SwTwips nTop = aRectFnSet.GetTop(aRect);
            SwTwips nBottom = aRectFnSet.GetBottom(aRect);
            SwTwips nLeft = aRectFnSet.GetLeft(aRect);
            SwTwips nRight = aRectFnSet.GetRight(aRect);
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
                    if( aRectFnSet.IsVert() )
                    {
                        aPoint.setX( nLastY );
                        aPoint.setY( nLeft );
                    }
                    else
                    {
                        aPoint.setX( nLeft );
                        aPoint.setY( nLastY );
                    }
                    // Looking for the position of the left border of the rectangle
                    // in this text line
                    SwCursorMoveState aState( MV_UPDOWN );
                    if( GetCursorOfst( &aPosL, aPoint, &aState ) )
                    {
                        if( aRectFnSet.IsVert() )
                        {
                            aPoint.setX( nLastY );
                            aPoint.setY( nRight );
                        }
                        else
                        {
                            aPoint.setX( nRight );
                            aPoint.setY( nLastY );
                        }
                        // If we get a right position and if the left position
                        // is not the same like the left position of the line before
                        // which could happen e.g. for field portions or fly frames
                        // a SwPaM will be inserted with these positions
                        if( GetCursorOfst( &aPosR, aPoint, &aState ) &&
                            aOld != aPosL)
                        {
                            SwPaM *pPam = new SwPaM( aPosL, aPosR );
                            rSelList.insertPaM( pPam );
                            aOld = aPosL;
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
        for (SwAnchoredObject* pAnchoredObj : rObjs)
        {
            if( dynamic_cast< const SwFlyFrame *>( pAnchoredObj ) ==  nullptr )
                continue;
            const SwFlyFrame* pFly = static_cast<const SwFlyFrame*>(pAnchoredObj);
            if( pFly->IsFlyInContentFrame() && pFly->FillSelection( rSelList, rRect ) )
                bRet = true;
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
