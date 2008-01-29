/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlsubti.hxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:38:18 $
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
#ifndef SC_XMLSUBTI_HXX
#define SC_XMLSUBTI_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGE_HPP_
#include <com/sun/star/drawing/XDrawPage.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#include <vector>
#include <list>

#ifndef _SC_XMLTABLESHAPERESIZER_HXX
#include "XMLTableShapeResizer.hxx"
#endif

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
    sal_Int32                           FindNextCol(const sal_Int32 nIndex) const;
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

//*******************************************************************************************************************************

struct ScMatrixRange
{
    rtl::OUString sFormula;
    com::sun::star::table::CellRangeAddress aRange;
    ScMatrixRange(const com::sun::star::table::CellRangeAddress& rRange, const rtl::OUString& rFormula) :
        sFormula(rFormula),
        aRange(rRange)
    {
    }
};

class ScMyTables
{
private:
    typedef std::list<ScMatrixRange>    ScMyMatrixRangeList;

    ScXMLImport&                        rImport;

    ScMyShapeResizer                    aResizeShapes;

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > xCurrentSheet;
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > xCurrentCellRange;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > xDrawPage;
    ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > xShapes;
    rtl::OUString                       sCurrentSheetName;
    rtl::OUString                       sPassword;
    std::vector<ScMyTableData*>         aTableVec;
    ScMyMatrixRangeList                 aMatrixRangeList;
    com::sun::star::table::CellAddress  aRealCellPos;
    sal_Int32                           nCurrentColStylePos;
    sal_Int16                           nCurrentDrawPage;
    sal_Int16                           nCurrentXShapes;
    sal_Int32                           nTableCount;
    sal_Int32                           nCurrentSheet;
    sal_Bool                            bProtection;

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
                                                const sal_Bool bProtection, const rtl::OUString& sPassword);
    void                                AddRow();
    void                                SetRowStyle(const rtl::OUString& rCellStyleName);
    void                                CloseRow();
    void                                AddColumn(sal_Bool bIsCovered);
    void                                NewTable(sal_Int32 nTempSpannedCols);
    void                                UpdateRowHeights();
    void                                ResizeShapes() { aResizeShapes.ResizeShapes(); }
    void                                DeleteTable();
    com::sun::star::table::CellAddress  GetRealCellPos();
    void                                AddColCount(sal_Int32 nTempColCount);
    void                                AddColStyle(const sal_Int32 nRepeat, const rtl::OUString& rCellStyleName);
    rtl::OUString                       GetCurrentSheetName() const { return sCurrentSheetName; }
    sal_Int32                           GetCurrentSheet() const { return nCurrentSheet; }
    sal_Int32                           GetCurrentColumn() const { return aTableVec[nTableCount - 1]->GetColCount(); }
    sal_Int32                           GetCurrentRow() const { return aTableVec[nTableCount - 1]->GetRow(); }
    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet >
                                        GetCurrentXSheet()  { return xCurrentSheet; }
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                                        GetCurrentXCellRange()  { return xCurrentCellRange; }
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                                        GetCurrentXDrawPage();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                                        GetCurrentXShapes();
    sal_Bool                            HasDrawPage();
    sal_Bool                            HasXShapes();
    void                                AddShape(com::sun::star::uno::Reference <com::sun::star::drawing::XShape>& rShape,
                                                rtl::OUString* pRangeList,
                                                com::sun::star::table::CellAddress& rStartAddress,
                                                com::sun::star::table::CellAddress& rEndAddress,
                                                sal_Int32 nEndX, sal_Int32 nEndY);

    void                                AddMatrixRange(sal_Int32 nStartColumn, sal_Int32 nStartRow, sal_Int32 nEndColumn, sal_Int32 nEndRow, const rtl::OUString& rFormula);
    sal_Bool                            IsPartOfMatrix(sal_Int32 nColumn, sal_Int32 nRow);
    void                                SetMatrix(const com::sun::star::table::CellRangeAddress& rRange, const rtl::OUString& rFormula);
};

#endif
