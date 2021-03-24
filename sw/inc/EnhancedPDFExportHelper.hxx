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

#ifndef INCLUDED_SW_INC_ENHANCEDPDFEXPORTHELPER_HXX
#define INCLUDED_SW_INC_ENHANCEDPDFEXPORTHELPER_HXX

#include <i18nlangtag/lang.h>
#include <vcl/pdfwriter.hxx>
#include "swrect.hxx"
#include "swtypes.hxx"

#include <map>
#include <memory>
#include <vector>
#include <set>

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
class SwTextNode;
class SwTable;
class SwNumberTreeNode;

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
    const SwFrame& mrFrame;
    Num_Info( const SwFrame& rFrame ) : mrFrame( rFrame ) {};
};

struct Frame_Info
{
    const SwFrame& mrFrame;
    Frame_Info( const SwFrame& rFrame ) : mrFrame( rFrame ) {};
};

struct Por_Info
{
    const SwLinePortion& mrPor;
    const SwTextPainter& mrTextPainter;
    Por_Info( const SwLinePortion& rPor, const SwTextPainter& rTextPainer )
            : mrPor( rPor ), mrTextPainter( rTextPainer ) {};
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

    void BeginTag( vcl::PDFWriter::StructElement aTagRole, const OUString& rTagName );
    void EndTag();

    void SetAttributes( vcl::PDFWriter::StructElement eType );

    // These functions are called by the c'tor, d'tor
    void BeginNumberedListStructureElements();
    void BeginBlockStructureElements();
    void BeginInlineStructureElements();
    void EndStructureElements();

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
};

/*
 * Analyses the document structure and export Notes, Hyperlinks, References,
 * and Outline. Link ids created during pdf export are stored in
 * aReferenceIdMap and aHyperlinkIdMap, in order to use them during
 * tagged pdf output. Therefore the SwEnhancedPDFExportHelper is used
 * before painting. Unfortunately links from the EditEngine into the
 * Writer document require to be exported after they have been painted.
 * Therefore SwEnhancedPDFExportHelper also has to be used after the
 * painting process, the parameter bEditEngineOnly indicated that only
 * the bookmarks from the EditEngine have to be processed.
 */
typedef std::set< tools::Long, lt_TableColumn > TableColumnsMapEntry;
typedef std::pair< SwRect, sal_Int32 > IdMapEntry;
typedef std::vector< IdMapEntry > LinkIdMap;
typedef std::map< const SwTable*, TableColumnsMapEntry > TableColumnsMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListIdMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListBodyIdMap;
typedef std::map< const void*, sal_Int32 > FrameTagIdMap;

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

    static TableColumnsMap s_aTableColumnsMap;
    static LinkIdMap s_aLinkIdMap;
    static NumListIdMap s_aNumListIdMap;
    static NumListBodyIdMap s_aNumListBodyIdMap;
    static FrameTagIdMap s_aFrameTagIdMap;

    static LanguageType s_eLanguageDefault;

    void EnhancedPDFExport();

    /// Exports bibliography entry links.
    void ExportAuthorityEntryLinks();

    sal_Int32 CalcOutputPageNum( const SwRect& rRect ) const;
    std::vector< sal_Int32 > CalcOutputPageNums( const SwRect& rRect ) const;

    void MakeHeaderFooterLinks( vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                const SwTextNode& rTNd, const SwRect& rLinkRect,
                                sal_Int32 nDestId, const OUString& rURL, bool bIntern ) const;

    public:

    SwEnhancedPDFExportHelper( SwEditShell& rSh,
                               OutputDevice& rOut,
                               const OUString& rPageRange,
                               bool bSkipEmptyPages,
                               bool bEditEngineOnly,
                               const SwPrintData& rPrintData );

    ~SwEnhancedPDFExportHelper();

    static TableColumnsMap& GetTableColumnsMap() {return s_aTableColumnsMap; }
    static LinkIdMap& GetLinkIdMap() { return s_aLinkIdMap; }
    static NumListIdMap& GetNumListIdMap() {return s_aNumListIdMap; }
    static NumListBodyIdMap& GetNumListBodyIdMap() {return s_aNumListBodyIdMap; }
    static FrameTagIdMap& GetFrameTagIdMap() { return s_aFrameTagIdMap; }

    static LanguageType GetDefaultLanguage() {return s_eLanguageDefault; }

    //scale and position rRectangle if we're scaling due to notes in margins.
    tools::Rectangle SwRectToPDFRect(const SwPageFrame* pCurrPage,
        const tools::Rectangle& rRectangle) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
