 /*************************************************************************
 *
 *  $RCSfile: accframe.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mib $ $Date: 2002-03-18 12:49:59 $
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
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifndef _ACCMAP_HXX
#include <accmap.hxx>
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
    const SwFrm *pLower = pFrm->GetLower();
    while( pLower )
    {
        if( pLower->Frm().IsOver( rVisArea ) )
        {
            if( pLower->IsAccessibleFrm() )
                nCount++;
            else
                nCount += GetChildCount( rVisArea, pLower );
        }
        pLower = pLower->GetNext();
    }

    return nCount;
}

const SwFrm *SwAccessibleFrame::GetChild( const Rectangle& rVisArea,
                                         const SwFrm *pFrm,
                                         sal_Int32& rPos )
{
    const SwFrm *pDesc = 0;

    if( rPos >= 0 )
    {
        const SwFrm *pLower = pFrm->GetLower();
        while( pLower && !pDesc )
        {
            if( pLower->Frm().IsOver( rVisArea ) )
            {
                if( pLower->IsAccessibleFrm() )
                {
                    if( 0 == rPos )
                        pDesc = pLower;
                    else
                        rPos--;
                }
                else
                {
                    pDesc = GetChild( rVisArea, pLower, rPos );
                }
            }
            pLower = pLower->GetNext();
        }
    }

    return pDesc;
}

sal_Bool SwAccessibleFrame::GetChildIndex( const Rectangle& rVisArea,
                                                 const SwFrm *pFrm,
                                                  const SwFrm *pChild,
                                                 sal_Int32& rPos )
{
    const SwFrm *pLower = pFrm->GetLower();
    sal_Bool bFound = sal_False;
    while( pLower && !bFound )
    {
        if( pLower->Frm().IsOver( rVisArea ) )
        {
            if( pLower->IsAccessibleFrm() )
            {
                if( pLower == pChild )
                    bFound = sal_True;
                else
                    rPos++;
            }
            else
            {
                bFound = GetChildIndex( rVisArea, pLower, pChild, rPos );
            }
        }
        pLower = pLower->GetNext();
    }

    return bFound;
}

const SwFrm *SwAccessibleFrame::GetChildAt(
                                        const Rectangle& rVisArea,
                                        const SwFrm *pFrm,
                                        const Point& rPos )
{
    const SwFrm *pLower = pFrm->GetLower();
    const SwFrm *pDesc = 0;
    while( pLower && !pDesc )
    {
        // A frame is returned if it's frame size is inside the visarea
        // and the positiion is inside the frame's paint area.
        if( pFrm->Frm().IsOver( rVisArea ) )
        {
            if( pLower->IsAccessibleFrm() )
            {
                if( pLower->PaintArea().IsInside( rPos ) )
                    pDesc = pLower;
            }
            else
            {
                pDesc = GetChildAt( rVisArea, pLower, rPos );
            }
        }
        pLower = pLower->GetNext();
    }

    return pDesc;
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
    const SwFrm *pLower = pFrm->GetLower();
    while( pLower )
    {
        SwRect aFrm( pLower->Frm() );
        if( pLower->IsAccessibleFrm() )
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
        else if( aFrm.IsOver( rOldVisArea ) ||
                 aFrm.IsOver( rNewVisArea ) )
        {
            SetVisArea( pLower, rOldVisArea, rNewVisArea, pAcc );
        }

        pLower = pLower->GetNext();
    }
}

void SwAccessibleFrame::CheckEditableStateChildren( const SwFrm *pFrm,
                                                const Rectangle& rOldVisArea,
                                                SwAccessibleFrame *pAcc )
{
    const SwFrm *pLower = pFrm->GetLower();
    while( pLower )
    {
        SwRect aFrm( pLower->Frm() );
        if( pLower->IsAccessibleFrm() )
        {
            sal_Bool bCheckLower = sal_False;
            if( aFrm.IsOver( rOldVisArea ) )
            {
                if( pAcc )
                    bCheckLower = pAcc->CheckEditableStateChild( pLower );
                else
                    bCheckLower = sal_True;
            }
            if( bCheckLower )
                CheckEditableStateChildren( pLower, rOldVisArea );
        }
        else if( aFrm.IsOver( rOldVisArea ) )
        {
            CheckEditableStateChildren( pLower, rOldVisArea, pAcc );
        }

        pLower = pLower->GetNext();
    }
}

void SwAccessibleFrame::DisposeChildren( const SwFrm *pFrm,
                                         const Rectangle& rOldVisArea,
                                         sal_Bool bRecursive,
                                         SwAccessibleFrame *pAcc )
{
    const SwFrm *pLower = pFrm->GetLower();
    while( pLower )
    {
        SwRect aFrm( pLower->Frm() );
        if( pLower->IsAccessibleFrm() )
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
        else if( aFrm.IsOver( rOldVisArea ) )
        {
            DisposeChildren( pLower, rOldVisArea, bRecursive, pAcc );
        }

        pLower = pLower->GetNext();
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

sal_Bool SwAccessibleFrame::CheckEditableStateChild( const SwFrm *pFrm )
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
            pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
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

const SwFrm *SwAccessibleFrame::GetParent() const
{
    const SwLayoutFrm *pUpper = pFrm->GetUpper();
    while( pUpper && !pUpper->IsAccessibleFrm() )
        pUpper = pUpper->GetUpper();

    return pUpper;
}

void SwAccessibleFrame::SetVisArea( const Rectangle& rNewVisArea )
{
    Rectangle aOldVisArea( aVisArea );
    aVisArea = rNewVisArea;

    SetVisArea( GetFrm(), aOldVisArea, aVisArea, this );
}
