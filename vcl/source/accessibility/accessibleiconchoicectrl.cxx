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

#include <accessibility/accessibleiconchoicectrl.hxx>
#include <accessibility/accessibleiconchoicectrlentry.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <vcl/toolkit/ivctrl.hxx>

// class AccessibleIconChoiceCtrl ----------------------------------------------

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;


// Ctor() and Dtor()

AccessibleIconChoiceCtrl::AccessibleIconChoiceCtrl(SvtIconChoiceCtrl& _rIconCtrl,
                                                   const Reference<XAccessible>& _xParent)
    : ImplInheritanceHelper(&_rIconCtrl)
    , m_xParent(_xParent)
{
}

void AccessibleIconChoiceCtrl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    if ( !isAlive() )
        return;

    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::ListboxSelect :
        {
            if (getCtrl())
            {
                SvxIconChoiceCtrlEntry* pEntry = static_cast< SvxIconChoiceCtrlEntry* >( rVclWindowEvent.GetData() );
                if ( pEntry )
                {
                    sal_Int32 nPos = getCtrl()->GetEntryListPos( pEntry );
                    Reference< XAccessible > xChild = new AccessibleIconChoiceCtrlEntry( *getCtrl(), nPos, this );
                    uno::Any aOldValue, aNewValue;
                    aNewValue <<= xChild;
                    if (getCtrl()->HasFocus())
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
                SvxIconChoiceCtrlEntry* pEntry = getCtrl()->GetSelectedEntry();
                if ( pEntry )
                {
                    sal_Int32 nPos = pCtrl->GetEntryListPos( pEntry );
                    Reference< XAccessible > xChild = new AccessibleIconChoiceCtrlEntry( *pCtrl, nPos, this );
                    uno::Any aOldValue, aNewValue;
                    aNewValue <<= xChild;
                    NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                }
            }
            break;
        }
        default:
            VCLXAccessibleComponent::ProcessWindowChildEvent (rVclWindowEvent);
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
    return u"com.sun.star.comp.svtools.AccessibleIconChoiceControl"_ustr;
}

Sequence< OUString > SAL_CALL AccessibleIconChoiceCtrl::getSupportedServiceNames()
{
    return {u"com.sun.star.accessibility.AccessibleContext"_ustr,
            u"com.sun.star.accessibility.AccessibleComponent"_ustr,
            u"com.sun.star.awt.AccessibleIconChoiceControl"_ustr};
}

// XAccessible

Reference< XAccessibleContext > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleContext(  )
{
    ensureAlive();
    return this;
}

// XAccessibleContext

sal_Int64 SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChildCount(  )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    return getCtrl()->GetEntryCount();
}

Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChild( sal_Int64 i )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    if (i < 0 || i >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
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

    return getCtrl()->GetAccessibleDescription();
}

OUString SAL_CALL AccessibleIconChoiceCtrl::getAccessibleName(  )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    return getCtrl()->GetAccessibleName();
}

// XAccessibleSelection

void SAL_CALL AccessibleIconChoiceCtrl::selectAccessibleChild( sal_Int64 nChildIndex )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
    SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
    if ( !pEntry )
        throw IndexOutOfBoundsException();

    pCtrl->SetCursor( pEntry );
}

sal_Bool SAL_CALL AccessibleIconChoiceCtrl::isAccessibleChildSelected( sal_Int64 nChildIndex )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw IndexOutOfBoundsException();

    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
    SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
    if ( !pEntry )
        throw IndexOutOfBoundsException();

    return ( pCtrl->GetCursor() == pEntry );
}

void SAL_CALL AccessibleIconChoiceCtrl::clearAccessibleSelection(  )
{
    // one entry should always be selected, so don't unselect
}

void SAL_CALL AccessibleIconChoiceCtrl::selectAllAccessibleChildren(  )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    // don't do anything if there are no or multiple entries, as only
    // a single one can be selected
    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
    sal_Int32 nCount = pCtrl->GetEntryCount();
    if (nCount != 1)
        return;

    pCtrl->SetCursor(pCtrl->GetEntry(0));
}

sal_Int64 SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChildCount(  )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    VclPtr<SvtIconChoiceCtrl> pCtrl = getCtrl();
    if (pCtrl->GetCursor())
        return 1;

    return 0;
}

Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    rtl::Reference< AccessibleIconChoiceCtrlEntry > xChild;
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

void SAL_CALL AccessibleIconChoiceCtrl::deselectAccessibleChild( sal_Int64 nSelectedChildIndex )
{
    ::comphelper::OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    // one entry should always be selected, so don't unselect
}

void AccessibleIconChoiceCtrl::FillAccessibleStateSet( sal_Int64& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
    if ( isAlive() )
    {
        rStateSet |= AccessibleStateType::FOCUSABLE;
        rStateSet |= AccessibleStateType::MANAGES_DESCENDANTS;
    }
}

VclPtr< SvtIconChoiceCtrl > AccessibleIconChoiceCtrl::getCtrl() const
{
    return GetAs<SvtIconChoiceCtrl >();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
