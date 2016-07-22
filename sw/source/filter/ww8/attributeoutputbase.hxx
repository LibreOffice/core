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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_ATTRIBUTEOUTPUTBASE_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_ATTRIBUTEOUTPUTBASE_HXX

#include "fields.hxx"
#include "WW8TableInfo.hxx"
#include "wrtww8.hxx"

#include <rtl/textenc.h>
#include <editeng/svxenum.hxx>
#include <tools/solar.h>
#include <boost/optional.hpp>

#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <swtypes.hxx>
#include <wrtswtbl.hxx>
#include <fldbas.hxx>
#include <IDocumentRedlineAccess.hxx>

#include <vector>

class Point;
class SvxCaseMapItem;
class SvxColorItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxEscapementItem;
class SvxFontItem;
class SvxFontHeightItem;
class SvxKerningItem;
class SvxLanguageItem;
class SvxPostureItem;
class SvxShadowedItem;
class SvxUnderlineItem;
class SvxWeightItem;
class SvxAutoKernItem;
class SvxBlinkItem;
class SvxBrushItem;
class XFillStyleItem;
class XFillGradientItem;
class SvxFontItem;
class SvxFontHeightItem;
class SvxLanguageItem;
class SvxPostureItem;
class SvxWeightItem;
class SvxFontItem;
class SvxFontHeightItem;
class SvxLanguageItem;
class SvxPostureItem;
class SvxWeightItem;
class SvxCharRotateItem;
class SvxEmphasisMarkItem;
class SvxTwoLinesItem;
class SvxCharScaleWidthItem;
class SvxCharReliefItem;
class SvxCharHiddenItem;
class SvxBoxItem;
class SwFormatINetFormat;
class SwFormatCharFormat;
class SwFormatField;
class SwFormatFlyCnt;
class SwFormatFootnote;
class SvxLineSpacingItem;
class SvxAdjustItem;
class SvxFormatSplitItem;
class SvxWidowsItem;
class SvxTabStopItem;
class SvxHyphenZoneItem;
class SwNumRuleItem;
class SfxBoolItem;
class SfxPoolItem;
class SfxItemSet;
class SvxParaVertAlignItem;
class SvxParaGridItem;
class SwFormatFrameSize;
class SvxPaperBinItem;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFormatPageDesc;
class SvxFormatBreakItem;
class SwFormatSurround;
class SwFormatVertOrient;
class SwFormatHoriOrient;
class SwFormatAnchor;
class SvxBrushItem;
class SvxBoxItem;
class SwFormatCol;
class SvxFormatKeepItem;
class SwTextGridItem;
class SwFormatLineNumber;
class SvxFrameDirectionItem;
class SfxGrabBagItem;
class SfxUInt16Item;
class SwFormatRuby;
class SwTextNode;
class SwTOXMark;
class SwRedlineData;
class SwSection;
class SwFormatDrop;
class SwFrameFormat;
class SwNumFormat;
class SwFormat;
struct WW8_SepInfo;
class SwLineNumberInfo;
class SwNumRule;
class wwFont;

namespace editeng { class SvxBorderLine; }

namespace rtl { class OUString; }

class MSWordExportBase;

namespace ww8 { class Frame; }

namespace msword {
    const sal_uInt8 ColumnBreak = 0xE;
    const sal_uInt8 PageBreak   = 0xC;
}

/// Type of a style in the style table.
enum StyleType
{
    STYLE_TYPE_PARA,
    STYLE_TYPE_CHAR,
    STYLE_TYPE_LIST
};

class AttributeOutputBase
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) = 0;

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo ) = 0;

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) = 0;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwNode& rNode) = 0;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() = 0;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted) = 0;

    /// Empty paragraph.
    virtual void EmptyParagraph() = 0;

    /// Start of the text run.
    virtual void StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false ) = 0;

    /// End of the text run.
    virtual void EndRun() = 0;

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties() = 0;

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData ) = 0;

    /// docx requires footnoteRef/endnoteRef tag at the beginning of each of them
    virtual void FootnoteEndnoteRefTag() {};

    /// for docx footnotePr/endnotePr inside sectPr
    virtual void SectFootnoteEndnotePr() {};

    /// for docx w:commentReference
    virtual void WritePostitFieldReference() {};

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 ) = 0;

    /// Output text (without markup).
    virtual void RawText(const OUString& rText, rtl_TextEncoding eCharSet) = 0;

    /// Output ruby start.
    virtual void StartRuby( const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby ) = 0;

    /// Output ruby end.
    virtual void EndRuby() = 0;

    /// Output URL start.
    virtual bool StartURL( const OUString& rUrl, const OUString& rTarget ) = 0;

    /// Output URL end.
    virtual bool EndURL(bool isAtEndOfParagraph) = 0;

    virtual void FieldVanish( const OUString& rText, ww::eField eType ) = 0;

    void StartTOX( const SwSection& rSect );

    void EndTOX( const SwSection& rSect,bool bCareEnd=true );

    virtual void OnTOXEnding() {}

    void TOXMark( const SwTextNode& rNode, const SwTOXMark& rAttr );

    /// Output redlining.
    virtual void Redline( const SwRedlineData* pRedline ) = 0;

    virtual void FormatDrop( const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) = 0;

    /// Output FKP (Formatted disK Page) - necessary for binary formats only.
    /// FIXME having it in AttributeOutputBase is probably a hack, it
    /// should be in WW8AttributeOutput only...
    virtual void OutputFKP(bool /*bForce*/ = false) {}

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle ) = 0;

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner ) = 0;

    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) = 0;

    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) = 0;

    virtual void TableRowEnd( sal_uInt32 nDepth ) = 0;

    /// Start of the styles table.
    virtual void StartStyles() = 0;

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles ) = 0;

    /// Write default style.
    virtual void DefaultStyle() = 0;

    /// Start of a style in the styles table.
    virtual void StartStyle( const OUString& rName, StyleType eType,
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
            bool bAutoUpdate ) = 0;

    /// End of a style in the styles table.
    virtual void EndStyle() = 0;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle ) = 0;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp ) = 0;

    /// Numbering rule and Id.
    virtual void OutlineNumbering(sal_uInt8 nLvl) = 0;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak ) = 0;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = nullptr ) = 0;

    // preserve page vertical alignment
    virtual void TextVerticalAdjustment( const css::drawing::TextVerticalAdjust) {};

    /// Start of the section properties.
    virtual void StartSection() = 0;

    /// End of the section properties.
    virtual void EndSection() = 0;

    /// Protection of forms.
    virtual void SectionFormProtection( bool bProtected ) = 0;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo ) = 0;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() = 0;

    /// Set the state of the Fly at current position
    virtual void SetStateOfFlyFrame( FlyProcessingState /*nStateOfFlyFrame*/ ){};
    /// If the node has an anchor linked.
    virtual void SetAnchorIsLinkedToNode( bool /*bAnchorLinkedToNode*/){};

    /// Is processing of fly postponed ?
    virtual bool IsFlyProcessingPostponed(){ return false; };

    /// Reset the flag for FlyProcessing
    virtual void ResetFlyProcessingFlag(){};

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrameFormat* pFormat, const SwFrameFormat* pFirstPageFormat ) = 0;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi ) = 0;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering( sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber ) = 0;

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode ) = 0;

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule ) = 0;

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering( sal_uInt16 /*nId*/ ) {}

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering() {}

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
        const SvxBrushItem* pBrush = nullptr) = 0; // #i120928 export graphic of bullet

protected:

    static void GetNumberPara( OUString& rStr, const SwField& rField );

    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl( const ww8::Frame& rFormat, const Point& rNdTopLeft ) = 0;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap( const SvxCaseMapItem& ) = 0;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor( const SvxColorItem& ) = 0;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour( const SvxContourItem& ) = 0;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut( const SvxCrossedOutItem& ) = 0;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement( const SvxEscapementItem& ) = 0;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont( const SvxFontItem& ) = 0;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize( const SvxFontHeightItem& ) = 0;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning( const SvxKerningItem& ) = 0;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage( const SvxLanguageItem& ) = 0;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture( const SvxPostureItem& ) = 0;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow( const SvxShadowedItem& ) = 0;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline( const SvxUnderlineItem& ) = 0;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight( const SvxWeightItem& ) = 0;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern( const SvxAutoKernItem& ) = 0;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText( const SvxBlinkItem& ) = 0;

    /// Sfx item RES_CHRATR_BACKGROUND
    void CharBackgroundBase( const SvxBrushItem& );
    virtual void CharBackground( const SvxBrushItem& ) = 0;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK( const SvxFontItem& ) = 0;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK( const SvxFontHeightItem& ) = 0;

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& ) = 0;

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& ) = 0;

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& ) = 0;

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& ) = 0;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& ) = 0;

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& ) = 0;

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL( const SvxPostureItem& ) = 0;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL( const SvxWeightItem& ) = 0;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL( const SfxPoolItem& ) = 0;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint( const SfxPoolItem& ) = 0;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate( const SvxCharRotateItem& ) = 0;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark( const SvxEmphasisMarkItem& ) = 0;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines( const SvxTwoLinesItem& ) = 0;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth( const SvxCharScaleWidthItem& ) = 0;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief( const SvxCharReliefItem& ) = 0;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden( const SvxCharHiddenItem& ) = 0;

    /// Sfx item RES_CHRATR_BOX
    void FormatCharBorder( const SvxBoxItem& rBox );
    virtual void CharBorder( const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow ) = 0;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight( const SvxBrushItem& ) = 0;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFormatINetFormat& ) = 0;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFormatCharFormat& ) = 0;

    /// Sfx item RES_TXTATR_FIELD, RES_TXTATR_ANNOTATION and RES_TXTATR_INPUTFIELD
    void TextField( const SwFormatField& );

    /// Sfx item RES_TXTATR_FLYCNT
    void TextFlyContent( const SwFormatFlyCnt& );

    /// Sfx item RES_TXTATR_FTN
    ///
    /// This one is common for both WW8AttributeOutput as well as
    /// DocxAttributeOutput.
    void TextFootnote( const SwFormatFootnote& );

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFormatFootnote& ) = 0;

    /// Sfx item RES_PARATR_LINESPACING
    void ParaLineSpacing( const SvxLineSpacingItem& );

    /// Count the values in ParaLineSpacing, and pass theme here.
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti ) = 0;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& ) = 0;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFormatSplitItem& ) = 0;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& ) = 0;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& ) = 0;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& ) = 0;

    /// Sfx item RES_PARATR_NUMRULE
    void ParaNumRule( const SwNumRuleItem& );

    /// Numbering - the implementation.
    virtual void ParaNumRule_Impl( const SwTextNode *pTextNd, sal_Int32 nLvl, sal_Int32 nNumId ) = 0;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& ) = 0;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation( const SfxBoolItem& ) = 0;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules( const SfxBoolItem& ) = 0;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& ) = 0;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& ) = 0;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFormatFrameSize& ) = 0;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& ) = 0;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& ) = 0;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& ) = 0;

    /// Sfx item RES_PAGEDESC
    void FormatPageDescription( const SwFormatPageDesc& );

    /// Sfx item RES_BREAK
    void FormatBreak( const SvxFormatBreakItem& );

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFormatSurround& ) = 0;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFormatVertOrient& ) = 0;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFormatHoriOrient& ) = 0;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFormatAnchor& ) = 0;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& ) = 0;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& ) = 0;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& ) = 0;

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& ) = 0;

    /// Sfx item RES_COL
    void FormatColumns( const SwFormatCol& );

    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol & rCol, bool bEven, SwTwips nPageSize ) = 0;

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFormatKeepItem& ) = 0;

    /// Compute the grid character pitch
    sal_uInt32 GridCharacterPitch( const SwTextGridItem& rGrid ) const;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& ) = 0;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFormatLineNumber& ) = 0;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& ) = 0;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& ) = 0;

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag( const SfxGrabBagItem& ) = 0;

    /// Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel( const SfxUInt16Item& ) = 0;

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pField ) = 0;

    virtual void RefField( const SwField& rField, const OUString& rRef ) = 0;
    virtual void HiddenField( const SwField& rField ) = 0;
    virtual void SetField( const SwField& rField, ww::eField eType, const OUString& rCmd ) = 0;
    virtual void PostitField( const SwField* pField ) = 0;
    virtual bool DropdownField( const SwField* pField ) = 0;
    virtual bool PlaceholderField( const SwField* pField ) = 0;

    virtual bool AnalyzeURL( const OUString& rUrl, const OUString& rTarget, OUString* pLinkURL, OUString* pMark );

    ww8::GridColsPtr GetGridCols( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );
    ww8::WidthsPtr   GetColumnWidths( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );

public:
    AttributeOutputBase() {}
    virtual ~AttributeOutputBase() {}

    /// Return the right export class.
    virtual MSWordExportBase& GetExport() = 0;

    /// @overload
    const MSWordExportBase& GetExport() const { return const_cast< AttributeOutputBase* >( this )->GetExport(); }

    /// Call the right virtual function according to the type of the item.
    void OutputItem( const SfxPoolItem& rHt );

    /// Use OutputItem() on an item set - for styles.
    void OutputStyleItemSet( const SfxItemSet& rSet, bool bDeep, bool bTestForDefault );

    /// Output frames.
    void OutputFlyFrame( const ww8::Frame& rFormat );

    void GetTablePageSize
    ( ww8::WW8TableNodeInfoInner * pTableTextNodeInfoInner,
      long& rPageSize, bool& rRelBoxSize );

    /// Exports the definition (image, size) of a single numbering picture bullet.
    virtual void BulletDefinition(int /*nId*/, const Graphic& /*rGraphic*/, Size /*aSize*/) {}

    // Returns whether or not the 'SwTextNode' has a paragraph marker inserted \ deleted (using 'track changes')
    const SwRedlineData* GetParagraphMarkerRedline( const SwTextNode& rNode, RedlineType_t aRedlineType );
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_ATTRIBUTEOUTPUTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
