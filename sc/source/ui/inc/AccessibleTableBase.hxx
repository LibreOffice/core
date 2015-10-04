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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLETABLEBASE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLETABLEBASE_HXX

#include "AccessibleContextBase.hxx"
#include "global.hxx"
#include "address.hxx"
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <cppuhelper/implbase.hxx>

/** @descr
        This base class provides an implementation of the
        <code>AccessibleTable</code> service.
*/

typedef cppu::ImplHelper < css::accessibility::XAccessibleTable,
                           css::accessibility::XAccessibleSelection >
                    ScAccessibleTableBaseImpl;

class ScAccessibleTableBase :
            public ScAccessibleContextBase,
            public   css::accessibility::XAccessibleTableSelection,
            public ScAccessibleTableBaseImpl
{
public:
    //=====  internal  ========================================================
    ScAccessibleTableBase(
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScDocument* pDoc,
        const ScRange& rRange);
protected:
    virtual ~ScAccessibleTableBase() override;
public:

    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing() override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    ///=====  XAccessibleTable  ================================================

    /// Returns the number of rows in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleRowCount(  )
                    throw (css::uno::RuntimeException,
                           std::exception) override;

    /// Returns the number of columns in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleColumnCount(  )
                    throw (css::uno::RuntimeException,
                           std::exception) override;

    /// Returns the description of the specified row in the table.
    virtual OUString SAL_CALL
                getAccessibleRowDescription( sal_Int32 nRow )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the description text of the specified column in the table.
    virtual OUString SAL_CALL
                getAccessibleColumnDescription( sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /** Returns the number of rows occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of rows the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /** Returns the number of columns occupied by the Accessible at a specified row and column in the table.
    Returns 1 if it is only a cell and the number of columns the cell is merged if the cell is a merged cell.
    */
    virtual sal_Int32 SAL_CALL
                getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the row headers as an AccessibleTable.
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleRowHeaders(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns the column headers as an AccessibleTable.
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                getAccessibleColumnHeaders(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns the selected rows in a table.
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleRows(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns the selected columns in a table.
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
                getSelectedAccessibleColumns(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns a boolean value indicating whether the specified row is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleRowSelected( sal_Int32 nRow )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns a boolean value indicating whether the specified column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the Accessible at a specified row and column in the table.
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the caption for the table.
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleCaption(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns the summary description of the table.
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                getAccessibleSummary(  )
                    throw (css::uno::RuntimeException, std::exception) override;

    /// Returns a boolean value indicating whether the accessible at a specified row and column is selected.
    virtual sal_Bool SAL_CALL
                isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    ///=====  XAccessibleExtendedTable  ========================================

    /// Returns the index of the cell on the given position.
    virtual sal_Int32 SAL_CALL
                getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the row number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleRow( sal_Int32 nChildIndex )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    /// Returns the column number of an index in the table.
    virtual sal_Int32 SAL_CALL
                getAccessibleColumn( sal_Int32 nChildIndex )
                    throw (css::uno::RuntimeException,
                           css::lang::IndexOutOfBoundsException,
                           std::exception) override;

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    /// override to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
                    throw (css::uno::RuntimeException,
                           std::exception) override;

    /// Return the specified child or NULL if index is invalid.
    /// override to calculate this on demand
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
               css::lang::IndexOutOfBoundsException,
               std::exception) override;
    virtual sal_Bool SAL_CALL selectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL selectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL unselectRow( sal_Int32 row )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL unselectColumn( sal_Int32 column )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException,
               std::exception) override;

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName()
        throw (css::uno::RuntimeException,
               std::exception) override;

public:
    /// Return NULL to indicate that an empty relation set.
    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    // perhaps sometimes to be implemented
    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL
        selectAccessibleChild( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    virtual sal_Bool SAL_CALL
        isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    virtual void SAL_CALL
        clearAccessibleSelection(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        selectAllAccessibleChildren(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL
        getSelectedAccessibleChildCount(  )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
        css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (css::lang::IndexOutOfBoundsException,
               css::uno::RuntimeException,
               std::exception) override;

    ///=====  XServiceInfo  ===================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /// returns the possible types
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    /// contains the range of the table, because it could be a subrange of the complete table
    ScRange maRange;

    ScDocument* mpDoc;

    void CommitTableModelChange(sal_Int32 nStartRow, sal_Int32 nStartCol, sal_Int32 nEndRow, sal_Int32 nEndCol, sal_uInt16 nId);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
