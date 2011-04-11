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
#ifndef SC_XMLSUBTI_HXX
#define SC_XMLSUBTI_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "XMLTableShapeResizer.hxx"
#include "formula/grammar.hxx"
#include "tabprotection.hxx"

#include <vector>
#include <list>
#include <boost/ptr_container/ptr_vector.hpp>

class ScXMLImport;

typedef std::vector<sal_Int32> ScMysalIntVec;
typedef std::list<sal_Int32> ScMysalIntList;

const ScMysalIntVec::size_type nDefaultRowCount = 20;
const ScMysalIntVec::size_type nDefaultColCount = 20;
const ScMysalIntVec::size_type nDefaultTabCount = 10;

class ScMyTableData
{
private:
    com::sun::star::table::CellAddress  aTableCellPos;
    ScMysalIntVec                       nColsPerCol;
    ScMysalIntVec                       nRealCols;
    ScMysalIntVec                       nRowsPerRow;
    ScMysalIntVec                       nRealRows;
    sal_Int32                           nSpannedCols;
    sal_Int32                           nColCount;
    sal_Int32                           nSubTableSpanned;
    ScMysalIntList                      nChangedCols;
public:
                                        ScMyTableData(sal_Int32 nSheet = -1, sal_Int32 nCol = -1, sal_Int32 nRow = -1);
                                        ~ScMyTableData();
    com::sun::star::table::CellAddress  GetCellPos() const { return aTableCellPos; }
    sal_Int32                           GetRow() const { return aTableCellPos.Row; }
    sal_Int32                           GetColumn() const { return aTableCellPos.Column; }
    void                                AddRow();
    void                                AddColumn();
    void                                SetFirstColumn() { aTableCellPos.Column = -1; }
    sal_Int32                           GetColsPerCol(const sal_Int32 nIndex) const { return nColsPerCol[nIndex]; }
    void                                SetColsPerCol(const sal_Int32 nIndex, sal_Int32 nValue = 1) { nColsPerCol[nIndex] = nValue; }
    sal_Int32                           GetRealCols(const sal_Int32 nIndex, const sal_Bool bIsNormal = sal_True) const;
    void                                SetRealCols(const sal_Int32 nIndex, const sal_Int32 nValue) { nRealCols[nIndex] = nValue; }
    sal_Int32                           GetRowsPerRow(const sal_Int32 nIndex) const { return nRowsPerRow[nIndex]; }
    void                                SetRowsPerRow(const sal_Int32 nIndex, const sal_Int32 nValue = 1) { nRowsPerRow[nIndex] = nValue; }
    sal_Int32                           GetRealRows(const sal_Int32 nIndex) const { return nIndex < 0 ? 0 : nRealRows[nIndex]; }
    void                                SetRealRows(const sal_Int32 nIndex, const sal_Int32 nValue) { nRealRows[nIndex] = nValue; }
    sal_Int32                           GetSpannedCols() const { return nSpannedCols; }
    void                                SetSpannedCols(const sal_Int32 nTempSpannedCols) { nSpannedCols = nTempSpannedCols; }
    sal_Int32                           GetColCount() const { return nColCount; }
    void                                SetColCount(const sal_Int32 nTempColCount) { nColCount = nTempColCount; }
    sal_Int32                           GetSubTableSpanned() const { return nSubTableSpanned; }
    void                                SetSubTableSpanned(const sal_Int32 nValue) { nSubTableSpanned = nValue; }
    sal_Int32                           GetChangedCols(const sal_Int32 nFromIndex, const sal_Int32 nToIndex) const;
    void                                SetChangedCols(const sal_Int32 nValue);
};


struct ScMatrixRange
{
    rtl::OUString sFormula;
    rtl::OUString sFormulaNmsp;
    formula::FormulaGrammar::Grammar eGrammar;
    com::sun::star::table::CellRangeAddress aRange;
    ScMatrixRange(const com::sun::star::table::CellRangeAddress& rRange, const rtl::OUString& rFormula, const rtl::OUString& rFormulaNmsp, const formula::FormulaGrammar::Grammar eGrammarP) :
        sFormula(rFormula),
        sFormulaNmsp(rFormulaNmsp),
        eGrammar(eGrammarP),
        aRange(rRange)
    {
    }
};

struct ScXMLTabProtectionData
{
    ::rtl::OUString maPassword;
    ScPasswordHash  meHash1;
    ScPasswordHash  meHash2;
    bool            mbProtected;
    bool            mbSelectProtectedCells;
    bool            mbSelectUnprotectedCells;

    ScXMLTabProtectionData();
};

class ScMyTables
{
private:
    typedef std::list<ScMatrixRange>    ScMyMatrixRangeList;

    ScXMLImport&                        rImport;

    ScMyOLEFixer                        aFixupOLEs;

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > xCurrentSheet;
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > xCurrentCellRange;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage;
    ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > xShapes;
    rtl::OUString                       sCurrentSheetName;
    ::boost::ptr_vector<ScMyTableData>  maTables;
    ScXMLTabProtectionData              maProtectionData;
    ScMyMatrixRangeList                 aMatrixRangeList;
    com::sun::star::table::CellAddress  aRealCellPos;
    sal_Int32                           nCurrentColStylePos;
    sal_Int16                           nCurrentDrawPage;
    sal_Int16                           nCurrentXShapes;
    sal_Int32                           nCurrentSheet;

    sal_Bool                            IsMerged (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
                                                const sal_Int32 nCol, const sal_Int32 nRow,
                                                com::sun::star::table::CellRangeAddress& aCellAddress) const;
    void                                UnMerge();
    void                                DoMerge(sal_Int32 nCount = -1);
    void                                InsertRow();
    void                                NewRow();
    void                                InsertColumn();
    void                                NewColumn(sal_Bool bIsCovered);
public:
                                        ScMyTables(ScXMLImport& rImport);
                                        ~ScMyTables();
    void                                NewSheet(const rtl::OUString& sTableName, const rtl::OUString& sStyleName,
                                                 const ScXMLTabProtectionData& rProtectData);
    void                                AddRow();
    void                                SetRowStyle(const rtl::OUString& rCellStyleName);
    void                                AddColumn(sal_Bool bIsCovered);
    void                                NewTable(sal_Int32 nTempSpannedCols);
    void                                UpdateRowHeights();
    void                                FixupOLEs() { aFixupOLEs.FixupOLEs(); }
    sal_Bool                            IsOLE(com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& rShape) const
        { return ScMyOLEFixer::IsOLE(rShape); }
    void                                DeleteTable();
    com::sun::star::table::CellAddress  GetRealCellPos();
    void                                AddColCount(sal_Int32 nTempColCount);
    void                                AddColStyle(const sal_Int32 nRepeat, const rtl::OUString& rCellStyleName);
    ScXMLTabProtectionData&             GetCurrentProtectionData() { return maProtectionData; }
    rtl::OUString                       GetCurrentSheetName() const { return sCurrentSheetName; }
    sal_Int32                           GetCurrentSheet() const { return nCurrentSheet; }
    sal_Int32                           GetCurrentColumn() const { return maTables.back().GetColCount(); }
    sal_Int32                           GetCurrentRow() const { return maTables.back().GetRow(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                                        GetCurrentXSheet() const { return xCurrentSheet; }
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                                        GetCurrentXCellRange() const { return xCurrentCellRange; }
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                                        GetCurrentXDrawPage();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                                        GetCurrentXShapes();
    sal_Bool                            HasDrawPage();
    sal_Bool                            HasXShapes();
    void                                AddOLE(com::sun::star::uno::Reference <com::sun::star::drawing::XShape>& rShape,
                                               const rtl::OUString &rRangeList);

    void                                AddMatrixRange( sal_Int32 nStartColumn,
                                                sal_Int32 nStartRow,
                                                sal_Int32 nEndColumn,
                                                sal_Int32 nEndRow,
                                                const rtl::OUString& rFormula,
                                                const rtl::OUString& rFormulaNmsp,
                                                const formula::FormulaGrammar::Grammar );

    sal_Bool                            IsPartOfMatrix(sal_Int32 nColumn, sal_Int32 nRow);
    void                                SetMatrix( const com::sun::star::table::CellRangeAddress& rRange,
                                                const rtl::OUString& rFormula,
                                                const rtl::OUString& rFormulaNmsp,
                                                const formula::FormulaGrammar::Grammar );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
