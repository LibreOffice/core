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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>

#include <algorithm>

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <libxml/xmlwriter.h>

#include <cell.hxx>
#include "cellcursor.hxx"
#include <tablemodel.hxx>
#include "tablerow.hxx"
#include "tablerows.hxx"
#include "tablecolumn.hxx"
#include "tablecolumns.hxx"
#include "tableundo.hxx"
#include <o3tl/safeint.hxx>
#include <svx/svdotable.hxx>
#include <svx/svdmodel.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;


namespace sdr { namespace table {


// removes the given range from a vector
template< class Vec, class Iter > static void remove_range( Vec& rVector, sal_Int32 nIndex, sal_Int32 nCount )
{
    const sal_Int32 nSize = static_cast<sal_Int32>(rVector.size());
    if( nCount && (nIndex >= 0) && (nIndex < nSize) )
    {
        if( (nIndex + nCount) >= nSize )
        {
            // remove at end
            rVector.resize( nIndex );
        }
        else
        {
            rVector.erase(rVector.begin() + nIndex, rVector.begin() + nIndex + nCount);
        }
    }
}


/** inserts a range into a vector */
template< class Vec, class Iter, class Entry > static sal_Int32 insert_range( Vec& rVector, sal_Int32 nIndex, sal_Int32 nCount )
{
    if( nCount )
    {
        if( nIndex >= static_cast< sal_Int32 >( rVector.size() ) )
        {
            // append at end
            nIndex = static_cast< sal_Int32 >( rVector.size() ); // cap to end
            rVector.resize( nIndex + nCount );
        }
        else
        {
            // insert
            Iter aIter( rVector.begin() );
            std::advance( aIter, nIndex );

            Entry aEmpty;
            rVector.insert( aIter, nCount, aEmpty );
        }
    }
    return nIndex;
}


TableModel::TableModel( SdrTableObj* pTableObj )
: TableModelBase( m_aMutex  )
, mpTableObj( pTableObj )
, mbModified( false )
, mbNotifyPending( false )
, mnNotifyLock( 0 )
{
}

TableModel::TableModel( SdrTableObj* pTableObj, const TableModelRef& xSourceTable )
: TableModelBase( m_aMutex  )
, mpTableObj( pTableObj )
, mbModified( false )
, mbNotifyPending( false )
, mnNotifyLock( 0 )
{
    if( xSourceTable.is() )
    {
        const sal_Int32 nColCount = xSourceTable->getColumnCountImpl();
        const sal_Int32 nRowCount = xSourceTable->getRowCountImpl();

        init( nColCount, nRowCount );

        sal_Int32 nRows = nRowCount;
        while( nRows-- )
            (*maRows[nRows]) = *xSourceTable->maRows[nRows];

        sal_Int32 nColumns = nColCount;
        while( nColumns-- )
            (*maColumns[nColumns]) = *xSourceTable->maColumns[nColumns];

        // copy cells
        for( sal_Int32 nCol = 0; nCol < nColCount; ++nCol )
        {
            for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
            {
                CellRef xTargetCell( getCell( nCol, nRow ) );
                if( xTargetCell.is() )
                    xTargetCell->cloneFrom( xSourceTable->getCell( nCol, nRow ) );
            }
        }
    }
}


TableModel::~TableModel()
{
}


void TableModel::init( sal_Int32 nColumns, sal_Int32 nRows )
{
    if( nRows < 20 )
        maRows.reserve( 20 );

    if( nColumns < 20 )
        maColumns.reserve( 20 );

    if( nRows && nColumns )
    {
        maColumns.resize( nColumns );
        maRows.resize( nRows );

        while( nRows-- )
            maRows[nRows].set( new TableRow( this, nRows, nColumns ) );

        while( nColumns-- )
            maColumns[nColumns].set( new TableColumn( this, nColumns ) );
    }
}


// ICellRange


sal_Int32 TableModel::getLeft()
{
    return 0;
}


sal_Int32 TableModel::getTop()
{
    return 0;
}


sal_Int32 TableModel::getRight()
{
    return getColumnCount();
}


sal_Int32 TableModel::getBottom()
{
    return getRowCount();
}


Reference< XTable > TableModel::getTable()
{
    return this;
}


void TableModel::UndoInsertRows( sal_Int32 nIndex, sal_Int32 nCount )
{
    TableModelNotifyGuard aGuard( this );

    // remove the rows
    remove_range<RowVector,RowVector::iterator>( maRows, nIndex, nCount );
    updateRows();
    setModified(true);
}


void TableModel::UndoRemoveRows( sal_Int32 nIndex, RowVector& aRows )
{
    TableModelNotifyGuard aGuard( this );

    const sal_Int32 nCount = sal::static_int_cast< sal_Int32 >( aRows.size() );

    nIndex = insert_range<RowVector,RowVector::iterator,TableRowRef>( maRows, nIndex, nCount );

    for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
        maRows[nIndex+nOffset] = aRows[nOffset];

    updateRows();
    setModified(true);
}


void TableModel::UndoInsertColumns( sal_Int32 nIndex, sal_Int32 nCount )
{
    TableModelNotifyGuard aGuard( this );

    // now remove the columns
    remove_range<ColumnVector,ColumnVector::iterator>( maColumns, nIndex, nCount );
    sal_Int32 nRows = getRowCountImpl();
    while( nRows-- )
        maRows[nRows]->removeColumns( nIndex, nCount );

    updateColumns();
    setModified(true);
}


void TableModel::UndoRemoveColumns( sal_Int32 nIndex, ColumnVector& aCols, CellVector& aCells )
{
    TableModelNotifyGuard aGuard( this );

    const sal_Int32 nCount = sal::static_int_cast< sal_Int32 >( aCols.size() );

    // assert if there are not enough cells saved
    DBG_ASSERT( (aCols.size() * maRows.size()) == aCells.size(), "sdr::table::TableModel::UndoRemoveColumns(), invalid undo data!" );

    nIndex = insert_range<ColumnVector,ColumnVector::iterator,TableColumnRef>( maColumns, nIndex, nCount );
    for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
        maColumns[nIndex+nOffset] = aCols[nOffset];

    CellVector::iterator aIter( aCells.begin() );

    sal_Int32 nRows = getRowCountImpl();
    for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
    {
        CellVector::iterator aIter2 = aIter + nRow * nCount;
        OSL_ENSURE(aIter2 < aCells.end(), "invalid iterator!");
        maRows[nRow]->insertColumns( nIndex, nCount, &aIter2 );
    }

    updateColumns();
    setModified(true);
}


// XTable


Reference< XCellCursor > SAL_CALL TableModel::createCursor()
{
    ::SolarMutexGuard aGuard;
    return createCursorByRange( Reference< XCellRange >( this ) );
}


Reference< XCellCursor > SAL_CALL TableModel::createCursorByRange( const Reference< XCellRange >& rRange )
{
    ::SolarMutexGuard aGuard;

    ICellRange* pRange = dynamic_cast< ICellRange* >( rRange.get() );
    if( (pRange == nullptr) || (pRange->getTable().get() != this) )
        throw IllegalArgumentException();

    TableModelRef xModel( this );
    return new CellCursor( xModel, pRange->getLeft(), pRange->getTop(), pRange->getRight(), pRange->getBottom() );
}


sal_Int32 SAL_CALL TableModel::getRowCount()
{
    ::SolarMutexGuard aGuard;
    return getRowCountImpl();
}


sal_Int32 SAL_CALL TableModel::getColumnCount()
{
    ::SolarMutexGuard aGuard;
    return getColumnCountImpl();
}

std::vector<sal_Int32> TableModel::getColumnWidths()
{
    std::vector<sal_Int32> aRet;
    for (const TableColumnRef& xColumn : maColumns)
        aRet.push_back(xColumn->getWidth());
    return aRet;
}

// XComponent


void TableModel::dispose()
{
    ::SolarMutexGuard aGuard;
    TableModelBase::dispose();
}


// XModifiable


sal_Bool SAL_CALL TableModel::isModified(  )
{
    ::SolarMutexGuard aGuard;
    return mbModified;
}


void SAL_CALL TableModel::setModified( sal_Bool bModified )
{
    {
        ::SolarMutexGuard aGuard;
        mbModified = bModified;
    }
    if( bModified )
        notifyModification();
}


// XModifyBroadcaster


void SAL_CALL TableModel::addModifyListener( const Reference< XModifyListener >& xListener )
{
    rBHelper.addListener( cppu::UnoType<XModifyListener>::get() , xListener );
}


void SAL_CALL TableModel::removeModifyListener( const Reference< XModifyListener >& xListener )
{
    rBHelper.removeListener( cppu::UnoType<XModifyListener>::get() , xListener );
}


// XColumnRowRange


Reference< XTableColumns > SAL_CALL TableModel::getColumns()
{
    ::SolarMutexGuard aGuard;

    if( !mxTableColumns.is() )
        mxTableColumns.set( new TableColumns( this ) );
    return mxTableColumns.get();
}


Reference< XTableRows > SAL_CALL TableModel::getRows()
{
    ::SolarMutexGuard aGuard;

    if( !mxTableRows.is() )
        mxTableRows.set( new TableRows( this ) );
    return mxTableRows.get();
}


// XCellRange


Reference< XCell > SAL_CALL TableModel::getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
{
    ::SolarMutexGuard aGuard;

    CellRef xCell( getCell( nColumn, nRow ) );
    if( xCell.is() )
        return xCell.get();

    throw IndexOutOfBoundsException();
}


Reference< XCellRange > SAL_CALL TableModel::getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
{
    ::SolarMutexGuard aGuard;

    if( (nLeft >= 0) && (nTop >= 0) && (nRight >= nLeft) && (nBottom >= nTop) && (nRight < getColumnCountImpl()) && (nBottom < getRowCountImpl() ) )
    {
        TableModelRef xModel( this );
        return new CellRange( xModel, nLeft, nTop, nRight, nBottom );
    }

    throw IndexOutOfBoundsException();
}


Reference< XCellRange > SAL_CALL TableModel::getCellRangeByName( const OUString& /*aRange*/ )
{
    return Reference< XCellRange >();
}


// XPropertySet


Reference< XPropertySetInfo > SAL_CALL TableModel::getPropertySetInfo(  )
{
    Reference< XPropertySetInfo > xInfo;
    return xInfo;
}


void SAL_CALL TableModel::setPropertyValue( const OUString& /*aPropertyName*/, const Any& /*aValue*/ )
{
}


Any SAL_CALL TableModel::getPropertyValue( const OUString& /*PropertyName*/ )
{
    return Any();
}


void SAL_CALL TableModel::addPropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ )
{
}


void SAL_CALL TableModel::removePropertyChangeListener( const OUString& /*aPropertyName*/, const Reference< XPropertyChangeListener >& /*xListener*/ )
{
}


void SAL_CALL TableModel::addVetoableChangeListener( const OUString& /*aPropertyName*/, const Reference< XVetoableChangeListener >& /*xListener*/ )
{
}


void SAL_CALL TableModel::removeVetoableChangeListener( const OUString& /*aPropertyName*/, const Reference< XVetoableChangeListener >& /*xListener*/ )
{
}


// XFastPropertySet


void SAL_CALL TableModel::setFastPropertyValue( ::sal_Int32 /*nHandle*/, const Any& /*aValue*/ )
{
}


Any SAL_CALL TableModel::getFastPropertyValue( ::sal_Int32 /*nHandle*/ )
{
    Any aAny;
    return aAny;
}


// internals


sal_Int32 TableModel::getRowCountImpl() const
{
    return static_cast< sal_Int32 >( maRows.size() );
}


sal_Int32 TableModel::getColumnCountImpl() const
{
    return static_cast< sal_Int32 >( maColumns.size() );
}


void TableModel::disposing()
{
    if( !maRows.empty() )
    {
        for( auto& rpRow : maRows )
            rpRow->dispose();
        RowVector().swap(maRows);
    }

    if( !maColumns.empty() )
    {
        for( auto& rpCol : maColumns )
            rpCol->dispose();
        ColumnVector().swap(maColumns);
    }

    if( mxTableColumns.is() )
    {
        mxTableColumns->dispose();
        mxTableColumns.clear();
    }

    if( mxTableRows.is() )
    {
        mxTableRows->dispose();
        mxTableRows.clear();
    }

    mpTableObj = nullptr;
}


// XBroadcaster


void TableModel::lockBroadcasts()
{
    ::SolarMutexGuard aGuard;
    ++mnNotifyLock;
}


void TableModel::unlockBroadcasts()
{
    ::SolarMutexGuard aGuard;
    --mnNotifyLock;
    if( mnNotifyLock <= 0 )
    {
        mnNotifyLock = 0;
        if( mbNotifyPending )
            notifyModification();
    }
}


void TableModel::notifyModification()
{
    ::osl::MutexGuard guard( m_aMutex );
    if( (mnNotifyLock == 0) && mpTableObj )
    {
        mbNotifyPending = false;

        ::cppu::OInterfaceContainerHelper * pModifyListeners = rBHelper.getContainer( cppu::UnoType<XModifyListener>::get() );
        if( pModifyListeners )
        {
            EventObject aSource;
            aSource.Source = static_cast< ::cppu::OWeakObject* >(this);
            pModifyListeners->notifyEach( &XModifyListener::modified, aSource);
        }
    }
    else
    {
        mbNotifyPending = true;
    }
}


CellRef TableModel::getCell( sal_Int32 nCol, sal_Int32 nRow ) const
{
    if( ((nRow >= 0) && (nRow < getRowCountImpl())) && (nCol >= 0) && (nCol < getColumnCountImpl()) )
    {
        return maRows[nRow]->maCells[nCol];
    }
    else
    {
        CellRef xRet;
        return xRet;
    }
}


CellRef TableModel::createCell()
{
    CellRef xCell;
    if( mpTableObj )
        mpTableObj->createCell( xCell );
    return xCell;
}


void TableModel::insertColumns( sal_Int32 nIndex, sal_Int32 nCount )
{
    if( nCount && mpTableObj )
    {
        try
        {
            SdrModel& rModel(mpTableObj->getSdrModelFromSdrObject());
            TableModelNotifyGuard aGuard( this );
            nIndex = insert_range<ColumnVector,ColumnVector::iterator,TableColumnRef>( maColumns, nIndex, nCount );

            sal_Int32 nRows = getRowCountImpl();
            while( nRows-- )
                maRows[nRows]->insertColumns( nIndex, nCount, nullptr );

            ColumnVector aNewColumns(nCount);
            for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
            {
                TableColumnRef xNewCol( new TableColumn( this, nIndex+nOffset ) );
                maColumns[nIndex+nOffset] = xNewCol;
                aNewColumns[nOffset] = xNewCol;
            }

            const bool bUndo(mpTableObj->IsInserted() && rModel.IsUndoEnabled());

            if( bUndo )
            {
                rModel.BegUndo( SvxResId(STR_TABLE_INSCOL) );
                rModel.AddUndo( rModel.GetSdrUndoFactory().CreateUndoGeoObject(*mpTableObj) );

                TableModelRef xThis( this );

                nRows = getRowCountImpl();
                CellVector aNewCells( nCount * nRows );
                CellVector::iterator aCellIter( aNewCells.begin() );

                nRows = getRowCountImpl();
                for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
                {
                    for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
                        (*aCellIter++) = getCell( nIndex + nOffset, nRow );
                }

                rModel.AddUndo( std::make_unique<InsertColUndo>( xThis, nIndex, aNewColumns, aNewCells ) );
            }

            const sal_Int32 nRowCount = getRowCountImpl();
            // check if cells merge over new columns
            for( sal_Int32 nCol = 0; nCol < nIndex; ++nCol )
            {
                for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
                {
                    CellRef xCell( getCell( nCol, nRow ) );
                    sal_Int32 nColSpan = (xCell.is() && !xCell->isMerged()) ? xCell->getColumnSpan() : 1;
                    if( (nColSpan != 1) && ((nColSpan + nCol ) > nIndex) )
                    {
                        // cell merges over newly created columns, so add the new columns to the merged cell
                        const sal_Int32 nRowSpan = xCell->getRowSpan();
                        nColSpan += nCount;
                        merge( nCol, nRow, nColSpan, nRowSpan );
                    }
                }
            }

            if( bUndo )
                rModel.EndUndo();

            rModel.SetChanged();
        }
        catch( Exception& )
        {
            OSL_FAIL("sdr::table::TableModel::insertColumns(), exception caught!");
        }
        updateColumns();
        setModified(true);
    }
}


void TableModel::removeColumns( sal_Int32 nIndex, sal_Int32 nCount )
{
    sal_Int32 nColCount = getColumnCountImpl();

    if( mpTableObj && nCount && (nIndex >= 0) && (nIndex < nColCount) )
    {
        try
        {
            TableModelNotifyGuard aGuard( this );

            // clip removed columns to columns actually available
            if( (nIndex + nCount) > nColCount )
                nCount = nColCount - nIndex;

            sal_Int32 nRows = getRowCountImpl();
            SdrModel& rModel(mpTableObj->getSdrModelFromSdrObject());
            const bool bUndo(mpTableObj->IsInserted() && rModel.IsUndoEnabled());

            if( bUndo  )
            {
                rModel.BegUndo( SvxResId(STR_UNDO_COL_DELETE) );
                rModel.AddUndo( rModel.GetSdrUndoFactory().CreateUndoGeoObject(*mpTableObj) );

                TableModelRef xThis( this );
                ColumnVector aRemovedCols( nCount );
                sal_Int32 nOffset;
                for( nOffset = 0; nOffset < nCount; ++nOffset )
                {
                    aRemovedCols[nOffset] = maColumns[nIndex+nOffset];
                }

                CellVector aRemovedCells( nCount * nRows );
                CellVector::iterator aCellIter( aRemovedCells.begin() );
                for( sal_Int32 nRow = 0; nRow < nRows; ++nRow )
                {
                    for( nOffset = 0; nOffset < nCount; ++nOffset )
                        (*aCellIter++) = getCell( nIndex + nOffset, nRow );
                }

                rModel.AddUndo( std::make_unique<RemoveColUndo>( xThis, nIndex, aRemovedCols, aRemovedCells ) );
            }

            // only rows before and inside the removed rows are considered
            nColCount = nIndex + nCount + 1;

            const sal_Int32 nRowCount = getRowCountImpl();

            // first check merged cells before and inside the removed rows
            for( sal_Int32 nCol = 0; nCol < nColCount; ++nCol )
            {
                for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
                {
                    CellRef xCell( getCell( nCol, nRow ) );
                    sal_Int32 nColSpan = (xCell.is() && !xCell->isMerged()) ? xCell->getColumnSpan() : 1;
                    if( nColSpan <= 1 )
                        continue;

                    if( nCol >= nIndex )
                    {
                        // current cell is inside the removed columns
                        if( (nCol + nColSpan) > ( nIndex + nCount ) )
                        {
                            // current cells merges with columns after the removed columns
                            const sal_Int32 nRemove = nCount - nCol + nIndex;

                            CellRef xTargetCell( getCell( nIndex + nCount, nRow ) );
                            if( xTargetCell.is() )
                            {
                                if( bUndo )
                                    xTargetCell->AddUndo();
                                xTargetCell->merge( nColSpan - nRemove, xCell->getRowSpan() );
                                xTargetCell->replaceContentAndFormating( xCell );
                            }
                        }
                    }
                    else if( nColSpan > (nIndex - nCol) )
                    {
                        // current cells spans inside the removed columns, so adjust
                        const sal_Int32 nRemove = ::std::min( nCount, nCol + nColSpan - nIndex );
                        if( bUndo )
                            xCell->AddUndo();
                        xCell->merge( nColSpan - nRemove, xCell->getRowSpan() );
                    }
                }
            }

            // now remove the columns
            remove_range<ColumnVector,ColumnVector::iterator>( maColumns, nIndex, nCount );
            while( nRows-- )
                maRows[nRows]->removeColumns( nIndex, nCount );

            if( bUndo )
                rModel.EndUndo();

            rModel.SetChanged();
        }
        catch( Exception& )
        {
            OSL_FAIL("sdr::table::TableModel::removeColumns(), exception caught!");
        }

        updateColumns();
        setModified(true);
    }
}


void TableModel::insertRows( sal_Int32 nIndex, sal_Int32 nCount )
{
    if( nCount && mpTableObj )
    {
        SdrModel& rModel(mpTableObj->getSdrModelFromSdrObject());
        const bool bUndo(mpTableObj->IsInserted() && rModel.IsUndoEnabled());

        try
        {
            TableModelNotifyGuard aGuard( this );

            nIndex = insert_range<RowVector,RowVector::iterator,TableRowRef>( maRows, nIndex, nCount );

            RowVector aNewRows(nCount);
            const sal_Int32 nColCount = getColumnCountImpl();
            for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
            {
                TableRowRef xNewRow( new TableRow( this, nIndex+nOffset, nColCount ) );
                maRows[nIndex+nOffset] = xNewRow;
                aNewRows[nOffset] = xNewRow;
            }

            if( bUndo )
            {
                rModel.BegUndo( SvxResId(STR_TABLE_INSROW) );
                rModel.AddUndo( rModel.GetSdrUndoFactory().CreateUndoGeoObject(*mpTableObj) );
                TableModelRef xThis( this );
                rModel.AddUndo( std::make_unique<InsertRowUndo>( xThis, nIndex, aNewRows ) );
            }

            // check if cells merge over new columns
            for( sal_Int32 nRow = 0; nRow < nIndex; ++nRow )
            {
                for( sal_Int32 nCol = 0; nCol < nColCount; ++nCol )
                {
                    CellRef xCell( getCell( nCol, nRow ) );
                    sal_Int32 nRowSpan = (xCell.is() && !xCell->isMerged()) ? xCell->getRowSpan() : 1;
                    if( (nRowSpan > 1) && ((nRowSpan + nRow) > nIndex) )
                    {
                        // cell merges over newly created columns, so add the new columns to the merged cell
                        const sal_Int32 nColSpan = xCell->getColumnSpan();
                        nRowSpan += nCount;
                        merge( nCol, nRow, nColSpan, nRowSpan );
                    }
                }
            }
        }
        catch( Exception& )
        {
            OSL_FAIL("sdr::table::TableModel::insertRows(), exception caught!");
        }
        if( bUndo )
            rModel.EndUndo();

        rModel.SetChanged();

        updateRows();
        setModified(true);
    }
}


void TableModel::removeRows( sal_Int32 nIndex, sal_Int32 nCount )
{
    sal_Int32 nRowCount = getRowCountImpl();

    if( mpTableObj && nCount && (nIndex >= 0) && (nIndex < nRowCount) )
    {
        SdrModel& rModel(mpTableObj->getSdrModelFromSdrObject());
        const bool bUndo(mpTableObj->IsInserted() && rModel.IsUndoEnabled());

        try
        {
            TableModelNotifyGuard aGuard( this );

            // clip removed rows to rows actually available
            if( (nIndex + nCount) > nRowCount )
                nCount = nRowCount - nIndex;

            if( bUndo )
            {
                rModel.BegUndo( SvxResId(STR_UNDO_ROW_DELETE) );
                rModel.AddUndo( rModel.GetSdrUndoFactory().CreateUndoGeoObject(*mpTableObj) );

                TableModelRef xThis( this );

                RowVector aRemovedRows( nCount );
                for( sal_Int32 nOffset = 0; nOffset < nCount; ++nOffset )
                    aRemovedRows[nOffset] = maRows[nIndex+nOffset];

                rModel.AddUndo( std::make_unique<RemoveRowUndo>( xThis, nIndex, aRemovedRows ) );
            }

            // only rows before and inside the removed rows are considered
            nRowCount = nIndex + nCount + 1;

            const sal_Int32 nColCount = getColumnCountImpl();

            // first check merged cells before and inside the removed rows
            for( sal_Int32 nRow = 0; nRow < nRowCount; ++nRow )
            {
                for( sal_Int32 nCol = 0; nCol < nColCount; ++nCol )
                {
                    CellRef xCell( getCell( nCol, nRow ) );
                    sal_Int32 nRowSpan = (xCell.is() && !xCell->isMerged()) ? xCell->getRowSpan() : 1;
                    if( nRowSpan <= 1 )
                        continue;

                    if( nRow >= nIndex )
                    {
                        // current cell is inside the removed rows
                        if( (nRow + nRowSpan) > (nIndex + nCount) )
                        {
                            // current cells merges with rows after the removed rows
                            const sal_Int32 nRemove = nCount - nRow + nIndex;

                            CellRef xTargetCell( getCell( nCol, nIndex + nCount ) );
                            if( xTargetCell.is() )
                            {
                                if( bUndo )
                                    xTargetCell->AddUndo();
                                xTargetCell->merge( xCell->getColumnSpan(), nRowSpan - nRemove );
                                xTargetCell->replaceContentAndFormating( xCell );
                            }
                        }
                    }
                    else if( nRowSpan > (nIndex - nRow) )
                    {
                        // current cells spans inside the removed rows, so adjust
                        const sal_Int32 nRemove = ::std::min( nCount, nRow + nRowSpan - nIndex );
                        if( bUndo )
                            xCell->AddUndo();
                        xCell->merge( xCell->getColumnSpan(), nRowSpan - nRemove );
                    }
                }
            }

            // now remove the rows
            remove_range<RowVector,RowVector::iterator>( maRows, nIndex, nCount );

            if( bUndo )
                rModel.EndUndo();

            rModel.SetChanged();
        }
        catch( Exception& )
        {
            OSL_FAIL("sdr::table::TableModel::removeRows(), exception caught!");
        }

        updateRows();
        setModified(true);
    }
}


TableRowRef const & TableModel::getRow( sal_Int32 nRow ) const
{
    if( (nRow >= 0) && (nRow < getRowCountImpl()) )
        return maRows[nRow];

    throw IndexOutOfBoundsException();
}


TableColumnRef const & TableModel::getColumn( sal_Int32 nColumn ) const
{
    if( (nColumn >= 0) && (nColumn < getColumnCountImpl()) )
        return maColumns[nColumn];

    throw IndexOutOfBoundsException();
}


/** deletes rows and columns that are completely merged. Must be called between BegUndo/EndUndo! */
void TableModel::optimize()
{
    TableModelNotifyGuard aGuard( this );

    bool bWasModified = false;

    if( !maRows.empty() && !maColumns.empty() )
    {
        sal_Int32 nCol = getColumnCountImpl() - 1;
        sal_Int32 nRows = getRowCountImpl();
        while( nCol > 0 )
        {
            bool bEmpty = true;
            for( sal_Int32 nRow = 0; (nRow < nRows) && bEmpty; nRow++ )
            {
                Reference< XMergeableCell > xCell( getCellByPosition( nCol, nRow ), UNO_QUERY );
                if( xCell.is() && !xCell->isMerged() )
                    bEmpty = false;
            }

            if( bEmpty )
            {
                try
                {
                    const OUString sWidth("Width");
                    sal_Int32 nWidth1 = 0, nWidth2 = 0;
                    Reference< XPropertySet > xSet1( static_cast< XCellRange* >( maColumns[nCol].get() ), UNO_QUERY_THROW );
                    Reference< XPropertySet > xSet2( static_cast< XCellRange* >( maColumns[nCol-1].get() ), UNO_QUERY_THROW );
                    xSet1->getPropertyValue( sWidth ) >>= nWidth1;
                    xSet2->getPropertyValue( sWidth ) >>= nWidth2;
                    nWidth1 = o3tl::saturating_add(nWidth1, nWidth2);
                    xSet2->setPropertyValue( sWidth, Any( nWidth1 ) );
                }
                catch( Exception& )
                {
                    OSL_FAIL("svx::TableModel::optimize(), exception caught!");
                }

                removeColumns( nCol, 1 );
                bWasModified = true;
            }

            nCol--;
        }

        sal_Int32 nRow = getRowCountImpl() - 1;
        sal_Int32 nCols = getColumnCountImpl();
        while( nRow > 0 )
        {
            bool bEmpty = true;
            for( nCol = 0; (nCol < nCols) && bEmpty; nCol++ )
            {
                Reference< XMergeableCell > xCell( getCellByPosition( nCol, nRow ), UNO_QUERY );
                if( xCell.is() && !xCell->isMerged() )
                    bEmpty = false;
            }

            if( bEmpty )
            {
                try
                {
                    const OUString sHeight("Height");
                    sal_Int32 nHeight1 = 0, nHeight2 = 0;
                    Reference< XPropertySet > xSet1( static_cast< XCellRange* >( maRows[nRow].get() ), UNO_QUERY_THROW );
                    Reference< XPropertySet > xSet2( static_cast< XCellRange* >( maRows[nRow-1].get() ), UNO_QUERY_THROW );
                    xSet1->getPropertyValue( sHeight ) >>= nHeight1;
                    xSet2->getPropertyValue( sHeight ) >>= nHeight2;
                    nHeight1 = o3tl::saturating_add(nHeight1, nHeight2);
                    xSet2->setPropertyValue( sHeight, Any( nHeight1 ) );
                }
                catch( Exception& )
                {
                    OSL_FAIL("svx::TableModel::optimize(), exception caught!");
                }

                removeRows( nRow, 1 );
                bWasModified = true;
            }

            nRow--;
        }
    }
    if( bWasModified )
        setModified(true);
}


void TableModel::merge( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan )
{
    if(nullptr == mpTableObj)
        return;

    SdrModel& rModel(mpTableObj->getSdrModelFromSdrObject());
    const bool bUndo(mpTableObj->IsInserted() && rModel.IsUndoEnabled());
    const sal_Int32 nLastRow = nRow + nRowSpan;
    const sal_Int32 nLastCol = nCol + nColSpan;

    if( (nLastRow > getRowCount()) || (nLastCol > getColumnCount() ) )
    {
        OSL_FAIL("TableModel::merge(), merge beyond the table!");
    }

    // merge first cell
    CellRef xOriginCell( dynamic_cast< Cell* >( getCellByPosition( nCol, nRow ).get() ) );
    if(!xOriginCell.is())
        return;

    if( bUndo )
        xOriginCell->AddUndo();
    xOriginCell->merge( nColSpan, nRowSpan );

    sal_Int32 nTempCol = nCol + 1;

    // merge remaining cells
    for( ; nRow < nLastRow; nRow++ )
    {
        for( ; nTempCol < nLastCol; nTempCol++ )
        {
            CellRef xCell( dynamic_cast< Cell* >( getCellByPosition( nTempCol, nRow ).get() ) );
            if( xCell.is() && !xCell->isMerged() )
            {
                if( bUndo )
                    xCell->AddUndo();
                xCell->setMerged();
                xOriginCell->mergeContent( xCell );
            }
        }
        nTempCol = nCol;
    }
}

void TableModel::updateRows()
{
    sal_Int32 nRow = 0;
    for( auto& rpRow : maRows )
    {
        rpRow->mnRow = nRow++;
    }
}

void TableModel::updateColumns()
{
    sal_Int32 nColumn = 0;
    for( auto& rpCol : maColumns )
    {
        rpCol->mnColumn = nColumn++;
    }
}

void TableModel::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("TableModel"));
    for (sal_Int32 nRow = 0; nRow < getRowCountImpl(); ++nRow)
        for (sal_Int32 nCol = 0; nCol < getColumnCountImpl(); ++nCol)
        {
            maRows[nRow]->maCells[nCol]->dumpAsXml(pWriter, nRow, nCol);
        }
    xmlTextWriterEndElement(pWriter);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
