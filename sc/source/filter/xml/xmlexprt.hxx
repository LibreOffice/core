/*************************************************************************
 *
 *  $RCSfile: xmlexprt.hxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-09 05:56:35 $
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

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HDL_
#include <com/sun/star/drawing/XShapes.hdl>
#endif

class ScOutlineArray;
class SvXMLExportPropertyMapper;
class ScMyShapesContainer;
class ScMyMergedRangesContainer;
class ScMyValidationsContainer;
class ScMyDetectiveObjContainer;
class ScMyNotEmptyCellsIterator;
class ScChangeTrackingExportHelper;
class ScColumnRowStyles;
class ScFormatRangeStyles;
class ScRowFormatRanges;
class ScMyOpenCloseColumnRowGroup;
class ScMyAreaLinksContainer;
class ScMyDetectiveOpContainer;
struct ScMyCell;
class ScDocument;
class ScMySharedData;

typedef std::vector< com::sun::star::uno::Reference < com::sun::star::drawing::XShapes > > ScMyXShapesVec;

class ScXMLExport : public SvXMLExport
{
    ScDocument*                 pDoc;
    com::sun::star::uno::Reference <com::sun::star::frame::XModel> xModel;
    com::sun::star::uno::Reference <com::sun::star::util::XStringMapping> xChartExportMapper;
    com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet> xCurrentTable;
    com::sun::star::uno::Reference <com::sun::star::container::XIndexAccess> xCurrentShapes;

    UniReference < XMLPropertyHandlerFactory >  xScPropHdlFactory;
    UniReference < XMLPropertySetMapper >       xCellStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xColumnStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xRowStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >       xTableStylesPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xCellStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xColumnStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xRowStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >  xTableStylesExportPropertySetMapper;
    ScMySharedData*                     pSharedData;
    ScColumnRowStyles*                  pColumnStyles;
    ScColumnRowStyles*                  pRowStyles;
    ScFormatRangeStyles*                pCellStyles;
    ScRowFormatRanges*                  pRowFormatRanges;
    std::vector<rtl::OUString>          aTableStyles;
    ScMyXShapesVec                      aXShapesVec;
    com::sun::star::table::CellRangeAddress aRowHeaderRange;
    ScMyOpenCloseColumnRowGroup*        pGroupColumns;
    ScMyOpenCloseColumnRowGroup*        pGroupRows;

    ScMyMergedRangesContainer*  pMergedRangesContainer;
    ScMyValidationsContainer*   pValidationsContainer;
    ScMyDetectiveObjContainer*  pDetectiveObjContainer;
    ScMyNotEmptyCellsIterator*  pCellsItr;
    ScChangeTrackingExportHelper*   pChangeTrackingExportHelper;
    sal_Int32                   nOpenRow;
    sal_Int16                   nCurrentTable;
    sal_Bool                    bHasRowHeader : 1;
    sal_Bool                    bRowHeaderOpen : 1;
    sal_Bool                    mbShowProgress : 1;
    sal_Bool                    bShapeStyles : 1;


    void            CollectSharedData(sal_Int32& nTableCount, sal_Int32& nShapesCount, const sal_Int32 nCellCount);
    void            CollectShapesAutoStyles(com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& xTable, const sal_Int32 nTable);
    void            WriteTablesView(const com::sun::star::uno::Any& aTableView);
    void            WriteView(const com::sun::star::uno::Any& aView);
    virtual void _ExportViewSettings();
    virtual void _ExportFontDecls();
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void SetBodyAttributes();
    virtual void _ExportContent();
    virtual void _ExportMeta();

    void CollectInternalShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );

    com::sun::star::table::CellRangeAddress GetEndAddress(com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& xTable,
                                                        const sal_Int16 nTable);
//  ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
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
    void FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups);
    void FillColumnRowGroups();

    sal_Bool GetMerge (const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet>& xTable,
        sal_Int32 nCol, sal_Int32 nRow,
        com::sun::star::table::CellRangeAddress& aCellAddress);
    void GetMerged (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
        const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet>& xTable);

    sal_Bool GetCellText (const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell,
        rtl::OUString& sOUTemp) const;
    sal_Int16 GetCellType(const sal_Int32 nNumberFormat, sal_Bool& bIsStandard);
    sal_Int32 GetCellNumberFormat(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const;
    sal_Bool GetCellStyleNameIndex(const ScMyCell& aCell, sal_Int32& nStyleNameIndex, sal_Bool& bIsAutoStyle, sal_Int32& nValidationIndex);

    rtl::OUString GetPrintRanges();

    void WriteCell (const ScMyCell& aCell);
    void WriteAreaLink(const ScMyCell& rMyCell);
    void WriteAnnotation(const ScMyCell& rMyCell);
    void WriteDetective(const ScMyCell& rMyCell);
    void ExportShape(const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape, com::sun::star::awt::Point* pPoint);
    void WriteShapes(const ScMyCell& rMyCell);
    void WriteTableShapes();
    void SetRepeatAttribute (const sal_Int32 nEqualCellCount);

    sal_Bool IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2) const;
    sal_Bool IsEditCell(const com::sun::star::uno::Reference <com::sun::star::table::XCell>& xCell) const;
    sal_Bool IsAnnotationEqual(const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell1,
                                const com::sun::star::uno::Reference<com::sun::star::table::XCell>& xCell2);
    sal_Bool IsCellEqual (const ScMyCell& aCell1, const ScMyCell& aCell2);

    void WriteCalculationSettings(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteTableSource();
    void WriteScenario();   // core implementation
    void WriteTheLabelRanges(const com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
    void WriteLabelRanges( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xRangesIAccess, sal_Bool bColumn );
    void WriteNamedExpressions(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteConsolidation();  // core implementation
protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();
public:
    ScXMLExport(const sal_uInt16 nExportFlag);
    virtual ~ScXMLExport();

    // XExporter
    virtual void SAL_CALL setSourceDocument(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
                throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Reference <com::sun::star::frame::XModel>& GetXModel() { return xModel; }
    const com::sun::star::uno::Reference <com::sun::star::frame::XModel>& GetXModel() const { return xModel; }

    static sal_Int16 GetFieldUnit();
    inline ScDocument*          GetDocument()           { return pDoc; }
    inline const ScDocument*    GetDocument() const     { return pDoc; }
    sal_Bool IsMatrix (const com::sun::star::uno::Reference <com::sun::star::table::XCellRange>& xCellRange,
        const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet>& xTable,
        const sal_Int32 nCol, const sal_Int32 nRow,
        com::sun::star::table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const;

    UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() { return xCellStylesPropertySetMapper; }

    void CreateSharedData(const sal_Int32 nTableCount);
    void SetSharedData(ScMySharedData* pTemp) { pSharedData = pTemp; }
    ScMySharedData* GetSharedData() { return pSharedData; }
};

#endif

