/*************************************************************************
 *
 *  $RCSfile: vclxaccessiblecomponent.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: tbe $ $Date: 2002-03-25 10:31:02 $
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


#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_KEYEVENT_HPP_
#include <com/sun/star/awt/KeyEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_KEY_HPP_
#include <com/sun/star/awt/Key.hpp>
#endif

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/window.hxx>
#include <tools/debug.hxx>

#include <unotools/accessiblestatesethelper.hxx>

#ifndef MNEMONIC_CHAR
#define MNEMONIC_CHAR               ((sal_Unicode)'~')
#endif


using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;


//  ----------------------------------------------------
//  class VCLXAccessibleComponent
//  ----------------------------------------------------
VCLXAccessibleComponent::VCLXAccessibleComponent( VCLXWindow* pVCLXindow )
    : maEventListeners( GetMutex() ), VCLXAccessibleComponentBase( GetMutex() )
{
    mpVCLXindow = pVCLXindow;
    mxWindow = pVCLXindow;

   DBG_ASSERT( pVCLXindow->GetWindow(), "VCLXAccessibleComponent - no window!" );
   if ( pVCLXindow->GetWindow() )
      pVCLXindow->GetWindow()->AddEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
}

VCLXAccessibleComponent::~VCLXAccessibleComponent()
{
   if ( mpVCLXindow && mpVCLXindow->GetWindow() )
      mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
}

IMPL_LINK( VCLXAccessibleComponent, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        ProcessWindowEvent( *(VclWindowEvent*)pEvent );
    }
    return 0;
}

void VCLXAccessibleComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    uno::Any aOldValue, aNewValue;

    Window* pWindow = rVclWindowEvent.GetWindow();
    DBG_ASSERT( pWindow, "VCLXAccessibleComponent::ProcessWindowEvent - Window?" );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:
        {
            aNewValue <<= accessibility::AccessibleStateType::VISIBLE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aNewValue <<= accessibility::AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aNewValue.clear();
            aOldValue <<= accessibility::AccessibleStateType::INVALID;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            aOldValue <<= accessibility::AccessibleStateType::VISIBLE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aOldValue <<= accessibility::AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aOldValue.clear();
            aNewValue <<= accessibility::AccessibleStateType::INVALID;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            aNewValue <<= accessibility::AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            aOldValue <<= accessibility::AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_CONTROL_GETFOCUS:
        {
            aNewValue <<= accessibility::AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
        case VCLEVENT_CONTROL_LOSEFOCUS:
        {
            aOldValue <<= accessibility::AccessibleStateType::FOCUSED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;

        // MT: Missing event for enabled !!!
    }
}

void VCLXAccessibleComponent::disposing()
{
   if ( mpVCLXindow && mpVCLXindow->GetWindow() )
      mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );

    NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, uno::Any(), uno::Any() );

    lang::EventObject aEvt( *this );
    maEventListeners.disposeAndClear( aEvt );

    mxWindow.clear();
    mpVCLXindow = NULL;
}

Window* VCLXAccessibleComponent::GetWindow() const
{
    return GetVCLXWindow() ? GetVCLXWindow()->GetWindow() : NULL;
}

void VCLXAccessibleComponent::NotifyAccessibleEvent( sal_Int16 nEventId, const uno::Any& rOldValue, const uno::Any& rNewValue )
{
    accessibility::AccessibleEventObject aEvt;
    aEvt.Source = *this;
    aEvt.EventId = nEventId;
    aEvt.OldValue = rOldValue;
    aEvt.NewValue = rNewValue;

    ::cppu::OInterfaceIteratorHelper aIter( maEventListeners );
    while ( aIter.hasMoreElements() )
    {
        uno::Reference< accessibility::XAccessibleEventListener > xEL( aIter.next(), uno::UNO_QUERY );
        if ( xEL.is() )
            xEL->notifyEvent( aEvt );
    }
}

// accessibility::XAccessibleEventBroadcaster
void SAL_CALL VCLXAccessibleComponent::addEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener ) throw (uno::RuntimeException)
{
    if ( rxListener.is() )
        maEventListeners.addInterface( rxListener );
}

void SAL_CALL VCLXAccessibleComponent::removeEventListener( const uno::Reference< accessibility::XAccessibleEventListener >& rxListener ) throw (uno::RuntimeException)
{
    if ( rxListener.is() )
        maEventListeners.removeInterface( rxListener );
}

// accessibility::XAccessible
uno::Reference< accessibility::XAccessibleContext > VCLXAccessibleComponent::getAccessibleContext(  ) throw (uno::RuntimeException)
{
    uno::Reference< accessibility::XAccessibleContext > xAcc( (::cppu::OWeakObject*)this, uno::UNO_QUERY );
    return xAcc;
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
            rStateSet.AddState( accessibility::AccessibleStateType::ENABLED );

        if ( pWindow->HasChildPathFocus() )
            rStateSet.AddState( accessibility::AccessibleStateType::ACTIVE );

        if ( pWindow->HasFocus() )
            rStateSet.AddState( accessibility::AccessibleStateType::FOCUSED );

        if ( pWindow->IsWait() )
            rStateSet.AddState( accessibility::AccessibleStateType::BUSY );

        if ( pWindow->GetStyle() & WB_SIZEABLE )
            rStateSet.AddState( accessibility::AccessibleStateType::RESIZABLE );
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
MODAL
MULTILINE
MULTISELECTABLE
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
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nChildren = 0;
    if ( GetWindow() )
        nChildren = GetWindow()->GetAccessibleChildWindowCount();

    return nChildren;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    if ( i >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();

    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pChild = GetWindow()->GetAccessibleChildWindow( (USHORT)i );
        if ( pChild )
            xAcc = pChild->GetAccessible();
    }

    return xAcc;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleParent(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
            xAcc = pParent->GetAccessible();
    }

    return xAcc;
}

sal_Int32 VCLXAccessibleComponent::getAccessibleIndexInParent(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nIndex = 0;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
        {
            for ( USHORT n = pParent->GetAccessibleChildWindowCount(); n; )
            {
                Window* pChild = pParent->GetAccessibleChildWindow( --n );
                if ( pChild == GetWindow() )
                {
                    nIndex = n;
                    break;
                }
            }
        }
    }
    return nIndex;
}

sal_Int16 VCLXAccessibleComponent::getAccessibleRole(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int16 nRole = 0;

    if ( GetWindow() )
        nRole = GetWindow()->GetAccessibleRole();

    return nRole;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleDescription(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aDescription;

    if ( GetWindow() )
        aDescription = GetWindow()->GetAccessibleDescription();

    return aDescription;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleName(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString aName;
    if ( GetWindow() )
    {
        aName = GetWindow()->GetAccessibleName();
#ifdef DEBUG
        aName += String( RTL_CONSTASCII_USTRINGPARAM( " (Type = " ) );
        aName += String::CreateFromInt32( GetWindow()->GetType() );
        aName += String( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
#endif
    }
    return aName;
}

uno::Reference< accessibility::XAccessibleRelationSet > VCLXAccessibleComponent::getAccessibleRelationSet(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );
    return NULL;
}

uno::Reference< accessibility::XAccessibleStateSet > VCLXAccessibleComponent::getAccessibleStateSet(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< accessibility::XAccessibleStateSet > xSet = pStateSetHelper;
    FillAccessibleStateSet( *pStateSetHelper );
    return xSet;
//    return NULL;
}

lang::Locale VCLXAccessibleComponent::getLocale() throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    lang::Locale aLocale;
    if ( GetWindow() )
        aLocale = GetWindow()->GetSettings().GetUILocale();

    return aLocale;
}

// accessibility::XAccessibleComponent
sal_Bool VCLXAccessibleComponent::contains( const awt::Point& rPoint ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bInside = sal_False;
    if ( GetWindow() )
    {
        Rectangle aRect( GetWindow()->GetPosPixel(), GetWindow()->GetSizePixel() );
        bInside = aRect.IsInside( VCLPoint( rPoint ) );
    }

    return bInside;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleAt( const awt::Point& rPoint ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Reference< accessibility::XAccessible > xAcc;

    if ( GetWindow() )
    {
        Window* pWindow = GetWindow()->FindWindow( VCLPoint( rPoint ) );
        if ( pWindow && pWindow != GetWindow() )
            xAcc = pWindow->GetAccessible();

    }
    return xAcc;
}

awt::Size VCLXAccessibleComponent::getSize() throw (uno::RuntimeException)
{
    awt::Size aSize;
    if ( GetWindow() )
        aSize = AWTSize( GetWindow()->GetSizePixel() );
    return aSize;

}

awt::Rectangle VCLXAccessibleComponent::getBounds() throw (uno::RuntimeException)
{
    awt::Rectangle aBounds;
    if ( mxWindow.is() )
        aBounds = mxWindow->getPosSize();
    return aBounds;

}

awt::Point VCLXAccessibleComponent::getLocation() throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    awt::Point aPos;
    if ( GetWindow() )
    {
        Point aVclPos = GetWindow()->GetPosPixel();
        aPos.X = aVclPos.X();
        aPos.Y = aVclPos.Y();
    }

    return aPos;
}

awt::Point VCLXAccessibleComponent::getLocationOnScreen(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    awt::Point aPos;
    if ( GetWindow() )
    {
        Rectangle aRect = GetWindow()->GetWindowExtentsRelative( NULL );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }

    return aPos;
}

sal_Bool VCLXAccessibleComponent::isShowing() throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bShowing = sal_False;
    if ( GetWindow() && GetWindow()->IsVisible() )
        bShowing = GetWindow()->IsReallyVisible();  // I hope IsReallyVisible is doing everything I need?

    return bShowing;
}

sal_Bool VCLXAccessibleComponent::isVisible() throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bVisible = sal_False;
    if ( GetWindow() )
        bVisible = GetWindow()->IsVisible();

    return bVisible;
}

sal_Bool VCLXAccessibleComponent::isFocusTraversable() throw (uno::RuntimeException)
{
    return FALSE;
}

void VCLXAccessibleComponent::addFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() )
        mxWindow->addFocusListener( xListener );
}

void VCLXAccessibleComponent::removeFocusListener( const uno::Reference< awt::XFocusListener >& xListener ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() )
        mxWindow->removeFocusListener( xListener );
}

void VCLXAccessibleComponent::grabFocus(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    if ( mxWindow.is() && isFocusTraversable() )
        mxWindow->setFocus();
}

uno::Any VCLXAccessibleComponent::getAccessibleKeyBinding() throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    uno::Any aRet;

    if ( GetWindow() )
    {
        String sText = GetWindow()->GetText();
        xub_StrLen nFound = sText.Search( MNEMONIC_CHAR );

        if ( STRING_NOTFOUND != nFound && ++nFound < sText.Len() )
        {
            sText.ToUpperAscii();
            sal_Unicode cChar = sText.GetChar( nFound );

            if ( ( cChar >= '0' && cChar <= '9' ) || ( cChar >= 'A' && cChar <= 'Z' ) )
            {
                awt::KeyEvent aEvent;

                if ( cChar >= '0' && cChar <= '9' )
                    aEvent.KeyCode = awt::Key::NUM0 + cChar - '0';
                else if ( cChar >= 'A' && cChar <= 'Z' )
                    aEvent.KeyCode = awt::Key::A + cChar - 'A';

                aEvent.KeyChar = cChar;
                aEvent.KeyFunc = 0;
                aEvent.Modifiers = awt::KeyModifier::MOD2;
                aRet <<= aEvent;
            }
        }
    }

    return aRet;
}

// XAccessibleExtendedComponent

sal_Int32 SAL_CALL VCLXAccessibleComponent::getForeground(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nColor = 0;
    if ( GetWindow() )
       nColor = GetWindow()->GetControlForeground().GetColor();

    return nColor;
}

sal_Int32 SAL_CALL VCLXAccessibleComponent::getBackground(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Int32 nColor = 0;
    if ( GetWindow() )
       nColor = GetWindow()->GetControlBackground().GetColor();

    return nColor;
}

uno::Reference< awt::XFont > SAL_CALL VCLXAccessibleComponent::getFont(  ) throw (uno::RuntimeException)
{
    return uno::Reference< awt::XFont >();
}

awt::FontDescriptor SAL_CALL VCLXAccessibleComponent::getFontMetrics( const uno::Reference< awt::XFont >& xFont ) throw (uno::RuntimeException)
{
    return xFont->getFontDescriptor();
}

sal_Bool SAL_CALL VCLXAccessibleComponent::isEnabled(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_Bool bEnabled = sal_False;
    if ( GetWindow() )
        bEnabled = GetWindow()->IsEnabled();

    return bEnabled;
}

::rtl::OUString SAL_CALL VCLXAccessibleComponent::getTitledBorderText(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetText();

    return sRet;
}

::rtl::OUString SAL_CALL VCLXAccessibleComponent::getToolTipText(  ) throw (uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::rtl::OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetQuickHelpText();

    return sRet;
}

