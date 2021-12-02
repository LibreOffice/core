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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORT_HXX

#include <memory>
#include "wrtww8.hxx"

class RtfAttributeOutput;
class RtfExportFilter;
class RtfSdrExport;
using RtfColorTable = std::map<sal_uInt16, Color>;
class SwNode;
class SwTextNode;
class SwGrfNode;
class SwOLENode;

/// The class that does all the actual RTF export-related work.
class RtfExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    RtfExportFilter* m_pFilter;
    Writer* m_pWriter;

    /// Attribute output for document.
    std::unique_ptr<RtfAttributeOutput> m_pAttrOutput;

    /// Sections/headers/footers
    std::unique_ptr<MSWordSections> m_pSections;

    std::unique_ptr<RtfSdrExport> m_pSdrExport;
    bool m_bOutOutlineOnly;

public:
    /// Access to the attribute output class.
    AttributeOutputBase& AttrOutput() const override;

    /// Access to the sections/headers/footres.
    MSWordSections& Sections() const override;

    /// Access to the Rtf Sdr exporter.
    RtfSdrExport& SdrExporter() const;

    bool FieldsQuoted() const override { return true; }

    bool AddSectionBreaksForTOX() const override { return false; }

    bool PreferPageBreakBefore() const override { return true; }

    /// Guess the script (asian/western).
    bool CollapseScriptsforWordOk(sal_uInt16 nScript, sal_uInt16 nWhich) override;

    void AppendBookmarks(const SwTextNode& rNode, sal_Int32 nCurrentPos, sal_Int32 nLen,
                         const SwRedlineData* pSwRedlineData = nullptr) override;

    void AppendBookmark(const OUString& rName) override;

    void AppendAnnotationMarks(const SwWW8AttrIter& rAttrs, sal_Int32 nCurrentPos,
                               sal_Int32 nLen) override;

    //For i120928,add an interface to export graphic of bullet
    void ExportGrfBullet(const SwTextNode& rNd) override;

    void
        WriteCR(ww8::WW8TableNodeInfoInner::
                    Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/)
            override
    {
        /* no-op for rtf, most probably should not even be in MSWordExportBase */
    }
    void WriteChar(sal_Unicode c) override;

    /// Write the numbering table.
    void WriteNumbering() override;

    /// Write the revision table.
    void WriteRevTab();

    /// Output the actual headers and footers.
    void WriteHeadersFooters(sal_uInt8 nHeadFootFlags, const SwFrameFormat& rFormat,
                             const SwFrameFormat& rLeftHeaderFormat,
                             const SwFrameFormat& rLeftFooterFormat,
                             const SwFrameFormat& rFirstPageFormat, sal_uInt8 nBreakCode,
                             bool bEvenAndOddHeaders) override;

    /// Write the field
    void OutputField(const SwField* pField, ww::eField eFieldType, const OUString& rFieldCmd,
                     FieldFlags nMode = FieldFlags::All) override;

    /// Write the data of the form field
    void WriteFormData(const ::sw::mark::IFieldmark& rFieldmark) override;
    void WriteHyperlinkData(const ::sw::mark::IFieldmark& rFieldmark) override;

    void DoComboBox(const OUString& rName, const OUString& rHelp, const OUString& ToolTip,
                    const OUString& rSelected,
                    const css::uno::Sequence<OUString>& rListItems) override;

    void DoFormText(const SwInputField* pField) override;

    sal_uLong ReplaceCr(sal_uInt8 nChar) override;

    ExportFormat GetExportFormat() const override { return ExportFormat::RTF; }

protected:
    /// Format-dependent part of the actual export.
    ErrCode ExportDocument_Impl() override;

    void SectionBreaksAndFrames(const SwTextNode& /*rNode*/) override {}

    /// Get ready for a new section.
    void PrepareNewPageDesc(const SfxItemSet* pSet, const SwNode& rNd,
                            const SwFormatPageDesc* pNewPgDescFormat, const SwPageDesc* pNewPgDesc,
                            bool bExtraPageBreak = false) override;

    /// Return value indicates if an inherited outline numbering is suppressed.
    bool DisallowInheritingOutlineNumbering(const SwFormat& rFormat) override;

    /// Output SwTextNode is depending on outline export mode
    void OutputTextNode(SwTextNode& rNode) override;

    /// Output SwEndNode
    void OutputEndNode(const SwEndNode& rEndNode) override;

    /// Output SwGrfNode
    void OutputGrfNode(const SwGrfNode& rGrfNode) override;

    /// Output SwOLENode
    void OutputOLENode(const SwOLENode& rOLENode) override;

    void OutputLinkedOLE(const OUString& rLink) override;

    void AppendSection(const SwPageDesc* pPageDesc, const SwSectionFormat* pFormat,
                       sal_uLong nLnNum) override;

public:
    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    RtfExport(RtfExportFilter* pFilter, SwDoc& rDocument, std::shared_ptr<SwUnoCursor>& pCurrentPam,
              SwPaM& rOriginalPam, Writer* pWriter, bool bOutOutlineOnly = false);

    RtfExport(const RtfExport&) = delete;

    RtfExport& operator=(const RtfExport&) = delete;

    /// Destructor.
    ~RtfExport() override;

private:
    rtl_TextEncoding m_eDefaultEncoding;
    rtl_TextEncoding m_eCurrentEncoding;
    /// This is used by OutputFlyFrame_Impl() to control the written syntax
    bool m_bRTFFlySyntax;
    /// Index of the current SwTextNode, if any.
    SwNodeOffset m_nCurrentNodeIndex;

public:
    rtl_TextEncoding GetDefaultEncoding() const { return m_eDefaultEncoding; }
    void SetCurrentEncoding(rtl_TextEncoding eCurrentEncoding)
    {
        m_eCurrentEncoding = eCurrentEncoding;
    }
    rtl_TextEncoding GetCurrentEncoding() const { return m_eCurrentEncoding; }
    void SetRTFFlySyntax(bool bRTFFlySyntax) { m_bRTFFlySyntax = bRTFFlySyntax; }
    bool GetRTFFlySyntax() const { return m_bRTFFlySyntax; }
    SwNodeOffset GetCurrentNodeIndex() const { return m_nCurrentNodeIndex; }
    SvStream& Strm();
    /// From now on, let Strm() return a memory stream, not a real one.
    void setStream();
    /// Get the contents of the memory stream as a string.
    OString getStream();
    /// Return back to the real stream.
    void resetStream();
    SvStream& OutULong(sal_uLong nVal);
    SvStream& OutLong(tools::Long nVal);
    void OutUnicode(const char* pToken, const OUString& rContent, bool bUpr = false);
    void OutDateTime(const char* pStr, const css::util::DateTime& rDT);
    void OutPageDescription(const SwPageDesc& rPgDsc, bool bCheckForFirstPage);

    sal_uInt16 GetColor(const Color& rColor) const;
    void InsColor(const Color& rCol);
    void InsColorLine(const SvxBoxItem& rBox);
    void OutColorTable();
    sal_uInt16 GetRedline(const OUString& rAuthor);
    const OUString* GetRedline(sal_uInt16 nId);

    void InsStyle(sal_uInt16 nId, const OString& rStyle);
    OString* GetStyle(sal_uInt16 nId);

    const SfxItemSet* GetFirstPageItemSet() const { return m_pFirstPageItemSet; }

private:
    void WriteFonts();
    void WriteStyles();
    void WriteFootnoteSettings();
    void WriteMainText();
    void WriteInfo();
    /// Writes a single user property type.
    void WriteUserPropType(int nType);
    /// Writes a single user property value.
    void WriteUserPropValue(const OUString& rValue);
    /// Writes the userprops group: user defined document properties.
    void WriteUserProps();
    /// Writes the writer-specific \pgdsctbl group.
    void WritePageDescTable();
    /// This is necessary to have the numbering table ready before the main text is being processed.
    void BuildNumbering();
    void WriteHeaderFooter(const SfxPoolItem& rItem, bool bHeader);
    void WriteHeaderFooter(const SwFrameFormat& rFormat, bool bHeader, const char* pStr,
                           bool bTitlepg = false);

    RtfColorTable m_aColTable;
    std::map<sal_uInt16, OString> m_aStyTable;
    std::map<OUString, sal_uInt16> m_aRedlineTable;
    /// If set, then Strm() returns this stream, instead of m_pWriter's stream.
    std::unique_ptr<SvMemoryStream> m_pStream;
    /// Item set of the first page during export of a follow page format.
    const SfxItemSet* m_pFirstPageItemSet = nullptr;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
