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

#include <standard/vclxaccessibletabcontrol.hxx>
#include <standard/vclxaccessibletabpage.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  class VCLXAccessibleTabControl


VCLXAccessibleTabControl::VCLXAccessibleTabControl( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pTabControl = static_cast< TabControl* >( GetWindow().get() );
    if (!m_pTabControl)
        return;
    if (m_pTabControl->isDisposed())
    {
        m_pTabControl.clear();
        return;
    }
    m_aAccessibleChildren.assign( m_pTabControl->GetPageCount(), Reference< XAccessible >() );
}


VCLXAccessibleTabControl::~VCLXAccessibleTabControl()
{
}


void VCLXAccessibleTabControl::UpdateFocused()
{
    for (Reference<XAccessible>& xChild : m_aAccessibleChildren)
    {
        if ( xChild.is() )
        {
            VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
            if ( pVCLXAccessibleTabPage )
                pVCLXAccessibleTabPage->SetFocused( pVCLXAccessibleTabPage->IsFocused() );
        }
    }
}


void VCLXAccessibleTabControl::UpdateSelected( sal_Int32 i, bool bSelected )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
            if ( pVCLXAccessibleTabPage )
                pVCLXAccessibleTabPage->SetSelected( bSelected );
        }
    }
}


void VCLXAccessibleTabControl::UpdatePageText( sal_Int32 i )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
            if ( pVCLXAccessibleTabPage )
                pVCLXAccessibleTabPage->SetPageText( pVCLXAccessibleTabPage->GetPageText() );
        }
    }
}


void VCLXAccessibleTabControl::UpdateTabPage( sal_Int32 i, bool bNew )
{
    if ( i >= 0 && i < (sal_Int32)m_aAccessibleChildren.size() )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
            if ( pVCLXAccessibleTabPage )
                pVCLXAccessibleTabPage->Update( bNew );
        }
    }
}


void VCLXAccessibleTabControl::InsertChild( sal_Int32 i )
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


void VCLXAccessibleTabControl::RemoveChild( sal_Int32 i )
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


void VCLXAccessibleTabControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::TabpageActivate:
        case VclEventId::TabpageDeactivate:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                UpdateFocused();
                UpdateSelected( nPagePos, rVclWindowEvent.GetId() == VclEventId::TabpageActivate );
            }
        }
        break;
        case VclEventId::TabpagePageTextChanged:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                UpdatePageText( nPagePos );
            }
        }
        break;
        case VclEventId::TabpageInserted:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                InsertChild( nPagePos );
            }
        }
        break;
        case VclEventId::TabpageRemoved:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData());
                for ( sal_Int32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
                {
                    Reference< XAccessible > xChild( getAccessibleChild( i ) );
                    if ( xChild.is() )
                    {
                        VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
                        if ( pVCLXAccessibleTabPage && pVCLXAccessibleTabPage->GetPageId() == nPageId )
                        {
                            RemoveChild( i );
                            break;
                        }
                    }
                }
            }
        }
        break;
        case VclEventId::TabpageRemovedAll:
        {
            for ( sal_Int32 i = m_aAccessibleChildren.size() - 1; i >= 0; --i )
                RemoveChild( i );
        }
        break;
        case VclEventId::WindowGetFocus:
        case VclEventId::WindowLoseFocus:
        {
            UpdateFocused();
        }
        break;
        case VclEventId::ObjectDying:
        {
            if ( m_pTabControl )
            {
                m_pTabControl = nullptr;

                // dispose all tab pages
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


void VCLXAccessibleTabControl::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::WindowShow:
        case VclEventId::WindowHide:
        {
            if ( m_pTabControl )
            {
                vcl::Window* pChild = static_cast< vcl::Window* >( rVclWindowEvent.GetData() );
                if ( pChild && pChild->GetType() == WINDOW_TABPAGE )
                {
                    for ( sal_Int32 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
                    {
                        sal_uInt16 nPageId = m_pTabControl->GetPageId( (sal_uInt16)i );
                        TabPage* pTabPage = m_pTabControl->GetTabPage( nPageId );
                        if ( pTabPage == static_cast<TabPage*>(pChild) )
                            UpdateTabPage( i, rVclWindowEvent.GetId() == VclEventId::WindowShow );
                    }
                }
            }
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );
    }
}


void VCLXAccessibleTabControl::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    if ( m_pTabControl )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
}


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTabControl, VCLXAccessibleComponent, VCLXAccessibleTabControl_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTabControl, VCLXAccessibleComponent, VCLXAccessibleTabControl_BASE )


// XComponent


void VCLXAccessibleTabControl::disposing()
{
    VCLXAccessibleComponent::disposing();

    if ( m_pTabControl )
    {
        m_pTabControl = nullptr;

        // dispose all tab pages
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


OUString VCLXAccessibleTabControl::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleTabControl" );
}


Sequence< OUString > VCLXAccessibleTabControl::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleTabControl" };
}


// XAccessibleContext


sal_Int32 VCLXAccessibleTabControl::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}


Reference< XAccessible > VCLXAccessibleTabControl::getAccessibleChild( sal_Int32 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        sal_uInt16 nPageId = m_pTabControl ? m_pTabControl->GetPageId((sal_uInt16)i) : 0;
        if (nPageId)
        {
            xChild = new VCLXAccessibleTabPage( m_pTabControl, nPageId );

            // insert into tab page list
            m_aAccessibleChildren[i] = xChild;
        }
    }

    return xChild;
}


sal_Int16 VCLXAccessibleTabControl::getAccessibleRole(  )
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::PAGE_TAB_LIST;
}


OUString VCLXAccessibleTabControl::getAccessibleName(  )
{
    OExternalLockGuard aGuard( this );

    return OUString();
}


// XAccessibleSelection


void VCLXAccessibleTabControl::selectAccessibleChild( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if ( m_pTabControl )
        m_pTabControl->SelectTabPage( m_pTabControl->GetPageId( (sal_uInt16)nChildIndex ) );
}


sal_Bool VCLXAccessibleTabControl::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    bool bSelected = false;
    if ( m_pTabControl && m_pTabControl->GetCurPageId() == m_pTabControl->GetPageId( (sal_uInt16)nChildIndex ) )
        bSelected = true;

    return bSelected;
}


void VCLXAccessibleTabControl::clearAccessibleSelection(  )
{
    // This method makes no sense in a tab control, and so does nothing.
}


void VCLXAccessibleTabControl::selectAllAccessibleChildren(  )
{
    OExternalLockGuard aGuard( this );

    selectAccessibleChild( 0 );
}


sal_Int32 VCLXAccessibleTabControl::getSelectedAccessibleChildCount(  )
{
    OExternalLockGuard aGuard( this );

    return 1;
}


Reference< XAccessible > VCLXAccessibleTabControl::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;

    for ( sal_Int32 i = 0, j = 0, nCount = getAccessibleChildCount(); i < nCount; i++ )
    {
        if ( isAccessibleChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
        {
            xChild = getAccessibleChild( i );
            break;
        }
    }

    return xChild;
}


void VCLXAccessibleTabControl::deselectAccessibleChild( sal_Int32 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    // This method makes no sense in a tab control, and so does nothing.
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
