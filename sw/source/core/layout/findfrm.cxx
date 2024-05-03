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
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <swtable.hxx>
#include <notxtfrm.hxx>
#include <tabfrm.hxx>
#include <sectfrm.hxx>
#include <frmatr.hxx>
#include <flyfrm.hxx>
#include <ftnfrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <fmtpdsc.hxx>
#include <fmtclbl.hxx>
#include <txtfrm.hxx>
#include <bodyfrm.hxx>
#include <calbck.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <IDocumentSettingAccess.hxx>
#include <formatflysplit.hxx>
#include <flyfrms.hxx>


/// Searches the first ContentFrame in BodyText below the page.
SwLayoutFrame *SwFootnoteBossFrame::FindBodyCont()
{
    SwFrame *pLay = Lower();
    while ( pLay && !pLay->IsBodyFrame() )
        pLay = pLay->GetNext();
    return static_cast<SwLayoutFrame*>(pLay);
}

/// Searches the last ContentFrame in BodyText below the page.
SwContentFrame *SwPageFrame::FindLastBodyContent()
{
    SwContentFrame *pRet = FindFirstBodyContent();
    SwContentFrame *pNxt = pRet;
    while ( pNxt && pNxt->IsInDocBody() && IsAnLower( pNxt ) )
    {   pRet = pNxt;
        pNxt = pNxt->FindNextCnt();
    }
    return pRet;
}

/**
 * Checks if the frame contains one or more ContentFrame's anywhere in his
 * subsidiary structure; if so the first found ContentFrame is returned.
 */
const SwContentFrame *SwLayoutFrame::ContainsContent() const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found or we leave "this".
    //Sections: Content next to sections would not be found this way (empty
    //sections directly next to ContentFrame) therefore we need to recursively
    //search for them even if it's more complex.

    const SwLayoutFrame *pLayLeaf = this;
    do
    {
        while ( (!pLayLeaf->IsSctFrame() || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrame() )
            pLayLeaf = static_cast<const SwLayoutFrame*>(pLayLeaf->Lower());

        if( pLayLeaf->IsSctFrame() && pLayLeaf != this )
        {
            const SwContentFrame *pCnt = pLayLeaf->ContainsContent();
            if( pCnt )
                return pCnt;
            if( pLayLeaf->GetNext() )
            {
                if( pLayLeaf->GetNext()->IsLayoutFrame() )
                {
                    pLayLeaf = static_cast<const SwLayoutFrame*>(pLayLeaf->GetNext());
                    continue;
                }
                else
                    return static_cast<const SwContentFrame*>(pLayLeaf->GetNext());
            }
        }
        else if ( pLayLeaf->Lower() )
            return static_cast<const SwContentFrame*>(pLayLeaf->Lower());

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( !IsAnLower( pLayLeaf) )
            return nullptr;
    } while( pLayLeaf );
    return nullptr;
}

/**
 * Calls ContainsAny first to reach the innermost cell. From there we walk back
 * up to the first SwCellFrame. Since we use SectionFrames, ContainsContent()->GetUpper()
 * is not enough anymore.
 */
const SwCellFrame *SwLayoutFrame::FirstCell() const
{
    const SwFrame* pCnt = ContainsAny();
    while( pCnt && !pCnt->IsCellFrame() )
        pCnt = pCnt->GetUpper();
    return static_cast<const SwCellFrame*>(pCnt);
}

/** return ContentFrames, sections, and tables.
 *
 * @param _bInvestigateFootnoteForSections controls investigation of content of footnotes for sections.
 * @see ContainsContent
 */
const SwFrame *SwLayoutFrame::ContainsAny( const bool _bInvestigateFootnoteForSections ) const
{
    //Search downwards the layout leaf and if there is no content, jump to the
    //next leaf until content is found, we leave "this" or until we found
    //a SectionFrame or a TabFrame.

    const SwLayoutFrame *pLayLeaf = this;
    const bool bNoFootnote = IsSctFrame() && !_bInvestigateFootnoteForSections;
    do
    {
        while ( ( (!pLayLeaf->IsSctFrame() && !pLayLeaf->IsTabFrame())
                 || pLayLeaf == this ) &&
                pLayLeaf->Lower() && pLayLeaf->Lower()->IsLayoutFrame() )
            pLayLeaf = static_cast<const SwLayoutFrame*>(pLayLeaf->Lower());

        if( ( pLayLeaf->IsTabFrame() || pLayLeaf->IsSctFrame() )
            && pLayLeaf != this )
        {
            // Now we also return "deleted" SectionFrames so they can be
            // maintained on SaveContent and RestoreContent
            return pLayLeaf;
        }
        else if ( pLayLeaf->Lower() )
            return static_cast<const SwContentFrame*>(pLayLeaf->Lower());

        pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
        if( bNoFootnote && pLayLeaf && pLayLeaf->IsInFootnote() )
        {
            do
            {
                pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
            } while( pLayLeaf && pLayLeaf->IsInFootnote() );
        }
        if( !IsAnLower( pLayLeaf) )
            return nullptr;
    } while( pLayLeaf );
    return nullptr;
}

bool SwLayoutFrame::ContainsDeleteForbiddenLayFrame() const
{
    if (IsDeleteForbidden())
    {
        return true;
    }
    for (SwFrame const* pFrame = Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (!pFrame->IsLayoutFrame())
        {
            continue;
        }
        SwLayoutFrame const*const pLay(static_cast<SwLayoutFrame const*>(pFrame));
        if (pLay->ContainsDeleteForbiddenLayFrame() || pLay->IsColLocked())
        {
            return true;
        }
    }
    return false;
}

const SwFrame* SwFrame::GetLower() const
{
    return IsLayoutFrame() ? static_cast<const SwLayoutFrame*>(this)->Lower() : nullptr;
}

SwFrame* SwFrame::GetLower()
{
    return IsLayoutFrame() ? static_cast<SwLayoutFrame*>(this)->Lower() : nullptr;
}

SwContentFrame* SwFrame::FindPrevCnt( )
{
    if ( GetPrev() && GetPrev()->IsContentFrame() )
        return static_cast<SwContentFrame*>(GetPrev());
    else
        return FindPrevCnt_();
}

const SwContentFrame* SwFrame::FindPrevCnt() const
{
    if ( GetPrev() && GetPrev()->IsContentFrame() )
        return static_cast<const SwContentFrame*>(GetPrev());
    else
        return const_cast<SwFrame*>(this)->FindPrevCnt_();
}

SwContentFrame *SwFrame::FindNextCnt( const bool _bInSameFootnote )
{
    if ( mpNext && mpNext->IsContentFrame() )
        return static_cast<SwContentFrame*>(mpNext);
    else
        return FindNextCnt_( _bInSameFootnote );
}

const SwContentFrame *SwFrame::FindNextCnt( const bool _bInSameFootnote ) const
{
    if ( mpNext && mpNext->IsContentFrame() )
        return static_cast<SwContentFrame*>(mpNext);
    else
        return const_cast<SwFrame*>(this)->FindNextCnt_( _bInSameFootnote );
}

bool SwLayoutFrame::IsAnLower( const SwFrame *pAssumed ) const
{
    const SwFrame *pUp = pAssumed;
    while ( pUp )
    {
        if (!pUp->IsInDtor())
        {
            if (pUp == this)
                return true;
            if (pUp->IsFlyFrame())
                pUp = static_cast<const SwFlyFrame*>(pUp)->GetAnchorFrame();
            else
                pUp = pUp->GetUpper();
        }
        else
            break;
    }
    return false;
}

/** method to check relative position of layout frame to
    a given layout frame.

    OD 08.11.2002 - refactoring of pseudo-local method <lcl_Apres(..)> in
    <txtftn.cxx> for #104840#.

    @param _aCheckRefLayFrame
    constant reference of an instance of class <SwLayoutFrame> which
    is used as the reference for the relative position check.

    @return true, if <this> is positioned before the layout frame <p>
*/
bool SwLayoutFrame::IsBefore( const SwLayoutFrame* _pCheckRefLayFrame ) const
{
    OSL_ENSURE( !IsRootFrame() , "<IsBefore> called at a <SwRootFrame>.");
    OSL_ENSURE( !_pCheckRefLayFrame->IsRootFrame() , "<IsBefore> called with a <SwRootFrame>.");

    bool bReturn;

    // check, if on different pages
    const SwPageFrame *pMyPage = FindPageFrame();
    const SwPageFrame *pCheckRefPage = _pCheckRefLayFrame->FindPageFrame();
    if( pMyPage != pCheckRefPage )
    {
        // being on different page as check reference
        bReturn = pMyPage->GetPhyPageNum() < pCheckRefPage->GetPhyPageNum();
    }
    else
    {
        // being on same page as check reference
        // --> search my supreme parent <pUp>, which doesn't contain check reference.
        const SwLayoutFrame* pUp = this;
        while ( pUp->GetUpper() &&
                !pUp->GetUpper()->IsAnLower( _pCheckRefLayFrame )
              )
            pUp = pUp->GetUpper();
        if( !pUp->GetUpper() )
        {
            // can occur, if <this> is a fly frm
            bReturn = false;
        }
        else
        {
            // travel through the next's of <pUp> and check if one of these
            // contain the check reference.
            const SwLayoutFrame* pUpNext = static_cast<const SwLayoutFrame*>(pUp->GetNext());
            while ( pUpNext &&
                    !pUpNext->IsAnLower( _pCheckRefLayFrame ) )
            {
                pUpNext = static_cast<const SwLayoutFrame*>(pUpNext->GetNext());
            }
            bReturn = pUpNext != nullptr;
        }
    }

    return bReturn;
}

// Local helper functions for GetNextLayoutLeaf

static const SwFrame* lcl_FindLayoutFrame( const SwFrame* pFrame, bool bNext )
{
    const SwFrame* pRet = nullptr;
    if ( pFrame->IsFlyFrame() )
    {
        auto pFlyFrame = static_cast<const SwFlyFrame*>(pFrame);
        if (pFlyFrame->IsFlySplitAllowed())
        {
            // This is a flow frame, look up the follow/precede.
            auto pFlyAtContent = static_cast<const SwFlyAtContentFrame*>(pFlyFrame);
            pRet = bNext ? pFlyAtContent->GetFollow() : pFlyAtContent->GetPrecede();
        }
        else
        {
            pRet = bNext ? pFlyFrame->GetNextLink() : pFlyFrame->GetPrevLink();
        }
    }
    else
        pRet = bNext ? pFrame->GetNext() : pFrame->GetPrev();

    return pRet;
}

static const SwFrame* lcl_GetLower( const SwFrame* pFrame, bool bFwd )
{
    if ( !pFrame->IsLayoutFrame() )
        return nullptr;

    return bFwd ?
           static_cast<const SwLayoutFrame*>(pFrame)->Lower() :
           static_cast<const SwLayoutFrame*>(pFrame)->GetLastLower();
}

/**
 * Finds the next layout leaf. This is a layout frame, which does not
 * have a lower which is a LayoutFrame. That means, pLower can be 0 or a
 * content frame.
 *
 * However, pLower may be a TabFrame
 */
const SwLayoutFrame *SwFrame::ImplGetNextLayoutLeaf( bool bFwd ) const
{
    const SwFrame       *pFrame = this;
    const SwLayoutFrame *pLayoutFrame = nullptr;
    const SwFrame       *p = nullptr;
    bool bGoingUp = !bFwd;          // false for forward, true for backward
    // The anchor is this frame, unless we traverse the anchor of a split fly.
    const SwFrame* pAnchor = this;
    do {

         bool bGoingFwdOrBwd = false;

         bool bGoingDown = !bGoingUp;
         if (bGoingDown)
         {
            p = lcl_GetLower( pFrame, bFwd );
            bGoingDown = nullptr != p;
         }
         if ( !bGoingDown )
         {
             // I cannot go down, because either I'm currently going up or
             // because the is no lower.
             // I'll try to go forward:
             p = lcl_FindLayoutFrame( pFrame, bFwd );
             bGoingFwdOrBwd = nullptr != p;
             if ( !bGoingFwdOrBwd )
             {
                 // I cannot go forward, because there is no next frame.
                 // I'll try to go up:
                 p = pFrame->GetUpper();

                 if (!p && pFrame->IsFlyFrame())
                 {
                     const SwFlyFrame* pFlyFrame = pFrame->FindFlyFrame();
                     if (pFlyFrame->IsFlySplitAllowed())
                     {
                         p = const_cast<SwFlyFrame*>(pFlyFrame)->FindAnchorCharFrame();
                         // Remember the anchor frame, so if we look for the leaf of a frame in a
                         // split fly (anchored in the master of a text frame, but rendered on the
                         // next page), we won't return a frame on the next page, rather return
                         // nullptr.
                         pAnchor = p;
                     }
                 }

                 bGoingUp = nullptr != p;
                 if ( !bGoingUp )
                 {
                    // I cannot go up, because there is no upper frame.
                    return nullptr;
                 }
             }
         }

         // If I could not go down or forward, I'll have to go up
         bGoingUp = !bGoingFwdOrBwd && !bGoingDown;

         pFrame = p;
         p = lcl_GetLower( pFrame, true );

    } while( ( p && !p->IsFlowFrame() ) ||
             pFrame == this ||
             nullptr == ( pLayoutFrame = pFrame->IsLayoutFrame() ? static_cast<const SwLayoutFrame*>(pFrame) : nullptr ) ||
             pLayoutFrame->IsAnLower( pAnchor ) );

    return pLayoutFrame;
}

/**
 * Walk back inside the tree: grab the subordinate Frame if one exists and the
 * last step was not moving up a level (this would lead to an infinite up/down
 * loop!). With this we ensure that during walking back we search through all
 * sub trees. If we walked downwards we have to go to the end of the chain first
 * because we go backwards from the last Frame inside another Frame. Walking
 * forward works the same.
 *
 * @warning fixes here may also need to be applied to the @{lcl_NextFrame} method above
 */
const SwContentFrame* SwContentFrame::ImplGetNextContentFrame( bool bFwd ) const
{
    const SwFrame *pFrame = this;
    const SwContentFrame *pContentFrame = nullptr;
    bool bGoingUp = false;
    do {
        const SwFrame *p = nullptr;
        bool bGoingFwdOrBwd = false;

        bool bGoingDown = !bGoingUp;
        if (bGoingDown)
        {
            p = lcl_GetLower( pFrame, true ) ;
            bGoingDown = nullptr != p;
        }
        if ( !bGoingDown )
        {
            p = lcl_FindLayoutFrame( pFrame, bFwd );
            bGoingFwdOrBwd = nullptr != p;
            if ( !bGoingFwdOrBwd )
            {
                p = pFrame->GetUpper();
                bGoingUp = nullptr != p;
                if ( !bGoingUp )
                {
                    return nullptr;
                }
            }
        }

        bGoingUp = !(bGoingFwdOrBwd || bGoingDown);
        assert(p);
        if (!bFwd && bGoingDown)
        {
                while ( p->GetNext() )
                    p = p->GetNext();
        }

        pFrame = p;
    } while ( nullptr == (pContentFrame = (pFrame->IsContentFrame() ? static_cast<const SwContentFrame*>(pFrame) : nullptr) ));

    return pContentFrame;
}

SwPageFrame* SwFrame::ImplFindPageFrame()
{
    SwFrame *pRet = this;
    while ( pRet )
    {
        if (pRet->IsInDtor())
            return nullptr;

        if (pRet->IsPageFrame())
            break;

        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrame() )
        {
            // #i28701# - use new method <GetPageFrame()>
            const auto pFly(static_cast<SwFlyFrame*>(pRet));
            pRet = pFly->GetPageFrame();
            if (pRet == nullptr)
                pRet = pFly->AnchorFrame();
        }
        else
            return nullptr;
    }
    return static_cast<SwPageFrame*>(pRet);
}

SwFootnoteBossFrame* SwFrame::FindFootnoteBossFrame( bool bFootnotes )
{
    SwFrame *pRet = this;
    // Footnote bosses can't exist inside a table; also sections with columns
    // don't contain footnote texts there
    if (SwFrame *pTabFrame = pRet->IsInTab() ? pRet->FindTabFrame() : nullptr)
        pRet = pTabFrame;

    // tdf139336: put the footnotes into the page frame (instead of a column frame)
    // to avoid maximizing the section to the full page.... if:
    // - it is in a section
    // - collect footnotes at section end (FootnoteAtEnd) is not set
    // - columns are evenly distributed (=balanced) is not set
    // Note 1: at page end, the footnotes have no multi-column-capability,
    //         so this fix is used only where there is better chance to help
    // Note 2: If balanced is not set, there is a higher chance that the 1. column will reach
    //         the end of the page... if that happens the section will be maximized anyway.
    // Note 3: The user will be able to easily choose the old layout (with multi-column footnotes)
    //         with this setting.
    //         similar case can be reached with a page break + FootnoteAtEnd setting
    SwSectionFrame* pSectframe = pRet->FindSctFrame();
    bool bMoveToPageFrame = false;
    // tdf146704: only if it is really a footnote, not an endnote.
    // tdf54465: compatibility flag to make old odt files keep these full page sections.
    if (bFootnotes && pSectframe
        && pSectframe->GetFormat()->getIDocumentSettingAccess().get(
            DocumentSettingId::FOOTNOTE_IN_COLUMN_TO_PAGEEND))
    {
        SwSection* pSect = pSectframe->GetSection();
        if (pSect) {
            bool bNoBalance = pSect->GetFormat()->GetBalancedColumns().GetValue();
            bool bFAtEnd = pSectframe->IsFootnoteAtEnd();
            bMoveToPageFrame = !bFAtEnd && !bNoBalance;
        }
    }
    while (pRet && !pRet->IsInDtor()
           && ((!bMoveToPageFrame && !pRet->IsFootnoteBossFrame())
               || (bMoveToPageFrame && !pRet->IsPageFrame())))
    {
        if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrame() )
        {
            // #i28701# - use new method <GetPageFrame()>
            if ( static_cast<SwFlyFrame*>(pRet)->GetPageFrame() )
                pRet = static_cast<SwFlyFrame*>(pRet)->GetPageFrame();
            else
                pRet = static_cast<SwFlyFrame*>(pRet)->AnchorFrame();
        }
        else
            return nullptr;
    }
    if( bFootnotes && pRet && pRet->IsColumnFrame() &&
        !pRet->GetNext() && !pRet->GetPrev() )
    {
        SwSectionFrame* pSct = pRet->FindSctFrame();
        assert(pSct && "FindFootnoteBossFrame: Single column outside section?");
        if( !pSct->IsFootnoteAtEnd() )
            return pSct->FindFootnoteBossFrame( true );
    }
    return static_cast<SwFootnoteBossFrame*>(pRet);
}

SwTabFrame* SwFrame::ImplFindTabFrame()
{
    SwFrame *pRet = this;
    if (pRet->IsInDtor())
        return nullptr;
    while ( !pRet->IsTabFrame() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return nullptr;
        if (pRet->IsInDtor())
            return nullptr;
    }
    return static_cast<SwTabFrame*>(pRet);
}

SwSectionFrame* SwFrame::ImplFindSctFrame()
{
    SwFrame *pRet = this;
    if (pRet->IsInDtor())
        return nullptr;
    while ( !pRet->IsSctFrame() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return nullptr;
        if (pRet->IsInDtor())
            return nullptr;
    }
    return static_cast<SwSectionFrame*>(pRet);
}

const SwBodyFrame* SwFrame::ImplFindBodyFrame() const
{
    const SwFrame *pRet = this;
    while ( !pRet->IsBodyFrame() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return nullptr;
    }
    return static_cast<const SwBodyFrame*>(pRet);
}

SwFootnoteFrame *SwFrame::ImplFindFootnoteFrame()
{
    SwFrame *pRet = this;
    while ( !pRet->IsFootnoteFrame() )
    {
        pRet = pRet->GetUpper();
        if ( !pRet )
            return nullptr;
    }
    return static_cast<SwFootnoteFrame*>(pRet);
}

SwFlyFrame *SwFrame::ImplFindFlyFrame()
{
    SwFrame *pRet = this;
    do
    {
        if ( pRet->IsFlyFrame() )
            return static_cast<SwFlyFrame*>(pRet);
        else
            pRet = pRet->GetUpper();
    } while ( pRet );
    return nullptr;
}

SwFrame *SwFrame::FindColFrame()
{
    SwFrame *pFrame = this;
    do
    {   pFrame = pFrame->GetUpper();
    } while ( pFrame && !pFrame->IsColumnFrame() );
    return pFrame;
}

SwRowFrame *SwFrame::FindRowFrame()
{
    SwFrame *pFrame = this;
    do
    {   pFrame = pFrame->GetUpper();
    } while ( pFrame && !pFrame->IsRowFrame() );
    return dynamic_cast< SwRowFrame* >( pFrame );
}

SwFrame* SwFrame::FindFooterOrHeader()
{
    SwFrame* pRet = this;
    do
    {
        if (pRet->GetType() & FRM_HEADFOOT) //header and footer
            return pRet;
        else if ( pRet->GetUpper() )
            pRet = pRet->GetUpper();
        else if ( pRet->IsFlyFrame() )
            pRet = static_cast<SwFlyFrame*>(pRet)->AnchorFrame();
        else
            return nullptr;
    } while ( pRet );
    return pRet;
}

const SwFootnoteFrame* SwFootnoteContFrame::FindFootNote() const
{
    const SwFootnoteFrame* pRet = static_cast<const SwFootnoteFrame*>(Lower());
    if( pRet && !pRet->GetAttr()->GetFootnote().IsEndNote() )
        return pRet;
    return nullptr;
}

const SwPageFrame* SwRootFrame::GetPageAtPos( const Point& rPt, const Size* pSize, bool bExtend ) const
{
    const SwPageFrame* pRet = nullptr;

    SwRect aRect;
    if ( pSize )
    {
        aRect.Pos()  = rPt;
        aRect.SSize( *pSize );
    }

    const SwFrame* pPage = Lower();

    if ( !bExtend )
    {
        if( !getFrameArea().Contains( rPt ) )
            return nullptr;

        // skip pages above point:
        while( pPage && rPt.Y() > pPage->getFrameArea().Bottom() )
            pPage = pPage->GetNext();
    }

    OSL_ENSURE( GetPageNum() <= maPageRects.size(), "number of pages differs from page rect array size" );
    size_t nPageIdx = 0;

    while ( pPage && !pRet )
    {
        const SwRect& rBoundRect = bExtend ? maPageRects[ nPageIdx++ ] : pPage->getFrameArea();

        if ( (!pSize && rBoundRect.Contains(rPt)) ||
              (pSize && rBoundRect.Overlaps(aRect)) )
        {
            pRet = static_cast<const SwPageFrame*>(pPage);
        }

        pPage = pPage->GetNext();
    }

    return pRet;
}

bool SwRootFrame::IsBetweenPages(const Point& rPt) const
{
    if (!getFrameArea().Contains(rPt))
        return false;

    // top visible page
    const SwFrame* pPage = Lower();
    if (pPage == nullptr)
        return false;

    // skip pages above point:
    while (pPage && rPt.Y() > pPage->getFrameArea().Bottom())
        pPage = pPage->GetNext();

    if (pPage &&
        rPt.X() >= pPage->getFrameArea().Left() &&
        rPt.X() <= pPage->getFrameArea().Right())
    {
        // Trivial case when we're right in between.
        if (!pPage->getFrameArea().Contains(rPt))
            return true;

        // In normal mode the gap is large enough and
        // header/footer mouse interaction competes with
        // handling hide-whitespace within them.
        // In hide-whitespace, however, the gap is too small
        // for convenience and there are no headers/footers.
        const SwViewShell *pSh = GetCurrShell();
        if (pSh && pSh->GetViewOptions()->IsWhitespaceHidden())
        {
            constexpr SwTwips constMargin = o3tl::convert(tools::Long(2), o3tl::Length::mm, o3tl::Length::twip);

            // If we are really close to the bottom or top of a page.
            const auto toEdge = std::min(std::abs(pPage->getFrameArea().Top() - rPt.Y()),
                                         std::abs(pPage->getFrameArea().Bottom() - rPt.Y()));
            return toEdge <= constMargin;
        }
    }

    return false;
}

const SvxFormatBreakItem& SwFrame::GetBreakItem() const
{
    return GetAttrSet()->GetBreak();
}

const SwFormatPageDesc& SwFrame::GetPageDescItem() const
{
    return GetAttrSet()->GetPageDesc();
}

const SvxFormatBreakItem& SwTextFrame::GetBreakItem() const
{
    return GetTextNodeFirst()->GetSwAttrSet().GetBreak();
}

const SwFormatPageDesc& SwTextFrame::GetPageDescItem() const
{
    return GetTextNodeFirst()->GetSwAttrSet().GetPageDesc();
}

const SwAttrSet* SwFrame::GetAttrSet() const
{
    if (IsTextFrame())
    {
        return &static_cast<const SwTextFrame*>(this)->GetTextNodeForParaProps()->GetSwAttrSet();
    }
    else if (IsNoTextFrame())
    {
        return &static_cast<const SwNoTextFrame*>(this)->GetNode()->GetSwAttrSet();
    }
    else
    {
        assert(IsLayoutFrame());
        return &static_cast<const SwLayoutFrame*>(this)->GetFormat()->GetAttrSet();
    }
}

drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFrame::getSdrAllFillAttributesHelper() const
{
    if (IsTextFrame())
    {
        return static_cast<const SwTextFrame*>(this)->GetTextNodeForParaProps()->getSdrAllFillAttributesHelper();
    }
    else if (IsNoTextFrame())
    {
        return static_cast<const SwNoTextFrame*>(this)->GetNode()->getSdrAllFillAttributesHelper();
    }
    else
    {
        return static_cast< const SwLayoutFrame* >(this)->GetFormat()->getSdrAllFillAttributesHelper();
    }
}

bool SwFrame::supportsFullDrawingLayerFillAttributeSet() const
{
    if (IsContentFrame())
    {
        return true;
    }
    else
    {
        return static_cast< const SwLayoutFrame* >(this)->GetFormat()->supportsFullDrawingLayerFillAttributeSet();
    }
}

/*
 *  SwFrame::FindNext_(), FindPrev_(), InvalidateNextPos()
 *         FindNextCnt_() visits tables and sections and only returns SwContentFrames.
 *
 *  Description         Invalidates the position of the next frame.
 *      This is the direct successor or in case of ContentFrames the next
 *      ContentFrame which sits in the same flow as I do:
 *      - body,
 *      - footnote,
 *      - in headers/footers the notification only needs to be forwarded
 *        inside the section
 *      - same for Flys
 *      - Contents in tabs remain only inside their cell
 *      - in principle tables behave exactly like the Contents
 *      - sections also
 */
// This helper function is an equivalent to the ImplGetNextContentFrame() method,
// besides ContentFrames this function also returns TabFrames and SectionFrames.
static SwFrame* lcl_NextFrame( SwFrame* pFrame )
{
    SwFrame *pRet = nullptr;
    bool bGoingUp = false;
    do {
        SwFrame *p = nullptr;

        bool bGoingFwd = false;
        bool bGoingDown = !bGoingUp && pFrame->IsLayoutFrame();
        if (bGoingDown)
        {
            p = static_cast<SwLayoutFrame*>(pFrame)->Lower();
            bGoingDown = nullptr != p;
        }
        if( !bGoingDown )
        {
            p = pFrame->IsFlyFrame() ? static_cast<SwFlyFrame*>(pFrame)->GetNextLink() : pFrame->GetNext();
            bGoingFwd = nullptr != p;
            if ( !bGoingFwd )
            {
                p = pFrame->GetUpper();
                bGoingUp = nullptr != p;
                if ( !bGoingUp )
                {
                    return nullptr;
                }
            }
        }
        bGoingUp = !(bGoingFwd || bGoingDown);
        pFrame = p;
    } while ( nullptr == (pRet = ( ( pFrame->IsContentFrame() || ( !bGoingUp &&
            ( pFrame->IsTabFrame() || pFrame->IsSctFrame() ) ) )? pFrame : nullptr ) ) );
    return pRet;
}

SwFrame *SwFrame::FindNext_()
{
    bool bIgnoreTab = false;
    SwFrame *pThis = this;

    if ( IsTabFrame() )
    {
        //The last Content of the table gets picked up and his follower is
        //returned. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( static_cast<SwTabFrame*>(this)->GetFollow() )
            return static_cast<SwTabFrame*>(this)->GetFollow();

        pThis = static_cast<SwTabFrame*>(this)->FindLastContentOrTable();
        if ( !pThis )
            pThis = this;
        bIgnoreTab = true;
    }
    else if ( IsSctFrame() )
    {
        //The last Content of the section gets picked and his follower is returned.
        if ( static_cast<SwSectionFrame*>(this)->GetFollow() )
            return static_cast<SwSectionFrame*>(this)->GetFollow();

        pThis = static_cast<SwSectionFrame*>(this)->FindLastContent();
        if ( !pThis )
            pThis = this;
    }
    else if ( IsContentFrame() )
    {
        if( static_cast<SwContentFrame*>(this)->GetFollow() )
            return static_cast<SwContentFrame*>(this)->GetFollow();
    }
    else if ( IsRowFrame() )
    {
        SwFrame* pMyUpper = GetUpper();
        if ( pMyUpper->IsTabFrame() && static_cast<SwTabFrame*>(pMyUpper)->GetFollow() )
            return static_cast<SwTabFrame*>(pMyUpper)->GetFollow()->GetLower();
        else return nullptr;
    }
    else
        return nullptr;

    SwFrame* pRet = nullptr;
    const bool bFootnote  = pThis->IsInFootnote();
    if ( !bIgnoreTab && pThis->IsInTab() )
    {
        SwLayoutFrame *pUp = pThis->GetUpper();
        while (pUp && !pUp->IsCellFrame())
            pUp = pUp->GetUpper();
        assert(pUp && "Content flag says it's in table but it's not in cell.");
        SwFrame* pNxt = pUp ? static_cast<SwCellFrame*>(pUp)->GetFollowCell() : nullptr;
        if ( pNxt )
            pNxt = static_cast<SwCellFrame*>(pNxt)->ContainsContent();
        if ( !pNxt )
        {
            pNxt = lcl_NextFrame( pThis );
            if (pUp && pUp->IsAnLower(pNxt))
                pRet = pNxt;
        }
        else
            pRet = pNxt;
    }
    else
    {
        const bool bBody = pThis->IsInDocBody();
        SwFrame *pNxtCnt = lcl_NextFrame( pThis );
        if ( pNxtCnt )
        {
            if ( bBody || bFootnote )
            {
                while ( pNxtCnt )
                {
                    // OD 02.04.2003 #108446# - check for endnote, only if found
                    // next content isn't contained in a section, that collect its
                    // endnotes at its end.
                    bool bEndn = IsInSct() && !IsSctFrame() &&
                                 ( !pNxtCnt->IsInSct() ||
                                   !pNxtCnt->FindSctFrame()->IsEndnAtEnd()
                                 );
                    if ( ( bBody && pNxtCnt->IsInDocBody() ) ||
                         ( pNxtCnt->IsInFootnote() &&
                           ( bFootnote ||
                             ( bEndn && pNxtCnt->FindFootnoteFrame()->GetAttr()->GetFootnote().IsEndNote() )
                           )
                         )
                       )
                    {
                        if (pNxtCnt->IsInTab())
                        {
                            if (!IsTabFrame() || !static_cast<SwLayoutFrame*>(this)->IsAnLower(pNxtCnt))
                            {
                                pRet = pNxtCnt->FindTabFrame();
                                break;
                            }
                        }
                        else
                        {
                            pRet = pNxtCnt;
                            break;
                        }
                    }
                    pNxtCnt = lcl_NextFrame( pNxtCnt );
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrame()
                                            : pNxtCnt;
            }
            else    //footer-/or header section
            {
                const SwFrame *pUp = pThis->GetUpper();
                const SwFrame *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrame() && !pUp->IsFooterFrame() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrame() && !pCntUp->IsFooterFrame() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pNxtCnt->IsInTab() ? pNxtCnt->FindTabFrame()
                                                : pNxtCnt;
                }
            }
        }
    }
    if( pRet && pRet->IsInSct() )
    {
        SwSectionFrame* pSct = pRet->FindSctFrame();
        //Footnotes in frames with columns must not return the section which
        //contains the footnote
        if( !pSct->IsAnLower( this ) &&
            (!bFootnote || pSct->IsInFootnote() ) )
            return pSct;
    }
    return pRet == this ? nullptr : pRet;
}

// #i27138# - add parameter <_bInSameFootnote>
SwContentFrame *SwFrame::FindNextCnt_( const bool _bInSameFootnote )
{
    SwFrame *pThis = this;

    if ( IsTabFrame() )
    {
        if ( static_cast<SwTabFrame*>(this)->GetFollow() )
        {
            pThis = static_cast<SwTabFrame*>(this)->GetFollow()->ContainsContent();
            if( pThis )
                return static_cast<SwContentFrame*>(pThis);
        }
        pThis = static_cast<SwTabFrame*>(this)->FindLastContentOrTable();
        if ( !pThis )
            return nullptr;
    }
    else if ( IsSctFrame() )
    {
        if ( static_cast<SwSectionFrame*>(this)->GetFollow() )
        {
            pThis = static_cast<SwSectionFrame*>(this)->GetFollow()->ContainsContent();
            if( pThis )
                return static_cast<SwContentFrame*>(pThis);
        }
        pThis = static_cast<SwSectionFrame*>(this)->FindLastContent();
        if ( !pThis )
            return nullptr;
    }
    else if ( IsContentFrame() && static_cast<SwContentFrame*>(this)->GetFollow() )
        return static_cast<SwContentFrame*>(this)->GetFollow();

    if ( pThis->IsContentFrame() )
    {
        const bool bBody = pThis->IsInDocBody();
        const bool bFootnote  = pThis->IsInFootnote();
        SwContentFrame *pNxtCnt = static_cast<SwContentFrame*>(pThis)->GetNextContentFrame();
        if ( pNxtCnt )
        {
            // #i27138#
            if ( bBody || ( bFootnote && !_bInSameFootnote ) )
            {
                // handling for environments 'footnotes' and 'document body frames':
                while ( pNxtCnt )
                {
                    if ( (bBody && pNxtCnt->IsInDocBody()) ||
                         (bFootnote  && pNxtCnt->IsInFootnote()) )
                        return pNxtCnt;
                    pNxtCnt = pNxtCnt->GetNextContentFrame();
                }
            }
            // #i27138#
            else if ( bFootnote && _bInSameFootnote )
            {
                // handling for environments 'each footnote':
                // Assure that found next content frame belongs to the same footnotes
                const SwFootnoteFrame* pFootnoteFrameOfNext( pNxtCnt->FindFootnoteFrame() );
                SwFootnoteFrame* pFootnoteFrameOfCurr(pThis->FindFootnoteFrame());
                OSL_ENSURE( pFootnoteFrameOfCurr,
                        "<SwFrame::FindNextCnt_() - unknown layout situation: current frame has to have an upper footnote frame." );
                if ( pFootnoteFrameOfNext == pFootnoteFrameOfCurr )
                {
                    return pNxtCnt;
                }
                else if ( pFootnoteFrameOfCurr->GetFollow() )
                {
                    // next content frame has to be the first content frame
                    // in the follow footnote, which contains a content frame.
                    SwFootnoteFrame* pFollowFootnoteFrameOfCurr(pFootnoteFrameOfCurr);
                    pNxtCnt = nullptr;
                    do {
                        pFollowFootnoteFrameOfCurr = pFollowFootnoteFrameOfCurr->GetFollow();
                        pNxtCnt = pFollowFootnoteFrameOfCurr->ContainsContent();
                    } while ( !pNxtCnt && pFollowFootnoteFrameOfCurr->GetFollow() );
                    return pNxtCnt;
                }
                else
                {
                    // current content frame is the last content frame in the
                    // footnote - no next content frame exists.
                    return nullptr;
                }
            }
            else if ( pThis->IsInFly() )
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                return pNxtCnt;
            else
            {
                // handling for environments 'page header' and 'page footer':
                const SwFrame *pUp = pThis->GetUpper();
                const SwFrame *pCntUp = pNxtCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrame() && !pUp->IsFooterFrame() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() &&
                        !pCntUp->IsHeaderFrame() && !pCntUp->IsFooterFrame() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                    return pNxtCnt;
            }
        }
    }
    return nullptr;
}

/** method to determine previous content frame in the same environment
    for a flow frame (content frame, table frame, section frame)

    OD 2005-11-30 #i27138#
*/
SwContentFrame* SwFrame::FindPrevCnt_()
{
    if ( !IsFlowFrame() )
    {
        // nothing to do, if current frame isn't a flow frame.
        return nullptr;
    }

    SwContentFrame* pPrevContentFrame( nullptr );

    // Because method <SwContentFrame::GetPrevContentFrame()> is used to travel
    // through the layout, a content frame, at which the travel starts, is needed.
    SwContentFrame* pCurrContentFrame = dynamic_cast<SwContentFrame*>(this);

    // perform shortcut, if current frame is a follow, and
    // determine <pCurrContentFrame>, if current frame is a table or section frame
    if ( pCurrContentFrame && pCurrContentFrame->IsFollow() )
    {
        // previous content frame is its master content frame
        pPrevContentFrame = pCurrContentFrame->FindMaster();
    }
    else if ( IsTabFrame() )
    {
        SwTabFrame* pTabFrame( static_cast<SwTabFrame*>(this) );
        if ( pTabFrame->IsFollow() )
        {
            // previous content frame is the last content of its master table frame
            pPrevContentFrame = pTabFrame->FindMaster()->FindLastContent();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the table frame.
            pCurrContentFrame = pTabFrame->ContainsContent();
        }
    }
    else if ( IsSctFrame() )
    {
        SwSectionFrame* pSectFrame( static_cast<SwSectionFrame*>(this) );
        if ( pSectFrame->IsFollow() )
        {
            // previous content frame is the last content of its master section frame
            pPrevContentFrame = pSectFrame->FindMaster()->FindLastContent();
        }
        else
        {
            // start content frame for the search is the first content frame of
            // the section frame.
            pCurrContentFrame = pSectFrame->ContainsContent();
        }
    }

    // search for next content frame, depending on the environment, in which
    // the current frame is in.
    if ( !pPrevContentFrame && pCurrContentFrame )
    {
        pPrevContentFrame = pCurrContentFrame->GetPrevContentFrame();
        if ( pPrevContentFrame )
        {
            if ( pCurrContentFrame->IsInFly() )
            {
                // handling for environments 'unlinked fly frame' and
                // 'group of linked fly frames':
                // Nothing to do, <pPrevContentFrame> is the one
            }
            else
            {
                const bool bInDocBody = pCurrContentFrame->IsInDocBody();
                const bool bInFootnote  = pCurrContentFrame->IsInFootnote();
                if ( bInDocBody )
                {
                    // handling for environments 'footnotes' and 'document body frames':
                    // Assure that found previous frame is also in one of these
                    // environments. Otherwise, travel further
                    while ( pPrevContentFrame )
                    {
                        if ( ( bInDocBody && pPrevContentFrame->IsInDocBody() ) ||
                             ( bInFootnote && pPrevContentFrame->IsInFootnote() ) )
                        {
                            break;
                        }
                        pPrevContentFrame = pPrevContentFrame->GetPrevContentFrame();
                    }
                }
                else if ( bInFootnote )
                {
                    // handling for environments 'each footnote':
                    // Assure that found next content frame belongs to the same footnotes
                    const SwFootnoteFrame* pFootnoteFrameOfPrev( pPrevContentFrame->FindFootnoteFrame() );
                    SwFootnoteFrame* pFootnoteFrameOfCurr(pCurrContentFrame->FindFootnoteFrame());
                    if ( pFootnoteFrameOfPrev != pFootnoteFrameOfCurr )
                    {
                        if ( pFootnoteFrameOfCurr->GetMaster() )
                        {
                            SwFootnoteFrame* pMasterFootnoteFrameOfCurr(pFootnoteFrameOfCurr);
                            pPrevContentFrame = nullptr;
                            // correct wrong loop-condition
                            do {
                                pMasterFootnoteFrameOfCurr = pMasterFootnoteFrameOfCurr->GetMaster();
                                pPrevContentFrame = pMasterFootnoteFrameOfCurr->FindLastContent();
                            } while ( !pPrevContentFrame &&
                                      pMasterFootnoteFrameOfCurr->GetMaster() );
                        }
                        else
                        {
                            // current content frame is the first content in the
                            // footnote - no previous content exists.
                            pPrevContentFrame = nullptr;
                        }
                    }
                }
                else
                {
                    // handling for environments 'page header' and 'page footer':
                    // Assure that found previous frame is also in the same
                    // page header respectively page footer as <pCurrContentFrame>
                    // Note: At this point it's clear that <pCurrContentFrame> has
                    //       to be inside a page header or page footer and that
                    //       neither <pCurrContentFrame> nor <pPrevContentFrame> are
                    //       inside a fly frame.
                    //       Thus, method <FindFooterOrHeader()> can be used.
                    OSL_ENSURE( pCurrContentFrame->FindFooterOrHeader(),
                            "<SwFrame::FindPrevCnt_()> - unknown layout situation: current frame should be in page header or page footer" );
                    OSL_ENSURE( !pPrevContentFrame->IsInFly(),
                            "<SwFrame::FindPrevCnt_()> - unknown layout situation: found previous frame should *not* be inside a fly frame." );
                    if ( pPrevContentFrame->FindFooterOrHeader() !=
                                            pCurrContentFrame->FindFooterOrHeader() )
                    {
                        pPrevContentFrame = nullptr;
                    }
                }
            }
        }
    }

    return pPrevContentFrame;
}

SwFrame *SwFrame::FindPrev_()
{
    bool bIgnoreTab = false;
    SwFrame *pThis = this;

    if ( IsTabFrame() )
    {
        //The first Content of the table gets picked up and his predecessor is
        //returned. To be able to deactivate the special case for tables
        //(see below) bIgnoreTab will be set.
        if ( static_cast<SwTabFrame*>(this)->IsFollow() )
            return static_cast<SwTabFrame*>(this)->FindMaster();
        else
            pThis = static_cast<SwTabFrame*>(this)->ContainsContent();
        bIgnoreTab = true;
    }

    if ( pThis && pThis->IsContentFrame() )
    {
        SwContentFrame *pPrvCnt = static_cast<SwContentFrame*>(pThis)->GetPrevContentFrame();
        if( !pPrvCnt )
            return nullptr;
        if ( !bIgnoreTab && pThis->IsInTab() )
        {
            SwLayoutFrame *pUp = pThis->GetUpper();
            while (pUp && !pUp->IsCellFrame())
                pUp = pUp->GetUpper();
            assert(pUp && "Content flag says it's in table but it's not in cell.");
            if (pUp && pUp->IsAnLower(pPrvCnt))
                return pPrvCnt;
        }
        else
        {
            SwFrame* pRet;
            const bool bBody = pThis->IsInDocBody();
            const bool bFootnote  = !bBody && pThis->IsInFootnote();
            if ( bBody || bFootnote )
            {
                while ( pPrvCnt )
                {
                    if ( (bBody && pPrvCnt->IsInDocBody()) ||
                            (bFootnote   && pPrvCnt->IsInFootnote()) )
                    {
                        pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrame()
                                                  : static_cast<SwFrame*>(pPrvCnt);
                        return pRet;
                    }
                    pPrvCnt = pPrvCnt->GetPrevContentFrame();
                }
            }
            else if ( pThis->IsInFly() )
            {
                pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrame()
                                            : static_cast<SwFrame*>(pPrvCnt);
                return pRet;
            }
            else // footer or header or Fly
            {
                const SwFrame *pUp = pThis->GetUpper();
                const SwFrame *pCntUp = pPrvCnt->GetUpper();
                while ( pUp && pUp->GetUpper() &&
                        !pUp->IsHeaderFrame() && !pUp->IsFooterFrame() )
                    pUp = pUp->GetUpper();
                while ( pCntUp && pCntUp->GetUpper() )
                    pCntUp = pCntUp->GetUpper();
                if ( pCntUp == pUp )
                {
                    pRet = pPrvCnt->IsInTab() ? pPrvCnt->FindTabFrame()
                                                : static_cast<SwFrame*>(pPrvCnt);
                    return pRet;
                }
            }
        }
    }
    return nullptr;
}

void SwFrame::ImplInvalidateNextPos( bool bNoFootnote )
{
    SwFrame *pFrame = FindNext_();
    if ( nullptr == pFrame )
        return;

    if( pFrame->IsSctFrame() )
    {
        while( pFrame && pFrame->IsSctFrame() )
        {
            if( static_cast<SwSectionFrame*>(pFrame)->GetSection() )
            {
                SwFrame* pTmp = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                if( pTmp )
                    pTmp->InvalidatePos();
                else if( !bNoFootnote )
                    static_cast<SwSectionFrame*>(pFrame)->InvalidateFootnotePos();
                if( !IsInSct() || FindSctFrame()->GetFollow() != pFrame )
                    pFrame->InvalidatePos();
                return;
            }
            pFrame = pFrame->FindNext();
        }
        if( pFrame )
        {
            if ( pFrame->IsSctFrame())
            {
                // We need to invalidate the section's content so it gets
                // the chance to flow to a different page.
                SwFrame* pTmp = static_cast<SwSectionFrame*>(pFrame)->ContainsAny();
                if( pTmp )
                    pTmp->InvalidatePos();
                if( !IsInSct() || FindSctFrame()->GetFollow() != pFrame )
                    pFrame->InvalidatePos();
            }
            else
                pFrame->InvalidatePos();
        }
    }
    else
        pFrame->InvalidatePos();
}

/** method to invalidate printing area of next frame

    OD 09.01.2004 #i11859#

    FME 2004-04-19 #i27145# Moved function from SwTextFrame to SwFrame
*/
void SwFrame::InvalidateNextPrtArea()
{
    // determine next frame
    SwFrame* pNextFrame = FindNext();
    // skip empty section frames and hidden text frames
    {
        while (pNextFrame && pNextFrame->IsHiddenNow())
        {
            pNextFrame = pNextFrame->FindNext();
        }
    }

    // Invalidate printing area of found next frame
    if ( !pNextFrame )
        return;

    if ( pNextFrame->IsSctFrame() )
    {
        // Invalidate printing area of found section frame, if
        // (1) this text frame isn't in a section OR
        // (2) found section frame isn't a follow of the section frame this
        //     text frame is in.
        if ( !IsInSct() || FindSctFrame()->GetFollow() != pNextFrame )
        {
            pNextFrame->InvalidatePrt();
        }

        // Invalidate printing area of first content in found section.
        SwFrame* pFstContentOfSctFrame =
                static_cast<SwSectionFrame*>(pNextFrame)->ContainsAny();
        if ( pFstContentOfSctFrame )
        {
            pFstContentOfSctFrame->InvalidatePrt();
        }
    }
    else
    {
        pNextFrame->InvalidatePrt();
    }
}

/// @returns true if the frame _directly_ sits in a section
///     but not if it sits in a table which itself sits in a section.
static bool lcl_IsInSectionDirectly( const SwFrame *pUp )
{
    bool bSeenColumn = false;

    while( pUp )
    {
        if( pUp->IsColumnFrame() )
            bSeenColumn = true;
        else if( pUp->IsSctFrame() )
        {
            auto pSection = static_cast<const SwSectionFrame*>(pUp);
            const SwFrame* pHeaderFooter = pSection->FindFooterOrHeader();
            // When the section frame is not in header/footer:
            // Allow move of frame in case our only column is not growable.
            // Also allow if there is a previous section frame (to move back).
            bool bAllowOutsideHeaderFooter = !pSection->Growable() || pSection->GetPrecede();
            return bSeenColumn || (!pHeaderFooter && bAllowOutsideHeaderFooter);
        }
        else if( pUp->IsTabFrame() )
            return false;
        pUp = pUp->GetUpper();
    }
    return false;
}

/** determine, if frame is moveable in given environment

    OD 08.08.2003 #110978#
    method replaced 'old' method <sal_Bool IsMoveable() const>.
    Determines, if frame is moveable in given environment. if no environment
    is given (parameter _pLayoutFrame == 0), the movability in the actual
    environment (<GetUpper()) is checked.
*/
bool SwFrame::IsMoveable( const SwLayoutFrame* _pLayoutFrame ) const
{
    bool bRetVal = false;

    if ( !_pLayoutFrame )
    {
        _pLayoutFrame = GetUpper();
    }

    if ( _pLayoutFrame && IsFlowFrame() )
    {
        if ( _pLayoutFrame->IsInSct() && lcl_IsInSectionDirectly( _pLayoutFrame ) )
        {
            bRetVal = true;
        }
        else if ( _pLayoutFrame->IsInFly() ||
                  _pLayoutFrame->IsInDocBody() ||
                  _pLayoutFrame->IsInFootnote() )
        {
            // If IsMovable() is called before a MoveFwd() the method
            // may return false if there is no NextCellLeaf. If
            // IsMovable() is called before a MoveBwd() the method may
            // return false if there is no PrevCellLeaf.
            if ( _pLayoutFrame->IsInTab() && !IsTabFrame() &&
                 ( !IsContentFrame() || (!const_cast<SwFrame*>(this)->GetNextCellLeaf()
                                      && !const_cast<SwFrame*>(this)->GetPrevCellLeaf()) )
                )
            {
                bRetVal = false;
            }
            else
            {
                if ( _pLayoutFrame->IsInFly() )
                {
                    // if fly frame has a follow (next linked fly frame) or can split,
                    // frame is moveable.
                    const SwFlyFrame* pFlyFrame = _pLayoutFrame->FindFlyFrame();
                    if ( pFlyFrame->GetNextLink() || pFlyFrame->IsFlySplitAllowed() )
                    {
                        bRetVal = true;
                    }
                    else
                    {
                        // if environment is columned, frame is moveable, if
                        // it isn't in last column.
                        // search for column frame
                        const SwFrame* pCol = _pLayoutFrame;
                        while ( pCol && !pCol->IsColumnFrame() )
                        {
                            pCol = pCol->GetUpper();
                        }
                        // frame is moveable, if found column frame isn't last one.
                        if ( pCol && pCol->GetNext() )
                        {
                            bRetVal = true;
                        }
                    }
                }
                else if (!(_pLayoutFrame->IsInFootnote() && (IsTabFrame() || IsInTab())))
                {
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

void SwFrame::SetInfFlags()
{
    if ( !IsFlyFrame() && !GetUpper() ) //not yet pasted, no information available
        return;

    mbInfInvalid = mbInfBody = mbInfTab = mbInfFly = mbInfFootnote = mbInfSct = false;

    SwFrame *pFrame = this;
    if( IsFootnoteContFrame() )
        mbInfFootnote = true;
    do
    {
        // mbInfBody is only set in the page body, but not in the column body
        if ( pFrame->IsBodyFrame() && !mbInfFootnote && pFrame->GetUpper()
             && pFrame->GetUpper()->IsPageFrame() )
            mbInfBody = true;
        else if ( pFrame->IsTabFrame() || pFrame->IsCellFrame() )
        {
            mbInfTab = true;
        }
        else if ( pFrame->IsFlyFrame() )
            mbInfFly = true;
        else if ( pFrame->IsSctFrame() )
            mbInfSct = true;
        else if ( pFrame->IsFootnoteFrame() )
            mbInfFootnote = true;

        pFrame = pFrame->GetUpper();

    } while ( pFrame && !pFrame->IsPageFrame() ); //there is nothing above the page
}

/** Updates the vertical or the righttoleft-flags.
 *
 * If the property is derived, it's from the upper or (for fly frames) from
 * the anchor. Otherwise we've to call a virtual method to check the property.
 */
void SwFrame::SetDirFlags( bool bVert )
{
    if( bVert )
    {
        // OD 2004-01-21 #114969# - if derived, valid vertical flag only if
        // vertical flag of upper/anchor is valid.
        if( mbDerivedVert )
        {
            const SwFrame* pAsk = IsFlyFrame() ?
                          static_cast<SwFlyFrame*>(this)->GetAnchorFrame() : GetUpper();

            OSL_ENSURE( pAsk != this, "Autsch! Stack overflow is about to happen" );

            if( pAsk )
            {
                mbVertical = pAsk->IsVertical();
                mbVertLR  = pAsk->IsVertLR();
                mbVertLRBT = pAsk->IsVertLRBT();

                if ( !pAsk->mbInvalidVert )
                    mbInvalidVert = false;

                if ( IsCellFrame() )
                {
                    SwCellFrame* pPrv = static_cast<SwCellFrame*>(this)->GetPreviousCell();
                    if ( pPrv && !mbVertical && pPrv->IsVertical() )
                    {
                        mbVertical = pPrv->IsVertical();
                        mbVertLR  = pPrv->IsVertLR();
                        mbVertLRBT = pPrv->IsVertLRBT();
                    }
                }
            }
        }
        else
            CheckDirection( bVert );
    }
    else
    {
        bool bInv = false;
        if( !mbDerivedR2L ) // CheckDirection is able to set bDerivedR2L!
            CheckDirection( bVert );
        if( mbDerivedR2L )
        {
            const SwFrame* pAsk = IsFlyFrame() ?
                          static_cast<SwFlyFrame*>(this)->GetAnchorFrame() : GetUpper();

            OSL_ENSURE( pAsk != this, "Oops! Stack overflow is about to happen" );

            if( pAsk )
                mbRightToLeft = pAsk->IsRightToLeft();
            if( !pAsk || pAsk->mbInvalidR2L )
                bInv = mbInvalidR2L;
        }
        mbInvalidR2L = bInv;
    }
}

SwLayoutFrame* SwFrame::GetNextCellLeaf()
{
    SwFrame* pTmpFrame = this;
    while (pTmpFrame && !pTmpFrame->IsCellFrame())
        pTmpFrame = pTmpFrame->GetUpper();

    SAL_WARN_IF(!pTmpFrame, "sw.core", "SwFrame::GetNextCellLeaf() without cell");
    return pTmpFrame ? static_cast<SwCellFrame*>(pTmpFrame)->GetFollowCell() : nullptr;
}

SwLayoutFrame* SwFrame::GetPrevCellLeaf()
{
    SwFrame* pTmpFrame = this;
    while (pTmpFrame && !pTmpFrame->IsCellFrame())
        pTmpFrame = pTmpFrame->GetUpper();

    SAL_WARN_IF(!pTmpFrame, "sw.core", "SwFrame::GetNextPreviousLeaf() without cell");
    return pTmpFrame ? static_cast<SwCellFrame*>(pTmpFrame)->GetPreviousCell() : nullptr;
}

static SwCellFrame* lcl_FindCorrespondingCellFrame( const SwRowFrame& rOrigRow,
                                         const SwCellFrame& rOrigCell,
                                         const SwRowFrame& rCorrRow,
                                         bool bInFollow )
{
    SwCellFrame* pRet = nullptr;
    const SwCellFrame* pCell = static_cast<const SwCellFrame*>(rOrigRow.Lower());
    SwCellFrame* pCorrCell = const_cast<SwCellFrame*>(static_cast<const SwCellFrame*>(rCorrRow.Lower()));

    while ( pCell != &rOrigCell && !pCell->IsAnLower( &rOrigCell ) )
    {
        pCell = static_cast<const SwCellFrame*>(pCell->GetNext());
        pCorrCell = static_cast<SwCellFrame*>(pCorrCell->GetNext());
    }

    assert(pCell && pCorrCell && "lcl_FindCorrespondingCellFrame does not work");

    if ( pCell != &rOrigCell )
    {
        // rOrigCell must be a lower of pCell. We need to recurse into the rows:
        assert(pCell->Lower() && pCell->Lower()->IsRowFrame() &&
               "lcl_FindCorrespondingCellFrame does not work");

        const SwRowFrame* pRow = static_cast<const SwRowFrame*>(pCell->Lower());
        while ( !pRow->IsAnLower( &rOrigCell ) )
            pRow = static_cast<const SwRowFrame*>(pRow->GetNext());

        SwRowFrame* pCorrRow = nullptr;
        if ( bInFollow )
            pCorrRow = pRow->GetFollowRow();
        else
        {
            SwRowFrame* pTmpRow = static_cast<SwRowFrame*>(pCorrCell->GetLastLower());

            if ( pTmpRow && pTmpRow->GetFollowRow() == pRow )
                pCorrRow = pTmpRow;
        }

        if ( pCorrRow )
            pRet = lcl_FindCorrespondingCellFrame( *pRow, rOrigCell, *pCorrRow, bInFollow );
    }
    else
        pRet = pCorrCell;

    return pRet;
}

// VERSION OF GetFollowCell() that assumes that we always have a follow flow line:
SwCellFrame* SwCellFrame::GetFollowCell() const
{
    SwCellFrame* pRet = nullptr;

    // NEW TABLES
    // Covered cells do not have follow cells!
    const tools::Long nRowSpan = GetLayoutRowSpan();
    if ( nRowSpan < 1 )
        return nullptr;

    // find most upper row frame
    const SwFrame* pRow = GetUpper();

    while (pRow && (!pRow->IsRowFrame() || !pRow->GetUpper()->IsTabFrame()))
        pRow = pRow->GetUpper();

    if (!pRow)
        return nullptr;

    const SwTabFrame* pTabFrame = static_cast<const SwTabFrame*>(pRow->GetUpper());
    if (!pTabFrame || !pTabFrame->GetFollow() || !pTabFrame->HasFollowFlowLine())
        return nullptr;

    const SwCellFrame* pThisCell = this;

    // Get last cell of the current table frame that belongs to the rowspan:
    if ( nRowSpan > 1 )
    {
        // optimization: Will end of row span be in last row or exceed row?
        tools::Long nMax = 0;
        while ( pRow->GetNext() && ++nMax < nRowSpan )
            pRow = pRow->GetNext();

        if ( !pRow->GetNext() )
        {
            pThisCell = &pThisCell->FindStartEndOfRowSpanCell( false );
            pRow = pThisCell->GetUpper();
        }
    }

    const SwRowFrame* pFollowRow = nullptr;
    if ( !pRow->GetNext() &&
         nullptr != ( pFollowRow = pRow->IsInSplitTableRow() ) &&
         ( !pFollowRow->IsRowSpanLine() || nRowSpan > 1 ) )
         pRet = lcl_FindCorrespondingCellFrame( *static_cast<const SwRowFrame*>(pRow), *pThisCell, *pFollowRow, true );

    return pRet;
}

// VERSION OF GetPreviousCell() THAT ASSUMES THAT WE ALWAYS HAVE A FFL
SwCellFrame* SwCellFrame::GetPreviousCell() const
{
    SwCellFrame* pRet = nullptr;

    // NEW TABLES
    // Covered cells do not have previous cells!
    if ( GetLayoutRowSpan() < 1 )
        return nullptr;

    // find most upper row frame
    const SwFrame* pRow = GetUpper();
    while( !pRow->IsRowFrame() || (pRow->GetUpper() && !pRow->GetUpper()->IsTabFrame()) )
        pRow = pRow->GetUpper();

    OSL_ENSURE( pRow->GetUpper() && pRow->GetUpper()->IsTabFrame(), "GetPreviousCell without Table" );

    const SwTabFrame* pTab = static_cast<const SwTabFrame*>(pRow->GetUpper());

    if ( pTab && pTab->IsFollow() )
    {
        const SwFrame* pTmp = pTab->GetFirstNonHeadlineRow();
        const bool bIsInFirstLine = ( pTmp == pRow );

        if ( bIsInFirstLine )
        {
            SwTabFrame *pMaster = pTab->FindMaster();
            if ( pMaster && pMaster->HasFollowFlowLine() )
            {
                SwRowFrame* pMasterRow = static_cast<SwRowFrame*>(pMaster->GetLastLower());
                if ( pMasterRow )
                    pRet = lcl_FindCorrespondingCellFrame( *static_cast<const SwRowFrame*>(pRow), *this, *pMasterRow, false );
                if ( pRet && pRet->GetTabBox()->getRowSpan() < 1 )
                    pRet = &const_cast<SwCellFrame&>(pRet->FindStartEndOfRowSpanCell( true ));
            }
        }
    }

    return pRet;
}

// --> NEW TABLES
const SwCellFrame& SwCellFrame::FindStartEndOfRowSpanCell( bool bStart ) const
{
    const SwTabFrame* pTableFrame = dynamic_cast<const SwTabFrame*>(GetUpper()->GetUpper());

    if ( !bStart && pTableFrame && pTableFrame->IsFollow() && pTableFrame->IsInHeadline( *this ) )
        return *this;

    OSL_ENSURE( pTableFrame &&
            (  (bStart && GetTabBox()->getRowSpan() < 1) ||
              (!bStart && GetLayoutRowSpan() > 1) ),
            "SwCellFrame::FindStartRowSpanCell: No rowspan, no table, no cookies" );

    if ( pTableFrame )
    {
        const SwTable* pTable = pTableFrame->GetTable();

        sal_uInt16 nMax = USHRT_MAX;
        const SwFrame* pCurrentRow = GetUpper();
        const bool bDoNotEnterHeadline = bStart && pTableFrame->IsFollow() &&
                                        !pTableFrame->IsInHeadline( *pCurrentRow );

        // check how many rows we are allowed to go up or down until we reach the end of
        // the current table frame:
        nMax = 0;
        while ( bStart ? pCurrentRow->GetPrev() : pCurrentRow->GetNext() )
        {
            if ( bStart )
            {
                // do not enter a repeated headline:
                if ( bDoNotEnterHeadline && pTableFrame->IsFollow() &&
                     pTableFrame->IsInHeadline( *pCurrentRow->GetPrev() ) )
                    break;

                pCurrentRow = pCurrentRow->GetPrev();
            }
            else
               pCurrentRow = pCurrentRow->GetNext();

            ++nMax;
        }

        // By passing the nMax value for Find*OfRowSpan (in case of bCurrentTableOnly
        // is set) we assure that we find a rMasterBox that has a SwCellFrame in
        // the current table frame:
        const SwTableBox& rMasterBox = bStart ?
                                       GetTabBox()->FindStartOfRowSpan( *pTable, nMax ) :
                                       GetTabBox()->FindEndOfRowSpan( *pTable, nMax );

        SwIterator<SwCellFrame,SwFormat> aIter( *rMasterBox.GetFrameFormat() );

        for ( SwCellFrame* pMasterCell = aIter.First(); pMasterCell; pMasterCell = aIter.Next() )
        {
            if ( pMasterCell->GetTabBox() == &rMasterBox )
            {
                const SwTabFrame* pMasterTable = static_cast<const SwTabFrame*>(pMasterCell->GetUpper()->GetUpper());

                if ( pMasterTable == pTableFrame )
                {
                    return *pMasterCell;
                }
            }
        }
    }

    SAL_WARN("sw.core", "SwCellFrame::FindStartRowSpanCell: No result");

    return *this;
}

// <-- NEW TABLES

const SwRowFrame* SwFrame::IsInSplitTableRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    const SwFrame* pRow = this;

    // find most upper row frame
    while( pRow && ( !pRow->IsRowFrame() || !pRow->GetUpper()->IsTabFrame() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return nullptr;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrame(), "Confusion in table layout" );

    const SwTabFrame* pTab = static_cast<const SwTabFrame*>(pRow->GetUpper());

    // If most upper row frame is a headline row, the current frame
    // can't be in a split table row. Thus, add corresponding condition.
    if ( pRow->GetNext() ||
         pTab->GetTable()->IsHeadline(
                    *(static_cast<const SwRowFrame*>(pRow)->GetTabLine()) ) ||
         !pTab->HasFollowFlowLine() ||
         !pTab->GetFollow() )
        return nullptr;

    // skip headline
    const SwRowFrame* pFollowRow = pTab->GetFollow()->GetFirstNonHeadlineRow();

    OSL_ENSURE( pFollowRow, "SwFrame::IsInSplitTableRow() does not work" );

    return pFollowRow;
}

const SwRowFrame* SwFrame::IsInFollowFlowRow() const
{
    OSL_ENSURE( IsInTab(), "IsInSplitTableRow should only be called for frames in tables" );

    // find most upper row frame
    const SwFrame* pRow = this;
    while( pRow && ( !pRow->IsRowFrame() || !pRow->GetUpper()->IsTabFrame() ) )
        pRow = pRow->GetUpper();

    if ( !pRow ) return nullptr;

    OSL_ENSURE( pRow->GetUpper()->IsTabFrame(), "Confusion in table layout" );

    const SwTabFrame* pTab = static_cast<const SwTabFrame*>(pRow->GetUpper());

    const SwTabFrame* pMaster = pTab->IsFollow() ? pTab->FindMaster() : nullptr;

    if ( !pMaster || !pMaster->HasFollowFlowLine() )
        return nullptr;

    const SwFrame* pTmp = pTab->GetFirstNonHeadlineRow();
    const bool bIsInFirstLine = ( pTmp == pRow );

    if ( !bIsInFirstLine )
        return nullptr;

    const SwRowFrame* pMasterRow = static_cast<const SwRowFrame*>(pMaster->GetLastLower());
    return pMasterRow;
}

bool SwFrame::IsInBalancedSection() const
{
    bool bRet = false;

    if ( IsInSct() )
    {
        const SwSectionFrame* pSectionFrame = FindSctFrame();
        if ( pSectionFrame )
            bRet = pSectionFrame->IsBalancedSection();
    }
    return bRet;
}

const SwFrame* SwLayoutFrame::GetLastLower() const
{
    const SwFrame* pRet = Lower();
    if ( !pRet )
        return nullptr;
    while ( pRet->GetNext() )
        pRet = pRet->GetNext();
    return pRet;
}

SwTextFrame* SwFrame::DynCastTextFrame()
{
    return IsTextFrame() ? static_cast<SwTextFrame*>(this) : nullptr;
}

const SwTextFrame* SwFrame::DynCastTextFrame() const
{
    return IsTextFrame() ? static_cast<const SwTextFrame*>(this) : nullptr;
}

SwPageFrame* SwFrame::DynCastPageFrame()
{
    return IsPageFrame() ? static_cast<SwPageFrame*>(this) : nullptr;
}

const SwPageFrame* SwFrame::DynCastPageFrame() const
{
    return IsPageFrame() ? static_cast<const SwPageFrame*>(this) : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
