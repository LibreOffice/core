/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DOCXEXPORT_HXX_
#define _DOCXEXPORT_HXX_

#include "docxattributeoutput.hxx"
#include "wrtww8.hxx"

#include <sax/fshelper.hxx>
#include <rtl/ustring.hxx>

#include <cstdio>
#include <vector>

class DocxExportFilter;
class SwNode;
class SwEndNode;
class SwTableNode;
class SwTxtNode;
class SwGrfNode;
class SwOLENode;
class SwSectionNode;
class SwNumRuleTbl;

namespace oox {
    namespace drawingml { class DrawingML; }
    namespace vml { class VMLExport; }
}

/// The class that does all the actual DOCX export-related work.
class DocxExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    DocxExportFilter *m_pFilter;

    /// Fast serializer for the document output.
    ::sax_fastparser::FSHelperPtr m_pDocumentFS;

    /// Access to the DrawingML writer.
    oox::drawingml::DrawingML *m_pDrawingML;

    /// Attribute output for document.
    DocxAttributeOutput *m_pAttrOutput;

    /// Sections/headers/footers
    MSWordSections *m_pSections;

    /// Header counter.
    sal_Int32 m_nHeaders;

    /// Footer counter.
    sal_Int32 m_nFooters;

    /// Exporter of the VML shapes.
    oox::vml::VMLExport *m_pVMLExport;

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const;

    /// Hack, unfortunately necessary at some places for now.
    /// FIXME remove it when possible.
    virtual bool HackIsWW8OrHigher() const { return true; }

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich );

    virtual void AppendBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen );

    virtual void AppendBookmark( const rtl::OUString& rName, bool bSkip = false );

    /// Returns the relationd id
    rtl::OString AddRelation( const rtl::OUString& rType, const rtl::OUString& rTarget, const rtl::OUString& rMode );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) { /* FIXME no-op for docx, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode ) { /* FIXME */ fprintf( stderr, "HACK! WriteChar() has nothing to do for docx.\n" ); }

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering( const SwFmt &rFmt );

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt );

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const String& rFldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL );

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark );

    virtual void DoComboBox(const rtl::OUString &rName,
                    const rtl::OUString &rHelp,
                    const rtl::OUString &ToolTip,
                    const rtl::OUString &rSelected,
                    com::sun::star::uno::Sequence<rtl::OUString> &rListItems);

    virtual void DoFormText(const SwInputField * pFld);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar );

protected:
    /// Format-dependant part of the actual export.
    virtual void ExportDocument_Impl();

    /// Output page/section breaks
    virtual void OutputPageSectionBreaks( const SwTxtNode& );

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& );

    /// Output SwTableNode
    virtual void OutputTableNode( const SwTableNode& );

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& );

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& );


    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum );

    virtual void SectionBreaksAndFrames( const SwTxtNode& rNode ) {}

    /// Get ready for a new section.
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 );

private:
    /// Setup pStyles and write styles.xml
    void InitStyles();

    /// Write footnotes.xml and endnotes.xml.
    void WriteFootnotesEndnotes();

    /// Write the numbering table.
    virtual void WriteNumbering();

    /// Write reference to a header/foorter + the actual xml containing the text.
    void WriteHeaderFooter( const SwFmt& rFmt, bool bHeader, const char* pType );

    /// Write word/fontTable.xml.
    void WriteFonts();

    /// Write docProps/core.xml
    void WriteProperties();

public:
    /// FIXME this is temporary, remotely reminding the method of the same
    /// name in WW8Export.
    void WriteMainText();

    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    DocxExport( DocxExportFilter *pFilter, SwDoc *pDocument,
            SwPaM *pCurrentPam, SwPaM *pOriginalPam );

    /// Destructor.
    virtual ~DocxExport();

    /// Reference to the VMLExport instance for the main document.
    oox::vml::VMLExport& VMLExporter();

private:
    /// No copying.
    DocxExport( const DocxExport& );

    /// No copying.
    DocxExport& operator=( const DocxExport& );
};

#endif // _DOCXEXPORT_HXX_
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
