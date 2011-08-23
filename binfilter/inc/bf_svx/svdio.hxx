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

#ifndef _SVDIO_HXX
#define _SVDIO_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
class SvStream;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

const char SdrIOMagic [2]={'D','r'};
const char SdrIOModlID[2]={'M','d'}; // Model
const char SdrIOItPlID[2]={'I','P'}; // ItemPool
const char SdrIOLayrID[2]={'L','y'}; // Layer
const char SdrIOLSetID[2]={'L','S'}; // Layerset
const char SdrIOPageID[2]={'P','g'}; // Page
const char SdrIOMaPgID[2]={'M','P'}; // MasterPage (Stammseite)
const char SdrIOMPgDID[2]={'M','D'}; // MasterPageDescriptor
const char SdrIOMPDLID[2]={'M','L'}; // MasterPageDescriptorList
const char SdrIOHlpLID[2]={'H','l'}; // HelpLine
const char SdrIOHLstID[2]={'H','L'}; // HelpLineList
const char SdrIODObjID[2]={'O','b'}; // Object
const char SdrIOConnID[2]={'C','n'}; // ObjConnection (EdgeObj)
const char SdrIOViewID[2]={'V','w'}; // View
const char SdrIOPgVwID[2]={'P','V'}; // PageView
const char SdrIOEndeID[2]={'X','X'}; // Ende
const char SdrIOJoeMagic[4]={'J','o','e','M'}; // Um meine eigenen SubRecords zu erkennen

const int nAktSdrFileMajorVersion =  0;

// #84529# increase fileversion to be able to correct gradient rotation for 5.2 and earlierl
const int nAktSdrFileMinorVersion = 17; // = 16;

const int nAktSdrFileVersion = (nAktSdrFileMajorVersion <<8) + (nAktSdrFileMinorVersion);
const int nSdrFileVersion31 = 12; // Fileversion des Office 3.1
const int nSdrFileVersion40 = 13; // Fileversion des Office 4.0
const int nSdrFileVersion50 = 14; // Fileversion des Office 5.0
const int nSdrFileVersion51 = 15; // Fileversion des Office 5.1

// Fileversionen:
//  0= ??-04-1995  Initialversion.
//     24-04-1995  Offizielles Office 3.0 Beta I Shipping
//  1= 25-04-1995  Zus. Flag fuer Layer: Standardlayer (JOE)
//     26-04-1995  AttrObj: Beim Stylesheet-Namen wird zusaetzlich der CharSet gemerkt (SB)
//     28-04-1995  Neu: MasterPageDescriptoren (VisibleLayers f. Masterpages) (JOE)
//  2= 18-05-1995  Drehwinkel und Shearwinkel am Gruppenobjekt (JOE)
//     20-05-1995  EdgeObj nun von TextObj statt von PathObj abgeleitet (JOE)
//  3= 23-05-1995  Textrahmen werden nun durch RectObj dargestellt (JOE)
//  4= 31-05-1995  Weiteres persistentes Flag am Zeichenobjekt bNotVisibleAsMaster (JOE)
//     12-06-1995  Offizielles Office 3.0 Beta II Shipping
//     13-06-1996  bTextToContour raus und durch Item ersetzt (JOE)
//     14-06-1996  SdrItemPool muss nun ohne RefCounts geladen werden (JOE)
//  5= 14-06-1995  SdrOutlinerSetItem als neuer Member im SdrAttrObj (JOE)
//  6= 28-06-1995  Neue Items: pMiscAttr; -> Eckenradius-Member entfaellt am RectObj (JOE)
//  6= 30-06-1995  Neuer Member am SdrGrafObj: aCropRect (JOE)
//  6= 07-07-1995  Neue Items fuer Fontwork am XOutDev (ESO)
//     10-07-1995  Office 3.0 Beta III Shipping an IBM
//  7= 18-07-1995  PolyObj raus, Import am PathObj
//  8= 25-07-1995  GrafObj: zus. Member: aFileName fuer FileLinks
//  9= 02-08-1995  GrafObj: zus. Member: aFilterName fuer FileLinks
//     14-10-1995  Office 3.0 PreRelease fuer Systems Muenchen
//     21-10-1995  Final Office 3.0 Shipping
// 10= 25-10-1995  *pFormTextBoundRect wird gestreamt; Neue Attribute: TextVAdjust, Textabstaende
//     09-11-1995  entgueltiges Format V10 durch erneute Poolumstellung (ExtraPool raus)
// 11= 10-11-1995  - Nicht rueckwaertskompatibel!
//                 - Einfuehrung von Major/MinorVers.
//                 - Aufraeumer: Datensack am Obj f. Konnektoren,...
//                 - bMirrored und aName fuer GrafikObjekt
//                 - DownCompat fuer Grosses, Komplexes und Unbekanntes:
//                   - SdrObject:
//                     - Konnektoren (Flag davor)
//                     - UserDataList (Flag davor)
//                       - UserData
//                   - SdrTextObj:
//                     - OutlinerParaObject (Flag war schon davor)
//                   - SdrPathObj:
//                     - PathPolygon
//                   - SdrEdgeObj:
//                     - EdgeTrack
//                     - Connections haben nun einen eigenen IOHeader
//                   - SdrGrafObj:
//                     - Graphic (Flag davor)
//                     - zus. Flag fuer HasGraphicLink
//                   - SdrOle2Obj:
//                     - Graphic (Flag war schon da)
//                   - SdrModel:
//                     - DocInfo
//                     - DocStatistic (Platz reserviert)
//                     - Misc eingepackt
//                   - SdrPage:
//                     - Misc eingepackt
//                 - Kennung fuer SubRecord meines Model und meiner Page (JoeMagic)
//                 - SvdAttrObj: CharSet fuer StyleSheet entfaellt
//                 - CharSet weg bei SdrModel und SdrAttrObj
//                 - WhichId's weg bei SdrAttrObj und SdrCaptionObj
// 12= 05-12-1995  Neues Item: SdrTextHorzAdjustItem. Textverankerung an Textrahmen und
//                 Grafikobjekten. Beim Import von <V12 werden Grafikobjekte hart attributiert.
//     12-02-1996  UIScale und UIUnit wird vom Model nun rausgeschrieben
//     16-02-1996  Shipping des Office 3.00.2 (SV304a)
//     29-02-1996  Pool V3: 17 neue Items im XOut von KA fuer Bitmapfuellungen
//     13-03-1996  Version fuer CeBit 96
//  ca 15-04-1996  Shipping Version 3.1
//     10-08-1996  Pool Version 4; eigene Items am Verbinder
//     28-08-1996  CeBIT Home
//     21-01-1997  EdgeObj: EdgeInfo und Attribute
// 13= 05-02-1997  Fuer Franz's Dateiformatsumstellungen an der 3D-Engine
// 14= 13-03-1998  Fuer FmFormPage (FormLayer)
// 15= 19-01-1999  Fuer StarDraw-Preview
// 16= 25-08-1999  SdrPage background object


class SdrIOHeader 
{
protected:
    // 10 Bytes persistent:
    char						cMagic[2];
    char						cHdrID[2];

    // Versionsnummer des Blocks
    UINT16						nVersion;    

    // Gesamtgroesse des Blocks inkl. Header und verschachtelten Bloecken
    UINT32						nBlkSize;    

    // ab hier Verwaltungsinformationen:
    SvStream&					rStream;
    UINT32						nFilePos;
    UINT16						nMode;
    BOOL						bOpen;
    BOOL						bClosed;
    // Header wird nur zum nachschauen verwendet.
    BOOL						bLookAhead; 

    virtual void Write();
    virtual void Read();

#if _SOLAR__PRIVATE
#ifdef DBG_UTIL
    void ImpCheckRecordIntegrity();
    void ImpGetRecordName(ByteString& rStr, INT32 nSubRecCount=-1, INT32 nSubRecReadCount=-1) const;
    BOOL ImpHasSubRecords() const;
#endif
#endif // __PRIVATE

public:
    SdrIOHeader(SvStream& rNewStream, UINT16 nNewMode, const char cID[2]=SdrIOEndeID, BOOL bAutoOpen=TRUE);
    virtual ~SdrIOHeader();

    BOOL IsMagic() const { return cMagic[0] == SdrIOMagic[0] && cMagic[1] == SdrIOMagic[1];  }
    void SetMagic() { cMagic[0] = SdrIOMagic[0]; cMagic[1] = SdrIOMagic[1]; }
    BOOL IsEnde() const { return cHdrID[0] == SdrIOEndeID[0] && cHdrID[1] == SdrIOEndeID[1]; }
    void SetEnde() { cHdrID[0] = SdrIOEndeID[0]; cHdrID[1] = SdrIOEndeID[1]; nBlkSize = 0; SetMagic(); }
    BOOL IsID(const char cID[2]) const { return cHdrID[0] == cID[0] && cHdrID[1] == cID[1]; }
    void SetID(const char cID[2]) { cHdrID[0] = cID[0]; cHdrID[1] = cID[1]; SetMagic(); }
    UINT16 GetVersion() const { return nVersion; }
    UINT16 GetMinorVersion() const { return nVersion | 0xFF; }
    UINT16 GetMajorVersion() const { return nVersion >>8; }
    UINT32 GetFilePos() const { return nFilePos; }
    UINT32 GetBlockSize() const { return nBlkSize; }
    void OpenRecord();
    void CloseRecord();
    UINT32 GetBytesLeft() const;
};

class SdrIOHeaderLookAhead: public SdrIOHeader 
{
public:
    SdrIOHeaderLookAhead(SvStream& rNewStream, BOOL bAutoOpen = TRUE);
    void SkipRecord() const { rStream.Seek(nFilePos+nBlkSize); }
};

/*
I/O-Format (12+n Bytes)
 4 Inventor
 2 Version
 2 Identifier
 4 Menge der nachfolgenden Daten in Bytes
-------
 2 Layer
16 Allumschliessendes Rechteck
 n>=18 Objektspezifische Daten
*/
//const SdrObjIOHeaderSize=6;

class SdrObject;

class SdrObjIOHeader: public SdrIOHeader 
{
public:
    // ab hier persistente Daten
    UINT32						nInventor;
    UINT16						nIdentifier;

    // ab hier Verwaltungsinformationen
    const SdrObject*			pObj;

protected:
    virtual void Write();
    virtual void Read();

public:
    SdrObjIOHeader(SvStream& rNewStream, UINT16 nNewMode, const SdrObject* pNewObj = NULL, BOOL bAutoOpen = TRUE);

    // DTor ueberladen, den in Dtor von SdrIOHeader lebt der SdrObjIOJHeader nicht mehr.
    virtual ~SdrObjIOHeader();
};

class SdrObjIOHeaderLookAhead: public SdrObjIOHeader 
{
public:
    SdrObjIOHeaderLookAhead(SvStream& rNewStream, BOOL bAutoOpen = TRUE);
    void SkipRecord() const { rStream.Seek(nFilePos+nBlkSize); }
};

// Klasse fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
// In der Regel reicht es aus, diese Klasse am Anfang des Lese/Schreibfokus
// auf dem Stack mit bAutoOpen=TRUE (default) zu instanziieren. Der Rest
// geschieht von selbst. Will man am Ende des Lesens wissen, ob noch weitere
// Daten im Sub-Record enthalten sind, ruft man GetBytesLeft().
// Den SubRecord schliesst man entweder mit CloseSubRecord() oder man laesst
// ihn vom Destruktor automatisch schliessen.
// Dieses Konzept setzt jedoch eine feste Reihenfolge und Anzahl der SubRecords
// voraus.
class SdrDownCompat 
{
protected:
    SvStream&					rStream;
    UINT32						nSubRecSiz;
    UINT32						nSubRecPos;
    UINT16						nMode;
    BOOL						bOpen;
    BOOL						bClosed;

    // nur fuer Debug
    const char*					pRecId; 

protected:
    virtual void Read();
    virtual void Write();

public:
    // Als Modus muss STREAM_READ oder STREAM_WRITE angegeben werden.
    SdrDownCompat(SvStream& rNewStream, UINT16 nNewMode, BOOL bAutoOpen = TRUE);
    ~SdrDownCompat();

#ifdef DBG_UTIL
    // fuer etwas aussagekraeftigere Assertions
    void SetID(const char* pId) { pRecId=pId; }
#endif

    // Mit GetBytesLeft() kann man abfragen, ob und wieviele Bytes noch
    // im SubRecord stehen. Nur gueltig bei STREAM_READ, sonst 0.
    // Nach CloseRecord() liefert diese Methode auch im Modus STREAM_READ 0.
    // (Denn dann sind die restlichen Bytes bereits uebersprungen.)
    UINT32 GetBytesLeft() const;
    BOOL IsOpen() const { return bOpen; }

    // Die Pos ist erst gueltig nach Open bzw nach CTor mit bAutoOpen=TRUE.
    UINT32 GetSubRecordPos() const { return nSubRecPos; }

    // Erst gueltig nach Open bzw. nach CTor mit bAutoOpen=TRUE.
    // Beim Modus STREAM_WRITE ist die Size erst nach Close() gueltig.
    UINT32 GetSubRecordSize() const { return nSubRecSiz; }
    void  OpenSubRecord();
    void  CloseSubRecord();
};

// Identifier fuer SdrNamedSubRecord bei Inventor==SdrInventor
enum {SDRIORECNAME_VIEWPAGEVIEWS   =0x0000,  // alle Pageviews, angezeigte und nicht angezeigte
      SDRIORECNAME_VIEWVISIELEM    =0x0001,  // sichtbare Elemente (Seite, Rand, Gitter, Hilfslinien,...)

      SDRIORECNAME_VIEWSNAP        =0x0010,  // Fangeinstellungen
      SDRIORECNAME_VIEWORTHO       =0x0011,  // Einstellungen fuer Ortho

      SDRIORECNAME_VIEWDRAGMODE    =0x0020,  // Move oder Rotate/Slant/..., bEditMode, RefPoints, FramDragSingles, PlusAlways
      SDRIORECNAME_VIEWMARKEDOBJ   =0x0021,  // (ni) markierte Objekte
      SDRIORECNAME_VIEWMARKEDPNT   =0x0022,  // (ni) markierte Punkte
      SDRIORECNAME_VIEWCROOKCENTER =0x0023,  // Kreismittelpunkt des letzten Crook
      SDRIORECNAME_VIEWMARKEDGLUE  =0x0024,  // (ni) markierte Klebepunkte

      SDRIORECNAME_VIEWDRAGSTRIPES =0x0030,  // Animierte Begrenzungslinien waerend des Draeggens
      SDRIORECNAME_VIEWDRAGHIDEHDL =0x0031,  // Handles waerend des Draeggens verstecken
      SDRIORECNAME_VIEWDRAGEDGES   =0x0032,  // (ni) Einstellungen fuer EdgeDragging
      SDRIORECNAME_VIEWOBJHITMOVES =0x0033,  // Bei MarkedHit Move oder DragMode
      SDRIORECNAME_VIEWMIRRDRAGOBJ =0x0034,  // Obj beim schieben der Spiegelachse zeigen

      SDRIORECNAME_VIEWCROBJECT    =0x0040,  // Aktuelles CreateObj
      SDRIORECNAME_VIEWAKTLAYER    =0x0041,  // Layer fuer Create und Paste (nun in der PaintView)
      SDRIORECNAME_VIEWCRATTRIBUTES=0x0042,  // (ni) Attribute fuer Create
      SDRIORECNAME_VIEWCRSTYLESHEET=0x0043,  // (ni) StyleSheet fuer Create
      SDRIORECNAME_VIEWCRFLAGS     =0x0044,  // Misc Flags fuer Create

      SDRIORECNAME_VIEWTEXTEDIT    =0x0050,  // Div. Einstellungen fuer TextEdit
      SDRIORECNAME_VIEWMACRO       =0x0051,  // Div. Einstellungen fuer Macro

      SDRIORECNAME_VIEWCHARSET     =0x0080,  // Hier merkt sich die SdrView den CharSet und setzt diesem beim Read am Stream

      SDRIORECNAME_PAGVIEW         =0x0100,  // Pageview sichtbar, Seitennummer, Position, Ursprung
      SDRIORECNAME_PAGVLAYER       =0x0101,  // angezeigte, gesperrte und druckbare Layer
      SDRIORECNAME_PAGVHELPLINES   =0x0102,  // Hilfslinien und -punkte
      SDRIORECNAME_PAGVAKTGROUP    =0x0103,  // (ni) aktuelle Gruppe

      SDRIORECNAME_MARK            =0x0200,  // (ni)

      SDRIORECNAME_USERDEFINEDFIRST=0x7000,
      SDRIORECNAME_USERDEFINEDLAST =0x7FFF};

class SdrNamedSubRecord: public SdrDownCompat 
{
    UINT32						nInventor;
    UINT16						nIdentifier;

protected:
    virtual void Read();
    virtual void Write();

public:
    SdrNamedSubRecord(SvStream& rNewStream, UINT16 nNewMode,
        UINT32 nNewInventor = 0xFFFF, UINT16 nNewIdentifier = 0xFFFF,
        BOOL bAutoOpen = TRUE);
    UINT32 GetInventor() const { return nInventor; }
    UINT16 GetIdentifier() const { return nIdentifier; }
};

// Zum Speichern einer View habe ich ein etwas anderes Konzept gewaehlt, da
// sich das Dateiformat hier sicherlich haeufiger und aufwendiger aendert.
// Es werden eine Anzahl von SdrNamedSubRecords geschrieben, wobei jeder
// dieser Records bestimmte Teilinformationen enthaelt, z.B. die Menge der
// sichtbaren Layer einer Page oder den Default-Stylesheet fuer neu zu
// erzeugende Objekte, ... . Diese Konzept hat zwar einen groesseren Overhead
// an Verwaltung und Daten zur Folge, bietet dafuer jedoch eine sehr grosse
// Flexibilitaet im Punkte auf-/abwaertskompatibilitaet. Da die Menge der
// persistenten Daten einer View recht gering ist spielt der Overhead an
// Streamdaten kaum eine Rolle.
// Eine abgeleitete View ueberlaed die Methoden ReadRecord() (lesen eines
// einzelnen Records) und WriteRecords() (schreiben aller Records). Ein Muster
// ist in SdrSnpV.CXX zu finden.

// Folgende Methoden schauen nach, ob sich im Stream an der aktuellen Position
// ein SdrModel bzw. eine SdrView befindet. Hat der Stream einen Fehlerstatus
// oder Eof(), so wird FALSE returniert.

}//end of namespace binfilter
#endif //_SVDIO_HXX

/* /////////////////////////////////////////////////////////////////////////////////////////////////

IO-Datenstruktur:

?--------?    ?---?              ?---?
| SdrData | --->´Modl??---->----?>´Ende?-->
?--------?    ?---?  ?---?   ?---?
                       ?-´View?-?
                          ?---?

?-----?        ?------?  ?------? ?-------?               ?---?
| Modl |     --->´ModlHdr?->´ModlDat?>´ItemPool?>?---->----?>´Ende?-->
?-----?        ?------?  ?------? ?-------?   ?---?   ?---?
                                                    ?-´Layr?-?
                                                    |  ?---? |
                                                      ?---? 
                                                    ?-´LSet?-?
                                                    |  ?---? |
                                                      ?---? 
                                                    ?-´Page?-?
                                                       ?---?

?-------?      ?----------?  ?----------?
|ItemPool|   --->´ItemPoolHdr?->´SfxItemPool?-->
?-------?      ?----------?  ?----------?

?-----?        ?------?  ?------?               ?-------?
| Page |     --->´PageHdr?->´PageDat?>?---->----?>´DObjList?-->
?-----?        ?------?  ?------?   ?---?   ?-------?
                                        ?-´Layr?-?
                                        |  ?---? |
                                          ?---? 
                                        ?-´LSet?-?
                                           ?---?

?---------?                    ?---?
| DObjList | --->?----->-----?>´Ende?-->
?---------?      ?-----?   ?---?
                 ?-?DObj ?-?
                    ?-----?

?-----?        ?------?  ?------?
| DObj |     --->´DObjHdr?->´DObjDat?->?----->-------?-->
?-----?        ?------?  ?------?    ?-------? 
                                         ?>´DObjList?>?
                                            ?-------?
?-----?        ?------?  ?------?
| Layr |     --->´LayrHdr?->´LayrDat?-->
?-----?        ?------?  ?------?

?-----?        ?------?  ?------?
| LSet |     --->´LSetHdr?->´LSetDat?-->
?-----?        ?------?  ?------?

?-----?        ?------?  ?------?
| View |     --->´ViewHdr?->´ViewDat?-->
?-----?        ?------?  ?------?

///////////////////////////////////////////////////////////////////////////////////////////////// */

