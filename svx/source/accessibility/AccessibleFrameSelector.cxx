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

#include "AccessibleFrameSelector.hxx"
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svx/frmsel.hxx>
#include <svx/dialmgr.hxx>
#include "editeng/unolingu.hxx"

#include <svx/dialogs.hrc>
#include "frmsel.hrc"

#ifndef MNEMONIC_CHAR
#define MNEMONIC_CHAR ((sal_Unicode)'~')
#endif

namespace svx {
namespace a11y {

using ::rtl::OUString;
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

namespace AwtKey                    = ::com::sun::star::awt::Key;
namespace AwtKeyModifier            = ::com::sun::star::awt::KeyModifier;
namespace AwtFocusChangeReason      = ::com::sun::star::awt::FocusChangeReason;

typedef ::com::sun::star::awt::Point        AwtPoint;
typedef ::com::sun::star::awt::Size         AwtSize;
typedef ::com::sun::star::awt::Rectangle    AwtRectangle;
typedef ::com::sun::star::awt::KeyEvent     AwtKeyEvent;
typedef ::com::sun::star::awt::FocusEvent   AwtFocusEvent;

// ============================================================================

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

// ----------------------------------------------------------------------------

AccFrameSelector::~AccFrameSelector()
{
    RemoveFrameSelEventListener();
}

// ----------------------------------------------------------------------------

void AccFrameSelector::RemoveFrameSelEventListener()
{
    if ( mpFrameSel )
    {
        mpFrameSel->RemoveEventListener( LINK( this, AccFrameSelector, WindowEventListener ) );
    }
}

// ----------------------------------------------------------------------------

Reference< XAccessibleContext > AccFrameSelector::getAccessibleContext(  )
    throw (RuntimeException)
{
    return this;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getAccessibleChildCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return (meBorder == FRAMEBORDER_NONE) ? mpFrameSel->GetEnabledBorderCount() : 0;
}

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleChild( sal_Int32 i )
    throw (RuntimeException)
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

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleParent(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    Reference< XAccessible > xRet;
    if(meBorder == FRAMEBORDER_NONE)
        xRet = mpFrameSel->GetParent()->GetAccessible( sal_True );
    else
        xRet = mpFrameSel->CreateAccessible();
    return xRet;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getAccessibleIndexInParent(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();

    sal_Int32 nIdx = 0;
    if( meBorder == FRAMEBORDER_NONE )
    {
        Window* pTabPage = mpFrameSel->GetParent();
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

// ----------------------------------------------------------------------------

sal_Int16 AccFrameSelector::getAccessibleRole(  ) throw (RuntimeException)
{
    return AccessibleRole::OPTION_PANE;
}

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getAccessibleDescription(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return maDescriptions.GetString(meBorder);
}

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getAccessibleName(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return maNames.GetString(meBorder);
}

// ----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > AccFrameSelector::getAccessibleRelationSet(  )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FRAMEBORDER_NONE)
    {
        //add the label relation
        Window* pPrev = mpFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            AccessibleRelation aLabelRelation;
            aLabelRelation.RelationType = AccessibleRelationType::LABELED_BY;
            aLabelRelation.TargetSet.realloc(1);
            aLabelRelation.TargetSet.getArray()[0]  = pPrev->GetAccessible();
            pHelper->AddRelation(aLabelRelation);
        }
    }
    return xRet;
}

// ----------------------------------------------------------------------------

Reference< XAccessibleStateSet > AccFrameSelector::getAccessibleStateSet(  )
    throw (RuntimeException)
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

        sal_Bool bIsParent = meBorder == FRAMEBORDER_NONE;
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

// ----------------------------------------------------------------------------

Locale AccFrameSelector::getLocale(  )
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    Locale aRet;
    SvxLanguageToLocale( aRet, Application::GetSettings().GetUILanguage() );
    return aRet;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::addPropertyChangeListener(
    const Reference< XPropertyChangeListener >& xListener )
        throw (RuntimeException)
{
    maPropertyListeners.addInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::removePropertyChangeListener( const Reference< XPropertyChangeListener >& xListener )
    throw (RuntimeException)
{
    maPropertyListeners.removeInterface( xListener );
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::containsPoint( const AwtPoint& aPt )
    throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->ContainsClickPoint( Point( aPt.X, aPt.Y ) );
}

// ----------------------------------------------------------------------------

Reference< XAccessible > AccFrameSelector::getAccessibleAtPoint(
    const AwtPoint& aPt )
        throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    //aPt is relative to the frame selector
    return mpFrameSel->GetChildAccessible( Point( aPt.X, aPt.Y ) );
}

AwtRectangle AccFrameSelector::getBounds(  ) throw (RuntimeException)
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
    AwtRectangle aRet;
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}

// ----------------------------------------------------------------------------

AwtPoint AccFrameSelector::getLocation(  ) throw (RuntimeException)
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
    AwtPoint aRet(aPos.X(), aPos.Y());
    return aRet;
}

// ----------------------------------------------------------------------------

AwtPoint AccFrameSelector::getLocationOnScreen(  ) throw (RuntimeException)
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
    AwtPoint aRet(aPos.X(), aPos.Y());
    return aRet;
}

// ----------------------------------------------------------------------------

AwtSize AccFrameSelector::getSize(  ) throw (RuntimeException)
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
    AwtSize aRet(aSz.Width(), aSz.Height());
    return aRet;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isShowing(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isVisible(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

sal_Bool AccFrameSelector::isFocusTraversable(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    return sal_True;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::addFocusListener( const Reference< XFocusListener >& xListener ) throw (RuntimeException)
{
    maFocusListeners.addInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::removeFocusListener( const Reference< XFocusListener >& xListener ) throw (RuntimeException)
{
    maFocusListeners.removeInterface( xListener );
}

// ----------------------------------------------------------------------------

void AccFrameSelector::grabFocus(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    IsValid();
    mpFrameSel->GrabFocus();
}

// ----------------------------------------------------------------------------

Any AccFrameSelector::getAccessibleKeyBinding(  ) throw (RuntimeException)
{
    Any aRet;
    SolarMutexGuard aGuard;
    IsValid();
    utl::AccessibleRelationSetHelper* pHelper;
    Reference< XAccessibleRelationSet > xRet = pHelper = new utl::AccessibleRelationSetHelper;
    if(meBorder == FRAMEBORDER_NONE)
    {
        Window* pPrev = mpFrameSel->GetWindow( WINDOW_PREV );
        if(pPrev && WINDOW_FIXEDTEXT == pPrev->GetType())
        {
            String sText = pPrev->GetText();
            xub_StrLen nFound = sText.Search( MNEMONIC_CHAR );
            if(STRING_NOTFOUND != nFound && ++nFound < sText.Len())
            {
                sText.ToUpperAscii();
                sal_Unicode cChar = sText.GetChar(nFound);
                AwtKeyEvent aEvent;

                aEvent.KeyCode = 0;
                aEvent.KeyChar = cChar;
                aEvent.KeyFunc = 0;
                if(cChar >= 'A' && cChar <= 'Z')
                {
                     aEvent.KeyCode = AwtKey::A + cChar - 'A';
                }
                aEvent.Modifiers = AwtKeyModifier::MOD2;
                aRet <<= aEvent;
            }
        }
    }
    return aRet;
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getForeground(  )
        throw (RuntimeException)
{
    Any aRet;
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlForeground().GetColor();
}

// ----------------------------------------------------------------------------

sal_Int32 AccFrameSelector::getBackground(  )
        throw (RuntimeException)
{
    Any aRet;
    SolarMutexGuard aGuard;
    IsValid();
    return mpFrameSel->GetControlBackground().GetColor();
}

// ----------------------------------------------------------------------------

void AccFrameSelector::addEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
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

// ----------------------------------------------------------------------------

void AccFrameSelector::removeEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
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

// ----------------------------------------------------------------------------

OUString AccFrameSelector::getImplementationName(  ) throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccFrameSelector"));
}

// ----------------------------------------------------------------------------

const sal_Char sAccessible[]          = "Accessible";
const sal_Char sAccessibleContext[]   = "AccessibleContext";
const sal_Char sAccessibleComponent[] = "AccessibleComponent";

sal_Bool AccFrameSelector::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return  rServiceName.equalsAsciiL( sAccessible         , sizeof(sAccessible         )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleContext  , sizeof(sAccessibleContext  )-1 ) ||
            rServiceName.equalsAsciiL( sAccessibleComponent, sizeof(sAccessibleComponent)-1 );
}

// ----------------------------------------------------------------------------

Sequence< OUString > AccFrameSelector::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessible         ) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleContext  ) );
    pArray[2] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleComponent) );
    return aRet;
}

// ----------------------------------------------------------------------------

void AccFrameSelector::IsValid() throw (RuntimeException)
{
    if(!mpFrameSel)
        throw RuntimeException();
}

// ----------------------------------------------------------------------------

void    AccFrameSelector::NotifyFocusListeners(sal_Bool bGetFocus)
{
    SolarMutexGuard aGuard;
    AwtFocusEvent aEvent;
    aEvent.FocusFlags = 0;
    if(bGetFocus)
    {
        sal_uInt16 nFocusFlags = mpFrameSel->GetGetFocusFlags();
        if(nFocusFlags&GETFOCUS_TAB)
            aEvent.FocusFlags |= AwtFocusChangeReason::TAB;
        if(nFocusFlags&GETFOCUS_CURSOR)
            aEvent.FocusFlags |= AwtFocusChangeReason::CURSOR;
        if(nFocusFlags&GETFOCUS_MNEMONIC)
            aEvent.FocusFlags |= AwtFocusChangeReason::MNEMONIC;
        if(nFocusFlags&GETFOCUS_FORWARD)
            aEvent.FocusFlags |= AwtFocusChangeReason::FORWARD;
        if(nFocusFlags&GETFOCUS_BACKWARD)
            aEvent.FocusFlags |= AwtFocusChangeReason::BACKWARD;
        if(nFocusFlags&GETFOCUS_AROUND)
            aEvent.FocusFlags |= AwtFocusChangeReason::AROUND;
        if(nFocusFlags&GETFOCUS_UNIQUEMNEMONIC)
            aEvent.FocusFlags |= AwtFocusChangeReason::UNIQUEMNEMONIC;
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

// ----------------------------------------------------------------------------

IMPL_LINK( AccFrameSelector, WindowEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent* pWinEvent = dynamic_cast< VclWindowEvent* >( pEvent );
    DBG_ASSERT( pWinEvent, "AccFrameSelector::WindowEventListener - unknown window event" );
    if ( pWinEvent )
    {
        Window* pWindow = pWinEvent->GetWindow();
        DBG_ASSERT( pWindow, "AccFrameSelector::WindowEventListener: no window!" );
        if ( !pWindow->IsAccessibilityEventsSuppressed() || ( pWinEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *pWinEvent );
        }
    }

    return 0;
}

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

// ============================================================================

} // namespace a11y
} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
