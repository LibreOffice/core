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

#include "extended/accessibleiconchoicectrl.hxx"
#include "extended/accessibleiconchoicectrlentry.hxx"
#include <svtools/ivctrl.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>


namespace accessibility
{


    // class AccessibleIconChoiceCtrl ----------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;


    // Ctor() and Dtor()

    AccessibleIconChoiceCtrl::AccessibleIconChoiceCtrl( SvtIconChoiceCtrl& _rIconCtrl, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rIconCtrl.GetWindowPeer() ),
        m_xParent       ( _xParent )
    {
    }

    AccessibleIconChoiceCtrl::~AccessibleIconChoiceCtrl()
    {
    }

    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleIconChoiceCtrl, VCLXAccessibleComponent, AccessibleIconChoiceCtrl_BASE)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleIconChoiceCtrl, VCLXAccessibleComponent, AccessibleIconChoiceCtrl_BASE)

    void AccessibleIconChoiceCtrl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
                case VclEventId::ListboxSelect :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
//                  NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

                    if ( getCtrl() && getCtrl()->HasFocus() )
                    {
                        SvxIconChoiceCtrlEntry* pEntry = static_cast< SvxIconChoiceCtrlEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            sal_uLong nPos = getCtrl()->GetEntryListPos( pEntry );
                            Reference< XAccessible > xChild = new AccessibleIconChoiceCtrlEntry( *getCtrl(), nPos, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );

                            NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, aOldValue, aNewValue );

                        }
                    }
                    break;
                }
                case VclEventId::WindowGetFocus :
                {
                    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
                    if ( pCtrl && pCtrl->HasFocus() )
                    {
                        SvxIconChoiceCtrlEntry* pEntry = static_cast< SvxIconChoiceCtrlEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry == nullptr )
                        {
                            pEntry = getCtrl()->GetSelectedEntry();
                        }
                        if ( pEntry )
                        {
                            sal_uLong nPos = pCtrl->GetEntryListPos( pEntry );
                            Reference< XAccessible > xChild = new AccessibleIconChoiceCtrlEntry( *pCtrl, nPos, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                            NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;
                }
                default:
                    VCLXAccessibleComponent::ProcessWindowChildEvent (rVclWindowEvent);
            }
        }
    }

    // XComponent

    void SAL_CALL AccessibleIconChoiceCtrl::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xParent = nullptr;
    }

    // XServiceInfo

    OUString SAL_CALL AccessibleIconChoiceCtrl::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleIconChoiceControl" );
    }

    Sequence< OUString > SAL_CALL AccessibleIconChoiceCtrl::getSupportedServiceNames()
    {
        return {"com.sun.star.accessibility.AccessibleContext",
                "com.sun.star.accessibility.AccessibleComponent",
                "com.sun.star.awt.AccessibleIconChoiceControl"};
    }

    sal_Bool SAL_CALL AccessibleIconChoiceCtrl::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    // XAccessible

    Reference< XAccessibleContext > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleContext(  )
    {
        ensureAlive();
        return this;
    }

    // XAccessibleContext

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChildCount(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getCtrl()->GetEntryCount();
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChild( sal_Int32 i )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry(i);
        if ( !pEntry )
            throw RuntimeException();

        return new AccessibleIconChoiceCtrlEntry( *pCtrl, i, this );
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }

    sal_Int16 SAL_CALL AccessibleIconChoiceCtrl::getAccessibleRole(  )
    {
        //return AccessibleRole::TREE;
        return AccessibleRole::LIST;
    }

    OUString SAL_CALL AccessibleIconChoiceCtrl::getAccessibleDescription(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getCtrl()->GetAccessibleDescription();
    }

    OUString SAL_CALL AccessibleIconChoiceCtrl::getAccessibleName(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        OUString sName = getCtrl()->GetAccessibleName();
        if ( sName.isEmpty() )
            sName = "IconChoiceControl";
        return sName;
    }

    // XAccessibleSelection

    void SAL_CALL AccessibleIconChoiceCtrl::selectAccessibleChild( sal_Int32 nChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        pCtrl->SetCursor( pEntry );
    }

    sal_Bool SAL_CALL AccessibleIconChoiceCtrl::isAccessibleChildSelected( sal_Int32 nChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return ( pCtrl->GetCursor() == pEntry );
    }

    void SAL_CALL AccessibleIconChoiceCtrl::clearAccessibleSelection(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        getCtrl()->SetNoSelection();
    }

    void SAL_CALL AccessibleIconChoiceCtrl::selectAllAccessibleChildren(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        sal_Int32 nCount = pCtrl->GetEntryCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() != pEntry )
                pCtrl->SetCursor( pEntry );
        }
    }

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChildCount(  )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nSelCount = 0;
        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        sal_Int32 nCount = pCtrl->GetEntryCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() == pEntry )
                ++nSelCount;
        }

        return nSelCount;
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 nSelCount = 0;
        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        sal_Int32 nCount = pCtrl->GetEntryCount();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() == pEntry )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleIconChoiceCtrlEntry( *pCtrl, i, this );
                break;
            }
        }

        return xChild;
    }

    void SAL_CALL AccessibleIconChoiceCtrl::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 nSelCount = 0;
        VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
        sal_Int32 nCount = pCtrl->GetEntryCount();
        bool bFound = false;
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pEntry->IsSelected() )
            {
                ++nSelCount;
                if ( i == nSelectedChildIndex )
                    bFound = true;
            }
        }

        // if only one entry is selected and its index is chosen to deselect -> no selection anymore
        if ( 1 == nSelCount && bFound )
            pCtrl->SetNoSelection();
    }

    void AccessibleIconChoiceCtrl::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            rStateSet.AddState( AccessibleStateType::SELECTABLE );
        }
    }

    VclPtr< SvtIconChoiceCtrl > AccessibleIconChoiceCtrl::getCtrl()
    {
        return GetAs<SvtIconChoiceCtrl >();
    }

}// namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
