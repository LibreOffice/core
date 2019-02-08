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
#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include <viewopt.hxx>
#include <hffrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <flyfrm.hxx>
#include <frmtool.hxx>
#include <hfspacingitem.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <ndindex.hxx>
#include <sal/log.hxx>

static SwTwips lcl_GetFrameMinHeight(const SwLayoutFrame & rFrame)
{
    const SwFormatFrameSize &rSz = rFrame.GetFormat()->GetFrameSize();
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

static SwTwips lcl_CalcContentHeight(SwLayoutFrame & frm)
{
    SwTwips nRemaining = 0;
    SwFrame* pFrame = frm.Lower();

    while ( pFrame )
    {
        SwTwips nTmp;

        nTmp = pFrame->getFrameArea().Height();
        nRemaining += nTmp;
        if( pFrame->IsTextFrame() && static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
        {
            nTmp = static_cast<SwTextFrame*>(pFrame)->GetParHeight()
                - pFrame->getFramePrintArea().Height();
            // This TextFrame would like to be a bit bigger
            nRemaining += nTmp;
        }
        else if( pFrame->IsSctFrame() && static_cast<SwSectionFrame*>(pFrame)->IsUndersized() )
        {
            nTmp = static_cast<SwSectionFrame*>(pFrame)->Undersize();
            nRemaining += nTmp;
        }
        pFrame = pFrame->GetNext();
    }

    return nRemaining;
}

static void lcl_LayoutFrameEnsureMinHeight(SwLayoutFrame & rFrame)
{
    SwTwips nMinHeight = lcl_GetFrameMinHeight(rFrame);

    if (rFrame.getFrameArea().Height() < nMinHeight)
    {
        rFrame.Grow(nMinHeight - rFrame.getFrameArea().Height());
    }
}

SwHeadFootFrame::SwHeadFootFrame( SwFrameFormat * pFormat, SwFrame* pSib, SwFrameType nTypeIn)
    : SwLayoutFrame( pFormat, pSib )
{
    mnFrameType = nTypeIn;
    SetDerivedVert( false );

    const SwFormatContent &rCnt = pFormat->GetContent();

    OSL_ENSURE( rCnt.GetContentIdx(), "No content for Header." );

    // Have the objects created right now for header and footer
    bool bOld = bObjsDirect;
    bObjsDirect = true;
    sal_uLong nIndex = rCnt.GetContentIdx()->GetIndex();
    ::InsertCnt_( this, pFormat->GetDoc(), ++nIndex );
    bObjsDirect = bOld;
}

void SwHeadFootFrame::FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs)
{
    if (GetEatSpacing())
    {
        /* The minimal height of the print area is the minimal height of the
           frame without the height needed for borders and shadow. */
        SwTwips nMinHeight = lcl_GetFrameMinHeight(*this);

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

        if (IsHeaderFrame())
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
        if (IsHeaderFrame())
            nUL = pAttrs->CalcTop() + nSpace;
        else
            nUL = pAttrs->CalcBottom() + nSpace;

        /* set print area */
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);

        aPrt.Left(pAttrs->CalcLeft(this));

        if (IsHeaderFrame())
        {
            aPrt.Top(pAttrs->CalcTop());
        }
        else
        {
            aPrt.Top(nSpace);
        }

        aPrt.Width(getFrameArea().Width() - nLR);

        SwTwips nNewHeight;

        if (nUL < getFrameArea().Height())
        {
            nNewHeight = getFrameArea().Height() - nUL;
        }
        else
        {
            nNewHeight = 0;
        }

        aPrt.Height(nNewHeight);
    }
    else
    {
        // Set position
        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
        aPrt.Left( pAttrs->CalcLeft( this ) );
        aPrt.Top ( pAttrs->CalcTop()  );

        // Set sizes - the sizes are given by the surrounding Frame, just
        // subtract the borders.
        // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
        SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
        aPrt.Width ( getFrameArea().Width() - nLR );
        aPrt.Height( getFrameArea().Height()- nUL );
    }

    setFramePrintAreaValid(true);
}

void SwHeadFootFrame::FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs)
{
    if ( !HasFixSize() )
    {
        if( !IsColLocked() )
        {
            setFramePrintAreaValid(true);
            setFrameAreaSizeValid(true);

            const SwTwips nBorder = nUL;
            SwTwips nMinHeight = lcl_GetFrameMinHeight(*this);
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
                nOldHeight = getFramePrintArea().Height();
                SwFrame* pFrame = Lower();
                // #i64301#
                if ( pFrame &&
                     aOldFooterPrtPos != ( getFrameArea().Pos() + getFramePrintArea().Pos() ) )
                {
                    pFrame->InvalidatePos_();
                    aOldFooterPrtPos = getFrameArea().Pos() + getFramePrintArea().Pos();
                }
                int nLoopControl = 0;
                while( pFrame )
                {
                    pFrame->Calc(getRootFrame()->GetCurrShell()->GetOut());
                    // #i43771# - format also object anchored
                    // at the frame
                    // #i46941# - frame has to be valid.
                    // Note: frame could be invalid after calling its format,
                    //       if it's locked
                    OSL_ENSURE( StackHack::IsLocked() || !pFrame->IsTextFrame() ||
                            pFrame->isFrameAreaDefinitionValid() ||
                            static_cast<SwTextFrame*>(pFrame)->IsJoinLocked(),
                            "<SwHeadFootFrame::FormatSize(..)> - text frame invalid and not locked." );

                    if ( pFrame->IsTextFrame() && pFrame->isFrameAreaDefinitionValid() )
                    {
                        if ( !SwObjectFormatter::FormatObjsAtFrame( *pFrame,
                                                                  *(pFrame->FindPageFrame()) ) )
                        {
                            if (nLoopControl++ < 20)
                            {
                                // restart format with first content
                                pFrame = Lower();
                                continue;
                            }
                            else
                                SAL_WARN("sw", "SwHeadFootFrame::FormatSize: loop detection triggered");
                        }
                    }
                    pFrame = pFrame->GetNext();
                }
                nRemaining = 0;
                pFrame = Lower();

                while ( pFrame )
                {
                    nRemaining += pFrame->getFrameArea().Height();

                    if( pFrame->IsTextFrame() &&
                        static_cast<SwTextFrame*>(pFrame)->IsUndersized() )
                        // This TextFrame would like to be a bit bigger
                        nRemaining += static_cast<SwTextFrame*>(pFrame)->GetParHeight()
                            - pFrame->getFramePrintArea().Height();
                    else if( pFrame->IsSctFrame() &&
                             static_cast<SwSectionFrame*>(pFrame)->IsUndersized() )
                        nRemaining += static_cast<SwSectionFrame*>(pFrame)->Undersize();
                    pFrame = pFrame->GetNext();
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
                            pFrame = Lower();

                            while ( pFrame )
                            {
                                if( pFrame->IsTextFrame())
                                {
                                    SwTextFrame * pTmpFrame = static_cast<SwTextFrame*>(pFrame);
                                    if (pTmpFrame->IsUndersized() )
                                    {
                                        pTmpFrame->InvalidateSize();
                                        pTmpFrame->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                /* #i3568# Undersized sections need to be
                                   invalidated too. */
                                else if (pFrame->IsSctFrame())
                                {
                                    SwSectionFrame * pTmpFrame =
                                        static_cast<SwSectionFrame*>(pFrame);
                                    if (pTmpFrame->IsUndersized() )
                                    {
                                        pTmpFrame->InvalidateSize();
                                        pTmpFrame->Prepare(PREP_ADJUST_FRM);
                                    }
                                }
                                pFrame = pFrame->GetNext();
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
                if ( GetUpper() && getFrameArea().Height() )
                {
                    const SwTwips nDeadLine = GetUpper()->getFrameArea().Top() + GetUpper()->getFramePrintArea().Bottom();
                    const SwTwips nBot = getFrameArea().Bottom();

                    if ( nBot > nDeadLine )
                    {
                        SwFrameAreaDefinition::FrameAreaWriteAccess aFrm(*this);
                        aFrm.Bottom( nDeadLine );

                        SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                        aPrt.SSize().setHeight( getFrameArea().Height() - nBorder );
                    }
                }

                setFramePrintAreaValid(true);
                setFrameAreaSizeValid(true);
            } while( nRemaining<=nMaxHeight && nOldHeight!=getFramePrintArea().Height() );
            ColUnlock();
        }

        setFramePrintAreaValid(true);
        setFrameAreaSizeValid(true);
    }
    else //if (GetType() & FRM_HEADFOOT)
    {
        do
        {
            if ( getFrameArea().Height() != pAttrs->GetSize().Height() )
            {
                ChgSize( Size( getFrameArea().Width(), pAttrs->GetSize().Height()));
            }

            setFrameAreaSizeValid(true);
            MakePos();
        } while ( !isFrameAreaSizeValid() );
    }
}

void SwHeadFootFrame::Format(vcl::RenderContext* pRenderContext, const SwBorderAttrs * pAttrs)
{
    OSL_ENSURE( pAttrs, "SwFooterFrame::Format, pAttrs is 0." );

    if ( isFramePrintAreaValid() && isFrameAreaSizeValid() )
        return;

    if ( ! GetEatSpacing() && IsHeaderFrame())
    {
        SwLayoutFrame::Format(pRenderContext, pAttrs);
    }
    else
    {
        lcl_LayoutFrameEnsureMinHeight(*this);

        long nUL = pAttrs->CalcTop()  + pAttrs->CalcBottom();

        if ( !isFramePrintAreaValid() )
            FormatPrt(nUL, pAttrs);

        if ( !isFrameAreaSizeValid() )
            FormatSize(nUL, pAttrs);
    }
}

SwTwips SwHeadFootFrame::GrowFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (!GetEatSpacing())
    {
        nResult = SwLayoutFrame::GrowFrame(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        auto pAccess = std::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
        OSL_ENSURE(pAccess, "no border attributes");

        SwBorderAttrs * pAttrs = pAccess->Get();

        /* First assume the whole amount to grow can be provided by eating
           spacing. */
        SwTwips nEat = nDist;
        SwTwips nMaxEat;

        /* calculate maximum eatable spacing */
        if (IsHeaderFrame())
            nMaxEat = getFrameArea().Height() - getFramePrintArea().Top() - getFramePrintArea().Height() - pAttrs->CalcBottomLine();
        else
            nMaxEat = getFramePrintArea().Top() - pAttrs->CalcTopLine();

        if (nMaxEat < 0)
            nMaxEat = 0;

        /* If the frame is too small, eat less spacing thus letting the frame
           grow more. */
        SwTwips nMinHeight = lcl_GetFrameMinHeight(*this);
        SwTwips nFrameTooSmall = nMinHeight - getFrameArea().Height();

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
                if (! IsHeaderFrame())
                {
                    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                    aPrt.Top(aPrt.Top() - nEat);
                    aPrt.Height(aPrt.Height() - nEat);
                }

                InvalidateAll();
            }

            nResult += nEat;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrame() )
            {
                bNotifyFlys = true;
            }
        }

        if (nDist - nEat > 0)
        {
            const SwTwips nFrameGrow =
                SwLayoutFrame::GrowFrame( nDist - nEat, bTst, bInfo );

            nResult += nFrameGrow;
            if ( nFrameGrow > 0 )
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

SwTwips SwHeadFootFrame::ShrinkFrame( SwTwips nDist, bool bTst, bool bInfo )
{
    SwTwips nResult;

    if ( IsColLocked() )
    {
        nResult = 0;
    }
    else if (! GetEatSpacing())
    {
        nResult = SwLayoutFrame::ShrinkFrame(nDist, bTst, bInfo);
    }
    else
    {
        nResult = 0;

        SwTwips nMinHeight = lcl_GetFrameMinHeight(*this);
        SwTwips nOldHeight = getFrameArea().Height();
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
            auto pAccess = std::make_unique<SwBorderAttrAccess>(SwFrame::GetCache(), this);
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
            SwTwips nMaxShrink = getFramePrintArea().Height() - nMinPrtHeight;

            /* shrink no more than maximum shrinking */
            if (nShrink > nMaxShrink)
            {
                //nRest -= nShrink - nMaxShrink;
                nShrink = nMaxShrink;
            }

            if (!bTst)
            {
                if (! IsHeaderFrame() )
                {
                    SwFrameAreaDefinition::FramePrintAreaWriteAccess aPrt(*this);
                    aPrt.Top(aPrt.Top() + nShrink);
                    aPrt.Height(aPrt.Height() - nShrink);
                }

                InvalidateAll();
            }
            nResult += nShrink;
            // OD 14.04.2003 #108719# - trigger fly frame notify.
            if ( IsHeaderFrame() )
            {
                bNotifyFlys = true;
            }
        }

        /* The shrinking not providable by spitting out spacing has to be done
           by the frame. */
        if (nDist - nRest > 0)
        {
            SwTwips nShrinkAmount = SwLayoutFrame::ShrinkFrame( nDist - nRest, bTst, bInfo );
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

bool SwHeadFootFrame::GetEatSpacing() const
{
    const SwFrameFormat * pFormat = GetFormat();
    OSL_ENSURE(pFormat, "SwHeadFootFrame: no format?");

    return pFormat->GetHeaderAndFooterEatSpacing().GetValue();
}

static void DelFlys( SwLayoutFrame const *pFrame, SwPageFrame *pPage )
{
    size_t i = 0;
    while ( pPage->GetSortedObjs() &&
            pPage->GetSortedObjs()->size() &&
            i < pPage->GetSortedObjs()->size() )
    {
        SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
        if ( dynamic_cast< const SwFlyFrame *>( pObj ) !=  nullptr )
        {
            SwFlyFrame* pFlyFrame = static_cast<SwFlyFrame*>(pObj);
            if ( pFrame->IsAnLower( pFlyFrame ) )
            {
                SwFrame::DestroyFrame(pFlyFrame);
                // Do not increment index, in this case
                continue;
            }
        }
        ++i;
    }
}

/// Creates or removes headers
void SwPageFrame::PrepareHeader()
{
    SwLayoutFrame *pLay = static_cast<SwLayoutFrame*>(Lower());
    if ( !pLay )
        return;

    const SwFormatHeader &rH = static_cast<SwFrameFormat*>(GetDep())->GetHeader();

    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bOn = !(pSh && (pSh->GetViewOptions()->getBrowseMode() ||
                               pSh->GetViewOptions()->IsWhitespaceHidden()));

    if ( bOn && rH.IsActive() )
    {   //Implant header, but remove first, if already present
        OSL_ENSURE( rH.GetHeaderFormat(), "FrameFormat for Header not found." );

        if ( pLay->GetFormat() == rH.GetHeaderFormat() )
            return; // Header is already the correct one.

        if ( pLay->IsHeaderFrame() )
        {   SwLayoutFrame *pDel = pLay;
            pLay = static_cast<SwLayoutFrame*>(pLay->GetNext());
            ::DelFlys( pDel, this );
            pDel->Cut();
            SwFrame::DestroyFrame(pDel);
        }
        OSL_ENSURE( pLay, "Where to with the Header?" );
        SwHeaderFrame *pH = new SwHeaderFrame( const_cast<SwFrameFormat*>(rH.GetHeaderFormat()), this );
        pH->Paste( this, pLay );
        if ( GetUpper() )
            ::RegistFlys( this, pH );
    }
    else if (pLay->IsHeaderFrame())
    {   // Remove header if present.
        ::DelFlys( pLay, this );
        pLay->Cut();
        SwFrame::DestroyFrame(pLay);
    }
}

/// Creates or removes footer
void SwPageFrame::PrepareFooter()
{
    SwLayoutFrame *pLay = static_cast<SwLayoutFrame*>(Lower());
    if ( !pLay )
        return;

    const SwFormatFooter &rF = static_cast<SwFrameFormat*>(GetDep())->GetFooter();
    while ( pLay->GetNext() )
        pLay = static_cast<SwLayoutFrame*>(pLay->GetNext());

    const SwViewShell *pSh = getRootFrame()->GetCurrShell();
    const bool bOn = !(pSh && (pSh->GetViewOptions()->getBrowseMode() ||
                               pSh->GetViewOptions()->IsWhitespaceHidden()));

    if ( bOn && rF.IsActive() )
    {   //Implant footer, but remove first, if already present
        OSL_ENSURE( rF.GetFooterFormat(), "FrameFormat for Footer not found." );

        if ( pLay->GetFormat() == rF.GetFooterFormat() )
            return;  // Footer is already the correct one.

        if ( pLay->IsFooterFrame() )
        {
            ::DelFlys( pLay, this );
            pLay->Cut();
            SwFrame::DestroyFrame(pLay);
        }
        SwFooterFrame *pF = new SwFooterFrame( const_cast<SwFrameFormat*>(rF.GetFooterFormat()), this );
        pF->Paste( this );
        if ( GetUpper() )
            ::RegistFlys( this, pF );
    }
    else if ( pLay->IsFooterFrame() )
    {   // Remove footer if already present
        ::DelFlys( pLay, this );
        SwViewShell *pShell;
        if ( pLay->GetPrev() && nullptr != (pShell = getRootFrame()->GetCurrShell()) &&
             pShell->VisArea().HasArea() )
            pShell->InvalidateWindows( pShell->VisArea() );
        pLay->Cut();
        SwFrame::DestroyFrame(pLay);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
