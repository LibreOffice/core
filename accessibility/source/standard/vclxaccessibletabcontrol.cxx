/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <accessibility/standard/vclxaccessibletabcontrol.hxx>
#include <accessibility/standard/vclxaccessibletabpage.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  ----------------------------------------------------
//  class VCLXAccessibleTabControl
//  ----------------------------------------------------

VCLXAccessibleTabControl::VCLXAccessibleTabControl( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    m_pTabControl = static_cast< TabControl* >( GetWindow() );

    if ( m_pTabControl )
        m_aAccessibleChildren.assign( m_pTabControl->GetPageCount(), Reference< XAccessible >() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleTabControl::~VCLXAccessibleTabControl()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::UpdateFocused()
{
    for ( sal_uInt32 i = 0; i < m_aAccessibleChildren.size(); ++i )
    {
        Reference< XAccessible > xChild( m_aAccessibleChildren[i] );
        if ( xChild.is() )
        {
            VCLXAccessibleTabPage* pVCLXAccessibleTabPage = static_cast< VCLXAccessibleTabPage* >( xChild.get() );
            if ( pVCLXAccessibleTabPage )
                pVCLXAccessibleTabPage->SetFocused( pVCLXAccessibleTabPage->IsFocused() );
        }
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::UpdateSelected( sal_Int32 i, bool bSelected )
{
    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABPAGE_ACTIVATE:
        case VCLEVENT_TABPAGE_DEACTIVATE:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                UpdateFocused();
                UpdateSelected( nPagePos, rVclWindowEvent.GetId() == VCLEVENT_TABPAGE_ACTIVATE );
            }
        }
        break;
        case VCLEVENT_TABPAGE_PAGETEXTCHANGED:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                UpdatePageText( nPagePos );
            }
        }
        break;
        case VCLEVENT_TABPAGE_INSERTED:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
                sal_uInt16 nPagePos = m_pTabControl->GetPagePos( nPageId );
                InsertChild( nPagePos );
            }
        }
        break;
        case VCLEVENT_TABPAGE_REMOVED:
        {
            if ( m_pTabControl )
            {
                sal_uInt16 nPageId = (sal_uInt16)(sal_IntPtr) rVclWindowEvent.GetData();
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
        case VCLEVENT_TABPAGE_REMOVEDALL:
        {
            for ( sal_Int32 i = m_aAccessibleChildren.size() - 1; i >= 0; --i )
                RemoveChild( i );
        }
        break;
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            UpdateFocused();
        }
        break;
        case VCLEVENT_OBJECT_DYING:
        {
            if ( m_pTabControl )
            {
                m_pTabControl = NULL;

                // dispose all tab pages
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

void VCLXAccessibleTabControl::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:
        case VCLEVENT_WINDOW_HIDE:
        {
            if ( m_pTabControl )
            {
                Window* pChild = static_cast< Window* >( rVclWindowEvent.GetData() );
                if ( pChild && pChild->GetType() == WINDOW_TABPAGE )
                {
                    for ( sal_Int32 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
                    {
                        sal_uInt16 nPageId = m_pTabControl->GetPageId( (sal_uInt16)i );
                        TabPage* pTabPage = m_pTabControl->GetTabPage( nPageId );
                        if ( pTabPage == (TabPage*) pChild )
                            UpdateTabPage( i, rVclWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW );
                    }
                }
            }
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );
    }
}


// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );

    if ( m_pTabControl )
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTabControl, VCLXAccessibleComponent, VCLXAccessibleTabControl_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTabControl, VCLXAccessibleComponent, VCLXAccessibleTabControl_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::disposing()
{
    VCLXAccessibleComponent::disposing();

    if ( m_pTabControl )
    {
        m_pTabControl = NULL;

        // dispose all tab pages
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

OUString VCLXAccessibleTabControl::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleTabControl" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleTabControl::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleTabControl";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabControl::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_aAccessibleChildren.size();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabControl::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild = m_aAccessibleChildren[i];
    if ( !xChild.is() )
    {
        if ( m_pTabControl )
        {
            sal_uInt16 nPageId = m_pTabControl->GetPageId( (sal_uInt16)i );

            xChild = new VCLXAccessibleTabPage( m_pTabControl, nPageId );

            // insert into tab page list
            m_aAccessibleChildren[i] = xChild;
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleTabControl::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::PAGE_TAB_LIST;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabControl::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleSelection
// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    if ( m_pTabControl )
        m_pTabControl->SelectTabPage( m_pTabControl->GetPageId( (sal_uInt16)nChildIndex ) );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTabControl::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    sal_Bool bSelected = sal_False;
    if ( m_pTabControl && m_pTabControl->GetCurPageId() == m_pTabControl->GetPageId( (sal_uInt16)nChildIndex ) )
        bSelected = sal_True;

    return bSelected;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::clearAccessibleSelection(  ) throw (RuntimeException)
{
    // This method makes no sense in a tab control, and so does nothing.
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    selectAccessibleChild( 0 );
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabControl::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 1;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabControl::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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

// -----------------------------------------------------------------------------

void VCLXAccessibleTabControl::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    // This method makes no sense in a tab control, and so does nothing.
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
