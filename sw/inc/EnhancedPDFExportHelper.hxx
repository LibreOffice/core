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

#pragma once

#include <i18nlangtag/lang.h>
#include <vcl/pdfwriter.hxx>
#include "swrect.hxx"
#include "swtypes.hxx"

#include <memory>
#include <vector>

namespace vcl
{
    class PDFExtOutDevData;
}
class OutputDevice;
class SwFrame;
class SwLinePortion;
class SwPageFrame;
class SwPrintData;
class SwTextPainter;
class SwEditShell;
class StringRangeEnumerator;
class SwTextAttr;
class SwTextNode;
class SwTable;
class SwNumberTreeNode;
class SwTextPaintInfo;
class SwTextFrame;

/*
 * Mapping of OOo elements to tagged pdf elements:
 *
 * OOo element                              tagged pdf element
 * -----------                              ------------------
 *
 * Grouping elements:
 *
 * SwRootFrame                                Document
 *                                          Part
 *                                          Art
 * SwSection                                Sect
 * SwFootnoteContFrame and SwFlyFrame                Div
 * SwFormat "Quotations"                       BlockQuote
 * SwFormat "Caption"                          Caption
 * SwSection (TOC)                          TOC
 * SwTextNode in TOC                         TOCI
 * SwSection (Index)                        Index
 *
 * Block-Level Structure Elements:
 *
 * SwTextNode                                P
 * SwFormat "Heading"                          H
 * SwTextNode with Outline                   H1 - H6
 * SwTextNode with NumRule                   L, LI, LBody
 * SwTable                                  Table
 * SwRowFrame                                 TR
 * SwCellFrame in Headline row or
 * SwFtm "Table Heading"                    TH
 * SwCellFrame                                TD
 *
 * Inline-Level Structure Elements:
 *
 * SwTextPortion                             Span
 * SwFormat "Quotation"                        Quote
 * SwFootnoteFrame                                 Note
 *                                          Form
 *                                          Reference
 * SwFieldPortion (AuthorityField)            BibEntry
 * SwFormat "Source Text"                      Code
 * SwFootnotePortion, SwFieldPortion (RefField)    Link
 *
 * Illustration elements:
 *
 * SwFlyFrame with SwNoTextFrame                 Figure
 * SwFlyFrame with Math OLE Object            Formula
 *
 */

struct Num_Info
{
    const SwTextFrame& mrFrame;
    Num_Info( const SwTextFrame& rFrame ) : mrFrame( rFrame ) {};
};

struct Frame_Info
{
    const SwFrame& mrFrame;
    bool const m_isLink;

    Frame_Info(const SwFrame& rFrame, bool const isLink)
        : mrFrame(rFrame), m_isLink(isLink) {}
};

struct Por_Info
{
    const SwLinePortion& mrPor;
    const SwTextPainter& mrTextPainter;
    /** this can be used to generate multiple different SE for the same portion:
      FootnoteNum: 0-> Link 1-> Lbl
      Double: 0-> Warichu 1-> WP 2-> WT
      Ruby: 0-> Ruby 1-> RT 2-> RB
    */
    int const m_Mode;

    Por_Info(const SwLinePortion& rPor, const SwTextPainter& rTextPainer, int const nMode)
        : mrPor(rPor), mrTextPainter(rTextPainer), m_Mode(nMode) {};
};

struct lt_TableColumn
{
    bool operator()( tools::Long nVal1, tools::Long nVal2 ) const
    {
        return nVal1 + ( MINLAY - 1 ) < nVal2;
    }
};

// Analyses a given frame during painting and generates the appropriate
// structure elements.
class SwTaggedPDFHelper
{
    private:

    // This will be incremented for each BeginTag() call.
    // It denotes the number of tags to close during EndStructureElements();
    sal_uInt8 m_nEndStructureElement;

    //  If an already existing tag is reopened for follows of flow frames,
    // this value stores the tag id which has to be restored.
    sal_Int32 m_nRestoreCurrentTag;

    vcl::PDFExtOutDevData* mpPDFExtOutDevData;

    const Num_Info* mpNumInfo;
    const Frame_Info* mpFrameInfo;
    const Por_Info* mpPorInfo;

    void OpenTagImpl(void const* pKey);
    sal_Int32 BeginTagImpl(void const* pKey,vcl::pdf::StructElement aTagRole, const OUString& rTagName);
    void BeginTag(vcl::pdf::StructElement aTagRole, const OUString& rTagName);
    void EndTag();

    void SetAttributes(vcl::pdf::StructElement eType);

    // These functions are called by the c'tor, d'tor
    void BeginNumberedListStructureElements();
    void BeginBlockStructureElements();
    void BeginInlineStructureElements();
    void EndStructureElements();

    void EndCurrentAll();
    void EndCurrentSpan();
    void CreateCurrentSpan(SwTextPaintInfo const& rInf, OUString const& rStyleName);
    bool CheckContinueSpan(SwTextPaintInfo const& rInf, std::u16string_view rStyleName, SwTextAttr const* pInetFormatAttr);

    bool CheckReopenTag();
    void CheckRestoreTag() const;

    public:

    // pFrameInfo != 0 => BeginBlockStructureElement
    // pPorInfo != 0 => BeginInlineStructureElement
    // pFrameInfo, pPorInfo = 0 => BeginNonStructureElement
    SwTaggedPDFHelper( const Num_Info* pNumInfo, const Frame_Info* pFrameInfo, const Por_Info* pPorInfo,
                       OutputDevice const & rOut );
    ~SwTaggedPDFHelper();

    static bool IsExportTaggedPDF( const OutputDevice& rOut );
    static void EndCurrentLink(OutputDevice const&);
};

/*
 * Analyses the document structure and export Notes, Hyperlinks, References,
 * and Outline. Link ids created during pdf export are stored in
 * SwEnhancedPDFState, in order to use them during
 * tagged pdf output. Therefore the SwEnhancedPDFExportHelper is used
 * before painting. Unfortunately links from the EditEngine into the
 * Writer document require to be exported after they have been painted.
 * Therefore SwEnhancedPDFExportHelper also has to be used after the
 * painting process, the parameter bEditEngineOnly indicated that only
 * the bookmarks from the EditEngine have to be processed.
 */
class SwEnhancedPDFExportHelper
{
    private:

    SwEditShell& mrSh;
    OutputDevice& mrOut;

    std::unique_ptr<StringRangeEnumerator> mpRangeEnum;
    /** The problem is that numbers in StringRangeEnumerator aren't accordant
     * to real page numbers if mbSkipEmptyPages is true, because in this case
     * empty pages are excluded from a page range and numbers in
     * StringRangeEnumerator are shifted.
     *
     * maPageNumberMap[real_page_number] is either a corresponding page number
     * in a page range without empty pages, or -1 if this page is empty. */
    std::vector< sal_Int32 > maPageNumberMap;

    bool mbSkipEmptyPages;
    bool mbEditEngineOnly;

    const SwPrintData& mrPrintData;

    void EnhancedPDFExport(LanguageType const eLanguageDefault);

    /// Exports bibliography entry links.
    void ExportAuthorityEntryLinks();

    sal_Int32 CalcOutputPageNum( const SwRect& rRect ) const;
    std::vector< sal_Int32 > CalcOutputPageNums( const SwRect& rRect ) const;

    void MakeHeaderFooterLinks( vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                const SwTextNode& rTNd, const SwRect& rLinkRect,
                                sal_Int32 nDestId, const OUString& rURL,
                                bool bIntern, OUString const& rContent) const;

    public:

    SwEnhancedPDFExportHelper( SwEditShell& rSh,
                               OutputDevice& rOut,
                               const OUString& rPageRange,
                               bool bSkipEmptyPages,
                               bool bEditEngineOnly,
                               const SwPrintData& rPrintData );

    ~SwEnhancedPDFExportHelper();

    //scale and position rRectangle if we're scaling due to notes in margins.
    tools::Rectangle SwRectToPDFRect(const SwPageFrame* pCurrPage,
        const tools::Rectangle& rRectangle) const;

    static tools::Rectangle MapSwRectToPDFRect(const SwPageFrame* pCurrPage,
                                               const tools::Rectangle& rRectangle);
    static double GetSwRectToPDFRectScale();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
