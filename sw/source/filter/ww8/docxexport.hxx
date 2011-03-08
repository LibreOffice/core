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

#ifndef _DOCXEXPORT_HXX_
#define _DOCXEXPORT_HXX_

#include "wrtww8.hxx"

#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <rtl/ustring.hxx>

#include <cstdio>
#include <vector>

class DocxAttributeOutput;
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

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
} } }

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

    DocxExportFilter& GetFilter() { return *m_pFilter; };
    const DocxExportFilter& GetFilter() const { return *m_pFilter; };

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const;

    /// Determines if the format is expected to support unicode.
    virtual bool SupportsUnicode() const { return true; }

    virtual bool ignoreAttributeForStyles( USHORT nWhich ) const;

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( USHORT nScript, USHORT nWhich );

    virtual void AppendBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen );

    virtual void AppendBookmark( const rtl::OUString& rName, bool bSkip = false );

    /// Returns the relationd id
    rtl::OString AddRelation( const rtl::OUString& rType, const rtl::OUString& rTarget, const rtl::OUString& rMode );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) { /* FIXME no-op for docx, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode ) { /* FIXME */ fprintf( stderr, "HACK! WriteChar() has nothing to do for docx.\n" ); }

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering( const SwFmt &rFmt );

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( BYTE nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, BYTE nBreakCode );

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const String& rFldCmd, BYTE nMode = nsFieldFlags::WRITEFIELD_ALL );

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark );
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark );

    virtual void DoComboBox(const rtl::OUString &rName,
                    const rtl::OUString &rHelp,
                    const rtl::OUString &ToolTip,
                    const rtl::OUString &rSelected,
                    com::sun::star::uno::Sequence<rtl::OUString> &rListItems);

    virtual void DoFormText(const SwInputField * pFld);

    virtual ULONG ReplaceCr( BYTE nChar );

    /// Returns the relationd id
    rtl::OString OutputChart( com::sun::star::uno::Reference< com::sun::star::frame::XModel >& xModel, sal_Int32 nCount );

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

    virtual void OutputLinkedOLE( const rtl::OUString& );

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, ULONG nLnNum );

    virtual void SectionBreaksAndFrames( const SwTxtNode& /*rNode*/ ) {}

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

    /// All xml namespaces to be used at the top of any text .xml file (main doc, headers, footers,...)
    sax_fastparser::XFastAttributeListRef MainXmlNamespaces( sax_fastparser::FSHelperPtr serializer );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
