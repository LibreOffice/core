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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_RTFATTRIBUTEOUTPUT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_RTFATTRIBUTEOUTPUT_HXX

#include <com/sun/star/drawing/FillStyle.hpp>

#include "attributeoutputbase.hxx"
#include "rtfexport.hxx"
#include "rtfstringbuffer.hxx"

#include <rtl/strbuf.hxx>
#include <svx/xenum.hxx>

#include <boost/optional.hpp>

class SwGrfNode;
class SwOLENode;
class SwFlyFrameFormat;

/// The class that has handlers for various resource types when exporting as RTF
class RtfAttributeOutput : public AttributeOutputBase
{
    friend class RtfStringBufferValue;
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState(bool bIsRTL, sal_uInt16 nScript) override;

    /// Start of the paragraph.
    virtual void StartParagraph(ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo) override;

    /// End of the paragraph.
    virtual void EndParagraph(ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner) override;

    /// Empty paragraph.
    virtual void EmptyParagraph() override;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwNode& rNode) override;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() override;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted) override;

    /// Start of the text run.
    virtual void StartRun(const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false) override;

    /// End of the text run.
    virtual void EndRun() override;

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties() override;

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties(const SwRedlineData* pRedlineData) override;

    /// Output text (inside a run).
    virtual void RunText(const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8) override;

    // Access to (anyway) private buffers, used by the sdr exporter
    OStringBuffer& RunText();
    OStringBuffer& Styles()
    {
        return m_aStyles;
    }

    /// Output text (without markup).
    virtual void RawText(const OUString& rText, rtl_TextEncoding eCharSet) override;

    /// Output ruby start.
    virtual void StartRuby(const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby) override;

    /// Output ruby end.
    virtual void EndRuby() override;

    /// Output URL start.
    virtual bool StartURL(const OUString& rUrl, const OUString& rTarget) override;

    /// Output URL end.
    virtual bool EndURL(bool isAtEndOfParagraph) override;

    virtual void FieldVanish(const OUString& rText, ww::eField eType) override;

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline(const SwRedlineData* pRedline) override;

    virtual void FormatDrop(const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner) override;

    /// Output style.
    virtual void ParagraphStyle(sal_uInt16 nStyle) override;

    virtual void TableInfoCell(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableInfoRow(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableDefinition(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableDefaultBorders(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableBackgrounds(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableRowRedline(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableCellRedline(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableHeight(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableCanSplit(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableBidi(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableVerticalCell(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableNodeInfoInner(ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner) override;
    virtual void TableOrientation(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableSpacing(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) override;
    virtual void TableRowEnd(sal_uInt32 nDepth) override;

    /// Start of the styles table.
    virtual void StartStyles() override;

    /// End of the styles table.
    virtual void EndStyles(sal_uInt16 nNumberOfStyles) override;

    /// Write default style.
    virtual void DefaultStyle(sal_uInt16 nStyle) override;

    /// Start of a style in the styles table.
    virtual void StartStyle(const OUString& rName, StyleType eType,
                            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
                            bool bAutoUpdate) override;

    /// End of a style in the styles table.
    virtual void EndStyle() override;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties(bool bParProp, sal_uInt16 nStyle) override;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties(bool bParProp) override;

    /// Numbering rule and Id.
    virtual void OutlineNumbering(sal_uInt8 nLvl) override;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore(bool bBreak) override;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak(sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL) override;

    /// Start of the section properties.
    virtual void StartSection() override;

    /// End of the section properties.
    virtual void EndSection() override;

    /// Protection of forms.
    virtual void SectionFormProtection(bool bProtected) override;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering(sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo) override;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() override;

    /// Description of the page borders.
    virtual void SectionPageBorders(const SwFrameFormat* pFormat, const SwFrameFormat* pFirstPageFormat) override;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi(bool bBiDi) override;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering(sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber) override;

    /// The type of breaking.
    virtual void SectionType(sal_uInt8 nBreakCode) override;

    /// Definition of a numbering instance.
    virtual void NumberingDefinition(sal_uInt16 nId, const SwNumRule& rRule) override;

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering(sal_uInt16 nId) override;

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering() override;

    /// All the numbering level information.
    virtual void NumberingLevel(sal_uInt8 nLevel,
                                sal_uInt16 nStart,
                                sal_uInt16 nNumberingType,
                                SvxAdjust eAdjust,
                                const sal_uInt8* pNumLvlPos,
                                sal_uInt8 nFollow,
                                const wwFont* pFont,
                                const SfxItemSet* pOutSet,
                                sal_Int16 nIndentAt,
                                sal_Int16 nFirstLineIndex,
                                sal_Int16 nListTabPos,
                                const OUString& rNumberingString,
                                const SvxBrushItem* pBrush = 0) override;//For i120928,to export graphic of bullet

    void WriteField_Impl(const SwField* pField, ww::eField eType, const OUString& rFieldCmd, sal_uInt8 nMode);
    void WriteBookmarks_Impl(std::vector< OUString >& rStarts, std::vector< OUString >& rEnds);
    void WriteAnnotationMarks_Impl(std::vector< OUString >& rStarts, std::vector< OUString >& rEnds);
    void WriteHeaderFooter_Impl(const SwFrameFormat& rFormat, bool bHeader, const sal_Char* pStr, bool bTitlepg);

protected:
    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl(const sw::Frame& rFormat, const Point& rNdTopLeft) override;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap(const SvxCaseMapItem& rCaseMap) override;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor(const SvxColorItem& rColor) override;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour(const SvxContourItem& rContour) override;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut(const SvxCrossedOutItem& rCrossedOut) override;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement(const SvxEscapementItem& rEscapement) override;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont(const SvxFontItem& rFont) override;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize(const SvxFontHeightItem& rFontSize) override;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning(const SvxKerningItem& rKerning) override;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage(const SvxLanguageItem& rLanguage) override;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture(const SvxPostureItem& rPosture) override;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow(const SvxShadowedItem& rShadow) override;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline(const SvxUnderlineItem& rUnderline) override;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight(const SvxWeightItem& rWeight) override;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern(const SvxAutoKernItem&) override;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText(const SvxBlinkItem& rBlink) override;

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground(const SvxBrushItem& rBrush) override;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK(const SvxFontItem& rFont) override;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK(const SvxFontHeightItem& rFontSize) override;

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK(const SvxLanguageItem& rLanguageItem) override;

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK(const SvxPostureItem& rPosture) override;

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK(const SvxWeightItem& rWeight) override;

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL(const SvxFontItem& rFont) override;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL(const SvxFontHeightItem& rFontSize) override;

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL(const SvxLanguageItem& rLanguageItem) override;

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL(const SvxPostureItem& rWeight) override;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL(const SvxWeightItem& rWeight) override;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL(const SfxPoolItem&) override;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint(const SfxPoolItem&) override;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate(const SvxCharRotateItem& rRotate) override;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark(const SvxEmphasisMarkItem& rEmphasisMark) override;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines(const SvxTwoLinesItem& rTwoLines) override;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth(const SvxCharScaleWidthItem& rScaleWidth) override;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief(const SvxCharReliefItem& rRelief) override;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden(const SvxCharHiddenItem& rHidden) override;

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder(const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow) override;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight(const SvxBrushItem&) override;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat(const SwFormatINetFormat&) override;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat(const SwFormatCharFormat&) override;

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl(const SwFormatFootnote&) override;

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl(short nSpace, short nMulti) override;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust(const SvxAdjustItem& rAdjust) override;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit(const SvxFormatSplitItem& rSplit) override;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows(const SvxWidowsItem& rWidows) override;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop(const SvxTabStopItem& rTabStop) override;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone(const SvxHyphenZoneItem&) override;

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl(const SwTextNode* pTextNd, sal_Int32 nLvl, sal_Int32 nNumId) override;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace(const SfxBoolItem&) override;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation(const SfxBoolItem&) override;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules(const SfxBoolItem&) override;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign(const SvxParaVertAlignItem& rAlign) override;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid(const SvxParaGridItem&) override;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize(const SwFormatFrmSize&) override;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin(const SvxPaperBinItem&) override;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace(const SvxLRSpaceItem& rLRSpace) override;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace(const SvxULSpaceItem& rULSpace) override;

    /// Sfx item RES_SURROUND
    virtual void FormatSurround(const SwFormatSurround&) override;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation(const SwFormatVertOrient&) override;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation(const SwFormatHoriOrient&) override;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor(const SwFormatAnchor&) override;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground(const SvxBrushItem&) override;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle(const XFillStyleItem&) override;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient(const XFillGradientItem&) override;

    /// Sfx item RES_BOX
    virtual void FormatBox(const SvxBoxItem&) override;

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl(sal_uInt16 nCols, const SwFormatCol& rCol, bool bEven, SwTwips nPageSize) override;

    /// Sfx item RES_KEEP
    virtual void FormatKeep(const SvxFormatKeepItem&) override;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid(const SwTextGridItem&) override;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering(const SwFormatLineNumber&) override;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection(const SvxFrameDirectionItem&) override;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag(const SfxGrabBagItem&) override;

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag(const SfxGrabBagItem&) override;

    /// Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel(const SfxUInt16Item&) override;

    /// Write the expanded field
    virtual void WriteExpand(const SwField* pField) override;

    virtual void RefField(const SwField& rField, const OUString& rRef) override;
    virtual void HiddenField(const SwField& rField) override;
    virtual void SetField(const SwField& rField, ww::eField eType, const OUString& rCmd) override;
    virtual void PostitField(const SwField* pField) override;
    virtual bool DropdownField(const SwField* pField) override;
    virtual bool PlaceholderField(const SwField* pField) override;

    /// Reference to the export, where to get the data from
    RtfExport& m_rExport;

private:

    /// Output graphic fly frames.
    void FlyFrameGraphic(const SwFlyFrameFormat* pFlyFrameFormat, const SwGrfNode* pGrfNode);
    void FlyFrameOLE(const SwFlyFrameFormat* pFlyFrameFormat, SwOLENode& rOLENode, const Size& rSize);
    void FlyFrameOLEReplacement(const SwFlyFrameFormat* pFlyFrameFormat, SwOLENode& rOLENode, const Size& rSize);
    /// Math export.
    bool FlyFrameOLEMath(const SwFlyFrameFormat* pFlyFrameFormat, SwOLENode& rOLENode, const Size& rSize);

    /*
     * Table methods.
     */
    void InitTableHelper(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner);
    void StartTable(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner);
    void StartTableRow(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner);
    void StartTableCell(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner);
    void TableCellProperties(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner);
    void EndTableCell();
    void EndTableRow();
    void EndTable();

    /// End cell, row, and even the entire table if necessary.
    void FinishTableRowCell(ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool bForceEmptyParagraph = false);

    void WriteTextFootnoteNumStr(const SwFormatFootnote& rFootnote);

    /*
     * Current style name and its ID.
     */
    OUString m_rStyleName;
    sal_uInt16 m_nStyleId;
    /*
     * Current list ID.
     */
    sal_uInt16 m_nListId;
    /*
     * This is needed because the call order is: run text, run properties, paragraph properties.
     * What we need is the opposite.
     */
    RtfStringBuffer m_aRun;
    RtfStringBuffer m_aRunText;
    /*
     * This is written after runs.
     */
    OStringBuffer m_aAfterRuns;
    /*
     * Same for colors and stylesheets: first we just want to output colors,
     * need to buffer the stylesheet table to output it after the color one.
     */
    OStringBuffer m_aStylesheet;
    /*
     * This one just holds the style commands in the current style.
     */
    OStringBuffer m_aStyles;
    /*
     * This is the same as m_aStyles but the contents of it is written last.
     */
    OStringBuffer m_aStylesEnd;

    /*
     * We just get a "end of strike" mark at the end of strike, store here what to finish: single or double strike.
     */
    bool m_bStrikeDouble;

    sal_Int32 m_nNextAnnotationMarkId;
    sal_Int32 m_nCurrentAnnotationMarkId;
    /// Maps annotation mark names to ID's.
    std::map<OString, sal_Int32> m_rOpenedAnnotationMarksIds;

    /*
     * The current table helper.
     */
    std::unique_ptr<SwWriteTable> m_pTableWrt;

    /*
     * Remember if we are in an open cell, or not.
     */
    bool m_bTableCellOpen;

    /*
     * Remember the current table depth.
     */
    sal_uInt32 m_nTableDepth;

    /*
     * Remember if we wrote a \cell or not.
     */
    bool m_bTableAfterCell;

    /*
     * For late output of row definitions.
     */
    OStringBuffer m_aRowDefs;

    /*
     * Is a column break needed after the next \par?
     */
    bool m_nColBreakNeeded;

    /*
     * If section breaks should be buffered to m_aSectionBreaks
     */
    bool m_bBufferSectionBreaks;
    OStringBuffer m_aSectionBreaks;

    /*
     * If section headers (and footers) should be buffered to
     * m_aSectionHeaders.
     */
    bool m_bBufferSectionHeaders;
    OStringBuffer m_aSectionHeaders;

    /*
     * Support for starting multiple tables at the same cell.
     * If the current table is the last started one.
     */
    bool m_bLastTable;
    /*
     * List of already started but not yet defined tables (need to be defined
     * after the nested tables).
     */
    std::vector< OString > m_aTables;
    /*
     * If cell info is already output.
     */
    bool m_bWroteCellInfo;

    /// If we ended a table row without starting a new one.
    bool m_bTableRowEnded;

    /// Number of cells from the table definition, by depth.
    std::map<sal_uInt32,sal_uInt32> m_aCells;

    /// If we're in a paragraph that has a single empty run only.
    bool m_bSingleEmptyRun;

    bool m_bInRun;

    /// Maps ID's to postit fields, used in atrfstart/end and atnref.
    std::map<sal_uInt16, const SwPostItField*> m_aPostitFields;

    /// When exporting fly frames, this holds the real size of the frame.
    const Size* m_pFlyFrameSize;

    std::vector< std::pair<OString, OString> > m_aFlyProperties;

    boost::optional<css::drawing::FillStyle> m_oFillStyle;

    /// If we're in the process of exporting a hyperlink, then its URL.
    OUString m_sURL;

public:
    explicit RtfAttributeOutput(RtfExport& rExport);

    virtual ~RtfAttributeOutput();

    /// Return the right export class.
    virtual MSWordExportBase& GetExport() override;

    OStringBuffer m_aTabStop;

    /// Access to the page style of the previous paragraph.
    const SwPageDesc* m_pPrevPageDesc;

    // These are used by wwFont::WriteRtf()
    /// Start the font.
    void StartFont(const OUString& rFamilyName) const;

    /// End the font.
    void EndFont() const;

    /// Alternate name for the font.
    void FontAlternateName(const OUString& rName) const;

    /// Font charset.
    void FontCharset(sal_uInt8 nCharSet) const;

    /// Font family.
    void FontFamilyType(FontFamily eFamily, const wwFont& rFont) const;

    /// Font pitch.
    void FontPitchType(FontPitch ePitch) const;

    /// Writes binary data as a hex dump.
    static OString WriteHex(const sal_uInt8* pData, sal_uInt32 nSize, SvStream* pStream = 0, sal_uInt32 nLimit = 64);

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) override;

    /// Handles just the {\shptxt ...} part of a shape export.
    void writeTextFrame(const sw::Frame& rFrame, bool bTextBox = false);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
