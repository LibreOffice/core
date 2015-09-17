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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPRT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLEXPRT_HXX

#include <xmloff/xmlexp.hxx>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include "address.hxx"

namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

#include <memory>
#include <unordered_map>

class ScOutlineArray;
class SvXMLExportPropertyMapper;
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
class XMLNumberFormatAttributesExportHelper;
class ScChartListener;
class SfxItemPool;
class ScAddress;
class ScXMLCachedRowAttrAccess;
class ScRangeName;
class ScXMLEditAttributeMap;
class EditTextObject;
class ScFormulaCell;

namespace sc {

class CompileFormulaContext;

}


class ScXMLExport : public SvXMLExport
{
    ScDocument*                 pDoc;
    com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet> xCurrentTable;
    com::sun::star::uno::Reference <com::sun::star::table::XCellRange> xCurrentTableCellRange;

    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xSourceStream;
    sal_Int32                   nSourceStreamPos;

    mutable std::unique_ptr<ScXMLEditAttributeMap> mpEditAttrMap;
    std::unique_ptr<ScMyNotEmptyCellsIterator> mpCellsItr;
    std::unique_ptr<sc::CompileFormulaContext> mpCompileFormulaCxt;
    rtl::Reference < XMLPropertyHandlerFactory >  xScPropHdlFactory;
    rtl::Reference < XMLPropertySetMapper >       xCellStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xColumnStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xRowStylesPropertySetMapper;
    rtl::Reference < XMLPropertySetMapper >       xTableStylesPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >  xCellStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >  xColumnStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >  xRowStylesExportPropertySetMapper;
    rtl::Reference < SvXMLExportPropertyMapper >  xTableStylesExportPropertySetMapper;
    XMLNumberFormatAttributesExportHelper* pNumberFormatAttributesExportHelper;
    typedef std::unordered_map<sal_Int32, sal_Int32>  NumberFormatIndexMap;
    NumberFormatIndexMap                aNumFmtIndexMap;
    ScMySharedData*                     pSharedData;
    ScColumnStyles*                 pColumnStyles;
    ScRowStyles*                    pRowStyles;
    ScFormatRangeStyles*                pCellStyles;
    ScRowFormatRanges*                  pRowFormatRanges;
    std::vector<OUString>          aTableStyles;
    com::sun::star::table::CellRangeAddress aRowHeaderRange;
    ScMyOpenCloseColumnRowGroup*        pGroupColumns;
    ScMyOpenCloseColumnRowGroup*        pGroupRows;
    ScMyDefaultStyles*                  pDefaults;
    ScChartListener*                    pChartListener;
    const ScMyCell*                     pCurrentCell;

    ScMyMergedRangesContainer*  pMergedRangesContainer;
    ScMyValidationsContainer*   pValidationsContainer;
    ScChangeTrackingExportHelper*   pChangeTrackingExportHelper;
    const OUString         sLayerID;
    const OUString         sCaptionShape;
    OUString               sExternalRefTabStyleName;
    OUString               sAttrName;
    OUString               sAttrStyleName;
    OUString               sAttrColumnsRepeated;
    OUString               sAttrFormula;
    OUString               sAttrValueType;
    OUString               sAttrStringValue;
    OUString               sElemCell;
    OUString               sElemCoveredCell;
    OUString               sElemCol;
    OUString               sElemRow;
    OUString               sElemTab;
    OUString               sElemP;
    sal_Int32                   nOpenRow;
    sal_Int32                   nProgressCount;
    sal_uInt16                  nCurrentTable;
    bool                        bHasRowHeader;
    bool                        bRowHeaderOpen;
    bool                        mbShowProgress;

    sal_Int32       GetNumberFormatStyleIndex(sal_Int32 nNumFmt) const;
    void            CollectSharedData(SCTAB& nTableCount, sal_Int32& nShapesCount);
    void            CollectShapesAutoStyles(SCTAB nTableCount);
    virtual void _ExportFontDecls() SAL_OVERRIDE;
    virtual void _ExportStyles( bool bUsed ) SAL_OVERRIDE;
    virtual void _ExportAutoStyles() SAL_OVERRIDE;
    virtual void _ExportMasterStyles() SAL_OVERRIDE;
    virtual void SetBodyAttributes() SAL_OVERRIDE;
    virtual void _ExportContent() SAL_OVERRIDE;
    virtual void _ExportMeta() SAL_OVERRIDE;

    void CollectInternalShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );

    static com::sun::star::table::CellRangeAddress GetEndAddress(const com::sun::star::uno::Reference<com::sun::star::sheet::XSpreadsheet>& xTable,
                                                        const sal_Int32 nTable);
//  ScMyEmptyDatabaseRangesContainer GetEmptyDatabaseRanges();
    void GetAreaLinks( ScMyAreaLinksContainer& rAreaLinks );
    void GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp );
    void WriteSingleColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex,
        const sal_Int32 nIndex, const bool bIsAutoStyle, const bool bIsVisible);
    void WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
        const sal_Int32 nStyleIndex, const bool bIsVisible);
    void OpenHeaderColumn();
    void CloseHeaderColumn();
    void ExportColumns(const sal_Int32 nTable, const com::sun::star::table::CellRangeAddress& aColumnHeaderRange, const bool bHasColumnHeader);
    void ExportExternalRefCacheStyles();
    void ExportCellTextAutoStyles(sal_Int32 nTable);
    void ExportFormatRanges(const sal_Int32 nStartCol, const sal_Int32 nStartRow,
        const sal_Int32 nEndCol, const sal_Int32 nEndRow, const sal_Int32 nSheet);
    void WriteRowContent();
    void WriteRowStartTag(const sal_Int32 nIndex, const sal_Int32 nEmptyRows, bool bHidden, bool bFiltered);
    void OpenHeaderRows();
    void CloseHeaderRows();
    void OpenNewRow(const sal_Int32 nIndex, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows,
                    bool bHidden, bool bFiltered);
    void OpenAndCloseRow(const sal_Int32 nIndex, const sal_Int32 nStartRow, const sal_Int32 nEmptyRows,
                         bool bHidden, bool bFiltered);
    void OpenRow(const sal_Int32 nTable, const sal_Int32 nStartRow, const sal_Int32 nRepeatRow, ScXMLCachedRowAttrAccess& rRowAttr);
    void CloseRow(const sal_Int32 nRow);
    void GetColumnRowHeader(bool& bHasColumnHeader, com::sun::star::table::CellRangeAddress& aColumnHeaderRange,
        bool& bHasRowHeader, com::sun::star::table::CellRangeAddress& aRowHeaderRange,
        OUString& rPrintRanges) const;
    static void FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups);
    void FillColumnRowGroups();

    bool GetMerged (const com::sun::star::table::CellRangeAddress* pCellRange,
        const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheet>& xTable);

    void WriteTable(sal_Int32 nTable, const ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet>& xTable);
    void WriteCell(ScMyCell& aCell, sal_Int32 nEqualCellCount);
    void WriteEditCell(const EditTextObject* pText);
    void WriteMultiLineFormulaResult(const ScFormulaCell* pCell);
    void WriteAreaLink(const ScMyCell& rMyCell);
    void WriteAnnotation(ScMyCell& rMyCell);
    void WriteDetective(const ScMyCell& rMyCell);
    void ExportShape(const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape, com::sun::star::awt::Point* pPoint);
    void WriteShapes(const ScMyCell& rMyCell);
    void WriteTableShapes();
    void SetRepeatAttribute(sal_Int32 nEqualCellCount, bool bIncProgress);

    static bool IsCellTypeEqual (const ScMyCell& aCell1, const ScMyCell& aCell2);
    static bool IsEditCell(ScMyCell& rCell);
    bool IsCellEqual(ScMyCell& aCell1, ScMyCell& aCell2);

    void WriteCalculationSettings(const com::sun::star::uno::Reference <com::sun::star::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteTableSource();
    void WriteScenario();   // core implementation
    void WriteTheLabelRanges(const com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheetDocument >& xSpreadDoc);
    void WriteLabelRanges( const com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xRangesIAccess, bool bColumn );
    void WriteNamedExpressions();
    void WriteDataStream();
    void WriteNamedRange(ScRangeName* pRangeName);
    void ExportConditionalFormat(SCTAB nTab);
    void WriteExternalRefCaches();
    void WriteConsolidation();  // core implementation

    void CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib);

    void AddStyleFromCells(
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xProperties,
        const com::sun::star::uno::Reference< com::sun::star::sheet::XSpreadsheet >& xTable,
        sal_Int32 nTable, const OUString* pOldName );
    void AddStyleFromColumn(
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xColumnProperties,
        const OUString* pOldName, sal_Int32& rIndex, bool& rIsVisible );
    void AddStyleFromRow(
        const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet >& xRowProperties,
        const OUString* pOldName, sal_Int32& rIndex );

    void IncrementProgressBar(bool bFlush, sal_Int32 nInc = 1);

    void CopySourceStream( sal_Int32 nStartOffset, sal_Int32 nEndOffset, sal_Int32& rNewStart, sal_Int32& rNewEnd );

    const ScXMLEditAttributeMap& GetEditAttributeMap() const;

protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool() SAL_OVERRIDE;
    virtual XMLPageExport* CreatePageExport() SAL_OVERRIDE;
    virtual XMLShapeExport* CreateShapeExport() SAL_OVERRIDE;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() SAL_OVERRIDE;
public:
    ScXMLExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLExportFlags nExportFlag);

    virtual ~ScXMLExport();

    static sal_Int16 GetMeasureUnit();
    inline ScDocument*          GetDocument()           { return pDoc; }
    inline const ScDocument*    GetDocument() const     { return pDoc; }
    bool IsMatrix (const ScAddress& aCell,
        com::sun::star::table::CellRangeAddress& aCellAddress, bool& bIsFirst) const;

    rtl::Reference < XMLPropertySetMapper > GetCellStylesPropertySetMapper() { return xCellStylesPropertySetMapper; }
    rtl::Reference < XMLPropertySetMapper > GetTableStylesPropertySetMapper() { return xTableStylesPropertySetMapper; }

    void SetSourceStream( const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xNewStream );

    void GetChangeTrackViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps);
    virtual void GetViewSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps) SAL_OVERRIDE;
    virtual void GetConfigurationSettings(com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProps) SAL_OVERRIDE;

    virtual void exportAnnotationMeta( const com::sun::star::uno::Reference < com::sun::star::drawing::XShape >& xShape) SAL_OVERRIDE;

    void CreateSharedData(const sal_Int32 nTableCount);
    void SetSharedData(ScMySharedData* pTemp) { pSharedData = pTemp; }
    ScMySharedData* GetSharedData() { return pSharedData; }
    XMLNumberFormatAttributesExportHelper* GetNumberFormatAttributesExportHelper();

    // Export the document.
    virtual sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) SAL_OVERRIDE;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void DisposingModel() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
