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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXATTRIBUTEOUTPUT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXATTRIBUTEOUTPUT_HXX

#include <memory>

#include "attributeoutputbase.hxx"
#include "fields.hxx"
#include <IMark.hxx>
#include "docxexport.hxx"

#include <editeng/boxitem.hxx>
#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <vcl/vclenum.hxx>
#include <svx/xenum.hxx>

#include <fldbas.hxx>

#include <vector>
#include <boost/optional.hpp>
#include <oox/export/vmlexport.hxx>
#include <oox/export/drawingml.hxx>
#include "docxtablestyleexport.hxx"

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

class SwGrfNode;
class SdrObject;

namespace docx { class FootnotesList; }
namespace oox { namespace drawingml { class DrawingML; } }

struct FieldInfos
{
    std::shared_ptr<const SwField> pField;
    const ::sw::mark::IFieldmark* pFieldmark;
    ww::eField  eType;
    bool        bOpen;
    bool        bClose;
    OUString    sCmd;
    FieldInfos() : pFieldmark(nullptr), eType(ww::eUNKNOWN), bOpen(false), bClose(false){}
};

enum DocxColBreakStatus
{
    COLBRK_NONE,
    COLBRK_POSTPONE,
    COLBRK_WRITE
};

/**
 * A structure that holds information about the options selected
 * when outputting a border to DOCX.
 *
 * There are 3 functions that initialize this structure:
 * - lcl_getTableDefaultBorderOptions - retrieves the options for when outputting table default borders
 * - lcl_getTableCellBorderOptions    - retrieves the options for when outputting table cell borders
 * - lcl_getBoxBorderOptions          - retrieves the options for when outputting box borders
 *
 */
struct OutputBorderOptions
{
    sal_Int32           tag = 0;
    bool                bUseStartEnd = false;
    bool                bWriteTag = true;
    bool                bWriteInsideHV = false;
    bool                bWriteDistance = false;
    SvxShadowLocation   aShadowLocation = SvxShadowLocation::NONE;
    std::shared_ptr<editeng::WordBorderDistances> pDistances;
};

/**
 * All the information that should be stashed away when we're in the middle of
 * of a table export and still have to do something else, e.g. export a shape.
 */
struct DocxTableExportContext
{
    ww8::WW8TableInfo::Pointer_t m_pTableInfo;
    bool m_bTableCellOpen;
    sal_uInt32 m_nTableDepth;
};

/**
 * A structure that holds flags for the table export.
 */
struct TableReference
{
    /// Remember if we are in an open cell, or not.
    bool m_bTableCellOpen;

    /// If paragraph sdt got opened in this table cell.
    bool m_bTableCellParaSdtOpen;

    /// Remember the current table depth.
    sal_uInt32 m_nTableDepth;

    TableReference()
        : m_bTableCellOpen(false),
        m_bTableCellParaSdtOpen(false),
        m_nTableDepth(0)
    {
    }
};

/// The class that has handlers for various resource types when exporting as DOCX.
class DocxAttributeOutput : public AttributeOutputBase, public oox::vml::VMLTextExport, public oox::drawingml::DMLTextExport
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) override;

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo ) override;

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) override;

    /// Empty paragraph.
    virtual void EmptyParagraph() override;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwNode& rNode) override;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() override;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted) override;

    /// Start of the text run.
    virtual void StartRun( const SwRedlineData* pRedlineData, sal_Int32 nPos, bool bSingleEmptyRun = false ) override;

    /// End of the text run.
    virtual void EndRun(const SwTextNode* pNode, sal_Int32 nPos, bool bLastRun = false) override;

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties() override;

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData ) override;

    virtual bool FootnoteEndnoteRefTag() override;

    virtual void SectFootnoteEndnotePr() override;

    virtual void WritePostitFieldReference() override;

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 ) override;

    /// Output text (without markup).
    virtual void RawText(const OUString& rText, rtl_TextEncoding eCharSet) override;

    /// Output ruby start.
    virtual void StartRuby( const SwTextNode& rNode, sal_Int32 nPos, const SwFormatRuby& rRuby ) override;

    /// Output ruby end.
    virtual void EndRuby(const SwTextNode& rNode, sal_Int32 nPos) override;

    /// Output URL start.
    virtual bool StartURL( const OUString& rUrl, const OUString& rTarget ) override;

    /// Output URL end.
    virtual bool EndURL(bool) override;

    virtual void FieldVanish( const OUString& rText, ww::eField eType ) override;

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline( const SwRedlineData* pRedline ) override;

    /// Output redlining.
    ///
    /// Start of the tag that encloses the run, fills the info according to
    /// the value of pRedlineData.
    void StartRedline( const SwRedlineData * pRedlineData );

    /// Output redlining.
    ///
    /// End of the tag that encloses the run.
    void EndRedline( const SwRedlineData * pRedlineData );

    virtual void SetStateOfFlyFrame( FlyProcessingState nStateOfFlyFrame ) override;
    virtual void SetAnchorIsLinkedToNode( bool bAnchorLinkedToNode ) override;
    virtual bool IsFlyProcessingPostponed() override;
    virtual void ResetFlyProcessingFlag() override;

    virtual void FormatDrop( const SwTextNode& rNode, const SwFormatDrop& rSwFormatDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) override;

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle ) override;

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    void TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );
    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner ) override;
    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) override;
    virtual void TableRowEnd( sal_uInt32 nDepth ) override;

    /// Start of the styles table.
    virtual void StartStyles() override;

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles ) override;

    /// Write default style.
    virtual void DefaultStyle() override;

    /// Write Doc Defaults
    void DocDefaults( );

    /// Write latent styles.
    void LatentStyles();

    /** Similar to OutputItem(), but write something only if it is not the default.

        This is to output the docDefaults, and we should write something out
        only in case it is not what MSO already uses for the document by default.
    */
    void OutputDefaultItem(const SfxPoolItem& rHt);

    /// Start of a style in the styles table.
    virtual void StartStyle( const OUString& rName, StyleType eType,
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nWwId, sal_uInt16 nId,
            bool bAutoUpdate ) override;

    /// End of a style in the styles table.
    virtual void EndStyle() override;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle ) override;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp ) override;

    /// Numbering rule and Id.
    virtual void OutlineNumbering(sal_uInt8 nLvl) override;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak ) override;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = nullptr ) override;

    // preserve DOCX page vertical alignment
    virtual void TextVerticalAdjustment( const css::drawing::TextVerticalAdjust ) SAL_OVERRIDE;

    /// Start of the section properties.
    virtual void StartSection() override;

    /// End of the section properties.
    virtual void EndSection() override;

    /// Protection of forms.
    virtual void SectionFormProtection( bool bProtected ) override;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo ) override;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() override;

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrameFormat* pFormat, const SwFrameFormat* pFirstPageFormat ) override;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi ) override;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering( sal_uInt16 nNumType, const ::boost::optional<sal_uInt16>& oPageRestartNumber ) override;

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode ) override;

    /// Start the font.
    void StartFont( const OUString& rFamilyName ) const;

    /// End the font.
    void EndFont() const;

    /// Alternate name for the font.
    void FontAlternateName( const OUString& rName ) const;

    /// Font charset.
    void FontCharset( sal_uInt8 nCharSet, rtl_TextEncoding nEncoding ) const;

    /// Font family.
    void FontFamilyType( FontFamily eFamily ) const;

    /// Font pitch.
    void FontPitchType( FontPitch ePitch ) const;

    /// Write out the font into the document, if it's an embedded font.
    void EmbedFont( const OUString& name, FontFamily family, FontPitch pitch );

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule ) override;

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering( sal_uInt16 nId ) override;

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering() override;

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
        const SvxBrushItem* pBrush ) override;

    void WriteField_Impl( const SwField* pField, ww::eField eType, const OUString& rFieldCmd, FieldFlags nMode );
    void WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark );

    void WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void WriteFinalBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void WriteAnnotationMarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void PushRelIdCache();
    void PopRelIdCache();
    /// End possibly opened paragraph sdt block.
    void EndParaSdtBlock();

private:
    /// Initialize the structures where we are going to collect some of the paragraph properties.
    ///
    /// Some of the properties have to be collected from more sources, and are
    /// actually not written between StartParagraphProperties and
    /// EndParagraphProperties.  They are output in this method, which is
    /// supposed to be called just before outputting </rPr> whenever it is done.
    void InitCollectedParagraphProperties();

    /// Output what we collected during the run properties output.
    ///
    /// @see WriteCollectedParagrapProperties().
    void WriteCollectedParagraphProperties();

    /// Initialize the structures where we are going to collect some of the run properties.
    ///
    /// This is an equivalent of InitCollectedParagraphProperties(), resp.
    /// WriteCollectectedParagraphProperties().
    ///
    /// @see InitCollectedParagraphProperties().
    void InitCollectedRunProperties();

    /// Output what we collected during the run properties output.
    ///
    /// @see InitCollectedRunProperies(), WriteCollectedParagraphProperties()
    void WriteCollectedRunProperties();

    /// Output graphic fly frames or replacement graphics for OLE nodes.
    ///
    /// For graphic frames, just use the first two parameters, for OLE
    /// replacement graphics, set the first as 0, and pass the remaining three.
    ///
    /// @see WriteOLE2Obj()
    void FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj = nullptr);
    void WriteSrcRect( const SdrObject* pSdrObj, const SwFrameFormat* pFrameFormat );
    void WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat);
    bool WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize );
    bool WriteOLEMath( const SwOLENode& rNode );
    bool PostponeOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat );
    void WriteOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* rFlyFrameFormat );

    void WriteActiveXControl(const SdrObject* pObject, const SwFrameFormat& rFrameFormat, bool bInsideRun);
    bool ExportAsActiveXControl(const SdrObject* pObject) const;

    /// checks whether the current component is a diagram
    static bool IsDiagram (const SdrObject* sdrObject);

    void InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );
    void StartTable( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );
    void StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner );
    void StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow );
    void TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t const & pTableTextNodeInfoInner, sal_uInt32 nCell, sal_uInt32 nRow );
    void EndTableCell( sal_uInt32 nCell );
    void EndTableRow( );
    void EndTable();
    void SyncNodelessCells(ww8::WW8TableNodeInfoInner::Pointer_t const & pInner, sal_Int32 nCell, sal_uInt32 nRow);
    void PopulateFrameProperties(const SwFrameFormat* pFrameFormat, const Size& rSize);
    static bool TextBoxIsFramePr(const SwFrameFormat& rFrameFormat);
    /// End cell, row, and even the entire table if necessary.
    void FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t const & pInner, bool bForceEmptyParagraph = false );

    void WriteFFData( const FieldInfos& rInfos );
    void WritePendingPlaceholder();

    void EmbedFontStyle( const OUString& name, int tag, FontFamily family, FontItalic italic, FontWeight weight,
        FontPitch pitch );

    /**
     * Translate an ico value to the corresponding HighlightColorValues enumaration item
     *
     * @param[in]   nIco      ico value [0..16]
     * @return      color name (e.g. "red"), if color is inside [1..16] range
     *              empty string, otherwise
    **/
    static OString TransHighlightColor( sal_uInt8 nIco );
protected:

    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl( const ww8::Frame& rFormat, const Point& rNdTopLeft ) override;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap( const SvxCaseMapItem& rCaseMap ) override;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor( const SvxColorItem& rColor) override;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour( const SvxContourItem& rContour ) override;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut( const SvxCrossedOutItem& rCrossedOut ) override;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement( const SvxEscapementItem& rEscapement ) override;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont( const SvxFontItem& rFont ) override;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize( const SvxFontHeightItem& rFontSize ) override;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning( const SvxKerningItem& rKerning ) override;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage( const SvxLanguageItem& rLanguage ) override;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture( const SvxPostureItem& rPosture ) override;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow( const SvxShadowedItem& rShadow ) override;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline( const SvxUnderlineItem& rUnderline ) override;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight( const SvxWeightItem& rWeight ) override;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern( const SvxAutoKernItem& ) override;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText( const SvxBlinkItem& rBlink ) override;

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground( const SvxBrushItem& rBrush ) override;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK( const SvxFontItem& rFont ) override;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK( const SvxFontHeightItem& rFontSize ) override { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& rLanguageItem ) override { CharLanguage( rLanguageItem ); }

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& rPosture ) override;

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& rWeight ) override;

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& rFont ) override;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& rFontSize ) override { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& rLanguageItem ) override { CharLanguage( rLanguageItem); }

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL( const SvxPostureItem& rWeight ) override;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL( const SvxWeightItem& rWeight ) override;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL( const SfxPoolItem& ) override;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint( const SfxPoolItem& ) override;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate( const SvxCharRotateItem& rRotate ) override;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark ) override;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines( const SvxTwoLinesItem& rTwoLines ) override;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth ) override;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief( const SvxCharReliefItem& rRelief) override;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden( const SvxCharHiddenItem& rHidden ) override;

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder( const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow ) override;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight( const SvxBrushItem& rHighlight ) override;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFormatINetFormat& ) override;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFormatCharFormat& ) override;

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFormatFootnote& ) override;

    /// Output the footnote/endnote reference (if there's one to output).
    void FootnoteEndnoteReference();

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti ) override;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& rAdjust ) override;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFormatSplitItem& rSplit ) override;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& rWidows ) override;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& rTabStop ) override;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& ) override;

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl( const SwTextNode *pTextNd, sal_Int32 nLvl, sal_Int32 nNumId ) override;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& ) override;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation( const SfxBoolItem& ) override;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules( const SfxBoolItem& ) override;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& rAlign ) override;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& ) override;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFormatFrameSize& ) override;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& ) override;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& rLRSpace ) override;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& rULSpace ) override;

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFormatSurround& ) override;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFormatVertOrient& ) override;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFormatHoriOrient& ) override;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFormatAnchor& ) override;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& ) override;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& ) override;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& ) override;

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& ) override;

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFormatCol & rCol, bool bEven, SwTwips nPageSize ) override;

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFormatKeepItem& ) override;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& ) override;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFormatLineNumber& ) override;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& ) override;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& ) override;

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag( const SfxGrabBagItem& ) override;

    // Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel( const SfxUInt16Item& ) override;

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pField ) override;

    virtual void RefField( const SwField& rField, const OUString& rRef ) override;
    virtual void HiddenField( const SwField& rField ) override;
    virtual void SetField( const SwField& rField, ww::eField eType, const OUString& rCmd ) override;
    virtual void PostitField( const SwField* pField ) override;
    virtual bool DropdownField( const SwField* pField ) override;
    virtual bool PlaceholderField( const SwField* pField ) override;

    virtual bool AnalyzeURL( const OUString& rURL, const OUString& rTarget, OUString* pLinkURL, OUString* pMark ) override;

    virtual void WriteBookmarkInActParagraph( const OUString& rName, sal_Int32 nFirstRunPos, sal_Int32 nLastRunPos ) override;

    /// Reference to the export, where to get the data from
    DocxExport &m_rExport;

    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

    /// DrawingML access
    oox::drawingml::DrawingML &m_rDrawingML;

private:

    void DoWriteBookmarkTagStart(const OUString & bookmarkName);
    void DoWriteBookmarkTagEnd(const OUString & bookmarkName);
    void DoWriteBookmarksStart(std::vector<OUString>& rStarts);
    void DoWriteBookmarksEnd(std::vector<OUString>& rEnds);
    void DoWriteBookmarkStartIfExist(sal_Int32 nRunPos);
    void DoWriteBookmarkEndIfExist(sal_Int32 nRunPos);

    void DoWritePermissionTagStart(const OUString & permission);
    void DoWritePermissionTagEnd(const OUString & permission);
    void DoWritePermissionsStart();
    void DoWritePermissionsEnd();

    void DoWriteAnnotationMarks( );
    void WritePostponedGraphic();
    void WritePostponedMath(const SwOLENode* pObject);
    void WritePostponedFormControl(const SdrObject* pObject);
    void WritePostponedActiveXControl(bool bInsideRun);
    void WritePostponedDiagram();
    void WritePostponedChart();
    void WritePostponedOLE();
    void WritePostponedDMLDrawing();
    void WritePostponedCustomShape();

    void WriteSdtBlock(sal_Int32& nSdtPrToken,
                       rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenChildren,
                       rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrTokenAttributes,
                       rtl::Reference<sax_fastparser::FastAttributeList>& pSdtPrDataBindingAttrs,
                       OUString& rSdtPrAlias,
                       bool bPara);
    /// Closes a currently open SDT block.
    void EndSdtBlock();

    void StartField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun = false );
    void DoWriteCmd( const OUString& rCmd );
    void CmdField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun );
    void EndField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos& rInfos );
    void DoWriteFieldRunProperties( const SwTextNode* pNode, sal_Int32 nPos, bool bWriteCombChars = false );

    static void AddToAttrList( rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nAttrName, const sal_Char* sAttrValue );
    static void AddToAttrList( rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList, sal_Int32 nArgs, ... );

    rtl::Reference<sax_fastparser::FastAttributeList> m_pFontsAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pEastAsianLayoutAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pCharLangAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pSectionSpacingAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pParagraphSpacingAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pHyperlinkAttrList;
    /// If the current SDT around runs should be ended before the current run.
    bool m_bEndCharSdt;
    /// If an SDT around runs is currently open.
    bool m_bStartedCharSdt;
    /// If an SDT around paragraphs is currently open.
    bool m_bStartedParaSdt;
    /// Attributes of the run color
    rtl::Reference<sax_fastparser::FastAttributeList> m_pColorAttrList;
    /// Attributes of the paragraph background
    rtl::Reference<sax_fastparser::FastAttributeList> m_pBackgroundAttrList;
    OUString m_sOriginalBackgroundColor;
    OUString m_hyperLinkAnchor;
    bool m_endPageRef;
    std::unique_ptr<docx::FootnotesList> m_pFootnotesList;
    std::unique_ptr<docx::FootnotesList> m_pEndnotesList;
    int m_footnoteEndnoteRefTag;
    std::unique_ptr< const WW8_SepInfo > m_pSectionInfo;

    /// Redline data to remember in the text run.
    const SwRedlineData *m_pRedlineData;

    /// Id of the redline
    sal_Int32 m_nRedlineId;

    /// Flag indicating that the section properties are being written
    bool m_bOpenedSectPr;
    /// Did we have a section break in this paragraph? Set by StartSection(), reset by the next StartParagraph().
    bool m_bHadSectPr;

    /// Flag indicating that the Run Text is being written
    bool m_bRunTextIsOn;

    /// Flag indicating that the header \ footer are being written
    bool m_bWritingHeaderFooter;
    bool m_bAnchorLinkedToNode;

    /// Field data to remember in the text run
    bool m_bPreventDoubleFieldsHandling;
    std::vector< FieldInfos > m_Fields;
    OUString m_sFieldBkm;
    sal_Int32 m_nNextBookmarkId;
    sal_Int32 m_nNextAnnotationMarkId;

    /// Bookmarks to output
    std::vector<OUString> m_rBookmarksStart;
    std::vector<OUString> m_rBookmarksEnd;

    /// Bookmarks to output at the end
    std::vector<OUString> m_rFinalBookmarksStart;
    std::vector<OUString> m_rFinalBookmarksEnd;

    /// Bookmarks of the current paragraph
    std::multimap<sal_Int32, OUString> m_aBookmarksOfParagraphStart;
    std::multimap<sal_Int32, OUString> m_aBookmarksOfParagraphEnd;

    /// Permissions to output
    std::vector<OUString> m_rPermissionsStart;
    std::vector<OUString> m_rPermissionsEnd;

    /// Annotation marks to output
    std::vector<OString> m_rAnnotationMarksStart;
    std::vector<OString> m_rAnnotationMarksEnd;

    /// Maps of the bookmarks ids
    std::map<OUString, sal_Int32> m_rOpenedBookmarksIds;

    /// Name of the last opened bookmark.
    OString m_sLastOpenedBookmark;

    /// Maps of the annotation marks ids
    std::map<OString, sal_Int32> m_rOpenedAnnotationMarksIds;

    /// Name of the last opened annotation mark.
    OString m_sLastOpenedAnnotationMark;

    /// If there are bookmarks around sequence fields, this map contains the
    /// names of these bookmarks for each sequence.
    std::map<OUString, std::vector<OString> > m_aSeqBookmarksNames;

    /// GrabBag for text effects like glow, shadow, ...
    std::vector<css::beans::PropertyValue> m_aTextEffectsGrabBag;

    /// The current table helper
    std::unique_ptr<SwWriteTable> m_xTableWrt;

    ww8::Frame* m_pCurrentFrame;

    bool m_bParagraphOpened;
    bool m_bParagraphFrameOpen;
    bool m_bIsFirstParagraph;
    bool m_bAlternateContentChoiceOpen;
    bool m_bPostponedProcessingFly;

    // Remember that a column break has to be opened at the
    // beginning of the next paragraph
    DocxColBreakStatus m_nColBreakStatus;

    // Remember that a page break has to be opened at the
    // beginning of the next paragraph
    bool m_bPostponedPageBreak;

    std::vector<ww8::Frame> m_aFramesOfParagraph;
    sal_Int32 m_nTextFrameLevel;

    // close of hyperlink needed
    bool m_closeHyperlinkInThisRun;
    bool m_closeHyperlinkInPreviousRun;
    bool m_startedHyperlink;
    // Count nested HyperLinks
    sal_Int32 m_nHyperLinkCount;
    sal_Int16 m_nFieldsInHyperlink;

    // If the exported numbering rule defines the outlines
    bool m_bExportingOutline;

    struct PostponedGraphic
    {
        PostponedGraphic( const SwGrfNode* n, Size s, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* sObj )
            : grfNode( n ), size( s ), mOLEFrameFormat( pOLEFrameFormat ), mOLENode( pOLENode ), pSdrObj(sObj) {};

        const SwGrfNode* grfNode;
        Size size;
        const SwFlyFrameFormat* mOLEFrameFormat;
        SwOLENode* mOLENode;
        const SdrObject* pSdrObj;
    };
    std::unique_ptr< std::vector<PostponedGraphic> > m_pPostponedGraphic;
    struct PostponedDiagram
    {
        PostponedDiagram( const SdrObject* o, const SwFrameFormat* frm ) : object( o ), frame( frm ) {};
        const SdrObject* object;
        const SwFrameFormat* frame;
    };
    std::unique_ptr< std::vector<PostponedDiagram> > m_pPostponedDiagrams;

    struct PostponedDrawing
    {
        PostponedDrawing( const SdrObject* sdrObj, const SwFrameFormat* frm) : object( sdrObj ), frame( frm ) {};
        const SdrObject* object;
        const SwFrameFormat* frame;
    };
    std::unique_ptr< std::vector<PostponedDrawing> > m_pPostponedDMLDrawings;
    std::unique_ptr< std::vector<PostponedDrawing> > m_pPostponedCustomShape;

    struct PostponedOLE
    {
        PostponedOLE( SwOLENode* rObject, const Size& rSize, const SwFlyFrameFormat* rFrame ) : object( rObject ), size( rSize ), frame( rFrame ) {};
        SwOLENode* object;
        const Size size;
        const SwFlyFrameFormat* frame;
    };
    std::unique_ptr< std::vector<PostponedOLE> > m_pPostponedOLEs;

    std::vector<const SwOLENode*> m_aPostponedMaths;
    /// count charts consistently for unit tests
    unsigned int m_nChartCount;
    const SdrObject* m_postponedChart;
    Size m_postponedChartSize;
    std::vector<const SdrObject*> m_aPostponedFormControls;
    std::vector<PostponedDrawing> m_aPostponedActiveXControls;
    const SwField* pendingPlaceholder;
    /// Maps postit fields to ID's, used in commentRangeStart/End, commentReference and comment.xml.
    std::vector< std::pair<const SwPostItField*, sal_Int32> > m_postitFields;
    /// Number of postit fields which already have a commentReference written.
    unsigned int m_postitFieldsMaxId;
    int m_anchorId;
    int m_nextFontId;
    struct EmbeddedFontRef
    {
        OString relId;
        OString fontKey;
    };

    std::unique_ptr<TableReference> m_tableReference;

    std::map< OUString, EmbeddedFontRef > fontFilesMap; // font file url to data

    // Remember first cell (used for default borders/margins) of each table
    std::vector<ww8::WW8TableNodeInfoInner::Pointer_t> tableFirstCells;
    // Remember last open and closed cells on each level
    std::vector<sal_Int32> lastOpenCell;
    std::vector<sal_Int32> lastClosedCell;

    boost::optional<css::drawing::FillStyle> m_oFillStyle;
    /// If FormatBox() already handled fill style / gradient.
    bool m_bIgnoreNextFill;

    /// Is fake rotation detected, so rotation with 90 degrees should be ignored in this cell?
    bool m_bBtLr;

    editeng::WordPageMargins m_pageMargins;

    std::shared_ptr<DocxTableStyleExport> m_pTableStyleExport;
    // flag to check if auto spacing was set in original file
    bool m_bParaBeforeAutoSpacing,m_bParaAfterAutoSpacing;
    // store hardcoded value which was set during import.
    sal_Int32 m_nParaBeforeSpacing,m_nParaAfterSpacing;

    bool m_setFootnote;

    /// RelId <-> Graphic* cache, so that in case of alternate content, the same graphic only gets written once.
    std::stack< std::map<const Graphic*, OString> > m_aRelIdCache;
    /// RelId <-> BitmapChecksum cache, similar to m_aRelIdCache, but used for non-Writer graphics, handled in oox.
    std::stack< std::map<BitmapChecksum, OUString> > m_aSdrRelIdCache;

    /// members to control the existence of grabbagged SDT properties in the paragraph
    sal_Int32 m_nParagraphSdtPrToken;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pParagraphSdtPrTokenChildren;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pParagraphSdtPrTokenAttributes;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pParagraphSdtPrDataBindingAttrs;
    /// members to control the existence of grabbagged SDT properties in the text run
    sal_Int32 m_nRunSdtPrToken;
    /// State of the Fly at current position
    FlyProcessingState m_nStateOfFlyFrame;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pRunSdtPrTokenChildren;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pRunSdtPrDataBindingAttrs;
    /// Value of the <w:alias> paragraph SDT element.
    OUString m_aParagraphSdtPrAlias;
    /// Same as m_aParagraphSdtPrAlias, but its content is available till the SDT is closed.
    OUString m_aStartedParagraphSdtPrAlias;
    OUString m_aRunSdtPrAlias;
    /// Currently paragraph SDT has a <w:id> child element.
    bool m_bParagraphSdtHasId;

    std::map<SvxBoxItemLine, css::table::BorderLine2> m_aTableStyleConf;

public:
    DocxAttributeOutput( DocxExport &rExport, const ::sax_fastparser::FSHelperPtr& pSerializer, oox::drawingml::DrawingML* pDrawingML );

    virtual ~DocxAttributeOutput() override;

    /// Return the right export class.
    virtual DocxExport& GetExport() override;
    const DocxExport& GetExport() const { return const_cast< DocxAttributeOutput* >( this )->GetExport(); }

    /// For e.g. the output of the styles, we need to switch the serializer to another one.
    void SetSerializer( ::sax_fastparser::FSHelperPtr const & pSerializer );

    /// Occasionally need to use this serializer from the outside
    const ::sax_fastparser::FSHelperPtr& GetSerializer( ) { return m_pSerializer; }

    /// Do we have any footnotes?
    bool HasFootnotes() const;

    /// Do we have any endnotes?
    bool HasEndnotes() const;

    /// Output the content of the footnotes.xml resp. endnotes.xml
    void FootnotesEndnotes( bool bFootnotes );

    /// writes the footnotePr/endnotePr (depending on tag) section
    static void WriteFootnoteEndnotePr( ::sax_fastparser::FSHelperPtr const & fs, int tag, const SwEndNoteInfo& info, int listtag );

    bool HasPostitFields() const;
    void WritePostitFields();

    /// VMLTextExport
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) override;
    virtual void WriteVMLTextBox(css::uno::Reference<css::drawing::XShape> xShape) override;
    /// DMLTextExport
    virtual void WriteTextBox(css::uno::Reference<css::drawing::XShape> xShape) override;
    virtual OUString FindRelId(BitmapChecksum nChecksum) override;
    virtual void CacheRelId(BitmapChecksum nChecksum, const OUString& rRelId) override;
    virtual oox::drawingml::DrawingML& GetDrawingML() override;

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) override;

    void SetWritingHeaderFooter( bool bWritingHeaderFooter )    {   m_bWritingHeaderFooter = bWritingHeaderFooter;   }
    bool GetWritingHeaderFooter( )  {   return m_bWritingHeaderFooter;  }
    void SetAlternateContentChoiceOpen( bool bAltContentChoiceOpen ) { m_bAlternateContentChoiceOpen = bAltContentChoiceOpen; }
    bool IsAlternateContentChoiceOpen( ) { return m_bAlternateContentChoiceOpen; }
    void GetSdtEndBefore(const SdrObject* pSdrObj);
    void SetStartedParaSdt(bool bStartedParaSdt);
    bool IsStartedParaSdt();
    bool IsFirstParagraph() { return m_bIsFirstParagraph; }

    /// Stores the table export state to the passed context and resets own state.
    void pushToTableExportContext(DocxTableExportContext& rContext);
    /// Restores from the remembered state.
    void popFromTableExportContext(DocxTableExportContext const & rContext);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
