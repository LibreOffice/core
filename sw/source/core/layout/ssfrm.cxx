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

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <dcontact.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <cellfrm.hxx>
#include <swtable.hxx>
#include <fmtfsize.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <fmtclds.hxx>
#include <viewimp.hxx>
#include <sortedobjs.hxx>
#include <hints.hxx>

    // No inline cause we need the function pointers
long SwFrm::GetTopMargin() const
    { return Prt().Top(); }
long SwFrm::GetBottomMargin() const
    { return Frm().Height() -Prt().Height() -Prt().Top(); }
long SwFrm::GetLeftMargin() const
    { return Prt().Left(); }
long SwFrm::GetRightMargin() const
    { return Frm().Width() - Prt().Width() - Prt().Left(); }
long SwFrm::GetPrtLeft() const
    { return Frm().Left() + Prt().Left(); }
long SwFrm::GetPrtBottom() const
    { return Frm().Top() + Prt().Height() + Prt().Top(); }
long SwFrm::GetPrtRight() const
    { return Frm().Left() + Prt().Width() + Prt().Left(); }
long SwFrm::GetPrtTop() const
    { return Frm().Top() + Prt().Top(); }

bool SwFrm::SetMinLeft( long nDeadline )
{
    SwTwips nDiff = nDeadline - Frm().Left();
    if( nDiff > 0 )
    {
        Frm().Left( nDeadline );
        Prt().Width( Prt().Width() - nDiff );
        return true;
    }
    return false;
}

bool SwFrm::SetMaxBottom( long nDeadline )
{
    SwTwips nDiff = Frm().Top() + Frm().Height() - nDeadline;
    if( nDiff > 0 )
    {
        Frm().Height( Frm().Height() - nDiff );
        Prt().Height( Prt().Height() - nDiff );
        return true;
    }
    return false;
}

bool SwFrm::SetMinTop( long nDeadline )
{
    SwTwips nDiff = nDeadline - Frm().Top();
    if( nDiff > 0 )
    {
        Frm().Top( nDeadline );
        Prt().Height( Prt().Height() - nDiff );
        return true;
    }
    return false;
}

bool SwFrm::SetMaxRight( long nDeadline )
{
    SwTwips nDiff = Frm().Left() + Frm().Width() - nDeadline;
    if( nDiff > 0 )
    {
        Frm().Width( Frm().Width() - nDiff );
        Prt().Width( Prt().Width() - nDiff );
        return true;
    }
    return false;
}

void SwFrm::MakeBelowPos( const SwFrm* pUp, const SwFrm* pPrv, bool bNotify )
{
    if( pPrv )
    {
        maFrm.Pos( pPrv->Frm().Pos() );
        maFrm.Pos().Y() += pPrv->Frm().Height();
    }
    else
    {
        maFrm.Pos( pUp->Frm().Pos() );
        maFrm.Pos() += pUp->Prt().Pos();
    }
    if( bNotify )
        maFrm.Pos().Y() += 1;
}

void SwFrm::MakeUpperPos( const SwFrm* pUp, const SwFrm* pPrv, bool bNotify )
{
    if( pPrv )
    {
        maFrm.Pos( pPrv->Frm().Pos() );
        maFrm.Pos().Y() -= Frm().Height();
    }
    else
    {
        maFrm.Pos( pUp->Frm().Pos() );
        maFrm.Pos() += pUp->Prt().Pos();
        maFrm.Pos().Y() += pUp->Prt().Height() - maFrm.Height();
    }
    if( bNotify )
        maFrm.Pos().Y() -= 1;
}

void SwFrm::MakeLeftPos( const SwFrm* pUp, const SwFrm* pPrv, bool bNotify )
{
    if( pPrv )
    {
        maFrm.Pos( pPrv->Frm().Pos() );
        maFrm.Pos().X() -= Frm().Width();
    }
    else
    {
        maFrm.Pos( pUp->Frm().Pos() );
        maFrm.Pos() += pUp->Prt().Pos();
        maFrm.Pos().X() += pUp->Prt().Width() - maFrm.Width();
    }
    if( bNotify )
        maFrm.Pos().X() -= 1;
}

void SwFrm::MakeRightPos( const SwFrm* pUp, const SwFrm* pPrv, bool bNotify )
{
    if( pPrv )
    {
        maFrm.Pos( pPrv->Frm().Pos() );
        maFrm.Pos().X() += pPrv->Frm().Width();
    }
    else
    {
        maFrm.Pos( pUp->Frm().Pos() );
        maFrm.Pos() += pUp->Prt().Pos();
    }
    if( bNotify )
        maFrm.Pos().X() += 1;
}

void SwFrm::SetTopBottomMargins( long nTop, long nBot )
{
    Prt().Top( nTop );
    Prt().Height( Frm().Height() - nTop - nBot );
}

void SwFrm::SetBottomTopMargins( long nBot, long nTop )
{
    Prt().Top( nTop );
    Prt().Height( Frm().Height() - nTop - nBot );
}

void SwFrm::SetLeftRightMargins( long nLeft, long nRight)
{
    Prt().Left( nLeft );
    Prt().Width( Frm().Width() - nLeft - nRight );
}

void SwFrm::SetRightLeftMargins( long nRight, long nLeft)
{
    Prt().Left( nLeft );
    Prt().Width( Frm().Width() - nLeft - nRight );
}

/// checks the layout direction and invalidates the lower frames recursively, if necessary.
void SwFrm::CheckDirChange()
{
    bool bOldVert = GetVerticalFlag();
    bool bOldRev = IsReverse();
    bool bOldR2L = GetRightToLeftFlag();
    SetInvalidVert( true );
    SetInvalidR2L( true );
    bool bChg = bOldR2L != IsRightToLeft();
    bool bOldVertL2R = IsVertLR();
    if( ( IsVertical() != bOldVert ) || bChg || IsReverse() != bOldRev || bOldVertL2R != IsVertLR() )
    {
        InvalidateAll();
        if( IsLayoutFrm() )
        {
            // set minimum row height for vertical cells in horizontal table:
            if ( IsCellFrm() && GetUpper() )
            {
                if ( IsVertical() != GetUpper()->IsVertical() &&
                     static_cast<SwCellFrm*>(this)->GetTabBox()->getRowSpan() == 1 )
                {
                    enum {
                        MIN_VERT_CELL_HEIGHT = 1135
                    };

                    SwTableLine* pLine = const_cast<SwTableLine*>(static_cast<SwCellFrm*>(this)->GetTabBox()->GetUpper());
                    SwFrameFormat* pFrameFormat = pLine->GetFrameFormat();
                    SwFormatFrmSize aNew( pFrameFormat->GetFrmSize() );
                    if ( ATT_FIX_SIZE != aNew.GetHeightSizeType() )
                        aNew.SetHeightSizeType( ATT_MIN_SIZE );
                    if ( aNew.GetHeight() < MIN_VERT_CELL_HEIGHT )
                        aNew.SetHeight( MIN_VERT_CELL_HEIGHT );
                    SwDoc* pDoc = pFrameFormat->GetDoc();
                    pDoc->SetAttr( aNew, *pLine->ClaimFrameFormat() );
                }
            }

            SwFrm* pFrm = static_cast<SwLayoutFrm*>(this)->Lower();
            const SwFormatCol* pCol = NULL;
            SwLayoutFrm* pBody = 0;
            if( pFrm )
            {
                if( IsPageFrm() )
                {
                    // If we're a page frame and we change our layout direction,
                    // we have to look for columns and rearrange them.
                    pBody = static_cast<SwPageFrm*>(this)->FindBodyCont();
                    if(pBody && pBody->Lower() && pBody->Lower()->IsColumnFrm())
                        pCol = &static_cast<SwPageFrm*>(this)->GetFormat()->GetCol();
                }
                else if( pFrm->IsColumnFrm() )
                {
                    pBody = static_cast<SwLayoutFrm*>(this);
                    const SwFrameFormat *pFormat = pBody->GetFormat();
                    if( pFormat )
                        pCol = &pFormat->GetCol();
                }
            }
            while( pFrm )
            {
                pFrm->CheckDirChange();
                pFrm = pFrm->GetNext();
            }
            if( pCol )
                pBody->AdjustColumns( pCol, true );
        }
        else if( IsTextFrm() )
            static_cast<SwTextFrm*>(this)->Prepare( PREP_CLEAR );

        // #i31698# - notify anchored objects also for page frames.
        // Remove code above for special handling of page frames
        if ( GetDrawObjs() )
        {
            const SwSortedObjs *pObjs = GetDrawObjs();
            const size_t nCnt = pObjs->size();
            for ( size_t i = 0; i < nCnt; ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if( pAnchoredObj->ISA(SwFlyFrm) )
                    static_cast<SwFlyFrm*>(pAnchoredObj)->CheckDirChange();
                else
                {
                    // OD 2004-04-06 #i26791# - direct object
                    // positioning no longer needed. Instead
                    // invalidate
                    pAnchoredObj->InvalidateObjPos();
                }
                // #i31698# - update layout direction of
                // anchored object
                {
                    ::setContextWritingMode( pAnchoredObj->DrawObj(), pAnchoredObj->GetAnchorFrmContainingAnchPos() );
                    pAnchoredObj->UpdateLayoutDir();
                }
            }
        }
    }
}

/// returns the position for anchors based on frame direction
// OD 2004-03-10 #i11860# - consider lower space and line spacing of
// previous frame according to new option 'Use former object positioning'
Point SwFrm::GetFrmAnchorPos( bool bIgnoreFlysAnchoredAtThisFrame ) const
{
    Point aAnchor = Frm().Pos();

    if ( ( IsVertical() && !IsVertLR() ) || IsRightToLeft() )
        aAnchor.X() += Frm().Width();

    if ( IsTextFrm() )
    {
        SwTwips nBaseOfstForFly =
            static_cast<const SwTextFrm*>(this)->GetBaseOfstForFly( bIgnoreFlysAnchoredAtThisFrame );
        if ( IsVertical() )
            aAnchor.Y() += nBaseOfstForFly;
        else
            aAnchor.X() += nBaseOfstForFly;

        // OD 2004-03-10 #i11860# - if option 'Use former object positioning'
        // is OFF, consider the lower space and the line spacing of the
        // previous frame and the spacing considered for the page grid
        const SwTextFrm* pThisTextFrm = static_cast<const SwTextFrm*>(this);
        const SwTwips nUpperSpaceAmountConsideredForPrevFrmAndPageGrid =
                pThisTextFrm->GetUpperSpaceAmountConsideredForPrevFrmAndPageGrid();
        if ( IsVertical() )
        {
            aAnchor.X() -= nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
        }
        else
        {
            aAnchor.Y() += nUpperSpaceAmountConsideredForPrevFrmAndPageGrid;
        }
    }

    return aAnchor;
}

void SwFrm::DestroyImpl()
{
    mbInDtor = true;

    // accessible objects for fly and cell frames have been already disposed
    // by the destructors of the derived classes.
    if( IsAccessibleFrm() && !(IsFlyFrm() || IsCellFrm()) && GetDep() )
    {
        SwRootFrm *pRootFrm = getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
        {
            SwViewShell *pVSh = pRootFrm->GetCurrShell();
            if( pVSh && pVSh->Imp() )
            {
                OSL_ENSURE( !GetLower(), "Lowers should be dispose already!" );
                pVSh->Imp()->DisposeAccessibleFrm( this );
            }
        }
    }

    if( mpDrawObjs )
    {
        for ( size_t i = mpDrawObjs->size(); i; )
        {
            SwAnchoredObject* pAnchoredObj = (*mpDrawObjs)[--i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFrm::DestroyFrm(static_cast<SwFlyFrm*>(pAnchoredObj));
            }
            else
            {
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                SwDrawContact* pContact =
                        static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                OSL_ENSURE( pContact,
                        "<SwFrm::~SwFrm> - missing contact for drawing object" );
                if ( pContact )
                {
                    pContact->DisconnectObjFromLayout( pSdrObj );
                }
            }
        }
        delete mpDrawObjs;
        mpDrawObjs = 0;
    }
}

SwFrm::~SwFrm()
{
    assert(m_isInDestroy); // check that only DestroySwFrm does "delete"
    assert(!IsDeleteForbidden()); // check that its not deleted while deletes are forbidden
#if OSL_DEBUG_LEVEL > 0
    // JP 15.10.2001: for detection of access to deleted frames
    mpDrawObjs = reinterpret_cast<SwSortedObjs*>(0x33333333);
#endif
}

void SwFrm::DestroyFrm(SwFrm *const pFrm)
{
    if (pFrm)
    {
        pFrm->m_isInDestroy = true;
        pFrm->DestroyImpl();
        assert(pFrm->mbInDtor); // check that nobody forgot to call base class
        delete pFrm;
    }
}

const SwFrameFormat * SwLayoutFrm::GetFormat() const
{
    return static_cast< const SwFrameFormat * >( GetDep() );
}

SwFrameFormat * SwLayoutFrm::GetFormat()
{
    return static_cast< SwFrameFormat * >( GetDep() );
}

void SwLayoutFrm::SetFrameFormat( SwFrameFormat *pNew )
{
    if ( pNew != GetFormat() )
    {
        SwFormatChg aOldFormat( GetFormat() );
        pNew->Add( this );
        SwFormatChg aNewFormat( pNew );
        ModifyNotification( &aOldFormat, &aNewFormat );
    }
}

SwContentFrm::SwContentFrm( SwContentNode * const pContent, SwFrm* pSib ) :
    SwFrm( pContent, pSib ),
    SwFlowFrm( (SwFrm&)*this )
{
}

void SwContentFrm::DestroyImpl()
{
    const SwContentNode* pCNd;
    if( 0 != ( pCNd = PTR_CAST( SwContentNode, GetRegisteredIn() )) &&
        !pCNd->GetDoc()->IsInDtor() )
    {
        //Unregister from root if I'm still in turbo there.
        SwRootFrm *pRoot = getRootFrm();
        if( pRoot && pRoot->GetTurbo() == this )
        {
            pRoot->DisallowTurbo();
            pRoot->ResetTurbo();
        }
    }

    SwFrm::DestroyImpl();
}

SwContentFrm::~SwContentFrm()
{
}

void SwContentFrm::RegisterToNode( SwContentNode& rNode )
{
    rNode.Add( this );
}

void SwLayoutFrm::DestroyImpl()
{
    while (!m_VertPosOrientFrmsFor.empty())
    {
        SwAnchoredObject *pObj = *m_VertPosOrientFrmsFor.begin();
        pObj->ClearVertPosOrientFrm();
    }

    assert(m_VertPosOrientFrmsFor.empty());

    SwFrm *pFrm = m_pLower;

    if( GetFormat() && !GetFormat()->GetDoc()->IsInDtor() )
    {
        while ( pFrm )
        {
            //First delete the Objs of the Frm because they can't unregister
            //from the page after remove.
            //We don't want to create an endless loop only because one couldn't
            //unregister.

            while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->size() )
            {
                const size_t nCnt = pFrm->GetDrawObjs()->size();
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[0];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    SwFrm::DestroyFrm(static_cast<SwFlyFrm*>(pAnchoredObj));
                    assert(!pFrm->GetDrawObjs() || nCnt > pFrm->GetDrawObjs()->size());
                }
                else
                {
                    SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                    SwDrawContact* pContact =
                            static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                    OSL_ENSURE( pContact,
                            "<SwFrm::~SwFrm> - missing contact for drawing object" );
                    if ( pContact )
                    {
                        pContact->DisconnectObjFromLayout( pSdrObj );
                    }

                    if ( pFrm->GetDrawObjs() &&
                         nCnt == pFrm->GetDrawObjs()->size() )
                    {
                        pFrm->GetDrawObjs()->Remove( *pAnchoredObj );
                    }
                }
            }
            pFrm->RemoveFromLayout();
            SwFrm::DestroyFrm(pFrm);
            pFrm = m_pLower;
        }
        //Delete the Flys, the last one also deletes the array.
        while ( GetDrawObjs() && GetDrawObjs()->size() )
        {
            const size_t nCnt = GetDrawObjs()->size();

            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetDrawObjs())[0];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFrm::DestroyFrm(static_cast<SwFlyFrm*>(pAnchoredObj));
                assert(!GetDrawObjs() || nCnt > GetDrawObjs()->size());
            }
            else
            {
                SdrObject* pSdrObj = pAnchoredObj->DrawObj();
                SwDrawContact* pContact =
                        static_cast<SwDrawContact*>(pSdrObj->GetUserCall());
                OSL_ENSURE( pContact,
                        "<SwFrm::~SwFrm> - missing contact for drawing object" );
                if ( pContact )
                {
                    pContact->DisconnectObjFromLayout( pSdrObj );
                }

                if ( GetDrawObjs() && nCnt == GetDrawObjs()->size() )
                {
                    GetDrawObjs()->Remove( *pAnchoredObj );
                }
            }
        }
    }
    else
    {
        while( pFrm )
        {
            SwFrm *pNxt = pFrm->GetNext();
            SwFrm::DestroyFrm(pFrm);
            pFrm = pNxt;
        }
    }

    SwFrm::DestroyImpl();
}

SwLayoutFrm::~SwLayoutFrm()
{
}

/**
|*  The paintarea is the area, in which the content of a frame is allowed
|*  to be displayed. This region could be larger than the printarea (Prt())
|*  of the upper, it includes e.g. often the margin of the page.
|*/
const SwRect SwFrm::PaintArea() const
{
    // NEW TABLES
    // Cell frames may not leave their upper:
    SwRect aRect = IsRowFrm() ? GetUpper()->Frm() : Frm();
    const bool bVert = IsVertical();
    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;
    long nRight = (aRect.*fnRect->fnGetRight)();
    long nLeft  = (aRect.*fnRect->fnGetLeft)();
    const SwFrm* pTmp = this;
    bool bLeft = true;
    bool bRight = true;
    long nRowSpan = 0;
    while( pTmp )
    {
        if( pTmp->IsCellFrm() && pTmp->GetUpper() &&
            pTmp->GetUpper()->IsVertical() != pTmp->IsVertical() )
            nRowSpan = static_cast<const SwCellFrm*>(pTmp)->GetTabBox()->getRowSpan();
        long nTmpRight = (pTmp->Frm().*fnRect->fnGetRight)();
        long nTmpLeft = (pTmp->Frm().*fnRect->fnGetLeft)();
        if( pTmp->IsRowFrm() && nRowSpan > 1 )
        {
            const SwFrm* pNxt = pTmp;
            while( --nRowSpan > 0 && pNxt->GetNext() )
                pNxt = pNxt->GetNext();
            if( pTmp->IsVertical() )
                nTmpLeft = (pNxt->Frm().*fnRect->fnGetLeft)();
            else
                nTmpRight = (pNxt->Frm().*fnRect->fnGetRight)();
        }
        OSL_ENSURE( pTmp, "PaintArea lost in time and space" );
        if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() ||
            pTmp->IsCellFrm() || pTmp->IsRowFrm() || //nobody leaves a table!
            pTmp->IsRootFrm() )
        {
            if( bLeft || nLeft < nTmpLeft )
                nLeft = nTmpLeft;
            if( bRight || nTmpRight < nRight )
                nRight = nTmpRight;
            if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() || pTmp->IsRootFrm() )
                break;
            bLeft = false;
            bRight = false;
        }
        else if( pTmp->IsColumnFrm() )  // nobody enters neightbour columns
        {
            bool bR2L = pTmp->IsRightToLeft();
            // the first column has _no_ influence to the left range
            if( bR2L ? pTmp->GetNext() : pTmp->GetPrev() )
            {
                if( bLeft || nLeft < nTmpLeft )
                    nLeft = nTmpLeft;
                bLeft = false;
            }
             // the last column has _no_ influence to the right range
            if( bR2L ? pTmp->GetPrev() : pTmp->GetNext() )
            {
                if( bRight || nTmpRight < nRight )
                    nRight = nTmpRight;
                bRight = false;
            }
        }
        else if( bVert && pTmp->IsBodyFrm() )
        {
            // Header and footer frames have always horizontal direction and
            // limit the body frame.
            // A previous frame of a body frame must be a header,
            // the next frame of a body frame may be a footnotecontainer or
            // a footer. The footnotecontainer has the same direction like
            // the body frame.
            if( pTmp->GetPrev() && ( bLeft || nLeft < nTmpLeft ) )
            {
                nLeft = nTmpLeft;
                bLeft = false;
            }
            if( pTmp->GetNext() &&
                ( pTmp->GetNext()->IsFooterFrm() || pTmp->GetNext()->GetNext() )
                && ( bRight || nTmpRight < nRight ) )
            {
                nRight = nTmpRight;
                bRight = false;
            }
        }
        pTmp = pTmp->GetUpper();
    }
    (aRect.*fnRect->fnSetLeft)( nLeft );
    (aRect.*fnRect->fnSetRight)( nRight );
    return aRect;
}

/**
|*  The unionframe is the framearea (Frm()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*/
const SwRect SwFrm::UnionFrm( bool bBorder ) const
{
    bool bVert = IsVertical();
    SwRectFn fnRect = bVert ? ( IsVertLR() ? fnRectVertL2R : fnRectVert ) : fnRectHori;
    long nLeft = (Frm().*fnRect->fnGetLeft)();
    long nWidth = (Frm().*fnRect->fnGetWidth)();
    long nPrtLeft = (Prt().*fnRect->fnGetLeft)();
    long nPrtWidth = (Prt().*fnRect->fnGetWidth)();
    if( nPrtLeft + nPrtWidth > nWidth )
        nWidth = nPrtLeft + nPrtWidth;
    if( nPrtLeft < 0 )
    {
        nLeft += nPrtLeft;
        nWidth -= nPrtLeft;
    }
    SwTwips nRight = nLeft + nWidth;
    long nAdd = 0;
    if( bBorder )
    {
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        const SvxBoxItem &rBox = rAttrs.GetBox();
        if ( rBox.GetLeft() )
            nLeft -= rBox.CalcLineSpace( SvxBoxItemLine::LEFT );
        else if ( rAttrs.IsBorderDist() )
            nLeft -= rBox.GetDistance( SvxBoxItemLine::LEFT ) + 1;
        if ( rBox.GetRight() )
            nAdd += rBox.CalcLineSpace( SvxBoxItemLine::RIGHT );
        else if ( rAttrs.IsBorderDist() )
            nAdd += rBox.GetDistance( SvxBoxItemLine::RIGHT ) + 1;
        if( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
        {
            const SvxShadowItem &rShadow = rAttrs.GetShadow();
            nLeft -= rShadow.CalcShadowSpace( SvxShadowItemSide::LEFT );
            nAdd += rShadow.CalcShadowSpace( SvxShadowItemSide::RIGHT );
        }
    }
    if( IsTextFrm() && static_cast<const SwTextFrm*>(this)->HasPara() )
    {
        long nTmp = static_cast<const SwTextFrm*>(this)->HangingMargin();
        if( nTmp > nAdd )
            nAdd = nTmp;
    }
    nWidth = nRight + nAdd - nLeft;
    SwRect aRet( Frm() );
    (aRet.*fnRect->fnSetPosX)( nLeft );
    (aRet.*fnRect->fnSetWidth)( nWidth );
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
