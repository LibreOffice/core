/*************************************************************************
 *
 *  $RCSfile: wrtww8.hxx,v $
 *
 *  $Revision: 1.60 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:56:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _WRTWW8_HXX
#define _WRTWW8_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>        // UINTXX
#endif
#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svtools/svstdarr.hxx>
#endif

#ifndef __SGI_STL_MAP
#include <map>
#endif
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef WRT_FN_HXX
#include <wrt_fn.hxx>
#endif
#ifndef _ORNTENUM_HXX
#include <orntenum.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#ifndef WW8STRUC_HXX
#include "ww8struc.hxx"
#endif
#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"
#endif
#ifndef WW_FIELDS_HXX
#include "fields.hxx"
#endif

#ifndef WW_TYPES
#include "types.hxx"
#endif
#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif

// einige Forward Deklarationen
class BitmapPalette;
class SwEscherEx;
class DateTime;
class Font;
class SdrObject;
class SfxItemSet;
class SvStream;
class SvxBorderLine;
class SvxFontItem;
class SvxBoxItem;
class SwAttrSet;
class SwBookmark;
class SwCharFmt;
class SwCntntNode;
class SwField;
class SwFlyFrmFmt;
class SwFmt;
class SwFmtCntnt;
class SwFmtFtn;
class SwFmtINetFmt;
class SwFmtRefMark;
class SwFrmFmt;
class SwGrfNode;
class SwModify;
class SwNodeNum;
class SwNoTxtNode;
class SwNumFmt;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwFmtPageDesc;
class SwpHints;
class SwOLENode;
class SwPosFlyFrms;
class SwPostItField;
class SwRedlineData;
class SwSection;
class SwSectionFmt;
class SwSectionNode;
class SwTableNode;
class SwTOXType;
class SwTOXMark;
class SwTxtAttr;
class SwTxtFmtColl;
class SwTxtNode;
class SwWW8WrGrf;
class SwWW8Writer;
class WW8WrtStyle;
class WW8Bytes;
class WW8_AttrIter;
class WW8_WrFkp;
class WW8_WrPlc0;
class WW8_WrPlc1;
class WW8_WrPlcFld;
class WW8_WrMagicTable;
class WW8_WrPlcFtnEdn;
class WW8_WrPlcPn;
class WW8_WrPlcPostIt;
class WW8_WrPlcSepx;
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

#define GRF_MAGIC_1 0x12    // 3 magic Bytes fuer PicLocFc-Attribute
#define GRF_MAGIC_2 0x34
#define GRF_MAGIC_3 0x56
#define GRF_MAGIC_321 0x563412L

#define OLE_PREVIEW_AS_EMF  //If we want to export ole2 previews as emf in ww8+

enum FieldFlags // enums for InsertField- Method
{
    WRITEFIELD_START = 0x01, WRITEFIELD_CMD_START = 0x02,
    WRITEFIELD_CMD_END = 0x04, WRITEFIELD_END = 0x10, WRITEFIELD_CLOSE = 0x20,
    WRITEFIELD_ALL = 0xFF
};

enum TxtTypes  //enums for TextTypes
{
    TXT_MAINTEXT = 0, /*TXT_FTNEDN = 1,*/ TXT_HDFT = 2, TXT_FTN = 3,
    TXT_EDN = 4, TXT_ATN = 5, TXT_TXTBOX = 6, TXT_HFTXTBOX= 7
};

extern SwNodeFnTab aWW8NodeFnTab;
extern SwAttrFnTab aWW8AttrFnTab;

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
        ULONG nLnRestart )
        : pPageDesc(pPD), pSectionFmt(pFmt), pPDNd(0), pNumNd(0),
        nLnNumRestartNo(nLnRestart), nPgRestartNo(0)
    {}
    bool IsProtected() const;
};
SV_DECL_VARARR( WW8_WrSepInfoPtrs, WW8_SepInfo, 4, 4 )


class WW8_WrPlcSepx     // Plc fuer PageDescs -> Sepx ( Section Extensions )
{
private:
    bool mbDocumentIsProtected;
    WW8_WrSepInfoPtrs aSects;   // PTRARR von SwPageDesc und SwSectionFmt
    SvULongs aCps;              // PTRARR von CPs
    WW8_PdAttrDesc* pAttrs;
    WW8_WrPlc0* pTxtPos;        // Pos der einzelnen Header / Footer

    void CheckForFacinPg( SwWW8Writer& rWrt ) const;
    void WriteOlst( SwWW8Writer& rWrt, USHORT i );
    void WriteFtnEndTxt( SwWW8Writer& rWrt, ULONG nCpStt );
    void OutHeader( SwWW8Writer& rWrt, const SwFmt& rFmt,
                        ULONG& rCpPos, BYTE nHFFlags, BYTE nFlag );
    void OutFooter( SwWW8Writer& rWrt, const SwFmt& rFmt,
                        ULONG& rCpPos, BYTE nHFFlags, BYTE nFlag );
    static void SetHeaderFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                  BYTE nFlag );
    static void SetFooterFlag( BYTE& rHeadFootFlags, const SwFmt& rFmt,
                                   BYTE nFlag );
    static int HasBorderItem( const SwFmt& rFmt );

    void NeedsDocumentProtected(const WW8_SepInfo &rInfo);

    //No copy, no assign
    WW8_WrPlcSepx(const WW8_WrPlcSepx&);
    WW8_WrPlcSepx& operator=(const WW8_WrPlcSepx&);
public:
    WW8_WrPlcSepx();
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
    void SetNum( const SwTxtNode* pNumNd );
    bool WriteKFTxt( SwWW8Writer& rWrt );
    void WriteSepx( SvStream& rStrm ) const;
    void WritePlcSed( SwWW8Writer& rWrt ) const;
    void WritePlcHdd( SwWW8Writer& rWrt ) const;
    sal_uInt16 CurrentNoColumns(const SwDoc &rDoc) const;
    bool DocumentIsProtected() const { return mbDocumentIsProtected; }
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
    void WritePc(SwWW8Writer& rWrt);
    void SetParaBreak();
    bool IsUnicode() const  { return bIsUni; }
    ULONG Fc2Cp( ULONG nFc ) const;
};

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
public:
    wwFont(const String &rFamilyName, FontPitch ePitch, FontFamily eFamily,
        rtl_TextEncoding eChrSet, bool bWrtWW8);
    bool Write(SvStream *pTableStram) const;
    friend bool operator < (const wwFont &r1, const wwFont &r2);
};

class wwFontHelper
{
private:
    /*
     * Keep track of fonts that need to be exported.
    */
    ::std::map<wwFont, USHORT> maFonts;
    bool mbWrtWW8;
public:
    wwFontHelper() : mbWrtWW8(false) {}
    //rDoc used only to get the initial standard font(s) in use.
    void InitFontTable(bool bWrtWW8, const SwDoc& rDoc);
    USHORT GetId(const Font& rFont);
    USHORT GetId(const SvxFontItem& rFont);
    USHORT GetId(const wwFont& rFont);
    void WriteFontTable(SvStream *pTableStream, WW8Fib& pFib );
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
private:
    //No assignment
    DrawObj& operator=(const DrawObj&);
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
    void WritePlc(SwWW8Writer& rWrt) const;
    bool Append(SwWW8Writer&, WW8_CP nCp, const sw::Frame& rFmt,
        const Point& rNdTopLeft);
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

typedef ::std::pair<String, ULONG> aPair;
typedef std::vector<aPair> SwImplBookmarks;
typedef std::vector<aPair>::iterator SwImplBookmarksIter;


// der WW8-Writer
class SwWW8Writer: public StgWriter
{
friend bool WW8_WrPlcSepx::WriteKFTxt( SwWW8Writer& rWrt ); // pO
friend class WW8_WrPlcSepx;
friend Writer& OutWW8_SwTxtNode( Writer& rWrt, SwCntntNode& rNode );

    wwFontHelper maFontHelper;
    std::vector<ULONG> maChapterFieldLocs;
    typedef std::vector<ULONG>::const_iterator mycCFIter;
    String aMainStg;
    SvPtrarr aTOXArr;
    const SfxItemSet* pISet;    // fuer Doppel-Attribute
    WW8_WrPct*  pPiece;         // Pointer auf Piece-Table
    SwNumRuleTbl* pUsedNumTbl;  // alle used NumRules
    const SwTxtNode *mpTopNodeOfHdFtPage; //top node of host page when in hd/ft
    std::map<USHORT, USHORT> aRuleDuplicates; //map to Duplicated numrules
    std::stack<xub_StrLen> maCurrentCharPropStarts;
    WW8_WrtBookmarks* pBkmks;
    WW8_WrtRedlineAuthor* pRedlAuthors;
    BitmapPalette* pBmpPal;
    void* pKeyMap;
    SvxMSExportOLEObjects* pOLEExp;
    SwMSConvertControls* pOCXExp;
    WW8OleMaps* pOleMap;

    USHORT nCharFmtStart;
    USHORT nFmtCollStart;
    USHORT nStyleBeforeFly;     // Style-Nummer des Nodes,
                                //       in/an dem ein Fly verankert ist
    USHORT nLastFmtId;          // Style of last TxtNode in normal range
    USHORT nUniqueList;         // current number for creating unique list names
    unsigned int mnHdFtIndex;

    virtual ULONG WriteStorage();

    void PrepareStorage();
    void WriteFkpPlcUsw();
    void WriteMainText();
    void StoreDoc1();
    ULONG StoreDoc();
    void Out_Olst( const SwNumRule& rRule );
    void Out_WwNumLvl( BYTE nWwLevel );
    void Out_SwNumLvl( BYTE nSwLevel );
    void Out_NumRuleAnld( const SwNumRule& rRul, const SwNumFmt& rFmt,
                          BYTE nSwLevel );
    void BuildAnlvBulletBase( WW8_ANLV& rAnlv, BYTE*& rpCh, USHORT& rCharLen,
                              const SwNumFmt& rFmt );
    void SubstituteBullet(String& rNumStr, rtl_TextEncoding& rChrSet,
        String& rFontName) const;

    static void BuildAnlvBase( WW8_ANLV& rAnlv, BYTE*& rpCh, USHORT& rCharLen,
                   const SwNumRule& rRul, const SwNumFmt& rFmt, BYTE nSwLevel );

    void Out_BorderLine(WW8Bytes& rO, const SvxBorderLine* pLine,
        USHORT nDist, USHORT nSprmNo, bool bShadow);

    void OutListTab();
    void OutOverrideListTab();
    void OutListNamesTab();

    void RestoreMacroCmds();

    void InitFontTable();

    bool MiserableFormFieldExportHack(const SwFrmFmt& rFrmFmt);
    void DoComboBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);
    void DoCheckBox(com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> xPropSet);
    void GatherChapterFields();
    bool FmtHdFtContainsChapterField(const SwFrmFmt &rFmt) const;
    bool CntntContainsChapterField(const SwFmtCntnt &rCntnt) const;
public:

    /* implicit bookmark vector containing pairs of node indexes and bookmark names */
    SwImplBookmarks maImplicitBookmarks;
    sw::Frames maFrames;             // The floating frames in this document
    const SwPageDesc *pAktPageDesc;
    WW8Fib* pFib;
    WW8Dop* pDop;
    WW8_WrPlcPn* pPapPlc;
    WW8_WrPlcPn* pChpPlc;
    WW8_AttrIter* pChpIter;
    WW8WrtStyle* pStyles;
    WW8_WrPlcSepx* pSepx;
    WW8_WrPlcFtnEdn* pFtn;
    WW8_WrPlcFtnEdn* pEdn;
    WW8_WrPlcPostIt* pAtn;
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
    WW8_WrPlcFld* pFldTxtBxs;       // fields in textboxes
    WW8_WrPlcFld* pFldHFTxtBxs;     // fields in header/footer textboxes
    WW8_WrMagicTable *pMagicTable;  // keeps track of table cell positions, and
                                    // marks those that contain graphics,
                                    // which is required to make word display
                                    // graphics inside tables
    SwWW8WrGrf* pGrf;
    const SwAttrSet* pStyAttr;      // StyleAttr fuer Tabulatoren
    const SwModify* pOutFmtNode;    // write Format or Node

    MainTxtPlcDrawObj *pSdrObjs;   // Draw-/Fly-Objects
    HdFtPlcDrawObj *pHFSdrObjs;     // Draw-/Fly-Objects in header or footer

    WW8Bytes* pO;

    SvStream *pTableStrm, *pDataStrm;   // Streams fuer WW97 Export

    SwEscherEx* pEscher;            // escher export class
    SwTwips nFlyWidth, nFlyHeight;  // Fuer Anpassung Graphic

    BYTE nTxtTyp;

    BYTE bStyDef : 1;           // wird Style geschrieben ?
    BYTE bBreakBefore : 1;      // Breaks werden 2mal ausgegeben
    BYTE bOutKF : 1;            // Kopf/Fusstexte werden ausgegeben
    BYTE bOutFlyFrmAttrs : 1;   // Rahmen-Attr von Flys werden ausgegeben
    BYTE bOutPageDescs : 1;     // PageDescs werden ausgegeben ( am Doc-Ende )
    BYTE bOutFirstPage : 1;     // write Attrset of FirstPageDesc
    BYTE bOutTable : 1;         // Tabelle wird ausgegeben
                                //    ( wird zB bei Flys in Tabelle zurueckgesetzt )
    BYTE bIsInTable : 1;        // wird sind innerhalb der Ausgabe einer Tabelle
                                //    ( wird erst nach der Tabelle zurueckgesetzt )
    BYTE bOutGrf : 1;           // Grafik wird ausgegeben
    BYTE bWrtWW8 : 1;           // Schreibe WW95 oder WW97 FileFormat
    BYTE bInWriteEscher : 1;    // in write textboxes
    BYTE bStartTOX : 1;         // true: a TOX is startet
    BYTE bInWriteTOX : 1;       // true: all content are in a TOX
    BYTE bFtnAtTxtEnd : 1;      // true: all FTN at Textend
    BYTE bEndAtTxtEnd : 1;      // true: all END at Textend




    SvxMSExportOLEObjects& GetOLEExp()      { return *pOLEExp; }
    SwMSConvertControls& GetOCXExp()        { return *pOCXExp; }
    WW8OleMaps& GetOLEMap()                 { return *pOleMap; }
    void ExportDopTypography(WW8DopTypography &rTypo);

    const SfxPoolItem* HasItem( USHORT nWhich ) const;
    const SfxPoolItem& GetItem( USHORT nWhich ) const;

    USHORT GetId( const SwTxtFmtColl& rColl ) const;
    USHORT GetId( const SwCharFmt& rFmt ) const;
    USHORT GetId( const SwNumRule& rNumRule ) const;
    USHORT GetId( const SwTOXType& rTOXType );
    USHORT GetId( const SvxFontItem& rFont)
    {
        return maFontHelper.GetId(rFont);
    }

    void OutRedline( const SwRedlineData& rRedline );
    USHORT AddRedlineAuthor( USHORT nId );

    void StartTOX( const SwSection& rSect );
    void EndTOX( const SwSection& rSect );

    void WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp );
    void WriteKFTxt1( const SwFmtCntnt& rCntnt );
    void WriteFtnBegin( const SwFmtFtn& rFtn, WW8Bytes* pO = 0 );
    void WritePostItBegin( WW8Bytes* pO = 0 );
    short TrueFrameDirection(const SwFrmFmt &rFlyFmt) const;
    short GetCurrentPageDirection() const;
    const SvxBrushItem* GetCurrentPageBgBrush() const;
    SvxBrushItem TrueFrameBgBrush(const SwFrmFmt &rFlyFmt) const;
    void OutWW8FlyFrmsInCntnt( const SwTxtNode& rNd );
    void OutWW8FlyFrm(const sw::Frame& rFmt, const Point& rNdTopLeft);
    void OutFlyFrm(const sw::Frame& rFmt);
    void AppendFlyInFlys(const sw::Frame& rFrmFmt, const Point& rNdTopLeft);
    void WriteSdrTextObj(const SdrObject& rObj, BYTE nTyp);

    UINT32 GetSdrOrdNum( const SwFrmFmt& rFmt ) const;
    void CreateEscher();
    void WriteEscher();

    bool Out_SwNum(const SwTxtNode* pNd);
    void Out_SwFmt(const SwFmt& rFmt, bool bPapFmt, bool bChpFmt,
        bool bFlyFmt = false);
    bool GetNumberFmt(const SwField& rFld, String& rStr);
    void OutField(const SwField* pFld, ww::eField eFldType,
        const String& rFldCmd, BYTE nMode = WRITEFIELD_ALL);
    void StartCommentOutput( const String& rName );
    void EndCommentOutput(   const String& rName );
    void OutGrf(const sw::Frame &rFrame);
    bool TestOleNeedsGraphic(const SwAttrSet& rSet, SvStorageRef xOleStg,
        SvStorageRef xObjStg, String &rStorageName, SwOLENode *pOLENd);
    void AppendBookmarks( const SwTxtNode& rNd, xub_StrLen nAktPos,
        xub_StrLen nLen );
    void AppendBookmark( const String& rName, USHORT nOffset = 0 );
    String GetBookmarkName( USHORT nTyp, const String* pNm, USHORT nSeqNo );
    void MoveFieldBookmarks(ULONG nFrom, ULONG nTo);
    bool HasRefToObject(USHORT nTyp, const String* pNm, USHORT nSeqNo);

    void WriteAsStringTable(const ::std::vector<String>&, INT32& rfcSttbf,
        INT32& rlcbSttbf, USHORT nExtraLen = 0);
    void WriteText();
    void WriteCR();
    void WriteChar( sal_Unicode c );
    void WriteCellEnd();
    void WriteRowEnd();
    USHORT StartTableFromFrmFmt(WW8Bytes &rAt, const SwFrmFmt *pFmt,
        SwTwips &rPageSize);

    void OutSwString(const String&, xub_StrLen nStt, xub_StrLen nLen,
        bool bUnicode, rtl_TextEncoding eChrSet);

    ULONG ReplaceCr( BYTE nChar );

    ULONG Fc2Cp( ULONG nFc ) const          { return pPiece->Fc2Cp( nFc ); }

            // einige z.T. static halb-interne Funktions-Deklarationen

    void OutSprmBytes( BYTE* pBytes, USHORT nSiz )
                                { pO->Insert( pBytes, nSiz, pO->Count() ); }

    inline bool IsUnicode() const           { return pPiece->IsUnicode(); }

    const SfxItemSet* GetCurItemSet() const         { return pISet; }
    void SetCurItemSet( const SfxItemSet* pS )      { pISet = pS; }

    void ExportPoolItemsToCHP(sw::PoolItems &rItems, USHORT nScript);
    void Out_SfxItemSet(const SfxItemSet& rSet, bool bPapFmt, bool bChpFmt,
        USHORT nScript);
    void Out_SfxBreakItems(const SfxItemSet *pSet, const SwNode& rNd);
    bool SetAktPageDescFromNode(const SwNode &rNd);

    void Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow);
    void Out_SwFmtTableBox( WW8Bytes& rO, const SvxBoxItem& rBox );
    BYTE TransCol( const Color& rCol );
    bool TransBrush(const Color& rCol, WW8_SHD& rShd);
    WW8_BRC TranslateBorderLine(const SvxBorderLine& pLine,
        USHORT nDist, bool bShadow);

    void ExportOutlineNumbering(BYTE nLvl, const SwNumFmt &rNFmt,
        const SwFmt &rFmt);
    void DisallowInheritingOutlineNumbering(const SwFmt &rFmt);

    unsigned int GetHdFtIndex() const { return mnHdFtIndex; }
    void SetHdFtIndex(unsigned int nHdFtIndex) { mnHdFtIndex = nHdFtIndex; }

    static long GetDTTM( const DateTime& rDT );

    static BYTE GetNumId( USHORT eNumType );
    static void CorrTabStopInSet( SfxItemSet& rSet, USHORT nAbsLeft );

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

#if 1
    //Prefer ww::bytes to WW8Bytes, migrate away from the other ones.
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
    bool CollapseScriptsforWordOk(USHORT nScript, USHORT nWhich);
    USHORT DupNumRuleWithLvlStart(const SwNumRule *pRule,BYTE nLvl,USHORT nVal);

    SwTwips CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const;
    bool MiserableRTLFrmFmtHack(long &rLeft, long &rRight,
        const sw::Frame &rFrmFmt);
    void InsUInt16( UINT16 n )      { SwWW8Writer::InsUInt16( *pO, n ); }
    void InsUInt32( UINT32 n )      { SwWW8Writer::InsUInt32( *pO, n ); }
    void InsAsString16( const String& rStr )
                        { SwWW8Writer::InsAsString16( *pO, rStr ); }
    void InsAsString8( const String& rStr, rtl_TextEncoding eCodeSet )
                        { SwWW8Writer::InsAsString8( *pO, rStr, eCodeSet ); }
    void WriteStringAsPara( const String& rTxt, USHORT nStyleId = 0 );

            // nach aussen interessant

    SwWW8Writer( const String& rFltName );
    virtual ~SwWW8Writer();

    // fuer WW8SaveData
    SwPaM* GetEndPaM()              { return pOrigPam; }
    void SetEndPaM( SwPaM* pPam )   { pOrigPam = pPam; }

    void DoComboBox(const rtl::OUString &rName, const rtl::OUString &rSelected,
            com::sun::star::uno::Sequence<rtl::OUString> &rListItems);

    static bool NoPageBreakSection(const SfxItemSet *pSet);
    void push_charpropstart(xub_StrLen nPos);
    void pop_charpropstart();
    xub_StrLen top_charpropstart() const;
    bool empty_charpropstart() const;
    void GetCurrentItems(WW8Bytes &rItems) const;
    void SetHdFtPageRoot(const SwTxtNode *pNd) { mpTopNodeOfHdFtPage = pNd; }
    const SwTxtNode *GetHdFtPageRoot() const { return mpTopNodeOfHdFtPage; }

    void AddLinkTarget(const String& rURL);
    void CollectOutlineBookmarks(const SwDoc &rDoc);
    void AddBookmark(String sBkmkName);
private:
    //No copying
    SwWW8Writer(const SwWW8Writer&);
    SwWW8Writer& operator=(const SwWW8Writer&);
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

    bool WriteGenericTxt(SwWW8Writer& rWrt, BYTE nTTyp, long& rCount);
    void WriteGenericPlc( SwWW8Writer& rWrt, BYTE nTTyp, long& rTxtStt,
        long& rTxtCnt, long& rRefStt, long& rRefCnt ) const;

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

    bool WriteTxt(SwWW8Writer& rWrt);
    void WritePlc( SwWW8Writer& rWrt ) const;

    void Append( WW8_CP nCp, const SwFmtFtn& rFtn );
};

class WW8_WrPlcPostIt : public WW8_WrPlcSubDoc  // Doppel-Plc fuer PostIts
{
private:
    //No copying
    WW8_WrPlcPostIt(const WW8_WrPlcPostIt&);
    WW8_WrPlcPostIt& operator=(WW8_WrPlcPostIt&);
public:
    WW8_WrPlcPostIt() {}

    void Append( WW8_CP nCp, const SwPostItField& rPostIt );
    bool WriteTxt(SwWW8Writer& rWrt);
    void WritePlc( SwWW8Writer& rWrt ) const;
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

    bool WriteTxt(SwWW8Writer& rWrt);
    void WritePlc( SwWW8Writer& rWrt ) const;
    void Append( const SdrObject& rObj, UINT32 nShapeId );
    USHORT Count() const { return aCntnt.Count(); }
    USHORT GetPos( const VoidPtr& p ) const { return aCntnt.GetPos( p ); }
};

typedef WW8_WrFkp* WW8_FkpPtr;  // Plc fuer Chpx und Papx ( incl PN-Plc )
SV_DECL_PTRARR( WW8_WrFkpPtrs, WW8_FkpPtr, 4, 4 )

class WW8_WrPlcPn                   // Plc fuer Page Numbers
{
private:
    SwWW8Writer& rWrt;
    WW8_WrFkpPtrs aFkps;            // PTRARR
    USHORT nFkpStartPage;
    ePLCFT ePlc;
    bool bWrtWW8;                   // Fuer Writererkennung
    USHORT nMark;

    //No copying
    WW8_WrPlcPn(const WW8_WrPlcPn&);
    WW8_WrPlcPn& operator=(const WW8_WrPlcPn&);
public:
    WW8_WrPlcPn( SwWW8Writer& rWrt, ePLCFT ePl, WW8_FC nStartFc );
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

    //No copying
    WW8_WrPlcFld(const WW8_WrPlcFld&);
    WW8_WrPlcFld& operator=(const WW8_WrPlcFld&);
public:
    WW8_WrPlcFld( USHORT nStructSz, BYTE nTTyp )
        : WW8_WrPlc1( nStructSz ), nTxtTyp( nTTyp )
    {}
    bool Write( SwWW8Writer& rWrt );
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
    bool Write(SwWW8Writer& rWrt);
};

class GraphicDetails
{
public:
    const sw::Frame maFly;      // Umgebende FlyFrms dazu
    ULONG mnPos;                // FilePos der Grafiken
    UINT16 mnWid;               // Breite der Grafiken
    UINT16 mnHei;               // Hoehe der Grafiken
    GraphicDetails(const sw::Frame &rFly, UINT16 nWid, UINT16 nHei)
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

// class SwWW8WrGrf sammelt Grafiken und gibt sie aus
class SwWW8WrGrf
{
private:
    SwWW8Writer& rWrt;  // SwWW8Writer fuer Zugriff auf die Vars
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
    SwWW8WrGrf(SwWW8Writer& rW) : rWrt(rW), mnIdx(0) {}
    void Insert(const sw::Frame &rFly);
    void Write();
    ULONG GetFPos()
        { return (mnIdx < maDetails.size()) ? maDetails[mnIdx++].mnPos : 0; }
};

// The class WW8_AttrIter is a helper class to build the Fkp.chpx. This
// class may be overloaded for output the SwTxtAttrs and the
// EditEngineTxtAttrs.
class WW8_AttrIter
{
private:
    WW8_AttrIter* pOld;
    //No copying
    WW8_AttrIter(const WW8_AttrIter&);
    WW8_AttrIter& operator=(const WW8_AttrIter&);
protected:
    SwWW8Writer& rWrt;
public:
    WW8_AttrIter( SwWW8Writer& rWrt );
    virtual ~WW8_AttrIter();

    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const = 0;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const = 0;
    void StartURL(const String &rUrl, const String &rTarget);
    void EndURL();
};

class WW8WrtStyle
{
    SwWW8Writer& rWrt;
    SwFmt** pFmtA;
    USHORT nPOPosStdLen1, nPOPosStdLen2;
    USHORT nUsedSlots;

    void BuildStyleTab();
    void BuildUpx(const SwFmt* pFmt, bool bPap, USHORT nPos,
        bool bInsDefCharSiz);
    USHORT Build_GetWWSlot( const SwFmt& rFmt );
    USHORT GetWWId( const SwFmt& rFmt ) const;
    void Set1StyleDefaults(const SwFmt& rFmt, bool bPap);
    void Out1Style( SwFmt* pFmt, USHORT nPos );

    void WriteStyle( SvStream& rStrm );
    void SkipOdd();
    void BuildStd(const String& rName, bool bPapFmt, short nWwBase,
        short nWwNext, USHORT nWwId);
    //No copying
    WW8WrtStyle(const WW8WrtStyle&);
    WW8WrtStyle& operator=(const WW8WrtStyle&);
public:
    WW8WrtStyle( SwWW8Writer& rWr );
    ~WW8WrtStyle();

    void OutStyleTab();
    USHORT Sty_GetWWSlot( const SwFmt& rFmt ) const;
};

class WW8SaveData
{
private:
    //No copying
    WW8SaveData(const WW8SaveData&);
    WW8SaveData& operator=(const WW8SaveData &);
public:
    SwWW8Writer& rWrt;
    Point* pOldFlyOffset;
    RndStdIds eOldAnchorType;
    WW8Bytes* pOOld;
    SwPaM* pOldPam, *pOldEnd;
    const sw::Frame* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;

    BYTE bOldWriteAll : 1;
    BYTE bOldOutTable : 1;
    BYTE bOldIsInTable: 1;
    BYTE bOldFlyFrmAttrs : 1;
    BYTE bOldStartTOX : 1;
    BYTE bOldInWriteTOX : 1;
    // bOutPageDesc muss nicht gesichert werden, da es nur nicht waehrend der
    // Ausgabe von Spezial-Texten veraendert wird.

    WW8SaveData( SwWW8Writer&, ULONG nStt, ULONG nEnd );
    ~WW8SaveData();
};

// einige halb-interne Funktions-Deklarationen fuer die Node-Tabelle

Writer& OutWW8_SwGrfNode( Writer& rWrt, SwCntntNode& rNode );
Writer& OutWW8_SwOleNode( Writer& rWrt, SwCntntNode& rNode );
Writer& OutWW8_SwSectionNode(Writer& rWrt, SwSectionNode& rSectionNode );
Writer& OutWW8_SwTblNode( Writer& rWrt, SwTableNode & rNode );

Writer& OutWW8_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& rHt );
Writer& OutWW8_SwFmtVertOrient( Writer& rWrt, const SfxPoolItem& rHt );

sal_Int16 GetWordFirstLineOffset(const SwNumFmt &rFmt);
//A bit of a bag on the side for now
String FieldString(ww::eField eIndex);
String BookmarkToWord(const String &rBookmark);
#endif  //  _WRTWW8_HXX

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
