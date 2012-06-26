/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DOCXATTRIBUTEOUTPUT_HXX_
#define _DOCXATTRIBUTEOUTPUT_HXX_

#include "attributeoutputbase.hxx"
#include "fields.hxx"
#include "IMark.hxx"
#include "docxexport.hxx"

#include <sax/fshelper.hxx>
#include <sax/fastattribs.hxx>
#include <vcl/vclenum.hxx>

#include <fldbas.hxx>

#include <vector>
#include <boost/scoped_ptr.hpp>

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

/// The class that has handlers for various resource types when exporting as DOCX.
class DocxAttributeOutput : public AttributeOutputBase
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

    /// Output text (inside a run).
    virtual void RunText( const String& rText, rtl_TextEncoding eCharSet = RTL_TEXTENCODING_UTF8 );

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
    void StartRedline( const SwRedlineData* pRedlineData );

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
        const String &rNumberingString );

    void WriteField_Impl( const SwField* pFld, ww::eField eType, const String& rFldCmd, sal_uInt8 nMode );
    void WriteFormData_Impl( const ::sw::mark::IFieldmark& rFieldmark );

    void WriteBookmarks_Impl( std::vector< rtl::OUString >& rStarts, std::vector< rtl::OUString >& rEnds );

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

    /// Output graphic fly frames.
    void FlyFrameGraphic( const SwGrfNode& rGrfNode, const Size& rSize );
    void WriteOLE2Obj( const SdrObject* pSdrObj, const SwOLENode& rNode, const Size& rSize );
    bool WriteOLEChart( const SdrObject* pSdrObj, const Size& rSize );
    bool WriteOLEMath( const SdrObject* pSdrObj, const SwOLENode& rNode, const Size& rSize );

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

    /// Write the expanded field
    virtual void WriteExpand( const SwField* pFld );

    virtual void RefField( const SwField& rFld, const String& rRef );
    virtual void HiddenField( const SwField& rFld );
    virtual void SetField( const SwField& rFld, ww::eField eType, const String& rCmd );
    virtual void PostitField( const SwField* pFld );
    virtual bool DropdownField( const SwField* pFld );

    virtual bool AnalyzeURL( const String& rURL, const String& rTarget, String* pLinkURL, String* pMark );

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

    void StartField_Impl( FieldInfos& rInfos, sal_Bool bWriteRun = sal_False );
    void DoWriteCmd( String& rCmd );
    void CmdField_Impl( FieldInfos& rInfos );
    void EndField_Impl( FieldInfos& rInfos );

    ::sax_fastparser::FastAttributeList *m_pFontsAttrList, *m_pEastAsianLayoutAttrList;
    ::sax_fastparser::FastAttributeList *m_pCharLangAttrList;
    ::sax_fastparser::FastAttributeList *m_pSectionSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pParagraphSpacingAttrList;
    ::sax_fastparser::FastAttributeList *m_pHyperlinkAttrList;
    ::sax_fastparser::FastAttributeList *m_pFlyAttrList;

    ::docx::FootnotesList *m_pFootnotesList;
    ::docx::FootnotesList *m_pEndnotesList;
    int m_footnoteEndnoteRefTag;

    boost::scoped_ptr< const WW8_SepInfo > m_pSectionInfo;

    /// Redline data to remember in the text run.
    const SwRedlineData *m_pRedlineData;

    /// Id of the redline
    sal_Int32 m_nRedlineId;

    /// Flag indicating that the section properties are being written
    sal_Bool m_bOpenedSectPr;

    /// Field data to remember in the text run
    std::vector< FieldInfos > m_Fields;
    String m_sFieldBkm;
    sal_Int32 m_nNextMarkId;

    /// Bookmarks to output
    std::vector<rtl::OString> m_rMarksStart;
    std::vector<rtl::OString> m_rMarksEnd;

    /// Maps of the bookmarks ids
    std::map<rtl::OString, sal_uInt16> m_rOpenedMarksIds;

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

    const sw::Frame *m_pParentFrame;
    // close of hyperlink needed
    bool m_closeHyperlinkInThisRun;
    bool m_closeHyperlinkInPreviousRun;

    struct PostponedGraphic
    {
        PostponedGraphic( const SwGrfNode* n, Size s ) : grfNode( n ), size( s ) {};
        const SwGrfNode* grfNode;
        Size size;
    };
    std::list< PostponedGraphic >* m_postponedGraphic;
    const SwOLENode* m_postponedMath;
    std::vector< const SwPostItField* > m_postitFields;
    unsigned int m_postitFieldsMaxId;

public:
    DocxAttributeOutput( DocxExport &rExport, ::sax_fastparser::FSHelperPtr pSerializer, oox::drawingml::DrawingML* pDrawingML );

    virtual ~DocxAttributeOutput();

    /// Return the right export class.
    virtual DocxExport& GetExport();
    const DocxExport& GetExport() const { return const_cast< DocxAttributeOutput* >( this )->GetExport(); }

    /// For eg. the output of the styles, we need to switch the serializer to enother one.
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
};

#endif // _DOCXATTRIBUTEOUTPUT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
