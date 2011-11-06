/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <editeng/AccessibleSelectionBase.hxx>

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
