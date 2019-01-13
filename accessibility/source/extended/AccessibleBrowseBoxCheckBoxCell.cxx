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

#include <extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <vcl/accessibletableprovider.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

namespace accessibility
{
    using namespace com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::accessibility::AccessibleEventId;

    AccessibleCheckBoxCell::AccessibleCheckBoxCell(const Reference<XAccessible >& _rxParent,
                                vcl::IAccessibleTableProvider& _rBrowseBox,
                                const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos
                                ,const TriState& _eState,
                                bool _bIsTriState)
        :AccessibleBrowseBoxCell(_rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos, vcl::BBTYPE_CHECKBOXCELL)
        ,m_eState(_eState)
        ,m_bIsTriState(_bIsTriState)
    {
    }
    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleCheckBoxCell, AccessibleBrowseBoxCell, AccessibleCheckBoxCell_BASE )

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleCheckBoxCell, AccessibleBrowseBoxCell, AccessibleCheckBoxCell_BASE )

    Reference< XAccessibleContext > SAL_CALL AccessibleCheckBoxCell::getAccessibleContext(  )
    {
        osl::MutexGuard aGuard( getMutex() );
        ensureIsAlive();
        return this;
    }

    ::utl::AccessibleStateSetHelper* AccessibleCheckBoxCell::implCreateStateSetHelper()
    {
        ::utl::AccessibleStateSetHelper* pStateSetHelper =
            AccessibleBrowseBoxCell::implCreateStateSetHelper();
        if( isAlive() )
        {
            mpBrowseBox->FillAccessibleStateSetForCell(
                *pStateSetHelper, getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
            if ( m_eState == TRISTATE_TRUE )
                pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        }
        return pStateSetHelper;
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

    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleCheckBoxCell::getAccessibleChildCount(  )
    {
        return 0;
    }

    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL AccessibleCheckBoxCell::getAccessibleChild( sal_Int32 )
    {
        throw css::lang::IndexOutOfBoundsException();
    }

    OUString SAL_CALL AccessibleCheckBoxCell::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.TableCheckBoxCell" );
    }

    sal_Int32 SAL_CALL AccessibleCheckBoxCell::getAccessibleIndexInParent()
    {
        ::osl::MutexGuard aGuard( getMutex() );
        ensureIsAlive();

        return ( getRowPos() * mpBrowseBox->GetColumnCount() ) + getColumnPos();
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
