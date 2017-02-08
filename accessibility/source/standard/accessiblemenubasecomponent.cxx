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

#include <standard/accessiblemenubasecomponent.hxx>
#include <standard/vclxaccessiblemenu.hxx>
#include <standard/vclxaccessiblemenuitem.hxx>
#include <standard/vclxaccessiblemenuseparator.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// OAccessibleMenuBaseComponent


OAccessibleMenuBaseComponent::OAccessibleMenuBaseComponent( Menu* pMenu )
    :OAccessibleExtendedComponentHelper( new VCLExternalSolarLock() )
    ,m_pMenu( pMenu )
    ,m_bEnabled( false )
    ,m_bFocused( false )
    ,m_bVisible( false )
    ,m_bSelected( false )
    ,m_bChecked( false )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    if ( m_pMenu )
    {
        m_aAccessibleChildren.assign( m_pMenu->GetItemCount(), Reference< XAccessible >() );
        m_pMenu->AddEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );
    }
}


OAccessibleMenuBaseComponent::~OAccessibleMenuBaseComponent()
{
    if ( m_pMenu )
        m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

    delete m_pExternalLock;
    m_pExternalLock = nullptr;
}


bool OAccessibleMenuBaseComponent::IsEnabled()
{
    return false;
}


bool OAccessibleMenuBaseComponent::IsFocused()
{
    return false;
}


bool OAccessibleMenuBaseComponent::IsVisible()
{
    return false;
}


bool OAccessibleMenuBaseComponent::IsSelected()
{
    return false;
}


bool OAccessibleMenuBaseComponent::IsChecked()
{
    return false;
}


void OAccessibleMenuBaseComponent::SetStates()
{
    m_bEnabled = IsEnabled();
    m_bFocused = IsFocused();
    m_bVisible = IsVisible();
    m_bSelected = IsSelected();
    m_bChecked = IsChecked();
}


void OAccessibleMenuBaseComponent::SetEnabled( bool bEnabled )
{
    if ( m_bEnabled != bEnabled )
    {
        sal_Int16 nStateType=AccessibleStateType::ENABLED;
        if (IsMenuHideDisabledEntries())
        {
            nStateType = AccessibleStateType::VISIBLE;
        }
        Any aOldValue[2], aNewValue[2];
        if ( m_bEnabled )
        {
            aOldValue[0] <<= AccessibleStateType::SENSITIVE;
            aOldValue[1] <<= nStateType;
        }
        else
        {
            aNewValue[0] <<= nStateType;
            aNewValue[1] <<= AccessibleStateType::SENSITIVE;
        }
        m_bEnabled = bEnabled;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[0], aNewValue[0] );
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[1], aNewValue[1] );
    }
}


void OAccessibleMenuBaseComponent::SetFocused( bool bFocused )
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


void OAccessibleMenuBaseComponent::SetVisible( bool bVisible )
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


void OAccessibleMenuBaseComponent::SetSelected( bool bSelected )
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


void OAccessibleMenuBaseComponent::SetChecked( bool bChecked )
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


void OAccessibleMenuBaseComponent::UpdateEnabled( sal_Int32 i, bool bEnabled )
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


void OAccessibleMenuBaseComponent::UpdateFocused( sal_Int32 i, bool bFocused )
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


void OAccessibleMenuBaseComponent::UpdateVisible()
{
    SetVisible( IsVisible() );
    for (Reference<XAccessible>& xChild : m_aAccessibleChildren)
    {
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp )
                pComp->SetVisible( pComp->IsVisible() );
        }
    }
}


void OAccessibleMenuBaseComponent::UpdateSelected( sal_Int32 i, bool bSelected )
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


void OAccessibleMenuBaseComponent::UpdateChecked( sal_Int32 i, bool bChecked )
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


sal_Int32 OAccessibleMenuBaseComponent::GetChildCount()
{
    return m_aAccessibleChildren.size();
}


Reference< XAccessible > OAccessibleMenuBaseComponent::GetChild( sal_Int32 i )
{
    Reference< XAccessible > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        if ( m_pMenu )
        {
            // create a new child
            OAccessibleMenuBaseComponent* pChild;

            if ( m_pMenu->GetItemType( (sal_uInt16)i ) == MenuItemType::SEPARATOR )
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


bool OAccessibleMenuBaseComponent::IsHighlighted()
{
    return false;
}


bool OAccessibleMenuBaseComponent::IsChildHighlighted()
{
    bool bChildHighlighted = false;

    for (Reference<XAccessible>& xChild : m_aAccessibleChildren)
    {
        if ( xChild.is() )
        {
            OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xChild.get() );
            if ( pComp && pComp->IsHighlighted() )
            {
                bChildHighlighted = true;
                break;
            }
        }
    }

    return bChildHighlighted;
}


void OAccessibleMenuBaseComponent::SelectChild( sal_Int32 i )
{
    // open the menu
    if ( getAccessibleRole() == AccessibleRole::MENU && !IsPopupMenuOpen() )
        Click();

    // highlight the child
    if ( m_pMenu )
        m_pMenu->HighlightItem( (sal_uInt16)i );
}


void OAccessibleMenuBaseComponent::DeSelectAll()
{
    if ( m_pMenu )
        m_pMenu->DeHighlight();
}


bool OAccessibleMenuBaseComponent::IsChildSelected( sal_Int32 i )
{
    bool bSelected = false;

    if ( m_pMenu && m_pMenu->IsHighlighted( (sal_uInt16)i ) )
        bSelected = true;

    return bSelected;
}


void OAccessibleMenuBaseComponent::Select()
{
}


void OAccessibleMenuBaseComponent::DeSelect()
{
}


void OAccessibleMenuBaseComponent::Click()
{
}


bool OAccessibleMenuBaseComponent::IsPopupMenuOpen()
{
    return false;
}


IMPL_LINK( OAccessibleMenuBaseComponent, MenuEventListener, VclMenuEvent&, rEvent, void )
{
    OSL_ENSURE( rEvent.GetMenu(), "OAccessibleMenuBaseComponent - Menu?" );
    ProcessMenuEvent( rEvent );
}


void OAccessibleMenuBaseComponent::ProcessMenuEvent( const VclMenuEvent& rVclMenuEvent )
{
    sal_uInt16 nItemPos = rVclMenuEvent.GetItemPos();

    switch ( rVclMenuEvent.GetId() )
    {
        case VclEventId::MenuShow:
        case VclEventId::MenuHide:
        {
            UpdateVisible();
        }
        break;
        case VclEventId::MenuHighlight:
        {
            SetFocused( false );
            UpdateFocused( nItemPos, true );
            UpdateSelected( nItemPos, true );
        }
        break;
        case VclEventId::MenuDehighlight:
        {
            UpdateFocused( nItemPos, false );
            UpdateSelected( nItemPos, false );
        }
        break;
        case VclEventId::MenuSubmenuActivate:
        {
        }
        break;
        case VclEventId::MenuSubmenuDeactivate:
        {
            UpdateFocused( nItemPos, true );
        }
        break;
        case VclEventId::MenuEnable:
        {
            UpdateEnabled( nItemPos, true );
        }
        break;
        case VclEventId::MenuDisable:
        {
            UpdateEnabled( nItemPos, false );
        }
        break;
        case VclEventId::MenuSubmenuChanged:
        {
            RemoveChild( nItemPos );
            InsertChild( nItemPos );
        }
        break;
        case VclEventId::MenuInsertItem:
        {
            InsertChild( nItemPos );
        }
        break;
        case VclEventId::MenuRemoveItem:
        {
            RemoveChild( nItemPos );
        }
        break;
        case VclEventId::MenuItemTextChanged:
        {
            UpdateAccessibleName( nItemPos );
            UpdateItemText( nItemPos );
        }
        break;
        case VclEventId::MenuItemChecked:
        {
            UpdateChecked( nItemPos, true );
        }
        break;
        case VclEventId::MenuItemUnchecked:
        {
            UpdateChecked( nItemPos, false );
        }
        break;
        case VclEventId::ObjectDying:
        {
            if ( m_pMenu )
            {
                m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

                m_pMenu = nullptr;

                // dispose all menu items
                for (Reference<XAccessible>& i : m_aAccessibleChildren)
                {
                    Reference< XComponent > xComponent( i, UNO_QUERY );
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


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleMenuBaseComponent, OAccessibleExtendedComponentHelper, OAccessibleMenuBaseComponent_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleMenuBaseComponent, OAccessibleExtendedComponentHelper, OAccessibleMenuBaseComponent_BASE )


// XComponent


void OAccessibleMenuBaseComponent::disposing()
{
    OAccessibleExtendedComponentHelper::disposing();

    if ( m_pMenu )
    {
        m_pMenu->RemoveEventListener( LINK( this, OAccessibleMenuBaseComponent, MenuEventListener ) );

        m_pMenu = nullptr;

        // dispose all menu items
        for (Reference<XAccessible>& i : m_aAccessibleChildren)
        {
            Reference< XComponent > xComponent( i, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        m_aAccessibleChildren.clear();
    }
}


// XServiceInfo


sal_Bool OAccessibleMenuBaseComponent::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}


// XAccessible


Reference< XAccessibleContext > OAccessibleMenuBaseComponent::getAccessibleContext(  )
{
    OExternalLockGuard aGuard( this );

    return this;
}


// XAccessibleContext


Reference< XAccessibleStateSet > OAccessibleMenuBaseComponent::getAccessibleStateSet(  )
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


bool OAccessibleMenuBaseComponent::IsMenuHideDisabledEntries()
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
