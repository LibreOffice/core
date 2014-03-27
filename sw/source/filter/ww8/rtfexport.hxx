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

#include "wrtww8.hxx"

class RtfAttributeOutput;
class RtfExportFilter;
class RtfSdrExport;
typedef std::map<sal_uInt16,Color> RtfColorTbl;
typedef std::map<sal_uInt16,OString> RtfStyleTbl;
typedef std::map<OUString,sal_uInt16> RtfRedlineTbl;
class SwNode;
class SwTxtNode;
class SwGrfNode;
class SwOLENode;

/// The class that does all the actual RTF export-related work.
class RtfExport : public MSWordExportBase
{
    /// Pointer to the filter that owns us.
    RtfExportFilter *m_pFilter;
    Writer* m_pWriter;

    /// Attribute output for document.
    RtfAttributeOutput *m_pAttrOutput;

    /// Sections/headers/footers
    MSWordSections *m_pSections;

    RtfSdrExport *m_pSdrExport;
    bool m_bOutOutlineOnly;

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const SAL_OVERRIDE;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const SAL_OVERRIDE;

    /// Access to the Rtf Sdr exporter.
    virtual RtfSdrExport& SdrExporter() const;

    /// Determines if the format is expected to support unicode.
    virtual bool SupportsUnicode() const SAL_OVERRIDE { return true; }

    virtual bool SupportsOneColumnBreak() const SAL_OVERRIDE { return false; }

    virtual bool FieldsQuoted() const SAL_OVERRIDE { return true; }

    virtual bool AddSectionBreaksForTOX() const SAL_OVERRIDE { return false; }

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) SAL_OVERRIDE;

    virtual void AppendBookmarks( const SwTxtNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;

    virtual void AppendBookmark( const OUString& rName, bool bSkip = false ) SAL_OVERRIDE;

    virtual void AppendAnnotationMarks( const SwTxtNode& rNode, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;

    //For i120928,add an interface to export graphic of bullet
    virtual void ExportGrfBullet(const SwTxtNode& rNd) SAL_OVERRIDE;

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t /*pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t()*/ ) SAL_OVERRIDE { /* no-op for rtf, most probably should not even be in MSWordExportBase */ }
    virtual void WriteChar( sal_Unicode ) SAL_OVERRIDE;

    /// Write the numbering table.
    virtual void WriteNumbering() SAL_OVERRIDE;

    /// Write the revision table.
    virtual void WriteRevTab();

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt, sal_uInt8 nBreakCode ) SAL_OVERRIDE;

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const OUString& rFldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL ) SAL_OVERRIDE;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    com::sun::star::uno::Sequence<OUString> &rListItems) SAL_OVERRIDE;

    virtual void DoFormText(const SwInputField * pFld) SAL_OVERRIDE;

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) SAL_OVERRIDE;

protected:
    /// Format-dependent part of the actual export.
    virtual void ExportDocument_Impl() SAL_OVERRIDE;

    virtual void SectionBreaksAndFrames( const SwTxtNode& /*rNode*/ ) SAL_OVERRIDE {}

    /// Get ready for a new section.
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 ) SAL_OVERRIDE;

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering(const SwFmt &rFmt) SAL_OVERRIDE;

    /// Output SwTxtNode is depending on outline export mode
    virtual void OutputTextNode( const SwTxtNode& ) SAL_OVERRIDE;

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) SAL_OVERRIDE;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) SAL_OVERRIDE;

    virtual void OutputLinkedOLE(const OUString&) SAL_OVERRIDE;

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum ) SAL_OVERRIDE;

public:
    /// Pass the pDocument, pCurrentPam and pOriginalPam to the base class.
    RtfExport( RtfExportFilter *pFilter, SwDoc *pDocument,
            SwPaM *pCurrentPam, SwPaM *pOriginalPam, Writer* pWriter,
            bool bOutOutlineOnly = false );

    /// Destructor.
    virtual ~RtfExport();

    rtl_TextEncoding eDefaultEncoding;
    rtl_TextEncoding eCurrentEncoding;
    /// This is used by OutputFlyFrame_Impl() to control the written syntax
    bool bRTFFlySyntax;
    /// Index of the current SwTxtNode, if any.
    sal_uLong m_nCurrentNodeIndex;

    SvStream& Strm();
    SvStream& OutULong( sal_uLong nVal );
    SvStream& OutLong( long nVal );
    void OutUnicode(const sal_Char *pToken, const OUString &rContent, bool bUpr = false);
    void OutDateTime(const sal_Char* pStr, const css::util::DateTime& rDT );
    void OutPageDescription( const SwPageDesc& rPgDsc, bool bWriteReset, bool bCheckForFirstPage );

    sal_uInt16 GetColor( const Color& rColor ) const;
    void InsColor( const Color& rCol );
    void InsColorLine( const SvxBoxItem& rBox );
    void OutColorTable();
    sal_uInt16 GetRedline( const OUString& rAuthor );
    const OUString* GetRedline( sal_uInt16 nId );

    void InsStyle( sal_uInt16 nId, const OString& rStyle );
    OString* GetStyle( sal_uInt16 nId );

private:
    /// No copying.
    RtfExport( const RtfExport& );

    /// No copying.
    RtfExport& operator=( const RtfExport& );

    void WriteFonts();
    void WriteStyles();
    void WriteFootnoteSettings();
    void WriteMainText();
    void WriteInfo();
    /// Writes the writer-specific \pgdsctbl group.
    void WritePageDescTable();
    /// This is necessary to have the numbering table ready before the main text is being processed.
    void BuildNumbering();
    void WriteHeaderFooter(const SfxPoolItem& rItem, bool bHeader);
    void WriteHeaderFooter(const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr, bool bTitlepg = false);

    RtfColorTbl m_aColTbl;
    RtfStyleTbl m_aStyTbl;
    RtfRedlineTbl m_aRedlineTbl;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
