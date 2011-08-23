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

#ifndef SC_XMLEXPRT_HXX
#define SC_XMLEXPRT_HXX

#include <bf_xmloff/xmlexp.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/drawing/XShapes.hdl>
namespace binfilter {

class SfxItemPool;
class ScOutlineArray;
class ScMyShapesContainer;
class ScMyMergedRangesContainer;
class ScMyValidationsContainer;
class ScMyNotEmptyCellsIterator;
class ScChangeTrackingExportHelper;
class ScColumnStyles;
class ScRowStyles;
class ScFormatRangeStyles;
class ScRowFormatRanges;
class ScMyOpenCloseColumnRowGroup;
class ScMyAreaLinksContainer;
class ScMyDetectiveOpContainer;
struct ScMyCell;
class ScDocument;
class ScMySharedData;
class ScMyDefaultStyles;
class ScChartListener;
class SvXMLExportPropertyMapper;
class XMLNumberFormatAttributesExportHelper;

typedef std::vector< ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShapes > > ScMyXShapesVec;

class ScXMLExport : public SvXMLExport
{
    ScDocument*					pDoc;
    ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheet> xCurrentTable;

    UniReference < XMLPropertyHandlerFactory >	xScPropHdlFactory;
    UniReference < XMLPropertySetMapper >		xCellStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >		xColumnStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >		xRowStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >		xTableStylesPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >	xCellStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >	xColumnStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >	xRowStylesExportPropertySetMapper;
    UniReference < SvXMLExportPropertyMapper >	xTableStylesExportPropertySetMapper;
    XMLNumberFormatAttributesExportHelper* pNumberFormatAttributesExportHelper;
    ScMySharedData*						pSharedData;
    ScColumnStyles*					pColumnStyles;
    ScRowStyles*					pRowStyles;
    ScFormatRangeStyles*				pCellStyles;
    ScRowFormatRanges*					pRowFormatRanges;
    std::vector<rtl::OUString>			aTableStyles;
    ::com::sun::star::table::CellRangeAddress	aRowHeaderRange;
    ScMyOpenCloseColumnRowGroup*		pGroupColumns;
    ScMyOpenCloseColumnRowGroup*		pGroupRows;
    ScMyDefaultStyles*					pDefaults;
    ScChartListener*					pChartListener;

    ScMyMergedRangesContainer*	pMergedRangesContainer;
    ScMyValidationsContainer*	pValidationsContainer;
    ScMyNotEmptyCellsIterator*	pCellsItr;
    ScChangeTrackingExportHelper*	pChangeTrackingExportHelper;
    const ::rtl::OUString			sLayerID;
    const ::rtl::OUString			sCaptionShape;
    ::rtl::OUString               sAttrName;
    ::rtl::OUString               sAttrStyleName;
    ::rtl::OUString               sAttrColumnsRepeated;
    ::rtl::OUString               sAttrFormula;
    ::rtl::OUString               sAttrStringValue;
    ::rtl::OUString               sElemCell;
    ::rtl::OUString               sElemCoveredCell;
    ::rtl::OUString               sElemCol;
    ::rtl::OUString               sElemRow;
    ::rtl::OUString               sElemTab;
    ::rtl::OUString               sElemP;
    sal_Int32					nOpenRow;
    sal_uInt16					nCurrentTable;
    sal_Bool					bHasRowHeader : 1;
    sal_Bool					bRowHeaderOpen : 1;
    sal_Bool					mbShowProgress : 1;


    sal_Bool		HasDrawPages(::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xDoc);
    void			CollectSharedData(sal_Int32& nTableCount, sal_Int32& nShapesCount, const sal_Int32 nCellCount);
    void			CollectShapesAutoStyles(const sal_Int32 nTableCount);
    void			WriteTablesView(const ::com::sun::star::uno::Any& aTableView);
    void			WriteView(const ::com::sun::star::uno::Any& aView);
    virtual void _ExportFontDecls();
    virtual void _ExportStyles( sal_Bool bUsed );
    virtual void _ExportAutoStyles();
    virtual void _ExportMasterStyles();
    virtual void SetBodyAttributes();
    virtual void _ExportContent();
    virtual void _ExportMeta();

    void CollectInternalShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );

    ::com::sun::star::table::CellRangeAddress GetEndAddress(::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet>& xTable,
                                                        const sal_uInt16 nTable);
//	ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
    void GetAreaLinks( ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc, ScMyAreaLinksContainer& rAreaLinks );
    void GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp );
    void WriteSingleColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex,
        const sal_Int32 nIndex, const sal_Bool bIsAutoStyle, const sal_Bool bIsVisible);
    void WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
        const sal_Int32 nStyleIndex, const sal_Bool bIsVisible);
    void OpenHeaderColumn();
    void CloseHeaderColumn();
    void ExportColumns(const sal_uInt16 nTable, const ::com::sun::star::table::CellRangeAddress& aColumnHeaderRange, const sal_Bool bHasColumnHeader);
    void ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
        const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_uInt16 nSheet);
    void WriteRowContent();
    void WriteRowStartTag(sal_Int32 nRow, const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nEmptyRows);
    void OpenHeaderRows();
    void CloseHeaderRows();
    void OpenNewRow(const sal_Int32 nIndex, const sal_Int8 nFlag, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int8 nFlag,
        const sal_Int32 nStartRow, const sal_Int32 nEmptyRows);
    void OpenRow(const sal_uInt16 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow);
    void CloseRow(const sal_Int32 nRow);
    sal_Bool GetColumnHeader(::com::sun::star::table::CellRangeAddress& aColumnHeaderRange) const;
    sal_Bool GetRowHeader(::com::sun::star::table::CellRangeAddress& aRowHeaderRange) const;
    void FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups);
    void FillColumnRowGroups();

    sal_Bool GetMerged (const ::com::sun::star::table::CellRangeAddress* pCellRange,
        const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheet>& xTable);

    sal_Bool GetCellText (ScMyCell& rMyCell) const;

    ::rtl::OUString GetPrintRanges();

    void WriteCell (ScMyCell& aCell);
    void WriteAreaLink(const ScMyCell& rMyCell);
    void WriteAnnotation(const ScMyCell& rMyCell);
    void WriteDetective(const ScMyCell& rMyCell);
    void ExportShape(const ::com::sun::star::uno::Reference < ::com::sun::star::drawing::XShape >& xShape, ::com::sun::star::awt::Point* pPoint);
    void WriteShapes(const ScMyCell& rMyCell);
    void WriteTableShapes();
    void SetRepeatAttribute (const sal_Int32 nEqualCellCount);

    sal_Bool IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2) const;
    sal_Bool IsEditCell(const ::com::sun::star::table::CellAddress& aAddress) const;
    sal_Bool IsEditCell(const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell>& xCell) const;
    sal_Bool IsEditCell(ScMyCell& rCell) const;
    sal_Bool IsAnnotationEqual(const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell>& xCell1,
                                const ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell>& xCell2);
    sal_Bool IsCellEqual (ScMyCell& aCell1, ScMyCell& aCell2);

    void WriteCalculationSettings(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteTableSource();
    void WriteScenario();	// core implementation
    void WriteTheLabelRanges(const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
    void WriteLabelRanges( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& xRangesIAccess, sal_Bool bColumn );
    void WriteNamedExpressions(const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteConsolidation();	// core implementation

    void CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib);
protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool();
    virtual XMLPageExport* CreatePageExport();
    virtual XMLShapeExport* CreateShapeExport();
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool();
public:
    // #110680#
    ScXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const sal_uInt16 nExportFlag);

    virtual ~ScXMLExport();

    static sal_Int16 GetFieldUnit();
    inline ScDocument*			GetDocument()			{ return pDoc; }
    inline const ScDocument*	GetDocument() const		{ return pDoc; }
    sal_Bool IsMatrix (const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell>& xCell,
        const ::com::sun::star::uno::Reference < ::com::sun::star::sheet::XSpreadsheet>& xTable,
        const sal_Int32 nCol, const sal_Int32 nRow,
        ::com::sun::star::table::CellRangeAddress& aCellAddress, sal_Bool& bIsFirst) const;

    UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() { return xCellStylesPropertySetMapper; }
    UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper() { return xTableStylesPropertySetMapper; }

    void GetChangeTrackViewSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProps);
    virtual void GetViewSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProps);
    virtual void GetConfigurationSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProps);

    void CreateSharedData(const sal_Int32 nTableCount);
    void SetSharedData(ScMySharedData* pTemp) { pSharedData = pTemp; }
    ScMySharedData* GetSharedData() { return pSharedData; }
    XMLNumberFormatAttributesExportHelper* GetNumberFormatAttributesExportHelper();

    // Export the document.
    virtual sal_uInt32 exportDoc( enum ::binfilter::xmloff::token::XMLTokenEnum eClass = ::binfilter::xmloff::token::XML_TOKEN_INVALID );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel() throw(::com::sun::star::uno::RuntimeException);

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    virtual void DisposingModel();
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
