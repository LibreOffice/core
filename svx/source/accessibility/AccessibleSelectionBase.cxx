/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleSelectionBase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:01:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
