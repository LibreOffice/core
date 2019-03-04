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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WRTWW8_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WRTWW8_HXX

#include <sot/storage.hxx>
#include <tools/solar.h>
#include <tools/gen.hxx>
#include <editeng/editdata.hxx>

#include <shellio.hxx>
#include <wrt_fn.hxx>

#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "fields.hxx"
#include "types.hxx"
#include "writerhelper.hxx"
#include <msfilter.hxx>
#include <expfld.hxx>
#include "WW8TableInfo.hxx"

#include <vcl/graph.hxx>

#include <boost/optional.hpp>
#include <o3tl/typed_flags_set.hxx>

#include <cstddef>
#include <memory>
#include <map>
#include <vector>


class SvxBrushItem;

// some forward declarations
class SwWW8AttrIter;
namespace msfilter
{
    class MSCodec_Std97;
}

namespace editeng { class SvxBorderLine; }

class AttributeOutputBase;
class DocxAttributeOutput;
class RtfAttributeOutput;
class BitmapPalette;
class SwEscherEx;
class DateTime;
namespace vcl { class Font; }
class MSWordExportBase;
class SdrObject;
class SdrTextObj;
class SfxItemSet;
class SvStream;
class SvxFontItem;
class SvxBoxItem;
class SwAttrSet;
class SwCharFormat;
class SwContentNode;
class SwField;
class SwFormat;
class SwFormatContent;
class SwFormatFootnote;
class SwFrameFormat;
class SwGrfNode;
class SwModify;
class SwNumFormat;
class SwNumRule;
class SwNumRuleTable;
class SwPageDesc;
class SwFormatPageDesc;
class SwOLENode;
class SwPostItField;
class SwRedlineData;
class SwSectionFormat;
class SwSectionNode;
class SwTableNode;
class SwTOXType;
class SwTextFormatColl;
class SwTextNode;
class SwWW8WrGrf;
class SwWW8Writer;
class MSWordStyles;
class WW8AttributeOutput;
class WW8Export;
class MSWordAttrIter;
class WW8_WrFkp;
class WW8_WrPlc0;
class WW8_WrPlc1;
class WW8_WrPlcField;
class WW8_WrMagicTable;
class WW8_WrPlcFootnoteEdn;
class WW8_WrPlcPn;
class WW8_WrPlcAnnotations;
class WW8_WrtFactoids;
class MSWordSections;
class WW8_WrPlcTextBoxes;
class WW8_WrPct;            // administration
class WW8_WrtBookmarks;
class WW8_WrtRedlineAuthor;
class SvxMSExportOLEObjects;
class SwMSConvertControls;
class WW8_WrPc;

namespace com { namespace sun { namespace star { namespace embed {
class XEmbeddedObject;
} } } }
typedef std::map<const css::embed::XEmbeddedObject*, sal_Int32> WW8OleMap;
struct WW8_PdAttrDesc;
class SvxBrushItem;
namespace sw
{
namespace mark
{
class IFieldmark;
}
}
typedef std::set< sal_Int32 > SwSoftPageBreakList;

#define GRF_MAGIC_1 0x12    // 3 magic bytes for PicLocFc attribute
#define GRF_MAGIC_2 0x34
#define GRF_MAGIC_3 0x56
#define GRF_MAGIC_321 0x563412L

#define OLE_PREVIEW_AS_EMF  //If we want to export ole2 previews as emf in ww8+

enum class FieldFlags : sal_uInt8 // for InsertField- Method
{
    NONE        = 0x00,
    Start       = 0x01,
    CmdStart    = 0x02,
    CmdEnd      = 0x04,
    End         = 0x10,
    Close       = 0x20,
    All         = 0x37
};
namespace o3tl {
    template<> struct typed_flags<FieldFlags> : is_typed_flags<FieldFlags, 0x37> {};
}

enum TextTypes  //enums for TextTypes
{
    TXT_MAINTEXT = 0, /*TXT_FTNEDN = 1,*/ TXT_HDFT = 2, TXT_FTN = 3,
    TXT_EDN = 4, TXT_ATN = 5, TXT_TXTBOX = 6, TXT_HFTXTBOX= 7
};

/**
enum to state the present state of the fly
*/
enum FlyProcessingState
{
    FLY_PROCESSED,
    FLY_POSTPONED,
    FLY_NOT_PROCESSED
};

struct WW8_SepInfo
{
    const SwPageDesc* pPageDesc;
    const SwSectionFormat* pSectionFormat;
    const SwNode* pPDNd;
    sal_uLong const  nLnNumRestartNo;
    ::boost::optional<sal_uInt16> const oPgRestartNo;
    bool const bIsFirstParagraph;

    WW8_SepInfo( const SwPageDesc* pPD, const SwSectionFormat* pFormat,
                 sal_uLong nLnRestart, ::boost::optional<sal_uInt16> oPgRestart = boost::none,
                 const SwNode* pNd = nullptr, bool bIsFirstPara = false )
        : pPageDesc( pPD ), pSectionFormat( pFormat ), pPDNd( pNd ),
          nLnNumRestartNo( nLnRestart ), oPgRestartNo( oPgRestart ),
          bIsFirstParagraph( bIsFirstPara )
    {}

    bool IsProtected() const;
};

/// Class to collect and output the sections/headers/footers.
// Plc for PageDescs -> Sepx ( Section Extensions )
class MSWordSections
{
protected:
    bool mbDocumentIsProtected;
    std::vector<WW8_SepInfo> aSects;

    void CheckForFacinPg( const WW8Export& rWrt ) const;
    void NeedsDocumentProtected(const WW8_SepInfo &rInfo);

    //No copy, no assign
    MSWordSections( const MSWordSections& );
    MSWordSections& operator=( const MSWordSections& );
public:
    explicit MSWordSections( MSWordExportBase& rExport );
    virtual ~MSWordSections();

    virtual bool HeaderFooterWritten();

    void AppendSection( const SwPageDesc* pPd,
                    const SwSectionFormat* pSectionFormat,
                    sal_uLong nLnNumRestartNo,
                    bool bIsFirstParagraph = false );
    void AppendSection( const SwFormatPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFormat* pSectionFormat,
                    sal_uLong nLnNumRestartNo );

    /// Number of columns based on the most recent WW8_SepInfo.
    sal_uInt16 CurrentNumberOfColumns( const SwDoc &rDoc ) const;

    /// Number of columns of the provided WW8_SepInfo.
    static sal_uInt16 NumberOfColumns( const SwDoc &rDoc, const WW8_SepInfo& rInfo );

    bool DocumentIsProtected() const { return mbDocumentIsProtected; }

    /// The most recent WW8_SepInfo.
    const WW8_SepInfo* CurrentSectionInfo();

    static void SetHeaderFlag( sal_uInt8& rHeadFootFlags, const SwFormat& rFormat,
                                  sal_uInt8 nFlag );
    static void SetFooterFlag( sal_uInt8& rHeadFootFlags, const SwFormat& rFormat,
                                   sal_uInt8 nFlag );

    /// Should we output borders?
    static bool HasBorderItem( const SwFormat& rFormat );
};

class WW8_WrPlcSepx : public MSWordSections
{
    std::vector<WW8_CP> aCps;
    std::vector< std::shared_ptr<WW8_PdAttrDesc> > m_SectionAttributes;
    // hack to prevent adding sections in endnotes
    bool m_bHeaderFooterWritten;
    std::unique_ptr<WW8_WrPlc0> pTextPos;        // Position of the headers/footers

    WW8_WrPlcSepx( const WW8_WrPlcSepx& ) = delete;
    WW8_WrPlcSepx& operator=( const WW8_WrPlcSepx& ) = delete;

public:
    explicit WW8_WrPlcSepx( MSWordExportBase& rExport );
    virtual ~WW8_WrPlcSepx() override;

    virtual bool HeaderFooterWritten() override; // override

    void AppendSep( WW8_CP nStartCp,
                    const SwPageDesc* pPd,
                    const SwSectionFormat* pSectionFormat,
                    sal_uLong nLnNumRestartNo );
    void AppendSep( WW8_CP nStartCp, const SwFormatPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFormat* pSectionFormat,
                    sal_uLong nLnNumRestartNo );
    void Finish( WW8_CP nEndCp ) { aCps.push_back( nEndCp ); }

    bool WriteKFText( WW8Export& rWrt );
    void WriteSepx( SvStream& rStrm ) const;
    void WritePlcSed( WW8Export& rWrt ) const;
    void WritePlcHdd( WW8Export& rWrt ) const;

private:
    void WriteFootnoteEndText( WW8Export& rWrt, sal_uLong nCpStt );
public:
    void OutHeaderFooter(WW8Export& rWrt, bool bHeader,
            const SwFormat& rFormat, sal_uLong& rCpPos, sal_uInt8 nHFFlags, sal_uInt8 nFlag,  sal_uInt8 nBreakCode);
};

// class WW8_WrPct to construct the piece table
class WW8_WrPct
{
    std::vector<std::unique_ptr<WW8_WrPc>> m_Pcts;
    WW8_FC nOldFc;
public:
    explicit WW8_WrPct(WW8_FC nStartFc);
    ~WW8_WrPct();
    void AppendPc(WW8_FC nStartFc);
    void WritePc( WW8Export& rWrt );
    void SetParaBreak();
    WW8_CP Fc2Cp( sal_uLong nFc ) const;
};

/// Collects and outputs fonts.
class wwFont
{
//In some future land the stream could be converted to a nice stream interface
//and we could have harmony
private:
sal_uInt8 maWW8_FFN[6];
    OUString msFamilyNm;
    OUString msAltNm;
    bool mbAlt;
    FontPitch const mePitch;
    FontFamily const meFamily;
    rtl_TextEncoding const meChrSet;
public:
     wwFont( const OUString &rFamilyName, FontPitch ePitch, FontFamily eFamily,
        rtl_TextEncoding eChrSet);
    void Write( SvStream *pTableStram ) const;
    void WriteDocx( DocxAttributeOutput* rAttrOutput ) const;
    void WriteRtf( const RtfAttributeOutput* rAttrOutput ) const;
    OUString const & GetFamilyName() const { return msFamilyNm; }
    friend bool operator < (const wwFont &r1, const wwFont &r2);
};

class wwFontHelper
{
private:
    /// Keep track of fonts that need to be exported.
    std::map<wwFont, sal_uInt16> maFonts;

    /// Convert from fast insertion map to linear vector in the order that we want to write.
    std::vector< const wwFont* > AsVector() const;

public:
    wwFontHelper() : bLoadAllFonts(false) {}
    /// rDoc used only to get the initial standard font(s) in use.
    void InitFontTable(const SwDoc& rDoc);
    sal_uInt16 GetId(const SvxFontItem& rFont);
    sal_uInt16 GetId(const wwFont& rFont);
    void WriteFontTable( SvStream *pTableStream, WW8Fib& pFib );
    void WriteFontTable( DocxAttributeOutput& rAttrOutput );
    void WriteFontTable( const RtfAttributeOutput& rAttrOutput );

    /// If true, all fonts are loaded before processing the document.
    bool bLoadAllFonts: 1;
};

class DrawObj
{
public:
    WW8_CP const mnCp;                // CP-Pos of references
    sal_uInt32 mnShapeId;       // ShapeId for the SwFrameFormats
    ww8::Frame maContent;       // the frame itself
    Point const maParentPos;          // Points
    sal_Int32 mnThick;          // Border Thicknesses
    SvxFrameDirection const mnDirection; // If BiDi or not
    unsigned int const mnHdFtIndex;   // 0 for main text, +1 for each subsequent
                                // msword hd/ft

    DrawObj(const ww8::Frame &rContent, WW8_CP nCp, Point aParentPos, SvxFrameDirection nDir,
            unsigned int nHdFtIndex)
        : mnCp(nCp), mnShapeId(0), maContent(rContent), maParentPos(aParentPos),
        mnThick(0), mnDirection(nDir), mnHdFtIndex(nHdFtIndex) {}
    void SetShapeDetails(sal_uInt32 nId, sal_Int32 nThick);
};

typedef std::vector<DrawObj> DrawObjVector;
typedef std::vector<DrawObj *> DrawObjPointerVector;

class PlcDrawObj // PC for DrawObjects and Text-/OLE-/GRF-Boxes
{
private:
    DrawObjVector maDrawObjs;  // vector of drawobjs
protected:
    virtual void RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
        sal_uInt32 nLen) const = 0;
    virtual WW8_CP GetCpOffset(const WW8Fib &rFib) const = 0;
public:
    PlcDrawObj() {}
    void WritePlc( WW8Export& rWrt ) const;
    bool Append( WW8Export const &, WW8_CP nCp, const ww8::Frame& rFormat,
        const Point& rNdTopLeft );
    int size() { return maDrawObjs.size(); };
    DrawObjVector &GetObjArr() { return maDrawObjs; }
    virtual ~PlcDrawObj();
private:
    PlcDrawObj(const PlcDrawObj&) = delete;
    PlcDrawObj& operator=(const PlcDrawObj&) = delete;
};

class MainTextPlcDrawObj : public PlcDrawObj
{
public:
    MainTextPlcDrawObj() {}
private:
    virtual void RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
        sal_uInt32 nLen) const override;
    virtual WW8_CP GetCpOffset(const WW8Fib &) const override;
private:
    MainTextPlcDrawObj(const MainTextPlcDrawObj&) = delete;
    MainTextPlcDrawObj& operator=(const MainTextPlcDrawObj&) = delete;
};

class HdFtPlcDrawObj : public PlcDrawObj
{
public:
    HdFtPlcDrawObj() {}
private:
    virtual void RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
        sal_uInt32 nLen) const override;
    virtual WW8_CP GetCpOffset(const WW8Fib &rFib) const override;
private:
    HdFtPlcDrawObj(const HdFtPlcDrawObj&) = delete;
    HdFtPlcDrawObj& operator=(const HdFtPlcDrawObj&) = delete;
};

typedef std::pair<OUString, sal_uLong> aBookmarkPair;

class WW8_WrtRedlineAuthor : public sw::util::WrtRedlineAuthor
{
    public:
    virtual void Write(Writer &rWrt) override;
};

/** Structure that is used to save some of the WW8Export/DocxExport data.

    It is used to be able to recurse inside of the WW8Export/DocxExport (eg.
    for the needs of the tables) - you need to tall WriteText() from there with
    new values of PaM etc.

    It must contain all the stuff that might be saved either in WW8Export or in
    DocxExport, because it makes no sense to do it abstract, and specialize it
    for each of the cases.  If you implement other *Export, just add the needed
    members here, and store them in the appropriate SaveData() method.
 */
struct MSWordSaveData
{
    Point* pOldFlyOffset;
    RndStdIds eOldAnchorType;
    std::unique_ptr<ww::bytes> pOOld; ///< WW8Export only
    SwPaM* pOldPam, *pOldEnd;
    sal_uLong nOldStart, nOldEnd;
    const ww8::Frame* pOldFlyFormat;
    const SwPageDesc* pOldPageDesc;

    bool bOldWriteAll : 1;          ///< WW8Export only
    bool bOldOutTable : 1;
    bool bOldFlyFrameAttrs : 1;
    bool bOldStartTOX : 1;
    bool bOldInWriteTOX : 1;
    // m_bOutPageDescs does not have to be saved in MSWordExportBase::SaveData
    // since it is only modified when outputting special texts.
};

/// Base class for WW8Export and DocxExport
class MSWordExportBase
{
public:
    wwFontHelper m_aFontHelper;
    std::vector<sal_uLong> m_aChapterFieldLocs;
    OUString const m_aMainStg;
    std::vector<const SwTOXType*> m_aTOXArr;
    const SfxItemSet* m_pISet;    // for double attributes
    WW8_WrPct*  m_pPiece;         // Pointer to Piece-Table
    std::unique_ptr<SwNumRuleTable> m_pUsedNumTable;  // all used NumRules
    const SwTextNode *m_pTopNodeOfHdFtPage; ///< Top node of host page when in hd/ft
    std::map< sal_uInt16, sal_uInt16 > m_aRuleDuplicates; //map to Duplicated numrules
    std::stack< sal_Int32 > m_aCurrentCharPropStarts; ///< To remember the position in a run.
    WW8_WrtBookmarks* m_pBkmks;
    WW8_WrtRedlineAuthor* m_pRedlAuthors;
    std::shared_ptr<NfKeywordTable> m_pKeyMap;
    std::unique_ptr<SvxMSExportOLEObjects> m_pOLEExp;
    std::unique_ptr<SwMSConvertControls> m_pOCXExp;
    WW8OleMap m_aOleMap;    // To remember all already exported ole objects
    ww8::WW8TableInfo::Pointer_t m_pTableInfo;

    sal_uInt16 m_nCharFormatStart;
    sal_uInt16 m_nFormatCollStart;
    sal_uInt16 m_nStyleBeforeFly;     ///< style number of the node
                                ///<       to which the Fly is connected
    sal_uInt16 m_nLastFormatId;          ///< Style of last TextNode in normal range
    sal_uInt16 m_nUniqueList;         ///< current number for creating unique list names
    unsigned int m_nHdFtIndex;

    RedlineFlags m_nOrigRedlineFlags;   ///< Remember the original redline mode

public:
    /* implicit bookmark vector containing pairs of node indexes and bookmark names */
    std::vector<aBookmarkPair> m_aImplicitBookmarks;
    ww8::Frames m_aFrames;             // The floating frames in this document
    const SwPageDesc *m_pCurrentPageDesc;
    bool m_bPrevTextNodeIsEmpty;
    bool m_bFirstTOCNodeWithSection;
    std::unique_ptr<WW8_WrPlcPn> m_pPapPlc;
    std::unique_ptr<WW8_WrPlcPn> m_pChpPlc;
    MSWordAttrIter* m_pChpIter;
    std::unique_ptr<MSWordStyles> m_pStyles;
    WW8_WrPlcAnnotations* m_pAtn;
    std::unique_ptr<WW8_WrtFactoids> m_pFactoids;
    WW8_WrPlcTextBoxes *m_pTextBxs, *m_pHFTextBxs;

    struct LinkedTextboxInfo        //help analyze textbox flow links
    {
        sal_Int32 nId;
        sal_Int32 nSeq;
        OUString sNextChain;
        OUString sPrevChain;
        LinkedTextboxInfo(): nId(0), nSeq(0) {}
    };
    std::map<OUString,LinkedTextboxInfo> m_aLinkedTextboxesHelper;
    bool m_bLinkedTextboxesHelperInitialized = false;
    sal_Int32 m_nLinkedTextboxesChainId=0;

    const ww8::Frame *m_pParentFrame; // If set we are exporting content inside
                                    // a frame, e.g. a graphic node

    Point* m_pFlyOffset;              // for adjusting of character-bound Fly in the Writer,
    RndStdIds m_eNewAnchorType;       // that is paragraph-bound in the WW.

    std::unique_ptr<WW8_WrPlcField> m_pFieldMain;         // fields in MainText
    std::unique_ptr<WW8_WrPlcField> m_pFieldHdFt;         // fields in Header/Footer
    std::unique_ptr<WW8_WrPlcField> m_pFieldFootnote;          // fields in FootNotes
    std::unique_ptr<WW8_WrPlcField> m_pFieldEdn;          // fields in EndNotes
    std::unique_ptr<WW8_WrPlcField> m_pFieldAtn;          // fields in Annotations
    std::unique_ptr<WW8_WrPlcField> m_pFieldTextBxs;       // fields in textboxes
    std::unique_ptr<WW8_WrPlcField> m_pFieldHFTextBxs;     // fields in header/footer textboxes
    std::unique_ptr<WW8_WrMagicTable> m_pMagicTable;  // keeps track of table cell positions, and
                                    // marks those that contain graphics,
                                    // which is required to make word display
                                    // graphics inside tables
    std::unique_ptr<SwWW8WrGrf> m_pGrf;
    const SwAttrSet* m_pStyAttr;      // StyleAttr for Tabs
    const SwModify* m_pOutFormatNode;    // write Format or Node
    const SwFormat *m_pCurrentStyle;     // iff bStyDef=true, then this store the current style

    MainTextPlcDrawObj *m_pSdrObjs;   // Draw-/Fly-Objects
    HdFtPlcDrawObj *m_pHFSdrObjs;     // Draw-/Fly-Objects in header or footer

    SwEscherEx* m_pEscher;            // escher export class
    // #i43447# - removed
//    SwTwips nFlyWidth, nFlyHeight;  // for adaptation of graphics

    sal_uInt8 m_nTextTyp;

    bool m_bStyDef : 1;           // should Style be written?
    bool m_bBreakBefore : 1;      // Breaks are being written 2 times
    bool m_bOutKF : 1;            // Header/Footer texts are being written
    bool m_bOutFlyFrameAttrs : 1;   // Frame-attr of Flys are being written
    bool m_bOutPageDescs : 1;     ///< PageDescs (section properties) are being written
    bool m_bOutFirstPage : 1;     // write Attrset of FirstPageDesc
    bool m_bOutTable : 1;         // table is being written
                                     // ( is reset e.g. for Flys in a table )
    bool m_bOutGrf : 1;           // graphics are being written
    bool m_bInWriteEscher : 1;    // in write textboxes
    bool m_bStartTOX : 1;         // true: a TOX is started
    bool m_bInWriteTOX : 1;       // true: all content are in a TOX
    bool m_bFootnoteAtTextEnd : 1;      // true: all FTN at Textend
    bool m_bEndAtTextEnd : 1;      // true: all END at Textend
    bool m_bHasHdr : 1;
    bool m_bHasFtr : 1;
    bool m_bSubstituteBullets : 1; // true: SubstituteBullet() gets called
    bool m_bTabInTOC : 1; //true for TOC field flag 'w'

    bool m_bHideTabLeaderAndPageNumbers : 1 ; // true: the 'z' field of TOC is set.
    bool m_bExportModeRTF;
    /// Is font size written already as part of the current character properties?
    bool m_bFontSizeWritten;
    bool m_bAddFootnoteTab;     // only one aesthetic spacing tab per footnote

    SwDoc *m_pDoc;
    sal_uLong m_nCurStart, m_nCurEnd;
    SwPaM *m_pCurPam, *m_pOrigPam;

    /// Stack to remember the nesting (see MSWordSaveData for more)
    std::stack< MSWordSaveData > m_aSaveData;

    /// Used to split the runs according to the bookmarks start and ends
    typedef std::vector< ::sw::mark::IMark* > IMarkVector;
    IMarkVector m_rSortedBookmarksStart;
    IMarkVector m_rSortedBookmarksEnd;
    IMarkVector m_rSortedAnnotationMarksStart;
    IMarkVector m_rSortedAnnotationMarksEnd;

public:
    /// The main function to export the document.
    ErrCode ExportDocument( bool bWriteAll );

    /// Iterate through the nodes and call the appropriate OutputNode() on them.
    void WriteText();

    /// Return whether currently exported node is in table.
    bool IsInTable() const;

    /// Set the pCurPam appropriately and call WriteText().
    ///
    /// Used to export paragraphs in footnotes/endnotes/etc.
    void WriteSpecialText( sal_uLong nStart, sal_uLong nEnd, sal_uInt8 nTTyp );

    /// Export the pool items to attributes (through an attribute output class).
    void ExportPoolItemsToCHP( ww8::PoolItems &rItems, sal_uInt16 nScript, const SvxFontItem *pFont, bool bWriteCombChars = false );

    /// Return the numeric id of the numbering rule
    sal_uInt16 GetId( const SwNumRule& rNumRule );

    /// Return the numeric id of the style.
    sal_uInt16 GetId( const SwTextFormatColl& rColl ) const;

    /// Return the numeric id of the style.
    sal_uInt16 GetId( const SwCharFormat* pFormat ) const;

    sal_uInt16 GetId( const SwTOXType& rTOXType );

    /// Return the numeric id of the font (and add it to the font list if needed)
    sal_uInt16 GetId( const SvxFontItem& rFont)
    {
        return m_aFontHelper.GetId(rFont);
    }
    /// @overload
    void GetId( const wwFont& rFont)
    {
        m_aFontHelper.GetId(rFont);
    }

    const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const;
    template<class T> const T& GetItem( TypedWhichId<T> nWhich ) const
    {
        return static_cast<const T&>(GetItem(sal_uInt16(nWhich)));
    }

    /// Find the reference.
    bool HasRefToObject( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo );

    /// Find the bookmark name.
    static OUString GetBookmarkName( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo );

    /// Use OutputItem() on an item set according to the parameters.
    void OutputItemSet( const SfxItemSet& rSet, bool bPapFormat, bool bChpFormat, sal_uInt16 nScript, bool bExportParentItemSet );

    SvxFrameDirection GetDefaultFrameDirection( ) const;

    /// Right to left?
    SvxFrameDirection TrueFrameDirection( const SwFrameFormat& rFlyFormat ) const;

    /// Right to left?
    SvxFrameDirection GetCurrentPageDirection() const;

    /// In case of numbering restart.

    /// List is set to restart at a particular value so for export make a
    /// completely new list based on this one and export that instead,
    /// which duplicates words behaviour in this respect.
    sal_uInt16 DuplicateNumRule( const SwNumRule *pRule, sal_uInt8 nLevel, sal_uInt16 nVal );

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const = 0;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const = 0;

    /// Determines if column break with one column should be exported or not.
    virtual bool SupportsOneColumnBreak() const = 0;

    /// Determines if the import filter already quoted fields or not.
    virtual bool FieldsQuoted() const = 0;

    /// Determines the Section Breaks are to be added for TOX Section
    virtual bool AddSectionBreaksForTOX() const = 0;

    /// Used to filter out attributes that can be e.g. written to .doc but not to .docx
    virtual bool ignoreAttributeForStyleDefaults( sal_uInt16 /*nWhich*/ ) const { return false; }

    /// If saving page break is preferred as a paragraph attribute (yes) or as a special character (no).
    virtual bool PreferPageBreakBefore() const = 0;

    /// Text in tables can be postponed except for .doc
    virtual bool AllowPostponedTextInTable() const { return true; }

    /// Guess the script (asian/western).
    ///
    /// Sadly word does not have two different sizes for asian font size and
    /// western font size, it has two different fonts, but not sizes, so we
    /// have to use our guess as to the script used and disable the export of
    /// one type. The same occurs for font weight and posture (bold and
    /// italic).
    ///
    /// In addition WW7- has only one character language identifier while WW8+
    /// has two
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) = 0;

    virtual void AppendBookmarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen ) = 0;

    virtual void AppendBookmark( const OUString& rName ) = 0;

    virtual void AppendAnnotationMarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen ) = 0;

    virtual void AppendSmartTags(SwTextNode& /*rTextNode*/) { }

    //For i120928,add this interface to export graphic of bullet
    virtual void ExportGrfBullet(const SwTextNode& rNd) = 0;

    // FIXME probably a hack...
    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t() ) = 0;

    // FIXME definitely a hack, must not be here - it can't do anything
    // sensible for docx
    virtual void WriteChar( sal_Unicode c ) = 0;

    /// Output attributes.
    void OutputFormat( const SwFormat& rFormat, bool bPapFormat, bool bChpFormat, bool bFlyFormat = false );

    /// Getter for pISet.
    const SfxItemSet* GetCurItemSet() const { return m_pISet; }

    /// Setter for pISet.
    void SetCurItemSet( const SfxItemSet* pS ) { m_pISet = pS; }

    /// Remember some of the members so that we can recurse in WriteText().
    virtual void SaveData( sal_uLong nStt, sal_uLong nEnd );

    /// Restore what was saved in SaveData().
    virtual void RestoreData();

    /// The return value indicates, if a follow page desc is written.
    bool OutputFollowPageDesc( const SfxItemSet* pSet,
                               const SwTextNode* pNd );

    /// Write header/footer text.
    void WriteHeaderFooterText( const SwFormat& rFormat, bool bHeader);

    /// Format of the section.
    static const SwSectionFormat* GetSectionFormat( const SwNode& rNd );

    /// Line number of the section start.
    static sal_uLong GetSectionLineNo( const SfxItemSet* pSet, const SwNode& rNd );

    /// Start new section.
    void OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd, bool isCellOpen = false, bool isTextNodeEmpty = false);

    /// Write section properties.
    ///
    /// pA is ignored for docx.
    void SectionProperties( const WW8_SepInfo& rSectionInfo, WW8_PdAttrDesc* pA = nullptr );

    /// Output the numbering table.
    virtual void WriteNumbering() = 0;

    /// Write static data of SwNumRule - LSTF
    void NumberingDefinitions();

    /// Write all Levels for all SwNumRules - LVLF
    void AbstractNumberingDefinitions();

    // Convert the bullet according to the font.
    void SubstituteBullet( OUString& rNumStr, rtl_TextEncoding& rChrSet,
        OUString& rFontName ) const;

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* /*pA*/ ) {}

    /// Top node of host page when in header/footer.
    void SetHdFtPageRoot( const SwTextNode *pNd ) { m_pTopNodeOfHdFtPage = pNd; }

    /// Top node of host page when in header/footer.
    const SwTextNode *GetHdFtPageRoot() const { return m_pTopNodeOfHdFtPage; }

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrameFormat& rFormat, const SwFrameFormat& rLeftFormat, const SwFrameFormat& rFirstPageFormat,
        sal_uInt8 nBreakCode) = 0;

    /// Write the field
    virtual void OutputField( const SwField* pField, ww::eField eFieldType,
            const OUString& rFieldCmd, FieldFlags nMode = FieldFlags::All ) = 0;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    css::uno::Sequence<OUString> &rListItems) = 0;

    virtual void DoFormText(const SwInputField * pField) = 0;

    static bool NoPageBreakSection( const SfxItemSet *pSet );

    // Compute the number format for WW dates
    bool GetNumberFormat(const SwField& rField, OUString& rStr);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) = 0;

    const SfxPoolItem* HasItem( sal_uInt16 nWhich ) const;

    /// Returns the index of a picture bullet, used in numberings.
    int GetGrfIndex(const SvxBrushItem& rBrush);

    enum ExportFormat { DOC = 0, RTF = 1, DOCX = 2};
    virtual ExportFormat GetExportFormat() const = 0;

protected:
    /// Format-dependent part of the actual export.
    virtual ErrCode ExportDocument_Impl() = 0;

    /// Get the next position in the text node to output
    sal_Int32 GetNextPos( SwWW8AttrIter const * pAttrIter, const SwTextNode& rNode, sal_Int32 nCurrentPos );

    /// Update the information for GetNextPos().
    void UpdatePosition( SwWW8AttrIter* pAttrIter, sal_Int32 nCurrentPos );

    /// Output SwTextNode
    virtual void OutputTextNode( SwTextNode& );

    /// Setup the chapter fields (maChapterFieldLocs).
    void GatherChapterFields();

    void AddLinkTarget( const OUString& rURL );
    void CollectOutlineBookmarks( const SwDoc &rDoc );

    bool SetCurrentPageDescFromNode(const SwNode &rNd);
    bool ContentContainsChapterField(const SwFormatContent &rContent) const;
    bool FormatHdFtContainsChapterField(const SwFrameFormat &rFormat) const;

    virtual void SectionBreaksAndFrames( const SwTextNode& rNode ) = 0;

    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFormatPageDesc* pNewPgDescFormat,
                                     const SwPageDesc* pNewPgDesc ) = 0;

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering(const SwFormat &rFormat) = 0;

    /// Output SwStartNode
    void OutputStartNode( const SwStartNode& );

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& );

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) = 0;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) = 0;

    virtual void OutputLinkedOLE( const OUString& ) = 0;

    /// Output SwSectionNode
    void OutputSectionNode( const SwSectionNode& );
    static void UpdateTocSectionNodeProperties(const SwSectionNode& rSectionNode);

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum ) = 0;

    /// Call the right (virtual) function according to the type of the item.
    ///
    /// One of OutputTextNode(), OutputGrfNode(), or OutputOLENode()
    void OutputContentNode( SwContentNode& );

    /// Find the nearest bookmark from the current position.
    ///
    /// Returns false when there is no bookmark.
    bool NearestBookmark( sal_Int32& rNearest, const sal_Int32 nCurrentPos, bool bNextPositionOnly );

    void GetSortedBookmarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen );

    bool GetBookmarks( const SwTextNode& rNd, sal_Int32 nStt, sal_Int32 nEnd,
            IMarkVector& rArr );

    /// Find the nearest annotation mark from the current position.
    ///
    void NearestAnnotationMark( sal_Int32& rNearest, const sal_Int32 nCurrentPos, bool bNextPositionOnly );

    void GetSortedAnnotationMarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen );

    bool GetAnnotationMarks( const SwTextNode& rNd, sal_Int32 nStt, sal_Int32 nEnd,
            IMarkVector& rArr );

    const NfKeywordTable & GetNfKeywordTable();

    void SetCurPam(sal_uLong nStt, sal_uLong nEnd);

    /// Get background color of the document, if there is one.
    boost::optional<SvxBrushItem> getBackground();
    /// Populates m_vecBulletPic with all the bullet graphics used by numberings.
    int CollectGrfsOfBullets();
    /// Write the numbering picture bullets.
    void BulletDefinitions();

    bool NeedSectionBreak( const SwNode& rNd ) const;
    bool NeedTextNodeSplit( const SwTextNode& rNd, SwSoftPageBreakList& pList ) const;

    std::vector<const Graphic*> m_vecBulletPic; ///< Vector to record all the graphics of bullets

public:
    MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam );
    virtual ~MSWordExportBase();

    // TODO move as much as possible here from WW8Export! ;-)

    static void CorrectTabStopInSet( SfxItemSet& rSet, sal_Int32 nAbsLeft );

private:
    MSWordExportBase( const MSWordExportBase& ) = delete;
    MSWordExportBase& operator=( const MSWordExportBase& ) = delete;
};

/// The writer class that gets called for the WW8 filter.
class SwWW8Writer: public StgWriter
{
// friends to get access to m_pExport
// FIXME avoid that, this is probably not what we want
// (if yes, remove the friends, and provide here a GetExport() method)
friend void WW8_WrtRedlineAuthor::Write(Writer &rWrt);

    WW8Export *m_pExport;
    SfxMedium *mpMedium;

public:
    SwWW8Writer(const OUString& rFltName, const OUString& rBaseURL);
    virtual ~SwWW8Writer() override;

    virtual ErrCode WriteStorage() override;
    virtual ErrCode WriteMedium( SfxMedium& ) override;

    // TODO most probably we want to be able to get these in
    // MSExportFilterBase
    using Writer::getIDocumentSettingAccess;

public:
    static void InsUInt16(ww::bytes &rO, sal_uInt16 n);
    static void InsUInt32(ww::bytes &rO, sal_uInt32 n);
    static void InsAsString16(ww::bytes &rO, const OUString& rStr);
    static void InsAsString8(ww::bytes & O, const OUString& rStr,
        rtl_TextEncoding eCodeSet);

    static sal_uLong FillUntil( SvStream& rStrm, sal_uLong nEndPos = 0 );
    static void FillCount( SvStream& rStrm, sal_uLong nCount );

    static void WriteShort( SvStream& rStrm, sal_Int16 nVal ) { rStrm.WriteInt16( nVal ); }
    static void WriteShort( SvStream& rStrm, sal_uLong nPos, sal_Int16 nVal );

    static void WriteLong( SvStream& rStrm, sal_Int32 nVal ) { rStrm.WriteInt32( nVal ); }
    static void WriteLong( SvStream& rStrm, sal_uLong nPos, sal_Int32 nVal );

    static void WriteString16(SvStream& rStrm, const OUString& rStr,
        bool bAddZero);
    static void WriteString8(SvStream& rStrm, const OUString& rStr,
        bool bAddZero, rtl_TextEncoding eCodeSet);

    static void WriteString_xstz(SvStream& rStrm, const OUString& rStr, bool bAddZero);

    bool InitStd97CodecUpdateMedium( ::msfilter::MSCodec_Std97& rCodec );

    using StgWriter::Write;
    virtual ErrCode Write( SwPaM&, SfxMedium&, const OUString* ) override;
    //Seems not an expected to provide method to access the private member
    SfxMedium* GetMedia() { return mpMedium; }

private:
    SwWW8Writer(const SwWW8Writer&) = delete;
    SwWW8Writer& operator=(const SwWW8Writer&) = delete;
};

/// Exporter of the binary Word file formats.
class WW8Export : public MSWordExportBase
{
public:
    std::unique_ptr<ww::bytes> pO;      ///< Buffer

    SvStream *pTableStrm, *pDataStrm;   ///< Streams for WW97 Export

    std::unique_ptr<WW8Fib> pFib;                       ///< File Information Block
    std::unique_ptr<WW8Dop> pDop;                       ///< DOcument Properties
    std::unique_ptr<WW8_WrPlcFootnoteEdn> pFootnote;    ///< Footnotes - structure to remember them, and output
    std::unique_ptr<WW8_WrPlcFootnoteEdn> pEdn;         ///< Endnotes - structure to remember them, and output
    std::unique_ptr<WW8_WrPlcSepx> pSepx;               ///< Sections/headers/footers

    bool const m_bDot; ///< Template or document.

protected:
    SwWW8Writer        *m_pWriter;      ///< Pointer to the writer
    std::unique_ptr<WW8AttributeOutput> m_pAttrOutput;  ///< Converting attributes to stream data

private:
    tools::SvRef<SotStorage>       xEscherStg;      /// memory leak #i120098#, to hold the reference to unnamed SotStorage obj

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const override;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const override;

    virtual bool PreferPageBreakBefore() const override { return true; }

    virtual bool AllowPostponedTextInTable() const override { return false; }

    virtual bool SupportsOneColumnBreak() const override { return false; }

    virtual bool FieldsQuoted() const override { return false; }

    virtual bool AddSectionBreaksForTOX() const override { return false; }
private:
    /// Format-dependent part of the actual export.
    virtual ErrCode ExportDocument_Impl() override;

    void PrepareStorage();
    void WriteFkpPlcUsw();
    void WriteMainText();
    void StoreDoc1();

    /// Output the numbering table.
    virtual void WriteNumbering() override;

    void OutOverrideListTab();
    void OutListNamesTab();

    void RestoreMacroCmds();

    void DoComboBox(css::uno::Reference<css::beans::XPropertySet> const & xPropSet);

public:

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* pA ) override;

    bool MiserableFormFieldExportHack(const SwFrameFormat& rFrameFormat);

    SvxMSExportOLEObjects& GetOLEExp()      { return *m_pOLEExp; }
    SwMSConvertControls& GetOCXExp()        { return *m_pOCXExp; }
    void ExportDopTypography(WW8DopTypography &rTypo);

    sal_uInt16 AddRedlineAuthor( std::size_t nId );

    void WriteFootnoteBegin( const SwFormatFootnote& rFootnote, ww::bytes* pO = nullptr );
    void WritePostItBegin( ww::bytes* pO = nullptr );
    const SvxBrushItem* GetCurrentPageBgBrush() const;
    SvxBrushItem TrueFrameBgBrush(const SwFrameFormat &rFlyFormat) const;

    void AppendFlyInFlys(const ww8::Frame& rFrameFormat, const Point& rNdTopLeft);
    void WriteOutliner(const OutlinerParaObject& rOutliner, sal_uInt8 nTyp);
    void WriteSdrTextObj(const SdrTextObj& rObj, sal_uInt8 nTyp);

    sal_uInt32 GetSdrOrdNum( const SwFrameFormat& rFormat ) const;
    void CreateEscher();
    void WriteEscher();

    /// Write the field
    virtual void OutputField( const SwField* pField, ww::eField eFieldType,
            const OUString& rFieldCmd, FieldFlags nMode = FieldFlags::All ) override;

    void StartCommentOutput( const OUString& rName );
    void EndCommentOutput(   const OUString& rName );
    void OutGrf(const ww8::Frame &rFrame);
    bool TestOleNeedsGraphic(const SwAttrSet& rSet, tools::SvRef<SotStorage> const& xOleStg,
                             const tools::SvRef<SotStorage>& xObjStg, OUString const& rStorageName,
                             SwOLENode* pOLENd);

    virtual void AppendBookmarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen ) override;
    virtual void AppendBookmark( const OUString& rName ) override;
    void AppendBookmarkEndWithCorrection( const OUString& rName );

    virtual void AppendAnnotationMarks( const SwTextNode& rNd, sal_Int32 nCurrentPos, sal_Int32 nLen ) override;

    virtual void AppendSmartTags(SwTextNode& rTextNode) override;

    virtual void ExportGrfBullet(const SwTextNode& rNd) override;
    void OutGrfBullets(const ww8::Frame &rFrame);

    void MoveFieldMarks(WW8_CP nFrom, WW8_CP nTo);

    void WriteAsStringTable(const std::vector<OUString>&, sal_Int32& rfcSttbf,
        sal_Int32& rlcbSttbf);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) override;

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t() ) override;
    void WriteChar( sal_Unicode c ) override;

    void OutSwString(const OUString&, sal_Int32 nStt, sal_Int32 nLen);

    WW8_CP Fc2Cp( sal_uLong nFc ) const          { return m_pPiece->Fc2Cp( nFc ); }

            // some partly static semi-internal function declarations

    void OutSprmBytes( sal_uInt8* pBytes, sal_uInt16 nSiz )
                                { pO->insert( pO->end(), pBytes, pBytes+nSiz ); }

    virtual void SectionBreaksAndFrames( const SwTextNode& rNode ) override;

    /// Helper method for OutputSectionBreaks() and OutputFollowPageDesc().
    // #i76300#
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFormatPageDesc* pNewPgDescFormat,
                                     const SwPageDesc* pNewPgDesc ) override;

    static void Out_BorderLine(ww::bytes& rO, const ::editeng::SvxBorderLine* pLine,
        sal_uInt16 nDist, sal_uInt16 nSprmNo, sal_uInt16 nSprmNoVer9,
        bool bShadow);

    void Out_SwFormatBox(const SvxBoxItem& rBox, bool bShadow);
    static void Out_SwFormatTableBox( ww::bytes& rO, const SvxBoxItem * rBox );
    void Out_CellRangeBorders(const SvxBoxItem * pBox, sal_uInt8 nStart,
        sal_uInt8 nLimit);
    static bool TransBrush(const Color& rCol, WW8_SHD& rShd);
    static WW8_BRCVer9 TranslateBorderLine(const ::editeng::SvxBorderLine& pLine,
        sal_uInt16 nDist, bool bShadow);

    // #i77805# - new return value indicates, if an inherited outline numbering is suppressed
    virtual bool DisallowInheritingOutlineNumbering(const SwFormat &rFormat) override;

    unsigned int GetHdFtIndex() const { return m_nHdFtIndex; }
    void SetHdFtIndex(unsigned int nHdFtIndex) { m_nHdFtIndex = nHdFtIndex; }
    void IncrementHdFtIndex() { ++m_nHdFtIndex; }

    /// Convert the SVX numbering type to id
    static sal_uInt8 GetNumId( sal_uInt16 eNumType );

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) override;

    SwTwips CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const;

    /// Nasty swap for bidi if necessary
    void MiserableRTLFrameFormatHack(SwTwips &rLeft, SwTwips &rRight,
        const ww8::Frame &rFrameFormat);

    void InsUInt16( sal_uInt16 n )      { SwWW8Writer::InsUInt16( *pO, n ); }
    void InsInt16(sal_Int16 n) { InsUInt16(sal_uInt16(n)); }
    void InsUInt32( sal_uInt32 n )      { SwWW8Writer::InsUInt32( *pO, n ); }
    void WriteStringAsPara( const OUString& rText );

    /// Setup the exporter.
    WW8Export( SwWW8Writer *pWriter,
            SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
            bool bDot );
    virtual ~WW8Export() override;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    css::uno::Sequence<OUString> &rListItems) override;

    virtual void DoFormText(const SwInputField * pField) override;

    void GetCurrentItems(ww::bytes &rItems) const;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) override;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) override;

    /// Fields.
    WW8_WrPlcField* CurrentFieldPlc() const;

    SwWW8Writer& GetWriter() const { return *m_pWriter; }
    SvStream& Strm() const { return m_pWriter->Strm(); }

    /// Remember some of the members so that we can recurse in WriteText().
    virtual void SaveData( sal_uLong nStt, sal_uLong nEnd ) override;

    /// Restore what was saved in SaveData().
    virtual void RestoreData() override;

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrameFormat& rFormat, const SwFrameFormat& rLeftFormat, const SwFrameFormat& rFirstPageFormat,
        sal_uInt8 nBreakCode) override;

    virtual ExportFormat GetExportFormat() const override { return ExportFormat::DOC; }

protected:
    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) override;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) override;

    virtual void OutputLinkedOLE( const OUString& ) override;

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFormat* pFormat, sal_uLong nLnNum ) override;

private:
    WW8Export(const WW8Export&) = delete;
    WW8Export& operator=(const WW8Export&) = delete;
};

class WW8_WrPlcSubDoc   // double Plc for Footnotes/Endnotes and Postits
{
private:
    WW8_WrPlcSubDoc(const WW8_WrPlcSubDoc&) = delete;
    WW8_WrPlcSubDoc& operator=(const WW8_WrPlcSubDoc&) = delete;
protected:
    std::vector<WW8_CP> aCps;
    std::vector<const void*> aContent;                // PTRARR of SwFormatFootnote/PostIts/..
    std::vector<const SwFrameFormat*> aSpareFormats;  // a backup for aContent: if there's no SdrObject, stores the fmt directly here
    std::unique_ptr<WW8_WrPlc0> pTextPos;             // positions of the individual texts

    WW8_WrPlcSubDoc();
    virtual ~WW8_WrPlcSubDoc();

    bool WriteGenericText( WW8Export& rWrt, sal_uInt8 nTTyp, WW8_CP& rCount );
    void WriteGenericPlc( WW8Export& rWrt, sal_uInt8 nTTyp, WW8_FC& rTextStt,
        sal_Int32& rTextCnt, WW8_FC& rRefStt, sal_Int32& rRefCnt ) const;

    virtual const std::vector<sal_uInt32>* GetShapeIdArr() const;
};

// double Plc for Footnotes/Endnotes
class WW8_WrPlcFootnoteEdn : public WW8_WrPlcSubDoc
{
private:
    sal_uInt8 const nTyp;

    WW8_WrPlcFootnoteEdn(const WW8_WrPlcFootnoteEdn&) = delete;
    WW8_WrPlcFootnoteEdn& operator=(WW8_WrPlcFootnoteEdn const &) = delete;
public:
    explicit WW8_WrPlcFootnoteEdn( sal_uInt8 nTTyp ) : nTyp( nTTyp ) {}

    bool WriteText( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;

    void Append( WW8_CP nCp, const SwFormatFootnote& rFootnote );
};

struct WW8_Annotation
{
    const OutlinerParaObject* mpRichText;
    OUString msSimpleText;
    OUString msOwner;
    OUString m_sInitials;
    DateTime maDateTime;
    WW8_CP m_nRangeStart, m_nRangeEnd;
    WW8_Annotation(const SwPostItField* pPostIt, WW8_CP nRangeStart, WW8_CP nRangeEnd);
    explicit WW8_Annotation(const SwRedlineData* pRedline);
};

class WW8_WrPlcAnnotations : public WW8_WrPlcSubDoc  // double Plc for Postits
{
private:
    WW8_WrPlcAnnotations(const WW8_WrPlcAnnotations&) = delete;
    WW8_WrPlcAnnotations& operator=(WW8_WrPlcAnnotations const &) = delete;
    std::set<const SwRedlineData*> maProcessedRedlines;

    std::map<const OUString, WW8_CP> m_aRangeStartPositions;
public:
    WW8_WrPlcAnnotations() {}
    virtual ~WW8_WrPlcAnnotations() override;

    void AddRangeStartPosition(const OUString& rName, WW8_CP nStartCp);
    void Append( WW8_CP nCp, const SwPostItField* pPostIt );
    void Append( WW8_CP nCp, const SwRedlineData* pRedLine );
    bool IsNewRedlineComment( const SwRedlineData* pRedLine );
    bool WriteText( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
};

class WW8_WrPlcTextBoxes : public WW8_WrPlcSubDoc // double Plc for Textboxes
{                        // Frame/DrawTextboxes!
private:
    sal_uInt8 nTyp;
    std::vector<sal_uInt32> aShapeIds;        // VARARR of ShapeIds for the SwFrameFormats
    virtual const std::vector<sal_uInt32>* GetShapeIdArr() const override;

    WW8_WrPlcTextBoxes(const WW8_WrPlcTextBoxes&) = delete;
    WW8_WrPlcTextBoxes& operator=(WW8_WrPlcTextBoxes const &) = delete;
public:
    explicit WW8_WrPlcTextBoxes( sal_uInt8 nTTyp ) : nTyp( nTTyp ) {}

    bool WriteText( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
    void Append( const SdrObject& rObj, sal_uInt32 nShapeId );
    void Append( const SwFrameFormat* pFormat, sal_uInt32 nShapeId );
    sal_uInt16 Count() const { return aContent.size(); }
    sal_uInt16 GetPos( const void* p ) const
    {
        std::vector<const void*>::const_iterator it
            = std::find( aContent.begin(), aContent.end(), p );
        return it == aContent.end() ? USHRT_MAX : it - aContent.begin();
    }
};

class WW8_WrPlcPn                   // Plc for Page Numbers
{
private:
    WW8Export& rWrt;
    // Plc for Chpx and Papx ( incl PN-Plc )
    std::vector<std::unique_ptr<WW8_WrFkp>> m_Fkps;
    sal_uInt16 nFkpStartPage;
    ePLCFT ePlc;

    WW8_WrPlcPn(const WW8_WrPlcPn&) = delete;
    WW8_WrPlcPn& operator=(const WW8_WrPlcPn&) = delete;
public:
    WW8_WrPlcPn( WW8Export& rWrt, ePLCFT ePl, WW8_FC nStartFc );
    ~WW8_WrPlcPn();
    void AppendFkpEntry(WW8_FC nEndFc,short nVarLen = 0,const sal_uInt8* pSprms = nullptr);
    void WriteFkps();
    void WritePlc();
    sal_uInt8 *CopyLastSprms(sal_uInt8 &rLen);
};

// class WW8_WrPlc1 is only used for fields
class WW8_WrPlc1
{
private:
    std::vector<WW8_CP> aPos;
    std::unique_ptr<sal_uInt8[]> pData;                // content ( structures )
    sal_uLong nDataLen;
    sal_uInt16 const nStructSiz;

    WW8_WrPlc1(const WW8_WrPlc1&) = delete;
    WW8_WrPlc1& operator=(const WW8_WrPlc1&) = delete;
protected:
    sal_uInt16 Count() const { return aPos.size(); }
    void Write( SvStream& rStrm );
    WW8_CP Prev() const;
public:
    explicit WW8_WrPlc1( sal_uInt16 nStructSz );
    ~WW8_WrPlc1();
    void Append( WW8_CP nCp, const void* pData );
    void Finish( sal_uLong nLastCp, sal_uLong nStartCp );
};

// class WW8_WrPlcField is for fields
class WW8_WrPlcField : public WW8_WrPlc1
{
private:
    sal_uInt8 const nTextTyp;
    sal_uInt16 nResults;

    WW8_WrPlcField(const WW8_WrPlcField&) = delete;
    WW8_WrPlcField& operator=(const WW8_WrPlcField&) = delete;
public:
    WW8_WrPlcField( sal_uInt16 nStructSz, sal_uInt8 nTTyp )
        : WW8_WrPlc1( nStructSz ), nTextTyp( nTTyp ), nResults(0)
    {}
    void Write( WW8Export& rWrt );
    void ResultAdded() { ++nResults; }
    sal_uInt16 ResultCount() const { return nResults; }
};

class WW8_WrMagicTable : public WW8_WrPlc1
{
private:
    WW8_WrMagicTable(const WW8_WrMagicTable&) = delete;
    WW8_WrMagicTable& operator=(const WW8_WrMagicTable&) = delete;
public:
    WW8_WrMagicTable() : WW8_WrPlc1( 4 ) {Append(0,0);}
    void Append( WW8_CP nCp, sal_uLong nData );
    void Write( WW8Export& rWrt );
};

class GraphicDetails
{
public:
    ww8::Frame const maFly;                // surrounding FlyFrames
    sal_uLong mnPos;                // FilePos of the graphics
    sal_uInt16 const mnWid;               // Width of the graphics
    sal_uInt16 const mnHei;               // Height of the graphics

    GraphicDetails(const ww8::Frame &rFly, sal_uInt16 nWid, sal_uInt16 nHei)
        : maFly(rFly), mnPos(0), mnWid(nWid), mnHei(nHei)
    {}

    bool operator==(const GraphicDetails& rIn) const
    {
        return (
                 (mnWid == rIn.mnWid) && (mnHei == rIn.mnHei) &&
                 (maFly.RefersToSameFrameAs(rIn.maFly))
               );
    }
};

// class SwWW8WrGrf collects graphics and issues them
class SwWW8WrGrf
{
private:
    /// for access to the variables
    WW8Export& rWrt;

    std::vector<GraphicDetails> maDetails;
    sal_uInt16 mnIdx;       // index in file positions

    static void WritePICFHeader(SvStream& rStrm, const ww8::Frame &rFly,
            sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight,
            const SwAttrSet* pAttrSet = nullptr);
    void WriteGraphicNode(SvStream& rStrm, const GraphicDetails &rItem);
    void WriteGrfFromGrfNode(SvStream& rStrm, const SwGrfNode &rNd,
        const ww8::Frame &rFly, sal_uInt16 nWidth, sal_uInt16 nHeight);

    static void WritePICBulletFHeader(SvStream& rStrm, const Graphic &rGrf, sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight);
    void WriteGrfForBullet(SvStream& rStrm,  const Graphic &rGrf, sal_uInt16 nWidth, sal_uInt16 nHeight);

    SwWW8WrGrf(const SwWW8WrGrf&) = delete;
    SwWW8WrGrf& operator=(const SwWW8WrGrf&) = delete;
public:
    explicit SwWW8WrGrf( WW8Export& rW ) : rWrt( rW ), mnIdx( 0 ) {}
    void Insert(const ww8::Frame &rFly);
    void Write();
    sal_uLong GetFPos()
        { return (mnIdx < maDetails.size()) ? maDetails[mnIdx++].mnPos : 0; }
};

/** The class MSWordAttrIter is a helper class to build the Fkp.chpx.
    This is a base class to output the SwTextAttrs and the EditEngineTextAttrs.
*/
class MSWordAttrIter
{
private:
    MSWordAttrIter* const pOld;
    MSWordAttrIter(const MSWordAttrIter&) = delete;
    MSWordAttrIter& operator=(const MSWordAttrIter&) = delete;
protected:
    MSWordExportBase& m_rExport;
public:
    explicit MSWordAttrIter( MSWordExportBase& rExport );
    virtual ~MSWordAttrIter();

    virtual const SfxPoolItem* HasTextItem( sal_uInt16 nWhich ) const = 0;
    virtual const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const = 0;
 };

/// Used to export formatted text associated to drawings.
class MSWord_SdrAttrIter : public MSWordAttrIter
{
private:
    const EditTextObject* pEditObj;
    const SfxItemPool* pEditPool;
    std::vector<EECharAttrib> aTextAtrArr;
    std::vector<const EECharAttrib*> aChrTextAtrArr;
    std::vector<rtl_TextEncoding> aChrSetArr;
    sal_Int32 nPara;
    sal_Int32 nCurrentSwPos;
    sal_Int32 nTmpSwPos;                   // for HasItem()
    rtl_TextEncoding eNdChrSet;
    sal_uInt16 nScript;
    sal_uInt8 const mnTyp;

    sal_Int32 SearchNext( sal_Int32 nStartPos );
    void SetCharSet(const EECharAttrib& rTextAttr, bool bStart);

    void SetItemsThatDifferFromStandard(bool bCharAttr, SfxItemSet& rSet);

    MSWord_SdrAttrIter(const MSWord_SdrAttrIter&) = delete;
    MSWord_SdrAttrIter& operator=(const MSWord_SdrAttrIter&) = delete;
public:
    MSWord_SdrAttrIter( MSWordExportBase& rWr, const EditTextObject& rEditObj,
        sal_uInt8 nType );
    void NextPara( sal_Int32 nPar );
    void OutParaAttr(bool bCharAttr, const std::set<sal_uInt16>* pWhichsToIgnore = nullptr);
    void OutEEField(const SfxPoolItem& rHt);

    bool IsTextAttr(sal_Int32 nSwPos);

    void NextPos() { if ( nCurrentSwPos < SAL_MAX_INT32 ) nCurrentSwPos = SearchNext( nCurrentSwPos + 1 ); }

    void OutAttr( sal_Int32 nSwPos );
    virtual const SfxPoolItem* HasTextItem( sal_uInt16 nWhich ) const override;
    virtual const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const override;
    sal_Int32 WhereNext() const                { return nCurrentSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const     { return eNdChrSet; }
};

// class SwWW8AttrIter is a helper for constructing the Fkp.chpx.
// Only character attributes are considered; paragraph attributes do not need this treatment.
// The paragraph and text attributes of the Writer are passed, and
// Where() returns the next position where the attributes change.
// IsTextAtr() tells if, at the position returned by Where(), there is
// an attribute without end and with \xff in the text.
// Using OutAttr(), the attributes on the passed SwPos are returned.
class SwWW8AttrIter : public MSWordAttrIter
{
private:
    const SwTextNode& rNd;

    sw::util::CharRuns maCharRuns;
    sw::util::CharRuns::const_iterator maCharRunIter;

    rtl_TextEncoding meChrSet;
    sal_uInt16 mnScript;
    bool mbCharIsRTL;

    const SwRangeRedline* pCurRedline;
    sal_Int32 nCurrentSwPos;
    SwRedlineTable::size_type nCurRedlinePos;

    bool mbParaIsRTL;

    const SwFormatDrop &mrSwFormatDrop;

    ww8::Frames maFlyFrames;     // #i2916#
    ww8::FrameIter maFlyIter;

    sal_Int32 SearchNext( sal_Int32 nStartPos );

    void OutSwFormatRefMark(const SwFormatRefMark& rAttr);

    void IterToCurrent();

    SwWW8AttrIter(const SwWW8AttrIter&) = delete;
    SwWW8AttrIter& operator=(const SwWW8AttrIter&) = delete;
public:
    SwWW8AttrIter( MSWordExportBase& rWr, const SwTextNode& rNd );

    bool IsTextAttr( sal_Int32 nSwPos ) const;
    bool IsExportableAttr(sal_Int32 nSwPos) const;
    bool IncludeEndOfParaCRInRedlineProperties(sal_Int32 nPos) const;
    bool IsDropCap( int nSwPos );
    bool RequiresImplicitBookmark();

    void NextPos() { if ( nCurrentSwPos < SAL_MAX_INT32 ) nCurrentSwPos = SearchNext( nCurrentSwPos + 1 ); }

    void OutAttr( sal_Int32 nSwPos, bool bWriteCombinedChars );
    virtual const SfxPoolItem* HasTextItem( sal_uInt16 nWhich ) const override;
    virtual const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const override;
    int OutAttrWithRange(const SwTextNode& rNode, sal_Int32 nPos);
    const SwRedlineData* GetParagraphLevelRedline( );
    const SwRedlineData* GetRunLevelRedline( sal_Int32 nPos );
    FlyProcessingState OutFlys(sal_Int32 nSwPos);

    sal_Int32 WhereNext() const { return nCurrentSwPos; }
    sal_uInt16 GetScript() const { return mnScript; }
    bool IsParaRTL() const { return mbParaIsRTL; }
    rtl_TextEncoding GetCharSet() const { return meChrSet; }
    OUString GetSnippet(const OUString &rStr, sal_Int32 nCurrentPos,
        sal_Int32 nLen) const;
    const SwFormatDrop& GetSwFormatDrop() const { return mrSwFormatDrop; }

    bool IsWatermarkFrame();
    bool IsAnchorLinkedToThisNode( sal_uLong nNodePos );

    void SplitRun( sal_Int32 nSplitEndPos );
};

/// Class to collect and output the styles table.
class MSWordStyles
{
    MSWordExportBase& m_rExport;
    sal_uInt16 m_aHeadingParagraphStyles[MAXLEVEL];
    std::unique_ptr<SwFormat*[]> m_pFormatA; ///< Slot <-> Character and paragraph style array (0 for list styles).
    sal_uInt16 m_nUsedSlots;
    bool const m_bListStyles; ///< If list styles are requested to be exported as well.
    std::map<sal_uInt16, const SwNumRule*> m_aNumRules; ///< Slot <-> List style map.

    /// We need to build style id's for DOCX export; ideally we should roundtrip that, but this is good enough.
    std::vector<OString> m_aStyleIds;

    /// Create the style table, called from the constructor.
    void BuildStylesTable();

    /// Based on pFormatA, fill in m_aStyleIds with unique, MS-like names.
    void BuildStyleIds();

    /// Get slot number during building the style table.
    sal_uInt16 BuildGetSlot( const SwFormat& rFormat );
    sal_uInt16 BuildGetSlot( const SwNumRule& /*rNumRule*/ ) { return m_nUsedSlots++;}

    /// Return information about one style.
    void GetStyleData( SwFormat* pFormat, bool& bFormatColl, sal_uInt16& nBase, sal_uInt16& nNext );

    /// Outputs attributes of one style.
    void WriteProperties( const SwFormat* pFormat, bool bPap, sal_uInt16 nPos, bool bInsDefCharSiz );

    static sal_uInt16 GetWWId( const SwFormat& rFormat );

    void SetStyleDefaults( const SwFormat& rFormat, bool bPap );

    /// Outputs one style - called (in a loop) from OutputStylesTable().
    void OutputStyle( SwFormat* pFormat, sal_uInt16 nPos );
    void OutputStyle( const SwNumRule* pNumRule, sal_uInt16 nPos );

    MSWordStyles( const MSWordStyles& ) = delete;
    MSWordStyles& operator=( const MSWordStyles& ) = delete;

public:
    MSWordStyles( MSWordExportBase& rExport, bool bListStyles = false );
    ~MSWordStyles();

    /// Output the styles table.
    void OutputStylesTable();

    /// Get id of the style (rFormat).
    sal_uInt16 GetSlot( const SwFormat* pFormat ) const;

    /// Get styleId of the nId-th style (nId is its position in pFormatA).
    OString const & GetStyleId(sal_uInt16 nId) const;

    const SwFormat* GetSwFormat(sal_uInt16 nId) const { return m_pFormatA[nId]; }
    /// Get numbering rule of the nId-th style
    const SwNumRule* GetSwNumRule(sal_uInt16 nId) const;
    sal_uInt16 GetHeadingParagraphStyleId(sal_uInt16 nLevel) const { return m_aHeadingParagraphStyles[ nLevel ]; }
};

#define MSWORD_MAX_STYLES_LIMIT 4091

sal_Int16 GetWordFirstLineOffset(const SwNumFormat &rFormat);
// A bit of a bag on the side for now
OUString FieldString(ww::eField eIndex);
OUString BookmarkToWord(const OUString &rBookmark);

class WW8SHDLong
{
    sal_uInt32 m_cvFore;
    sal_uInt32 m_cvBack;

public:
    WW8SHDLong() : m_cvFore(0), m_cvBack(0) {}

    void Write(WW8Export & rExport);
    void setCvFore(sal_uInt32 cvFore) { m_cvFore = cvFore; }
    void setCvBack(sal_uInt32 cvBack) { m_cvBack = cvBack; }
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_WRTWW8_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
