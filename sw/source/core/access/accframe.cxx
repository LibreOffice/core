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
#include <editeng/brushitem.hxx>
#include <flyfrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <section.hxx>
#include <viewsh.hxx>
#include <viewopt.hxx>
#include <doc.hxx>
#include <frmatr.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <fldbas.hxx>
#include <dcontact.hxx>
#include <accmap.hxx>
#include <accfrmobjslist.hxx>
#include <accfrmobjmap.hxx>
#include <accframe.hxx>

using namespace sw::access;

// Regarding visibility (or in terms of accessibility: regarding the showing
// state): A frame is visible and therefore contained in the tree if its frame
// size overlaps with the visible area. The bounding box however is the
// frame's paint area.
sal_Int32 SwAccessibleFrame::GetChildCount( SwAccessibleMap& rAccMap,
                                            const SwRect& rVisArea,
                                            const SwFrame *pFrame,
                                            bool bInPagePreview )
{
    sal_Int32 nCount = 0;

    const SwAccessibleChildSList aVisList( rVisArea, *pFrame, rAccMap );

    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        if( rLower.IsAccessible( bInPagePreview ) )
        {
            nCount++;
        }
        else if( rLower.GetSwFrame() )
        {
            // There are no unaccessible SdrObjects that count
            nCount += GetChildCount( rAccMap,
                                     rVisArea, rLower.GetSwFrame(),
                                     bInPagePreview );
        }
        ++aIter;
    }

    return nCount;
}

SwAccessibleChild SwAccessibleFrame::GetChild(
                                   SwAccessibleMap& rAccMap,
                                   const SwRect& rVisArea,
                                   const SwFrame& rFrame,
                                   sal_Int32& rPos,
                                   bool bInPagePreview )
{
    SwAccessibleChild aRet;

    if( rPos >= 0 )
    {
        if( SwAccessibleChildMap::IsSortingRequired( rFrame ) )
        {
            // We need a sorted list here
            const SwAccessibleChildMap aVisMap( rVisArea, rFrame, rAccMap );
            SwAccessibleChildMap::const_iterator aIter( aVisMap.cbegin() );
            while( aIter != aVisMap.cend() && !aRet.IsValid() )
            {
                const SwAccessibleChild& rLower = (*aIter).second;
                if( rLower.IsAccessible( bInPagePreview ) )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrame() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rAccMap,
                                     rVisArea, *(rLower.GetSwFrame()), rPos,
                                     bInPagePreview );
                }
                ++aIter;
            }
        }
        else
        {
            // The unsorted list is sorted enough, because it returns lower
            // frames in the correct order.
            const SwAccessibleChildSList aVisList( rVisArea, rFrame, rAccMap );
            SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
            while( aIter != aVisList.end() && !aRet.IsValid() )
            {
                const SwAccessibleChild& rLower = *aIter;
                if( rLower.IsAccessible( bInPagePreview ) )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrame() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rAccMap,
                                     rVisArea, *(rLower.GetSwFrame()), rPos,
                                     bInPagePreview );
                }
                ++aIter;
            }
        }
    }

    return aRet;
}

bool SwAccessibleFrame::GetChildIndex(
                                   SwAccessibleMap& rAccMap,
                                   const SwRect& rVisArea,
                                   const SwFrame& rFrame,
                                   const SwAccessibleChild& rChild,
                                   sal_Int32& rPos,
                                   bool bInPagePreview )
{
    bool bFound = false;

    if( SwAccessibleChildMap::IsSortingRequired( rFrame ) )
    {
        // We need a sorted list here
        const SwAccessibleChildMap aVisMap( rVisArea, rFrame, rAccMap );
        SwAccessibleChildMap::const_iterator aIter( aVisMap.cbegin() );
        while( aIter != aVisMap.cend() && !bFound )
        {
            const SwAccessibleChild& rLower = (*aIter).second;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                if( rChild == rLower )
                    bFound = true;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rAccMap,
                                        rVisArea, *(rLower.GetSwFrame()), rChild,
                                        rPos, bInPagePreview );
            }
            ++aIter;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it returns lower
        // frames in the correct order.

        const SwAccessibleChildSList aVisList( rVisArea, rFrame, rAccMap );
        SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() && !bFound )
        {
            const SwAccessibleChild& rLower = *aIter;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                if( rChild == rLower )
                    bFound = true;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rAccMap,
                                        rVisArea, *(rLower.GetSwFrame()), rChild,
                                        rPos, bInPagePreview );
            }
            ++aIter;
        }
    }

    return bFound;
}

SwAccessibleChild SwAccessibleFrame::GetChildAtPixel( const SwRect& rVisArea,
                                          const SwFrame& rFrame,
                                          const Point& rPixPos,
                                          bool bInPagePreview,
                                          SwAccessibleMap& rAccMap )
{
    SwAccessibleChild aRet;

    if( SwAccessibleChildMap::IsSortingRequired( rFrame ) )
    {
        // We need a sorted list here, and we have to reverse iterate,
        // because objects in front should be returned.
        const SwAccessibleChildMap aVisMap( rVisArea, rFrame, rAccMap );
        SwAccessibleChildMap::const_reverse_iterator aRIter( aVisMap.crbegin() );
        while( aRIter != aVisMap.crend() && !aRet.IsValid() )
        {
            const SwAccessibleChild& rLower = (*aRIter).second;
            // A frame is returned if it's frame size is inside the visarea
            // and the position is inside the frame's paint area.
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                SwRect aLogBounds( rLower.GetBounds( rAccMap ) );
                if( !aLogBounds.IsEmpty() )
                {
                    Rectangle aPixBounds( rAccMap.CoreToPixel( aLogBounds.SVRect() ) );
                    if( aPixBounds.IsInside( rPixPos ) )
                        aRet = rLower;
                }
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAtPixel( rVisArea, *(rLower.GetSwFrame()), rPixPos,
                                        bInPagePreview, rAccMap );
            }
            ++aRIter;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it returns lower
        // frames in the correct order. Moreover, we can iterate forward,
        // because the lowers don't overlap!
        const SwAccessibleChildSList aVisList( rVisArea, rFrame, rAccMap );
        SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() && !aRet.IsValid() )
        {
            const SwAccessibleChild& rLower = *aIter;
            // A frame is returned if it's frame size is inside the visarea
            // and the position is inside the frame's paint area.
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                SwRect aLogBounds( rLower.GetBounds( rAccMap ) );
                if( !aLogBounds.IsEmpty() )
                {
                    Rectangle aPixBounds( rAccMap.CoreToPixel( aLogBounds.SVRect() ) );
                    if( aPixBounds.IsInside( rPixPos ) )
                        aRet = rLower;
                }
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAtPixel( rVisArea, *(rLower.GetSwFrame()), rPixPos,
                                   bInPagePreview, rAccMap );
            }
            ++aIter;
        }
    }

    return aRet;
}

void SwAccessibleFrame::GetChildren( SwAccessibleMap& rAccMap,
                                     const SwRect& rVisArea,
                                     const SwFrame& rFrame,
                                     std::list< SwAccessibleChild >& rChildren,
                                     bool bInPagePreview )
{
    if( SwAccessibleChildMap::IsSortingRequired( rFrame ) )
    {
        // We need a sorted list here
        const SwAccessibleChildMap aVisMap( rVisArea, rFrame, rAccMap );
        SwAccessibleChildMap::const_iterator aIter( aVisMap.cbegin() );
        while( aIter != aVisMap.cend() )
        {
            const SwAccessibleChild& rLower = (*aIter).second;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                rChildren.push_back( rLower );
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                GetChildren( rAccMap, rVisArea, *(rLower.GetSwFrame()),
                             rChildren, bInPagePreview );
            }
            ++aIter;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it returns lower
        // frames in the correct order.
        const SwAccessibleChildSList aVisList( rVisArea, rFrame, rAccMap );
        SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() )
        {
            const SwAccessibleChild& rLower = *aIter;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                rChildren.push_back( rLower );
            }
            else if( rLower.GetSwFrame() )
            {
                // There are no unaccessible SdrObjects that count
                GetChildren( rAccMap, rVisArea, *(rLower.GetSwFrame()),
                             rChildren, bInPagePreview );
            }
            ++aIter;
        }
    }
}

SwRect SwAccessibleFrame::GetBounds( const SwAccessibleMap& rAccMap,
                                     const SwFrame *pFrame )
{
    if( !pFrame )
        pFrame = GetFrame();

    SwAccessibleChild aFrame( pFrame );
    SwRect aBounds( aFrame.GetBounds( rAccMap ).Intersection( maVisArea ) );
    return aBounds;
}

bool SwAccessibleFrame::IsEditable( SwViewShell *pVSh ) const
{
    const SwFrame *pFrame = GetFrame();
    if( !pFrame )
        return false;

    OSL_ENSURE( pVSh, "no view shell" );
    if( pVSh && (pVSh->GetViewOptions()->IsReadonly() ||
                 pVSh->IsPreview()) )
        return false;

    if( !pFrame->IsRootFrame() && pFrame->IsProtected() )
        return false;

    return true;
}

bool SwAccessibleFrame::IsOpaque( SwViewShell *pVSh ) const
{
    SwAccessibleChild aFrame( GetFrame() );
    if( !aFrame.GetSwFrame() )
        return false;

    OSL_ENSURE( pVSh, "no view shell" );
    if( !pVSh )
        return false;

    const SwViewOption *pVOpt = pVSh->GetViewOptions();
    do
    {
        const SwFrame *pFrame = aFrame.GetSwFrame();
        if( pFrame->IsRootFrame() )
            return true;

        if( pFrame->IsPageFrame() && !pVOpt->IsPageBack() )
            return false;

        const SvxBrushItem &rBack = pFrame->GetAttrSet()->GetBackground();
        if( !rBack.GetColor().GetTransparency() ||
             rBack.GetGraphicPos() != GPOS_NONE )
            return true;

        // If a fly frame has a transparent background color, we have to consider the background.
        // But a background color "no fill"/"auto fill" should *not* be considered.
        if( pFrame->IsFlyFrame() &&
            (rBack.GetColor().GetTransparency() != 0) &&
            (rBack.GetColor() != COL_TRANSPARENT)
          )
            return true;

        if( pFrame->IsSctFrame() )
        {
            const SwSection* pSection = static_cast<const SwSectionFrame*>(pFrame)->GetSection();
            if( pSection && ( TOX_HEADER_SECTION == pSection->GetType() ||
                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                !pVOpt->IsReadonly() &&
                SwViewOption::IsIndexShadings() )
                return true;
        }
        if( pFrame->IsFlyFrame() )
            aFrame = static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame();
        else
            aFrame = pFrame->GetUpper();
    } while( aFrame.GetSwFrame() && !aFrame.IsAccessible( IsInPagePreview() ) );

    return false;
}

SwAccessibleFrame::SwAccessibleFrame( const SwRect& rVisArea,
                                      const SwFrame *pF,
                                      bool bIsPagePreview ) :
    maVisArea( rVisArea ),
    mpFrame( pF ),
    mbIsInPagePreview( bIsPagePreview ),
    m_bIsAccDocUse( false )
{
}

SwAccessibleFrame::~SwAccessibleFrame()
{
}

const SwFrame* SwAccessibleFrame::GetParent( const SwAccessibleChild& rFrameOrObj,
                                           bool bInPagePreview )
{
    return rFrameOrObj.GetParent( bInPagePreview );
}

OUString SwAccessibleFrame::GetFormattedPageNumber() const
{
    sal_uInt16 nPageNum = GetFrame()->GetVirtPageNum();
    sal_uInt32 nFormat = GetFrame()->FindPageFrame()->GetPageDesc()
                              ->GetNumType().GetNumberingType();
    if( SVX_NUM_NUMBER_NONE == nFormat )
        nFormat = SVX_NUM_ARABIC;

    OUString sRet( FormatNumber( nPageNum, nFormat ) );
    return sRet;
}

sal_Int32 SwAccessibleFrame::GetChildCount( SwAccessibleMap& rAccMap ) const
{
    return GetChildCount( rAccMap, maVisArea, mpFrame, IsInPagePreview() );
}

sw::access::SwAccessibleChild SwAccessibleFrame::GetChild(
                                                SwAccessibleMap& rAccMap,
                                                sal_Int32 nPos ) const
{
    return SwAccessibleFrame::GetChild( rAccMap, maVisArea, *mpFrame, nPos, IsInPagePreview() );
}

sal_Int32 SwAccessibleFrame::GetChildIndex( SwAccessibleMap& rAccMap,
                                            const sw::access::SwAccessibleChild& rChild ) const
{
    sal_Int32 nPos = 0;
    return GetChildIndex( rAccMap, maVisArea, *mpFrame, rChild, nPos, IsInPagePreview() )
           ? nPos
           : -1L;
}

sw::access::SwAccessibleChild SwAccessibleFrame::GetChildAtPixel(
                                                const Point& rPos,
                                                SwAccessibleMap& rAccMap ) const
{
    return GetChildAtPixel( maVisArea, *mpFrame, rPos, IsInPagePreview(), rAccMap );
}

void SwAccessibleFrame::GetChildren( SwAccessibleMap& rAccMap,
                                     std::list< sw::access::SwAccessibleChild >& rChildren ) const
{
    GetChildren( rAccMap, maVisArea, *mpFrame, rChildren, IsInPagePreview() );
}

bool SwAccessibleFrame::IsShowing( const SwAccessibleMap& rAccMap,
                                       const sw::access::SwAccessibleChild& rFrameOrObj ) const
{
    return IsShowing( rFrameOrObj.GetBox( rAccMap ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
