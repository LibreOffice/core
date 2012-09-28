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

#include <accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <svtools/accessibletableprovider.hxx>

namespace accessibility
{
    using namespace com::sun::star::accessibility;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::accessibility::AccessibleEventId;
    using namespace ::svt;

    AccessibleCheckBoxCell::AccessibleCheckBoxCell(const Reference<XAccessible >& _rxParent,
                                IAccessibleTableProvider& _rBrowseBox,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
                                sal_Int32 _nRowPos,
                                sal_uInt16 _nColPos
                                ,const TriState& _eState,
                                sal_Bool _bIsTriState)
        :AccessibleBrowseBoxCell(_rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos, BBTYPE_CHECKBOXCELL)
        ,m_eState(_eState)
        ,m_bIsTriState(_bIsTriState)
    {
    }
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleCheckBoxCell, AccessibleBrowseBoxCell, AccessibleCheckBoxCell_BASE )
        // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleCheckBoxCell, AccessibleBrowseBoxCell, AccessibleCheckBoxCell_BASE )
    //--------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleCheckBoxCell::getAccessibleContext(  ) throw (RuntimeException)
    {
        ensureIsAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    ::utl::AccessibleStateSetHelper* AccessibleCheckBoxCell::implCreateStateSetHelper()
    {
        ::utl::AccessibleStateSetHelper* pStateSetHelper =
            AccessibleBrowseBoxCell::implCreateStateSetHelper();
        if( isAlive() )
        {
            mpBrowseBox->FillAccessibleStateSetForCell(
                *pStateSetHelper, getRowPos(), static_cast< sal_uInt16 >( getColumnPos() ) );
            if ( m_eState == STATE_CHECK )
                pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        }
        return pStateSetHelper;
    }
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    // XAccessibleValue
    // -----------------------------------------------------------------------------

    Any SAL_CALL AccessibleCheckBoxCell::getCurrentValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getOslMutex() );

        sal_Int32 nValue = 0;
        switch( m_eState )
        {
            case STATE_NOCHECK:
                nValue = 0;
                break;
            case STATE_CHECK:
                nValue = 1;
                break;
            case STATE_DONTKNOW:
                nValue = 2;
                break;
        }
        return makeAny(nValue);
    }

    // -----------------------------------------------------------------------------

    sal_Bool SAL_CALL AccessibleCheckBoxCell::setCurrentValue( const Any& ) throw (RuntimeException)
    {
        return sal_False;
    }

    // -----------------------------------------------------------------------------

    Any SAL_CALL AccessibleCheckBoxCell::getMaximumValue(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( getOslMutex() );

        Any aValue;

        if ( m_bIsTriState )
            aValue <<= (sal_Int32) 2;
        else
            aValue <<= (sal_Int32) 1;

        return aValue;
    }

    // -----------------------------------------------------------------------------

    Any SAL_CALL AccessibleCheckBoxCell::getMinimumValue(  ) throw (RuntimeException)
    {
        Any aValue;
        aValue <<= (sal_Int32) 0;

        return aValue;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    sal_Int32 SAL_CALL AccessibleCheckBoxCell::getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return 0;
    }
    // -----------------------------------------------------------------------------
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL AccessibleCheckBoxCell::getAccessibleChild( sal_Int32 ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleCheckBoxCell::getImplementationName() throw ( ::com::sun::star::uno::RuntimeException )
    {
        return OUString( "com.sun.star.comp.svtools.TableCheckBoxCell" );
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleCheckBoxCell::getAccessibleIndexInParent()
            throw ( ::com::sun::star::uno::RuntimeException )
    {
        ::osl::MutexGuard aGuard( getOslMutex() );

        return ( getRowPos() * mpBrowseBox->GetColumnCount() ) + getColumnPos();
    }
    // -----------------------------------------------------------------------------
    void AccessibleCheckBoxCell::SetChecked( sal_Bool _bChecked )
    {
        m_eState = _bChecked ? STATE_CHECK : STATE_NOCHECK;
        Any aOldValue, aNewValue;
        if ( _bChecked )
            aNewValue <<= AccessibleStateType::CHECKED;
        else
            aOldValue <<= AccessibleStateType::CHECKED;
        commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
