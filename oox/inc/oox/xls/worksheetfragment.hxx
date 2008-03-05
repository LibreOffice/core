/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: worksheetfragment.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:10:37 $
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

#ifndef OOX_XLS_WORKSHEETFRAGMENT_HXX
#define OOX_XLS_WORKSHEETFRAGMENT_HXX

#include "oox/xls/bifffragmenthandler.hxx"
#include "oox/xls/excelhandlers.hxx"

namespace oox {
namespace xls {

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

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

    virtual ContextWrapper onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm );
    virtual void        onStartRecord( RecordInputStream& rStrm );

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
    /** Imports the dataValidation element containing data validation settings. */
    void                importDataValidation( const AttributeList& rAttribs );
    /** Imports the hyperlink element containing a hyperlink for a cell range. */
    void                importHyperlink( const AttributeList& rAttribs );
    /** Imports individual break that is either within row or column break context. */
    void                importBrk( const AttributeList& rAttribs );
    /** Imports the the relation identifier for the DrawingML part. */
    void                importDrawing( const AttributeList& rAttribs );

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
    /** Imports the DATAVALIDATION record containing data validation settings. */
    void                importDataValidation( RecordInputStream& rStrm );
    /** Imports the BRK record for an individual row or column page break. */
    void                importBrk( RecordInputStream& rStrm );
    /** Imports the DRAWING record containing the relation identifier for the DrawingML part. */
    void                importDrawing( RecordInputStream& rStrm );

private:
    ::std::auto_ptr< OoxValidationData > mxValData;
};

// ============================================================================

class BiffWorksheetFragment : public BiffWorksheetFragmentBase
{
public:
    explicit            BiffWorksheetFragment(
                            const WorkbookHelper& rHelper,
                            ISegmentProgressBarRef xProgressBar,
                            WorksheetType eSheetType,
                            sal_Int32 nSheet );

    /** Imports the entire worksheet fragment, returns true, if EOF record has been reached. */
    virtual bool        importFragment( BiffInputStream& rStrm );

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
    /** Imports the STANDARDWIDTH record and sets standard column width. */
    void                importStandardWidth( BiffInputStream& rStrm );
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

