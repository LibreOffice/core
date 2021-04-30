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

#include <extended/AccessibleBrowseBoxBase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>


namespace accessibility {

typedef ::cppu::ImplHelper1< css::accessibility::XAccessibleTable >
        AccessibleBrowseBoxTableImplHelper;

/** The BrowseBox accessible table objects inherit from this base class. It
    implements basic functionality for the XAccessibleTable interface.
    BrowseBox table objects are: the data table, the column header bar and the
    row header bar. */
class AccessibleBrowseBoxTableBase :
    public BrowseBoxAccessibleElement,
    public AccessibleBrowseBoxTableImplHelper
{
public:
    /** Constructor sets specified name and description. If the constant of a
        text is BBTEXT_NONE, the derived class has to set the text via
        implSetName() and implSetDescription() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).
        @param rxParent         XAccessible interface of the parent object.
        @param rBrowseBox       The BrowseBox control.
        @param eObjType         Type of BrowseBox object. */
    AccessibleBrowseBoxTableBase(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        ::vcl::IAccessibleTableProvider& rBrowseBox,
        ::vcl::AccessibleBrowseBoxObjType eObjType );

protected:
    virtual ~AccessibleBrowseBoxTableBase() override = default;

public:
    // XAccessibleContext

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /** @return  The role of this object (a table). */
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    /*  Derived classes have to implement:
        -   getAccessibleChild,
        -   getAccessibleIndexInParent. */

    // XAccessibleComponent

    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus. */

    // XAccessibleTable

    /** @return  The number of used rows in the table (0 = empty table). */
    virtual sal_Int32 SAL_CALL getAccessibleRowCount() override;

    /** @return  The number of used columns in the table (0 = empty table). */
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount() override;

    /** @return  The row extent of the specified cell (always 1). */
    virtual sal_Int32 SAL_CALL
    getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** @return  The column extent of the specified cell (always 1). */
    virtual sal_Int32 SAL_CALL
    getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** @return  The caption cell of the table (not supported). */
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
    getAccessibleCaption() override;

    /** @return  The summary object of the table (not supported). */
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
    getAccessibleSummary() override;

    /** @return  The child index of the specified cell. */
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn ) override;

    /** @return  The row index of the specified child cell. */
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex ) override;

    /** @return  The column index of the specified child cell. */
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex ) override;

    /*  Derived classes have to implement:
        -   getAccessibleRowDescription,
        -   getAccessibleColumnDescription,
        -   getAccessibleRowHeaders,
        -   getAccessibleColumnHeaders,
        -   getSelectedAccessibleRows,
        -   getSelectedAccessibleColumns,
        -   isAccessibleRowSelected,
        -   isAccessibleColumnSelected,
        -   getAccessibleCellAt,
        -   isAccessibleSelected. */

    // XInterface

    /** Queries for a new interface. */
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;

    /** Acquires the object (calls acquire() on base class). */
    virtual void SAL_CALL acquire() noexcept override;

    /** Releases the object (calls release() on base class). */
    virtual void SAL_CALL release() noexcept override;

    // XTypeProvider

    /** @return  a sequence of possible types (received from base classes). */
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    /** @return  a unique implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo

    /*  Derived classes have to implement:
        -   getImplementationName */

protected:
    // internal virtual methods

    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of data rows without header bar. */
    virtual sal_Int32 implGetRowCount() const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of data columns without "handle column". */
    virtual sal_Int32 implGetColumnCount() const;

    // internal helper methods

    /** @return  TRUE, if first BrowseBox column is the "handle column". */
    bool implHasHandleColumn() const;

    /** @attention  This method requires locked mutex's and a living object.
        @param nColumn
            the position of the column in the Accessible world
        @return
            the position of the column in VCL the Accessible world
    */
    sal_uInt16 implToVCLColumnPos( sal_Int32 nColumn ) const;

    /** @attention  This method requires locked mutex's and a living object.
        @return  The number of cells of the table. */
    sal_Int32 implGetChildCount() const;

    /** @attention  This method requires locked mutex's and a living object.
        @return  The row index of the specified cell index. */
    sal_Int32 implGetRow( sal_Int32 nChildIndex ) const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The column index of the specified cell index. */
    sal_Int32 implGetColumn( sal_Int32 nChildIndex ) const;

    /** @attention  This method requires locked mutex's and a living object.
        @return  TRUE, if the specified row is selected. */
    bool implIsRowSelected( sal_Int32 nRow ) const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  TRUE, if the specified column is selected. */
    bool implIsColumnSelected( sal_Int32 nColumn ) const;

    /** Selects/deselects a row (tries to expand selection).
        @attention  This method requires locked mutex's and a living object.
        @param nRow     Row to select
        @param bSelect  TRUE = select, FALSE = deselect */
    void implSelectRow( sal_Int32 nRow, bool bSelect );
    /** Selects/deselects a column (tries to expand selection).
        @attention  This method requires locked mutex's and a living object.
        @param nColumnPos column number to select
        @param bSelect  TRUE = select, FALSE = deselect */
    void implSelectColumn( sal_Int32 nColumnPos, bool bSelect );

    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of selected rows. */
    sal_Int32 implGetSelectedRowCount() const;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The count of selected columns. */
    sal_Int32 implGetSelectedColumnCount() const;

    /** Fills a sequence with sorted indexes of completely selected rows.
        @attention  This method requires locked mutex's and a living object.
        @param rSeq  Out-parameter that takes the sorted row index list. */
    void implGetSelectedRows( css::uno::Sequence< sal_Int32 >& rSeq );
    /** Fills a sequence with sorted indexes of completely selected columns.
        @attention  This method requires locked mutex's and a living object.
        @param rSeq  Out-parameter that takes the sorted column index list. */
    void implGetSelectedColumns( css::uno::Sequence< sal_Int32 >& rSeq );

    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified row index is invalid. */
    void ensureIsValidRow( sal_Int32 nRow );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified column index is invalid. */
    void ensureIsValidColumn( sal_Int32 nColumn );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified cell address is invalid. */
    void ensureIsValidAddress( sal_Int32 nRow, sal_Int32 nColumn );
    /** @attention  This method requires locked mutex's and a living object.
        @throws <type>IndexOutOfBoundsException</type>
        If the specified child index is invalid. */
    void ensureIsValidIndex( sal_Int32 nChildIndex );
};


} // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
