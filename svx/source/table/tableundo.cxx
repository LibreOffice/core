/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tableundo.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:07:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/sdr/properties/textproperties.hxx"
#include "svx/outlobj.hxx"

#include "cell.hxx"
#include "tableundo.hxx"
#include "svx/svdotable.hxx"
#include "tablerow.hxx"
#include "tablecolumn.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::table;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

CellUndo::CellUndo( const SdrObjectWeakRef& xObjRef, const CellRef& xCell )
: SdrUndoAction( *xCell->GetModel() )
, mxObjRef( xObjRef )
, mxCell( xCell )
, mbUndo( true )
{

    getDataFromCell( maUndoData );
}

CellUndo::~CellUndo()
{
    if( mbUndo )
    {
        delete maUndoData.mpProperties;
        delete maUndoData.mpOutlinerParaObject;
    }
    else
    {
        delete maRedoData.mpProperties;
        delete maRedoData.mpOutlinerParaObject;
    }
}

void CellUndo::Undo()
{
    if( mxCell.is() && mbUndo )
    {
        if( maRedoData.mpProperties == 0 )
            getDataFromCell( maRedoData );

        setDataToCell( maUndoData );
        mbUndo = false;
    }
}

void CellUndo::Redo()
{
    if( mxCell.is() && mbUndo )
    {
        setDataToCell( maRedoData );
        mbUndo = true;
    }
}

BOOL CellUndo::Merge( SfxUndoAction *pNextAction )
{
    CellUndo* pNext = dynamic_cast< CellUndo* >( pNextAction );
    if( pNext && pNext->mxCell.get() == mxCell.get() )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CellUndo::setDataToCell( const Data& rData )
{
    mxCell->mpProperties = rData.mpProperties;


    if( rData.mpOutlinerParaObject )
        mxCell->SetOutlinerParaObject( rData.mpOutlinerParaObject->Clone() );
    else
        mxCell->RemoveOutlinerParaObject();

    mxCell->msFormula = rData.msFormula;
    mxCell->mfValue = rData.mfValue;
    mxCell->mnError = rData.mnError;
    mxCell->mbMerged = rData.mbMerged;
    mxCell->mnRowSpan = rData.mnRowSpan;
    mxCell->mnColSpan = rData.mnColSpan;

    if( mxObjRef.is() )
        mxObjRef->ActionChanged();
}

void CellUndo::getDataFromCell( Data& rData )
{
    if( mxObjRef.is() && mxCell.is() )
    {
        if( mxCell->mpProperties )
            rData.mpProperties = mxCell->CloneProperties( *mxObjRef.get(), *mxCell.get());

        if( mxCell->GetOutlinerParaObject() )
            rData.mpOutlinerParaObject = mxCell->GetOutlinerParaObject()->Clone();
        else
            rData.mpOutlinerParaObject =  0;

        rData.mnCellContentType = mxCell->mnCellContentType;

        rData.msFormula = mxCell->msFormula;
        rData.mfValue = mxCell->mfValue;
        rData.mnError = mxCell->mnError;
        rData.mbMerged = mxCell->mbMerged;
        rData.mnRowSpan = mxCell->mnRowSpan;
        rData.mnColSpan = mxCell->mnColSpan;
    }
}

// -----------------------------------------------------------------------------
// class InsertRowUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

static void Dispose( RowVector& rRows )
{
    RowVector::iterator aIter( rRows.begin() );
    while( aIter != rRows.end() )
        (*aIter++)->dispose();
}

// -----------------------------------------------------------------------------

InsertRowUndo::InsertRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aNewRows )
: SdrUndoAction( *xTable->getSdrTableObj()->GetModel() )
, mxTable( xTable )
, mnIndex( nIndex )
, mbUndo( true )
{
    maRows.swap( aNewRows );
}

// -----------------------------------------------------------------------------

InsertRowUndo::~InsertRowUndo()
{
    if( !mbUndo )
        Dispose( maRows );
}

// -----------------------------------------------------------------------------

void InsertRowUndo::Undo()
{
    if( mxTable.is() )
    {
        mxTable->UndoInsertRows( mnIndex, sal::static_int_cast< sal_Int32 >( maRows.size() ) );
        mbUndo = false;
    }
}

// -----------------------------------------------------------------------------

void InsertRowUndo::Redo()
{
    if( mxTable.is() )
    {
        mxTable->UndoRemoveRows( mnIndex, maRows );
        mbUndo = true;
    }
}

// -----------------------------------------------------------------------------
// class RemoveRowUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

RemoveRowUndo::RemoveRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aRemovedRows )
: SdrUndoAction( *xTable->getSdrTableObj()->GetModel() )
, mxTable( xTable )
, mnIndex( nIndex )
, mbUndo( true )
{
    maRows.swap( aRemovedRows );
}

// -----------------------------------------------------------------------------

RemoveRowUndo::~RemoveRowUndo()
{
    if( mbUndo )
        Dispose( maRows );
}

// -----------------------------------------------------------------------------

void RemoveRowUndo::Undo()
{
    if( mxTable.is() )
    {
        mxTable->UndoRemoveRows( mnIndex, maRows );
        mbUndo = false;
    }
}

// -----------------------------------------------------------------------------

void RemoveRowUndo::Redo()
{
    if( mxTable.is() )
    {
        mxTable->UndoInsertRows( mnIndex, sal::static_int_cast< sal_Int32 >( maRows.size() ) );
        mbUndo = true;
    }
}

// -----------------------------------------------------------------------------
// class InsertColUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

static void Dispose( ColumnVector& rCols )
{
    ColumnVector::iterator aIter( rCols.begin() );
    while( aIter != rCols.end() )
        (*aIter++)->dispose();
}

// -----------------------------------------------------------------------------

static void Dispose( CellVector& rCells )
{
    CellVector::iterator aIter( rCells.begin() );
    while( aIter != rCells.end() )
        (*aIter++)->dispose();
}

// -----------------------------------------------------------------------------

InsertColUndo::InsertColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells  )
: SdrUndoAction( *xTable->getSdrTableObj()->GetModel() )
, mxTable( xTable )
, mnIndex( nIndex )
, mbUndo( true )
{
    maColumns.swap( aNewCols );
    maCells.swap( aCells );
}

// -----------------------------------------------------------------------------

InsertColUndo::~InsertColUndo()
{
    if( !mbUndo )
    {
        Dispose( maColumns );
        Dispose( maCells );
    }
}

// -----------------------------------------------------------------------------

void InsertColUndo::Undo()
{
    if( mxTable.is() )
    {
        mxTable->UndoInsertColumns( mnIndex, sal::static_int_cast< sal_Int32 >( maColumns.size() ) );
        mbUndo = false;
    }
}

// -----------------------------------------------------------------------------

void InsertColUndo::Redo()
{
    if( mxTable.is() )
    {
        mxTable->UndoRemoveColumns( mnIndex, maColumns, maCells );
        mbUndo = true;
    }
}

// -----------------------------------------------------------------------------
// class RemoveColUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

RemoveColUndo::RemoveColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells )
: SdrUndoAction( *xTable->getSdrTableObj()->GetModel() )
, mxTable( xTable )
, mnIndex( nIndex )
, mbUndo( true )
{
    maColumns.swap( aNewCols );
    maCells.swap( aCells );
}

// -----------------------------------------------------------------------------

RemoveColUndo::~RemoveColUndo()
{
    if( mbUndo )
    {
        Dispose( maColumns );
        Dispose( maCells );
    }
}

// -----------------------------------------------------------------------------

void RemoveColUndo::Undo()
{
    if( mxTable.is() )
    {
        mxTable->UndoRemoveColumns( mnIndex, maColumns, maCells );
        mbUndo = false;
    }
}

// -----------------------------------------------------------------------------

void RemoveColUndo::Redo()
{
    if( mxTable.is() )
    {
        mxTable->UndoInsertColumns( mnIndex, sal::static_int_cast< sal_Int32 >( maColumns.size() ) );
        mbUndo = true;
    }
}

// -----------------------------------------------------------------------------
// class TableColumnUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

TableColumnUndo::TableColumnUndo( const TableColumnRef& xCol )
: SdrUndoAction( *xCol->mxTableModel->getSdrTableObj()->GetModel() )
, mxCol( xCol )
, mbHasRedoData( false )
{
    getData( maUndoData );
}

// -----------------------------------------------------------------------------

TableColumnUndo::~TableColumnUndo()
{
}

// -----------------------------------------------------------------------------

void TableColumnUndo::Undo()
{
    if( !mbHasRedoData )
    {
        getData( maRedoData );
        mbHasRedoData = true;
    }
    setData( maUndoData );
}

// -----------------------------------------------------------------------------

void TableColumnUndo::Redo()
{
    setData( maRedoData );
}

// -----------------------------------------------------------------------------

BOOL TableColumnUndo::Merge( SfxUndoAction *pNextAction )
{
    TableColumnUndo* pNext = dynamic_cast< TableColumnUndo* >( pNextAction );
    return pNext && pNext->mxCol == mxCol;
}

// -----------------------------------------------------------------------------

void TableColumnUndo::setData( const Data& rData )
{
    mxCol->mnColumn = rData.mnColumn;
    mxCol->mnWidth = rData.mnWidth;
    mxCol->mbOptimalWidth = rData.mbOptimalWidth;
    mxCol->mbIsVisible = rData.mbIsVisible;
    mxCol->mbIsStartOfNewPage = rData.mbIsStartOfNewPage;
    mxCol->maName = rData.maName;
}

// -----------------------------------------------------------------------------

void TableColumnUndo::getData( Data& rData )
{
    rData.mnColumn = mxCol->mnColumn;
    rData.mnWidth = mxCol->mnWidth;
    rData.mbOptimalWidth = mxCol->mbOptimalWidth;
    rData.mbIsVisible = mxCol->mbIsVisible;
    rData.mbIsStartOfNewPage = mxCol->mbIsStartOfNewPage;
    rData.maName = mxCol->maName;
}

// -----------------------------------------------------------------------------
// class TableRowUndo : public SdrUndoAction
// -----------------------------------------------------------------------------

TableRowUndo::TableRowUndo( const TableRowRef& xRow )
: SdrUndoAction( *xRow->mxTableModel->getSdrTableObj()->GetModel() )
, mxRow( xRow )
, mbHasRedoData( false )
{
    getData( maUndoData );
}

// -----------------------------------------------------------------------------

TableRowUndo::~TableRowUndo()
{
}

// -----------------------------------------------------------------------------

void TableRowUndo::Undo()
{
    if( !mbHasRedoData )
    {
        getData( maRedoData );
        mbHasRedoData = true;
    }
    setData( maUndoData );
}

// -----------------------------------------------------------------------------

void TableRowUndo::Redo()
{
    setData( maRedoData );
}

// -----------------------------------------------------------------------------

BOOL TableRowUndo::Merge( SfxUndoAction *pNextAction )
{
    TableRowUndo* pNext = dynamic_cast< TableRowUndo* >( pNextAction );
    return pNext && pNext->mxRow == mxRow;
}

// -----------------------------------------------------------------------------

void TableRowUndo::setData( const Data& rData )
{
    mxRow->mnRow = rData.mnRow;
    mxRow->mnHeight = rData.mnHeight;
    mxRow->mbOptimalHeight = rData.mbOptimalHeight;
    mxRow->mbIsVisible = rData.mbIsVisible;
    mxRow->mbIsStartOfNewPage = rData.mbIsStartOfNewPage;
    mxRow->maName = rData.maName;
 }

// -----------------------------------------------------------------------------

void TableRowUndo::getData( Data& rData )
{
    rData.mnRow = mxRow->mnRow;
    rData.mnHeight = mxRow->mnHeight;
    rData.mbOptimalHeight = mxRow->mbOptimalHeight;
    rData.mbIsVisible = mxRow->mbIsVisible;
    rData.mbIsStartOfNewPage = mxRow->mbIsStartOfNewPage;
    rData.maName = mxRow->maName;
}

// -----------------------------------------------------------------------------

TableStyleUndo::TableStyleUndo( const SdrTableObj& rTableObj )
: SdrUndoAction( *rTableObj.GetModel() )
, mxObjRef( const_cast< sdr::table::SdrTableObj*>( &rTableObj ) )
{
    getData( maUndoData );
}

void TableStyleUndo::Undo()
{
    if( !mbHasRedoData )
    {
        getData( maRedoData );
        mbHasRedoData = true;
    }
    setData( maUndoData );
}

void TableStyleUndo::Redo()
{
    setData( maRedoData );
}

void TableStyleUndo::setData( const Data& rData )
{
    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxObjRef.get() );
    if( pTableObj )
    {
        pTableObj->setTableStyle( rData.mxTableStyle );
        pTableObj->setTableStyleSettings( rData.maSettings );
    }
}

void TableStyleUndo::getData( Data& rData )
{
    SdrTableObj* pTableObj = dynamic_cast< SdrTableObj* >( mxObjRef.get() );
    if( pTableObj )
    {
        rData.maSettings = pTableObj->getTableStyleSettings();
        rData.mxTableStyle = pTableObj->getTableStyle();
    }
}

} }
