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

#include <vcl/window.hxx>
#include <swtypes.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <viewsh.hxx>
#include <crsrsh.hxx>
#include <fesh.hxx>
#include <wrtsh.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <flyfrm.hxx>
#include <dflyobj.hxx>
#include <pam.hxx>
#include <viewimp.hxx>
#include <accmap.hxx>
#include <accfrmobjslist.hxx>
#include <acccontext.hxx>
#include <svx/AccessibleShape.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <PostItMgr.hxx>

using namespace sw::access;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

void SwAccessibleContext::InitStates()
{
    bIsShowingState = GetMap() ? IsShowing( *(GetMap()) ) : sal_False;

    ViewShell *pVSh = GetMap()->GetShell();
    bIsEditableState = pVSh && IsEditable( pVSh );
    bIsOpaqueState = pVSh && IsOpaque( pVSh );
    bIsDefuncState = sal_False;
}

void SwAccessibleContext::SetParent( SwAccessibleContext *pParent )
{
    osl::MutexGuard aGuard( aMutex );

    uno::Reference < XAccessible > xParent( pParent );
    xWeakParent = xParent;
}

uno::Reference< XAccessible > SwAccessibleContext::GetWeakParent() const
{
    osl::MutexGuard aGuard( aMutex );

    uno::Reference< XAccessible > xParent( xWeakParent );
    return xParent;
}

Window *SwAccessibleContext::GetWindow()
{
    Window *pWin = 0;

    if( GetMap() )
    {
        const ViewShell *pVSh = GetMap()->GetShell();
        OSL_ENSURE( pVSh, "no view shell" );
        if( pVSh )
            pWin = pVSh->GetWin();

        OSL_ENSURE( pWin, "no window" );
    }

    return pWin;
}

// get ViewShell from accessibility map, and cast to cursor shell
SwCrsrShell* SwAccessibleContext::GetCrsrShell()
{
    SwCrsrShell* pCrsrShell;
    ViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : 0;
    OSL_ENSURE( pViewShell, "no view shell" );
    if( pViewShell && pViewShell->ISA( SwCrsrShell ) )
        pCrsrShell = static_cast<SwCrsrShell*>( pViewShell );
    else
        pCrsrShell = NULL;

    return pCrsrShell;
}

const SwCrsrShell* SwAccessibleContext::GetCrsrShell() const
{
    // just like non-const GetCrsrShell
    const SwCrsrShell* pCrsrShell;
    const ViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : 0;
    OSL_ENSURE( pViewShell, "no view shell" );
    if( pViewShell && pViewShell->ISA( SwCrsrShell ) )
        pCrsrShell = static_cast<const SwCrsrShell*>( pViewShell );
    else
        pCrsrShell = NULL;

    return pCrsrShell;
}

enum Action { NONE, SCROLLED, SCROLLED_WITHIN,
                          SCROLLED_IN, SCROLLED_OUT };

void SwAccessibleContext::ChildrenScrolled( const SwFrm *pFrm,
                                            const SwRect& rOldVisArea )
{
    const SwRect& rNewVisArea = GetVisArea();
    const bool bVisibleChildrenOnly = SwAccessibleChild( pFrm ).IsVisibleChildrenOnly();

    const SwAccessibleChildSList aList( *pFrm, *(GetMap()) );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwRect aBox( rLower.GetBox( *(GetMap()) ) );
        if( rLower.IsAccessible( GetShell()->IsPreView() ) )
        {
            Action eAction = NONE;
            if( aBox.IsOver( rNewVisArea ) )
            {
                if( aBox.IsOver( rOldVisArea ) )
                {
                    eAction = SCROLLED_WITHIN;
                }
                else
                {
                    if ( bVisibleChildrenOnly &&
                         !rLower.AlwaysIncludeAsChild() )
                    {
                        eAction = SCROLLED_IN;
                    }
                    else
                    {
                        eAction = SCROLLED;
                    }
                }
            }
            else if( aBox.IsOver( rOldVisArea ) )
            {
                if ( bVisibleChildrenOnly &&
                     !rLower.AlwaysIncludeAsChild() )
                {
                    eAction = SCROLLED_OUT;
                }
                else
                {
                    eAction = SCROLLED;
                }
            }
            else if( !bVisibleChildrenOnly ||
                     rLower.AlwaysIncludeAsChild() )
            {
                // This wouldn't be required if the SwAccessibleFrame,
                // wouldn't know about the visible area.
                eAction = SCROLLED;
            }
            if( NONE != eAction )
            {
                if ( rLower.GetSwFrm() )
                {
                    OSL_ENSURE( !rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - always included child not considered!" );
                    const SwFrm* pLower( rLower.GetSwFrm() );
                    ::rtl::Reference< SwAccessibleContext > xAccImpl =
                        GetMap()->GetContextImpl( pLower, SCROLLED_OUT == eAction ||
                                                SCROLLED_IN == eAction );
                    if( xAccImpl.is() )
                    {
                        switch( eAction )
                        {
                        case SCROLLED:
                            xAccImpl->Scrolled( rOldVisArea );
                            break;
                        case SCROLLED_WITHIN:
                            xAccImpl->ScrolledWithin( rOldVisArea );
                            break;
                        case SCROLLED_IN:
                            xAccImpl->ScrolledIn();
                            break;
                        case SCROLLED_OUT:
                            xAccImpl->ScrolledOut( rOldVisArea );
                            break;
                        case NONE:
                            break;
                        }
                    }
                    else
                    {
                        ChildrenScrolled( pLower, rOldVisArea );
                    }
                }
                else if ( rLower.GetDrawObject() )
                {
                    OSL_ENSURE( !rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - always included child not considered!" );
                    ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rLower.GetDrawObject(),
                                                  this,
                                                  SCROLLED_OUT == eAction ||
                                                  SCROLLED_IN == eAction );
                    if( xAccImpl.is() )
                    {
                        switch( eAction )
                        {
                        case SCROLLED:
                        case SCROLLED_WITHIN:
                            xAccImpl->ViewForwarderChanged(
                                ::accessibility::IAccessibleViewForwarderListener::VISIBLE_AREA,
                                GetMap() );
                            break;
                        case SCROLLED_IN:
                            ScrolledInShape( rLower.GetDrawObject(),
                                             xAccImpl.get() );
                            break;
                        case SCROLLED_OUT:
                            {
                                xAccImpl->ViewForwarderChanged(
                                    ::accessibility::IAccessibleViewForwarderListener::VISIBLE_AREA,
                                    GetMap() );
                                DisposeShape( rLower.GetDrawObject(),
                                              xAccImpl.get() );
                            }
                            break;
                        case NONE:
                            break;
                        }
                    }
                }
                else if ( rLower.GetWindow() )
                {
                    // nothing to do - as such children are always included as children.
                    OSL_ENSURE( rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - not always included child not considered!" );
                }
            }
        }
        else if ( rLower.GetSwFrm() &&
                  ( !bVisibleChildrenOnly ||
                    aBox.IsOver( rOldVisArea ) ||
                    aBox.IsOver( rNewVisArea ) ) )
        {
            // There are no unaccessible SdrObjects that need to be notified
            ChildrenScrolled( rLower.GetSwFrm(), rOldVisArea );
        }
        ++aIter;
    }
}

void SwAccessibleContext::Scrolled( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    ChildrenScrolled( GetFrm(), rOldVisArea );

    sal_Bool bIsOldShowingState;
    sal_Bool bIsNewShowingState = IsShowing( *(GetMap()) );
    {
        osl::MutexGuard aGuard( aMutex );
        bIsOldShowingState = bIsShowingState;
        bIsShowingState = bIsNewShowingState;
    }

    if( bIsOldShowingState != bIsNewShowingState )
        FireStateChangedEvent( AccessibleStateType::SHOWING,
                               bIsNewShowingState  );
}

void SwAccessibleContext::ScrolledWithin( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    ChildrenScrolled( GetFrm(), rOldVisArea );

    FireVisibleDataEvent();
}

void SwAccessibleContext::ScrolledIn()
{
    // This accessible should be freshly created, because it
    // was not visisble before. Therefor, its vis area must already
    // reflect the scrolling.
    OSL_ENSURE( GetVisArea() == GetMap()->GetVisArea(),
            "Vis area of child is wrong. Did it exist already?" );

    // Send child event at parent. That's all we have to do here.
    const SwFrm* pParent = GetParent();
    ::rtl::Reference< SwAccessibleContext > xParentImpl(
         GetMap()->GetContextImpl( pParent, sal_False ) );
    uno::Reference < XAccessibleContext > xThis( this );
    if( xParentImpl.is() )
    {
        SetParent( xParentImpl.get() );

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.NewValue <<= xThis;

        xParentImpl->FireAccessibleEvent( aEvent );

        if( HasCursor() )
        {
            Window *pWin = GetWindow();
            if( pWin && pWin->HasFocus() )
            {
                FireStateChangedEvent( AccessibleStateType::FOCUSED, sal_True );
            }
        }

    }
}

void SwAccessibleContext::ScrolledOut( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    // First of all, update the children. That's required to dispose
    // all children that are existing only if they are visible. They
    // are not disposed by the recusive Dispose call that follows later on,
    // because this call will only dispose children that are in the
    // new vis area. The children we want to dispode however are in the
    // old vis area all.
    ChildrenScrolled( GetFrm(), rOldVisArea );

    // Broadcast a state changed event for the showing state.
    // It might be that the child is freshly created just to send
    // the child event. In this case no listener will exist.
    FireStateChangedEvent( AccessibleStateType::SHOWING, sal_False );

    // We now dispose the frame
    Dispose( sal_True );
}

// #i27301# - use new type definition for <_nStates>
void SwAccessibleContext::InvalidateChildrenStates( const SwFrm* _pFrm,
                                                    tAccessibleStates _nStates )
{
    const SwAccessibleChildSList aVisList( GetVisArea(), *_pFrm, *(GetMap()) );

    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm* pLower = rLower.GetSwFrm();
        if( pLower )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreView() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, sal_False );
            if( xAccImpl.is() )
                xAccImpl->InvalidateStates( _nStates );
            else
                InvalidateChildrenStates( pLower, _nStates );
        }
        else if ( rLower.GetDrawObject() )
        {
            // TODO: SdrObjects
        }
        else if ( rLower.GetWindow() )
        {
            // nothing to do ?
        }

        ++aIter;
    }
}

void SwAccessibleContext::DisposeChildren( const SwFrm *pFrm,
                                       sal_Bool bRecursive )
{
    const SwAccessibleChildSList aVisList( GetVisArea(), *pFrm, *(GetMap()) );
    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrm* pLower = rLower.GetSwFrm();
        if( pLower )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreView() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, sal_False );
            if( xAccImpl.is() )
                xAccImpl->Dispose( bRecursive );
            else if( bRecursive )
                DisposeChildren( pLower, bRecursive );
        }
        else if ( rLower.GetDrawObject() )
        {
            ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl(
                    GetMap()->GetContextImpl( rLower.GetDrawObject(),
                                          this, sal_False )  );
            if( xAccImpl.is() )
                DisposeShape( rLower.GetDrawObject(), xAccImpl.get() );
        }
        else if ( rLower.GetWindow() )
        {
            DisposeChild( rLower, sal_False );
        }
        ++aIter;
    }
}

void SwAccessibleContext::_InvalidateContent( sal_Bool )
{
}

void SwAccessibleContext::_InvalidateCursorPos()
{
}

void SwAccessibleContext::_InvalidateFocus()
{
}

void SwAccessibleContext::FireAccessibleEvent( AccessibleEventObject& rEvent )
{
    OSL_ENSURE( GetFrm(), "fire event for diposed frame?" );
    if( !GetFrm() )
        return;

    if( !rEvent.Source.is() )
    {
        uno::Reference < XAccessibleContext > xThis( this );
        rEvent.Source = xThis;
    }

    if (nClientId)
        comphelper::AccessibleEventNotifier::addEvent( nClientId, rEvent );
}

void SwAccessibleContext::FireVisibleDataEvent()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;

    FireAccessibleEvent( aEvent );
}

void SwAccessibleContext::FireStateChangedEvent( sal_Int16 nState,
                                                 sal_Bool bNewState )
{
    AccessibleEventObject aEvent;

    aEvent.EventId = AccessibleEventId::STATE_CHANGED;
    if( bNewState )
        aEvent.NewValue <<= nState;
    else
        aEvent.OldValue <<= nState;

    FireAccessibleEvent( aEvent );
}

void SwAccessibleContext::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SolarMutexGuard aGuard;

    // SHOWING
    if( bIsShowingState )
        rStateSet.AddState( AccessibleStateType::SHOWING );

    // EDITABLE
    if( bIsEditableState )
        rStateSet.AddState( AccessibleStateType::EDITABLE );

    // ENABLED
    rStateSet.AddState( AccessibleStateType::ENABLED );

    // OPAQUE
    if( bIsOpaqueState )
        rStateSet.AddState( AccessibleStateType::OPAQUE );

    // VISIBLE
    rStateSet.AddState( AccessibleStateType::VISIBLE );

    if( bIsDefuncState )
        rStateSet.AddState( AccessibleStateType::DEFUNC );
}

sal_Bool SwAccessibleContext::IsEditableState()
{
    sal_Bool bRet;
    {
        osl::MutexGuard aGuard( aMutex );
        bRet = bIsEditableState;
    }

    return bRet;
}

SwAccessibleContext::SwAccessibleContext( SwAccessibleMap *pM,
                                          sal_Int16 nR,
                                          const SwFrm *pF )
    : SwAccessibleFrame( pM->GetVisArea().SVRect(), pF,
                         pM->GetShell()->IsPreView() )
    , pMap( pM )
    , nClientId(0)
    , nRole( nR )
    , bDisposing( sal_False )
    , bRegisteredAtAccessibleMap( true )
{
    InitStates();
}

SwAccessibleContext::~SwAccessibleContext()
{
    SolarMutexGuard aGuard;
    RemoveFrmFromAccessibleMap();
}

uno::Reference< XAccessibleContext > SAL_CALL
    SwAccessibleContext::getAccessibleContext( void )
        throw (uno::RuntimeException)
{
    uno::Reference < XAccessibleContext > xRet( this );
    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleChildCount( void )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return bDisposing ? 0 : GetChildCount( *(GetMap()) );
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwAccessibleChild aChild( GetChild( *(GetMap()), nIndex ) );
    if( !aChild.IsValid() )
    {
        uno::Reference < XAccessibleContext > xThis( this );
        lang::IndexOutOfBoundsException aExcept(
                OUString( "index out of bounds" ),
                xThis );
        throw aExcept;
    }

    uno::Reference< XAccessible > xChild;
    if( aChild.GetSwFrm() )
    {
        ::rtl::Reference < SwAccessibleContext > xChildImpl(
                GetMap()->GetContextImpl( aChild.GetSwFrm(), !bDisposing )  );
        if( xChildImpl.is() )
        {
            xChildImpl->SetParent( this );
            xChild = xChildImpl.get();
        }
    }
    else if ( aChild.GetDrawObject() )
    {
        ::rtl::Reference < ::accessibility::AccessibleShape > xChildImpl(
                GetMap()->GetContextImpl( aChild.GetDrawObject(),
                                          this, !bDisposing )  );
        if( xChildImpl.is() )
            xChild = xChildImpl.get();
    }
    else if ( aChild.GetWindow() )
    {
        xChild = aChild.GetWindow()->GetAccessible();
    }

    return xChild;
}

uno::Reference< XAccessible> SAL_CALL SwAccessibleContext::getAccessibleParent (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    OSL_ENSURE( pUpper != 0 || bDisposing, "no upper found" );

    uno::Reference< XAccessible > xAcc;
    if( pUpper )
        xAcc = GetMap()->GetContext( pUpper, !bDisposing );

    OSL_ENSURE( xAcc.is() || bDisposing, "no parent found" );

    // Remember the parent as weak ref.
    {
        osl::MutexGuard aWeakParentGuard( aMutex );
        xWeakParent = xAcc;
    }

    return xAcc;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    OSL_ENSURE( pUpper != 0 || bDisposing, "no upper found" );

    sal_Int32 nIndex = -1;
    if( pUpper )
    {
        ::rtl::Reference < SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pUpper, !bDisposing )  );
        OSL_ENSURE( xAccImpl.is() || bDisposing, "no parent found" );
        if( xAccImpl.is() )
            nIndex = xAccImpl->GetChildIndex( *(GetMap()), SwAccessibleChild(GetFrm()) );
    }

    return nIndex;
}

sal_Int16 SAL_CALL SwAccessibleContext::getAccessibleRole (void)
        throw (uno::RuntimeException)
{
    return nRole;
}

OUString SAL_CALL SwAccessibleContext::getAccessibleDescription (void)
        throw (uno::RuntimeException)
{
    OSL_ENSURE( !this, "description needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (method must be overloaded)" );
}

OUString SAL_CALL SwAccessibleContext::getAccessibleName (void)
        throw (uno::RuntimeException)
{
    return sName;
}

uno::Reference< XAccessibleRelationSet> SAL_CALL
    SwAccessibleContext::getAccessibleRelationSet (void)
        throw (uno::RuntimeException)
{
    // by default there are no relations
    uno::Reference< XAccessibleRelationSet> xRet( new utl::AccessibleRelationSetHelper() );
    return xRet;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    SwAccessibleContext::getAccessibleStateSet (void)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    ::utl::AccessibleStateSetHelper *pStateSet =
        new ::utl::AccessibleStateSetHelper;

    uno::Reference<XAccessibleStateSet> xStateSet( pStateSet );
    GetStates( *pStateSet );

    return xStateSet;
}

lang::Locale SAL_CALL SwAccessibleContext::getLocale (void)
        throw (IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    lang::Locale aLoc( Application::GetSettings().GetLanguageTag().getLocale() );
    return aLoc;
}

void SAL_CALL SwAccessibleContext::addAccessibleEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
        throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!nClientId)
            nClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( nClientId, xListener );
    }
}

void SAL_CALL SwAccessibleContext::removeAccessibleEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
        throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( nClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( nClientId );
            nClientId = 0;
        }
    }
}

static sal_Bool lcl_PointInRectangle(const awt::Point & aPoint,
                                     const awt::Rectangle & aRect)
{
    long nDiffX = aPoint.X - aRect.X;
    long nDiffY = aPoint.Y - aRect.Y;

    return
        nDiffX >= 0 && nDiffX < aRect.Width && nDiffY >= 0 &&
        nDiffY < aRect.Height;

}

sal_Bool SAL_CALL SwAccessibleContext::containsPoint(
            const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    awt::Rectangle aPixBounds = getBoundsImpl(sal_True);
    aPixBounds.X = 0;
    aPixBounds.Y = 0;

    return lcl_PointInRectangle(aPoint, aPixBounds);
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleContext::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    uno::Reference< XAccessible > xAcc;

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to parent
    if( !GetFrm()->IsRootFrm() )
    {
        SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
        Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
        aPixPoint.setX(aPixPoint.getX() + aPixPos.getX());
        aPixPoint.setY(aPixPoint.getY() + aPixPos.getY());
    }

    const SwAccessibleChild aChild( GetChildAtPixel( aPixPoint, *(GetMap()) ) );
    if( aChild.GetSwFrm() )
    {
        xAcc = GetMap()->GetContext( aChild.GetSwFrm() );
    }
    else if( aChild.GetDrawObject() )
    {
        xAcc = GetMap()->GetContext( aChild.GetDrawObject(), this );
    }
    else if ( aChild.GetWindow() )
    {
        xAcc = aChild.GetWindow()->GetAccessible();
    }

    return xAcc;
}

/**
   Get bounding box.

   There are two modes.

   - relative

     Return bounding box relative to parent if parent is no root
     frame. Otherwise return the absolute bounding box.

   - absolute

     Return the absolute bounding box.

   @param bRelative
   true: Use relative mode.
   false: Use absolute mode.
*/
awt::Rectangle SAL_CALL SwAccessibleContext::getBoundsImpl(sal_Bool bRelative)
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pParent = GetParent();
    OSL_ENSURE( pParent, "no Parent found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pParent )

    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
    Rectangle aPixBounds( 0, 0, 0, 0 );
    if( GetFrm()->IsPageFrm() &&
        static_cast < const SwPageFrm * >( GetFrm() )->IsEmptyPage() )
    {
        OSL_ENSURE( GetShell()->IsPreView(), "empty page accessible?" );
        if( GetShell()->IsPreView() )
        {
            // adjust method call <GetMap()->GetPreViewPageSize()>
            sal_uInt16 nPageNum =
                static_cast < const SwPageFrm * >( GetFrm() )->GetPhyPageNum();
            aLogBounds.SSize( GetMap()->GetPreViewPageSize( nPageNum ) );
        }
    }
    if( !aLogBounds.IsEmpty() )
    {
        aPixBounds = GetMap()->CoreToPixel( aLogBounds.SVRect() );
        if( !pParent->IsRootFrm() && bRelative)
        {
            SwRect aParentLogBounds( GetBounds( *(GetMap()), pParent ) ); // twip rel to doc root
            Point aParentPixPos( GetMap()->CoreToPixel( aParentLogBounds.SVRect() ).TopLeft() );
            aPixBounds.Move( -aParentPixPos.getX(), -aParentPixPos.getY() );
        }
    }

    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}

awt::Rectangle SAL_CALL SwAccessibleContext::getBounds()
        throw (uno::RuntimeException)
{
    return getBoundsImpl(sal_True);
}

awt::Point SAL_CALL SwAccessibleContext::getLocation()
    throw (uno::RuntimeException)
{
    awt::Rectangle aRect = getBoundsImpl(sal_True);
    awt::Point aPoint(aRect.X, aRect.Y);

    return aPoint;
}

awt::Point SAL_CALL SwAccessibleContext::getLocationOnScreen()
        throw (uno::RuntimeException)
{
    awt::Rectangle aRect = getBoundsImpl(sal_False);

    Point aPixPos(aRect.X, aRect.Y);

    /* getBoundsImpl already checked that GetWindow returns valid pointer. */
    aPixPos = GetWindow()->OutputToAbsoluteScreenPixel(aPixPos);
    awt::Point aPoint(aPixPos.getX(), aPixPos.getY());

    return aPoint;
}

awt::Size SAL_CALL SwAccessibleContext::getSize()
        throw (uno::RuntimeException)
{
    awt::Rectangle aRect = getBoundsImpl(sal_False);
    awt::Size aSize( aRect.Width, aRect.Height );

    return aSize;
}

void SAL_CALL SwAccessibleContext::grabFocus()
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    if( GetFrm()->IsFlyFrm() )
    {
        const SdrObject *pObj =
            static_cast < const SwFlyFrm * >( GetFrm() )->GetVirtDrawObj();
        if( pObj )
            Select( const_cast < SdrObject * >( pObj ), sal_False );
    }
    else
    {
        const SwCntntFrm *pCFrm = 0;
        if( GetFrm()->IsCntntFrm() )
            pCFrm = static_cast< const SwCntntFrm * >( GetFrm() );
        else if( GetFrm()->IsLayoutFrm() )
            pCFrm = static_cast< const SwLayoutFrm * >( GetFrm() )->ContainsCntnt();

        if( pCFrm && pCFrm->IsTxtFrm() )
        {
            const SwTxtFrm *pTxtFrm = static_cast< const SwTxtFrm * >( pCFrm );
            const SwTxtNode *pTxtNd = pTxtFrm->GetTxtNode();
            if( pTxtNd )
            {
                // create pam for selection
                SwIndex aIndex( const_cast< SwTxtNode * >( pTxtNd ),
                                pTxtFrm->GetOfst() );
                SwPosition aStartPos( *pTxtNd, aIndex );
                SwPaM aPaM( aStartPos );

                // set PaM at cursor shell
                Select( aPaM );
            }
        }
    }
}

uno::Any SAL_CALL SwAccessibleContext::getAccessibleKeyBinding()
        throw (uno::RuntimeException)
{
    // There are no key bindings
    return uno::Any();
}

sal_Int32 SAL_CALL SwAccessibleContext::getForeground()
        throw (uno::RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL SwAccessibleContext::getBackground()
        throw (uno::RuntimeException)
{
    return 0xffffff;
}

OUString SAL_CALL SwAccessibleContext::getImplementationName()
        throw( uno::RuntimeException )
{
    OSL_ENSURE( !this, "implementation name needs to be overloaded" );

    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "implementation name needs to be overloaded" )
}

sal_Bool SAL_CALL
    SwAccessibleContext::supportsService (const OUString& )
        throw (uno::RuntimeException)
{
    OSL_ENSURE( !this, "supports service needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "supports service needs to be overloaded" )
}

uno::Sequence< OUString > SAL_CALL SwAccessibleContext::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    OSL_ENSURE( !this, "supported services names needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "supported services needs to be overloaded" )
}

void SwAccessibleContext::DisposeShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl )
{
    ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl( pAccImpl );
    if( !xAccImpl.is() )
        xAccImpl = GetMap()->GetContextImpl( pObj, this, sal_True );

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::CHILD;
    uno::Reference< XAccessible > xAcc( xAccImpl.get() );
    aEvent.OldValue <<= xAcc;
    FireAccessibleEvent( aEvent );

    GetMap()->RemoveContext( pObj );
    xAccImpl->dispose();
}

void SwAccessibleContext::ScrolledInShape( const SdrObject* ,
                                ::accessibility::AccessibleShape *pAccImpl )
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::CHILD;
    uno::Reference< XAccessible > xAcc( pAccImpl );
    aEvent.NewValue <<= xAcc;
    FireAccessibleEvent( aEvent );

    if( pAccImpl->GetState( AccessibleStateType::FOCUSED ) )
    {
        Window *pWin = GetWindow();
        if( pWin && pWin->HasFocus() )
        {
            AccessibleEventObject aStateChangedEvent;
            aStateChangedEvent.EventId = AccessibleEventId::STATE_CHANGED;
            aStateChangedEvent.NewValue <<= AccessibleStateType::FOCUSED;
            aStateChangedEvent.Source = xAcc;

            FireAccessibleEvent( aStateChangedEvent );
        }
    }
}

void SwAccessibleContext::Dispose( sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( GetFrm() && GetMap(), "already disposed" );
    OSL_ENSURE( GetMap()->GetVisArea() == GetVisArea(),
                "invalid vis area for dispose" );

    bDisposing = sal_True;

    // dispose children
    if( bRecursive )
        DisposeChildren( GetFrm(), bRecursive );

    // get parent
    uno::Reference< XAccessible > xParent( GetWeakParent() );
    uno::Reference < XAccessibleContext > xThis( this );

    // send child event at parent
    if( xParent.is() )
    {
        SwAccessibleContext *pAcc = (SwAccessibleContext *)xParent.get();

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= xThis;
        pAcc->FireAccessibleEvent( aEvent );
    }

    // set defunc state (its not required to broadcast a state changed
    // event if the object is diposed afterwards)
    {
        osl::MutexGuard aDefuncStateGuard( aMutex );
        bIsDefuncState = sal_True;
    }

    // broadcast dispose event
    if ( nClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, *this );
        nClientId =  0;
    }

    RemoveFrmFromAccessibleMap();
    ClearFrm();
    pMap = 0;

    bDisposing = sal_False;
}

void SwAccessibleContext::DisposeChild( const SwAccessibleChild& rChildFrmOrObj,
                                        sal_Bool bRecursive )
{
    SolarMutexGuard aGuard;

    if ( IsShowing( *(GetMap()), rChildFrmOrObj ) ||
         rChildFrmOrObj.AlwaysIncludeAsChild() ||
         !SwAccessibleChild( GetFrm() ).IsVisibleChildrenOnly() )
    {
        // If the object could have existed before, than there is nothing to do,
        // because no wrapper exists now and therefor no one is interested to
        // get notified of the movement.
        if( rChildFrmOrObj.GetSwFrm() )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
            xAccImpl->Dispose( bRecursive );
        }
        else if ( rChildFrmOrObj.GetDrawObject() )
        {
            ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                              this, sal_True );
            DisposeShape( rChildFrmOrObj.GetDrawObject(),
                          xAccImpl.get() );
        }
        else if ( rChildFrmOrObj.GetWindow() )
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            uno::Reference< XAccessible > xAcc =
                                    rChildFrmOrObj.GetWindow()->GetAccessible();
            aEvent.OldValue <<= xAcc;
            FireAccessibleEvent( aEvent );
        }
    }
    else if( bRecursive && rChildFrmOrObj.GetSwFrm() )
        DisposeChildren( rChildFrmOrObj.GetSwFrm(), bRecursive );
}

void SwAccessibleContext::InvalidatePosOrSize( const SwRect& )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( GetFrm() && !GetFrm()->Frm().IsEmpty(), "context should have a size" );

    sal_Bool bIsOldShowingState;
    sal_Bool bIsNewShowingState = IsShowing( *(GetMap()) );
    {
        osl::MutexGuard aShowingStateGuard( aMutex );
        bIsOldShowingState = bIsShowingState;
        bIsShowingState = bIsNewShowingState;
    }

    if( bIsOldShowingState != bIsNewShowingState )
    {
        FireStateChangedEvent( AccessibleStateType::SHOWING,
                               bIsNewShowingState  );
    }
    else if( bIsNewShowingState )
    {
        // The frame stays visible -> broadcast event
        FireVisibleDataEvent();
    }

    if( !bIsNewShowingState &&
        SwAccessibleChild( GetParent() ).IsVisibleChildrenOnly() )
    {
        // The frame is now invisible -> dispose it
        Dispose( sal_True );
    }
    else
    {
        _InvalidateContent( sal_True );
    }
}

void SwAccessibleContext::InvalidateChildPosOrSize(
                    const SwAccessibleChild& rChildFrmOrObj,
                    const SwRect& rOldFrm )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( !rChildFrmOrObj.GetSwFrm() ||
            !rChildFrmOrObj.GetSwFrm()->Frm().IsEmpty(),
            "child context should have a size" );

    if ( rChildFrmOrObj.AlwaysIncludeAsChild() )
    {
        // nothing to do;
        return;
    }

    const bool bVisibleChildrenOnly = SwAccessibleChild( GetFrm() ).IsVisibleChildrenOnly();
    const bool bNew = rOldFrm.IsEmpty() ||
                     ( rOldFrm.Left() == 0 && rOldFrm.Top() == 0 );
    if( IsShowing( *(GetMap()), rChildFrmOrObj ) )
    {
        // If the object could have existed before, than there is nothing to do,
        // because no wrapper exists now and therefor no one is interested to
        // get notified of the movement.
        if( bNew || (bVisibleChildrenOnly && !IsShowing( rOldFrm )) )
        {
            if( rChildFrmOrObj.GetSwFrm() )
            {
                // The frame becomes visible. A child event must be send.
                ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
                xAccImpl->ScrolledIn();
            }
            else if ( rChildFrmOrObj.GetDrawObject() )
            {
                ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                                  this, sal_True );
                // #i37790#
                if ( xAccImpl.is() )
                {
                    ScrolledInShape( rChildFrmOrObj.GetDrawObject(),
                                     xAccImpl.get() );
                }
                else
                {
                    OSL_FAIL( "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - no accessible shape found." );
                }
            }
            else if ( rChildFrmOrObj.GetWindow() )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::CHILD;
                aEvent.NewValue <<= (rChildFrmOrObj.GetWindow()->GetAccessible());
                FireAccessibleEvent( aEvent );
            }
        }
    }
    else
    {
        // If the frame was visible before, than a child event for the parent
        // needs to be send. However, there is no wrapper existing, and so
        // no notifications for grandchildren are required. If the are
        // grandgrandchildren, they would be notified by the layout.
        if( bVisibleChildrenOnly &&
            !bNew && IsShowing( rOldFrm ) )
        {
            if( rChildFrmOrObj.GetSwFrm() )
            {
                ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
                xAccImpl->SetParent( this );
                xAccImpl->Dispose( sal_True );
            }
            else if ( rChildFrmOrObj.GetDrawObject() )
            {
                ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                                  this, sal_True );
                DisposeShape( rChildFrmOrObj.GetDrawObject(),
                          xAccImpl.get() );
            }
            else if ( rChildFrmOrObj.GetWindow() )
            {
                OSL_FAIL( "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - not expected to handle dispose of child of type <Window>." );
            }
        }
    }
}

void SwAccessibleContext::InvalidateContent()
{
    SolarMutexGuard aGuard;

    _InvalidateContent( sal_False );
}

void SwAccessibleContext::InvalidateCursorPos()
{
    SolarMutexGuard aGuard;

    _InvalidateCursorPos();
}

void SwAccessibleContext::InvalidateFocus()
{
    SolarMutexGuard aGuard;

    _InvalidateFocus();
}

// #i27301# - use new type definition for <_nStates>
void SwAccessibleContext::InvalidateStates( tAccessibleStates _nStates )
{
    if( GetMap() )
    {
        ViewShell *pVSh = GetMap()->GetShell();
        if( pVSh )
        {
            if( (_nStates & ACC_STATE_EDITABLE) != 0 )
            {
                sal_Bool bIsOldEditableState;
                sal_Bool bIsNewEditableState = IsEditable( pVSh );
                {
                    osl::MutexGuard aGuard( aMutex );
                    bIsOldEditableState = bIsEditableState;
                    bIsEditableState = bIsNewEditableState;
                }

                if( bIsOldEditableState != bIsNewEditableState )
                    FireStateChangedEvent( AccessibleStateType::EDITABLE,
                                           bIsNewEditableState  );
            }
            if( (_nStates & ACC_STATE_OPAQUE) != 0 )
            {
                sal_Bool bIsOldOpaqueState;
                sal_Bool bIsNewOpaqueState = IsOpaque( pVSh );
                {
                    osl::MutexGuard aGuard( aMutex );
                    bIsOldOpaqueState = bIsOpaqueState;
                    bIsOpaqueState = bIsNewOpaqueState;
                }

                if( bIsOldOpaqueState != bIsNewOpaqueState )
                    FireStateChangedEvent( AccessibleStateType::OPAQUE,
                                           bIsNewOpaqueState  );
            }
        }

        InvalidateChildrenStates( GetFrm(), _nStates );
    }
}

void SwAccessibleContext::InvalidateRelation( sal_uInt16 nType )
{
    AccessibleEventObject aEvent;
    aEvent.EventId = nType;

    FireAccessibleEvent( aEvent );
}

/** #i27301# - text selection has changed */
void SwAccessibleContext::InvalidateTextSelection()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::TEXT_SELECTION_CHANGED;

    FireAccessibleEvent( aEvent );
}

/** #i88069# - attributes has changed */
void SwAccessibleContext::InvalidateAttr()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::TEXT_ATTRIBUTE_CHANGED;

    FireAccessibleEvent( aEvent );
}

sal_Bool SwAccessibleContext::HasCursor()
{
    return sal_False;
}

sal_Bool SwAccessibleContext::Select( SwPaM *pPaM, SdrObject *pObj,
                                      sal_Bool bAdd )
{
    SwCrsrShell* pCrsrShell = GetCrsrShell();
    if( !pCrsrShell )
        return sal_False;

    SwFEShell* pFEShell = pCrsrShell->ISA( SwFEShell )
                                ? static_cast<SwFEShell*>( pCrsrShell )
                                : 0;
    // Get rid of activated OLE object
    if( pFEShell )
        pFEShell->FinishOLEObj();

    SwWrtShell* pWrtShell = pCrsrShell->ISA( SwWrtShell )
                                ? static_cast<SwWrtShell*>( pCrsrShell )
                                : 0;

    sal_Bool bRet = sal_False;
    if( pObj )
    {
        if( pFEShell )
        {
            Point aDummy;
            sal_uInt8 nFlags = bAdd ? SW_ADD_SELECT : 0;
            pFEShell->SelectObj( aDummy, nFlags, pObj );
            bRet = sal_True;
        }
    }
    else if( pPaM )
    {
        // Get rid of frame selection. If there is one, make text cursor
        // visible again.
        sal_Bool bCallShowCrsr = sal_False;
        if( pFEShell && (pFEShell->IsFrmSelected() ||
                         pFEShell->IsObjSelected()) )
        {
            Point aPt( LONG_MIN, LONG_MIN );
            pFEShell->SelectObj( aPt, 0 );
            bCallShowCrsr = sal_True;
        }
        pCrsrShell->KillPams();
        if( pWrtShell && pPaM->HasMark() )
            // We have to do this or SwWrtShell can't figure out that it needs
            // to kill the selection later, when the user moves the cursor.
            pWrtShell->SttSelect();
        pCrsrShell->SetSelection( *pPaM );
        if( pPaM->HasMark() && *pPaM->GetPoint() == *pPaM->GetMark())
            // Setting a "Selection" that starts and ends at the same spot
            // should remove the selection rather than create an empty one, so
            // that we get defined behavior if accessibility sets the cursor
            // later.
            pCrsrShell->ClearMark();
        if( bCallShowCrsr )
            pCrsrShell->ShowCrsr();
        bRet = sal_True;
    }

    return bRet;
}

OUString SwAccessibleContext::GetResource( sal_uInt16 nResId,
                                           const OUString *pArg1,
                                           const OUString *pArg2 )
{
    String sStr;
    {
        SolarMutexGuard aGuard;

        sStr = SW_RESSTR( nResId );
    }

    if( pArg1 )
    {
        sStr.SearchAndReplace( OUString("$(ARG1)"),
                               String( *pArg1 ) );
    }
    if( pArg2 )
    {
        sStr.SearchAndReplace( OUString("$(ARG2)"),
                               String( *pArg2 ) );
    }

    return OUString( sStr );
}

void SwAccessibleContext::RemoveFrmFromAccessibleMap()
{
    if( bRegisteredAtAccessibleMap && GetFrm() && GetMap() )
        GetMap()->RemoveContext( GetFrm() );
}

bool SwAccessibleContext::HasAdditionalAccessibleChildren()
{
    bool bRet( false );

    if ( GetFrm()->IsTxtFrm() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            bRet = pPostItMgr->HasFrmConnectedSidebarWins( *(GetFrm()) );
        }
    }

    return bRet;
}

/** #i88070# - get additional accessible child by index */
Window* SwAccessibleContext::GetAdditionalAccessibleChild( const sal_Int32 nIndex )
{
    Window* pAdditionalAccessibleChild( 0 );

    if ( GetFrm()->IsTxtFrm() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            pAdditionalAccessibleChild =
                    pPostItMgr->GetSidebarWinForFrmByIndex( *(GetFrm()), nIndex );
        }
    }

    return pAdditionalAccessibleChild;
}

/** #i88070# - get all additional accessible children */
void SwAccessibleContext::GetAdditionalAccessibleChildren( std::vector< Window* >* pChildren )
{
    if ( GetFrm()->IsTxtFrm() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            pPostItMgr->GetAllSidebarWinForFrm( *(GetFrm()), pChildren );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
