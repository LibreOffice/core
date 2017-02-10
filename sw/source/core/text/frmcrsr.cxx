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

#include "ndtxt.hxx"
#include "pam.hxx"
#include "frmtool.hxx"
#include "viewopt.hxx"
#include "paratr.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "swtypes.hxx"
#include <sfx2/printer.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <pormulti.hxx>
#include <doc.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <sortedobjs.hxx>

#include <unicode/ubidi.h>

#include "txtfrm.hxx"
#include "inftxt.hxx"
#include "itrtxt.hxx"
#include "crstate.hxx"
#include "viewsh.hxx"
#include "swfntcch.hxx"
#include "flyfrm.hxx"

#define MIN_OFFSET_STEP 10

using namespace ::com::sun::star;

/*
 * - SurvivalKit: For how long do we get past the last char of the line.
 * - RightMargin abstains from adjusting position with -1
 * - GetCharRect returns a GetEndCharRect for MV_RIGHTMARGIN
 * - GetEndCharRect sets bRightMargin to true
 * - SwTextCursor::bRightMargin is set to false by CharCursorToLine
 */

namespace
{

SwTextFrame *GetAdjFrameAtPos( SwTextFrame *pFrame, const SwPosition &rPos,
                          const bool bRightMargin, const bool bNoScroll = true )
{
    // RightMargin in the last master line
    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTextFrame *pFrameAtPos = pFrame;
    if( !bNoScroll || pFrame->GetFollow() )
    {
        pFrameAtPos = pFrame->GetFrameAtPos( rPos );
        if( nOffset < pFrameAtPos->GetOfst() &&
            !pFrameAtPos->IsFollow() )
        {
            sal_Int32 nNew = nOffset;
            if( nNew < MIN_OFFSET_STEP )
                nNew = 0;
            else
                nNew -= MIN_OFFSET_STEP;
            sw_ChangeOffset( pFrameAtPos, nNew );
        }
    }
    while( pFrame != pFrameAtPos )
    {
        pFrame = pFrameAtPos;
        pFrame->GetFormatted();
        pFrameAtPos = pFrame->GetFrameAtPos( rPos );
    }

    if( nOffset && bRightMargin )
    {
        while( pFrameAtPos && pFrameAtPos->GetOfst() == nOffset &&
               pFrameAtPos->IsFollow() )
        {
            pFrameAtPos->GetFormatted();
            pFrameAtPos = pFrameAtPos->FindMaster();
        }
        OSL_ENSURE( pFrameAtPos, "+GetCharRect: no frame with my rightmargin" );
    }
    return pFrameAtPos ? pFrameAtPos : pFrame;
}

}

bool sw_ChangeOffset( SwTextFrame* pFrame, sal_Int32 nNew )
{
    // Do not scroll in areas and outside of flies
    OSL_ENSURE( !pFrame->IsFollow(), "Illegal Scrolling by Follow!" );
    if( pFrame->GetOfst() != nNew && !pFrame->IsInSct() )
    {
        SwFlyFrame *pFly = pFrame->FindFlyFrame();
        // Attention: if e.g. in a column frame the size is still invalid
        // we must not scroll around just like that
        if ( ( pFly && pFly->IsValid() &&
             !pFly->GetNextLink() && !pFly->GetPrevLink() ) ||
             ( !pFly && pFrame->IsInTab() ) )
        {
            SwViewShell* pVsh = pFrame->getRootFrame()->GetCurrShell();
            if( pVsh )
            {
                if( pVsh->GetRingContainer().size() > 1 ||
                    ( pFrame->GetDrawObjs() && pFrame->GetDrawObjs()->size() ) )
                {
                    if( !pFrame->GetOfst() )
                        return false;
                    nNew = 0;
                }
                pFrame->SetOfst( nNew );
                pFrame->SetPara( nullptr );
                pFrame->GetFormatted();
                if( pFrame->Frame().HasArea() )
                    pFrame->getRootFrame()->GetCurrShell()->InvalidateWindows( pFrame->Frame() );
                return true;
            }
        }
    }
    return false;
}

SwTextFrame& SwTextFrame::GetFrameAtOfst( const sal_Int32 nWhere )
{
    SwTextFrame* pRet = this;
    while( pRet->HasFollow() && nWhere >= pRet->GetFollow()->GetOfst() )
        pRet = pRet->GetFollow();
    return *pRet;
}

SwTextFrame *SwTextFrame::GetFrameAtPos( const SwPosition &rPos )
{
    SwTextFrame *pFoll = this;
    while( pFoll->GetFollow() )
    {
        if( rPos.nContent.GetIndex() > pFoll->GetFollow()->GetOfst() )
            pFoll = pFoll->GetFollow();
        else
        {
            if( rPos.nContent.GetIndex() == pFoll->GetFollow()->GetOfst()
                 && !SwTextCursor::IsRightMargin() )
                 pFoll = pFoll->GetFollow();
            else
                break;
        }
    }
    return pFoll;
}

/*
 * GetCharRect() returns the char's char line described by aPos.
 * GetCursorOfst() does the reverse: It goes from a document coordinate to
 * a Pam.
 * Both are virtual in the frame base class and thus are redefined here.
 */

bool SwTextFrame::GetCharRect( SwRect& rOrig, const SwPosition &rPos,
                            SwCursorMoveState *pCMS ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTextFrame::GetCharRect with swapped frame" );

    if( IsLocked() || IsHiddenNow() )
        return false;

    // Find the right frame first. We need to keep in mind that:
    // - the cached information could be invalid  (GetPara() == 0)
    // - we could have a Follow
    // - the Follow chain grows dynamically; the one we end up in
    //   needs to be formatted

    // Optimisation: reading ahead saves us a GetAdjFrameAtPos
    const bool bRightMargin = pCMS && ( MV_RIGHTMARGIN == pCMS->m_eState );
    const bool bNoScroll = pCMS && pCMS->m_bNoScroll;
    SwTextFrame *pFrame = GetAdjFrameAtPos( const_cast<SwTextFrame*>(this), rPos, bRightMargin,
                                     bNoScroll );
    pFrame->GetFormatted();
    const SwFrame* pTmpFrame = static_cast<SwFrame*>(pFrame->GetUpper());

    SwRectFnSet aRectFnSet(pFrame);
    const SwTwips nUpperMaxY = aRectFnSet.GetPrtBottom(*pTmpFrame);
    const SwTwips nFrameMaxY = aRectFnSet.GetPrtBottom(*pFrame);

    // nMaxY is an absolute value
    SwTwips nMaxY = aRectFnSet.IsVert() ?
                    ( aRectFnSet.IsVertL2R() ? std::min( nFrameMaxY, nUpperMaxY ) : std::max( nFrameMaxY, nUpperMaxY ) ) :
                    std::min( nFrameMaxY, nUpperMaxY );

    bool bRet = false;

    if ( pFrame->IsEmpty() || ! aRectFnSet.GetHeight(pFrame->Prt()) )
    {
        Point aPnt1 = pFrame->Frame().Pos() + pFrame->Prt().Pos();
        SwTextNode* pTextNd = const_cast<SwTextFrame*>(this)->GetTextNode();
        short nFirstOffset;
        pTextNd->GetFirstLineOfsWithNum( nFirstOffset );

        Point aPnt2;
        if ( aRectFnSet.IsVert() )
        {
            if( nFirstOffset > 0 )
                aPnt1.Y() += nFirstOffset;
            if ( aPnt1.X() < nMaxY && !aRectFnSet.IsVertL2R() )
                aPnt1.X() = nMaxY;
            aPnt2.X() = aPnt1.X() + pFrame->Prt().Width();
            aPnt2.Y() = aPnt1.Y();
            if( aPnt2.X() < nMaxY )
                aPnt2.X() = nMaxY;
        }
        else
        {
            if( nFirstOffset > 0 )
                aPnt1.X() += nFirstOffset;

            if( aPnt1.Y() > nMaxY )
                aPnt1.Y() = nMaxY;
            aPnt2.X() = aPnt1.X();
            aPnt2.Y() = aPnt1.Y() + pFrame->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }

        rOrig = SwRect( aPnt1, aPnt2 );

        if ( pCMS )
        {
            pCMS->m_aRealHeight.X() = 0;
            pCMS->m_aRealHeight.Y() = aRectFnSet.IsVert() ? -rOrig.Width() : rOrig.Height();
        }

        if ( pFrame->IsRightToLeft() )
            pFrame->SwitchLTRtoRTL( rOrig );

        bRet = true;
    }
    else
    {
        if( !pFrame->HasPara() )
            return false;

        SwFrameSwapper aSwapper( pFrame, true );
        if ( aRectFnSet.IsVert() )
            nMaxY = pFrame->SwitchVerticalToHorizontal( nMaxY );

        bool bGoOn = true;
        const sal_Int32 nOffset = rPos.nContent.GetIndex();
        sal_Int32 nNextOfst;

        do
        {
            {
                SwTextSizeInfo aInf( pFrame );
                SwTextCursor  aLine( pFrame, &aInf );
                nNextOfst = aLine.GetEnd();
                // See comment in AdjustFrame
                // Include the line's last char?
                bRet = bRightMargin ? aLine.GetEndCharRect( &rOrig, nOffset, pCMS, nMaxY )
                                : aLine.GetCharRect( &rOrig, nOffset, pCMS, nMaxY );
            }

            if ( pFrame->IsRightToLeft() )
                pFrame->SwitchLTRtoRTL( rOrig );

            if ( aRectFnSet.IsVert() )
                pFrame->SwitchHorizontalToVertical( rOrig );

            if( pFrame->IsUndersized() && pCMS && !pFrame->GetNext() &&
                aRectFnSet.GetBottom(rOrig) == nUpperMaxY &&
                pFrame->GetOfst() < nOffset &&
                !pFrame->IsFollow() && !bNoScroll &&
                pFrame->GetTextNode()->GetText().getLength() != nNextOfst)
            {
                bGoOn = sw_ChangeOffset( pFrame, nNextOfst );
            }
            else
                bGoOn = false;
        } while ( bGoOn );

        if ( pCMS )
        {
            if ( pFrame->IsRightToLeft() )
            {
                if( pCMS->m_b2Lines && pCMS->m_p2Lines)
                {
                    pFrame->SwitchLTRtoRTL( pCMS->m_p2Lines->aLine );
                    pFrame->SwitchLTRtoRTL( pCMS->m_p2Lines->aPortion );
                }
            }

            if ( aRectFnSet.IsVert() )
            {
                if ( pCMS->m_bRealHeight )
                {
                    pCMS->m_aRealHeight.Y() = -pCMS->m_aRealHeight.Y();
                    if ( pCMS->m_aRealHeight.Y() < 0 )
                    {
                        // writing direction is from top to bottom
                        pCMS->m_aRealHeight.X() =  ( rOrig.Width() -
                                                    pCMS->m_aRealHeight.X() +
                                                    pCMS->m_aRealHeight.Y() );
                    }
                }
                if( pCMS->m_b2Lines && pCMS->m_p2Lines)
                {
                    pFrame->SwitchHorizontalToVertical( pCMS->m_p2Lines->aLine );
                    pFrame->SwitchHorizontalToVertical( pCMS->m_p2Lines->aPortion );
                }
            }

        }
    }
    if( bRet )
    {
        SwPageFrame *pPage = pFrame->FindPageFrame();
        OSL_ENSURE( pPage, "Text escaped from page?" );
        const SwTwips nOrigTop = aRectFnSet.GetTop(rOrig);
        const SwTwips nPageTop = aRectFnSet.GetTop(pPage->Frame());
        const SwTwips nPageBott = aRectFnSet.GetBottom(pPage->Frame());

        // We have the following situation: if the frame is in an invalid
        // sectionframe, it's possible that the frame is outside the page.
        // If we restrict the cursor position to the page area, we enforce
        // the formatting of the page, of the section frame and the frame itself.
        if( aRectFnSet.YDiff( nPageTop, nOrigTop ) > 0 )
            aRectFnSet.SetTop( rOrig, nPageTop );

        if ( aRectFnSet.YDiff( nOrigTop, nPageBott ) > 0 )
            aRectFnSet.SetTop( rOrig, nPageBott );
    }

    return bRet;
}

/*
 * GetAutoPos() looks up the char's char line which is described by rPos
 * and is used by the auto-positioned frame.
 */

bool SwTextFrame::GetAutoPos( SwRect& rOrig, const SwPosition &rPos ) const
{
    if( IsHiddenNow() )
        return false;

    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTextFrame* pFrame = &(const_cast<SwTextFrame*>(this)->GetFrameAtOfst( nOffset ));

    pFrame->GetFormatted();
    const SwFrame* pTmpFrame = static_cast<SwFrame*>(pFrame->GetUpper());

    SwRectFnSet aRectFnSet(pTmpFrame);
    SwTwips nUpperMaxY = aRectFnSet.GetPrtBottom(*pTmpFrame);

    // nMaxY is in absolute value
    SwTwips nMaxY;
    if ( aRectFnSet.IsVert() )
    {
        if ( aRectFnSet.IsVertL2R() )
            nMaxY = std::min( aRectFnSet.GetPrtBottom(*pFrame), nUpperMaxY );
        else
            nMaxY = std::max( aRectFnSet.GetPrtBottom(*pFrame), nUpperMaxY );
    }
    else
        nMaxY = std::min( aRectFnSet.GetPrtBottom(*pFrame), nUpperMaxY );
    if ( pFrame->IsEmpty() || ! aRectFnSet.GetHeight(pFrame->Prt()) )
    {
        Point aPnt1 = pFrame->Frame().Pos() + pFrame->Prt().Pos();
        Point aPnt2;
        if ( aRectFnSet.IsVert() )
        {
            if ( aPnt1.X() < nMaxY && !aRectFnSet.IsVertL2R() )
                aPnt1.X() = nMaxY;

            aPnt2.X() = aPnt1.X() + pFrame->Prt().Width();
            aPnt2.Y() = aPnt1.Y();
            if( aPnt2.X() < nMaxY )
                aPnt2.X() = nMaxY;
        }
        else
        {
            if( aPnt1.Y() > nMaxY )
                aPnt1.Y() = nMaxY;
            aPnt2.X() = aPnt1.X();
            aPnt2.Y() = aPnt1.Y() + pFrame->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }
        rOrig = SwRect( aPnt1, aPnt2 );
        return true;
    }
    else
    {
        if( !pFrame->HasPara() )
            return false;

        SwFrameSwapper aSwapper( pFrame, true );
        if ( aRectFnSet.IsVert() )
            nMaxY = pFrame->SwitchVerticalToHorizontal( nMaxY );

        SwTextSizeInfo aInf( pFrame );
        SwTextCursor aLine( pFrame, &aInf );
        SwCursorMoveState aTmpState( MV_SETONLYTEXT );
        aTmpState.m_bRealHeight = true;
        if( aLine.GetCharRect( &rOrig, nOffset, &aTmpState, nMaxY ) )
        {
            if( aTmpState.m_aRealHeight.X() >= 0 )
            {
                rOrig.Pos().Y() += aTmpState.m_aRealHeight.X();
                rOrig.Height( aTmpState.m_aRealHeight.Y() );
            }

            if ( pFrame->IsRightToLeft() )
                pFrame->SwitchLTRtoRTL( rOrig );

            if ( aRectFnSet.IsVert() )
                pFrame->SwitchHorizontalToVertical( rOrig );

            return true;
        }
        return false;
    }
}

/** determine top of line for given position in the text frame

    - Top of first paragraph line is the top of the printing area of the text frame
    - If a proportional line spacing is applied use top of anchor character as
      top of the line.
*/
bool SwTextFrame::GetTopOfLine( SwTwips& _onTopOfLine,
                             const SwPosition& _rPos ) const
{
    bool bRet = true;

    // get position offset
    const sal_Int32 nOffset = _rPos.nContent.GetIndex();

    if ( GetText().getLength() < nOffset )
    {
        bRet = false;
    }
    else
    {
        SwRectFnSet aRectFnSet(this);
        if ( IsEmpty() || !aRectFnSet.GetHeight(Prt()) )
        {
            // consider upper space amount considered
            // for previous frame and the page grid.
            _onTopOfLine = aRectFnSet.GetPrtTop(*this);
        }
        else
        {
            // determine formatted text frame that contains the requested position
            SwTextFrame* pFrame = &(const_cast<SwTextFrame*>(this)->GetFrameAtOfst( nOffset ));
            pFrame->GetFormatted();
            aRectFnSet.Refresh(pFrame);
            // If proportional line spacing is applied
            // to the text frame, the top of the anchor character is also the
            // top of the line.
            // Otherwise the line layout determines the top of the line
            const SvxLineSpacingItem& rSpace = GetAttrSet()->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SvxInterLineSpaceRule::Prop )
            {
                SwRect aCharRect;
                if ( GetAutoPos( aCharRect, _rPos ) )
                {
                    _onTopOfLine = aRectFnSet.GetTop(aCharRect);
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                // assure that text frame is in a horizontal layout
                SwFrameSwapper aSwapper( pFrame, true );
                // determine text line that contains the requested position
                SwTextSizeInfo aInf( pFrame );
                SwTextCursor aLine( pFrame, &aInf );
                aLine.CharCursorToLine( nOffset );
                // determine top of line
                _onTopOfLine = aLine.Y();
                if ( aRectFnSet.IsVert() )
                {
                    _onTopOfLine = pFrame->SwitchHorizontalToVertical( _onTopOfLine );
                }
            }
        }
    }

    return bRet;
}

// Minimum distance of non-empty lines is a little less than 2 cm
#define FILL_MIN_DIST 1100

struct SwFillData
{
    SwRect aFrame;
    const SwCursorMoveState *pCMS;
    SwPosition* pPos;
    const Point& rPoint;
    SwTwips nLineWidth;
    bool bFirstLine : 1;
    bool bInner     : 1;
    bool bColumn    : 1;
    bool bEmpty     : 1;
    SwFillData( const SwCursorMoveState *pC, SwPosition* pP, const SwRect& rR,
        const Point& rPt ) : aFrame( rR ), pCMS( pC ), pPos( pP ), rPoint( rPt ),
        nLineWidth( 0 ), bFirstLine( true ), bInner( false ), bColumn( false ),
        bEmpty( true ){}
    SwFillMode Mode() const { return pCMS->m_pFill->eMode; }
    long X() const { return rPoint.X(); }
    long Y() const { return rPoint.Y(); }
    long Left() const { return aFrame.Left(); }
    long Right() const { return aFrame.Right(); }
    long Bottom() const { return aFrame.Bottom(); }
    SwFillCursorPos &Fill() const { return *pCMS->m_pFill; }
    void SetTab( sal_uInt16 nNew ) { pCMS->m_pFill->nTabCnt = nNew; }
    void SetSpace( sal_uInt16 nNew ) { pCMS->m_pFill->nSpaceCnt = nNew; }
    void SetOrient( const sal_Int16 eNew ){ pCMS->m_pFill->eOrient = eNew; }
};

bool SwTextFrame::GetCursorOfst_(SwPosition* pPos, const Point& rPoint,
                    const bool bChgFrame, SwCursorMoveState* pCMS ) const
{
    // GetCursorOfst_ is called by GetCursorOfst and GetKeyCursorOfst.
    // Never just a return false.

    if( IsLocked() || IsHiddenNow() )
        return false;

    const_cast<SwTextFrame*>(this)->GetFormatted();

    Point aOldPoint( rPoint );

    if ( IsVertical() )
    {
        SwitchVerticalToHorizontal( (Point&)rPoint );
        const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() )
        SwitchRTLtoLTR( (Point&)rPoint );

    SwFillData *pFillData = ( pCMS && pCMS->m_pFill ) ?
                        new SwFillData( pCMS, pPos, Frame(), rPoint ) : nullptr;

    if ( IsEmpty() )
    {
        SwTextNode* pTextNd = const_cast<SwTextFrame*>(this)->GetTextNode();
        pPos->nNode = *pTextNd;
        pPos->nContent.Assign( pTextNd, 0 );
        if( pCMS && pCMS->m_bFieldInfo )
        {
            SwTwips nDiff = rPoint.X() - Frame().Left() - Prt().Left();
            if( nDiff > 50 || nDiff < 0 )
                pCMS->m_bPosCorr = true;
        }
    }
    else
    {
        SwTextSizeInfo aInf( const_cast<SwTextFrame*>(this) );
        SwTextCursor  aLine( const_cast<SwTextFrame*>(this), &aInf );

        // See comment in AdjustFrame()
        SwTwips nMaxY = Frame().Top() + Prt().Top() + Prt().Height();
        aLine.TwipsToLine( rPoint.Y() );
        while( aLine.Y() + aLine.GetLineHeight() > nMaxY )
        {
            if( !aLine.Prev() )
                break;
        }

        if( aLine.GetDropLines() >= aLine.GetLineNr() && 1 != aLine.GetLineNr()
            && rPoint.X() < aLine.FirstLeft() + aLine.GetDropLeft() )
            while( aLine.GetLineNr() > 1 )
                aLine.Prev();

        sal_Int32 nOffset = aLine.GetCursorOfst( pPos, rPoint, bChgFrame, pCMS );

        if( pCMS && pCMS->m_eState == MV_NONE && aLine.GetEnd() == nOffset )
            pCMS->m_eState = MV_RIGHTMARGIN;

    // pPos is a pure IN parameter and must not be evaluated.
    // pIter->GetCursorOfst returns from a nesting with COMPLETE_STRING.
    // If SwTextIter::GetCursorOfst calls GetCursorOfst further by itself
    // nNode changes the position.
    // In such cases, pPos must not be calculated.
        if( COMPLETE_STRING != nOffset )
        {
            SwTextNode* pTextNd = const_cast<SwTextFrame*>(this)->GetTextNode();
            pPos->nNode = *pTextNd;
            pPos->nContent.Assign( pTextNd, nOffset );
            if( pFillData )
            {
                if (pTextNd->GetText().getLength() > nOffset ||
                    rPoint.Y() < Frame().Top() )
                    pFillData->bInner = true;
                pFillData->bFirstLine = aLine.GetLineNr() < 2;
                if (pTextNd->GetText().getLength())
                {
                    pFillData->bEmpty = false;
                    pFillData->nLineWidth = aLine.GetCurr()->Width();
                }
            }
        }
    }
    bool bChgFillData = false;
    if( pFillData && FindPageFrame()->Frame().IsInside( aOldPoint ) )
    {
        FillCursorPos( *pFillData );
        bChgFillData = true;
    }

    if ( IsVertical() )
    {
        if ( bChgFillData )
            SwitchHorizontalToVertical( pFillData->Fill().aCursor.Pos() );
        const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() && bChgFillData )
    {
            SwitchLTRtoRTL( pFillData->Fill().aCursor.Pos() );
            const sal_Int16 eOrient = pFillData->pCMS->m_pFill->eOrient;

            if ( text::HoriOrientation::LEFT == eOrient )
                pFillData->SetOrient( text::HoriOrientation::RIGHT );
            else if ( text::HoriOrientation::RIGHT == eOrient )
                pFillData->SetOrient( text::HoriOrientation::LEFT );
    }

    (Point&)rPoint = aOldPoint;
    delete pFillData;

    return true;
}

bool SwTextFrame::GetCursorOfst(SwPosition* pPos, Point& rPoint,
                               SwCursorMoveState* pCMS, bool ) const
{
    const bool bChgFrame = !(pCMS && MV_UPDOWN == pCMS->m_eState);
    return GetCursorOfst_( pPos, rPoint, bChgFrame, pCMS );
}

/*
 * Layout-oriented cursor movement to the line start.
 */

bool SwTextFrame::LeftMargin(SwPaM *pPam) const
{
    if( &pPam->GetNode() != GetNode() )
        pPam->GetPoint()->nNode = *const_cast<SwTextFrame*>(this)->GetTextNode();

    SwTextFrame *pFrame = GetAdjFrameAtPos( const_cast<SwTextFrame*>(this), *pPam->GetPoint(),
                                     SwTextCursor::IsRightMargin() );
    pFrame->GetFormatted();
    sal_Int32 nIndx;
    if ( pFrame->IsEmpty() )
        nIndx = 0;
    else
    {
        SwTextSizeInfo aInf( pFrame );
        SwTextCursor  aLine( pFrame, &aInf );

        aLine.CharCursorToLine(pPam->GetPoint()->nContent.GetIndex());
        nIndx = aLine.GetStart();
        if( pFrame->GetOfst() && !pFrame->IsFollow() && !aLine.GetPrev() )
        {
            sw_ChangeOffset( pFrame, 0 );
            nIndx = 0;
        }
    }
    pPam->GetPoint()->nContent.Assign(pFrame->GetTextNode(), nIndx);
    SwTextCursor::SetRightMargin( false );
    return true;
}

/*
 * To the line end: That's the position before the last char of the line.
 * Exception: In the last line, it should be able to place the cursor after
 * the last char in order to append text.
 */

bool SwTextFrame::RightMargin(SwPaM *pPam, bool bAPI) const
{
    if( &pPam->GetNode() != GetNode() )
        pPam->GetPoint()->nNode = *const_cast<SwTextFrame*>(this)->GetTextNode();

    SwTextFrame *pFrame = GetAdjFrameAtPos( const_cast<SwTextFrame*>(this), *pPam->GetPoint(),
                                     SwTextCursor::IsRightMargin() );
    pFrame->GetFormatted();
    sal_Int32 nRightMargin;
    if ( IsEmpty() )
        nRightMargin = 0;
    else
    {
        SwTextSizeInfo aInf( pFrame );
        SwTextCursor  aLine( pFrame, &aInf );

        aLine.CharCursorToLine(pPam->GetPoint()->nContent.GetIndex());
        nRightMargin = aLine.GetStart() + aLine.GetCurr()->GetLen();

        // We skip hard line brakes
        if( aLine.GetCurr()->GetLen() &&
            CH_BREAK == aInf.GetText()[nRightMargin - 1])
            --nRightMargin;
        else if( !bAPI && (aLine.GetNext() || pFrame->GetFollow()) )
        {
            while( nRightMargin > aLine.GetStart() &&
                ' ' == aInf.GetText()[nRightMargin - 1])
                --nRightMargin;
        }
    }
    pPam->GetPoint()->nContent.Assign(pFrame->GetTextNode(), nRightMargin);
    SwTextCursor::SetRightMargin( !bAPI );
    return true;
}

// The following two methods try to put the Cursor into the next/succsessive
// line. If we do not have a preceding/successive line we forward the call
// to the base class.
// The Cursor's horizontal justification is done afterwards by the CursorShell.

class SwSetToRightMargin
{
    bool bRight;
public:
    inline SwSetToRightMargin() : bRight( false ) { }
    inline ~SwSetToRightMargin() { SwTextCursor::SetRightMargin( bRight ); }
    inline void SetRight( const bool bNew ) { bRight = bNew; }
};

bool SwTextFrame::UnitUp_( SwPaM *pPam, const SwTwips nOffset,
                        bool bSetInReadOnly ) const
{
    // Set the RightMargin if needed
    SwSetToRightMargin aSet;

    if( IsInTab() &&
        pPam->GetNode().StartOfSectionNode() !=
        pPam->GetNode( false ).StartOfSectionNode() )
    {
        // If the PaM is located within different boxes, we have a table selection,
        // which is handled by the base class.
        return SwContentFrame::UnitUp( pPam, nOffset, bSetInReadOnly );
    }

    const_cast<SwTextFrame*>(this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;

    if( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() )
                sw_ChangeOffset( const_cast<SwTextFrame*>(this), nFormat );

            SwTextSizeInfo aInf( const_cast<SwTextFrame*>(this) );
            SwTextCursor  aLine( const_cast<SwTextFrame*>(this), &aInf );

            // Optimize away flys with no flow and IsDummy()
            if( nPos )
                aLine.CharCursorToLine( nPos );
            else
                aLine.Top();

            const SwLineLayout *pPrevLine = aLine.GetPrevLine();
            const sal_Int32 nStart = aLine.GetStart();
            aLine.GetCharRect( &aCharBox, nPos );

            bool bSecondOfDouble = ( aInf.IsMulti() && ! aInf.IsFirstMulti() );
            bool bPrevLine = ( pPrevLine && pPrevLine != aLine.GetCurr() );

            if( !pPrevLine && !bSecondOfDouble && GetOfst() && !IsFollow() )
            {
                nFormat = GetOfst();
                sal_Int32 nDiff = aLine.GetLength();
                if( !nDiff )
                    nDiff = MIN_OFFSET_STEP;
                if( nFormat > nDiff )
                    nFormat = nFormat - nDiff;
                else
                    nFormat = 0;
                continue;
            }

            // We select the target line for the cursor, in case we are in a
            // double line portion, prev line = curr line
            if( bPrevLine && !bSecondOfDouble )
            {
                aLine.PrevLine();
                while ( aLine.GetStart() == nStart &&
                        nullptr != ( pPrevLine = aLine.GetPrevLine() ) &&
                        pPrevLine != aLine.GetCurr() )
                    aLine.PrevLine();
            }

            if ( bPrevLine || bSecondOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
                aCharBox.Pos().X() = aCharBox.Pos().X() - 150;

                // See comment in SwTextFrame::GetCursorOfst()
#if OSL_DEBUG_LEVEL > 0
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                // The node should not be changed
                sal_Int32 nTmpOfst = aLine.GetCursorOfst( pPam->GetPoint(),
                                                         aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                        "SwTextFrame::UnitUp: illegal node change" );
#endif

                // We make sure that we move up.
                if( nTmpOfst >= nStart && nStart && !bSecondOfDouble )
                {
                    nTmpOfst = nStart;
                    aSet.SetRight( true );
                }
                pPam->GetPoint()->nContent.Assign(const_cast<SwTextFrame*>(this)->GetTextNode(), nTmpOfst);
                return true;
            }

            if ( IsFollow() )
            {
                aLine.GetCharRect( &aCharBox, nPos );
                aCharBox.SSize().Width() /= 2;
            }
            break;
        } while ( true );
    }
    /* If 'this' is a follow and a prev failed, we need to go to the
     * last line of the master, which is us.
     * Or: If we are a follow with follow, we need to get the master.
     */
    if ( IsFollow() )
    {
        const SwTextFrame *pTmpPrev = FindMaster();
        sal_Int32 nOffs = GetOfst();
        if( pTmpPrev )
        {
            SwViewShell *pSh = getRootFrame()->GetCurrShell();
            const bool bProtectedAllowed = pSh && pSh->GetViewOptions()->IsCursorInProtectedArea();
            const SwTextFrame *pPrevPrev = pTmpPrev;
            // We skip protected frames and frames without content here
            while( pPrevPrev && ( pPrevPrev->GetOfst() == nOffs ||
                   ( !bProtectedAllowed && pPrevPrev->IsProtected() ) ) )
            {
                pTmpPrev = pPrevPrev;
                nOffs = pTmpPrev->GetOfst();
                if ( pPrevPrev->IsFollow() )
                    pPrevPrev = pTmpPrev->FindMaster();
                else
                    pPrevPrev = nullptr;
            }
            if ( !pPrevPrev )
                return pTmpPrev->SwContentFrame::UnitUp( pPam, nOffset, bSetInReadOnly );
            aCharBox.Pos().Y() = pPrevPrev->Frame().Bottom() - 1;
            return pPrevPrev->GetKeyCursorOfst( pPam->GetPoint(), aCharBox.Pos() );
        }
    }
    return SwContentFrame::UnitUp( pPam, nOffset, bSetInReadOnly );
}

// Used for Bidi. nPos is the logical position in the string, bLeft indicates
// if left arrow or right arrow was pressed. The return values are:
// nPos: the new visual position
// bLeft: whether the break iterator has to add or subtract from the
//        current position
static void lcl_VisualMoveRecursion( const SwLineLayout& rCurrLine, sal_Int32 nIdx,
                              sal_Int32& nPos, bool& bRight,
                              sal_uInt8& nCursorLevel, sal_uInt8 nDefaultDir )
{
    const SwLinePortion* pPor = rCurrLine.GetFirstPortion();
    const SwLinePortion* pLast = nullptr;

    // What's the current portion?
    while ( pPor && nIdx + pPor->GetLen() <= nPos )
    {
        nIdx = nIdx + pPor->GetLen();
        pLast = pPor;
        pPor = pPor->GetPortion();
    }

    if ( bRight )
    {
        bool bRecurse = pPor && pPor->IsMultiPortion() &&
                           static_cast<const SwMultiPortion*>(pPor)->IsBidi();

        // 1. special case: at beginning of bidi portion
        if ( bRecurse && nIdx == nPos )
        {
            nPos = nPos + pPor->GetLen();

            // leave bidi portion
            if ( nCursorLevel != nDefaultDir )
            {
                bRecurse = false;
            }
            else
                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is between c and X in the buffer and cursor level = 0
                nCursorLevel++;
        }

        // 2. special case: at beginning of portion after bidi portion
        else if ( pLast && pLast->IsMultiPortion() &&
                 static_cast<const SwMultiPortion*>(pLast)->IsBidi() && nIdx == nPos )
        {
            // enter bidi portion
            if ( nCursorLevel != nDefaultDir )
            {
                bRecurse = true;
                nIdx = nIdx - pLast->GetLen();
                pPor = pLast;
            }
        }

        // Recursion
        if ( bRecurse )
        {
            const SwLineLayout& rLine = static_cast<const SwMultiPortion*>(pPor)->GetRoot();
            sal_Int32 nTmpPos = nPos - nIdx;
            bool bTmpForward = ! bRight;
            sal_uInt8 nTmpCursorLevel = nCursorLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCursorLevel, nDefaultDir + 1 );

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCursorLevel = nTmpCursorLevel;
        }

        // go forward
        else
        {
            bRight = true;
            nCursorLevel = nDefaultDir;
        }

    }
    else
    {
        bool bRecurse = pPor && pPor->IsMultiPortion() && static_cast<const SwMultiPortion*>(pPor)->IsBidi();

        // 1. special case: at beginning of bidi portion
        if ( bRecurse && nIdx == nPos )
        {
            // leave bidi portion
            if ( nCursorLevel == nDefaultDir )
            {
                bRecurse = false;
            }
        }

        // 2. special case: at beginning of portion after bidi portion
        else if ( pLast && pLast->IsMultiPortion() &&
                 static_cast<const SwMultiPortion*>(pLast)->IsBidi() && nIdx == nPos )
        {
            nPos = nPos - pLast->GetLen();

            // enter bidi portion
            if ( nCursorLevel % 2 == nDefaultDir % 2 )
            {
                bRecurse = true;
                nIdx = nIdx - pLast->GetLen();
                pPor = pLast;

                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is behind 3 in the buffer and cursor level = 2
                if ( nDefaultDir + 2 == nCursorLevel )
                    nPos = nPos + pLast->GetLen();
            }
        }

        // go forward
        if ( bRecurse )
        {
            const SwLineLayout& rLine = static_cast<const SwMultiPortion*>(pPor)->GetRoot();
            sal_Int32 nTmpPos = nPos - nIdx;
            bool bTmpForward = ! bRight;
            sal_uInt8 nTmpCursorLevel = nCursorLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCursorLevel, nDefaultDir + 1 );

            // special case:
            // buffer: abcXYZ123 in LTR paragraph
            // view:   abc123ZYX
            // cursor is between Z and 1 in the buffer and cursor level = 2
            if ( nTmpPos == pPor->GetLen() && nTmpCursorLevel == nDefaultDir + 1 )
            {
                nTmpPos = nTmpPos - pPor->GetLen();
                nTmpCursorLevel = nDefaultDir;
                bTmpForward = ! bTmpForward;
            }

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCursorLevel = nTmpCursorLevel;
        }

        // go backward
        else
        {
            bRight = false;
            nCursorLevel = nDefaultDir;
        }
    }
}

void SwTextFrame::PrepareVisualMove( sal_Int32& nPos, sal_uInt8& nCursorLevel,
                                  bool& bForward, bool bInsertCursor )
{
    if( IsEmpty() || IsHiddenNow() )
        return;

    GetFormatted();

    SwTextSizeInfo aInf(this);
    SwTextCursor  aLine(this, &aInf);

    if( nPos )
        aLine.CharCursorToLine( nPos );
    else
        aLine.Top();

    const SwLineLayout* pLine = aLine.GetCurr();
    const sal_Int32 nStt = aLine.GetStart();
    const sal_Int32 nLen = pLine->GetLen();

    // We have to distinguish between an insert and overwrite cursor:
    // The insert cursor position depends on the cursor level:
    // buffer:  abcXYZdef in LTR paragraph
    // display: abcZYXdef
    // If cursor is between c and X in the buffer and cursor level is 0,
    // the cursor blinks between c and Z and -> sets the cursor between Z and Y.
    // If the cursor level is 1, the cursor blinks between X and d and
    // -> sets the cursor between d and e.
    // The overwrite cursor simply travels to the next visual character.
    if ( bInsertCursor )
    {
        lcl_VisualMoveRecursion( *pLine, nStt, nPos, bForward,
                                 nCursorLevel, IsRightToLeft() ? 1 : 0 );
        return;
    }

    const sal_uInt8 nDefaultDir = static_cast<sal_uInt8>(IsRightToLeft() ? UBIDI_RTL : UBIDI_LTR);
    const bool bVisualRight = ( nDefaultDir == UBIDI_LTR && bForward ) ||
                                  ( nDefaultDir == UBIDI_RTL && ! bForward );

    // Bidi functions from icu 2.0

    const sal_Unicode* pLineString = GetTextNode()->GetText().getStr();

    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( nLen, 0, &nError );
    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), nLen, nDefaultDir, nullptr, &nError );

    sal_Int32 nTmpPos = 0;
    bool bOutOfBounds = false;

    if ( nPos < nStt + nLen )
    {
        nTmpPos = ubidi_getVisualIndex( pBidi, nPos, &nError );

        // visual indices are always LTR aligned
        if ( bVisualRight )
        {
            if ( nTmpPos + 1 < nStt + nLen )
                ++nTmpPos;
            else
            {
                nPos = nDefaultDir == UBIDI_RTL ? 0 : nStt + nLen;
                bOutOfBounds = true;
            }
        }
        else
        {
            if ( nTmpPos )
                --nTmpPos;
            else
            {
                nPos = nDefaultDir == UBIDI_RTL ? nStt + nLen : 0;
                bOutOfBounds = true;
            }
        }
    }
    else
    {
        nTmpPos = nDefaultDir == UBIDI_LTR ? nPos - 1 : 0;
    }

    if ( ! bOutOfBounds )
    {
        nPos = ubidi_getLogicalIndex( pBidi, nTmpPos, &nError );

        if ( bForward )
        {
            if ( nPos )
                --nPos;
            else
            {
                ++nPos;
                bForward = ! bForward;
            }
        }
        else
            ++nPos;
    }

    ubidi_close( pBidi );
}

bool SwTextFrame::UnitDown_(SwPaM *pPam, const SwTwips nOffset,
                         bool bSetInReadOnly ) const
{

    if ( IsInTab() &&
        pPam->GetNode().StartOfSectionNode() !=
        pPam->GetNode( false ).StartOfSectionNode() )
    {
        // If the PaM is located within different boxes, we have a table selection,
        // which is handled by the base class.
        return SwContentFrame::UnitDown( pPam, nOffset, bSetInReadOnly );
    }
    const_cast<SwTextFrame*>(this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;
    const SwContentFrame *pTmpFollow = nullptr;

    if ( IsVertical() )
        const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();

    if ( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() &&
                !sw_ChangeOffset( const_cast<SwTextFrame*>(this), nFormat ) )
                break;

            SwTextSizeInfo aInf( const_cast<SwTextFrame*>(this) );
            SwTextCursor  aLine( const_cast<SwTextFrame*>(this), &aInf );
            nFormat = aLine.GetEnd();

            aLine.CharCursorToLine( nPos );

            const SwLineLayout* pNextLine = aLine.GetNextLine();
            const sal_Int32 nStart = aLine.GetStart();
            aLine.GetCharRect( &aCharBox, nPos );

            bool bFirstOfDouble = ( aInf.IsMulti() && aInf.IsFirstMulti() );

            if( pNextLine || bFirstOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
#if OSL_DEBUG_LEVEL > 0
                // See comment in SwTextFrame::GetCursorOfst()
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                if ( pNextLine && ! bFirstOfDouble )
                    aLine.NextLine();

                sal_Int32 nTmpOfst = aLine.GetCursorOfst( pPam->GetPoint(),
                                 aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                    "SwTextFrame::UnitDown: illegal node change" );
#endif

                // We make sure that we move down.
                if( nTmpOfst <= nStart && ! bFirstOfDouble )
                    nTmpOfst = nStart + 1;
                pPam->GetPoint()->nContent.Assign(const_cast<SwTextFrame*>(this)->GetTextNode(), nTmpOfst);

                if ( IsVertical() )
                    const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();

                return true;
            }
            if( nullptr != ( pTmpFollow = GetFollow() ) )
            {   // Skip protected follows
                const SwContentFrame* pTmp = pTmpFollow;
                SwViewShell *pSh = getRootFrame()->GetCurrShell();
                if( !pSh || !pSh->GetViewOptions()->IsCursorInProtectedArea() )
                {
                    while( pTmpFollow && pTmpFollow->IsProtected() )
                    {
                        pTmp = pTmpFollow;
                        pTmpFollow = pTmpFollow->GetFollow();
                    }
                }
                if( !pTmpFollow ) // Only protected ones left
                {
                    if ( IsVertical() )
                        const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();
                    return pTmp->SwContentFrame::UnitDown( pPam, nOffset, bSetInReadOnly );
                }

                aLine.GetCharRect( &aCharBox, nPos );
                aCharBox.SSize().Width() /= 2;
            }
            else if( !IsFollow() )
            {
                sal_Int32 nTmpLen = aInf.GetText().getLength();
                if( aLine.GetEnd() < nTmpLen )
                {
                    if( nFormat <= GetOfst() )
                    {
                        nFormat = std::min( sal_Int32( GetOfst() + MIN_OFFSET_STEP ),
                                       static_cast<sal_Int32>(nTmpLen) );
                        if( nFormat <= GetOfst() )
                            break;
                    }
                    continue;
                }
            }
            break;
        } while( true );
    }
    else
        pTmpFollow = GetFollow();

    if ( IsVertical() )
        const_cast<SwTextFrame*>(this)->SwapWidthAndHeight();

    // We take a shortcut for follows
    if( pTmpFollow )
    {
        aCharBox.Pos().Y() = pTmpFollow->Frame().Top() + 1;
        return static_cast<const SwTextFrame*>(pTmpFollow)->GetKeyCursorOfst( pPam->GetPoint(),
                                                     aCharBox.Pos() );
    }
    return SwContentFrame::UnitDown( pPam, nOffset, bSetInReadOnly );
}

bool SwTextFrame::UnitUp(SwPaM *pPam, const SwTwips nOffset,
                      bool bSetInReadOnly ) const
{
    /* We call ContentNode::GertFrame() in CursorSh::Up().
     * This _always returns the master.
     * In order to not mess with cursor travelling, we correct here
     * in SwTextFrame.
     * We calculate UnitUp for pFrame. pFrame is either a master (= this) or a
     * follow (!= this).
     */
    const SwTextFrame *pFrame = GetAdjFrameAtPos( const_cast<SwTextFrame*>(this), *(pPam->GetPoint()),
                                           SwTextCursor::IsRightMargin() );
    const bool bRet = pFrame->UnitUp_( pPam, nOffset, bSetInReadOnly );

    // No SwTextCursor::SetRightMargin( false );
    // Instead we have a SwSetToRightMargin in UnitUp_
    return bRet;
}

bool SwTextFrame::UnitDown(SwPaM *pPam, const SwTwips nOffset,
                        bool bSetInReadOnly ) const
{
    const SwTextFrame *pFrame = GetAdjFrameAtPos(const_cast<SwTextFrame*>(this), *(pPam->GetPoint()),
                                           SwTextCursor::IsRightMargin() );
    const bool bRet = pFrame->UnitDown_( pPam, nOffset, bSetInReadOnly );
    SwTextCursor::SetRightMargin( false );
    return bRet;
}

void SwTextFrame::FillCursorPos( SwFillData& rFill ) const
{
    if( !rFill.bColumn && GetUpper()->IsColBodyFrame() ) // ColumnFrames now with BodyFrame
    {
        const SwColumnFrame* pTmp =
            static_cast<const SwColumnFrame*>(GetUpper()->GetUpper()->GetUpper()->Lower()); // The 1st column
        // The first SwFrame in BodyFrame of the first column
        const SwFrame* pFrame = static_cast<const SwLayoutFrame*>(pTmp->Lower())->Lower();
        sal_uInt16 nNextCol = 0;
        // In which column do we end up in?
        while( rFill.X() > pTmp->Frame().Right() && pTmp->GetNext() )
        {
            pTmp = static_cast<const SwColumnFrame*>(pTmp->GetNext());
            if( static_cast<const SwLayoutFrame*>(pTmp->Lower())->Lower() ) // ColumnFrames now with BodyFrame
            {
                pFrame = static_cast<const SwLayoutFrame*>(pTmp->Lower())->Lower();
                nNextCol = 0;
            }
            else
                ++nNextCol; // Empty columns require column brakes
        }
        if( pTmp != GetUpper()->GetUpper() ) // Did we end up in another column?
        {
            if( !pFrame )
                return;
            if( nNextCol )
            {
                while( pFrame->GetNext() )
                    pFrame = pFrame->GetNext();
            }
            else
            {
                while( pFrame->GetNext() && pFrame->Frame().Bottom() < rFill.Y() )
                    pFrame = pFrame->GetNext();
            }
            // No filling, if the last frame in the targeted column does
            // not contain a paragraph, but e.g. a table
            if( pFrame->IsTextFrame() )
            {
                rFill.Fill().nColumnCnt = nNextCol;
                rFill.bColumn = true;
                if( rFill.pPos )
                {
                    SwTextNode* pTextNd = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pFrame))->GetTextNode();
                    rFill.pPos->nNode = *pTextNd;
                    rFill.pPos->nContent.Assign(
                            pTextNd, pTextNd->GetText().getLength());
                }
                if( nNextCol )
                {
                    rFill.aFrame = pTmp->Prt();
                    rFill.aFrame += pTmp->Frame().Pos();
                }
                else
                    rFill.aFrame = pFrame->Frame();
                static_cast<const SwTextFrame*>(pFrame)->FillCursorPos( rFill );
            }
            return;
        }
    }
    SwFont *pFnt;
    SwTextFormatColl* pColl = GetTextNode()->GetTextColl();
    SwTwips nFirst = GetTextNode()->GetSwAttrSet().GetULSpace().GetLower();
    SwTwips nDiff = rFill.Y() - Frame().Bottom();
    if( nDiff < nFirst )
        nDiff = -1;
    else
        pColl = &pColl->GetNextTextFormatColl();
    SwAttrSet aSet( const_cast<SwDoc*>(GetTextNode()->GetDoc())->GetAttrPool(), aTextFormatCollSetRange );
    const SwAttrSet* pSet = &pColl->GetAttrSet();
    SwViewShell *pSh = getRootFrame()->GetCurrShell();
    if( GetTextNode()->HasSwAttrSet() )
    {
        aSet.Put( *GetTextNode()->GetpSwAttrSet() );
        aSet.SetParent( pSet );
        pSet = &aSet;
        pFnt = new SwFont( pSet, GetNode()->getIDocumentSettingAccess() );
    }
    else
    {
        SwFontAccess aFontAccess( pColl, pSh );
        pFnt = new SwFont( aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }
    OutputDevice* pOut = pSh->GetOut();
    if( !pSh->GetViewOptions()->getBrowseMode() || pSh->GetViewOptions()->IsPrtFormat() )
        pOut = GetTextNode()->getIDocumentDeviceAccess().getReferenceDevice( true );

    pFnt->SetFntChg( true );
    pFnt->ChgPhysFnt( pSh, *pOut );

    SwTwips nLineHeight = pFnt->GetHeight( pSh, *pOut );

    bool bFill = false;
    if( nLineHeight )
    {
        bFill = true;
        const SvxULSpaceItem &rUL = pSet->GetULSpace();
        SwTwips nDist = std::max( rUL.GetLower(), rUL.GetUpper() );
        if( rFill.Fill().nColumnCnt )
        {
            rFill.aFrame.Height( nLineHeight );
            nDiff = rFill.Y() - rFill.Bottom();
            nFirst = 0;
        }
        else if( nDist < nFirst )
            nFirst = nFirst - nDist;
        else
            nFirst = 0;
        nDist = std::max( nDist, long( GetLineSpace() ) );
        nDist += nLineHeight;
        nDiff -= nFirst;

        if( nDiff > 0 )
        {
            nDiff /= nDist;
            rFill.Fill().nParaCnt = static_cast<sal_uInt16>(nDiff + 1);
            rFill.nLineWidth = 0;
            rFill.bInner = false;
            rFill.bEmpty = true;
            rFill.SetOrient( text::HoriOrientation::LEFT );
        }
        else
            nDiff = -1;
        if( rFill.bInner )
            bFill = false;
        else
        {
            const SvxTabStopItem &rRuler = pSet->GetTabStops();
            const SvxLRSpaceItem &rLRSpace = pSet->GetLRSpace();

            SwRect &rRect = rFill.Fill().aCursor;
            rRect.Top( rFill.Bottom() + (nDiff+1) * nDist - nLineHeight );
            if( nFirst && nDiff > -1 )
                rRect.Top( rRect.Top() + nFirst );
            rRect.Height( nLineHeight );
            SwTwips nLeft = rFill.Left() + rLRSpace.GetLeft() +
                            GetTextNode()->GetLeftMarginWithNum();
            SwTwips nRight = rFill.Right() - rLRSpace.GetRight();
            SwTwips nCenter = ( nLeft + nRight ) / 2;
            rRect.Left( nLeft );
            if( FILL_MARGIN == rFill.Mode() )
            {
                if( rFill.bEmpty )
                {
                    rFill.SetOrient( text::HoriOrientation::LEFT );
                    if( rFill.X() < nCenter )
                    {
                        if( rFill.X() > ( nLeft + 2 * nCenter ) / 3 )
                        {
                            rFill.SetOrient( text::HoriOrientation::CENTER );
                            rRect.Left( nCenter );
                        }
                    }
                    else if( rFill.X() > ( nRight + 2 * nCenter ) / 3 )
                    {
                        rFill.SetOrient( text::HoriOrientation::RIGHT );
                        rRect.Left( nRight );
                    }
                    else
                    {
                        rFill.SetOrient( text::HoriOrientation::CENTER );
                        rRect.Left( nCenter );
                    }
                }
                else
                    bFill = false;
            }
            else
            {
                SwTwips nSpace = 0;
                if( FILL_TAB != rFill.Mode() )
                {
                    const OUString aTmp("  ");
                    SwDrawTextInfo aDrawInf( pSh, *pOut, nullptr, aTmp, 0, 2 );
                    nSpace = pFnt->GetTextSize_( aDrawInf ).Width()/2;
                }
                if( rFill.X() >= nRight )
                {
                    if( FILL_INDENT != rFill.Mode() && ( rFill.bEmpty ||
                        rFill.X() > rFill.nLineWidth + FILL_MIN_DIST ) )
                    {
                        rFill.SetOrient( text::HoriOrientation::RIGHT );
                        rRect.Left( nRight );
                    }
                    else
                        bFill = false;
                }
                else if( FILL_INDENT == rFill.Mode() )
                {
                    SwTwips nIndent = rFill.X();
                    if( !rFill.bEmpty || nIndent > nRight )
                        bFill = false;
                    else
                    {
                        nIndent -= rFill.Left();
                        if( nIndent >= 0 && nSpace )
                        {
                            nIndent /= nSpace;
                            nIndent *= nSpace;
                            rFill.SetTab( sal_uInt16( nIndent ) );
                            rRect.Left( nIndent + rFill.Left() );
                        }
                        else
                            bFill = false;
                    }
                }
                else if( rFill.X() > nLeft )
                {
                    SwTwips nTextLeft = rFill.Left() + rLRSpace.GetTextLeft() +
                                    GetTextNode()->GetLeftMarginWithNum( true );
                    rFill.nLineWidth += rFill.bFirstLine ? nLeft : nTextLeft;
                    SwTwips nLeftTab;
                    SwTwips nRightTab = nLeft;
                    sal_uInt16 nSpaceCnt = 0;
                    sal_uInt16 nTabCnt = 0;
                    sal_uInt16 nIdx = 0;
                    do
                    {
                        nLeftTab = nRightTab;
                        if( nIdx < rRuler.Count() )
                        {
                            const SvxTabStop &rTabStop = rRuler.operator[](nIdx);
                            nRightTab = nTextLeft + rTabStop.GetTabPos();
                            if( nLeftTab < nTextLeft && nRightTab > nTextLeft )
                                nRightTab = nTextLeft;
                            else
                                ++nIdx;
                            if( nRightTab > rFill.nLineWidth )
                                ++nTabCnt;
                        }
                        else
                        {
                            const SvxTabStopItem& rTab =
                                static_cast<const SvxTabStopItem &>(pSet->
                                GetPool()->GetDefaultItem( RES_PARATR_TABSTOP ));
                            const SwTwips nDefTabDist = rTab[0].GetTabPos();
                            nRightTab = nLeftTab - nTextLeft;
                            nRightTab /= nDefTabDist;
                            nRightTab = nRightTab * nDefTabDist + nTextLeft;
                            while ( nRightTab <= nLeftTab )
                                nRightTab += nDefTabDist;
                            if( nRightTab > rFill.nLineWidth )
                                ++nTabCnt;
                            while ( nRightTab < rFill.X() )
                            {
                                nRightTab += nDefTabDist;
                                if( nRightTab > rFill.nLineWidth )
                                    ++nTabCnt;
                            }
                            if( nLeftTab < nRightTab - nDefTabDist )
                                nLeftTab = nRightTab - nDefTabDist;
                        }
                        if( nRightTab > nRight )
                            nRightTab = nRight;
                    }
                    while( rFill.X() > nRightTab );
                    --nTabCnt;
                    if( FILL_TAB != rFill.Mode() )
                    {
                        if( nSpace > 0 )
                        {
                            if( !nTabCnt )
                                nLeftTab = rFill.nLineWidth;
                            while( nLeftTab < rFill.X() )
                            {
                                nLeftTab += nSpace;
                                ++nSpaceCnt;
                            }
                            if( nSpaceCnt )
                            {
                                nLeftTab -= nSpace;
                                --nSpaceCnt;
                            }
                            if( rFill.X() - nLeftTab > nRightTab - rFill.X() )
                            {
                                nSpaceCnt = 0;
                                ++nTabCnt;
                                rRect.Left( nRightTab );
                            }
                            else
                            {
                                if( rFill.X() - nLeftTab > nSpace/2 )
                                {
                                    ++nSpaceCnt;
                                    rRect.Left( nLeftTab + nSpace );
                                }
                                else
                                    rRect.Left( nLeftTab );
                            }
                        }
                        else if( rFill.X() - nLeftTab < nRightTab - rFill.X() )
                            rRect.Left( nLeftTab );
                        else
                        {
                            if( nRightTab >= nRight )
                            {
                                rFill.SetOrient( text::HoriOrientation::RIGHT );
                                rRect.Left( nRight );
                                nTabCnt = 0;
                                nSpaceCnt = 0;
                            }
                            else
                            {
                                rRect.Left( nRightTab );
                                ++nTabCnt;
                            }
                        }
                    }
                    else
                    {
                        if( rFill.X() - nLeftTab < nRightTab - rFill.X() )
                            rRect.Left( nLeftTab );
                        else
                        {
                            if( nRightTab >= nRight )
                            {
                                rFill.SetOrient( text::HoriOrientation::RIGHT );
                                rRect.Left( nRight );
                                nTabCnt = 0;
                                nSpaceCnt = 0;
                            }
                            else
                            {
                                rRect.Left( nRightTab );
                                ++nTabCnt;
                            }
                        }
                    }
                    rFill.SetTab( nTabCnt );
                    rFill.SetSpace( nSpaceCnt );
                    if( bFill )
                    {
                        if( std::abs( rFill.X() - nCenter ) <=
                            std::abs( rFill.X() - rRect.Left() ) )
                        {
                            rFill.SetOrient( text::HoriOrientation::CENTER );
                            rFill.SetTab( 0 );
                            rFill.SetSpace( 0 );
                            rRect.Left( nCenter );
                        }
                        if( !rFill.bEmpty )
                            rFill.nLineWidth += FILL_MIN_DIST;
                        if( rRect.Left() < rFill.nLineWidth )
                            bFill = false;
                    }
                }
            }
            // Do we extend over the page's/column's/etc. lower edge?
            const SwFrame* pUp = GetUpper();
            if( pUp->IsInSct() )
            {
                if( pUp->IsSctFrame() )
                    pUp = pUp->GetUpper();
                else if( pUp->IsColBodyFrame() &&
                         pUp->GetUpper()->GetUpper()->IsSctFrame() )
                    pUp = pUp->GetUpper()->GetUpper()->GetUpper();
            }
            SwRectFnSet aRectFnSet(this);
            SwTwips nLimit = aRectFnSet.GetPrtBottom(*pUp);
            SwTwips nRectBottom = rRect.Bottom();
            if ( aRectFnSet.IsVert() )
                nRectBottom = SwitchHorizontalToVertical( nRectBottom );

            if( aRectFnSet.YDiff( nLimit, nRectBottom ) < 0 )
                bFill = false;
            else
                rRect.Width( 1 );
        }
    }
    const_cast<SwCursorMoveState*>(rFill.pCMS)->m_bFillRet = bFill;
    delete pFnt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
