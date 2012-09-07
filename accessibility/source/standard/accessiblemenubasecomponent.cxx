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

#include <accessibility/standard/accessiblemenubasecomponent.hxx>
#include <accessibility/standard/vclxaccessiblemenu.hxx>
#include <accessibility/standard/vclxaccessiblemenuitem.hxx>
#include <accessibility/standard/vclxaccessiblemenuseparator.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// OAccessibleMenuBaseComponent
// -----------------------------------------------------------------------------

OAccessibleMenuBaseComponent::OAccessibleMenuBaseComponent( Menu* pMenu )
    :AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    ,m_pMenu( pMenu )
    ,m_bEnabled( sal_False )
    ,m_bFocused( sal_False )
    ,m_bVisible( sal_False )
    ,m_bSelected( sal_False )
    ,m_bChecked( sal_False )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    if ( m_pMenu )
    {
        m_aAccessibleChildren.assign( m_pMenu->GetItemCount(), Reference< XAccessible >() );
        m_pMenu->AddEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );
    }
}

// -----------------------------------------------------------------------------

OAccessibleMenuBaseComponent::~OAccessibleMenuBaseComponent()
{
    if ( m_pMenu )
        m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

    delete m_pExternalLock;
    m_pExternalLock = NULL;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsEnabled()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsFocused()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsVisible()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsSelected()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsChecked()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetStates()
{
    m_bEnabled = IsEnabled();
    m_bFocused = IsFocused();
    m_bVisible = IsVisible();
    m_bSelected = IsSelected();
    m_bChecked = IsChecked();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetEnabled( sal_Bool bEnabled )
{
    if ( m_bEnabled != bEnabled )
    {
        Any aOldValue[2], aNewValue[2];
        if ( m_bEnabled )
        {
            aOldValue[0] <<= AccessibleStateType::SENSITIVE;
            aOldValue[1] <<= AccessibleStateType::ENABLED;
        }
        else
        {
            aNewValue[0] <<= AccessibleStateType::ENABLED;
            aNewValue[1] <<= AccessibleStateType::SENSITIVE;
        }
        m_bEnabled = bEnabled;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[0], aNewValue[0] );
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[1], aNewValue[1] );
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetFocused( sal_Bool bFocused )
{
    if ( m_bFocused != bFocused )
    {
        Any aOldValue, aNewValue;
        if ( m_bFocused )
            aOldValue <<= AccessibleStateType::FOCUSED;
        else
            aNewValue <<= AccessibleStateType::FOCUSED;
        m_bFocused = bFocused;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetVisible( sal_Bool bVisible )
{
    if ( m_bVisible != bVisible )
    {
        Any aOldValue, aNewValue;
        if ( m_bVisible )
            aOldValue <<= AccessibleStateType::VISIBLE;
        else
            aNewValue <<= AccessibleStateType::VISIBLE;
        m_bVisible = bVisible;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetSelected( sal_Bool bSelected )
{
    if ( m_bSelected != bSelected )
    {
        Any aOldValue, aNewValue;
        if ( m_bSelected )
            aOldValue <<= AccessibleStateType::SELECTED;
        else
            aNewValue <<= AccessibleStateType::SELECTED;
        m_bSelected = bSelected;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SetChecked( sal_Bool bChecked )
{
    if ( m_bChecked != bChecked )
    {
        Any aOldValue, aNewValue;
        if ( m_bChecked )
            aOldValue <<= AccessibleStateType::CHECKED;
        else
            aNewValue <<= AccessibleStateType::CHECKED;
        m_bChecked = bChecked;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateEnabled( sal_Int32 i, sal_Bool bEnabled )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetEnabled( bEnabled );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateFocused( sal_Int32 i, sal_Bool bFocused )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetFocused( bFocused );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateVisible()
{
    SetVisible( IsVisible() );
    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetVisible( pComp->IsVisible() );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateSelected( sal_Int32 i, sal_Bool bSelected )
{
    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetSelected( bSelected );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateChecked( sal_Int32 i, sal_Bool bChecked )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetChecked( bChecked );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateAccessibleName( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuItemComponent* pComp = static_cast< OAccessibleMenuItemComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetAccessibleName( pComp->GetAccessibleName() );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::UpdateItemText( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuItemComponent* pComp = static_cast< OAccessibleMenuItemComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetItemText( pComp->GetItemText() );
        }
    }
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuBaseComponent::GetChildCount()
{
    return m_aAccessibleChildren.size();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuBaseComponent::GetChild( sal_Int32 i )
{
    Reference< XAccessible > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        if ( m_pMenu )
        {
            // create a new child
            OAccessibleMenuBaseComponent* pChild;

            if ( m_pMenu->GetItemType( (sal_uInt16)i ) == MENUITEM_SEPARATOR )
            {
                pChild = new VCLXAccessibleMenuSeparator( m_pMenu, (sal_uInt16)i );
            }
            else
            {
                PopupMenu* pPopupMenu = m_pMenu->GetPopupMenu( m_pMenu->GetItemId( (sal_uInt16)i ) );
                if ( pPopupMenu )
                {
                    pChild = new VCLXAccessibleMenu( m_pMenu, (sal_uInt16)i, pPopupMenu );
                    pPopupMenu->SetAccessible( pChild );
                }
                else
                {
                    pChild = new VCLXAccessibleMenuItem( m_pMenu, (sal_uInt16)i );
                }
            }

            // set states
            pChild->SetStates();

            xChild = pChild;

            // insert into menu item list
            m_aAccessibleChildren[i] = xChild;
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuBaseComponent::GetChildAt( const awt::Point& rPoint )
{
    Reference< XAccessible > xChild;
    for ( sal_uInt32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
    {
        Reference< XAccessible > xAcc = getAccessibleChild( i );
        if ( xAcc.is() )
        {
            Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
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

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::InsertChild( sal_Int32 i )
{
    if ( i > (sal_Int32)m_aAccessibleChildren.size() )
        i = m_aAccessibleChildren.size();

    if ( i >= 0 )
    {
        // insert entry in child list
        m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + i, Reference< XAccessible >() );

        // update item position of accessible children
        for ( sal_uInt32 j = i, nCount = m_aAccessibleChildren.size(); j < nCount; ++j )
        {
            Reference< XAccessible > xAcc( m_aAccessibleChildren[j] );
            if ( xAcc.is() )
            {
                OAccessibleMenuItemComponent* pComp = static_cast< OAccessibleMenuItemComponent* >( xAcc.get() );
                if ( pComp )
                    pComp->SetItemPos( (sal_uInt16)j );
            }
        }

        // send accessible child event
        Reference< XAccessible > xChild( GetChild( i ) );
        if ( xChild.is() )
        {
            Any aOldValue, aNewValue;
            aNewValue <<= xChild;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::RemoveChild( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        // keep the accessible of the removed item
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );

        // remove entry in child list
        m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

        // update item position of accessible children
        for ( sal_uInt32 j = i, nCount = m_aAccessibleChildren.size(); j < nCount; ++j )
        {
            Reference< XAccessible > xAcc( m_aAccessibleChildren[j] );
            if ( xAcc.is() )
            {
                OAccessibleMenuItemComponent* pComp = static_cast< OAccessibleMenuItemComponent* >( xAcc.get() );
                if ( pComp )
                    pComp->SetItemPos( (sal_uInt16)j );
            }
        }

        // send accessible child event
        if ( xChild.is() )
        {
            Any aOldValue, aNewValue;
            aOldValue <<= xChild;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );

            Reference< XComponent > xComponent( xChild, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
    }
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsHighlighted()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsChildHighlighted()
{
    sal_Bool bChildHighlighted = sal_False;

    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp && pComp->IsHighlighted() )
            {
                bChildHighlighted = sal_True;
                break;
            }
        }
    }

    return bChildHighlighted;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::SelectChild( sal_Int32 i )
{
    // open the menu
    if ( getAccessibleRole() == AccessibleRole::MENU && !IsPopupMenuOpen() )
        Click();

    // highlight the child
    if ( m_pMenu )
        m_pMenu->HighlightItem( (sal_uInt16)i );
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::DeSelectAll()
{
    if ( m_pMenu )
        m_pMenu->DeHighlight();
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsChildSelected( sal_Int32 i )
{
    sal_Bool bSelected = sal_False;

    if ( m_pMenu && m_pMenu->IsHighlighted( (sal_uInt16)i ) )
        bSelected = sal_True;

    return bSelected;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::Select()
{
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::DeSelect()
{
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::Click()
{
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::IsPopupMenuOpen()
{
    return sal_False;
}

// -----------------------------------------------------------------------------

IMPL_LINK( OAccessibleMenuBaseComponent, MenuEventListener, VclSimpleEvent*, pEvent )
{
    OSL_ENSURE( pEvent && pEvent->ISA( VclMenuEvent ), "OAccessibleMenuBaseComponent - Unknown MenuEvent!" );
    if ( pEvent && pEvent->ISA( VclMenuEvent ) )
    {
        OSL_ENSURE( ((VclMenuEvent*)pEvent)->GetMenu(), "OAccessibleMenuBaseComponent - Menu?" );
        ProcessMenuEvent( *(VclMenuEvent*)pEvent );
    }
    return 0;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::ProcessMenuEvent( const VclMenuEvent& rVclMenuEvent )
{
    sal_uInt16 nItemPos = rVclMenuEvent.GetItemPos();

    switch ( rVclMenuEvent.GetId() )
    {
        case VCLEVENT_MENU_SHOW:
        case VCLEVENT_MENU_HIDE:
        {
            UpdateVisible();
        }
        break;
        case VCLEVENT_MENU_HIGHLIGHT:
        {
            SetFocused( sal_False );
            UpdateFocused( nItemPos, sal_True );
            UpdateSelected( nItemPos, sal_True );
        }
        break;
        case VCLEVENT_MENU_DEHIGHLIGHT:
        {
            UpdateFocused( nItemPos, sal_False );
            UpdateSelected( nItemPos, sal_False );
        }
        break;
        case VCLEVENT_MENU_SUBMENUACTIVATE:
        {
        }
        break;
        case VCLEVENT_MENU_SUBMENUDEACTIVATE:
        {
            UpdateFocused( nItemPos, sal_True );
        }
        break;
        case VCLEVENT_MENU_ENABLE:
        {
            UpdateEnabled( nItemPos, sal_True );
        }
        break;
        case VCLEVENT_MENU_DISABLE:
        {
            UpdateEnabled( nItemPos, sal_False );
        }
        break;
        case VCLEVENT_MENU_SUBMENUCHANGED:
        {
            RemoveChild( nItemPos );
            InsertChild( nItemPos );
        }
        break;
        case VCLEVENT_MENU_INSERTITEM:
        {
            InsertChild( nItemPos );
        }
        break;
        case VCLEVENT_MENU_REMOVEITEM:
        {
            RemoveChild( nItemPos );
        }
        break;
        case VCLEVENT_MENU_ACCESSIBLENAMECHANGED:
        {
            UpdateAccessibleName( nItemPos );
        }
        break;
        case VCLEVENT_MENU_ITEMTEXTCHANGED:
        {
            UpdateAccessibleName( nItemPos );
            UpdateItemText( nItemPos );
        }
        break;
        case VCLEVENT_MENU_ITEMCHECKED:
        {
            UpdateChecked( nItemPos, sal_True );
        }
        break;
        case VCLEVENT_MENU_ITEMUNCHECKED:
        {
            UpdateChecked( nItemPos, sal_False );
        }
        break;
        case VCLEVENT_OBJECT_DYING:
        {
            if ( m_pMenu )
            {
                m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

                m_pMenu = NULL;

                // dispose all menu items
                for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
                {
                    Reference< XComponent > xComponent( m_aAccessibleChildren[i], UNO_QUERY );
                    if ( xComponent.is() )
                        xComponent->dispose();
                }
                m_aAccessibleChildren.clear();
            }
        }
        break;
        default:
        {
        }
        break;
    }
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleMenuBaseComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleMenuBaseComponent_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleMenuBaseComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleMenuBaseComponent_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void OAccessibleMenuBaseComponent::disposing()
{
    AccessibleExtendedComponentHelper_BASE::disposing();

    if ( m_pMenu )
    {
        m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

        m_pMenu = NULL;

        // dispose all menu items
        for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
        {
            Reference< XComponent > xComponent( m_aAccessibleChildren[i], UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuBaseComponent::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< OUString > aNames( getSupportedServiceNames() );
    const OUString* pNames = aNames.getConstArray();
    const OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > OAccessibleMenuBaseComponent::getAccessibleContext(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > OAccessibleMenuBaseComponent::getAccessibleStateSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xSet = pStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        FillAccessibleStateSet( *pStateSetHelper );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }

    return xSet;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
