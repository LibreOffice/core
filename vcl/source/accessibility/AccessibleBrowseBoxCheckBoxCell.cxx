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

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/accessibility/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

AccessibleCheckBoxCell::AccessibleCheckBoxCell(const Reference<XAccessible>& _rxParent,
                                               vcl::IAccessibleTableProvider& _rBrowseBox,
                                               sal_Int32 _nRowPos, sal_uInt16 _nColPos,
                                               const TriState& _eState, bool _bIsTriState)
    : ImplInheritanceHelper(_rxParent, _rBrowseBox, nullptr, _nRowPos, _nColPos,
                            AccessibleBrowseBoxObjType::CheckBoxCell)
    , m_eState(_eState)
    , m_bIsTriState(_bIsTriState)
{
}

sal_Int64 AccessibleCheckBoxCell::implCreateStateSet()
{
    sal_Int64 nStateSet = AccessibleBrowseBoxCell::implCreateStateSet();
    if( isAlive() )
    {
        nStateSet |= AccessibleStateType::CHECKABLE;
        mpBrowseBox->FillAccessibleStateSetForCell(
            nStateSet, getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
        if ( m_eState == TRISTATE_TRUE )
            nStateSet |= AccessibleStateType::CHECKED;
    }
    return nStateSet;
}

// XAccessibleValue

Any SAL_CALL AccessibleCheckBoxCell::getCurrentValue(  )
{
    ::osl::MutexGuard aGuard( getMutex() );

    sal_Int32 nValue = 0;
    switch( m_eState )
    {
        case TRISTATE_FALSE:
            nValue = 0;
            break;
        case TRISTATE_TRUE:
            nValue = 1;
            break;
        case TRISTATE_INDET:
            nValue = 2;
            break;
    }
    return Any(nValue);
}

sal_Bool SAL_CALL AccessibleCheckBoxCell::setCurrentValue( const Any& )
{
    return false;
}

Any SAL_CALL AccessibleCheckBoxCell::getMaximumValue(  )
{
    ::osl::MutexGuard aGuard( getMutex() );

    Any aValue;

    if ( m_bIsTriState )
        aValue <<= sal_Int32(2);
    else
        aValue <<= sal_Int32(1);

    return aValue;
}

Any SAL_CALL AccessibleCheckBoxCell::getMinimumValue(  )
{
    Any aValue;
    aValue <<= sal_Int32(0);

    return aValue;
}

Any SAL_CALL AccessibleCheckBoxCell::getMinimumIncrement(  )
{
    Any aValue;
    aValue <<= sal_Int32(1);

    return aValue;
}

// XAccessibleContext
sal_Int64 SAL_CALL AccessibleCheckBoxCell::getAccessibleChildCount(  )
{
    return 0;
}

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL AccessibleCheckBoxCell::getAccessibleChild( sal_Int64 )
{
    throw css::lang::IndexOutOfBoundsException();
}

OUString SAL_CALL AccessibleCheckBoxCell::getImplementationName()
{
    return u"com.sun.star.comp.svtools.TableCheckBoxCell"_ustr;
}

sal_Int64 SAL_CALL AccessibleCheckBoxCell::getAccessibleIndexInParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();

    return (static_cast<sal_Int64>(getRowPos()) * static_cast<sal_Int64>(mpBrowseBox->GetColumnCount())) + getColumnPos();
}

void AccessibleCheckBoxCell::SetChecked( bool _bChecked )
{
    m_eState = _bChecked ? TRISTATE_TRUE : TRISTATE_FALSE;
    Any aOldValue, aNewValue;
    if ( _bChecked )
        aNewValue <<= AccessibleStateType::CHECKED;
    else
        aOldValue <<= AccessibleStateType::CHECKED;
    commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
