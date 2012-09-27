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

#include <accessibility/standard/accessiblemenuitemcomponent.hxx>


#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>

#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/unohelp2.hxx>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class OAccessibleMenuItemComponent
// -----------------------------------------------------------------------------

OAccessibleMenuItemComponent::OAccessibleMenuItemComponent( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu )
    :OAccessibleMenuBaseComponent( pMenu )
    ,m_pParent( pParent )
    ,m_nItemPos( nItemPos )
{
    m_sAccessibleName = GetAccessibleName();
    m_sItemText = GetItemText();
}

// -----------------------------------------------------------------------------

OAccessibleMenuItemComponent::~OAccessibleMenuItemComponent()
{
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuItemComponent::IsEnabled()
{
    OExternalLockGuard aGuard( this );

    sal_Bool bEnabled = sal_False;
    if ( m_pParent )
        bEnabled = m_pParent->IsItemEnabled( m_pParent->GetItemId( m_nItemPos ) );

    return bEnabled;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuItemComponent::IsVisible()
{
    sal_Bool bVisible = sal_False;

    if ( m_pParent )
        bVisible = m_pParent->IsItemPosVisible( m_nItemPos );

    return bVisible;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::Select()
{
    // open the parent menu
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    // highlight the menu item
    if ( m_pParent )
        m_pParent->HighlightItem( m_nItemPos );
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::DeSelect()
{
    if ( m_pParent && IsSelected() )
        m_pParent->DeHighlight();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::Click()
{
    // open the parent menu
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    // click the menu item
    if ( m_pParent )
    {
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            // #102438# Menu items are not selectable
            // Popup menus are executed asynchronously, triggered by a timer.
            // As Menu::SelectItem only works, if the corresponding menu window is
            // already created, we have to set the menu delay to 0, so
            // that the popup menus are executed synchronously.
            AllSettings aSettings = pWindow->GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();
            sal_uLong nDelay = aMouseSettings.GetMenuDelay();
            aMouseSettings.SetMenuDelay( 0 );
            aSettings.SetMouseSettings( aMouseSettings );
            pWindow->SetSettings( aSettings );

            m_pParent->SelectItem( m_pParent->GetItemId( m_nItemPos ) );

            // meanwhile the window pointer may be invalid
            pWindow = m_pParent->GetWindow();
            if ( pWindow )
            {
                // set the menu delay back to the old value
                aSettings = pWindow->GetSettings();
                aMouseSettings = aSettings.GetMouseSettings();
                aMouseSettings.SetMenuDelay( nDelay );
                aSettings.SetMouseSettings( aMouseSettings );
                pWindow->SetSettings( aSettings );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetItemPos( sal_uInt16 nItemPos )
{
    m_nItemPos = nItemPos;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetAccessibleName( const OUString& sAccessibleName )
{
    if ( !m_sAccessibleName.equals( sAccessibleName ) )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= m_sAccessibleName;
        aNewValue <<= sAccessibleName;
        m_sAccessibleName = sAccessibleName;
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::GetAccessibleName()
{
    OUString sName;
    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        sName = m_pParent->GetAccessibleName( nItemId );
        if ( sName.isEmpty() )
            sName = m_pParent->GetItemText( nItemId );
        sName = OutputDevice::GetNonMnemonicString( sName );
    }

    return sName;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetItemText( const OUString& sItemText )
{
    Any aOldValue, aNewValue;
    if ( OCommonAccessibleText::implInitTextChangedEvent( m_sItemText, sItemText, aOldValue, aNewValue ) )
    {
        m_sItemText = sItemText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::GetItemText()
{
    OUString sText;
    if ( m_pParent )
        sText = OutputDevice::GetNonMnemonicString( m_pParent->GetItemText( m_pParent->GetItemId( m_nItemPos ) ) );

    return sText;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    if ( IsEnabled() )
    {
        rStateSet.AddState( AccessibleStateType::ENABLED );
        rStateSet.AddState( AccessibleStateType::SENSITIVE );
    }

    if ( IsVisible() )
    {
        rStateSet.AddState( AccessibleStateType::VISIBLE );
        rStateSet.AddState( AccessibleStateType::SHOWING );
    }

    rStateSet.AddState( AccessibleStateType::OPAQUE );
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle OAccessibleMenuItemComponent::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pParent )
    {
        // get bounding rectangle of the item relative to the containing window
        aBounds = AWTRectangle( m_pParent->GetBoundingRectangle( m_nItemPos ) );

        // get position of containing window in screen coordinates
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
            awt::Point aWindowScreenLoc = AWTPoint( aRect.TopLeft() );

            // get position of accessible parent in screen coordinates
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();

                    // calculate bounding rectangle of the item relative to the accessible parent
                    aBounds.X += aWindowScreenLoc.X - aParentScreenLoc.X;
                    aBounds.Y += aWindowScreenLoc.Y - aParentScreenLoc.Y;
                }
            }
        }
    }

    return aBounds;
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void SAL_CALL OAccessibleMenuItemComponent::disposing()
{
    OAccessibleMenuBaseComponent::disposing();

    m_pParent = NULL;
    m_sAccessibleName = OUString();
    m_sItemText = OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_pParent->GetAccessible();
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_nItemPos;
}

// -----------------------------------------------------------------------------

sal_Int16 OAccessibleMenuItemComponent::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::UNKNOWN;
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::getAccessibleDescription( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pParent )
        sDescription = m_pParent->GetHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sDescription;
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_sAccessibleName;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > OAccessibleMenuItemComponent::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Locale OAccessibleMenuItemComponent::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::grabFocus(  ) throw (RuntimeException)
{
    // no focus for items
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getForeground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getBackground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > OAccessibleMenuItemComponent::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleExtendedComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            xFont = xParentComp->getFont();
    }

    return xFont;
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}

// -----------------------------------------------------------------------------

OUString OAccessibleMenuItemComponent::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( m_pParent )
        sRet = m_pParent->GetTipHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sRet;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
