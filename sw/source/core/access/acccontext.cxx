/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#if (OSL_DEBUG_LEVEL > 1) && defined TEST_MIB
    #ifndef _STRING_HXX
    #include <tools/string.hxx>
    #endif

    #ifndef _STREAM_HXX
    #include <tools/stream.hxx>
    #endif
#endif // #if (OSL_DEBUG_LEVEL > 1) && defined TEST_MIB
#include <tools/debug.hxx>
#include <vcl/window.hxx>
#include <errhdl.hxx>
#include <swtypes.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <viewsh.hxx>
#include <crsrsh.hxx>
#include <fesh.hxx>
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
#ifndef _ACCPARA_HXX
#include "accpara.hxx"
#endif
#include <PostItMgr.hxx>

using namespace sw::access;

#if (OSL_DEBUG_LEVEL > 1) && defined TEST_MIB
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
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

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
    vos::OGuard aGuard( aMutex );

    uno::Reference < XAccessible > xParent( pParent );
    xWeakParent = xParent;
}

uno::Reference< XAccessible > SwAccessibleContext::GetWeakParent() const
{
    vos::OGuard aGuard( aMutex );

    uno::Reference< XAccessible > xParent( xWeakParent );
    return xParent;
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

// get ViewShell from accessibility map, and cast to cursor shell
SwCrsrShell* SwAccessibleContext::GetCrsrShell()
{
    SwCrsrShell* pCrsrShell;
    ViewShell* pViewShell = GetMap() ? GetMap()->GetShell() : 0;
    ASSERT( pViewShell, "no view shell" );
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
    ASSERT( pViewShell, "no view shell" );
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
                // wouldn't know about the vis area.
                eAction = SCROLLED;
            }
            if( NONE != eAction )
            {
                if ( rLower.GetSwFrm() )
                {
                    ASSERT( !rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - always included child not considered!" );
                    const SwFrm* pLower( rLower.GetSwFrm() );
                    ::vos::ORef< SwAccessibleContext > xAccImpl =
                        //O is: GetMap()->GetContextImpl( pLower, SCROLLED_OUT == eAction ||
                        //                      SCROLLED_IN == eAction );
                        GetMap()->GetContextImpl( pLower, sal_True );
                    if( xAccImpl.isValid() )
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
                    ASSERT( !rLower.AlwaysIncludeAsChild(),
                            "<SwAccessibleContext::ChildrenScrolled(..)> - always included child not considered!" );
                    ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl =
                        //O is: GetMap()->GetContextImpl( rLower.GetSdrObject(),
                        //                        this,
                        //                        SCROLLED_OUT == eAction ||
                        //                        SCROLLED_IN == eAction );
                        GetMap()->GetContextImpl( rLower.GetDrawObject(),
                                                  this,
                                                  sal_True );
                    if( xAccImpl.isValid() )
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
                                             xAccImpl.getBodyPtr() );
                            break;
                        case SCROLLED_OUT:
                            {
                                xAccImpl->ViewForwarderChanged(
                                    ::accessibility::IAccessibleViewForwarderListener::VISIBLE_AREA,
                                    GetMap() );
                                //Remove
                                //DisposeShape( rLower.GetDrawObject(),
                                //            xAccImpl.getBodyPtr() );
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
                    ASSERT( rLower.AlwaysIncludeAsChild(),
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
        vos::OGuard aGuard( aMutex );
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
    ASSERT( GetVisArea() == GetMap()->GetVisArea(),
            "Vis area of child is wrong. Did it exist already?" );

    // Send child event at parent. That's all we have to do here.
    const SwFrm* pParent = GetParent();
    ::vos::ORef< SwAccessibleContext > xParentImpl(
         GetMap()->GetContextImpl( pParent, sal_False ) );
    uno::Reference < XAccessibleContext > xThis( this );
    if( xParentImpl.isValid() )
    {
        SetParent( xParentImpl.getBodyPtr() );

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.NewValue <<= xThis;

        xParentImpl->FireAccessibleEvent( aEvent );
        DBG_MSG_PARAM( "AccessibleChild (added)", xChildImpl.getBodyPtr() );

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

    //Remove Dispose When scrolledout
    // We now dispose the frame
    //  Dispose( sal_True );
}

// --> OD 2005-12-12 #i27301# - use new type definition for <_nStates>
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
            ::vos::ORef< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreView() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, sal_False );
            if( xAccImpl.isValid() )
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
// <--

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
            ::vos::ORef< SwAccessibleContext > xAccImpl;
            if( rLower.IsAccessible( GetShell()->IsPreView() ) )
                xAccImpl = GetMap()->GetContextImpl( pLower, sal_False );
            if( xAccImpl.isValid() )
                xAccImpl->Dispose( bRecursive );
            else if( bRecursive )
                DisposeChildren( pLower, bRecursive );
        }
        else if ( rLower.GetDrawObject() )
        {
            ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl(
                    GetMap()->GetContextImpl( rLower.GetDrawObject(),
                                          this, sal_False )  );
            if( xAccImpl.isValid() )
                DisposeShape( rLower.GetDrawObject(), xAccImpl.getBodyPtr() );
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
    ASSERT( GetFrm(), "fire event for diposed frame?" );
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
    DBG_MSG( "AccessibleVisibleData" )
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
    DBG_MSG( "StateChanged" )
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
    //Solution:Set editable state to graphic and other object when the document is editable
    {
        rStateSet.AddState( AccessibleStateType::EDITABLE );
        rStateSet.AddState( AccessibleStateType::RESIZABLE );
        rStateSet.AddState( AccessibleStateType::MOVEABLE );
    }
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
        vos::OGuard aGuard( aMutex );
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
    //Solution:Initialize the begin document load and IfAsynLoad to true
    , bBeginDocumentLoad( sal_True )
    , isIfAsynLoad( sal_True )
    , bIsSeletedInDoc( sal_False)
{
    InitStates();
    DBG_MSG_CD( "constructed" )
}

SwAccessibleContext::~SwAccessibleContext()
{
    if(Application::GetUnoWrapper())
        vos::OGuard aGuard(Application::GetSolarMutex());

    DBG_MSG_CD( "destructed" )
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )
    //Solution:Notify the frame is a document
    if( nRole == AccessibleRole::DOCUMENT )
        bIsAccDocUse = sal_True;

    return bDisposing ? 0 : GetChildCount( *(GetMap()) );
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    //Solution:Notify the frame is a document
    if( nRole == AccessibleRole::DOCUMENT )
        bIsAccDocUse = sal_True;

    const SwAccessibleChild aChild( GetChild( *(GetMap()), nIndex ) );
    if( !aChild.IsValid() )
    {
        uno::Reference < XAccessibleContext > xThis( this );
        lang::IndexOutOfBoundsException aExcept(
                OUString( RTL_CONSTASCII_USTRINGPARAM("index out of bounds") ),
                xThis );
        throw aExcept;
    }

    uno::Reference< XAccessible > xChild;
    if( aChild.GetSwFrm() )
    {
        ::vos::ORef < SwAccessibleContext > xChildImpl(
                GetMap()->GetContextImpl( aChild.GetSwFrm(), !bDisposing )  );
        //Solution:Send out accessible event when begin load.
        if( bBeginDocumentLoad && nRole == AccessibleRole::DOCUMENT )
        {

            FireStateChangedEvent( AccessibleStateType::FOCUSABLE,sal_True );
            FireStateChangedEvent( AccessibleStateType::BUSY,sal_True );
            if( !isIfAsynLoad )
            {
                FireStateChangedEvent( AccessibleStateType::FOCUSED,sal_True );
                // IA2 CWS. MT: OFFSCREEN == !SHOWING, should stay consistent
                // FireStateChangedEvent( AccessibleStateType::OFFSCREEN,sal_True );
                FireStateChangedEvent( AccessibleStateType::SHOWING,sal_True );
                FireStateChangedEvent( AccessibleStateType::BUSY,sal_False );
                // MT: OFFSCREEN again?
                // FireStateChangedEvent( AccessibleStateType::OFFSCREEN,sal_False );
            }
            bBeginDocumentLoad = sal_False;
        }
        if( xChildImpl.isValid() )
        {
            xChildImpl->SetParent( this );
            xChild = xChildImpl.getBodyPtr();
        }
    }
    else if ( aChild.GetDrawObject() )
    {
        ::vos::ORef < ::accessibility::AccessibleShape > xChildImpl(
                GetMap()->GetContextImpl( aChild.GetDrawObject(),
                                          this, !bDisposing )  );
        if( xChildImpl.isValid() )
            xChild = xChildImpl.getBodyPtr();
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    ASSERT( pUpper != 0 || bDisposing, "no upper found" );

    uno::Reference< XAccessible > xAcc;
    if( pUpper )
        xAcc = GetMap()->GetContext( pUpper, !bDisposing );

    ASSERT( xAcc.is() || bDisposing, "no parent found" );

    // Remember the parent as weak ref.
    {
        vos::OGuard aWeakParentGuard( aMutex );
        xWeakParent = xAcc;
    }

    return xAcc;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetParent();
    ASSERT( pUpper != 0 || bDisposing, "no upper found" );

    sal_Int32 nIndex = -1;
    if( pUpper )
    {
        ::vos::ORef < SwAccessibleContext > xAccImpl(
            GetMap()->GetContextImpl( pUpper, !bDisposing )  );
        ASSERT( xAccImpl.isValid() || bDisposing, "no parent found" );
        if( xAccImpl.isValid() )
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
    ASSERT( !this, "description needs to be overloaded" );
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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

    lang::Locale aLoc( Application::GetSettings().GetLocale() );
    return aLoc;
}

void SAL_CALL SwAccessibleContext::addEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
        throw (uno::RuntimeException)
{
    DBG_MSG( "accessible event listener added" )

    if (xListener.is())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
        if (!nClientId)
            nClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( nClientId, xListener );
    }
}

void SAL_CALL SwAccessibleContext::removeEventListener(
            const uno::Reference< XAccessibleEventListener >& xListener )
        throw (uno::RuntimeException)
{
    DBG_MSG( "accessible event listener removed" )

    if (xListener.is())
    {
        vos::OGuard aGuard(Application::GetSolarMutex());
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    uno::Reference< XAccessible > xAcc;

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to parent
    if( !GetFrm()->IsRootFrm() )
    {
        SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
        Point aPixPos( GetMap()->CoreToPixel( aLogBounds.SVRect() ).TopLeft() );
        aPixPoint.X() += aPixPos.X();
        aPixPoint.Y() += aPixPos.Y();
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

   - realative

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
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pParent = GetParent();
    ASSERT( pParent, "no Parent found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pParent )

    SwRect aLogBounds( GetBounds( *(GetMap()), GetFrm() ) ); // twip rel to doc root
    Rectangle aPixBounds( 0, 0, 0, 0 );
    if( GetFrm()->IsPageFrm() &&
        static_cast < const SwPageFrm * >( GetFrm() )->IsEmptyPage() )
    {
        ASSERT( GetShell()->IsPreView(), "empty page accessible?" );
        if( GetShell()->IsPreView() )
        {
            // OD 15.01.2003 #103492# - adjust method call <GetMap()->GetPreViewPageSize()>
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
            aPixBounds.Move( -aParentPixPos.X(), -aParentPixPos.Y() );
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
    awt::Point aPoint(aPixPos.X(), aPixPos.Y());

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
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    ASSERT( !this, "implementation name needs to be overloaded" );

    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "implementation name needs to be overloaded" )
}

sal_Bool SAL_CALL
    SwAccessibleContext::supportsService (const ::rtl::OUString& )
        throw (uno::RuntimeException)
{
    ASSERT( !this, "supports service needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "supports service needs to be overloaded" )
}

uno::Sequence< OUString > SAL_CALL SwAccessibleContext::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    ASSERT( !this, "supported services names needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( lang::XServiceInfo, "supported services needs to be overloaded" )
}

void SwAccessibleContext::DisposeShape( const SdrObject *pObj,
                                ::accessibility::AccessibleShape *pAccImpl )
{
    ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl( pAccImpl );
    if( !xAccImpl.isValid() )
        xAccImpl = GetMap()->GetContextImpl( pObj, this, sal_True );

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::CHILD;
    uno::Reference< XAccessible > xAcc( xAccImpl.getBodyPtr() );
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( GetFrm() && GetMap(), "already disposed" );
    ASSERT( GetMap()->GetVisArea() == GetVisArea(),
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
        DBG_MSG_THIS_PARAM( "AccessibleChild (removed)", pAcc, this )
    }

    // set defunc state (its not required to broadcast a state changed
    // event if the object is diposed afterwards)
    {
        vos::OGuard aDefuncStateGuard( aMutex );
        bIsDefuncState = sal_True;
    }

    // broadcast dispose event
    if ( nClientId )
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, *this );
        nClientId =  0;
        DBG_MSG_CD( "dispose" )
    }

    RemoveFrmFromAccessibleMap();
    ClearFrm();
    pMap = 0;

    bDisposing = sal_False;
}

void SwAccessibleContext::DisposeChild( const SwAccessibleChild& rChildFrmOrObj,
                                        sal_Bool bRecursive )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if ( IsShowing( *(GetMap()), rChildFrmOrObj ) ||
         rChildFrmOrObj.AlwaysIncludeAsChild() ||
         !SwAccessibleChild( GetFrm() ).IsVisibleChildrenOnly() )
    {
        // If the object could have existed before, than there is nothing to do,
        // because no wrapper exists now and therefor no one is interested to
        // get notified of the movement.
        if( rChildFrmOrObj.GetSwFrm() )
        {
            ::vos::ORef< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
            xAccImpl->Dispose( bRecursive );
        }
        else if ( rChildFrmOrObj.GetDrawObject() )
        {
            ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                              this, sal_True );
            DisposeShape( rChildFrmOrObj.GetDrawObject(),
                          xAccImpl.getBodyPtr() );
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( GetFrm() && !GetFrm()->Frm().IsEmpty(), "context should have a size" );

    sal_Bool bIsOldShowingState;
    sal_Bool bIsNewShowingState = IsShowing( *(GetMap()) );
    {
        vos::OGuard aShowingStateGuard( aMutex );
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
        //Remove
        //Dispose( sal_True );
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
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !rChildFrmOrObj.GetSwFrm() ||
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
                ::vos::ORef< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
                xAccImpl->ScrolledIn();
            }
            else if ( rChildFrmOrObj.GetDrawObject() )
            {
                ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                                  this, sal_True );
                // --> OD 2004-11-29 #i37790#
                if ( xAccImpl.isValid() )
                {
                    ScrolledInShape( rChildFrmOrObj.GetDrawObject(),
                                     xAccImpl.getBodyPtr() );
                }
                else
                {
                    ASSERT( false ,
                            "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - no accessible shape found." );
                }
                // <--
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
                ::vos::ORef< SwAccessibleContext > xAccImpl =
                    GetMap()->GetContextImpl( rChildFrmOrObj.GetSwFrm(),
                                              sal_True );
                xAccImpl->SetParent( this );
                xAccImpl->Dispose( sal_True );
            }
            else if ( rChildFrmOrObj.GetDrawObject() )
            {
                ::vos::ORef< ::accessibility::AccessibleShape > xAccImpl =
                        GetMap()->GetContextImpl( rChildFrmOrObj.GetDrawObject(),
                                                  this, sal_True );
                DisposeShape( rChildFrmOrObj.GetDrawObject(),
                          xAccImpl.getBodyPtr() );
            }
            else if ( rChildFrmOrObj.GetWindow() )
            {
                ASSERT( false,
                        "<SwAccessibleContext::InvalidateChildPosOrSize(..)> - not expected to handle dispose of child of type <Window>." );
            }
        }
    }
}

void SwAccessibleContext::InvalidateContent()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    _InvalidateContent( sal_False );
}

void SwAccessibleContext::InvalidateCursorPos()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    _InvalidateCursorPos();
}

void SwAccessibleContext::InvalidateFocus()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    _InvalidateFocus();
}

// --> OD 2005-12-12 #i27301# - use new type definition for <_nStates>
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
                    vos::OGuard aGuard( aMutex );
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
                    vos::OGuard aGuard( aMutex );
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
// <--

void SwAccessibleContext::InvalidateRelation( sal_uInt16 nType )
{
    AccessibleEventObject aEvent;
    aEvent.EventId = nType;

    FireAccessibleEvent( aEvent );
}

/** text selection has changed

    OD 2005-12-14 #i27301#

    @author OD
*/
void SwAccessibleContext::InvalidateTextSelection()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::TEXT_SELECTION_CHANGED;

    FireAccessibleEvent( aEvent );
}

/** attributes has changed

    OD 2009-01-06 #i88069#

    @author OD
*/
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
        pCrsrShell->SetSelection( *pPaM );
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
/** get additional accessible child by index

    OD 2010-01-27 #i88070#

    @author OD
*/
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

/** get all additional accessible children

    OD 2010-01-27 #i88070#

    @author OD
*/
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

#if (OSL_DEBUG_LEVEL > 1) && defined TEST_MIB
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
