 /*************************************************************************
 *
 *  $RCSfile: acccontext.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: dvo $ $Date: 2002-03-21 11:07:26 $
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
#if defined DEBUG && defined TEST_MIB
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _ERRHDL_HXX
#include "errhdl.hxx"
#endif
#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

#pragma hdrstop

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATESET_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleStateSet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif

#if defined DEBUG && defined TEST_MIB
#define DBG_MSG( _msg ) \
    lcl_SwAccessibleContext_DbgMsg( this, _msg, 0, sal_False );
#define DBG_MSG_CD( _msg ) \
    lcl_SwAccessibleContext_DbgMsg( this, _msg, 0, sal_True );
#define DBG_MSG_PARAM( _msg, _param ) \
    lcl_SwAccessibleContext_DbgMsg( this, _msg, _param, sal_False );
#define DBG_MSG_THIS_PARAM( _msg, _this, _param ) \
    lcl_SwAccessibleContext_DbgMsg( _this, _msg, _param, sal_False );

void lcl_SwAccessibleContext_DbgMsg( SwAccessibleContext *pThisAcc,
                                     const char *pMsg,
                                     SwAccessibleContext *pChildAcc,
                                       sal_Bool bConstrDestr );
#else
#define DBG_MSG( _msg )
#define DBG_MSG_PARAM( _msg, _param )
#define DBG_MSG_THIS_PARAM( _msg, _this, _param )
#define DBG_MSG_CD( _msg )
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

void SwAccessibleContext::FireVisibleDataEvent()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT;

    FireAccessibleEvent( aEvent );
    DBG_MSG( "AccessibleVisibleData" )
}

void SwAccessibleContext::FireStateChangedEvent( sal_Int16 nState,
                                                 sal_Bool bNewState )
{
    AccessibleEventObject aEvent;

    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    if( bNewState )
        aEvent.NewValue <<= nState;
    else
        aEvent.OldValue <<= nState;

    FireAccessibleEvent( aEvent );
    DBG_MSG( "StateChanged" )
}

void SwAccessibleContext::SetParent( SwAccessibleContext *pParent )
{
    vos::OGuard aGuard( aMutex );

    ::com::sun::star::uno::Reference < ::drafts::com::sun::star::accessibility::XAccessible > xParent( pParent );
    xWeakParent = xParent;
}


sal_Bool SwAccessibleContext::ChildScrolledIn( const SwFrm *pFrm )
{
    WeakReference < XAccessible > xWeakChild;
    SwAccessibleContext *pChildImpl = 0;

    if( GetMap() )
    {
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm ) );
        xChildImpl->SetParent( this );

        // The accessible should be freshly created, because it
        // was not visisble before. Therfor, its vis area must already
        // the scrolling.
        ASSERT( GetVisArea() == xChildImpl->GetVisArea(),
                "Vis area of child is wrong. Did it exist already?" );

        // Send a child event
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
        Reference < XAccessible > xChild( xChildImpl.getBodyPtr() );
        aEvent.NewValue <<= xChild;

        FireAccessibleEvent( aEvent );
        DBG_MSG_PARAM( "AccessibleChild (added)", xChildImpl.getBodyPtr() )

        xWeakChild = xChild;
        pChildImpl = xChildImpl.getBodyPtr();
    }

    Reference < XAccessible > xChild( xWeakChild );
    if( xChild.is() )
    {
        // If the AT tool remembered the child, then we send
        // notifcations for the child's children, too. Otherwise
        // there is nothing to do, because children can only created
        // by a parent itself, so there could in fact be children now,
        // but these children already inherited the correct vis area.
        // This vis area in update this area is the same as the old one,
        // because
        pChildImpl->SetVisArea( GetVisArea() );
    }

    return sal_False;
}

sal_Bool SwAccessibleContext::ChildScrolledOut( const SwFrm *pFrm )
{
    sal_Bool bUpdateChildren = sal_True;
    WeakReference < XAccessible > xWeakChild;
    SwAccessibleContext *pChildImpl = 0;

    if( GetMap() )
    {
        // If the child is existing, the child's chilren have to be
        // removed, too. If not, some grandchildren might exist anyway.
        // They are removed by seekaing the SwFrm tree. This is indicated
        // by the return value.
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm, sal_False ) );

        if( xChildImpl.isValid() )
        {
            xChildImpl->SetVisArea( GetVisArea() );
            bUpdateChildren = sal_False;

            pChildImpl = xChildImpl.getBodyPtr();
            xWeakChild = Reference < XAccessible >( pChildImpl );

            // If the child is existing, we broadcast a state changed event
            // for the showing state. If it is not eixting, no event
            // is required.
            xChildImpl->FireStateChangedEvent( AccessibleStateType::SHOWING,
                                               sal_False );
        }
    }

    // When disposing the child, the child's context must exist,
    // because we have to broadcast a child event!
    Reference < XAccessible > xChild( xWeakChild );
    if( !xChild.is() && GetMap() )
    {
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm ) );
        pChildImpl = xChildImpl.getBodyPtr();
        xChild = pChildImpl;
    }

    pChildImpl->SetParent( this );
    pChildImpl->Dispose();

    return bUpdateChildren;
}

sal_Bool SwAccessibleContext::ChildScrolled( const SwFrm *pFrm )
{
    sal_Bool bUpdateChildren = sal_True;

    // get child
    if( GetMap() )
    {
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm, sal_False ) );

        if( xChildImpl.isValid() )
        {

            // Now update the frame's children. In fact, they are updated before
            // the update event for the parent has been send.
            xChildImpl->SetVisArea( GetVisArea() );

            xChildImpl->FireVisibleDataEvent();
            bUpdateChildren = sal_False;
        }
    }

    return bUpdateChildren;
}

sal_Bool SwAccessibleContext::CheckStatesChild( const SwFrm *pFrm,
                                                sal_uInt8 nStates )
{
    sal_Bool bCheckChildren = sal_True;

    if( GetMap() )
    {
        // If the child is existing, the child's chilren have to be
        // checked, too. If not, some grandchildren might exist anyway.
        // They are checked by seeking the SwFrm tree. This is indicated
        // by the return value.
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm, sal_False ) );

        if( xChildImpl.isValid() )
        {
            xChildImpl->CheckStates( nStates );
            bCheckChildren = sal_False;
        }
    }

    return bCheckChildren;
}

sal_Bool SwAccessibleContext::DisposeChild( const SwFrm *pFrm,
                                             sal_Bool bRecursive )
{
    sal_Bool bDisposeChildren = sal_True;

    if( GetMap() )
    {
        // If the child is existing, the child's chilren have to be
        // removed, too. If not, some grandchildren might exist anyway.
        // They are removed by seeking the SwFrm tree. This is indicated
        // by the return value.
        ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pFrm, sal_False ) );

        if( xChildImpl.isValid() )
        {
            xChildImpl->Dispose( bRecursive );
            bDisposeChildren = sal_False;
        }
    }

    return bDisposeChildren;
}

void SwAccessibleContext::CheckStates( sal_uInt8 nStates )
{
    if( GetMap() )
    {
        ViewShell *pVSh = GetMap()->GetShell();
        if( pVSh )
        {
            if( (nStates & ACC_STATE_EDITABLE) != 0 )
            {
                sal_Bool bIsOldEditableState;
                sal_Bool bIsNewEditableState = IsEditable( pVSh );
                {
                    vos::OGuard aGuard( aMutex );
                    bIsOldEditableState = bIsEditableState;
                    bIsEditableState = bIsNewEditableState;
                }

                if( bIsOldEditableState != bIsNewEditableState )
                    FireStateChangedEvent( AccessibleStateType::EDITABLE,
                                           bIsNewEditableState  );
            }
            if( (nStates & ACC_STATE_OPAQUE) != 0 )
            {
                sal_Bool bIsOldOpaqueState;
                sal_Bool bIsNewOpaqueState = IsOpaque( pVSh );
                {
                    vos::OGuard aGuard( aMutex );
                    bIsOldOpaqueState = bIsOpaqueState;
                    bIsOpaqueState = bIsNewOpaqueState;
                }

                if( bIsOldOpaqueState != bIsNewOpaqueState )
                    FireStateChangedEvent( AccessibleStateType::OPAQUE,
                                           bIsNewOpaqueState  );
            }
        }
    }

    CheckStatesChildren( nStates );
}

void SwAccessibleContext::Dispose( sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // dispose children
    if( bRecursive )
        DisposeChildren( bRecursive );

    // get parent
    Reference< XAccessible > xParent;
    {
        vos::OGuard aGuard( aMutex );
        xParent = xWeakParent;
    }
    Reference < XAccessibleContext > xThis( this );

    // send child event at parent
    if( xParent.is() )
    {
        SwAccessibleContext *pAcc = (SwAccessibleContext *)xParent.get();

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
        aEvent.OldValue <<= xThis;
        pAcc->FireAccessibleEvent( aEvent );
        DBG_MSG_THIS_PARAM( "AccessibleChild (removed)", pAcc, this )
    }

    // set defunc state
    {
        vos::OGuard aGuard( aMutex );
        bIsDefuncState = sal_True;
    }

    // broadcast state change
    FireStateChangedEvent( AccessibleStateType::DEFUNC, sal_True );

    // broadcast dispose event
    {
        EventObject aEvent;
        aEvent.Source = xThis;
        aAccessibleEventListeners.disposeAndClear( aEvent );
        aFocusListeners.disposeAndClear( aEvent );
        DBG_MSG_CD( "dispose" )
    }

    ASSERT( GetFrm(), "already disposed" );
    if( GetMap() && GetFrm() )
        GetMap()->RemoveContext( GetFrm() );
    ClearFrm();
    pMap = 0;
}

void SwAccessibleContext::PosChanged()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( IsShowing() )
    {
        // The frame stays visible -> broadcast event
        FireVisibleDataEvent();

        _InvalidateContent( sal_True );
    }
    else
    {
        // set object not showing
        {
            vos::OGuard aGuard( aMutex );
            bIsShowingState = sal_False;
        }

        // Fire state change event for showing state
        FireStateChangedEvent( AccessibleStateType::SHOWING, sal_False );

        // The frame is now invisible -> dispose it
        Dispose();
    }
}

void SwAccessibleContext::ChildPosChanged( const SwFrm *pFrm,
                                           const SwRect& rOldFrm )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( IsShowing( pFrm ) )
    {
        // If the object was not showing before, than there is nothing to do,
        // because no wrapper exists and therefor no one is interested to
        // get notified of the movement.
        if( rOldFrm.IsEmpty() || !IsShowing( rOldFrm.SVRect() ) )
        {
            // The frame becomes visible. A child event for the parent
            // must be send.
            ChildScrolledIn( pFrm );
        }
    }
    else
    {
        // If the frame was visible before, than a child event for the parent
        // needs to be send. However, there is no wrapper existing, and so
        // no notifications for grandchildren are required. If the are
        // grandgrandchildren, they would be notified by the layout.
        if( !rOldFrm.IsEmpty() && IsShowing( rOldFrm.SVRect() ) )
            ChildScrolledOut( pFrm );
    }
}

void SwAccessibleContext::InvalidateContent()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    _InvalidateContent( sal_False );
}

void SwAccessibleContext::InvalidateCaretPos()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    _InvalidateCaretPos();
}

void SwAccessibleContext::SetVisArea( const Rectangle& rNewVisArea )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwAccessibleFrame::SetVisArea( rNewVisArea );
}


void SwAccessibleContext::FireAccessibleEvent( AccessibleEventObject& rEvent )
{
    Reference < XAccessibleContext > xThis( this );
    rEvent.Source = xThis;

    ::cppu::OInterfaceIteratorHelper aIter( aAccessibleEventListeners );
    while( aIter.hasMoreElements() )
    {
        Reference < XAccessibleEventListener > xListener( aIter.next(),
                                                         UNO_QUERY );
        if( xListener.is() ) // TODO: test is unneccessary soon
        {
            try
            {
                xListener->notifyEvent( rEvent );
            }
            catch( ::com::sun::star::uno::RuntimeException& r )
            {
#ifdef DEBUG
                ByteString aError( "Runtime exception caught for event" );
                aError += ByteString::CreateFromInt32( rEvent.EventId );
                aError += ".:\n";
                aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
                DBG_ERROR( aError.GetBuffer() );
#endif
//              aIter.remove();
            }
        }
    }

}

void SwAccessibleContext::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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

void SwAccessibleContext::_InvalidateContent( sal_Bool )
{
}

void SwAccessibleContext::_InvalidateCaretPos()
{
}

OUString SwAccessibleContext::GetResource( sal_uInt16 nResId,
                                           const OUString *pArg1,
                                           const OUString *pArg2 )
{
    String sStr;
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        sStr = SW_RES( nResId );
    }

    if( pArg1 )
    {
        sStr.SearchAndReplace( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "$(ARG1)" )),
                               String( *pArg1 ) );
    }
    if( pArg2 )
    {
        sStr.SearchAndReplace( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "$(ARG2)" )),
                               String( *pArg2 ) );
    }

    return OUString( sStr );
}

Window *SwAccessibleContext::GetWindow()
{
    Window *pWin = 0;

    if( GetMap() )
    {
        const ViewShell *pVSh = GetMap()->GetShell();
        ASSERT( pVSh, "no view shell" );
        if( pVSh )
            pWin = pVSh->GetWin();

        ASSERT( pWin, "no window" );
    }

    return pWin;
}

void SwAccessibleContext::InitStates()
{
    bIsShowingState = IsShowing();

    ViewShell *pVSh = GetMap()->GetShell();
    bIsEditableState = pVSh &&  IsEditable( pVSh );
    bIsOpaqueState = pVSh && IsOpaque( pVSh );
    bIsDefuncState = sal_False;
}


SwAccessibleContext::SwAccessibleContext( SwAccessibleMap *pM,
                                          sal_Int16 nR,
                                          const SwFrm *pF ) :
    SwAccessibleFrame( pM->GetShell()->VisArea().SVRect(), pF ),
    aAccessibleEventListeners( aListenerMutex ),
    aFocusListeners( aListenerMutex ),
    pMap( pM ),
    nRole( nR )
{
    InitStates();
    DBG_MSG_CD( "constructed" )
}

SwAccessibleContext::SwAccessibleContext( SwAccessibleMap *pM,
                                          const OUString& rName,
                                          sal_Int16 nR,
                                          const SwFrm *pF ) :
    SwAccessibleFrame( pM->GetShell()->VisArea().SVRect(), pF ),
    sName( rName ),
    aAccessibleEventListeners( aListenerMutex ),
    aFocusListeners( aListenerMutex ),
    pMap( pM ),
    nRole( nR )
{
    InitStates();
    DBG_MSG_CD( "constructed" )
}

SwAccessibleContext::~SwAccessibleContext()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    DBG_MSG_CD( "destructed" )
    if( GetFrm() && GetMap() )
        GetMap()->RemoveContext( GetFrm() );
}

Reference< XAccessibleContext > SAL_CALL
    SwAccessibleContext::getAccessibleContext( void )
        throw (::com::sun::star::uno::RuntimeException)
{
    Reference < XAccessibleContext > xRet( this );
    return xRet;
}

long SAL_CALL SwAccessibleContext::getAccessibleChildCount( void )
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    return GetChildCount();
}

Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( long nIndex )
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pChild = GetChild( nIndex );
    if( 0 == pChild )
    {
        Reference < XAccessibleContext > xThis( this );
        IndexOutOfBoundsException aExcept(
                OUString( RTL_CONSTASCII_USTRINGPARAM("index out of bounds") ),
                xThis );                                        \
        throw aExcept;
    }

    ::vos::ORef < SwAccessibleContext > xChildImpl(
            GetMap()->GetContextImpl( pChild )  );
    xChildImpl->SetParent( this );
    Reference< XAccessible > xChild( xChildImpl.getBodyPtr() );

    return xChild;
}

Reference< XAccessible> SAL_CALL SwAccessibleContext::getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    ASSERT( pUpper, "no upper found" );

    Reference< XAccessible > xAcc;
    if( pUpper )
        xAcc = GetMap()->GetContext( pUpper );

    ASSERT( xAcc.is(), "no parent found" );
    if( !xAcc.is() )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "parent missing" );
    }

    // Remember the parent as weak ref.
    {
        vos::OGuard aGuard( aMutex );
        xWeakParent = xAcc;
    }

    return xAcc;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    ASSERT( pUpper, "no upper found" );

    sal_Int32 nIndex = -1;
    if( pUpper )
    {
        ::vos::ORef < SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pUpper )  );
        ASSERT( xAccImpl.isValid(), "no parent found" );
        if( xAccImpl.isValid() )
            nIndex = xAccImpl->GetChildIndex( GetFrm() );
    }
//  if( -1 == nIndex )
//  {
//      THROW_RUNTIME_EXCEPTION( XAccessibleContext, "child not contained in parent" );
//  }

    return nIndex;
}

sal_Int16 SAL_CALL SwAccessibleContext::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return nRole;
}

OUString SAL_CALL SwAccessibleContext::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ASSERT( !this, "description needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (method must be overloaded)" );
}

OUString SAL_CALL SwAccessibleContext::getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return sName;
}

Reference< XAccessibleRelationSet> SAL_CALL
    SwAccessibleContext::getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    // by default there are no relations
    Reference< XAccessibleRelationSet> xRet;
    return xRet;
}

Reference<XAccessibleStateSet> SAL_CALL
    SwAccessibleContext::getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ::utl::AccessibleStateSetHelper *pStateSet =
        new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );
    GetStates( *pStateSet );

    return xStateSet;
}

Locale SAL_CALL SwAccessibleContext::getLocale (void)
        throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Locale aLoc( Application::GetSettings().GetUILocale() );
    return aLoc;
}

void SAL_CALL SwAccessibleContext::addEventListener(
            const Reference< XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException)
{
    DBG_MSG( "accessible event listener added" )
    aAccessibleEventListeners.addInterface( xListener );
}

void SAL_CALL SwAccessibleContext::removeEventListener(
            const Reference< XAccessibleEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException)
{
    DBG_MSG( "accessible event listener removed" )
    aAccessibleEventListeners.removeInterface( xListener );
}

sal_Bool SAL_CALL SwAccessibleContext::contains(
            const ::com::sun::star::awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aLogBounds( GetBounds( GetFrm() ) ); // twip rel to doc root

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
    Point aLogPoint( pWin->PixelToLogic( aPixPoint ) ); // twip rel to doc root

    return aLogBounds.IsInside( aLogPoint );
}


Reference< XAccessible > SAL_CALL SwAccessibleContext::getAccessibleAt(
                const awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Reference< XAccessible > xAcc;

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
    Point aLogPoint( pWin->PixelToLogic( aPixPoint ) ); // twip rel to doc root

    const SwFrm *pFrm = GetChildAt( aLogPoint );
    if( pFrm )
        xAcc = GetMap()->GetContext( pFrm );

    return xAcc;
}


awt::Rectangle SAL_CALL SwAccessibleContext::getBounds()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pParent = GetParent();
    ASSERT( pParent, "no Parent found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pParent )

    Rectangle aLogBounds( GetBounds( GetFrm() ) ); // twip rel to doc root
    Rectangle aPixBounds;
    if( pParent->IsRootFrm() )
    {
        aPixBounds = pWin->LogicToPixel( aLogBounds );
    }
    else
    {
        Point aParentLogPos( GetBounds( pParent ).TopLeft() ); // twip rel to doc root
        MapMode aMapMode( pWin->GetMapMode() );
        aParentLogPos.X() *= -1;
        aParentLogPos.Y() *= -1;
        aMapMode.SetOrigin( aParentLogPos );
        aPixBounds = pWin->LogicToPixel( aLogBounds, aMapMode );
    }

    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}


awt::Point SAL_CALL SwAccessibleContext::getLocation()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pParent = GetParent();
    ASSERT( pParent, "no parent found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pParent )

    Point aLogPos( GetBounds( GetFrm() ).TopLeft() ); // twip rel to doc root
    Point aPixPos;
    if( pParent->IsRootFrm() )
    {
        aPixPos = pWin->LogicToPixel( aLogPos );
    }
    else
    {
        Point aParentLogPos( GetBounds( pParent ).TopLeft() ); // twip rel to doc root
        MapMode aMapMode( pWin->GetMapMode() );
        aParentLogPos.X() *= -1;
        aParentLogPos.Y() *= -1;
        aMapMode.SetOrigin( aParentLogPos );
        aPixPos = pWin->LogicToPixel( aLogPos, aMapMode );
    }
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleContext::getLocationOnScreen()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aLogPos( GetBounds( GetFrm() ).TopLeft() ); // twip rel to doc root
    Point aPixPos( pWin->LogicToPixel( aLogPos ) );
    aPixPos = pWin->OutputToAbsoluteScreenPixel( aPixPos );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleContext::getSize()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aLogSize( GetBounds( GetFrm() ).GetSize() ); // twip rel to whatever

    Size aPixSize( pWin->LogicToPixel( aLogSize ) );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}


sal_Bool SAL_CALL SwAccessibleContext::isShowing()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return IsShowing();
}


sal_Bool SAL_CALL SwAccessibleContext::isVisible()
        throw (RuntimeException)
{
    return sal_True;
}


sal_Bool SAL_CALL SwAccessibleContext::isFocusTraversable()
        throw (RuntimeException)
{
    return sal_False;
}


void SAL_CALL SwAccessibleContext::addFocusListener(
            const Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (RuntimeException)
{
    DBG_MSG( "focus listener added" )
    aFocusListeners.addInterface( xListener );
}


void SAL_CALL SwAccessibleContext::removeFocusListener(
            const Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (RuntimeException)
{
    DBG_MSG( "focus listener removed" )
    aFocusListeners.removeInterface( xListener );
}

void SAL_CALL SwAccessibleContext::grabFocus()
        throw (RuntimeException)
{
    // impossible
}


Any SAL_CALL SwAccessibleContext::getAccessibleKeyBinding()
        throw (RuntimeException)
{
    // There are no key bindings
    Any aAny;
    return aAny;
}


OUString SAL_CALL SwAccessibleContext::getImplementationName()
        throw( RuntimeException )
{
    ASSERT( !this, "implementation name needs to be overloaded" );

    THROW_RUNTIME_EXCEPTION( XServiceInfo, "implementation name needs to be overloaded" )
}

sal_Bool SAL_CALL
    SwAccessibleContext::supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException)
{
    ASSERT( !this, "supports service needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XServiceInfo, "supports service needs to be overloaded" )
}

Sequence< OUString > SAL_CALL SwAccessibleContext::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    ASSERT( !this, "supported services names needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XServiceInfo, "supported services needs to be overloaded" )
}

#if defined DEBUG && defined TEST_MIB
void lcl_SwAccessibleContext_DbgMsg( SwAccessibleContext *pThisAcc,
                                     const char *pMsg,
                                     SwAccessibleContext *pChildAcc,
                                       sal_Bool bConstrDestr )
{
    static SvFileStream aStrm( String::CreateFromAscii("j:\\acc.log"),
                    STREAM_WRITE|STREAM_TRUNC|STREAM_SHARE_DENYNONE );
    ByteString aName( String(pThisAcc->GetName()),
                      RTL_TEXTENCODING_ISO_8859_1 );
    if( aName.Len() )
    {
        aStrm << aName.GetBuffer()
              << ": ";
    }
    aStrm << pMsg;
    if( pChildAcc )
    {
        ByteString aChild( String(pChildAcc->GetName()),
                           RTL_TEXTENCODING_ISO_8859_1 );
        aStrm << ": "
              << aChild.GetBuffer();
    }
    aStrm << "\r\n    (";

    if( !bConstrDestr )
    {
        ByteString aDesc( String(pThisAcc->getAccessibleDescription()),
                           RTL_TEXTENCODING_ISO_8859_1 );
        aStrm << aDesc.GetBuffer()
              << ", ";
    }

    Rectangle aVisArea( pThisAcc->GetVisArea() );
    aStrm << "VA: "
          << ByteString::CreateFromInt32( aVisArea.Left() ).GetBuffer()
          << ","
          << ByteString::CreateFromInt32( aVisArea.Top() ).GetBuffer()
          << ","
          << ByteString::CreateFromInt32( aVisArea.GetWidth() ).GetBuffer()
          << ","
          << ByteString::CreateFromInt32( aVisArea.GetHeight() ).GetBuffer();

    if( pThisAcc->GetFrm() )
    {
        Rectangle aBounds( pThisAcc->GetBounds( pThisAcc->GetFrm() ) );
        aStrm << ", BB: "
              << ByteString::CreateFromInt32( aBounds.Left() ).GetBuffer()
              << ","
              << ByteString::CreateFromInt32( aBounds.Top() ).GetBuffer()
              << ","
              << ByteString::CreateFromInt32( aBounds.GetWidth() ).GetBuffer()
              << ","
              << ByteString::CreateFromInt32( aBounds.GetHeight() ).GetBuffer()
              << ")\r\n";
    }

    aStrm.Flush();
}
#endif
