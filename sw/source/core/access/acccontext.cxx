/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    bIsShowingState = GetMap() ? IsShowing( *(GetMap()) ) : sal_False;

    SwViewShell *pVSh = GetMap()->GetShell();
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
        const SwViewShell *pVSh = GetMap()->GetShell();
        OSL_ENSURE( pVSh, "no view shell" );
        if( pVSh )
            pWin = pVSh->GetWin();

        OSL_ENSURE( pWin, "no window" );
    }

    return pWin;
}


SwCrsrShell* SwAccessibleContext::GetCrsrShell()
{
    SwCrsrShell* pCrsrShell;
    SwViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : 0;
    OSL_ENSURE( pViewShell, "no view shell" );
    if( pViewShell && pViewShell->ISA( SwCrsrShell ) )
        pCrsrShell = static_cast<SwCrsrShell*>( pViewShell );
    else
        pCrsrShell = NULL;

    return pCrsrShell;
}

const SwCrsrShell* SwAccessibleContext::GetCrsrShell() const
{
    
    const SwCrsrShell* pCrsrShell;
    const SwViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : 0;
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
        if( rLower.IsAccessible( GetShell()->IsPreview() ) )
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
                        GetMap()->GetContextImpl( pLower, true );
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
                                                  true );
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
                            }
                            break;
                        case NONE:
                            break;
                        }
                    }
                }
                else if ( rLower.GetWindow() )
                {
                    
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
    
    
    
    OSL_ENSURE( GetVisArea() == GetMap()->GetVisArea(),
            "Vis area of child is wrong. Did it exist already?" );

    
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

    
    
    
    
    
    
    ChildrenScrolled( GetFrm(), rOldVisArea );

    
    
    
    FireStateChangedEvent( AccessibleStateType::SHOWING, sal_False );
}


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
            if( rLower.IsAccessible( GetShell()->IsPreview() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, sal_False );
            if( xAccImpl.is() )
                xAccImpl->InvalidateStates( _nStates );
            else
                InvalidateChildrenStates( pLower, _nStates );
        }
        else if ( rLower.GetDrawObject() )
        {
            
        }
        else if ( rLower.GetWindow() )
        {
            
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
            if( rLower.IsAccessible( GetShell()->IsPreview() ) )
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

    
    if( bIsShowingState )
        rStateSet.AddState( AccessibleStateType::SHOWING );

    
    if( bIsEditableState )
    
    {
        rStateSet.AddState( AccessibleStateType::EDITABLE );
        rStateSet.AddState( AccessibleStateType::RESIZABLE );
        rStateSet.AddState( AccessibleStateType::MOVEABLE );
    }
    
    rStateSet.AddState( AccessibleStateType::ENABLED );

    
    if( bIsOpaqueState )
        rStateSet.AddState( AccessibleStateType::OPAQUE );

    
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
                         pM->GetShell()->IsPreview() )
    , pMap( pM )
    , nClientId(0)
    , nRole( nR )
    , bDisposing( sal_False )
    , bRegisteredAtAccessibleMap( true )
    
    , bBeginDocumentLoad( sal_True )
    , isIfAsynLoad( sal_True )
    , bIsSeletedInDoc( sal_False)
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
    
    if( nRole == AccessibleRole::DOCUMENT_TEXT )
        bIsAccDocUse = sal_True;

    return bDisposing ? 0 : GetChildCount( *(GetMap()) );
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    SolarMutexGuard aGuard;

    CHECK_FOR_DEFUNC( XAccessibleContext )

    
    if( nRole == AccessibleRole::DOCUMENT_TEXT )
        bIsAccDocUse = sal_True;

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
        
        if( bBeginDocumentLoad && nRole == AccessibleRole::DOCUMENT_TEXT )
        {

            FireStateChangedEvent( AccessibleStateType::FOCUSABLE,sal_True );
            FireStateChangedEvent( AccessibleStateType::BUSY,sal_True );
            if( !isIfAsynLoad )
            {
                FireStateChangedEvent( AccessibleStateType::FOCUSED,sal_True );
                
                
                FireStateChangedEvent( AccessibleStateType::SHOWING,sal_True );
                FireStateChangedEvent( AccessibleStateType::BUSY,sal_False );
                
                
            }
            bBeginDocumentLoad = sal_False;
        }
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

    if( bIsSeletedInDoc )
        pStateSet->AddState( AccessibleStateType::SELECTED );

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

    Point aPixPoint( aPoint.X, aPoint.Y ); 
    if( !GetFrm()->IsRootFrm() )
    {
        SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); 
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

    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); 
    Rectangle aPixBounds( 0, 0, 0, 0 );
    if( GetFrm()->IsPageFrm() &&
        static_cast < const SwPageFrm * >( GetFrm() )->IsEmptyPage() )
    {
        OSL_ENSURE( GetShell()->IsPreview(), "empty page accessible?" );
        if( GetShell()->IsPreview() )
        {
            
            sal_uInt16 nPageNum =
                static_cast < const SwPageFrm * >( GetFrm() )->GetPhyPageNum();
            aLogBounds.SSize( GetMap()->GetPreviewPageSize( nPageNum ) );
        }
    }
    if( !aLogBounds.IsEmpty() )
    {
        aPixBounds = GetMap()->CoreToPixel( aLogBounds.SVRect() );
        if( !pParent->IsRootFrm() && bRelative)
        {
            SwRect aParentLogBounds( GetBounds( *(GetMap()), pParent ) ); 
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
                
                SwIndex aIndex( const_cast< SwTxtNode * >( pTxtNd ),
                                pTxtFrm->GetOfst() );
                SwPosition aStartPos( *pTxtNd, aIndex );
                SwPaM aPaM( aStartPos );

                
                Select( aPaM );
            }
        }
    }
}

uno::Any SAL_CALL SwAccessibleContext::getAccessibleKeyBinding()
        throw (uno::RuntimeException)
{
    
    return uno::Any();
}

sal_Int32 SAL_CALL SwAccessibleContext::getForeground()
        throw (uno::RuntimeException)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL SwAccessibleContext::getBackground()
        throw (uno::RuntimeException)
{
    return COL_WHITE;
}

OUString SAL_CALL SwAccessibleContext::getImplementationName()
        throw( uno::RuntimeException )
{
    OSL_ENSURE( !this, "implementation name needs to be overloaded" );

    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "implementation name needs to be overloaded" )
}

sal_Bool SAL_CALL SwAccessibleContext::supportsService (const OUString& ServiceName)
        throw (uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
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
    if(NULL == pAccImpl)
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

    
    if( bRecursive )
        DisposeChildren( GetFrm(), bRecursive );

    
    uno::Reference< XAccessible > xParent( GetWeakParent() );
    uno::Reference < XAccessibleContext > xThis( this );

    
    if( xParent.is() )
    {
        SwAccessibleContext *pAcc = (SwAccessibleContext *)xParent.get();

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= xThis;
        pAcc->FireAccessibleEvent( aEvent );
    }

    
    
    {
        osl::MutexGuard aDefuncStateGuard( aMutex );
        bIsDefuncState = sal_True;
    }

    
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
        
        FireVisibleDataEvent();
    }

    if( !bIsNewShowingState &&
        SwAccessibleChild( GetParent() ).IsVisibleChildrenOnly() )
    {
        return;
    }

    _InvalidateContent( sal_True );
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
        
        return;
    }

    const bool bVisibleChildrenOnly = SwAccessibleChild( GetFrm() ).IsVisibleChildrenOnly();
    const bool bNew = rOldFrm.IsEmpty() ||
                     ( rOldFrm.Left() == 0 && rOldFrm.Top() == 0 );
    if( IsShowing( *(GetMap()), rChildFrmOrObj ) )
    {
        
        
        
        if( bNew || (bVisibleChildrenOnly && !IsShowing( rOldFrm )) )
        {
            if( rChildFrmOrObj.GetSwFrm() )
            {
                
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


void SwAccessibleContext::InvalidateStates( tAccessibleStates _nStates )
{
    if( GetMap() )
    {
        SwViewShell *pVSh = GetMap()->GetShell();
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
            
            
            pWrtShell->SttSelect();
        pCrsrShell->SetSelection( *pPaM );
        if( pPaM->HasMark() && *pPaM->GetPoint() == *pPaM->GetMark())
            
            
            
            
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
    OUString sStr;
    {
        SolarMutexGuard aGuard;

        sStr = SW_RESSTR( nResId );
    }

    if( pArg1 )
    {
        sStr = sStr.replaceFirst( OUString("$(ARG1)"), *pArg1 );
    }
    if( pArg2 )
    {
        sStr = sStr.replaceFirst( OUString("$(ARG2)"), *pArg2 );
    }

    return sStr;
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

sal_Bool SwAccessibleContext::SetSelectedState(sal_Bool bSeleted)
{
    if(bIsSeletedInDoc != bSeleted)
    {
        bIsSeletedInDoc = bSeleted;
        FireStateChangedEvent( AccessibleStateType::SELECTED, bSeleted );
        return sal_True;
    }
    return sal_False;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
