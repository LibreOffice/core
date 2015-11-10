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
 * - SwTextCursor::bRightMargin is set to false by CharCrsrToLine
 */

namespace
{

SwTextFrm *GetAdjFrmAtPos( SwTextFrm *pFrm, const SwPosition &rPos,
                          const bool bRightMargin, const bool bNoScroll = true )
{
    // RightMargin in the last master line
    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTextFrm *pFrmAtPos = pFrm;
    if( !bNoScroll || pFrm->GetFollow() )
    {
        pFrmAtPos = pFrm->GetFrmAtPos( rPos );
        if( nOffset < pFrmAtPos->GetOfst() &&
            !pFrmAtPos->IsFollow() )
        {
            sal_Int32 nNew = nOffset;
            if( nNew < MIN_OFFSET_STEP )
                nNew = 0;
            else
                nNew -= MIN_OFFSET_STEP;
            sw_ChangeOffset( pFrmAtPos, nNew );
        }
    }
    while( pFrm != pFrmAtPos )
    {
        pFrm = pFrmAtPos;
        pFrm->GetFormatted();
        pFrmAtPos = pFrm->GetFrmAtPos( rPos );
    }

    if( nOffset && bRightMargin )
    {
        while( pFrmAtPos && pFrmAtPos->GetOfst() == nOffset &&
               pFrmAtPos->IsFollow() )
        {
            pFrmAtPos->GetFormatted();
            pFrmAtPos = pFrmAtPos->FindMaster();
        }
        OSL_ENSURE( pFrmAtPos, "+GetCharRect: no frame with my rightmargin" );
    }
    return pFrmAtPos ? pFrmAtPos : pFrm;
}

}

bool sw_ChangeOffset( SwTextFrm* pFrm, sal_Int32 nNew )
{
    // Do not scroll in areas and outside of flies
    OSL_ENSURE( !pFrm->IsFollow(), "Illegal Scrolling by Follow!" );
    if( pFrm->GetOfst() != nNew && !pFrm->IsInSct() )
    {
        SwFlyFrm *pFly = pFrm->FindFlyFrm();
        // Attention: if e.g. in a column frame the size is still invalid
        // we must not scroll around just like that
        if ( ( pFly && pFly->IsValid() &&
             !pFly->GetNextLink() && !pFly->GetPrevLink() ) ||
             ( !pFly && pFrm->IsInTab() ) )
        {
            SwViewShell* pVsh = pFrm->getRootFrm()->GetCurrShell();
            if( pVsh )
            {
                if( pVsh->GetRingContainer().size() > 1 ||
                    ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->size() ) )
                {
                    if( !pFrm->GetOfst() )
                        return false;
                    nNew = 0;
                }
                pFrm->SetOfst( nNew );
                pFrm->SetPara( nullptr );
                pFrm->GetFormatted();
                if( pFrm->Frm().HasArea() )
                    pFrm->getRootFrm()->GetCurrShell()->InvalidateWindows( pFrm->Frm() );
                return true;
            }
        }
    }
    return false;
}

SwTextFrm& SwTextFrm::GetFrmAtOfst( const sal_Int32 nWhere )
{
    SwTextFrm* pRet = this;
    while( pRet->HasFollow() && nWhere >= pRet->GetFollow()->GetOfst() )
        pRet = pRet->GetFollow();
    return *pRet;
}

SwTextFrm *SwTextFrm::GetFrmAtPos( const SwPosition &rPos )
{
    SwTextFrm *pFoll = this;
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
 * GetCrsrOfst() does the reverse: It goes from a document coordinate to
 * a Pam.
 * Both are virtual in the frame base class and thus are redefined here.
 */

bool SwTextFrm::GetCharRect( SwRect& rOrig, const SwPosition &rPos,
                            SwCrsrMoveState *pCMS ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTextFrm::GetCharRect with swapped frame" );

    if( IsLocked() || IsHiddenNow() )
        return false;

    // Find the right frame first. We need to keep in mind that:
    // - the cached information could be invalid  (GetPara() == 0)
    // - we could have a Follow
    // - the Follow chain grows dynamically; the one we end up in
    //   needs to be formatted

    // Optimisation: reading ahead saves us a GetAdjFrmAtPos
    const bool bRightMargin = pCMS && ( MV_RIGHTMARGIN == pCMS->m_eState );
    const bool bNoScroll = pCMS && pCMS->m_bNoScroll;
    SwTextFrm *pFrm = GetAdjFrmAtPos( const_cast<SwTextFrm*>(this), rPos, bRightMargin,
                                     bNoScroll );
    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = static_cast<SwFrm*>(pFrm->GetUpper());

    SWRECTFN ( pFrm )
    const SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();
    const SwTwips nFrmMaxY = (pFrm->*fnRect->fnGetPrtBottom)();

    // nMaxY is an absolute value
    SwTwips nMaxY = bVert ?
                    ( bVertL2R ? std::min( nFrmMaxY, nUpperMaxY ) : std::max( nFrmMaxY, nUpperMaxY ) ) :
                    std::min( nFrmMaxY, nUpperMaxY );

    bool bRet = false;

    if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
    {
        Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
        SwTextNode* pTextNd = const_cast<SwTextFrm*>(this)->GetTextNode();
        short nFirstOffset;
        pTextNd->GetFirstLineOfsWithNum( nFirstOffset );

        Point aPnt2;
        if ( bVert )
        {
            if( nFirstOffset > 0 )
                aPnt1.Y() += nFirstOffset;
            if ( aPnt1.X() < nMaxY && !bVertL2R )
                aPnt1.X() = nMaxY;
            aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
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
            aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }

        rOrig = SwRect( aPnt1, aPnt2 );

        if ( pCMS )
        {
            pCMS->m_aRealHeight.X() = 0;
            pCMS->m_aRealHeight.Y() = bVert ? -rOrig.Width() : rOrig.Height();
        }

        if ( pFrm->IsRightToLeft() )
            pFrm->SwitchLTRtoRTL( rOrig );

        bRet = true;
    }
    else
    {
        if( !pFrm->HasPara() )
            return false;

        SwFrmSwapper aSwapper( pFrm, true );
        if ( bVert )
            nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );

        bool bGoOn = true;
        const sal_Int32 nOffset = rPos.nContent.GetIndex();
        sal_Int32 nNextOfst;

        do
        {
            {
                SwTextSizeInfo aInf( pFrm );
                SwTextCursor  aLine( pFrm, &aInf );
                nNextOfst = aLine.GetEnd();
                // See comment in AdjustFrm
                // Include the line's last char?
                bRet = bRightMargin ? aLine.GetEndCharRect( &rOrig, nOffset, pCMS, nMaxY )
                                : aLine.GetCharRect( &rOrig, nOffset, pCMS, nMaxY );
            }

            if ( pFrm->IsRightToLeft() )
                pFrm->SwitchLTRtoRTL( rOrig );

            if ( bVert )
                pFrm->SwitchHorizontalToVertical( rOrig );

            if( pFrm->IsUndersized() && pCMS && !pFrm->GetNext() &&
                (rOrig.*fnRect->fnGetBottom)() == nUpperMaxY &&
                pFrm->GetOfst() < nOffset &&
                !pFrm->IsFollow() && !bNoScroll &&
                pFrm->GetTextNode()->GetText().getLength() != nNextOfst)
            {
                bGoOn = sw_ChangeOffset( pFrm, nNextOfst );
            }
            else
                bGoOn = false;
        } while ( bGoOn );

        if ( pCMS )
        {
            if ( pFrm->IsRightToLeft() )
            {
                if( pCMS->m_b2Lines && pCMS->m_p2Lines)
                {
                    pFrm->SwitchLTRtoRTL( pCMS->m_p2Lines->aLine );
                    pFrm->SwitchLTRtoRTL( pCMS->m_p2Lines->aPortion );
                }
            }

            if ( bVert )
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
                    pFrm->SwitchHorizontalToVertical( pCMS->m_p2Lines->aLine );
                    pFrm->SwitchHorizontalToVertical( pCMS->m_p2Lines->aPortion );
                }
            }

        }
    }
    if( bRet )
    {
        SwPageFrm *pPage = pFrm->FindPageFrm();
        OSL_ENSURE( pPage, "Text escaped from page?" );
        const SwTwips nOrigTop = (rOrig.*fnRect->fnGetTop)();
        const SwTwips nPageTop = (pPage->Frm().*fnRect->fnGetTop)();
        const SwTwips nPageBott = (pPage->Frm().*fnRect->fnGetBottom)();

        // We have the following situation: if the frame is in an invalid
        // sectionframe, it's possible that the frame is outside the page.
        // If we restrict the cursor position to the page area, we enforce
        // the formatting of the page, of the section frame and the frame itself.
        if( (*fnRect->fnYDiff)( nPageTop, nOrigTop ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageTop );

        if ( (*fnRect->fnYDiff)( nOrigTop, nPageBott ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageBott );
    }

    return bRet;
}

/*
 * GetAutoPos() looks up the char's char line which is described by rPos
 * and is used by the auto-positioned frame.
 */

bool SwTextFrm::GetAutoPos( SwRect& rOrig, const SwPosition &rPos ) const
{
    if( IsHiddenNow() )
        return false;

    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTextFrm* pFrm = &(const_cast<SwTextFrm*>(this)->GetFrmAtOfst( nOffset ));

    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = static_cast<SwFrm*>(pFrm->GetUpper());

    SWRECTFN( pTmpFrm )
    SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();

    // nMaxY is in absolute value
    SwTwips nMaxY;
    if ( bVert )
    {
        if ( bVertL2R )
            nMaxY = std::min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );
        else
            nMaxY = std::max( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );
    }
    else
        nMaxY = std::min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );
    if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
    {
        Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
        Point aPnt2;
        if ( bVert )
        {
            if ( aPnt1.X() < nMaxY && !bVertL2R )
                aPnt1.X() = nMaxY;

            aPnt2.X() = aPnt1.X() + pFrm->Prt().Width();
            aPnt2.Y() = aPnt1.Y();
            if( aPnt2.X() < nMaxY )
                aPnt2.X() = nMaxY;
        }
        else
        {
            if( aPnt1.Y() > nMaxY )
                aPnt1.Y() = nMaxY;
            aPnt2.X() = aPnt1.X();
            aPnt2.Y() = aPnt1.Y() + pFrm->Prt().Height();
            if( aPnt2.Y() > nMaxY )
                aPnt2.Y() = nMaxY;
        }
        rOrig = SwRect( aPnt1, aPnt2 );
        return true;
    }
    else
    {
        if( !pFrm->HasPara() )
            return false;

        SwFrmSwapper aSwapper( pFrm, true );
        if ( bVert )
            nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );

        SwTextSizeInfo aInf( pFrm );
        SwTextCursor aLine( pFrm, &aInf );
        SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
        aTmpState.m_bRealHeight = true;
        if( aLine.GetCharRect( &rOrig, nOffset, &aTmpState, nMaxY ) )
        {
            if( aTmpState.m_aRealHeight.X() >= 0 )
            {
                rOrig.Pos().Y() += aTmpState.m_aRealHeight.X();
                rOrig.Height( aTmpState.m_aRealHeight.Y() );
            }

            if ( pFrm->IsRightToLeft() )
                pFrm->SwitchLTRtoRTL( rOrig );

            if ( bVert )
                pFrm->SwitchHorizontalToVertical( rOrig );

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
bool SwTextFrm::GetTopOfLine( SwTwips& _onTopOfLine,
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
        SWRECTFN( this )
        if ( IsEmpty() || !(Prt().*fnRect->fnGetHeight)() )
        {
            // consider upper space amount considered
            // for previous frame and the page grid.
            _onTopOfLine = (this->*fnRect->fnGetPrtTop)();
        }
        else
        {
            // determine formatted text frame that contains the requested position
            SwTextFrm* pFrm = &(const_cast<SwTextFrm*>(this)->GetFrmAtOfst( nOffset ));
            pFrm->GetFormatted();
            SWREFRESHFN( pFrm )
            // If proportional line spacing is applied
            // to the text frame, the top of the anchor character is also the
            // top of the line.
            // Otherwise the line layout determines the top of the line
            const SvxLineSpacingItem& rSpace = GetAttrSet()->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                SwRect aCharRect;
                if ( GetAutoPos( aCharRect, _rPos ) )
                {
                    _onTopOfLine = (aCharRect.*fnRect->fnGetTop)();
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                // assure that text frame is in a horizontal layout
                SwFrmSwapper aSwapper( pFrm, true );
                // determine text line that contains the requested position
                SwTextSizeInfo aInf( pFrm );
                SwTextCursor aLine( pFrm, &aInf );
                aLine.CharCrsrToLine( nOffset );
                // determine top of line
                _onTopOfLine = aLine.Y();
                if ( bVert )
                {
                    _onTopOfLine = pFrm->SwitchHorizontalToVertical( _onTopOfLine );
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
    SwRect aFrm;
    const SwCrsrMoveState *pCMS;
    SwPosition* pPos;
    const Point& rPoint;
    SwTwips nLineWidth;
    bool bFirstLine : 1;
    bool bInner     : 1;
    bool bColumn    : 1;
    bool bEmpty     : 1;
    SwFillData( const SwCrsrMoveState *pC, SwPosition* pP, const SwRect& rR,
        const Point& rPt ) : aFrm( rR ), pCMS( pC ), pPos( pP ), rPoint( rPt ),
        nLineWidth( 0 ), bFirstLine( true ), bInner( false ), bColumn( false ),
        bEmpty( true ){}
    SwFillMode Mode() const { return pCMS->m_pFill->eMode; }
    long X() const { return rPoint.X(); }
    long Y() const { return rPoint.Y(); }
    long Left() const { return aFrm.Left(); }
    long Right() const { return aFrm.Right(); }
    long Bottom() const { return aFrm.Bottom(); }
    SwFillCrsrPos &Fill() const { return *pCMS->m_pFill; }
    void SetTab( sal_uInt16 nNew ) { pCMS->m_pFill->nTabCnt = nNew; }
    void SetSpace( sal_uInt16 nNew ) { pCMS->m_pFill->nSpaceCnt = nNew; }
    void SetOrient( const sal_Int16 eNew ){ pCMS->m_pFill->eOrient = eNew; }
};

bool SwTextFrm::_GetCrsrOfst(SwPosition* pPos, const Point& rPoint,
                    const bool bChgFrm, SwCrsrMoveState* pCMS ) const
{
    // _GetCrsrOfst is called by GetCrsrOfst and GetKeyCrsrOfst.
    // Never just a return false.

    if( IsLocked() || IsHiddenNow() )
        return false;

    const_cast<SwTextFrm*>(this)->GetFormatted();

    Point aOldPoint( rPoint );

    if ( IsVertical() )
    {
        SwitchVerticalToHorizontal( (Point&)rPoint );
        const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() )
        SwitchRTLtoLTR( (Point&)rPoint );

    SwFillData *pFillData = ( pCMS && pCMS->m_pFill ) ?
                        new SwFillData( pCMS, pPos, Frm(), rPoint ) : nullptr;

    if ( IsEmpty() )
    {
        SwTextNode* pTextNd = const_cast<SwTextFrm*>(this)->GetTextNode();
        pPos->nNode = *pTextNd;
        pPos->nContent.Assign( pTextNd, 0 );
        if( pCMS && pCMS->m_bFieldInfo )
        {
            SwTwips nDiff = rPoint.X() - Frm().Left() - Prt().Left();
            if( nDiff > 50 || nDiff < 0 )
                pCMS->m_bPosCorr = true;
        }
    }
    else
    {
        SwTextSizeInfo aInf( const_cast<SwTextFrm*>(this) );
        SwTextCursor  aLine( const_cast<SwTextFrm*>(this), &aInf );

        // See comment in AdjustFrm()
        SwTwips nMaxY = Frm().Top() + Prt().Top() + Prt().Height();
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

        sal_Int32 nOffset = aLine.GetCrsrOfst( pPos, rPoint, bChgFrm, pCMS );

        if( pCMS && pCMS->m_eState == MV_NONE && aLine.GetEnd() == nOffset )
            pCMS->m_eState = MV_RIGHTMARGIN;

    // pPos is a pure IN parameter and must not be evaluated.
    // pIter->GetCrsrOfst returns from a nesting with COMPLETE_STRING.
    // If SwTextIter::GetCrsrOfst calls GetCrsrOfst further by itself
    // nNode changes the position.
    // In such cases, pPos must not be calculated.
        if( COMPLETE_STRING != nOffset )
        {
            SwTextNode* pTextNd = const_cast<SwTextFrm*>(this)->GetTextNode();
            pPos->nNode = *pTextNd;
            pPos->nContent.Assign( pTextNd, nOffset );
            if( pFillData )
            {
                if (pTextNd->GetText().getLength() > nOffset ||
                    rPoint.Y() < Frm().Top() )
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
    if( pFillData && FindPageFrm()->Frm().IsInside( aOldPoint ) )
    {
        FillCrsrPos( *pFillData );
        bChgFillData = true;
    }

    if ( IsVertical() )
    {
        if ( bChgFillData )
            SwitchHorizontalToVertical( pFillData->Fill().aCrsr.Pos() );
        const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() && bChgFillData )
    {
            SwitchLTRtoRTL( pFillData->Fill().aCrsr.Pos() );
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

bool SwTextFrm::GetCrsrOfst(SwPosition* pPos, Point& rPoint,
                               SwCrsrMoveState* pCMS, bool ) const
{
    const bool bChgFrm = !(pCMS && MV_UPDOWN == pCMS->m_eState);
    return _GetCrsrOfst( pPos, rPoint, bChgFrm, pCMS );
}

/*
 * Layout-oriented cursor movement to the line start.
 */

bool SwTextFrm::LeftMargin(SwPaM *pPam) const
{
    if( &pPam->GetNode() != GetNode() )
        pPam->GetPoint()->nNode = *const_cast<SwTextFrm*>(this)->GetTextNode();

    SwTextFrm *pFrm = GetAdjFrmAtPos( const_cast<SwTextFrm*>(this), *pPam->GetPoint(),
                                     SwTextCursor::IsRightMargin() );
    pFrm->GetFormatted();
    sal_Int32 nIndx;
    if ( pFrm->IsEmpty() )
        nIndx = 0;
    else
    {
        SwTextSizeInfo aInf( pFrm );
        SwTextCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nIndx = aLine.GetStart();
        if( pFrm->GetOfst() && !pFrm->IsFollow() && !aLine.GetPrev() )
        {
            sw_ChangeOffset( pFrm, 0 );
            nIndx = 0;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTextNode(), nIndx );
    SwTextCursor::SetRightMargin( false );
    return true;
}

/*
 * To the line end: That's the position before the last char of the line.
 * Exception: In the last line, it should be able to place the cursor after
 * the last char in order to append text.
 */

bool SwTextFrm::RightMargin(SwPaM *pPam, bool bAPI) const
{
    if( &pPam->GetNode() != GetNode() )
        pPam->GetPoint()->nNode = *const_cast<SwTextFrm*>(this)->GetTextNode();

    SwTextFrm *pFrm = GetAdjFrmAtPos( const_cast<SwTextFrm*>(this), *pPam->GetPoint(),
                                     SwTextCursor::IsRightMargin() );
    pFrm->GetFormatted();
    sal_Int32 nRightMargin;
    if ( IsEmpty() )
        nRightMargin = 0;
    else
    {
        SwTextSizeInfo aInf( pFrm );
        SwTextCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nRightMargin = aLine.GetStart() + aLine.GetCurr()->GetLen();

        // We skip hard line brakes
        if( aLine.GetCurr()->GetLen() &&
            CH_BREAK == aInf.GetText()[nRightMargin - 1])
            --nRightMargin;
        else if( !bAPI && (aLine.GetNext() || pFrm->GetFollow()) )
        {
            while( nRightMargin > aLine.GetStart() &&
                ' ' == aInf.GetText()[nRightMargin - 1])
                --nRightMargin;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTextNode(), nRightMargin );
    SwTextCursor::SetRightMargin( !bAPI );
    return true;
}

// The following two methods try to put the Crsr into the next/succsessive
// line. If we do not have a preceding/successive line we forward the call
// to the base class.
// The Crsr's horizontal justification is done afterwards by the CrsrShell.

class SwSetToRightMargin
{
    bool bRight;
public:
    inline SwSetToRightMargin() : bRight( false ) { }
    inline ~SwSetToRightMargin() { SwTextCursor::SetRightMargin( bRight ); }
    inline void SetRight( const bool bNew ) { bRight = bNew; }
};

bool SwTextFrm::_UnitUp( SwPaM *pPam, const SwTwips nOffset,
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
        return SwContentFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
    }

    const_cast<SwTextFrm*>(this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;

    if( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() )
                sw_ChangeOffset( const_cast<SwTextFrm*>(this), nFormat );

            SwTextSizeInfo aInf( const_cast<SwTextFrm*>(this) );
            SwTextCursor  aLine( const_cast<SwTextFrm*>(this), &aInf );

            // Optimize away flys with no flow and IsDummy()
            if( nPos )
                aLine.CharCrsrToLine( nPos );
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

                // See comment in SwTextFrm::GetCrsrOfst()
#if OSL_DEBUG_LEVEL > 0
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                // The node should not be changed
                sal_Int32 nTmpOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                                         aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                        "SwTextFrm::UnitUp: illegal node change" );
#endif

                // We make sure that we move up.
                if( nTmpOfst >= nStart && nStart && !bSecondOfDouble )
                {
                    nTmpOfst = nStart;
                    aSet.SetRight( true );
                }
                pPam->GetPoint()->nContent =
                      SwIndex( const_cast<SwTextFrm*>(this)->GetTextNode(), nTmpOfst );
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
        const SwTextFrm *pTmpPrev = FindMaster();
        sal_Int32 nOffs = GetOfst();
        if( pTmpPrev )
        {
            SwViewShell *pSh = getRootFrm()->GetCurrShell();
            const bool bProtectedAllowed = pSh && pSh->GetViewOptions()->IsCursorInProtectedArea();
            const SwTextFrm *pPrevPrev = pTmpPrev;
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
                return pTmpPrev->SwContentFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
            aCharBox.Pos().Y() = pPrevPrev->Frm().Bottom() - 1;
            return pPrevPrev->GetKeyCrsrOfst( pPam->GetPoint(), aCharBox.Pos() );
        }
    }
    return SwContentFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
}

// Used for Bidi. nPos is the logical position in the string, bLeft indicates
// if left arrow or right arrow was pressed. The return values are:
// nPos: the new visual position
// bLeft: whether the break iterator has to add or subtract from the
//        current position
static void lcl_VisualMoveRecursion( const SwLineLayout& rCurrLine, sal_Int32 nIdx,
                              sal_Int32& nPos, bool& bRight,
                              sal_uInt8& nCrsrLevel, sal_uInt8 nDefaultDir )
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
            if ( nCrsrLevel != nDefaultDir )
            {
                bRecurse = false;
            }
            else
                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is between c and X in the buffer and cursor level = 0
                nCrsrLevel++;
        }

        // 2. special case: at beginning of portion after bidi portion
        else if ( pLast && pLast->IsMultiPortion() &&
                 static_cast<const SwMultiPortion*>(pLast)->IsBidi() && nIdx == nPos )
        {
            // enter bidi portion
            if ( nCrsrLevel != nDefaultDir )
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
            sal_uInt8 nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCrsrLevel = nTmpCrsrLevel;
        }

        // go forward
        else
        {
            bRight = true;
            nCrsrLevel = nDefaultDir;
        }

    }
    else
    {
        bool bRecurse = pPor && pPor->IsMultiPortion() && static_cast<const SwMultiPortion*>(pPor)->IsBidi();

        // 1. special case: at beginning of bidi portion
        if ( bRecurse && nIdx == nPos )
        {
            // leave bidi portion
            if ( nCrsrLevel == nDefaultDir )
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
            if ( nCrsrLevel % 2 == nDefaultDir % 2 )
            {
                bRecurse = true;
                nIdx = nIdx - pLast->GetLen();
                pPor = pLast;

                // special case:
                // buffer: abcXYZ123 in LTR paragraph
                // view:   abc123ZYX
                // cursor is behind 3 in the buffer and cursor level = 2
                if ( nDefaultDir + 2 == nCrsrLevel )
                    nPos = nPos + pLast->GetLen();
            }
        }

        // go forward
        if ( bRecurse )
        {
            const SwLineLayout& rLine = static_cast<const SwMultiPortion*>(pPor)->GetRoot();
            sal_Int32 nTmpPos = nPos - nIdx;
            bool bTmpForward = ! bRight;
            sal_uInt8 nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            // special case:
            // buffer: abcXYZ123 in LTR paragraph
            // view:   abc123ZYX
            // cursor is between Z and 1 in the buffer and cursor level = 2
            if ( nTmpPos == pPor->GetLen() && nTmpCrsrLevel == nDefaultDir + 1 )
            {
                nTmpPos = nTmpPos - pPor->GetLen();
                nTmpCrsrLevel = nDefaultDir;
                bTmpForward = ! bTmpForward;
            }

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCrsrLevel = nTmpCrsrLevel;
        }

        // go backward
        else
        {
            bRight = false;
            nCrsrLevel = nDefaultDir;
        }
    }
}

void SwTextFrm::PrepareVisualMove( sal_Int32& nPos, sal_uInt8& nCrsrLevel,
                                  bool& bForward, bool bInsertCrsr )
{
    if( IsEmpty() || IsHiddenNow() )
        return;

    GetFormatted();

    SwTextSizeInfo aInf(this);
    SwTextCursor  aLine(this, &aInf);

    if( nPos )
        aLine.CharCrsrToLine( nPos );
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
    if ( bInsertCrsr )
    {
        lcl_VisualMoveRecursion( *pLine, nStt, nPos, bForward,
                                 nCrsrLevel, IsRightToLeft() ? 1 : 0 );
        return;
    }

    const sal_uInt8 nDefaultDir = static_cast<sal_uInt8>(IsRightToLeft() ? UBIDI_RTL : UBIDI_LTR);
    const bool bVisualRight = ( nDefaultDir == UBIDI_LTR && bForward ) ||
                                  ( nDefaultDir == UBIDI_RTL && ! bForward );

    // Bidi functions from icu 2.0

    const sal_Unicode* pLineString = GetTextNode()->GetText().getStr();

    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( nLen, 0, &nError );
    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), nLen, nDefaultDir, nullptr, &nError ); // UChar != sal_Unicode in MinGW

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

bool SwTextFrm::_UnitDown(SwPaM *pPam, const SwTwips nOffset,
                         bool bSetInReadOnly ) const
{

    if ( IsInTab() &&
        pPam->GetNode().StartOfSectionNode() !=
        pPam->GetNode( false ).StartOfSectionNode() )
    {
        // If the PaM is located within different boxes, we have a table selection,
        // which is handled by the base class.
        return SwContentFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
    }
    const_cast<SwTextFrm*>(this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;
    const SwContentFrm *pTmpFollow = nullptr;

    if ( IsVertical() )
        const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();

    if ( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() &&
                !sw_ChangeOffset( const_cast<SwTextFrm*>(this), nFormat ) )
                break;

            SwTextSizeInfo aInf( const_cast<SwTextFrm*>(this) );
            SwTextCursor  aLine( const_cast<SwTextFrm*>(this), &aInf );
            nFormat = aLine.GetEnd();

            aLine.CharCrsrToLine( nPos );

            const SwLineLayout* pNextLine = aLine.GetNextLine();
            const sal_Int32 nStart = aLine.GetStart();
            aLine.GetCharRect( &aCharBox, nPos );

            bool bFirstOfDouble = ( aInf.IsMulti() && aInf.IsFirstMulti() );

            if( pNextLine || bFirstOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
#if OSL_DEBUG_LEVEL > 0
                // See comment in SwTextFrm::GetCrsrOfst()
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                if ( pNextLine && ! bFirstOfDouble )
                    aLine.NextLine();

                sal_Int32 nTmpOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                 aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                    "SwTextFrm::UnitDown: illegal node change" );
#endif

                // We make sure that we move down.
                if( nTmpOfst <= nStart && ! bFirstOfDouble )
                    nTmpOfst = nStart + 1;
                pPam->GetPoint()->nContent =
                      SwIndex( const_cast<SwTextFrm*>(this)->GetTextNode(), nTmpOfst );

                if ( IsVertical() )
                    const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();

                return true;
            }
            if( nullptr != ( pTmpFollow = GetFollow() ) )
            {   // Skip protected follows
                const SwContentFrm* pTmp = pTmpFollow;
                SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
                        const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();
                    return pTmp->SwContentFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
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
        const_cast<SwTextFrm*>(this)->SwapWidthAndHeight();

    // We take a shortcut for follows
    if( pTmpFollow )
    {
        aCharBox.Pos().Y() = pTmpFollow->Frm().Top() + 1;
        return static_cast<const SwTextFrm*>(pTmpFollow)->GetKeyCrsrOfst( pPam->GetPoint(),
                                                     aCharBox.Pos() );
    }
    return SwContentFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
}

bool SwTextFrm::UnitUp(SwPaM *pPam, const SwTwips nOffset,
                      bool bSetInReadOnly ) const
{
    /* We call ContentNode::GertFrm() in CrsrSh::Up().
     * This _always returns the master.
     * In order to not mess with cursor travelling, we correct here
     * in SwTextFrm.
     * We calculate UnitUp for pFrm. pFrm is either a master (= this) or a
     * follow (!= this).
     */
    const SwTextFrm *pFrm = GetAdjFrmAtPos( const_cast<SwTextFrm*>(this), *(pPam->GetPoint()),
                                           SwTextCursor::IsRightMargin() );
    const bool bRet = pFrm->_UnitUp( pPam, nOffset, bSetInReadOnly );

    // No SwTextCursor::SetRightMargin( false );
    // Instead we have a SwSetToRightMargin in _UnitUp
    return bRet;
}

bool SwTextFrm::UnitDown(SwPaM *pPam, const SwTwips nOffset,
                        bool bSetInReadOnly ) const
{
    const SwTextFrm *pFrm = GetAdjFrmAtPos(const_cast<SwTextFrm*>(this), *(pPam->GetPoint()),
                                           SwTextCursor::IsRightMargin() );
    const bool bRet = pFrm->_UnitDown( pPam, nOffset, bSetInReadOnly );
    SwTextCursor::SetRightMargin( false );
    return bRet;
}

void SwTextFrm::FillCrsrPos( SwFillData& rFill ) const
{
    if( !rFill.bColumn && GetUpper()->IsColBodyFrm() ) // ColumnFrms now with BodyFrm
    {
        const SwColumnFrm* pTmp =
            static_cast<const SwColumnFrm*>(GetUpper()->GetUpper()->GetUpper()->Lower()); // The 1st column
        // The first SwFrm in BodyFrm of the first column
        const SwFrm* pFrm = static_cast<const SwLayoutFrm*>(pTmp->Lower())->Lower();
        sal_uInt16 nNextCol = 0;
        // In which column do we end up in?
        while( rFill.X() > pTmp->Frm().Right() && pTmp->GetNext() )
        {
            pTmp = static_cast<const SwColumnFrm*>(pTmp->GetNext());
            if( static_cast<const SwLayoutFrm*>(pTmp->Lower())->Lower() ) // ColumnFrms now with BodyFrm
            {
                pFrm = static_cast<const SwLayoutFrm*>(pTmp->Lower())->Lower();
                nNextCol = 0;
            }
            else
                ++nNextCol; // Empty columns require column brakes
        }
        if( pTmp != GetUpper()->GetUpper() ) // Did we end up in another column?
        {
            if( !pFrm )
                return;
            if( nNextCol )
            {
                while( pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
            }
            else
            {
                while( pFrm->GetNext() && pFrm->Frm().Bottom() < rFill.Y() )
                    pFrm = pFrm->GetNext();
            }
            // No filling, if the last frame in the targeted column does
            // not contain a paragraph, but e.g. a table
            if( pFrm->IsTextFrm() )
            {
                rFill.Fill().nColumnCnt = nNextCol;
                rFill.bColumn = true;
                if( rFill.pPos )
                {
                    SwTextNode* pTextNd = const_cast<SwTextFrm*>(static_cast<const SwTextFrm*>(pFrm))->GetTextNode();
                    rFill.pPos->nNode = *pTextNd;
                    rFill.pPos->nContent.Assign(
                            pTextNd, pTextNd->GetText().getLength());
                }
                if( nNextCol )
                {
                    rFill.aFrm = pTmp->Prt();
                    rFill.aFrm += pTmp->Frm().Pos();
                }
                else
                    rFill.aFrm = pFrm->Frm();
                static_cast<const SwTextFrm*>(pFrm)->FillCrsrPos( rFill );
            }
            return;
        }
    }
    SwFont *pFnt;
    SwTextFormatColl* pColl = GetTextNode()->GetTextColl();
    SwTwips nFirst = GetTextNode()->GetSwAttrSet().GetULSpace().GetLower();
    SwTwips nDiff = rFill.Y() - Frm().Bottom();
    if( nDiff < nFirst )
        nDiff = -1;
    else
        pColl = &pColl->GetNextTextFormatColl();
    SwAttrSet aSet( const_cast<SwDoc*>(GetTextNode()->GetDoc())->GetAttrPool(), aTextFormatCollSetRange );
    const SwAttrSet* pSet = &pColl->GetAttrSet();
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
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
            rFill.aFrm.Height( nLineHeight );
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

            SwRect &rRect = rFill.Fill().aCrsr;
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
                    nSpace = pFnt->_GetTextSize( aDrawInf ).Width()/2;
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
                    SwTwips nLeftTab = nLeft;
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
            const SwFrm* pUp = GetUpper();
            if( pUp->IsInSct() )
            {
                if( pUp->IsSctFrm() )
                    pUp = pUp->GetUpper();
                else if( pUp->IsColBodyFrm() &&
                         pUp->GetUpper()->GetUpper()->IsSctFrm() )
                    pUp = pUp->GetUpper()->GetUpper()->GetUpper();
            }
            SWRECTFN( this )
            SwTwips nLimit = (pUp->*fnRect->fnGetPrtBottom)();
            SwTwips nRectBottom = rRect.Bottom();
            if ( bVert )
                nRectBottom = SwitchHorizontalToVertical( nRectBottom );

            if( (*fnRect->fnYDiff)( nLimit, nRectBottom ) < 0 )
                bFill = false;
            else
                rRect.Width( 1 );
        }
    }
    const_cast<SwCrsrMoveState*>(rFill.pCMS)->m_bFillRet = bFill;
    delete pFnt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
