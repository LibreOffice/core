/*************************************************************************
 *
 *  $RCSfile: xmlexprt.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-10 09:57:28 $
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
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETFILTERDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALDESCRIPTOR_HPP_
#include <com/sun/star/sheet/XSubTotalDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XLABELRANGES_HPP_
#include <com/sun/star/sheet/XLabelRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_GENERALFUNCTION_HPP_
#include <com/sun/star/sheet/GeneralFunction.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif
#ifndef SC_XMLMAPCH_HXX_
#include "xmlmapch.hxx"
#endif
#ifndef _SC_XMLEXPORTITERATOR_HXX
#include "XMLExportIterator.hxx"
#endif

class SvXMLExportItemMapper;
class SfxPoolItem;
class SfxItemSet;
class OUStrings_Impl;
class OUStringsSort_Impl;
class SvXMLNumFmtExport;
class ScDocument;
class ScRange;
class ScRangeList;
class ScArea;
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

struct ScMyColumnRowGroup
{
    sal_Int32   nField;
    sal_Int16   nLevel;
    sal_Bool    bDisplay;

    ScMyColumnRowGroup();
};

typedef std::vector <ScMyColumnRowGroup> ScMyColumnRowGroupVec;
typedef std::vector <sal_Int32> ScMyFieldGroupVec;

class ScMyOpenCloseColumnRowGroup
{
    ScXMLExport&                rExport;
    rtl::OUString               sName;
    ScMyColumnRowGroupVec       aTableStart;
    ScMyFieldGroupVec           aTableEnd;
    sal_Bool                    bNamespaced;

    void OpenGroup(const ScMyColumnRowGroup* pGroup);
    void CloseGroup();
public:
    ScMyOpenCloseColumnRowGroup(ScXMLExport& rExport, const sal_Char *pName);
    ~ScMyOpenCloseColumnRowGroup();

    void NewTable();
    void AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField);
    sal_Bool IsGroupStart(const sal_Int32 nField);
    void OpenGroups(const sal_Int32 nField);
    sal_Bool IsGroupEnd(const sal_Int32 nField);
    void CloseGroups(const sal_Int32 nField);
    sal_Int32 GetLast();
    void Sort();
};

class ScOutlineArray;

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
    UniReference < XMLPropertyHandlerFactory >  xScPropHdlFactory;
    UniReference < XMLPropertySetMapper >       xCellStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xColumnStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xRowStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xTableStylesPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xCellStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xColumnStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xRowStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xTableStylesExportPropertySetMapper;
    ScColumnRowStyles                   aColumnStyles;
    ScColumnRowStyles                   aRowStyles;
    ScFormatRangeStyles                 aCellStyles;
    ScRowFormatRanges                   aRowFormatRanges;
    std::vector<rtl::OUString>          aTableStyles;
    com::sun::star::table::CellRangeAddress aRowHeaderRange;
    ScMyOpenCloseColumnRowGroup         aGroupColumns;
    ScMyOpenCloseColumnRowGroup         aGroupRows;

    sal_Bool                    bHasRowHeader;
    sal_Bool                    bRowHeaderOpen;
    sal_Bool                    mbShowProgress : 1;
    sal_Int32                   nOpenRow;
    sal_Int16                   nCurrentTable;
    std::vector<sal_Int32>      nLastColumns;
    std::vector<sal_Int32>      nLastRows;

    ScMyShapesContainer         aShapesContainer;
    ScMyMergedRangesContainer   aMergedRangesContainer;
    ScMyValidationsContainer    aValidationsContainer;
    ScMyDetectiveObjContainer   aDetectiveObjContainer;
    ScMyNotEmptyCellsIterator*  pCellsItr;

    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void _ExportContent();

    void CollectInternalShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );

    void SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol);// { if(nCol > nLastColumns[nTable]) nLastColumns[nTable] = nCol; }
    void SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow);// { if(nRow > nLastRows[nTable]) nLastRows[nTable] = nRow; }

    com::sun::star::table::CellRangeAddress GetEndAddress(com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& xTable,
                                                        const sal_Int16 nTable);
    ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
    void GetAreaLinks( com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc, ScMyAreaLinksContainer& rAreaLinks );
    void GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp );
    sal_Bool GetxCurrentShapes(com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess>& xShapes);
    void WriteColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex, const sal_Bool bIsVisible);
    void OpenHeaderColumn();
    void CloseHeaderColumn();
    void ExportColumns(const sal_Int16 nTable, const com::sun::star::table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader);
    void ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
        const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_Int16 nSheet);
    void WriteRowContent();
    void WriteRowStartTag(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nEmptyRows);
    void OpenHeaderRows();
    void CloseHeaderRows();
    void OpenNewRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenRow(const sal_Int16 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow);
    void CloseRow(const sal_Int32 nRow);
    sal_Bool GetColumnHeader(com::sun::star::table::CellRangeAddress& aColumnHeaderRange) const;
    sal_Bool GetRowHeader(com::sun::star::table::CellRangeAddress& aRowHeaderRange) const;
    void FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup& rGroups);
    void FillColumnRowGroups();

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
    void WriteAreaLink(const ScMyCell& rMyCell);
    void WriteAnnotation(const ScMyCell& rMyCell);
    void WriteDetective(const ScMyCell& rMyCell);
    void WriteShapes(const ScMyCell& rMyCell);
    void SetRepeatAttribute (const sal_Int32 nEqualCellCount);

    sal_Bool IsCellTypeEqual (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell1,
        const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell2) const;
    sal_Bool IsEditCell(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const;
    sal_Bool IsAnnotationEqual(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell1,
                                const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell2);
    sal_Bool IsCellEqual (const ScMyCell& aCell1, const ScMyCell& aCell2);

    void WriteScenario();   // core implementation
    void WriteTheLabelRanges(const com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
    void WriteLabelRanges( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xRangesIAccess, sal_Bool bColumn );
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
    void WriteConsolidation();  // core implementation
protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual XMLPageExport* CreatePageExport();
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

    UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() { return xCellStylesPropertySetMapper; }

//  SvXMLExportItemMapper& GetParaItemMapper() { return *pParaItemMapper; }
//  SvXMLExportItemMapper& GetTableItemMapper() { return *pTableItemMapper; }
//  SwXMLAutoStylePool& GetAutoStylePool() { return *pAutoStylePool; }
};

#endif

