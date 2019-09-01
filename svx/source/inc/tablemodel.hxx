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

#ifndef INCLUDED_SVX_SOURCE_INC_TABLEMODEL_HXX
#define INCLUDED_SVX_SOURCE_INC_TABLEMODEL_HXX

#include <sal/types.h>
#include <com/sun/star/util/XBroadcaster.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include "celltypes.hxx"

struct _xmlTextWriter;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sdr { namespace table {

class SdrTableObj;

/** base class for each object implementing an XCellRange */
class ICellRange
{
public:
    virtual sal_Int32 getLeft() = 0;
    virtual sal_Int32 getTop() = 0;
    virtual sal_Int32 getRight() = 0;
    virtual sal_Int32 getBottom() = 0;
    virtual css::uno::Reference< css::table::XTable > getTable() = 0;

protected:
    ~ICellRange() {}
};

typedef ::cppu::WeakComponentImplHelper< css::table::XTable, css::util::XBroadcaster > TableModelBase;

class TableModel : public ::cppu::BaseMutex,
                   public TableModelBase,
                   public ICellRange
{
    friend class InsertRowUndo;
    friend class RemoveRowUndo;
    friend class InsertColUndo;
    friend class RemoveColUndo;
    friend class TableColumnUndo;
    friend class TableRowUndo;
    friend class TableColumn;
    friend class TableRow;
    friend class TableRows;
    friend class TableColumns;
    friend class TableModelNotifyGuard;

public:
    explicit TableModel( SdrTableObj* pTableObj );
    TableModel( SdrTableObj* pTableObj, const TableModelRef& xSourceTable );
    virtual ~TableModel() override;

    void init( sal_Int32 nColumns, sal_Int32 nRows );

    SdrTableObj* getSdrTableObj() const { return mpTableObj; }

    /** deletes rows and columns that are completely merged. Must be called between BegUndo/EndUndo! */
    void optimize();

    /// merges the cell at the given position with the given span
    void merge( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan );
    /// Get the width of all columns in this table.
    std::vector<sal_Int32> getColumnWidths();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    // ICellRange
    virtual sal_Int32 getLeft() override;
    virtual sal_Int32 getTop() override;
    virtual sal_Int32 getRight() override;
    virtual sal_Int32 getBottom() override;
    virtual css::uno::Reference< css::table::XTable > getTable() override;

    // XTable
    virtual css::uno::Reference< css::table::XCellCursor > SAL_CALL createCursor(  ) override;
    virtual css::uno::Reference< css::table::XCellCursor > SAL_CALL createCursorByRange( const css::uno::Reference< css::table::XCellRange >& rRange ) override;
    virtual ::sal_Int32 SAL_CALL getRowCount() override;
    virtual ::sal_Int32 SAL_CALL getColumnCount() override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

    // XColumnRowRange
    virtual css::uno::Reference< css::table::XTableColumns > SAL_CALL getColumns() override;
    virtual css::uno::Reference< css::table::XTableRows > SAL_CALL getRows() override;

    // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByPosition( ::sal_Int32 nColumn, ::sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByPosition( ::sal_Int32 nLeft, ::sal_Int32 nTop, ::sal_Int32 nRight, ::sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) override;

    // XBroadcaster
    virtual void SAL_CALL lockBroadcasts() override;
    virtual void SAL_CALL unlockBroadcasts() override;

protected:
    void notifyModification();

    void insertColumns( sal_Int32 nIndex, sal_Int32 nCount );
    void removeColumns( sal_Int32 nIndex, sal_Int32 nCount );
    void insertRows( sal_Int32 nIndex, sal_Int32 nCount );
    void removeRows( sal_Int32 nIndex, sal_Int32 nCount );

    sal_Int32 getRowCountImpl() const;
    sal_Int32 getColumnCountImpl() const;

    CellRef createCell();
    CellRef getCell( ::sal_Int32 nCol, ::sal_Int32 nRow ) const;

    void UndoInsertRows( sal_Int32 nIndex, sal_Int32 nCount );
    void UndoRemoveRows( sal_Int32 nIndex, RowVector& aNewRows );

    void UndoInsertColumns( sal_Int32 nIndex, sal_Int32 nCount );
    void UndoRemoveColumns( sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells );

private:
    /** this function is called upon disposing the component
    */
    virtual void SAL_CALL disposing() override;

    /// @throws css::lang::IndexOutOfBoundsException
    TableRowRef const & getRow( sal_Int32 nRow ) const;
    /// @throws css::lang::IndexOutOfBoundsException
    TableColumnRef const & getColumn( sal_Int32 nColumn ) const;

    void updateRows();
    void updateColumns();

    RowVector       maRows;
    ColumnVector    maColumns;

    rtl::Reference< TableColumns > mxTableColumns;
    rtl::Reference< TableRows >    mxTableRows;

    SdrTableObj* mpTableObj; // TTTT should be reference

    bool mbModified;
    bool mbNotifyPending;

    sal_Int32 mnNotifyLock;
};

class TableModelNotifyGuard
{
public:
    explicit TableModelNotifyGuard( TableModel* pModel )
    : mxBroadcaster( static_cast< css::util::XBroadcaster* >( pModel ) )
    {
        if( mxBroadcaster.is() )
            mxBroadcaster->lockBroadcasts();
    }

    ~TableModelNotifyGuard()
    {
        if( mxBroadcaster.is() )
            mxBroadcaster->unlockBroadcasts();
    }

private:
    css::uno::Reference< css::util::XBroadcaster > mxBroadcaster;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
