/*************************************************************************
 *
 *  $RCSfile: wrtww8.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cmc $ $Date: 2000-10-10 16:54:06 $
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
#ifndef _WRTWW8_HXX
#define _WRTWW8_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>        // UINTXX
#endif
#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef WRT_FN_HXX
#include <wrt_fn.hxx>
#endif
#ifndef WW8STRUC_HXX
#include <ww8struc.hxx>
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
#endif


// einige Forward Deklarationen
class BitmapPalette;
class Brush;                //JP 04.06.99: die Brush obsolete!!
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
class WW8_WrPlcDrawObj;
class WW8_WrPlcFld;
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
struct WW8_PdAttrDesc;
struct WW8_SHD;


#define WWFL_ULSPACE_LIKE_SWG   0x00000001
#define WWFL_NO_GRAF            0x00000080
#define WWFL_NO_OLE             0x00020000
#define WWFL_KA_DEBUG           0x08000000
#define WWFL_READ_FROM_FILE     0x40000000


#define WW8_CSTY_FTN 15     // Slot# fuer C-Format Fussnotenzeichen

#define GRF_MAGIC_1 0x12    // 3 magic Bytes fuer PicLocFc-Attribute
#define GRF_MAGIC_2 0x34
#define GRF_MAGIC_3 0x56
#define GRF_MAGIC_321 0x563412L

// defines for InsertField- Method
#define WRITEFIELD_ALL              0xFF
#define WRITEFIELD_START            0x01
#define WRITEFIELD_CMD_START        0x02
#define WRITEFIELD_CMD_END          0x04
#define WRITEFIELD_END              0x10
#define WRITEFIELD_CLOSE            0x20

#define TXT_MAINTEXT    0           // Defines fuer TextTyp
//!!#define TXT_FTNEDN 1
#define TXT_HDFT        2
#define TXT_FTN         3
#define TXT_EDN         4
#define TXT_ATN         5
#define TXT_TXTBOX      6
#define TXT_HFTXTBOX    7


extern SwNodeFnTab aWW8NodeFnTab;
extern SwAttrFnTab aWW8AttrFnTab;

SV_DECL_VARARR( WW8Bytes, BYTE, 128, 128 );

struct WW8_SepInfo
{
    const SwPageDesc* pPageDesc;
    const SwSectionFmt* pSectionFmt;
    const SwNode* pPDNd;
    const SwTxtNode* pNumNd;
    ULONG  nLnNumRestartNo;
    USHORT nPgRestartNo;

    WW8_SepInfo()
        : pPageDesc(0), pSectionFmt(0), pNumNd(0), pPDNd(0), nPgRestartNo(0),
          nLnNumRestartNo(0)
    {}

    WW8_SepInfo( const SwPageDesc* pPD, const SwSectionFmt* pFmt, ULONG nLnRestart )
        : pPageDesc(pPD), pSectionFmt(pFmt), pNumNd(0), nPgRestartNo(0),
          nLnNumRestartNo( nLnRestart ), pPDNd( 0 )
    {}
};
SV_DECL_VARARR( WW8_WrSepInfoPtrs, WW8_SepInfo, 4, 4 );


class WW8_WrPlcSepx     // Plc fuer PageDescs -> Sepx ( Section Extensions )
{
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

public:
    WW8_WrPlcSepx();
    ~WW8_WrPlcSepx();
    void AppendSep( WW8_CP nStartCp,
                    const SwPageDesc* pPd,
                    const SwSectionFmt* pSectionFmt = 0,
                    ULONG nLnNumRestartNo = 0 );
    void AppendSep( WW8_CP nStartCp, const SwFmtPageDesc& rPd,
                    const SwNode& rNd,
                    ULONG nLnNumRestartNo );
    void Finish( WW8_CP nEndCp ) { aCps.Insert( nEndCp, aCps.Count() ); }
    void SetNum( const SwTxtNode* pNumNd );
    void WriteKFTxt( SwWW8Writer& rWrt );
    void WriteSepx( SvStream& rStrm ) const;
    void WritePlcSed( SwWW8Writer& rWrt ) const;
    void WritePlcHdd( SwWW8Writer& rWrt ) const;
};

//--------------------------------------------------------------------------
// class WW8_WrPct zum Aufbau der Piece-Table
//--------------------------------------------------------------------------
class WW8_WrPct
{
    WW8_WrPcPtrs* pPcts;
    WW8_FC nOldFc;
    BOOL bIsUni : 1;
public:
    WW8_WrPct( WW8_FC nStartFc );
    ~WW8_WrPct();
    void AppendPc( WW8_FC nStartFc, BOOL bIsUnicode );
    void WritePc( SwWW8Writer& rWrt );
    void SetParaBreak();
    BOOL IsUnicode() const  { return bIsUni; }
    ULONG Fc2Cp( ULONG nFc ) const;
};



// der WW8-Writer
class SwWW8Writer: public StgWriter
{
friend void WW8_WrPlcSepx::WriteKFTxt( SwWW8Writer& rWrt ); // pO
friend void WW8_WrPlcSepx::WriteOlst( SwWW8Writer& rWrt, USHORT i );
friend Writer& OutWW8_SwTxtNode( Writer& rWrt, SwCntntNode& rNode );

    String aMainStg;
    SvPtrarr aTOXArr;
    SwPosFlyFrms* pFlyPos;          // Pointer auf die aktuelle "FlyFrmTabelle"
    const SfxItemSet* pISet;        // fuer Doppel-Attribute
    WW8_WrPct*  pPiece;             // Pointer auf Piece-Table
    SwNumRuleTbl* pUsedNumTbl;      // alle used NumRules
    WW8_WrtBookmarks* pBkmks;
    WW8_WrtRedlineAuthor* pRedlAuthors;
    BitmapPalette* pBmpPal;
    void* pKeyMap;
    SvxMSExportOLEObjects* pOLEExp;
    SwMSConvertControls* pOCXExp;

    ULONG nIniFlags;                // Flags aus der writer.ini
    USHORT nCharFmtStart;
    USHORT nFmtCollStart;
    USHORT nAktFlyPos;              // Index auf das naechste "FlyFrmFmt"
    USHORT nStyleBeforeFly;         // Style-Nummer des Nodes,
                                    //       in/an dem ein Fly verankert ist
    USHORT nLastFmtId;              // Style of last TxtNode in normal range

    virtual ULONG WriteStorage();

    void PrepareStorage();
    void WriteFkpPlcUsw();
    void OutFontTab( WW8Fib& pFib );
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
    static void BuildAnlvBase( WW8_ANLV& rAnlv, BYTE*& rpCh, USHORT& rCharLen,
                   const SwNumRule& rRul, const SwNumFmt& rFmt, BYTE nSwLevel );

    void Out_BorderLine( WW8Bytes& rO, const SvxBorderLine* pLine,
                            USHORT nDist, USHORT nSprmNo, BOOL bShadow );

    void OutListTab();
    void OutOverrideListTab();
    void OutListNamesTab();


public:
    const SwPageDesc* pAktPageDesc;
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

    SwFlyFrmFmt* pFlyFmt;           // liegt der Node in einem FlyFrame, ist
                                    // das Format gesetzt, sonst 0

    Point* pFlyOffset;              // zur Justierung eines im Writer als Zeichen
                                    // gebundenen Flys, der im WW Absatzgebunden wird.

    WW8_WrPlcFld* pFldMain;         // Felder im Haupttext
    WW8_WrPlcFld* pFldHdFt;         // Felder in Header/Footer
    WW8_WrPlcFld* pFldFtn;          // Felder in FootNotes
    WW8_WrPlcFld* pFldEdn;          // Felder in EndNotes
    WW8_WrPlcFld* pFldTxtBxs;       // fields in textboxes
    WW8_WrPlcFld* pFldHFTxtBxs;     // fields in header/footer textboxes
    SwWW8WrGrf* pGrf;
    const SwAttrSet* pStyAttr;      // StyleAttr fuer Tabulatoren
    const SwModify* pOutFmtNode;    // write Format or Node

    WW8_WrPlcDrawObj *pSdrObjs,     // Draw-/Fly-Objects
                     *pHFSdrObjs;   // Draw-/Fly-Objects in header or footer

    WW8Bytes* pO;

    SvStream *pTableStrm, *pDataStrm;   // Streams fuer WW97 Export

    SwEscherEx* pEscher;            // escher export class
    SwTwips nFlyWidth, nFlyHeight;  // Fuer Anpassung Graphic

    BYTE nTxtTyp;

    BOOL bStyDef : 1;           // wird Style geschrieben ?
    BOOL bBreakBefore : 1;      // Breaks werden 2mal ausgegeben
    BOOL bOutKF : 1;            // Kopf/Fusstexte werden ausgegeben
    BOOL bOutFlyFrmAttrs : 1;   // Rahmen-Attr von Flys werden ausgegeben
    BOOL bOutPageDescs : 1;     // PageDescs werden ausgegeben ( am Doc-Ende )
    BOOL bOutFirstPage : 1;     // write Attrset of FirstPageDesc
    BOOL bOutTable : 1;         // Tabelle wird ausgegeben
                                //    ( wird zB bei Flys in Tabelle zurueckgesetzt )
    BOOL bIsInTable : 1;        // wird sind innerhalb der Ausgabe einer Tabelle
                                //    ( wird erst nach der Tabelle zurueckgesetzt )
    BOOL bOutGrf : 1;           // Grafik wird ausgegeben
    BOOL bWrtWW8 : 1;           // Schreibe WW95 oder WW97 FileFormat
    BOOL bInWriteEscher : 1;    // in write textboxes
    BOOL bStartTOX : 1;         // TRUE: a TOX is startet
    BOOL bInWriteTOX : 1;       // TRUE: all content are in a TOX
    BOOL bFtnAtTxtEnd : 1;      // TRUE: all FTN at Textend
    BOOL bEndAtTxtEnd : 1;      // TRUE: all END at Textend


    SvxMSExportOLEObjects& GetOLEExp()      { return *pOLEExp; }
    SwMSConvertControls& GetOCXExp()        { return *pOCXExp; }

    static USHORT TranslateToEditEngineId( USHORT nWhich );
    static USHORT TranslateFromEditEngineId( USHORT nWhich );
    const SfxPoolItem* HasItem( USHORT nWhich ) const;
    const SfxPoolItem& GetItem( USHORT nWhich ) const;

    USHORT GetId( const SwTxtFmtColl& rColl ) const;
    USHORT GetId( const SwCharFmt& rFmt ) const;
    USHORT GetId( const SvxFontItem& rFont ) const;
    USHORT GetId( const Font& rFont ) const;
    USHORT GetId( const SwNumRule& rNumRule ) const;
    USHORT GetId( const SwTOXType& rTOXType );

    void OutRedline( const SwRedlineData& rRedline );
    USHORT AddRedlineAuthor( USHORT nId );

    void StartTOX( const SwSection& rSect );
    void EndTOX( const SwSection& rSect );

    void WriteSpecialText( ULONG nStart, ULONG nEnd, BYTE nTTyp );
    void WriteKFTxt1( const SwFmtCntnt& rCntnt );
    void WriteFtnBegin( const SwFmtFtn& rFtn, WW8Bytes* pO = 0 );
    void WritePostItBegin( const SwPostItField& rFld, WW8Bytes* pO = 0 );
    void OutWW8FlyFrmsInCntnt( const SwTxtNode& rNd );
    void OutWW8FlyFrm( const SwFrmFmt& rFlyFrmFmt, const Point& rNdTopLeft );
    void OutFlyFrms( const SwCntntNode& rNode );
    void AppendFlyInFlys( WW8_CP& rCP, const SwFrmFmt& rFrmFmt,
                            const Point& rNdTopLeft );
    void WriteSdrTextObj( const SdrObject& rObj );

    UINT32 GetSdrOrdNum( const SwFrmFmt& rFmt ) const;
    void CreateEscher();
    void WriteEscher();

    BOOL Out_SwNum( const SwTxtNode* pNd );
    void Out_SwFmt( const SwFmt& rFmt, BOOL bPapFmt, BOOL bChpFmt,
                    BOOL bFlyFmt = FALSE );
    BOOL GetNumberFmt( const SwField& rFld, String& rStr );
    void OutField( const SwField* pFld, BYTE nFldType,
                    const String& rFldCmd, BYTE nMode = WRITEFIELD_ALL );
    void StartCommentOutput( const String& rName );
    void EndCommentOutput(   const String& rName );
    void OutGrf( const SwNoTxtNode* pNd );
    void AppendBookmarks( const SwTxtNode& rNd, xub_StrLen nAktPos, xub_StrLen nLen );
    void AppendBookmark( const String& rName, USHORT nOffset = 0 );
    String GetBookmarkName( USHORT nTyp, const String* pNm, USHORT nSeqNo );
    BOOL HasRefToObject( USHORT nTyp, const String* pNm, USHORT nSeqNo );

    void WriteAsStringTable( const SvStrings&, INT32& rfcSttbf,
                                INT32& rlcbSttbf, USHORT nExtraLen = 0 );
    void WriteText();
    void WriteCR();
    void WriteChar( sal_Unicode c );
    void OutSwString( const String&, xub_StrLen nStt, xub_StrLen nLen,
                        BOOL bUnicode, rtl_TextEncoding eChrSet );

    ULONG ReplaceCr( BYTE nChar );

    ULONG Fc2Cp( ULONG nFc ) const          { return pPiece->Fc2Cp( nFc ); }

            // einige z.T. static halb-interne Funktions-Deklarationen

    void OutSprmBytes( BYTE* pBytes, USHORT nSiz )
                                { pO->Insert( pBytes, nSiz, pO->Count() ); }

    ULONG GetIniFlags() const               { return nIniFlags; }
    inline BOOL IsUnicode() const           { return pPiece->IsUnicode(); }

    const SfxItemSet* GetCurItemSet() const         { return pISet; }
    void SetCurItemSet( const SfxItemSet* pS )      { pISet = pS; }

    void Out_SfxItemSet( const SfxItemSet& rSet, BOOL bPapFmt, BOOL bChpFmt );
    void Out_SfxBreakItems( const SfxItemSet& rSet, const SwNode& rNd );

    void Out_SwFmtBox( const SvxBoxItem& rBox, BOOL bShadow );
    void Out_SwFmtTableBox( WW8Bytes& rO, const SvxBoxItem& rBox );
    BYTE TransCol( const Color& rCol );
    BOOL TransBrush( const Color& rCol, WW8_SHD& rShd );
    UINT32 TranslateBorderLine( const SvxBorderLine& pLine,
                                       USHORT nDist, BOOL bShadow );

    static long GetDTTM( const DateTime& rDT );

    static BYTE GetNumId( USHORT eNumType );
    static void CorrTabStopInSet( SfxItemSet& rSet, USHORT nAbsLeft );

    static ULONG FillUntil( SvStream& rStrm, ULONG nEndPos = 0 );
    static void FillCount( SvStream& rStrm, ULONG nCount );

    static void WriteShort( SvStream& rStrm, INT16 nVal ) { rStrm << nVal; }
    static void WriteShort( SvStream& rStrm, ULONG nPos, INT16 nVal );

    static void WriteLong( SvStream& rStrm, INT32 nVal ) { rStrm << nVal; }
    static void WriteLong( SvStream& rStrm, ULONG nPos, INT32 nVal );

    static void WriteString16( SvStream& rStrm, const String& rStr,
                                BOOL bAddZero );
    static void WriteString8( SvStream& rStrm, const String& rStr,
                                BOOL bAddZero, rtl_TextEncoding eCodeSet );

    static void InsUInt16( WW8Bytes& rO, UINT16 );
    static void InsUInt32( WW8Bytes& rO, UINT32 );
    static void InsAsString16( WW8Bytes& rO, const String& );
    static void InsAsString8( WW8Bytes& rO, const String& rStr,
                                rtl_TextEncoding eCodeSet );

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
};


class WW8_WrPlcSubDoc   // Doppel-Plc fuer Foot-/Endnotes und Postits
{
protected:
    SvULongs aCps;                  // PTRARR CP-Pos der Verweise
    SvPtrarr aCntnt;                // PTRARR von SwFmtFtn/PostIts/..
    WW8_WrPlc0* pTxtPos;            // Pos der einzelnen Texte

    WW8_WrPlcSubDoc();
    ~WW8_WrPlcSubDoc();

    void WriteTxt( SwWW8Writer& rWrt, BYTE nTTyp, long& rCount );
    void WritePlc( SwWW8Writer& rWrt, BYTE nTTyp,
                    long& rTxtStt, long& rTxtCnt,
                    long& rRefStt, long& rRefCnt ) const;

    virtual const SvULongs* GetShapeIdArr() const;
};

// Doppel-Plc fuer Footnotes/Endnotes
class WW8_WrPlcFtnEdn : public WW8_WrPlcSubDoc
{
    BYTE nTyp;
public:
    WW8_WrPlcFtnEdn( BYTE nTTyp ) : nTyp( nTTyp ) {}

    inline void WriteTxt( SwWW8Writer& rWrt );
    inline void WritePlc( SwWW8Writer& rWrt ) const;

    void Append( WW8_CP nCp, const SwFmtFtn& rFtn );
};

class WW8_WrPlcPostIt : public WW8_WrPlcSubDoc  // Doppel-Plc fuer PostIts
{
public:
    WW8_WrPlcPostIt() {}

    void Append( WW8_CP nCp, const SwPostItField& rPostIt );
    inline void WriteTxt( SwWW8Writer& rWrt );
    inline void WritePlc( SwWW8Writer& rWrt ) const;
};


class WW8_WrPlcTxtBoxes : public WW8_WrPlcSubDoc    // Doppel-Plc fuer Textboxen
{                                                   // Rahmen/DrawTextboxes!
    BYTE nTyp;
    SvULongs aShapeIds;             // VARARR of ShapeIds for the SwFrmFmts

    virtual const SvULongs* GetShapeIdArr() const;

public:
    WW8_WrPlcTxtBoxes( BYTE nTTyp ) : nTyp( nTTyp ) {}

    void WriteTxt( SwWW8Writer& rWrt );
    inline void WritePlc( SwWW8Writer& rWrt ) const;
    void Append( const SdrObject& rObj, UINT32 nShapeId );
    USHORT Count() const { return aCntnt.Count(); }
    USHORT GetPos( const VoidPtr& p ) const { return aCntnt.GetPos( p ); }
};


class WW8_WrPlcDrawObj      // PC for DrawObjects and Text-/OLE-/GRF-Boxes
{
    SvULongs aCps;                  // VARARR CP-Pos der Verweise
    SvULongs aShapeIds;             // VARARR of ShapeIds for the SwFrmFmts
    SvPtrarr aCntnt;                // PTRARR of SwFrmFmt
    SvPtrarr aParentPos;            // PTRARR of Points
    BYTE nTTyp;

public:
    WW8_WrPlcDrawObj( BYTE nType );
    ~WW8_WrPlcDrawObj();

    void WritePlc( SwWW8Writer& rWrt ) const;
    BOOL Append( SwWW8Writer&, WW8_CP nCp, const SwFrmFmt& rFmt,
                const Point& rNdTopLeft );
    const SvPtrarr& GetCntntArr() const { return aCntnt; }
    void SetShapeId( const SwFrmFmt& rFmt, UINT32 nId );
    UINT32 GetShapeId( USHORT n ) const { return aShapeIds[ n ]; }
};


typedef WW8_WrFkp* WW8_FkpPtr;  // Plc fuer Chpx und Papx ( incl PN-Plc )
SV_DECL_PTRARR( WW8_WrFkpPtrs, WW8_FkpPtr, 4, 4 );

class WW8_WrPlcPn                   // Plc fuer Page Numbers
{
    SwWW8Writer& rWrt;
    WW8_WrFkpPtrs aFkps;            // PTRARR
    USHORT nFkpStartPage;
    ePLCFT ePlc;
    BOOL bWrtWW8;                   // Fuer Writererkennung

public:
    WW8_WrPlcPn( SwWW8Writer& rWrt, ePLCFT ePl, WW8_FC nStartFc );
    ~WW8_WrPlcPn();
    void AppendFkpEntry( WW8_FC nEndFc, short nVarLen = 0, const BYTE* pSprms = 0 );
    void WriteFkps();
    void WritePlc();
};

// class WW8_WrPlc1 ist erstmal nur fuer Felder
class WW8_WrPlc1
{
    SvULongs aPos;              // PTRARR von CPs
    WW8Bytes aDat;              // Inhalte ( Strukturen )
    USHORT nStructSiz;
protected:
    USHORT Count() const { return aPos.Count(); }
    void Write( SvStream& rStrm );

public:
    WW8_WrPlc1( USHORT nStructSz );
    void Append( WW8_CP nCp, const void* pData );
    void Finish( ULONG nLastCp, ULONG nStartCp );
};

// class WW8_WrPlcFld ist fuer Felder
class WW8_WrPlcFld: public WW8_WrPlc1
{
    BYTE nTxtTyp;
public:
    WW8_WrPlcFld( USHORT nStructSz, BYTE nTTyp )
        : WW8_WrPlc1( nStructSz ), nTxtTyp( nTTyp )
    {}
    BOOL Write( SwWW8Writer& rWrt );
};


// class SwWW8WrGrf sammelt Grafiken und gibt sie aus
class SwWW8WrGrf
{
    SwWW8Writer& rWrt;  // SwWW8Writer fuer Zugriff auf die Vars
    SvPtrarr aNds;      // Positionen der SwGrfNodes und SwOleNodes
    SvPtrarr aFlys;     // Umgebende FlyFrms dazu
    SvULongs aPos;      // FilePos der Grafiken
    SvUShorts aWid;     // Breite der Grafiken
    SvUShorts aHei;     // Hoehe der Grafiken
    USHORT nIdx;        // Index in File-Positionen

    void Write1GrfHdr( SvStream& rStrm, const SwNoTxtNode* pNd,
            const SwFlyFrmFmt* pFly, UINT16 mm, UINT16 nWidth, UINT16 nHeight );
    void Write1Grf1( SvStream& rStrm, const SwGrfNode* pGrfNd,
                    const SwFlyFrmFmt* pFly, UINT16 nWidth, UINT16 nHeight );
    void Write1Grf( SvStream& rStrm, const SwNoTxtNode* pNd,
                    const SwFlyFrmFmt* pFly, UINT16 nWidth, UINT16 nHeight );
public:
    SwWW8WrGrf( SwWW8Writer& rW )
        : rWrt( rW ), aNds( 4, 4 ), aFlys( 4, 4 ), aPos( 4, 4 ),
          aWid( 4, 4 ), aHei( 4, 4 )
    {}
    void Insert( const SwNoTxtNode* pNd, const SwFlyFrmFmt* pFly );
    void Write();
    ULONG GetFPos() { return ( nIdx < aNds.Count() ) ? aPos[nIdx++] : 0; }
};

// The class WW8_AttrIter is a helper class to build the Fkp.chpx. This
// class may be overloaded for output the SwTxtAttrs and the
// EditEngineTxtAttrs.
class WW8_AttrIter
{
    WW8_AttrIter* pOld;
protected:
    SwWW8Writer& rWrt;
public:
    WW8_AttrIter( SwWW8Writer& rWrt );
    ~WW8_AttrIter();

    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const = 0;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const = 0;
    virtual void GetItems( WW8Bytes& rItems ) const;
};


class WW8WrtStyle
{
    SwWW8Writer& rWrt;
    SwFmt** pFmtA;
    USHORT nPOPosStdLen1, nPOPosStdLen2;
    USHORT nUsedSlots;

    void BuildStyleTab();
    void BuildUpx( const SwFmt* pFmt, BOOL bPap, USHORT nPos,
                   BOOL bInsDefCharSiz );
    USHORT Build_GetWWSlot( const SwFmt& rFmt );
    USHORT GetWWId( const SwFmt& rFmt ) const;
    void Set1StyleDefaults( const SwFmt& rFmt, BOOL bPap );
    void Out1Style( SwFmt* pFmt, USHORT nPos );

    void WriteStyle( SvStream& rStrm );
    void SkipOdd();
    void BuildStd( const String& rName, BOOL bPapFmt, short nWwBase,
                   short nWwNext, USHORT nWwId );

public:
    WW8WrtStyle( SwWW8Writer& rWr );
    ~WW8WrtStyle();

    void OutStyleTab();
    USHORT Sty_GetWWSlot( const SwFmt& rFmt ) const;
};


struct WW8SaveData
{
    SwWW8Writer& rWrt;
    Point* pOldFlyOffset;
    WW8Bytes* pOOld;
    SwPaM* pOldPam, *pOldEnd;
    SwFlyFrmFmt* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;

    BOOL bOldWriteAll : 1;
    BOOL bOldOutTable : 1;
    BOOL bOldIsInTable: 1;
    BOOL bOldFlyFrmAttrs : 1;
    BOOL bOldStartTOX : 1;
    BOOL bOldInWriteTOX : 1;
        // bOutPageDesc muss nicht gesichert werden, da es nur nicht waehrend
        // der Ausgabe von Spezial-Texten veraendert wird.

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


// --------------------------- inlines ---------------------------------

inline void WW8_WrPlcFtnEdn::WriteTxt( SwWW8Writer& rWrt )
{
    if( TXT_FTN == nTyp )
    {
        WW8_WrPlcSubDoc::WriteTxt( rWrt, TXT_FTN, rWrt.pFib->ccpFtn );
        rWrt.pFldFtn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->ccpText );
    }
    else
    {
        WW8_WrPlcSubDoc::WriteTxt( rWrt, TXT_EDN, rWrt.pFib->ccpEdn );
        rWrt.pFldEdn->Finish( rWrt.Fc2Cp( rWrt.Strm().Tell() ),
                            rWrt.pFib->ccpText + rWrt.pFib->ccpFtn
                            + rWrt.pFib->ccpHdr + rWrt.pFib->ccpAtn );
    }
}

inline void WW8_WrPlcFtnEdn::WritePlc( SwWW8Writer& rWrt ) const
{
    if( TXT_FTN == nTyp )
        WW8_WrPlcSubDoc::WritePlc( rWrt, TXT_FTN,
                    rWrt.pFib->fcPlcffndTxt, rWrt.pFib->lcbPlcffndTxt,
                    rWrt.pFib->fcPlcffndRef, rWrt.pFib->lcbPlcffndRef );
    else
        WW8_WrPlcSubDoc::WritePlc( rWrt, TXT_EDN,
                    rWrt.pFib->fcPlcfendTxt, rWrt.pFib->lcbPlcfendTxt,
                    rWrt.pFib->fcPlcfendRef, rWrt.pFib->lcbPlcfendRef );
}


void WW8_WrPlcPostIt::WriteTxt( SwWW8Writer& rWrt )
{
    WW8_WrPlcSubDoc::WriteTxt( rWrt, TXT_ATN, rWrt.pFib->ccpAtn );
}

void WW8_WrPlcPostIt::WritePlc( SwWW8Writer& rWrt ) const
{
    WW8_WrPlcSubDoc::WritePlc( rWrt, TXT_ATN,
                    rWrt.pFib->fcPlcfandTxt, rWrt.pFib->lcbPlcfandTxt,
                    rWrt.pFib->fcPlcfandRef, rWrt.pFib->lcbPlcfandRef );
}

void WW8_WrPlcTxtBoxes::WritePlc( SwWW8Writer& rWrt ) const
{
    if( TXT_TXTBOX == nTyp )
        WW8_WrPlcSubDoc::WritePlc( rWrt, nTyp,
                rWrt.pFib->fcPlcftxbxBkd, rWrt.pFib->lcbPlcftxbxBkd,
                rWrt.pFib->fcPlcftxbxTxt, rWrt.pFib->lcbPlcftxbxTxt );
    else
        WW8_WrPlcSubDoc::WritePlc( rWrt, nTyp,
                rWrt.pFib->fcPlcfHdrtxbxBkd, rWrt.pFib->lcbPlcfHdrtxbxBkd,
                rWrt.pFib->fcPlcfHdrtxbxTxt, rWrt.pFib->lcbPlcfHdrtxbxTxt );
}



#endif  //  _WRTWW8_HXX

