/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <vcl/window.hxx>
#include <rootfrm.hxx>


#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <tools/link.hxx>
#include <sfx2/viewsh.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <accmap.hxx>
#include <accdoc.hxx>
#include "access.hrc"
#include <pagefrm.hxx>

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentView";


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

using lang::IndexOutOfBoundsException;



//
// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview
//

SwAccessibleDocumentBase::SwAccessibleDocumentBase ( SwAccessibleMap *_pMap ) :
    SwAccessibleContext( _pMap, AccessibleRole::DOCUMENT,
                         _pMap->GetShell()->GetLayout() ),//swmod 071107//swmod 071225
    mxParent( _pMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible() ),
    mpChildWin( 0 )
{
}

SwAccessibleDocumentBase::~SwAccessibleDocumentBase()
{
}

void SwAccessibleDocumentBase::SetVisArea()
{
    SolarMutexGuard aGuard;

    SwRect aOldVisArea( GetVisArea() );
    const SwRect& rNewVisArea = GetMap()->GetVisArea();
    if( aOldVisArea != rNewVisArea )
    {
        SwAccessibleFrame::SetVisArea( GetMap()->GetVisArea() );
        // #i58139# - showing state of document view needs also be updated.
        // Thus, call method <Scrolled(..)> instead of <ChildrenScrolled(..)>
//        ChildrenScrolled( GetFrm(), aOldVisArea );
        Scrolled( aOldVisArea );
    }
}

void SwAccessibleDocumentBase::AddChild( Window *pWin, sal_Bool bFireEvent )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !mpChildWin, "only one child window is supported" );
    if( !mpChildWin )
    {
        mpChildWin = pWin;

        if( bFireEvent )
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.NewValue <<= mpChildWin->GetAccessible();
            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleDocumentBase::RemoveChild( Window *pWin )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !mpChildWin || pWin == mpChildWin, "invalid child window to remove" );
    if( mpChildWin && pWin == mpChildWin )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= mpChildWin->GetAccessible();
        FireAccessibleEvent( aEvent );

        mpChildWin = 0;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount( void )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // CHECK_FOR_DEFUNC is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException,
                lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleContext )
        if ( nIndex == GetChildCount( *(GetMap()) ) )
        {
            return mpChildWin->GetAccessible();
        }
    }

    return SwAccessibleContext::getAccessibleChild( nIndex );
}


uno::Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent (void)
        throw (uno::RuntimeException)
{
    return mxParent;
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference < XAccessibleContext > xAcc( mxParent->getAccessibleContext() );
    uno::Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        if( xAcc->getAccessibleChild( i ) == xThis )
            return i;
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ) );
    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}


awt::Point SAL_CALL SwAccessibleDocumentBase::getLocation()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::containsPoint(
            const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( 0 ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleComponent )

        Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

//
// SwAccessibeDocument
//

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
}


SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap* pInitMap ) :
    SwAccessibleDocumentBase( pInitMap ),
    maSelectionHelper( *this )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
    Window *pWin = pInitMap->GetShell()->GetWin();
    if( pWin )
    {
        pWin->AddChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
        sal_uInt16 nCount =   pWin->GetChildCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            Window* pChildWin = pWin->GetChild( i );
            if( pChildWin &&
                AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                AddChild( pChildWin, sal_False );
        }
    }
}

SwAccessibleDocument::~SwAccessibleDocument()
{
    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose( sal_Bool bRecursive )
{
    OSL_ENSURE( GetFrm() && GetMap(), "already disposed" );

    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
    SwAccessibleContext::Dispose( bRecursive );
}

IMPL_LINK( SwAccessibleDocument, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        VclWindowEvent *pVclEvent = static_cast< VclWindowEvent * >( pEvent );
        DBG_ASSERT( pVclEvent->GetWindow(), "Window???" );
        switch ( pVclEvent->GetId() )
        {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
            {
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    AddChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_OBJECT_DYING:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = pVclEvent->GetWindow();
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
        }
    }
    return 0;
}


OUString SAL_CALL SwAccessibleDocument::getImplementationName()
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

//=====  XInterface  ======================================================

uno::Any SwAccessibleDocument::queryInterface(
    const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;
    if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleDocument::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleDocumentBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

//=====  XAccessibleSelection  ============================================

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    return maSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleDocument::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
