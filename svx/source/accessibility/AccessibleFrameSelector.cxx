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

#include "AccessibleFrameSelector.hxx"
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/frmsel.hxx>
#include <svx/dialmgr.hxx>
#include "editeng/unolingu.hxx"

#include <svx/dialogs.hrc>
#include "frmsel.hrc"

namespace svx {
namespace a11y {

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::beans::XPropertyChangeListener;
using ::com::sun::star::awt::XFocusListener;

using namespace ::com::sun::star::accessibility;


AccFrameSelector::AccFrameSelector( FrameSelector& rFrameSel, FrameBorderType eBorder ) :
    Resource( SVX_RES( RID_SVXSTR_BORDER_CONTROL ) ),
    mpFrameSel( &rFrameSel ),
    meBorder( eBorder ),
    maFocusListeners( maFocusMutex ),
    maPropertyListeners( maPropertyMutex ),
    maNames( SVX_RES( ARR_TEXTS ) ),
    maDescriptions( SVX_RES(ARR_DESCRIPTIONS ) ),
    mnClientId( 0 )
{
    FreeResource();

    if ( mpFrameSel )
    {
        mpFrameSel->AddEventListener( LINK( this, AccFrameSelector, WindowEventListener ) );
    }
}



AccFrameSelector::~AccFrameSelector()
{
    RemoveFrameSelEventListener();
}



void AccFrameSelector::RemoveFrameSelEventListener()
{
    if ( mpFrameSel )
    {
        mpFrameSel->RemoveEventListener( LINK( this, AccFrameSelector, WindowEventListener ) );
    }
}



Reference< XAccessibleContext > AccFrameSelector::getAccessibleContext(  )
    throw (RuntimeException, std::exception)
{
    return this;
}



sal_Int32 AccFrameSelector::getAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    return (meBorder == FRAMEBORDER_NONE) ? mpFrameSel->GetEnabledBorderCount() : 0;
}

Reference< XAccessible > AccFrameSelector::getAccessibleChild( sal_Int32 i )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet;
    if( meBorder == FRAMEBORDER_NONE )
        xRet = mpFrameSel->GetChildAccessible( i );
    if( !xRet.is() )
        throw RuntimeException();
    return xRet;
}

Reference< XAccessible > AccFrameSelector::getAccessibleParent(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet;
    if(meBorder == FRAMEBORDER_NONE)
        xRet = mpFrameSel->GetParent()->GetAccessible();
    else
        xRet = mpFrameSel->CreateAccessible();
    return xRet;
}

sal_Int32 AccFrameSelector::getAccessibleIndexInParent(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();

    sal_Int32 nIdx = 0;
    if( meBorder == FRAMEBORDER_NONE )
    {
        vcl::Window* pTabPage = mpFrameSel->GetParent();
        sal_Int32 nChildren = pTabPage->GetChildCount();
        for( nIdx = 0; nIdx < nChildren; ++nIdx )
            if( pTabPage->GetChild( static_cast< sal_uInt16 >( nIdx ) ) == mpFrameSel )
                break;
    }
    else
        nIdx = mpFrameSel->GetEnabledBorderIndex( meBorder );

    if( nIdx < 0 )
        throw RuntimeException();
    return nIdx;
}

sal_Int16 AccFrameSelector::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    return meBorder == FRAMEBORDER_NONE ? AccessibleRole::OPTION_PANE : AccessibleRole::CHECK_BOX;
}

OUString AccFrameSelector::getAccessibleDescription(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    return maDescriptions.GetString(meBorder);
}

OUString AccFrameSelector::getAccessibleName(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    return maNames.GetString(meBorder);
}

Reference< XAccessibleRelationSet > AccFrameSelector::getAccessibleRelationSet(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FRAMEBORDER_NONE)
    {
        //add the label relation
        vcl::Window *pLabeledBy = mpFrameSel->GetAccessibleRelationLabeledBy();
        if ( pLabeledBy && pLabeledBy != mpFrameSel )
        {
            AccessibleRelation aLabelRelation;
            aLabelRelation.RelationType = AccessibleRelationType::LABELED_BY;
            aLabelRelation.TargetSet.realloc(1);
            aLabelRelation.TargetSet.getArray()[0]  = pLabeledBy->GetAccessible();
            pHelper->AddRelation(aLabelRelation);
        }
        vcl::Window* pMemberOf = mpFrameSel->GetAccessibleRelationMemberOf();
        if ( pMemberOf && pMemberOf != mpFrameSel )
        {
            AccessibleRelation aMemberOfRelation;
            aMemberOfRelation.RelationType = AccessibleRelationType::MEMBER_OF;
            aMemberOfRelation.TargetSet.realloc(1);
            aMemberOfRelation.TargetSet.getArray()[0]  = pMemberOf->GetAccessible();
            pHelper->AddRelation(aMemberOfRelation);
        }
    }
    return xRet;
}

Reference< XAccessibleStateSet > AccFrameSelector::getAccessibleStateSet(  )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xRet = pStateSetHelper;

    if(!mpFrameSel)
        pStateSetHelper->AddState(AccessibleStateType::DEFUNC);
    else
    {
        const sal_Int16 aStandardStates[] =
        {
            AccessibleStateType::EDITABLE,
            AccessibleStateType::FOCUSABLE,
            AccessibleStateType::MULTI_SELECTABLE,
            AccessibleStateType::SELECTABLE,
            AccessibleStateType::SHOWING,
            AccessibleStateType::VISIBLE,
            AccessibleStateType::OPAQUE,
            0};
        sal_Int16 nState = 0;
        while(aStandardStates[nState])
        {
            pStateSetHelper->AddState(aStandardStates[nState++]);
        }
        if(mpFrameSel->IsEnabled())
        {
            pStateSetHelper->AddState(AccessibleStateType::ENABLED);
            pStateSetHelper->AddState(AccessibleStateType::SENSITIVE);
        }

        bool bIsParent = meBorder == FRAMEBORDER_NONE;
        if(mpFrameSel->HasFocus() &&
            (bIsParent || mpFrameSel->IsBorderSelected(meBorder)))
        {
            pStateSetHelper->AddState(AccessibleStateType::ACTIVE);
            pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
            pStateSetHelper->AddState(AccessibleStateType::SELECTED);
        }
    }
    return xRet;
}

Locale AccFrameSelector::getLocale(  )
    throw (IllegalAccessibleComponentStateException, RuntimeException, std::exception)
{
    return Application::GetSettings().GetUILanguageTag().getLocale();
}

sal_Bool AccFrameSelector::containsPoint( const css::awt::Point& aPt )
    throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->ContainsClickPoint( Point( aPt.X, aPt.Y ) );
}

Reference< XAccessible > AccFrameSelector::getAccessibleAtPoint(
    const css::awt::Point& aPt )
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

css::awt::Rectangle AccFrameSelector::getBounds(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Size aSz;
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aSz = mpFrameSel->GetSizePixel();
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
            aSz = aSpot.GetSize();
    }
    css::awt::Rectangle aRet;
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}



css::awt::Point AccFrameSelector::getLocation(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    css::awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}



css::awt::Point AccFrameSelector::getLocationOnScreen(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    aPos = mpFrameSel->OutputToAbsoluteScreenPixel( aPos );
    css::awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}



css::awt::Size AccFrameSelector::getSize(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    Size aSz;
    switch(meBorder)
    {
        case FRAMEBORDER_NONE:
            aSz = mpFrameSel->GetSizePixel();
        break;
        default:
            const Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aSz = aSpot.GetSize();
    }
    css::awt::Size aRet(aSz.Width(), aSz.Height());
    return aRet;
}

void AccFrameSelector::grabFocus(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    mpFrameSel->GrabFocus();
}

sal_Int32 AccFrameSelector::getForeground(  )
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlForeground().GetColor();
}

sal_Int32 AccFrameSelector::getBackground(  )
        throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlBackground().GetColor();
}

void AccFrameSelector::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( xListener.is() )
    {
        if ( !mnClientId )
        {
            mnClientId = ::comphelper::AccessibleEventNotifier::registerClient();
        }
        ::comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void AccFrameSelector::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( xListener.is() && mnClientId != 0 &&
         ::comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener ) == 0 )
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        ::comphelper::AccessibleEventNotifier::TClientId nId( mnClientId );
        mnClientId = 0;
        ::comphelper::AccessibleEventNotifier::revokeClient( nId );
    }
}

OUString AccFrameSelector::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("AccFrameSelector");
}

sal_Bool AccFrameSelector::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > AccFrameSelector::getSupportedServiceNames(  )
    throw (RuntimeException, std::exception)
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = "Accessible";
    pArray[1] = "AccessibleContext";
    pArray[2] = "AccessibleComponent";
    return aRet;
}

void AccFrameSelector::IsValid() throw (RuntimeException)
{
    if(!mpFrameSel)
        throw RuntimeException();
}

void AccFrameSelector::NotifyFocusListeners(bool bGetFocus)
{
    SolarMutexGuard aGuard;
    css::awt::FocusEvent aEvent;
    aEvent.FocusFlags = 0;
    if(bGetFocus)
    {
        GetFocusFlags nFocusFlags = mpFrameSel->GetGetFocusFlags();
        if(nFocusFlags & GetFocusFlags::Tab)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::TAB;
        if(nFocusFlags & GetFocusFlags::CURSOR)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::CURSOR;
        if(nFocusFlags & GetFocusFlags::Mnemonic)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::MNEMONIC;
        if(nFocusFlags & GetFocusFlags::Forward)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::FORWARD;
        if(nFocusFlags & GetFocusFlags::Backward)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::BACKWARD;
        if(nFocusFlags & GetFocusFlags::Around)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::AROUND;
        if(nFocusFlags & GetFocusFlags::UniqueMnemonic)
            aEvent.FocusFlags |= css::awt::FocusChangeReason::UNIQUEMNEMONIC;
    }
    aEvent.Temporary = sal_False;

    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;

    ::cppu::OInterfaceIteratorHelper aIter( maFocusListeners );
    while( aIter.hasMoreElements() )
    {
        Reference < XFocusListener > xListener( aIter.next(), UNO_QUERY );
        if(bGetFocus)
            xListener->focusGained( aEvent );
        else
            xListener->focusLost( aEvent );
    }
}



IMPL_LINK_TYPED( AccFrameSelector, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    vcl::Window* pWindow = rEvent.GetWindow();
    DBG_ASSERT( pWindow, "AccFrameSelector::WindowEventListener: no window!" );
    if ( !pWindow->IsAccessibilityEventsSuppressed() || ( rEvent.GetId() == VCLEVENT_OBJECT_DYING ) )
    {
        ProcessWindowEvent( rEvent );
    }
}



void AccFrameSelector::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_GETFOCUS:
        {
            if ( meBorder == FRAMEBORDER_NONE )
            {
                Any aOldValue, aNewValue;
                aNewValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            if ( meBorder == FRAMEBORDER_NONE )
            {
                Any aOldValue, aNewValue;
                aOldValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        default:
        {
        }
        break;
    }
}



void AccFrameSelector::NotifyAccessibleEvent( const sal_Int16 _nEventId,
    const Any& _rOldValue, const Any& _rNewValue )
{
    if ( mnClientId )
    {
        Reference< XInterface > xSource( *this );
        AccessibleEventObject aEvent( xSource, _nEventId, _rNewValue, _rOldValue );
        ::comphelper::AccessibleEventNotifier::addEvent( mnClientId, aEvent );
    }
}



void AccFrameSelector::Invalidate()
{
    RemoveFrameSelEventListener();
    mpFrameSel = 0;
    EventObject aEvent;
    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;
    maFocusListeners.disposeAndClear( aEvent );
    maPropertyListeners.disposeAndClear( aEvent );
}



}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
