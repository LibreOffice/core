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

#ifndef _DOCXATTRIBUTEOUTPUT_HXX_
#define _DOCXATTRIBUTEOUTPUT_HXX_

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
    String     sCmd;
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

/// The class that has handlers for various resource types when exporting as DOCX.
class DocxAttributeOutput : public AttributeOutputBase, public oox::vml::VMLTextExport
{
public:
    /// Export the state of RTL/CJK.
    virtual void RTLAndCJKState( bool bIsRTL, sal_uInt16 nScript );

    /// Start of the paragraph.
    virtual void StartParagraph( ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo );

    /// End of the paragraph.
    virtual void EndParagraph( ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner );

    /// Empty paragraph.
    virtual void EmptyParagraph();

    /// Called before we start outputting the attributes.
    virtual void StartParagraphProperties( const SwTxtNode& rNode );

    /// Called after we end outputting the attributes.
    virtual void EndParagraphProperties();

    /// Start of the text run.
    virtual void StartRun( const SwRedlineData* pRedlineData, bool bSingleEmptyRun = false );

    /// End of the text run.
    virtual void EndRun();

    /// Called before we start outputting the attributes.
    virtual void StartRunProperties();

    /// Called after we end outputting the attributes.
    virtual void EndRunProperties( const SwRedlineData* pRedlineData );

    virtual void FootnoteEndnoteRefTag();

    virtual void SectFootnoteEndnotePr();

    virtual void WritePostitFieldReference();

    virtual void WritePostitFieldStart();

    virtual void WritePostitFieldEnd();

    /// Output text (inside a run).
    virtual void RunText( const OUString& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 );

    /// Output text (without markup).
    virtual void RawText( const String& rText, bool bForceUnicode, rtl_TextEncoding eCharSet );

    /// Output ruby start.
    virtual void StartRuby( const SwTxtNode& rNode, xub_StrLen nPos, const SwFmtRuby& rRuby );

    /// Output ruby end.
    virtual void EndRuby();

    /// Output URL start.
    virtual bool StartURL( const String& rUrl, const String& rTarget );

    /// Output URL end.
    virtual bool EndURL();

    virtual void FieldVanish( const String& rTxt, ww::eField eType );

    /// Output redlining.
    ///
    /// The common attribute that can be among the run properties.
    virtual void Redline( const SwRedlineData* pRedline );

    /// Output redlining.
    ///
    /// Start of the tag that encloses the run, fills the info according to
    /// the value of m_pRedlineData.
    void StartRedline();

    /// Output redlining.
    ///
    /// End of the tag that encloses the run.
    void EndRedline();

    virtual void FormatDrop( const SwTxtNode& rNode, const SwFmtDrop& rSwFmtDrop, sal_uInt16 nStyle, ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner );

    /// Output style.
    virtual void ParagraphStyle( sal_uInt16 nStyle );

    virtual void TableInfoCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableInfoRow( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableDefinition( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableDefaultBorders( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableDefaultCellMargins( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableBackgrounds( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableHeight( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableCanSplit( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableBidi( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableVerticalCell( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo );
    virtual void TableNodeInfoInner( ww8::WW8TableNodeInfoInner::Pointer_t pNodeInfoInner );
    virtual void TableOrientation( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableSpacing( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner );
    virtual void TableRowEnd( sal_uInt32 nDepth = 1 );

    /// Start of the styles table.
    virtual void StartStyles();

    /// End of the styles table.
    virtual void EndStyles( sal_uInt16 nNumberOfStyles );

    /// Write default style.
    virtual void DefaultStyle( sal_uInt16 nStyle );

    /// Write Doc Defaults
    void DocDefaults( );

    /// Start of a style in the styles table.
    virtual void StartStyle( const String& rName, bool bPapFmt,
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
    /// nPageRestartNumberr being 0 means no restart.
    virtual void SectionPageNumbering( sal_uInt16 nNumType, sal_uInt16 nPageRestartNumber );

    /// The type of breaking.
    virtual void SectionType( sal_uInt8 nBreakCode );

    /// Start the font.
    void StartFont( const String& rFamilyName ) const;

    /// End the font.
    void EndFont() const;

    /// Alternate name for the font.
    void FontAlternateName( const String& rName ) const;

    /// Font charset.
    void FontCharset( sal_uInt8 nCharSet, rtl_TextEncoding nEncoding ) const;

    /// Font family.
    void FontFamilyType( FontFamily eFamily ) const;

    /// Font pitch.
    void FontPitchType( FontPitch ePitch ) const;

    /// Write out the font into the document, if it's an embedded font.
    void EmbedFont( const OUString& name, FontFamily family, FontPitch pitch, rtl_TextEncoding encoding );

    /// Definition of a numbering instance.
    virtual void NumberingDefinition( sal_uInt16 nId, const SwNumRule &rRule );

    /// Start of the abstract numbering definition instance.
    virtual void StartAbstractNumbering( sal_uInt16 nId );

    /// End of the abstract numbering definition instance.
    virtual void EndAbstractNumbering();

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
        const SvxBrushItem* pBrush = 0 );

    void WriteField_Impl( const SwField* pFld, ww::eField eType, const String& rFldCmd, sal_uInt8 nMode );
    void WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark );

    void WriteBookmarks_Impl( std::vector< OUString >& rStarts, std::vector< OUString >& rEnds );

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
    void FlyFrameGraphic( const SwGrfNode* pGrfNode, const Size& rSize, const SwFlyFrmFmt* pOLEFrmFmt = 0, SwOLENode* pOLENode = 0);
    void WriteOLE2Obj( const SdrObject* pSdrObj, SwOLENode& rNode, const Size& rSize, const SwFlyFrmFmt* pFlyFrmFmt);
    bool WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize );
    bool WriteOLEMath( const SdrObject* pSdrObj, const SwOLENode& rNode, const Size& rSize );

    /// checks whether the current component is a diagram
    bool IsDiagram (const SdrObject* sdrObject);
    /// writes a diagram
    void WriteDiagram(const SdrObject* sdrObject, const Size& size);

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

    void WriteFFData( const FieldInfos& rInfos );
    void WritePendingPlaceholder();

    void EmbedFontStyle( const OUString& name, int tag, FontFamily family, FontItalic italic, FontWeight weight,
        FontPitch pitch, rtl_TextEncoding encoding );

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
    virtual void CharFontSizeCJK( const SvxFontHeightItem& rFontSize ) { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CJK_LANGUAGE
    virtual void CharLanguageCJK( const SvxLanguageItem& rLanguageItem ) { CharLanguage( rLanguageItem ); }

    /// Sfx item RES_CHRATR_CJK_POSTURE
    virtual void CharPostureCJK( const SvxPostureItem& rPosture );

    /// Sfx item RES_CHRATR_CJK_WEIGHT
    virtual void CharWeightCJK( const SvxWeightItem& rWeight );

    /// Sfx item RES_CHRATR_CTL_FONT
    virtual void CharFontCTL( const SvxFontItem& rFont );

    /// Sfx item RES_CHRATR_CTL_FONTSIZE
    virtual void CharFontSizeCTL( const SvxFontHeightItem& rFontSize ) { CharFontSize( rFontSize ); }

    /// Sfx item RES_CHRATR_CTL_LANGUAGE
    virtual void CharLanguageCTL( const SvxLanguageItem& rLanguageItem ) { CharLanguage( rLanguageItem); }

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

    /// Sfx item RES_TXTATR_INETFMT
    virtual void TextINetFormat( const SwFmtINetFmt& );

    /// Sfx item RES_TXTATR_CHARFMT
    virtual void TextCharFormat( const SwFmtCharFmt& );

    /// Sfx item RES_TXTATR_FTN
    virtual void TextFootnote_Impl( const SwFmtFtn& );

    /// Output the footnote/endnote reference (if there's one to output).
    void FootnoteEndnoteReference();

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

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pFld );

    virtual void RefField( const SwField& rFld, const String& rRef );
    virtual void HiddenField( const SwField& rFld );
    virtual void SetField( const SwField& rFld, ww::eField eType, const String& rCmd );
    virtual void PostitField( const SwField* pFld );
    virtual bool DropdownField( const SwField* pFld );
    virtual bool PlaceholderField( const SwField* pFld );

    virtual bool AnalyzeURL( const OUString& rURL, const OUString& rTarget, OUString* pLinkURL, OUString* pMark );

    /// Reference to the export, where to get the data from
    DocxExport &m_rExport;

    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

    /// DrawingML access
    oox::drawingml::DrawingML &m_rDrawingML;

private:

    void DoWriteBookmarks( );
    void WritePostponedGraphic();
    void WritePostponedMath();
    void WritePostponedDiagram();
    void WritePostponedChart();
    void WriteCommentRanges();

    void StartField_Impl( FieldInfos& rInfos, bool bWriteRun = sal_False );
    void DoWriteCmd( String& rCmd );
    void CmdField_Impl( FieldInfos& rInfos );
    void EndField_Impl( FieldInfos& rInfos );

    ::sax_fastparser::FastAttributeList *m_pFontsAttrList, *m_pEastAsianLayoutAttrList;
    ::sax_fastparser::FastAttributeList *m_pCharLangAttrList;
    ::sax_fastparser::FastAttributeList *m_pSectionSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pParagraphSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pHyperlinkAttrList;
    ::sax_fastparser::FastAttributeList *m_pFlyAttrList;
    ::sax_fastparser::FastAttributeList *m_pFlyFillAttrList;
    ::sax_fastparser::FastAttributeList *m_pFlyWrapAttrList;
    /// Attributes of the next v:textbox element.
    ::sax_fastparser::FastAttributeList *m_pTextboxAttrList;
    /// When exporting fly frames, this holds the real size of the frame.
    const Size* m_pFlyFrameSize;

    ::docx::FootnotesList *m_pFootnotesList;
    ::docx::FootnotesList *m_pEndnotesList;
    int m_footnoteEndnoteRefTag;
    int m_docPrID;
    boost::scoped_ptr< const WW8_SepInfo > m_pSectionInfo;

    /// Redline data to remember in the text run.
    const SwRedlineData *m_pRedlineData;

    /// Id of the redline
    sal_Int32 m_nRedlineId;

    /// Flag indicating that the section properties are being written
    bool m_bOpenedSectPr;

    /// Flag indicating that the header \ footer are being written
    bool m_bWritingHeaderFooter;

    /// Field data to remember in the text run
    std::vector< FieldInfos > m_Fields;
    String m_sFieldBkm;
    sal_Int32 m_nNextMarkId;

    /// Bookmarks to output
    std::vector<OString> m_rMarksStart;
    std::vector<OString> m_rMarksEnd;

    /// Is there a postit start to output?
    bool m_bPostitStart;
    /// Is there a postit end to output?
    bool m_bPostitEnd;

    /// Maps of the bookmarks ids
    std::map<OString, sal_uInt16> m_rOpenedMarksIds;

    /// Name of the last opened bookmark.
    OString m_sLastOpenedMark;

    /// If there are bookmarks around sequence fields, this map contains the
    /// names of these bookmarks for each sequence.
    std::map<OUString, std::vector<OString> > m_aSeqMarksNames;

    /// The current table helper
    SwWriteTable *m_pTableWrt;

    /// Remember if we are in an open cell, or not.
    bool m_bTableCellOpen;

    /// Remember the current table depth.
    sal_uInt32 m_nTableDepth;

    bool m_bParagraphOpened;

    // Remember that a column break has to be opened at the
    // beginning of the next paragraph
    DocxColBreakStatus m_nColBreakStatus;

    std::vector<sw::Frame> m_aParentFrames;
    bool m_bTextFrameSyntax;
    OStringBuffer m_aTextFrameStyle;
    // close of hyperlink needed
    bool m_closeHyperlinkInThisRun;
    bool m_closeHyperlinkInPreviousRun;
    bool m_startedHyperlink;

    struct PostponedGraphic
    {
        PostponedGraphic( const SwGrfNode* n, Size s ) : grfNode( n ), size( s ) {};
        const SwGrfNode* grfNode;
        Size size;
    };
    std::list< PostponedGraphic >* m_postponedGraphic;
    struct PostponedDiagram
    {
        PostponedDiagram( const SdrObject* o, Size s ) : object( o ), size( s ) {};
        const SdrObject* object;
        Size size;
    };
    std::list< PostponedDiagram >* m_postponedDiagram;
    const SwOLENode* m_postponedMath;
    const SdrObject* m_postponedChart;
    Size m_postponedChartSize;
    const SwField* pendingPlaceholder;
    std::vector< const SwPostItField* > m_postitFields;
    unsigned int m_postitFieldsMaxId;
    int m_anchorId;
    int m_nextFontId;
    struct EmbeddedFontRef
    {
        OString relId;
        OString fontKey;
    };
    std::map< OUString, EmbeddedFontRef > fontFilesMap; // font file url to data

    // Remember first cell (used for for default borders/margins) of each table
    std::vector<ww8::WW8TableNodeInfoInner::Pointer_t> tableFirstCells;

    boost::optional<XFillStyle> m_oFillStyle;

    /// Is fake rotation detected, so rotation with 90 degrees should be ignored in this cell?
    bool m_bBtLr;

    PageMargins m_pageMargins;

public:
    DocxAttributeOutput( DocxExport &rExport, ::sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML );

    virtual ~DocxAttributeOutput();

    /// Return the right export class.
    virtual DocxExport& GetExport();
    const DocxExport& GetExport() const { return const_cast< DocxAttributeOutput* >( this )->GetExport(); }

    /// For eg. the output of the styles, we need to switch the serializer to an other one.
    void SetSerializer( ::sax_fastparser::FSHelperPtr pSerializer ) { m_pSerializer = pSerializer; }

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
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj);
    virtual oox::drawingml::DrawingML& GetDrawingML();

    void BulletDefinition(int nId, const Graphic& rGraphic, Size aSize) SAL_OVERRIDE;

    void SetWritingHeaderFooter( bool bWritingHeaderFooter )    {   m_bWritingHeaderFooter = bWritingHeaderFooter;   }
    bool GetWritingHeaderFooter( )  {   return m_bWritingHeaderFooter;  }
};

#endif // _DOCXATTRIBUTEOUTPUT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
