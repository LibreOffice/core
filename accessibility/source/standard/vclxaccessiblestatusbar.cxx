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

#include <standard/vclxaccessiblestatusbar.hxx>
#include <standard/vclxaccessiblestatusbaritem.hxx>
#include <toolkit/helper/convert.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/status.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  class VCLXAccessibleStatusBar


VCLXAccessibleStatusBar::VCLXAccessibleStatusBar( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pStatusBar = static_cast< StatusBar *>( GetWindow().get() );

    if ( m_pStatusBar )
        m_aAccessibleChildren.assign( m_pStatusBar->GetItemCount(), Reference< XAccessible >() );
}


VCLXAccessibleStatusBar::~VCLXAccessibleStatusBar()
{
}


void VCLXAccessibleStatusBar::UpdateShowing( sal_Int32 i, bool bShowing )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleStatusBarItem* pVCLXAccessibleStatusBarItem = static_cast< VCLXAccessibleStatusBarItem* >( xChild.get() );
            if ( pVCLXAccessibleStatusBarItem )
                pVCLXAccessibleStatusBarItem->SetShowing( bShowing );
        }
    }
}


void VCLXAccessibleStatusBar::UpdateItemName( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleStatusBarItem* pVCLXAccessibleStatusBarItem = static_cast< VCLXAccessibleStatusBarItem* >( xChild.get() );
            if ( pVCLXAccessibleStatusBarItem )
            {
                OUString sItemName = pVCLXAccessibleStatusBarItem->GetItemName();
                pVCLXAccessibleStatusBarItem->SetItemName( sItemName );
            }
        }
    }
}


void VCLXAccessibleStatusBar::UpdateItemText( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleStatusBarItem* pVCLXAccessibleStatusBarItem = static_cast< VCLXAccessibleStatusBarItem* >( xChild.get() );
            if ( pVCLXAccessibleStatusBarItem )
            {
                OUString sItemText = pVCLXAccessibleStatusBarItem->GetItemText();
                pVCLXAccessibleStatusBarItem->SetItemText( sItemText );
            }
        }
    }
}


void VCLXAccessibleStatusBar::InsertChild( sal_Int32 i )
{
    if ( i >= 0 && i <= (sal_Int32)m_aAccessibleChildren.size() )
    {
        // insert entry in child list
        m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + i, Reference< XAccessible >() );

        // send accessible child event
        Reference< XAccessible > xChild( getAccessibleChild( i ) );
        if ( xChild.is() )
        {
            Any aOldValue, aNewValue;
            aNewValue <<= xChild;
            NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
        }
    }
}


void VCLXAccessibleStatusBar::RemoveChild( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        // get the accessible of the removed page
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );

        // remove entry in child list
        m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

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


void VCLXAccessibleStatusBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::StatusbarItemAdded:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                InsertChild( nItemPos );
            }
        }
        break;
        case VclEventId::StatusbarItemRemoved:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
                {
                    Reference< XAccessible > xChild( getAccessibleChild( i ) );
                    if ( xChild.is() )
                    {
                        VCLXAccessibleStatusBarItem* pVCLXAccessibleStatusBarItem = static_cast< VCLXAccessibleStatusBarItem* >( xChild.get() );
                        if ( pVCLXAccessibleStatusBarItem && pVCLXAccessibleStatusBarItem->GetItemId() == nItemId )
                        {
                            RemoveChild( i );
                            break;
                        }
                    }
                }
            }
        }
        break;
        case VclEventId::StatusbarAllItemsRemoved:
        {
            for ( sal_Int32 i = m_aAccessibleChildren.size() - 1; i >= 0; --i )
                RemoveChild( i );
        }
        break;
        case VclEventId::StatusbarShowItem:
        case VclEventId::StatusbarHideItem:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateShowing( nItemPos, rVclWindowEvent.GetId() == VclEventId::StatusbarShowItem );
            }
        }
        break;
        case VclEventId::StatusbarShowAllItems:
        {
            for ( size_t i = 0; i < m_aAccessibleChildren.size(); ++i )
                UpdateShowing( i, rVclWindowEvent.GetId() == VclEventId::StatusbarShowAllItems );
        }
        break;
        case VclEventId::StatusbarNameChanged:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateItemName( nItemPos );
            }
        }
        break;
        case VclEventId::StatusbarDrawItem:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateItemText( nItemPos );
            }
        }
        break;
        case VclEventId::ObjectDying:
        {
            if ( m_pStatusBar )
            {
                m_pStatusBar = nullptr;

                // dispose all children
                for (Reference<XAccessible>& i : m_aAccessibleChildren)
                {
                    Reference< XComponent > xComponent( i, UNO_QUERY );
                    if ( xComponent.is() )
                        xComponent->dispose();
                }
                m_aAccessibleChildren.clear();
            }

            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}


// XComponent


void VCLXAccessibleStatusBar::disposing()
{
    VCLXAccessibleComponent::disposing();

    if ( m_pStatusBar )
    {
        m_pStatusBar = nullptr;

        // dispose all children
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


OUString VCLXAccessibleStatusBar::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleStatusBar" );
}


Sequence< OUString > VCLXAccessibleStatusBar::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleStatusBar" };
}


// XAccessibleContext


sal_Int32 VCLXAccessibleStatusBar::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}


Reference< XAccessible > VCLXAccessibleStatusBar::getAccessibleChild( sal_Int32 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        if ( m_pStatusBar )
        {
            sal_uInt16 nItemId = m_pStatusBar->GetItemId( (sal_uInt16)i );

            xChild = new VCLXAccessibleStatusBarItem( m_pStatusBar, nItemId );

            // insert into status bar item list
            m_aAccessibleChildren[i] = xChild;
        }
    }

    return xChild;
}


// XAccessibleComponent


Reference< XAccessible > VCLXAccessibleStatusBar::getAccessibleAtPoint( const awt::Point& rPoint )
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xChild;
    if ( m_pStatusBar )
    {
        sal_uInt16 nItemId = m_pStatusBar->GetItemId( VCLPoint( rPoint ) );
        sal_Int32 nItemPos = m_pStatusBar->GetItemPos( nItemId );
        if ( nItemPos >= 0 && nItemPos < (sal_Int32)m_aAccessibleChildren.size() )
            xChild = getAccessibleChild( nItemPos );
    }

    return xChild;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
