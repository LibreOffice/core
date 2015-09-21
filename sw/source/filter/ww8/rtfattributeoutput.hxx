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
    virtual void RTLAndCJKState(bool bIsRTL, sal_uInt16 nScript) SAL_OVERRIDE;

    /// Start of the paragraph.
    virtual void StartParagraph(ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo) SAL_OVERRIDE;

    /// End of the paragraph.
    virtual void EndParagraph(ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner) SAL_OVERRIDE;

    /// Empty paragraph.
    virtual void EmptyParagraph() SAL_OVERRIDE;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwNode& rNode) SAL_OVERRIDE;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() SAL_OVERRIDE;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted) SAL_OVERRIDE;

    /// Start of the text run.
    virtual void StartRun(const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false) SAL_OVERRIDE;

    /// End of the text run.
    virtual void EndRun() SAL_OVERRIDE;

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties() SAL_OVERRIDE;

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties(const SwRedlineData* pRedlineData) SAL_OVERRIDE;

    /// Output text (inside a run).
    virtual void RunText(const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8) SAL_OVERRIDE;

    // Access to (anyway) private buffers, used by the sdr exporter
    OStringBuffer& RunText();
    OStringBuffer& Styles()
    {
        return m_aStyles;
    }

    /// Output text (without markup).
    virtual void RawText(const OUString& rText, bool bForceUnicode, rtl_TextEncoding eCharSet) SAL_OVERRIDE;

    /// Output ruby start.
    virtual void StartRuby(const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby) SAL_OVERRIDE;

    /// Output ruby end.
    virtual void EndRuby() SAL_OVERRIDE;

    /// Output URL start.
    virtual bool StartURL(const OUString& rUrl, const OUString& rTarget) SAL_OVERRIDE;

    /// Output URL end.
    virtual bool EndURL(bool isAtEndOfParagraph) SAL_OVERRIDE;

    virtual void FieldVanish(const OUString& rText, ww::eField eType) SAL_OVERRIDE;

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline(const SwRedlineData* pRedline) SAL_OVERRIDE;

    virtual void FormatDrop(const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner) SAL_OVERRIDE;

    /// Output style.
    virtual void ParagraphStyle(sal_uInt16 nStyle) SAL_OVERRIDE;

    virtual void TableInfoCell(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableInfoRow(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableDefinition(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableDefaultBorders(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableBackgrounds(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableRowRedline(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableCellRedline(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableHeight(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableCanSplit(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableBidi(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableVerticalCell(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableNodeInfo(ww8::WW8TableNodeInfo::Pointer_t pNodeInfo) SAL_OVERRIDE;
    virtual void TableNodeInfoInner(ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner) SAL_OVERRIDE;
    virtual void TableOrientation(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableSpacing(ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner) SAL_OVERRIDE;
    virtual void TableRowEnd(sal_uInt32 nDepth) SAL_OVERRIDE;

    /// Start of the styles table.
    virtual void StartStyles() SAL_OVERRIDE;

    /// End of the styles table.
    virtual void EndStyles(sal_uInt16 nNumberOfStyles) SAL_OVERRIDE;

    /// Write default style.
    virtual void DefaultStyle(sal_uInt16 nStyle) SAL_OVERRIDE;

    /// Start of a style in the styles table.
    virtual void StartStyle(const OUString& rName, StyleType eType,
                            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
                            bool bAutoUpdate) SAL_OVERRIDE;

    /// End of a style in the styles table.
    virtual void EndStyle() SAL_OVERRIDE;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties(bool bParProp, sal_uInt16 nStyle) SAL_OVERRIDE;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties(bool bParProp) SAL_OVERRIDE;

    /// Numbering rule and Id.
    virtual void OutlineNumbering(sal_uInt8 nLvl, const SwNumFormat& rNFormat, const SwFormat& rFormat) SAL_OVERRIDE;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore(bool bBreak) SAL_OVERRIDE;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak(sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL) SAL_OVERRIDE;

    /// Start of the section properties.
    virtual void StartSection() SAL_OVERRIDE;

    /// End of the section properties.
    virtual void EndSection() SAL_OVERRIDE;

    /// Protection of forms.
    virtual void SectionFormProtection(bool bProtected) SAL_OVERRIDE;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering(sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo) SAL_OVERRIDE;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() SAL_OVERRIDE;

    /// Description of the page borders.
    virtual void SectionPageBorders(const SwFrameFormat* pFormat, const SwFrameFormat* pFirstPageFormat) SAL_OVERRIDE;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi(bool bBiDi) SAL_OVERRIDE;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering(sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber) SAL_OVERRIDE;

    /// The type of breaking.
    virtual void SectionType(sal_uInt8 nBreakCode) SAL_OVERRIDE;

    /// Definition of a numbering instance.
    virtual void NumberingDefinition(sal_uInt16 nId, const SwNumRule& rRule) SAL_OVERRIDE;

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering(sal_uInt16 nId) SAL_OVERRIDE;

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering() SAL_OVERRIDE;

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
                                const SvxBrushItem* pBrush = 0) SAL_OVERRIDE;//For i120928,to export graphic of bullet

    void WriteField_Impl(const SwField* pField, ww::eField eType, const OUString& rFieldCmd, sal_uInt8 nMode);
    void WriteBookmarks_Impl(std::vector< OUString >& rStarts, std::vector< OUString >& rEnds);
    void WriteAnnotationMarks_Impl(std::vector< OUString >& rStarts, std::vector< OUString >& rEnds);
    void WriteHeaderFooter_Impl(const SwFrameFormat& rFormat, bool bHeader, const sal_Char* pStr, bool bTitlepg);

protected:
    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl(const sw::Frame& rFormat, const Point& rNdTopLeft) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap(const SvxCaseMapItem& rCaseMap) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor(const SvxColorItem& rColor) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour(const SvxContourItem& rContour) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut(const SvxCrossedOutItem& rCrossedOut) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement(const SvxEscapementItem& rEscapement) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont(const SvxFontItem& rFont) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize(const SvxFontHeightItem& rFontSize) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning(const SvxKerningItem& rKerning) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage(const SvxLanguageItem& rLanguage) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture(const SvxPostureItem& rPosture) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow(const SvxShadowedItem& rShadow) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline(const SvxUnderlineItem& rUnderline) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight(const SvxWeightItem& rWeight) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern(const SvxAutoKernItem&) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText(const SvxBlinkItem& rBlink) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground(const SvxBrushItem& rBrush) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK(const SvxFontItem& rFont) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK(const SvxFontHeightItem& rFontSize) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK(const SvxLanguageItem& rLanguageItem) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK(const SvxPostureItem& rPosture) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK(const SvxWeightItem& rWeight) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL(const SvxFontItem& rFont) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL(const SvxFontHeightItem& rFontSize) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL(const SvxLanguageItem& rLanguageItem) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL(const SvxPostureItem& rWeight) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL(const SvxWeightItem& rWeight) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL(const SfxPoolItem&) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint(const SfxPoolItem&) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate(const SvxCharRotateItem& rRotate) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark(const SvxEmphasisMarkItem& rEmphasisMark) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines(const SvxTwoLinesItem& rTwoLines) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth(const SvxCharScaleWidthItem& rScaleWidth) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief(const SvxCharReliefItem& rRelief) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden(const SvxCharHiddenItem& rHidden) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder(const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight(const SvxBrushItem&) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat(const SwFormatINetFormat&) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat(const SwFormatCharFormat&) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl(const SwFormatFootnote&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl(short nSpace, short nMulti) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust(const SvxAdjustItem& rAdjust) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit(const SvxFormatSplitItem& rSplit) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows(const SvxWidowsItem& rWidows) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop(const SvxTabStopItem& rTabStop) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone(const SvxHyphenZoneItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl(const SwTextNode* pTextNd, sal_Int32 nLvl, sal_Int32 nNumId) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace(const SfxBoolItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation(const SfxBoolItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules(const SfxBoolItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign(const SvxParaVertAlignItem& rAlign) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid(const SvxParaGridItem&) SAL_OVERRIDE;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize(const SwFormatFrmSize&) SAL_OVERRIDE;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin(const SvxPaperBinItem&) SAL_OVERRIDE;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace(const SvxLRSpaceItem& rLRSpace) SAL_OVERRIDE;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace(const SvxULSpaceItem& rULSpace) SAL_OVERRIDE;

    /// Sfx item RES_SURROUND
    virtual void FormatSurround(const SwFormatSurround&) SAL_OVERRIDE;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation(const SwFormatVertOrient&) SAL_OVERRIDE;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation(const SwFormatHoriOrient&) SAL_OVERRIDE;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor(const SwFormatAnchor&) SAL_OVERRIDE;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground(const SvxBrushItem&) SAL_OVERRIDE;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle(const XFillStyleItem&) SAL_OVERRIDE;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient(const XFillGradientItem&) SAL_OVERRIDE;

    /// Sfx item RES_BOX
    virtual void FormatBox(const SvxBoxItem&) SAL_OVERRIDE;

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl(sal_uInt16 nCols, const SwFormatCol& rCol, bool bEven, SwTwips nPageSize) SAL_OVERRIDE;

    /// Sfx item RES_KEEP
    virtual void FormatKeep(const SvxFormatKeepItem&) SAL_OVERRIDE;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid(const SwTextGridItem&) SAL_OVERRIDE;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering(const SwFormatLineNumber&) SAL_OVERRIDE;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection(const SvxFrameDirectionItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag(const SfxGrabBagItem&) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag(const SfxGrabBagItem&) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel(const SfxUInt16Item&) SAL_OVERRIDE;

    /// Write the expanded field
    virtual void WriteExpand(const SwField* pField) SAL_OVERRIDE;

    virtual void RefField(const SwField& rField, const OUString& rRef) SAL_OVERRIDE;
    virtual void HiddenField(const SwField& rField) SAL_OVERRIDE;
    virtual void SetField(const SwField& rField, ww::eField eType, const OUString& rCmd) SAL_OVERRIDE;
    virtual void PostitField(const SwField* pField) SAL_OVERRIDE;
    virtual bool DropdownField(const SwField* pField) SAL_OVERRIDE;
    virtual bool PlaceholderField(const SwField* pField) SAL_OVERRIDE;

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
    std::map<OString, sal_uInt16> m_rOpenedAnnotationMarksIds;

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
    RtfAttributeOutput(RtfExport& rExport);

    virtual ~RtfAttributeOutput();

    /// Return the right export class.
    virtual MSWordExportBase& GetExport() SAL_OVERRIDE;

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

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) SAL_OVERRIDE;

    /// Handles just the {\shptxt ...} part of a shape export.
    void writeTextFrame(const sw::Frame& rFrame, bool bTextBox = false);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
