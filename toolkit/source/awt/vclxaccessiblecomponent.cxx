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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <vcl/dialog.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/window.hxx>
#include <tools/debug.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;


DBG_NAME(VCLXAccessibleComponent)


VCLXAccessibleComponent::VCLXAccessibleComponent( VCLXWindow* pVCLXindow )
    : AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    , OAccessibleImplementationAccess( )
{
    DBG_CTOR( VCLXAccessibleComponent, 0 );
    mpVCLXindow = pVCLXindow;
    mxWindow = pVCLXindow;

    m_pSolarLock = static_cast< VCLExternalSolarLock* >( getExternalLock( ) );

    DBG_ASSERT( pVCLXindow->GetWindow(), "VCLXAccessibleComponent - no window!" );
    if ( pVCLXindow->GetWindow() )
    {
      pVCLXindow->GetWindow()->AddEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
      pVCLXindow->GetWindow()->AddChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    // announce the XAccessible of our creator to the base class
    lateInit( pVCLXindow );
}

VCLXAccessibleComponent::~VCLXAccessibleComponent()
{
    DBG_DTOR( VCLXAccessibleComponent, 0 );

    ensureDisposed();

    if ( mpVCLXindow && mpVCLXindow->GetWindow() )
    {
        mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
        mpVCLXindow->GetWindow()->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    delete m_pSolarLock;
    m_pSolarLock = NULL;
    // This is not completely safe. If we assume that the base class dtor calls some method which
    // uses this lock, the we crash. However, as the base class' dtor does not have a chance to call _out_
    // virtual methods, this is no problem as long as the base class is safe, i.e. does not use the external
    // lock from within it's dtor. At the moment, we _know_ the base class is safe in this respect, so
    // let's assume it keeps this way.
    // @see OAccessibleContextHelper::OAccessibleContextHelper( IMutex* )
}

IMPLEMENT_FORWARD_XINTERFACE3( VCLXAccessibleComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleImplementationAccess, VCLXAccessibleComponent_BASE )
IMPLEMENT_FORWARD_XTYPEPROVIDER3( VCLXAccessibleComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleImplementationAccess, VCLXAccessibleComponent_BASE )

OUString VCLXAccessibleComponent::getImplementationName() throw (uno::RuntimeException)
{
    return OUString("com.sun.star.comp.toolkit.AccessibleWindow");
}

sal_Bool VCLXAccessibleComponent::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames( getSupportedServiceNames() );
    const OUString* pNames = aNames.getConstArray();
    const OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

uno::Sequence< OUString > VCLXAccessibleComponent::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames(1);
    aNames[0] = OUString("com.sun.star.awt.AccessibleWindow");
    return aNames;
}

IMPL_LINK( VCLXAccessibleComponent, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_CHKTHIS(VCLXAccessibleComponent,0);

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );

        /* Ignore VCLEVENT_WINDOW_ENDPOPUPMODE, because the UNO accessibility wrapper
         * might have been destroyed by the previous VCLEventListener (if no AT tool
         * is running), e.g. sub-toolbars in impress.
         */
    if ( pEvent && pEvent->ISA( VclWindowEvent ) && mxWindow.is() /* #122218# */ && (pEvent->GetId() != VCLEVENT_WINDOW_ENDPOPUPMODE) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        if( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() || ( pEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

IMPL_LINK( VCLXAccessibleComponent, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    DBG_CHKTHIS(VCLXAccessibleComponent,0);

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) && mxWindow.is() /* #i68079# */ )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        if( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() )
        {
            // #103087# to prevent an early release of the component
            uno::Reference< accessibility::XAccessibleContext > xTmp = this;

            ProcessWindowChildEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::GetChildAccessible( const VclWindowEvent& rVclWindowEvent )
{
    // checks if the data in the window event is our direct child
    // and returns its accessible

    // MT: Change this later, normaly a show/hide event shouldn't have the Window* in pData.
    Window* pChildWindow = (Window *) rVclWindowEvent.GetData();
    if( pChildWindow && GetWindow() == pChildWindow->GetAccessibleParentWindow() )
        return pChildWindow->GetAccessible( rVclWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW );
    else
        return uno::Reference< accessibility::XAccessible > ();
}

void VCLXAccessibleComponent::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    uno::Any aOldValue, aNewValue;
    uno::Reference< accessibility::XAccessible > xAcc;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
        {
            xAcc = GetChildAccessible( rVclWindowEvent );
            if( xAcc.is() )
            {
                aNewValue <<= xAcc;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::CHILD, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
        {
            xAcc = GetChildAccessible( rVclWindowEvent );
            if( xAcc.is() )
            {
                aOldValue <<= xAcc;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::CHILD, aOldValue, aNewValue );
            }
        }
        break;
    }
}

void VCLXAccessibleComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    uno::Any aOldValue, aNewValue;

    Window* pAccWindow = rVclWindowEvent.GetWindow();
    DBG_ASSERT( pAccWindow, "VCLXAccessibleComponent::ProcessWindowEvent - Window?" );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_OBJECT_DYING:
        {
            pAccWindow->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
            pAccWindow->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
            mxWindow.clear();
            mpVCLXindow = NULL;
        }
        break;
        case VCLEVENT_WINDOW_CHILDDESTROYED:
        {
            Window* pWindow = (Window*) rVclWindowEvent.GetData();
            DBG_ASSERT( pWindow, "VCLEVENT_WINDOW_CHILDDESTROYED - Window=?" );
            if ( pWindow->GetAccessible( sal_False ).is() )
            {
                aOldValue <<= pWindow->GetAccessible( sal_False );
                NotifyAccessibleEvent( accessibility::AccessibleEventId::CHILD, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            // avoid notification if a child frame is already active
            // only one frame may be active at a given time
            if ( !pAccWindow->HasActiveChildFrame() &&
                 ( getAccessibleRole() == accessibility::AccessibleRole::FRAME ||
                   getAccessibleRole() == accessibility::AccessibleRole::ALERT ||
                   getAccessibleRole() == accessibility::AccessibleRole::DIALOG ) )  // #i18891#
            {
                aNewValue <<= accessibility::AccessibleStateType::ACTIVE;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            if ( getAccessibleRole() == accessibility::AccessibleRole::FRAME ||
                 getAccessibleRole() == accessibility::AccessibleRole::ALERT ||
                 getAccessibleRole() == accessibility::AccessibleRole::DIALOG )  // #i18891#
            {
                aOldValue <<= accessibility::AccessibleStateType::ACTIVE;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_CONTROL_GETFOCUS:
        {
            if( (pAccWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_CONTROL_GETFOCUS) ||
                (!pAccWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_WINDOW_GETFOCUS) )
            {
                // if multiple listeners were registered it is possible that the
                // focus was changed during event processing (eg SfxTopWindow )
                // #106082# allow ChildPathFocus only for CompoundControls, for windows the focus must be in the window itself
                if( (pAccWindow->IsCompoundControl() && pAccWindow->HasChildPathFocus()) ||
                    (!pAccWindow->IsCompoundControl() && pAccWindow->HasFocus()) )
                {
                    aNewValue <<= accessibility::AccessibleStateType::FOCUSED;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
        case VCLEVENT_CONTROL_LOSEFOCUS:
        {
            if( (pAccWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_CONTROL_LOSEFOCUS) ||
                (!pAccWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_WINDOW_LOSEFOCUS) )
            {
                aOldValue <<= accessibility::AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_FRAMETITLECHANGED:
        {
            OUString aOldName( *((OUString*) rVclWindowEvent.GetData()) );
            OUString aNewName( getAccessibleName() );
            aOldValue <<= aOldName;
            aNewValue <<= aNewName;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_ENABLED:
        {
            aNewValue <<= accessibility::AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            aNewValue <<= accessibility::AccessibleStateType::SENSITIVE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_DISABLED:
        {
            aOldValue <<= accessibility::AccessibleStateType::SENSITIVE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );

            aOldValue <<= accessibility::AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        case VCLEVENT_WINDOW_RESIZE:
        {
            NotifyAccessibleEvent( accessibility::AccessibleEventId::BOUNDRECT_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_MENUBARADDED:
        {
            MenuBar* pMenuBar = (MenuBar*) rVclWindowEvent.GetData();
            if ( pMenuBar )
            {
                uno::Reference< accessibility::XAccessible > xChild( pMenuBar->GetAccessible() );
                if ( xChild.is() )
                {
                    aNewValue <<= xChild;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::CHILD, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MENUBARREMOVED:
        {
            MenuBar* pMenuBar = (MenuBar*) rVclWindowEvent.GetData();
            if ( pMenuBar )
            {
                uno::Reference< accessibility::XAccessible > xChild( pMenuBar->GetAccessible() );
                if ( xChild.is() )
                {
                    aOldValue <<= xChild;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::CHILD, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MINIMIZE:
        {
            aNewValue <<= accessibility::AccessibleStateType::ICONIFIED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_NORMALIZE:
        {
            aOldValue <<= accessibility::AccessibleStateType::ICONIFIED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        default:
        {
        }
        break;
    }
}

void VCLXAccessibleComponent::disposing()
{
    if ( mpVCLXindow && mpVCLXindow->GetWindow() )
    {
        mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
        mpVCLXindow->GetWindow()->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    AccessibleExtendedComponentHelper_BASE::disposing();

    mxWindow.clear();
    mpVCLXindow = NULL;
}

Window* VCLXAccessibleComponent::GetWindow() const
{
    return GetVCLXWindow() ? GetVCLXWindow()->GetWindow() : NULL;
}

void VCLXAccessibleComponent::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window *pLabeledBy = pWindow->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabeledBy->GetAccessible();
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::LABELED_BY, aSequence ) );
        }

        Window* pLabelFor = pWindow->GetAccessibleRelationLabelFor();
        if ( pLabelFor && pLabelFor != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabelFor->GetAccessible();
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::LABEL_FOR, aSequence ) );
        }

        Window* pMemberOf = pWindow->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pMemberOf->GetAccessible();
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
    }
}

void VCLXAccessibleComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsVisible() )
        {
            rStateSet.AddState( accessibility::AccessibleStateType::VISIBLE );
            rStateSet.AddState( accessibility::AccessibleStateType::SHOWING );
        }
        else
        {
            rStateSet.AddState( accessibility::AccessibleStateType::INVALID );
        }

        if ( pWindow->IsEnabled() )
        {
            rStateSet.AddState( accessibility::AccessibleStateType::ENABLED );
            rStateSet.AddState( accessibility::AccessibleStateType::SENSITIVE );
        }

        if ( pWindow->HasChildPathFocus() &&
             ( getAccessibleRole() == accessibility::AccessibleRole::FRAME ||
               getAccessibleRole() == accessibility::AccessibleRole::ALERT ||
               getAccessibleRole() == accessibility::AccessibleRole::DIALOG ) )  // #i18891#
            rStateSet.AddState( accessibility::AccessibleStateType::ACTIVE );

        if ( pWindow->HasFocus() || ( pWindow->IsCompoundControl() && pWindow->HasChildPathFocus() ) )
            rStateSet.AddState( accessibility::AccessibleStateType::FOCUSED );

        if ( pWindow->IsWait() )
            rStateSet.AddState( accessibility::AccessibleStateType::BUSY );

        if ( pWindow->GetStyle() & WB_SIZEABLE )
            rStateSet.AddState( accessibility::AccessibleStateType::RESIZABLE );

        if( pWindow->IsDialog() )
        {
            Dialog *pDlg = static_cast< Dialog* >( pWindow );
            if( pDlg->IsInExecute() )
                rStateSet.AddState( accessibility::AccessibleStateType::MODAL );
        }
    }
    else
    {
        rStateSet.AddState( accessibility::AccessibleStateType::DEFUNC );
    }

/*

MUST BE SET FROM DERIVED CLASSES:

CHECKED
COLLAPSED
EXPANDED
EXPANDABLE
EDITABLE
FOCUSABLE
HORIZONTAL
VERTICAL
ICONIFIED
MULTILINE
MULTI_SELECTABLE
PRESSED
SELECTABLE
SELECTED
SINGLE_LINE
TRANSIENT

    */
}


// accessibility::XAccessibleContext
sal_Int32 VCLXAccessibleComponent::getAccessibleChildCount() throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nChildren = 0;
    if ( GetWindow() )
        nChildren = GetWindow()->GetAccessibleChildWindowCount();

    return nChildren;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pChild = GetWindow()->GetAccessibleChildWindow( (sal_uInt16)i );
        if ( pChild )
            xAcc = pChild->GetAccessible();
    }

    return xAcc;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getVclParent() const
{
    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
            xAcc = pParent->GetAccessible();
    }
    return xAcc;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleParent(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessible > xAcc( implGetForeignControlledParent() );
    if ( !xAcc.is() )
        // we do _not_ have a foreign-controlled parent -> default to our VCL parent
        xAcc = getVclParent();

    return xAcc;
}

sal_Int32 VCLXAccessibleComponent::getAccessibleIndexInParent(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;

    uno::Reference< accessibility::XAccessible > xAcc( implGetForeignControlledParent() );
    if ( xAcc.is() )
    {   // we _do_ have a foreign-controlled parent -> use the base class' implementation,
        // which goes the UNO way
        nIndex = AccessibleExtendedComponentHelper_BASE::getAccessibleIndexInParent( );
    }
    else
    {
        if ( GetWindow() )
        {
            Window* pParent = GetWindow()->GetAccessibleParentWindow();
            if ( pParent )
            {
                //  Iterate over all the parent's children and search for this object.
                // this should be compatible with the code in SVX
                uno::Reference< accessibility::XAccessible > xParentAcc( pParent->GetAccessible() );
                if ( xParentAcc.is() )
                {
                    uno::Reference< accessibility::XAccessibleContext > xParentContext ( xParentAcc->getAccessibleContext() );
                    if ( xParentContext.is() )
                    {
                        sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
                        for ( sal_Int32 i=0; i<nChildCount; i++ )
                        {
                            uno::Reference< accessibility::XAccessible > xChild( xParentContext->getAccessibleChild(i) );
                            if ( xChild.is() )
                            {
                                uno::Reference< accessibility::XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                                if ( xChildContext == (accessibility::XAccessibleContext*) this )
                                {
                                    nIndex = i;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return nIndex;
}

sal_Int16 VCLXAccessibleComponent::getAccessibleRole(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int16 nRole = 0;

    if ( GetWindow() )
        nRole = GetWindow()->GetAccessibleRole();

    return nRole;
}

OUString VCLXAccessibleComponent::getAccessibleDescription(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString aDescription;

    if ( GetWindow() )
        aDescription = GetWindow()->GetAccessibleDescription();

    return aDescription;
}

OUString VCLXAccessibleComponent::getAccessibleName(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString aName;
    if ( GetWindow() )
    {
        aName = GetWindow()->GetAccessibleName();
#if OSL_DEBUG_LEVEL > 1
        aName += OUString(" (Type = ");
        aName += OUString::number(static_cast<sal_Int32>(GetWindow()->GetType()));
        aName += OUString( ")");
#endif
    }
    return aName;
}

uno::Reference< accessibility::XAccessibleRelationSet > VCLXAccessibleComponent::getAccessibleRelationSet(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    uno::Reference< accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;
    FillAccessibleRelationSet( *pRelationSetHelper );
    return xSet;
}

uno::Reference< accessibility::XAccessibleStateSet > VCLXAccessibleComponent::getAccessibleStateSet(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< accessibility::XAccessibleStateSet > xSet = pStateSetHelper;
    FillAccessibleStateSet( *pStateSetHelper );
    return xSet;
}

lang::Locale VCLXAccessibleComponent::getLocale() throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleAtPoint( const awt::Point& rPoint ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessible > xChild;
    for ( sal_uInt32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
    {
        uno::Reference< accessibility::XAccessible > xAcc = getAccessibleChild( i );
        if ( xAcc.is() )
        {
            uno::Reference< accessibility::XAccessibleComponent > xComp( xAcc->getAccessibleContext(), uno::UNO_QUERY );
            if ( xComp.is() )
            {
                Rectangle aRect = VCLRectangle( xComp->getBounds() );
                Point aPos = VCLPoint( rPoint );
                if ( aRect.IsInside( aPos ) )
                {
                    xChild = xAcc;
                    break;
                }
            }
        }
    }

    return xChild;
}

// accessibility::XAccessibleComponent
awt::Rectangle VCLXAccessibleComponent::implGetBounds() throw (uno::RuntimeException)
{
    awt::Rectangle aBounds ( 0, 0, 0, 0 );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
        aBounds = AWTRectangle( aRect );
        Window* pParent = pWindow->GetAccessibleParentWindow();
        if ( pParent )
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative( NULL );
            awt::Point aParentScreenLoc = AWTPoint( aParentRect.TopLeft() );
            aBounds.X -= aParentScreenLoc.X;
            aBounds.Y -= aParentScreenLoc.Y;
        }
    }

    uno::Reference< accessibility::XAccessible > xParent( implGetForeignControlledParent() );
    if ( xParent.is() )
    {   // hmm, we can't rely on our VCL coordinates, as in the Accessibility Hierarchy, somebody gave
        // us a parent which is different from our VCL parent
        // (actually, we did not check if it's really different ...)

        // the screen location of the foreign parent
        uno::Reference< accessibility::XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), uno::UNO_QUERY );
        DBG_ASSERT( xParentComponent.is(), "VCLXAccessibleComponent::implGetBounds: invalid (foreign) parent component!" );

        awt::Point aScreenLocForeign( 0, 0 );
        if ( xParentComponent.is() )
            aScreenLocForeign = xParentComponent->getLocationOnScreen();

        // the screen location of the VCL parent
        xParent = getVclParent();
        if ( xParent.is() )
            xParentComponent = xParentComponent.query( xParent->getAccessibleContext() );

        awt::Point aScreenLocVCL( 0, 0 );
        if ( xParentComponent.is() )
            aScreenLocVCL = xParentComponent->getLocationOnScreen();

        // the difference between them
        awt::Size aOffset( aScreenLocVCL.X - aScreenLocForeign.X, aScreenLocVCL.Y - aScreenLocForeign.Y );
        // move the bounds
        aBounds.X += aOffset.Width;
        aBounds.Y += aOffset.Height;
    }

    return aBounds;
}

awt::Point VCLXAccessibleComponent::getLocationOnScreen(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    awt::Point aPos;
    if ( GetWindow() )
    {
        Rectangle aRect = GetWindow()->GetWindowExtentsRelative( NULL );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }

    return aPos;
}

void VCLXAccessibleComponent::grabFocus(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessibleStateSet > xStates = getAccessibleStateSet();
    if ( mxWindow.is() && xStates.is() && xStates->contains( accessibility::AccessibleStateType::FOCUSABLE ) )
        mxWindow->setFocus();
}

sal_Int32 SAL_CALL VCLXAccessibleComponent::getForeground(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlForeground() )
            nColor = pWindow->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}

sal_Int32 SAL_CALL VCLXAccessibleComponent::getBackground(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlBackground() )
            nColor = pWindow->GetControlBackground().GetColor();
        else
            nColor = pWindow->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

// XAccessibleExtendedComponent

uno::Reference< awt::XFont > SAL_CALL VCLXAccessibleComponent::getFont(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< awt::XFont > xFont;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        uno::Reference< awt::XDevice > xDev( pWindow->GetComponentInterface(), uno::UNO_QUERY );
        if ( xDev.is() )
        {
            Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            VCLXFont* pVCLXFont = new VCLXFont;
            pVCLXFont->Init( *xDev.get(), aFont );
            xFont = pVCLXFont;
        }
    }

    return xFont;
}

OUString SAL_CALL VCLXAccessibleComponent::getTitledBorderText(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetText();

    return sRet;
}

OUString SAL_CALL VCLXAccessibleComponent::getToolTipText(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetQuickHelpText();

    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
