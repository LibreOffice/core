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
#include <vcl/settings.hxx>
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
#include <cppuhelper/supportsservice.hxx>
#include "accpara.hxx"
#include <PostItMgr.hxx>

using namespace sw::access;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

void SwAccessibleContext::InitStates()
{
    m_isShowingState = GetMap() && IsShowing( *(GetMap()) );

    SwViewShell *pVSh = GetMap()->GetShell();
    m_isEditableState = pVSh && IsEditable( pVSh );
    m_isOpaqueState = pVSh && IsOpaque( pVSh );
    m_isDefuncState = false;
}

void SwAccessibleContext::SetParent( SwAccessibleContext *pParent )
{
    osl::MutexGuard aGuard( m_Mutex );

    uno::Reference < XAccessible > xParent( pParent );
    m_xWeakParent = xParent;
}

uno::Reference< XAccessible > SwAccessibleContext::GetWeakParent() const
{
    osl::MutexGuard aGuard( m_Mutex );

    uno::Reference< XAccessible > xParent( m_xWeakParent );
    return xParent;
}

vcl::Window *SwAccessibleContext::GetWindow()
{
    vcl::Window *pWin = nullptr;

    if( GetMap() )
    {
        const SwViewShell *pVSh = GetMap()->GetShell();
        OSL_ENSURE( pVSh, "no view shell" );
        if( pVSh )
            pWin = pVSh->GetWin();

        OSL_ENSURE( pWin, "no window" );
    }

    return pWin;
}

// get SwViewShell from accessibility map, and cast to cursor shell
SwCursorShell* SwAccessibleContext::GetCursorShell()
{
    SwCursorShell* pCursorShell;
    SwViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : nullptr;
    OSL_ENSURE( pViewShell, "no view shell" );
    if( pViewShell && dynamic_cast<const SwCursorShell*>( pViewShell) !=  nullptr )
        pCursorShell = static_cast<SwCursorShell*>( pViewShell );
    else
        pCursorShell = nullptr;

    return pCursorShell;
}

const SwCursorShell* SwAccessibleContext::GetCursorShell() const
{
    // just like non-const GetCursorShell
    const SwCursorShell* pCursorShell;
    const SwViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : nullptr;
    OSL_ENSURE( pViewShell, "no view shell" );
    if( pViewShell && dynamic_cast<const SwCursorShell*>( pViewShell) !=  nullptr )
        pCursorShell = static_cast<const SwCursorShell*>( pViewShell );
    else
        pCursorShell = nullptr;

    return pCursorShell;
}

enum class Action { NONE, SCROLLED, SCROLLED_WITHIN,
                          SCROLLED_IN, SCROLLED_OUT };

void SwAccessibleContext::ChildrenScrolled( const SwFrame *pFrame,
                                            const SwRect& rOldVisArea )
{
    const SwRect& rNewVisArea = GetVisArea();
    const bool bVisibleChildrenOnly = SwAccessibleChild( pFrame ).IsVisibleChildrenOnly();

    const SwAccessibleChildSList aList( *pFrame, *(GetMap()) );
    SwAccessibleChildSList::const_iterator aIter( aList.begin() );
    while( aIter != aList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwRect aBox( rLower.GetBox( *(GetMap()) ) );
        if( rLower.IsAccessible( GetShell()->IsPreview() ) )
        {
            Action eAction = Action::NONE;
            if( aBox.IsOver( rNewVisArea ) )
            {
                if( aBox.IsOver( rOldVisArea ) )
                {
                    eAction = Action::SCROLLED_WITHIN;
                }
                else
                {
                    if ( bVisibleChildrenOnly &&
                         !rLower.AlwaysIncludeAsChild() )
                    {
                        eAction = Action::SCROLLED_IN;
                    }
                    else
                    {
                        eAction = Action::SCROLLED;
                    }
                }
            }
            else if( aBox.IsOver( rOldVisArea ) )
            {
                if ( bVisibleChildrenOnly &&
                     !rLower.AlwaysIncludeAsChild() )
                {
                    eAction = Action::SCROLLED_OUT;
                }
                else
                {
                    eAction = Action::SCROLLED;
                }
            }
            else if( !bVisibleChildrenOnly ||
                     rLower.AlwaysIncludeAsChild() )
            {
                // This wouldn't be required if the SwAccessibleFrame,
                // wouldn't know about the visible area.
                eAction = Action::SCROLLED;
            }
            if( Action::NONE != eAction )
            {
                if ( rLower.GetSwFrame() )
                {
                    OSL_ENSURE( !rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - always included child not considered!" );
                    const SwFrame* pLower( rLower.GetSwFrame() );
                    ::rtl::Reference< SwAccessibleContext > xAccImpl =
                        GetMap()->GetContextImpl( pLower );
                    if( xAccImpl.is() )
                    {
                        switch( eAction )
                        {
                        case Action::SCROLLED:
                            xAccImpl->Scrolled( rOldVisArea );
                            break;
                        case Action::SCROLLED_WITHIN:
                            xAccImpl->ScrolledWithin( rOldVisArea );
                            break;
                        case Action::SCROLLED_IN:
                            xAccImpl->ScrolledIn();
                            break;
                        case Action::SCROLLED_OUT:
                            xAccImpl->ScrolledOut( rOldVisArea );
                            break;
                        case Action::NONE:
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
                                                  this );
                    if( xAccImpl.is() )
                    {
                        switch( eAction )
                        {
                        case Action::SCROLLED:
                        case Action::SCROLLED_WITHIN:
                            xAccImpl->ViewForwarderChanged();
                            break;
                        case Action::SCROLLED_IN:
                            ScrolledInShape( rLower.GetDrawObject(),
                                             xAccImpl.get() );
                            break;
                        case Action::SCROLLED_OUT:
                            {
                                xAccImpl->ViewForwarderChanged();
                                // this DisposeShape call was removed by
                                // IAccessibility2 implementation
                                // without giving any reason why
                                DisposeShape( rLower.GetDrawObject(),
                                          xAccImpl.get() );
                            }
                            break;
                        // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                        case Action::NONE:
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
        else if ( rLower.GetSwFrame() &&
                  ( !bVisibleChildrenOnly ||
                    aBox.IsOver( rOldVisArea ) ||
                    aBox.IsOver( rNewVisArea ) ) )
        {
            // There are no unaccessible SdrObjects that need to be notified
            ChildrenScrolled( rLower.GetSwFrame(), rOldVisArea );
        }
        ++aIter;
    }
}

void SwAccessibleContext::Scrolled( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    ChildrenScrolled( GetFrame(), rOldVisArea );

    bool bIsOldShowingState;
    bool bIsNewShowingState = IsShowing( *(GetMap()) );
    {
        osl::MutexGuard aGuard( m_Mutex );
        bIsOldShowingState = m_isShowingState;
        m_isShowingState = bIsNewShowingState;
    }

    if( bIsOldShowingState != bIsNewShowingState )
        FireStateChangedEvent( AccessibleStateType::SHOWING,
                               bIsNewShowingState  );
}

void SwAccessibleContext::ScrolledWithin( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    ChildrenScrolled( GetFrame(), rOldVisArea );

    FireVisibleDataEvent();
}

void SwAccessibleContext::ScrolledIn()
{
    // This accessible should be freshly created, because it
    // was not visible before. Therefore, its vis area must already
    // reflect the scrolling.
    OSL_ENSURE( GetVisArea() == GetMap()->GetVisArea(),
            "Vis area of child is wrong. Did it exist already?" );

    // Send child event at parent. That's all we have to do here.
    const SwFrame* pParent = GetParent();
    ::rtl::Reference< SwAccessibleContext > xParentImpl(
         GetMap()->GetContextImpl( pParent, false ) );
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
            vcl::Window *pWin = GetWindow();
            if( pWin && pWin->HasFocus() )
            {
                FireStateChangedEvent( AccessibleStateType::FOCUSED, true );
            }
        }

    }
}

void SwAccessibleContext::ScrolledOut( const SwRect& rOldVisArea )
{
    SetVisArea( GetMap()->GetVisArea() );

    // First of all, update the children. That's required to dispose
    // all children that are existing only if they are visible. They
    // are not disposed by the recursive Dispose call that follows later on,
    // because this call will only dispose children that are in the
    // new vis area. The children we want to dispose however are in the
    // old vis area all.
    ChildrenScrolled( GetFrame(), rOldVisArea );

    // Broadcast a state changed event for the showing state.
    // It might be that the child is freshly created just to send
    // the child event. In this case no listener will exist.
    FireStateChangedEvent( AccessibleStateType::SHOWING, false );

    // this Dispose call was removed by IAccessibility2 implementation
    // without giving any reason why - without it we get stale
    // entries in SwAccessibleMap::mpFrameMap.
    Dispose(true);
}

// #i27301# - use new type definition for <_nStates>
void SwAccessibleContext::InvalidateChildrenStates( const SwFrame* _pFrame,
                                                    AccessibleStates _nStates )
{
    const SwAccessibleChildSList aVisList( GetVisArea(), *_pFrame, *(GetMap()) );

    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrame* pLower = rLower.GetSwFrame();
        if( pLower )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreview() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, false );
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

void SwAccessibleContext::DisposeChildren(const SwFrame *pFrame,
                                          bool bRecursive,
                                          bool bCanSkipInvisible)
{
    const SwAccessibleChildSList aVisList( GetVisArea(), *pFrame, *(GetMap()) );
    SwAccessibleChildSList::const_iterator aIter( aVisList.begin() );
    while( aIter != aVisList.end() )
    {
        const SwAccessibleChild& rLower = *aIter;
        const SwFrame* pLower = rLower.GetSwFrame();
        if( pLower )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreview() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, false );
            if( xAccImpl.is() )
                xAccImpl->Dispose( bRecursive );
            else if( bRecursive )
                DisposeChildren(pLower, bRecursive, bCanSkipInvisible);
        }
        else if ( rLower.GetDrawObject() )
        {
            ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl(
                    GetMap()->GetContextImpl( rLower.GetDrawObject(),
                                          this, false )  );
            if( xAccImpl.is() )
                DisposeShape( rLower.GetDrawObject(), xAccImpl.get() );
        }
        else if ( rLower.GetWindow() )
        {
            DisposeChild(rLower, false, bCanSkipInvisible);
        }
        ++aIter;
    }
}

void SwAccessibleContext::InvalidateContent_( bool )
{
}

void SwAccessibleContext::InvalidateCursorPos_()
{
}

void SwAccessibleContext::InvalidateFocus_()
{
}

void SwAccessibleContext::FireAccessibleEvent( AccessibleEventObject& rEvent )
{
    OSL_ENSURE( GetFrame(), "fire event for disposed frame?" );
    if( !GetFrame() )
        return;

    if( !rEvent.Source.is() )
    {
        uno::Reference < XAccessibleContext > xThis( this );
        rEvent.Source = xThis;
    }

    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent( m_nClientId, rEvent );
}

void SwAccessibleContext::FireVisibleDataEvent()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;

    FireAccessibleEvent( aEvent );
}

void SwAccessibleContext::FireStateChangedEvent( sal_Int16 nState,
                                                 bool bNewState )
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
    if (m_isShowingState)
        rStateSet.AddState( AccessibleStateType::SHOWING );

    // EDITABLE
    if (m_isEditableState)
    //Set editable state to graphic and other object when the document is editable
    {
        rStateSet.AddState( AccessibleStateType::EDITABLE );
        rStateSet.AddState( AccessibleStateType::RESIZABLE );
        rStateSet.AddState( AccessibleStateType::MOVEABLE );
    }
    // ENABLED
    rStateSet.AddState( AccessibleStateType::ENABLED );

    // OPAQUE
    if (m_isOpaqueState)
        rStateSet.AddState( AccessibleStateType::OPAQUE );

    // VISIBLE
    rStateSet.AddState( AccessibleStateType::VISIBLE );

    if (m_isDefuncState)
        rStateSet.AddState( AccessibleStateType::DEFUNC );
}

bool SwAccessibleContext::IsEditableState()
{
    bool bRet;
    {
        osl::MutexGuard aGuard( m_Mutex );
        bRet = m_isEditableState;
    }

    return bRet;
}

SwAccessibleContext::SwAccessibleContext( SwAccessibleMap *const pMap,
                                          sal_Int16 const nRole,
                                          const SwFrame *pF )
    : SwAccessibleFrame( pMap->GetVisArea().SVRect(), pF,
                         pMap->GetShell()->IsPreview() )
    , m_pMap( pMap )
    , m_nClientId(0)
    , m_nRole(nRole)
    , m_isDisposing( false )
    , m_isRegisteredAtAccessibleMap( true )
    , m_isSelectedInDoc(false)
{
    InitStates();
}

SwAccessibleContext::~SwAccessibleContext()
{
    SolarMutexGuard aGuard;
    RemoveFrameFromAccessibleMap();
}

uno::Reference< XAccessibleContext > SAL_CALL
    SwAccessibleContext::getAccessibleContext()
{
    uno::Reference < XAccessibleContext > xRet( this );
    return xRet;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )
    //Notify the frame is a document
    if (m_nRole == AccessibleRole::DOCUMENT_TEXT)
        m_bIsAccDocUse = true;

    return m_isDisposing ? 0 : GetChildCount( *(GetMap()) );
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    //Notify the frame is a document
    if (m_nRole == AccessibleRole::DOCUMENT_TEXT)
        m_bIsAccDocUse = true;

    const SwAccessibleChild aChild( GetChild( *(GetMap()), nIndex ) );
    if( !aChild.IsValid() )
    {
        uno::Reference < XAccessibleContext > xThis( this );
        lang::IndexOutOfBoundsException aExcept(
                "index out of bounds",
                xThis );
        throw aExcept;
    }

    uno::Reference< XAccessible > xChild;
    if( aChild.GetSwFrame() )
    {
        ::rtl::Reference < SwAccessibleContext > xChildImpl(
                GetMap()->GetContextImpl( aChild.GetSwFrame(), !m_isDisposing )  );
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
                                          this, !m_isDisposing) );
        if( xChildImpl.is() )
            xChild = xChildImpl.get();
    }
    else if ( aChild.GetWindow() )
    {
        xChild = aChild.GetWindow()->GetAccessible();
    }

    return xChild;
}

uno::Reference< XAccessible> SAL_CALL SwAccessibleContext::getAccessibleParent()
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrame *pUpper = GetParent();
    OSL_ENSURE( pUpper != nullptr || m_isDisposing, "no upper found" );

    uno::Reference< XAccessible > xAcc;
    if( pUpper )
        xAcc = GetMap()->GetContext( pUpper, !m_isDisposing );

    OSL_ENSURE( xAcc.is() || m_isDisposing, "no parent found" );

    // Remember the parent as weak ref.
    {
        osl::MutexGuard aWeakParentGuard( m_Mutex );
        m_xWeakParent = xAcc;
    }

    return xAcc;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleIndexInParent()
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrame *pUpper = GetParent();
    OSL_ENSURE( pUpper != nullptr || m_isDisposing, "no upper found" );

    sal_Int32 nIndex = -1;
    if( pUpper )
    {
        ::rtl::Reference < SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl(pUpper, !m_isDisposing) );
        OSL_ENSURE( xAccImpl.is() || m_isDisposing, "no parent found" );
        if( xAccImpl.is() )
            nIndex = xAccImpl->GetChildIndex( *(GetMap()), SwAccessibleChild(GetFrame()) );
    }

    return nIndex;
}

sal_Int16 SAL_CALL SwAccessibleContext::getAccessibleRole()
{
    return m_nRole;
}

OUString SAL_CALL SwAccessibleContext::getAccessibleDescription()
{
    OSL_ENSURE(false, "description needs to be overriden");
    THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (method must be overridden)" );
}

OUString SAL_CALL SwAccessibleContext::getAccessibleName()
{
    return m_sName;
}

uno::Reference< XAccessibleRelationSet> SAL_CALL
    SwAccessibleContext::getAccessibleRelationSet()
{
    // by default there are no relations
    uno::Reference< XAccessibleRelationSet> xRet( new utl::AccessibleRelationSetHelper() );
    return xRet;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    SwAccessibleContext::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    ::utl::AccessibleStateSetHelper *pStateSet =
        new ::utl::AccessibleStateSetHelper;

    if (m_isSelectedInDoc)
        pStateSet->AddState( AccessibleStateType::SELECTED );

    uno::Reference<XAccessibleStateSet> xStateSet( pStateSet );
    GetStates( *pStateSet );

    return xStateSet;
}

lang::Locale SAL_CALL SwAccessibleContext::getLocale()
{
    SolarMutexGuard aGuard;

    lang::Locale aLoc( Application::GetSettings().GetLanguageTag().getLocale() );
    return aLoc;
}

void SAL_CALL SwAccessibleContext::addAccessibleEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!m_nClientId)
            m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
    }
}

void SAL_CALL SwAccessibleContext::removeAccessibleEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
{
    if (xListener.is() && m_nClientId)
    {
        SolarMutexGuard aGuard;
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( m_nClientId );
            m_nClientId = 0;
        }
    }
}

static bool lcl_PointInRectangle(const awt::Point & aPoint,
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
{
    awt::Rectangle aPixBounds = getBoundsImpl(true);
    aPixBounds.X = 0;
    aPixBounds.Y = 0;

    return lcl_PointInRectangle(aPoint, aPixBounds);
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleContext::getAccessibleAtPoint(
                const awt::Point& aPoint )
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    uno::Reference< XAccessible > xAcc;

    vcl::Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to parent
    if( !GetFrame()->IsRootFrame() )
    {
        SwRect aLogBounds( GetBounds( *(GetMap()), GetFrame() ) ); // twip rel to doc root
        Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
        aPixPoint.setX(aPixPoint.getX() + aPixPos.getX());
        aPixPoint.setY(aPixPoint.getY() + aPixPos.getY());
    }

    const SwAccessibleChild aChild( GetChildAtPixel( aPixPoint, *(GetMap()) ) );
    if( aChild.GetSwFrame() )
    {
        xAcc = GetMap()->GetContext( aChild.GetSwFrame() );
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
awt::Rectangle SAL_CALL SwAccessibleContext::getBoundsImpl(bool bRelative)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrame *pParent = GetParent();
    OSL_ENSURE( pParent, "no Parent found" );
    vcl::Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pParent )

    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrame() ) ); // twip rel to doc root
    Rectangle aPixBounds( 0, 0, 0, 0 );
    if( GetFrame()->IsPageFrame() &&
        static_cast < const SwPageFrame * >( GetFrame() )->IsEmptyPage() )
    {
        OSL_ENSURE( GetShell()->IsPreview(), "empty page accessible?" );
        if( GetShell()->IsPreview() )
        {
            // adjust method call <GetMap()->GetPreviewPageSize()>
            sal_uInt16 nPageNum =
                static_cast < const SwPageFrame * >( GetFrame() )->GetPhyPageNum();
            aLogBounds.SSize( GetMap()->GetPreviewPageSize( nPageNum ) );
        }
    }
    if( !aLogBounds.IsEmpty() )
    {
        aPixBounds = GetMap()->CoreToPixel( aLogBounds.SVRect() );
        if( !pParent->IsRootFrame() && bRelative)
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
{
    return getBoundsImpl(true);
}

awt::Point SAL_CALL SwAccessibleContext::getLocation()
{
    awt::Rectangle aRect = getBoundsImpl(true);
    awt::Point aPoint(aRect.X, aRect.Y);

    return aPoint;
}

awt::Point SAL_CALL SwAccessibleContext::getLocationOnScreen()
{
    awt::Rectangle aRect = getBoundsImpl(false);

    Point aPixPos(aRect.X, aRect.Y);

    vcl::Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    aPixPos = pWin->OutputToAbsoluteScreenPixel(aPixPos);
    awt::Point aPoint(aPixPos.getX(), aPixPos.getY());

    return aPoint;
}

awt::Size SAL_CALL SwAccessibleContext::getSize()
{
    awt::Rectangle aRect = getBoundsImpl(false);
    awt::Size aSize( aRect.Width, aRect.Height );

    return aSize;
}

void SAL_CALL SwAccessibleContext::grabFocus()
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext );

    if( GetFrame()->IsFlyFrame() )
    {
        const SdrObject *pObj =
            static_cast < const SwFlyFrame * >( GetFrame() )->GetVirtDrawObj();
        if( pObj )
            Select( const_cast < SdrObject * >( pObj ), false );
    }
    else
    {
        const SwContentFrame *pCFrame = nullptr;
        if( GetFrame()->IsContentFrame() )
            pCFrame = static_cast< const SwContentFrame * >( GetFrame() );
        else if( GetFrame()->IsLayoutFrame() )
            pCFrame = static_cast< const SwLayoutFrame * >( GetFrame() )->ContainsContent();

        if( pCFrame && pCFrame->IsTextFrame() )
        {
            const SwTextFrame *pTextFrame = static_cast< const SwTextFrame * >( pCFrame );
            const SwTextNode *pTextNd = pTextFrame->GetTextNode();
            if( pTextNd )
            {
                // create pam for selection
                SwIndex aIndex( const_cast< SwTextNode * >( pTextNd ),
                                pTextFrame->GetOfst() );
                SwPosition aStartPos( *pTextNd, aIndex );
                SwPaM aPaM( aStartPos );

                // set PaM at cursor shell
                Select( aPaM );
            }
        }
    }
}

sal_Int32 SAL_CALL SwAccessibleContext::getForeground()
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL SwAccessibleContext::getBackground()
{
    return COL_WHITE;
}

OUString SAL_CALL SwAccessibleContext::getImplementationName()
{
    OSL_ENSURE( false, "implementation name needs to be overridden" );

    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "implementation name needs to be overridden" )
}

sal_Bool SAL_CALL SwAccessibleContext::supportsService (const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SwAccessibleContext::getSupportedServiceNames()
{
    OSL_ENSURE( false, "supported services names needs to be overridden" );
    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "supported services needs to be overridden" )
}

void SwAccessibleContext::DisposeShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl )
{
    ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl( pAccImpl );
    if( !xAccImpl.is() )
        xAccImpl = GetMap()->GetContextImpl( pObj, this );

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
    if(nullptr == pAccImpl)
    {
        return ;
    }
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::CHILD;
    uno::Reference< XAccessible > xAcc( pAccImpl );
    aEvent.NewValue <<= xAcc;
    FireAccessibleEvent( aEvent );

    if( pAccImpl->GetState( AccessibleStateType::FOCUSED ) )
    {
        vcl::Window *pWin = GetWindow();
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

void SwAccessibleContext::Dispose(bool bRecursive, bool bCanSkipInvisible)
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( GetFrame() && GetMap(), "already disposed" );
    OSL_ENSURE( GetMap()->GetVisArea() == GetVisArea(),
                "invalid vis area for dispose" );

    m_isDisposing = true;

    // dispose children
    if( bRecursive )
        DisposeChildren(GetFrame(), bRecursive, bCanSkipInvisible);

    // get parent
    uno::Reference< XAccessible > xParent( GetWeakParent() );
    uno::Reference < XAccessibleContext > xThis( this );

    // send child event at parent
    if( xParent.is() )
    {
        SwAccessibleContext *pAcc = static_cast<SwAccessibleContext *>(xParent.get());

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= xThis;
        pAcc->FireAccessibleEvent( aEvent );
    }

    // set defunc state (it's not required to broadcast a state changed
    // event if the object is disposed afterwards)
    {
        osl::MutexGuard aDefuncStateGuard( m_Mutex );
        m_isDefuncState = true;
    }

    // broadcast dispose event
    if (m_nClientId)
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( m_nClientId, *this );
        m_nClientId =  0;
    }

    RemoveFrameFromAccessibleMap();
    ClearFrame();
    m_pMap = nullptr;

    m_isDisposing = false;
}

void SwAccessibleContext::DisposeChild( const SwAccessibleChild& rChildFrameOrObj,
                                        bool bRecursive, bool bCanSkipInvisible )
{
    SolarMutexGuard aGuard;

    if ( !bCanSkipInvisible ||
         rChildFrameOrObj.AlwaysIncludeAsChild() ||
         IsShowing( *(GetMap()), rChildFrameOrObj ) ||
         !SwAccessibleChild( GetFrame() ).IsVisibleChildrenOnly() )
    {
        // If the object could have existed before, than there is nothing to do,
        // because no wrapper exists now and therefore no one is interested to
        // get notified of the movement.
        if( rChildFrameOrObj.GetSwFrame() )
        {
            ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrameOrObj.GetSwFrame() );
            xAccImpl->Dispose( bRecursive );
        }
        else if ( rChildFrameOrObj.GetDrawObject() )
        {
            ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrameOrObj.GetDrawObject(),
                                              this );
            DisposeShape( rChildFrameOrObj.GetDrawObject(),
                          xAccImpl.get() );
        }
        else if ( rChildFrameOrObj.GetWindow() )
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            uno::Reference< XAccessible > xAcc =
                                    rChildFrameOrObj.GetWindow()->GetAccessible();
            aEvent.OldValue <<= xAcc;
            FireAccessibleEvent( aEvent );
        }
    }
    else if( bRecursive && rChildFrameOrObj.GetSwFrame() )
        DisposeChildren(rChildFrameOrObj.GetSwFrame(), bRecursive, bCanSkipInvisible);
}

void SwAccessibleContext::InvalidatePosOrSize( const SwRect& )
{
    SolarMutexGuard aGuard;

    OSL_ENSURE( GetFrame() && !GetFrame()->Frame().IsEmpty(), "context should have a size" );

    bool bIsOldShowingState;
    bool bIsNewShowingState = IsShowing( *(GetMap()) );
    {
        osl::MutexGuard aShowingStateGuard( m_Mutex );
        bIsOldShowingState = m_isShowingState;
        m_isShowingState = bIsNewShowingState;
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

    // note: InvalidatePosOrSize must call InvalidateContent_ so that
    // SwAccessibleParagraph updates its portions, or dispose it
    // (see accmap.cxx: INVALID_CONTENT is contained in POS_CHANGED)
    if( !bIsNewShowingState &&
        SwAccessibleChild( GetParent() ).IsVisibleChildrenOnly() )
    {
        // this Dispose call was removed by IAccessibility2 implementation
        // without giving any reason why - without it we get stale
        // entries in SwAccessibleMap::mpFrameMap.
        Dispose(true);
    }
    else
    {
        InvalidateContent_( true );
    }
}

void SwAccessibleContext::InvalidateChildPosOrSize(
                    const SwAccessibleChild& rChildFrameOrObj,
                    const SwRect& rOldFrame )
{
    SolarMutexGuard aGuard;

    // this happens during layout, e.g. when a page is deleted and next page's
    // header/footer moves backward such an event is generated
    SAL_INFO_IF(rChildFrameOrObj.GetSwFrame() &&
            rChildFrameOrObj.GetSwFrame()->Frame().IsEmpty(),
            "sw.a11y", "child context should have a size");

    if ( rChildFrameOrObj.AlwaysIncludeAsChild() )
    {
        // nothing to do;
        return;
    }

    const bool bVisibleChildrenOnly = SwAccessibleChild( GetFrame() ).IsVisibleChildrenOnly();
    const bool bNew = rOldFrame.IsEmpty() ||
                     ( rOldFrame.Left() == 0 && rOldFrame.Top() == 0 );
    if( IsShowing( *(GetMap()), rChildFrameOrObj ) )
    {
        // If the object could have existed before, than there is nothing to do,
        // because no wrapper exists now and therefore no one is interested to
        // get notified of the movement.
        if( bNew || (bVisibleChildrenOnly && !IsShowing( rOldFrame )) )
        {
            if( rChildFrameOrObj.GetSwFrame() )
            {
                // The frame becomes visible. A child event must be send.
                ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrameOrObj.GetSwFrame() );
                xAccImpl->ScrolledIn();
            }
            else if ( rChildFrameOrObj.GetDrawObject() )
            {
                ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrameOrObj.GetDrawObject(),
                                                  this );
                // #i37790#
                if ( xAccImpl.is() )
                {
                    ScrolledInShape( rChildFrameOrObj.GetDrawObject(),
                                     xAccImpl.get() );
                }
                else
                {
                    OSL_FAIL( "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - no accessible shape found." );
                }
            }
            else if ( rChildFrameOrObj.GetWindow() )
            {
                AccessibleEventObject aEvent;
                aEvent.EventId = AccessibleEventId::CHILD;
                aEvent.NewValue <<= (rChildFrameOrObj.GetWindow()->GetAccessible());
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
            !bNew && IsShowing( rOldFrame ) )
        {
            if( rChildFrameOrObj.GetSwFrame() )
            {
                ::rtl::Reference< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrameOrObj.GetSwFrame() );
                xAccImpl->SetParent( this );
                xAccImpl->Dispose( true );
            }
            else if ( rChildFrameOrObj.GetDrawObject() )
            {
                ::rtl::Reference< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrameOrObj.GetDrawObject(),
                                                  this );
                DisposeShape( rChildFrameOrObj.GetDrawObject(),
                          xAccImpl.get() );
            }
            else if ( rChildFrameOrObj.GetWindow() )
            {
                OSL_FAIL( "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - not expected to handle dispose of child of type <vcl::Window>." );
            }
        }
    }
}

void SwAccessibleContext::InvalidateContent()
{
    SolarMutexGuard aGuard;

    InvalidateContent_( false );
}

void SwAccessibleContext::InvalidateCursorPos()
{
    SolarMutexGuard aGuard;

    InvalidateCursorPos_();
}

void SwAccessibleContext::InvalidateFocus()
{
    SolarMutexGuard aGuard;

    InvalidateFocus_();
}

// #i27301# - use new type definition for <_nStates>
void SwAccessibleContext::InvalidateStates( AccessibleStates _nStates )
{
    if( GetMap() )
    {
        SwViewShell *pVSh = GetMap()->GetShell();
        if( pVSh )
        {
            if( _nStates & AccessibleStates::EDITABLE )
            {
                bool bIsOldEditableState;
                bool bIsNewEditableState = IsEditable( pVSh );
                {
                    osl::MutexGuard aGuard( m_Mutex );
                    bIsOldEditableState = m_isEditableState;
                    m_isEditableState = bIsNewEditableState;
                }

                if( bIsOldEditableState != bIsNewEditableState )
                    FireStateChangedEvent( AccessibleStateType::EDITABLE,
                                           bIsNewEditableState  );
            }
            if( _nStates & AccessibleStates::OPAQUE )
            {
                bool bIsOldOpaqueState;
                bool bIsNewOpaqueState = IsOpaque( pVSh );
                {
                    osl::MutexGuard aGuard( m_Mutex );
                    bIsOldOpaqueState = m_isOpaqueState;
                    m_isOpaqueState = bIsNewOpaqueState;
                }

                if( bIsOldOpaqueState != bIsNewOpaqueState )
                    FireStateChangedEvent( AccessibleStateType::OPAQUE,
                                           bIsNewOpaqueState  );
            }
        }

        InvalidateChildrenStates( GetFrame(), _nStates );
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

bool SwAccessibleContext::HasCursor()
{
    return false;
}

bool SwAccessibleContext::Select( SwPaM *pPaM, SdrObject *pObj,
                                  bool bAdd )
{
    SwCursorShell* pCursorShell = GetCursorShell();
    if( !pCursorShell )
        return false;

    SwFEShell* pFEShell = dynamic_cast<const SwFEShell*>( pCursorShell) !=  nullptr
                                ? static_cast<SwFEShell*>( pCursorShell )
                                : nullptr;
    // Get rid of activated OLE object
    if( pFEShell )
        pFEShell->FinishOLEObj();

    SwWrtShell* pWrtShell = dynamic_cast<const SwWrtShell*>( pCursorShell) !=  nullptr
                                ? static_cast<SwWrtShell*>( pCursorShell )
                                : nullptr;

    bool bRet = false;
    if( pObj )
    {
        if( pFEShell )
        {
            Point aDummy;
            sal_uInt8 nFlags = bAdd ? SW_ADD_SELECT : 0;
            pFEShell->SelectObj( aDummy, nFlags, pObj );
            bRet = true;
        }
    }
    else if( pPaM )
    {
        // Get rid of frame selection. If there is one, make text cursor
        // visible again.
        bool bCallShowCursor = false;
        if( pFEShell && (pFEShell->IsFrameSelected() ||
                         pFEShell->IsObjSelected()) )
        {
            Point aPt( LONG_MIN, LONG_MIN );
            pFEShell->SelectObj( aPt );
            bCallShowCursor = true;
        }
        pCursorShell->KillPams();
        if( pWrtShell && pPaM->HasMark() )
            // We have to do this or SwWrtShell can't figure out that it needs
            // to kill the selection later, when the user moves the cursor.
            pWrtShell->SttSelect();
        pCursorShell->SetSelection( *pPaM );
        if( pPaM->HasMark() && *pPaM->GetPoint() == *pPaM->GetMark())
            // Setting a "Selection" that starts and ends at the same spot
            // should remove the selection rather than create an empty one, so
            // that we get defined behavior if accessibility sets the cursor
            // later.
            pCursorShell->ClearMark();
        if( bCallShowCursor )
            pCursorShell->ShowCursor();
        bRet = true;
    }

    return bRet;
}

OUString SwAccessibleContext::GetResource( sal_uInt16 nResId,
                                           const OUString *pArg1,
                                           const OUString *pArg2 )
{
    OUString sStr;
    {
        SolarMutexGuard aGuard;

        sStr = SW_RESSTR( nResId );
    }

    if( pArg1 )
    {
        sStr = sStr.replaceFirst( "$(ARG1)", *pArg1 );
    }
    if( pArg2 )
    {
        sStr = sStr.replaceFirst( "$(ARG2)", *pArg2 );
    }

    return sStr;
}

void SwAccessibleContext::RemoveFrameFromAccessibleMap()
{
    if (m_isRegisteredAtAccessibleMap && GetFrame() && GetMap())
        GetMap()->RemoveContext( GetFrame() );
}

bool SwAccessibleContext::HasAdditionalAccessibleChildren()
{
    bool bRet( false );

    if ( GetFrame()->IsTextFrame() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            bRet = pPostItMgr->HasFrameConnectedSidebarWins( *(GetFrame()) );
        }
    }

    return bRet;
}

/** #i88070# - get additional accessible child by index */
vcl::Window* SwAccessibleContext::GetAdditionalAccessibleChild( const sal_Int32 nIndex )
{
    vcl::Window* pAdditionalAccessibleChild( nullptr );

    if ( GetFrame()->IsTextFrame() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            pAdditionalAccessibleChild =
                    pPostItMgr->GetSidebarWinForFrameByIndex( *(GetFrame()), nIndex );
        }
    }

    return pAdditionalAccessibleChild;
}

/** #i88070# - get all additional accessible children */
void SwAccessibleContext::GetAdditionalAccessibleChildren( std::vector< vcl::Window* >* pChildren )
{
    if ( GetFrame()->IsTextFrame() )
    {
        SwPostItMgr* pPostItMgr = GetMap()->GetShell()->GetPostItMgr();
        if ( pPostItMgr && pPostItMgr->HasNotes() && pPostItMgr->ShowNotes() )
        {
            pPostItMgr->GetAllSidebarWinForFrame( *(GetFrame()), pChildren );
        }
    }
}

bool SwAccessibleContext::SetSelectedState(bool const bSelected)
{
    if (m_isSelectedInDoc != bSelected)
    {
        m_isSelectedInDoc = bSelected;
        FireStateChangedEvent( AccessibleStateType::SELECTED, bSelected );
        return true;
    }
    return false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
