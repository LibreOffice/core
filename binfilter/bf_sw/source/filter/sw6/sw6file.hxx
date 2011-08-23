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
#ifndef _SW6FILE_HXX
#define _SW6FILE_HXX

#include <stdlib.h>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include "swtypes.hxx"
#include "swerror.h"
namespace binfilter {

class SwCharFmt;
class SwTxtFmtColl;


#define TWIPINCH 1440   // Wieviele Twips per Inch

#define NVERSION '6'    // Neue Versionsnummer
#define AVERSION '5'    // Alte Versionsnummer

#define KFEND       "\7"
#define NOTENEND    "\14"
#define MLDBOOJAJA  "JA"
#define MLDBOONEIN  "NEIN"
#define DOKUKENN    ".\\\\\\ WRITER # \\\\\\"
#define DOKUKENNE   ".\\\\\\ WRITER #E \\\\\\"
#define LAYOUTKENN  ".\\\\\\ LAYOUT # \\\\\\"
#define LAYOUTKENNE ".\\\\\\ LAYOUT #E \\\\\\"

#define MAXALAY  64     // Maximalanzahl Absatzlays
#define MAXSLAY  32     // Maximalanzahl Seitenlays
#define MAXBLAY  16     // Maximalanzahl Fontuebersetzungen
#define MAXTAB   32     // Maximale Anzahl Tabulatoren
#define MAXGLIED 5      // Wieviele Gliederungsebenen

// Art der Nummerierung einer automatisch zu erzeugenden
// Zahl wie z.B. Seitenummer, Gliedernummer etc.

enum NUMMERIERNG {
    Arabisch,Roemisch,Grossbuch,Kleinbuch,Bullets
};

// Alle Steuerzeichen die vorkommen

enum CTRLS {
    PrGraf,             // ^@ Praefix fuer Grafiksteuerzeichen
    InhKenn,            // ^A Zeichen fuer Inhaltsvz.-Kennzeichnung, ingnore
    IdxKenn,            // ^B Zeichen fuer Indexvz.-Kennzeichnung, ignore
    PrtPause,           // ^C Zeichen fuer Druckunterbrechung, ignore
    PrSoftSp,           // ^D Zeichen Praefix Softspace
    SoftSp,             // ^E Soft-Leerzeichen, ' '
    HardSp,             // ^F Hartspace (wird nicht umbrochen) ,' '
    NewCol,             // ^G Neue Spalte, ignore
    BackSpc,            // ^H Backspace-Steuerzeichen
    TabZch,             // ^I Tabulatorzeichen, Pfeil
    NewLine,            // ^J Neue Zeile
    SoftK,              // ^K Zeichen fuer k-c-Austausch bei Trennung, 'k'
    NewPage,            // ^L Neue Seite, ignore
    NewPara,            // ^M Neuer Absatz
    PrSForm,            // ^N Praefix harte Formatierung Seite
    PrAForm,            // ^O Praefix harte Formatierung Absatz
    HTrenn,             // ^P Hartes Trennzeichen (wird nicht umbrochen), '-'
    PrMerker,           // ^Q Versteckter Text fuer Merker
    PrPrFix,            // ^R Praefix-Praefix fuer spaetere Erweiterungen
    SoftAdd,            // ^S Zusatz-Zeichen Trennung von "Schiff-fahrt"
    PrKomm,             // ^T Praefix Textkommando
    ParaGraf,           // ^U Zeichen welches fuer Paragraf-Zeichen
    PrGlied,            // ^V Praefix fuer Gliederungsnummer
    PrHidd,             // ^W Praefix fuer Hidden Text
    PrAttr,             // ^X Praefix Attribut
    PrSize,             // ^Y Praefix Fontsize
    NewText,            // ^Z Textende
    PrUser,             // ^[ Praefix fuer User-Controlzeichen
    PrFnote,            // ^\ Praefix fuer Fussnotenkennzeichnung
    PrEnote,            // ^] Praefix fuer Endnotenkennzeichnung
    PrFont,             // ^^ Praefix Font
    STrenn              // ^_ Weiches Trennzeichen, '-' nur Zeilenende
};

// Fuers harte Seitenformat, 2. Steuerzeichen nach PrSForm

enum AFORMTYP {
    AF_Res,             // Nichts
    AF_Ein,             // Einrueckung Twips
    AF_Lks,             // Linker Rand Twips
    AF_Rts,             // Rechter Rand Twips
    AF_AbAb,            // Absatzabstand Twips
    AF_Form,            // Absatzformat Enum
    AF_SpAb,            // Spaltenabstand Twips
    AF_SpAn,            // Spaltenanzahl Byte
    AF_SAbs,            // Spaltenabsatz Bool
    AF_Auto,            // Durchschuss Bool
    AF_ZlAb,            // Zeilenabstand Twips
    AF_Glied,           // Gliederungslay Enum
    AF_Tab,             // Tabulator alt Struct
    AF_TabN,            // Tabulator neu Struct
    AF_Zus              // Zusammenhalten
};

// Fuers harte Absatzformat, 2. Steuerzeichen nach PrAForm

enum SFORMTYP {
    SF_Res,SF_KoAb,SF_FuAb,SF_Lks,SF_Rts,SF_Obn,SF_Unt,
    SF_SpAb,SF_Guel,SF_SpAn,SF_KoTx,SF_FuTx
};

// Art den Rahmen in Layouts oder harter Formatierung
// Dies sind eigentlich Bitflags mit (1 << Frm_xxx)

enum FRMARTEN {
    Frm_Lks,Frm_Rts,Frm_Obn,Frm_Unt,Frm_Frm,MaxFRMARTEN
};

// Bitflags Grafikrand

struct GRAFFRAM {
    int NoFrame:1;      // Kein Rahmen
    int Einfach:1;      // Einfacher Rahmen
    int Doppelt:1;      // Doppelter Rahmen
};

// Bitflags Grafik

struct GRAFFLAG {
    int DoppelDruck:1;  // Doppelt drucken
    int InversDruck:1;  // Invers drucken
    int KeineGraust:1;  // Keine Graustufen
    int Kompatibel :1;  // Kompatibel (siehe GRAFDESC)
    int SGFHeller  :1;  // Grafik aufhellen (f. Laserdrucker)
    int SGFBlkWhite:1;  // Schwarz/Weiss erzwingen
    int SGFRawform :1;  // Wenn Grafik mit 256 Farben
    int SGFFormel  :1;  // Wenn es eine Formel ist
};

// Ausrichtung einer Grafik

enum GBUNDTYP {
    G_Nicht,G_Links,G_Rechts,G_Zent
};

// Format einer Grafik (Je nach Treiber)

enum GRAFSTAT {
    NoGraf,Pic,Pcx,Hpgl,Img,Msp,Tiff,Dxf,Lot,Usr,Sgf
};

// Writer Grafikdescribtor, steht direkt in der Textzeile
// Aus Kompatibilitaetsgruenden wird die Liniendicke nicht im GrafDesc
// aufgefuehrt. Wenn Bit3(Flags)=1 dann steht die Liniendicke hinter
// Name/UsrT als Wort im Grafdesc drin. LPos enthaelt evt. auch LWdt.

struct GRAFDESC {
    SwTwips  Hoch;              // Hoehe Grafik in Twips
    SwTwips  Breit;             // Breite Grafik in Twips
    SwTwips  Rand;              // Breite/Hoehe des Randes (zusaetzlich)
    BYTE  	 Rahm;              // 1=Kein Rahmen, 2=Einfach, 3=Doppelt
    GBUNDTYP Bund;              // Nichtbuendig, Links, Rechts, Zent
    GRAFSTAT Form;              // Datenformat, welcher Treiber
    BYTE    Flag;              // Bits:0=Dopp, 1=Invs, 2=NoGrau, Bit3=Kompatibel
    String   Name;              // Enthaelt den Grafiknamen
};

// Ein Bitarray fuer Attribute, von denen die wichtigsten
// mal festgelegt wurden. Sonstige Attributte gibts nicht

enum ATTRBITS {
    Nix0,Fett,Kursiv,Unter,DUnter,Durch,DDurch,
    Hoch,Tief,Nix1,Outl,Shad,OutShad,Invs,Hoch2,
    Hoch4,Gross2,Gross4,Nix2,Nix3,Nix4,FRot,
    FBlau,FMag,FGelb,FOrange,FGruen,FCyan,
    ATTRBITS_END
};

typedef long ATTRS;

// Schluesselbuchstaben und Anmerkung fuer Layouts

struct RKEY {
    String Key;                 // Tastenschluessel zum rufen
    String Remark;              // Anmerkungen des Benutzers
};

// Art der Tabs in Layouts oder harter Formatierung

enum TABARTEN {
    Tab_L,Tab_R,Tab_Z,Tab_D,Tab_LF,Tab_RF
};

// Ein Tabulator

struct TABU {
    TABARTEN TabArt;            // Art des Tabulators
    SwTwips  TabPos;            // Position Tab
    sal_Char TabZch;            // Fuellzeichen, #0 oder Spc fuer nix
};

// Ein Bereichslayout

struct BLAY:RKEY {
    SwCharFmt *pCharFmt;        // Zeiger auf Fertige FmtColl zum Uebergeben

    short BFNum;                // Nummer des Fonts
    short BFSiz;                // Hoehe des Fonts in Halbpunkt
    BOOL  bTran;                // Sollen Zeichen uebersetzt werden?
    ATTRS BAttr;                // Welches Attribut anschalten
};

// Art der Formatierung in Layouts oder harter Formatierung. Tab-
// satz ist eine Satzform die nie in einer Datei stehen sollte.

enum SATZTYP {
    BlockSatz,LinksSatz,RechtsSatz,ZentrSatz,
    AustrSatz,UnformSatz,SperrSatz,TabSatz
};

// Ein Absatzlayout

struct ALAY:RKEY {
    SwTxtFmtColl *pFmtColl;     // Zeiger auf Fertige FmtColl zum Uebergeben

    short   ABlay;              // Welche Fontuebersetz, 0=Hart
    short   AFNum;              // Nummer des Fonts
    short   AFSiz;              // Hoehe des Fonts in Halbpunkt
    BOOL    bTran;              // Sollen Zeichen uebersetzt werden?
    ATTRS   AAttr;              // Welches  Attribut anschalten
    SwTwips Einzug;             // Absatzeinzug in  Spalten
    SwTwips LRand;              // Einrueckung lks in Spalten
    SwTwips RRand;              // Einrueckung rts in Spalten
    SwTwips ORand;              // Abstand  vom Vorherg. Abs.
    SwTwips SpaltAbst;          // Abstand  zw. den Spalten
    BOOL    SpTrenn;            // Spaltentrenn
    short   SpAnzahl;           // Wenn SpAbsatz>Spaltenzahl
    BOOL    SpAbsatz;           // Ist  Spaltenabsatz?
    BOOL    AutoZlAb;           // Durchschuss oder  Zlabstand
    short   ZlAbstand;          // Zeilenabstand in Pt
    short   GliedLay;           // 0=Nein,  sonst Gliederebene
    SATZTYP FormatFlg;          // Format:  Block,Rts/Lks,Zen,Unf.
    SwTwips FrmAbst;            // Abstand  der Linien vom Absatz
    SwTwips FrmWdth;            // Strichstaerke Linien  vom Absatz
    short   FrmArt;             // Was  fuer Linien, BitArray
    short   LinLTyp;            // Gpm-Linienart
    short   LinRTyp;            // Gpm-Rasterart
    BOOL    Zusammen;           // Absatz zusammenhalten
    short   MaxTabs;            // Wieviel  gueltige Tabs
    TABU    Tabs[MAXTAB];       // Tabulatoren in Spalten
};

// Ein Seitenlayout

struct SLAY:RKEY {
    short   Follow;             // Merker fuer Follow-Layout
    short   Gueltig;            // 0=alle,1=ung.,2=gerade S.,3=1.Seite
    SwTwips KopfHoch;           // Wieviel Zeilenhoehe Kopftext
    SwTwips FussHoch;           // Wieviel Zeilenhoehe Fusstext
    SwTwips KopfAbst;           // Abstand  Kopftext
    SwTwips FussAbst;           // Abstand  Fusstext
    SwTwips LRand;              // Textrand links
    SwTwips RRand;              // Textrand rechts
    SwTwips ORand;              // Textrand oben
    SwTwips URand;              // Textrand unten
    SwTwips SpaltAbst;          // Abstand  zw. den Spalten
    BOOL    SpTrenn;            // Spaltentrenner
    short   SpAnzahl;           // Spaltenzahl  Slayout
    short   KopfTNum;           // Nummer harter KopfText,  0=Weicher
    short   FussTNum;           // Nummer harter Fusstext, 0=Weicher
    long    FussText;           // Weicher Kopftext
    long    KopfText;           // Weicher Kopftext
};

struct KOFU:RKEY {
    long   KFText;              // Dateipos fuer Kopf oder Fusstexte
};

// Describtor eines Gesamt Layouts des Writers V5.0-V6.0

struct LAYDESC {
    BOOL        FNotenFlg;             // True=Dokumentweise
    BOOL        FNotenLin;             // Soll Fussnotenstrich?
    BOOL        ANotenFlg;             // Sollen Anmerkungen gedruckt?
    SATZTYP     FNotSForm;             // Notenstrich (1=Lks,2=Rts,3=Zent)
    short       FNotSPerc;             // Notenstrich-Prozente
    SwTwips     FNotURand;             // Unterer Rand Fussnoten
    short       NumAlay;               // Wieviele Alays?
    short       NumSlay;               // Wieviele Slays?
    short       NumBlay;               // Wieviele Fontuebersetzungen
    short       NumKopf;               // Wieviele Kopftexte
    short       NumFuss;               // Wieviele Fusstexte
    long        LastLayTm;             // Vergleichsdatum Lay-Datei, 0=Kein Datum
    long        FNotenLink;            // Zeiger auf Fussnotenzeilen
    long        ANotenLink;            // Zeiger auf Anmerkungen
    long        ENotenLink;            // Zeiger auf Endnotenzeilen
    SwTwips     SpTrennWd;             // Strichstaerke Spaltentrenner Twips
    SwTwips     PapierLen;             // . Papierlaenge
    SwTwips     PapierBrt;             // . Papierbreite
    NUMMERIERNG FNotenForm;            // Wie FNotennummern?
    NUMMERIERNG ENotenForm;            // Wie ENotennummern?
    NUMMERIERNG SeitenForm;            // Wie Seitennummern?
    NUMMERIERNG GliedForm[MAXGLIED];   // Wie Gliedernummern?
    ALAY        *FileAlay[MAXALAY];    // Alle Alays eines Textes
    SLAY        *FileSlay[MAXSLAY];    // Alle Slays eines Textes
    BLAY        *FileBlay[MAXBLAY];    // Alle Fontuebersetzungen
    KOFU        *HartKopf[MAXSLAY];    // Halter fuer den Kopftext
    KOFU        *HartFuss[MAXSLAY];    // Halter fuer den Fusstext

    void DelLayout(BOOL GLay,BOOL Kopf,BOOL Fuss);

    LAYDESC(void):NumAlay(0),NumSlay(0),
      NumBlay(0),NumKopf(0),NumFuss(0) {}
   ~LAYDESC(void) {DelLayout(TRUE,TRUE,TRUE);}
};

class Sw6Stack
{

private:

    struct DATA {
        DATA   *pNext;
        size_t  nSize;
    } *pData;
    void Clr(void);

public:

         Sw6Stack(void):pData(NULL){};
        ~Sw6Stack(void){Clear();};
    BOOL Push(void *pSrce,size_t nMaxSize);
    BOOL Get(void *pDest,size_t nMaxSize,size_t *pnSize=NULL);
    BOOL Pop(void *pDest,size_t nMaxSize,size_t *pnSize=NULL);
    void Clear(void);
};

class Sw6File
{

private:

    struct STAT
    {
        BOOL bEof;                     // TRUE wenn CtrlZ gelesen...
        BOOL bErr;                     // Errorstatus ausgegeben?
        BOOL bTrs;                     // Soll OEM->eDfltCharSet?
    } aSta;

    struct BUFFR
    {
        size_t nMax;                   // Wieviele Byte im Puffer gltig
        size_t nSiz;                   // Gr”áe des Puffers in pBuf
        size_t nIdx;                   // Leseindex, wandert bis nMax
        sal_Char  *pBuf;                   // Zeiger auf Eingabepuffer
        BUFFR(void):nMax(0),nSiz(0),nIdx(0),pBuf(NULL){};
       ~BUFFR(void){DELETEZ(pBuf);};
    };

    BUFFR    aBuf;                     // Buffer fuer Eingabe
    BUFFR    aPwd;                     // 2. Buffer wenn Passwort aktiv

    size_t   nCtr;                     // Bei Ctrls: Max Anzahl der Zchn
    SvStream *pInp;                     // Eingabesteam schon geoeffnet //$ istream
    Sw6Stack aStk;                     // Fuer PushStat und PopStat

    short    nLev;                     // Zur Begrenzung des Stacks
    BYTE     nBit;                     // 8 Bits fuer ReadBit
    BYTE     nOvl;                     // Overflow des Stacks?

    struct NODE
    {
        NODE *pLks;                    // Linker Zweig
        NODE *pRts;                    // Rechter Zweig
        BOOL  bAst;                    // Ast oder Blatt?
        BYTE  cZch;                    // Falls !bAst dann Zeicheninhalt
    } aNode;

    BOOL FlushPhys(short nAnz,BUFFR &rBuf);
    BOOL Flush(short nAnz);            // Sorgt f. nAnz chars bei pBuf[nIdx]

    BYTE  ReadBit(BUFFR &rBuf);        // Lese ein Bit ein
    void  ReadTree(NODE *pNode);       // Lese den Huffman-Baum
    void  DelTree(NODE **ppNode);      // L”schen des Baums

protected:

    BOOL ActivatePass(void);

    ULONG *pErr;                     // Zeiger auf Errornummer
    // virtual muss nicht mehr sein, es gibt nur noch den Parser!
    virtual size_t PutRest(String &rStr,sal_Char  *pCtrl) = 0;
    /*virtual*/ void PushStat(long nFPos); // Merke des Status
    /*virtual*/ void PopStat(void);        // Wiederherstellen Status

public:

    void SetTrans(BOOL bOn){aSta.bTrs=bOn;} // OEM-Translation Ein/Aus
    BOOL IsTrans() const { return aSta.bTrs;} // OEM-Translation Ein/Aus
    BOOL Eof(void) const;              // Gibt TRUE zurck wenn End of File
    long Tell(void) const;             // Gibt die Fileposition zurck
    void Seek(const long lWhere);      // Sucht zu einer Fileposition
    BOOL TestCtrl(sal_Char cZch);          // Spezial:Test auf Ende Extratext
    BOOL FileOk(void)                  // FALSE und 1*Error() wenn Fehler
    {
        if (!aSta.bErr && pInp->GetError() != SVSTREAM_OK)
        {
            *pErr = ERR_SW6_INPUT_FILE;
            aSta.bErr=TRUE;
        }
        return !aSta.bErr;
    }
    BOOL ReadLn(String &rStr);         // Lese eine PC-Zeile ein (CR/LF)

    Sw6File(                         // ctor
        SvStream &rInp,              // Input-Stream, mu?offen sein //$ istream
        size_t nBufSize,             // Lesepuffer-Groesse, mu?>0 sein
        size_t nCtrSize,             // Maximale L„nge Ctrl-Zeichenkette
        ULONG *pErrno );             // Optionale Fehlernummer
   ~Sw6File(void);                   // dtor
};


} //namespace binfilter
#endif // _SW6FILE_HXX
