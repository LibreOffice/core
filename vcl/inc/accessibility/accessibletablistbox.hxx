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

#pragma once

#include <accessibility/AccessibleBrowseBox.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/accessibletableprovider.hxx>

class AccessibleBrowseBoxTable;
class SvHeaderTabListBox;

/** !!! */
class AccessibleTabListBox final
    : public cppu::ImplInheritanceHelper<AccessibleBrowseBox, css::accessibility::XAccessible>
{
private:
    VclPtr<SvHeaderTabListBox>        m_pTabListBox;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBox(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int64 nChildIndex ) override;

    // XAccessibleContext
    css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext() override;

    // IAccessibleTabListBox

    css::uno::Reference< css::accessibility::XAccessible >
        getHeaderBar()
    {
        return AccessibleBrowseBox::getHeaderBar( AccessibleBrowseBoxObjType::ColumnHeaderBar );
    }

private:
    /** dtor() */
    virtual ~AccessibleTabListBox() override;

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual rtl::Reference<AccessibleBrowseBoxTable> createAccessibleTable() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
