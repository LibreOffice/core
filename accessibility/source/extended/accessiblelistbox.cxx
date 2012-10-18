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

#include <accessibility/extended/accessiblelistbox.hxx>
#include <accessibility/extended/accessiblelistboxentry.hxx>
#include <svtools/treelistbox.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleListBox::AccessibleListBox( SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rListBox.GetWindowPeer() ),
        m_xParent( _xParent )
    {
    }
    // -----------------------------------------------------------------------------
    AccessibleListBox::~AccessibleListBox()
    {
        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    // -----------------------------------------------------------------------------
    SvTreeListBox* AccessibleListBox::getListBox() const
    {
        return  static_cast< SvTreeListBox* >( const_cast<AccessibleListBox*>(this)->GetWindow() );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
                case  VCLEVENT_CHECKBOX_TOGGLE :
                {
                    if ( getListBox() && getListBox()->HasFocus() )
                    {
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if ( !pEntry )
                            pEntry = getListBox()->GetCurEntry();

                        if ( pEntry )
                        {
                            Reference< XAccessible > xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;
                }

                case VCLEVENT_LISTBOX_SELECT :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
                    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    if ( getListBox() && getListBox()->HasFocus() )
                    {
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            Reference< XAccessible > xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;

                // #i92103#
                case VCLEVENT_ITEM_EXPANDED :
                case VCLEVENT_ITEM_COLLAPSED :
                {
                    SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                    if ( pEntry )
                    {
                        AccessibleListBoxEntry* pAccListBoxEntry =
                            new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                        Reference< XAccessible > xChild = pAccListBoxEntry;
                        const short nAccEvent =
                                ( rVclWindowEvent.GetId() == VCLEVENT_ITEM_EXPANDED )
                                ? AccessibleEventId::LISTBOX_ENTRY_EXPANDED
                                : AccessibleEventId::LISTBOX_ENTRY_COLLAPSED;
                        uno::Any aListBoxEntry;
                        aListBoxEntry <<= xChild;
                        NotifyAccessibleEvent( nAccEvent, Any(), aListBoxEntry );
                        if ( getListBox() && getListBox()->HasFocus() )
                        {
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), aListBoxEntry );
                        }
                    }
                    break;
                }
                }
                default:
                    VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            }
        }
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_HIDE:
            {
            }
            break;
            default:
            {
                VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );
            }
            break;
        }
    }

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        VCLXAccessibleComponent::disposing();
        m_xParent = NULL;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBox::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< OUString > SAL_CALL AccessibleListBox::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
    {
        return cppu::supportsService(this, _rServiceName);
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< OUString > AccessibleListBox::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< OUString > aSupported(3);
        aSupported[0] = "com.sun.star.accessibility.AccessibleContext";
        aSupported[1] = "com.sun.star.accessibility.AccessibleComponent";
        aSupported[2] = "com.sun.star.awt.AccessibleTreeListBox";
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    OUString AccessibleListBox::getImplementationName_Static(void) throw( RuntimeException )
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTreeListBox" );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleListBox::getAccessibleContext(  ) throw (RuntimeException)
    {
        ensureAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = 0;
        SvTreeListBox* pSvTreeListBox = getListBox();
        if ( pSvTreeListBox )
            nCount = pSvTreeListBox->GetLevelChildCount( NULL );

        return nCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        SvTreeListEntry* pEntry = getListBox()->GetEntry(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleListBox::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::TREE;
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBox::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleDescription();
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBox::getAccessibleName(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleName();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = getListBox()->GetLevelChildCount( NULL );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_False );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = getListBox()->GetLevelChildCount( NULL );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_True );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nSelCount = 0;
        sal_Int32 nCount = getListBox()->GetLevelChildCount( NULL );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;
        }

        return nSelCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 nSelCount= 0;
        sal_Int32 nCount = getListBox()->GetLevelChildCount( NULL );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvTreeListEntry* pEntry = getListBox()->GetEntry( nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_False );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( getListBox() && isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            if ( getListBox()->GetSelectionMode() == MULTIPLE_SELECTION )
                rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
        }
    }


//........................................................................
}// namespace accessibility
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
