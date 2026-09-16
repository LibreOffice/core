/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "AccessibleContextBase.hxx"
#include <address.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <cppuhelper/implbase.hxx>

class ScAccessibleTableBase
    : public cppu::ImplInheritanceHelper<
          ScAccessibleContextBase, css::accessibility::XAccessibleTableSelection,
          css::accessibility::XAccessibleTable, css::accessibility::XAccessibleSelection>
{
public:
    ScAccessibleTableBase(
        const cpo::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScRange& rRange);
protected:
    virtual ~ScAccessibleTableBase() override;
public:

    using ScAccessibleContextBase::disposing;
     virtual void disposing() override;

    ///=====  XAccessibleTable  ================================================

    /// Returns the number of rows in the table.
    virtual sal_Int32
                getAccessibleRowCount(  ) override;

    /// Returns the number of columns in the table.
    virtual sal_Int32
                getAccessibleColumnCount(  ) override;

    /// Returns the description of the specified row in the table.
    virtual OUString
                getAccessibleRowDescription( sal_Int32 nRow ) override;

    /// Returns the description text of the specified column in the table.
    virtual OUString
                getAccessibleColumnDescription( sal_Int32 nColumn ) override;

    /** Returns the number of rows occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of rows the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32
                getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** Returns the number of columns occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of columns the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32
                getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /// Returns the row headers as an AccessibleTable.
    virtual cpo::uno::Reference< css::accessibility::XAccessibleTable >
                getAccessibleRowHeaders(  ) override;

    /// Returns the column headers as an AccessibleTable.
    virtual cpo::uno::Reference< css::accessibility::XAccessibleTable >
                getAccessibleColumnHeaders(  ) override;

    /// Returns the selected rows in a table.
    virtual cpo::uno::Sequence< sal_Int32 >
                getSelectedAccessibleRows(  ) override;

    /// Returns the selected columns in a table.
    virtual cpo::uno::Sequence< sal_Int32 >
                getSelectedAccessibleColumns(  ) override;

    /// Returns a boolean value indicating whether the specified row is selected.
    virtual bool
                isAccessibleRowSelected( sal_Int32 nRow ) override;

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual bool
                isAccessibleColumnSelected( sal_Int32 nColumn ) override;

    /// Returns the Accessible at a specified row and column in the table.
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /// Returns the caption for the table.
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                getAccessibleCaption(  ) override;

    /// Returns the summary description of the table.
    virtual cpo::uno::Reference< css::accessibility::XAccessible >
                getAccessibleSummary(  ) override;

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual bool
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn ) override;

    ///=====  XAccessibleExtendedTable  ========================================

    /// Returns the index of the cell on the given position.
    virtual sal_Int64
                getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /// Returns the row number of an index in the table.
    virtual sal_Int32
                getAccessibleRow( sal_Int64 nChildIndex ) override;

    /// Returns the column number of an index in the table.
    virtual sal_Int32
                getAccessibleColumn( sal_Int64 nChildIndex ) override;

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int64
        getAccessibleChildCount() override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual cpo::uno::Reference< css::accessibility::XAccessible>
        getAccessibleChild(sal_Int64 nIndex) override;
    virtual bool selectRow( sal_Int32 row ) override;
    virtual bool selectColumn( sal_Int32 column ) override;
    virtual bool unselectRow( sal_Int32 row ) override;
    virtual bool unselectColumn( sal_Int32 column ) override;

protected:
    /// Return this object's description.
    virtual OUString
        createAccessibleDescription() override;

    /// Return the object's current name.
    virtual OUString
        createAccessibleName() override;

public:
    /// Return NULL to indicate that an empty relation set.
    virtual cpo::uno::Reference<css::accessibility::XAccessibleRelationSet>
        getAccessibleRelationSet() override;

    /// Return the set of current states.
    // perhaps sometimes to be implemented
    virtual sal_Int64
        getAccessibleStateSet() override;

    ///=====  XAccessibleSelection  ===========================================

    virtual void
        selectAccessibleChild( sal_Int64 nChildIndex ) override;

    virtual bool
        isAccessibleChildSelected( sal_Int64 nChildIndex ) override;

    virtual void
        clearAccessibleSelection(  ) override;

    virtual void
        selectAllAccessibleChildren(  ) override;

    virtual sal_Int64
        getSelectedAccessibleChildCount(  ) override;

    virtual cpo::uno::Reference< css::accessibility::XAccessible >
        getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    virtual void
        deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

protected:
    /// contains the range of the table, because it could be a subrange of the complete table
    ScRange maRange;

    ScDocument* mpDoc;

    void CommitTableModelChange(sal_Int32 nStartRow, sal_Int32 nStartCol, sal_Int32 nEndRow, sal_Int32 nEndCol, sal_uInt16 nId);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
