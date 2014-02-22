/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ndtxt.hxx"
#include "pam.hxx"
#include "frmtool.hxx"
#include "viewopt.hxx"
#include "paratr.hxx"
#include "rootfrm.hxx"
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "txttypes.hxx"
#include <sfx2/printer.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <pormulti.hxx>
#include <doc.hxx>
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
 * - GetEndCharRect sets bRightMargin to sal_True
 * - SwTxtCursor::bRightMargin is set to sal_False by CharCrsrToLine
 */

/*************************************************************************
 *                      GetAdjFrmAtPos()
 *************************************************************************/

SwTxtFrm *GetAdjFrmAtPos( SwTxtFrm *pFrm, const SwPosition &rPos,
                          const sal_Bool bRightMargin, const sal_Bool bNoScroll = sal_True )
{
    
    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTxtFrm *pFrmAtPos = pFrm;
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
        pFrmAtPos = (SwTxtFrm*)pFrm->GetFrmAtPos( rPos );
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

bool sw_ChangeOffset( SwTxtFrm* pFrm, sal_Int32 nNew )
{
    
    OSL_ENSURE( !pFrm->IsFollow(), "Illegal Scrolling by Follow!" );
    if( pFrm->GetOfst() != nNew && !pFrm->IsInSct() )
    {
        SwFlyFrm *pFly = pFrm->FindFlyFrm();
        
        
        if ( ( pFly && pFly->IsValid() &&
             !pFly->GetNextLink() && !pFly->GetPrevLink() ) ||
             ( !pFly && pFrm->IsInTab() ) )
        {
            SwViewShell* pVsh = pFrm->getRootFrm()->GetCurrShell();
            if( pVsh )
            {
                if( pVsh->GetNext() != pVsh ||
                    ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() ) )
                {
                    if( !pFrm->GetOfst() )
                        return false;
                    nNew = 0;
                }
                pFrm->SetOfst( nNew );
                pFrm->SetPara( 0 );
                pFrm->GetFormatted();
                if( pFrm->Frm().HasArea() )
                    pFrm->getRootFrm()->GetCurrShell()->InvalidateWindows( pFrm->Frm() );
                return true;
            }
        }
    }
    return false;
}

/*************************************************************************
 *                      GetFrmAtOfst(), GetFrmAtPos()
 *************************************************************************/

SwTxtFrm& SwTxtFrm::GetFrmAtOfst( const sal_Int32 nWhere )
{
    SwTxtFrm* pRet = this;
    while( pRet->HasFollow() && nWhere >= pRet->GetFollow()->GetOfst() )
        pRet = pRet->GetFollow();
    return *pRet;
}

SwTxtFrm *SwTxtFrm::GetFrmAtPos( const SwPosition &rPos )
{
    SwTxtFrm *pFoll = (SwTxtFrm*)this;
    while( pFoll->GetFollow() )
    {
        if( rPos.nContent.GetIndex() > pFoll->GetFollow()->GetOfst() )
            pFoll = pFoll->GetFollow();
        else
        {
            if( rPos.nContent.GetIndex() == pFoll->GetFollow()->GetOfst()
                 && !SwTxtCursor::IsRightMargin() )
                 pFoll = pFoll->GetFollow();
            else
                break;
        }
    }
    return pFoll;
}

/*************************************************************************
 *                      SwTxtFrm::GetCharRect()
 *************************************************************************/

/*
 * GetCharRect() returns the char's char line described by aPos.
 * GetCrsrOfst() does the reverse: It goes from a document coordinate to
 * a Pam.
 * Both are virtual in the frame base class and thus are redefined here.
 */

sal_Bool SwTxtFrm::GetCharRect( SwRect& rOrig, const SwPosition &rPos,
                            SwCrsrMoveState *pCMS ) const
{
    OSL_ENSURE( ! IsVertical() || ! IsSwapped(),"SwTxtFrm::GetCharRect with swapped frame" );

    if( IsLocked() || IsHiddenNow() )
        return sal_False;

    
    
    
    
    

    
    const sal_Bool bRightMargin = pCMS && ( MV_RIGHTMARGIN == pCMS->eState );
    const sal_Bool bNoScroll = pCMS && pCMS->bNoScroll;
    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, rPos, bRightMargin,
                                     bNoScroll );
    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();

    SWRECTFN ( pFrm )
    const SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();
    const SwTwips nFrmMaxY = (pFrm->*fnRect->fnGetPrtBottom)();

    
    
    SwTwips nMaxY = bVert ?
                    ( bVertL2R ? std::min( nFrmMaxY, nUpperMaxY ) : std::max( nFrmMaxY, nUpperMaxY ) ) :
                    std::min( nFrmMaxY, nUpperMaxY );

    sal_Bool bRet = sal_False;

    if ( pFrm->IsEmpty() || ! (pFrm->Prt().*fnRect->fnGetHeight)() )
    {
        Point aPnt1 = pFrm->Frm().Pos() + pFrm->Prt().Pos();
        SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
        short nFirstOffset;
        pTxtNd->GetFirstLineOfsWithNum( nFirstOffset );

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
            pCMS->aRealHeight.X() = 0;
            pCMS->aRealHeight.Y() = bVert ? -rOrig.Width() : rOrig.Height();
        }

        if ( pFrm->IsRightToLeft() )
            pFrm->SwitchLTRtoRTL( rOrig );

        bRet = sal_True;
    }
    else
    {
        if( !pFrm->HasPara() )
            return sal_False;

        SwFrmSwapper aSwapper( pFrm, true );
        if ( bVert )
            nMaxY = pFrm->SwitchVerticalToHorizontal( nMaxY );

        bool bGoOn = true;
        const sal_Int32 nOffset = rPos.nContent.GetIndex();
        sal_Int32 nNextOfst;

        do
        {
            {
                SwTxtSizeInfo aInf( pFrm );
                SwTxtCursor  aLine( pFrm, &aInf );
                nNextOfst = aLine.GetEnd();
                
                
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
                pFrm->GetTxtNode()->GetTxt().getLength() != nNextOfst)
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
                if( pCMS->b2Lines && pCMS->p2Lines)
                {
                    pFrm->SwitchLTRtoRTL( pCMS->p2Lines->aLine );
                    pFrm->SwitchLTRtoRTL( pCMS->p2Lines->aPortion );
                }
            }

            if ( bVert )
            {
                if ( pCMS->bRealHeight )
                {
                    pCMS->aRealHeight.Y() = -pCMS->aRealHeight.Y();
                    if ( pCMS->aRealHeight.Y() < 0 )
                    {
                        
                        pCMS->aRealHeight.X() =  ( rOrig.Width() -
                                                    pCMS->aRealHeight.X() +
                                                    pCMS->aRealHeight.Y() );
                    }
                }
                if( pCMS->b2Lines && pCMS->p2Lines)
                {
                    pFrm->SwitchHorizontalToVertical( pCMS->p2Lines->aLine );
                    pFrm->SwitchHorizontalToVertical( pCMS->p2Lines->aPortion );
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

        
        
        
        
        if( (*fnRect->fnYDiff)( nPageTop, nOrigTop ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageTop );

        if ( (*fnRect->fnYDiff)( nOrigTop, nPageBott ) > 0 )
            (rOrig.*fnRect->fnSetTop)( nPageBott );
    }

    return bRet;
}

/*************************************************************************
 *                      SwTxtFrm::GetAutoPos()
 *************************************************************************/

/*
 * GetAutoPos() looks up the char's char line which is described by rPos
 * and is used by the auto-positioned frame.
 */

bool SwTxtFrm::GetAutoPos( SwRect& rOrig, const SwPosition &rPos ) const
{
    if( IsHiddenNow() )
        return false;

    const sal_Int32 nOffset = rPos.nContent.GetIndex();
    SwTxtFrm* pFrm = &(const_cast<SwTxtFrm*>(this)->GetFrmAtOfst( nOffset ));

    pFrm->GetFormatted();
    const SwFrm* pTmpFrm = (SwFrm*)pFrm->GetUpper();

    SWRECTFN( pTmpFrm )
    SwTwips nUpperMaxY = (pTmpFrm->*fnRect->fnGetPrtBottom)();

    
    
    SwTwips nMaxY = bVert ?
                    ( bVertL2R ? std::min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY ) : std::max( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY ) ) :
                    std::min( (pFrm->*fnRect->fnGetPrtBottom)(), nUpperMaxY );

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

        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor aLine( pFrm, &aInf );
        SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
        aTmpState.bRealHeight = sal_True;
        if( aLine.GetCharRect( &rOrig, nOffset, &aTmpState, nMaxY ) )
        {
            if( aTmpState.aRealHeight.X() >= 0 )
            {
                rOrig.Pos().Y() += aTmpState.aRealHeight.X();
                rOrig.Height( aTmpState.aRealHeight.Y() );
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
bool SwTxtFrm::GetTopOfLine( SwTwips& _onTopOfLine,
                             const SwPosition& _rPos ) const
{
    bool bRet = true;

    
    const sal_Int32 nOffset = _rPos.nContent.GetIndex();

    if ( GetTxt().getLength() < nOffset )
    {
        bRet = false;
    }
    else
    {
        SWRECTFN( this )
        if ( IsEmpty() || !(Prt().*fnRect->fnGetHeight)() )
        {
            
            
            _onTopOfLine = (this->*fnRect->fnGetPrtTop)();
        }
        else
        {
            
            SwTxtFrm* pFrm = &(const_cast<SwTxtFrm*>(this)->GetFrmAtOfst( nOffset ));
            pFrm->GetFormatted();
            SWREFRESHFN( pFrm )
            
            
            
            
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
                
                SwFrmSwapper aSwapper( pFrm, true );
                
                SwTxtSizeInfo aInf( pFrm );
                SwTxtCursor aLine( pFrm, &aInf );
                aLine.CharCrsrToLine( nOffset );
                
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

/*************************************************************************
 *                      SwTxtFrm::_GetCrsrOfst()
 *************************************************************************/


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
    void SetOrient( const sal_Int16 eNew ){ pCMS->pFill->eOrient = eNew; }
};

bool SwTxtFrm::_GetCrsrOfst(SwPosition* pPos, const Point& rPoint,
                    const bool bChgFrm, SwCrsrMoveState* pCMS ) const
{
    
    

    if( IsLocked() || IsHiddenNow() )
        return false;

    ((SwTxtFrm*)this)->GetFormatted();

    Point aOldPoint( rPoint );

    if ( IsVertical() )
    {
        SwitchVerticalToHorizontal( (Point&)rPoint );
        ((SwTxtFrm*)this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() )
        SwitchRTLtoLTR( (Point&)rPoint );

    SwFillData *pFillData = ( pCMS && pCMS->pFill ) ?
                        new SwFillData( pCMS, pPos, Frm(), rPoint ) : NULL;

    if ( IsEmpty() )
    {
        SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
        pPos->nNode = *pTxtNd;
        pPos->nContent.Assign( pTxtNd, 0 );
        if( pCMS && pCMS->bFieldInfo )
        {
            SwTwips nDiff = rPoint.X() - Frm().Left() - Prt().Left();
            if( nDiff > 50 || nDiff < 0 )
                ((SwCrsrMoveState*)pCMS)->bPosCorr = true;
        }
    }
    else
    {
        SwTxtSizeInfo aInf( (SwTxtFrm*)this );
        SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

        
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

        if( pCMS && pCMS->eState == MV_NONE && aLine.GetEnd() == nOffset )
            ((SwCrsrMoveState*)pCMS)->eState = MV_RIGHTMARGIN;

    
    
    
    
    
        if( COMPLETE_STRING != nOffset )
        {
            SwTxtNode* pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
            pPos->nNode = *pTxtNd;
            pPos->nContent.Assign( pTxtNd, nOffset );
            if( pFillData )
            {
                if (pTxtNd->GetTxt().getLength() > nOffset ||
                    rPoint.Y() < Frm().Top() )
                    pFillData->bInner = true;
                pFillData->bFirstLine = aLine.GetLineNr() < 2;
                if (pTxtNd->GetTxt().getLength())
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
        ((SwTxtFrm*)this)->SwapWidthAndHeight();
    }

    if ( IsRightToLeft() && bChgFillData )
    {
            SwitchLTRtoRTL( pFillData->Fill().aCrsr.Pos() );
            const sal_Int16 eOrient = pFillData->pCMS->pFill->eOrient;

            if ( text::HoriOrientation::LEFT == eOrient )
                pFillData->SetOrient( text::HoriOrientation::RIGHT );
            else if ( text::HoriOrientation::RIGHT == eOrient )
                pFillData->SetOrient( text::HoriOrientation::LEFT );
    }

    (Point&)rPoint = aOldPoint;
    delete pFillData;

    return true;
}

/*************************************************************************
 *                 virtual SwTxtFrm::GetCrsrOfst()
 *************************************************************************/

sal_Bool SwTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& rPoint,
                               SwCrsrMoveState* pCMS, bool ) const
{
    MSHORT nChgFrm = 2;
    if( pCMS )
    {
        if( MV_UPDOWN == pCMS->eState )
            nChgFrm = 0;
        else if( MV_SETONLYTEXT == pCMS->eState ||
                 MV_TBLSEL == pCMS->eState )
            nChgFrm = 1;
    }
    return _GetCrsrOfst( pPos, rPoint, nChgFrm != 0, pCMS );
}

/*************************************************************************
 *                      SwTxtFrm::LeftMargin()
 *************************************************************************/

/*
 * Layout-oriented cursor movement to the line start.
 */

sal_Bool SwTxtFrm::LeftMargin(SwPaM *pPam) const
{
    if( ((const SwNode*)pPam->GetNode()) != GetNode() )
        pPam->GetPoint()->nNode = *((SwTxtFrm*)this)->GetTxtNode();

    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *pPam->GetPoint(),
                                     SwTxtCursor::IsRightMargin() );
    pFrm->GetFormatted();
    sal_Int32 nIndx;
    if ( pFrm->IsEmpty() )
        nIndx = 0;
    else
    {
        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nIndx = aLine.GetStart();
        if( pFrm->GetOfst() && !pFrm->IsFollow() && !aLine.GetPrev() )
        {
            sw_ChangeOffset( pFrm, 0 );
            nIndx = 0;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTxtNode(), nIndx );
    SwTxtCursor::SetRightMargin( false );
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::RightMargin()
 *************************************************************************/

/*
 * To the line end: That's the position before the last char of the line.
 * Exception: In the last line, it should be able to place the cursor after
 * the last char in order to append text.
 */

sal_Bool SwTxtFrm::RightMargin(SwPaM *pPam, sal_Bool bAPI) const
{
    if( ((const SwNode*)pPam->GetNode()) != GetNode() )
        pPam->GetPoint()->nNode = *((SwTxtFrm*)this)->GetTxtNode();

    SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *pPam->GetPoint(),
                                     SwTxtCursor::IsRightMargin() );
    pFrm->GetFormatted();
    sal_Int32 nRightMargin;
    if ( IsEmpty() )
        nRightMargin = 0;
    else
    {
        SwTxtSizeInfo aInf( pFrm );
        SwTxtCursor  aLine( pFrm, &aInf );

        aLine.CharCrsrToLine(pPam->GetPoint()->nContent.GetIndex());
        nRightMargin = aLine.GetStart() + aLine.GetCurr()->GetLen();

        
        if( aLine.GetCurr()->GetLen() &&
            CH_BREAK == aInf.GetTxt()[nRightMargin - 1])
            --nRightMargin;
        else if( !bAPI && (aLine.GetNext() || pFrm->GetFollow()) )
        {
            while( nRightMargin > aLine.GetStart() &&
                ' ' == aInf.GetTxt()[nRightMargin - 1])
                --nRightMargin;
        }
    }
    pPam->GetPoint()->nContent = SwIndex( pFrm->GetTxtNode(), nRightMargin );
    SwTxtCursor::SetRightMargin( !bAPI );
    return sal_True;
}

/*************************************************************************
 *                      SwTxtFrm::_UnitUp()
 *************************************************************************/






class SwSetToRightMargin
{
    sal_Bool bRight;
public:
    inline SwSetToRightMargin() : bRight( sal_False ) { }
    inline ~SwSetToRightMargin() { SwTxtCursor::SetRightMargin( bRight ); }
    inline void SetRight( const sal_Bool bNew ) { bRight = bNew; }
};

bool SwTxtFrm::_UnitUp( SwPaM *pPam, const SwTwips nOffset,
                        bool bSetInReadOnly ) const
{
    
    SwSetToRightMargin aSet;

    if( IsInTab() &&
        pPam->GetNode( true )->StartOfSectionNode() !=
        pPam->GetNode( false )->StartOfSectionNode() )
    {
        
        
        return SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
    }

    ((SwTxtFrm*)this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;

    if( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() )
                sw_ChangeOffset( ((SwTxtFrm*)this), nFormat );

            SwTxtSizeInfo aInf( (SwTxtFrm*)this );
            SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

            
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

            
            
            if( bPrevLine && !bSecondOfDouble )
            {
                aLine.PrevLine();
                while ( aLine.GetStart() == nStart &&
                        0 != ( pPrevLine = aLine.GetPrevLine() ) &&
                        pPrevLine != aLine.GetCurr() )
                    aLine.PrevLine();
            }

            if ( bPrevLine || bSecondOfDouble )
            {
                aCharBox.SSize().Width() /= 2;
                aCharBox.Pos().X() = aCharBox.Pos().X() - 150;

                
#if OSL_DEBUG_LEVEL > 0
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                
                sal_Int32 nTmpOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                                         aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                        "SwTxtFrm::UnitUp: illegal node change" );
#endif

                
                if( nTmpOfst >= nStart && nStart && !bSecondOfDouble )
                {
                    nTmpOfst = nStart;
                    aSet.SetRight( true );
                }
                pPam->GetPoint()->nContent =
                      SwIndex( ((SwTxtFrm*)this)->GetTxtNode(), nTmpOfst );
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
        const SwTxtFrm *pTmpPrev = FindMaster();
        sal_Int32 nOffs = GetOfst();
        if( pTmpPrev )
        {
            SwViewShell *pSh = getRootFrm()->GetCurrShell();
            const bool bProtectedAllowed = pSh && pSh->GetViewOptions()->IsCursorInProtectedArea();
            const SwTxtFrm *pPrevPrev = pTmpPrev;
            
            while( pPrevPrev && ( pPrevPrev->GetOfst() == nOffs ||
                   ( !bProtectedAllowed && pPrevPrev->IsProtected() ) ) )
            {
                pTmpPrev = pPrevPrev;
                nOffs = pTmpPrev->GetOfst();
                if ( pPrevPrev->IsFollow() )
                    pPrevPrev = pTmpPrev->FindMaster();
                else
                    pPrevPrev = NULL;
            }
            if ( !pPrevPrev )
                return pTmpPrev->SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
            aCharBox.Pos().Y() = pPrevPrev->Frm().Bottom() - 1;
            return pPrevPrev->GetKeyCrsrOfst( pPam->GetPoint(), aCharBox.Pos() );
        }
    }
    return SwCntntFrm::UnitUp( pPam, nOffset, bSetInReadOnly );
}

//





static void lcl_VisualMoveRecursion( const SwLineLayout& rCurrLine, sal_Int32 nIdx,
                              sal_Int32& nPos, bool& bRight,
                              sal_uInt8& nCrsrLevel, sal_uInt8 nDefaultDir )
{
    const SwLinePortion* pPor = rCurrLine.GetFirstPortion();
    const SwLinePortion* pLast = 0;

    
    while ( pPor && nIdx + pPor->GetLen() <= nPos )
    {
        nIdx = nIdx + pPor->GetLen();
        pLast = pPor;
        pPor = pPor->GetPortion();
    }

    if ( bRight )
    {
        bool bRecurse = pPor && pPor->IsMultiPortion() &&
                           ((SwMultiPortion*)pPor)->IsBidi();

        
        if ( bRecurse && nIdx == nPos )
        {
            nPos = nPos + pPor->GetLen();

            
            if ( nCrsrLevel != nDefaultDir )
            {
                bRecurse = false;
            }
            else
                
                
                
                
                nCrsrLevel++;
        }

        
        else if ( pLast && pLast->IsMultiPortion() &&
                 ((SwMultiPortion*)pLast)->IsBidi() && nIdx == nPos )
        {
            
            if ( nCrsrLevel != nDefaultDir )
            {
                bRecurse = true;
                nIdx = nIdx - pLast->GetLen();
                pPor = pLast;
            }
        }

        
        if ( bRecurse )
        {
            const SwLineLayout& rLine = ((SwMultiPortion*)pPor)->GetRoot();
            sal_Int32 nTmpPos = nPos - nIdx;
            bool bTmpForward = ! bRight;
            sal_uInt8 nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            nPos = nTmpPos + nIdx;
            bRight = bTmpForward;
            nCrsrLevel = nTmpCrsrLevel;
        }

        
        else
        {
            bRight = true;
            nCrsrLevel = nDefaultDir;
        }

    }
    else
    {
        bool bRecurse = pPor && pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi();

        
        if ( bRecurse && nIdx == nPos )
        {
            
            if ( nCrsrLevel == nDefaultDir )
            {
                bRecurse = false;
            }
        }

        
        else if ( pLast && pLast->IsMultiPortion() &&
                 ((SwMultiPortion*)pLast)->IsBidi() && nIdx == nPos )
        {
            nPos = nPos - pLast->GetLen();

            
            if ( nCrsrLevel % 2 == nDefaultDir % 2 )
            {
                bRecurse = true;
                nIdx = nIdx - pLast->GetLen();
                pPor = pLast;

                
                
                
                
                if ( nDefaultDir + 2 == nCrsrLevel )
                    nPos = nPos + pLast->GetLen();
            }
        }

        
        if ( bRecurse )
        {
            const SwLineLayout& rLine = ((SwMultiPortion*)pPor)->GetRoot();
            sal_Int32 nTmpPos = nPos - nIdx;
            bool bTmpForward = ! bRight;
            sal_uInt8 nTmpCrsrLevel = nCrsrLevel;
            lcl_VisualMoveRecursion( rLine, 0, nTmpPos, bTmpForward,
                                     nTmpCrsrLevel, nDefaultDir + 1 );

            
            
            
            
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

        
        else
        {
            bRight = false;
            nCrsrLevel = nDefaultDir;
        }
    }
}

void SwTxtFrm::PrepareVisualMove( sal_Int32& nPos, sal_uInt8& nCrsrLevel,
                                  bool& bForward, bool bInsertCrsr )
{
    if( IsEmpty() || IsHiddenNow() )
        return;

    ((SwTxtFrm*)this)->GetFormatted();

    SwTxtSizeInfo aInf( (SwTxtFrm*)this );
    SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );

    if( nPos )
        aLine.CharCrsrToLine( nPos );
    else
        aLine.Top();

    const SwLineLayout* pLine = aLine.GetCurr();
    const sal_Int32 nStt = aLine.GetStart();
    const sal_Int32 nLen = pLine->GetLen();

    
    
    
    
    
    
    
    
    
    if ( bInsertCrsr )
    {
        lcl_VisualMoveRecursion( *pLine, nStt, nPos, bForward,
                                 nCrsrLevel, IsRightToLeft() ? 1 : 0 );
        return;
    }

    const sal_uInt8 nDefaultDir = static_cast<sal_uInt8>(IsRightToLeft() ? UBIDI_RTL : UBIDI_LTR);
    const bool bVisualRight = ( nDefaultDir == UBIDI_LTR && bForward ) ||
                                  ( nDefaultDir == UBIDI_RTL && ! bForward );

    //
    
    //
    const sal_Unicode* pLineString = GetTxtNode()->GetTxt().getStr();

    UErrorCode nError = U_ZERO_ERROR;
    UBiDi* pBidi = ubidi_openSized( nLen, 0, &nError );
    ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(pLineString), nLen, nDefaultDir, NULL, &nError ); 

    sal_Int32 nTmpPos = 0;
    bool bOutOfBounds = false;

    if ( nPos < nStt + nLen )
    {
        nTmpPos = ubidi_getVisualIndex( pBidi, nPos, &nError );

        
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

/*************************************************************************
 *                      SwTxtFrm::_UnitDown()
 *************************************************************************/

bool SwTxtFrm::_UnitDown(SwPaM *pPam, const SwTwips nOffset,
                         bool bSetInReadOnly ) const
{

    if ( IsInTab() &&
        pPam->GetNode( true )->StartOfSectionNode() !=
        pPam->GetNode( false )->StartOfSectionNode() )
    {
        
        
        return SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
    }
    ((SwTxtFrm*)this)->GetFormatted();
    const sal_Int32 nPos = pPam->GetPoint()->nContent.GetIndex();
    SwRect aCharBox;
    const SwCntntFrm *pTmpFollow = 0;

    if ( IsVertical() )
        ((SwTxtFrm*)this)->SwapWidthAndHeight();

    if ( !IsEmpty() && !IsHiddenNow() )
    {
        sal_Int32 nFormat = COMPLETE_STRING;
        do
        {
            if( nFormat != COMPLETE_STRING && !IsFollow() &&
                !sw_ChangeOffset( ((SwTxtFrm*)this), nFormat ) )
                break;

            SwTxtSizeInfo aInf( (SwTxtFrm*)this );
            SwTxtCursor  aLine( ((SwTxtFrm*)this), &aInf );
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
                
                const sal_uLong nOldNode = pPam->GetPoint()->nNode.GetIndex();
#endif
                if ( pNextLine && ! bFirstOfDouble )
                    aLine.NextLine();

                sal_Int32 nTmpOfst = aLine.GetCrsrOfst( pPam->GetPoint(),
                                 aCharBox.Pos(), false );
#if OSL_DEBUG_LEVEL > 0
                OSL_ENSURE( nOldNode == pPam->GetPoint()->nNode.GetIndex(),
                    "SwTxtFrm::UnitDown: illegal node change" );
#endif

                
                if( nTmpOfst <= nStart && ! bFirstOfDouble )
                    nTmpOfst = nStart + 1;
                pPam->GetPoint()->nContent =
                      SwIndex( ((SwTxtFrm*)this)->GetTxtNode(), nTmpOfst );

                if ( IsVertical() )
                    ((SwTxtFrm*)this)->SwapWidthAndHeight();

                return true;
            }
            if( 0 != ( pTmpFollow = GetFollow() ) )
            {   
                const SwCntntFrm* pTmp = pTmpFollow;
                SwViewShell *pSh = getRootFrm()->GetCurrShell();
                if( !pSh || !pSh->GetViewOptions()->IsCursorInProtectedArea() )
                {
                    while( pTmpFollow && pTmpFollow->IsProtected() )
                    {
                        pTmp = pTmpFollow;
                        pTmpFollow = pTmpFollow->GetFollow();
                    }
                }
                if( !pTmpFollow ) 
                {
                    if ( IsVertical() )
                        ((SwTxtFrm*)this)->SwapWidthAndHeight();
                    return pTmp->SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
                }

                aLine.GetCharRect( &aCharBox, nPos );
                aCharBox.SSize().Width() /= 2;
            }
            else if( !IsFollow() )
            {
                sal_Int32 nTmpLen = aInf.GetTxt().getLength();
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
        ((SwTxtFrm*)this)->SwapWidthAndHeight();

    
    if( pTmpFollow )
    {
        aCharBox.Pos().Y() = pTmpFollow->Frm().Top() + 1;
        return ((SwTxtFrm*)pTmpFollow)->GetKeyCrsrOfst( pPam->GetPoint(),
                                                     aCharBox.Pos() );
    }
    return SwCntntFrm::UnitDown( pPam, nOffset, bSetInReadOnly );
}

/*************************************************************************
 *                   virtual SwTxtFrm::UnitUp()
 *************************************************************************/

sal_Bool SwTxtFrm::UnitUp(SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{
    /* We call CntntNode::GertFrm() in CrsrSh::Up().
     * This _always returns the master.
     * In order to not mess with cursor travelling, we correct here
     * in SwTxtFrm.
     * We calculate UnitUp for pFrm. pFrm is either a master (= this) or a
     * follow (!= this).
     */
    const SwTxtFrm *pFrm = GetAdjFrmAtPos( (SwTxtFrm*)this, *(pPam->GetPoint()),
                                           SwTxtCursor::IsRightMargin() );
    const sal_Bool bRet = pFrm->_UnitUp( pPam, nOffset, bSetInReadOnly );

    
    
    return bRet;
}

/*************************************************************************
 *                   virtual SwTxtFrm::UnitDown()
 *************************************************************************/

sal_Bool SwTxtFrm::UnitDown(SwPaM *pPam, const SwTwips nOffset,
                            sal_Bool bSetInReadOnly ) const
{
    const SwTxtFrm *pFrm = GetAdjFrmAtPos((SwTxtFrm*)this, *(pPam->GetPoint()),
                                           SwTxtCursor::IsRightMargin() );
    const sal_Bool bRet = pFrm->_UnitDown( pPam, nOffset, bSetInReadOnly );
    SwTxtCursor::SetRightMargin( false );
    return bRet;
}

void SwTxtFrm::FillCrsrPos( SwFillData& rFill ) const
{
    if( !rFill.bColumn && GetUpper()->IsColBodyFrm() ) 
    {
        const SwColumnFrm* pTmp =
            (SwColumnFrm*)GetUpper()->GetUpper()->GetUpper()->Lower(); 
        
        const SwFrm* pFrm = ((SwLayoutFrm*)pTmp->Lower())->Lower();
        MSHORT nNextCol = 0;
        
        while( rFill.X() > pTmp->Frm().Right() && pTmp->GetNext() )
        {
            pTmp = (SwColumnFrm*)pTmp->GetNext();
            if( ((SwLayoutFrm*)pTmp->Lower())->Lower() ) 
            {
                pFrm = ((SwLayoutFrm*)pTmp->Lower())->Lower();
                nNextCol = 0;
            }
            else
                ++nNextCol; 
        }
        if( pTmp != GetUpper()->GetUpper() ) 
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
            
            
            if( pFrm->IsTxtFrm() )
            {
                rFill.Fill().nColumnCnt = nNextCol;
                rFill.bColumn = true;
                if( rFill.pPos )
                {
                    SwTxtNode* pTxtNd = ((SwTxtFrm*)pFrm)->GetTxtNode();
                    rFill.pPos->nNode = *pTxtNd;
                    rFill.pPos->nContent.Assign(
                            pTxtNd, pTxtNd->GetTxt().getLength());
                }
                if( nNextCol )
                {
                    rFill.aFrm = pTmp->Prt();
                    rFill.aFrm += pTmp->Frm().Pos();
                }
                else
                    rFill.aFrm = pFrm->Frm();
                ((SwTxtFrm*)pFrm)->FillCrsrPos( rFill );
            }
            return;
        }
    }
    sal_Bool bFill = sal_True;
    SwFont *pFnt;
    SwTxtFmtColl* pColl = GetTxtNode()->GetTxtColl();
    MSHORT nFirst = GetTxtNode()->GetSwAttrSet().GetULSpace().GetLower();
    SwTwips nDiff = rFill.Y() - Frm().Bottom();
    if( nDiff < nFirst )
        nDiff = -1;
    else
        pColl = &pColl->GetNextTxtFmtColl();
    SwAttrSet aSet( ((SwDoc*)GetTxtNode()->GetDoc())->GetAttrPool(), aTxtFmtCollSetRange );
    const SwAttrSet* pSet = &pColl->GetAttrSet();
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if( GetTxtNode()->HasSwAttrSet() )
    {
        aSet.Put( *GetTxtNode()->GetpSwAttrSet() );
        aSet.SetParent( pSet );
        pSet = &aSet;
        pFnt = new SwFont( pSet, GetNode()->getIDocumentSettingAccess() );
    }
    else
    {
        SwFontAccess aFontAccess( pColl, pSh );
        pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
        pFnt->ChkMagic( pSh, pFnt->GetActual() );
    }
    OutputDevice* pOut = pSh->GetOut();
    if( !pSh->GetViewOptions()->getBrowseMode() || pSh->GetViewOptions()->IsPrtFormat() )
        pOut = GetTxtNode()->getIDocumentDeviceAccess()->getReferenceDevice( true );

    pFnt->SetFntChg( sal_True );
    pFnt->ChgPhysFnt( pSh, *pOut );

    SwTwips nLineHeight = pFnt->GetHeight( pSh, *pOut );

    if( nLineHeight )
    {
        const SvxULSpaceItem &rUL = pSet->GetULSpace();
        SwTwips nDist = std::max( rUL.GetLower(), rUL.GetUpper() );
        if( rFill.Fill().nColumnCnt )
        {
            rFill.aFrm.Height( nLineHeight );
            nDiff = rFill.Y() - rFill.Bottom();
            nFirst = 0;
        }
        else if( nDist < nFirst )
            nFirst = nFirst - (sal_uInt16)nDist;
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
            bFill = sal_False;
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
                            GetTxtNode()->GetLeftMarginWithNum();
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
                    bFill = sal_False;
            }
            else
            {
                SwTwips nSpace = 0;
                if( FILL_TAB != rFill.Mode() )
                {
                    const OUString aTmp("  ");
                    SwDrawTextInfo aDrawInf( pSh, *pOut, 0, aTmp, 0, 2 );
                    nSpace = pFnt->_GetTxtSize( aDrawInf ).Width()/2;
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
                        bFill = sal_False;
                }
                else if( FILL_INDENT == rFill.Mode() )
                {
                    SwTwips nIndent = rFill.X();
                    if( !rFill.bEmpty || nIndent > nRight )
                        bFill = sal_False;
                    else
                    {
                        nIndent -= rFill.Left();
                        if( nIndent >= 0 && nSpace )
                        {
                            nIndent /= nSpace;
                            nIndent *= nSpace;
                            rFill.SetTab( MSHORT( nIndent ) );
                            rRect.Left( nIndent + rFill.Left() );
                        }
                        else
                            bFill = sal_False;
                    }
                }
                else if( rFill.X() > nLeft )
                {
                    SwTwips nTxtLeft = rFill.Left() + rLRSpace.GetTxtLeft() +
                                    GetTxtNode()->GetLeftMarginWithNum( true );
                    rFill.nLineWidth += rFill.bFirstLine ? nLeft : nTxtLeft;
                    SwTwips nLeftTab = nLeft;
                    SwTwips nRightTab = nLeft;
                    MSHORT nSpaceCnt = 0;
                    MSHORT nTabCnt = 0;
                    MSHORT nIdx = 0;
                    do
                    {
                        nLeftTab = nRightTab;
                        if( nIdx < rRuler.Count() )
                        {
                            const SvxTabStop &rTabStop = rRuler.operator[](nIdx);
                            nRightTab = nTxtLeft + rTabStop.GetTabPos();
                            if( nLeftTab < nTxtLeft && nRightTab > nTxtLeft )
                                nRightTab = nTxtLeft;
                            else
                                ++nIdx;
                            if( nRightTab > rFill.nLineWidth )
                                ++nTabCnt;
                        }
                        else
                        {
                            const SvxTabStopItem& rTab =
                                (const SvxTabStopItem &)pSet->
                                GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
                            MSHORT nDefTabDist = (MSHORT)rTab[0].GetTabPos();
                            nRightTab = nLeftTab - nTxtLeft;
                            nRightTab /= nDefTabDist;
                            nRightTab = nRightTab * nDefTabDist + nTxtLeft;
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
                            bFill = sal_False;
                    }
                }
            }
            
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
                bFill = sal_False;
            else
                rRect.Width( 1 );
        }
    }
    else
        bFill = sal_False;
    ((SwCrsrMoveState*)rFill.pCMS)->bFillRet = bFill;
    delete pFnt;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
