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
#ifndef _RDSWG_HXX
#define _RDSWG_HXX

#ifndef _SV_PRNTYPES_HXX //autogen
#include <vcl/prntypes.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif
#ifndef _SWGSTR_HXX
#include <swgstr.hxx>
#endif
#ifndef _SWGIDS_HXX
#include <swgids.hxx>
#endif
class Brush;
class Color;
class Pen;
namespace binfilter {
class SfxItemSet;

class SwRect;
class SwPaM;
class SwDoc;
class SwFrm;
class SwFmt;
class SwAttrSet;
class SwNumFmt;
class SwNumRule;
class SwPageDesc;
class SwPageFtnInfo;
class SwNode;
class SwCntntNode;
class SwTxtNode;
class SwTOXBase;
class SwNodeIndex;
class SwTable;
class SwTableBox;
class SwTableBoxes;
class SwTableLine;
class SwTableLines;
class SwField;
class SwFieldType;
class SwpHints;


class SwBlockNames;
class SwStartNode;

class SfxDocumentInfo;

struct FmtInfo {				// Format-Deskriptor:
    union {
        SfxItemSet* pSet;		// AttrSet oder
        SwFmt*      pFmt;		// Format
    };
    BYTE cFmt;					// Bitflags
};

#define	FINFO_FORMAT	0x01	// Eintrag ist Format
#define	FINFO_LOCAL		0x02	// Eintrag ist lokal

struct TxtCollInfo {			// Textcollection-Info:
    USHORT nDerived;			// Abhaengigkeit
    USHORT nFollow;				// Folgeindex
};

struct PageDescInfo {			// Seitenbeschreibungs-Info:
    USHORT nPoolId;				// Pool-ID
    USHORT nActualIdx;			// der tatsaechliche Index
    USHORT nFollow;				// Folgeindex
};

struct NumRuleInfo {			// Numerierungs-Regeln:
    SwNumRule* pNumRule;		// dazugehoerige Regel
    SwTxtNode* pBgn, *pEnd;		// Anfang + Ende im Nodes-Array
};

struct FrmInfo {				// Frame-ID-Info:
    SwFrm* pFrm;				// der Frame
    USHORT nId;					// und sein ID
};

struct LanguageInfo {			// Sprach-Info:
    USHORT nLanguage;			// Sprache
    BOOL   bSupported;			// TRUE, wenn unterstuetzt
};

class PageDescLink : public String{	// Verknuepfung Format mit Seite
public:
    PageDescLink* pLink;		// Link zum naechsten
    USHORT  nOffset;			// Offset
    union {
        SwAttrSet* pSet; 		// AttrSet oder
        SwFmt*     pFmt; 		// Format
    };
    BYTE cFmt;					// TRUE: Ptr ist Format
    PageDescLink( const String& s, USHORT n ) : String( s )
    { pLink = NULL; pSet = NULL; cFmt = FALSE; nOffset = n; }
};

struct SectionInfo {			// Section-Info:
    USHORT  nId;				// Section ID
    SwNode* pNode1;				// StartNode
};

typedef const SwTable *SwgSwTablePtr;
SV_DECL_PTRARR(SwgTables,SwgSwTablePtr,4,4)

 class SwSwgReader
 {
    friend class Sw2TextBlocks;

 private:
    ULONG         nErrno;       // Fehlernummer
    SwPaM*        pPaM;         // WriterG-Einfuegepos
    long          nDocStart;    // Beginn des Dokuments
    long          nRecStart;    // Beginn der Records im Dokument
    long          nNextDoc;     // Offset des naechsten Dokuments
    FrmInfo*      pFrms;        // Frame-ID-Infos
    FmtInfo*      pFmts;        // Format-Infos
    TxtCollInfo*  pCollIdx;     // Collection Index translation table
    PageDescInfo* pLayIdx;      // Layout Index translation table
    PageDescLink* pPageLinks;   // Format-Seiten-Links
    NumRuleInfo*  pRules;       // Num-Regelwerk
    SectionInfo*  pSects;       // Sections
    SectionInfo*  pLastSect;    // letzte Section
    LanguageInfo* pLanguages;   // Sprachen
    const ByteString* pNdOrigTxt;       // original text of the textnode

    USHORT  nColl, nLay;        // Anzahl Collections, Seitenlayouts
    USHORT  nRules;             // Anzahl Regeln
    USHORT  nNamedFmt;          // Anzahl benannte Formate (fuer Textbausteine)
    USHORT  nFrm, nFrmSize;     // Anzahl Frame-IDs, Tabellengroesse
    USHORT  nSect, nSectSize;   // Anzahl Sections, Tabellengroesse
    USHORT  nTbl, nTblSize;     // Anzahl Tabellen, Tabellengroesse
    USHORT  nLang, nLangSize;   // Anzahl Sprachen, Tabellengroesse
    USHORT  nOptions;           // Lese-Optionen
    USHORT  nHelpFileId;        // Hilfe-ID fuer Vorlagen-Templates
    USHORT  nFlyLevel;          // != 0: FlyFmt wird eingelesen
    USHORT  nFlagBits;          // temp fuer PageDesc: Bits aus Format
    USHORT  nFmtLvl;            // Rekursionstiefe fuer InFormat()
    USHORT  nTblBoxLvl;         // Rekursionstiefe fuer InFormat in InTableBox()

    SfxDocumentInfo* pInfo;
    String  aFileName;          // Name vom Eingabe-Stream
    SwgTables *pTables;

    // gc25: swistream besitzt privaten Copy-CTOR
    SwSwgReader( const SwSwgReader & );

    BOOL    LoadDocInfo( SfxDocumentInfo& );

    short   LayoutIdx( short n );
    void    UpdateRuleRange( USHORT nIdx, SwTxtNode* pNd );
    void    ResolvePageDescLinks();
    String  ParseText();
 // void    ConvertLineTerminators( String&, GUIType );
    void    LoadHeader();

    void    InTxtFmtColls();
    void    InTxtFmtColl( short nIdx );

    void    InPageDescs();
    void    InPageDesc( short nIdx );
    void    InPageFtnInfo( SwPageFtnInfo& rFtn );

    void    InTextHints( SwTxtNode& rNd, xub_StrLen nOffset );
    void    InNodeBookmark( SwNodeIndex& rPos );
    USHORT  GetNextSymbolFontHint( SwpHints*, USHORT, xub_StrLen&, xub_StrLen& );
    USHORT  GetNextCharsetColorHint( SwpHints*, USHORT, xub_StrLen&,
                                     xub_StrLen&, rtl_TextEncoding,
                                     rtl_TextEncoding&);
    void    ConvertText( SwTxtNode& rNd, rtl_TextEncoding eSrc );
    void    SetAttrSet( SwCntntNode& rNd, USHORT nId );
    void    InTxtNodeAttrs( SwTxtNode& rNd, USHORT nWhich1, USHORT nWhich2 );
    void    FillTxtNode( SwTxtNode* pNd, SwNodeIndex& rPos, xub_StrLen nOffset, USHORT=0);
    void    FillString( String& );
    void    InGrfNode( SwNodeIndex& rPos );
    void    InOleNode( SwNodeIndex& rPos );

    SwTOXBase* InTOXBase();
    void    InTOXContents( SwNodeIndex& rPos, SwTxtNode* );
    void    InTOX( SwNodeIndex& rPos, SwTxtNode* );

    void    InContents( BOOL bNode1 = TRUE, BOOL bBlock = FALSE, USHORT=0 );
    String  InContentsText( BOOL = TRUE );
    void    ScanContents( SwBlockNames* );

    void    InFlyFrames( const SwNodeIndex* pNdIdx );
    void    InFlyFrame( const SwNodeIndex* pNdIdx );

    void    InDfltFmts();
    void    InNamedFmts( USHORT );
    void    ClearFmtIds();

    void    InTableBox( SwTableBoxes&, int, SwTableLine*, SwNodeIndex&,
                        const SwTable *pTable=0 );
    void    InTableLine( SwTableLines&, SwTableBox*, int, SwNodeIndex&,
                         const SwTable *pTable=0 );
    void    InTable( SwNodeIndex& rPos );

    void    InNumFmt( SwNumFmt& rFmt );
    SwNumRule* InNumRule();
    void    InOutlineRule();
    void    InTxtNumRule();
    void    InFtnInfo();

    void    InGlobalMacroTbl();
    void    InJobSetup();
    void    InDocInfo();
    void    InStaticDocInfo( SfxDocumentInfo& );
    void    InDynamicDocInfo( SfxDocumentInfo& );
    void    InComment();

    void    InLayoutFrames();

    USHORT  InHint( SfxItemSet& );
    USHORT  InHint( SwTxtNode&, xub_StrLen, xub_StrLen );

    void    InFieldTypes();
    SwFieldType* InFieldType();

    BOOL    CopyRecord( SvStream* );

 public:
    SwDoc*    pDoc;             // WriterG-Dokumentzugriff
    BOOL      bNew;             // Darf es was neues sein?
    xub_StrLen  nCntntCol;      // Spalte fuer Rahmen im Node (RES_FLY_ANCHOR)
    USHORT    nPage1;           // Start-Seite bei Einfuegen (RES_FLY_ANCHOR)
    swistream r;                // Input-Stream
 // SvStream r;             // Input-Stream
    DocHeader aHdr;             // Dokument-Header
    FileHeader aFile;           // Datei-Header
    USHORT  nStatus;            // Status-Flags:

 #define    SWGSTAT_SHAREDFMT       0x0001      // Shared Fmt wird eingelesen
 #define    SWGSTAT_UPDATEEXPR      0x0002      // ExprFields-Update noetig
 #define    SWGSTAT_NO_BOOKMARKS    0x0004      // Bookmarks ueberlesen
 #define    SWGSTAT_LOCALFMTS       0x0008      // lokale Formate
 #define    SWGSTAT_CONVBLOCKS      0x0010      // Konversion von Textbausteinen

                                // Ablage-Variable fuer Lesen von Layout-Frames
    USHORT    nFrmFlags;        // Layout-Frame-Flags
    SwFrm*    pUpper;           // Upper Layout-Frame
    SwFrm*    pMaster;          // Master Layout-Frame
    USHORT    eStartNodeType;   // fuers erzeugen von StartNodes
                                // Fly/Footer/Header!

    SwSwgReader( SwDoc *pSwDoc, const SwPaM* pSwPaM, SvStream& rStream,
                    const String& rFileName, BOOL bNewDoc );
    ~SwSwgReader();
    ULONG   GetError() { return nErrno; }

    const SfxDocumentInfo& GetDocInfo() { return *pInfo; }

    void    LoadFileHeader();
    ULONG   Read( USHORT nOptions );
    ULONG   Scan( SwBlockNames* );
    BOOL    CheckPasswd( const String& );

    void    RegisterFmt( SwFmt& rFmt, const SwTable *pTable=0 );
    void    RegisterAttrSet( SfxItemSet* pSet, USHORT nIdx );
    void    ReleaseAttrSets();
    void    ReRegisterFmt( const SwFmt& rFmtOld, const SwFmt& rFmtNew,
                           const SwTable *pTable=0 );
    SwFmt*  FindFmt( USHORT nIdx, BYTE cType );
    SfxItemSet* FindAttrSet( USHORT nIdx );
    SwFmt*  InFormat( SwFmt* pFmt = NULL, USHORT* pParentId = 0 );
    USHORT  InAttrSet( SwAttrSet& rSet );
    USHORT  InAttrSet( SwCntntNode& rNd );
    void    FillAttrSet( SwAttrSet& rSet, USHORT nId );
    BOOL    TestPoolFmt( SwFmt&, int );

    void    RegisterFrmId( SwFrm& rFrm, USHORT nId );
    SwFrm*  FindFrmId( USHORT nId );

    void    RegisterTable( USHORT nIdx, const SwTable *pTable );
    const SwTable *FindTable( USHORT nIdx );

    USHORT  RegisterSection( const SwNodeIndex& rStart, USHORT nId );

    SwNode* InNodeId();
    SwFrm*  InFrame();
    void    InFrmRect( SwRect&, SwRect* );

    SwPageDesc& FindPageDesc( USHORT nIdx );

  // Methoden fuer Hints:
    void    Error( ULONG = 0 );
    void    AddPageDescLink( const String&, USHORT nOff );
    String  GetText( BOOL bReq = TRUE );
    ByteString GetAsciiText( BOOL bReq = TRUE );
    SwField* InField();
    SwStartNode* InSection();
    void    FillSection( SwNodeIndex& rPos );
    void    InTextBlock( long );
    Color   InColor();
    void    InPen(USHORT& nWidth, Color& rCol);
    Color   InBrush();

    void SetReadTxt( const ByteString* pTxt )   { pNdOrigTxt = pTxt; }
    const ByteString* GetReadTxt() const        { return pNdOrigTxt; }

    BOOL    IsTableBoxFrmFmt() { return BOOL( nFmtLvl == (nTblBoxLvl+1) ); }
 };


} //namespace binfilter
#endif // _RDSWG_HXX
