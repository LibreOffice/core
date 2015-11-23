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

#ifndef INCLUDED_SVX_SOURCE_TABLE_TABLEUNDO_HXX
#define INCLUDED_SVX_SOURCE_TABLE_TABLEUNDO_HXX

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/CellContentType.hpp>

#include "svx/svdotable.hxx"
#include "svx/svdobj.hxx"
#include "svx/svdundo.hxx"
#include <svx/sdrobjectuser.hxx>

#include "celltypes.hxx"

namespace sdr { namespace properties {
    class TextProperties;
} }

class OutlinerParaObject;



namespace sdr { namespace table {

class CellUndo : public SdrUndoAction, public sdr::ObjectUser
{
public:
    CellUndo( const SdrObjectWeakRef& xObjRef, const CellRef& xCell );
    virtual ~CellUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;
    virtual bool            Merge( SfxUndoAction *pNextAction ) override;

    void dispose();
    virtual void ObjectInDestruction(const SdrObject& rObject) override;

private:
    struct Data
    {
        sdr::properties::TextProperties* mpProperties;
        OutlinerParaObject* mpOutlinerParaObject;

        css::table::CellContentType mnCellContentType;

        OUString        msFormula;
        double          mfValue;
        ::sal_Int32     mnError;
        bool            mbMerged;
        ::sal_Int32     mnRowSpan;
        ::sal_Int32     mnColSpan;

        Data()
            : mpProperties(nullptr)
            , mpOutlinerParaObject(nullptr)
            , mnCellContentType(css::table::CellContentType_EMPTY)
            , mfValue(0)
            , mnError(0)
            , mbMerged(false)
            , mnRowSpan(0)
            , mnColSpan(0)
        {
        }
    };

    void setDataToCell( const Data& rData );
    void getDataFromCell( Data& rData );

    SdrObjectWeakRef mxObjRef;
    CellRef mxCell;
    Data maUndoData;
    Data maRedoData;
    bool mbUndo;
};



class InsertRowUndo : public SdrUndoAction
{
public:
    InsertRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aNewRows );
    virtual ~InsertRowUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    RowVector maRows;
    bool mbUndo;
};



class RemoveRowUndo : public SdrUndoAction
{
public:
    RemoveRowUndo( const TableModelRef& xTable, sal_Int32 nIndex, RowVector& aRemovedRows );
    virtual ~RemoveRowUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    RowVector maRows;
    bool mbUndo;
};



class InsertColUndo : public SdrUndoAction
{
public:
    InsertColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells );
    virtual ~InsertColUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    ColumnVector maColumns;
    CellVector maCells;
    bool mbUndo;
};



class RemoveColUndo : public SdrUndoAction
{
public:
    RemoveColUndo( const TableModelRef& xTable, sal_Int32 nIndex, ColumnVector& aNewCols, CellVector& aCells );
    virtual ~RemoveColUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;

private:
    TableModelRef mxTable;
    sal_Int32 mnIndex;
    ColumnVector maColumns;
    CellVector maCells;
    bool mbUndo;
};



class TableColumnUndo : public SdrUndoAction
{
public:
    explicit TableColumnUndo( const TableColumnRef& xCol );
    virtual ~TableColumnUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;
    virtual bool            Merge( SfxUndoAction *pNextAction ) override;

private:
    struct Data
    {
        sal_Int32   mnColumn;
        sal_Int32   mnWidth;
        bool    mbOptimalWidth;
        bool    mbIsVisible;
        bool    mbIsStartOfNewPage;
        OUString maName;

        Data()
            : mnColumn(0)
            , mnWidth(0)
            , mbOptimalWidth(false)
            , mbIsVisible(false)
            , mbIsStartOfNewPage(false)
        {
        }
    };

    void setData( const Data& rData );
    void getData( Data& rData );

    TableColumnRef mxCol;
    Data maUndoData;
    Data maRedoData;
    bool mbHasRedoData;
};



class TableRowUndo : public SdrUndoAction
{
public:
    explicit TableRowUndo( const TableRowRef& xRow );
    virtual ~TableRowUndo();

    virtual void            Undo() override;
    virtual void            Redo() override;
    virtual bool            Merge( SfxUndoAction *pNextAction ) override;

private:
    struct Data
    {
        sal_Int32   mnRow;
        sal_Int32   mnHeight;
        bool    mbOptimalHeight;
        bool    mbIsVisible;
        bool    mbIsStartOfNewPage;
        OUString maName;
    };

    void setData( const Data& rData );
    void getData( Data& rData );

    TableRowRef mxRow;
    Data maUndoData;
    Data maRedoData;
    bool mbHasRedoData;
};

class TableStyleUndo : public SdrUndoAction
{
public:
    explicit TableStyleUndo( const SdrTableObj& rTableObj );

    virtual void            Undo() override;
    virtual void            Redo() override;

private:
    SdrObjectWeakRef mxObjRef;

    struct Data
    {
        TableStyleSettings maSettings;
        css::uno::Reference< css::container::XIndexAccess > mxTableStyle;
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
