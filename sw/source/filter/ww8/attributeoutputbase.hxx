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

#ifndef _ATTRIBUTEOUTPUTBASE_HXX_
#define _ATTRIBUTEOUTPUTBASE_HXX_

#include "fields.hxx"
#include "WW8TableInfo.hxx"

#include <rtl/textenc.h>
#include <editeng/svxenum.hxx>
#include <tools/solar.h>

#include <swtypes.hxx>
#include <wrtswtbl.hxx>
#include <fldbas.hxx>

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
class SwFmtINetFmt;
class SwFmtCharFmt;
class SwFmtFld;
class SwFmtFlyCnt;
class SwFmtFtn;
class SvxLineSpacingItem;
class SvxAdjustItem;
class SvxFmtSplitItem;
class SvxWidowsItem;
class SvxTabStopItem;
class SvxHyphenZoneItem;
class SwNumRuleItem;
class SfxBoolItem;
class SfxPoolItem;
class SfxItemSet;
class SvxParaVertAlignItem;
class SvxParaGridItem;
class SwFmtFrmSize;
class SvxPaperBinItem;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class SwFmtPageDesc;
class SvxFmtBreakItem;
class SwFmtSurround;
class SwFmtVertOrient;
class SwFmtHoriOrient;
class SwFmtAnchor;
class SvxBrushItem;
class SvxBoxItem;
class SwFmtCol;
class SvxFmtKeepItem;
class SwTextGridItem;
class SwFmtLineNumber;
class SvxFrameDirectionItem;
class SfxGrabBagItem;
class SwFmtRuby;
class SwTxtNode;
class SwTOXMark;
class SwRedlineData;
class SwSection;
class SwFmtDrop;
class SwFrmFmt;
class SwNumFmt;
class SwFmt;
struct WW8_SepInfo;
class SwLineNumberInfo;
class SwNumRule;
class wwFont;

using ::editeng::SvxBorderLine;

class String;
namespace rtl { class OUString; }

class MSWordExportBase;

namespace sw { class Frame; }

namespace msword {
    const sal_uInt8 ColumnBreak = 0xE;
    const sal_uInt8 PageBreak   = 0xC;
}

class AttributeOutputBase
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) = 0;

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo ) = 0;

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) = 0;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties( const SwTxtNode& rNode ) = 0;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties() = 0;

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

    /// for docx w:commentRangeStart
    virtual void WritePostitFieldStart() {};

    /// for docx w:commentRangeEnd
    virtual void WritePostitFieldEnd() {};

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet ) = 0;

    /// Output text (without markup).
    virtual void RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet ) = 0;

    /// Output ruby start.
    virtual void StartRuby( const SwTxtNode& rNode, xub_StrLen nPos, const SwFmtRuby& rRuby ) = 0;

    /// Output ruby end.
    virtual void EndRuby() = 0;

    /// Output URL start.
    virtual bool StartURL( const String& rUrl, const String& rTarget ) = 0;

    /// Output URL end.
    virtual bool EndURL() = 0;

    virtual void FieldVanish( const String& rTxt, ww::eField eType ) = 0;

    void StartTOX( const SwSection& rSect );

    void EndTOX( const SwSection& rSect );

    virtual void TOXMark( const SwTxtNode& rNode, const SwTOXMark& rAttr );

    /// Output redlining.
    virtual void Redline( const SwRedlineData* pRedline ) = 0;

    virtual void FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) = 0;

    /// Output FKP (Formatted disK Page) - necessary for binary formats only.
    /// FIXME having it in AttributeOutputBase is probably a hack, it
    /// should be in WW8AttributeOutput only...
    virtual void OutputFKP() {}

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle ) = 0;

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfo ) = 0;

    virtual void TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo ) = 0;

    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner ) = 0;

    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) = 0;

    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) = 0;

    virtual void TableRowEnd( sal_uInt32 nDepth ) = 0;

    /// Start of the styles table.
    virtual void StartStyles() = 0;

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles ) = 0;

    /// Write default style.
    virtual void DefaultStyle( sal_uInt16 nStyle ) = 0;

    /// Start of a style in the styles table.
    virtual void StartStyle( const String& rName, bool bPapFmt,
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
            bool bAutoUpdate ) = 0;

    /// End of a style in the styles table.
    virtual void EndStyle() = 0;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle ) = 0;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp ) = 0;

    /// Numbering rule and Id.
    virtual void OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt& rNFmt, const SwFmt& rFmt ) = 0;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak ) = 0;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL ) = 0;

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

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* pFirstPageFmt ) = 0;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi ) = 0;

    /// The style of the page numbers.
    ///
    /// nPageRestartNumberr being 0 means no restart.
    virtual void SectionPageNumbering( sal_uInt16 nNumType, sal_uInt16 nPageRestartNumber ) = 0;

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode ) = 0;

    /// Special header/footer flags, needed for old versions only.
    virtual void SectionWW6HeaderFooterFlags( sal_uInt8 /*nHeadFootFlags*/ ) {}

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
        const String &rNumberingString,
        const SvxBrushItem* pBrush = 0) = 0; // #i120928 export graphic of bullet

protected:

    void GetNumberPara( String& rStr, const SwField& rFld );

    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl( const sw::Frame& rFmt, const Point& rNdTopLeft ) = 0;

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
    virtual void CharBorder( const SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow ) = 0;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFmtINetFmt& ) = 0;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFmtCharFmt& ) = 0;

    /// Sfx item RES_TXTATR_FIELD
    void TextField( const SwFmtFld& );

    /// Sfx item RES_TXTATR_FLYCNT
    virtual void TextFlyContent( const SwFmtFlyCnt& );

    /// Sfx item RES_TXTATR_FTN
    ///
    /// This one is common for both WW8AttributeOutput as well as
    /// DocxAttributeOutput.
    void TextFootnote( const SwFmtFtn& );

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFmtFtn& ) = 0;

    /// Sfx item RES_PARATR_LINESPACING
    void ParaLineSpacing( const SvxLineSpacingItem& );

    /// Count the values in ParaLineSpacing, and pass theme here.
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti ) = 0;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& ) = 0;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFmtSplitItem& ) = 0;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& ) = 0;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& ) = 0;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& ) = 0;

    /// Sfx item RES_PARATR_NUMRULE
    void ParaNumRule( const SwNumRuleItem& );

    /// Numbering - the implementation.
    virtual void ParaNumRule_Impl( const SwTxtNode *pTxtNd, sal_Int32 nLvl, sal_Int32 nNumId ) = 0;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& ) = 0;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    void ParaHangingPunctuation( const SfxBoolItem& rHt ) { ParaScriptSpace( rHt ); };

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    void ParaForbiddenRules( const SfxBoolItem& rHt ) { ParaScriptSpace( rHt ); };

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& ) = 0;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& ) = 0;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFmtFrmSize& ) = 0;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& ) = 0;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& ) = 0;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& ) = 0;

    /// Sfx item RES_PAGEDESC
    void FormatPageDescription( const SwFmtPageDesc& );

    /// Sfx item RES_BREAK
    void FormatBreak( const SvxFmtBreakItem& );

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFmtSurround& ) = 0;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFmtVertOrient& ) = 0;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFmtHoriOrient& ) = 0;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFmtAnchor& ) = 0;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& ) = 0;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& ) = 0;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& ) = 0;

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& ) = 0;

    /// Sfx item RES_COL
    void FormatColumns( const SwFmtCol& );

    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol & rCol, bool bEven, SwTwips nPageSize ) = 0;

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFmtKeepItem& ) = 0;

    /// Compute the grid character pitch
    sal_uInt32 GridCharacterPitch( const SwTextGridItem& rGrid ) const;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& ) = 0;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFmtLineNumber& ) = 0;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& ) = 0;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& ) = 0;

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pFld ) = 0;

    virtual void RefField( const SwField& rFld, const String& rRef ) = 0;
    virtual void HiddenField( const SwField& rFld ) = 0;
    virtual void SetField( const SwField& rFld, ww::eField eType, const String& rCmd ) = 0;
    virtual void PostitField( const SwField* pFld ) = 0;
    virtual bool DropdownField( const SwField* pFld ) = 0;
    virtual bool PlaceholderField( const SwField* pFld ) = 0;

    virtual bool AnalyzeURL( const OUString& rUrl, const OUString& rTarget, OUString* pLinkURL, OUString* pMark );

    ww8::GridColsPtr GetGridCols( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );

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
    void OutputStyleItemSet( const SfxItemSet& rSet, sal_Bool bDeep, sal_Bool bTestForDefault );

    /// Output frames.
    void OutputFlyFrame( const sw::Frame& rFmt );

    void GetTablePageSize
    ( ww8::WW8TableNodeInfoInner * pTableTextNodeInfoInner,
      sal_uInt32& rPageSize, bool& rRelBoxSize );

    /// Exports the definition (image, size) of a single numbering picture bullet.
    virtual void BulletDefinition(int /*nId*/, const Graphic& /*rGraphic*/, Size /*aSize*/) {}

};

#endif // _ATTRIBUTEOUTPUTBASE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
