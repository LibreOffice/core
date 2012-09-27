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

#include <accessibility/standard/vclxaccessiblestatusbar.hxx>
#include <accessibility/standard/vclxaccessiblestatusbaritem.hxx>
#include <toolkit/helper/convert.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <vcl/status.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  ----------------------------------------------------
//  class VCLXAccessibleStatusBar
//  ----------------------------------------------------

VCLXAccessibleStatusBar::VCLXAccessibleStatusBar( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pStatusBar = static_cast< StatusBar* >( GetWindow() );

    if ( m_pStatusBar )
        m_aAccessibleChildren.assign( m_pStatusBar->GetItemCount(), Reference< XAccessible >() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleStatusBar::~VCLXAccessibleStatusBar()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBar::UpdateShowing( sal_Int32 i, sal_Bool bShowing )
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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_STATUSBAR_ITEMADDED:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                InsertChild( nItemPos );
            }
        }
        break;
        case VCLEVENT_STATUSBAR_ITEMREMOVED:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
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
        case VCLEVENT_STATUSBAR_ALLITEMSREMOVED:
        {
            for ( sal_Int32 i = m_aAccessibleChildren.size() - 1; i >= 0; --i )
                RemoveChild( i );
        }
        break;
        case VCLEVENT_STATUSBAR_SHOWITEM:
        case VCLEVENT_STATUSBAR_HIDEITEM:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateShowing( nItemPos, rVclWindowEvent.GetId() == VCLEVENT_STATUSBAR_SHOWITEM );
            }
        }
        break;
        case VCLEVENT_STATUSBAR_SHOWALLITEMS:
        case VCLEVENT_STATUSBAR_HIDEALLITEMS:
        {
            for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
                UpdateShowing( i, rVclWindowEvent.GetId() == VCLEVENT_STATUSBAR_SHOWALLITEMS );
        }
        break;
        case VCLEVENT_STATUSBAR_NAMECHANGED:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateItemName( nItemPos );
            }
        }
        break;
        case VCLEVENT_STATUSBAR_DRAWITEM:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateItemText( nItemPos );
            }
        }
        break;
        case VCLEVENT_OBJECT_DYING:
        {
            if ( m_pStatusBar )
            {
                m_pStatusBar = NULL;

                // dispose all children
                for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
                {
                    Reference< XComponent > xComponent( m_aAccessibleChildren[i], UNO_QUERY );
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

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBar::disposing()
{
    VCLXAccessibleComponent::disposing();

    if ( m_pStatusBar )
    {
        m_pStatusBar = NULL;

        // dispose all children
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

OUString VCLXAccessibleStatusBar::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleStatusBar" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleStatusBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString( "com.sun.star.awt.AccessibleStatusBar" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBar::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleStatusBar::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
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

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleStatusBar::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
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

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
