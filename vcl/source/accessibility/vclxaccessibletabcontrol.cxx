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

#include <accessibility/vclxaccessibletabcontrol.hxx>
#include <accessibility/vclxaccessibletabpage.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/vclevent.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;




VCLXAccessibleTabControl::VCLXAccessibleTabControl(vcl::Window* pWindow)
    : ImplInheritanceHelper(pWindow)
{
    m_pTabControl = GetAs<TabControl>();
    if (!m_pTabControl)
        return;
    if (m_pTabControl->isDisposed())
    {
        m_pTabControl.reset();
        return;
    }
    m_aAccessibleChildren.assign( m_pTabControl->GetPageCount(), rtl::Reference< VCLXAccessibleTabPage >() );
}


void VCLXAccessibleTabControl::UpdateFocused()
{
    for (const rtl::Reference<VCLXAccessibleTabPage>& pVCLXAccessibleTabPage : m_aAccessibleChildren)
    {
        if ( pVCLXAccessibleTabPage )
            pVCLXAccessibleTabPage->SetFocused( pVCLXAccessibleTabPage->IsFocused() );
    }
}


void VCLXAccessibleTabControl::UpdateSelected( sal_Int32 i, bool bSelected )
{
    if ( i >= 0 && o3tl::make_unsigned(i) < m_aAccessibleChildren.size() )
    {
        rtl::Reference< VCLXAccessibleTabPage > pVCLXAccessibleTabPage( m_aAccessibleChildren[i] );
        if ( pVCLXAccessibleTabPage )
            pVCLXAccessibleTabPage->SetSelected( bSelected );
    }
}


void VCLXAccessibleTabControl::UpdatePageText( sal_Int32 i )
{
    if ( i >= 0 && o3tl::make_unsigned(i) < m_aAccessibleChildren.size() )
    {
        rtl::Reference< VCLXAccessibleTabPage > pVCLXAccessibleTabPage( m_aAccessibleChildren[i] );
        if ( pVCLXAccessibleTabPage )
            pVCLXAccessibleTabPage->SetPageText( pVCLXAccessibleTabPage->GetPageText() );
    }
}


void VCLXAccessibleTabControl::UpdateTabPage( sal_Int32 i, bool bNew )
{
    if ( i >= 0 && o3tl::make_unsigned(i) < m_aAccessibleChildren.size() )
    {
        rtl::Reference< VCLXAccessibleTabPage > pVCLXAccessibleTabPage( m_aAccessibleChildren[i] );
        if ( pVCLXAccessibleTabPage )
            pVCLXAccessibleTabPage->Update( bNew );
    }
}


void VCLXAccessibleTabControl::InsertChild( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) > m_aAccessibleChildren.size() )
        return;

    // insert entry in child list
    m_aAccessibleChildren.insert( m_aAccessibleChildren.begin() + i, rtl::Reference< VCLXAccessibleTabPage >() );

    // send accessible child event
    Reference< XAccessible > xChild( getAccessibleChild( i ) );
    if ( xChild.is() )
    {
        Any aOldValue, aNewValue;
        aNewValue <<= xChild;
        NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
    }
}


void VCLXAccessibleTabControl::RemoveChild( sal_Int32 i )
{
    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        return;

    // get the accessible of the removed page
    rtl::Reference< VCLXAccessibleTabPage > xChild( m_aAccessibleChildren[i] );

    // remove entry in child list
    m_aAccessibleChildren.erase( m_aAccessibleChildren.begin() + i );

    // send accessible child event
    if ( xChild.is() )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= uno::Reference<XAccessible>(xChild);
        NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );

        xChild->dispose();
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
                sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
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
                sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                UpdatePageText( nPagePos );
            }
        }
        break;
        case VclEventId::TabpageInserted:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                InsertChild( nPagePos );
            }
        }
        break;
        case VclEventId::TabpageRemoved:
        {
            if ( m_pTabControl )
            {
                OExternalLockGuard aGuard( this );
                sal_uInt16 nPageId = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rVclWindowEvent.GetData()));
                for ( sal_Int64 i = 0, nCount = m_aAccessibleChildren.size(); i < nCount; ++i )
                {
                    if ( m_aAccessibleChildren[i] && m_aAccessibleChildren[i]->GetPageId() == nPageId )
                    {
                        RemoveChild( i );
                        break;
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
                for (const rtl::Reference<VCLXAccessibleTabPage>& i : m_aAccessibleChildren)
                {
                    if (i.is())
                        i->dispose();
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
                if ( pChild && pChild->GetType() == WindowType::TABPAGE )
                {
                    for ( sal_Int32 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
                    {
                        sal_uInt16 nPageId = m_pTabControl->GetPageId( static_cast<sal_uInt16>(i) );
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


void VCLXAccessibleTabControl::FillAccessibleStateSet( sal_Int64& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    if ( m_pTabControl )
        rStateSet |= AccessibleStateType::FOCUSABLE;
}


// XComponent


void VCLXAccessibleTabControl::disposing()
{
    VCLXAccessibleComponent::disposing();

    if ( !m_pTabControl )
        return;

    m_pTabControl = nullptr;

    // dispose all tab pages
    for (const rtl::Reference<VCLXAccessibleTabPage>& xComponent : m_aAccessibleChildren)
        if ( xComponent.is() )
            xComponent->dispose();
    m_aAccessibleChildren.clear();
}


// XServiceInfo


OUString VCLXAccessibleTabControl::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleTabControl"_ustr;
}


Sequence< OUString > VCLXAccessibleTabControl::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.AccessibleTabControl"_ustr };
}


// XAccessibleContext


sal_Int64 VCLXAccessibleTabControl::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}


Reference< XAccessible > VCLXAccessibleTabControl::getAccessibleChild( sal_Int64 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || o3tl::make_unsigned(i) >= m_aAccessibleChildren.size() )
        throw IndexOutOfBoundsException();

    return implGetAccessibleChild( i );
}

Reference< XAccessible > VCLXAccessibleTabControl::implGetAccessibleChild( sal_Int64 i )
{
    rtl::Reference< VCLXAccessibleTabPage > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        sal_uInt16 nPageId = m_pTabControl ? m_pTabControl->GetPageId(static_cast<sal_uInt16>(i)) : 0;
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


void VCLXAccessibleTabControl::selectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || o3tl::make_unsigned(nChildIndex) >= m_aAccessibleChildren.size() )
        throw IndexOutOfBoundsException();

    if ( m_pTabControl )
        m_pTabControl->SelectTabPage( m_pTabControl->GetPageId( static_cast<sal_uInt16>(nChildIndex) ) );
}


sal_Bool VCLXAccessibleTabControl::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || o3tl::make_unsigned(nChildIndex) >= m_aAccessibleChildren.size() )
        throw IndexOutOfBoundsException();

    return implIsAccessibleChildSelected( nChildIndex );
}

bool VCLXAccessibleTabControl::implIsAccessibleChildSelected( sal_Int32 nChildIndex )
{
    bool bSelected = false;
    if ( m_pTabControl && m_pTabControl->GetCurPageId() == m_pTabControl->GetPageId( static_cast<sal_uInt16>(nChildIndex) ) )
        bSelected = true;

    return bSelected;
}


void VCLXAccessibleTabControl::clearAccessibleSelection(  )
{
    // This method makes no sense in a tab control, and so does nothing.
}


void VCLXAccessibleTabControl::selectAllAccessibleChildren(  )
{
    selectAccessibleChild( 0 );
}


sal_Int64 VCLXAccessibleTabControl::getSelectedAccessibleChildCount(  )
{
    OExternalLockGuard aGuard( this );

    return 1;
}


Reference< XAccessible > VCLXAccessibleTabControl::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex != 0 )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;

    for ( std::size_t i = 0, nCount = m_aAccessibleChildren.size(); i < nCount; i++ )
    {
        if ( implIsAccessibleChildSelected( i ) )
        {
            xChild = implGetAccessibleChild( i );
            break;
        }
    }

    return xChild;
}


void VCLXAccessibleTabControl::deselectAccessibleChild( sal_Int64 nChildIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || o3tl::make_unsigned(nChildIndex) >= m_aAccessibleChildren.size() )
        throw IndexOutOfBoundsException();

    // This method makes no sense in a tab control, and so does nothing.
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
