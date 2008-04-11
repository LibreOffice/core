/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleBrowseBoxCheckBoxCell.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
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
                                sal_Bool _bEnabled,
                                sal_Bool _bIsTriState)
        :AccessibleBrowseBoxCell(_rxParent, _rBrowseBox, _xFocusWindow, _nRowPos, _nColPos, BBTYPE_CHECKBOXCELL)
        ,m_eState(_eState)
        ,m_bEnabled(_bEnabled)
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
    ::rtl::OUString SAL_CALL AccessibleCheckBoxCell::getImplementationName() throw ( ::com::sun::star::uno::RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.TableCheckBoxCell" ) );
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

