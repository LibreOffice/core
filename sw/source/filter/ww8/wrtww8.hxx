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

#ifndef _WRTWW8_HXX
#define _WRTWW8_HXX

#include <tools/solar.h>        // UINTXX
#include <tools/gen.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svl/svstdarr.hxx>
#endif
#include <editeng/editdata.hxx>

#include <map>
#include <vector>

#include <shellio.hxx>
#include <wrt_fn.hxx>
#include <filter/msfilter/msocximex.hxx>

#include "ww8struc.hxx"
#include "ww8scan.hxx"
#include "fields.hxx"
#include "types.hxx"
#include "writerhelper.hxx"
#include "../inc/msfilter.hxx"
#include <expfld.hxx>

// einige Forward Deklarationen
class SwWW8AttrIter;
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
class SvxBorderLine;
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
class SwSection;
class SwSectionFmt;
class SwSectionNode;
class SwTableNode;
class SwTOXType;
class SwTxtAttr;
class SwTxtFmtColl;
class SwTxtNode;
class SwWW8WrGrf;
class SwWW8Writer;
class MSWordStyles;
class WW8AttributeOutput;
class WW8Bytes;
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
class WW8_WrPcPtrs;
class WW8_WrtBookmarks;
class WW8_WrtRedlineAuthor;
class SvxMSExportOLEObjects;
class SwMSConvertControls;
class WW8OleMaps;
class SvStorageRef;
struct WW8_PdAttrDesc;
class SvxBrushItem;

#include "WW8TableInfo.hxx"

#define GRF_MAGIC_1 0x12    // 3 magic Bytes fuer PicLocFc-Attribute
#define GRF_MAGIC_2 0x34
#define GRF_MAGIC_3 0x56
#define GRF_MAGIC_321 0x563412L

#define OLE_PREVIEW_AS_EMF  //If we want to export ole2 previews as emf in ww8+

typedef BYTE FieldFlags;
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

SV_DECL_VARARR( WW8Bytes, BYTE, 128, 128 )

struct WW8_SepInfo
{
    const SwPageDesc* pPageDesc;
    const SwSectionFmt* pSectionFmt;
    const SwNode* pPDNd;
    const SwTxtNode* pNumNd;
    ULONG  nLnNumRestartNo;
    USHORT nPgRestartNo;

    WW8_SepInfo()
        : pPageDesc(0), pSectionFmt(0), pPDNd(0), pNumNd(0), nLnNumRestartNo(0), nPgRestartNo(0)

    {}

    WW8_SepInfo( const SwPageDesc* pPD, const SwSectionFmt* pFmt,
        ULONG nLnRestart, USHORT nPgRestart = 0, const SwNode* pNd = NULL )
        : pPageDesc( pPD ), pSectionFmt( pFmt ), pPDNd( pNd ), pNumNd( 0 ),
          nLnNumRestartNo( nLnRestart ), nPgRestartNo( nPgRestart )
    {}

    bool IsProtected() const;
};

SV_DECL_VARARR( WW8_WrSepInfoPtrs, WW8_SepInfo, 4, 4 )

/// Class to collect and output the sections/headers/footers.
// Plc fuer PageDescs -> Sepx ( Section Extensions )
class MSWordSections
{
protected:
    bool mbDocumentIsProtected;
    WW8_WrSepInfoPtrs aSects;   // PTRARR von SwPageDesc und SwSectionFmt

    void CheckForFacinPg( WW8Export& rWrt ) const;
    void WriteOlst( WW8Export& rWrt, const WW8_SepInfo& rSectionInfo );
    void NeedsDocumentProtected(const WW8_SepInfo &rInfo);

    //No copy, no assign
    MSWordSections( const MSWordSections& );
    MSWordSections& operator=( const MSWordSections& );
public:
    MSWordSections( MSWordExportBase& rExport );
    virtual ~MSWordSections();

    void AppendSection( const SwPageDesc* pPd,
                    const SwSectionFmt* pSectionFmt = 0,
                    ULONG nLnNumRestartNo = 0 );
    void AppendSection( const SwFmtPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFmt* pSectionFmt,
                    ULONG nLnNumRestartNo );
    void SetNum( const SwTxtNode* pNumNd );

    /// Number of columns based on the most recent WW8_SepInfo.
    sal_uInt16 CurrentNumberOfColumns( const SwDoc &rDoc ) const;

    /// Number of columns of the provided WW8_SepInfo.
    sal_uInt16 NumberOfColumns( const SwDoc &rDoc, const WW8_SepInfo& rInfo ) const;

    bool DocumentIsProtected() const { return mbDocumentIsProtected; }

    /// The most recent WW8_SepInfo.
    const WW8_SepInfo* CurrentSectionInfo();

    static void SetHeaderFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                  BYTE nFlag );
    static void SetFooterFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                   BYTE nFlag );

    /// Should we output borders?
    static int HasBorderItem( const SwFmt& rFmt );
};

class WW8_WrPlcSepx : public MSWordSections
{
    SvULongs aCps;              // PTRARR von CPs
    WW8_PdAttrDesc* pAttrs;
    WW8_WrPlc0* pTxtPos;        // Pos der einzelnen Header / Footer
    bool bNoMoreSections;

    // No copy, no assign
    WW8_WrPlcSepx( const WW8_WrPlcSepx& );
    WW8_WrPlcSepx& operator=( const WW8_WrPlcSepx& );

public:
    WW8_WrPlcSepx( MSWordExportBase& rExport );
    ~WW8_WrPlcSepx();

    void AppendSep( WW8_CP nStartCp,
                    const SwPageDesc* pPd,
                    const SwSectionFmt* pSectionFmt = 0,
                    ULONG nLnNumRestartNo = 0 );
    void AppendSep( WW8_CP nStartCp, const SwFmtPageDesc& rPd,
                    const SwNode& rNd,
                    const SwSectionFmt* pSectionFmt,
                    ULONG nLnNumRestartNo );
    void Finish( WW8_CP nEndCp ) { aCps.Insert( nEndCp, aCps.Count() ); }

    bool WriteKFTxt( WW8Export& rWrt );
    void WriteSepx( SvStream& rStrm ) const;
    void WritePlcSed( WW8Export& rWrt ) const;
    void WritePlcHdd( WW8Export& rWrt ) const;

private:
    void WriteFtnEndTxt( WW8Export& rWrt, ULONG nCpStt );
public:
    void OutHeaderFooter(WW8Export& rWrt, bool bHeader,
            const SwFmt& rFmt, ULONG& rCpPos, BYTE nHFFlags, BYTE nFlag,  BYTE nBreakCode);
};

//--------------------------------------------------------------------------
// class WW8_WrPct zum Aufbau der Piece-Table
//--------------------------------------------------------------------------
class WW8_WrPct
{
    WW8_WrPcPtrs* pPcts;
    WW8_FC nOldFc;
    bool bIsUni;
public:
    WW8_WrPct(WW8_FC nStartFc, bool bSaveUniCode);
    ~WW8_WrPct();
    void AppendPc(WW8_FC nStartFc, bool bIsUnicode);
    void WritePc( WW8Export& rWrt );
    void SetParaBreak();
    bool IsUnicode() const  { return bIsUni; }
    WW8_CP Fc2Cp( ULONG nFc ) const;
};

/// Collects and outputs fonts.
class wwFont
{
//In some future land the stream could be converted to a nice stream interface
//and we could have harmony
private:
    BYTE maWW8_FFN[6];
    String msFamilyNm;
    String msAltNm;
    bool mbAlt;
    bool mbWrtWW8;
    FontPitch mePitch;
    FontFamily meFamily;
    rtl_TextEncoding meChrSet;
public:
    wwFont( const String &rFamilyName, FontPitch ePitch, FontFamily eFamily,
        rtl_TextEncoding eChrSet, bool bWrtWW8 );
    bool Write( SvStream *pTableStram ) const;
    void WriteDocx( const DocxAttributeOutput* rAttrOutput ) const;
    void WriteRtf( const RtfAttributeOutput* rAttrOutput ) const;
    rtl::OUString GetFamilyName() const { return rtl::OUString( msFamilyNm ); }
    friend bool operator < (const wwFont &r1, const wwFont &r2);
};

class wwFontHelper
{
private:
    /// Keep track of fonts that need to be exported.
    ::std::map<wwFont, USHORT> maFonts;
    bool mbWrtWW8;

    /// Convert from fast insertion map to linear vector in the order that we want to write.
    ::std::vector< const wwFont* > AsVector() const;

public:
    wwFontHelper() : mbWrtWW8(false), bLoadAllFonts(false) {}
    /// rDoc used only to get the initial standard font(s) in use.
    void InitFontTable(bool bWrtWW8, const SwDoc& rDoc);
    USHORT GetId(const Font& rFont);
    USHORT GetId(const SvxFontItem& rFont);
    USHORT GetId(const wwFont& rFont);
    void WriteFontTable( SvStream *pTableStream, WW8Fib& pFib );
    void WriteFontTable( const DocxAttributeOutput& rAttrOutput );
    void WriteFontTable( const RtfAttributeOutput& rAttrOutput );

    /// If true, all fonts are loaded before processing the document.
    BYTE bLoadAllFonts: 1;
};

class DrawObj
{
public:
    WW8_CP mnCp;                // CP-Pos der Verweise
    UINT32 mnShapeId;           // ShapeId for the SwFrmFmts
    sw::Frame maCntnt;          // the frame itself
    Point maParentPos;          // Points
    INT32 mnThick;              // Border Thicknesses
    short mnDirection;          // If BiDi or not
    unsigned int mnHdFtIndex;   // 0 for main text, +1 for each subsequent
                                // msword hd/ft

    DrawObj(const sw::Frame &rCntnt, WW8_CP nCp, Point aParentPos, short nDir,
            unsigned int nHdFtIndex)
        : mnCp(nCp), mnShapeId(0), maCntnt(rCntnt), maParentPos(aParentPos),
        mnThick(0), mnDirection(nDir), mnHdFtIndex(nHdFtIndex) {}
    void SetShapeDetails(UINT32 nId, INT32 nThick);
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
        sal_uInt32 nLen) const;
    virtual WW8_CP GetCpOffset(const WW8Fib &) const;
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
        sal_uInt32 nLen) const;
    virtual WW8_CP GetCpOffset(const WW8Fib &rFib) const;
private:
    //No copying
    HdFtPlcDrawObj(const HdFtPlcDrawObj&);
    HdFtPlcDrawObj& operator=(const HdFtPlcDrawObj&);
};

typedef ::std::pair<String, ULONG> aBookmarkPair;
typedef std::vector<aBookmarkPair> SwImplBookmarks;
typedef std::vector<aBookmarkPair>::iterator SwImplBookmarksIter;

class WW8_WrtRedlineAuthor : public sw::util::WrtRedlineAuthor
{
    public:
    virtual void Write(Writer &rWrt);
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
    WW8Bytes* pOOld;                ///< WW8Export only
    WW8Bytes* mpTableAtOld;         ///< WW8Export only: Additional buffer for the output of the tables
    sal_uInt16 mnTableStdAtLenOld;  ///< WW8Export only: Standard length of mpTableAt
    SwPaM* pOldPam, *pOldEnd;
    const sw::Frame* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;

    BYTE bOldWriteAll : 1;          ///< WW8Export only
    BYTE bOldOutTable : 1;
    BYTE bOldIsInTable: 1;
    BYTE bOldFlyFrmAttrs : 1;
    BYTE bOldStartTOX : 1;
    BYTE bOldInWriteTOX : 1;
    // bOutPageDesc muss nicht gesichert werden, da es nur nicht waehrend der
    // Ausgabe von Spezial-Texten veraendert wird.
};

/// Base class for WW8Export and DocxExport
class MSWordExportBase
{
public:
    wwFontHelper maFontHelper;
    std::vector<ULONG> maChapterFieldLocs;
    typedef std::vector<ULONG>::const_iterator mycCFIter;
    String aMainStg;
    SvPtrarr aTOXArr;
    const SfxItemSet* pISet;    // fuer Doppel-Attribute
    WW8_WrPct*  pPiece;         // Pointer auf Piece-Table
    SwNumRuleTbl* pUsedNumTbl;  // alle used NumRules
    const SwTxtNode *mpTopNodeOfHdFtPage; ///< Top node of host page when in hd/ft
    std::map< USHORT, USHORT > aRuleDuplicates; //map to Duplicated numrules
    std::stack< xub_StrLen > m_aCurrentCharPropStarts; ///< To remember the position in a run.
    WW8_WrtBookmarks* pBkmks;
    WW8_WrtRedlineAuthor* pRedlAuthors;
    BitmapPalette* pBmpPal;
    void* pKeyMap;
    SvxMSExportOLEObjects* pOLEExp;
    SwMSConvertControls* pOCXExp;
    WW8OleMaps* pOleMap;
    ww8::WW8TableInfo::Pointer_t mpTableInfo;

    USHORT nCharFmtStart;
    USHORT nFmtCollStart;
    USHORT nStyleBeforeFly;     ///< Style-Nummer des Nodes,
                                ///<       in/an dem ein Fly verankert ist
    USHORT nLastFmtId;          ///< Style of last TxtNode in normal range
    USHORT nUniqueList;         ///< current number for creating unique list names
    unsigned int mnHdFtIndex;

    USHORT mnRedlineMode;       ///< Remember the original redline mode

public:
    /* implicit bookmark vector containing pairs of node indexes and bookmark names */
    SwImplBookmarks maImplicitBookmarks;
    sw::Frames maFrames;             // The floating frames in this document
    const SwPageDesc *pAktPageDesc;
    WW8_WrPlcPn* pPapPlc;
    WW8_WrPlcPn* pChpPlc;
    MSWordAttrIter* pChpIter;
    MSWordStyles* pStyles;
    WW8_WrPlcAnnotations* pAtn;
    WW8_WrPlcTxtBoxes *pTxtBxs, *pHFTxtBxs;

    const sw::Frame *mpParentFrame; //If set we are exporting content inside
                                    //a frame, e.g. a graphic node

    Point* pFlyOffset;              // zur Justierung eines im Writer als
    RndStdIds eNewAnchorType;       // Zeichen gebundenen Flys, der im WW
                                    // Absatzgebunden wird.

    WW8_WrPlcFld* pFldMain;         // Felder im Haupttext
    WW8_WrPlcFld* pFldHdFt;         // Felder in Header/Footer
    WW8_WrPlcFld* pFldFtn;          // Felder in FootNotes
    WW8_WrPlcFld* pFldEdn;          // Felder in EndNotes
    WW8_WrPlcFld* pFldAtn;          // Felder in Annotations
    WW8_WrPlcFld* pFldTxtBxs;       // fields in textboxes
    WW8_WrPlcFld* pFldHFTxtBxs;     // fields in header/footer textboxes
    WW8_WrMagicTable *pMagicTable;  // keeps track of table cell positions, and
                                    // marks those that contain graphics,
                                    // which is required to make word display
                                    // graphics inside tables
    SwWW8WrGrf* pGrf;
    const SwAttrSet* pStyAttr;      // StyleAttr fuer Tabulatoren
    const SwModify* pOutFmtNode;    // write Format or Node
    const SwFmt *pCurrentStyle;     // iff bStyDef=true, then this store the current style

    MainTxtPlcDrawObj *pSdrObjs;   // Draw-/Fly-Objects
    HdFtPlcDrawObj *pHFSdrObjs;     // Draw-/Fly-Objects in header or footer

    SwEscherEx* pEscher;            // escher export class
    // #i43447# - removed
//    SwTwips nFlyWidth, nFlyHeight;  // Fuer Anpassung Graphic
    // <--

    BYTE nTxtTyp;

    BYTE bStyDef : 1;           // wird Style geschrieben ?
    BYTE bBreakBefore : 1;      // Breaks werden 2mal ausgegeben
    BYTE bOutKF : 1;            // Kopf/Fusstexte werden ausgegeben
    BYTE bOutFlyFrmAttrs : 1;   // Rahmen-Attr von Flys werden ausgegeben
    BYTE bOutPageDescs : 1;     ///< PageDescs (section properties) are being written
    BYTE bOutFirstPage : 1;     // write Attrset of FirstPageDesc
    BYTE bOutTable : 1;         // Tabelle wird ausgegeben
                                //    ( wird zB bei Flys in Tabelle zurueckgesetzt )
    BYTE bIsInTable : 1;        // wird sind innerhalb der Ausgabe einer Tabelle
                                //    ( wird erst nach der Tabelle zurueckgesetzt )
    BYTE bOutGrf : 1;           // Grafik wird ausgegeben
    BYTE bInWriteEscher : 1;    // in write textboxes
    BYTE bStartTOX : 1;         // true: a TOX is startet
    BYTE bInWriteTOX : 1;       // true: all content are in a TOX
    BYTE bFtnAtTxtEnd : 1;      // true: all FTN at Textend
    BYTE bEndAtTxtEnd : 1;      // true: all END at Textend
    BYTE bHasHdr : 1;
    BYTE bHasFtr : 1;
    BYTE bSubstituteBullets : 1; // true: SubstituteBullet() gets called

    bool mbExportModeRTF;

    SwDoc *pDoc;
    SwPaM *pCurPam, *pOrigPam;

    /// Stack to remember the nesting (see MSWordSaveData for more)
    ::std::stack< MSWordSaveData > maSaveData;

    /// Used to split the runs according to the bookmarks start and ends
    typedef std::vector< ::sw::mark::IMark* > IMarkVector;
    IMarkVector m_rSortedMarksStart;
    IMarkVector m_rSortedMarksEnd;

public:
    /// The main function to export the document.
    void ExportDocument( bool bWriteAll );

    /// Iterate through the nodes and call the appropriate OutputNode() on them.
    void WriteText();

    /// Set the pCurPam appropriately and call WriteText().
    ///
    /// Used to export paragraphs in footnotes/endnotes/etc.
    void WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp );

    /// Export the pool items to attributes (through an attribute output class).
    void ExportPoolItemsToCHP( sw::PoolItems &rItems, USHORT nScript );

    /// Return the numeric id of the numbering rule
    USHORT GetId( const SwNumRule& rNumRule );

    /// Return the numeric id of the style.
    USHORT GetId( const SwTxtFmtColl& rColl ) const;

    /// Return the numeric id of the style.
    USHORT GetId( const SwCharFmt& rFmt ) const;

    USHORT GetId( const SwTOXType& rTOXType );

    /// Return the numeric id of the font (and add it to the font list if needed)
    USHORT GetId( const SvxFontItem& rFont)
    {
        return maFontHelper.GetId(rFont);
    }
    /// @overload
    USHORT GetId( const wwFont& rFont)
    {
        return maFontHelper.GetId(rFont);
    }


    const SfxPoolItem& GetItem( USHORT nWhich ) const;

    /// Find the reference.
    bool HasRefToObject( USHORT nTyp, const String* pName, USHORT nSeqNo );

    /// Find the bookmark name.
    String GetBookmarkName( USHORT nTyp, const String* pName, USHORT nSeqNo );

    /// Add a bookmark converted to a Word name.
    void AppendWordBookmark( const String& rName );

    /// Use OutputItem() on an item set according to the parameters.
    void OutputItemSet( const SfxItemSet& rSet, bool bPapFmt, bool bChpFmt, USHORT nScript, bool bExportParentItemSet );

    short GetDefaultFrameDirection( ) const;

    /// Right to left?
    short TrueFrameDirection( const SwFrmFmt& rFlyFmt ) const;

    /// Right to left?
    short GetCurrentPageDirection() const;

    /// In case of numbering restart.

    /// List is set to restart at a particular value so for export make a
    /// completely new list based on this one and export that instead,
    /// which duplicates words behaviour in this respect.
    USHORT DuplicateNumRule( const SwNumRule *pRule, BYTE nLevel, USHORT nVal );

    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const = 0;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const = 0;

    /// Determines if the format is expected to support unicode.
    virtual bool SupportsUnicode() const = 0;

    /// Used to filter out attributes that can be e.g. written to .doc but not to .docx
    virtual bool ignoreAttributeForStyles( USHORT /*nWhich*/ ) const { return false; }

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
    virtual bool CollapseScriptsforWordOk( USHORT nScript, USHORT nWhich ) = 0;

    virtual void AppendBookmarks( const SwTxtNode& rNd, xub_StrLen nAktPos, xub_StrLen nLen ) = 0;

    virtual void AppendBookmark( const rtl::OUString& rName, bool bSkip = false ) = 0;

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
    virtual void SaveData( ULONG nStt, ULONG nEnd );

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
    ULONG GetSectionLineNo( const SfxItemSet* pSet, const SwNode& rNd ) const;

    /// Start new section.
    void OutputSectionBreaks( const SfxItemSet *pSet, const SwNode& rNd );

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
    void SubstituteBullet( String& rNumStr, rtl_TextEncoding& rChrSet,
        String& rFontName ) const;

    /// No-op for the newer WW versions.
    virtual void OutputOlst( const SwNumRule& /*rRule*/ ) {}

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* /*pA*/ ) {}

    /// Top node of host page when in header/footer.
    void SetHdFtPageRoot( const SwTxtNode *pNd ) { mpTopNodeOfHdFtPage = pNd; }

    /// Top node of host page when in header/footer.
    const SwTxtNode *GetHdFtPageRoot() const { return mpTopNodeOfHdFtPage; }

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( BYTE nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt,
        BYTE nBreakCode) = 0;

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const String& rFldCmd, BYTE nMode = nsFieldFlags::WRITEFIELD_ALL ) = 0;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark ) = 0;

    virtual void DoComboBox(const rtl::OUString &rName,
                    const rtl::OUString &rHelp,
                    const rtl::OUString &ToolTip,
                    const rtl::OUString &rSelected,
                    com::sun::star::uno::Sequence<rtl::OUString> &rListItems) = 0;

    virtual void DoFormText(const SwInputField * pFld) = 0;

    static bool NoPageBreakSection( const SfxItemSet *pSet );

    // Compute the number format for WW dates
    bool GetNumberFmt(const SwField& rFld, String& rStr);

    virtual ULONG ReplaceCr( BYTE nChar ) = 0;

    const SfxPoolItem* HasItem( USHORT nWhich ) const;


protected:
    /// Format-dependant part of the actual export.
    virtual void ExportDocument_Impl() = 0;

    /// Get the next position in the text node to output
    virtual xub_StrLen GetNextPos( SwWW8AttrIter* pAttrIter, const SwTxtNode& rNode, xub_StrLen nAktPos );

    /// Update the information for GetNextPos().
    virtual void UpdatePosition( SwWW8AttrIter* pAttrIter, xub_StrLen nAktPos, xub_StrLen nEnd );

    /// Output SwTxtNode
    virtual void OutputTextNode( const SwTxtNode& );

    /// Output SwTableNode
    void OutputTableNode( const SwTableNode& );

    /// Setup the chapter fields (maChapterFieldLocs).
    void GatherChapterFields();

    void AddLinkTarget( const String& rURL );
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

protected:
    /// Output SwStartNode
    virtual void OutputStartNode( const SwStartNode& );

    /// Output SwEndNode
    virtual void OutputEndNode( const SwEndNode& );

    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& ) = 0;

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& ) = 0;

    virtual void OutputLinkedOLE( const rtl::OUString& ) = 0;


    /// Output SwSectionNode
    virtual void OutputSectionNode( const SwSectionNode& );

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, ULONG nLnNum ) = 0;

    /// Call the right (virtual) function according to the type of the item.
    ///
    /// One of OutputTextNode(), OutputGrfNode(), or OutputOLENode()
    void OutputContentNode( const SwCntntNode& );

    /// Find the nearest bookmark from the current position.
    ///
    /// Returns false when there is no bookmark.
    bool NearestBookmark( xub_StrLen& rNearest, const xub_StrLen nAktPos, bool bNextPositionOnly );

    void GetSortedBookmarks( const SwTxtNode& rNd, xub_StrLen nAktPos,
                xub_StrLen nLen );

    bool GetBookmarks( const SwTxtNode& rNd, xub_StrLen nStt, xub_StrLen nEnd,
            IMarkVector& rArr );

public:
    MSWordExportBase( SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam );
    virtual ~MSWordExportBase();

    // TODO move as much as possible here from WW8Export! ;-)

    static void CorrectTabStopInSet( SfxItemSet& rSet, USHORT nAbsLeft );

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
    SwWW8Writer( const String& rFltName, const String& rBaseURL );
    virtual ~SwWW8Writer();

    virtual ULONG WriteStorage();
    virtual ULONG WriteMedium( SfxMedium& );

    // TODO most probably we want to be able to get these in
    // MSExportFilterBase
    using Writer::getIDocumentSettingAccess;

public:
#if 1
    /// Prefer ww::bytes to WW8Bytes, migrate away from the other ones.
    static void InsUInt16(ww::bytes &rO, sal_uInt16 n);
    static void InsUInt32(ww::bytes &rO, sal_uInt32 n);
    static void InsAsString16(ww::bytes &rO, const String& rStr);
    static void InsAsString8(ww::bytes & O, const String& rStr,
        rtl_TextEncoding eCodeSet);
#endif

    static void InsUInt16( WW8Bytes& rO, UINT16 );
    static void InsUInt32( WW8Bytes& rO, UINT32 );
    static void InsAsString16( WW8Bytes& rO, const String& );
    static void InsAsString8( WW8Bytes& rO, const String& rStr,
                                rtl_TextEncoding eCodeSet );

    static ULONG FillUntil( SvStream& rStrm, ULONG nEndPos = 0 );
    static void FillCount( SvStream& rStrm, ULONG nCount );

    static void WriteShort( SvStream& rStrm, INT16 nVal ) { rStrm << nVal; }
    static void WriteShort( SvStream& rStrm, ULONG nPos, INT16 nVal );

    static void WriteLong( SvStream& rStrm, INT32 nVal ) { rStrm << nVal; }
    static void WriteLong( SvStream& rStrm, ULONG nPos, INT32 nVal );

    static void WriteString16(SvStream& rStrm, const String& rStr,
        bool bAddZero);
    static void WriteString8(SvStream& rStrm, const String& rStr,
        bool bAddZero, rtl_TextEncoding eCodeSet);

    static void WriteString_xstz(SvStream& rStrm, const String& rStr, bool bAddZero);

    String GetPassword();

    using StgWriter::Write;
    virtual ULONG Write( SwPaM&, SfxMedium&, const String* = 0 );

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
    WW8Bytes* pO;                       ///< Buffer
    WW8Bytes* mpTableAt;                ///< Additional buffer for the output of the tables
    sal_uInt16 mnTableStdAtLen;         ///< Standard length of mpTableAt

    SvStream *pTableStrm, *pDataStrm;   ///< Streams for WW97 Export

    WW8Fib* pFib;                       ///< File Information Block
    WW8Dop* pDop;                       ///< DOcument Properties
    WW8_WrPlcFtnEdn *pFtn;              ///< Footnotes - structure to remember them, and output
    WW8_WrPlcFtnEdn *pEdn;              ///< Endnotes - structure to remember them, and output
    WW8_WrPlcSepx* pSepx;               ///< Sections/headers/footers

    BYTE bWrtWW8 : 1;                   ///< Write WW95 (false) or WW97 (true) file format

protected:
    SwWW8Writer        *m_pWriter;      ///< Pointer to the writer
    WW8AttributeOutput *m_pAttrOutput;  ///< Converting attributes to stream data

public:
    /// Access to the attribute output class.
    virtual AttributeOutputBase& AttrOutput() const;

    /// Access to the sections/headers/footres.
    virtual MSWordSections& Sections() const;

    /// False for WW6, true for WW8.
    virtual bool SupportsUnicode() const { return bWrtWW8; }

private:
    /// Format-dependant part of the actual export.
    virtual void ExportDocument_Impl();

    void PrepareStorage();
    void WriteFkpPlcUsw();
    void WriteMainText();
    void StoreDoc1();
    void Out_WwNumLvl( BYTE nWwLevel );
    void BuildAnlvBulletBase( WW8_ANLV& rAnlv, BYTE*& rpCh, USHORT& rCharLen,
                              const SwNumFmt& rFmt );
    static void BuildAnlvBase( WW8_ANLV& rAnlv, BYTE*& rpCh, USHORT& rCharLen,
                   const SwNumRule& rRul, const SwNumFmt& rFmt, BYTE nSwLevel );

    void Out_BorderLine(WW8Bytes& rO, const SvxBorderLine* pLine,
        USHORT nDist, sal_uInt16 nSprmNo, bool bShadow);

    /// Output the numbering table.
    virtual void WriteNumbering();

    void OutOverrideListTab();
    void OutListNamesTab();

    void RestoreMacroCmds();

    void InitFontTable();

    void DoComboBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);
    void DoCheckBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);

public:
    virtual void OutputOlst( const SwNumRule& rRule );

    /// Setup the pA's info.
    virtual void SetupSectionPositions( WW8_PdAttrDesc* pA );

    void Out_SwNumLvl( BYTE nSwLevel );
    void Out_NumRuleAnld( const SwNumRule& rRul, const SwNumFmt& rFmt,
                          BYTE nSwLevel );

    bool MiserableFormFieldExportHack(const SwFrmFmt& rFrmFmt);

    SvxMSExportOLEObjects& GetOLEExp()      { return *pOLEExp; }
    SwMSConvertControls& GetOCXExp()        { return *pOCXExp; }
    WW8OleMaps& GetOLEMap()                 { return *pOleMap; }
    void ExportDopTypography(WW8DopTypography &rTypo);

    USHORT AddRedlineAuthor( USHORT nId );

    void WriteFtnBegin( const SwFmtFtn& rFtn, WW8Bytes* pO = 0 );
    void WritePostItBegin( WW8Bytes* pO = 0 );
    const SvxBrushItem* GetCurrentPageBgBrush() const;
    SvxBrushItem TrueFrameBgBrush(const SwFrmFmt &rFlyFmt) const;

    /// Output all textframes anchored as character for the winword 7- format.
    void OutWW6FlyFrmsInCntnt( const SwTxtNode& rNd );

    void AppendFlyInFlys(const sw::Frame& rFrmFmt, const Point& rNdTopLeft);
    void WriteOutliner(const OutlinerParaObject& rOutliner, BYTE nTyp);
    void WriteSdrTextObj(const SdrObject& rObj, BYTE nTyp);

    UINT32 GetSdrOrdNum( const SwFrmFmt& rFmt ) const;
    void CreateEscher();
    void WriteEscher();

    bool Out_SwNum(const SwTxtNode* pNd);

    /// Write the field
    virtual void OutputField( const SwField* pFld, ww::eField eFldType,
            const String& rFldCmd, BYTE nMode = nsFieldFlags::WRITEFIELD_ALL );

    void StartCommentOutput( const String& rName );
    void EndCommentOutput(   const String& rName );
    void OutGrf(const sw::Frame &rFrame);
    bool TestOleNeedsGraphic(const SwAttrSet& rSet, SvStorageRef xOleStg,
        SvStorageRef xObjStg, String &rStorageName, SwOLENode *pOLENd);

    virtual void AppendBookmarks( const SwTxtNode& rNd, xub_StrLen nAktPos, xub_StrLen nLen );
    virtual void AppendBookmark( const rtl::OUString& rName, bool bSkip = false );
    void MoveFieldMarks(ULONG nFrom, ULONG nTo);

    void WriteAsStringTable(const ::std::vector<String>&, INT32& rfcSttbf,
        INT32& rlcbSttbf, USHORT nExtraLen = 0);

    virtual ULONG ReplaceCr( BYTE nChar );

    virtual void WriteCR( ww8::WW8TableNodeInfoInner::Pointer_t pTableTextNodeInfoInner = ww8::WW8TableNodeInfoInner::Pointer_t() );
    void WriteChar( sal_Unicode c );

    void OutSwString(const String&, xub_StrLen nStt, xub_StrLen nLen,
        bool bUnicode, rtl_TextEncoding eChrSet);

    WW8_CP Fc2Cp( ULONG nFc ) const          { return pPiece->Fc2Cp( nFc ); }

            // einige z.T. static halb-interne Funktions-Deklarationen

    void OutSprmBytes( BYTE* pBytes, USHORT nSiz )
                                { pO->Insert( pBytes, nSiz, pO->Count() ); }

    inline bool IsUnicode() const           { return pPiece->IsUnicode(); }

    virtual void SectionBreaksAndFrames( const SwTxtNode& rNode );

    /// Helper method for OutputSectionBreaks() and OutputFollowPageDesc().
    // #i76300#
    virtual void PrepareNewPageDesc( const SfxItemSet* pSet,
                                     const SwNode& rNd,
                                     const SwFmtPageDesc* pNewPgDescFmt = 0,
                                     const SwPageDesc* pNewPgDesc = 0 );

    void Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow);
    void Out_SwFmtTableBox( WW8Bytes& rO, const SvxBoxItem * rBox );
    BYTE TransCol( const Color& rCol );
    bool TransBrush(const Color& rCol, WW8_SHD& rShd);
    WW8_BRC TranslateBorderLine(const SvxBorderLine& pLine,
        USHORT nDist, bool bShadow);

    // #i77805# - new return value indicates, if an inherited outline numbering is suppressed
    virtual bool DisallowInheritingOutlineNumbering(const SwFmt &rFmt);

    unsigned int GetHdFtIndex() const { return mnHdFtIndex; }
    void SetHdFtIndex(unsigned int nHdFtIndex) { mnHdFtIndex = nHdFtIndex; }
    void IncrementHdFtIndex() { ++mnHdFtIndex; }

    static long GetDTTM( const DateTime& rDT );

    /// Convert the SVX numbering type to id
    static BYTE GetNumId( USHORT eNumType );

    /// Guess the script (asian/western).
    virtual bool CollapseScriptsforWordOk( USHORT nScript, USHORT nWhich );

    USHORT DupNumRuleWithLvlStart(const SwNumRule *pRule,BYTE nLvl,USHORT nVal);

    SwTwips CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const;

    /// Nasty swap for bidi if neccessary
    bool MiserableRTLFrmFmtHack(SwTwips &rLeft, SwTwips &rRight,
        const sw::Frame &rFrmFmt);

    void InsUInt16( UINT16 n )      { SwWW8Writer::InsUInt16( *pO, n ); }
    void InsUInt32( UINT32 n )      { SwWW8Writer::InsUInt32( *pO, n ); }
    void InsAsString16( const String& rStr )
                        { SwWW8Writer::InsAsString16( *pO, rStr ); }
    void InsAsString8( const String& rStr, rtl_TextEncoding eCodeSet )
                        { SwWW8Writer::InsAsString8( *pO, rStr, eCodeSet ); }
    void WriteStringAsPara( const String& rTxt, USHORT nStyleId = 0 );

    /// Setup the exporter.
    WW8Export( SwWW8Writer *pWriter,
            SwDoc *pDocument, SwPaM *pCurrentPam, SwPaM *pOriginalPam,
            bool bIsWW8 );
    virtual ~WW8Export();

    virtual void DoComboBox(const rtl::OUString &rName,
                    const rtl::OUString &rHelp,
                    const rtl::OUString &ToolTip,
                    const rtl::OUString &rSelected,
                    com::sun::star::uno::Sequence<rtl::OUString> &rListItems);

    virtual void DoFormText(const SwInputField * pFld);

    void GetCurrentItems(WW8Bytes &rItems) const;

    /// Write the data of the form field
    virtual void WriteFormData( const ::sw::mark::IFieldmark& rFieldmark );
    virtual void WriteHyperlinkData( const ::sw::mark::IFieldmark& rFieldmark );

    /// Fields.
    WW8_WrPlcFld* CurrentFieldPlc() const;

    SwWW8Writer& GetWriter() const { return *m_pWriter; }
    SvStream& Strm() const { return m_pWriter->Strm(); }

    /// Remember some of the memebers so that we can recurse in WriteText().
    virtual void SaveData( ULONG nStt, ULONG nEnd );

    /// Restore what was saved in SaveData().
    virtual void RestoreData();

    /// Output the actual headers and footers.
    virtual void WriteHeadersFooters( BYTE nHeadFootFlags,
            const SwFrmFmt& rFmt, const SwFrmFmt& rLeftFmt, const SwFrmFmt& rFirstPageFmt,
        BYTE nBreakCode);

protected:
    /// Output SwGrfNode
    virtual void OutputGrfNode( const SwGrfNode& );

    /// Output SwOLENode
    virtual void OutputOLENode( const SwOLENode& );

    virtual void OutputLinkedOLE( const rtl::OUString& );

    virtual void AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, ULONG nLnNum );

private:
    /// No copying.
    WW8Export(const WW8Export&);
    /// No copying.
    WW8Export& operator=(const WW8Export&);
};

class WW8_WrPlcSubDoc   // Doppel-Plc fuer Foot-/Endnotes und Postits
{
private:
    //No copying
    WW8_WrPlcSubDoc(const WW8_WrPlcSubDoc&);
    WW8_WrPlcSubDoc& operator=(const WW8_WrPlcSubDoc&);
protected:
    SvULongs aCps;                  // PTRARR CP-Pos der Verweise
    SvPtrarr aCntnt;                // PTRARR von SwFmtFtn/PostIts/..
    WW8_WrPlc0* pTxtPos;            // Pos der einzelnen Texte

    WW8_WrPlcSubDoc();
    virtual ~WW8_WrPlcSubDoc();

    bool WriteGenericTxt( WW8Export& rWrt, BYTE nTTyp, WW8_CP& rCount );
    void WriteGenericPlc( WW8Export& rWrt, BYTE nTTyp, WW8_FC& rTxtStt,
        sal_Int32& rTxtCnt, WW8_FC& rRefStt, sal_Int32& rRefCnt ) const;

    virtual const SvULongs* GetShapeIdArr() const;
};

// Doppel-Plc fuer Footnotes/Endnotes
class WW8_WrPlcFtnEdn : public WW8_WrPlcSubDoc
{
private:
    BYTE nTyp;

    //No copying
    WW8_WrPlcFtnEdn(const WW8_WrPlcFtnEdn&);
    WW8_WrPlcFtnEdn& operator=(WW8_WrPlcFtnEdn &);
public:
    WW8_WrPlcFtnEdn( BYTE nTTyp ) : nTyp( nTTyp ) {}

    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;

    void Append( WW8_CP nCp, const SwFmtFtn& rFtn );
};

struct WW8_Annotation
{
    const OutlinerParaObject* mpRichText;
    String msSimpleText;
    String msOwner;
    DateTime maDateTime;
    WW8_Annotation(const SwPostItField* pPostIt);
    WW8_Annotation(const SwRedlineData* pRedline);
};

class WW8_WrPlcAnnotations : public WW8_WrPlcSubDoc  // Doppel-Plc fuer PostIts
{
private:
    //No copying
    WW8_WrPlcAnnotations(const WW8_WrPlcAnnotations&);
    WW8_WrPlcAnnotations& operator=(WW8_WrPlcAnnotations&);
    std::set<const SwRedlineData*> maProcessedRedlines;
public:
    WW8_WrPlcAnnotations() {}
    ~WW8_WrPlcAnnotations();

    void Append( WW8_CP nCp, const SwPostItField* pPostIt );
    void Append( WW8_CP nCp, const SwRedlineData* pRedLine );
    bool IsNewRedlineComment( const SwRedlineData* pRedLine );
    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
};

class WW8_WrPlcTxtBoxes : public WW8_WrPlcSubDoc // Doppel-Plc fuer Textboxen
{                        // Rahmen/DrawTextboxes!
private:
    BYTE nTyp;
    SvULongs aShapeIds;        // VARARR of ShapeIds for the SwFrmFmts
    virtual const SvULongs* GetShapeIdArr() const;

    //No copying
    WW8_WrPlcTxtBoxes(const WW8_WrPlcTxtBoxes&);
    WW8_WrPlcTxtBoxes& operator=(WW8_WrPlcTxtBoxes&);
public:
    WW8_WrPlcTxtBoxes( BYTE nTTyp ) : nTyp( nTTyp ) {}

    bool WriteTxt( WW8Export& rWrt );
    void WritePlc( WW8Export& rWrt ) const;
    void Append( const SdrObject& rObj, UINT32 nShapeId );
    USHORT Count() const { return aCntnt.Count(); }
    USHORT GetPos( const VoidPtr& p ) const { return aCntnt.GetPos( p ); }
};

typedef WW8_WrFkp* WW8_FkpPtr;  // Plc fuer Chpx und Papx ( incl PN-Plc )
SV_DECL_PTRARR( WW8_WrFkpPtrs, WW8_FkpPtr, 4, 4 )

class WW8_WrPlcPn                   // Plc fuer Page Numbers
{
private:
    WW8Export& rWrt;
    WW8_WrFkpPtrs aFkps;            // PTRARR
    USHORT nFkpStartPage;
    ePLCFT ePlc;
    bool bWrtWW8;                   // Fuer Writererkennung
    USHORT nMark;

    //No copying
    WW8_WrPlcPn(const WW8_WrPlcPn&);
    WW8_WrPlcPn& operator=(const WW8_WrPlcPn&);
public:
    WW8_WrPlcPn( WW8Export& rWrt, ePLCFT ePl, WW8_FC nStartFc );
    ~WW8_WrPlcPn();
    void AppendFkpEntry(WW8_FC nEndFc,short nVarLen = 0,const BYTE* pSprms = 0);
    void WriteFkps();
    void WritePlc();
    BYTE *CopyLastSprms(BYTE &rLen);
};

// class WW8_WrPlc1 ist erstmal nur fuer Felder
class WW8_WrPlc1
{
private:
    SvULongs aPos;              // PTRARR von CPs
    BYTE* pData;                // Inhalte ( Strukturen )
    ULONG nDataLen;
    USHORT nStructSiz;

    //No copying
    WW8_WrPlc1(const WW8_WrPlc1&);
    WW8_WrPlc1& operator=(const WW8_WrPlc1&);
protected:
    USHORT Count() const { return aPos.Count(); }
    void Write( SvStream& rStrm );
    WW8_CP Prev() const;
public:
    WW8_WrPlc1( USHORT nStructSz );
    ~WW8_WrPlc1();
    void Append( WW8_CP nCp, const void* pData );
    void Finish( ULONG nLastCp, ULONG nStartCp );
};

// class WW8_WrPlcFld ist fuer Felder
class WW8_WrPlcFld : public WW8_WrPlc1
{
private:
    BYTE nTxtTyp;
    USHORT nResults;

    //No copying
    WW8_WrPlcFld(const WW8_WrPlcFld&);
    WW8_WrPlcFld& operator=(const WW8_WrPlcFld&);
public:
    WW8_WrPlcFld( USHORT nStructSz, BYTE nTTyp )
        : WW8_WrPlc1( nStructSz ), nTxtTyp( nTTyp ), nResults(0)
    {}
    bool Write( WW8Export& rWrt );
    void ResultAdded() { ++nResults; }
    USHORT ResultCount() const { return nResults; }
};

class WW8_WrMagicTable : public WW8_WrPlc1
{
private:
    //No copying
    WW8_WrMagicTable(const WW8_WrMagicTable&);
    WW8_WrMagicTable& operator=(const WW8_WrMagicTable&);
public:
    WW8_WrMagicTable() : WW8_WrPlc1( 4 ) {Append(0,0);}
    void Append( WW8_CP nCp, ULONG nData );
    bool Write( WW8Export& rWrt );
};

class GraphicDetails
{
public:
    sw::Frame maFly;            // Umgebende FlyFrms dazu
    ULONG mnPos;                // FilePos der Grafiken
    UINT16 mnWid;               // Breite der Grafiken
    UINT16 mnHei;               // Hoehe der Grafiken

    GraphicDetails(const sw::Frame &rFly, UINT16 nWid, UINT16 nHei)
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

// class SwWW8WrGrf sammelt Grafiken und gibt sie aus
class SwWW8WrGrf
{
private:
    /// for access to the variables
    WW8Export& rWrt;

    std::vector<GraphicDetails> maDetails;
    typedef std::vector<GraphicDetails>::iterator myiter;
    USHORT mnIdx;       // Index in File-Positionen

    void WritePICFHeader(SvStream& rStrm, const sw::Frame &rFly,
            UINT16 mm, UINT16 nWidth, UINT16 nHeight,
            const SwAttrSet* pAttrSet = 0);
    void WriteGraphicNode(SvStream& rStrm, const GraphicDetails &rItem);
    void WriteGrfFromGrfNode(SvStream& rStrm, const SwGrfNode &rNd,
        const sw::Frame &rFly, UINT16 nWidth, UINT16 nHeight);

    //No copying
    SwWW8WrGrf(const SwWW8WrGrf&);
    SwWW8WrGrf& operator=(const SwWW8WrGrf&);
public:
    SwWW8WrGrf( WW8Export& rW ) : rWrt( rW ), mnIdx( 0 ) {}
    void Insert(const sw::Frame &rFly);
    void Write();
    ULONG GetFPos()
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

    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const = 0;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const = 0;
};

/// Used to export formatted text associated to drawings.
class MSWord_SdrAttrIter : public MSWordAttrIter
{
private:
    const EditTextObject* pEditObj;
    const SfxItemPool* pEditPool;
    EECharAttribArray aTxtAtrArr;
    SvPtrarr aChrTxtAtrArr;
    SvUShorts aChrSetArr;
    USHORT nPara;
    xub_StrLen nAktSwPos;
    xub_StrLen nTmpSwPos;                   // for HasItem()
    rtl_TextEncoding eNdChrSet;
    USHORT nScript;
    BYTE mnTyp;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    void SetCharSet(const EECharAttrib& rTxtAttr, bool bStart);

    //No copying
    MSWord_SdrAttrIter(const MSWord_SdrAttrIter&);
    MSWord_SdrAttrIter& operator=(const MSWord_SdrAttrIter&);
public:
    MSWord_SdrAttrIter( MSWordExportBase& rWr, const EditTextObject& rEditObj,
        BYTE nType );
    void NextPara( USHORT nPar );
    void OutParaAttr(bool bCharAttr);
    void OutEEField(const SfxPoolItem& rHt);

    bool IsTxtAttr(xub_StrLen nSwPos);

    void NextPos() { if ( nAktSwPos < STRING_NOTFOUND ) nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( xub_StrLen nSwPos );
    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const;
    bool OutAttrWithRange(xub_StrLen nPos);
    xub_StrLen WhereNext() const                { return nAktSwPos; }
    rtl_TextEncoding GetNextCharSet() const;
    rtl_TextEncoding GetNodeCharSet() const     { return eNdChrSet; }
};

// Die Klasse SwWW8AttrIter ist eine Hilfe zum Aufbauen der Fkp.chpx.
// Dabei werden nur Zeichen-Attribute beachtet; Absatz-Attribute brauchen
// diese Behandlung nicht.
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

    const SwRedline* pCurRedline;
    xub_StrLen nAktSwPos;
    USHORT nCurRedlinePos;

    bool mbParaIsRTL;

    const SwFmtDrop &mrSwFmtDrop;

    sw::Frames maFlyFrms;     // #i2916#
    sw::FrameIter maFlyIter;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    void FieldVanish( const String& rTxt );

    void OutSwFmtRefMark(const SwFmtRefMark& rAttr, bool bStart);

    void IterToCurrent();

    //No copying
    SwWW8AttrIter(const SwWW8AttrIter&);
    SwWW8AttrIter& operator=(const SwWW8AttrIter&);
public:
    SwWW8AttrIter( MSWordExportBase& rWr, const SwTxtNode& rNd );

    bool IsTxtAttr( xub_StrLen nSwPos );
    bool IsRedlineAtEnd( xub_StrLen nPos ) const;
    bool IsDropCap( int nSwPos );
    bool RequiresImplicitBookmark();

    void NextPos() { if ( nAktSwPos < STRING_NOTFOUND ) nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( xub_StrLen nSwPos, bool bRuby = false );
    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const;
    int OutAttrWithRange(xub_StrLen nPos);
    const SwRedlineData* GetRedline( xub_StrLen nPos );
    void OutFlys(xub_StrLen nSwPos);

    xub_StrLen WhereNext() const    { return nAktSwPos; }
    sal_uInt16 GetScript() const { return mnScript; }
    bool IsCharRTL() const { return mbCharIsRTL; }
    bool IsParaRTL() const { return mbParaIsRTL; }
    rtl_TextEncoding GetCharSet() const { return meChrSet; }
    String GetSnippet(const String &rStr, xub_StrLen nAktPos,
        xub_StrLen nLen) const;
    const SwFmtDrop& GetSwFmtDrop() const { return mrSwFmtDrop; }
};

/// Class to collect and output the styles table.
class MSWordStyles
{
    MSWordExportBase& m_rExport;
    SwFmt** pFmtA;
    USHORT nUsedSlots;

    /// Create the style table, called from the constructor.
    void BuildStylesTable();

    /// Get slot number during building the style table.
    USHORT BuildGetSlot( const SwFmt& rFmt );

    /// Return information about one style.
    void GetStyleData( SwFmt* pFmt, bool& bFmtColl, USHORT& nBase, USHORT& nNext );

    /// Outputs attributes of one style.
    void WriteProperties( const SwFmt* pFmt, bool bPap, USHORT nPos, bool bInsDefCharSiz );

    USHORT GetWWId( const SwFmt& rFmt ) const;

    void SetStyleDefaults( const SwFmt& rFmt, bool bPap );

    /// Outputs one style - called (in a loop) from OutputStylesTable().
    void OutputStyle( SwFmt* pFmt, USHORT nPos );

    // No copying
    MSWordStyles( const MSWordStyles& );
    MSWordStyles& operator=( const MSWordStyles& );

public:
    MSWordStyles( MSWordExportBase& rExport );
    ~MSWordStyles();

    /// Output the styles table.
    void OutputStylesTable();

    /// Get id of the style (rFmt).
    USHORT GetSlot( const SwFmt& rFmt ) const;

    SwFmt* GetSwFmt() { return (*pFmtA); }
};

sal_Int16 GetWordFirstLineOffset(const SwNumFmt &rFmt);
//A bit of a bag on the side for now
String FieldString(ww::eField eIndex);
String BookmarkToWord(const String &rBookmark);

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

/// For the output of sections.
struct WW8_PdAttrDesc
{
    BYTE* pData;
    USHORT nLen;
    WW8_FC nSepxFcPos;
};

#endif  //  _WRTWW8_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
