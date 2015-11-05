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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCSELECTIONHELPER_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCSELECTIONHELPER_HXX

class SwAccessibleContext;
class SwFEShell;

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SwAccessibleSelectionHelper
{
    /// the context on which this helper works
    SwAccessibleContext& m_rContext;

    /// get FE-Shell
    SwFEShell* GetFEShell();

    void throwIndexOutOfBoundsException()
        throw ( css::lang::IndexOutOfBoundsException );

public:
    SwAccessibleSelectionHelper( SwAccessibleContext& rContext );
    ~SwAccessibleSelectionHelper();

    // XAccessibleSelection

    void selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException );

    bool isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException );
    void selectAllAccessibleChildren(  )
        throw ( css::uno::RuntimeException );
    sal_Int32 getSelectedAccessibleChildCount(  )
        throw ( css::uno::RuntimeException );
    css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException);
    // index has to be treated as global child index.
    void deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
