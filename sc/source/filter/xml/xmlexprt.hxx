/*************************************************************************
 *
 *  $RCSfile: xmlexprt.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-19 16:00:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_XMLEXPRT_HXX
#define SC_XMLEXPRT_HXX

#ifndef _XMLOFF_XMLEXP_HXX
#include <xmloff/xmlexp.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef _XMLOFF_XMLASTPLP_HXX
#include <xmloff/xmlaustp.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMERGEABLE_HPP_
#include <com/sun/star/util/XMergeable.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELLRANGE_HPP_
#include <com/sun/star/table/XCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCELL_HPP_
#include <com/sun/star/table/XCell.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETFILTERDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_VALIDATIONALERTSTYLE_HPP_
#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_VALIDATIONTYPE_HPP_
#include <com/sun/star/sheet/ValidationType.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_CONDITIONOPERATOR_HPP_
#include <com/sun/star/sheet/ConditionOperator.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef SC_XMLMAPCH_HXX_
#include "xmlmapch.hxx"
#endif

#include "global.hxx"

class SvXMLExportItemMapper;
class SfxPoolItem;
class SfxItemSet;
class OUStrings_Impl;
class OUStringsSort_Impl;
class SvXMLNumFmtExport;
class ScDocument;
class ScRange;
class ScRangeList;
class ScHorizontalCellIterator;
struct ScQueryEntry;
struct ScQueryParam;

struct ScMyRowFormatRange
{
    com::sun::star::table::CellRangeAddress aRangeAddress;
    sal_Int32   nStartColumn;
    sal_Int32   nRepeatColumns;
    sal_Int32   nRepeatRows;
    sal_Int32   nIndex;
    sal_Bool    bIsAutoStyle;

    ScMyRowFormatRange();
};

typedef std::vector<ScMyRowFormatRange> ScMyRowFormatRangesVec;

class ScRowFormatRanges
{
    ScMyRowFormatRangesVec      aRowFormatRanges;

public:
    ScRowFormatRanges();
    ~ScRowFormatRanges();

    void Clear();
    void AddRange(const ScMyRowFormatRange& aFormatRange);
    sal_Bool GetNext(ScMyRowFormatRange& aFormatRange);
    sal_Int32 GetMaxRows();
    sal_Int32 GetSize();
    void Sort();
};

typedef std::vector<rtl::OUString*>     ScMyOUStringVec;

struct ScMyFormatRange
{
    com::sun::star::table::CellRangeAddress aRangeAddress;
    sal_Int32                               nStyleNameIndex;
    sal_Bool                                bIsAutoStyle;

    ScMyFormatRange();
};

typedef std::vector<ScMyFormatRange>            ScMyFormatRangeAddresses;
typedef std::vector<ScMyFormatRangeAddresses*>  ScMyFormatRangeVectorVec;

class ScFormatRangeStyles
{
    ScMyFormatRangeVectorVec    aTables;
    ScMyOUStringVec             aStyleNames;
    ScMyOUStringVec             aAutoStyleNames;

public:
    ScFormatRangeStyles();
    ~ScFormatRangeStyles();

    void AddNewTable(const sal_Int16 nTable);
    sal_Int32 AddStyleName(rtl::OUString* pString, const sal_Bool bIsAutoStyle = sal_True);
    sal_Int32 GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix, sal_Bool& bIsAutoStyle);
    sal_Int32 GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow, sal_Bool& bIsAutoStyle);
    void GetFormatRanges(const sal_Int32 nStartColumn, const sal_Int32 nEndColumn, const sal_Int32 nRow,
                    const sal_Int16 nTable, ScRowFormatRanges& aFormatRanges);
    void AddRangeStyleName(const com::sun::star::table::CellRangeAddress aCellRangeAddress, const sal_Int32 nStringIndex, const sal_Bool bIsAutoStyle);
    rtl::OUString* GetStyleName(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow);
    rtl::OUString* GetStyleNameByIndex(const sal_Int32 nIndex, const sal_Bool bIsAutoStyle);
    void Sort();
};

typedef std::vector<sal_Int32>          ScMysalInt32Vec;
typedef std::vector<ScMysalInt32Vec>    ScMyVectorVec;

class ScColumnRowStyles
{
    ScMyVectorVec               aTables;
    ScMyOUStringVec             aStyleNames;

public:
    ScColumnRowStyles();
    ~ScColumnRowStyles();

    void AddNewTable(const sal_Int16 nTable, const sal_Int32 nFields);
    sal_Int32 AddStyleName(rtl::OUString* pString);
    sal_Int32 GetIndexOfStyleName(const rtl::OUString& rString, const rtl::OUString& rPrefix);
    sal_Int32 GetStyleNameIndex(const sal_Int16 nTable, const sal_Int32 nField);
    void AddFieldStyleName(const sal_Int16 nTable, const sal_Int32 nField, const sal_Int32 nStringIndex);
    rtl::OUString* GetStyleName(const sal_Int16 nTable, const sal_Int32 nField);
    rtl::OUString* GetStyleNameByIndex(const sal_Int32 nIndex);
};

struct ScMyShape
{
    ScAddress   aAddress;
    sal_Int32   nIndex;
};

typedef std::vector<ScMyShape>      ScMyShapes;
typedef std::vector<ScMyShapes>     ScMyShapesVector;

class ScShapesContainer
{
    ScMyShapesVector    aDrawPages;
public:
    ScShapesContainer();
    ~ScShapesContainer();
    void AddNewTable();
    void AddNewShape(const sal_Int16 nTable, const ScMyShape& aShape);
    sal_Bool HasShape(const sal_Int16 nTable, const sal_Int32 nColumn, const sal_Int32 nRow);
    sal_Bool GetNextShape(const sal_Int16 nTable, ScMyShape& aShape);
    void Sort();
};

struct ScMyRange
{
    com::sun::star::table::CellRangeAddress aCellRange;
    sal_Int32                               nRows;
    sal_Bool                                bIsFirst;
};

typedef std::vector<ScMyRange> ScMyMergedRanges;
typedef std::vector<ScMyMergedRanges> ScMyMergedRangesVec;

class ScMyMergedCells
{
    ScMyMergedRangesVec                     aTables;
public:
    ScMyMergedCells();
    ~ScMyMergedCells();
    void AddNewTable();
    void AddRange(const sal_Int16 nTable, const com::sun::star::table::CellRangeAddress aMergedRange);
    sal_Bool GetNextMergedRange(const sal_Int16 nTable, ScMyRange& aMyRange);
    void SortAndRemoveDoublets();
};

typedef std::vector<com::sun::star::table::CellRangeAddress> ScMyEmptyDatabaseRangesVec;
typedef std::vector<ScMyEmptyDatabaseRangesVec> ScMyEmptyDatabaseRangesVecVec;

class ScMyEmptyDatabaseRanges
{
    ScMyEmptyDatabaseRangesVecVec           aTables;
public:
    ScMyEmptyDatabaseRanges(const sal_Int16 nTables);
    ~ScMyEmptyDatabaseRanges();
    void AddNewEmptyDatabaseRange(const com::sun::star::table::CellRangeAddress& aCellRangeAddress);
    sal_Bool GetNextEmptyDatabaseRange(const sal_Int16 nTable, com::sun::star::table::CellRangeAddress& aCellRange);
    void Sort();
};

struct ScMyValidationRange
{
    com::sun::star::table::CellRangeAddress aRange;
    rtl::OUString sName;
    sal_Int32 nIndex;
    sal_Bool bUsed;

    ScMyValidationRange();
    ~ScMyValidationRange();
};

struct ScMyValidation
{
    rtl::OUString sName;
    rtl::OUString sErrorMessage;
    rtl::OUString sErrorTitle;
    rtl::OUString sImputMessage;
    rtl::OUString sImputTitle;
    rtl::OUString sFormula1;
    rtl::OUString sFormula2;
    com::sun::star::table::CellAddress aBaseCell;
    com::sun::star::sheet::ValidationAlertStyle aAlertStyle;
    com::sun::star::sheet::ValidationType aValidationType;
    com::sun::star::sheet::ConditionOperator aOperator;
    sal_Bool bShowErrorMessage;
    sal_Bool bShowImputMessage;
    sal_Bool bIgnoreBlanks;

    ScMyValidation();
    ~ScMyValidation();

    sal_Bool IsEqual(const ScMyValidation& aVal) const;
};

struct ScMyCell
{
    com::sun::star::uno::Reference<com::sun::star::table::XCell> xCell;
    com::sun::star::table::CellAddress  aCellAddress;
    com::sun::star::table::CellRangeAddress aMatrixRange;
    com::sun::star::table::CellRangeAddress aMergeRange;
    std::vector<ScMyShape>              aShapes;
    sal_Int32                           nValidationIndex;
    sal_Bool                            bHasShape;
    sal_Bool                            bIsMergedBase;
    sal_Bool                            bIsCovered;
    sal_Bool                            bHasAnnotation;
    sal_Bool                            bIsMatrixBase;
    sal_Bool                            bIsMatrixCovered;

    ScMyCell();
    ~ScMyCell();
};

class ScMyNotEmptyCellsIterator;
class ScMyValidations;

class ScXMLExport : public SvXMLExport
{
    ScDocument*                 pDoc;
    com::sun::star::uno::Reference <com::sun::star::frame::XModel> xModel;
    com::sun::star::uno::Reference <com::sun::star::util::XStringMapping> xChartExportMapper;
    com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet> xCurrentTable;
    com::sun::star::uno::Reference <com::sun::star::container::XIndexAccess> xCurrentShapes;

//  ULONG                       nDateKey;
//  ULONG                       nTimeKey;
//  ULONG                       nDateTimeKey;

//  SvXMLExportItemMapper       *pParaItemMapper;
//  SvXMLExportItemMapper       *pTableItemMapper;

//  SvXMLAutoStylePoolP         *pScAutoStylePool;
    XMLScPropHdlFactory         *pScPropHdlFactory;
    XMLCellStylesPropertySetMapper      *pCellStylesPropertySetMapper;
    XMLColumnStylesPropertySetMapper    *pColumnStylesPropertySetMapper;
    XMLRowStylesPropertySetMapper       *pRowStylesPropertySetMapper;
    XMLTableStylesPropertySetMapper     *pTableStylesPropertySetMapper;
    ScColumnRowStyles                   aColumnStyles;
    ScColumnRowStyles                   aRowStyles;
    ScFormatRangeStyles                 aCellStyles;
    ScShapesContainer                   aShapesContainer;
    ScMyMergedCells                     aMergedCells;
    ScRowFormatRanges                   aRowFormatRanges;
    std::vector<rtl::OUString>          aTableStyles;
    com::sun::star::table::CellRangeAddress aRowHeaderRange;

    sal_Bool                    bHasRowHeader;
    sal_Bool                    bRowHeaderOpen;
    sal_Bool                    mbShowProgress : 1;
    sal_Int32                   nOpenRow;
    sal_Int16                   nCurrentTable;
    std::vector<sal_Int32>      nLastColumns;
    std::vector<sal_Int32>      nLastRows;

    ScMyNotEmptyCellsIterator*  pCellsItr;
    ScMyValidations*            pValidations;

    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

    void SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol);// { if(nCol > nLastColumns[nTable]) nLastColumns[nTable] = nCol; }
    void SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow);// { if(nRow > nLastRows[nTable]) nLastRows[nTable] = nRow; }

    com::sun::star::table::CellRangeAddress GetEndAddress(com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& xTable,
                                                        const sal_Int16 nTable);
    ScMyEmptyDatabaseRanges GetEmptyDatabaseRanges(const sal_Int16 nTableCount);
    sal_Bool GetxCurrentShapes(com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess>& xShapes);
    void WriteColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex, const sal_Bool bIsVisible);
    void ExportColumns(const sal_Int16 nTable, const com::sun::star::table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader);
    void ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
        const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_Int16 nSheet);
    void WriteRowContent();
    void WriteRowStartTag(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nEmptyRows);
    void OpenHeaderRows();
    void OpenNewRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenRow(const sal_Int16 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow);
    void CloseRow(const sal_Int32 nRow);
    sal_Bool GetColumnHeader(com::sun::star::table::CellRangeAddress& aColumnHeaderRange) const;
    sal_Bool GetRowHeader(com::sun::star::table::CellRangeAddress& aRowHeaderRange) const;

    sal_Bool IsMerged (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
        const sal_Int32 nCol, const sal_Int32 nRow,
        com::sun::star::table::CellRangeAddress& aCellAddress) const;

//  sal_Bool GetCell (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
//      const sal_Int32 nCol, const sal_Int32 nRow, com::sun::star::uno::Reference <com::sun::star::table::XCell>& xTempCell) const;
    sal_Bool GetCellText (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell,
        rtl::OUString& sOUTemp) const;
    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard);
    sal_Int32 GetCellNumberFormat(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const;
    sal_Bool GetCellStyleNameIndex(const ScMyCell& aCell, sal_Int32& nStyleNameIndex, sal_Bool& bIsAutoStyle);

    rtl::OUString GetPrintRanges();

    void WriteCell (const ScMyCell& aCell);
    void WriteAnnotation(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell);
    void WriteShapes(const ScMyCell& aCell);
    void SetRepeatAttribute (const sal_Int32 nEqualCellCount);

    sal_Bool IsCellTypeEqual (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell1,
        const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell2) const;
    sal_Bool IsEditCell(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const;
    sal_Bool IsAnnotationEqual(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell1,
                                const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell2);
    sal_Bool IsCellEqual (const ScMyCell& aCell1, const ScMyCell& aCell2);

    void GetStringFromRange(const ScRange& aRange, rtl::OUString& rString) const;
    void AddStringFromRange(const ScRange& aRange, rtl::OUString& rString) const;
    void GetStringFromRangeList(const ScRangeList* pRangeList, rtl::OUString& rString) const;
    void GetStringFromRange(const com::sun::star::table::CellRangeAddress& aRange, rtl::OUString& rString) const;
    void AddStringFromRange(const com::sun::star::table::CellRangeAddress& aRange, rtl::OUString& rString) const;

    void GetStringOfFunction(const sal_Int32 nFunction, rtl::OUString& rString) const;
    void WriteScenario();
    void WriteNamedExpressions(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteImportDescriptor(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aImportDescriptor);
    rtl::OUString getOperatorXML(const com::sun::star::sheet::FilterOperator aFilterOperator, const sal_Bool bUseRegularExpressions) const;
    void WriteCondition(const com::sun::star::sheet::TableFilterField& aFilterField, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteFilterDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSheetFilterDescriptor>& xSheetFilterDescriptor, const rtl::OUString sDatabaseRangeName);
    void WriteSortDescriptor(const com::sun::star::uno::Sequence <com::sun::star::beans::PropertyValue> aSortProperties);
    void WriteSubTotalDescriptor(const com::sun::star::uno::Reference <com::sun::star::sheet::XSubTotalDescriptor> xSubTotalDescriptor, const rtl::OUString sDatabaseRangeName);
    void WriteDatabaseRanges(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    rtl::OUString getDPOperatorXML(const ScQueryOp aFilterOperator, const sal_Bool bUseRegularExpressions,
                                    const sal_Bool bIsString, const double dVal) const;
    void WriteDPCondition(const ScQueryEntry& aQueryEntry, sal_Bool bIsCaseSensitive, sal_Bool bUseRegularExpressions);
    void WriteDPFilter(const ScQueryParam& aQueryParam);
    void WriteDataPilots(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreaDoc);
protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
public:
    ScXMLExport( const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& xTempModel, const ::rtl::OUString& rFileName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XDocumentHandler>& rHandler,
                 sal_Bool bShowProgr );
    virtual ~ScXMLExport();

    com::sun::star::uno::Reference <com::sun::star::frame::XModel>& GetXModel() { return xModel; }
    const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& GetXModel() const { return xModel; }

    static sal_Int16 GetFieldUnit();
    ScDocument* GetDocument();
    sal_Int32 GetLastColumn(const sal_Int32 nTable);// { return nLastColumns[nTable]; }
    sal_Int32 GetLastRow(const sal_Int32 nTable);// { return nLastRows[nTable]; }
    sal_Bool IsMatrix (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
        const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet>& xTable,
        const sal_Int32 nCol, const sal_Int32 nRow,
        com::sun::star::table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const;

    XMLCellStylesPropertySetMapper* GetCellStylesPropertySetMapper() { return pCellStylesPropertySetMapper; }

    ScMyValidations* GetValidations() { return pValidations; }

//  SvXMLExportItemMapper& GetParaItemMapper() { return *pParaItemMapper; }
//  SvXMLExportItemMapper& GetTableItemMapper() { return *pTableItemMapper; }
//  SwXMLAutoStylePool& GetAutoStylePool() { return *pAutoStylePool; }
};

class ScMyValidations
{
    std::vector<ScMyValidation>     aValidations;
    std::vector<ScMyValidationRange> aValidationRanges;
    rtl::OUString                   sEmptyString;
public:
    ScMyValidations();
    ~ScMyValidations();
    sal_Bool AddValidation(const com::sun::star::uno::Any& aAny,
        const com::sun::star::table::CellRangeAddress& aCellRange);
    rtl::OUString GetCondition(const ScMyValidation& aValidation);
    rtl::OUString GetBaseCellAddress(ScDocument* pDoc, const com::sun::star::table::CellAddress& aCell);
    void WriteMessage(ScXMLExport& rExport,
        const rtl::OUString& sTitle, const rtl::OUString& sMessage,
        const sal_Bool bShowMessage, const sal_Bool bIsHelpMessage);
    void WriteValidations(ScXMLExport& rExport);
    const rtl::OUString& GetValidationName(const sal_Int32 nIndex);
    const sal_Int32 GetValidationIndex(const com::sun::star::table::CellAddress& aCell);
    void Sort();
};

class ScMyNotEmptyCellsIterator
{
    ScShapesContainer*                      pShapes;
    ScMyEmptyDatabaseRanges*                pEmptyDatabaseRanges;
    ScMyMergedCells*                        pMergedCells;
    com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet> xTable;

    ScXMLExport&                            rExport;
    ScHorizontalCellIterator*               pCellItr;

    ScMyCell                                aCurrentCell;
    ScMyShape                               aCurrentShape;
    ScMyRange                               aNextMergedCells;
    com::sun::star::table::CellRangeAddress aNextEmptyCells;
    sal_Int16                               nCurrentTable;

    sal_Bool                                bHasShapes; // Current Table has shapes
    sal_Bool                                bHasShape; // Current Cell has shapes
    sal_Bool                                bHasEmptyDatabaseRanges; // Current Table has empty DatabaseRanges
    sal_Bool                                bIsEmptyDatabaseRange; // Current Cell is in a empty Database Range
    sal_Bool                                bHasMergedCells; // Current Table has merged Cells
    sal_Bool                                bIsMergedBase; // Current Cell is the left top edge of merged Cells
    sal_Bool                                bIsCovered; // Current Cell is covered by a merged Cell
    sal_Bool                                bHasAnnotation; // Current Cell has Annotation
    sal_Bool                                bIsMatrixBase; // Current Cell is the left top edge of a matrix Cell
    sal_Bool                                bIsMatrixCovered; // Current Cell is in a Matrix but not the left top edge of a matrix

    void  HasAnnotation(ScMyCell& aCell);
public:
    ScMyNotEmptyCellsIterator(ScXMLExport& rExport);
    ~ScMyNotEmptyCellsIterator();

    void SetShapes(ScShapesContainer* pTempShapes);
    void SetEmptyDatabaseRanges(ScMyEmptyDatabaseRanges* pTempEmptyDatabaseRanges);
    void SetMergedCells(ScMyMergedCells* pTempMergedCells);
    void SetCurrentTable(const sal_Int32 nTable);

    sal_Bool GetNext(ScMyCell& aCell);
};

#endif

