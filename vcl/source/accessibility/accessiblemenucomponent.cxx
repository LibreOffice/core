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

#include <accessibility/accessiblemenucomponent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/unohelp.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;




bool OAccessibleMenuComponent::IsEnabled()
{
    return true;
}


bool OAccessibleMenuComponent::IsVisible()
{
    bool bVisible = false;

    if ( m_pMenu )
        bVisible = m_pMenu->IsMenuVisible();

    return bVisible;
}


void OAccessibleMenuComponent::FillAccessibleStateSet( sal_Int64& rStateSet )
{
    if ( IsEnabled() )
    {
        rStateSet |= AccessibleStateType::ENABLED;
        rStateSet |= AccessibleStateType::SENSITIVE;
    }

    rStateSet |= AccessibleStateType::FOCUSABLE;

    if ( IsFocused() )
        rStateSet |= AccessibleStateType::FOCUSED;

    if ( IsVisible() )
    {
        rStateSet |= AccessibleStateType::VISIBLE;
        rStateSet |= AccessibleStateType::SHOWING;
    }

    rStateSet |= AccessibleStateType::OPAQUE;
}


// OCommonAccessibleComponent


awt::Rectangle OAccessibleMenuComponent::implGetBounds()
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            // get bounding rectangle of the window in screen coordinates
            AbsoluteScreenPixelRectangle aRect = pWindow->GetWindowExtentsAbsolute();
            aBounds = vcl::unohelper::ConvertToAWTRect(aRect);

            // get position of the accessible parent in screen coordinates
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();

                    // calculate position of the window relative to the accessible parent
                    aBounds.X -= aParentScreenLoc.X;
                    aBounds.Y -= aParentScreenLoc.Y;
                }
            }
        }
    }

    return aBounds;
}


// XAccessibleContext


sal_Int64 OAccessibleMenuComponent::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return GetChildCount();
}


Reference< XAccessible > OAccessibleMenuComponent::getAccessibleChild( sal_Int64 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= GetChildCount() )
        throw IndexOutOfBoundsException();

    return GetChild( i );
}


Reference< XAccessible > OAccessibleMenuComponent::getAccessibleParent(  )
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;

    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
            xParent = pWindow->GetAccessibleParent();
    }

    return xParent;
}


sal_Int16 OAccessibleMenuComponent::getAccessibleRole(  )
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::UNKNOWN;
}


OUString OAccessibleMenuComponent::getAccessibleDescription( )
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
            sDescription = pWindow->GetAccessibleDescription();
    }

    return sDescription;
}


OUString OAccessibleMenuComponent::getAccessibleName(  )
{
    OExternalLockGuard aGuard( this );

    return OUString();
}


Reference< XAccessibleRelationSet > OAccessibleMenuComponent::getAccessibleRelationSet(  )
{
    OExternalLockGuard aGuard( this );

    return new utl::AccessibleRelationSetHelper;
}


Locale OAccessibleMenuComponent::getLocale(  )
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}


// XAccessibleComponent


Reference< XAccessible > OAccessibleMenuComponent::getAccessibleAtPoint( const awt::Point& rPoint )
{
    OExternalLockGuard aGuard( this );

    return GetChildAt( rPoint );
}


awt::Point OAccessibleMenuComponent::getLocationOnScreen(  )
{
    OExternalLockGuard aGuard( this );

    awt::Point aPos;

    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            AbsoluteScreenPixelRectangle aRect = pWindow->GetWindowExtentsAbsolute();
            aPos = vcl::unohelper::ConvertToAWTPoint(aRect.TopLeft());
        }
    }

    return aPos;
}


void OAccessibleMenuComponent::grabFocus(  )
{
    OExternalLockGuard aGuard( this );

    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
            pWindow->GrabFocus();
    }
}


sal_Int32 OAccessibleMenuComponent::getForeground(  )
{
    OExternalLockGuard aGuard( this );

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    Color nColor = rStyleSettings.GetMenuTextColor();

    return sal_Int32(nColor);
}


sal_Int32 OAccessibleMenuComponent::getBackground(  )
{
    OExternalLockGuard aGuard( this );

    return 0;
}


// XAccessibleExtendedComponent

OUString OAccessibleMenuComponent::getTitledBorderText(  )
{
    OExternalLockGuard aGuard( this );

    return OUString();
}


OUString OAccessibleMenuComponent::getToolTipText(  )
{
    OExternalLockGuard aGuard( this );

    return OUString();
}


// XAccessibleSelection


void OAccessibleMenuComponent::selectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    SelectChild( nChildIndex );
}


sal_Bool OAccessibleMenuComponent::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    return IsChildSelected( nChildIndex );
}


void OAccessibleMenuComponent::clearAccessibleSelection(  )
{
    OExternalLockGuard aGuard( this );

    DeSelectAll();
}


void OAccessibleMenuComponent::selectAllAccessibleChildren(  )
{
    // This method makes no sense in a menu, and so does nothing.
}


sal_Int64 OAccessibleMenuComponent::getSelectedAccessibleChildCount(  )
{
    OExternalLockGuard aGuard( this );

    sal_Int64 nRet = 0;

    for ( sal_Int64 i = 0, nCount = GetChildCount(); i < nCount; i++ )
    {
        if ( IsChildSelected( i ) )
            ++nRet;
    }

    return nRet;
}


Reference< XAccessible > OAccessibleMenuComponent::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;

    for ( sal_Int64 i = 0, j = 0, nCount = GetChildCount(); i < nCount; i++ )
    {
        if ( IsChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
        {
            xChild = GetChild( i );
            break;
        }
    }

    return xChild;
}


void OAccessibleMenuComponent::deselectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    DeSelectAll();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
