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

#ifndef INCLUDED_SVX_SOURCE_TABLE_TABLEMODEL_HXX
#define INCLUDED_SVX_SOURCE_TABLE_TABLEMODEL_HXX

#include <sal/types.h>
#include <com/sun/star/util/XBroadcaster.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/listenernotification.hxx>
#include "celltypes.hxx"



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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > getTable() = 0;

protected:
    ~ICellRange() {}
};

typedef ::cppu::WeakComponentImplHelper< ::com::sun::star::table::XTable, ::com::sun::star::util::XBroadcaster > TableModelBase;

class TableModel : public ::comphelper::OBaseMutex,
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
    virtual ~TableModel();

    void init( sal_Int32 nColumns, sal_Int32 nRows );

    SdrTableObj* getSdrTableObj() const { return mpTableObj; }

    /** deletes rows and columns that are completely merged. Must be called between BegUndo/EndUndo! */
    void optimize();

    /// merges the cell at the given position with the given span
    void merge( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan );

    // ICellRange
    virtual sal_Int32 getLeft() override;
    virtual sal_Int32 getTop() override;
    virtual sal_Int32 getRight() override;
    virtual sal_Int32 getBottom() override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > getTable() override;

    // XTable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellCursor > SAL_CALL createCursor(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellCursor > SAL_CALL createCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >& rRange ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XColumnRowRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL getColumns() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL getRows() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( ::sal_Int32 nColumn, ::sal_Int32 nRow ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( ::sal_Int32 nLeft, ::sal_Int32 nTop, ::sal_Int32 nRight, ::sal_Int32 nBottom ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XBroadcaster
    virtual void SAL_CALL lockBroadcasts() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unlockBroadcasts() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

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

    TableRowRef getRow( sal_Int32 nRow ) const throw (::com::sun::star::lang::IndexOutOfBoundsException);
    TableColumnRef getColumn( sal_Int32 nColumn ) const throw (::com::sun::star::lang::IndexOutOfBoundsException);

    void updateRows();
    void updateColumns();

    RowVector       maRows;
    ColumnVector    maColumns;

    rtl::Reference< TableColumns > mxTableColumns;
    rtl::Reference< TableRows >    mxTableRows;

    SdrTableObj* mpTableObj;

    bool mbModified;
    bool mbNotifyPending;

    sal_Int32 mnNotifyLock;
};

class TableModelNotifyGuard
{
public:
    explicit TableModelNotifyGuard( TableModel* pModel )
    : mxBroadcaster( static_cast< ::com::sun::star::util::XBroadcaster* >( pModel ) )
    {
        if( mxBroadcaster.is() )
            mxBroadcaster->lockBroadcasts();
    }

    explicit TableModelNotifyGuard( ::com::sun::star::uno::XInterface* pInterface )
    : mxBroadcaster( pInterface, ::com::sun::star::uno::UNO_QUERY )
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
    com::sun::star::uno::Reference< ::com::sun::star::util::XBroadcaster > mxBroadcaster;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
