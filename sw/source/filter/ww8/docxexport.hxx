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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORT_HXX

#include "wrtww8.hxx"

#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <rtl/ustring.hxx>

#include <cstdio>
#include <vector>
#include <boost/optional.hpp>
#include <ndole.hxx>

class DocxAttributeOutput;
class DocxExportFilter;
class SwNode;
class SwEndNode;
class SwTableNode;
class SwTextNode;
class SwGrfNode;
class SwOLENode;
class DocxSdrExport;

namespace oox {
    namespace drawingml { class DrawingML; }
    namespace vml { class VMLExport; }
}

namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace drawing { class XShape; }
} } }

/// Data to be written in the document settings part of the document
struct DocxSettingsData
{
    DocxSettingsData();
    bool hasData() const; /// returns true if there are any non-default settings (i.e. something to write)
    bool evenAndOddHeaders;
    int defaultTabStop;
    bool trackRevisions;    // Should 'Track Revisions' be set
};

/// The class that does all the actual DOCX export-related work.
class DocxExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    DocxExportFilter *m_pFilter;

    /// Fast serializer for the document output.
    ::sax_fastparser::FSHelperPtr m_pDocumentFS;

    /// Fast serializer to output the data.
    ::sax_fastparser::FSHelperPtr mpFS;

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

    /// OLE objects counter.
    sal_Int32 m_nOLEObjects;

    ///Footer and Header counter in Section properties
    sal_Int32 m_nHeadersFootersInSection;

    /// Exporter of the VML shapes.
    oox::vml::VMLExport *m_pVMLExport;

    /// Exporter of drawings.
    DocxSdrExport* m_pSdrExport;

    DocxSettingsData m_aSettings;

public:

    DocxExportFilter& GetFilter() { return *m_pFilter; };
    const DocxExportFilter& GetFilter() const { return *m_pFilter; };

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const SAL_OVERRIDE;

    /// Access to the derived attribute output class.
    DocxAttributeOutput& DocxAttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const SAL_OVERRIDE;

    virtual bool SupportsOneColumnBreak() const SAL_OVERRIDE { return true; }

    virtual bool FieldsQuoted() const SAL_OVERRIDE { return true; }

    virtual bool AddSectionBreaksForTOX() const SAL_OVERRIDE { return true; }

    virtual bool ignoreAttributeForStyleDefaults( sal_uInt16 nWhich ) const SAL_OVERRIDE;

    virtual bool PreferPageBreakBefore() const SAL_OVERRIDE { return false; }

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) SAL_OVERRIDE;

    virtual void AppendBookmarks( const SwTextNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;

    virtual void AppendBookmark( const OUString& rName, bool bSkip = false ) SAL_OVERRIDE;

    virtual void AppendAnnotationMarks( const SwTextNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;

    virtual void ExportGrfBullet(const SwTextNode&) SAL_OVERRIDE;

    /// Returns the relationd id
    OString AddRelation( const OUString& rType, const OUString& rTarget );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) SAL_OVERRIDE { /* FIXME no-op for docx, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode ) SAL_OVERRIDE { /* FIXME */ fprintf( stderr, "HACK! WriteChar() has nothing to do for docx.\n" ); }

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering( const SwFormat &rFormat ) SAL_OVERRIDE;

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrameFormat& rFormat, const SwFrameFormat& rLeftFormat, const SwFrameFormat& rFirstPageFormat, sal_uInt8 nBreakCode ) SAL_OVERRIDE;

    /// Write the field
    virtual void OutputField( const SwField* pField, ww::eField eFieldType,
            const OUString& rFieldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL ) SAL_OVERRIDE;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    com::sun::star::uno::Sequence<OUString> &rListItems) SAL_OVERRIDE;

    virtual void DoFormText(const SwInputField * pField) SAL_OVERRIDE;

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) SAL_OVERRIDE;

    /// Returns the relationd id
    OString OutputChart( com::sun::star::uno::Reference< com::sun::star::frame::XModel >& xModel, sal_Int32 nCount, ::sax_fastparser::FSHelperPtr m_pSerializer );
    OString WriteOLEObject( SwOLEObj& rObject, const OUString& sMediaType, const OUString& sRelationType, const OUString& sFileExtension );
    static bool lcl_CopyStream( css::uno::Reference< css::io::XInputStream> xIn, css::uno::Reference< css::io::XOutputStream > xOut );

    /// Writes the shape using drawingML syntax.
    void OutputDML( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xShape );

    void WriteOutliner(const OutlinerParaObject& rOutliner, sal_uInt8 nTyp);

protected:
    /// Format-dependent part of the actual export.
    virtual void ExportDocument_Impl() SAL_OVERRIDE;

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& ) SAL_OVERRIDE;

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) SAL_OVERRIDE;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) SAL_OVERRIDE;

    virtual void OutputLinkedOLE( const OUString& ) SAL_OVERRIDE;

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum ) SAL_OVERRIDE;

    virtual void SectionBreaksAndFrames( const SwTextNode& /*rNode*/ ) SAL_OVERRIDE {}

    /// Get ready for a new section.
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFormatPageDesc* pNewPgDescFormat = 0,
                                     const SwPageDesc* pNewPgDesc = 0 ) SAL_OVERRIDE;

private:
    /// Setup pStyles and write styles.xml
    void InitStyles();

    /// Write footnotes.xml and endnotes.xml.
    void WriteFootnotesEndnotes();

    /// Write comments.xml
    void WritePostitFields();

    /// Write the numbering table.
    virtual void WriteNumbering() SAL_OVERRIDE;

    /// Write reference to a header/footer + the actual xml containing the text.
    void WriteHeaderFooter( const SwFormat& rFormat, bool bHeader, const char* pType );

    /// Write word/fontTable.xml.
    void WriteFonts();

    /// Write docProps/core.xml
    void WriteProperties();

    /// Write word/settings.xml
    void WriteSettings();

    /// Write word/theme/theme1.xml
    void WriteTheme();

    void WriteGlossary();

    /// Write customXml/item[n].xml and customXml/itemProps[n].xml
    void WriteCustomXml();

    /// Write word/activeX/activeX[n].xml
    void WriteActiveX();

    /// Write word/embeddings/Worksheet[n].xlsx
    void WriteEmbeddings();

    /// Get background color of the document, if there is one.
    boost::optional<SvxBrushItem> getBackground();

    /// return true if Page Layout is set as Mirrored
    bool isMirroredMargin();

public:
    /// All xml namespaces to be used at the top of any text .xml file (main doc, headers, footers,...)
    static sax_fastparser::XFastAttributeListRef MainXmlNamespaces();

    /// FIXME this is temporary, remotely reminding the method of the same
    /// name in WW8Export.
    void WriteMainText();

    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    DocxExport( DocxExportFilter *pFilter, SwDoc *pDocument,
            SwPaM *pCurrentPam, SwPaM *pOriginalPam );

    /// Destructor.
    virtual ~DocxExport();

    /// Reference to the VMLExport instance for the main document.
    oox::vml::VMLExport& VMLExporter() { return *m_pVMLExport; }

    /// Reference to the DocxSdrExport instance for the main document.
    DocxSdrExport& SdrExporter() { return *m_pSdrExport; }

    /// Set the document default tab stop.
    void setDefaultTabStop( int stop ) { m_aSettings.defaultTabStop = stop; }

    ::sax_fastparser::FSHelperPtr GetFS() { return mpFS; }

    void SetFS(::sax_fastparser::FSHelperPtr mpFS);

private:
    DocxExport( const DocxExport& ) = delete;

    DocxExport& operator=( const DocxExport& ) = delete;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
