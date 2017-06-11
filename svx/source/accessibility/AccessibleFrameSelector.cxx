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
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/frmsel.hxx>
#include <svx/dialmgr.hxx>
#include "editeng/unolingu.hxx"

#include <svx/strings.hrc>
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
using ::com::sun::star::awt::XFocusListener;

using namespace ::com::sun::star::accessibility;


AccFrameSelector::AccFrameSelector( FrameSelector& rFrameSel, FrameBorderType eBorder ) :
    mpFrameSel( &rFrameSel ),
    meBorder( eBorder ),
    maFocusListeners( maFocusMutex ),
    maPropertyListeners( maPropertyMutex ),
    mnClientId( 0 )
{
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
{
    return this;
}


sal_Int32 AccFrameSelector::getAccessibleChildCount(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return (meBorder == FrameBorderType::NONE) ? mpFrameSel->GetEnabledBorderCount() : 0;
}

Reference< XAccessible > AccFrameSelector::getAccessibleChild( sal_Int32 i )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet;
    if( meBorder == FrameBorderType::NONE )
        xRet = mpFrameSel->GetChildAccessible( i );
    if( !xRet.is() )
        throw RuntimeException();
    return xRet;
}

Reference< XAccessible > AccFrameSelector::getAccessibleParent(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet;
    if(meBorder == FrameBorderType::NONE)
        xRet = mpFrameSel->GetParent()->GetAccessible();
    else
        xRet = mpFrameSel->CreateAccessible();
    return xRet;
}

sal_Int32 AccFrameSelector::getAccessibleIndexInParent(  )
{
    SolarMutexGuard aGuard;
    IsValid();

    sal_Int32 nIdx = 0;
    if( meBorder == FrameBorderType::NONE )
    {
        vcl::Window* pTabPage = mpFrameSel->GetParent();
        if (!pTabPage)
            return nIdx;
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

sal_Int16 AccFrameSelector::getAccessibleRole(  )
{
    return meBorder == FrameBorderType::NONE ? AccessibleRole::OPTION_PANE : AccessibleRole::CHECK_BOX;
}

OUString AccFrameSelector::getAccessibleDescription(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_DESCRIPTIONS[(sal_uInt32)meBorder].first);
}

OUString AccFrameSelector::getAccessibleName(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return SvxResId(RID_SVXSTR_FRMSEL_TEXTS[(sal_uInt32)meBorder].first);
}

Reference< XAccessibleRelationSet > AccFrameSelector::getAccessibleRelationSet(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FrameBorderType::NONE)
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

        bool bIsParent = meBorder == FrameBorderType::NONE;
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
{
    return Application::GetSettings().GetUILanguageTag().getLocale();
}

sal_Bool AccFrameSelector::containsPoint( const css::awt::Point& aPt )
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->ContainsClickPoint( Point( aPt.X, aPt.Y ) );
}

Reference< XAccessible > AccFrameSelector::getAccessibleAtPoint(
    const css::awt::Point& aPt )
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

css::awt::Rectangle AccFrameSelector::getBounds(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Size aSz;
    Point aPos;
    switch(meBorder)
    {
        case FrameBorderType::NONE:
            aSz = mpFrameSel->GetSizePixel();
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const tools::Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
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


css::awt::Point AccFrameSelector::getLocation(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FrameBorderType::NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const tools::Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    css::awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}


css::awt::Point AccFrameSelector::getLocationOnScreen(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Point aPos;
    switch(meBorder)
    {
        case FrameBorderType::NONE:
            aPos = mpFrameSel->GetPosPixel();
        break;
        default:
            const tools::Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aPos = aSpot.TopLeft();
    }
    aPos = mpFrameSel->OutputToAbsoluteScreenPixel( aPos );
    css::awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}


css::awt::Size AccFrameSelector::getSize(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    Size aSz;
    switch(meBorder)
    {
        case FrameBorderType::NONE:
            aSz = mpFrameSel->GetSizePixel();
        break;
        default:
            const tools::Rectangle aSpot = mpFrameSel->GetClickBoundRect( meBorder );
            aSz = aSpot.GetSize();
    }
    css::awt::Size aRet(aSz.Width(), aSz.Height());
    return aRet;
}

void AccFrameSelector::grabFocus(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    mpFrameSel->GrabFocus();
}

sal_Int32 AccFrameSelector::getForeground(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlForeground().GetColor();
}

sal_Int32 AccFrameSelector::getBackground(  )
{
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlBackground().GetColor();
}

void AccFrameSelector::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
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

void AccFrameSelector::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
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

OUString AccFrameSelector::getImplementationName(  )
{
    return OUString("AccFrameSelector");
}

sal_Bool AccFrameSelector::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > AccFrameSelector::getSupportedServiceNames(  )
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = "Accessible";
    pArray[1] = "AccessibleContext";
    pArray[2] = "AccessibleComponent";
    return aRet;
}

void AccFrameSelector::IsValid()
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
    aEvent.Temporary = false;

    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;

    ::comphelper::OInterfaceIteratorHelper2 aIter( maFocusListeners );
    while( aIter.hasMoreElements() )
    {
        Reference < XFocusListener > xListener( aIter.next(), UNO_QUERY );
        if(bGetFocus)
            xListener->focusGained( aEvent );
        else
            xListener->focusLost( aEvent );
    }
}


IMPL_LINK( AccFrameSelector, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    vcl::Window* pWindow = rEvent.GetWindow();
    DBG_ASSERT( pWindow, "AccFrameSelector::WindowEventListener: no window!" );
    if ( pWindow->IsAccessibilityEventsSuppressed() && ( rEvent.GetId() != VclEventId::ObjectDying ) )
        return;

    switch ( rEvent.GetId() )
    {
        case VclEventId::WindowGetFocus:
        {
            if ( meBorder == FrameBorderType::NONE )
            {
                Any aOldValue, aNewValue;
                aNewValue <<= AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
            }
        }
        break;
        case VclEventId::WindowLoseFocus:
        {
            if ( meBorder == FrameBorderType::NONE )
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
    mpFrameSel = nullptr;
    EventObject aEvent;
    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;
    maFocusListeners.disposeAndClear( aEvent );
    maPropertyListeners.disposeAndClear( aEvent );
}


}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
