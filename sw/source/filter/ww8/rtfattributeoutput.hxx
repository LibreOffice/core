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

#include "attributeoutputbase.hxx"
#include "rtfexport.hxx"
#include "rtfstringbuffer.hxx"

#include <rtl/strbuf.hxx>
#include <svx/xenum.hxx>

#include <boost/optional.hpp>

class SwGrfNode;
class SwOLENode;
class SwFlyFrmFmt;

/// The class that has handlers for various resource types when exporting as RTF
class RtfAttributeOutput : public AttributeOutputBase
{
    friend class RtfStringBufferValue;
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript );

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo );

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner );

    /// Empty paragraph.
    virtual void EmptyParagraph();

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwTxtNode& rNode);

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties();

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties( const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted );

    /// Start of the text run.
    virtual void StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false );

    /// End of the text run.
    virtual void EndRun();

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties();

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData );

    virtual void WritePostitFieldStart() SAL_OVERRIDE;

    virtual void WritePostitFieldEnd() SAL_OVERRIDE;

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 );

    // Access to (anyway) private buffers, used by the sdr exporter
    OStringBuffer& RunText();
    OStringBuffer& Styles();

    /// Output text (without markup).
    virtual void RawText( const OUString& rText, bool bForceUnicode, rtl_TextEncoding eCharSet );

    /// Output ruby start.
    virtual void StartRuby( const SwTxtNode& rNode, sal_Int32 nPos, const SwFmtRuby& rRuby );

    /// Output ruby end.
    virtual void EndRuby();

    /// Output URL start.
    virtual bool StartURL( const OUString& rUrl, const OUString& rTarget );

    /// Output URL end.
    virtual bool EndURL();

    virtual void FieldVanish( const OUString& rTxt, ww::eField eType );

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline( const SwRedlineData* pRedline );

    virtual void FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner );

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle );

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo );
    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner );
    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableRowEnd( sal_uInt32 nDepth );

    /// Start of the styles table.
    virtual void StartStyles();

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles );

    /// Write default style.
    virtual void DefaultStyle( sal_uInt16 nStyle );

    /// Start of a style in the styles table.
    virtual void StartStyle( const OUString& rName, StyleType eType,
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
            bool bAutoUpdate );

    /// End of a style in the styles table.
    virtual void EndStyle();

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle );

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp );

    /// Numbering rule and Id.
    virtual void OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt &rNFmt, const SwFmt &rFmt );

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak );

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL );

    /// Start of the section properties.
    virtual void StartSection();

    /// End of the section properties.
    virtual void EndSection();

    /// Protection of forms.
    virtual void SectionFormProtection( bool bProtected );

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo );

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage();

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* pFirstPageFmt );

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi );

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering( sal_uInt16 nNumType, ::boost::optional<sal_uInt16> oPageRestartNumber );

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode );

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule );

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering( sal_uInt16 nId ) SAL_OVERRIDE;

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering() SAL_OVERRIDE;

    /// All the numbering level information.
    virtual void NumberingLevel( sal_uInt8 nLevel,
        sal_uInt16 nStart,
        sal_uInt16 nNumberingType,
        SvxAdjust eAdjust,
        const sal_uInt8 *pNumLvlPos,
        sal_uInt8 nFollow,
        const wwFont *pFont,
        const SfxItemSet *pOutSet,
        sal_Int16 nIndentAt,
        sal_Int16 nFirstLineIndex,
        sal_Int16 nListTabPos,
        const OUString &rNumberingString,
        const SvxBrushItem* pBrush = 0);//For i120928,to export graphic of bullet

    void WriteField_Impl( const SwField* pFld, ww::eField eType, const OUString& rFldCmd, sal_uInt8 nMode );
    void WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void WriteHeaderFooter_Impl( const SwFrmFmt& rFmt, bool bHeader, const sal_Char* pStr, bool bTitlepg );

protected:
    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl( const sw::Frame& rFmt, const Point& rNdTopLeft );

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap( const SvxCaseMapItem& rCaseMap );

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor( const SvxColorItem& rColor);

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour( const SvxContourItem& rContour );

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut( const SvxCrossedOutItem& rCrossedOut );

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement( const SvxEscapementItem& rEscapement );

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont( const SvxFontItem& rFont );

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize( const SvxFontHeightItem& rFontSize );

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning( const SvxKerningItem& rKerning );

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage( const SvxLanguageItem& rLanguage );

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture( const SvxPostureItem& rPosture );

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow( const SvxShadowedItem& rShadow );

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline( const SvxUnderlineItem& rUnderline );

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight( const SvxWeightItem& rWeight );

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern( const SvxAutoKernItem& );

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText( const SvxBlinkItem& rBlink );

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground( const SvxBrushItem& rBrush );

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK( const SvxFontItem& rFont );

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK( const SvxFontHeightItem& rFontSize );

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& rLanguageItem );

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& rPosture );

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& rWeight );

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& rFont );

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& rFontSize );

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& rLanguageItem );

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL( const SvxPostureItem& rWeight );

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL( const SvxWeightItem& rWeight );

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate( const SvxCharRotateItem& rRotate );

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark );

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines( const SvxTwoLinesItem& rTwoLines );

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth );

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief( const SvxCharReliefItem& rRelief);

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden( const SvxCharHiddenItem& rHidden );

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder( const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow );

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight( const SvxBrushItem& ){};

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFmtINetFmt& );

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFmtCharFmt& );

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFmtFtn& );

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti );

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& rAdjust );

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFmtSplitItem& rSplit );

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& rWidows );

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& rTabStop );

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& );

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl( const SwTxtNode *pTxtNd, sal_Int32 nLvl, sal_Int32 nNumId );

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& );

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation( const SfxBoolItem& );

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules( const SfxBoolItem& );

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& rAlign );

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& );

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFmtFrmSize& );

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& );

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& rLRSpace );

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& rULSpace );

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFmtSurround& );

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFmtVertOrient& );

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFmtHoriOrient& );

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFmtAnchor& );

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& );

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& );

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& );

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& );

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol & rCol, bool bEven, SwTwips nPageSize );

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFmtKeepItem& );

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& );

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFmtLineNumber& );

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& );

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& );

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag( const SfxGrabBagItem& );

    /// Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel( const SfxUInt16Item& );

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pFld );

    virtual void RefField( const SwField& rFld, const OUString& rRef );
    virtual void HiddenField( const SwField& rFld );
    virtual void SetField( const SwField& rFld, ww::eField eType, const OUString& rCmd );
    virtual void PostitField( const SwField* pFld );
    virtual bool DropdownField( const SwField* pFld );
    virtual bool PlaceholderField( const SwField* pFld );

    /// Reference to the export, where to get the data from
    RtfExport &m_rExport;

private:

    /// Output graphic fly frames.
    void FlyFrameGraphic( const SwFlyFrmFmt* pFlyFrmFmt, const SwGrfNode* pGrfNode );
    void FlyFrameOLE( const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize );
    void FlyFrameOLEReplacement(const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize);
    /// Math export.
    bool FlyFrameOLEMath(const SwFlyFrmFmt* pFlyFrmFmt, SwOLENode& rOLENode, const Size& rSize);

    /*
     * Table methods.
     */
    void InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTable( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void EndTableCell( );
    void EndTableRow( );
    void EndTable();

    /// End cell, row, and even the entire table if necessary.
    void FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool bForceEmptyParagraph = false );

    void WriteTextFootnoteNumStr(const SwFmtFtn& rFootnote);

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
     * This is the same as m_aStyles but the conents of it is written last.
     */
    OStringBuffer m_aStylesEnd;

    /*
     * We just get a "end of strike" mark at the end of strike, store here what to finish: single or double strike.
     */
    bool m_bStrikeDouble;

    /*
     * The current table helper.
     */
    SwWriteTable *m_pTableWrt;

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
    bool m_bTblAfterCell;

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

    /*
     * If we had a field result in the URL.
     */
    bool m_bHadFieldResult;

    /// If we ended a table row without starting a new one.
    bool m_bTableRowEnded;

    /// Number of cells from the table definition, by depth.
    std::map<sal_uInt32,sal_uInt32> m_aCells;

    /// If we're in a paragraph that has a single empty run only.
    bool m_bSingleEmptyRun;

    bool m_bInRun;

    unsigned int m_nPostitFieldsMaxId;

    /// When exporting fly frames, this holds the real size of the frame.
    const Size* m_pFlyFrameSize;

    std::vector< std::pair<OString, OString> > m_aFlyProperties;

    boost::optional<XFillStyle> m_oFillStyle;

public:
    RtfAttributeOutput( RtfExport &rExport );

    virtual ~RtfAttributeOutput();

    /// Return the right export class.
    virtual MSWordExportBase& GetExport();

    OStringBuffer m_aTabStop;

    /// Access to the page style of the previous paragraph.
    const SwPageDesc* m_pPrevPageDesc;

    // These are used by wwFont::WriteRtf()
    /// Start the font.
    void StartFont( const OUString& rFamilyName ) const;

    /// End the font.
    void EndFont() const;

    /// Alternate name for the font.
    void FontAlternateName( const OUString& rName ) const;

    /// Font charset.
    void FontCharset( sal_uInt8 nCharSet ) const;

    /// Font family.
    void FontFamilyType( FontFamily eFamily, const wwFont &rFont ) const;

    /// Font pitch.
    void FontPitchType( FontPitch ePitch ) const;

    /// Writes binary data as a hex dump.
    static OString WriteHex(const sal_uInt8* pData, sal_uInt32 nSize, SvStream* pStream = 0, sal_uInt32 nLimit = 64);

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) SAL_OVERRIDE;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
