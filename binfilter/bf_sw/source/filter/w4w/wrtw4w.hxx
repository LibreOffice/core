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
#ifndef _WRTW4W_HXX
#define _WRTW4W_HXX

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _WRT_FN_HXX
#include <wrt_fn.hxx>
#endif
#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
class SvMemoryStream; 
namespace binfilter {

// einige Forward Deklarationen

class SvxBoxItem;
class SvxFontItem;
class SwAttrSet;
class SwDoc;
class SwFlyFrmFmt;
class SwFmt;
class SwFrmFmt;
class SwGrfNode;
class SwPageDesc;
class SwPageIter;
class SwPosFlyFrms;
class SwTable;
class SwTableLine;
class SwTableNode;
class SwTxtFmtColl;
struct SwPosition;

extern SwAttrFnTab aW4WAttrFnTab;
extern SwNodeFnTab aW4WNodeFnTab;

#define cW4W_BEGICF		'\x1b'
#define cW4W_LED		'\x1d'
#define cW4W_TXTERM		'\x1f'
#define cW4W_RED		'\x1e'

#ifdef JP_STR_OPTIMIERUNG
    JP 23.09.94: leider kann der MAC-Compiler keine String-Concatinierung

#define sW4W_BEGICF		"\x1b"
#define sW4W_LED		"\x1d"
#define sW4W_TXTERM		"\x1f"
#define sW4W_RED		"\x1e"

// lasse den Compiler die Strings zusammenbauen!
#define CNCT_STR2(a,b)   						( a b )
#define CNCT_STR3(a,b,c)   						( a b c )
#define CNCT_STR4(a,b,c,d)   					( a b c d )
#define CNCT_STR5(a,b,c,d,e)   					( a b c d e )
#define CNCT_STR6(a,b,c,d,e,f)   				( a b c d e f )
#define CNCT_STR7(a,b,c,d,e,f,g)   				( a b c d e f g )
#define CNCT_STR12(a,b,c,d,e,f,g,h,i,j,k,l)   	( a b c d e f g h i j k l )

#define CNCT_STR1_RECSTT(a)			( sW4W_BEGICF sW4W_LED a )
#define CNCT_STR2_RECSTT(a,b)		( sW4W_BEGICF sW4W_LED a b )
#define CNCT_STR3_RECSTT(a,b,c)		( sW4W_BEGICF sW4W_LED a b c )
#define CNCT_STR4_RECSTT(a,b,c,d)	( sW4W_BEGICF sW4W_LED a b c d )

#endif

extern const sal_Char __FAR_DATA sW4W_RECBEGIN[3];	// W4W_BEGICF W4W_LED
extern const sal_Char __FAR_DATA sW4W_TERMEND[3]; 	// W4W_TXTREM W4W_RED

// der W4W-Writer

class SwW4WWriter : public Writer
{
    String sVersion;		// fuer die Verschiedenen Formate
    SwPosFlyFrms* pFlyPos;	// Pointer auf die aktuelle "FlyFrmTabelle"

    ULONG nIniFlags;		// Flags aus der writer.ini
                            // obere 16 Bits fuer den W4W-Writer
                            // Bedeutung siehe Datei filter\inc\w4wpar.hxx
    ULONG nMaxNode;			// Fuer Prozent-Balken

    USHORT nFilter;
    USHORT nAktFlyPos;		// Index auf das naechste "FlyFrmFmt"
    USHORT nPgLeft;
    USHORT nPgRight;

    BOOL bStorageFlag;


    void OutW4WFontTab();
    void OutW4WStyleTab();

    void MakeHeader();

        // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    void OutFlyFrm();
    void OutW4WFlyFrms( const SwFlyFrmFmt& );

    ULONG WriteStream();

public:

    SwFlyFrmFmt* pFlyFmt;			// liegt der Node in einem FlyFrame, ist
                                    // das Format gesetzt, sonst 0
    SvMemoryStream* pStyleEndStrm;	// fuer StyleOn/Off
    SvMemoryStream* pPostNlStrm;	// fuer Attribute, die nach dem naechsten
                                    // HNL abgeschaltet werden muessen
    const SwPageDesc* pPageDesc;   	// aktuelles Seitenlayout
    SwPosition* pNxtPg;				// AnfangsPosition der naechsten Seite
    const SwPageDesc* pPgDsc2;     	// aktuelles Seitenlayout,
                                    // doppelt gemoppelt !?
    SwPageIter* pIter;				// Iterator fuer seitenweise Spruenge
    const SwFrmFmt* pPgFrm;			// akt. PageFrame, i.A. pPgDsc2->GetMaster()
    const SwFmt* pNdFmt;			// akt. Format/Collection vom Node
                                    // fuer den Zugriff auf einige Attribute
                                    // z.B. Font-Size, LR-Space,..

    SwTwips nFlyWidth, nFlyHeight;	// Fuer Anpassung Graphic
    SwTwips nFlyLeft, nFlyRight;	// fuer Anpassung Raender

    rtl_TextEncoding eTargCharSet;	// Target-CharSet

    USHORT nFontHeight;				// akt. Hoehe vom Font
    USHORT nFontId;					// Id vom akt. Font
    USHORT nDefLanguage;			// Default-Language zum Ausschalten d. Attr

    BOOL bFirstLine : 1;			// wird die 1. Zeile ausgegeben ?
    BOOL bStyleDef : 1;				// ist gerade eine Style-Def am werden ?
    BOOL bStyleOnOff : 1;			// wird gerade ein Style An/Ausgeschaltet ?
    BOOL bTable : 1;				// ist gerade eine Tabelle am werden ?
    BOOL bPageDesc : 1;				// wird gerade ein PageDesc ausgegeben ?
    BOOL bEvenPage : 1;				// ist gerade Seitenzahl ?
    BOOL bLastPg :1;				// Letzte Seite ?
    BOOL bHdFt : 1;					// wird gerade Header oder Footer ausgegeben ?
    BOOL bIsTxtInPgDesc : 1;        // gegen ueberfluessige HNLs
    BOOL bFly : 1;					// ist gerade ein FlyFrame am werden ?
    BOOL bTxtAttr : 1;				// werden TextAttribute ausgegeben ?
    BOOL bAttrOnOff: 1;				// TRUE: Attribut StartCode; sonst
                                    // den EndCode ausgeben

    SwW4WWriter( const String& rFilterName );
    virtual ~SwW4WWriter();

    void Out_SwDoc( SwPaM*, BOOL bAttrOn );

    void OutW4W_CountTableSize( const SwTable*, USHORT&, USHORT&, USHORT& );
    BYTE OutW4W_GetTableColPos( const SwTableLine*, SwHoriOrient, SwTwips* );
    void OutW4W_WriteTableHeader( BYTE nCode, USHORT nCols, SwTwips* pCellPos );
    BOOL OutW4W_GetBorder( const SvxBoxItem* pBox, USHORT& rBorder );
    void OutW4W_WriteTableLineHeader( USHORT nCols, USHORT , USHORT* = 0);
    void OutW4W_WriteTableBoxHeader( USHORT nNumCol, USHORT nSpan );
    BOOL OutW4W_TestTableLineWW2( const SwTable*, USHORT, USHORT, SwTwips* );
    void OutW4W_TableLineWW2( const SwTable*, USHORT, USHORT, USHORT, SwTwips* );
    void OutW4W_SwTableWW2( const SwTable*, USHORT, USHORT, USHORT,
                             SwHoriOrient, SwTwips* );
    void OutW4W_TableLineNorm( const SwTable*, USHORT, USHORT, USHORT, SwTwips* );
    void OutW4W_SwTableNorm( const SwTable*, USHORT, USHORT, USHORT,
                             SwHoriOrient, SwTwips* );

    void OutW4W_SwTable( const SwTableNode* pTableNd );

    USHORT GetId( const SwTxtFmtColl& );
    USHORT GetId( const SvxFontItem& );
    const SvxFontItem& GetFont( USHORT nId ) const;

    void GetMargins( SwTwips& rLeft, SwTwips& rWidth ); // ermittle Raender

    USHORT GetFilter() const 				{ return nFilter; }
    const String& GetVersion() const 		{ return sVersion; }
    void ChgVersion( const String&  rVers )	{ sVersion = rVers; }
    void ChgFilter( USHORT nFltr ) 			{ nFilter = nFltr; }
    ULONG GetIniFlags() const 				{ return nIniFlags; }

        // gebe die TextCollection am TextNode aus. Die Attribute
        // werden im Style an- und abgeschaltet. Das Abschalten wird im
        // extra Stream gespeichert !!
    void Out_SwTxtColl_OnOff( const SwTxtFmtColl& rTxtColl,
                                const SwAttrSet* pAutoSet, BOOL bOn );

        // nicht inline, um Pointer zu pruefen
        // bStyleStream = FALSE returnt den normalen Ausgabe-Stream (wird
        // fuer die TextAttribute benoetigt!)
    SvStream& GetStrm( BOOL bStyleStream = TRUE ); //$ ostream

    SvStream& GetNlStrm(); //$ ostream

    // fuer W4WSaveData
    SwPaM* GetEndPaM() 				{ return pOrigPam; }
    void SetEndPaM( SwPaM* pPam ) 	{ pOrigPam = pPam; }

    // fuer Grafiken
    void OutW4W_Grf( const SwGrfNode* pGrfNd );

    // fuer Storage / Stream
    virtual BOOL IsStgWriter();
};

struct W4WSaveData
{
    SwW4WWriter& rWrt;
    SwPaM* pOldPam, *pOldEnd;
    SwFlyFrmFmt* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;
    SvMemoryStream* pOldStyleEndStrm;		// fuer StyleOn/Off
    const SwFmt* pOldNdFmt;				// akt. Attribute vom Node
    BOOL bOldWriteAll;

    W4WSaveData( SwW4WWriter&, ULONG nStt, ULONG nEnd );
    ~W4WSaveData();
};

// einige Funktions-Deklarationen
Writer& OutW4W_SwChar( Writer& rWrt, sal_Unicode c, BOOL bRaw );
Writer& OutW4W_String( Writer& rWrt, const String& rStr );
Writer& OutW4W_SwFmt( Writer& rWrt, const SwFmt& rFmt );

SvStream& W4WOutHex( SvStream& rStrm, USHORT nHex );
SvStream& W4WOutHex4( SvStream& rStrm, USHORT nHex );
SvStream& W4WOutLong( SvStream& rStrm, long nVal );
SvStream& W4WOutULong( SvStream& rStrm, ULONG nVal );
void OutW4W_SwFmtPageDesc1( SwW4WWriter& rW4WWrt, const SwPageDesc* pPg );


} //namespace binfilter
#endif	//  _WRTW4W_HXX
