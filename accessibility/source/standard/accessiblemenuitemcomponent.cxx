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
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;






OAccessibleMenuItemComponent::OAccessibleMenuItemComponent( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu )
    :OAccessibleMenuBaseComponent( pMenu )
    ,m_pParent( pParent )
    ,m_nItemPos( nItemPos )
{
    m_sAccessibleName = GetAccessibleName();
    m_sItemText = GetItemText();
}



OAccessibleMenuItemComponent::~OAccessibleMenuItemComponent()
{
}



sal_Bool OAccessibleMenuItemComponent::IsEnabled()
{
    OExternalLockGuard aGuard( this );

    sal_Bool bEnabled = sal_False;
    if ( m_pParent )
        bEnabled = m_pParent->IsItemEnabled( m_pParent->GetItemId( m_nItemPos ) );

    return bEnabled;
}



sal_Bool OAccessibleMenuItemComponent::IsVisible()
{
    sal_Bool bVisible = sal_False;

    if ( m_pParent )
        bVisible = m_pParent->IsItemPosVisible( m_nItemPos );

    return bVisible;
}



void OAccessibleMenuItemComponent::Select()
{
    
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    
    if ( m_pParent )
        m_pParent->HighlightItem( m_nItemPos );
}



void OAccessibleMenuItemComponent::DeSelect()
{
    if ( m_pParent && IsSelected() )
        m_pParent->DeHighlight();
}



void OAccessibleMenuItemComponent::Click()
{
    
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    
    if ( m_pParent )
    {
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            
            
            
            
            
            AllSettings aSettings = pWindow->GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();
            sal_uLong nDelay = aMouseSettings.GetMenuDelay();
            aMouseSettings.SetMenuDelay( 0 );
            aSettings.SetMouseSettings( aMouseSettings );
            pWindow->SetSettings( aSettings );

            m_pParent->SelectItem( m_pParent->GetItemId( m_nItemPos ) );

            
            pWindow = m_pParent->GetWindow();
            if ( pWindow )
            {
                
                aSettings = pWindow->GetSettings();
                aMouseSettings = aSettings.GetMouseSettings();
                aMouseSettings.SetMenuDelay( nDelay );
                aSettings.SetMouseSettings( aMouseSettings );
                pWindow->SetSettings( aSettings );
            }
        }
    }
}



void OAccessibleMenuItemComponent::SetItemPos( sal_uInt16 nItemPos )
{
    m_nItemPos = nItemPos;
}



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



void OAccessibleMenuItemComponent::SetItemText( const OUString& sItemText )
{
    Any aOldValue, aNewValue;
    if ( OCommonAccessibleText::implInitTextChangedEvent( m_sItemText, sItemText, aOldValue, aNewValue ) )
    {
        m_sItemText = sItemText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}



OUString OAccessibleMenuItemComponent::GetItemText()
{
    OUString sText;
    if ( m_pParent )
        sText = OutputDevice::GetNonMnemonicString( m_pParent->GetItemText( m_pParent->GetItemId( m_nItemPos ) ) );

    return sText;
}



void OAccessibleMenuItemComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    sal_Bool bEnabled = IsEnabled();
    if ( bEnabled )
    {
        rStateSet.AddState( AccessibleStateType::ENABLED );
        rStateSet.AddState( AccessibleStateType::SENSITIVE );
    }

    if ( IsVisible() )
    {
        rStateSet.AddState( AccessibleStateType::SHOWING );
        if( !IsMenuHideDisabledEntries() || bEnabled )
            rStateSet.AddState( AccessibleStateType::VISIBLE );
    }
    rStateSet.AddState( AccessibleStateType::OPAQUE );
}





awt::Rectangle OAccessibleMenuItemComponent::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pParent )
    {
        
        aBounds = AWTRectangle( m_pParent->GetBoundingRectangle( m_nItemPos ) );

        
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
            awt::Point aWindowScreenLoc = AWTPoint( aRect.TopLeft() );

            
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();

                    
                    aBounds.X += aWindowScreenLoc.X - aParentScreenLoc.X;
                    aBounds.Y += aWindowScreenLoc.Y - aParentScreenLoc.Y;
                }
            }
        }
    }

    return aBounds;
}





void SAL_CALL OAccessibleMenuItemComponent::disposing()
{
    OAccessibleMenuBaseComponent::disposing();

    m_pParent = NULL;
    m_sAccessibleName = OUString();
    m_sItemText = OUString();
}





sal_Int32 OAccessibleMenuItemComponent::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}



Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}



Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_pParent->GetAccessible();
}



sal_Int32 OAccessibleMenuItemComponent::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_nItemPos;
}



sal_Int16 OAccessibleMenuItemComponent::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::UNKNOWN;
}



OUString OAccessibleMenuItemComponent::getAccessibleDescription( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pParent )
        sDescription = m_pParent->GetHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sDescription;
}



OUString OAccessibleMenuItemComponent::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_sAccessibleName;
}



Reference< XAccessibleRelationSet > OAccessibleMenuItemComponent::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}



Locale OAccessibleMenuItemComponent::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}





Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}



void OAccessibleMenuItemComponent::grabFocus(  ) throw (RuntimeException)
{
    
}



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



OUString OAccessibleMenuItemComponent::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}



OUString OAccessibleMenuItemComponent::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( m_pParent )
        sRet = m_pParent->GetTipHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sRet;
}



sal_Bool OAccessibleMenuItemComponent::IsMenuHideDisabledEntries()
{
    if (m_pParent )
    {
        if( m_pParent->GetMenuFlags() & MENU_FLAG_HIDEDISABLEDENTRIES)
        {
            return sal_True;
        }
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
