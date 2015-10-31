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

#include "pagefrm.hxx"
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include "viewopt.hxx"
#include "hffrm.hxx"
#include "rootfrm.hxx"
#include "txtfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "hfspacingitem.hxx"
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <o3tl/make_unique.hxx>

extern bool bObjsDirect;    //frmtool.cxx

static SwTwips lcl_GetFrmMinHeight(const SwLayoutFrm & rFrm)
{
    const SwFormatFrmSize &rSz = rFrm.GetFormat()->GetFrmSize();
    SwTwips nMinHeight;

    switch (rSz.GetHeightSizeType())
    {
    case ATT_MIN_SIZE:
        nMinHeight = rSz.GetHeight();

        break;

    default:
        nMinHeight = 0;
    }

    return nMinHeight;
}

static SwTwips lcl_CalcContentHeight(SwLayoutFrm & frm)
{
    SwTwips nRemaining = 0;
    SwFrm* pFrm = frm.Lower();

    while ( pFrm )
    {
        SwTwips nTmp;

        nTmp = pFrm->Frm().Height();
        nRemaining += nTmp;
        if( pFrm->IsTextFrm() && static_cast<SwTextFrm*>(pFrm)->IsUndersized() )
        {
            nTmp = static_cast<SwTextFrm*>(pFrm)->GetParHeight()
                - pFrm->Prt().Height();
            // This TextFrm would like to be a bit bigger
            nRemaining += nTmp;
        }
        else if( pFrm->IsSctFrm() && static_cast<SwSectionFrm*>(pFrm)->IsUndersized() )
        {
            nTmp = static_cast<SwSectionFrm*>(pFrm)->Undersize();
            nRemaining += nTmp;
        }
        pFrm = pFrm->GetNext();
    }

    return nRemaining;
}

static void lcl_LayoutFrmEnsureMinHeight(SwLayoutFrm & rFrm,
                                         const SwBorderAttrs * )
{
    SwTwips nMinHeight = lcl_GetFrmMinHeight(rFrm);

    if (rFrm.Frm().Height() < nMinHeight)
    {
        rFrm.Grow(nMinHeight - rFrm.Frm().Height());
    }
}

SwHeadFootFrm::SwHeadFootFrm( SwFrameFormat * pFormat, SwFrm* pSib, sal_uInt16 nTypeIn)
    : SwLayoutFrm( pFormat, pSib )
{
    mnFrmType = nTypeIn;
    SetDerivedVert( false );

    const SwFormatContent &rCnt = pFormat->GetContent();

    OSL_ENSURE( rCnt.GetContentIdx(), "No content for Header." );

    // Have the objects created right now for header and footer
    bool bOld = bObjsDirect;
    bObjsDirect = true;
    sal_uLong nIndex = rCnt.GetContentIdx()->GetIndex();
    ::_InsertCnt( this, pFormat->GetDoc(), ++nIndex );
    bObjsDirect = bOld;
}

void SwHeadFootFrm::FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs)
{
    if (GetEatSpacing())
    {
        /* The minimal height of the print area is the minimal height of the
           frame without the height needed for borders and shadow. */
        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);

        nMinHeight -= pAttrs->CalcTop();
        nMinHeight -= pAttrs->CalcBottom();

        /* If the minimal height of the print area is negative, try to
           compensate by overlapping */
        SwTwips nOverlap = 0;
        if (nMinHeight < 0)
        {
            nOverlap = -nMinHeight;
            nMinHeight = 0;
        }

        /* Calculate desired height of content. The minimal height has to be
           adhered. */
        SwTwips nHeight;

        if ( ! HasFixSize() )
            nHeight = lcl_CalcContentHeight(*this);
        else
            nHeight = nMinHeight;

        if (nHeight < nMinHeight)
            nHeight = nMinHeight;

        /* calculate initial spacing/line space */
        SwTwips nSpace, nLine;

        if (IsHeaderFrm())
        {
            nSpace = pAttrs->CalcBottom();
            nLine = pAttrs->CalcBottomLine();
        }
        else
        {
            nSpace = pAttrs->CalcTop();
            nLine = pAttrs->CalcTopLine();
        }

        /* calculate overlap and correct spacing */
        nOverlap += nHeight - nMinHeight;
        if (nOverlap < nSpace - nLine)
            nSpace -= nOverlap;
        else
            nSpace = nLine;

        /* calculate real vertical space between frame and print area */
        if (IsHeaderFrm())
            nUL = pAttrs->CalcTop() + nSpace;
        else
            nUL = pAttrs->CalcBottom() + nSpace;

        /* set print area */
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );

        maPrt.Left(pAttrs->CalcLeft(this));

        if (IsHeaderFrm())
            maPrt.Top(pAttrs->CalcTop());
        else
            maPrt.Top(nSpace);

        maPrt.Width(maFrm.Width() - nLR);

        SwTwips nNewHeight;

        if (nUL < maFrm.Height())
            nNewHeight = maFrm.Height() - nUL;
        else
            nNewHeight = 0;

        maPrt.Height(nNewHeight);
    }
    else
    {
        // Set position
        maPrt.Left( pAttrs->CalcLeft( this ) );
        maPrt.Top ( pAttrs->CalcTop()  );

        // Set sizes - the sizes are given by the surrounding Frm, just
        // subtract the borders.
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
        maPrt.Width ( maFrm.Width() - nLR );
        maPrt.Height( maFrm.Height()- nUL );

    }

    mbValidPrtArea = true;
}

void SwHeadFootFrm::FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs)
{
    if ( !HasFixSize() )
    {
        if( !IsColLocked() )
        {
            mbValidSize = mbValidPrtArea = true;

            const SwTwips nBorder = nUL;
            SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
            nMinHeight -= pAttrs->CalcTop();
            nMinHeight -= pAttrs->CalcBottom();

            if (nMinHeight < 0)
                nMinHeight = 0;

            ColLock();

            SwTwips nMaxHeight = LONG_MAX;
            SwTwips nRemaining, nOldHeight;
            // #i64301#
            // use the position of the footer printing area to control invalidation
            // of the first footer content.
            Point aOldFooterPrtPos;

            do
            {
                nOldHeight = Prt().Height();
                SwFrm* pFrm = Lower();
                // #i64301#
                if ( pFrm &&
                     aOldFooterPrtPos != ( Frm().Pos() + Prt().Pos() ) )
                {
                    pFrm->_InvalidatePos();
                    aOldFooterPrtPos = Frm().Pos() + Prt().Pos();
                }
                int nLoopControl = 0;
                while( pFrm )
                {
                    pFrm->Calc(getRootFrm()->GetCurrShell()->GetOut());
                    // #i43771# - format also object anchored
                    // at the frame
                    // #i46941# - frame has to be valid.
                    // Note: frame could be invalid after calling its format,
                    //       if it's locked
                    OSL_ENSURE( StackHack::IsLocked() || !pFrm->IsTextFrm() ||
                            pFrm->IsValid() ||
                            static_cast<SwTextFrm*>(pFrm)->IsJoinLocked(),
                            "<SwHeadFootFrm::FormatSize(..)> - text frame invalid and not locked." );
                    if ( pFrm->IsTextFrm() && pFrm->IsValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrm( *pFrm,
                                                                  *(pFrm->FindPageFrm()) ) )
                        {
                            if (nLoopControl++ < 20)
                            {
                                // restart format with first content
                                pFrm = Lower();
                                continue;
                            }
                            else
                                SAL_WARN("sw", "SwHeadFootFrm::FormatSize: loop detection triggered");
                        }
                    }
                    pFrm = pFrm->GetNext();
                }
                nRemaining = 0;
                pFrm = Lower();

                while ( pFrm )
                {
                    nRemaining += pFrm->Frm().Height();

                    if( pFrm->IsTextFrm() &&
                        static_cast<SwTextFrm*>(pFrm)->IsUndersized() )
                        // This TextFrm would like to be a bit bigger
                        nRemaining += static_cast<SwTextFrm*>(pFrm)->GetParHeight()
                            - pFrm->Prt().Height();
                    else if( pFrm->IsSctFrm() &&
                             static_cast<SwSectionFrm*>(pFrm)->IsUndersized() )
                        nRemaining += static_cast<SwSectionFrm*>(pFrm)->Undersize();
                    pFrm = pFrm->GetNext();
                }
                if ( nRemaining < nMinHeight )
                    nRemaining = nMinHeight;

                SwTwips nDiff = nRemaining - nOldHeight;

                if( !nDiff )
                    break;
                if( nDiff < 0 )
                {
                    nMaxHeight = nOldHeight;

                    if( nRemaining <= nMinHeight )
                        nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
                }
                else
                {
                    if (nOldHeight > nMinHeight)
                        nMinHeight = nOldHeight;

                    if( nRemaining >= nMaxHeight )
                        nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
                }

                nDiff = nRemaining - nOldHeight;

                if ( nDiff )
                {
                    ColUnlock();
                    if ( nDiff > 0 )
                    {
                        if ( Grow( nDiff ) )
                        {
                            pFrm = Lower();

                            while ( pFrm )
                            {
                                if( pFrm->IsTextFrm())
                                {
                                    SwTextFrm * pTmpFrm = static_cast<SwTextFrm*>(pFrm);
                                    if (pTmpFrm->IsUndersized() )
                                    {
                                        pTmpFrm->InvalidateSize();
                                        pTmpFrm->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                /* #i3568# Undersized sections need to be
                                   invalidated too. */
                                else if (pFrm->IsSctFrm())
                                {
                                    SwSectionFrm * pTmpFrm =
                                        static_cast<SwSectionFrm*>(pFrm);
                                    if (pTmpFrm->IsUndersized() )
                                    {
                                        pTmpFrm->InvalidateSize();
                                        pTmpFrm->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                pFrm = pFrm->GetNext();
                            }
                        }
                    }
                    else
                        Shrink( -nDiff );
                    // Quickly update the position

                    MakePos();
                    ColLock();
                }
                else
                    break;
                // Don't overwrite the lower edge of the upper
                if ( GetUpper() && Frm().Height() )
                {
                    const SwTwips nDeadLine = GetUpper()->Frm().Top() +
                        GetUpper()->Prt().Bottom();
                    const SwTwips nBot = Frm().Bottom();
                    if ( nBot > nDeadLine )
                    {
                        Frm().Bottom( nDeadLine );
                        Prt().SSize().Height() = Frm().Height() - nBorder;
                    }
                }
                mbValidSize = mbValidPrtArea = true;
            } while( nRemaining<=nMaxHeight && nOldHeight!=Prt().Height() );
            ColUnlock();
        }
        mbValidSize = mbValidPrtArea = true;
    }
    else //if (GetType() & FRM_HEADFOOT)
    {
        do
        {
            if ( Frm().Height() != pAttrs->GetSize().Height() )
                ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
            mbValidSize = true;
            MakePos();
        } while ( !mbValidSize );
    }
}

void SwHeadFootFrm::Format(vcl::RenderContext* pRenderContext, const SwBorderAttrs * pAttrs)
{
    OSL_ENSURE( pAttrs, "SwFooterFrm::Format, pAttrs is 0." );

    if ( mbValidPrtArea && mbValidSize )
        return;

    if ( ! GetEatSpacing() && IsHeaderFrm())
    {
        SwLayoutFrm::Format(pRenderContext, pAttrs);
    }
    else
    {
        lcl_LayoutFrmEnsureMinHeight(*this, pAttrs);

        long nUL = pAttrs->CalcTop()  + pAttrs->CalcBottom();

        if ( !mbValidPrtArea )
            FormatPrt(nUL, pAttrs);

        if ( !mbValidSize )
            FormatSize(nUL, pAttrs);
    }
}

SwTwips SwHeadFootFrm::GrowFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (!GetEatSpacing())
    {
        nResult = SwLayoutFrm::GrowFrm(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        auto pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
        OSL_ENSURE(pAccess, "no border attributes");

        SwBorderAttrs * pAttrs = pAccess->Get();

        /* First assume the whole amount to grow can be provided by eating
           spacing. */
        SwTwips nEat = nDist;
        SwTwips nMaxEat;

        /* calculate maximum eatable spacing */
        if (IsHeaderFrm())
            nMaxEat = maFrm.Height() - maPrt.Top() - maPrt.Height() - pAttrs->CalcBottomLine();
        else
            nMaxEat = maPrt.Top() - pAttrs->CalcTopLine();

        if (nMaxEat < 0)
            nMaxEat = 0;

        /* If the frame is too small, eat less spacing thus letting the frame
           grow more. */
        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
        SwTwips nFrameTooSmall = nMinHeight - Frm().Height();

        if (nFrameTooSmall > 0)
            nEat -= nFrameTooSmall;

        /* No negative eating, not eating more than allowed. */
        if (nEat < 0)
            nEat = 0;
        else if (nEat > nMaxEat)
            nEat = nMaxEat;

        // OD 10.04.2003 #108719# - Notify fly frame, if header frame
        // grows. Consider, that 'normal' grow of layout frame already notifys
        // the fly frames.
        bool bNotifyFlys = false;
        if (nEat > 0)
        {
            if ( ! bTst)
            {
                if (! IsHeaderFrm())
                {
                    maPrt.Top(maPrt.Top() - nEat);
                    maPrt.Height(maPrt.Height() - nEat);
                }

                InvalidateAll();
            }

            nResult += nEat;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrm() )
            {
                bNotifyFlys = true;
            }
        }

        if (nDist - nEat > 0)
        {
            const SwTwips nFrmGrow =
                SwLayoutFrm::GrowFrm( nDist - nEat, bTst, bInfo );

            nResult += nFrmGrow;
            if ( nFrmGrow > 0 )
            {
                bNotifyFlys = false;
            }
        }

        // OD 10.04.2003 #108719# - notify fly frames, if necessary and triggered.
        if ( ( nResult > 0 ) && bNotifyFlys )
        {
            NotifyLowerObjs();
        }
    }

    if ( nResult && !bTst )
        SetCompletePaint();

    return nResult;
}

SwTwips SwHeadFootFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (! GetEatSpacing())
    {
        nResult = SwLayoutFrm::ShrinkFrm(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
        SwTwips nOldHeight = Frm().Height();
        SwTwips nRest = 0; // Amount to shrink by spitting out spacing

        if ( nOldHeight >= nMinHeight )
        {
            /* If the frame's height is bigger than its minimum height, shrink
               the frame towards its minimum height. If this is not sufficient
               to provide the shrinking requested provide the rest by spitting
               out spacing. */

            SwTwips nBiggerThanMin = nOldHeight - nMinHeight;

            if (nBiggerThanMin < nDist)
            {
                nRest = nDist - nBiggerThanMin;
            }
            /* info: declaration of nRest -> else nRest = 0 */
        }
        else
            /* The frame cannot shrink. Provide shrinking by spitting out
               spacing. */
            nRest = nDist;

        // OD 10.04.2003 #108719# - Notify fly frame, if header/footer frame
        // shrinks. Consider, that 'normal' shrink of layout frame already notifys
        // the fly frames.
        bool bNotifyFlys = false;
        if (nRest > 0)
        {
            auto pAccess = o3tl::make_unique<SwBorderAttrAccess>(SwFrm::GetCache(), this);
            OSL_ENSURE(pAccess, "no border attributes");

            SwBorderAttrs * pAttrs = pAccess->Get();

            /* minimal height of print area */
            SwTwips nMinPrtHeight = nMinHeight
                - pAttrs->CalcTop()
                - pAttrs->CalcBottom();

            if (nMinPrtHeight < 0)
                nMinPrtHeight = 0;

            /* assume all shrinking can be provided */
            SwTwips nShrink = nRest;

            /* calculate maximum shrinking */
            SwTwips nMaxShrink = maPrt.Height() - nMinPrtHeight;

            /* shrink no more than maximum shrinking */
            if (nShrink > nMaxShrink)
            {
                //nRest -= nShrink - nMaxShrink;
                nShrink = nMaxShrink;
            }

            if (!bTst)
            {
                if (! IsHeaderFrm() )
                {
                    maPrt.Top(maPrt.Top() + nShrink);
                    maPrt.Height(maPrt.Height() - nShrink);
                }

                InvalidateAll();
            }
            nResult += nShrink;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrm() )
            {
                bNotifyFlys = true;
            }
        }

        /* The shrinking not providable by spitting out spacing has to be done
           by the frame. */
        if (nDist - nRest > 0)
        {
            SwTwips nShrinkAmount = SwLayoutFrm::ShrinkFrm( nDist - nRest, bTst, bInfo );
            nResult += nShrinkAmount;
            if ( nShrinkAmount > 0 )
            {
                bNotifyFlys = false;
            }
        }

        // OD 10.04.2003 #108719# - notify fly frames, if necessary.
        if ( ( nResult > 0 ) && bNotifyFlys )
        {
            NotifyLowerObjs();
        }
    }

    return nResult;
}

bool SwHeadFootFrm::GetEatSpacing() const
{
    const SwFrameFormat * pFormat = GetFormat();
    OSL_ENSURE(pFormat, "SwHeadFootFrm: no format?");

    if (pFormat->GetHeaderAndFooterEatSpacing().GetValue())
        return true;

    return false;
}

void DelFlys( SwLayoutFrm *pFrm, SwPageFrm *pPage )
{
    size_t i = 0;
    while ( pPage->GetSortedObjs() &&
            pPage->GetSortedObjs()->size() &&
            i < pPage->GetSortedObjs()->size() )
    {
        SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
        if ( dynamic_cast< const SwFlyFrm *>( pObj ) !=  nullptr )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
            if ( pFrm->IsAnLower( pFlyFrm ) )
            {
                SwFrm::DestroyFrm(pFlyFrm);
                // Do not increment index, in this case
                continue;
            }
        }
        ++i;
    }
}

/// Creates or removes headers
void SwPageFrm::PrepareHeader()
{
    SwLayoutFrm *pLay = static_cast<SwLayoutFrm*>(Lower());
    if ( !pLay )
        return;

    const SwFormatHeader &rH = static_cast<SwFrameFormat*>(GetRegisteredIn())->GetHeader();

    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bOn = !(pSh && (pSh->GetViewOptions()->getBrowseMode() ||
                               pSh->GetViewOptions()->IsWhitespaceHidden()));

    if ( bOn && rH.IsActive() )
    {   //Implant header, but remove first, if already present
        OSL_ENSURE( rH.GetHeaderFormat(), "FrameFormat for Header not found." );

        if ( pLay->GetFormat() == rH.GetHeaderFormat() )
            return; // Header is already the correct one.

        if ( pLay->IsHeaderFrm() )
        {   SwLayoutFrm *pDel = pLay;
            pLay = static_cast<SwLayoutFrm*>(pLay->GetNext());
            ::DelFlys( pDel, this );
            pDel->Cut();
            SwFrm::DestroyFrm(pDel);
        }
        OSL_ENSURE( pLay, "Where to with the Header?" );
        SwHeaderFrm *pH = new SwHeaderFrm( const_cast<SwFrameFormat*>(rH.GetHeaderFormat()), this );
        pH->Paste( this, pLay );
        if ( GetUpper() )
            ::RegistFlys( this, pH );
    }
    else if ( pLay && pLay->IsHeaderFrm() )
    {   // Remove header if present.
        ::DelFlys( pLay, this );
        pLay->Cut();
        SwFrm::DestroyFrm(pLay);
    }
}

/// Creates or removes footer
void SwPageFrm::PrepareFooter()
{
    SwLayoutFrm *pLay = static_cast<SwLayoutFrm*>(Lower());
    if ( !pLay )
        return;

    const SwFormatFooter &rF = static_cast<SwFrameFormat*>(GetRegisteredIn())->GetFooter();
    while ( pLay->GetNext() )
        pLay = static_cast<SwLayoutFrm*>(pLay->GetNext());

    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bOn = !(pSh && (pSh->GetViewOptions()->getBrowseMode() ||
                               pSh->GetViewOptions()->IsWhitespaceHidden()));

    if ( bOn && rF.IsActive() )
    {   //Implant footer, but remove first, if already present
        OSL_ENSURE( rF.GetFooterFormat(), "FrameFormat for Footer not found." );

        if ( pLay->GetFormat() == rF.GetFooterFormat() )
            return;  // Footer is already the correct one.

        if ( pLay->IsFooterFrm() )
        {
            ::DelFlys( pLay, this );
            pLay->Cut();
            SwFrm::DestroyFrm(pLay);
        }
        SwFooterFrm *pF = new SwFooterFrm( const_cast<SwFrameFormat*>(rF.GetFooterFormat()), this );
        pF->Paste( this );
        if ( GetUpper() )
            ::RegistFlys( this, pF );
    }
    else if ( pLay->IsFooterFrm() )
    {   // Remove footer if already present
        ::DelFlys( pLay, this );
        SwViewShell *pShell;
        if ( pLay->GetPrev() && 0 != (pShell = getRootFrm()->GetCurrShell()) &&
             pShell->VisArea().HasArea() )
            pShell->InvalidateWindows( pShell->VisArea() );
        pLay->Cut();
        SwFrm::DestroyFrm(pLay);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
