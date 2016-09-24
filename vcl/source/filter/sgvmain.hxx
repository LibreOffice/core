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

#ifndef INCLUDED_VCL_SOURCE_FILTER_SGVMAIN_HXX
#define INCLUDED_VCL_SOURCE_FILTER_SGVMAIN_HXX

#include <vcl/font.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#define UCHAR unsigned char

struct PointType {
    sal_Int16 x;
    sal_Int16 y;
    PointType()
        : x(0)
        , y(0)
    {
    }
};

#define SgfDpmm 40

#define DtHdSize 256
class DtHdType {
public:
    sal_uInt8  Reserved[256];
    friend void DtHdOverSeek(SvStream& rInp);
};

struct Seitenformat {
    PointType        Size;        // 0.00mm...819.175mm (papersize)
    sal_Int16        RandL;       // left     border on
    sal_Int16        RandR;       // right    paper
    sal_Int16        RandO;       // upper    border on
    sal_Int16        RandU;       // lower    paper
    sal_uInt8        PColor;      // future use
    sal_uInt8        PIntens;     // future use
};

#define PageSize 146
class PageType {
public:
    sal_uInt32       Next;        // next page
    sal_uInt32       nList;       // objectdata, first record
    sal_uInt32       ListEnd;     // objectdata, last record
    Seitenformat     Paper;       // data of paper
    bool         BorderClip;  // clip objects at border (cheat due to alignment in NT)
    sal_uInt8        StdPg;       // insert which standardpage ?
    PointType        U;           // origin
    sal_Int16        HlpLnH[20];  // guides
    sal_Int16        HlpLnV[20];
    sal_uInt8        LnAnzH;
    sal_uInt8        LnAnzV;
    UCHAR            PgName[32];  // page name
    friend SvStream& ReadPageType(SvStream& rIStream, PageType& rPage);
    PageType();
};

enum ObjArtType {ObjStrk,ObjRect,ObjPoly,ObjCirc,ObjSpln,
                 ObjText,ObjGrup,ObjBmap,ObjVirt,ObjTxtX,ObjMaxi};

struct ObjLineType {
    sal_uInt8   LFarbe;    //  [Index]
    sal_uInt8   LBFarbe;   //  [Index]
    sal_uInt8   LIntens;   //  [%]
    sal_uInt8   LMuster;   //  [Index] including transparency
    sal_Int16   LMSize;    //  [coefficient/100]
    sal_Int16   LDicke;    //  line thickness
};

struct ObjAreaType {
    sal_uInt8   FFarbe;    //  [Index]
    sal_uInt8   FBFarbe;   //  [Index]
    sal_uInt8   FIntens;   //  [%]
    // Despite the "Dummy" in the name, and not being used anywhere in
    // the code, these two fields are *essential*. These structs are
    // 1:1 mappings of actual on-disk/wire file format structures...
    // So don't clean the "unused dummy" fields away. (Actually it
    // might well be that many other fields around here are never used
    // in our code either.)
    sal_uInt8   FDummy1;
    sal_Int16   FDummy2;
    sal_uInt16  FMuster;   //  [Index] incl. Invers, transparency

    ObjAreaType()
        : FFarbe(0)
        , FBFarbe(0)
        , FIntens(0)
        , FDummy1(0)
        , FDummy2(0)
        , FMuster(0)
    {
    }
};

#define ObjTextTypeSize 64
class ObjTextType {
public:
    ObjLineType  L;             // text outline (future)
    ObjAreaType  F;             // inner text
    sal_uInt16   FontLo,FontHi; // e.g. 92500 (CG Times), split due to DWordAllign in TextType.
    sal_uInt16   Grad;          // 0.5..32767.5 pt - we should stop at 1000 pt
    sal_uInt16   Breite;        // 1..65535%  please not more as 500%
    sal_uInt8    Justify;       // 2 bit vertical (Hi), 3 bit horizontal (Lo)
    sal_uInt8    Kapit;         // 1..255%
    sal_uInt16   Schnitt;       // 8 flags
    sal_uInt16   LnFeed;        // 1..32767% of max font size of the line
    sal_uInt16   Slant;         // slant angle 0.00..89.99deg default 15.00deg viewed double width)
    sal_uInt8    ZAbst;         // character spacing 0..255% (0=on the line; 100=normal; 200=Zeichen wird als
    signed char  ChrVPos;       // vertical position of char. Default 0=on baseline, 10=5Pt below (-64..63")
    ObjLineType  ShdL;          // shadow outline (new 2.0)
    ObjAreaType  ShdF;          // shadow inside  (new 2.0)
    PointType    ShdVers;       // shadow offset max. 300.00%
    bool     ShdAbs;        // True-> shadow offset is absolute instead of relative to font size
    bool     NoSpc;         // True-> no space (for background area)
    ObjAreaType  BackF;         // background area
    sal_uInt32 GetFont();
    void   SetFont(sal_uInt32 FontID);
};

#define ObjkSize 20                /* should be 21. due to alignment we shifted the flag */
class ObjkType {  // basic components of all Stardraw objects
public:
    sal_uInt32     Last;
    sal_uInt32     Next;
    sal_uInt16     MemSize;    // in bytes
    PointType      ObjMin;     // XY minimum of the object
    PointType      ObjMax;     // XY maximum of the object
    sal_uInt8      Art;
    sal_uInt8      Layer;
    ObjkType()
        : Last(0)
        , Next(0)
        , MemSize(0)
        , Art(0)
        , Layer(0)
    {
    }
    virtual ~ObjkType() {}
    virtual void Draw(OutputDevice& rOut);
};

#define StrkSize 38
class StrkType: public ObjkType {
public:
    sal_uInt8       Flags;     // (cheat due to alignment in NT)
    sal_uInt8       LEnden;    // line ends
    ObjLineType     L;
    PointType       Pos1;      // start point
    PointType       Pos2;      // end point
    friend SvStream& ReadStrkType(SvStream& rIStream, StrkType& rStrk);
    virtual void Draw(OutputDevice& rOut) override;
};

#define RectSize 52
class RectType: public ObjkType {
public:
    sal_uInt8       Flags;     // (cheat due to alignment in NT)
    sal_uInt8       Reserve;
    ObjLineType     L;
    ObjAreaType     F;
    PointType       Pos1;      // lower left corner = reference point
    PointType       Pos2;      // right corner
    sal_Int16       Radius;    // radius of corner
    sal_uInt16      RotationAngle;  //  315...<45
    sal_uInt16      Slant;          // >270...<90
    friend SvStream& ReadRectType(SvStream& rIStream, RectType& rRect);
    virtual void Draw(OutputDevice& rOut) override;
};

#define PolySize 44
class PolyType: public ObjkType { // identical to spline !
public:
    sal_uInt8        Flags;   // (cheat due to alignment in NT)
    sal_uInt8        LEnden;  // only for polyline
    ObjLineType      L;
    ObjAreaType      F;       // not for polyline
    sal_uInt8        nPoints;
    sal_uInt8        Reserve;
    sal_uInt32       SD_EckP; // pointer to corner point (StarDraw)
    PointType*       EckP;    // pointer to corner points (StarView (is not read from disk!))
    friend SvStream& ReadPolyType(SvStream& rIStream, PolyType& rPoly);
    virtual void Draw(OutputDevice& rOut) override;
};
#define  PolyClosBit 0x01     // kinds of Poly:   0: polyLine  1: polygon

#define SplnSize 44
class SplnType: public ObjkType { // identical to Poly !
public:
    sal_uInt8        Flags;   // (cheat due to alignment in NT)
    sal_uInt8        LEnden;  // only for nSpline
    ObjLineType L;
    ObjAreaType F;            // not for nSpline
    sal_uInt8        nPoints;
    sal_uInt8        Reserve;
    sal_uInt32       SD_EckP; // pointer to corner points (StarDraw)
    PointType*       EckP;    // pointer to corner points (StarView (is not read from disk!))
    friend SvStream& ReadSplnType(SvStream& rIStream, SplnType& rSpln);
    virtual void Draw(OutputDevice& rOut) override;
};
// kinds of Spline: see Poly

#define CircSize 52
class CircType: public ObjkType {
public:
    sal_uInt8       Flags;     // (cheat due to alignment in NT)
    sal_uInt8       LEnden;    // only arcs (circle & ellips)
    ObjLineType     L;
    ObjAreaType     F;         // not for arcs (circle & ellips)
    PointType       Center;    // center
    PointType       Radius;    // radius
    sal_uInt16      RotationAngle;  // only ellipses
    sal_uInt16      StartAngle; // and not for full circles
    sal_uInt16      RelAngle;   // and full ellipses
    friend SvStream& ReadCircType(SvStream& rIStream, CircType& rCirc);
    virtual void Draw(OutputDevice& rOut) override;
};
#define CircFull 0x00  /* kinds of circle: 0: full circle    */
#define CircSect 0x01  /*                  1: circle sector  */
#define CircAbsn 0x02  /*                  2: circle segment */
#define CircArc  0x03  /*                  3: circle arc     */

#define TextSize 116
class TextType: public ObjkType {
public:
    sal_uInt8   Flags;       // (cheat due to alignment in NT)
    sal_uInt8   Reserve;     // for Word Align
    ObjTextType T;           // 64 bytes  << DWord-Allign needed for FontID
    PointType   Pos1;        // reference point (upper left)
    PointType   Pos2;        //                 (lower right)
    sal_Int16   TopOfs;      // from upper side to start of text (future for vJustify)
    sal_uInt16  RotationAngle;    //    0...<360
    sal_uInt16  BoxSlant;    // >270...<90 (only Box)
    sal_uInt16  BufSize;     // size of buffer for Load, Save, Copy etc.
    sal_uInt16  BufLo,BufHi; // (UCHAR*) pointer to Textbuffer << ShortArr, otherwise DWord-Allign needed
    sal_uInt16  ExtLo,ExtHi; // (Ptr)  text over more frames << ShortArr, otherwise DWord-Allign needed
    PointType   FitSize;     // size of origin for Fit2Size
    sal_Int16   FitBreit;    // width to format for Fit2Size
    UCHAR*      Buffer;      // this variable is not set by reading from disk, but explicit!
    friend SvStream& ReadTextType(SvStream& rIStream, TextType& rText);
    virtual void Draw(OutputDevice& rOut) override;
};
#define TextOutlBit 0x01     /*       1=Sourcecode for outliner (ignored byDrawObjekt()) */
#define TextFitSBit 0x02     /* Bit1: 1=Text-Fit2Size, also outliner  (2.0)              */
#define TextFitZBit 0x08     /* Bit3: 1=Fit2Size line by line         (2.0)              */
#define TextFitBits (TextFitSBit | TextFitZBit)

enum GrafStat {NoGraf,Pic,Pcx,Hpgl,Img,Msp,Tiff,Dxf,Lot,Usr,Sgf};

#define BmapSize 132
class BmapType: public ObjkType {
public:
    sal_uInt8   Flags;        // (cheat due to alignment in NT)
    sal_uInt8   Reserve;
    ObjAreaType F;            // colour and pattern of first plane bitmap
    PointType   Pos1;
    PointType   Pos2;
    sal_uInt16  RotationAngle;//  315...<45   (future)
    sal_uInt16  Slant;        // >270...<90   (future)
    UCHAR       Filename[80]; // path
    PointType   PixSize;      // size in Pixel (0 for vector)
    GrafStat    Format;       // see: GpmDef.Pas
    sal_uInt8   nPlanes;      // number of bitplanes (0 for vector)
    bool    RawOut;       // output as raw  ?
    bool    InvOut;       // output inverted ?
    bool    LightOut;     // brighten? (SD20)
    sal_uInt8   GrfFlg;       // (SD20) 0=nSGF 1=Pcx 2=HPGL 4=Raw $FF=Undef (to fix DrawBmp)
    INetURLObject aFltPath;   // for GraphicFilter

    BmapType()
        : Flags(0)
        , Reserve(0)
        , RotationAngle(0)
        , Slant(0)
        , Format(NoGraf)
        , nPlanes(0)
        , RawOut(false)
        , InvOut(false)
        , LightOut(false)
        , GrfFlg(0)
    {
        memset(Filename, 0, sizeof(Filename));
    }

    friend SvStream& ReadBmapType(SvStream& rIStream, BmapType& rBmap);
    virtual void Draw(OutputDevice& rOut) override;
};

#define GrupSize 48
class GrupType: public ObjkType {
public:
    sal_uInt8   Flags;         // (cheat due to alignment in NT)
    UCHAR       Name[13];      // name of group
    sal_uInt16  SbLo,SbHi;     // (Ptr) group list << ShortArr, as otherwise DWord alignment needed
    sal_uInt16  UpLo,UpHi;     // (Ptr) parent list << ShortArr, as otherwise DWord alignment needed
    sal_uInt16  ChartSize;     // required amount of memory for structure of diagram structure
    sal_uInt32  ChartPtr;      // diagram structure
    sal_uInt32  GetSubPtr();   // only to check if Sublist is empty
    friend SvStream& ReadGrupType(SvStream& rIStream, GrupType& rGrup);
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
    SgfFontOne*      Next;        // pointer to list
    sal_uInt32       IFID;
    bool         Bold;
    bool         Ital;
    bool         Sans;
    bool         Serf;
    bool         Fixd;
    FontFamily       SVFamil;
    rtl_TextEncoding SVChSet;
    OUString         SVFName;    // e.g. "Times New Roman" = 15 chars
    sal_uInt16       SVWidth;    // average character width in %
    SgfFontOne();
    void ReadOne(const OString& rID, OString& rDsc);
};

class SgfFontLst {
public:
    OUString    FNam;   // complete filename of the Ini-file
    SgfFontOne* pList;  // start of list
    SgfFontOne* Last;   // end of list
    sal_uInt32  LastID; // for faster access during repeats
    SgfFontOne* LastLn; // for faster access during repeats
    bool        Tried;
                SgfFontLst();
                ~SgfFontLst();
    void AssignFN(const OUString& rFName);
    void ReadList();
    void RausList();
    SgfFontOne* GetFontDesc(sal_uInt32 ID);
};

#endif // INCLUDED_VCL_SOURCE_FILTER_SGVMAIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
