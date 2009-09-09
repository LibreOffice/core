/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: worksheetfragment.hxx,v $
 * $Revision: 1.4.20.5 $
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

#ifndef OOX_XLS_WORKSHEETFRAGMENT_HXX
#define OOX_XLS_WORKSHEETFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxDataValidationsContext : public OoxWorksheetContextBase
{
public:
    explicit            OoxDataValidationsContext( OoxWorksheetFragmentBase& rFragment );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

private:
    /** Imports the dataValidation element containing data validation settings. */
    void                importDataValidation( const AttributeList& rAttribs );
    /** Imports the DATAVALIDATION record containing data validation settings. */
    void                importDataValidation( RecordInputStream& rStrm );

private:
    ::std::auto_ptr< ValidationModel > mxValModel;
};

// ============================================================================

class OoxWorksheetFragment : public OoxWorksheetFragmentBase
{
public:
    explicit            OoxWorksheetFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

    // oox.core.FragmentHandler2 interface ------------------------------------

    virtual const ::oox::core::RecordInfo* getRecordInfos() const;
    virtual void        initializeImport();
    virtual void        finalizeImport();

private:
    /** Imports page settings from a pageSetUpPr element. */
    void                importPageSetUpPr( const AttributeList& rAttribs );
    /** Imports the dimension element containing the used area of the sheet. */
    void                importDimension( const AttributeList& rAttribs );
    /** Imports sheet format properties from a sheetFormatPr element. */
    void                importSheetFormatPr( const AttributeList& rAttribs );
    /** Imports column settings from a col element. */
    void                importCol( const AttributeList& rAttribs );
    /** Imports a merged cell range from a mergeCell element. */
    void                importMergeCell( const AttributeList& rAttribs );
    /** Imports the hyperlink element containing a hyperlink for a cell range. */
    void                importHyperlink( const AttributeList& rAttribs );
    /** Imports individual break that is either within row or column break context. */
    void                importBrk( const AttributeList& rAttribs, bool bRowBreak );
    /** Imports the the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );
    /** Imports the the relation identifier for the legacy VML drawing part. */
    void                importLegacyDrawing( const AttributeList& rAttribs );
    /** Imports additional data for an OLE object. */
    void                importOleObject( const AttributeList& rAttribs );
    /** Imports additional data for an OCX form control. */
    void                importControl( const AttributeList& rAttribs );

    /** Imports the DIMENSION record containing the used area of the sheet. */
    void                importDimension( RecordInputStream& rStrm );
    /** Imports sheet format properties from a SHEETFORMATPR record. */
    void                importSheetFormatPr( RecordInputStream& rStrm );
    /** Imports column settings from a COL record. */
    void                importCol( RecordInputStream& rStrm );
    /** Imports a merged cell range from a MERGECELL record. */
    void                importMergeCell( RecordInputStream& rStrm );
    /** Imports a hyperlink for a cell range from a HYPERLINK record. */
    void                importHyperlink( RecordInputStream& rStrm );
    /** Imports the BRK record for an individual row or column page break. */
    void                importBrk( RecordInputStream& rStrm, bool bRowBreak );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( RecordInputStream& rStrm );
    /** Imports the LEGACYDRAWING record containing the relation identifier for the VML drawing part. */
    void                importLegacyDrawing( RecordInputStream& rStrm );
    /** Imports additional data for an OLE object. */
    void                importOleObject( RecordInputStream& rStrm );
    /** Imports additional data for an OCX form control. */
    void                importControl( RecordInputStream& rStrm );

    /** Imports the binary data of an embedded OLE object from the fragment with the passed ID. */
    void                importEmbeddedOleData( StreamDataSequence& orEmbeddedData, const ::rtl::OUString& rRelId );
};

// ============================================================================

class BiffPivotTableContext;

class BiffWorksheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffWorksheetFragment(
                            const BiffWorkbookFragmentBase& rParent,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );
    virtual             ~BiffWorksheetFragment();

    /** Imports the entire worksheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment();

private:
    /** Imports the COLINFO record and sets column properties and formatting. */
    void                importColInfo();
    /** Imports the BIFF2 COLUMNDEFAULT record and sets column default formatting. */
    void                importColumnDefault();
    /** Imports the BIFF2 COLWIDTH record and sets column width. */
    void                importColWidth();
    /** Imports the DATAVALIDATION record containing cell ranges with data validation settings. */
    void                importDataValidation();
    /** Imports the DATAVALIDATIONS record containing global data validation settings. */
    void                importDataValidations();
    /** Imports the DEFCOLWIDTH record and sets default column width. */
    void                importDefColWidth();
    /** Imports the DEFROWHEIGHT record and sets default row height and properties. */
    void                importDefRowHeight();
    /** Imports the DIMENSION record containing the used area of the sheet. */
    void                importDimension();
    /** Imports the HYPERLINK record and sets a cell hyperlink. */
    void                importHyperlink();
    /** Imports the LABELRANGES record and sets the imported label ranges. */
    void                importLabelRanges();
    /** Imports the MEREDCELLS record and merges all cells in the document. */
    void                importMergedCells();
    /** Imports the HORPAGEBREAKS or VERPAGEBREAKS record and inserts page breaks. */
    void                importPageBreaks( bool bRowBreak );
    /** Imports a pivot table. */
    void                importPTDefinition();
    /** Imports the SCENARIOS record and the following scenarios. */
    void                importScenarios();
    /** Imports the STANDARDWIDTH record and sets standard column width. */
    void                importStandardWidth();

private:
    ::boost::shared_ptr< BiffPivotTableContext > mxPTContext;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

