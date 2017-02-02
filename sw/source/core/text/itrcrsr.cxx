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
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <pagefrm.hxx>

#include "itrtxt.hxx"
#include "txtfrm.hxx"
#include "flyfrms.hxx"
#include "porglue.hxx"
#include "porfld.hxx"
#include "porfly.hxx"
#include "pordrop.hxx"
#include "crstate.hxx"
#include <pormulti.hxx>
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

            if ( ! pPor->GetPortion() || nFieldIdx + nFieldLen > nCharOfst )
                break;

            nFieldIdx = nFieldIdx + nFieldLen;
            rOrig.Pos().X() += pPor->Width();
            pPor = pPor->GetPortion();

        } while ( true );

        OSL_ENSURE( nCharOfst >= nFieldIdx, "Request of position inside field failed" );
        sal_Int32 nLen = nCharOfst - nFieldIdx + 1;

        if ( pString )
        {
            // get script for field portion
            rInf.GetFont()->SetActual( SwScriptInfo::WhichFont( 0, pString, nullptr ) );

            sal_Int32 nOldLen = pPor->GetLen();
            const_cast<SwLinePortion*>(pPor)->SetLen( nLen - 1 );
            const SwTwips nX1 = pPor->GetLen() ?
                                pPor->GetTextSize( rInf ).Width() :
                                0;

            SwTwips nX2 = 0;
            if ( rCMS.m_bRealWidth )
            {
                const_cast<SwLinePortion*>(pPor)->SetLen( nLen );
                nX2 = pPor->GetTextSize( rInf ).Width();
            }

            const_cast<SwLinePortion*>(pPor)->SetLen( nOldLen );

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
    const SwTextNode *pNode = m_pFrame->GetTextNode();

    const SvxLRSpaceItem &rSpace = m_pFrame->GetTextNode()->GetSwAttrSet().GetLRSpace();
    // #i95907#
    // #i111284#
    const bool bListLevelIndentsApplicableAndLabelAlignmentActive(
        AreListLevelIndentsApplicableAndLabelAlignmentActive( *(m_pFrame->GetTextNode()) ) );

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
        nLeft = m_pFrame->Frame().Left() +
                m_pFrame->Prt().Left() +
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
            nLeft = m_pFrame->Frame().Left() +
                    m_pFrame->Prt().Left() +
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
            nLeft = m_pFrame->Frame().Left() +
                    std::max( long( rSpace.GetTextLeft() + nLMWithNum ),
                         m_pFrame->Prt().Left() );
        }
    }

    nRight = m_pFrame->Frame().Left() + m_pFrame->Prt().Left() + m_pFrame->Prt().Width();

    if( nLeft >= nRight &&
         // #i53066# Omit adjustment of nLeft for numbered
         // paras inside cells inside new documents:
        ( pNode->getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING) ||
          !m_pFrame->IsInTab() ||
          !nLMWithNum ) )
    {
        nLeft = m_pFrame->Prt().Left() + m_pFrame->Frame().Left();
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
            LanguageType aLang = pNode->GetLang( 0, 1, css::i18n::ScriptType::ASIAN);
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
              nFirst = m_pFrame->Frame().Left() +
                     std::max( rSpace.GetTextLeft() + nLMWithNum+ nFirstLineOfs,
                          m_pFrame->Prt().Left() );
        }

        // Note: <SwTextFrame::GetAdditionalFirstLineOffset()> returns a negative
        //       value for the new list label position and space mode LABEL_ALIGNMENT
        //       and label alignment CENTER and RIGHT in L2R layout respectively
        //       label alignment LEFT and CENTER in R2L layout
        nFirst += m_pFrame->GetAdditionalFirstLineOffset();

        if( nFirst >= nRight )
            nFirst = nRight - 1;
    }
    const SvxAdjustItem& rAdjust = m_pFrame->GetTextNode()->GetSwAttrSet().GetAdjust();
    nAdjust = static_cast<sal_uInt16>(rAdjust.GetAdjust());

    // left is left and right is right
    if ( m_pFrame->IsRightToLeft() )
    {
        if ( SVX_ADJUST_LEFT == nAdjust )
            nAdjust = SVX_ADJUST_RIGHT;
        else if ( SVX_ADJUST_RIGHT == nAdjust )
            nAdjust = SVX_ADJUST_LEFT;
    }

    m_bOneBlock = rAdjust.GetOneWord() == SVX_ADJUST_BLOCK;
    m_bLastBlock = rAdjust.GetLastBlock() == SVX_ADJUST_BLOCK;
    m_bLastCenter = rAdjust.GetLastBlock() == SVX_ADJUST_CENTER;

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
    if( GetAdjust() != SVX_ADJUST_LEFT &&
        !m_pCurr->GetFirstPortion()->IsMarginPortion() )
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

void SwTextCursor::CtorInitTextCursor( SwTextFrame *pNewFrame, SwTextSizeInfo *pNewInf )
{
    CtorInitTextMargin( pNewFrame, pNewInf );
    // 6096: Attention, the iterators are derived!
    // GetInfo().SetOut( GetInfo().GetWin() );
}

// 1170: Ancient bug: Shift-End forgets the last character ...
bool SwTextCursor::GetEndCharRect( SwRect* pOrig, const sal_Int32 nOfst,
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
        const bool bRet = GetCharRect( pOrig, nOfst, pCMS, nMax );
        bRightMargin = nOfst >= GetEnd() && nOfst < GetInfo().GetText().getLength();
        return bRet;
    }

    if( !GetPrev() || !GetPrev()->GetLen() || !PrevLine() )
        return GetCharRect( pOrig, nOfst, pCMS, nMax );

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
        pPor = pPor->GetPortion();
    }

    const Size aCharSize( 1, nTmpHeight );
    pOrig->Pos( GetTopLeft() );
    pOrig->SSize( aCharSize );
    pOrig->Pos().X() += nLast;
    const SwTwips nTmpRight = Right() - 1;
    if( pOrig->Left() > nTmpRight )
        pOrig->Pos().X() = nTmpRight;

    if ( pCMS && pCMS->m_bRealHeight )
    {
        if ( nTmpAscent > nPorAscent )
            pCMS->m_aRealHeight.X() = nTmpAscent - nPorAscent;
        else
            pCMS->m_aRealHeight.X() = 0;
        OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
        pCMS->m_aRealHeight.Y() = nPorHeight;
    }

    return true;
}

// internal function, called by SwTextCursor::GetCharRect() to calculate
// the relative character position in the current line.
// pOrig referes to x and y coordinates, width and height of the cursor
// pCMS is used for restricting the cursor, if there are different font
// heights in one line ( first value = offset to y of pOrig, second
// value = real height of (shortened) cursor
void SwTextCursor::GetCharRect_( SwRect* pOrig, const sal_Int32 nOfst,
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
            pCMS->m_aRealHeight.X() = 0;
            pCMS->m_aRealHeight.Y() = nTmpHeight;
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
        SwTwips nLastBidiPorWidth = 0;
        std::deque<sal_uInt16>* pKanaComp = m_pCurr->GetpKanaComp();
        sal_uInt16 nSpaceIdx = 0;
        size_t nKanaIdx = 0;
        long nSpaceAdd = m_pCurr->IsSpaceAdd() ? m_pCurr->GetLLSpaceAdd( 0 ) : 0;

        bool bNoText = true;

        // First all portions without Len at beginning of line are skipped.
        // Exceptions are the mean special portions from WhichFirstPortion:
        // Num, ErgoSum, FootnoteNum, FeldReste
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
                const sal_Int32 nExtra = pPor->IsMultiPortion() &&
                                    ! static_cast<SwMultiPortion*>(pPor)->IsBidi() &&
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
                        if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                                !pPor->GetPortion()->IsMarginPortion() ) )
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
                             nLastBidiPorWidth = pLastBidiPor->Width() +
                                                 pLastBidiPor->CalcSpacing( nSpaceAdd, aInf );
                        }
                    }

                    aInf.SetIdx( aInf.GetIdx() + pPor->GetLen() );
                    pPor = pPor->GetPortion();
                }
                else
                {
                    if( pPor->IsMultiPortion() )
                    {
                        nTmpAscent = AdjustBaseLine( *m_pCurr, pPor );
                        GetInfo().SetMulti( true );
                        pOrig->Pos().Y() += nTmpAscent - nPorAscent;

                        if( pCMS && pCMS->m_b2Lines )
                        {
                            const bool bRecursion = pCMS->m_p2Lines;
                            if ( !bRecursion )
                            {
                                pCMS->m_p2Lines = new Sw2LinesPos;
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
                        sal_Int32 nOldStart = m_nStart;
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

                            pOrig->Pos().Y() += nOffset;
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

                            pOrig->Pos().X() = nX + aOldPos.X();
                            if( static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                                pOrig->Pos().Y() = aOldPos.Y() + nTmp;
                            else
                                pOrig->Pos().Y() = aOldPos.Y()
                                    + pPor->Height() - nTmp - pOrig->Height();
                            if ( pCMS && pCMS->m_bRealHeight )
                            {
                                pCMS->m_aRealHeight.Y() = -pCMS->m_aRealHeight.Y();
                                // result for rotated multi portion is not
                                // correct for reverse (270 degree) portions
                                if( static_cast<SwMultiPortion*>(pPor)->IsRevers() )
                                {
                                    if ( SvxParaVertAlignItem::Align::Automatic ==
                                         GetLineInfo().GetVertAlign() )
                                        // if vertical alignment is set to auto,
                                        // we switch from base line alignment
                                        // to centered alignment
                                        pCMS->m_aRealHeight.X() =
                                            ( pOrig->Width() +
                                              pCMS->m_aRealHeight.Y() ) / 2;
                                    else
                                        pCMS->m_aRealHeight.X() =
                                            ( pOrig->Width() -
                                              pCMS->m_aRealHeight.X() +
                                              pCMS->m_aRealHeight.Y() );
                                }
                            }
                        }
                        else
                        {
                            pOrig->Pos().Y() += aOldPos.Y();
                            if ( static_cast<SwMultiPortion*>(pPor)->IsBidi() )
                            {
                                const SwTwips nPorWidth = pPor->Width() +
                                                         pPor->CalcSpacing( nSpaceAdd, aInf );
                                const SwTwips nInsideOfst = pOrig->Pos().X();
                                pOrig->Pos().X() = nX + nPorWidth -
                                                   nInsideOfst - pOrig->Width();
                            }
                            else
                                pOrig->Pos().X() += nX;

                            if( static_cast<SwMultiPortion*>(pPor)->HasBrackets() )
                                pOrig->Pos().X() +=
                                    static_cast<SwDoubleLinePortion*>(pPor)->PreWidth();
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
                        sal_Int32 nOldLen = pPor->GetLen();
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
                            nX += pPor->GetTextSize( aInf ).Width();
                            aInf.SetOnWin( bOldOnWin );
                            if ( pPor->InSpaceGrp() && nSpaceAdd )
                                nX += pPor->CalcSpacing( nSpaceAdd, aInf );
                            if( bWidth )
                            {
                                pPor->SetLen( pPor->GetLen() + 1 );
                                aInf.SetLen( pPor->GetLen() );
                                aInf.SetOnWin( false ); // keine BULLETs!
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
                                sal_Int16 nSumLength = 0;
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
                    SwLinePortion *pNext = pTmp->GetPortion();
                    while( pNext && !pNext->InFieldGrp() )
                    {
                        OSL_ENSURE( !pNext->GetLen(), "Where's my field follow?" );
                        nAddX = nAddX + pNext->Width();
                        pNext = pNext->GetPortion();
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
                    if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
                            !pPor->GetPortion()->IsMarginPortion() ) )
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
                    if( 2 > pPor->GetLen() )
                    {
                        nTmp = pPor->Width();
                        if ( pPor->InSpaceGrp() && nSpaceAdd )
                            nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
                    }
                    else
                    {
                        const bool bOldOnWin = aInf.OnWin();
                        sal_Int32 nOldLen = pPor->GetLen();
                        pPor->SetLen( 1 );
                        aInf.SetLen( pPor->GetLen() );
                        SeekAndChg( aInf );
                        aInf.SetOnWin( false ); // keine BULLETs!
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
                    pOrig->Pos().X() -= nLastBidiPorWidth;

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
                        pOrig->Pos().X() += pLast->Width() +
                                            pLast->CalcSpacing( nSpaceAdd, aInf );
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
                    pOrig->Pos().X() += pPor->Width() +
                                        pPor->CalcSpacing( nSpaceAdd, aInf );
                }
            }
        }

        pOrig->Pos().X() += nX;

        if ( pCMS && pCMS->m_bRealHeight )
        {
            nTmpAscent = AdjustBaseLine( *m_pCurr, nullptr, nPorHeight, nPorAscent );
            if ( nTmpAscent > nPorAscent )
                pCMS->m_aRealHeight.X() = nTmpAscent - nPorAscent;
            else
                pCMS->m_aRealHeight.X() = 0;
            OSL_ENSURE( nPorHeight, "GetCharRect: Missing Portion-Height" );
            if ( nTmpHeight > nPorHeight )
                pCMS->m_aRealHeight.Y() = nPorHeight;
            else
                pCMS->m_aRealHeight.Y() = nTmpHeight;
        }
    }
}

bool SwTextCursor::GetCharRect( SwRect* pOrig, const sal_Int32 nOfst,
                               SwCursorMoveState* pCMS, const long nMax )
{
    CharCursorToLine(nOfst);

    // Indicates that a position inside a special portion (field, number portion)
    // is requested.
    const bool bSpecialPos = pCMS && pCMS->m_pSpecialPos;
    sal_Int32 nFindOfst = nOfst;

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
    bool bRet = true;

    GetCharRect_( pOrig, nFindOfst, pCMS );

    // This actually would have to be "-1 LogicToPixel", but that seems too
    // expensive, so it's a value (-12), that should hopefully be OK.
    const SwTwips nTmpRight = Right() - 12;

    pOrig->Pos().X() += aCharPos.X();
    pOrig->Pos().Y() += aCharPos.Y();

    if( pCMS && pCMS->m_b2Lines && pCMS->m_p2Lines )
    {
        pCMS->m_p2Lines->aLine.Pos().X() += aCharPos.X();
        pCMS->m_p2Lines->aLine.Pos().Y() += aCharPos.Y();
        pCMS->m_p2Lines->aPortion.Pos().X() += aCharPos.X();
        pCMS->m_p2Lines->aPortion.Pos().Y() += aCharPos.Y();
    }

    const bool bTabOverMargin = GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_OVER_MARGIN);
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
        if ( pCMS && pCMS->m_bRealHeight && pCMS->m_aRealHeight.Y() >= 0 )
        {
            long nTmp = pCMS->m_aRealHeight.X() + pOrig->Top();
            if( nTmp >= nMax )
            {
                pCMS->m_aRealHeight.X() = nMax - pOrig->Top();
                pCMS->m_aRealHeight.Y() = 0;
            }
            else if( nTmp + pCMS->m_aRealHeight.Y() > nMax )
                pCMS->m_aRealHeight.Y() = nMax - nTmp;
        }
    }
    long nOut = pOrig->Right() - GetTextFrame()->Frame().Right();
    if( nOut > 0 )
    {
        if( GetTextFrame()->Frame().Width() < GetTextFrame()->Prt().Left()
                                   + GetTextFrame()->Prt().Width() )
            nOut += GetTextFrame()->Frame().Width() - GetTextFrame()->Prt().Left()
                    - GetTextFrame()->Prt().Width();
        if( nOut > 0 )
            pOrig->Pos().X() -= nOut + 10;
    }

    return bRet;
}

// Return: Offset in String
sal_Int32 SwTextCursor::GetCursorOfst( SwPosition *pPos, const Point &rPoint,
                                    bool bChgNode, SwCursorMoveState* pCMS ) const
{
    // If necessary, as catch up, do the adjustment
    GetAdjusted();

    const OUString &rText = GetInfo().GetText();
    sal_Int32 nOffset = 0;

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
    sal_Int32 nCurrStart  = m_nStart;
    bool bHolePortion = false;
    bool bLastHyph = false;

    std::deque<sal_uInt16> *pKanaComp = m_pCurr->GetpKanaComp();
    sal_Int32 nOldIdx = GetInfo().GetIdx();
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

    while( pPor->GetPortion() && nWidth30 < nX && !pPor->IsBreakPortion() )
    {
        nX = nX - nWidth;
        nCurrStart = nCurrStart + pPor->GetLen();
        bHolePortion = pPor->IsHolePortion();
        pPor = pPor->GetPortion();
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

    const bool bLastPortion = (nullptr == pPor->GetPortion());

    if( nX==nWidth )
    {
        SwLinePortion *pNextPor = pPor->GetPortion();
        while( pNextPor && pNextPor->InFieldGrp() && !pNextPor->Width() )
        {
            nCurrStart = nCurrStart + pPor->GetLen();
            pPor = pNextPor;
            if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
                bLastHyph = pPor->InHyphGrp();
            pNextPor = pPor->GetPortion();
        }
    }

    const_cast<SwTextSizeInfo&>(GetInfo()).SetIdx( nOldIdx );

    sal_Int32 nLength = pPor->GetLen();

    const bool bFieldInfo = pCMS && pCMS->m_bFieldInfo;

    if( bFieldInfo && ( nWidth30 < nX || bRightOver || bLeftOver ||
        ( pPor->InNumberGrp() && !pPor->IsFootnoteNumPortion() ) ||
        ( pPor->IsMarginPortion() && nWidth > nX + 30 ) ) )
        pCMS->m_bPosCorr = true;

    // #i27615#
    if (pCMS)
    {
        if( pCMS->m_bInFrontOfLabel)
        {
            if (! (2 * nX < nWidth && pPor->InNumberGrp() &&
                   !pPor->IsFootnoteNumPortion()))
                pCMS->m_bInFrontOfLabel = false;
        }
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
            return 0;

        // 7849, 7816: pPor->GetHyphPortion is mandatory!
        if( bHolePortion || ( !bRightAllowed && bLastHyph ) ||
            ( pPor->IsMarginPortion() && !pPor->GetPortion() &&
              // 46598: Consider the situation: We might end up behind the last character,
              // in the last line of a centered paragraph
              nCurrStart < rText.getLength() ) )
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
    if ( 1 == nLength )
    {
        if ( nWidth )
        {
            // Else we may not enter the character-supplying frame...
            if( !( bChgNode && pPos && pPor->IsFlyCntPortion() ) )
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
                if ( !pCMS || !pCMS->m_pSpecialPos )
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
        && rText[nCurrStart + nLength - 1] == ' ' )
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
                nX = (sal_uInt16)nTmpY;
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
                    while( pCurrPart && nSumWidth <= nX - nCurrStart )
                    {
                        nSumWidth += pCurrPart->GetWidth();
                        if( pCurrPart->GetFont().GetLeftBorder() && !pCurrPart->GetJoinBorderWithPrev() )
                        {
                            nSumBorderWidth += pCurrPart->GetFont().GetLeftBorderSpace();
                        }
                        if( nSumWidth <= nX - nCurrStart && pCurrPart->GetFont().GetRightBorder() &&
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
                    sal_Int32 nCharCnt = 0;
                    // #i41860# Thai justified alignment needs some
                    // additional information:
                    aDrawInf.SetNumberOfBlanks( pPor->InTextGrp() ?
                                                static_cast<const SwTextPortion*>(pPor)->GetSpaceCnt( aSizeInf, nCharCnt ) :
                                                0 );
                }

                if ( pPor->InFieldGrp() && pCMS && pCMS->m_pSpecialPos )
                    aDrawInf.SetLen( COMPLETE_STRING );

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
                    pCMS->m_pSpecialPos->nCharOfst = nLength;
                    nLength = 0;
                }

                // set cursor bidi level
                if ( pCMS )
                    pCMS->m_nCursorBidiLevel =
                        aDrawInf.GetCursorBidiLevel();

                if( bFieldInfo && nLength == pPor->GetLen() &&
                    ( ! pPor->GetPortion() ||
                      ! pPor->GetPortion()->IsPostItsPortion() ) )
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

                if( bChgNodeInner && pTmp->Frame().IsInside( aTmpPoint ) &&
                    !( pTmp->IsProtected() ) )
                {
                    pFlyPor->GetFlyCursorOfst(aTmpPoint, *pPos, pCMS);
                    // After a change of the frame, our font must be still
                    // available for/in the OutputDevice.
                    // For comparison: Paint and new SwFlyCntPortion !
                    static_cast<SwTextSizeInfo*>(m_pInf)->SelectFont();

                    // 6776: The pIter->GetCursorOfst is returning here
                    // from a nesting with COMPLETE_STRING.
                    return COMPLETE_STRING;
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
bool SwTextFrame::FillSelection( SwSelectionList& rSelList, const SwRect& rRect ) const
{
    bool bRet = false;
    // PaintArea() instead Frame() for negative indents
    SwRect aTmpFrame( PaintArea() );
    if( !rRect.IsOver( aTmpFrame ) )
        return false;
    if( rSelList.checkContext( this ) )
    {
        SwRect aRect( aTmpFrame );
        aRect.Intersection( rRect );
        // rNode without const to create SwPaMs
        SwContentNode &rNode = const_cast<SwContentNode&>( *GetNode() );
        SwNodeIndex aIdx( rNode );
        SwPosition aPosL( aIdx, SwIndex( &rNode, 0 ) );
        if( IsEmpty() )
        {
            SwPaM *pPam = new SwPaM( aPosL, aPosL );
            rSelList.insertPaM( pPam );
        }
        else if( aRect.HasArea() )
        {
            sal_Int32 nOld = -1;
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
                    SwCursorMoveState aState( MV_UPDOWN );
                    if( GetCursorOfst( &aPosL, aPoint, &aState ) )
                    {
                        if( aRectFnSet.IsVert() )
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
                        if( GetCursorOfst( &aPosR, aPoint, &aState ) &&
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
