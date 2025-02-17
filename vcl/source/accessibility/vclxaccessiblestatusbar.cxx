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

#include <accessibility/vclxaccessiblestatusbar.hxx>
#include <accessibility/vclxaccessiblestatusbaritem.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/status.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/vclevent.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;




VCLXAccessibleStatusBar::VCLXAccessibleStatusBar(vcl::Window* pWindow)
    : VCLXAccessibleComponent(pWindow)
{
    m_pStatusBar = GetAs<StatusBar>();

    if ( m_pStatusBar )
        m_aAccessibleChildren.assign( m_pStatusBar->GetItemCount(), rtl::Reference< VCLXAccessibleStatusBarItem >() );
}


void VCLXAccessibleStatusBar::UpdateShowing( sal_Int32 i, bool bShowing )
{
    if ( i >= 0 && o3tl::make_unsigned(i) < m_aAccessibleChildren.size() )
    {
        rtl::Reference< VCLXAccessibleStatusBarItem > pVCLXAccessibleStatusBarItem( m_aAccessibleChildren[i] );
        if ( pVCLXAccessibleStatusBarItem )
            pVCLXAccessibleStatusBarItem->SetShowing( bShowing );
    }
}


void VCLXAccessibleStatusBar::UpdateItemName( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        return;

    rtl::Reference< VCLXAccessibleStatusBarItem > pVCLXAccessibleStatusBarItem( m_aAccessibleChildren[i] );
    if ( pVCLXAccessibleStatusBarItem.is() )
    {
        OUString sItemName = pVCLXAccessibleStatusBarItem->GetItemName();
        pVCLXAccessibleStatusBarItem->SetItemName( sItemName );
    }
}


void VCLXAccessibleStatusBar::UpdateItemText( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        return;

    rtl::Reference< VCLXAccessibleStatusBarItem > pVCLXAccessibleStatusBarItem( m_aAccessibleChildren[i] );
    if ( pVCLXAccessibleStatusBarItem.is() )
    {
        OUString sItemText = pVCLXAccessibleStatusBarItem->GetItemText();
        pVCLXAccessibleStatusBarItem->SetItemText( sItemText );
    }
}


void VCLXAccessibleStatusBar::InsertChild( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) > m_aAccessibleChildren.size() )
        return;

    // insert entry in child list
    m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + i, rtl::Reference< VCLXAccessibleStatusBarItem >() );

    // send accessible child event
    Reference< XAccessible > xChild( getAccessibleChild( i ) );
    if ( xChild.is() )
    {
        Any aOldValue, aNewValue;
        aNewValue <<= xChild;
        NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
    }
}


void VCLXAccessibleStatusBar::RemoveChild( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        return;

    // get the accessible of the removed page
    rtl::Reference< VCLXAccessibleStatusBarItem > xChild( m_aAccessibleChildren[i] );

    // remove entry in child list
    m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

    // send accessible child event
    if ( xChild.is() )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= Reference< XAccessible >(xChild);
        NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );

        xChild->dispose();
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
                sal_uInt16 nItemId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                InsertChild( nItemPos );
            }
        }
        break;
        case VclEventId::StatusbarItemRemoved:
        {
            if ( m_pStatusBar )
            {
                OExternalLockGuard aGuard( this );

                sal_uInt16 nItemId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                for ( sal_Int64 i = 0, nCount = m_aAccessibleChildren.size(); i < nCount; ++i )
                {
                    sal_uInt16 nChildItemId = m_pStatusBar->GetItemId( static_cast<sal_uInt16>(i) );
                    if ( nChildItemId == nItemId )
                    {
                        RemoveChild( i );
                        break;
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
                sal_uInt16 nItemId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateShowing( nItemPos, rVclWindowEvent.GetId() == VclEventId::StatusbarShowItem );
            }
        }
        break;
        case VclEventId::StatusbarNameChanged:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nItemPos = m_pStatusBar->GetItemPos( nItemId );
                UpdateItemName( nItemPos );
            }
        }
        break;
        case VclEventId::StatusbarDrawItem:
        {
            if ( m_pStatusBar )
            {
                sal_uInt16 nItemId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
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
                for (const rtl::Reference<VCLXAccessibleStatusBarItem>& xComponent : m_aAccessibleChildren)
                {
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

    if ( !m_pStatusBar )
        return;

    m_pStatusBar = nullptr;

    // dispose all children
    for (const rtl::Reference<VCLXAccessibleStatusBarItem>& xComponent : m_aAccessibleChildren)
    {
        if ( xComponent.is() )
            xComponent->dispose();
    }
    m_aAccessibleChildren.clear();
}


// XServiceInfo


OUString VCLXAccessibleStatusBar::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleStatusBar"_ustr;
}


Sequence< OUString > VCLXAccessibleStatusBar::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.AccessibleStatusBar"_ustr };
}


// XAccessibleContext


sal_Int64 VCLXAccessibleStatusBar::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}


Reference< XAccessible > VCLXAccessibleStatusBar::getAccessibleChild( sal_Int64 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        throw IndexOutOfBoundsException();

    rtl::Reference< VCLXAccessibleStatusBarItem > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        if ( m_pStatusBar )
        {
            sal_uInt16 nItemId = m_pStatusBar->GetItemId( static_cast<sal_uInt16>(i) );

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
        sal_uInt16 nItemId = m_pStatusBar->GetItemId(vcl::unohelper::ConvertToVCLPoint(rPoint));
        sal_Int32 nItemPos = m_pStatusBar->GetItemPos( nItemId );
        if ( nItemPos >= 0 && o3tl::make_unsigned(nItemPos) < m_aAccessibleChildren.size() )
            xChild = getAccessibleChild( nItemPos );
    }

    return xChild;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
