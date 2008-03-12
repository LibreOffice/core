/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tableundo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:07:15 $
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

#ifndef _SVX_TABLEUNDO_HXX_
#define _SVX_TABLEUNDO_HXX_

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/CellContentType.hpp>

#include "svx/svdotable.hxx"
#include "svx/svdobj.hxx"
#include "svx/svdundo.hxx"

#include "celltypes.hxx"

namespace sdr { namespace properties {
    class TextProperties;
} }

class OutlinerParaObject;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

class CellUndo : public SdrUndoAction
{
public:
    CellUndo( const SdrObjectWeakRef& xObjRef, const CellRef& xCell );
    virtual ~CellUndo();

    virtual void            Undo();
    virtual void            Redo();
    virtual BOOL            Merge( SfxUndoAction *pNextAction );

private:
    struct Data
    {
        sdr::properties::TextProperties* mpProperties;
        OutlinerParaObject* mpOutlinerParaObject;

        ::com::sun::star::table::CellContentType mnCellContentType;

        ::rtl::OUString msFormula;
        double          mfValue;
        ::sal_Int32     mnError;
        ::sal_Bool      mbMerged;
        ::sal_Int32     mnRowSpan;
        ::sal_Int32     mnColSpan;

        Data() : mpProperties(0), mpOutlinerParaObject(0) {};
    };

    void setDataToCell( const Data& rData );
    void getDataFromCell( Data& rData );

    SdrObjectWeakRef mxObjRef;
    CellRef mxCell;
    Data maUndoData;
    Data maRedoData;
    bool mbUndo;
};

// -----------------------------------------------------------------------------

class InsertRowUndo : public SdrUndoAction
{
public:
    InsertRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aNewRows );
    virtual ~InsertRowUndo();

    virtual void            Undo();
    virtual void            Redo();

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    RowVector maRows;
    bool mbUndo;
};

// -----------------------------------------------------------------------------

class RemoveRowUndo : public SdrUndoAction
{
public:
    RemoveRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aRemovedRows );
    virtual ~RemoveRowUndo();

    virtual void            Undo();
    virtual void            Redo();

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    RowVector maRows;
    bool mbUndo;
};

// -----------------------------------------------------------------------------

class InsertColUndo : public SdrUndoAction
{
public:
    InsertColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells );
    virtual ~InsertColUndo();

    virtual void            Undo();
    virtual void            Redo();

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    ColumnVector maColumns;
    CellVector maCells;
    bool mbUndo;
};

// -----------------------------------------------------------------------------

class RemoveColUndo : public SdrUndoAction
{
public:
    RemoveColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells );
    virtual ~RemoveColUndo();

    virtual void            Undo();
    virtual void            Redo();

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    ColumnVector maColumns;
    CellVector maCells;
    bool mbUndo;
};

// -----------------------------------------------------------------------------

class TableColumnUndo : public SdrUndoAction
{
public:
    TableColumnUndo( const TableColumnRef& xCol );
    virtual ~TableColumnUndo();

    virtual void            Undo();
    virtual void            Redo();
    virtual BOOL            Merge( SfxUndoAction *pNextAction );

private:
    struct Data
    {
        sal_Int32   mnColumn;
        sal_Int32   mnWidth;
        sal_Bool    mbOptimalWidth;
        sal_Bool    mbIsVisible;
        sal_Bool    mbIsStartOfNewPage;
        ::rtl::OUString maName;
    };

    void setData( const Data& rData );
    void getData( Data& rData );

    TableColumnRef mxCol;
    Data maUndoData;
    Data maRedoData;
    bool mbHasRedoData;
};

// -----------------------------------------------------------------------------

class TableRowUndo : public SdrUndoAction
{
public:
    TableRowUndo( const TableRowRef& xRow );
    virtual ~TableRowUndo();

    virtual void            Undo();
    virtual void            Redo();
    virtual BOOL            Merge( SfxUndoAction *pNextAction );

private:
    struct Data
    {
        CellVector  maCells;
        sal_Int32   mnRow;
        sal_Int32   mnHeight;
        sal_Bool    mbOptimalHeight;
        sal_Bool    mbIsVisible;
        sal_Bool    mbIsStartOfNewPage;
        ::rtl::OUString maName;
    };

    void setData( const Data& rData );
    void getData( Data& rData );

    TableRowRef mxRow;
    Data maUndoData;
    Data maRedoData;
    bool mbHasRedoData;
};

// -----------------------------------------------------------------------------

class TableStyleUndo : public SdrUndoAction
{
public:
    TableStyleUndo( const SdrTableObj& rTableObj );

    virtual void            Undo();
    virtual void            Redo();

private:
    SdrObjectWeakRef mxObjRef;

    struct Data
    {
        TableStyleSettings maSettings;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > mxTableStyle;
    };

    void setData( const Data& rData );
    void getData( Data& rData );

    Data maUndoData;
    Data maRedoData;
    bool mbHasRedoData;
};

} }

#endif
