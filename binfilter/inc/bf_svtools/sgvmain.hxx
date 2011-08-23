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

#ifndef _SGVMAIN_HXX
#define _SGVMAIN_HXX

#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

namespace binfilter
{


#define UCHAR unsigned char

struct PointType {
    INT16 x;
    INT16 y;
};

#define SgfDpmm 40

#define DtHdSize 256
class DtHdType {
public:
    BYTE  Reserved[256];
    friend SvStream& operator>>(SvStream& rIStream, DtHdType& rDtHd);
    friend void DtHdOverSeek(SvStream& rInp);
};


struct Seitenformat {
    PointType Size;       // 0.00mm...819.175mm (Papiergr��e)
    INT16     RandL;      // links     Rand auf
    INT16     RandR;      // rechts    dem Papier
    INT16     RandO;      // oben      Rand auf
    INT16     RandU;      // unten     dem Papier
    BYTE      PColor;     // Future Use
    BYTE      PIntens;    // erst recht Future use
//    BOOL      BorderClip; // Objekte am Rand abschneiden (Schummel wg. Allignment unter NT)
};


#define PageSize 146
class PageType {
public:
    UINT32       Next;       // N�chste Seite
    UINT32       nList;      // Objektdaten, erster Record
    UINT32       ListEnd;    // Objektdaten, letzter Record
    Seitenformat Paper;      // Papierdaten
    BOOL         BorderClip; // Objekte am Rand abschneiden (Schummel wg. Allignment unter NT)
    BYTE         StdPg;      // welche Standardseite einblenden ?
    PointType    U;          // Nullpunkt
    INT16        HlpLnH[20]; // Hilfslinien
    INT16        HlpLnV[20];
    BYTE         LnAnzH;
    BYTE         LnAnzV;
    UCHAR        PgName[32]; // Seitenname
    friend SvStream& operator>>(SvStream& rIStream, PageType& rPage);
};


enum ObjArtType {ObjStrk,ObjRect,ObjPoly,ObjCirc,ObjSpln,
                 ObjText,ObjGrup,ObjBmap,ObjVirt,ObjTxtX,ObjMaxi};

struct ObjLineType {
    BYTE  LFarbe;    //  [Index]
    BYTE  LBFarbe;   //  [Index]
    BYTE  LIntens;   //  [%]
    BYTE  LMuster;   //  [Index] inkl. Transparenz
    INT16 LMSize;    //  [Koeffizient/100]
    INT16 LDicke;    //  Strichst�rke
};

struct ObjAreaType {
    BYTE   FFarbe;   //  [Index]
    BYTE   FBFarbe;  //  [Index]
    BYTE   FIntens;  //  [%]
    BYTE   FDummy1;  //
    INT16  FDummy2;  //
    UINT16 FMuster;  //  [Index] inkl. Invers, Transparenz
};

#define ObjTextTypeSize 64
class ObjTextType {
public:
    ObjLineType  L;        // Text-Outline (future)
    ObjAreaType  F;        // Text innen
    UINT16       FontLo,FontHi;// z.B. 92500 (CG Times), zweigeteilt wegen DWordAllign in TextType.
    UINT16       Grad;     // 0.5..32767.5 Pt - bei 1000 Pt sollte aber schlu� sein
    UINT16       Breite;   // 1..65535%  bitte nicht mehr als 500%
    BYTE         Justify;  // 2 Bit Vert (Hi), 3 Bit Hor (Lo)
    BYTE         Kapit;    // 1..255%
    UINT16       Schnitt;  // 8 Flags
    UINT16       LnFeed;   // 1..32767% vom max. Schriftgrad der Zeile
    UINT16       Slant;    // Kursivwinkel 0.00..89.99� default 15.00�          doppelt Breit angesehen)
    BYTE         ZAbst;    // Zeichenabstand 0..255% (0=auf der Stelle; 100=normal; 200=Zeichen wird als
    INT8         ChrVPos;  // Zeichen V-Position default 0= on Baseline, 10= 5Pt drunter (-64..63�)
    ObjLineType  ShdL;     // Schatten-Outline (neu 2.0)
    ObjAreaType  ShdF;     // Schatten-innen   (neu 2.0)
    PointType    ShdVers;  // Schattenversatz Max.300.00%
    BOOL         ShdAbs;   // True-> Schattenversatz ist absolut statt relativ zum Schriftgrad
    BOOL         NoSpc;    // True-> kein Zwischenraum (f�r BackArea)
    ObjAreaType  BackF;    // Hintergrundfl�che
    UINT32 GetFont();
    void   SetFont(UINT32 FontID);
};

class Obj0Type {           // SuperClass f�r Apple-VMT
public:
    virtual void Draw(OutputDevice& rOut);
    virtual ~Obj0Type() {}
};

#define ObjkSize 20  /* eigentlich 21. Wg. Allignment ist Flags jedoch verschoben worden*/
class ObjkType: public Obj0Type {  // Grundkomponenten aller Stardraw-Objekte
public:
    UINT32     Last;
    UINT32     Next;
    UINT16     MemSize;    // in Bytes
    PointType  ObjMin;     // XY-Minimum des Objekts
    PointType  ObjMax;     // XY-Maximum des Objekts
    BYTE       Art;
    BYTE       Layer;
//    BYTE       Flags;    // (Schummel f�r Allignment unter NT)
    friend SvStream& operator>>(SvStream& rIStream, ObjkType& rObjk);
    friend BOOL ObjOverSeek(SvStream& rInp, ObjkType& rObjk);
    virtual void Draw(OutputDevice& rOut);
};


#define StrkSize 38
class StrkType: public ObjkType {
public:
    BYTE        Flags;     // (Schummel f�r Allignment unter NT)
    BYTE        LEnden;    // Linienenden
    ObjLineType L;
    PointType   Pos1;      // Anfangspunkt
    PointType   Pos2;      // Endpunkt
    friend SvStream& operator>>(SvStream& rIStream, StrkType& rStrk);
    virtual void Draw(OutputDevice& rOut);
};


#define RectSize 52
class RectType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        Reserve;
    ObjLineType L;
    ObjAreaType F;
    PointType   Pos1;      // LO-Ecke = Bezugspunkt
    PointType   Pos2;      // R-Ecke
    INT16       Radius;    // Eckenradius
    UINT16      DrehWink;  //  315...<45
    UINT16      Slant;     // >270...<90
    friend SvStream& operator>>(SvStream& rIStream, RectType& rRect);
    virtual void Draw(OutputDevice& rOut);
};


#define PolySize 44
class PolyType: public ObjkType { // identisch mit Spline !
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        LEnden;  // nur f�r Polyline
    ObjLineType L;
    ObjAreaType F;       // nicht f�r Polyline
    BYTE        nPoints;
    BYTE        Reserve;
    UINT32      SD_EckP; // Zeiger auf die Eckpunkte (StarDraw)
    PointType*  EckP;    // Zeiger auf die Eckpunkte (StarView (wird nicht von Disk gelesen!))
    friend SvStream& operator>>(SvStream& rIStream, PolyType& rPoly);
    virtual void Draw(OutputDevice& rOut);
};
#define  PolyClosBit 0x01   // Unterarten von Poly:   0: PolyLine  1: Polygon


#define SplnSize 44
class SplnType: public ObjkType { // identisch mit Poly !
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        LEnden;  // nur f�r nSpline
    ObjLineType L;
    ObjAreaType F;       // nicht f�r nSpline
    BYTE        nPoints;
    BYTE        Reserve;
    UINT32      SD_EckP; // Zeiger auf die Eckpunkte (StarDraw)
    PointType*  EckP;    // Zeiger auf die Eckpunkte (StarView (wird nicht von Disk gelesen!))
    friend SvStream& operator>>(SvStream& rIStream, SplnType& rSpln);
    virtual void Draw(OutputDevice& rOut);
};
// Unterarten von Spline: siehe Poly


#define CircSize 52
class CircType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        LEnden;    // nur Bogen (Kr & El)
    ObjLineType L;
    ObjAreaType F;         // nicht f�r Bogen (Kr & El)
    PointType   Center;    // Mittelpunkt
    PointType   Radius;    // Radius
    UINT16      DrehWink;  // nur Ellipse
    UINT16      StartWink; // � nicht f�r Vollkreis
    UINT16      RelWink;   // � und Vollellipse
    friend SvStream& operator>>(SvStream& rIStream, CircType& rCirc);
    virtual void Draw(OutputDevice& rOut);
};
#define CircFull 0x00  /* Unterarten von Kreis:  0: Kreis          */
#define CircSect 0x01  /*                        1: Kreissektor    */
#define CircAbsn 0x02  /*                        2: Kreisabschnitt */
#define CircArc  0x03  /*                        3: Kreisbogen     */


#define TextSize 116
class TextType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        Reserve;   // f�r Word Allign
    ObjTextType T;         // 64 Bytes  << DWord-Allign bei FontID erforderlich
    PointType   Pos1;      // Bezugspunkt (ObenLinks)
    PointType   Pos2;      //             (untenRechts)
    INT16       TopOfs;    // Von Oberkante bis Textbegin (future f�r vJustify)
    UINT16      DrehWink;  //    0...<360
    UINT16      BoxSlant;  // >270...<90 (nur Box)
    UINT16      BufSize;   // Gr��e von Buf f�r Load, Save, Copy und so
    UINT16      BufLo,BufHi;// (UCHAR*) Zeiger auf den Textbuffer << ShortArr, weil sonst DWord-Allign erforderlich
    UINT16      ExtLo,ExtHi;// (Ptr)  Text �ber mehrere Rahmen    << ShortArr, weil sonst DWord-Allign erforderlich
    PointType   FitSize;   // Ursprungsgr��e f�r Fit2Size
    INT16       FitBreit;  // Breite zum formatieren bei Fit2Size
    UCHAR*      Buffer;    // Diese Variable wird nicht durch Lesen von Disk gef�llt, sondern explizit!
    friend SvStream& operator>>(SvStream& rIStream, TextType& rText);
    virtual void Draw(OutputDevice& rOut);
};
#define TextOutlBit 0x01     /*       1=Sourcecode f�r Outliner (wird von DrawObjekt() ignoriert) */
#define TextFitSBit 0x02     /* Bit1: 1=Text-FitToSize, auch Outliner (2.0)       */
#define TextFitZBit 0x08     /* Bit3: 1=Fit2Size Zeilenweise          (2.0)       */
#define TextDrftBit 0x04     /* Bit2: 1=DraftDraw                     (2.0)       */
#define TextFitBits (TextFitSBit | TextFitZBit)


enum GrafStat {NoGraf,Pic,Pcx,Hpgl,Img,Msp,Tiff,Dxf,Lot,Usr,Sgf};

#define BmapSize 132
class BmapType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    BYTE        Reserve;
    ObjAreaType F;            // Farbe und Muster der 1-Plane Bitmap
    PointType   Pos1;
    PointType   Pos2;
    UINT16      DrehWink;     //  315...<45   (Future)
    UINT16      Slant;        // >270...<90   (Future)
    UCHAR       Filename[80]; //  Pfad
    PointType   PixSize;      // Gr��e in Pixel (0 bei Vektor)
    GrafStat    Format;       // siehe GpmDef.Pas
    BYTE        nPlanes;      // Anzahl der Bitplanes (0 bei Vektor)
    BOOL        RawOut;       // als Raw ausgeben ?
    BOOL        InvOut;       // invertiert ausgeben ?
    BOOL        LightOut;     // aufhellen? (SD20)
    BYTE        GrfFlg;       // (SD20) 0=nSGF 1=Pcx 2=Hpgl 4=Raw $FF=Undef(f�r Fix in DrawBmp)

    INetURLObject aFltPath;     // F�r GraphicFilter
    friend SvStream& operator>>(SvStream& rIStream, BmapType& rBmap);
    virtual void Draw(OutputDevice& rOut);
    void SetPaths( const INetURLObject rFltPath );
};


#define GrupSize 48
class GrupType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel f�r Allignment unter NT)
    UCHAR       Name[13];  // Name der Gruppe
    UINT16      SbLo,SbHi; // (Ptr) Gruppenliste << ShortArr, weil sonst DWord Allign erforderlich
    UINT16      UpLo,UpHi; // (Ptr) Vaterliste   << ShortArr, weil sonst DWord Allign erforderlich
    UINT16      ChartSize; // Speicherbedarf der Diagrammstruktur Struktur
    UINT32      ChartPtr;  // Diagrammstruktur
    UINT32 GetSubPtr();    // hier nur zum Checken, ob Sublist evtl. leer ist.
    friend SvStream& operator>>(SvStream& rIStream, GrupType& rGrup);
//    virtual void Draw(OutputDevice& rOut);
};


void SetLine(ObjLineType& rLine, OutputDevice& rOut);
void SetArea(ObjAreaType& rArea, OutputDevice& rOut);
Color Sgv2SvFarbe(BYTE nFrb1, BYTE nFrb2, BYTE nInts);
void RotatePoint(PointType& P, INT16 cx, INT16 cy, double sn, double cs);
void RotatePoint(Point& P, INT16 cx, INT16 cy, double sn, double cs);
INT16 iMulDiv(INT16 a, INT16 Mul, INT16 Div);
UINT16 MulDiv(UINT16 a, UINT16 Mul, UINT16 Div);


class SgfFontOne {
public:
    SgfFontOne* Next;        // Zeiger f�r Listenverkettung
    UINT32      IFID;
    BOOL        Bold;
    BOOL        Ital;
    BOOL        Sans;
    BOOL        Serf;
    BOOL        Fixd;
    FontFamily  SVFamil;
    CharSet     SVChSet;
    String      SVFName;    // z.B. "Times New Roman" = 15 Chars
    USHORT      SVWidth;    // Durchschnittliche Zeichenbreite in %
         SgfFontOne();
    void ReadOne( ByteString& ID, ByteString& Dsc);
};

class SgfFontLst {
public:
    String      FNam;   // vollst�ndiger Filename des Inifiles
    SgfFontOne* pList;  // Listenanfang
    SgfFontOne* Last;   // Listenende
    UINT32      LastID; // f�r schnelleren Zugriff bei Wiederholungen
    SgfFontOne* LastLn; // f�r schnelleren Zugriff bei Wiederholungen
    BOOL        Tried;
                SgfFontLst();
                ~SgfFontLst();
    void AssignFN(const String& rFName);
    void ReadList();
    void RausList();
    SgfFontOne* GetFontDesc(UINT32 ID);
};

}

#endif //_SGVMAIN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
