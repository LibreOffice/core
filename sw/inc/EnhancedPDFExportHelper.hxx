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

#ifndef _ENHANCEDPDFEXPORTHELPER_HXX
#define _ENHANCEDPDFEXPORTHELPER_HXX

#include <vcl/pdfextoutdevdata.hxx>
#include <i18npool/lang.h>
#include <swrect.hxx>
#include <swtypes.hxx>

#include <map>
#include <vector>
#include <set>

namespace vcl
{
    class PDFExtOutDevData;
}
class OutputDevice;
class SwFrm;
class SwLinePortion;
class SwTxtPainter;
class SwEditShell;
namespace rtl
{
    class OUString;
}
class MultiSelection;
class SwTxtNode;
class SwNumRule;
class SwTable;
class SwNumberTreeNode;
class String;
class SvxLanguageItem;


/*
 * Mapping of OOo elements to tagged pdf elements:
 *
 * OOo element                              tagged pdf element
 * -----------                              ------------------
 *
 * Grouping elements:
 *
 * SwRootFrm                                Document
 *                                          Part
 *                                          Art
 * SwSection                                Sect
 * SwFtnContFrm and SwFlyFrm                Div
 * SwFmt "Quotations"                       BlockQuote
 * SwFmt "Caption"                          Caption
 * SwSection (TOC)                          TOC
 * SwTxtNode in TOC                         TOCI
 * SwSection (Index)                        Index
 *
 * Block-Level Structure Elements:
 *
 * SwTxtNode                                P
 * SwFmt "Heading"                          H
 * SwTxtNode with Outline                   H1 - H6
 * SwTxtNode with NumRule                   L, LI, LBody
 * SwTable                                  Table
 * SwRowFrm                                 TR
 * SwCellFrm in Headline row or
 * SwFtm "Table Heading"                    TH
 * SwCellFrm                                TD
 *
 * Inline-Level Structure Elements:
 *
 * SwTxtPortion                             Span
 * SwFmt "Quotation"                        Quote
 * SwFtnFrm                                 Note
 *                                          Form
 *                                          Reference
 * SwFldPortion (AuthorityField)            BibEntry
 * SwFmt "Source Text"                      Code
 * SwFtnPortion, SwFldPortion (RefField)    Link
 *
 * Illustration elements:
 *
 * SwFlyFrm with SwNoTxtFrm                 Figure
 * SwFlyFrm with Math OLE Object            Formula
 *
 */

struct Num_Info
{
    const SwFrm& mrFrm;
    Num_Info( const SwFrm& rFrm ) : mrFrm( rFrm ) {};
};

struct Frm_Info
{
    const SwFrm& mrFrm;
    Frm_Info( const SwFrm& rFrm ) : mrFrm( rFrm ) {};
};

struct Por_Info
{
    const SwLinePortion& mrPor;
    const SwTxtPainter& mrTxtPainter;
    Por_Info( const SwLinePortion& rPor, const SwTxtPainter& rTxtPainer )
            : mrPor( rPor ), mrTxtPainter( rTxtPainer ) {};
};

struct lt_TableColumn
{
    bool operator()( long nVal1, long nVal2 ) const
    {
        return nVal1 + ( MINLAY - 1 ) < nVal2;
    }
};

/*************************************************************************
 *                class SwTaggedPDFHelper
 * Analyses a given frame during painting and generates the appropriate
 * structure elements.
 *************************************************************************/

class SwTaggedPDFHelper
{
    private:

    // This will be incremented for each BeginTag() call.
    // It denotes the number of tags to close during EndStructureElements();
    sal_uInt8 nEndStructureElement;

    //  If an already existing tag is reopened for follows of flow frames,
    // this value stores the tag id which has to be restored.
    sal_Int32 nRestoreCurrentTag;

    vcl::PDFExtOutDevData* mpPDFExtOutDevData;

    const Num_Info* mpNumInfo;
    const Frm_Info* mpFrmInfo;
    const Por_Info* mpPorInfo;

    void BeginTag( vcl::PDFWriter::StructElement aTagRole, const String& rTagName );
    void EndTag();

    void SetAttributes( vcl::PDFWriter::StructElement eType );

    // These functions are called by the c'tor, d'tor
    void BeginNumberedListStructureElements();
    void BeginBlockStructureElements();
    void BeginInlineStructureElements();
    void EndStructureElements();

    bool CheckReopenTag();
    bool CheckRestoreTag() const;

    public:

    // pFrmInfo != 0 => BeginBlockStructureElement
    // pPorInfo != 0 => BeginInlineStructureElement
    // pFrmInfo, pPorInfo = 0 => BeginNonStructureElement
    SwTaggedPDFHelper( const Num_Info* pNumInfo, const Frm_Info* pFrmInfo, const Por_Info* pPorInfo,
                       OutputDevice& rOut );
    ~SwTaggedPDFHelper();

    static bool IsExportTaggedPDF( const OutputDevice& rOut );
};

/*************************************************************************
 *                class SwEnhancedPDFExportHelper
 * Analyses the document structure and export Notes, Hyperlinks, References,
 * and Outline. Link ids created during pdf export are stored in
 * aReferenceIdMap and aHyperlinkIdMap, in order to use them during
 * tagged pdf output. Therefore the SwEnhancedPDFExportHelper is used
 * before painting. Unfortunately links from the EditEngine into the
 * Writer document require to be exported after they have been painted.
 * Therefore SwEnhancedPDFExportHelper also has to be used after the
 * painting process, the parameter bEditEngineOnly indicated that only
 * the bookmarks from the EditEngine have to be processed.
 *************************************************************************/

typedef std::set< long, lt_TableColumn > TableColumnsMapEntry;
typedef std::pair< SwRect, sal_Int32 > IdMapEntry;
typedef std::vector< IdMapEntry > LinkIdMap;
typedef std::map< const SwTable*, TableColumnsMapEntry > TableColumnsMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListIdMap;
typedef std::map< const SwNumberTreeNode*, sal_Int32 > NumListBodyIdMap;
typedef std::map< const void*, sal_Int32 > FrmTagIdMap;

class SwEnhancedPDFExportHelper
{
    private:

    SwEditShell& mrSh;
    OutputDevice& mrOut;

    MultiSelection* pPageRange;

    bool mbSkipEmptyPages;
    bool mbEditEngineOnly;

    static TableColumnsMap aTableColumnsMap;
    static LinkIdMap aLinkIdMap;
    static NumListIdMap aNumListIdMap;
    static NumListBodyIdMap aNumListBodyIdMap;
    static FrmTagIdMap aFrmTagIdMap;

    static LanguageType eLanguageDefault;

    void EnhancedPDFExport();
    sal_Int32 CalcOutputPageNum( const SwRect& rRect ) const;

    void MakeHeaderFooterLinks( vcl::PDFExtOutDevData& rPDFExtOutDevData,
                                const SwTxtNode& rTNd, const SwRect& rLinkRect,
                                sal_Int32 nDestId, const String& rURL, bool bIntern ) const;

    public:

    SwEnhancedPDFExportHelper( SwEditShell& rSh,
                               OutputDevice& rOut,
                               const rtl::OUString& rPageRange,
                               bool bSkipEmptyPages,
                               bool bEditEngineOnly );

    ~SwEnhancedPDFExportHelper();

    static TableColumnsMap& GetTableColumnsMap() {return aTableColumnsMap; }
    static LinkIdMap& GetLinkIdMap() { return aLinkIdMap; }
    static NumListIdMap& GetNumListIdMap() {return aNumListIdMap; }
    static NumListBodyIdMap& GetNumListBodyIdMap() {return aNumListBodyIdMap; }
    static FrmTagIdMap& GetFrmTagIdMap() { return aFrmTagIdMap; }

    static LanguageType GetDefaultLanguage() {return eLanguageDefault; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
