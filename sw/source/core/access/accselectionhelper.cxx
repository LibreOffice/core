 /*************************************************************************
 *
 *  $RCSfile: accselectionhelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2002-04-12 12:48:59 $
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

#ifndef _ACCSELECTIONHELPER_HXX_
#include <accselectionhelper.hxx>
#endif

#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>        // for SolarMutex
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif



using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::drafts::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::lang::IndexOutOfBoundsException;



SwAccessibleSelectionHelper::SwAccessibleSelectionHelper(
    SwAccessibleContext& rCtxt ) :
        rContext( rCtxt )
{
}

SwAccessibleSelectionHelper::~SwAccessibleSelectionHelper()
{
}

SwFEShell* SwAccessibleSelectionHelper::GetFEShell()
{
    DBG_ASSERT( rContext.GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = rContext.GetMap()->GetShell();
    DBG_ASSERT( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = NULL;
    if( pViewShell->ISA( SwFEShell ) )
    {
        pFEShell = static_cast<SwFEShell*>( pViewShell );
    }

    return pFEShell;
}

const SwFlyFrm* SwAccessibleSelectionHelper::GetSelectedChildFlyFrame()
{
    const SwFlyFrm* pSelectedChildFlyFrame = NULL;

    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != NULL )
    {
        // Get the selected frame, and check if it's ours (rather
        // than, say, child of a paragraph).
        const SwFlyFrm* pFlyFrm = pFEShell->GetCurrFlyFrm();
        if( (pFlyFrm != NULL) &&
            (rContext.GetParent(pFlyFrm) == rContext.GetFrm()) )
            pSelectedChildFlyFrame = pFlyFrm;
    }
    // else: no FE-Shell -> no selected frames -> no selected children

    return pSelectedChildFlyFrame;
}


//=====  XAccessibleSelection  ============================================

void SwAccessibleSelectionHelper::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwFrmOrObj aChild = rContext.GetChild( nChildIndex );
    if( !aChild.IsValid() )
        throw IndexOutOfBoundsException();

    // we can only select fly frames, so we ignore (should: return
    // false) all other attempts at child selection
    sal_Bool bRet = sal_False;
    const SwFrm* pFrm = aChild.GetSwFrm();
    SwFEShell* pFEShell = GetFEShell();
    if( (pFEShell != NULL) && (pFrm != NULL) && (pFrm->IsFlyFrm()) )
    {
        pFEShell->SelectFlyFrm(
            *(static_cast<SwFlyFrm*>(const_cast<SwFrm*>(pFrm))), TRUE );
        bRet = sal_True;
    }
    // no frame shell, or no frame, or no fly frame -> can't select

    // return bRet;
}

sal_Bool SwAccessibleSelectionHelper::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwFrmOrObj aChild = rContext.GetChild( nChildIndex );
    if( !aChild.IsValid() )
        throw IndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    const SwFlyFrm* pSelectedFrame = GetSelectedChildFlyFrame();
    return (pSelectedFrame == aChild.GetSwFrm());
}

void SwAccessibleSelectionHelper::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    // return sal_False     // we can't deselect
}

void SwAccessibleSelectionHelper::selectAllAccessible(  )
    throw ( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // We can select only one. So iterate over the children to find
    // the first we can select, and select it.

    sal_Int32 nIndex = 0;
    const SwFlyFrm* pFirstSelectable = NULL;
    sal_Bool bContinue = sal_True;
    do
    {
        const SwFrmOrObj aChild = rContext.GetChild( nIndex );
        if( aChild.IsValid() )
        {
            const SwFrm* pFrm = aChild.GetSwFrm();
            if( (pFrm != NULL) && pFrm->IsFlyFrm() )
            {
                pFirstSelectable = static_cast<const SwFlyFrm*>( pFrm );
                bContinue = sal_False;
            }
        }
        else
            bContinue = sal_False;

        nIndex++;
    }
    while( bContinue );

    // select frame (if we found any)
    if( pFirstSelectable != NULL )
    {
        SwFEShell* pFEShell = GetFEShell();
        if( pFEShell != NULL )
        {
            pFEShell->SelectFlyFrm( *(const_cast<SwFlyFrm*>(pFirstSelectable)),
                                    TRUE );
        }
    }
}

sal_Int32 SwAccessibleSelectionHelper::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Only one frame can be selected at a time, and we only frames
    // for selectable children.
    return (GetSelectedChildFlyFrame() != NULL) ? 1 : 0;
}

Reference<XAccessible> SwAccessibleSelectionHelper::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    const SwFlyFrm* pFlyFrm = GetSelectedChildFlyFrame();

    // Since the index is relative to the selected children, and since
    // there can be at most one selected frame child, the index must
    // be 0, and a selection must exist, otherwise we have to throw an
    // IndexOutOfBoundsException

    if( (pFlyFrm == NULL) || (nSelectedChildIndex != 0) )
        throw IndexOutOfBoundsException();

    DBG_ASSERT( rContext.GetMap() != NULL, "We need the map." )
    return rContext.GetMap()->GetContext( pFlyFrm, sal_True );
}

void SwAccessibleSelectionHelper::deselectSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    // return sal_False     // we can't deselect
}
