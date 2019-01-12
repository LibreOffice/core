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
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <address.hxx>

#include <memory>
#include <unordered_map>


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
} } }

namespace com { namespace sun { namespace star { namespace table { class XCellRange; } } } }
namespace com { namespace sun { namespace star { namespace sheet { class XSpreadsheet; } } } }
namespace com { namespace sun { namespace star { namespace sheet { class XSpreadsheetDocument; } } } }

namespace sc { class DataTransformation; }

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
class SfxItemPool;
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
    css::uno::Reference <css::sheet::XSpreadsheet> xCurrentTable;
    css::uno::Reference <css::table::XCellRange> xCurrentTableCellRange;

    css::uno::Reference<css::io::XInputStream> xSourceStream;
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
    std::unique_ptr<XMLNumberFormatAttributesExportHelper> pNumberFormatAttributesExportHelper;
    typedef std::unordered_map<sal_Int32, sal_Int32>  NumberFormatIndexMap;
    NumberFormatIndexMap                aNumFmtIndexMap;
    std::unique_ptr<ScMySharedData>                     pSharedData;
    std::unique_ptr<ScColumnStyles>                     pColumnStyles;
    std::unique_ptr<ScRowStyles>                        pRowStyles;
    std::unique_ptr<ScFormatRangeStyles>                pCellStyles;
    std::unique_ptr<ScRowFormatRanges>                  pRowFormatRanges;
    std::vector<OUString>               aTableStyles;
    ScRange                             aRowHeaderRange;
    std::unique_ptr<ScMyOpenCloseColumnRowGroup>        pGroupColumns;
    std::unique_ptr<ScMyOpenCloseColumnRowGroup>        pGroupRows;
    std::unique_ptr<ScMyDefaultStyles>                  pDefaults;
    const ScMyCell*                     pCurrentCell;

    std::unique_ptr<ScMyMergedRangesContainer>  pMergedRangesContainer;
    std::unique_ptr<ScMyValidationsContainer>   pValidationsContainer;
    std::unique_ptr<ScChangeTrackingExportHelper> pChangeTrackingExportHelper;
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

    sal_Int32       GetNumberFormatStyleIndex(sal_Int32 nNumFmt) const;
    void            CollectSharedData(SCTAB& nTableCount, sal_Int32& nShapesCount);
    void            CollectShapesAutoStyles(SCTAB nTableCount);
    void            RegisterDefinedStyleNames( const css::uno::Reference< css::sheet::XSpreadsheetDocument > & xSpreadDoc );
    virtual void ExportFontDecls_() override;
    virtual void ExportStyles_( bool bUsed ) override;
    virtual void ExportAutoStyles_() override;
    virtual void ExportMasterStyles_() override;
    virtual void SetBodyAttributes() override;
    virtual void ExportContent_() override;
    virtual void ExportMeta_() override;

    void CollectInternalShape( css::uno::Reference< css::drawing::XShape > const & xShape );

    static css::table::CellRangeAddress GetEndAddress(const css::uno::Reference<css::sheet::XSpreadsheet>& xTable);
    void GetAreaLinks( ScMyAreaLinksContainer& rAreaLinks );
    void GetDetectiveOpList( ScMyDetectiveOpContainer& rDetOp );
    void WriteSingleColumn(const sal_Int32 nRepeatColumns, const sal_Int32 nStyleIndex,
        const sal_Int32 nIndex, const bool bIsAutoStyle, const bool bIsVisible);
    void WriteColumn(const sal_Int32 nColumn, const sal_Int32 nRepeatColumns,
        const sal_Int32 nStyleIndex, const bool bIsVisible);
    void OpenHeaderColumn();
    void CloseHeaderColumn();
    void ExportColumns(const sal_Int32 nTable, const ScRange& aColumnHeaderRange, const bool bHasColumnHeader);
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
    void GetColumnRowHeader(bool& bHasColumnHeader, ScRange& aColumnHeaderRange,
        bool& bHasRowHeader, ScRange& aRowHeaderRange, OUString& rPrintRanges) const;
    static void FillFieldGroup(ScOutlineArray* pFields, ScMyOpenCloseColumnRowGroup* pGroups);
    void FillColumnRowGroups();

    bool GetMerged (const css::table::CellRangeAddress* pCellRange,
        const css::uno::Reference <css::sheet::XSpreadsheet>& xTable);

    void WriteTable(sal_Int32 nTable, const css::uno::Reference< css::sheet::XSpreadsheet>& xTable);
    void WriteCell(ScMyCell& aCell, sal_Int32 nEqualCellCount);
    void WriteEditCell(const EditTextObject* pText);
    void WriteMultiLineFormulaResult(const ScFormulaCell* pCell);
    void WriteAreaLink(const ScMyCell& rMyCell);
    void WriteAnnotation(ScMyCell& rMyCell);
    void WriteDetective(const ScMyCell& rMyCell);
    void ExportShape(const css::uno::Reference < css::drawing::XShape >& xShape, css::awt::Point* pPoint);
    void WriteShapes(const ScMyCell& rMyCell);
    void WriteTableShapes();
    void SetRepeatAttribute(sal_Int32 nEqualCellCount, bool bIncProgress);

    static bool IsEditCell(const ScMyCell& rCell);
    bool IsCellEqual(ScMyCell& aCell1, ScMyCell& aCell2);

    void WriteCalculationSettings(const css::uno::Reference <css::sheet::XSpreadsheetDocument>& xSpreadDoc);
    void WriteTableSource();
    void WriteScenario();   // core implementation
    void WriteTheLabelRanges(const css::uno::Reference< css::sheet::XSpreadsheetDocument >& xSpreadDoc);
    void WriteLabelRanges( const css::uno::Reference< css::container::XIndexAccess >& xRangesIAccess, bool bColumn );
    void WriteNamedExpressions();
    void WriteExternalDataMapping();
    void WriteExternalDataTransformations(const std::vector<std::shared_ptr<sc::DataTransformation>>& aDataTransformations);
    void WriteDataStream();
    void WriteNamedRange(ScRangeName* pRangeName);
    void ExportConditionalFormat(SCTAB nTab);
    void WriteExternalRefCaches();
    void WriteConsolidation();  // core implementation

    void CollectUserDefinedNamespaces(const SfxItemPool* pPool, sal_uInt16 nAttrib);

    void AddStyleFromCells(
        const css::uno::Reference< css::beans::XPropertySet >& xProperties,
        const css::uno::Reference< css::sheet::XSpreadsheet >& xTable,
        sal_Int32 nTable, const OUString* pOldName );
    void AddStyleFromColumn(
        const css::uno::Reference< css::beans::XPropertySet >& xColumnProperties,
        const OUString* pOldName, sal_Int32& rIndex, bool& rIsVisible );
    void AddStyleFromRow(
        const css::uno::Reference< css::beans::XPropertySet >& xRowProperties,
        const OUString* pOldName, sal_Int32& rIndex );

    void IncrementProgressBar(bool bFlush, sal_Int32 nInc = 1);

    void CopySourceStream( sal_Int32 nStartOffset, sal_Int32 nEndOffset, sal_Int32& rNewStart, sal_Int32& rNewEnd );

    const ScXMLEditAttributeMap& GetEditAttributeMap() const;

protected:
    virtual SvXMLAutoStylePoolP* CreateAutoStylePool() override;
    virtual XMLPageExport* CreatePageExport() override;
    virtual XMLShapeExport* CreateShapeExport() override;
    virtual XMLFontAutoStylePool* CreateFontAutoStylePool() override;
public:
    ScXMLExport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString const & implementationName, SvXMLExportFlags nExportFlag);

    virtual ~ScXMLExport() override;

    void collectAutoStyles() override;

    static sal_Int16 GetMeasureUnit();
    ScDocument*          GetDocument()           { return pDoc; }
    const ScDocument*    GetDocument() const     { return pDoc; }
    bool IsMatrix (const ScAddress& aCell,
        ScRange& aCellAddress, bool& bIsFirst) const;

    const rtl::Reference < XMLPropertySetMapper >& GetCellStylesPropertySetMapper() { return xCellStylesPropertySetMapper; }
    const rtl::Reference < XMLPropertySetMapper >& GetTableStylesPropertySetMapper() { return xTableStylesPropertySetMapper; }

    void SetSourceStream( const css::uno::Reference<css::io::XInputStream>& xNewStream );

    void GetChangeTrackViewSettings(css::uno::Sequence<css::beans::PropertyValue>& rProps);
    virtual void GetViewSettings(css::uno::Sequence<css::beans::PropertyValue>& rProps) override;
    virtual void GetConfigurationSettings(css::uno::Sequence<css::beans::PropertyValue>& rProps) override;

    virtual void exportAnnotationMeta( const css::uno::Reference < css::drawing::XShape >& xShape) override;

    void SetSharedData(std::unique_ptr<ScMySharedData> pTemp);
    ScMySharedData* GetSharedData() { return pSharedData.get(); }
    std::unique_ptr<ScMySharedData> ReleaseSharedData();
    XMLNumberFormatAttributesExportHelper* GetNumberFormatAttributesExportHelper();

    // Export the document.
    virtual ErrCode exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) override;

    // XExporter
    virtual void SAL_CALL setSourceDocument( const css::uno::Reference< css::lang::XComponent >& xDoc ) override;

    // XFilter
    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;
    virtual void SAL_CALL cancel() override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    virtual void DisposingModel() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
