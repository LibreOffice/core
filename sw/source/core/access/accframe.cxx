 /*************************************************************************
 *
 *  $RCSfile: accframe.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mib $ $Date: 2002-04-05 12:10:10 $
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
sal_Int32 SwAccessibleFrame::GetChildCount( const Rectangle& rVisArea,
                                            const SwFrm *pFrm )
{
    sal_Int32 nCount = 0;

    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        if( rLower.GetBox().IsOver( rVisArea ) )
        {
            if( rLower.IsAccessible() )
            {
                nCount++;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                nCount += GetChildCount( rVisArea, rLower.GetSwFrm() );
            }
        }
        ++aIter;
    }

    return nCount;
}

SwFrmOrObj SwAccessibleFrame::GetChild( const Rectangle& rVisArea,
                                           const SwFrm *pFrm,
                                           sal_Int32& rPos )
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
                if( rLower.IsAccessible() )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrm() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rVisArea, rLower.GetSwFrm(), rPos );
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
                if( rLower.IsAccessible() )
                {
                    if( 0 == rPos )
                        aRet = rLower;
                    else
                        rPos--;
                }
                else if( rLower.GetSwFrm() )
                {
                    // There are no unaccessible SdrObjects that count
                    aRet = GetChild( rVisArea, rLower.GetSwFrm(), rPos );
                }
                ++aIter;
            }
        }
    }

    return aRet;
}

sal_Bool SwAccessibleFrame::GetChildIndex( const Rectangle& rVisArea,
                                           const SwFrm *pFrm,
                                           const SwFrmOrObj& rChild,
                                           sal_Int32& rPos )
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
            if( rLower.IsAccessible() )
            {
                if( rChild == rLower )
                    bFound = sal_True;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rVisArea, rLower.GetSwFrm(), rChild, rPos );
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
            if( rLower.IsAccessible() )
            {
                if( rChild == rLower )
                    bFound = sal_True;
                else
                    rPos++;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                bFound = GetChildIndex( rVisArea, rLower.GetSwFrm(), rChild, rPos );
            }
            ++aIter;
        }
    }

    return bFound;
}

SwFrmOrObj SwAccessibleFrame::GetChildAt( const Rectangle& rVisArea,
                                            const SwFrm *pFrm,
                                            const Point& rPos )
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
            if( rLower.IsAccessible() )
            {
                if( rLower.GetBounds().IsInside( rPos ) )
                    aRet = rLower;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAt( rVisArea, rLower.GetSwFrm(), rPos );
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
            if( rLower.IsAccessible() )
            {
                if( rLower.GetBounds().IsInside( rPos ) )
                    aRet = rLower;
            }
            else if( rLower.GetSwFrm() )
            {
                // There are no unaccessible SdrObjects that count
                aRet = GetChildAt( rVisArea, rLower.GetSwFrm(), rPos );
            }
            ++aIter;
        }
    }

    return aRet;
}

void SwAccessibleFrame::MergeLowerBounds( SwRect& rBounds,
                                             const Rectangle& rVisArea,
                                          const SwFrm *pFrm )
{
    const SwFrm *pLower = pFrm->GetLower();
    while( pLower )
    {
        if( pLower->Frm().IsOver( rVisArea ) )
        {
            if( pLower->IsAccessibleFrm() )
            {
                SwRect aPaintArea( pLower->PaintArea() );
                rBounds.Union( aPaintArea  );
            }
            else
            {
                MergeLowerBounds( rBounds, rVisArea, pLower );
            }
        }
        pLower = pLower->GetNext();
    }
}


void SwAccessibleFrame::SetVisArea( const SwFrm *pFrm,
                                    const Rectangle& rOldVisArea,
                                    const Rectangle& rNewVisArea,
                                    SwAccessibleFrame *pAcc )
{
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        SwRect aFrm( rLower.GetBox() );
        if( rLower.IsAccessible() )
        {
            if( pLower  )
            {
                sal_Bool bUpdateLower = sal_False;
                if( aFrm.IsOver( rNewVisArea ) )
                {
                    if( pAcc )
                    {
                        if( aFrm.IsOver( rOldVisArea ) )
                            bUpdateLower = pAcc->ChildScrolled( pLower );
                        else
                            bUpdateLower = pAcc->ChildScrolledIn( pLower );
                    }
                    else
                        bUpdateLower = sal_True;
                }
                else if( aFrm.IsOver( rOldVisArea ) )
                {
                    if( pAcc )
                        bUpdateLower = pAcc->ChildScrolledOut( pLower );
                    else
                        bUpdateLower = sal_True;
                }
                if( bUpdateLower )
                    SetVisArea( pLower, rOldVisArea, rNewVisArea  );
            }
            else
            {
                // TODO: SdrObjects
            }
        }
        else if( pLower && (aFrm.IsOver( rOldVisArea ) ||
                              aFrm.IsOver( rNewVisArea )) )
        {
            // There are no unaccessible SdrObjects that need to be notified
            SetVisArea( pLower, rOldVisArea, rNewVisArea, pAcc );
        }
        ++aIter;
    }
}

void SwAccessibleFrame::CheckStatesChildren( const SwFrm *pFrm,
                                              const Rectangle& rOldVisArea,
                                             sal_uInt8 nStates,
                                              SwAccessibleFrame *pAcc )
{
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        SwRect aFrm( rLower.GetBox() );
        if( rLower.IsAccessible() )
        {
            if( pLower )
            {
                sal_Bool bCheckLower = sal_False;
                if( aFrm.IsOver( rOldVisArea ) )
                {
                    if( pAcc )
                        bCheckLower = pAcc->CheckStatesChild( pLower, nStates );
                    else
                        bCheckLower = sal_True;
                }
                if( bCheckLower )
                    CheckStatesChildren( pLower, rOldVisArea, nStates );
            }
            else
            {
                // TODO: SdrObjects
            }
        }
        else if( pLower && rLower.GetBox().IsOver( rOldVisArea ) )
        {
            // There are no unaccessible SdrObjects that need to be notified
            CheckStatesChildren( pLower, rOldVisArea, nStates, pAcc );
        }
        ++aIter;
    }
}

void SwAccessibleFrame::DisposeChildren( const SwFrm *pFrm,
                                         const Rectangle& rOldVisArea,
                                         sal_Bool bRecursive,
                                         SwAccessibleFrame *pAcc )
{
    const SwFrmOrObjSList aList( pFrm );
    SwFrmOrObjSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwFrmOrObj& rLower = *aIter;
        const SwFrm *pLower = rLower.GetSwFrm();
        SwRect aFrm( rLower.GetBox() );
        if( rLower.IsAccessible() )
        {
            if( pLower )
            {
                sal_Bool bDisposeLower = sal_False;
                if( aFrm.IsOver( rOldVisArea ) )
                {
                    if( pAcc )
                        bDisposeLower = pAcc->DisposeChild( pLower, bRecursive );
                    else
                        bDisposeLower = sal_True;
                }
                if( bDisposeLower && bRecursive )
                    DisposeChildren( pLower, rOldVisArea, bRecursive );
            }
            else
            {
                // TODO: SdrObjects
            }
        }
        else if( pLower && aFrm.IsOver( rOldVisArea ) )
        {
            // There are no unaccessible SdrObjects that need to be notified
            DisposeChildren( pLower, rOldVisArea, bRecursive, pAcc );
        }
        ++aIter;
    }
}


sal_Bool SwAccessibleFrame::ChildScrolledIn( const SwFrm *pFrm )
{
    return sal_True;
}

sal_Bool SwAccessibleFrame::ChildScrolledOut( const SwFrm *pFrm )
{
    return sal_True;
}

sal_Bool SwAccessibleFrame::ChildScrolled( const SwFrm *pFrm )
{
    return sal_True;
}

sal_Bool SwAccessibleFrame::CheckStatesChild( const SwFrm *pFrm,
                                              sal_uInt8 nStates )
{
    return sal_True;
}

sal_Bool SwAccessibleFrame::DisposeChild( const SwFrm *pFrm,
                                          sal_Bool bRecursive )
{
    return sal_True;
}

Rectangle SwAccessibleFrame::GetBounds( const SwFrm *pFrm )
{
    if( !pFrm )
        pFrm = GetFrm();

    // The bounds are the paint area!
    SwRect aPaintArea( pFrm->PaintArea() );

    Rectangle aBounds( aPaintArea.SVRect().Intersection( aVisArea ) );
    return aBounds;
}

sal_Bool SwAccessibleFrame::IsEditable( ViewShell *pVSh ) const
{
    const SwFrm *pFrm = GetFrm();
    if( !pFrm )
        return sal_False;

    ASSERT( pVSh, "no view shell" );
    if( pVSh && pVSh->GetViewOptions()->IsReadonly() )
        return sal_False;

    if( !pFrm->IsRootFrm() && pFrm->IsProtected() )
        return sal_False;

    return sal_True;
}

sal_Bool SwAccessibleFrame::IsOpaque( ViewShell *pVSh ) const
{
    const SwFrm *pFrm = GetFrm();
    if( !pFrm )
        return sal_False;

    ASSERT( pVSh, "no view shell" );
    if( !pVSh )
        return sal_False;

    const SwViewOption *pVOpt = pVSh->GetViewOptions();
    do
    {
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
                pVOpt->IsIndexBackground() )
                return sal_True;
        }
        if ( pFrm->IsFlyFrm() )
            pFrm = static_cast<const SwFlyFrm*>(pFrm)->GetAnchor();
        else
            pFrm = pFrm->GetUpper();
    } while( pFrm && !pFrm->IsAccessibleFrm() );

    return sal_False;
}

SwAccessibleFrame::SwAccessibleFrame( const Rectangle& rVisArea,
                                      const SwFrm *pF ) :
    aVisArea( rVisArea ),
    pFrm( pF )
{
}

SwAccessibleFrame::~SwAccessibleFrame()
{
}

const SwFrm *SwAccessibleFrame::GetParent( const SwFrm *pFrm )
{
    const SwFrm *pParent;
    if ( pFrm->IsFlyFrm() )
    {
        const SwFlyFrm *pFly = static_cast< const SwFlyFrm *>( pFrm );
        if( pFly->IsFlyInCntFrm() )
        {
            // For FLY_IN_CNTNT the parent is the anchor
            pParent = pFly->GetAnchor();
            ASSERT( !pParent || pParent->IsAccessibleFrm(),
                    "parent is not accessible" );
        }
        else
        {
            // In any other case the parent is the root frm
            pParent = pFly->FindRootFrm();
        }
    }
    else
    {
        const SwLayoutFrm *pUpper = pFrm->GetUpper();
        while( pUpper && !pUpper->IsAccessibleFrm() )
            pUpper = pUpper->GetUpper();
        pParent = pUpper;
    }

    return pParent;
}

void SwAccessibleFrame::SetVisArea( const Rectangle& rNewVisArea )
{
    Rectangle aOldVisArea( aVisArea );
    aVisArea = rNewVisArea;

    SetVisArea( GetFrm(), aOldVisArea, aVisArea, this );
}
