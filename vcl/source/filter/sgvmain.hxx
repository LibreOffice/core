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

#ifndef _SGVMAIN_HXX
#define _SGVMAIN_HXX

#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#define UCHAR unsigned char

struct PointType {
    sal_Int16 x;
    sal_Int16 y;
};

#define SgfDpmm 40

#define DtHdSize 256
class DtHdType {
public:
    sal_uInt8  Reserved[256];
    friend SvStream& operator>>(SvStream& rIStream, DtHdType& rDtHd);
    friend void DtHdOverSeek(SvStream& rInp);
};

struct Seitenformat {
    PointType Size;       // 0.00mm...819.175mm (Papiergr"u"ue)
    sal_Int16     RandL;      // links     Rand auf
    sal_Int16     RandR;      // rechts    dem Papier
    sal_Int16     RandO;      // oben      Rand auf
    sal_Int16     RandU;      // unten     dem Papier
    sal_uInt8      PColor;     // Future Use
    sal_uInt8      PIntens;    // erst recht Future use
};

#define PageSize 146
class PageType {
public:
    sal_uInt32       Next;       // N"uchste Seite
    sal_uInt32       nList;      // Objektdaten, erster Record
    sal_uInt32       ListEnd;    // Objektdaten, letzter Record
    Seitenformat Paper;      // Papierdaten
    sal_Bool         BorderClip; // Objekte am Rand abschneiden (Schummel wg. Allignment unter NT)
    sal_uInt8         StdPg;      // welche Standardseite einblenden ?
    PointType    U;          // Nullpunkt
    sal_Int16        HlpLnH[20]; // Hilfslinien
    sal_Int16        HlpLnV[20];
    sal_uInt8         LnAnzH;
    sal_uInt8         LnAnzV;
    UCHAR        PgName[32]; // Seitenname
    friend SvStream& operator>>(SvStream& rIStream, PageType& rPage);
};

enum ObjArtType {ObjStrk,ObjRect,ObjPoly,ObjCirc,ObjSpln,
                 ObjText,ObjGrup,ObjBmap,ObjVirt,ObjTxtX,ObjMaxi};

struct ObjLineType {
    sal_uInt8  LFarbe;    //  [Index]
    sal_uInt8  LBFarbe;   //  [Index]
    sal_uInt8  LIntens;   //  [%]
    sal_uInt8  LMuster;   //  [Index] inkl. Transparenz
    sal_Int16 LMSize;    //  [Koeffizient/100]
    sal_Int16 LDicke;    //  Strichst"urke
};

struct ObjAreaType {
    sal_uInt8   FFarbe;   //  [Index]
    sal_uInt8   FBFarbe;  //  [Index]
    sal_uInt8   FIntens;  //  [%]
    // Despite the "Dummy" in the name, and not being used anywhere in
    // the code, these two fields are *essential*. These structs are
    // 1:1 mappings of actual on-disk/wire file format structures...
    // So don't clean the "unused dummy" fields away. (Actually it
    // might well be that many other fields around here are never used
    // in our code either.)
    sal_uInt8   FDummy1;  //
    sal_Int16  FDummy2;  //
    sal_uInt16 FMuster;  //  [Index] inkl. Invers, Transparenz
};

#define ObjTextTypeSize 64
class ObjTextType {
public:
    ObjLineType  L;        // Text-Outline (future)
    ObjAreaType  F;        // Text innen
    sal_uInt16       FontLo,FontHi;// z.B. 92500 (CG Times), zweigeteilt wegen DWordAllign in TextType.
    sal_uInt16       Grad;     // 0.5..32767.5 Pt - bei 1000 Pt sollte aber schlu"u sein
    sal_uInt16       Breite;   // 1..65535%  bitte nicht mehr als 500%
    sal_uInt8         Justify;  // 2 Bit Vert (Hi), 3 Bit Hor (Lo)
    sal_uInt8         Kapit;    // 1..255%
    sal_uInt16       Schnitt;  // 8 Flags
    sal_uInt16       LnFeed;   // 1..32767% vom max. Schriftgrad der Zeile
    sal_uInt16       Slant;    // Kursivwinkel 0.00..89.99"u default 15.00"u          doppelt Breit angesehen)
    sal_uInt8         ZAbst;    // Zeichenabstand 0..255% (0=auf der Stelle; 100=normal; 200=Zeichen wird als
    sal_sChar         ChrVPos;  // Zeichen V-Position default 0= on Baseline, 10= 5Pt drunter (-64..63"u)
    ObjLineType  ShdL;     // Schatten-Outline (neu 2.0)
    ObjAreaType  ShdF;     // Schatten-innen   (neu 2.0)
    PointType    ShdVers;  // Schattenversatz Max.300.00%
    sal_Bool         ShdAbs;   // True-> Schattenversatz ist absolut statt relativ zum Schriftgrad
    sal_Bool         NoSpc;    // True-> kein Zwischenraum (f"ur BackArea)
    ObjAreaType  BackF;    // Hintergrundfl"uche
    sal_uInt32 GetFont();
    void   SetFont(sal_uInt32 FontID);
};

class Obj0Type {           // SuperClass f"ur Apple-VMT
public:
    virtual void Draw(OutputDevice& rOut);
    virtual ~Obj0Type() {}
};

#define ObjkSize 20  /* eigentlich 21. Wg. Allignment ist Flags jedoch verschoben worden*/
class ObjkType: public Obj0Type {  // Grundkomponenten aller Stardraw-Objekte
public:
    sal_uInt32     Last;
    sal_uInt32     Next;
    sal_uInt16     MemSize;    // in Bytes
    PointType  ObjMin;     // XY-Minimum des Objekts
    PointType  ObjMax;     // XY-Maximum des Objekts
    sal_uInt8       Art;
    sal_uInt8       Layer;
    friend SvStream& operator>>(SvStream& rIStream, ObjkType& rObjk);
    friend sal_Bool ObjOverSeek(SvStream& rInp, ObjkType& rObjk);
    virtual void Draw(OutputDevice& rOut);
};

#define StrkSize 38
class StrkType: public ObjkType {
public:
    sal_uInt8        Flags;     // (Schummel f"ur Allignment unter NT)
    sal_uInt8        LEnden;    // Linienenden
    ObjLineType L;
    PointType   Pos1;      // Anfangspunkt
    PointType   Pos2;      // Endpunkt
    friend SvStream& operator>>(SvStream& rIStream, StrkType& rStrk);
    virtual void Draw(OutputDevice& rOut);
};

#define RectSize 52
class RectType: public ObjkType {
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        Reserve;
    ObjLineType L;
    ObjAreaType F;
    PointType   Pos1;      // LO-Ecke = Bezugspunkt
    PointType   Pos2;      // R-Ecke
    sal_Int16       Radius;    // Eckenradius
    sal_uInt16      DrehWink;  //  315...<45
    sal_uInt16      Slant;     // >270...<90
    friend SvStream& operator>>(SvStream& rIStream, RectType& rRect);
    virtual void Draw(OutputDevice& rOut);
};

#define PolySize 44
class PolyType: public ObjkType { // identisch mit Spline !
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        LEnden;  // nur f"ur Polyline
    ObjLineType L;
    ObjAreaType F;       // nicht f"ur Polyline
    sal_uInt8        nPoints;
    sal_uInt8        Reserve;
    sal_uInt32      SD_EckP; // Zeiger auf die Eckpunkte (StarDraw)
    PointType*  EckP;    // Zeiger auf die Eckpunkte (StarView (wird nicht von Disk gelesen!))
    friend SvStream& operator>>(SvStream& rIStream, PolyType& rPoly);
    virtual void Draw(OutputDevice& rOut);
};
#define  PolyClosBit 0x01   // Unterarten von Poly:   0: PolyLine  1: Polygon

#define SplnSize 44
class SplnType: public ObjkType { // identisch mit Poly !
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        LEnden;  // nur f"ur nSpline
    ObjLineType L;
    ObjAreaType F;       // nicht f"ur nSpline
    sal_uInt8        nPoints;
    sal_uInt8        Reserve;
    sal_uInt32      SD_EckP; // Zeiger auf die Eckpunkte (StarDraw)
    PointType*  EckP;    // Zeiger auf die Eckpunkte (StarView (wird nicht von Disk gelesen!))
    friend SvStream& operator>>(SvStream& rIStream, SplnType& rSpln);
    virtual void Draw(OutputDevice& rOut);
};
// Unterarten von Spline: siehe Poly

#define CircSize 52
class CircType: public ObjkType {
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        LEnden;    // nur Bogen (Kr & El)
    ObjLineType L;
    ObjAreaType F;         // nicht f"ur Bogen (Kr & El)
    PointType   Center;    // Mittelpunkt
    PointType   Radius;    // Radius
    sal_uInt16      DrehWink;  // nur Ellipse
    sal_uInt16      StartWink; // "u nicht f"ur Vollkreis
    sal_uInt16      RelWink;   // "u und Vollellipse
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
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        Reserve;   // f"ur Word Allign
    ObjTextType T;         // 64 Bytes  << DWord-Allign bei FontID erforderlich
    PointType   Pos1;      // Bezugspunkt (ObenLinks)
    PointType   Pos2;      //             (untenRechts)
    sal_Int16       TopOfs;    // Von Oberkante bis Textbegin (future f"ur vJustify)
    sal_uInt16      DrehWink;  //    0...<360
    sal_uInt16      BoxSlant;  // >270...<90 (nur Box)
    sal_uInt16      BufSize;   // Gr"u"ue von Buf f"ur Load, Save, Copy und so
    sal_uInt16      BufLo,BufHi;// (UCHAR*) Zeiger auf den Textbuffer << ShortArr, weil sonst DWord-Allign erforderlich
    sal_uInt16      ExtLo,ExtHi;// (Ptr)  Text "uber mehrere Rahmen    << ShortArr, weil sonst DWord-Allign erforderlich
    PointType   FitSize;   // Ursprungsgr"u"ue f"ur Fit2Size
    sal_Int16       FitBreit;  // Breite zum formatieren bei Fit2Size
    UCHAR*      Buffer;    // Diese Variable wird nicht durch Lesen von Disk gef"ullt, sondern explizit!
    friend SvStream& operator>>(SvStream& rIStream, TextType& rText);
    virtual void Draw(OutputDevice& rOut);
};
#define TextOutlBit 0x01     /*       1=Sourcecode f"ur Outliner (wird von DrawObjekt() ignoriert) */
#define TextFitSBit 0x02     /* Bit1: 1=Text-FitToSize, auch Outliner (2.0)       */
#define TextFitZBit 0x08     /* Bit3: 1=Fit2Size Zeilenweise          (2.0)       */
#define TextDrftBit 0x04     /* Bit2: 1=DraftDraw                     (2.0)       */
#define TextFitBits (TextFitSBit | TextFitZBit)

enum GrafStat {NoGraf,Pic,Pcx,Hpgl,Img,Msp,Tiff,Dxf,Lot,Usr,Sgf};

#define BmapSize 132
class BmapType: public ObjkType {
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    sal_uInt8        Reserve;
    ObjAreaType F;            // Farbe und Muster der 1-Plane Bitmap
    PointType   Pos1;
    PointType   Pos2;
    sal_uInt16      DrehWink;     //  315...<45   (Future)
    sal_uInt16      Slant;        // >270...<90   (Future)
    UCHAR       Filename[80]; //  Pfad
    PointType   PixSize;      // Gr"u"ue in Pixel (0 bei Vektor)
    GrafStat    Format;       // siehe GpmDef.Pas
    sal_uInt8        nPlanes;      // Anzahl der Bitplanes (0 bei Vektor)
    sal_Bool        RawOut;       // als Raw ausgeben ?
    sal_Bool        InvOut;       // invertiert ausgeben ?
    sal_Bool        LightOut;     // aufhellen? (SD20)
    sal_uInt8        GrfFlg;       // (SD20) 0=nSGF 1=Pcx 2=Hpgl 4=Raw $FF=Undef(f"ur Fix in DrawBmp)

    INetURLObject aFltPath;     // F"ur GraphicFilter
    friend SvStream& operator>>(SvStream& rIStream, BmapType& rBmap);
    virtual void Draw(OutputDevice& rOut);
    void SetPaths( const INetURLObject rFltPath );
};

#define GrupSize 48
class GrupType: public ObjkType {
public:
    sal_uInt8        Flags;    // (Schummel f"ur Allignment unter NT)
    UCHAR       Name[13];  // Name der Gruppe
    sal_uInt16      SbLo,SbHi; // (Ptr) Gruppenliste << ShortArr, weil sonst DWord Allign erforderlich
    sal_uInt16      UpLo,UpHi; // (Ptr) Vaterliste   << ShortArr, weil sonst DWord Allign erforderlich
    sal_uInt16      ChartSize; // Speicherbedarf der Diagrammstruktur Struktur
    sal_uInt32      ChartPtr;  // Diagrammstruktur
    sal_uInt32 GetSubPtr();    // hier nur zum Checken, ob Sublist evtl. leer ist.
    friend SvStream& operator>>(SvStream& rIStream, GrupType& rGrup);
};

void SetLine(ObjLineType& rLine, OutputDevice& rOut);
void SetArea(ObjAreaType& rArea, OutputDevice& rOut);
Color Sgv2SvFarbe(sal_uInt8 nFrb1, sal_uInt8 nFrb2, sal_uInt8 nInts);
void RotatePoint(PointType& P, sal_Int16 cx, sal_Int16 cy, double sn, double cs);
void RotatePoint(Point& P, sal_Int16 cx, sal_Int16 cy, double sn, double cs);
sal_Int16 iMulDiv(sal_Int16 a, sal_Int16 Mul, sal_Int16 Div);
sal_uInt16 MulDiv(sal_uInt16 a, sal_uInt16 Mul, sal_uInt16 Div);

class SgfFontOne {
public:
    SgfFontOne*      Next;        // Zeiger f"ur Listenverkettung
    sal_uInt32       IFID;
    sal_Bool         Bold;
    sal_Bool         Ital;
    sal_Bool         Sans;
    sal_Bool         Serf;
    sal_Bool         Fixd;
    FontFamily       SVFamil;
    rtl_TextEncoding SVChSet;
    OUString         SVFName;    // z.B. "Times New Roman" = 15 Chars
    sal_uInt16       SVWidth;    // Durchschnittliche Zeichenbreite in %
         SgfFontOne();
    void ReadOne(const OString& rID, OString& rDsc);
};

class SgfFontLst {
public:
    OUString    FNam;   // vollst"undiger Filename des Inifiles
    SgfFontOne* pList;  // Listenanfang
    SgfFontOne* Last;   // Listenende
    sal_uInt32      LastID; // f"ur schnelleren Zugriff bei Wiederholungen
    SgfFontOne* LastLn; // f"ur schnelleren Zugriff bei Wiederholungen
    bool        Tried;
                SgfFontLst();
                ~SgfFontLst();
    void AssignFN(const OUString& rFName);
    void ReadList();
    void RausList();
    SgfFontOne* GetFontDesc(sal_uInt32 ID);
};

#endif //_SGVMAIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
