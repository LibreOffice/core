/*************************************************************************
 *
 *  $RCSfile: sgvmain.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-02-21 18:28:59 $
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

#ifndef _SGVMAIN_HXX
#define _SGVMAIN_HXX

#ifdef VCL
#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#else // VCL
#include <svgen.hxx>
#endif


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
    PointType Size;       // 0.00mm...819.175mm (Papiergröße)
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
    UINT32       Next;       // Nächste Seite
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
    INT16 LDicke;    //  Strichstärke
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
    UINT16       Grad;     // 0.5..32767.5 Pt - bei 1000 Pt sollte aber schluß sein
    UINT16       Breite;   // 1..65535%  bitte nicht mehr als 500%
    BYTE         Justify;  // 2 Bit Vert (Hi), 3 Bit Hor (Lo)
    BYTE         Kapit;    // 1..255%
    UINT16       Schnitt;  // 8 Flags
    UINT16       LnFeed;   // 1..32767% vom max. Schriftgrad der Zeile
    UINT16       Slant;    // Kursivwinkel 0.00..89.99ø default 15.00ø          doppelt Breit angesehen)
    BYTE         ZAbst;    // Zeichenabstand 0..255% (0=auf der Stelle; 100=normal; 200=Zeichen wird als
    INT8         ChrVPos;  // Zeichen V-Position default 0= on Baseline, 10= 5Pt drunter (-64..63«)
    ObjLineType  ShdL;     // Schatten-Outline (neu 2.0)
    ObjAreaType  ShdF;     // Schatten-innen   (neu 2.0)
    PointType    ShdVers;  // Schattenversatz Max.300.00%
    BOOL         ShdAbs;   // True-> Schattenversatz ist absolut statt relativ zum Schriftgrad
    BOOL         NoSpc;    // True-> kein Zwischenraum (für BackArea)
    ObjAreaType  BackF;    // Hintergrundfläche
    UINT32 GetFont();
    void   SetFont(UINT32 FontID);
};

class Obj0Type {           // SuperClass für Apple-VMT
public:
    virtual void Draw(OutputDevice& rOut);
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
//    BYTE       Flags;    // (Schummel für Allignment unter NT)
    friend SvStream& operator>>(SvStream& rIStream, ObjkType& rObjk);
    friend BOOL ObjOverSeek(SvStream& rInp, ObjkType& rObjk);
    virtual void Draw(OutputDevice& rOut);
};


#define StrkSize 38
class StrkType: public ObjkType {
public:
    BYTE        Flags;     // (Schummel für Allignment unter NT)
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
    BYTE        Flags;    // (Schummel für Allignment unter NT)
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
    BYTE        Flags;    // (Schummel für Allignment unter NT)
    BYTE        LEnden;  // nur für Polyline
    ObjLineType L;
    ObjAreaType F;       // nicht für Polyline
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
    BYTE        Flags;    // (Schummel für Allignment unter NT)
    BYTE        LEnden;  // nur für nSpline
    ObjLineType L;
    ObjAreaType F;       // nicht für nSpline
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
    BYTE        Flags;    // (Schummel für Allignment unter NT)
    BYTE        LEnden;    // nur Bogen (Kr & El)
    ObjLineType L;
    ObjAreaType F;         // nicht für Bogen (Kr & El)
    PointType   Center;    // Mittelpunkt
    PointType   Radius;    // Radius
    UINT16      DrehWink;  // nur Ellipse
    UINT16      StartWink; // ¿ nicht für Vollkreis
    UINT16      RelWink;   // Ù und Vollellipse
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
    BYTE        Flags;    // (Schummel für Allignment unter NT)
    BYTE        Reserve;   // für Word Allign
    ObjTextType T;         // 64 Bytes  << DWord-Allign bei FontID erforderlich
    PointType   Pos1;      // Bezugspunkt (ObenLinks)
    PointType   Pos2;      //             (untenRechts)
    INT16       TopOfs;    // Von Oberkante bis Textbegin (future für vJustify)
    UINT16      DrehWink;  //    0...<360
    UINT16      BoxSlant;  // >270...<90 (nur Box)
    UINT16      BufSize;   // Größe von Buf für Load, Save, Copy und so
    UINT16      BufLo,BufHi;// (UCHAR*) Zeiger auf den Textbuffer << ShortArr, weil sonst DWord-Allign erforderlich
    UINT16      ExtLo,ExtHi;// (Ptr)  Text über mehrere Rahmen    << ShortArr, weil sonst DWord-Allign erforderlich
    PointType   FitSize;   // Ursprungsgröße für Fit2Size
    INT16       FitBreit;  // Breite zum formatieren bei Fit2Size
    UCHAR*      Buffer;    // Diese Variable wird nicht durch Lesen von Disk gefüllt, sondern explizit!
    friend SvStream& operator>>(SvStream& rIStream, TextType& rText);
    virtual void Draw(OutputDevice& rOut);
};
#define TextOutlBit 0x01     /*       1=Sourcecode für Outliner (wird von DrawObjekt() ignoriert) */
#define TextFitSBit 0x02     /* Bit1: 1=Text-FitToSize, auch Outliner (2.0)       */
#define TextFitZBit 0x08     /* Bit3: 1=Fit2Size Zeilenweise          (2.0)       */
#define TextDrftBit 0x04     /* Bit2: 1=DraftDraw                     (2.0)       */
#define TextFitBits (TextFitSBit | TextFitZBit)


enum GrafStat {NoGraf,Pic,Pcx,Hpgl,Img,Msp,Tiff,Dxf,Lot,Usr,Sgf};

#define BmapSize 132
class BmapType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel für Allignment unter NT)
    BYTE        Reserve;
    ObjAreaType F;            // Farbe und Muster der 1-Plane Bitmap
    PointType   Pos1;
    PointType   Pos2;
    UINT16      DrehWink;     //  315...<45   (Future)
    UINT16      Slant;        // >270...<90   (Future)
    UCHAR       Filename[80]; //  Pfad
    PointType   PixSize;      // Größe in Pixel (0 bei Vektor)
    GrafStat    Format;       // siehe GpmDef.Pas
    BYTE        nPlanes;      // Anzahl der Bitplanes (0 bei Vektor)
    BOOL        RawOut;       // als Raw ausgeben ?
    BOOL        InvOut;       // invertiert ausgeben ?
    BOOL        LightOut;     // aufhellen? (SD20)
    BYTE        GrfFlg;       // (SD20) 0=nSGF 1=Pcx 2=Hpgl 4=Raw $FF=Undef(für Fix in DrawBmp)

    INetURLObject aFltPath;     // Für GraphicFilter
    friend SvStream& operator>>(SvStream& rIStream, BmapType& rBmap);
    virtual void Draw(OutputDevice& rOut);
    void SetPaths( const INetURLObject rFltPath );
};


#define GrupSize 48
class GrupType: public ObjkType {
public:
    BYTE        Flags;    // (Schummel für Allignment unter NT)
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
    SgfFontOne* Next;        // Zeiger für Listenverkettung
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
    String      FNam;   // vollständiger Filename des Inifiles
    SgfFontOne* pList;  // Listenanfang
    SgfFontOne* Last;   // Listenende
    UINT32      LastID; // für schnelleren Zugriff bei Wiederholungen
    SgfFontOne* LastLn; // für schnelleren Zugriff bei Wiederholungen
    BOOL        Tried;
                SgfFontLst();
                ~SgfFontLst();
    void AssignFN(const String& rFName);
    void ReadList();
    void RausList();
    SgfFontOne* GetFontDesc(UINT32 ID);
};

#endif //_SGVMAIN_HXX


