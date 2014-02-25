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


#include <editeng/AccessibleSelectionBase.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility
{

    // - AccessibleSelectionBase -


    AccessibleSelectionBase::AccessibleSelectionBase()
    {
    }



    AccessibleSelectionBase::~AccessibleSelectionBase()
    {
    }



    void SAL_CALL AccessibleSelectionBase::selectAccessibleChild( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::selectAccessibleChild( nChildIndex );
    }



    sal_Bool SAL_CALL AccessibleSelectionBase::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::isAccessibleChildSelected( nChildIndex ) );
    }



    void SAL_CALL AccessibleSelectionBase::clearAccessibleSelection(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::clearAccessibleSelection();
    }



    void SAL_CALL AccessibleSelectionBase::selectAllAccessibleChildren(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::selectAllAccessibleChildren();
    }



    sal_Int32 SAL_CALL AccessibleSelectionBase::getSelectedAccessibleChildCount(  ) throw (uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::getSelectedAccessibleChildCount() );
    }



    uno::Reference< XAccessible > SAL_CALL AccessibleSelectionBase::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        return( OCommonAccessibleSelection::getSelectedAccessibleChild( nSelectedChildIndex ) );
    }



    void SAL_CALL AccessibleSelectionBase::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
    {
        ::osl::MutexGuard aGuard( implGetMutex() );
        OCommonAccessibleSelection::deselectAccessibleChild( nSelectedChildIndex );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
