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

#include "attributeoutputbase.hxx"
#include "fields.hxx"
#include "IMark.hxx"
#include "docxexport.hxx"

#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <vcl/vclenum.hxx>
#include <svx/xenum.hxx>

#include <fldbas.hxx>

#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/optional.hpp>
#include <oox/export/vmlexport.hxx>
#include <oox/export/drawingml.hxx>
#include <docxtablestyleexport.hxx>

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>

class SwGrfNode;
class SdrObject;

namespace docx { class FootnotesList; }
namespace oox { namespace drawingml { class DrawingML; } }

struct FieldInfos
{
    const SwField*    pField;
    const ::sw::mark::IFieldmark* pFieldmark;
    ww::eField  eType;
    bool        bOpen;
    bool        bClose;
    OUString    sCmd;
    FieldInfos() : pField(NULL), pFieldmark(NULL), eType(ww::eUNKNOWN), bOpen(false), bClose(false){}
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
    sal_Int32           tag;
    bool                bUseStartEnd;
    bool                bWriteTag;
    bool                bWriteInsideHV;
    bool                bWriteDistance;
    SvxShadowLocation   aShadowLocation;
    bool                bCheckDistanceSize;

    OutputBorderOptions() : tag(0), bUseStartEnd(false), bWriteTag(true), bWriteInsideHV(false), bWriteDistance(false), aShadowLocation(SVX_SHADOW_NONE), bCheckDistanceSize(false) {}
};

/**
 * A structure that holds information about the page margins.
 *
 */
struct PageMargins
{
    sal_uInt16 nPageMarginLeft;
    sal_uInt16 nPageMarginRight;
    sal_uInt16 nPageMarginTop;
    sal_uInt16 nPageMarginBottom;

    PageMargins() : nPageMarginLeft(0), nPageMarginRight(0), nPageMarginTop(0), nPageMarginBottom(0) {}
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

    ww8::WW8TableNodeInfoInner::Pointer_t m_pOldTablepInner;

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
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript ) SAL_OVERRIDE;

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo ) SAL_OVERRIDE;

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) SAL_OVERRIDE;

    /// Empty paragraph.
    virtual void EmptyParagraph() SAL_OVERRIDE;

    /// Called in order to output section breaks.
    virtual void SectionBreaks(const SwTxtNode& rNode) SAL_OVERRIDE;

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties() SAL_OVERRIDE;

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties(const SfxItemSet& rParagraphMarkerProperties, const SwRedlineData* pRedlineData, const SwRedlineData* pRedlineParagraphMarkerDeleted, const SwRedlineData* pRedlineParagraphMarkerInserted) SAL_OVERRIDE;

    /// Start of the text run.
    virtual void StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false ) SAL_OVERRIDE;

    /// End of the text run.
    virtual void EndRun() SAL_OVERRIDE;

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties() SAL_OVERRIDE;

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData ) SAL_OVERRIDE;

    virtual void FootnoteEndnoteRefTag() SAL_OVERRIDE;

    virtual void SectFootnoteEndnotePr() SAL_OVERRIDE;

    virtual void WritePostitFieldReference() SAL_OVERRIDE;

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 ) SAL_OVERRIDE;

    /// Output text (without markup).
    virtual void RawText( const OUString& rText, bool bForceUnicode, rtl_TextEncoding eCharSet ) SAL_OVERRIDE;

    /// Output ruby start.
    virtual void StartRuby( const SwTxtNode& rNode, sal_Int32 nPos, const SwFmtRuby& rRuby ) SAL_OVERRIDE;

    /// Output ruby end.
    virtual void EndRuby() SAL_OVERRIDE;

    /// Output URL start.
    virtual bool StartURL( const OUString& rUrl, const OUString& rTarget ) SAL_OVERRIDE;

    /// Output URL end.
    virtual bool EndURL(bool) SAL_OVERRIDE;

    virtual void FieldVanish( const OUString& rTxt, ww::eField eType ) SAL_OVERRIDE;

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline( const SwRedlineData* pRedline ) SAL_OVERRIDE;

    /// Output redlining.
    ///
    /// Start of the tag that encloses the run, fills the info according to
    /// the value of pRedlineData.
    void StartRedline( const SwRedlineData * pRedlineData );

    /// Output redlining.
    ///
    /// End of the tag that encloses the run.
    void EndRedline( const SwRedlineData * pRedlineData );

    virtual void SetStateOfFlyFrame( FlyProcessingState nStateOfFlyFrame ) SAL_OVERRIDE;
    virtual void SetAnchorIsLinkedToNode( bool bAnchorLinkedToNode = false ) SAL_OVERRIDE;
    virtual bool IsFlyProcessingPostponed() SAL_OVERRIDE;
    virtual void ResetFlyProcessingFlag() SAL_OVERRIDE;

    virtual void FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner ) SAL_OVERRIDE;

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle ) SAL_OVERRIDE;

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableRowRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableCellRedline( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner ) SAL_OVERRIDE;
    virtual void TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo ) SAL_OVERRIDE;
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
            bool bAutoUpdate ) SAL_OVERRIDE;

    /// End of a style in the styles table.
    virtual void EndStyle() SAL_OVERRIDE;

    /// Start of (paragraph or run) properties of a style.
    virtual void StartStyleProperties( bool bParProp, sal_uInt16 nStyle ) SAL_OVERRIDE;

    /// End of (paragraph or run) properties of a style.
    virtual void EndStyleProperties( bool bParProp ) SAL_OVERRIDE;

    /// Numbering rule and Id.
    virtual void OutlineNumbering( sal_uInt8 nLvl, const SwNumFmt &rNFmt, const SwFmt &rFmt ) SAL_OVERRIDE;

    /// Page break
    /// As a paragraph property - the paragraph should be on the next page.
    virtual void PageBreakBefore( bool bBreak ) SAL_OVERRIDE;

    /// Write a section break
    /// msword::ColumnBreak or msword::PageBreak
    virtual void SectionBreak( sal_uInt8 nC, const WW8_SepInfo* pSectionInfo = NULL ) SAL_OVERRIDE;

    /// Start of the section properties.
    virtual void StartSection() SAL_OVERRIDE;

    /// End of the section properties.
    virtual void EndSection() SAL_OVERRIDE;

    /// Protection of forms.
    virtual void SectionFormProtection( bool bProtected ) SAL_OVERRIDE;

    /// Numbering of the lines in the document.
    virtual void SectionLineNumbering( sal_uLong nRestartNo, const SwLineNumberInfo& rLnNumInfo ) SAL_OVERRIDE;

    /// Has different headers/footers for the title page.
    virtual void SectionTitlePage() SAL_OVERRIDE;

    /// Description of the page borders.
    virtual void SectionPageBorders( const SwFrmFmt* pFmt, const SwFrmFmt* pFirstPageFmt ) SAL_OVERRIDE;

    /// Columns populated from right/numbers on the right side?
    virtual void SectionBiDi( bool bBiDi ) SAL_OVERRIDE;

    /// The style of the page numbers.
    ///
    virtual void SectionPageNumbering( sal_uInt16 nNumType, ::boost::optional<sal_uInt16> oPageRestartNumber ) SAL_OVERRIDE;

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode ) SAL_OVERRIDE;

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
    void EmbedFont( const OUString& name, FontFamily family, FontPitch pitch, rtl_TextEncoding encoding );

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule ) SAL_OVERRIDE;

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
        const SvxBrushItem* pBrush = 0 ) SAL_OVERRIDE;

    void WriteField_Impl( const SwField* pFld, ww::eField eType, const OUString& rFldCmd, sal_uInt8 nMode );
    void WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark );

    void WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void WriteAnnotationMarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );
    void ClearRelIdCache();
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
    void FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrmFmt* pOLEFrmFmt = 0, SwOLENode* pOLENode = 0, const SdrObject* pSdrObj = 0);
    void WriteSrcRect( const SdrObject* pSdrObj );
    void WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rNode, const Size& rSize, const SwFlyFrmFmt* pFlyFrmFmt);
    bool WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize );
    bool WriteOLEMath( const SdrObject* pSdrObj, const SwOLENode& rNode, const Size& rSize );
    bool PostponeOLE( const SdrObject* pSdrObj, SwOLENode& rNode, const Size& rSize, const SwFlyFrmFmt* pFlyFrmFmt );
    void WriteOLE( SwOLENode& rNode, const Size& rSize, const SwFlyFrmFmt* rFlyFrmFmt );

    /// checks whether the current component is a diagram
    bool IsDiagram (const SdrObject* sdrObject);

    void InitTableHelper( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTable( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTableRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void StartTableCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void TableCellProperties( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    void EndTableCell( );
    void EndTableRow( );
    void EndTable();
    void PopulateFrameProperties(const SwFrmFmt* pFrmFmt, const Size& rSize);
    bool TextBoxIsFramePr(const SwFrmFmt& rFrmFmt);
    /// End cell, row, and even the entire table if necessary.
    void FinishTableRowCell( ww8::WW8TableNodeInfoInner::Pointer_t pInner, bool bForceEmptyParagraph = false );

    void WriteFFData( const FieldInfos& rInfos );
    void WritePendingPlaceholder();

    void EmbedFontStyle( const OUString& name, int tag, FontFamily family, FontItalic italic, FontWeight weight,
        FontPitch pitch, rtl_TextEncoding encoding );

    /**
     * Translate a color object to the corresponding HighlightColorValues enumaration item
     *
     * @param[in]   rColor      a color object to translate
     * @return      color name (e.g. "red"), if color is inside the enumeration's range
     *              empty string, otherwise
    **/
    OString TransHighlightColor( const Color& rColor );
protected:

    /// Output frames - the implementation.
    virtual void OutputFlyFrame_Impl( const sw::Frame& rFmt, const Point& rNdTopLeft ) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CASEMAP
    virtual void CharCaseMap( const SvxCaseMapItem& rCaseMap ) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_COLOR
    virtual void CharColor( const SvxColorItem& rColor) SAL_OVERRIDE;

    /// Sfx item Sfx item RES_CHRATR_CONTOUR
    virtual void CharContour( const SvxContourItem& rContour ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CROSSEDOUT
    virtual void CharCrossedOut( const SvxCrossedOutItem& rCrossedOut ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ESCAPEMENT
    virtual void CharEscapement( const SvxEscapementItem& rEscapement ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONT
    virtual void CharFont( const SvxFontItem& rFont ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_FONTSIZE
    virtual void CharFontSize( const SvxFontHeightItem& rFontSize ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_KERNING
    virtual void CharKerning( const SvxKerningItem& rKerning ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_LANGUAGE
    virtual void CharLanguage( const SvxLanguageItem& rLanguage ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_POSTURE
    virtual void CharPosture( const SvxPostureItem& rPosture ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SHADOWED
    virtual void CharShadow( const SvxShadowedItem& rShadow ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_UNDERLINE
    virtual void CharUnderline( const SvxUnderlineItem& rUnderline ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_WEIGHT
    virtual void CharWeight( const SvxWeightItem& rWeight ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_AUTOKERN
    virtual void CharAutoKern( const SvxAutoKernItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BLINK
    virtual void CharAnimatedText( const SvxBlinkItem& rBlink ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BACKGROUND
    virtual void CharBackground( const SvxBrushItem& rBrush ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONT
    virtual void CharFontCJK( const SvxFontItem& rFont ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_FONTSIZE
    virtual void CharFontSizeCJK( const SvxFontHeightItem& rFontSize ) SAL_OVERRIDE { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& rLanguageItem ) SAL_OVERRIDE { CharLanguage( rLanguageItem ); }

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& rPosture ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& rWeight ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& rFont ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& rFontSize ) SAL_OVERRIDE { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& rLanguageItem ) SAL_OVERRIDE { CharLanguage( rLanguageItem); }

    /// Sfx item RES_CHRATR_CTL_POSTURE
    virtual void CharPostureCTL( const SvxPostureItem& rWeight ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_CTL_WEIGHT
    virtual void CharWeightCTL( const SvxWeightItem& rWeight ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BidiRTL
    virtual void CharBidiRTL( const SfxPoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_IdctHint
    virtual void CharIdctHint( const SfxPoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_ROTATE
    virtual void CharRotate( const SvxCharRotateItem& rRotate ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_EMPHASIS_MARK
    virtual void CharEmphasisMark( const SvxEmphasisMarkItem& rEmphasisMark ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_TWO_LINES
    virtual void CharTwoLines( const SvxTwoLinesItem& rTwoLines ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_SCALEW
    virtual void CharScaleWidth( const SvxCharScaleWidthItem& rScaleWidth ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_RELIEF
    virtual void CharRelief( const SvxCharReliefItem& rRelief) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIDDEN
    virtual void CharHidden( const SvxCharHiddenItem& rHidden ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_BOX
    virtual void CharBorder( const ::editeng::SvxBorderLine* pAllBorder, const sal_uInt16 nDist, const bool bShadow ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_HIGHLIGHT
    virtual void CharHighlight( const SvxBrushItem& rHighlight ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFmtINetFmt& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFmtCharFmt& ) SAL_OVERRIDE;

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFmtFtn& ) SAL_OVERRIDE;

    /// Output the footnote/endnote reference (if there's one to output).
    void FootnoteEndnoteReference();

    /// Sfx item RES_PARATR_LINESPACING
    virtual void ParaLineSpacing_Impl( short nSpace, short nMulti ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_ADJUST
    virtual void ParaAdjust( const SvxAdjustItem& rAdjust ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SPLIT
    virtual void ParaSplit( const SvxFmtSplitItem& rSplit ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_WIDOWS
    virtual void ParaWidows( const SvxWidowsItem& rWidows ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_TABSTOP
    virtual void ParaTabStop( const SvxTabStopItem& rTabStop ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HYPHENZONE
    virtual void ParaHyphenZone( const SvxHyphenZoneItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_NUMRULE
    virtual void ParaNumRule_Impl( const SwTxtNode *pTxtNd, sal_Int32 nLvl, sal_Int32 nNumId ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SCRIPTSPACE
    virtual void ParaScriptSpace( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_HANGINGPUNCTUATION
    virtual void ParaHangingPunctuation( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_FORBIDDEN_RULES
    virtual void ParaForbiddenRules( const SfxBoolItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_VERTALIGN
    virtual void ParaVerticalAlign( const SvxParaVertAlignItem& rAlign ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_SNAPTOGRID
    virtual void ParaSnapToGrid( const SvxParaGridItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FRM_SIZE
    virtual void FormatFrameSize( const SwFmtFrmSize& ) SAL_OVERRIDE;

    /// Sfx item RES_PAPER_BIN
    virtual void FormatPaperBin( const SvxPaperBinItem& ) SAL_OVERRIDE;

    /// Sfx item RES_LR_SPACE
    virtual void FormatLRSpace( const SvxLRSpaceItem& rLRSpace ) SAL_OVERRIDE;

    /// Sfx item RES_UL_SPACE
    virtual void FormatULSpace( const SvxULSpaceItem& rULSpace ) SAL_OVERRIDE;

    /// Sfx item RES_SURROUND
    virtual void FormatSurround( const SwFmtSurround& ) SAL_OVERRIDE;

    /// Sfx item RES_VERT_ORIENT
    virtual void FormatVertOrientation( const SwFmtVertOrient& ) SAL_OVERRIDE;

    /// Sfx item RES_HORI_ORIENT
    virtual void FormatHorizOrientation( const SwFmtHoriOrient& ) SAL_OVERRIDE;

    /// Sfx item RES_ANCHOR
    virtual void FormatAnchor( const SwFmtAnchor& ) SAL_OVERRIDE;

    /// Sfx item RES_BACKGROUND
    virtual void FormatBackground( const SvxBrushItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FILL_STYLE
    virtual void FormatFillStyle( const XFillStyleItem& ) SAL_OVERRIDE;

    /// Sfx item RES_FILL_GRADIENT
    virtual void FormatFillGradient( const XFillGradientItem& ) SAL_OVERRIDE;

    /// Sfx item RES_BOX
    virtual void FormatBox( const SvxBoxItem& ) SAL_OVERRIDE;

    /// Sfx item RES_COL
    virtual void FormatColumns_Impl( sal_uInt16 nCols, const SwFmtCol & rCol, bool bEven, SwTwips nPageSize ) SAL_OVERRIDE;

    /// Sfx item RES_KEEP
    virtual void FormatKeep( const SvxFmtKeepItem& ) SAL_OVERRIDE;

    /// Sfx item RES_TEXTGRID
    virtual void FormatTextGrid( const SwTextGridItem& ) SAL_OVERRIDE;

    /// Sfx item RES_LINENUMBER
    virtual void FormatLineNumbering( const SwFmtLineNumber& ) SAL_OVERRIDE;

    /// Sfx item RES_FRAMEDIR
    virtual void FormatFrameDirection( const SvxFrameDirectionItem& ) SAL_OVERRIDE;

    /// Sfx item RES_PARATR_GRABBAG
    virtual void ParaGrabBag( const SfxGrabBagItem& ) SAL_OVERRIDE;

    /// Sfx item RES_CHRATR_GRABBAG
    virtual void CharGrabBag( const SfxGrabBagItem& ) SAL_OVERRIDE;

    // Sfx item RES_PARATR_OUTLINELEVEL
    virtual void ParaOutlineLevel( const SfxUInt16Item& ) SAL_OVERRIDE;

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pFld ) SAL_OVERRIDE;

    virtual void RefField( const SwField& rFld, const OUString& rRef ) SAL_OVERRIDE;
    virtual void HiddenField( const SwField& rFld ) SAL_OVERRIDE;
    virtual void SetField( const SwField& rFld, ww::eField eType, const OUString& rCmd ) SAL_OVERRIDE;
    virtual void PostitField( const SwField* pFld ) SAL_OVERRIDE;
    virtual bool DropdownField( const SwField* pFld ) SAL_OVERRIDE;
    virtual bool PlaceholderField( const SwField* pFld ) SAL_OVERRIDE;

    virtual bool AnalyzeURL( const OUString& rURL, const OUString& rTarget, OUString* pLinkURL, OUString* pMark ) SAL_OVERRIDE;

    /// Reference to the export, where to get the data from
    DocxExport &m_rExport;

    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

    /// DrawingML access
    oox::drawingml::DrawingML &m_rDrawingML;

private:

    void DoWriteBookmarks( );
    void DoWriteAnnotationMarks( );
    void WritePostponedGraphic();
    void WritePostponedMath();
    void WritePostponedFormControl(const SdrObject* pObject);
    void WritePostponedDiagram();
    void WritePostponedChart();
    void WritePostponedOLE();
    void WritePostponedVMLDrawing();
    void WritePostponedDMLDrawing();
    void WritePostponedCustomShape();

    void WriteSdtBlock(sal_Int32& nSdtPrToken,
                       ::sax_fastparser::FastAttributeList*& pSdtPrTokenChildren,
                       ::sax_fastparser::FastAttributeList*& pSdtPrTokenAttributes,
                       ::sax_fastparser::FastAttributeList*& pSdtPrDataBindingAttrs,
                       OUString& rSdtPrAlias,
                       bool bPara);
    /// Closes a currently open SDT block.
    void EndSdtBlock();

    void StartField_Impl( FieldInfos& rInfos, bool bWriteRun = false );
    void DoWriteCmd( const OUString& rCmd );
    void CmdField_Impl( FieldInfos& rInfos );
    void EndField_Impl( FieldInfos& rInfos );

    void AddToAttrList( ::sax_fastparser::FastAttributeList* &pAttrList, sal_Int32 nAttrName, const sal_Char* sAttrValue );
    void AddToAttrList( ::sax_fastparser::FastAttributeList* &pAttrList, sal_Int32 nArgs, ... );

    ::sax_fastparser::FastAttributeList *m_pFontsAttrList, *m_pEastAsianLayoutAttrList;
    ::sax_fastparser::FastAttributeList *m_pCharLangAttrList;
    ::sax_fastparser::FastAttributeList *m_pSectionSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pParagraphSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pHyperlinkAttrList;
    /// If the current SDT around runs should be ended before the current run.
    bool m_bEndCharSdt;
    /// If an SDT around runs is currently open.
    bool m_bStartedCharSdt;
    /// If an SDT around paragraphs is currently open.
    bool m_bStartedParaSdt;
    /// Attributes of the run color
    ::sax_fastparser::FastAttributeList *m_pColorAttrList;
    /// Attributes of the paragraph background
    ::sax_fastparser::FastAttributeList *m_pBackgroundAttrList;
    OUString m_sOriginalBackgroundColor;
    OUString m_hyperLinkAnchor;
    bool m_endPageRef;
    ::docx::FootnotesList *m_pFootnotesList;
    ::docx::FootnotesList *m_pEndnotesList;
    int m_footnoteEndnoteRefTag;
    boost::scoped_ptr< const WW8_SepInfo > m_pSectionInfo;

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
    std::vector< FieldInfos > m_Fields;
    OUString m_sFieldBkm;
    sal_Int32 m_nNextBookmarkId;
    sal_Int32 m_nNextAnnotationMarkId;

    /// Bookmarks to output
    std::vector<OString> m_rBookmarksStart;
    std::vector<OString> m_rBookmarksEnd;

    /// Annotation marks to output
    std::vector<OString> m_rAnnotationMarksStart;
    std::vector<OString> m_rAnnotationMarksEnd;

    /// Maps of the bookmarks ids
    std::map<OString, sal_uInt16> m_rOpenedBookmarksIds;

    /// Name of the last opened bookmark.
    OString m_sLastOpenedBookmark;

    /// Maps of the annotation marks ids
    std::map<OString, sal_uInt16> m_rOpenedAnnotationMarksIds;

    /// Name of the last opened annotation mark.
    OString m_sLastOpenedAnnotationMark;

    /// If there are bookmarks around sequence fields, this map contains the
    /// names of these bookmarks for each sequence.
    std::map<OUString, std::vector<OString> > m_aSeqBookmarksNames;

    /// GrabBag for text effexts like glow, shadow, ...
    std::vector<css::beans::PropertyValue> m_aTextEffectsGrabBag;

    /// The current table helper
    SwWriteTable *m_pTableWrt;

    sw::Frame* m_pCurrentFrame;

    bool m_bParagraphOpened;
    bool m_bParagraphFrameOpen;
    bool m_bIsFirstParagraph;
    bool m_bAlternateContentChoiceOpen;
    bool m_bPostponedProcessingFly;

    // Remember that a column break has to be opened at the
    // beginning of the next paragraph
    DocxColBreakStatus m_nColBreakStatus;

    std::vector<sw::Frame> m_aFramesOfParagraph;
    sal_Int32 m_nTextFrameLevel;

    // close of hyperlink needed
    bool m_closeHyperlinkInThisRun;
    bool m_closeHyperlinkInPreviousRun;
    bool m_startedHyperlink;
    // Count nested HyperLinks
    sal_Int32 m_nHyperLinkCount;
    sal_Int16 m_nFieldsInHyperlink;

    struct PostponedGraphic
    {
        PostponedGraphic( const SwGrfNode* n, Size s, const SwFlyFrmFmt* pOLEFrmFmt, SwOLENode* pOLENode, const SdrObject* sObj )
            : grfNode( n ), size( s ), mOLEFrmFmt( pOLEFrmFmt ), mOLENode( pOLENode ), pSdrObj(sObj) {};

        const SwGrfNode* grfNode;
        Size size;
        const SwFlyFrmFmt* mOLEFrmFmt;
        SwOLENode* mOLENode;
        const SdrObject* pSdrObj;
    };
    std::list< PostponedGraphic >* m_postponedGraphic;
    struct PostponedDiagram
    {
        PostponedDiagram( const SdrObject* o, const SwFrmFmt* frm ) : object( o ), frame( frm ) {};
        const SdrObject* object;
        const SwFrmFmt* frame;
    };
    std::list< PostponedDiagram >* m_postponedDiagram;

    struct PostponedDrawing
    {
        PostponedDrawing( const SdrObject* sdrObj, const SwFrmFmt* frm, const Point* pt ) : object( sdrObj ), frame( frm ), point( pt ) {};
        const SdrObject* object;
        const SwFrmFmt* frame;
        const Point* point;
    };
    std::list< PostponedDrawing >* m_postponedVMLDrawing;
    std::list< PostponedDrawing >* m_postponedDMLDrawing;
    std::list< PostponedDrawing >* m_postponedCustomShape;

    struct PostponedOLE
    {
        PostponedOLE( SwOLENode* rObject, const Size& rSize, const SwFlyFrmFmt* rFrame ) : object( rObject ), size( rSize ), frame( rFrame ) {};
        SwOLENode* object;
        const Size size;
        const SwFlyFrmFmt* frame;
    };
    std::list< PostponedOLE >* m_postponedOLE;

    const SwOLENode* m_postponedMath;
    const SdrObject* m_postponedChart;
    Size m_postponedChartSize;
    std::vector<const SdrObject*> m_aPostponedFormControls;
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

    boost::scoped_ptr<TableReference> m_tableReference;

    std::map< OUString, EmbeddedFontRef > fontFilesMap; // font file url to data

    // Remember first cell (used for default borders/margins) of each table
    std::vector<ww8::WW8TableNodeInfoInner::Pointer_t> tableFirstCells;

    boost::optional<css::drawing::FillStyle> m_oFillStyle;
    /// If FormatBox() already handled fill style / gradient.
    bool m_bIgnoreNextFill;

    /// Is fake rotation detected, so rotation with 90 degrees should be ignored in this cell?
    bool m_bBtLr;

    PageMargins m_pageMargins;

    boost::shared_ptr<DocxTableStyleExport> m_pTableStyleExport;
    // flag to check if auto spacing was set in original file
    bool m_bParaBeforeAutoSpacing,m_bParaAfterAutoSpacing;
    // store hardcoded value which was set during import.
    sal_Int32 m_nParaBeforeSpacing,m_nParaAfterSpacing;

    bool m_setFootnote;

    /// RelId <-> Graphic* cache, so that in case of alternate content, the same graphic only gets written once.
    std::map<const Graphic*, OString> m_aRelIdCache;
    /// RelId <-> sal_uInt32 (bitmap checksum) cache, similar to m_aRelIdCache, but used for non-Writer graphics, handled in oox.
    std::map<sal_uInt32, OUString> m_aSdrRelIdCache;

    /// members to control the existence of grabbagged SDT properties in the paragraph
    sal_Int32 m_nParagraphSdtPrToken;
    ::sax_fastparser::FastAttributeList *m_pParagraphSdtPrTokenChildren;
    ::sax_fastparser::FastAttributeList *m_pParagraphSdtPrTokenAttributes;
    ::sax_fastparser::FastAttributeList *m_pParagraphSdtPrDataBindingAttrs;
    /// members to control the existence of grabbagged SDT properties in the text run
    sal_Int32 m_nRunSdtPrToken;
    /// State of the Fly at current position
    FlyProcessingState m_nStateOfFlyFrame;
    ::sax_fastparser::FastAttributeList *m_pRunSdtPrTokenChildren;
    ::sax_fastparser::FastAttributeList *m_pRunSdtPrDataBindingAttrs;
    /// Value of the <w:alias> paragraph SDT element.
    OUString m_aParagraphSdtPrAlias;
    /// Same as m_aParagraphSdtPrAlias, but its content is aviailable till the SDT is closed.
    OUString m_aStartedParagraphSdtPrAlias;
    OUString m_aRunSdtPrAlias;
    /// Currently paragraph SDT has a <w:id> child element.
    bool m_bParagraphSdtHasId;

    std::map<sal_uInt16, css::table::BorderLine2> m_aTableStyleConf;

public:
    DocxAttributeOutput( DocxExport &rExport, ::sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML );

    virtual ~DocxAttributeOutput();

    /// Return the right export class.
    virtual DocxExport& GetExport() SAL_OVERRIDE;
    const DocxExport& GetExport() const { return const_cast< DocxAttributeOutput* >( this )->GetExport(); }

    /// For eg. the output of the styles, we need to switch the serializer to an other one.
    void SetSerializer( ::sax_fastparser::FSHelperPtr pSerializer );

    /// Occasionnaly need to use this serializer from the outside
    ::sax_fastparser::FSHelperPtr GetSerializer( ) { return m_pSerializer; }

    /// Do we have any footnotes?
    bool HasFootnotes() const;

    /// Do we have any endnotes?
    bool HasEndnotes() const;

    /// Output the content of the footnotes.xml resp. endnotes.xml
    void FootnotesEndnotes( bool bFootnotes );

    /// writes the footnotePr/endnotePr (depending on tag) section
    void WriteFootnoteEndnotePr( ::sax_fastparser::FSHelperPtr fs, int tag, const SwEndNoteInfo& info, int listtag );

    bool HasPostitFields() const;
    void WritePostitFields();

    /// VMLTextExport
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) SAL_OVERRIDE;
    virtual void WriteVMLTextBox(css::uno::Reference<css::drawing::XShape> xShape) SAL_OVERRIDE;
    /// DMLTextExport
    virtual void WriteTextBox(css::uno::Reference<css::drawing::XShape> xShape) SAL_OVERRIDE;
    virtual OUString FindRelId(sal_uInt32 nChecksum) SAL_OVERRIDE;
    virtual void CacheRelId(sal_uInt32 nChecksum, const OUString& rRelId) SAL_OVERRIDE;
    virtual oox::drawingml::DrawingML& GetDrawingML() SAL_OVERRIDE;

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) SAL_OVERRIDE;

    void SetWritingHeaderFooter( bool bWritingHeaderFooter )    {   m_bWritingHeaderFooter = bWritingHeaderFooter;   }
    bool GetWritingHeaderFooter( )  {   return m_bWritingHeaderFooter;  }
    void SetAlternateContentChoiceOpen( bool bAltContentChoiceOpen ) { m_bAlternateContentChoiceOpen = bAltContentChoiceOpen; }
    bool IsAlternateContentChoiceOpen( ) { return m_bAlternateContentChoiceOpen; }
    void GetSdtEndBefore(const SdrObject* pSdrObj);
    void SetStartedParaSdt(bool bStartedParaSdt);
    bool IsStartedParaSdt();

    /// Stores the table export state to the passed context and resets own state.
    void pushToTableExportContext(DocxTableExportContext& rContext);
    /// Restores from the remembered state.
    void popFromTableExportContext(DocxTableExportContext& rContext);
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXATTRIBUTEOUTPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
