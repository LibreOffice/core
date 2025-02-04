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

#include <accessibility/AccessibleBrowseBoxTable.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SvHeaderTabListBox;

class AccessibleTabListBoxTable final
    : public cppu::ImplInheritanceHelper<AccessibleBrowseBoxTable,
                                         css::accessibility::XAccessibleSelection>
{
private:
    VclPtr<SvHeaderTabListBox>     m_pTabListBox;

    css::uno::Reference< css::accessibility::XAccessible >  m_xCurChild;

    void                    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    DECL_LINK(        WindowEventListener, VclWindowEvent&, void );

    // helpers ----------------------------------------------------------------

    /** Throws an exception, if nIndex is not a valid child index. */
    void ensureValidIndex( sal_Int64 _nIndex ) const;

    /** Selects the specified row. */
    void implSelectRow( sal_Int32 _nRow, bool _bSelect );

    /** Returns the count of rows in the table. */
    sal_Int32 implGetRowCount() const override;
    /** Returns the total column count in the table. */
    sal_Int32 implGetColumnCount() const override;
    /** Returns the count of selected rows in the table. */
    sal_Int32 implGetSelRowCount() const;

    /** Returns the row index from cell index. */
    sal_Int32 implGetRow( sal_Int64 _nIndex ) const
    {
        auto nColCount = implGetColumnCount();
        assert(nColCount != 0);
        return _nIndex / nColCount;
    }

    /** Returns the absolute row index of the nSelRow-th selected row. */
    sal_Int32 implGetSelRow( sal_Int32 _nSelRow ) const;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBoxTable(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

private:
    /** dtor() */
    virtual ~AccessibleTabListBoxTable() override;

public:
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    void SAL_CALL clearAccessibleSelection(  ) override;
    void SAL_CALL selectAllAccessibleChildren(  ) override;
    sal_Int64 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    void SAL_CALL deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
