/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_TABLE_TABLEMODEL_HXX_
#define _SVX_TABLE_TABLEMODEL_HXX_

#include <com/sun/star/util/XBroadcaster.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/listenernotification.hxx>
#include <tools/gen.hxx>
#include "celltypes.hxx"

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

class SdrTableObj;

// -----------------------------------------------------------------------------
// ICellRange
// -----------------------------------------------------------------------------

/** base class for each object implementing an XCellRange */
class ICellRange
{
public:
    virtual sal_Int32 getLeft() = 0;
    virtual sal_Int32 getTop() = 0;
    virtual sal_Int32 getRight() = 0;
    virtual sal_Int32 getBottom() = 0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > getTable() = 0;
};

// -----------------------------------------------------------------------------
// TableModel
// -----------------------------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::table::XTable, ::com::sun::star::util::XBroadcaster > TableModelBase;

class TableModel : public TableModelBase,
                   public ::comphelper::OBaseMutex,
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
    TableModel( SdrTableObj* pTableObj );
    TableModel( SdrTableObj* pTableObj, const TableModelRef& xSourceTable );
    virtual ~TableModel();

    void init( sal_Int32 nColumns, sal_Int32 nRows );

    SdrTableObj* getSdrTableObj() const { return mpTableObj; }

    /** deletes rows and columns that are completly merged. Must be called between BegUndo/EndUndo! */
    void optimize();

    /// merges the cell at the given position with the given span
    void merge( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan );

    // ICellRange
    virtual sal_Int32 getLeft();
    virtual sal_Int32 getTop();
    virtual sal_Int32 getRight();
    virtual sal_Int32 getBottom();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTable > getTable();

    // XTable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellCursor > SAL_CALL createCursor(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellCursor > SAL_CALL createCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >& Range ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getColumnCount() throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XModifiable
    virtual ::sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( ::sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XColumnRowRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL getColumns() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL getRows() throw (::com::sun::star::uno::RuntimeException);

    // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( ::sal_Int32 nColumn, ::sal_Int32 nRow ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( ::sal_Int32 nLeft, ::sal_Int32 nTop, ::sal_Int32 nRight, ::sal_Int32 nBottom ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const ::rtl::OUString& aRange ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( ::sal_Int32 nHandle, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XBroadcaster
    virtual void SAL_CALL lockBroadcasts() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL unlockBroadcasts() throw (::com::sun::star::uno::RuntimeException);

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
    virtual void SAL_CALL disposing();

    TableRowRef getRow( sal_Int32 nRow ) const throw (::com::sun::star::lang::IndexOutOfBoundsException);
    TableColumnRef getColumn( sal_Int32 nColumn ) const throw (::com::sun::star::lang::IndexOutOfBoundsException);

    void updateRows();
    void updateColumns();

    RowVector       maRows;
    ColumnVector    maColumns;

    TableColumnsRef mxTableColumns;
    TableRowsRef mxTableRows;

    SdrTableObj* mpTableObj;

    sal_Bool mbModified;
    bool mbNotifyPending;

    sal_Int32 mnNotifyLock;
};

class TableModelNotifyGuard
{
public:
    TableModelNotifyGuard( TableModel* pModel )
    : mxBroadcaster( static_cast< ::com::sun::star::util::XBroadcaster* >( pModel ) )
    {
        if( mxBroadcaster.is() )
            mxBroadcaster->lockBroadcasts();
    }

    TableModelNotifyGuard( ::com::sun::star::uno::XInterface* pInterface )
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
