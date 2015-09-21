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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8ATTRIBUTEOUTPUT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8ATTRIBUTEOUTPUT_HXX

#include "attributeoutputbase.hxx"
#include "wrtww8.hxx"

class WW8AttributeOutput : public AttributeOutputBase
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) SAL_OVERRIDE;

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t /*pTextNodeInfo*/ ) SAL_OVERRIDE {}

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) SAL_OVERRIDE;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwNode& /*rNode*/) SAL_OVERRIDE {}

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() SAL_OVERRIDE {}

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& /*rParagraphMarkerProperties*/, const SwRedlineData* /*pRedlineData*/, const SwRedlineData* /*pRedlineParagraphMarkerDeleted*/, const SwRedlineData* /*pRedlineParagraphMarkerInserted*/) SAL_OVERRIDE {}

    /// Empty paragraph.
    virtual void EmptyParagraph() SAL_OVERRIDE;

    /// Start of the text run.
    ///
    virtual void StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false ) SAL_OVERRIDE;

    virtual void OnTOXEnding() SAL_OVERRIDE;

    /// End of the text run.
    ///
    /// No-op for binary filters.
    virtual void EndRun() SAL_OVERRIDE {}

    /// Before we start outputting the attributes.
    virtual void StartRunProperties() SAL_OVERRIDE;

    /// After we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData ) SAL_OVERRIDE;

    /// Output text.
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet ) SAL_OVERRIDE;

    /// Output text (without markup).
    virtual void RawText(const OUString& rText, rtl_TextEncoding eCharSet) SAL_OVERRIDE;

    /// Output ruby start.
    virtual void StartRuby( const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby ) SAL_OVERRIDE;

    /// Output ruby end.
    virtual void EndRuby() SAL_OVERRIDE;

    /// Output URL start.
    virtual bool StartURL( const OUString &rUrl, const OUString &rTarget ) SAL_OVERRIDE;

    /// Output URL end.
    virtual bool EndURL(bool) SAL_OVERRIDE;

    virtual void FieldVanish( const OUString& rText, ww::eField eType ) SAL_OVERRIDE;

    /// Output redlining.
    virtual void Redline( const SwRedlineData* pRedline ) SAL_OVERRIDE;

    virtual void FormatDrop( const SwTextNode& rNode, const SwFormatDrop &rSwFormatDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) SAL_OVERRIDE;

    /// Output FKP (Formatted disK Page) - necessary for binary formats only.
    /// FIXME having it in AttributeOutputBase is probably a hack, it
    /// should be in WW8AttributeOutput only...
    virtual void OutputFKP(bool bForce = false) SAL_OVERRIDE;

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle ) SAL_OVERRIDE;

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) SAL_OVERRIDE;
    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableRowEnd( sal_uInt32 nDepth = 1 ) SAL_OVERRIDE;

    /// Start of the styles table.
    virtual void StartStyles() SAL_OVERRIDE;

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles ) SAL_OVERRIDE;

    /// Write default style.
    virtual void DefaultStyle( sal_uInt16 nStyle ) SAL_OVERRIDE;

    /// Start of a style in the styles table.
    virtual void StartStyle( const OUString& rName, StyleType eType,
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwIdi, sal_uInt16 nId,
            bool bAutoUpdate ) SAL_OVERRIDE;

    /// End of a style in the styles table.
    virtual void EndStyle() SAL_OVERRIDE;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle ) SAL_OVERRIDE;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp ) SAL_OVERRIDE;

    /// Numbering rule and Id.
    virtual void OutlineNumbering(sal_uInt8 nLvl) SAL_OVERRIDE;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak ) SAL_OVERRIDE;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL ) SAL_OVERRIDE;

    /// Start of the section properties.
    virtual void StartSection() SAL_OVERRIDE;

    /// End of the section properties.
    ///
    /// No-op for binary filters.
    virtual void EndSection() SAL_OVERRIDE {}

    /// Protection of forms.
    virtual void SectionFormProtection( bool bProtected ) SAL_OVERRIDE;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo ) SAL_OVERRIDE;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() SAL_OVERRIDE;

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrameFormat* pFormat, const SwFrameFormat* pFirstPageFormat ) SAL_OVERRIDE;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi ) SAL_OVERRIDE;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering( sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber ) SAL_OVERRIDE;

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode ) SAL_OVERRIDE;

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule ) SAL_OVERRIDE;

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
        const SvxBrushItem* pBrush = 0) SAL_OVERRIDE; //For i120928,transfer graphic of bullet

protected:
    /// Output frames - the implementation.
    void OutputFlyFrame_Impl( const sw::Frame& rFormat, const Point& rNdTopLeft ) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap( const SvxCaseMapItem& ) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor( const SvxColorItem& ) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour( const SvxContourItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut( const SvxCrossedOutItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement( const SvxEscapementItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont( const SvxFontItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize( const SvxFontHeightItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning( const SvxKerningItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage( const SvxLanguageItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture( const SvxPostureItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow( const SvxShadowedItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline( const SvxUnderlineItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight( const SvxWeightItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern( const SvxAutoKernItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText( const SvxBlinkItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground( const SvxBrushItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK( const SvxFontItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK( const SvxFontHeightItem& rHt ) SAL_OVERRIDE { CharFontSize( rHt ); }

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& rHt ) SAL_OVERRIDE { CharLanguage( rHt ); }

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& rHt ) SAL_OVERRIDE { CharPosture( rHt ); }

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& rHt ) SAL_OVERRIDE { CharWeight( rHt ); }

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& rHt ) SAL_OVERRIDE { CharFontSize( rHt ); }

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& rHt ) SAL_OVERRIDE { CharLanguage( rHt ); }

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL( const SvxPostureItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL( const SvxWeightItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL( const SfxPoolItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint( const SfxPoolItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate( const SvxCharRotateItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark( const SvxEmphasisMarkItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines( const SvxTwoLinesItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth( const SvxCharScaleWidthItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief( const SvxCharReliefItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden( const SvxCharHiddenItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder( const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight( const SvxBrushItem& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFormatINetFormat& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFormatCharFormat& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFormatFootnote& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFormatSplitItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl( const SwTextNode *pTextNd, sal_Int32 nLvl, sal_Int32 nNumId ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFormatFrmSize& ) SAL_OVERRIDE;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& ) SAL_OVERRIDE;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& ) SAL_OVERRIDE;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& rHt ) SAL_OVERRIDE;

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFormatSurround& ) SAL_OVERRIDE;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFormatVertOrient& ) SAL_OVERRIDE;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFormatHoriOrient& ) SAL_OVERRIDE;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFormatAnchor& ) SAL_OVERRIDE;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& ) SAL_OVERRIDE;

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& ) SAL_OVERRIDE;

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol & rCol, bool bEven, SwTwips nPageSize ) SAL_OVERRIDE;

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFormatKeepItem& ) SAL_OVERRIDE;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& ) SAL_OVERRIDE;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFormatLineNumber& ) SAL_OVERRIDE;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_GRABBAG
    virtual void CharGrabBag( const SfxGrabBagItem& ) SAL_OVERRIDE;

    // Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel( const SfxUInt16Item& ) SAL_OVERRIDE;

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pField ) SAL_OVERRIDE;

    virtual void RefField ( const SwField& rField, const OUString& rRef ) SAL_OVERRIDE;
    virtual void HiddenField( const SwField& rField ) SAL_OVERRIDE;
    virtual void SetField( const SwField& rField, ww::eField eType, const OUString& rCmd ) SAL_OVERRIDE;
    virtual void PostitField( const SwField* pField ) SAL_OVERRIDE;
    virtual bool DropdownField( const SwField* pField ) SAL_OVERRIDE;
    virtual bool PlaceholderField( const SwField* pField ) SAL_OVERRIDE;

    virtual bool AnalyzeURL( const OUString& rURL, const OUString& rTarget, OUString* pLinkURL, OUString* pMark ) SAL_OVERRIDE;

    /// Reference to the export, where to get the data from
    WW8Export &m_rWW8Export;

    /// For output of styles.
    ///
    /// We have to remember these positions between the StartStyle() and
    /// EndStyle().
    sal_uInt16 nPOPosStdLen1, nPOPosStdLen2;

    /// For output of styles.
    ///
    /// We have to remember this position between StartStyleProperties() and
    /// EndStyleProperties().
    sal_uInt16 m_nStyleStartSize, m_nStyleLenPos;

    /// For output of styles.
    ///
    /// Used between StartStyles() and EndStyles().
    sal_uLong m_nStyAnzPos;

    /// For output of run properties.
    ///
    /// We have to remember the number of field results, and do not export end
    /// of the field results if we were forced to split text.
    sal_uInt16 m_nFieldResults;

    bool mbOnTOXEnding;

public:
    explicit WW8AttributeOutput( WW8Export &rWW8Export )
        : AttributeOutputBase()
        , m_rWW8Export(rWW8Export)
        , nPOPosStdLen1(0)
        , nPOPosStdLen2(0)
        , m_nStyleStartSize(0)
        , m_nStyleLenPos(0)
        , m_nStyAnzPos(0)
        , m_nFieldResults(0)
        , mbOnTOXEnding(false)
    {
    }

    virtual ~WW8AttributeOutput() {}

    /// Return the right export class.
    virtual WW8Export& GetExport() SAL_OVERRIDE { return m_rWW8Export; }

protected:
    /// Output the bold etc. attributes
    void OutputWW8Attribute( sal_uInt8 nId, bool bVal );

    /// Output the bold etc. attributes, the Complex Text Layout version
    void OutputWW8AttributeCTL( sal_uInt8 nId, bool bVal );

    void TableCellBorders(
        ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );

};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_WW8ATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
