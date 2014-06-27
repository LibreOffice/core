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
#include <boost/optional.hpp>

#include <map>
#include <vector>

#include <shellio.hxx>
#include <wrt_fn.hxx>

#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "fields.hxx"
#include "types.hxx"
#include "writerhelper.hxx"
#include "../inc/msfilter.hxx"
#include <expfld.hxx>

#include <vcl/graph.hxx>
class SvxBrushItem;

// einige Forward Deklarationen
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
class Font;
class MSWordExportBase;
class SdrObject;
class SfxItemSet;
class SvStream;
class SvxFontItem;
class SvxBoxItem;
class SwAttrSet;
class SwCharFmt;
class SwCntntNode;
class SwField;
class SwFmt;
class SwFmtCntnt;
class SwFmtFtn;
class SwFrmFmt;
class SwGrfNode;
class SwModify;
class SwNumFmt;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwFmtPageDesc;
class SwOLENode;
class SwPostItField;
class SwRedlineData;
class SwSectionFmt;
class SwSectionNode;
class SwTableNode;
class SwTOXType;
class SwTxtFmtColl;
class SwTxtNode;
class SwWW8WrGrf;
class SwWW8Writer;
class MSWordStyles;
class WW8AttributeOutput;
class WW8Export;
class MSWordAttrIter;
class WW8_WrFkp;
class WW8_WrPlc0;
class WW8_WrPlc1;
class WW8_WrPlcFld;
class WW8_WrMagicTable;
class WW8_WrPlcFtnEdn;
class WW8_WrPlcPn;
class WW8_WrPlcAnnotations;
class MSWordSections;
class WW8_WrPlcTxtBoxes;
class WW8_WrPct;            // Verwaltung
class WW8_WrtBookmarks;
class WW8_WrtRedlineAuthor;
class SvxMSExportOLEObjects;
class SwMSConvertControls;
class WW8_WrPc;

namespace com { namespace sun { namespace star { namespace embed {
class XEmbeddedObject;
} } } }
typedef std::map<const com::sun::star::embed::XEmbeddedObject*, sal_Int32> WW8OleMap;
struct WW8_PdAttrDesc;
class SvxBrushItem;

#include "WW8TableInfo.hxx"

#define GRF_MAGIC_1 0x12    // 3 magic Bytes fuer PicLocFc-Attribute
#define GRF_MAGIC_2 0x34
#define GRF_MAGIC_3 0x56
#define GRF_MAGIC_321 0x563412L

#define OLE_PREVIEW_AS_EMF  //If we want to export ole2 previews as emf in ww8+

typedef sal_uInt8 FieldFlags;
namespace nsFieldFlags // for InsertField- Method
{
    const FieldFlags WRITEFIELD_START         = 0x01;
    const FieldFlags WRITEFIELD_CMD_START     = 0x02;
    const FieldFlags WRITEFIELD_CMD_END     = 0x04;
    const FieldFlags WRITEFIELD_END         = 0x10;
    const FieldFlags WRITEFIELD_CLOSE         = 0x20;
    const FieldFlags WRITEFIELD_ALL         = 0xFF;
}

enum TxtTypes  //enums for TextTypes
{
    TXT_MAINTEXT = 0, /*TXT_FTNEDN = 1,*/ TXT_HDFT = 2, TXT_FTN = 3,
    TXT_EDN = 4, TXT_ATN = 5, TXT_TXTBOX = 6, TXT_HFTXTBOX= 7
};

struct WW8_SepInfo
{
    const SwPageDesc* pPageDesc;
    const SwSectionFmt* pSectionFmt;
    const SwNode* pPDNd;
    const SwTxtNode* pNumNd;
    sal_uLong  nLnNumRestartNo;
    ::boost::optional<sal_uInt16> oPgRestartNo;

    WW8_SepInfo()
        : pPageDesc(0), pSectionFmt(0), pPDNd(0), pNumNd(0), nLnNumRestartNo(0)

    {}

    WW8_SepInfo( const SwPageDesc* pPD, const SwSectionFmt* pFmt,
        sal_uLong nLnRestart, ::boost::optional<sal_uInt16> oPgRestart = boost::none, const SwNode* pNd = NULL )
        : pPageDesc( pPD ), pSectionFmt( pFmt ), pPDNd( pNd ), pNumNd( 0 ),
          nLnNumRestartNo( nLnRestart ), oPgRestartNo( oPgRestart )
    {}

    bool IsProtected() const;
};

/// Class to collect and output the sections/headers/footers.
// Plc fuer PageDescs -> Sepx ( Section Extensions )
class MSWordSections
{
protected:
    bool mbDocumentIsProtected;
    std::vector<WW8_SepInfo> aSects;

    void CheckForFacinPg( WW8Export& rWrt ) const;
    void WriteOlst( WW8Export& rWrt, const WW8_SepInfo& rSectionInfo );
    void NeedsDocumentProtected(const WW8_SepInfo &rInfo);

    //No copy, no assign
    MSWordSections( const MSWordSections& );
    MSWordSections& operator=( const MSWordSections& );
public:
    MSWordSections( MSWordExportBase& rExport );
    virtual ~MSWordSections();

    virtual bool HeaderFooterWritten();

    void AppendSection( const SwPageDesc* pPd,
                    const SwSectionFmt* pSectionFmt = 0,
                    sal_uLong nLnNumRestartNo = 0 );
    void AppendSection( const SwFmtPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFmt* pSectionFmt,
                    sal_uLong nLnNumRestartNo );
    void SetNum( const SwTxtNode* pNumNd );

    /// Number of columns based on the most recent WW8_SepInfo.
    sal_uInt16 CurrentNumberOfColumns( const SwDoc &rDoc ) const;

    /// Number of columns of the provided WW8_SepInfo.
    sal_uInt16 NumberOfColumns( const SwDoc &rDoc, const WW8_SepInfo& rInfo ) const;

    bool DocumentIsProtected() const { return mbDocumentIsProtected; }

    /// The most recent WW8_SepInfo.
    const WW8_SepInfo* CurrentSectionInfo();

    static void SetHeaderFlag( sal_uInt8& rHeadFootFlags, const SwFmt& rFmt,
                                  sal_uInt8 nFlag );
    static void SetFooterFlag( sal_uInt8& rHeadFootFlags, const SwFmt& rFmt,
                                   sal_uInt8 nFlag );

    /// Should we output borders?
    static bool HasBorderItem( const SwFmt& rFmt );
};

class WW8_WrPlcSepx : public MSWordSections
{
    std::vector<WW8_CP> aCps;
    ::std::vector< ::boost::shared_ptr<WW8_PdAttrDesc> > m_SectionAttributes;
    // hack to prevent adding sections in endnotes
    bool m_bHeaderFooterWritten;
    WW8_WrPlc0* pTxtPos;        // Pos der einzelnen Header / Footer

    // No copy, no assign
    WW8_WrPlcSepx( const WW8_WrPlcSepx& );
    WW8_WrPlcSepx& operator=( const WW8_WrPlcSepx& );

public:
    WW8_WrPlcSepx( MSWordExportBase& rExport );
    virtual ~WW8_WrPlcSepx();

    virtual bool HeaderFooterWritten() SAL_OVERRIDE; // override

    void AppendSep( WW8_CP nStartCp,
                    const SwPageDesc* pPd,
                    const SwSectionFmt* pSectionFmt = 0,
                    sal_uLong nLnNumRestartNo = 0 );
    void AppendSep( WW8_CP nStartCp, const SwFmtPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFmt* pSectionFmt,
                    sal_uLong nLnNumRestartNo );
    void Finish( WW8_CP nEndCp ) { aCps.push_back( nEndCp ); }

    bool WriteKFTxt( WW8Export& rWrt );
    void WriteSepx( SvStream& rStrm ) const;
    void WritePlcSed( WW8Export& rWrt ) const;
    void WritePlcHdd( WW8Export& rWrt ) const;

private:
    void WriteFtnEndTxt( WW8Export& rWrt, sal_uLong nCpStt );
public:
    void OutHeaderFooter(WW8Export& rWrt, bool bHeader,
            const SwFmt& rFmt, sal_uLong& rCpPos, sal_uInt8 nHFFlags, sal_uInt8 nFlag,  sal_uInt8 nBreakCode);
};

// class WW8_WrPct to construct the piece table
class WW8_WrPct
{
    boost::ptr_vector<WW8_WrPc > aPcts;
    WW8_FC nOldFc;
    bool bIsUni;
public:
    WW8_WrPct(WW8_FC nStartFc, bool bSaveUniCode);
    ~WW8_WrPct();
    void AppendPc(WW8_FC nStartFc, bool bIsUnicode);
    void WritePc( WW8Export& rWrt );
    void SetParaBreak();
    bool IsUnicode() const  { return bIsUni; }
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
    bool mbWrtWW8;
    FontPitch mePitch;
    FontFamily meFamily;
    rtl_TextEncoding meChrSet;
public:
    wwFont( const OUString &rFamilyName, FontPitch ePitch, FontFamily eFamily,
        rtl_TextEncoding eChrSet, bool bWrtWW8 );
    bool Write( SvStream *pTableStram ) const;
    void WriteDocx( DocxAttributeOutput* rAttrOutput ) const;
    void WriteRtf( const RtfAttributeOutput* rAttrOutput ) const;
    OUString GetFamilyName() const { return OUString( msFamilyNm ); }
    friend bool operator < (const wwFont &r1, const wwFont &r2);
};

class wwFontHelper
{
private:
    /// Keep track of fonts that need to be exported.
    ::std::map<wwFont, sal_uInt16> maFonts;
    bool mbWrtWW8;

    /// Convert from fast insertion map to linear vector in the order that we want to write.
    ::std::vector< const wwFont* > AsVector() const;

public:
    wwFontHelper() : mbWrtWW8(false), bLoadAllFonts(false) {}
    /// rDoc used only to get the initial standard font(s) in use.
    void InitFontTable(bool bWrtWW8, const SwDoc& rDoc);
    sal_uInt16 GetId(const Font& rFont);
    sal_uInt16 GetId(const SvxFontItem& rFont);
    sal_uInt16 GetId(const wwFont& rFont);
    void WriteFontTable( SvStream *pTableStream, WW8Fib& pFib );
    void WriteFontTable( DocxAttributeOutput& rAttrOutput );
    void WriteFontTable( const RtfAttributeOutput& rAttrOutput );

    /// If true, all fonts are loaded before processing the document.
    sal_uInt8 bLoadAllFonts: 1;
};

class DrawObj
{
public:
    WW8_CP mnCp;                // CP-Pos of references
    sal_uInt32 mnShapeId;       // ShapeId for the SwFrmFmts
    sw::Frame maCntnt;          // the frame itself
    Point maParentPos;          // Points
    sal_Int32 mnThick;          // Border Thicknesses
    short mnDirection;          // If BiDi or not
    unsigned int mnHdFtIndex;   // 0 for main text, +1 for each subsequent
                                // msword hd/ft

    DrawObj(const sw::Frame &rCntnt, WW8_CP nCp, Point aParentPos, short nDir,
            unsigned int nHdFtIndex)
        : mnCp(nCp), mnShapeId(0), maCntnt(rCntnt), maParentPos(aParentPos),
        mnThick(0), mnDirection(nDir), mnHdFtIndex(nHdFtIndex) {}
    void SetShapeDetails(sal_uInt32 nId, sal_Int32 nThick);
    DrawObj& operator=(const DrawObj &rOther);
};

typedef std::vector<DrawObj> DrawObjVector;
typedef DrawObjVector::iterator DrawObjIter;
typedef DrawObjVector::const_iterator cDrawObjIter;

typedef std::vector<DrawObj *> DrawObjPointerVector;
typedef DrawObjPointerVector::iterator DrawObjPointerIter;

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
    bool Append( WW8Export&, WW8_CP nCp, const sw::Frame& rFmt,
        const Point& rNdTopLeft );
    int size() { return maDrawObjs.size(); };
    DrawObjVector &GetObjArr() { return maDrawObjs; }
    virtual ~PlcDrawObj();
private:
    //No copying
    PlcDrawObj(const PlcDrawObj&);
    PlcDrawObj& operator=(const PlcDrawObj&);
};

class MainTxtPlcDrawObj : public PlcDrawObj
{
public:
    MainTxtPlcDrawObj() {}
private:
    virtual void RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
        sal_uInt32 nLen) const SAL_OVERRIDE;
    virtual WW8_CP GetCpOffset(const WW8Fib &) const SAL_OVERRIDE;
private:
    //No copying
    MainTxtPlcDrawObj(const MainTxtPlcDrawObj&);
    MainTxtPlcDrawObj& operator=(const MainTxtPlcDrawObj&);
};

class HdFtPlcDrawObj : public PlcDrawObj
{
public:
    HdFtPlcDrawObj() {}
private:
    virtual void RegisterWithFib(WW8Fib &rFib, sal_uInt32 nStart,
        sal_uInt32 nLen) const SAL_OVERRIDE;
    virtual WW8_CP GetCpOffset(const WW8Fib &rFib) const SAL_OVERRIDE;
private:
    //No copying
    HdFtPlcDrawObj(const HdFtPlcDrawObj&);
    HdFtPlcDrawObj& operator=(const HdFtPlcDrawObj&);
};

typedef ::std::pair<OUString, sal_uLong> aBookmarkPair;
typedef std::vector<aBookmarkPair> SwImplBookmarks;
typedef std::vector<aBookmarkPair>::iterator SwImplBookmarksIter;

class WW8_WrtRedlineAuthor : public sw::util::WrtRedlineAuthor
{
    public:
    virtual void Write(Writer &rWrt) SAL_OVERRIDE;
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
    ww::bytes* pOOld;                ///< WW8Export only
    SwPaM* pOldPam, *pOldEnd;
    const sw::Frame* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;

    sal_uInt8 bOldWriteAll : 1;          ///< WW8Export only
    sal_uInt8 bOldOutTable : 1;
    sal_uInt8 bOldIsInTable: 1;
    sal_uInt8 bOldFlyFrmAttrs : 1;
    sal_uInt8 bOldStartTOX : 1;
    sal_uInt8 bOldInWriteTOX : 1;
    // bOutPageDesc muss nicht gesichert werden, da es nur nicht waehrend der
    // Ausgabe von Spezial-Texten veraendert wird.
};

/// Base class for WW8Export and DocxExport
class MSWordExportBase
{
public:
    wwFontHelper maFontHelper;
    std::vector<sal_uLong> maChapterFieldLocs;
    typedef std::vector<sal_uLong>::const_iterator mycCFIter;
    OUString aMainStg;
    std::vector<const SwTOXType*> aTOXArr;
    const SfxItemSet* pISet;    // fuer Doppel-Attribute
    WW8_WrPct*  pPiece;         // Pointer auf Piece-Table
    SwNumRuleTbl* pUsedNumTbl;  // alle used NumRules
    const SwTxtNode *mpTopNodeOfHdFtPage; ///< Top node of host page when in hd/ft
    std::map< sal_uInt16, sal_uInt16 > aRuleDuplicates; //map to Duplicated numrules
    std::stack< sal_Int32 > m_aCurrentCharPropStarts; ///< To remember the position in a run.
    WW8_WrtBookmarks* pBkmks;
    WW8_WrtRedlineAuthor* pRedlAuthors;
    BitmapPalette* pBmpPal;
    boost::shared_ptr<NfKeywordTable> pKeyMap;
    SvxMSExportOLEObjects* pOLEExp;
    SwMSConvertControls* pOCXExp;
    WW8OleMap m_aOleMap;    // To remember all already exported ole objects
    ww8::WW8TableInfo::Pointer_t mpTableInfo;

    sal_uInt16 nCharFmtStart;
    sal_uInt16 nFmtCollStart;
    sal_uInt16 nStyleBeforeFly;     ///< Style-Nummer des Nodes,
                                ///<       in/an dem ein Fly verankert ist
    sal_uInt16 nLastFmtId;          ///< Style of last TxtNode in normal range
    sal_uInt16 nUniqueList;         ///< current number for creating unique list names
    unsigned int mnHdFtIndex;

    sal_uInt16 mnRedlineMode;       ///< Remember the original redline mode

public:
    /* implicit bookmark vector containing pairs of node indexes and bookmark names */
    SwImplBookmarks maImplicitBookmarks;
    sw::Frames maFrames;             // The floating frames in this document
    const SwPageDesc *pAktPageDesc;
    bool bPrevTextNodeIsEmpty;
    WW8_WrPlcPn* pPapPlc;
    WW8_WrPlcPn* pChpPlc;
    MSWordAttrIter* pChpIter;
    MSWordStyles* pStyles;
    WW8_WrPlcAnnotations* pAtn;
    WW8_WrPlcTxtBoxes *pTxtBxs, *pHFTxtBxs;

    const sw::Frame *mpParentFrame; // If set we are exporting content inside
                                    // a frame, e.g. a graphic node

    Point* pFlyOffset;              // zur Justierung eines im Writer als
    RndStdIds eNewAnchorType;       // Zeichen gebundenen Flys, der im WW
                                    // Absatzgebunden wird.

    WW8_WrPlcFld* pFldMain;         // fields in MainText
    WW8_WrPlcFld* pFldHdFt;         // fields in Header/Footer
    WW8_WrPlcFld* pFldFtn;          // fields in FootNotes
    WW8_WrPlcFld* pFldEdn;          // fields in EndNotes
    WW8_WrPlcFld* pFldAtn;          // fields in Annotations
    WW8_WrPlcFld* pFldTxtBxs;       // fields in textboxes
    WW8_WrPlcFld* pFldHFTxtBxs;     // fields in header/footer textboxes
    WW8_WrMagicTable *pMagicTable;  // keeps track of table cell positions, and
                                    // marks those that contain graphics,
                                    // which is required to make word display
                                    // graphics inside tables
    SwWW8WrGrf* pGrf;
    const SwAttrSet* pStyAttr;      // StyleAttr for Tabs
    const SwModify* pOutFmtNode;    // write Format or Node
    const SwFmt *pCurrentStyle;     // iff bStyDef=true, then this store the current style

    MainTxtPlcDrawObj *pSdrObjs;   // Draw-/Fly-Objects
    HdFtPlcDrawObj *pHFSdrObjs;     // Draw-/Fly-Objects in header or footer

    SwEscherEx* pEscher;            // escher export class
    // #i43447# - removed
//    SwTwips nFlyWidth, nFlyHeight;  // Fuer Anpassung Graphic

    sal_uInt8 nTxtTyp;

    bool bStyDef : 1;           // should Style be written?
    bool bBreakBefore : 1;      // Breaks are being written 2 times
    bool bOutKF : 1;            // Header/Footer texts are being written
    bool bOutFlyFrmAttrs : 1;   // Frame-attr of Flys are being written
    bool bOutPageDescs : 1;     ///< PageDescs (section properties) are being written
    bool bOutFirstPage : 1;     // write Attrset of FirstPageDesc
    bool bOutTable : 1;         // table is being written
                                     // ( wird zB bei Flys in Tabelle zurueckgesetzt )
    bool bOutGrf : 1;           // graphics are being written
    bool bInWriteEscher : 1;    // in write textboxes
    bool bStartTOX : 1;         // true: a TOX is startet
    bool bInWriteTOX : 1;       // true: all content are in a TOX
    bool bFtnAtTxtEnd : 1;      // true: all FTN at Textend
    bool bEndAtTxtEnd : 1;      // true: all END at Textend
    bool bHasHdr : 1;
    bool bHasFtr : 1;
    bool bSubstituteBullets : 1; // true: SubstituteBullet() gets called
    bool bTabInTOC : 1; //true for TOC field flag 'w'

    bool bHideTabLeaderAndPageNumbers : 1 ; // true: the 'z' field of TOC is set.
    bool mbExportModeRTF;
    bool mbOutOutlineOnly;   // export outline nodes, only (send outline to clipboard/presentation)

    SwDoc *pDoc;
    SwPaM *pCurPam, *pOrigPam;

    /// Stack to remember the nesting (see MSWordSaveData for more)
    ::std::stack< MSWordSaveData > maSaveData;

    /// Used to split the runs according to the bookmarks start and ends
    typedef std::vector< ::sw::mark::IMark* > IMarkVector;
    IMarkVector m_rSortedBookmarksStart;
    IMarkVector m_rSortedBookmarksEnd;
    IMarkVector m_rSortedAnnotationMarksStart;
    IMarkVector m_rSortedAnnotationMarksEnd;

public:
    /// The main function to export the document.
    void ExportDocument( bool bWriteAll );

    /// Iterate through the nodes and call the appropriate OutputNode() on them.
    void WriteText();

    /// Return whether cuurently exported node is in table.
    bool IsInTable() const;

    /// Set the pCurPam appropriately and call WriteText().
    ///
    /// Used to export paragraphs in footnotes/endnotes/etc.
    void WriteSpecialText( sal_uLong nStart, sal_uLong nEnd, sal_uInt8 nTTyp );

    /// Export the pool items to attributes (through an attribute output class).
    void ExportPoolItemsToCHP( sw::PoolItems &rItems, sal_uInt16 nScript );

    /// Return the numeric id of the numbering rule
    sal_uInt16 GetId( const SwNumRule& rNumRule );

    /// Return the numeric id of the style.
    sal_uInt16 GetId( const SwTxtFmtColl& rColl ) const;

    /// Return the numeric id of the style.
    sal_uInt16 GetId( const SwCharFmt& rFmt ) const;

    sal_uInt16 GetId( const SwTOXType& rTOXType );

    /// Return the numeric id of the font (and add it to the font list if needed)
    sal_uInt16 GetId( const SvxFontItem& rFont)
    {
        return maFontHelper.GetId(rFont);
    }
    /// @overload
    sal_uInt16 GetId( const wwFont& rFont)
    {
        return maFontHelper.GetId(rFont);
    }

    const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const;

    /// Find the reference.
    bool HasRefToObject( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo );

    /// Find the bookmark name.
    OUString GetBookmarkName( sal_uInt16 nTyp, const OUString* pName, sal_uInt16 nSeqNo );

    /// Add a bookmark converted to a Word name.
    void AppendWordBookmark( const OUString& rName );

    /// Use OutputItem() on an item set according to the parameters.
    void OutputItemSet( const SfxItemSet& rSet, bool bPapFmt, bool bChpFmt, sal_uInt16 nScript, bool bExportParentItemSet );

    short GetDefaultFrameDirection( ) const;

    /// Right to left?
    short TrueFrameDirection( const SwFrmFmt& rFlyFmt ) const;

    /// Right to left?
    short GetCurrentPageDirection() const;

    /// In case of numbering restart.

    /// List is set to restart at a particular value so for export make a
    /// completely new list based on this one and export that instead,
    /// which duplicates words behaviour in this respect.
    sal_uInt16 DuplicateNumRule( const SwNumRule *pRule, sal_uInt8 nLevel, sal_uInt16 nVal );

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const = 0;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const = 0;

    /// Determines if the format is expected to support unicode.
    virtual bool SupportsUnicode() const = 0;

    /// Determines if column break with one column should be exported or not.
    virtual bool SupportsOneColumnBreak() const = 0;

    /// Determines if the import filter already quoted fields or not.
    virtual bool FieldsQuoted() const = 0;

    /// Determines the Section Breaks are to be added for TOX Section
    virtual bool AddSectionBreaksForTOX() const = 0;

    /// Used to filter out attributes that can be e.g. written to .doc but not to .docx
    virtual bool ignoreAttributeForStyles( sal_uInt16 /*nWhich*/ ) const { return false; }

    /// If saving page break is preferred as a paragraph attribute (yes) or as a special character (no).
    virtual bool PreferPageBreakBefore() const = 0;

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

    virtual void AppendBookmarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen ) = 0;

    virtual void AppendBookmark( const OUString& rName, bool bSkip = false ) = 0;

    virtual void AppendAnnotationMarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen ) = 0;

    //For i120928,add this interface to export graphic of bullet
    virtual void ExportGrfBullet(const SwTxtNode& rNd) = 0;

    // FIXME probably a hack...
    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t() ) = 0;

    // FIXME definitely a hack, must not be here - it can't do anything
    // sensible for docx
    virtual void WriteChar( sal_Unicode c ) = 0;

    /// Output attributes.
    void OutputFormat( const SwFmt& rFmt, bool bPapFmt, bool bChpFmt, bool bFlyFmt = false );

    /// Getter for pISet.
    const SfxItemSet* GetCurItemSet() const { return pISet; }

    /// Setter for pISet.
    void SetCurItemSet( const SfxItemSet* pS ) { pISet = pS; }

    /// Remember some of the memebers so that we can recurse in WriteText().
    virtual void SaveData( sal_uLong nStt, sal_uLong nEnd );

    /// Restore what was saved in SaveData().
    virtual void RestoreData();

    /// The return value indicates, if a follow page desc is written.
    bool OutputFollowPageDesc( const SfxItemSet* pSet,
                               const SwTxtNode* pNd );

    /// Write header/footer text.
    void WriteHeaderFooterText( const SwFmt& rFmt, bool bHeader);

    /// Format of the section.
    const SwSectionFmt* GetSectionFormat( const SwNode& rNd ) const;

    /// Line number of the section start.
    sal_uLong GetSectionLineNo( const SfxItemSet* pSet, const SwNode& rNd ) const;

    /// Start new section.
    void OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd, bool isCellOpen = false, bool isTextNodeEmpty = false);

    /// Write section properties.
    ///
    /// pA is ignored for docx.
    void SectionProperties( const WW8_SepInfo& rSectionInfo, WW8_PdAttrDesc* pA = NULL );

    /// Output the numbering table.
    virtual void WriteNumbering() = 0;

    /// Write static data of SwNumRule - LSTF
    void NumberingDefinitions();

    /// Write all Levels for all SwNumRules - LVLF
    void AbstractNumberingDefinitions();

    // Convert the bullet according to the font.
    void SubstituteBullet( OUString& rNumStr, rtl_TextEncoding& rChrSet,
        OUString& rFontName ) const;

    /// No-op for the newer WW versions.
    virtual void OutputOlst( const SwNumRule& /*rRule*/ ) {}

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* /*pA*/ ) {}

    /// Top node of host page when in header/footer.
    void SetHdFtPageRoot( const SwTxtNode *pNd ) { mpTopNodeOfHdFtPage = pNd; }

    /// Top node of host page when in header/footer.
    const SwTxtNode *GetHdFtPageRoot() const { return mpTopNodeOfHdFtPage; }

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt,
        sal_uInt8 nBreakCode) = 0;

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const OUString& rFldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL ) = 0;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    com::sun::star::uno::Sequence<OUString> &rListItems) = 0;

    virtual void DoFormText(const SwInputField * pFld) = 0;

    static bool NoPageBreakSection( const SfxItemSet *pSet );

    // Compute the number format for WW dates
    bool GetNumberFmt(const SwField& rFld, OUString& rStr);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) = 0;

    const SfxPoolItem* HasItem( sal_uInt16 nWhich ) const;

    /// Returns the index of a picture bullet, used in numberings.
    int GetGrfIndex(const SvxBrushItem& rBrush);

protected:
    /// Format-dependent part of the actual export.
    virtual void ExportDocument_Impl() = 0;

    /// Get the next position in the text node to output
    virtual sal_Int32 GetNextPos( SwWW8AttrIter* pAttrIter, const SwTxtNode& rNode, sal_Int32 nAktPos );

    /// Update the information for GetNextPos().
    virtual void UpdatePosition( SwWW8AttrIter* pAttrIter, sal_Int32 nAktPos, sal_Int32 nEnd );

    /// Output SwTxtNode
    virtual void OutputTextNode( const SwTxtNode& );

    /// Output SwTableNode
    void OutputTableNode( const SwTableNode& );

    /// Setup the chapter fields (maChapterFieldLocs).
    void GatherChapterFields();

    void AddLinkTarget( const OUString& rURL );
    void CollectOutlineBookmarks( const SwDoc &rDoc );

    bool SetAktPageDescFromNode(const SwNode &rNd);
    bool CntntContainsChapterField(const SwFmtCntnt &rCntnt) const;
    bool FmtHdFtContainsChapterField(const SwFrmFmt &rFmt) const;

    virtual void SectionBreaksAndFrames( const SwTxtNode& rNode ) = 0;

    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 ) = 0;

    /// Return value indicates if an inherited outline numbering is suppressed.
    virtual bool DisallowInheritingOutlineNumbering(const SwFmt &rFmt) = 0;

    /// Output SwStartNode
    virtual void OutputStartNode( const SwStartNode& );

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& );

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) = 0;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) = 0;

    virtual void OutputLinkedOLE( const OUString& ) = 0;

    /// Output SwSectionNode
    virtual void OutputSectionNode( const SwSectionNode& );

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum ) = 0;

    /// Call the right (virtual) function according to the type of the item.
    ///
    /// One of OutputTextNode(), OutputGrfNode(), or OutputOLENode()
    void OutputContentNode( const SwCntntNode& );

    /// Find the nearest bookmark from the current position.
    ///
    /// Returns false when there is no bookmark.
    bool NearestBookmark( sal_Int32& rNearest, const sal_Int32 nAktPos, bool bNextPositionOnly );

    void GetSortedBookmarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen );

    bool GetBookmarks( const SwTxtNode& rNd, sal_Int32 nStt, sal_Int32 nEnd,
            IMarkVector& rArr );

    /// Find the nearest annotation mark from the current position.
    ///
    /// Returns false when there is no annotation mark.
    bool NearestAnnotationMark( sal_Int32& rNearest, const sal_Int32 nAktPos, bool bNextPositionOnly );

    void GetSortedAnnotationMarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen );

    bool GetAnnotationMarks( const SwTxtNode& rNd, sal_Int32 nStt, sal_Int32 nEnd,
            IMarkVector& rArr );

    const NfKeywordTable & GetNfKeywordTable();

    /// Populates m_vecBulletPic with all the bullet graphics used by numberings.
    int CollectGrfsOfBullets();
    /// Write the numbering picture bullets.
    void BulletDefinitions();
    std::vector<const Graphic*> m_vecBulletPic; ///< Vector to record all the graphics of bullets

public:
    MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam );
    virtual ~MSWordExportBase();

    // TODO move as much as possible here from WW8Export! ;-)

    static void CorrectTabStopInSet( SfxItemSet& rSet, sal_uInt16 nAbsLeft );

private:
    /// No copying.
    MSWordExportBase( const MSWordExportBase& );
    /// No copying.
    MSWordExportBase& operator=( const MSWordExportBase& );
};

/// The writer class that gets called for the WW8 filter.
class SwWW8Writer: public StgWriter
{
// friends to get access to m_pExport
// FIXME avoid that, this is probably not what we want
// (if yes, remove the friends, and provide here a GetExport() method)
friend void WW8_WrtRedlineAuthor::Write(Writer &rWrt);

    bool       m_bWrtWW8;
    WW8Export *m_pExport;
    SfxMedium *mpMedium;

public:
    SwWW8Writer(const OUString& rFltName, const OUString& rBaseURL);
    virtual ~SwWW8Writer();

    virtual sal_uLong WriteStorage() SAL_OVERRIDE;
    virtual sal_uLong WriteMedium( SfxMedium& ) SAL_OVERRIDE;

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
    virtual sal_uLong Write( SwPaM&, SfxMedium&, const OUString* = 0 ) SAL_OVERRIDE;
    //Seems not an expected to provide method to access the private member
    SfxMedium* GetMedia() { return mpMedium; }

private:
    /// No copying.
    SwWW8Writer(const SwWW8Writer&);
    /// No copying.
    SwWW8Writer& operator=(const SwWW8Writer&);
};

/// Exporter of the binary Word file formats.
class WW8Export : public MSWordExportBase
{
public:
    ww::bytes* pO;                       ///< Buffer

    SvStream *pTableStrm, *pDataStrm;   ///< Streams for WW97 Export

    WW8Fib* pFib;                       ///< File Information Block
    WW8Dop* pDop;                       ///< DOcument Properties
    WW8_WrPlcFtnEdn *pFtn;              ///< Footnotes - structure to remember them, and output
    WW8_WrPlcFtnEdn *pEdn;              ///< Endnotes - structure to remember them, and output
    WW8_WrPlcSepx* pSepx;               ///< Sections/headers/footers

    sal_uInt8 bWrtWW8 : 1;                   ///< Write WW95 (false) or WW97 (true) file format
    bool m_bDot; ///< Template or document.

protected:
    SwWW8Writer        *m_pWriter;      ///< Pointer to the writer
    WW8AttributeOutput *m_pAttrOutput;  ///< Converting attributes to stream data

private:
    SvStorageRef       xEscherStg;      /// memory leak #i120098#, to hold the reference to unnamed SotStorage obj

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const SAL_OVERRIDE;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const SAL_OVERRIDE;

    /// False for WW6, true for WW8.
    virtual bool SupportsUnicode() const SAL_OVERRIDE { return bWrtWW8; }

    virtual bool PreferPageBreakBefore() const SAL_OVERRIDE { return true; }

    virtual bool SupportsOneColumnBreak() const SAL_OVERRIDE { return false; }

    virtual bool FieldsQuoted() const SAL_OVERRIDE { return false; }

    virtual bool AddSectionBreaksForTOX() const SAL_OVERRIDE { return false; }
private:
    /// Format-dependent part of the actual export.
    virtual void ExportDocument_Impl() SAL_OVERRIDE;

    void PrepareStorage();
    void WriteFkpPlcUsw();
    void WriteMainText();
    void StoreDoc1();
    void Out_WwNumLvl( sal_uInt8 nWwLevel );
    void BuildAnlvBulletBase( WW8_ANLV& rAnlv, sal_uInt8*& rpCh, sal_uInt16& rCharLen,
                              const SwNumFmt& rFmt );
    static void BuildAnlvBase( WW8_ANLV& rAnlv, sal_uInt8*& rpCh, sal_uInt16& rCharLen,
                   const SwNumRule& rRul, const SwNumFmt& rFmt, sal_uInt8 nSwLevel );

    /// Output the numbering table.
    virtual void WriteNumbering() SAL_OVERRIDE;

    void OutOverrideListTab();
    void OutListNamesTab();

    void RestoreMacroCmds();

    void InitFontTable();

    void DoComboBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);
    void DoCheckBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);

public:
    virtual void OutputOlst( const SwNumRule& rRule ) SAL_OVERRIDE;

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* pA ) SAL_OVERRIDE;

    void Out_SwNumLvl( sal_uInt8 nSwLevel );
    void Out_NumRuleAnld( const SwNumRule& rRul, const SwNumFmt& rFmt,
                          sal_uInt8 nSwLevel );

    bool MiserableFormFieldExportHack(const SwFrmFmt& rFrmFmt);

    SvxMSExportOLEObjects& GetOLEExp()      { return *pOLEExp; }
    SwMSConvertControls& GetOCXExp()        { return *pOCXExp; }
    WW8OleMap& GetOLEMap()                  { return m_aOleMap; }
    void ExportDopTypography(WW8DopTypography &rTypo);

    sal_uInt16 AddRedlineAuthor( sal_uInt16 nId );

    void WriteFtnBegin( const SwFmtFtn& rFtn, ww::bytes* pO = 0 );
    void WritePostItBegin( ww::bytes* pO = 0 );
    const SvxBrushItem* GetCurrentPageBgBrush() const;
    SvxBrushItem TrueFrameBgBrush(const SwFrmFmt &rFlyFmt) const;

    /// Output all textframes anchored as character for the winword 7- format.
    void OutWW6FlyFrmsInCntnt( const SwTxtNode& rNd );

    void AppendFlyInFlys(const sw::Frame& rFrmFmt, const Point& rNdTopLeft);
    void WriteOutliner(const OutlinerParaObject& rOutliner, sal_uInt8 nTyp);
    void WriteSdrTextObj(const SdrObject& rObj, sal_uInt8 nTyp);

    sal_uInt32 GetSdrOrdNum( const SwFrmFmt& rFmt ) const;
    void CreateEscher();
    void WriteEscher();

    bool Out_SwNum(const SwTxtNode* pNd);

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const OUString& rFldCmd, sal_uInt8 nMode = nsFieldFlags::WRITEFIELD_ALL ) SAL_OVERRIDE;

    void StartCommentOutput( const OUString& rName );
    void EndCommentOutput(   const OUString& rName );
    void OutGrf(const sw::Frame &rFrame);
    bool TestOleNeedsGraphic(const SwAttrSet& rSet, SvStorageRef xOleStg,
        SvStorageRef xObjStg, OUString &rStorageName, SwOLENode *pOLENd);

    virtual void AppendBookmarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;
    virtual void AppendBookmark( const OUString& rName, bool bSkip = false ) SAL_OVERRIDE;

    virtual void AppendAnnotationMarks( const SwTxtNode& rNd, sal_Int32 nAktPos, sal_Int32 nLen ) SAL_OVERRIDE;

    virtual void ExportGrfBullet(const SwTxtNode& rNd) SAL_OVERRIDE;
    void OutGrfBullets(const sw::Frame &rFrame);

    void MoveFieldMarks(WW8_CP nFrom, WW8_CP nTo);

    void WriteAsStringTable(const ::std::vector<OUString>&, sal_Int32& rfcSttbf,
        sal_Int32& rlcbSttbf, sal_uInt16 nExtraLen = 0);

    virtual sal_uLong ReplaceCr( sal_uInt8 nChar ) SAL_OVERRIDE;

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t() ) SAL_OVERRIDE;
    void WriteChar( sal_Unicode c ) SAL_OVERRIDE;

    void OutSwString(const OUString&, sal_Int32 nStt, sal_Int32 nLen,
        bool bUnicode, rtl_TextEncoding eChrSet);

    WW8_CP Fc2Cp( sal_uLong nFc ) const          { return pPiece->Fc2Cp( nFc ); }

            // some partly static semi-internal function declarations

    void OutSprmBytes( sal_uInt8* pBytes, sal_uInt16 nSiz )
                                { pO->insert( pO->end(), pBytes, pBytes+nSiz ); }

    inline bool IsUnicode() const           { return pPiece->IsUnicode(); }

    virtual void SectionBreaksAndFrames( const SwTxtNode& rNode ) SAL_OVERRIDE;

    /// Helper method for OutputSectionBreaks() and OutputFollowPageDesc().
    // #i76300#
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 ) SAL_OVERRIDE;

    void Out_BorderLine(ww::bytes& rO, const ::editeng::SvxBorderLine* pLine,
        sal_uInt16 nDist, sal_uInt16 nSprmNo, sal_uInt16 nSprmNoVer9,
        bool bShadow);

    void Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow);
    void Out_SwFmtTableBox( ww::bytes& rO, const SvxBoxItem * rBox );
    void Out_CellRangeBorders(const SvxBoxItem * pBox, sal_uInt8 nStart,
        sal_uInt8 nLimit);
    sal_uInt8 TransCol( const Color& rCol );
    bool TransBrush(const Color& rCol, WW8_SHD& rShd);
    WW8_BRCVer9 TranslateBorderLine(const ::editeng::SvxBorderLine& pLine,
        sal_uInt16 nDist, bool bShadow);

    // #i77805# - new return value indicates, if an inherited outline numbering is suppressed
    virtual bool DisallowInheritingOutlineNumbering(const SwFmt &rFmt) SAL_OVERRIDE;

    unsigned int GetHdFtIndex() const { return mnHdFtIndex; }
    void SetHdFtIndex(unsigned int nHdFtIndex) { mnHdFtIndex = nHdFtIndex; }
    void IncrementHdFtIndex() { ++mnHdFtIndex; }

    static long GetDTTM( const DateTime& rDT );

    /// Convert the SVX numbering type to id
    static sal_uInt8 GetNumId( sal_uInt16 eNumType );

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( sal_uInt16 nScript, sal_uInt16 nWhich ) SAL_OVERRIDE;

    sal_uInt16 DupNumRuleWithLvlStart(const SwNumRule *pRule,sal_uInt8 nLvl,sal_uInt16 nVal);

    SwTwips CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const;

    /// Nasty swap for bidi if necessary
    bool MiserableRTLFrmFmtHack(SwTwips &rLeft, SwTwips &rRight,
        const sw::Frame &rFrmFmt);

    void InsUInt16( sal_uInt16 n )      { SwWW8Writer::InsUInt16( *pO, n ); }
    void InsUInt32( sal_uInt32 n )      { SwWW8Writer::InsUInt32( *pO, n ); }
    void InsAsString16( const OUString& rStr )
                        { SwWW8Writer::InsAsString16( *pO, rStr ); }
    void InsAsString8( const OUString& rStr, rtl_TextEncoding eCodeSet )
                        { SwWW8Writer::InsAsString8( *pO, rStr, eCodeSet ); }
    void WriteStringAsPara( const OUString& rTxt, sal_uInt16 nStyleId = 0 );

    /// Setup the exporter.
    WW8Export( SwWW8Writer *pWriter,
            SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
            bool bIsWW8, bool bDot );
    virtual ~WW8Export();

    virtual void DoComboBox(const OUString &rName,
                    const OUString &rHelp,
                    const OUString &ToolTip,
                    const OUString &rSelected,
                    com::sun::star::uno::Sequence<OUString> &rListItems) SAL_OVERRIDE;

    virtual void DoFormText(const SwInputField * pFld) SAL_OVERRIDE;

    void GetCurrentItems(ww::bytes &rItems) const;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) SAL_OVERRIDE;

    /// Fields.
    WW8_WrPlcFld* CurrentFieldPlc() const;

    SwWW8Writer& GetWriter() const { return *m_pWriter; }
    SvStream& Strm() const { return m_pWriter->Strm(); }

    /// Remember some of the memebers so that we can recurse in WriteText().
    virtual void SaveData( sal_uLong nStt, sal_uLong nEnd ) SAL_OVERRIDE;

    /// Restore what was saved in SaveData().
    virtual void RestoreData() SAL_OVERRIDE;

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( sal_uInt8 nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt,
        sal_uInt8 nBreakCode) SAL_OVERRIDE;

protected:
    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) SAL_OVERRIDE;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) SAL_OVERRIDE;

    virtual void OutputLinkedOLE( const OUString& ) SAL_OVERRIDE;

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum ) SAL_OVERRIDE;

private:
    /// No copying.
    WW8Export(const WW8Export&);
    /// No copying.
    WW8Export& operator=(const WW8Export&);
};

class WW8_WrPlcSubDoc   // double Plc for Footnotes/Endnotes and Postits
{
private:
    // No copying
    WW8_WrPlcSubDoc(const WW8_WrPlcSubDoc&);
    WW8_WrPlcSubDoc& operator=(const WW8_WrPlcSubDoc&);
protected:
    std::vector<WW8_CP> aCps;
    std::vector<const void*> aCntnt;                // PTRARR of SwFmtFtn/PostIts/..
    std::vector<const SwFrmFmt*> aSpareFmts;        //a backup for aCntnt: if there's no SdrObject, stores the fmt directly here
    WW8_WrPlc0* pTxtPos;            // positions of the individual texts

    WW8_WrPlcSubDoc();
    virtual ~WW8_WrPlcSubDoc();

    bool WriteGenericTxt( WW8Export& rWrt, sal_uInt8 nTTyp, WW8_CP& rCount );
    void WriteGenericPlc( WW8Export& rWrt, sal_uInt8 nTTyp, WW8_FC& rTxtStt,
        sal_Int32& rTxtCnt, WW8_FC& rRefStt, sal_Int32& rRefCnt ) const;

    virtual const std::vector<sal_uInt32>* GetShapeIdArr() const;
};

// double Plc for Footnotes/Endnotes
class WW8_WrPlcFtnEdn : public WW8_WrPlcSubDoc
{
private:
    sal_uInt8 nTyp;

    // No copying
    WW8_WrPlcFtnEdn(const WW8_WrPlcFtnEdn&);
    WW8_WrPlcFtnEdn& operator=(WW8_WrPlcFtnEdn &);
public:
    WW8_WrPlcFtnEdn( sal_uInt8 nTTyp ) : nTyp( nTTyp ) {}

    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;

    void Append( WW8_CP nCp, const SwFmtFtn& rFtn );
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
    WW8_Annotation(const SwRedlineData* pRedline);
};

class WW8_WrPlcAnnotations : public WW8_WrPlcSubDoc  // double Plc for Postits
{
private:
    //No copying
    WW8_WrPlcAnnotations(const WW8_WrPlcAnnotations&);
    WW8_WrPlcAnnotations& operator=(WW8_WrPlcAnnotations&);
    std::set<const SwRedlineData*> maProcessedRedlines;

    std::map<const OUString, WW8_CP> m_aRangeStartPositions;
public:
    WW8_WrPlcAnnotations() {}
    virtual ~WW8_WrPlcAnnotations();

    void AddRangeStartPosition(const OUString& rName, WW8_CP nStartCp);
    void Append( WW8_CP nCp, const SwPostItField* pPostIt );
    void Append( WW8_CP nCp, const SwRedlineData* pRedLine );
    bool IsNewRedlineComment( const SwRedlineData* pRedLine );
    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
};

class WW8_WrPlcTxtBoxes : public WW8_WrPlcSubDoc // double Plc for Textboxes
{                        // Frame/DrawTextboxes!
private:
    sal_uInt8 nTyp;
    std::vector<sal_uInt32> aShapeIds;        // VARARR of ShapeIds for the SwFrmFmts
    virtual const std::vector<sal_uInt32>* GetShapeIdArr() const SAL_OVERRIDE;

    //No copying
    WW8_WrPlcTxtBoxes(const WW8_WrPlcTxtBoxes&);
    WW8_WrPlcTxtBoxes& operator=(WW8_WrPlcTxtBoxes&);
public:
    WW8_WrPlcTxtBoxes( sal_uInt8 nTTyp ) : nTyp( nTTyp ) {}

    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
    void Append( const SdrObject& rObj, sal_uInt32 nShapeId );
    void Append( const SwFrmFmt* pFmt, sal_uInt32 nShapeId );
    sal_uInt16 Count() const { return aCntnt.size(); }
    sal_uInt16 GetPos( const void* p ) const
    {
        std::vector<const void*>::const_iterator it
            = std::find( aCntnt.begin(), aCntnt.end(), p );
        return it == aCntnt.end() ? USHRT_MAX : it - aCntnt.begin();
    }
};

// Plc for Chpx and Papx ( incl PN-Plc )
typedef boost::ptr_vector<WW8_WrFkp> WW8_WrFkpPtrs;

class WW8_WrPlcPn                   // Plc for Page Numbers
{
private:
    WW8Export& rWrt;
    WW8_WrFkpPtrs aFkps;            // PTRARR
    sal_uInt16 nFkpStartPage;
    ePLCFT ePlc;
    bool bWrtWW8;                   // for writer detection
    sal_uInt16 nMark;

    //No copying
    WW8_WrPlcPn(const WW8_WrPlcPn&);
    WW8_WrPlcPn& operator=(const WW8_WrPlcPn&);
public:
    WW8_WrPlcPn( WW8Export& rWrt, ePLCFT ePl, WW8_FC nStartFc );
    ~WW8_WrPlcPn();
    void AppendFkpEntry(WW8_FC nEndFc,short nVarLen = 0,const sal_uInt8* pSprms = 0);
    void WriteFkps();
    void WritePlc();
    sal_uInt8 *CopyLastSprms(sal_uInt8 &rLen);
};

// class WW8_WrPlc1 is only used for fields
class WW8_WrPlc1
{
private:
    std::vector<WW8_CP> aPos;
    sal_uInt8* pData;                // content ( structures )
    sal_uLong nDataLen;
    sal_uInt16 nStructSiz;

    //No copying
    WW8_WrPlc1(const WW8_WrPlc1&);
    WW8_WrPlc1& operator=(const WW8_WrPlc1&);
protected:
    sal_uInt16 Count() const { return aPos.size(); }
    void Write( SvStream& rStrm );
    WW8_CP Prev() const;
public:
    WW8_WrPlc1( sal_uInt16 nStructSz );
    ~WW8_WrPlc1();
    void Append( WW8_CP nCp, const void* pData );
    void Finish( sal_uLong nLastCp, sal_uLong nStartCp );
};

// class WW8_WrPlcFld is for fields
class WW8_WrPlcFld : public WW8_WrPlc1
{
private:
    sal_uInt8 nTxtTyp;
    sal_uInt16 nResults;

    //No copying
    WW8_WrPlcFld(const WW8_WrPlcFld&);
    WW8_WrPlcFld& operator=(const WW8_WrPlcFld&);
public:
    WW8_WrPlcFld( sal_uInt16 nStructSz, sal_uInt8 nTTyp )
        : WW8_WrPlc1( nStructSz ), nTxtTyp( nTTyp ), nResults(0)
    {}
    bool Write( WW8Export& rWrt );
    void ResultAdded() { ++nResults; }
    sal_uInt16 ResultCount() const { return nResults; }
};

class WW8_WrMagicTable : public WW8_WrPlc1
{
private:
    //No copying
    WW8_WrMagicTable(const WW8_WrMagicTable&);
    WW8_WrMagicTable& operator=(const WW8_WrMagicTable&);
public:
    WW8_WrMagicTable() : WW8_WrPlc1( 4 ) {Append(0,0);}
    void Append( WW8_CP nCp, sal_uLong nData );
    bool Write( WW8Export& rWrt );
};

class GraphicDetails
{
public:
    sw::Frame maFly;                // surrounding FlyFrms
    sal_uLong mnPos;                // FilePos of the graphics
    sal_uInt16 mnWid;               // Width of the graphics
    sal_uInt16 mnHei;               // Height of the graphics

    GraphicDetails(const sw::Frame &rFly, sal_uInt16 nWid, sal_uInt16 nHei)
        : maFly(rFly), mnPos(0), mnWid(nWid), mnHei(nHei)
    {}
    GraphicDetails& operator=(const GraphicDetails& rOther);

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
    typedef std::vector<GraphicDetails>::iterator myiter;
    sal_uInt16 mnIdx;       // Index in File-Positionen

    void WritePICFHeader(SvStream& rStrm, const sw::Frame &rFly,
            sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight,
            const SwAttrSet* pAttrSet = 0);
    void WriteGraphicNode(SvStream& rStrm, const GraphicDetails &rItem);
    void WriteGrfFromGrfNode(SvStream& rStrm, const SwGrfNode &rNd,
        const sw::Frame &rFly, sal_uInt16 nWidth, sal_uInt16 nHeight);

    void WritePICBulletFHeader(SvStream& rStrm, const Graphic &rGrf, sal_uInt16 mm, sal_uInt16 nWidth, sal_uInt16 nHeight);
    void WriteGrfForBullet(SvStream& rStrm,  const Graphic &rGrf, sal_uInt16 nWidth, sal_uInt16 nHeight);

    //No copying
    SwWW8WrGrf(const SwWW8WrGrf&);
    SwWW8WrGrf& operator=(const SwWW8WrGrf&);
public:
    SwWW8WrGrf( WW8Export& rW ) : rWrt( rW ), mnIdx( 0 ) {}
    void Insert(const sw::Frame &rFly);
    void Write();
    sal_uLong GetFPos()
        { return (mnIdx < maDetails.size()) ? maDetails[mnIdx++].mnPos : 0; }
};

/** The class MSWordAttrIter is a helper class to build the Fkp.chpx.
    This class may be overloaded for output the SwTxtAttrs and the
    EditEngineTxtAttrs.
*/
class MSWordAttrIter
{
private:
    MSWordAttrIter* pOld;
    //No copying
    MSWordAttrIter(const MSWordAttrIter&);
    MSWordAttrIter& operator=(const MSWordAttrIter&);
protected:
    MSWordExportBase& m_rExport;
public:
    MSWordAttrIter( MSWordExportBase& rExport );
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
    std::vector<EECharAttrib> aTxtAtrArr;
    std::vector<const EECharAttrib*> aChrTxtAtrArr;
    std::vector<rtl_TextEncoding> aChrSetArr;
    sal_Int32 nPara;
    sal_Int32 nAktSwPos;
    sal_Int32 nTmpSwPos;                   // for HasItem()
    rtl_TextEncoding eNdChrSet;
    sal_uInt16 nScript;
    sal_uInt8 mnTyp;

    sal_Int32 SearchNext( sal_Int32 nStartPos );
    void SetCharSet(const EECharAttrib& rTxtAttr, bool bStart);

    //No copying
    MSWord_SdrAttrIter(const MSWord_SdrAttrIter&);
    MSWord_SdrAttrIter& operator=(const MSWord_SdrAttrIter&);
public:
    MSWord_SdrAttrIter( MSWordExportBase& rWr, const EditTextObject& rEditObj,
        sal_uInt8 nType );
    void NextPara( sal_Int32 nPar );
    void OutParaAttr(bool bCharAttr);
    void OutEEField(const SfxPoolItem& rHt);

    bool IsTxtAttr(sal_Int32 nSwPos);

    void NextPos() { if ( nAktSwPos < SAL_MAX_INT32 ) nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( sal_Int32 nSwPos );
    virtual const SfxPoolItem* HasTextItem( sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const SAL_OVERRIDE;
    bool OutAttrWithRange(sal_Int32 nPos);
    sal_Int32 WhereNext() const                { return nAktSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const     { return eNdChrSet; }
};

// class SwWW8AttrIter is a helper for constructing the Fkp.chpx.
// Only character attributes are considered; paragraph attributes do not need this treatment.
// Die Absatz- und Textattribute des Writers kommen rein, und es wird
// mit Where() die naechste Position geliefert, an der sich die Attribute
// aendern. IsTxtAtr() sagt, ob sich an der mit Where() gelieferten Position
// ein Attribut ohne Ende und mit \xff im Text befindet.
// Mit OutAttr() werden die Attribute an der angegebenen SwPos
// ausgegeben.
class SwWW8AttrIter : public MSWordAttrIter
{
private:
    const SwTxtNode& rNd;

    sw::util::CharRuns maCharRuns;
    sw::util::cCharRunIter maCharRunIter;

    rtl_TextEncoding meChrSet;
    sal_uInt16 mnScript;
    bool mbCharIsRTL;

    const SwRangeRedline* pCurRedline;
    sal_Int32 nAktSwPos;
    sal_uInt16 nCurRedlinePos;

    bool mbParaIsRTL;

    const SwFmtDrop &mrSwFmtDrop;

    sw::Frames maFlyFrms;     // #i2916#
    sw::FrameIter maFlyIter;

    sal_Int32 SearchNext( sal_Int32 nStartPos );
    void FieldVanish( const OUString& rTxt );

    void OutSwFmtRefMark(const SwFmtRefMark& rAttr, bool bStart);

    void IterToCurrent();

    //No copying
    SwWW8AttrIter(const SwWW8AttrIter&);
    SwWW8AttrIter& operator=(const SwWW8AttrIter&);
public:
    SwWW8AttrIter( MSWordExportBase& rWr, const SwTxtNode& rNd );

    bool IsTxtAttr( sal_Int32 nSwPos );
    bool IsRedlineAtEnd( sal_Int32 nPos ) const;
    bool IsDropCap( int nSwPos );
    bool RequiresImplicitBookmark();

    void NextPos() { if ( nAktSwPos < SAL_MAX_INT32 ) nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( sal_Int32 nSwPos, bool bRuby = false );
    virtual const SfxPoolItem* HasTextItem( sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual const SfxPoolItem& GetItem( sal_uInt16 nWhich ) const SAL_OVERRIDE;
    int OutAttrWithRange(sal_Int32 nPos);
    const SwRedlineData* GetParagraphLevelRedline( );
    const SwRedlineData* GetRunLevelRedline( sal_Int32 nPos );
    void OutFlys(sal_Int32 nSwPos);

    sal_Int32 WhereNext() const { return nAktSwPos; }
    sal_uInt16 GetScript() const { return mnScript; }
    bool IsCharRTL() const { return mbCharIsRTL; }
    bool IsParaRTL() const { return mbParaIsRTL; }
    rtl_TextEncoding GetCharSet() const { return meChrSet; }
    OUString GetSnippet(const OUString &rStr, sal_Int32 nAktPos,
        sal_Int32 nLen) const;
    const SwFmtDrop& GetSwFmtDrop() const { return mrSwFmtDrop; }

    bool IsWatermarkFrame();
};

/// Class to collect and output the styles table.
class MSWordStyles
{
    MSWordExportBase& m_rExport;
    SwFmt** pFmtA; ///< Slot <-> Character and paragraph style array (0 for list styles).
    sal_uInt16 nUsedSlots;
    bool m_bListStyles; ///< If list styles are requested to be exported as well.
    std::map<sal_uInt16, const SwNumRule*> m_aNumRules; ///< Slot <-> List style map.

    /// We need to build style id's for DOCX export; ideally we should roundtrip that, but this is good enough.
    std::vector<OString> m_aStyleIds;

    /// Create the style table, called from the constructor.
    void BuildStylesTable();

    /// Based on pFmtA, fill in m_aStyleIds with unique, MS-like names.
    void BuildStyleIds();

    /// Get slot number during building the style table.
    sal_uInt16 BuildGetSlot( const SwFmt& rFmt );
    sal_uInt16 BuildGetSlot( const SwNumRule& rNumRule );

    /// Return information about one style.
    void GetStyleData( SwFmt* pFmt, bool& bFmtColl, sal_uInt16& nBase, sal_uInt16& nNext );

    /// Outputs attributes of one style.
    void WriteProperties( const SwFmt* pFmt, bool bPap, sal_uInt16 nPos, bool bInsDefCharSiz );

    sal_uInt16 GetWWId( const SwFmt& rFmt ) const;

    void SetStyleDefaults( const SwFmt& rFmt, bool bPap );

    /// Outputs one style - called (in a loop) from OutputStylesTable().
    void OutputStyle( SwFmt* pFmt, sal_uInt16 nPos );
    void OutputStyle( const SwNumRule* pNumRule, sal_uInt16 nPos );

    // No copying
    MSWordStyles( const MSWordStyles& );
    MSWordStyles& operator=( const MSWordStyles& );

public:
    MSWordStyles( MSWordExportBase& rExport, bool bListStyles = false );
    ~MSWordStyles();

    /// Output the styles table.
    void OutputStylesTable();

    /// Get id of the style (rFmt).
    sal_uInt16 GetSlot( const SwFmt& rFmt ) const;

    /// Get styleId of the nId-th style (nId is its position in pFmtA).
    OString GetStyleId(sal_uInt16 nId) const;

    const SwFmt* GetSwFmt(sal_uInt16 nId) const { return pFmtA[nId]; }
    /// Get numbering rule of the nId-th style
    const SwNumRule* GetSwNumRule(sal_uInt16 nId) const;
};

#define MSWORD_MAX_STYLES_LIMIT 4091

sal_Int16 GetWordFirstLineOffset(const SwNumFmt &rFmt);
// A bit of a bag on the side for now
OUString FieldString(ww::eField eIndex);
OUString BookmarkToWord(const OUString &rBookmark);

class WW8SHDLong
{
    sal_uInt32 m_cvFore;
    sal_uInt32 m_cvBack;
    sal_uInt16 m_ipat;

public:
    WW8SHDLong() : m_cvFore(0), m_cvBack(0), m_ipat(0) {}
    virtual ~WW8SHDLong() {}

    void Write(WW8Export & rExport);
    void setCvFore(sal_uInt32 cvFore) { m_cvFore = cvFore; }
    void setCvBack(sal_uInt32 cvBack) { m_cvBack = cvBack; }
    void setIPat(sal_uInt16 ipat) { m_ipat = ipat; }
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_WRTWW8_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
