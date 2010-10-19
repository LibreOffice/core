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

#ifndef OOX_XLS_WORKSHEETFRAGMENT_HXX
#define OOX_XLS_WORKSHEETFRAGMENT_HXX

#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

// ============================================================================

class DataValidationsContext : public WorksheetContextBase
{
public:
    explicit            DataValidationsContext( WorksheetFragmentBase& rFragment );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );
    virtual void        onEndElement();

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

class WorksheetFragment : public WorksheetFragmentBase
{
public:
    explicit            WorksheetFragment(
                            const WorkbookHelper& rHelper,
                            const ::rtl::OUString& rFragmentPath,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );

protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onCharacters( const ::rtl::OUString& rChars );

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );

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

class BiffWorksheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffWorksheetFragment(
                            const BiffWorkbookFragmentBase& rParent,
                            const ISegmentProgressBarRef& rxProgressBar,
                            WorksheetType eSheetType,
                            sal_Int16 nSheet );
    virtual             ~BiffWorksheetFragment();

    /** Imports the entire worksheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment();

private:
    /** Imports the COLINFO record and sets column properties and formatting. */
    void                importColInfo( BiffInputStream& rStrm );
    /** Imports the BIFF2 COLUMNDEFAULT record and sets column default formatting. */
    void                importColumnDefault( BiffInputStream& rStrm );
    /** Imports the BIFF2 COLWIDTH record and sets column width. */
    void                importColWidth( BiffInputStream& rStrm );
    /** Imports the DATAVALIDATION record containing cell ranges with data validation settings. */
    void                importDataValidation( BiffInputStream& rStrm );
    /** Imports the DATAVALIDATIONS record containing global data validation settings. */
    void                importDataValidations( BiffInputStream& rStrm );
    /** Imports the DEFCOLWIDTH record and sets default column width. */
    void                importDefColWidth( BiffInputStream& rStrm );
    /** Imports the DEFROWHEIGHT record and sets default row height and properties. */
    void                importDefRowHeight( BiffInputStream& rStrm );
    /** Imports the DIMENSION record containing the used area of the sheet. */
    void                importDimension( BiffInputStream& rStrm );
    /** Imports the HYPERLINK record and sets a cell hyperlink. */
    void                importHyperlink( BiffInputStream& rStrm );
    /** Imports the LABELRANGES record and sets the imported label ranges. */
    void                importLabelRanges( BiffInputStream& rStrm );
    /** Imports the MEREDCELLS record and merges all cells in the document. */
    void                importMergedCells( BiffInputStream& rStrm );
    /** Imports the HORPAGEBREAKS or VERPAGEBREAKS record and inserts page breaks. */
    void                importPageBreaks( BiffInputStream& rStrm, bool bRowBreak );
    /** Imports a pivot table. */
    void                importPTDefinition( BiffInputStream& rStrm );
    /** Imports the QUERYTABLE and following records and inserts a web query. */
    void                importQueryTable( BiffInputStream& rStrm );
    /** Imports the SCENARIOS record and the following scenarios. */
    void                importScenarios( BiffInputStream& rStrm );
    /** Imports the SHAREDFEATHEAD record. */
    void                importSharedFeatHead( BiffInputStream& rStrm );
    /** Imports the STANDARDWIDTH record and sets standard column width. */
    void                importStandardWidth( BiffInputStream& rStrm );

private:
    ::boost::shared_ptr< BiffWorksheetContextBase > mxContext;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
