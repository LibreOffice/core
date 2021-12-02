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

#include <sal/log.hxx>
#include <sax/fshelper.hxx>
#include <rtl/ustring.hxx>

#include <memory>
#include <ndole.hxx>
#include <unotools/securityoptions.hxx>

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

namespace com::sun::star {
    namespace frame { class XModel; }
    namespace drawing { class XShape; }
    namespace awt { class XControlModel; }
}

/// Data to be written in the document settings part of the document
struct DocxSettingsData
{
    DocxSettingsData();
    bool hasData() const; /// returns true if there are any non-default settings (i.e. something to write)
    bool evenAndOddHeaders;
    int defaultTabStop;
    bool revisionView;      // don't show tracked changes
    bool trackRevisions;    // Should 'Track Revisions' be set
};

/// The class that does all the actual DOCX export-related work.
class DocxExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    DocxExportFilter& m_rFilter;

    /// Fast serializer for the document output.
    ::sax_fastparser::FSHelperPtr m_pDocumentFS;

    /// Fast serializer to output the data.
    ::sax_fastparser::FSHelperPtr mpFS;

    /// Access to the DrawingML writer.
    std::unique_ptr<oox::drawingml::DrawingML> m_pDrawingML;

    /// Attribute output for document.
    std::unique_ptr<DocxAttributeOutput> m_pAttrOutput;

    /// Sections/headers/footers
    std::unique_ptr<MSWordSections> m_pSections;

    /// Header counter.
    sal_Int32 m_nHeaders;

    /// Footer counter.
    sal_Int32 m_nFooters;

    /// OLE objects counter.
    sal_Int32 m_nOLEObjects;

    /// ActiveX controls counter
    sal_Int32 m_nActiveXControls;

    ///Footer and Header counter in Section properties
    sal_Int32 m_nHeadersFootersInSection;

    /// Exporter of the VML shapes.
    std::unique_ptr<oox::vml::VMLExport> m_pVMLExport;

    /// Exporter of drawings.
    std::unique_ptr<DocxSdrExport> m_pSdrExport;

    /// If the result will be a .docm file or not.
    bool m_bDocm;

    /// Export is done into template (.dotx)
    bool const m_bTemplate;

    DocxSettingsData m_aSettings;

    /// Pointer to the Frame of a floating table it is nested in
    const ww8::Frame *m_pFloatingTableFrame = nullptr;

    /// Map authors to remove personal info
    std::unique_ptr<SvtSecurityMapPersonalInfo> m_pAuthorIDs;

public:

    DocxExportFilter& GetFilter() { return m_rFilter; };
    const DocxExportFilter& GetFilter() const { return m_rFilter; };

    const ww8::Frame* GetFloatingTableFrame() const { return m_pFloatingTableFrame; }

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const override;

    /// Access to the derived attribute output class.
    DocxAttributeOutput& DocxAttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const override;

    virtual bool FieldsQuoted() const override { return true; }

    virtual bool AddSectionBreaksForTOX() const override { return true; }

    virtual bool ignoreAttributeForStyleDefaults( sal_uInt16 nWhich ) const override;

    virtual bool PreferPageBreakBefore() const override { return false; }

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) override;

    virtual void AppendBookmarks( const SwTextNode& rNode, sal_Int32 nCurrentPos, sal_Int32 nLen, const SwRedlineData* pRedlineData = nullptr ) override;

    virtual void AppendBookmark( const OUString& rName ) override;

    virtual void AppendAnnotationMarks( const SwWW8AttrIter& rAttrs, sal_Int32 nCurrentPos, sal_Int32 nLen ) override;

    virtual void ExportGrfBullet(const SwTextNode&) override;

    /// Returns the relationd id
    OString AddRelation( const OUString& rType, std::u16string_view rTarget );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) override { /* FIXME no-op for docx, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode ) override { SAL_WARN("sw.ww8", "FIXME: WriteChar() has nothing to do for docx."); }

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering( const SwFormat &rFormat ) override;

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrameFormat& rFormat, const SwFrameFormat& rLeftHeaderFormat, const SwFrameFormat& rLeftFooterFormat, const SwFrameFormat& rFirstPageFormat,
            sal_uInt8 nBreakCode, bool bEvenAndOddHeaders ) override;

    /// Write the field
    virtual void OutputField( const SwField* pField, ww::eField eFieldType,
            const OUString& rFieldCmd, FieldFlags nMode = FieldFlags::All ) override;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) override;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) override;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    const css::uno::Sequence<OUString> &rListItems) override;

    virtual void DoFormText(const SwInputField * pField) override;

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) override;

    /// Returns the relationd id
    OString OutputChart( css::uno::Reference< css::frame::XModel > const & xModel, sal_Int32 nCount, ::sax_fastparser::FSHelperPtr const & m_pSerializer );
    OString WriteOLEObject(SwOLEObj& rObject, OUString & io_rProgID);
    std::pair<OString,OString> WriteActiveXObject(const uno::Reference<css::drawing::XShape>& rxShape,
                                                  const uno::Reference<awt::XControlModel>& rxControlModel);

    /// Writes the shape using drawingML syntax.
    void OutputDML( css::uno::Reference< css::drawing::XShape > const & xShape );

    sal_Int32 WriteOutliner(const OutlinerParaObject& rOutliner, sal_uInt8 nTyp, bool bNeedsLastParaId);

    virtual ExportFormat GetExportFormat() const override { return ExportFormat::DOCX; }

protected:
    /// Format-dependent part of the actual export.
    virtual ErrCode ExportDocument_Impl() override;

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& ) override;

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) override;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) override;

    virtual void OutputLinkedOLE( const OUString& ) override;

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum ) override;

    virtual void SectionBreaksAndFrames( const SwTextNode& /*rNode*/ ) override {}

    /// Get ready for a new section.
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFormatPageDesc* pNewPgDescFormat,
                                     const SwPageDesc* pNewPgDesc,
                                     bool bExtraPageBreak = false) override;

private:
    /// Setup pStyles and write styles.xml
    void InitStyles();

    /// Write footnotes.xml and endnotes.xml.
    void WriteFootnotesEndnotes();

    /// Write comments.xml
    void WritePostitFields();

    /// Write the numbering table.
    virtual void WriteNumbering() override;

    /// Write reference to a header/footer + the actual xml containing the text.
    void WriteHeaderFooter( const SwFormat* pFormat, bool bHeader, const char* pType );

    /// Write word/fontTable.xml.
    void WriteFonts();

    /// Write docProps/core.xml
    void WriteProperties();

    /// Write word/settings.xml
    void WriteSettings();

    /// Writes the <w:docVars> part of settings.xml
    void WriteDocVars(const sax_fastparser::FSHelperPtr& pFS);

    /// Write word/theme/theme1.xml
    void WriteTheme();

    void WriteGlossary();

    /// Write customXml/item[n].xml and customXml/itemProps[n].xml
    void WriteCustomXml();

    /// Write word/embeddings/Worksheet[n].xlsx
    void WriteEmbeddings();

    /// Writes word/vbaProject.bin.
    void WriteVBA();

    /// return true if Page Layout is set as Mirrored
    bool isMirroredMargin();

public:
    /// All xml namespaces to be used at the top of any text .xml file (main doc, headers, footers,...)
    rtl::Reference<sax_fastparser::FastAttributeList> MainXmlNamespaces();

    /// FIXME this is temporary, remotely reminding the method of the same
    /// name in WW8Export.
    void WriteMainText();

    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    DocxExport( DocxExportFilter& rFilter, SwDoc& rDocument,
            std::shared_ptr<SwUnoCursor> & pCurrentPam, SwPaM& rOriginalPam,
               bool bDocm, bool bTemplate);

    /// Destructor.
    virtual ~DocxExport() override;

    /// Reference to the VMLExport instance for the main document.
    oox::vml::VMLExport& VMLExporter() { return *m_pVMLExport; }

    /// Reference to the DocxSdrExport instance for the main document.
    DocxSdrExport& SdrExporter() { return *m_pSdrExport; }

    /// Set the document default tab stop.
    void setDefaultTabStop( int stop ) { m_aSettings.defaultTabStop = stop; }

    const ::sax_fastparser::FSHelperPtr& GetFS() const { return mpFS; }

    void SetFS(::sax_fastparser::FSHelperPtr const & mpFS);

    void SetFloatingTableFrame(const ww8::Frame* pF) { m_pFloatingTableFrame = pF; }

    // Get author id to remove personal info
    size_t GetInfoID( const OUString sPersonalInfo ) const { return m_pAuthorIDs->GetInfoID(sPersonalInfo); }

    // needed in docxsdrexport.cxx and docxattributeoutput.cxx
    sal_Int32 getWordCompatibilityModeFromGrabBag() const;

private:
    DocxExport( const DocxExport& ) = delete;

    DocxExport& operator=( const DocxExport& ) = delete;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
