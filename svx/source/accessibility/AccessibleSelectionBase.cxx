/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleSelectionBase.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_svx.hxx"

#include "AccessibleSelectionBase.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{
    // ---------------------------
    // - AccessibleSelectionBase -
    // ---------------------------

    AccessibleSelectionBase::AccessibleSelectionBase()
    {
    }

    //--------------------------------------------------------------------

    AccessibleSelectionBase::~AccessibleSelectionBase()
    {
    }

    //--------------------------------------------------------------------

    void SAL_CALL AccessibleSelectionBase::selectAccessibleChild( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::selectAccessibleChild( nChildIndex );
    }

    //--------------------------------------------------------------------

    sal_Bool SAL_CALL AccessibleSelectionBase::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::isAccessibleChildSelected( nChildIndex ) );
    }

    //--------------------------------------------------------------------

    void SAL_CALL AccessibleSelectionBase::clearAccessibleSelection(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::clearAccessibleSelection();
    }

    //--------------------------------------------------------------------

    void SAL_CALL AccessibleSelectionBase::selectAllAccessibleChildren(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::selectAllAccessibleChildren();
    }

    //--------------------------------------------------------------------

    sal_Int32 SAL_CALL AccessibleSelectionBase::getSelectedAccessibleChildCount(  ) throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::getSelectedAccessibleChildCount() );
    }

    //--------------------------------------------------------------------

    uno::Reference< XAccessible > SAL_CALL AccessibleSelectionBase::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::getSelectedAccessibleChild( nSelectedChildIndex ) );
    }

    //--------------------------------------------------------------------

    void SAL_CALL AccessibleSelectionBase::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::deselectAccessibleChild( nSelectedChildIndex );
    }
}
