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
#ifndef _W4WPAR_HXX
#define _W4WPAR_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX //autogen fuer DECLARE_TABLE
#include <tools/table.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>			// SwSelBoxes
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>			// numerierte Listen
#endif

class String; 
class Graphic; 
namespace binfilter {

class SfxPoolItem;
class SfxItemSet; 

// Konstanten fuer die internen Fehler-Codes beim Einlesen
#define ERR_NOERROR 0
#define ERR_CHAR 	1
#define ERR_RECORD	2
#define ERR_NAMETOLONG 	3

// Konstanten fuer den Anfang und Ende der Records
#define W4WR_BEGICF		0x1B
#define W4WR_LED		0x1D
#define W4WR_TXTERM		0x1F
#define W4WR_RED		0x1E

#define W4W_INVALID (USHORT)-1

#define W4WFL_NO_FLY_IN_CNTNT 0x01
#define W4WFL_NO_GRAF_IN_CNTNT 0x02
#define W4WFL_EXACT_TABLES 0x04
#define W4WFL_NO_DBL_TBL_BORDER_REMOVE 0x08
#define W4WFL_NO_WW_SPECCHAR 0x10
#define W4WFL_NO_APO_HNL_REMOVE 0x20
#define W4WFL_NO_DBL_APO_BORDER_REMOVE 0x40
#define W4WFL_NO_HDFT_DIST 0x80
#define W4WFL_ALL_HDFT_DIST 0x100
#define W4WFL_NO_PCTEXT4_124 0x200
#define W4WFL_NO_PCTEXT4_INDENT 0x400
#define W4WFL_ALL_HDFT_MAINATTR 0x800
#define W4WFL_NO_PCTEXT4_HDFT_MAINATTR 0x1000
#define W4WFL_NO_PCTEXT4_STP_DEC2 0x2000

//      ( Standardverhalten: analysieren, was Standard sein duerfte )
//      W4WFL_FrameMaker_cleverFrames  0x00000000;

//      ( keine Frames: blind einlesen )
#define W4WFL_FM_neverFrames 0x00000001

//      ( ALLES als Frames einlesen:
//        kein Content fuer Header/Body/Footer )
#define W4WFL_FM_onlyFrames  0x00000002




typedef USHORT* UShortPtr;

class SwDoc;
class SwPaM;

class SwTxtFmtColl;
class SwPageDesc;
class SvxBoxItem;

class W4WTabBorders;

class W4WMergeGroups;	// enthaelt Sw_SelBoxes_SAR

class W4WStyleIdTab;
class W4WCtrlStack;
class SwTable;
class SwTableNode;
class SwTableLines;
class SwTableLine;
class SwTableBoxes;
class SwTableBox;

class SwW4WGraf;
class SvxLRSpaceItem;
class SvxULSpaceItem;
class swistream;
class SwFlyFrmFmt;
struct W4WRecord;


struct W4WRecType
{
    sal_Char c0, c1, c2;
    void Reset()
    {
        c0 = 0;
        c1 = 0;
        c2 = 0;
    }
    BOOL IsEqual(const W4WRecType& rRec) const
    {
        return (rRec.c0 == c0) && (rRec.c1 == c1) && (rRec.c2 == c2);
    }
    BOOL IsEqual( const sal_Char* pRec, int nLen ) const
    {
        return (3 == nLen )
            && (*(pRec+0) == c0)
            && (*(pRec+1) == c1)
            && (*(pRec+2) == c2);
    }
};


struct W4W_ColdT	   	// ColumnsDefinitionTyp, nur in W4WPar2.cxx benutzt
{
    long nLeft;		   	//!!! kann eigentlich nach w4wpar2.cxx
    long nRight;
    long nNC;
    long nLeftTw;
    long nRightTw;
    long nNC2;
    long nWidthFrac;
    USHORT nMergeGroupIdx;		// falls groesser -1 ist Merge-Gruppe offen
    long nRemainingMergeRows;	// wieweit nach unten mergen wir noch dazu
};


struct W4WRectInfo
{
    SwRect	aRect;
    int		nPDTId;	// Nummer der Seite im PDT array
    int		nFLOId;	// Nummer des FLO, in dem das Rect enthalten ist

    int		nFMId;	// urspruengliche FrameMaker-Id aus dem W4W-Code
    int		nSWId;	// Id des W4WRectInfo, den wir im SW dafuer nehmen
                    // ( z.B. bei Spaltigkeit: alle Spalten weisen
                    //                         auf den linken Rahmen )

    BOOL	bInside: 1; // Frame liegt in einem anderen drinnen
    BOOL	bColXYZ: 1; // Folge-Spalten eines spalt. Bereichs
    BOOL	bBody:   1; // Body-Bereich, also KEIN Fly
    BOOL	bHeader: 1; // Header " "
    BOOL	bFooter: 1; // Footer " "

    W4WRectInfo(const SwRect& rRect_, int nId_, int nPDTId_ = -1):
                aRect(  rRect_ ),
                nPDTId(	nPDTId_),
                nFMId(	nId_   ),
                nSWId(	nId_   )
        {
            bInside	= FALSE;
            bColXYZ	= FALSE;
            bBody	= FALSE;
            bHeader	= FALSE;
            bFooter	= FALSE;
        }
    /*
    void StoreRect(SwRect* pRect_, int nId_, int nPDTId_)
        {
            aRect	= pRect_;
            nPDTId	= nPDTId_;
            nFMId	= nId_;
            nSWId	= nId_;
            bInside	= FALSE;
            bColXYZ	= FALSE;
            bBody	= FALSE;
            bHeader	= FALSE;
            bFooter	= FALSE;
        }
    void StoreRect(W4WRectInfo& rInfo)
        {
            pRect	= rInfo.pRect;
            nPDTId	= rInfo.nPDTId;
            nFMId	= rInfo.nFMId;
            nSWId	= rInfo.nSWId;
            bInside	= rInfo.bInside;
            bColXYZ	= rInfo.bColXYZ;
            bBody	= rInfo.bBody;
            bHeader	= rInfo.bHeader;
            bFooter	= rInfo.bFooter;
        }
    */
    BOOL IsActive(){ return !(bColXYZ || bInside); }
    BYTE BodyHdFtMode(){ return bBody ? 1 : (bHeader ? 2 : 3); }
};


struct W4WPDTInfo
{
    String	sPageName;
    long*	pIds;
    long	nPageType;		// 1 == Body Page,  2 == Master Page,  3 == Reference Page
    long	nPageSubType;	// 1 == Left Page,  2 == Right Page,   3 == First Page ???
    long	nRects;
    USHORT	nSwPdId;
    USHORT	nBodyRectId;
    USHORT	nHdRectId;
    USHORT	nFtRectId;
    BOOL	bSwPdUsed			:1;
    BOOL	bPageWidthUnknown	:1;// Page Margin rechts noch nicht eingestellt
    BOOL	bPageHeightUnknown	:1;//			  unten
    W4WPDTInfo()
       :nPageType(    1 ),
        nPageSubType( 0 ),
        nRects(		  0 ),
        pIds(         0 ),
        nBodyRectId(  USHRT_MAX ),
        nHdRectId(    USHRT_MAX ),
        nFtRectId(    USHRT_MAX ),
        nSwPdId(      USHRT_MAX )
        {
            bSwPdUsed			= FALSE;
            bPageWidthUnknown	= TRUE;
            bPageHeightUnknown	= TRUE;
        }
    ~W4WPDTInfo(){ delete pIds; }
    BOOL PageSizeKnown(){ return 0 == (   bPageWidthUnknown
                                        | bPageHeightUnknown ); }
};
typedef W4WPDTInfo* W4WPDTInfo_Ptr;
SV_DECL_PTRARR_DEL(W4WPDTInfos, W4WPDTInfo_Ptr, 16, 16)


struct W4WFLOInfo
{
    long*	pIds;
    long	nFlowId;
    long	nRects;
    BOOL	bFlowUsed:1;
    W4WFLOInfo()
       :nFlowId( 0 ),
        nRects(  0 ),
        pIds(    0 )
        { bFlowUsed = FALSE; }
    ~W4WFLOInfo(){ delete pIds; }
};
typedef W4WFLOInfo* W4WFLOInfo_Ptr;
SV_DECL_PTRARR_DEL(W4WFLOInfos, W4WFLOInfo_Ptr, 16, 16)


DECLARE_TABLE(W4WRDTInfos, W4WRectInfo*)


class SwW4WParser
{
    // Zum Unterscheiden, von wo die Methode
    // SwW4WParser::UpdatePageMarginSettings() aufgerufen wurde
    enum W4W_UpdtPgMrgSet
    {
        CALLED_BY_HF2,			// Methode wurde aufgerufen wegen HF2-Flag,
        CALLED_BY_HF1, 			// wegen HF1-Flag,
        CALLED_BY_FLUSH_OR_HNL	// aus SwW4WParser::Flush() oder nach .<HNL>
    };

    // Zur Parametrisierung von SwW4WParser::CreatePageDesc()
    enum W4W_CreatePgDescSet
    {
        CRPGD_AS_FOLLOW = 0x01, // es soll ein Follow-PageDesc werden
        CRPGD_AND_INSERT= 0x02, // PageDesc wird ins pDoc inserted
        CRPGD_REMOVE_HD = 0x04, // Header des Vorgaengers NICHT uebernehmen
        CRPGD_REMOVE_FT = 0x08, // Footer des Vorgaengers NICHT uebernehmen
        CRPGD_UPDT_MRGN = 0x10  // ggfs. SwW4WParser::SetPageMgn() aufrufen
    };

    // zum Erkennen des unbekannten Zustands einer booleschen Variablen
    enum W4W_THREE_STATE
    {
        VALUE_FALSE   = 0x00,	// wahr
        VALUE_UNKNOWN = 0x01,	// (noch) unbekannter Wert
        VALUE_TRUE    = 0x02	// unwahr
    };


    sal_Unicode aCharBuffer[ 512 ];	// Puffer fuer die Zeichen

    const String& rVersion;		// Unterversion des Filteraufrufs
    //; Bit 0: Rahmen werden niemals an Zeichen gebunden
    //; Bit 1: Bilder werden niemals an Zeichen gebunden
    //; Bit 2: Tabelle punktgenau positionieren ( nicht so robust )
    //; Bit 3: Umrandungen von Tabellenzellen nicht ausduennen
    //; Bit 4: bei WinWord2 Umlauten nicht mogeln ( ab Version 94c )
    //; Bit 5: bei WinWord2 ueberfluessige Leerzeilen nicht ignorieren ( ab Version 102 )
    //; Bit 6: Bei Frames doppelte Umrandungen nicht unterdruecken

    //; Bit 16: ASCII-Export darf ^L enthalten
    //; Bit 17: Grafiken nicht exportieren

    /*
        Pointer
    */
    W4WCtrlStack* pCtrlStck;	// Stack fuer die Attribute

    SwW4WGraf* pGraf;			// Pointer auf W4W-Grafik-Klasse
    Graphic* pGraphic; 			// Hier ist die Grafik

    SwTableNode*  pTblNd;           // Tabellen-Node
    SwTableLine*  pTabLine;        	// akt. Zeile
    SwTableBoxes* pTabBoxes;		// Boxen-Array in akt. Zeile
    SwTableBox*   pTabBox;			// akt. Zelle

    W4W_ColdT* pTabDefs;			// ColumnsDefinition

    W4WTabBorders* pTabBorders;		// .<BRO>-Raender einer ganzen Tabelle

    W4WMergeGroups* pMergeGroups; 	// Listen aller zu verknuepfenden Zellen

    SwNodeIndex* pBehindSection;	// Node-Index zum Zuruecksetzen des PaM nach einem Bereich

    W4WPDTInfos* pPDTInfos;			// array von PageDefinitionTable
    W4WRDTInfos* pRDTInfos;			// array von RectangleDefinitionTable
    W4WFLOInfos* pFLOInfos;			// array von Text Folow

    W4WPDTInfo*  pPDTInfo;			// Id der derzeit aktuellen PDT (FrameMaker)
    W4WRectInfo* pActRectInfo;		// Rect-1 des derzeitigen TextFlow (FrameMaker)

    // aktuelles Seitenlayout, oder das bei naechstem
    // Seitenumbruch automatisch zu nehmende Layout
    SwPageDesc* pPageDesc;

    // letztes per PageAttr gesetztes Seitenlayout
    SwPageDesc* pLastActPageDesc;

    // aktuelles Seitenlayout, falls bereits ein Folge-Layout definiert wurde,
    // das in pPageDesc gespeichert ist, sonst 0
    SwPageDesc* pOpen1stPgPageDesc;

    // kommendes Seitenlayout ab naechstem .<HNP> fuer den Fall,
    // dass wir den Follow des in pPageDesc gespeicherten Layouts
    // nicht aendern wollen/duerfen, sonst 0
    SwPageDesc* pPageDescForNextHNP;

    SwPaM *pCurPaM;				// Writer-Einfuegepos
    SwFlyFrmFmt* pActFlySection;
    SwDoc *pDoc;				// aktuelles Dokument

    SvStream& rInp;				// gepufferter swistream dazu

    W4WStyleIdTab* pStyleTab;	// Tabelle fuers Mappen von Id auf Style
    W4WStyleIdTab* pStyleBaseTab; // Tabelle fuer "Style based on"

    SwNumRule* pActNumRule;		// derzeitige num. Liste laut letztem .<PND>

    W4WRecord *pActW4WRecord;	// letzte, passende Methode aus ::GetNextRecord()
    W4WRecType aLastRecType;	// in Main Loop: last Token that has been processed

    String* pReadTxtString;		// zum Einlesen in :Flush(),
                                // falls bReadTxtIntoString gesetzt ist

    /*
        long und ULONG
    */
    long nGrafPDSType;			// Typ der Graphik aus PDS-Befehl
    long nGrWidthTw;			// Groesse der Grafik
    long nGrHeightTw;
    long nParaLen; 				// zum Zaehlen gegen UEberschreitung d. max. Absatzlaenge
    long nRecNo;				// Befehlszaehler zum besseren Debuggen
    long nRSMLeft;				// Merke RSM-Werte bis zum naechsten Seitenwechsel
    long nRSMRight;
    long nRSMwpwParaLeft;		// Merke RSM-Werte fÅr WPWin - Absatz-Raender
    long nRSMwpwParaRight;

    // Hilfs-Varis zur Umrechnung
    // von W4W-Randangaben
    // zu denen im Writer
    long nLastValueSTP;			// zuletzt wegen STP gesetzter Wert
    long nLastValueSBP;         // zuletzt wegen SBP gesetzter Wert
    long nLastValueHTM;         // zuletzt wegen HTM gesetzter Wert
    long nLastValueHM;          // zuletzt wegen HM  gesetzter Wert
    long nLastValueFBM;         // zuletzt wegen FBM gesetzter Wert
    long nLastValueFM;          // zuletzt wegen FM  gesetzter Wert
    long nNewValueSTP;			// jetzt neu von STP angegebener Wert
    long nNewValueSBP;			// jetzt neu von SBP angegebener Wert
    long nNewValueHTM;			// jetzt neu von HTM angegebener Wert
    long nNewValueHM;			// jetzt neu von HM  angegebener Wert
    long nNewValueFBM;			// jetzt neu von FBM angegebener Wert
    long nNewValueFM;			// jetzt neu von FM  angegebener Wert

    long nW4WFileSize;			// fuer Prozentskala

    long nTabDeltaSpace;		// durchschnittl. Zellen-Zwischenraum
    long nTabWidthTw;	  		// GesamtBreite ALLER Spalten in Twips

    long nLastProcessedCol;     // zuletzt per .<BC0_> definierte Zelle

    long nStartOfActRecord;
    long nStreamPosInRec;

    ULONG nIniFlags;			// Flags aus der writer.ini
                                // untere 16 Bits fuer den W4W-Reader
    ULONG nIniHdSiz;			// Flags fuer Default-Size Header
    ULONG nIniFtSiz;			// dito fuer Footer
    ULONG nIniFMFlags;			// Flags fuer Import von FrameMaker Dokumenten

    W4W_THREE_STATE eFootNotePageRestart; // neu zaehlen ab jeder Seite

    /*
        USHORT und short
    */
    USHORT nDefLanguage;		// diese Sprache statt Ausschalter

    USHORT nDocType;			// Quellformat
    USHORT nAktPgDesc;			// momentan gueltiges Seitenlayout
    USHORT nApoBorderCode;		// Diese APO hat folgenden BorderCode
                                // gegen doppelte Border in WW2-Apos

    USHORT nColSize;			// wenn keine Angaben in Twips existieren
    USHORT nTablInTablDepth;	// Ebenen-Tiefe von Tabelle IN Tabelle

    USHORT nHdFtType;			// Typ des letzten Kopf/Fusstextes
    USHORT nPDType;				// Typ des zuletzt angelegten Seitenlayouts
    USHORT nTabRows;			// V-SSize der Tabelle
    USHORT nTabCols;            // H-SSize
    USHORT nTabCol;				// aktuelle H-Pos

    short nTabRow;				// V-Pos in Tabelle

    USHORT nTabDefBorder;		// Was fuer einen Standard-Rahmen ist gewaehlt

                                // gecachete Raender des SeitenLayouts
    USHORT nPgLeft;				// SLay L.Rand
                        // = pPageDesc->GetMaster()->GetLRSpace()->GetTxtLeft()

    USHORT nLeftMgnCorr;

    USHORT nPgRightDelta;       // dito ...->GetRight()

    USHORT nPgRight;            // dito ...->GetWidth() - ->GetRight()

    USHORT nPgWidth;            // dito ...->GetFrmSize().GetFixSize()

    USHORT nAktStyleId;			// Id vom aktuellen StyleSheet (W4W-Nummerierung)
    USHORT nTabStyleId;			// Id des Style vor Tabellen-Anfang (W4W-Nummerierung)
    USHORT nNoNameStylesCount;	// Anzahl der Styles mit ungueltigem oder fehlendem W4W-Namen

    USHORT nChrCnt;				// Counter fuer aCharBuffer

    USHORT nLastReadFootNo;		// zuletzt eingelesene Fussnoten-Nr.
    USHORT nLastReadEndNo;		// zuletzt eingelesene Endnoten-Nr.

    /*
        char und BYTE
    */
    char nFtnType;				// Fuss- oder EndNote, -1 fuer noch nicht bekannt

    BYTE nError;				// igendwelche Fehler ??

    BYTE nTabCode;					// Tabellen-Definitions-Byte

    BYTE nUnderlineMode;		// alles oder nur Worte unterstreichen

    BYTE nActNumLevel;			// Nummerierungs-Tiefe des akt. Absatz

    BYTE nFootNoOverflow;		// Ueberlaufzaehler fuer Fussnoten-Nummern
                                // beim FNI-Typ Gross- oder Kleinbuchstaben
    BYTE nEndNoOverflow;		// Ueberlaufzaehler fuer Endnoten-Nummern
                                // beim FNI-Typ Gross- oder Kleinbuchstaben
    /*
        BOOL mit 1 Bit
    */
    BOOL bPDTanalyzed	: 1;	// FrameMaker-Flag, ob Page Definition Table
                                //       bereits ausgewertet wurde
                                //       und die entrsprechenden PgDesc erzeugt sind

    BOOL bStyleDef : 1;			// Flag, ob gerade Style definitiniert wird
    BOOL bHeadFootDef : 1;      // Flag, ob gerade Header oder Footer definiert werden
    BOOL bFootnoteDef : 1;		// Flag, ob gerade Fussnote definiert wird

    BOOL bStyleEndRec	: 1;	// StyleEndRec erkannt
    BOOL bStyleOnOff	: 1;	// im StyleSheet On oder Off Record
    BOOL bStyleOn		: 1;	// im StyleSheet On Record
    BOOL bTxtInDoc		: 1;	// setze den akt. CharBuffer ins Dokument
    BOOL bPageDefRdy	: 1;	// Flag, ob Page-Definition abgescholssen ist
    BOOL bIsTxtInDoc	: 1;	// Flag, ob im Doc schon Text steht
                                // (leeren Absatz vor Tabelle erzeugen?)
    BOOL bIsTxtInFNote : 1;		// Kam schon Text (oder ein TAB) in dieser
                                // Fussnote? (der erste TAB in e. Fussnote
                                //     wird naemlich jeweils uebersprungen)
    BOOL bIsTxtInPara : 1;		// Ist der Absatz nicht mehr leer ?
                                // Mogelei gegen leere Absaetze WW2
                                // + Sicherheit, dass PageDesc nicht in leeren Abs.
    BOOL bIsTxtInPgDesc : 1;    // Flag, ob dieser PageDescriptor schon Text beinhaltet
                                // (ist die Erzeugung eines neuen Seitenlayouts noetig?)
    BOOL bWasTxtSinceLastHF2 : 1; // Flag, ob Text seit HF2 bzw. Textbeginn
    BOOL bWasTxtSinceLastHF1 : 1; // Flag, ob Text seit HF1 bzw. Textbeginn
    BOOL bWasTxtSince_BREAK_PAGE : 1; // ob seit letztem HNP schon Text kam
                                // (Ist Pagebreak bei folgendem Pagedesc ueberfluessig?)
    BOOL bWasHNPInPgDesc : 1;	// Gab es in diesem PgDesc bereits .<HNP>-Kodes ?
                                // (wir verschlucken erstes .<HNP> in leerem PgDesc)

    BOOL bStyleOff : 1;			// Flag, ob am Absatzende Default-Layout gesetzt werden muss
    BOOL bBCMStep1 : 1; 		// Flag, ob .<BCM> die Tabelle zum 1. Mal liest
    BOOL bBCMStep2 : 1;	  		// Flag, ob .<BCM> die Tabelle zum 2. Mal liest
    BOOL bNoExec : 1;           // Flag, ob gerade Kommandos ausser "HFX" (Ende Kopf/Fusstext)
                                //  ignoriert werden sollen
    BOOL bSepar : 1;			// nach (SEP) in der Style-Definition gesetzt
    BOOL bPDFirstOnly : 1;		// zuletzt angelegtes Seitenlayout gilt nur fuer 1. Seite
    BOOL bIsNLN : 1;			// ist nach (NLN) bis ca. Anfang des naechsten Textes gesetzt
    BOOL bDefFontSet : 1;		// ist bereits Default-Font gesetzt ?
    BOOL bIsColMode : 1;		// ist Column-Mode (z.B. eine Tabelle am entstehen) ?
    BOOL bWasCellAfterCBreak:1;	// FALSE, wenn 2 HCB nacheinander (ohne BCO )
    BOOL bWasPGNAfterPND : 1;
    BOOL bTabBorder : 1;		// Umrandung der Zellen in Tabelle
    BOOL bTabSpezBorder : 1;	// andere als Standart-Umrandung der Zellen in Tabelle
    BOOL bCheckTabAppendMode:1; // Pruef-Flag fuer .<CDS>
    BOOL bIsTabAppendMode : 1;  // aktuelle Tabelle und vorher definierte Tabelle
                                // gehoeren zusammen und werden in EINE eingelesen
    BOOL bIsSTYInTab : 1;		// um Style glaubwuerdig zu simulieren
    BOOL bIsColDefTab : 1;		// Tabelle statt Mehrspaltigkeit
    BOOL bPicPossible : 1;		// hier kann eine Grafik stehen
    BOOL bPic : 1;				// Ein Bild ist erfolgreich eingelesen worden
    BOOL bIsIpsInPara : 1;		// beinhaltet dieser Absatz einen IPS-Befehl ?
                                //			->  STM ignorieren
    BOOL bToxOpen : 1;			// in Tox
    BOOL bPersAlign : 1;		// persistent Alignment (BRJ)
    BOOL bPgMgnChanged : 1;		// Page Margin has changed
    BOOL bWasXCS : 1;			// Um nach XCS-Records 1 Zeichen zu ueberlesen
    BOOL bWasSLG : 1;			// 1. Laguage Group ist Default
    BOOL bIgnoreNTB : 1;		// nach Auftreten von RUL wird NTB ignoriert

    BOOL bWWWasRSM : 1;			// 1. RSM wg. WW-Bug bescheissen
    BOOL bIsTab : 1;			// fuer AmiPro StyleDef virt. Tabs

    BOOL bWPWWasRSM : 1;	 	// fuer WPWin-Import RSM-Sonderbehandlung
    BOOL bSingleParaMgn : 1;    // fuer WPWin-Import STM/IPS-Behandlung

    BOOL bIsSTMInPara : 1;      // fuer WPWin-Import STM-Sonderbehandlung
    BOOL bIsNumListPara : 1;    // Absatz einer nummerierten Liste

    BOOL bSetPgWidth : 1;
    BOOL bSetPgHeight : 1;

    BOOL bTabOwnFrm : 1;		// Flag : eigenes FraneFmt fuer alle Zellen

    BOOL bNew : 1;				// neues Dokument oder zufuegen

    BOOL bEndNoteInfoAlreadySet  :1;
    BOOL bFtnInfoAlreadySet      :1;
    BOOL bFootNoteNumberAutomatic:1;
    BOOL bEndNoteNumberAutomatic :1;
    BOOL bWasFootNoteOnW4WPage   :1;

    BOOL bReadTxtIntoString :1;	// :Flush() speichert dann *nicht* ins pDoc

    BOOL bNixNoughtNothingExec :1;	// keinerlei Methoden werden ausgefuehrt


    /*
        Methoden
    */
    BOOL ContinueHdFtDefinition( BOOL bFollow, long nLMarg, long nRMarg );
    void Read_HdFtDefinition(  BOOL bHeader );
//	void Read_ColBrk();

    void SetPamInCell( USHORT nRow, USHORT nCol, BOOL bSetPaM = TRUE );
    SwTableBox* UpdateTableMergeGroup(  SwSelBoxes_SAR* pActGroup,
                                        SwTableBox* pActBox,
                                        USHORT nCol );

    void Flush();				// speichert Buffer im Document
    void FlushChar( sal_Unicode c );

        // lese und bearbeiten den naechsten Record
    int GetNextRecord();

        // returnt W4WR_RED/W4WR_TXTERM wenn einstellige HexZahl, sonst 0
        // in rHexVal steht der gewandelte Hex-Wert
    BYTE GetChar( BYTE& rHexVal );
    BYTE GetDeciByte( BYTE& rByteVal );
    BYTE GetHexByte( BYTE& rHexVal );
    BYTE GetHexUShort( USHORT& rHexVal );
        // returnt W4WR_RED/W4WR_TXTERM
        // in rDecVal steht der gewandelte Decimal-Wert
    BYTE GetDecimal( long& rDecVal );
        // ueberliest alles bis zum W4WR_TXTERM oder W4WR_RED (wird returnt)

    BOOL GetString( String& rString, const int nEndCode1, const int nEndCode2 );

    BOOL GetNextName(); // liest Namen (inc. Sonderzchn.) in aCharBuffer ein

    SwPageDesc* CreatePageDesc( USHORT eCreateMode );
//	SwPageDesc& GetAktPgDsc();

    BYTE SkipPara();
    BYTE SkipParaX();
    BYTE SkipEndRecord();

    void StyleAnfang(); 		// liest StyleName und Id

    const SfxPoolItem* GetFmtAttr( USHORT nWhich );
    void SetAttr( const SfxPoolItem& rHint );
    void SetAttrOff( const SfxPoolItem& rHint );

        // returnt den abs. Rand der Seite (SeitenRand+NodeRand)
    long GetLeftMargin() const;
    void Read_FootNoteStart(char Type, BYTE   nNoLow,
                                       BYTE   nNoHigh,
                                       USHORT FootNo);
    void DeleteZStk( W4WCtrlStack*& rpStk );
    USHORT Read_SetBorder( USHORT nBor, SvxBoxItem& rFmtBox );
    USHORT Read_SetTabBorder( USHORT nW4WRow , USHORT nW4WCol , SvxBoxItem& rFmtBox );
    void Adjust_pTabDefs();
    BOOL GetULSpace( SvxULSpaceItem*& rpUL );
    BOOL GetLRULHint( const SfxPoolItem*& rpH, RES_FRMATR eTyp );
    void SetPageMgn();
    void DoSplit();
    void UpdateCacheVars();
    void SetPageDescVSpaces( SwPageDesc& rPageDesc,
                             W4W_UpdtPgMrgSet eCalledByWhom);
    void UpdateHdFtMarginSettings( SwFrmFmt *pHdFmt,
                                   long nHeight,
                                   long nDelta,
                                   BOOL bDoTheHeader );
    void AdjustTempVar1( long& rHeight, long& rPgDelta,
                         long& rHdFtDelta, long  nIniHdFtSiz );
    void UpdatePageMarginSettings( W4W_UpdtPgMrgSet eCalledByWhom );
    void Read_IndentPara1(long  nLeft, long n1st, long nRight);

    void FlySecur( BOOL bAlignCol,
                    long& rXPos, long& rYPos, long& rWidthTw, long& rHeightTw,
                    RndStdIds& rAnchor, long* pTop = 0, long* pLeft = 0,
                    long* pBot = 0, long* pRight = 0, USHORT nBorderCode = 0 );

    void Read_HardAttrOff( USHORT nResId );

    void ReadTabDefs( BYTE nCode, long nCols, W4W_ColdT* pActTabDefs );

    SwFlyFrmFmt* MakeTxtFly( RndStdIds eAnchor, const SfxItemSet& );

    void UpdatePercent( ULONG nPos, ULONG nFileSize );

    void ActivateTxtFlags();

    void SetFtnInfoIntoDoc( BOOL bEndNote, BYTE nRestart, BYTE nStyle,
                            String& rPrefixTxt,
                            String& rSuffixTxt );

    void Read_ProcessHiddenText( BOOL bInline );
    void ProcessRecordInHiddenText();

public:
    SwW4WParser( const SwPaM &, SvStream& rIstream, BOOL bNewDoc, //$ istream
                 USHORT nFilterNo, const String& rVersion );

    BOOL CallParser();

    BOOL IsNewDocument() 	{ return bNew; }
    SwDoc& GetDoc() const 	{ return *pDoc; }			// aktuelles Dokument
    sal_Char ReadChar() 	{ sal_Char c = 0; rInp.Read( &c, 1 ); return c; }

    USHORT GetAktCollId() { return nAktStyleId; }
    void SetAktCollId( USHORT  nId ) { nAktStyleId = nId; }
    SwTxtFmtColl* GetAktColl(  BOOL* pSetAttrFlag = 0 ) const;	// hole die durch nAktStyleId angegebe Style

    void MakeUniqueStyleName( String& rName );

    // die Read-Methoden fuer die verschiedenen Records
    void Read_DefineDocumentType();		// zur Zeit Dummies

    void Read_UpperCharSet();
    void Read_ExtendCharSet();
    void Read_HardNewLine();
    void Read_SoftNewLine();
    void Read_SoftNewPage();
    void Read_HardNewPage();
    void Read_HardSpace();
    void Read_NewLineInPara();
    void Read_Tab();
    void Read_StyleTable();
    void Read_StyleEnd();
    void Read_StyleOn();
    void Read_StyleOff();
    void Read_DateTime();

        // Read-Methoden fuer die Attribute
    void Read_BeginBold();
    void Read_EndBold();
    void Read_BeginUnderline();
    void Read_EndUnderline();
    void Read_BeginItalic();
    void Read_EndItalic();
    void Read_BeginDoubleUnderline();
    void Read_EndDoubleUnderline();
    void Read_BeginSuperScript();
    void Read_EndSuperScript();
    void Read_BeginSubScript();
    void Read_EndSubScript();
    void Read_BeginColoredText();
    void Read_EndColoredText();
    void Read_NewTabTable();
    void Read_BeginFlushRight();
    void Read_EndFlushRight();
    void Read_BeginCenterText();
    void Read_EndCenterText();
    void Read_BeginRightJustify();
    void Read_EndRightJustify();
    void Read_IndentPara();
    void Read_BeginStrikeOut();
    void Read_EndStrikeOut();
    void Read_BeginShadow();
    void Read_EndShadow();

        // Read-Methoden fuer die Fonts
    void Read_SetPitchAndOrFont();
    void Read_FontDefTable();

        // Read-Formatierungs Methoden
    void Read_SetFormLenght();
    void Read_SetPageNumber();
    void Read_PageWidth();
    void Read_ReSetLeftRightMargin();
    void Read_SetTopMarginOfPage();			// STP
    void Read_SetBottomMarginOfPage();		// SBP
    void Read_HeaderTopMargin();			// HTM
    void Read_HeadingMargin();				// HM
    void Read_FooterBottomMargin();			// FBM
    void Read_FooterMargin();				// FM
    void Read_SetTempLeftRightMargin();
    void Read_FooterStart();
    void Read_HeaderStart();
    void Read_HeadFootEnd();
    void Read_PrintPageNo();
    void Read_PrintLastPageNo();
    void Read_Separator();
    void Read_SetLeftMarginRelease();
    void Read_FootNoteStart1();
    void Read_FootNoteStart2();
    void Read_FootNoteEnd();
    void Read_FootNoteInfo();
    void Read_EndSection();
    void Read_ColumnsDefinition();
    void Read_BeginColumnMode();
    void Read_EndColumnMode();
    void Read_BeginTabCell();
    void Read_BeginTabRow();
    void Read_ColumnBreak();
    void Read_ParaBorder();
    void Read_BeginAbsPosObj();
    void Read_EndAbsPosObj();
    void Read_Picture();
    void Read_PictureDef();
    void Read_IncludeGraphic();
    void Read_SetSpaceAfter();
    void Read_SetSpaceBefore();
    void Read_BeginMarkedText();
    void Read_EndMarkedText();
    void Read_HardHyphen();
    void Read_SoftHyphen();
    void Read_PageOrient();
    void Read_SetTray();
    void Read_Hex();
    void Read_BeginSmallCaps();
    void Read_EndSmallCaps();
    void Read_BeginUpperCaps();
    void Read_EndUpperCaps();
    void Read_SetRightMarginIndent();
    void Read_ReSetLineSpacing();
    void Read_StyleBasedOn();
    void Read_Split();
    void Read_NonBreakSpace();
    void Read_WidowOrphOn();
    void Read_WidowOrphOff();
    void Read_BeginKerning();
    void Read_EndKerning();
    void Read_BeginHyphen();
    void Read_EndHyphen();
    void Read_LangGroup();
    void Read_SysInfo();
    void Read_Ruler();
    void Read_SetUnderlineMode();

    void Read_BeginHiddenText();
    void Read_EndHiddenText();
    void Read_CommentLine();

        // Read-Methoden fuer nummerierte Listen
    void Read_ParagraphNumber();
    void Read_EndOfParaNum();
    void Read_ParaNumberDef();

        // besondere Methoden fuer FrameMaker - Dokumente !!
    BOOL Read_Analyze_FLO_PDT();
    void Read_PageDefinitionTable();
    void Read_RectangleDefinitionTable();
    void Read_Flow();
    void Read_BeginTextFlow();
    void Read_EndTextFlow();
    void Read_TextRectangelId();
};

typedef void (SwW4WParser:: *FNReadRecord)();

struct W4WRecord
{
    W4WRecType   aRecType;	// der ID-String fuer den Record
    FNReadRecord fnReadRec;	// Methode fuer diesen spez. Record
};


ULONG GetW4WIniFlags( USHORT nFilterNo );	// auch fuer Writer

struct W4WStyleIdTabEntry
{
    SwTxtFmtColl* pColl;		// Pointer auf Writer-Collection, sobald
                                // angelegt. Beim Suchen 0, bis gefunden.
    USHORT	nStyleId;			// W4W-Style-Id, nicht Writer-Id
    BOOL	bSetAttributes;		// erlaubt, dass Attribute gesetzt werden duerfen

                // Bei Base-Tab (Style based on) ist alles etwas anders:
                // pColl ist die Vorlage (Collection), die noch einen
                // Verweis (SBO) nachgereicht bekommen muss.
                // nStyleId ist die W4W-Style-Id, auf die verwiesen werden
                // muss, sobald es diese gibt.


    W4WStyleIdTabEntry( SwW4WParser& rParser, USHORT nId,
                        const sal_Unicode* pName );
        // ctor fuers suchen !!
    W4WStyleIdTabEntry( USHORT nSeekId ) :	nStyleId( nSeekId ), pColl(0),
                                            bSetAttributes( TRUE ) {}

    inline BOOL operator==( const W4WStyleIdTabEntry&) const;
    inline BOOL operator<( const W4WStyleIdTabEntry&) const;
};
typedef W4WStyleIdTabEntry* W4WStyleIdTabEntryPtr;



} //namespace binfilter
#endif
