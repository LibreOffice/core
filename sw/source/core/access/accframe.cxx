 /*************************************************************************
 *
 *  $RCSfile: accframe.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: mib $ $Date: 2002-08-07 12:41:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop


#include <hintids.hxx>
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif

#ifndef _LAYFRM_HXX
#include <layfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif

#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCFRMOBJSLIST_HXX
#include <accfrmobjslist.hxx>
#endif
#ifndef _ACCFRMOBJMAP_HXX
#include <accfrmobjmap.hxx>
#endif
#ifndef _ACCFRAME_HXX
#include <accframe.hxx>
#endif


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

void SwAccessibleFrame::MergeLowerBounds( SwRect& rBounds,
                                             const SwRect& rVisArea,
                                          const SwFrm *pFrm,
                                          sal_Bool bInPagePreview )
{
    const SwFrmOrObjSList aVisList( rVisArea, pFrm );
    SwFrmOrObjSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        if( rLower.IsAccessible( bInPagePreview ) )
        {
            rBounds.Union( rLower.GetBounds() );
        }
        else if( rLower.GetSwFrm() )
        {
            MergeLowerBounds( rBounds, rVisArea, rLower.GetSwFrm(),
                              bInPagePreview );
        }
        ++aIter;
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
            aFrm = static_cast<const SwFlyFrm*>(pFrm)->GetAnchor();
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
                aParent = pFly->GetAnchor();
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
                aParent = pContact->GetAnchor();
                ASSERT( aParent.IsAccessible( bInPagePreview ),
                        "parent is not accessible" );

            }
            else
            {
                // In any other case the parent is the root frm
                if( bInPagePreview )
                    aParent = pContact->GetAnchor()->FindPageFrm();
                else
                    aParent = pContact->GetAnchor()->FindRootFrm();
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
