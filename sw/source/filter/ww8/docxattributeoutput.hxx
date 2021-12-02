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
#include <string_view>

#include "attributeoutputbase.hxx"
#include "fields.hxx"
#include <IMark.hxx>
#include "docxexport.hxx"
#include <wrtswtbl.hxx>
#include <redline.hxx>

#include <editeng/boxitem.hxx>
#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <vcl/vclenum.hxx>
#include <svx/xenum.hxx>

#include <fldbas.hxx>

#include <vector>
#include <optional>
#include <o3tl/sorted_vector.hxx>
#include <oox/export/vmlexport.hxx>
#include <oox/export/drawingml.hxx>
#include "docxtablestyleexport.hxx"

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

class SwGrfNode;
class SdrObject;
enum class SvxBoxItemLine;

namespace docx { class FootnotesList; }
namespace oox::drawingml { class DrawingML; }

struct FieldInfos
{
    std::shared_ptr<const SwField> pField;
    const ::sw::mark::IFieldmark* pFieldmark;
    ww::eField  eType;
    bool        bOpen;
    bool        bSep;
    bool        bClose;
    OUString    sCmd;
    FieldInfos()
        : pFieldmark(nullptr), eType(ww::eUNKNOWN)
        , bOpen(false), bSep(false), bClose(false)
    {}
};

enum DocxColBreakStatus
{
    COLBRK_NONE,
    COLBRK_POSTPONE,
    COLBRK_WRITEANDPOSTPONE,
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
    bool                bWriteDistance = false;
    SvxShadowLocation   aShadowLocation = SvxShadowLocation::NONE;
    std::shared_ptr<editeng::WordBorderDistances> pDistances;
};

struct DocxTableExportContext;

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

class SdtBlockHelper
{
public:
    SdtBlockHelper()
        : m_bHasId(false)
        , m_bStartedSdt(false)
        , m_nSdtPrToken(0)
    {}

    bool m_bHasId;
    bool m_bStartedSdt;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pTokenChildren;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pTokenAttributes;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pTextAttrs;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pDataBindingAttrs;
    OUString m_aColor;
    OUString m_aPlaceHolderDocPart;
    OUString m_aAlias;
    sal_Int32 m_nSdtPrToken;

    void DeleteAndResetTheLists();

    void WriteSdtBlock(::sax_fastparser::FSHelperPtr& pSerializer, bool bRunTextIsOn, bool bParagraphHasDrawing);
    void WriteExtraParams(::sax_fastparser::FSHelperPtr& pSerializer);

    /// Closes a currently open SDT block.
    void EndSdtBlock(::sax_fastparser::FSHelperPtr& pSerializer);

    void GetSdtParamsFromGrabBag(const uno::Sequence<beans::PropertyValue>& aGrabBagSdt);
};

/// The class that has handlers for various resource types when exporting as DOCX.
class DocxAttributeOutput : public AttributeOutputBase, public oox::vml::VMLTextExport, public oox::drawingml::DMLTextExport
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) override;

    /// Start of the paragraph.
    virtual sal_Int32 StartParagraph(ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo,
                                     bool bGenerateParaId) override;

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

    virtual void FieldVanish(const OUString& rText, ww::eField eType, OUString const*) override;

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
            sal_uInt16 nBase, sal_uInt16 nNext, sal_uInt16 nLink, sal_uInt16 nWwId, sal_uInt16 nId,
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
    /// bBreakAfter: the break must be scheduled for insertion in the end of current paragraph
    virtual void SectionBreak( sal_uInt8 nC, bool bBreakAfter, const WW8_SepInfo* pSectionInfo = nullptr, bool bExtraPageBreak = false ) override;

    // preserve DOCX page vertical alignment
    virtual void TextVerticalAdjustment( const css::drawing::TextVerticalAdjust ) override;

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
    virtual void SectionPageNumbering( sal_uInt16 nNumType, const ::std::optional<sal_uInt16>& oPageRestartNumber ) override;

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
    void EmbedFont( std::u16string_view name, FontFamily family, FontPitch pitch );

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule ) override;

    /// Numbering definition that overrides abstract numbering definition
    virtual void OverrideNumberingDefinition( SwNumRule const& rRule,
            sal_uInt16 nNum, sal_uInt16 nAbstractNum,
            const std::map< size_t, size_t > & rLevelOverrides ) override;

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

    void WriteField_Impl(const SwField* pField, ww::eField eType,
            const OUString& rFieldCmd, FieldFlags nMode,
            OUString const* pBookmarkName = nullptr);
    void WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark );

    void WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds, const SwRedlineData* pRedlineData = nullptr );
    void WriteFinalBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void WriteAnnotationMarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void PushRelIdCache();
    /// End possibly opened paragraph sdt block.
    void EndParaSdtBlock();

    void WriteFloatingTable(ww8::Frame const* pParentFrame);

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
    /// @see InitCollectedRunproperties(), WriteCollectedParagraphProperties()
    void WriteCollectedRunProperties();

    /// Output graphic fly frames or replacement graphics for OLE nodes.
    ///
    /// For graphic frames, just use the first two parameters, for OLE
    /// replacement graphics, set the first as 0, and pass the remaining three.
    ///
    /// @see WriteOLE2Obj()
    void FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrameFormat* pOLEFrameFormat, SwOLENode* pOLENode, const SdrObject* pSdrObj = nullptr);
    void WriteSrcRect(const css::uno::Reference<css::beans::XPropertySet>& xShapePropSet,
                      const SwFrameFormat* pFrameFormat);
    void WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat, const sal_Int8 nFormulaAlignment);
    bool WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat);
    bool WriteOLEMath( const SwOLENode& rNode, const sal_Int8 nAlign );
    void PostponeOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* pFlyFrameFormat );
    void WriteOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrameFormat* rFlyFrameFormat );
    void WriteOLEShape(const SwFlyFrameFormat& rFrameFormat, const Size& rSize,
                       const OString& rShapeId, const OUString& rImageId);
    static OString GetOLEStyle(const SwFlyFrameFormat& rFormat, const Size& rSize);
    void ExportOLESurround(const SwFormatSurround& rWrap);

    void WriteActiveXControl(const SdrObject* pObject, const SwFrameFormat& rFrameFormat, bool bInsideRun);
    bool ExportAsActiveXControl(const SdrObject* pObject) const;

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

    void EmbedFontStyle( std::u16string_view name, int tag, FontFamily family, FontItalic italic, FontWeight weight,
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

    void SectionRtlGutter( const SfxBoolItem& rRtlGutter) override;

    /// Reference to the export, where to get the data from
    DocxExport &m_rExport;

    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

    /// DrawingML access
    oox::drawingml::DrawingML &m_rDrawingML;

private:

    void DoWriteBookmarkTagStart(const OUString & bookmarkName);
    void DoWriteBookmarkTagEnd(sal_Int32 nId);
    void DoWriteMoveRangeTagStart(const OString & bookmarkName,
            bool bFrom, const SwRedlineData* pRedlineData);
    void DoWriteMoveRangeTagEnd(sal_Int32 nId, bool bFrom);
    void DoWriteBookmarksStart(std::vector<OUString>& rStarts, const SwRedlineData* pRedlineData = nullptr);
    void DoWriteBookmarksEnd(std::vector<OUString>& rEnds);
    void DoWriteBookmarkStartIfExist(sal_Int32 nRunPos);
    void DoWriteBookmarkEndIfExist(sal_Int32 nRunPos);

    void DoWritePermissionTagStart(std::u16string_view permission);
    void DoWritePermissionTagEnd(std::u16string_view permission);
    void DoWritePermissionsStart();
    void DoWritePermissionsEnd();

    void DoWriteAnnotationMarks( );
    void WritePostponedGraphic();
    void WritePostponedMath(const SwOLENode* pObject, sal_Int8 /*nAlign*/);
    void WritePostponedFormControl(const SdrObject* pObject);
    void WritePostponedActiveXControl(bool bInsideRun);
    void WritePostponedDiagram();
    void WritePostponedChart();
    void WritePostponedOLE();
    void WritePostponedDMLDrawing();
    void WritePostponedCustomShape();
    void WriteFlyFrame(const ww8::Frame& rFrame);

    void WriteFormDateStart(const OUString& sFullDate, const OUString& sDateFormat, const OUString& sLang, const uno::Sequence<beans::PropertyValue>& aGrabBagSdt);
    void WriteSdtDropDownStart(std::u16string_view rName, OUString const& rSelected, uno::Sequence<OUString> const& rListItems);
    void WriteSdtDropDownEnd(OUString const& rSelected, uno::Sequence<OUString> const& rListItems);
    void WriteSdtEnd();

    void StartField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun = false );
    void DoWriteCmd( const OUString& rCmd );
    void CmdField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos const & rInfos, bool bWriteRun );
    void CmdEndField_Impl( const SwTextNode* pNode, sal_Int32 nPos, bool bWriteRun );
    void EndField_Impl( const SwTextNode* pNode, sal_Int32 nPos, FieldInfos& rInfos );
    void DoWriteFieldRunProperties( const SwTextNode* pNode, sal_Int32 nPos, bool bWriteCombChars = false );

    rtl::Reference<sax_fastparser::FastAttributeList> m_pFontsAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pEastAsianLayoutAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pCharLangAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pSectionSpacingAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pParagraphSpacingAttrList;
    rtl::Reference<sax_fastparser::FastAttributeList> m_pHyperlinkAttrList;
    /// If the current SDT around runs should be ended before the current run.
    bool m_bEndCharSdt;
    /// Attributes of the run color
    rtl::Reference<sax_fastparser::FastAttributeList> m_pColorAttrList;
    sal_uInt8 m_nCharTransparence = 0;
    /// Attributes of the paragraph background
    rtl::Reference<sax_fastparser::FastAttributeList> m_pBackgroundAttrList;
    OUString m_sOriginalBackgroundColor;
    OUString m_hyperLinkAnchor;
    bool m_endPageRef;
    std::unique_ptr<docx::FootnotesList> m_pFootnotesList;
    std::unique_ptr<docx::FootnotesList> m_pEndnotesList;
    int m_footnoteEndnoteRefTag;
    OUString m_footnoteCustomLabel;
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

    /// Flag indicating that multiple runs of a field are being written
    bool m_bWritingField;

    /// Field data to remember in the text run
    bool m_bPreventDoubleFieldsHandling;
    std::vector< FieldInfos > m_Fields;
    OUString m_sFieldBkm;
    sal_Int32 m_nNextBookmarkId;
    sal_Int32 m_nNextAnnotationMarkId;

    /// [MS-DOCX] section 2.6.2.3
    sal_Int32 m_nNextParaId = 1; // MUST be greater than 0

    OUString m_sRawText;

    /// The first frame (anchored to the main text) is 0.
    /// The second frame what is anchored to the previous in, is 1
    /// The third anchored inside the second is the 2 etc.
    sal_uInt32 m_nEmbedFlyLevel;

    /// Stores the flys what are anchored inside a fly
    std::vector<ww8::Frame> m_vPostponedFlys;

    /// Bookmarks to output
    std::vector<OUString> m_rBookmarksStart;
    std::vector<OUString> m_rBookmarksEnd;
    SwRedlineData* m_pMoveRedlineData;

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

    // This paragraph must end with page break
    bool m_bPageBreakAfter = false;

    std::stack< std::vector<ww8::Frame> > m_aFramesOfParagraph;
    o3tl::sorted_vector<const SwFrameFormat*> m_aFloatingTablesOfParagraph;
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
        PostponedGraphic( const SwGrfNode* n, Size s, const SdrObject* sObj )
            : grfNode( n ), size( s ), pSdrObj(sObj) {};

        const SwGrfNode* grfNode;
        Size size;
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

    struct PostponedMathObjects
    {
        SwOLENode* pMathObject;
        sal_Int8 nMathObjAlignment;
    };
    std::vector<PostponedMathObjects> m_aPostponedMaths;
    /// count charts consistently for unit tests
    unsigned int m_nChartCount;
    struct PostponedChart
    {
        PostponedChart( const SdrObject* sdrObject, const Size& rSize, const SwFlyFrameFormat* rFrame ) : object(sdrObject), size(rSize), frame(rFrame) {};
        const SdrObject* object;
        const Size size;
        const SwFlyFrameFormat* frame;
    };
    std::vector<PostponedChart> m_aPostponedCharts;
    std::vector<const SdrObject*> m_aPostponedFormControls;
    std::vector<PostponedDrawing> m_aPostponedActiveXControls;
    const SwField* pendingPlaceholder;

    struct PostItDOCXData{
        sal_Int32 id;
        sal_Int32 lastParaId = 0; // [MS-DOCX] 2.5.3.1 CT_CommentEx needs paraId attribute
    };
    /// Maps postit fields to ID's, used in commentRangeStart/End, commentReference and comment.xml.
    std::vector<std::pair<const SwPostItField*, PostItDOCXData>> m_postitFields;
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

    std::optional<css::drawing::FillStyle> m_oFillStyle;
    /// If FormatBox() already handled fill style / gradient.
    bool m_bIgnoreNextFill;

    editeng::WordPageMargins m_pageMargins;

    std::shared_ptr<DocxTableStyleExport> m_pTableStyleExport;
    // flag to check if auto spacing was set in original file
    bool m_bParaBeforeAutoSpacing,m_bParaAfterAutoSpacing;
    // store hardcoded value which was set during import.
    sal_Int32 m_nParaBeforeSpacing,m_nParaAfterSpacing;

    std::pair<OString, OUString> getExistingGraphicRelId(BitmapChecksum aChecksum);
    void cacheGraphicRelId(BitmapChecksum nChecksum, OString const & rRelId, OUString const & rFileName);

    /// RelId <-> Graphic* cache, so that in case of alternate content, the same graphic only gets written once.
    std::stack< std::map<BitmapChecksum, std::pair<OString, OUString>> > m_aRelIdCache;

    /// RelId <-> BitmapChecksum cache, similar to m_aRelIdCache, but used for non-Writer graphics, handled in oox.
    std::stack< std::map<BitmapChecksum, std::pair<OUString, OUString>> > m_aSdrRelIdCache;

    SdtBlockHelper m_aParagraphSdt;
    SdtBlockHelper m_aRunSdt;

    /// State of the Fly at current position
    FlyProcessingState m_nStateOfFlyFrame;

    /// Same as m_aParagraphSdtPrAlias, but its content is available till the SDT is closed.
    OUString m_aStartedParagraphSdtPrAlias;

    std::vector<std::map<SvxBoxItemLine, css::table::BorderLine2>> m_aTableStyleConfs;

public:
    DocxAttributeOutput( DocxExport &rExport, const ::sax_fastparser::FSHelperPtr& pSerializer, oox::drawingml::DrawingML* pDrawingML );

    virtual ~DocxAttributeOutput() override;

    /// Return the right export class.
    virtual DocxExport& GetExport() override;
    const DocxExport& GetExport() const { return const_cast< DocxAttributeOutput* >( this )->GetExport(); }

    /// For e.g. the output of the styles, we need to switch the serializer to another one.
    void SetSerializer( ::sax_fastparser::FSHelperPtr const & pSerializer );

    /// Occasionally need to use this serializer from the outside
    const ::sax_fastparser::FSHelperPtr& GetSerializer( ) const { return m_pSerializer; }

    /// Do we have any footnotes?
    bool HasFootnotes() const;

    /// Do we have any endnotes?
    bool HasEndnotes() const;

    /// Output the content of the footnotes.xml resp. endnotes.xml
    void FootnotesEndnotes( bool bFootnotes );

    /// writes the footnotePr/endnotePr (depending on tag) section
    static void WriteFootnoteEndnotePr( ::sax_fastparser::FSHelperPtr const & fs, int tag, const SwEndNoteInfo& info, int listtag );

    bool HasPostitFields() const;
    enum class hasResolved { no, yes };
    hasResolved WritePostitFields();
    void WritePostItFieldsResolved();

    /// VMLTextExport
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) override;
    virtual void WriteVMLTextBox(css::uno::Reference<css::drawing::XShape> xShape) override;
    /// DMLTextExport
    virtual void WriteTextBox(css::uno::Reference<css::drawing::XShape> xShape) override;
    virtual OUString FindRelId(BitmapChecksum nChecksum) override;
    virtual OUString FindFileName(BitmapChecksum nChecksum) override;
    virtual void CacheRelId(BitmapChecksum nChecksum, const OUString& rRelId, const OUString& rFileName) override;
    virtual css::uno::Reference<css::text::XTextFrame> GetUnoTextFrame(
        css::uno::Reference<css::drawing::XShape> xShape) override;
    virtual oox::drawingml::DrawingML& GetDrawingML() override;
    virtual bool MaybeOutputBrushItem(SfxItemSet const&) override;

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) override;

    void SetWritingHeaderFooter( bool bWritingHeaderFooter )    {   m_bWritingHeaderFooter = bWritingHeaderFooter;   }
    bool GetWritingHeaderFooter( ) const  {   return m_bWritingHeaderFooter;  }
    void SetAlternateContentChoiceOpen( bool bAltContentChoiceOpen ) { m_bAlternateContentChoiceOpen = bAltContentChoiceOpen; }
    bool IsAlternateContentChoiceOpen( ) const { return m_bAlternateContentChoiceOpen; }
    void GetSdtEndBefore(const SdrObject* pSdrObj);
    bool IsFirstParagraph() const { return m_bIsFirstParagraph; }

    /// Stores the table export state to the passed context and resets own state.
    void pushToTableExportContext(DocxTableExportContext& rContext);
    /// Restores from the remembered state.
    void popFromTableExportContext(DocxTableExportContext const & rContext);
};

/**
* All the information that should be stashed away when we're in the middle of
* of a table export and still have to do something else, e.g. export a shape.
*/
struct DocxTableExportContext
{
    DocxAttributeOutput& m_rOutput;
    ww8::WW8TableInfo::Pointer_t m_pTableInfo;
    bool m_bTableCellOpen;
    bool m_bStartedParaSdt;
    sal_uInt32 m_nTableDepth;
    DocxTableExportContext(DocxAttributeOutput& rOutput) : m_rOutput(rOutput) { m_rOutput.pushToTableExportContext(*this); }
    ~DocxTableExportContext() { m_rOutput.popFromTableExportContext(*this); }
};

namespace docx {

rtl::Reference<sax_fastparser::FastAttributeList> SurroundToVMLWrap(SwFormatSurround const& rSurround);

}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
