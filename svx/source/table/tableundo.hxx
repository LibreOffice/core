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

class CellUndo : public SdrUndoAction, public sdr::ObjectUser
{
public:
    CellUndo( const SdrObjectWeakRef& xObjRef, const CellRef& xCell );
    virtual ~CellUndo();

    virtual void            Undo();
    virtual void            Redo();
    virtual bool            Merge( SfxUndoAction *pNextAction );

    void dispose();
    virtual void ObjectInDestruction(const SdrObject& rObject);

private:
    struct Data
    {
        sdr::properties::TextProperties* mpProperties;
        OutlinerParaObject* mpOutlinerParaObject;

        ::com::sun::star::table::CellContentType mnCellContentType;

        OUString msFormula;
        double          mfValue;
        ::sal_Int32     mnError;
        ::sal_Bool      mbMerged;
        ::sal_Int32     mnRowSpan;
        ::sal_Int32     mnColSpan;

        Data() : mpProperties(NULL), mpOutlinerParaObject(NULL), mfValue(0) {};
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
    virtual bool            Merge( SfxUndoAction *pNextAction );

private:
    struct Data
    {
        sal_Int32   mnColumn;
        sal_Int32   mnWidth;
        sal_Bool    mbOptimalWidth;
        sal_Bool    mbIsVisible;
        sal_Bool    mbIsStartOfNewPage;
        OUString maName;
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
    virtual bool            Merge( SfxUndoAction *pNextAction );

private:
    struct Data
    {
        CellVector  maCells;
        sal_Int32   mnRow;
        sal_Int32   mnHeight;
        sal_Bool    mbOptimalHeight;
        sal_Bool    mbIsVisible;
        sal_Bool    mbIsStartOfNewPage;
        OUString maName;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
