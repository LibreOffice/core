 /*************************************************************************
 *
 *  $RCSfile: accdoc.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dvo $ $Date: 2002-04-12 09:19:43 $
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

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif

#pragma hdrstop

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _FESH_HXX
#include "fesh.hxx"
#endif

#ifndef _ACCDOC_HXX
#include <accdoc.hxx>
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "SwAccessibleDocument";

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

using ::com::sun::star::lang::IndexOutOfBoundsException;


SwFEShell* SwAccessibleDocument::GetFEShell()
{
    DBG_ASSERT( GetMap() != NULL, "no map?" );
    ViewShell* pViewShell = GetMap()->GetShell();
    DBG_ASSERT( pViewShell != NULL,
                "No view shell? Then what are you looking at?" );

    SwFEShell* pFEShell = NULL;
    if( pViewShell->ISA( SwFEShell ) )
    {
        pFEShell = static_cast<SwFEShell*>( pViewShell );
    }

    return pFEShell;
}

const SwFlyFrm* SwAccessibleDocument::GetSelectedChildFlyFrame()
{
    const SwFlyFrm* pSelectedChildFlyFrame = NULL;

    SwFEShell* pFEShell = GetFEShell();
    if( pFEShell != NULL )
    {
        // Get the selected frame, and check if it's ours (rather
        // than, say, child of a paragraph).
        const SwFlyFrm* pFlyFrm = pFEShell->GetCurrFlyFrm();
        if( (pFlyFrm != NULL) && (GetParent(pFlyFrm) == GetFrm()) )
            pSelectedChildFlyFrame = pFlyFrm;
    }
    // else: no FE-Shell -> no selected frames -> no selected children

    return pSelectedChildFlyFrame;
}


void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTISELECTABLE );
}

SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap *pMap ) :
    SwAccessibleContext( pMap, AccessibleRole::DOCUMENT,
                           pMap->GetShell()->GetDoc()->GetRootFrm() ),
    xParent( pMap->GetShell()->GetWin()->GetParent()->GetAccessible() )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
}

SwAccessibleDocument::~SwAccessibleDocument()
{
}

void SwAccessibleDocument::SetVisArea()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwRect aOldVisArea( GetVisArea() );
    const SwRect& rNewVisArea = GetMap()->GetVisArea();
    if( aOldVisArea != rNewVisArea )
    {
        SwAccessibleFrame::SetVisArea( GetMap()->GetVisArea() );
        ChildrenScrolled( GetFrm(), aOldVisArea );
    }
}


Reference< XAccessible> SAL_CALL SwAccessibleDocument::getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return xParent;
}

sal_Int32 SAL_CALL SwAccessibleDocument::getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference < XAccessibleContext > xAcc( xParent->getAccessibleContext() );
    Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        if( xAcc->getAccessibleChild( i ) == xThis )
            return i;
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocument::getAccessibleDescription (void) throw (com::sun::star::uno::RuntimeException)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

awt::Rectangle SAL_CALL SwAccessibleDocument::getBounds()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetParent() ) );
    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}


awt::Point SAL_CALL SwAccessibleDocument::getLocation()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetParent() ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleDocument::getLocationOnScreen()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleDocument::getSize()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}


OUString SAL_CALL SwAccessibleDocument::getImplementationName()
        throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName, sizeof(sServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    return aRet;
}

//=====  XInterface  ======================================================

Any SwAccessibleDocument::queryInterface(
    const Type& rType )
    throw ( RuntimeException )
{
    Any aRet;
    if ( rType == ::getCppuType((Reference<XAccessibleSelection> *)NULL) )
    {
        Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

//=====  XAccessibleSelection  ============================================

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwFrmOrObj aChild = GetChild( nChildIndex );
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

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Get the respective child as SwFrm (also do index checking), ...
    const SwFrmOrObj aChild = GetChild( nChildIndex );
    if( !aChild.IsValid() )
        throw IndexOutOfBoundsException();

    // ... and compare to the currently selected frame
    const SwFlyFrm* pSelectedFrame = GetSelectedChildFlyFrame();
    return (pSelectedFrame == aChild.GetSwFrm());
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    // return sal_False     // we can't deselect
}

void SwAccessibleDocument::selectAllAccessible(  )
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
        const SwFrmOrObj aChild = GetChild( nIndex );
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

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // Only one frame can be selected at a time, and we only frames
    // for selectable children.
    return (GetSelectedChildFlyFrame() != NULL) ? 1 : 0;
}

Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
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

    DBG_ASSERT( GetMap() != NULL, "We need the map." )
    return GetMap()->GetContext( pFlyFrm, sal_True );
}

void SwAccessibleDocument::deselectSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    // return sal_False     // we can't deselect
}
