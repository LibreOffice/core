 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accframe.cxx,v $
 * $Revision: 1.26 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <hintids.hxx>
#include <editeng/brshitem.hxx>
#include <flyfrm.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <sectfrm.hxx>
#include <pagefrm.hxx>
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


// Regarding visibilily (or in terms of accessibility: regarding the showing
// state): A frame is visible and therfor contained in the tree if its frame
// size overlaps with the visible area. The bounding box however is the
// frame's paint area.
sal_Int32 SwAccessibleFrame::GetChildCount( const SwRect& rVisArea,
                                            const SwFrm *pFrm,
                                            sal_Bool bInPagePreview )
{
    sal_Int32 nCount = 0;

    const SwFrmOrObjSList aVisList( rVisArea, pFrm );
    SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        if( rLower.IsAccessible( bInPagePreview ) )
        {
            nCount++;
        }
        else if( rLower.GetSwFrm() )
        {
            // There are no unaccessible SdrObjects that count
            nCount += GetChildCount( rVisArea, rLower.GetSwFrm(),
                                     bInPagePreview );
        }
        ++aIter;
    }

    return nCount;
}

SwFrmOrObj SwAccessibleFrame::GetChild( const SwRect& rVisArea,
                                        const SwFrm *pFrm,
                                        sal_Int32& rPos,
                                        sal_Bool bInPagePreview )
{
    SwFrmOrObj aRet;

    if( rPos >= 0 )
    {
        if( SwFrmOrObjMap::IsSortingRequired( pFrm ) )
        {
            // We need a sorted list here
            const SwFrmOrObjMap aVisMap( rVisArea, pFrm );
            SwFrmOrObjMap::const_iterator aIter( aVisMap.begin() );
            while( aIter != aVisMap.end() && !aRet.IsValid() )
            {
                const SwFrmOrObj& rLower = (*aIter).second;
                if( rLower.IsAccessible( bInPagePreview ) )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrm() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rVisArea, rLower.GetSwFrm(), rPos,
                                     bInPagePreview );
                }
                ++aIter;
            }
        }
        else
        {
            // The unsorted list is sorted enough, because it return lower
            // frames in the correct order.
            const SwFrmOrObjSList aVisList( rVisArea, pFrm );
            SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
            while( aIter != aVisList.end() && !aRet.IsValid() )
            {
                const SwFrmOrObj& rLower = *aIter;
                if( rLower.IsAccessible( bInPagePreview ) )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrm() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rVisArea, rLower.GetSwFrm(), rPos,
                                     bInPagePreview );
                }
                ++aIter;
            }
        }
    }

    return aRet;
}

sal_Bool SwAccessibleFrame::GetChildIndex( const SwRect& rVisArea,
                                           const SwFrm *pFrm,
                                           const SwFrmOrObj& rChild,
                                           sal_Int32& rPos,
                                           sal_Bool bInPagePreview )
{
    sal_Bool bFound = sal_False;

    if( SwFrmOrObjMap::IsSortingRequired( pFrm ) )
    {
        // We need a sorted list here
        const SwFrmOrObjMap aVisMap( rVisArea, pFrm );
        SwFrmOrObjMap::const_iterator aIter( aVisMap.begin() );
        while( aIter != aVisMap.end() && !bFound )
        {
            const SwFrmOrObj& rLower = (*aIter).second;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                if( rChild == rLower )
                    bFound = sal_True;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rVisArea, rLower.GetSwFrm(), rChild,
                                        rPos, bInPagePreview );
            }
            ++aIter;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it return lower
        // frames in the correct order.
        const SwFrmOrObjSList aVisList( rVisArea, pFrm );
        SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() && !bFound )
        {
            const SwFrmOrObj& rLower = *aIter;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                if( rChild == rLower )
                    bFound = sal_True;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rVisArea, rLower.GetSwFrm(), rChild,
                                        rPos, bInPagePreview );
            }
            ++aIter;
        }
    }

    return bFound;
}

SwFrmOrObj SwAccessibleFrame::GetChildAtPixel( const SwRect& rVisArea,
                                          const SwFrm *pFrm,
                                          const Point& rPixPos,
                                          sal_Bool bInPagePreview,
                                          const SwAccessibleMap *pMap )
{
    SwFrmOrObj aRet;

    if( SwFrmOrObjMap::IsSortingRequired( pFrm ) )
    {
        // We need a sorted list here, and we have to reverse iterate,
        // because objects in front should be returned.
        const SwFrmOrObjMap aVisMap( rVisArea, pFrm );
        SwFrmOrObjMap::const_reverse_iterator aRIter( aVisMap.rbegin() );
        while( aRIter != aVisMap.rend() && !aRet.IsValid() )
        {
            const SwFrmOrObj& rLower = (*aRIter).second;
            // A frame is returned if it's frame size is inside the visarea
            // and the positiion is inside the frame's paint area.
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                SwRect aLogBounds( rLower.GetBounds( ) );
                if( !aLogBounds.IsEmpty() )
                {
                    Rectangle aPixBounds( pMap->CoreToPixel( aLogBounds.SVRect() ) );
                    if( aPixBounds.IsInside( rPixPos ) )
                        aRet = rLower;
                }
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAtPixel( rVisArea, rLower.GetSwFrm(), rPixPos,
                                   bInPagePreview, pMap );
            }
            aRIter++;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it returns lower
        // frames in the correct order. Morover, we can iterate forward,
        // because the lowers don't overlap!
        const SwFrmOrObjSList aVisList( rVisArea, pFrm );
        SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() && !aRet.IsValid() )
        {
            const SwFrmOrObj& rLower = *aIter;
            // A frame is returned if it's frame size is inside the visarea
            // and the positiion is inside the frame's paint area.
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                SwRect aLogBounds( rLower.GetBounds( ) );
                if( !aLogBounds.IsEmpty() )
                {
                    Rectangle aPixBounds( pMap->CoreToPixel( aLogBounds.SVRect() ) );
                    if( aPixBounds.IsInside( rPixPos ) )
                        aRet = rLower;
                }
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAtPixel( rVisArea, rLower.GetSwFrm(), rPixPos,
                                   bInPagePreview, pMap );
            }
            ++aIter;
        }
    }

    return aRet;
}

void SwAccessibleFrame::GetChildren( const SwRect& rVisArea, const SwFrm *pFrm,
                                     ::std::list< SwFrmOrObj >& rChildren,
                                     sal_Bool bInPagePreview )
{
    if( SwFrmOrObjMap::IsSortingRequired( pFrm ) )
    {
        // We need a sorted list here
        const SwFrmOrObjMap aVisMap( rVisArea, pFrm );
        SwFrmOrObjMap::const_iterator aIter( aVisMap.begin() );
        while( aIter != aVisMap.end() )
        {
            const SwFrmOrObj& rLower = (*aIter).second;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                rChildren.push_back( rLower );
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                GetChildren( rVisArea, rLower.GetSwFrm(), rChildren,
                             bInPagePreview );
            }
            ++aIter;
        }
    }
    else
    {
        // The unsorted list is sorted enough, because it return lower
        // frames in the correct order.
        const SwFrmOrObjSList aVisList( rVisArea, pFrm );
        SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
        while( aIter != aVisList.end() )
        {
            const SwFrmOrObj& rLower = *aIter;
            if( rLower.IsAccessible( bInPagePreview ) )
            {
                rChildren.push_back( rLower );
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                GetChildren( rVisArea, rLower.GetSwFrm(), rChildren,
                             bInPagePreview );
            }
            ++aIter;
        }
    }
}

SwRect SwAccessibleFrame::GetBounds( const SwFrm *pFrm )
{
    if( !pFrm )
        pFrm = GetFrm();

    SwFrmOrObj aFrm( pFrm );
    SwRect aBounds( aFrm.GetBounds().Intersection( maVisArea ) );
    return aBounds;
}

sal_Bool SwAccessibleFrame::IsEditable( ViewShell *pVSh ) const
{
    const SwFrm *pFrm = GetFrm();
    if( !pFrm )
        return sal_False;

    ASSERT( pVSh, "no view shell" );
    if( pVSh && (pVSh->GetViewOptions()->IsReadonly() ||
                 pVSh->IsPreView()) )
        return sal_False;

    if( !pFrm->IsRootFrm() && pFrm->IsProtected() )
        return sal_False;

    return sal_True;
}

sal_Bool SwAccessibleFrame::IsOpaque( ViewShell *pVSh ) const
{
    SwFrmOrObj aFrm( GetFrm() );
    if( !aFrm.GetSwFrm() )
        return sal_False;

    ASSERT( pVSh, "no view shell" );
    if( !pVSh )
        return sal_False;

    const SwViewOption *pVOpt = pVSh->GetViewOptions();
    do
    {
        const SwFrm *pFrm = aFrm.GetSwFrm();
        if( pFrm->IsRootFrm() )
            return sal_True;

        if( pFrm->IsPageFrm() && !pVOpt->IsPageBack() )
            return sal_False;

        const SvxBrushItem &rBack = pFrm->GetAttrSet()->GetBackground();
        if( !rBack.GetColor().GetTransparency() ||
             rBack.GetGraphicPos() != GPOS_NONE )
            return sal_True;

        /// OD 20.08.2002 #99657#
        ///     If a fly frame has a transparent background color, we have
        ///     to consider the background.
        ///     But a background color "no fill"/"auto fill" has *not* to be considered.
        if( pFrm->IsFlyFrm() &&
            (rBack.GetColor().GetTransparency() != 0) &&
            (rBack.GetColor() != COL_TRANSPARENT)
          )
            return sal_True;

        if( pFrm->IsSctFrm() )
        {
            const SwSection* pSection = ((SwSectionFrm*)pFrm)->GetSection();
            if( pSection && ( TOX_HEADER_SECTION == pSection->GetType() ||
                TOX_CONTENT_SECTION == pSection->GetType() ) &&
                !pVOpt->IsReadonly() &&
                SwViewOption::IsIndexShadings() )
                return sal_True;
        }
        if( pFrm->IsFlyFrm() )
            aFrm = static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm();
        else
            aFrm = pFrm->GetUpper();
    } while( aFrm.GetSwFrm() && !aFrm.IsAccessible( IsInPagePreview() ) );

    return sal_False;
}

SwAccessibleFrame::SwAccessibleFrame( const SwRect& rVisArea,
                                      const SwFrm *pF,
                                      sal_Bool bIsPagePreview ) :
    maVisArea( rVisArea ),
    mpFrm( pF ),
    mbIsInPagePreview( bIsPagePreview )
{
}

SwAccessibleFrame::~SwAccessibleFrame()
{
}

const SwFrm *SwAccessibleFrame::GetParent( const SwFrmOrObj& rFrmOrObj,
                                           sal_Bool bInPagePreview )
{
    SwFrmOrObj aParent;
    const SwFrm *pFrm = rFrmOrObj.GetSwFrm();
    if( pFrm )
    {
        if( pFrm->IsFlyFrm() )
        {
            const SwFlyFrm *pFly = static_cast< const SwFlyFrm *>( pFrm );
            if( pFly->IsFlyInCntFrm() )
            {
                // For FLY_IN_CNTNT the parent is the anchor
                aParent = pFly->GetAnchorFrm();
                ASSERT( aParent.IsAccessible( bInPagePreview ),
                        "parent is not accessible" );
            }
            else
            {
                // In any other case the parent is the root frm
                // (in page preview, the page frame)
                if( bInPagePreview )
                    aParent = pFly->FindPageFrm();
                else
                    aParent = pFly->FindRootFrm();
            }
        }
        else
        {
            SwFrmOrObj aUpper( pFrm->GetUpper() );
            while( aUpper.GetSwFrm() && !aUpper.IsAccessible(bInPagePreview) )
                aUpper = aUpper.GetSwFrm()->GetUpper();
            aParent = aUpper;
        }
    }
    else if( rFrmOrObj.GetSdrObject() )
    {
        const SwDrawContact *pContact =
            static_cast< const SwDrawContact* >(
                    GetUserCall( rFrmOrObj.GetSdrObject() ) );
        ASSERT( pContact, "sdr contact is missing" );
        if( pContact )
        {
            const SwFrmFmt *pFrmFmt = pContact->GetFmt();
            ASSERT( pFrmFmt, "frame format is missing" );
            if( pFrmFmt && FLY_IN_CNTNT == pFrmFmt->GetAnchor().GetAnchorId() )
            {
                // For FLY_IN_CNTNT the parent is the anchor
                aParent = pContact->GetAnchorFrm();
                ASSERT( aParent.IsAccessible( bInPagePreview ),
                        "parent is not accessible" );

            }
            else
            {
                // In any other case the parent is the root frm
                if( bInPagePreview )
                    aParent = pContact->GetAnchorFrm()->FindPageFrm();
                else
                    aParent = pContact->GetAnchorFrm()->FindRootFrm();
            }
        }
    }

    return aParent.GetSwFrm();
}

String SwAccessibleFrame::GetFormattedPageNumber() const
{
    sal_uInt16 nPageNum = GetFrm()->GetVirtPageNum();
    sal_uInt32 nFmt = GetFrm()->FindPageFrm()->GetPageDesc()
                              ->GetNumType().GetNumberingType();
    if( SVX_NUM_NUMBER_NONE == nFmt )
        nFmt = SVX_NUM_ARABIC;

    String sRet( FormatNumber( nPageNum, nFmt ) );
    return sRet;
}
