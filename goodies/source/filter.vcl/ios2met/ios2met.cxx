/*************************************************************************
 *
 *  $RCSfile: ios2met.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:15 $
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


#include <math.h>
#include <vcl/graph.hxx>
#include <vcl/poly.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lineinfo.hxx>
#include <svtools/fltcall.hxx>

#ifndef NOOLDSV
#include <vcl/gdiobj.hxx>
#else // NOOLDSV
enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
#endif // NOOLDSV


//============================== defines ===================================

#define OOODEBUG(str,Num) //InfoBox(NULL,String(str)+String(" ")+String(Num)).Execute();

// -----------------------------Feld-Typen-------------------------------

#define BegDocumnMagic 0xA8A8 /* Begin Document */
#define EndDocumnMagic 0xA8A9 /* End Document   */

#define BegResGrpMagic 0xC6A8 /* Begin Resource Group */
#define EndResGrpMagic 0xC6A9 /* End Resource Group   */

#define BegColAtrMagic 0x77A8 /* Begin Color Attribute Table */
#define EndColAtrMagic 0x77A9 /* End Color Attribute Table   */
#define BlkColAtrMagic 0x77B0 /* Color Attribute Table       */
#define MapColAtrMagic 0x77AB /* Map Color Attribute Table   */

#define BegImgObjMagic 0xFBA8 /* Begin Image Object    */
#define EndImgObjMagic 0xFBA9 /* End Image Object      */
#define DscImgObjMagic 0xFBA6 /* Image Data Descriptor */
#define DatImgObjMagic 0xFBEE /* Image Picture Data    */

#define BegObEnv1Magic 0xC7A8 /* Begin Object Environment Group */
#define EndObEnv1Magic 0xC7A9 /* End Object Environment Group   */

#define BegGrfObjMagic 0xBBA8 /* Begin Graphics Object   */
#define EndGrfObjMagic 0xBBA9 /* End Graphics Object     */
#define DscGrfObjMagic 0xBBA6 /* Graphics Data Descritor */
#define DatGrfObjMagic 0xBBEE /* Graphics Data           */

#define MapCodFntMagic 0x8AAB /* Map Coded Font    */
#define MapDatResMagic 0xC3AB /* Map Data Resource */

// -----------------------------Order-Typen-------------------------------

#define GOrdGivArc 0xC6   /* 1 Arc at given position   */
#define GOrdCurArc 0x86   /* 1 Arc at current position */
#define GOrdGivBzr 0xE5   /* 1 Beziercurve at given position   */
#define GOrdCurBzr 0xA5   /* 1 Beziercurve at current position */
#define GOrdGivBox 0xC0   /* 1 Box at given position   */
#define GOrdCurBox 0x80   /* 1 Box at current position */
#define GOrdGivFil 0xC5   /* 1 Fillet at given position   */
#define GOrdCurFil 0x85   /* 1 Fillet at current position */
#define GOrdGivCrc 0xC7   /* 1 Full arc (circle) at given position   */
#define GOrdCurCrc 0x87   /* 1 Full arc (circle) at current position */
#define GOrdGivLin 0xC1   /* 1 Line at given position   */
#define GOrdCurLin 0x81   /* 1 Line at current position */
#define GOrdGivMrk 0xC2   /* 1 Marker at given position   */
#define GOrdCurMrk 0x82   /* 1 Marker at current position */
#define GOrdGivArP 0xE3   /* 1 Partial arc at given position   */
#define GOrdCurArP 0xA3   /* 1 Partial arc at current position */
#define GOrdGivRLn 0xE1   /* 1 Relative line at given position   */
#define GOrdCurRLn 0xA1   /* 1 Relative line at current position */
#define GOrdGivSFl 0xE4   /* 1 Sharp fillet at given position   */
#define GOrdCurSFl 0xA4   /* 1 Sharp fillet at current position */

#define GOrdGivStM 0xF1   /* 1 Character string move at given position   */
#define GOrdCurStM 0xB1   /* 1 Character string move at current position */
#define GOrdGivStr 0xC3   /* 1 Character string at given position   */
#define GOrdCurStr 0x83   /* 1 Character string at current position */
#define GOrdGivStx 0xFEF0 /* 2 Character string extended at given position   */
#define GOrdCurStx 0xFEB0 /* 2 Character string extended at current position */

#define GOrdGivImg 0xD1   /* 1 Begin Image at given position   */
#define GOrdCurImg 0x91   /* 1 Begin Image at current position */
#define GOrdImgDat 0x92   /* 1 Image data                      */
#define GOrdEndImg 0x93   /* 1 End Image                       */
#define GOrdBegAra 0x68   /* 0 1 Begin area */
#define GOrdEndAra 0x60   /* 1 End area     */
#define GOrdBegElm 0xD2   /* 1 Begin element */
#define GOrdEndElm 0x49   /* 0 1 End element */

#define GOrdBegPth 0xD0   /* 1 Begin path    */
#define GOrdEndPth 0x7F   /* 0 1 End path    */
#define GOrdFilPth 0xD7   /* 1 Fill path     */
#define GOrdModPth 0xD8   /* 1 Modify path   */
#define GOrdOutPth 0xD4   /* 1 Outline path  */
#define GOrdSClPth 0xB4   /* 1 Set clip path */

#define GOrdNopNop 0x00   /* 0 0 No operation */
#define GOrdRemark 0x01   /* 1 Comment */
#define GOrdSegLab 0xD3   /* 1 Label */
#define GOrdBitBlt 0xD6   /* 1 Bitblt */
#define GOrdCalSeg 0x07   /* 1 Call Segment */
#define GOrdSSgBnd 0x32   /* 1 Set segment boundary */
#define GOrdSegChr 0x04   /* 1 Segment characteristics */
#define GOrdCloFig 0x7D   /* 0 1 Close Figure */
#define GOrdEndSym 0xFF   /* 0 0 End of symbol definition */
#define GOrdEndPlg 0x3E   /* 0 1 End prolog */
#define GOrdEscape 0xD5   /* 1 Escape */
#define GOrdExtEsc 0xFED5 /* 2 Extended Escape */
#define GOrdPolygn 0xF3   /* 2 Polygons */

#define GOrdStkPop 0x3F   /* 0 1 Pop */

#define GOrdSIvAtr 0x14   /* 1 Set individual attribute          */
#define GOrdPIvAtr 0x54   /* 1 Push and set individual attribute */
#define GOrdSColor 0x0A   /* 0 1 Set color          */
#define GOrdPColor 0x4A   /* 0 1 Push and set color */
#define GOrdSIxCol 0xA6   /* 1 Set indexed color          */
#define GOrdPIxCol 0xE6   /* 1 Push and set indexed color */
#define GOrdSXtCol 0x26   /* 1 Set extended color          */
#define GOrdPXtCol 0x66   /* 1 Push and set extended color */
#define GOrdSBgCol 0x25   /* 1 Set background color          */
#define GOrdPBgCol 0x65   /* 1 Push and set background color */
#define GOrdSBxCol 0xA7   /* 1 Set background indexed color          */
#define GOrdPBxCol 0xE7   /* 1 Push and set background indexed color */
#define GOrdSMixMd 0x0C   /* 0 1 Set mix          */
#define GOrdPMixMd 0x4C   /* 0 1 Push and set mix */
#define GOrdSBgMix 0x0D   /* 0 1 Set background mix          */
#define GOrdPBgMix 0x4D   /* 0 1 Push and set background mix */

#define GOrdSPtSet 0x08   /* 0 1 Set pattern set          */
#define GOrdPPtSet 0x48   /* 0 1 Push and set pattern set */
#define GOrdSPtSym 0x28   /* 0 1 Set pattern symbol          */
#define GOrdPPtSym 0x09   /* 0 1 Push and set pattern symbol */
#define GOrdSPtRef 0xA0   /* 1 Set model pattern reference          */
#define GOrdPPtRef 0xE0   /* 1 Push and set pattern reference point */

#define GOrdSLnEnd 0x1A   /* 0 1 Set line end          */
#define GOrdPLnEnd 0x5A   /* 0 1 Push and set line end */
#define GOrdSLnJoi 0x1B   /* 0 1 Set line join          */
#define GOrdPLnJoi 0x5B   /* 0 1 Push and set line join */
#define GOrdSLnTyp 0x18   /* 0 1 Set line type          */
#define GOrdPLnTyp 0x58   /* 0 1 Push and set line type */
#define GOrdSLnWdt 0x19   /* 0 1 Set line width          */
#define GOrdPLnWdt 0x59   /* 0 1 Push and set line width */
#define GOrdSFrLWd 0x11   /* 1 Set fractional line width          */
#define GOrdPFrLWd 0x51   /* 1 Push and set fractional line width */
#define GOrdSStLWd 0x15   /* 1 Set stroke line width          */
#define GOrdPStLWd 0x55   /* 1 Push and set stroke line width */

#define GOrdSChDir 0x3A   /* 0 1 Set character direction          */
#define GOrdPChDir 0x7A   /* 0 1 Push and set character direction */
#define GOrdSChPrc 0x39   /* 0 1 Set character precision          */
#define GOrdPChPrc 0x79   /* 0 1 Push and set character precision */
#define GOrdSChSet 0x38   /* 0 1 Set character set          */
#define GOrdPChSet 0x78   /* 0 1 Push and set character set */
#define GOrdSChAng 0x34   /* 1 Set character angle          */
#define GOrdPChAng 0x74   /* 1 Push and set character angle */
#define GOrdSChBrx 0x05   /* 1 Set character break extra          */
#define GOrdPChBrx 0x45   /* 1 Push and set character break extra */
#define GOrdSChCel 0x33   /* 1 Set character cell          */
#define GOrdPChCel 0x03   /* 1 Push and set character cell */
#define GOrdSChXtr 0x17   /* 1 Set character extra          */
#define GOrdPChXtr 0x57   /* 1 Push and set character extra */
#define GOrdSChShr 0x35   /* 1 Set character shear          */
#define GOrdPChShr 0x75   /* 1 Push and set character shear */
#define GOrdSTxAlg 0x36   /* 0 2 Set text allingment          */
#define GOrdPTxAlg 0x76   /* 0 2 Push and set text allingment */

#define GOrdSMkPrc 0x3B   /* 0 1 Set marker precision          */
#define GOrdPMkPrc 0x7B   /* 0 1 Push and set marker precision */
#define GOrdSMkSet 0x3C   /* 0 1 Set marker set          */
#define GOrdPMkSet 0x7C   /* 0 1 Push and set marker set */
#define GOrdSMkSym 0x29   /* 0 1 Set marker symbol          */
#define GOrdPMkSym 0x69   /* 0 1 Push and set marker symbol */
#define GOrdSMkCel 0x37   /* 1 Set marker cell          */
#define GOrdPMkCel 0x77   /* 1 Push and set marker cell */

#define GOrdSArcPa 0x22   /* 1 Set arc parameters          */
#define GOrdPArcPa 0x62   /* 1 Push and set arc parameters */

#define GOrdSCrPos 0x21   /* 1 Set current position          */
#define GOrdPCrPos 0x61   /* 1 Push and set current position */

#define GOrdSMdTrn 0x24   /* 1 Set model transform          */
#define GOrdPMdTrn 0x64   /* 1 Push and set model transform */
#define GOrdSPkIdn 0x43   /* 1 Set pick identifier          */
#define GOrdPPkIdn 0x23   /* 1 Push and set pick identifier */
#define GOrdSVwTrn 0x31   /* 1 Set viewing transform */
#define GOrdSVwWin 0x27   /* 1 Set viewing window          */
#define GOrdPVwWin 0x67   /* 1 Push and set viewing window */

//============================ OS2METReader ==================================

struct OSPalette {
    OSPalette * pSucc;
    ULONG * p0RGB; // Darf auch NULL sein!
    USHORT nSize;
};

struct OSArea {
    OSArea * pSucc;
    BYTE nFlags;
    PolyPolygon aPPoly;
    BOOL bClosed;
    Color      aCol;
    Color      aBgCol;
    RasterOp   eMix;
    RasterOp   eBgMix;
    BOOL       bFill;
    OSArea(){} ~OSArea(){}
};

struct OSPath
{
    OSPath*     pSucc;
    ULONG       nID;
    PolyPolygon aPPoly;
    BOOL        bClosed;
    BOOL        bStroke;

                OSPath(){}
                ~OSPath(){}
};

struct OSFont {
    OSFont * pSucc;
    ULONG nID;
    Font aFont;
    OSFont(){} ~OSFont(){}
};

struct OSBitmap {
    OSBitmap * pSucc;
    ULONG nID;
    Bitmap aBitmap;

    // Waehrend des Lesens der Bitmap benoetigt:
    SvStream * pBMP; // Zeiger auf temporaere Windows-BMP-Datei oder NULL
    ULONG nWidth, nHeight;
    USHORT nBitsPerPixel;
    ULONG nMapPos;
    OSBitmap(){} ~OSBitmap(){}
};

struct OSAttr {
    OSAttr * pSucc;
    USHORT nPushOrder;
    BYTE nIvAttrA, nIvAttrP; // Spezialvariablen fuer den Order "GOrdPIvAtr"

    Color    aLinCol;
    Color    aLinBgCol;
    RasterOp eLinMix;
    RasterOp eLinBgMix;
    Color    aChrCol;
    Color    aChrBgCol;
    RasterOp eChrMix;
    RasterOp eChrBgMix;
    Color    aMrkCol;
    Color    aMrkBgCol;
    RasterOp eMrkMix;
    RasterOp eMrkBgMix;
    Color    aPatCol;
    Color    aPatBgCol;
    RasterOp ePatMix;
    RasterOp ePatBgMix;
    Color    aImgCol;
    Color    aImgBgCol;
    RasterOp eImgMix;
    RasterOp eImgBgMix;
    long     nArcP, nArcQ, nArcR, nArcS;
    short    nChrAng;
//  long     nChrBreakExtra;
    Size     aChrCellSize;
//  BYTE     nChrDir;
//  long     nChrExtra;
//  BYTE     nChrPrec;
    ULONG    nChrSet;
//  Size     aChrShear;
    Point    aCurPos;
//  long     nFracLinWidth;
//  BYTE     nLinEnd;
//  BYTE     nLinJoin;
    PenStyle eLinStyle;
    USHORT   nLinWidth;
    Size     aMrkCellSize;
    BYTE     nMrkPrec;
    BYTE     nMrkSet;
    BYTE     nMrkSymbol;
//  //...    aModTransform;
//  Point    aPatRef;
//  BYTE     nPatSet;
    BOOL     bFill;
//  ULONG    nPickId;
//  //...    aSegBound;
    USHORT   nStrLinWidth;
//  BYTE     nTxtAlignHor,nTxtAlignVer;
//  //...    aViewTransform;
//  //...    aViewWindow;
    OSAttr(){} ~OSAttr(){}
};

class OS2METReader {

private:

    long ErrorCode;

    PFilterCallback pCallback;
    void * pCallerData;

    SvStream    * pOS2MET;             // Die einzulesende OS2MET-Datei
    VirtualDevice * pVirDev;         // Hier werden die Drawing-Methoden aufgerufen.
                                     // Dabei findet ein Recording in das GDIMetaFile
                                     // statt.
    ULONG         nOrigPos;          // Anfaengliche Position in pOS2MET
    UINT16        nOrigNumberFormat; // Anfaengliches Nummern-Format von pOS2MET
    Rectangle aBoundingRect; // Boundingrectangle wie in Datei angegeben
    Rectangle aCalcBndRect;  // selbst ermitteltes Boundingrectangle
    MapMode aGlobMapMode;    // Aufloesung des Bildes
    BOOL bCoord32;

    OSPalette  * pPaletteStack;

    LineInfo aLineInfo;

    OSArea   * pAreaStack; // Areas, die in Arbeit sind

    OSPath   * pPathStack; // Paths, die in Arbeit sind
    OSPath   * pPathList;  // Vollendete Paths

    OSFont   * pFontList;

    OSBitmap * pBitmapList;

    OSAttr   aDefAttr;
    OSAttr   aAttr;
    OSAttr   * pAttrStack;

    SvStream * pOrdFile;

    BOOL Callback(USHORT nPercent);

    void AddPointsToPath(const Polygon & rPoly);
    void AddPointsToArea(const Polygon & rPoly);
    void CloseFigure();
    void PushAttr(USHORT nPushOrder);
    void PopAttr();

    void ChangeBrush( const Color& rPatColor, const Color& rBGColor, BOOL bFill );
    void SetPen( const Color& rColor, USHORT nStrLinWidth = 0, PenStyle ePenStyle = PEN_SOLID );
    void SetRasterOp(RasterOp eROP);

    void SetPalette0RGB(USHORT nIndex, ULONG nCol);
    ULONG GetPalette0RGB(ULONG nIndex);
        // Holt Farbe aus der Palette, oder, wenn nicht vorhanden,
        // interpretiert nIndex als direkten RGB-Wert.
    Color GetPaletteColor(ULONG nIndex);


    BOOL        IsLineInfo();
    void        DrawPolyLine( const Polygon& rPolygon );
    void        DrawPolygon( const Polygon& rPolygon );
    void        DrawPolyPolygon( const PolyPolygon& rPolygon );
    USHORT      ReadBigEndianWord();
    ULONG       ReadBigEndian3BytesLong();
    ULONG       ReadLittleEndian3BytesLong();
    long        ReadCoord(BOOL b32);
    Point       ReadPoint( const BOOL bAdjustBoundRect = TRUE );
    RasterOp    OS2MixToRasterOp(BYTE nMix);
    void        ReadLine(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadRelLine(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadBox(BOOL bGivenPos);
    void        ReadBitBlt();
    void        ReadChrStr(BOOL bGivenPos, BOOL bMove, BOOL bExtra, USHORT nOrderLen);
    void        ReadArc(BOOL bGivenPos);
    void        ReadFullArc(BOOL bGivenPos, USHORT nOrderSize);
    void        ReadPartialArc(BOOL bGivenPos, USHORT nOrderSize);
    void        ReadPolygons();
    void        ReadBezier(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadFillet(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadFilletSharp(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadMarker(BOOL bGivenPos, USHORT nOrderLen);
    void        ReadOrder(USHORT nOrderID, USHORT nOrderLen);
    void        ReadDsc(USHORT nDscID, USHORT nDscLen);
    void        ReadImageData(USHORT nDataID, USHORT nDataLen);
    void        ReadFont(USHORT nFieldSize);
    void        ReadField(USHORT nFieldType, USHORT nFieldSize);

public:

    OS2METReader();
    ~OS2METReader();

    void ReadOS2MET( SvStream & rStreamOS2MET, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata);
        // Liesst aus dem Stream eine OS2MET-Datei und fuellt das GDIMetaFile

};

//=================== Methoden von OS2METReader ==============================

BOOL OS2METReader::Callback(USHORT nPercent)
{
    if (pCallback!=NULL) {
        if (((*pCallback)(pCallerData,nPercent))==TRUE) {
            pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
            return TRUE;
        }
    }
    return FALSE;
}

OS2METReader::OS2METReader()
{
}

OS2METReader::~OS2METReader()
{
}

BOOL OS2METReader::IsLineInfo()
{
    return ( ! ( aLineInfo.IsDefault() || ( aLineInfo.GetStyle() == LINE_NONE ) || ( pVirDev->GetLineColor() == COL_TRANSPARENT ) ) );
}

void OS2METReader::DrawPolyLine( const Polygon& rPolygon )
{
    if ( aLineInfo.GetStyle() == LINE_DASH || ( aLineInfo.GetWidth() > 1 ) )
        pVirDev->DrawPolyLine( rPolygon, aLineInfo );
    else
        pVirDev->DrawPolyLine( rPolygon );
}

void OS2METReader::DrawPolygon( const Polygon& rPolygon )
{
    if ( IsLineInfo() )
    {
        pVirDev->Push( PUSH_LINECOLOR );
        pVirDev->SetLineColor( COL_TRANSPARENT );
        pVirDev->DrawPolygon( rPolygon );
        pVirDev->Pop();
        pVirDev->DrawPolyLine( rPolygon, aLineInfo );
    }
    else
        pVirDev->DrawPolygon( rPolygon );
}

void OS2METReader::DrawPolyPolygon( const PolyPolygon& rPolyPolygon )
{
    if ( IsLineInfo() )
    {
        pVirDev->Push( PUSH_LINECOLOR );
        pVirDev->SetLineColor( COL_TRANSPARENT );
        pVirDev->DrawPolyPolygon( rPolyPolygon );
        pVirDev->Pop();
        for ( USHORT i = 0; i < rPolyPolygon.Count(); i++ )
            pVirDev->DrawPolyLine( rPolyPolygon.GetObject( i ), aLineInfo );
    }
    else
        pVirDev->DrawPolyPolygon( rPolyPolygon );
}

void OS2METReader::AddPointsToArea(const Polygon & rPoly)
{
    USHORT nOldSize, nNewSize,i;

    if (pAreaStack==NULL || rPoly.GetSize()==0) return;
    PolyPolygon * pPP=&(pAreaStack->aPPoly);
    if (pPP->Count()==0 || pAreaStack->bClosed==TRUE) pPP->Insert(rPoly);
    else {
        Polygon aLastPoly(pPP->GetObject(pPP->Count()-1));
        nOldSize=aLastPoly.GetSize();
        if (aLastPoly.GetPoint(nOldSize-1)==rPoly.GetPoint(0)) nOldSize--;
        nNewSize=nOldSize+rPoly.GetSize();
        aLastPoly.SetSize(nNewSize);
        for (i=nOldSize; i<nNewSize; i++) {
            aLastPoly.SetPoint(rPoly.GetPoint(i-nOldSize),i);
        }
        pPP->Replace(aLastPoly,pPP->Count()-1);
    }
    pAreaStack->bClosed=FALSE;
}

void OS2METReader::AddPointsToPath(const Polygon & rPoly)
{
    USHORT nOldSize, nNewSize,i;

    if (pPathStack==NULL || rPoly.GetSize()==0) return;
    PolyPolygon * pPP=&(pPathStack->aPPoly);
    if (pPP->Count()==0 /*|| pPathStack->bClosed==TRUE*/) pPP->Insert(rPoly);
    else {
        Polygon aLastPoly(pPP->GetObject(pPP->Count()-1));
        nOldSize=aLastPoly.GetSize();
        if (aLastPoly.GetPoint(nOldSize-1)!=rPoly.GetPoint(0)) pPP->Insert(rPoly);
        else {
            nOldSize--;
            nNewSize=nOldSize+rPoly.GetSize();
            aLastPoly.SetSize(nNewSize);
            for (i=nOldSize; i<nNewSize; i++) {
                aLastPoly.SetPoint(rPoly.GetPoint(i-nOldSize),i);
            }
            pPP->Replace(aLastPoly,pPP->Count()-1);
        }
    }
    pPathStack->bClosed=FALSE;
}

void OS2METReader::CloseFigure()
{
    if (pAreaStack!=NULL) pAreaStack->bClosed=TRUE;
    else if (pPathStack!=NULL) pPathStack->bClosed=TRUE;
}

void OS2METReader::PushAttr(USHORT nPushOrder)
{
    OSAttr * p;
    p=new OSAttr;
    *p=aAttr;
    p->pSucc=pAttrStack; pAttrStack=p;
    p->nPushOrder=nPushOrder;
}

void OS2METReader::PopAttr()
{
    OSAttr * p=pAttrStack;

    if (p==NULL) return;
    switch (p->nPushOrder) {

        case GOrdPIvAtr:
            switch (p->nIvAttrA) {
                case 1: switch (p->nIvAttrP) {
                    case 1: aAttr.aLinCol=p->aLinCol; break;
                    case 2: aAttr.aChrCol=p->aChrCol; break;
                    case 3: aAttr.aMrkCol=p->aMrkCol; break;
                    case 4: aAttr.aPatCol=p->aPatCol; break;
                    case 5: aAttr.aImgCol=p->aImgCol; break;
                } break;
                case 2: switch (p->nIvAttrP) {
                    case 1: aAttr.aLinBgCol=p->aLinBgCol; break;
                    case 2: aAttr.aChrBgCol=p->aChrBgCol; break;
                    case 3: aAttr.aMrkBgCol=p->aMrkBgCol; break;
                    case 4: aAttr.aPatBgCol=p->aPatBgCol; break;
                    case 5: aAttr.aImgBgCol=p->aImgBgCol; break;
                } break;
                case 3: switch (p->nIvAttrP) {
                    case 1: aAttr.eLinMix=p->eLinMix; break;
                    case 2: aAttr.eChrMix=p->eChrMix; break;
                    case 3: aAttr.eMrkMix=p->eMrkMix; break;
                    case 4: aAttr.ePatMix=p->ePatMix; break;
                    case 5: aAttr.eImgMix=p->eImgMix; break;
                } break;
                case 4: switch (p->nIvAttrP) {
                    case 1: aAttr.eLinBgMix=p->eLinBgMix; break;
                    case 2: aAttr.eChrBgMix=p->eChrBgMix; break;
                    case 3: aAttr.eMrkBgMix=p->eMrkBgMix; break;
                    case 4: aAttr.ePatBgMix=p->ePatBgMix; break;
                    case 5: aAttr.eImgBgMix=p->eImgBgMix; break;
                } break;
            }
            break;

        case GOrdPLnTyp: aAttr.eLinStyle=p->eLinStyle; break;

        case GOrdPLnWdt: aAttr.nLinWidth=p->nLinWidth; break;

        case GOrdPStLWd: aAttr.nStrLinWidth=p->nStrLinWidth; break;

        case GOrdPChSet: aAttr.nChrSet=p->nChrSet; break;

        case GOrdPChAng: aAttr.nChrAng=p->nChrAng; break;

        case GOrdPMixMd:
            aAttr.eLinMix=p->eLinMix;
            aAttr.eChrMix=p->eChrMix;
            aAttr.eMrkMix=p->eMrkMix;
            aAttr.ePatMix=p->ePatMix;
            aAttr.eImgMix=p->eImgMix;
            break;

        case GOrdPBgMix:
            aAttr.eLinBgMix=p->eLinBgMix;
            aAttr.eChrBgMix=p->eChrBgMix;
            aAttr.eMrkBgMix=p->eMrkBgMix;
            aAttr.ePatBgMix=p->ePatBgMix;
            aAttr.eImgBgMix=p->eImgBgMix;
            break;

        case GOrdPPtSym: aAttr.bFill = p->bFill; break;

        case GOrdPColor:
        case GOrdPIxCol:
        case GOrdPXtCol:
            aAttr.aLinCol=p->aLinCol;
            aAttr.aChrCol=p->aChrCol;
            aAttr.aMrkCol=p->aMrkCol;
            aAttr.aPatCol=p->aPatCol;
            aAttr.aImgCol=p->aImgCol;
            break;

        case GOrdPBgCol:
        case GOrdPBxCol:
            aAttr.aLinBgCol=p->aLinBgCol;
            aAttr.aChrBgCol=p->aChrBgCol;
            aAttr.aMrkBgCol=p->aMrkBgCol;
            aAttr.aPatBgCol=p->aPatBgCol;
            aAttr.aImgBgCol=p->aImgBgCol;
            break;

        case GOrdPMkPrc: aAttr.nMrkPrec=aDefAttr.nMrkPrec; break;

        case GOrdPMkSet: aAttr.nMrkSet=aDefAttr.nMrkSet; break;

        case GOrdPMkSym: aAttr.nMrkSymbol=aDefAttr.nMrkSymbol; break;

        case GOrdPMkCel: aAttr.aMrkCellSize=aDefAttr.aMrkCellSize; break;

        case GOrdPArcPa:
            aAttr.nArcP=p->nArcP; aAttr.nArcQ=p->nArcQ;
            aAttr.nArcR=p->nArcR; aAttr.nArcS=p->nArcS;
            break;

        case GOrdPCrPos:
            aAttr.aCurPos=p->aCurPos;
            break;
    }
    pAttrStack=p->pSucc;
    delete p;
}

void OS2METReader::ChangeBrush(const Color& rPatColor, const Color& rBGColor, BOOL bFill )
{
    Color aColor;

    if( bFill )
        aColor = rPatColor;
    else
        aColor = Color( COL_TRANSPARENT );

    if( pVirDev->GetFillColor() != aColor )
        pVirDev->SetFillColor( aColor );
}

void OS2METReader::SetPen( const Color& rColor, USHORT nLineWidth, PenStyle ePenStyle )
{
    LineStyle eLineStyle( LINE_SOLID );

    if ( pVirDev->GetLineColor() != rColor )
        pVirDev->SetLineColor( rColor );
    aLineInfo.SetWidth( nLineWidth );

    USHORT nDotCount = 0;
    USHORT nDashCount = 0;
    switch ( ePenStyle )
    {
        case PEN_NULL :
            eLineStyle = LINE_NONE;
        break;
        case PEN_DASHDOT :
            nDashCount++;
        case PEN_DOT :
            nDotCount++;
            nDashCount--;
        case PEN_DASH :
            nDashCount++;
            aLineInfo.SetDotCount( nDotCount );
            aLineInfo.SetDashCount( nDashCount );
            aLineInfo.SetDistance( nLineWidth );
            aLineInfo.SetDotLen( nLineWidth );
            aLineInfo.SetDashLen( nLineWidth << 2 );
            eLineStyle = LINE_DASH;
        break;
    }
    aLineInfo.SetStyle( eLineStyle );
}

void OS2METReader::SetRasterOp(RasterOp eROP)
{
    if (pVirDev->GetRasterOp()!=eROP) pVirDev->SetRasterOp(eROP);
}


void OS2METReader::SetPalette0RGB(USHORT nIndex, ULONG nCol)
{
    if (pPaletteStack==NULL) {
        pPaletteStack=new OSPalette;
        pPaletteStack->pSucc=NULL;
        pPaletteStack->p0RGB=NULL;
        pPaletteStack->nSize=0;
    }
    if (pPaletteStack->p0RGB==NULL || nIndex>=pPaletteStack->nSize) {
        ULONG * pOld0RGB=pPaletteStack->p0RGB;
        USHORT i,nOldSize=pPaletteStack->nSize;
        if (pOld0RGB==NULL) nOldSize=0;
        pPaletteStack->nSize=2*(nIndex+1);
        if (pPaletteStack->nSize<256) pPaletteStack->nSize=256;
        pPaletteStack->p0RGB = new ULONG[pPaletteStack->nSize];
        for (i=0; i<pPaletteStack->nSize; i++) {
            if (i<nOldSize) pPaletteStack->p0RGB[i]=pOld0RGB[i];
            else if (i==0) pPaletteStack->p0RGB[i]=0x00ffffff;
            else pPaletteStack->p0RGB[i]=0;
        }
        if (pOld0RGB!=NULL) delete pOld0RGB;
    }
    pPaletteStack->p0RGB[nIndex]=nCol;
}

ULONG OS2METReader::GetPalette0RGB(ULONG nIndex)
{
    if (pPaletteStack!=NULL && pPaletteStack->p0RGB!=NULL &&
        pPaletteStack->nSize>nIndex) nIndex=pPaletteStack->p0RGB[nIndex];
    return nIndex;
}

Color OS2METReader::GetPaletteColor(ULONG nIndex)
{
    nIndex=GetPalette0RGB(nIndex);
    return Color((((USHORT)(nIndex>>8))&0xff00)>>8,
                 (((USHORT) nIndex)&0xff00)>>8,
                 (((USHORT)(nIndex<<8))&0xff00)>>8);
}


USHORT OS2METReader::ReadBigEndianWord()
{
    BYTE nLo,nHi;
    *pOS2MET >> nHi >> nLo;
    return (((USHORT)nHi)<<8)|(((USHORT)nLo)&0x00ff);
}

ULONG OS2METReader::ReadBigEndian3BytesLong()
{
    USHORT nLo;
    BYTE nHi;
    *pOS2MET >> nHi;
    nLo=ReadBigEndianWord();
    return ((((ULONG)nHi)<<16)&0x00ff0000)|((ULONG)nLo);
}

ULONG OS2METReader::ReadLittleEndian3BytesLong()
{
    BYTE nHi,nMed,nLo;

    *pOS2MET >> nLo >> nMed >> nHi;
    return ((((ULONG)nHi)&0xff)<<16)|((((ULONG)nMed)&0xff)<<8)|(((ULONG)nLo)&0xff);
}

long OS2METReader::ReadCoord(BOOL b32)
{
    long l;
    short s;

    if (b32) *pOS2MET >> l;
    else  { *pOS2MET >> s; l=(long)s; }
    return l;
}

Point OS2METReader::ReadPoint( const BOOL bAdjustBoundRect )
{
    long x,y;

    x=ReadCoord(bCoord32);
    y=ReadCoord(bCoord32);
    x=x-aBoundingRect.Left();
    y=aBoundingRect.Bottom()-y;

    if ( bAdjustBoundRect )
        aCalcBndRect.Union(Rectangle(x,y,x+1,y+1));

    return Point(x,y);
}

RasterOp OS2METReader::OS2MixToRasterOp(BYTE nMix)
{
    switch (nMix) {
        case 0x0c: return ROP_INVERT;
        case 0x04: return ROP_XOR;
        case 0x0b: return ROP_XOR;
        default:   return ROP_OVERPAINT;
    }
}

void OS2METReader::ReadLine(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i,nPolySize;

    if (bCoord32) nPolySize=nOrderLen/8; else nPolySize=nOrderLen/4;
    if (!bGivenPos) nPolySize++;
    if (nPolySize==0) return;
    Polygon aPolygon(nPolySize);
    for (i=0; i<nPolySize; i++) {
        if (i==0 && !bGivenPos) aPolygon.SetPoint(aAttr.aCurPos,i);
        else aPolygon.SetPoint(ReadPoint(),i);
    }
    aAttr.aCurPos=aPolygon.GetPoint(nPolySize-1);
    if (pAreaStack!=NULL) AddPointsToArea(aPolygon);
    else if (pPathStack!=NULL) AddPointsToPath(aPolygon);
    else
    {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        SetRasterOp(aAttr.eLinMix);
        DrawPolyLine( aPolygon );
    }
}

void OS2METReader::ReadRelLine(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i,nPolySize;
    Point aP0;


    if (bGivenPos) {
        aP0=ReadPoint();
        if (bCoord32) nOrderLen-=8; else nOrderLen-=4;
    }
    else aP0=aAttr.aCurPos;
    nPolySize=nOrderLen/2;
    if (nPolySize==0) return;
    Polygon aPolygon(nPolySize);
    for (i=0; i<nPolySize; i++) {
#if (defined SOLARIS && defined PPC) || defined IRIX
        UINT8 nunsignedbyte;
        *pOS2MET >> nunsignedbyte; aP0.X()+=(INT8)nunsignedbyte;
        *pOS2MET >> nunsignedbyte; aP0.Y()+=(INT8)nunsignedbyte;
#else
        INT8 nsignedbyte;
        *pOS2MET >> nsignedbyte; aP0.X()+=(long)nsignedbyte;
        *pOS2MET >> nsignedbyte; aP0.Y()-=(long)nsignedbyte;
#endif
        aCalcBndRect.Union(Rectangle(aP0,Size(1,1)));
        aPolygon.SetPoint(aP0,i);
    }
    aAttr.aCurPos=aPolygon.GetPoint(nPolySize-1);
    if (pAreaStack!=NULL) AddPointsToArea(aPolygon);
    else if (pPathStack!=NULL) AddPointsToPath(aPolygon);
    else
    {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        SetRasterOp(aAttr.eLinMix);
        DrawPolyLine( aPolygon );
    }
}

void OS2METReader::ReadBox(BOOL bGivenPos)
{
    BYTE        nFlags;
    Point       P0;
    long        nHRound,nVRound;

    *pOS2MET >> nFlags;
    pOS2MET->SeekRel(1);

    if ( bGivenPos )
        P0 = ReadPoint();
    else
        P0 = aAttr.aCurPos;

    aAttr.aCurPos=ReadPoint();
    nHRound=ReadCoord(bCoord32);
    nVRound=ReadCoord(bCoord32);

    Rectangle aBoxRect( P0, aAttr.aCurPos );

    if ( pAreaStack )
        AddPointsToArea( Polygon( aBoxRect ) );
    else if ( pPathStack )
        AddPointsToPath( Polygon( aBoxRect ) );
    else
    {
        if ( nFlags & 0x20 )
            SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        else
            SetPen( COL_TRANSPARENT );

        if ( nFlags & 0x40 )
        {
            ChangeBrush(aAttr.aPatCol,aAttr.aPatBgCol,aAttr.bFill);
            SetRasterOp(aAttr.ePatMix);
        }
        else
        {
            ChangeBrush( Color( COL_TRANSPARENT ), Color( COL_TRANSPARENT ), FALSE );
            SetRasterOp(aAttr.eLinMix);
        }

        if ( IsLineInfo() )
        {
            Polygon aPolygon( aBoxRect, nHRound, nVRound );
            if ( nFlags & 0x40 )
            {
                pVirDev->Push( PUSH_LINECOLOR );
                pVirDev->SetLineColor( COL_TRANSPARENT );
                pVirDev->DrawRect( aBoxRect, nHRound, nVRound );
                pVirDev->Pop();
            }
            pVirDev->DrawPolyLine( aPolygon, aLineInfo );
        }
        else
            pVirDev->DrawRect( aBoxRect, nHRound, nVRound );
    }
}

void OS2METReader::ReadBitBlt()
{
    Point aP1,aP2;
    Size aSize;
    ULONG nID;
    OSBitmap * pB;
    long nt;

    pOS2MET->SeekRel(4);
    *pOS2MET >> nID;
    pOS2MET->SeekRel(4);
    aP1=ReadPoint(); aP2=ReadPoint();
    if (aP1.X() > aP2.X()) { nt=aP1.X(); aP1.X()=aP2.X(); aP2.X()=nt; }
    if (aP1.Y() > aP2.Y()) { nt=aP1.Y(); aP1.Y()=aP2.Y(); aP2.Y()=nt; }
    aSize=Size(aP2.X()-aP1.X(),aP2.Y()-aP1.Y());

    pB=pBitmapList;
    while (pB!=NULL && pB->nID!=nID) pB=pB->pSucc;
    if (pB!=NULL) {
        SetRasterOp(aAttr.ePatMix);
        pVirDev->DrawBitmap(aP1,aSize,pB->aBitmap);
    }
}

void OS2METReader::ReadChrStr(BOOL bGivenPos, BOOL bMove, BOOL bExtra, USHORT nOrderLen)
{
    Point aP0;
    USHORT i, nLen;
    char * pChr;
    OSFont * pF;
    Font aFont;
    Size aSize;

    pF = pFontList;
    while (pF!=NULL && pF->nID!=aAttr.nChrSet) pF=pF->pSucc;
    if (pF!=NULL)
        aFont = pF->aFont;
    aFont.SetColor(aAttr.aChrCol);
    aFont.SetSize(Size(0,aAttr.aChrCellSize.Height()));
    if ( aAttr.nChrAng != 0 )
        aFont.SetOrientation(aAttr.nChrAng);

    if (bGivenPos)
        aP0 = ReadPoint();
    else
        aP0 = aAttr.aCurPos;
    if (bExtra)
    {
        pOS2MET->SeekRel(2);
        ReadPoint( FALSE );
        ReadPoint( FALSE );
        *pOS2MET >> nLen;
    }
    else
    {
        if ( !bGivenPos )
            nLen = nOrderLen;
        else if ( bCoord32 )
            nLen = nOrderLen-8;
        else
            nLen = nOrderLen-4;
    }
    pChr = new char[nLen+1];
    for (i=0; i<nLen; i++)
        *pOS2MET >> pChr[i];
    pChr[nLen]=0;
    String aStr( String::CreateFromAscii( pChr ) );

    SetRasterOp(aAttr.eChrMix);
    if (pVirDev->GetFont()!=aFont)
        pVirDev->SetFont(aFont);
    pVirDev->DrawText(aP0,aStr);

    aSize = Size( pVirDev->GetTextWidth(aStr), pVirDev->GetTextHeight() );
    if ( aAttr.nChrAng == 0 )
    {
        aCalcBndRect.Union(Rectangle( Point(aP0.X(),aP0.Y()-aSize.Height()),
                                      Size(aSize.Width(),aSize.Height()*2)));
        if (bMove)
            aAttr.aCurPos = Point( aP0.X() + aSize.Width(), aP0.Y());
    }
    else
    {
        Polygon aDummyPoly(4);

        aDummyPoly.SetPoint( Point( aP0.X(), aP0.Y() ), 0);                                 // TOP LEFT
        aDummyPoly.SetPoint( Point( aP0.X(), aP0.Y() - aSize.Height() ), 1);                // BOTTOM LEFT
        aDummyPoly.SetPoint( Point( aP0.X() + aSize.Width(), aP0.Y() ), 2);                 // TOP RIGHT
        aDummyPoly.SetPoint( Point( aP0.X() + aSize.Width(), aP0.Y() - aSize.Height() ), 3);// BOTTOM RIGHT
        aDummyPoly.Rotate( aP0, (short)aAttr.nChrAng );
        if ( bMove )
            aAttr.aCurPos = aDummyPoly.GetPoint( 0 );
        aCalcBndRect.Union( Rectangle( aDummyPoly.GetPoint( 0 ), aDummyPoly.GetPoint( 3 ) ) );
        aCalcBndRect.Union( Rectangle( aDummyPoly.GetPoint( 1 ), aDummyPoly.GetPoint( 2 ) ) );
    }
    delete pChr;
}

void OS2METReader::ReadArc(BOOL bGivenPos)
{
    Point aP1, aP2, aP3;
    double x1,y1,x2,y2,x3,y3,p,q,cx,cy,ncx,ncy,r,rx,ry,w1,w3;
    if (bGivenPos) aP1=ReadPoint(); else aP1=aAttr.aCurPos;
    aP2=ReadPoint(); aP3=ReadPoint();
    aAttr.aCurPos=aP3;
    SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
    SetRasterOp(aAttr.eLinMix);
    // OK, gegeben sind 3 Punkte der Ellipse, und das Verhaeltnis
    // Breite zu Hoehe (als p zu q):
    x1=aP1.X(); y1=aP1.Y();
    x2=aP2.X(); y2=aP2.Y();
    x3=aP3.X(); y3=aP3.Y();
    p=aAttr.nArcP;q=aAttr.nArcQ;
    // Berechnet wird der Mittelpunkt cx,cy der Ellipse:
    ncy=2*p*p*((y3-y1)*(x2-x1)-(y1-y2)*(x1-x3));
    ncx=2*q*q*(x2-x1);
    if ( (ncx<0.001 && ncx>-0.001) || (ncy<0.001 && ncy>-0.001) ) {
        // Berechnung nicht moeglich, Punkte liegen auf einer Linie
        pVirDev->DrawLine(aP1,aP2);
        pVirDev->DrawLine(aP2,aP3);
        return;
    }
    cy=( q*q*((x3*x3-x1*x1)*(x2-x1)+(x2*x2-x1*x1)*(x1-x3)) +
         p*p*((y3*y3-y1*y1)*(x2-x1)+(y2*y2-y1*y1)*(x1-x3)) ) / ncy;
    cx=( q*q*(x2*x2-x1*x1)+p*p*(y2*y2-y1*y1)+cy*2*p*p*(y1-y2) ) / ncx;
    // Nun brauchen wir noch den Radius in x und y Richtung:
    r=sqrt(q*q*(x1-cx)*(x1-cx)+p*p*(y1-cy)*(y1-cy));
    rx=r/q; ry=r/p;
    // Jetzt stellt sich "nur noch" die Frage, wie Start- und Endpunkt
    // gewaehlt werden muessen, damit Punkt Nr. 2 innerhalb des
    // gezeichneten Bogens liegt:
    w1=fmod((atan2(x1-cx,y1-cy)-atan2(x2-cx,y2-cy)),6.28318530718); if (w1<0) w1+=6.28318530718;
    w3=fmod((atan2(x3-cx,y3-cy)-atan2(x2-cx,y2-cy)),6.28318530718); if (w3<0) w3+=6.28318530718;
    if (w3<w1) {
        pVirDev->DrawArc(Rectangle((long)(cx-rx),(long)(cy-ry),
                                   (long)(cx+rx),(long)(cy+ry)),aP1,aP3);
    }
    else {
        pVirDev->DrawArc(Rectangle((long)(cx-rx),(long)(cy-ry),
                                   (long)(cx+rx),(long)(cy+ry)),aP3,aP1);
    }
}

void OS2METReader::ReadFullArc(BOOL bGivenPos, USHORT nOrderSize)
{
    Point aCenter;
    long nP,nQ,nR,nS;
    Rectangle aRect;
    ULONG nMul; USHORT nMulS;

    if (bGivenPos) {
        aCenter=ReadPoint();
        if (bCoord32) nOrderSize-=8; else nOrderSize-=4;
    }
    else aCenter=aAttr.aCurPos;

    nP=aAttr.nArcP; nQ=aAttr.nArcQ; nR=aAttr.nArcR; nS=aAttr.nArcS;
    if (nP<0) nP=-nP;
    if (nQ<0) nQ=-nQ;
    if (nR<0) nR=-nR;
    if (nS<0) nS=-nS;
    if (nOrderSize>=4) *pOS2MET >> nMul;
    else { *pOS2MET >> nMulS; nMul=((ULONG)nMulS)<<8; }
    if (nMul!=0x00010000) {
        nP=(nP*nMul)>>16;
        nQ=(nQ*nMul)>>16;
        nR=(nR*nMul)>>16;
        nS=(nS*nMul)>>16;
    }

    aRect=Rectangle(aCenter.X()-nP,aCenter.Y()-nQ,
                    aCenter.X()+nP,aCenter.Y()+nQ);
    aCalcBndRect.Union(aRect);

    if (pAreaStack!=NULL) {
        ChangeBrush(aAttr.aPatCol,aAttr.aPatBgCol,aAttr.bFill);
        SetRasterOp(aAttr.ePatMix);
        if ((pAreaStack->nFlags&0x40)!=0)
            SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        else
            SetPen( COL_TRANSPARENT, 0, PEN_NULL );
    }
    else
    {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        ChangeBrush(Color( COL_TRANSPARENT ),Color( COL_TRANSPARENT ),FALSE);
        SetRasterOp(aAttr.eLinMix);
    }
    pVirDev->DrawEllipse(aRect);
}

void OS2METReader::ReadPartialArc(BOOL bGivenPos, USHORT nOrderSize)
{
    Point aP0, aCenter,aPStart,aPEnd;
    long nP,nQ,nR,nS,nStart, nSweep;
    Rectangle aRect;
    ULONG nMul; USHORT nMulS;
    double fStart, fEnd;

    if (bGivenPos) {
        aP0=ReadPoint();
        if (bCoord32) nOrderSize-=8; else nOrderSize-=4;
    }
    else aP0=aAttr.aCurPos;
    aCenter=ReadPoint();

    nP=aAttr.nArcP; nQ=aAttr.nArcQ; nR=aAttr.nArcR; nS=aAttr.nArcS;
    if (nP<0) nP=-nP;
    if (nQ<0) nQ=-nQ;
    if (nR<0) nR=-nR;
    if (nS<0) nS=-nS;
    if (nOrderSize>=12) *pOS2MET >> nMul;
    else { *pOS2MET >> nMulS; nMul=((ULONG)nMulS)<<8; }
    if (nMul!=0x00010000) {
        nP=(nP*nMul)>>16;
        nQ=(nQ*nMul)>>16;
        nR=(nR*nMul)>>16;
        nS=(nS*nMul)>>16;
    }

    *pOS2MET >> nStart >> nSweep;
    fStart=((double)nStart)/65536.0/180.0*3.14159265359;
    fEnd=fStart+((double)nSweep)/65536.0/180.0*3.14159265359;
    aPStart=Point(aCenter.X()+(long)( cos(fStart)*nP),
                  aCenter.Y()+(long)(-sin(fStart)*nQ));
    aPEnd=  Point(aCenter.X()+(long)( cos(fEnd)*nP),
                  aCenter.Y()+(long)(-sin(fEnd)*nQ));

    aRect=Rectangle(aCenter.X()-nP,aCenter.Y()-nQ,
                    aCenter.X()+nP,aCenter.Y()+nQ);
    aCalcBndRect.Union(aRect);

    SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
    SetRasterOp(aAttr.eLinMix);

    pVirDev->DrawLine(aP0,aPStart);
    pVirDev->DrawArc(aRect,aPStart,aPEnd);
    aAttr.aCurPos=aPEnd;
}

void OS2METReader::ReadPolygons()
{
    ULONG i,j,nNumPolys, nNumPoints;
    PolyPolygon aPolyPoly;
    Polygon aPoly;
    Point aPoint;
    BYTE nFlags;

    *pOS2MET >> nFlags >> nNumPolys;
    for (i=0; i<nNumPolys; i++) {
        *pOS2MET >> nNumPoints;
        if (i==0) nNumPoints++;
        aPoly.SetSize((short)nNumPoints);
        for (j=0; j<nNumPoints; j++) {
            if (i==0 && j==0) aPoint=aAttr.aCurPos;
            else aPoint=ReadPoint();
            aPoly.SetPoint(aPoint,(short)j);
            if (i==nNumPolys-1 && j==nNumPoints-1) aAttr.aCurPos=aPoint;
        }
        aPolyPoly.Insert(aPoly);
    }

    ChangeBrush(aAttr.aPatCol,aAttr.aPatBgCol,aAttr.bFill);
    SetRasterOp(aAttr.ePatMix);
    if ((nFlags&0x01)!=0)
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
    else
        SetPen( COL_TRANSPARENT, 0, PEN_NULL );
    DrawPolyPolygon( aPolyPoly );
}

void OS2METReader::ReadBezier(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i, nNumPoints = nOrderLen / ( bCoord32 ? 8 : 4 );

    if( !bGivenPos )
        nNumPoints++;

    if( !nNumPoints )
        return;

    Polygon aPolygon( nNumPoints );

    for( i=0; i < nNumPoints; i++ )
    {
        if( i==0 && !bGivenPos)
            aPolygon.SetPoint( aAttr.aCurPos, i );
        else
            aPolygon.SetPoint( ReadPoint(), i );
    }

    if( !( nNumPoints % 4 ) )
    {
        // create bezier polygon
        const USHORT    nSegPoints = 25;
        const USHORT    nSegments = aPolygon.GetSize() >> 2;
        Polygon         aBezPoly( nSegments * nSegPoints );

        USHORT nSeg, nBezPos, nStartPos;
        for( nSeg = 0, nBezPos = 0, nStartPos = 0; nSeg < nSegments; nSeg++, nStartPos += 4 )
        {
            const Polygon aSegPoly( aPolygon[ nStartPos ], aPolygon[ nStartPos + 1 ],
                                    aPolygon[ nStartPos + 3 ], aPolygon[ nStartPos + 2 ],
                                    nSegPoints );

            for( USHORT nSegPos = 0; nSegPos < nSegPoints; )
                aBezPoly[ nBezPos++ ] = aSegPoly[ nSegPos++ ];
        }

        nNumPoints = nBezPos;

        if( nNumPoints != aBezPoly.GetSize() )
            aBezPoly.SetSize( nNumPoints );

        aPolygon = aBezPoly;
    }

    aAttr.aCurPos = aPolygon[ nNumPoints - 1 ];

    if (pAreaStack!=NULL)
        AddPointsToArea(aPolygon);
    else if (pPathStack!=NULL)
        AddPointsToPath(aPolygon);
    else
    {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        SetRasterOp(aAttr.eLinMix);
        DrawPolyLine( aPolygon );
    }
}

void OS2METReader::ReadFillet(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i,nNumPoints;

    if (bCoord32) nNumPoints=nOrderLen/8; else nNumPoints=nOrderLen/4;
    if (!bGivenPos) nNumPoints++;
    if (nNumPoints==0) return;
    Polygon aPolygon(nNumPoints);
    for (i=0; i<nNumPoints; i++) {
        if (i==0 && !bGivenPos) aPolygon.SetPoint(aAttr.aCurPos,i);
        else aPolygon.SetPoint(ReadPoint(),i);
    }
    aAttr.aCurPos=aPolygon.GetPoint(nNumPoints-1);
    if (pAreaStack!=NULL) AddPointsToArea(aPolygon);
    else if (pPathStack!=NULL) AddPointsToPath(aPolygon);
    else {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        SetRasterOp(aAttr.eLinMix);
        DrawPolyLine( aPolygon );
    }
}

void OS2METReader::ReadFilletSharp(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i,nNumPoints;

    if (bGivenPos) {
        aAttr.aCurPos=ReadPoint();
        if (bCoord32) nOrderLen-=8; else nOrderLen-=4;
    }
    if (bCoord32) nNumPoints=1+nOrderLen/10;
    else nNumPoints=1+nOrderLen/6;
    Polygon aPolygon(nNumPoints);
    aPolygon.SetPoint(aAttr.aCurPos,0);
    for (i=1; i<nNumPoints; i++) aPolygon.SetPoint(ReadPoint(),i);
    aAttr.aCurPos=aPolygon.GetPoint(nNumPoints-1);
    if (pAreaStack!=NULL) AddPointsToArea(aPolygon);
    else if (pPathStack!=NULL) AddPointsToPath(aPolygon);
    else
    {
        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
        SetRasterOp(aAttr.eLinMix);
        DrawPolyLine( aPolygon );
    }
}

void OS2METReader::ReadMarker(BOOL bGivenPos, USHORT nOrderLen)
{
    USHORT i,nNumPoints;
    long x,y;

    SetPen( aAttr.aMrkCol );
    SetRasterOp(aAttr.eMrkMix);
    if (aAttr.nMrkSymbol>=5 && aAttr.nMrkSymbol<=9)
    {
        ChangeBrush(aAttr.aMrkCol,aAttr.aMrkCol,TRUE);
    }
    else
    {
        ChangeBrush(Color(COL_TRANSPARENT),Color(COL_TRANSPARENT),FALSE);
    }
    if (bCoord32) nNumPoints=nOrderLen/8; else nNumPoints=nOrderLen/4;
    if (!bGivenPos) nNumPoints++;
    for (i=0; i<nNumPoints; i++) {
        if (i!=0 || bGivenPos) aAttr.aCurPos=ReadPoint();
        x=aAttr.aCurPos.X(); y=aAttr.aCurPos.Y();
        aCalcBndRect.Union(Rectangle(x-5,y-5,x+5,y+5));
        switch (aAttr.nMrkSymbol) {
            case  2:   // PLUS
                pVirDev->DrawLine(Point(x-4,y),Point(x+4,y));
                pVirDev->DrawLine(Point(x,y-4),Point(x,y+4));
                break;
            case  3:   // DIAMOND
            case  7: { // SOLIDDIAMOND
                Polygon aPoly(4);
                aPoly.SetPoint(Point(x,y+4),0);
                aPoly.SetPoint(Point(x+4,y),1);
                aPoly.SetPoint(Point(x,y-4),2);
                aPoly.SetPoint(Point(x-4,y),3);
                pVirDev->DrawPolygon(aPoly);
                break;
            }
            case  4:   // SQARE
            case  8: { // SOLIDSUARE
                Polygon aPoly(4);
                aPoly.SetPoint(Point(x+4,y+4),0);
                aPoly.SetPoint(Point(x+4,y-4),1);
                aPoly.SetPoint(Point(x-4,y-4),2);
                aPoly.SetPoint(Point(x-4,y+4),3);
                pVirDev->DrawPolygon(aPoly);
                break;
            }
            case  5: { // SIXPOINTSTAR
                Polygon aPoly(12);
                aPoly.SetPoint(Point(x  ,y-4),0);
                aPoly.SetPoint(Point(x+2,y-2),1);
                aPoly.SetPoint(Point(x+4,y-2),2);
                aPoly.SetPoint(Point(x+2,y  ),3);
                aPoly.SetPoint(Point(x+4,y+2),4);
                aPoly.SetPoint(Point(x+2,y+2),5);
                aPoly.SetPoint(Point(x  ,y+4),6);
                aPoly.SetPoint(Point(x-2,y+2),7);
                aPoly.SetPoint(Point(x-4,y+2),8);
                aPoly.SetPoint(Point(x-2,y  ),9);
                aPoly.SetPoint(Point(x-4,y-2),10);
                aPoly.SetPoint(Point(x-2,y-2),11);
                pVirDev->DrawPolygon(aPoly);
                break;
            }
            case  6: { // EIGHTPOINTSTAR
                Polygon aPoly(16);
                aPoly.SetPoint(Point(x  ,y-4),0);
                aPoly.SetPoint(Point(x+1,y-2),1);
                aPoly.SetPoint(Point(x+3,y-3),2);
                aPoly.SetPoint(Point(x+2,y-1),3);
                aPoly.SetPoint(Point(x+4,y  ),4);
                aPoly.SetPoint(Point(x+2,y+1),5);
                aPoly.SetPoint(Point(x+3,y+3),6);
                aPoly.SetPoint(Point(x+1,y+2),7);
                aPoly.SetPoint(Point(x  ,y+4),8);
                aPoly.SetPoint(Point(x-1,y+2),9);
                aPoly.SetPoint(Point(x-3,y+3),10);
                aPoly.SetPoint(Point(x-2,y+1),11);
                aPoly.SetPoint(Point(x-4,y  ),12);
                aPoly.SetPoint(Point(x-2,y-1),13);
                aPoly.SetPoint(Point(x-3,y-3),14);
                aPoly.SetPoint(Point(x-1,y-2),15);
                pVirDev->DrawPolygon(aPoly);
                break;
            }
            case  9:   // DOT
                pVirDev->DrawEllipse(Rectangle(x-1,y-1,x+1,y+1));
                break;
            case 10:   // SMALLCIRCLE
                pVirDev->DrawEllipse(Rectangle(x-2,y-2,x+2,y+2));
                break;
            case 64:   // BLANK
                break;
            default:   // (=1) CROSS
                pVirDev->DrawLine(Point(x-4,y-4),Point(x+4,y+4));
                pVirDev->DrawLine(Point(x-4,y+4),Point(x+4,y-4));
                break;
        }
    }
}

void OS2METReader::ReadOrder(USHORT nOrderID, USHORT nOrderLen)
{
    switch (nOrderID) {

        case GOrdGivArc: ReadArc(TRUE); break;
        case GOrdCurArc: ReadArc(FALSE); break;

        case GOrdGivBzr: ReadBezier(TRUE,nOrderLen); break;
        case GOrdCurBzr: ReadBezier(FALSE,nOrderLen); break;

        case GOrdGivBox: ReadBox(TRUE); break;
        case GOrdCurBox: ReadBox(FALSE); break;

        case GOrdGivFil: ReadFillet(TRUE,nOrderLen); break;
        case GOrdCurFil: ReadFillet(FALSE,nOrderLen); break;

        case GOrdGivCrc: ReadFullArc(TRUE,nOrderLen); break;
        case GOrdCurCrc: ReadFullArc(FALSE,nOrderLen); break;

        case GOrdGivLin: ReadLine(TRUE, nOrderLen); break;
        case GOrdCurLin: ReadLine(FALSE, nOrderLen); break;

        case GOrdGivMrk: ReadMarker(TRUE, nOrderLen); break;
        case GOrdCurMrk: ReadMarker(FALSE, nOrderLen); break;

        case GOrdGivArP: ReadPartialArc(TRUE,nOrderLen); break;
        case GOrdCurArP: ReadPartialArc(FALSE,nOrderLen); break;

        case GOrdGivRLn: ReadRelLine(TRUE,nOrderLen); break;
        case GOrdCurRLn: ReadRelLine(FALSE,nOrderLen); break;

        case GOrdGivSFl: ReadFilletSharp(TRUE,nOrderLen); break;
        case GOrdCurSFl: ReadFilletSharp(FALSE,nOrderLen); break;

        case GOrdGivStM: ReadChrStr(TRUE , TRUE , FALSE, nOrderLen); break;
        case GOrdCurStM: ReadChrStr(FALSE, TRUE , FALSE, nOrderLen); break;
        case GOrdGivStr: ReadChrStr(TRUE , FALSE, FALSE, nOrderLen); break;
        case GOrdCurStr: ReadChrStr(FALSE, FALSE, FALSE, nOrderLen); break;
        case GOrdGivStx: ReadChrStr(TRUE , FALSE, TRUE , nOrderLen); break;
        case GOrdCurStx: ReadChrStr(FALSE, FALSE, TRUE , nOrderLen); break;

        case GOrdGivImg: OOODEBUG("GOrdGivImg",0);
            break;
        case GOrdCurImg: OOODEBUG("GOrdCurImg",0);
            break;
        case GOrdImgDat: OOODEBUG("GOrdImgDat",0);
            break;
        case GOrdEndImg: OOODEBUG("GOrdEndImg",0);
            break;

        case GOrdBegAra: {
            OSArea * p=new OSArea;
            p->bClosed=FALSE;
            p->pSucc=pAreaStack; pAreaStack=p;
            *pOS2MET >> (p->nFlags);
            p->aCol=aAttr.aPatCol;
            p->aBgCol=aAttr.aPatBgCol;
            p->eMix=aAttr.ePatMix;
            p->eBgMix=aAttr.ePatBgMix;
            p->bFill=aAttr.bFill;
            break;
        }
        case GOrdEndAra:
        {
            OSArea * p=pAreaStack;
            if ( p )
            {
                pAreaStack = p->pSucc;
                if ( pPathStack )
                {
                    for ( USHORT i=0; i<p->aPPoly.Count(); i++ )
                    {
                        AddPointsToPath( p->aPPoly.GetObject( i ) );
                        CloseFigure();
                    }
                }
                else
                {
                    if ( ( p->nFlags & 0x40 ) == 0 )
                        SetPen( COL_TRANSPARENT, 0, PEN_NULL );
                    else
                        SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );

                    ChangeBrush(p->aCol,p->aBgCol,p->bFill);
                    SetRasterOp(p->eMix);
                    DrawPolyPolygon( p->aPPoly );
                }
                delete p;
            }
        }
        break;

        case GOrdBegElm:// OOODEBUG("GOrdBegElm",0);
            break;
        case GOrdEndElm:// OOODEBUG("GOrdEndElm",0);
            break;

        case GOrdBegPth: {
            OSPath * p=new OSPath;
            p->pSucc=pPathStack; pPathStack=p;
            pOS2MET->SeekRel(2);
            *pOS2MET >> p->nID;
            p->bClosed=FALSE;
            p->bStroke=FALSE;
            break;
        }
        case GOrdEndPth: {
            OSPath * p, * pprev, * psucc;
            if (pPathStack==NULL) break;
            p=pPathList; pprev=NULL;
            while (p!=NULL) {
                psucc=p->pSucc;
                if (p->nID==pPathStack->nID) {
                    if (pprev==NULL) pPathList=psucc; else pprev->pSucc=psucc;
                    delete p;
                }
                else pprev=p;
                p=psucc;
            }
            p=pPathStack;
            pPathStack=p->pSucc;
            p->pSucc=pPathList; pPathList=p;
            break;
        }
        case GOrdFilPth:
        {
            ULONG   nID;
            UINT16  nDummy;
            OSPath* p = pPathList;

            *pOS2MET >> nDummy
                     >> nID;

            if ( ! ( nDummy & 0x20 ) )  // #30933# i do not know the exact meaning of this bit,
            {                           // but if set it seems to be better not to fill this path
                while( p && p->nID != nID )
                    p = p->pSucc;

                if( p )
                {
                    if( p->bStroke )
                    {
                        SetPen( aAttr.aPatCol, aAttr.nStrLinWidth, PEN_SOLID );
                        ChangeBrush(Color(COL_TRANSPARENT),Color(COL_TRANSPARENT),FALSE);
                        SetRasterOp( aAttr.ePatMix );
                        if ( IsLineInfo() )
                        {
                            for ( USHORT i = 0; i < p->aPPoly.Count(); i++ )
                                pVirDev->DrawPolyLine( p->aPPoly.GetObject( i ), aLineInfo );
                        }
                        else
                            pVirDev->DrawPolyPolygon( p->aPPoly );
                    }
                    else
                    {
                        SetPen( COL_TRANSPARENT, 0, PEN_NULL );
                        ChangeBrush( aAttr.aPatCol, aAttr.aPatBgCol, aAttr.bFill );
                        SetRasterOp( aAttr.ePatMix );
                        pVirDev->DrawPolyPolygon( p->aPPoly );
                    }
                }
            }
        }
        break;

        case GOrdModPth:
        {
            OSPath* p = pPathList;

            while( p && p->nID != 1 )
                p = p->pSucc;

            if( p )
                p->bStroke = TRUE;
        }
        break;

        case GOrdOutPth:
        {
            ULONG   nID;
            USHORT  i,nC;
            OSPath* p=pPathList;
            pOS2MET->SeekRel(2);
            *pOS2MET >> nID;
            while (p!=NULL && p->nID!=nID)
                p=p->pSucc;

            if( p!=NULL )
            {
                SetPen( aAttr.aLinCol, aAttr.nStrLinWidth, aAttr.eLinStyle );
                SetRasterOp(aAttr.eLinMix);
                ChangeBrush(Color(COL_TRANSPARENT),Color(COL_TRANSPARENT),FALSE);
                nC=p->aPPoly.Count();
                for (i=0; i<nC; i++)
                {
                    if (i+1<nC || p->bClosed==TRUE)
                        DrawPolygon( p->aPPoly.GetObject( i ) );
                    else
                        DrawPolyLine( p->aPPoly.GetObject( i ) );
                }
            }
            break;
        }
        case GOrdSClPth: {  OOODEBUG("GOrdSClPth",0);
            ULONG nID;
            OSPath * p=pPathList;
            pOS2MET->SeekRel(2);
            *pOS2MET >> nID;
            if (nID==0) p=NULL;
            while (p!=NULL && p->nID!=nID) p=p->pSucc;
            if (p!=NULL) pVirDev->SetClipRegion(Region(p->aPPoly));
            else pVirDev->SetClipRegion();
            break;
        }
        case GOrdNopNop:
            break;
        case GOrdRemark: //OOODEBUG("GOrdRemark",0);
            break;
        case GOrdSegLab: OOODEBUG("GOrdSegLab",0);
            break;

        case GOrdBitBlt: ReadBitBlt(); break;

        case GOrdCalSeg: OOODEBUG("GOrdCalSeg",0);
            break;
        case GOrdSSgBnd: OOODEBUG("GOrdSSgBnd",0);
            break;
        case GOrdSegChr: OOODEBUG("GOrdSegChr",0);
            break;
        case GOrdCloFig:
            CloseFigure();
            break;
        case GOrdEndSym: OOODEBUG("GOrdEndSym",0);
            break;
        case GOrdEndPlg: OOODEBUG("GOrdEndPlg",0);
            break;
        case GOrdEscape: OOODEBUG("GOrdEscape",0);
            break;
        case GOrdExtEsc: OOODEBUG("GOrdExtEsc",0);
            break;

        case GOrdPolygn: ReadPolygons(); break;

        case GOrdStkPop: PopAttr(); break;

        case GOrdPIvAtr: PushAttr(nOrderID);
        case GOrdSIvAtr: {
            BYTE nA, nP, nFlags, nMix;
            ULONG nVal;
            Color aCol;
            RasterOp eROP;
            *pOS2MET >> nA >> nP >> nFlags;
            if (nOrderID==GOrdPIvAtr) {
                pAttrStack->nIvAttrA=nA;
                pAttrStack->nIvAttrP=nP;
            }
            if (nA<=2) {
                if ((nFlags&0x80)!=0) {
                    if (nA==1) switch (nP) {
                        case 1: aAttr.aLinCol=aDefAttr.aLinCol; break;
                        case 2: aAttr.aChrCol=aDefAttr.aChrCol; break;
                        case 3: aAttr.aMrkCol=aDefAttr.aMrkCol; break;
                        case 4: aAttr.aPatCol=aDefAttr.aPatCol; break;
                        case 5: aAttr.aImgCol=aDefAttr.aImgCol; break;
                    }
                    else switch (nP) {
                        case 1: aAttr.aLinBgCol=aDefAttr.aLinBgCol; break;
                        case 2: aAttr.aChrBgCol=aDefAttr.aChrBgCol; break;
                        case 3: aAttr.aMrkBgCol=aDefAttr.aMrkBgCol; break;
                        case 4: aAttr.aPatBgCol=aDefAttr.aPatBgCol; break;
                        case 5: aAttr.aImgBgCol=aDefAttr.aImgBgCol; break;
                    }
                }
                else {
                    nVal=ReadLittleEndian3BytesLong();
                    if      ((nFlags&0x40)!=0 && nVal==1) aCol=Color(COL_BLACK);
                    else if ((nFlags&0x40)!=0 && nVal==2) aCol=Color(COL_WHITE);
                    else if ((nFlags&0x40)!=0 && nVal==4) aCol=Color(COL_WHITE);
                    else if ((nFlags&0x40)!=0 && nVal==5) aCol=Color(COL_BLACK);
                    else aCol=GetPaletteColor(nVal);
                    if (nA==1) switch (nP) {
                        case 1: aAttr.aLinCol=aCol; break;
                        case 2: aAttr.aChrCol=aCol; break;
                        case 3: aAttr.aMrkCol=aCol; break;
                        case 4: aAttr.aPatCol=aCol; break;
                        case 5: aAttr.aImgCol=aCol; break;
                    }
                    else switch (nP) {
                        case 1: aAttr.aLinBgCol=aCol; break;
                        case 2: aAttr.aChrBgCol=aCol; break;
                        case 3: aAttr.aMrkBgCol=aCol; break;
                        case 4: aAttr.aPatBgCol=aCol; break;
                        case 5: aAttr.aImgBgCol=aCol; break;
                    }
                }
            }
            else {
                *pOS2MET >> nMix;
                if (nMix==0) {
                    if (nA==1) switch (nP) {
                        case 1: aAttr.eLinMix=aDefAttr.eLinMix; break;
                        case 2: aAttr.eChrMix=aDefAttr.eChrMix; break;
                        case 3: aAttr.eMrkMix=aDefAttr.eMrkMix; break;
                        case 4: aAttr.ePatMix=aDefAttr.ePatMix; break;
                        case 5: aAttr.eImgMix=aDefAttr.eImgMix; break;
                    }
                    else switch (nP) {
                        case 1: aAttr.eLinBgMix=aDefAttr.eLinBgMix; break;
                        case 2: aAttr.eChrBgMix=aDefAttr.eChrBgMix; break;
                        case 3: aAttr.eMrkBgMix=aDefAttr.eMrkBgMix; break;
                        case 4: aAttr.ePatBgMix=aDefAttr.ePatBgMix; break;
                        case 5: aAttr.eImgBgMix=aDefAttr.eImgBgMix; break;
                    }
                }
                else {
                    eROP=OS2MixToRasterOp(nMix);
                    if (nA==1) switch (nP) {
                        case 1: aAttr.eLinMix=eROP; break;
                        case 2: aAttr.eChrMix=eROP; break;
                        case 3: aAttr.eMrkMix=eROP; break;
                        case 4: aAttr.ePatMix=eROP; break;
                        case 5: aAttr.eImgMix=eROP; break;
                    }
                    else switch (nP) {
                        case 1: aAttr.eLinBgMix=eROP; break;
                        case 2: aAttr.eChrBgMix=eROP; break;
                        case 3: aAttr.eMrkBgMix=eROP; break;
                        case 4: aAttr.ePatBgMix=eROP; break;
                        case 5: aAttr.eImgBgMix=eROP; break;
                    }
                }
            }
            break;
        }
        case GOrdPIxCol: PushAttr(nOrderID);
        case GOrdSIxCol: {
            BYTE nFlags;
            ULONG nVal;
            Color aCol;
            *pOS2MET >> nFlags;
            if ((nFlags&0x80)!=0) {
                aAttr.aLinCol=aDefAttr.aLinCol;
                aAttr.aChrCol=aDefAttr.aChrCol;
                aAttr.aMrkCol=aDefAttr.aMrkCol;
                aAttr.aPatCol=aDefAttr.aPatCol;
                aAttr.aImgCol=aDefAttr.aImgCol;
            }
            else {
                nVal=ReadLittleEndian3BytesLong();
                if      ((nFlags&0x40)!=0 && nVal==1) aCol=Color(COL_BLACK);
                else if ((nFlags&0x40)!=0 && nVal==2) aCol=Color(COL_WHITE);
                else if ((nFlags&0x40)!=0 && nVal==4) aCol=Color(COL_WHITE);
                else if ((nFlags&0x40)!=0 && nVal==5) aCol=Color(COL_BLACK);
                else aCol=GetPaletteColor(nVal);
                aAttr.aLinCol = aAttr.aChrCol = aAttr.aMrkCol = aAttr.aPatCol =
                aAttr.aImgCol = aCol;
            }
            break;
        }

        case GOrdPColor:
        case GOrdPXtCol: PushAttr(nOrderID);
        case GOrdSColor:
        case GOrdSXtCol: {
            BYTE nbyte;
            USHORT nVal;
            Color aCol;
            if (nOrderID==GOrdPColor || nOrderID==GOrdSColor) {
                *pOS2MET >> nbyte; nVal=((USHORT)nbyte)|0xff00;
            }
            else *pOS2MET >> nVal;
            if (nVal==0x0000 || nVal==0xff00)  {
                aAttr.aLinCol=aDefAttr.aLinCol;
                aAttr.aChrCol=aDefAttr.aChrCol;
                aAttr.aMrkCol=aDefAttr.aMrkCol;
                aAttr.aPatCol=aDefAttr.aPatCol;
                aAttr.aImgCol=aDefAttr.aImgCol;
            }
            else {
                if      (nVal==0x0007) aCol=Color(COL_WHITE);
                else if (nVal==0x0008) aCol=Color(COL_BLACK);
                else if (nVal==0xff08) aCol=GetPaletteColor(1);
                else aCol=GetPaletteColor(((ULONG)nVal) & 0x000000ff);
                aAttr.aLinCol = aAttr.aChrCol = aAttr.aMrkCol = aAttr.aPatCol =
                aAttr.aImgCol = aCol;
            }
            break;
        }

        case GOrdPBgCol: PushAttr(nOrderID);
        case GOrdSBgCol: {
            USHORT nVal;
            Color aCol;
            *pOS2MET >> nVal;
            if (nVal==0x0000 || nVal==0xff00)  {
                aAttr.aLinBgCol=aDefAttr.aLinBgCol;
                aAttr.aChrBgCol=aDefAttr.aChrBgCol;
                aAttr.aMrkBgCol=aDefAttr.aMrkBgCol;
                aAttr.aPatBgCol=aDefAttr.aPatBgCol;
                aAttr.aImgBgCol=aDefAttr.aImgBgCol;
            }
            else {
                if      (nVal==0x0007) aCol=Color(COL_WHITE);
                else if (nVal==0x0008) aCol=Color(COL_BLACK);
                else if (nVal==0xff08) aCol=GetPaletteColor(0);
                else aCol=GetPaletteColor(((ULONG)nVal) & 0x000000ff);
                aAttr.aLinBgCol = aAttr.aChrBgCol = aAttr.aMrkBgCol =
                aAttr.aPatBgCol = aAttr.aImgBgCol = aCol;
            }
            break;
        }
        case GOrdPBxCol: PushAttr(nOrderID);
        case GOrdSBxCol: {
            BYTE nFlags;
            ULONG nVal;
            Color aCol;
            *pOS2MET >> nFlags;
            if ((nFlags&0x80)!=0) {
                aAttr.aLinBgCol=aDefAttr.aLinBgCol;
                aAttr.aChrBgCol=aDefAttr.aChrBgCol;
                aAttr.aMrkBgCol=aDefAttr.aMrkBgCol;
                aAttr.aPatBgCol=aDefAttr.aPatBgCol;
                aAttr.aImgBgCol=aDefAttr.aImgBgCol;
            }
            else {
                nVal=ReadLittleEndian3BytesLong();
                if      ((nFlags&0x40)!=0 && nVal==1) aCol=Color(COL_BLACK);
                else if ((nFlags&0x40)!=0 && nVal==2) aCol=Color(COL_WHITE);
                else if ((nFlags&0x40)!=0 && nVal==4) aCol=Color(COL_WHITE);
                else if ((nFlags&0x40)!=0 && nVal==5) aCol=Color(COL_BLACK);
                else aCol=GetPaletteColor(nVal);
                aAttr.aLinBgCol = aAttr.aChrBgCol = aAttr.aMrkBgCol =
                aAttr.aPatBgCol = aAttr.aImgBgCol = aCol;
            }
            break;
        }

        case GOrdPMixMd: PushAttr(nOrderID);
        case GOrdSMixMd: {
            BYTE nMix;
            *pOS2MET >> nMix;
            if (nMix==0) {
                aAttr.eLinMix=aDefAttr.eLinMix;
                aAttr.eChrMix=aDefAttr.eChrMix;
                aAttr.eMrkMix=aDefAttr.eMrkMix;
                aAttr.ePatMix=aDefAttr.ePatMix;
                aAttr.eImgMix=aDefAttr.eImgMix;
            }
            else {
                aAttr.eLinMix = aAttr.eChrMix = aAttr.eMrkMix =
                aAttr.ePatMix = aAttr.eImgMix = OS2MixToRasterOp(nMix);
            }
            break;
        }
        case GOrdPBgMix: PushAttr(nOrderID);
        case GOrdSBgMix: {
            BYTE nMix;
            *pOS2MET >> nMix;
            if (nMix==0) {
                aAttr.eLinBgMix=aDefAttr.eLinBgMix;
                aAttr.eChrBgMix=aDefAttr.eChrBgMix;
                aAttr.eMrkBgMix=aDefAttr.eMrkBgMix;
                aAttr.ePatBgMix=aDefAttr.ePatBgMix;
                aAttr.eImgBgMix=aDefAttr.eImgBgMix;
            }
            else {
                aAttr.eLinBgMix = aAttr.eChrBgMix = aAttr.eMrkBgMix =
                aAttr.ePatBgMix = aAttr.eImgBgMix = OS2MixToRasterOp(nMix);
            }
            break;
        }
        case GOrdPPtSet: PushAttr(nOrderID);
        case GOrdSPtSet: OOODEBUG("GOrdSPtSet",0);
            break;

        case GOrdPPtSym: PushAttr(nOrderID);
        case GOrdSPtSym: {
            BYTE nPatt;
            *pOS2MET >> nPatt;
            aAttr.bFill = ( nPatt != 0x0f );
            break;
        }

        case GOrdPPtRef: PushAttr(nOrderID);
        case GOrdSPtRef: OOODEBUG("GOrdSPtRef",0);
            break;

        case GOrdPLnEnd: PushAttr(nOrderID);
        case GOrdSLnEnd:
            break;

        case GOrdPLnJoi: PushAttr(nOrderID);
        case GOrdSLnJoi:
            break;

        case GOrdPLnTyp: PushAttr(nOrderID);
        case GOrdSLnTyp: {
            BYTE nType;
            *pOS2MET >> nType;
            switch (nType) {
                case 0:         aAttr.eLinStyle=aDefAttr.eLinStyle; break;
                case 1: case 4: aAttr.eLinStyle=PEN_DOT; break;
                case 2: case 5: aAttr.eLinStyle=PEN_DASH; break;
                case 3: case 6: aAttr.eLinStyle=PEN_DASHDOT; break;
                case 8:         aAttr.eLinStyle=PEN_NULL; break;
                default:        aAttr.eLinStyle=PEN_SOLID;
            }
            break;
        }
        case GOrdPLnWdt: PushAttr(nOrderID);
        case GOrdSLnWdt: {
            BYTE nbyte;
            *pOS2MET >> nbyte;
            if (nbyte==0) aAttr.nLinWidth=aDefAttr.nLinWidth;
            else aAttr.nLinWidth=(USHORT)nbyte-1;
            break;
        }
        case GOrdPFrLWd: PushAttr(nOrderID);
        case GOrdSFrLWd:
            break;

        case GOrdPStLWd: PushAttr(nOrderID);
        case GOrdSStLWd :
        {
            BYTE nFlags;
            long nWd;

            *pOS2MET >> nFlags;
            if ( nFlags & 0x80 )
                aAttr.nStrLinWidth = aDefAttr.nStrLinWidth;
            else
            {
                pOS2MET->SeekRel( 1 );
                nWd = ReadCoord( bCoord32 );
                if ( nWd < 0 )
                    nWd = -nWd;
                aAttr.nStrLinWidth = (USHORT)nWd;
            }
            break;
        }
        case GOrdPChDir: PushAttr(nOrderID);
        case GOrdSChDir:
            break;

        case GOrdPChPrc: PushAttr(nOrderID);
        case GOrdSChPrc:
            break;

        case GOrdPChSet: PushAttr(nOrderID);
        case GOrdSChSet: {
            BYTE nbyte; *pOS2MET >> nbyte;
            aAttr.nChrSet=((ULONG)nbyte)&0xff;
            break;
        }
        case GOrdPChAng: PushAttr(nOrderID);
        case GOrdSChAng: {
            long nX,nY;
            nX=ReadCoord(bCoord32); nY=ReadCoord(bCoord32);
            if (nX>=0 && nY==0) aAttr.nChrAng=0;
            else {
                aAttr.nChrAng=(short)(atan2((double)nY,(double)nX)/3.1415926539*1800.0);
                while (aAttr.nChrAng<0) aAttr.nChrAng+=3600;
                aAttr.nChrAng%=3600;
            }
            break;
        }
        case GOrdPChBrx: PushAttr(nOrderID);
        case GOrdSChBrx:
            break;

        case GOrdPChCel: PushAttr(nOrderID);
        case GOrdSChCel: {
            BYTE nbyte;
            USHORT nLen=nOrderLen;
            aAttr.aChrCellSize.Width()=ReadCoord(bCoord32);
            aAttr.aChrCellSize.Height()=ReadCoord(bCoord32);
            if (bCoord32) nLen-=8; else nLen-=4;
            if (nLen>=4) {
                pOS2MET->SeekRel(4); nLen-=4;
            }
            if (nLen>=2) {
                *pOS2MET >> nbyte;
                if ((nbyte&0x80)==0 && aAttr.aChrCellSize==Size(0,0))
                    aAttr.aChrCellSize=aDefAttr.aChrCellSize;
            }
            break;
        }
        case GOrdPChXtr: PushAttr(nOrderID);
        case GOrdSChXtr:
            break;

        case GOrdPChShr: PushAttr(nOrderID);
        case GOrdSChShr:
            break;

        case GOrdPTxAlg: PushAttr(nOrderID);
        case GOrdSTxAlg: OOODEBUG("GOrdSTxAlg",0);
            break;

        case GOrdPMkPrc: PushAttr(nOrderID);
        case GOrdSMkPrc: {
            BYTE nbyte;
            *pOS2MET >> nbyte;
            if (nbyte==0) aAttr.nMrkPrec=aDefAttr.nMrkPrec;
            else aAttr.nMrkPrec=nbyte;
            break;
        }

        case GOrdPMkSet: PushAttr(nOrderID);
        case GOrdSMkSet: {
            BYTE nbyte;
            *pOS2MET >> nbyte;
            if (nbyte==0) aAttr.nMrkSet=aDefAttr.nMrkSet;
            else aAttr.nMrkSet=nbyte;
            break;
        }

        case GOrdPMkSym: PushAttr(nOrderID);
        case GOrdSMkSym: {
            BYTE nbyte;
            *pOS2MET >> nbyte;
            if (nbyte==0) aAttr.nMrkSymbol=aDefAttr.nMrkSymbol;
            else aAttr.nMrkSymbol=nbyte;
            break;
        }

        case GOrdPMkCel: PushAttr(nOrderID);
        case GOrdSMkCel: {
            BYTE nbyte;
            USHORT nLen=nOrderLen;
            aAttr.aMrkCellSize.Width()=ReadCoord(bCoord32);
            aAttr.aMrkCellSize.Height()=ReadCoord(bCoord32);
            if (bCoord32) nLen-=8; else nLen-=4;
            if (nLen>=2) {
                *pOS2MET >> nbyte;
                if ((nbyte&0x80)==0 && aAttr.aMrkCellSize==Size(0,0))
                    aAttr.aMrkCellSize=aDefAttr.aMrkCellSize;
            }
            break;
        }

        case GOrdPArcPa: PushAttr(nOrderID);
        case GOrdSArcPa:
            aAttr.nArcP=ReadCoord(bCoord32);
            aAttr.nArcQ=ReadCoord(bCoord32);
            aAttr.nArcR=ReadCoord(bCoord32);
            aAttr.nArcS=ReadCoord(bCoord32);
            break;

        case GOrdPCrPos: PushAttr(nOrderID);
        case GOrdSCrPos:
            aAttr.aCurPos=ReadPoint();
            break;

        case GOrdPMdTrn: PushAttr(nOrderID);
        case GOrdSMdTrn: OOODEBUG("GOrdSMdTrn",0);
            break;

        case GOrdPPkIdn: PushAttr(nOrderID);
        case GOrdSPkIdn: OOODEBUG("GOrdSPkIdn",0);
            break;

        case GOrdSVwTrn: OOODEBUG("GOrdSVwTrn",0);
            break;

        case GOrdPVwWin: PushAttr(nOrderID);
        case GOrdSVwWin: OOODEBUG("GOrdSVwWin",0);
            break;
        default: OOODEBUG("Order unbekannt:",nOrderID);
    }
}

void OS2METReader::ReadDsc(USHORT nDscID, USHORT /*nDscLen*/)
{
    switch (nDscID) {
        case 0x00f7: { // 'Specify GVM Subset'
            BYTE nbyte;
            pOS2MET->SeekRel(6);
            *pOS2MET >> nbyte;
            if      (nbyte==0x05) bCoord32=TRUE;
            else if (nbyte==0x04) bCoord32=FALSE;
            else {
                pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                ErrorCode=1;
            }
            break;
        }
        case 0x00f6:
        {
            // 'Set Picture Descriptor'
            BOOL b32;
            BYTE nbyte,nUnitType;
            long x1,y1,x2,y2,nt,xr,yr;

            pOS2MET->SeekRel(2);
            *pOS2MET >> nbyte;

            if (nbyte==0x05)
                b32=TRUE;
            else if(nbyte==0x04)
                b32=FALSE;
            else
            {
                pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                ErrorCode=2;
            }

            *pOS2MET >> nUnitType;

            xr=ReadCoord(b32);
            yr=ReadCoord(b32);

            ReadCoord(b32);

            if (nUnitType==0x00 && xr>0 && yr>0)
                aGlobMapMode=MapMode(MAP_INCH,Point(0,0),Fraction(10,xr),Fraction(10,yr));
            else if (nUnitType==0x01 && xr>0 && yr>0)
                aGlobMapMode=MapMode(MAP_CM,Point(0,0),Fraction(10,xr),Fraction(10,yr));
            else
                aGlobMapMode=MapMode();

            x1=ReadCoord(b32);
            x2=ReadCoord(b32);
            y1=ReadCoord(b32);
            y2=ReadCoord(b32);

            if (x1>x2)
            {
                nt=x1;
                x1=x2;
                x2=nt;
            }

            if (y1>y2)
            {
                nt=y1;
                y1=y2;
                y2=nt;
            }

            aBoundingRect.Left() = x1;
            aBoundingRect.Right() = x2;
            aBoundingRect.Top() = y1;
            aBoundingRect.Bottom() = y2;

            // no output beside this bounding rect
            pVirDev->IntersectClipRegion( Rectangle( Point(), aBoundingRect.GetSize() ) );

            break;
        }
        case 0x0021: // 'Set Current Defaults'
            break;
    }
}

void OS2METReader::ReadImageData(USHORT nDataID, USHORT nDataLen)
{
    OSBitmap * p=pBitmapList; if (p==NULL) return; // Nanu ?

    switch (nDataID) {

        case 0x0070:   // Begin Segment
            break;

        case 0x0091:   // Begin Image Content
            break;

        case 0x0094:   // Image Size
            pOS2MET->SeekRel(5);
            p->nHeight=ReadBigEndianWord();
            p->nWidth=ReadBigEndianWord();
            break;

        case 0x0095:   // Image Encoding
            break;

        case 0x0096: { // Image IDE-Size
            BYTE nbyte;
            *pOS2MET >> nbyte; p->nBitsPerPixel=nbyte;
            break;
        }

        case 0x0097:   // Image LUT-ID
            break;

        case 0x009b:   // IDE Structure
            break;

        case 0xfe92: { // Image Data
            // Spaetestens jetzt brauchen wir die temporaere BMP-Datei
            // und darin mindestens den Header + Palette.
            if (p->pBMP==NULL) {
                p->pBMP=new SvMemoryStream();
                p->pBMP->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                if (p->nWidth==0 || p->nHeight==0 || p->nBitsPerPixel==0) {
                    pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                    ErrorCode=3;
                    return;
                }
                // Schreibe (Windows-)BITMAPINFOHEADER:
                *(p->pBMP) << ((ULONG)40) << p->nWidth << p->nHeight;
                *(p->pBMP) << ((USHORT)1) << p->nBitsPerPixel;
                *(p->pBMP) << ((ULONG)0) << ((ULONG)0) << ((ULONG)0) << ((ULONG)0);
                *(p->pBMP) << ((ULONG)0) << ((ULONG)0);
                // Schreibe Farbtabelle:
                if (p->nBitsPerPixel<=8) {
                    USHORT i, nColTabSize=1<<(p->nBitsPerPixel);
                    for (i=0; i<nColTabSize; i++) *(p->pBMP) << GetPalette0RGB(i);
                }
            }
            // OK, nun werden die Map-Daten ruebergeschoben. Leider haben OS2 und
            // BMP eine unterschiedliche Reihenfolge von RGB bei 24-Bit.
            BYTE * pBuf=new BYTE[nDataLen];
            pOS2MET->Read(pBuf,nDataLen);
            if (p->nBitsPerPixel==24) {
                ULONG i, j, nAlign, nBytesPerLine;
                BYTE nTemp;
                nBytesPerLine=(p->nWidth*3+3)&0xfffffffc;
                nAlign=p->nMapPos-(p->nMapPos % nBytesPerLine);
                i=0;
                while (nAlign+i+2<p->nMapPos+nDataLen) {
                    if (nAlign+i>=p->nMapPos) {
                        j=nAlign+i-p->nMapPos;
                        nTemp=pBuf[j]; pBuf[j]=pBuf[j+2]; pBuf[j+2]=nTemp;
                    }
                    i+=3; if (i+2>=nBytesPerLine) {
                        nAlign+=nBytesPerLine;
                        i=0;
                    }
                }
            }
            p->pBMP->Write(pBuf,nDataLen);
            p->nMapPos+=nDataLen;
            delete pBuf;
            break;
        }
        case 0x0093:   // End Image Content
            break;

        case 0x0071:   // End Segment
            break;
    }
}

void OS2METReader::ReadFont(USHORT nFieldSize)
{
    ULONG nPos, nMaxPos;
    USHORT nLen,i;
    BYTE nbyte, nTripType, nTripType2;
    OSFont * pF=new OSFont;
    pF->pSucc=pFontList; pFontList=pF;
    pF->nID=0;
    pF->aFont.SetTransparent(TRUE);
    pF->aFont.SetAlign(ALIGN_BASELINE);

    nPos=pOS2MET->Tell();
    nMaxPos=nPos+(ULONG)nFieldSize;
    pOS2MET->SeekRel(2); nPos+=2;
    while (nPos<nMaxPos && pOS2MET->GetError()==0) {
        *pOS2MET >> nbyte; nLen =((USHORT)nbyte) & 0x00ff;
        *pOS2MET >> nTripType;
        switch (nTripType) {
            case 0x02:
                *pOS2MET >> nTripType2;
                switch (nTripType2) {
                    case 0x84:   // Font name
                        break;
                    case 0x08: { // Font Typeface
                        char str[33];
                        String aStr;
                        pOS2MET->SeekRel(1);
                        for (i=0; i<32; i++) *pOS2MET >> str[i];
                        str[32]=0;
                        aStr = String::CreateFromAscii( str );
                        if ( aStr.CompareIgnoreCaseToAscii( "Helv" ) == COMPARE_EQUAL )
                            aStr = String::CreateFromAscii( "Helvetica" );
                        pF->aFont.SetName( aStr );
                        break;
                    }
                }
                break;
            case 0x24:   // Icid
                *pOS2MET >> nTripType2;
                switch (nTripType2) {
                    case 0x05:   //Icid
                        *pOS2MET >> nbyte;
                        pF->nID=((ULONG)nbyte)&0xff;
                        break;
                }
                break;
            case 0x20:   // Font Binary GCID
                break;
            case 0x1f: { // Font Attributes
                FontWeight eWeight;
                BYTE nbyte;
                *pOS2MET >> nbyte;
                switch (nbyte) {
                    case 1:  eWeight=WEIGHT_THIN;       break;
                    case 2:  eWeight=WEIGHT_ULTRALIGHT; break;
                    case 3:  eWeight=WEIGHT_LIGHT;      break;
                    case 4:  eWeight=WEIGHT_SEMILIGHT;  break;
                    case 5:  eWeight=WEIGHT_NORMAL;     break;
                    case 6:  eWeight=WEIGHT_SEMIBOLD;   break;
                    case 7:  eWeight=WEIGHT_BOLD;       break;
                    case 8:  eWeight=WEIGHT_ULTRABOLD;  break;
                    case 9:  eWeight=WEIGHT_BLACK;      break;
                    default: eWeight=WEIGHT_DONTKNOW;
                }
                pF->aFont.SetWeight(eWeight);
                break;
            }
        }
        nPos+=nLen; pOS2MET->Seek(nPos);
    }
}

void OS2METReader::ReadField(USHORT nFieldType, USHORT nFieldSize)
{
    switch (nFieldType) {
        case BegDocumnMagic:
            break;
        case EndDocumnMagic:
            break;
        case BegResGrpMagic:
            break;
        case EndResGrpMagic:
            break;
        case BegColAtrMagic:
            break;
        case EndColAtrMagic:
            break;
        case BlkColAtrMagic: {
            ULONG nPos, nMaxPos;
            BYTE nbyte;
            ULONG nCol;
            USHORT nStartIndex, nEndIndex, i, nElemLen, nBytesPerCol;

            nPos=pOS2MET->Tell();
            nMaxPos=nPos+(ULONG)nFieldSize;
            pOS2MET->SeekRel(3); nPos+=3;
            while (nPos<nMaxPos && pOS2MET->GetError()==0) {
                *pOS2MET >> nbyte; nElemLen=((USHORT)nbyte) & 0x00ff;
                if (nElemLen>11) {
                    pOS2MET->SeekRel(4);
                    nStartIndex=ReadBigEndianWord();
                    pOS2MET->SeekRel(3);
                    *pOS2MET >> nbyte; nBytesPerCol=((USHORT)nbyte) & 0x00ff;
                    nEndIndex=nStartIndex+(nElemLen-11)/nBytesPerCol;
                    for (i=nStartIndex; i<nEndIndex; i++) {
                        if (nBytesPerCol > 3) pOS2MET->SeekRel(nBytesPerCol-3);
                        nCol=ReadBigEndian3BytesLong();
                        SetPalette0RGB(i,nCol);
                    }
                }
                else if (nElemLen<10) {
                    pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                    ErrorCode=4;
                }
                nPos+=(ULONG)nElemLen;
                pOS2MET->Seek(nPos);
            }
            break;
        }
        case MapColAtrMagic:
            break;
        case BegImgObjMagic: {
            // neue Bitmap schonmal herstellen: (wird spaeter gefuellt)
            OSBitmap * pB=new OSBitmap;
            pB->pSucc=pBitmapList; pBitmapList=pB;
            pB->pBMP=NULL; pB->nWidth=0; pB->nHeight=0; pB->nBitsPerPixel=0;
            pB->nMapPos=0;
            // ID der Bitmap ermitteln:
            BYTE i,nbyte,nbyte2;
            pB->nID=0;
            for (i=0; i<4; i++) {
                *pOS2MET >> nbyte >> nbyte2;
                nbyte=((nbyte-0x30)<<4)|(nbyte2-0x30);
                pB->nID=(pB->nID>>8)|(((ULONG)nbyte)<<24);
            }
            // neue Palette auf den Paletten-Stack bringen: (wird spaeter gefuellt)
            OSPalette * pP=new OSPalette;
            pP->pSucc=pPaletteStack; pPaletteStack=pP;
            pP->p0RGB=NULL; pP->nSize=0;
            break;
        }
        case EndImgObjMagic: {
            // Temporaere Windows-BMP-Datei auslesen:
            if (pBitmapList==NULL || pBitmapList->pBMP==NULL ||
                pBitmapList->pBMP->GetError()!=0) {
                pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                ErrorCode=5;
                return;
            }
            pBitmapList->pBMP->Seek(0);

            pBitmapList->aBitmap.Read( *( pBitmapList->pBMP ), FALSE );

            if (pBitmapList->pBMP->GetError()!=0) {
                pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                ErrorCode=6;
            }
            delete pBitmapList->pBMP; pBitmapList->pBMP=NULL;
            // Palette vom Stack killen:
            OSPalette * pP=pPaletteStack;
            if (pP!=NULL) {
                pPaletteStack=pP->pSucc;
                if (pP->p0RGB!=NULL) delete pP->p0RGB;
                delete pP;
            }
            break;
        }
        case DscImgObjMagic:
            break;
        case DatImgObjMagic: {
            USHORT nDataID, nDataLen;
            BYTE nbyte;
            ULONG nPos, nMaxPos;

            nPos=pOS2MET->Tell();
            nMaxPos=nPos+(ULONG)nFieldSize;
            while (nPos<nMaxPos && pOS2MET->GetError()==0) {
                *pOS2MET >> nbyte; nDataID=((USHORT)nbyte)&0x00ff;
                if (nDataID==0x00fe) {
                    *pOS2MET >> nbyte;
                    nDataID=(nDataID<<8)|(((USHORT)nbyte)&0x00ff);
                    nDataLen=ReadBigEndianWord();
                    nPos+=4;
                }
                else {
                    *pOS2MET >> nbyte; nDataLen=((USHORT)nbyte)&0x00ff;
                    nPos+=2;
                }
                ReadImageData(nDataID, nDataLen);
                nPos+=(ULONG)nDataLen;
                pOS2MET->Seek(nPos);
            }
            break;
        }

        case BegObEnv1Magic:
            break;
        case EndObEnv1Magic:
            break;
        case BegGrfObjMagic:
            break;
        case EndGrfObjMagic: {
            SvStream * pSave;
            ULONG nPos, nMaxPos;
            USHORT nOrderID, nOrderLen;
            BYTE nbyte;

            if (pOrdFile==NULL) break;

            // in pOrdFile wurden alle "DatGrfObj"-Felder gesammelt, so
            // dass die darin enthaltnen "Orders" zusammenhangend und nicht durch
            // "Fields" segmentiert sind. Um sie aus dem MemoryStream auszulesen,
            // ohne grosse Umstaende deswegen zu haben (frueher wurden die "Orders"
            // direkt aus pOS2MET gelesen), hier ein kleiner Trick:
            pSave=pOS2MET;
            pOS2MET=pOrdFile; //(!)
            nMaxPos=pOS2MET->Tell();
            pOS2MET->Seek(0);

            // "Segmentheader":
            *pOS2MET >> nbyte;
            if (nbyte==0x70) { // Header vorhanden
                pOS2MET->SeekRel(15); // brauchen wir aber nicht
            }
            else pOS2MET->SeekRel(-1); // Kein Header, Byte zurueck

            // Schleife ueber Order:
            while (pOS2MET->Tell()<nMaxPos && pOS2MET->GetError()==0) {
                *pOS2MET >> nbyte; nOrderID=((USHORT)nbyte) & 0x00ff;
                if (nOrderID==0x00fe) {
                    *pOS2MET >> nbyte;
                    nOrderID=(nOrderID << 8) | (((USHORT)nbyte) & 0x00ff);
                }
                if (nOrderID>0x00ff || nOrderID==GOrdPolygn) {
                    // ooo: Laut OS2-Doku sollte die Orderlaenge nun als Big-Endian-Word
                    // gegeben sein (Zitat: "Highorder byte precedes loworder byte").
                    // Tatsaechlich gibt es aber Dateien, die die Laenge als
                    // Little-Endian-Word angeben (zu mindestens fuer nOrderID==GOrdPolygn).
                    // Also werfen wir eine Muenze oder was ?
                    *pOS2MET >> nbyte; nOrderLen=(USHORT)nbyte&0x00ff;
                    *pOS2MET >> nbyte; if (nbyte!=0) nOrderLen=nOrderLen<<8|(((USHORT)nbyte)&0x00ff);
                }
                else if (nOrderID==GOrdSTxAlg || nOrderID==GOrdPTxAlg) nOrderLen=2;
                else if ((nOrderID&0xff88)==0x0008) nOrderLen=1;
                else if (nOrderID==0x0000 || nOrderID==0x00ff) nOrderLen=0;
                else { *pOS2MET >> nbyte; nOrderLen=((USHORT)nbyte) & 0x00ff; }
                nPos=pOS2MET->Tell();
                ReadOrder(nOrderID, nOrderLen);
                if (nPos+nOrderLen < pOS2MET->Tell()) {
                    OOODEBUG("Order kuerzer als er denkt! OrderID:",nOrderID);
                    OOODEBUG("...und zwar bei Position (Parameteranfang):",nPos);
                }
                else if (nPos+nOrderLen != pOS2MET->Tell()) {
                    OOODEBUG(String(nOrderID)+String(" Order nicht alles gelesen! bei:"),nPos);
                }
                pOS2MET->Seek(nPos+nOrderLen);
            }

            pOS2MET=pSave;
            if (pOrdFile->GetError()) {
                pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
                ErrorCode=10;
            }
            delete pOrdFile; pOrdFile=NULL;
            break;
        }
        case DscGrfObjMagic: {
            ULONG nPos, nMaxPos;
            USHORT nDscID, nDscLen;
            BYTE nbyte;

            nMaxPos=pOS2MET->Tell()+(ULONG)nFieldSize;
            while (pOS2MET->Tell()<nMaxPos && pOS2MET->GetError()==0) {
                *pOS2MET >> nbyte; nDscID =((USHORT)nbyte) & 0x00ff;
                *pOS2MET >> nbyte; nDscLen=((USHORT)nbyte) & 0x00ff;
                nPos=pOS2MET->Tell();
                ReadDsc(nDscID, nDscLen);
                pOS2MET->Seek(nPos+nDscLen);
            }
            break;
        }
        case DatGrfObjMagic: {
            if (pOrdFile==NULL) {
                pOrdFile = new SvMemoryStream;
                pOrdFile->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
            }
            BYTE * pBuf; pBuf = new BYTE[nFieldSize];
            pOS2MET->Read(pBuf,nFieldSize);
            pOrdFile->Write(pBuf,nFieldSize);
            delete pBuf;
            break;
        }
        case MapCodFntMagic:
            ReadFont(nFieldSize);
            break;

        case MapDatResMagic:
            break;
    }
}

void OS2METReader::ReadOS2MET( SvStream & rStreamOS2MET, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata)
{
    USHORT nFieldSize;
    USHORT nFieldType;
    ULONG nPos, nStartPos, nEndPos, nPercent, nLastPercent;
    BYTE nMagicByte;

    ErrorCode=0;

    pCallback=pcallback; pCallerData=pcallerdata;

    pOS2MET             = &rStreamOS2MET;
    nOrigPos            = pOS2MET->Tell();
    nOrigNumberFormat   = pOS2MET->GetNumberFormatInt();

    bCoord32 = TRUE;
    pPaletteStack=NULL;
    pAreaStack=NULL;
    pPathStack=NULL;
    pPathList=NULL;
    pFontList=NULL;
    pBitmapList=NULL;
    pAttrStack=NULL;

    aDefAttr.aLinCol     =Color(COL_BLACK);
    aDefAttr.aLinBgCol   =Color(COL_WHITE);
    aDefAttr.eLinMix     =ROP_OVERPAINT;
    aDefAttr.eLinBgMix   =ROP_OVERPAINT;
    aDefAttr.aChrCol     =Color(COL_BLACK);
    aDefAttr.aChrBgCol   =Color(COL_WHITE);
    aDefAttr.eChrMix     =ROP_OVERPAINT;
    aDefAttr.eChrBgMix   =ROP_OVERPAINT;
    aDefAttr.aMrkCol     =Color(COL_BLACK);
    aDefAttr.aMrkBgCol   =Color(COL_WHITE);
    aDefAttr.eMrkMix     =ROP_OVERPAINT;
    aDefAttr.eMrkBgMix   =ROP_OVERPAINT;
    aDefAttr.aPatCol     =Color(COL_BLACK);
    aDefAttr.aPatBgCol   =Color(COL_WHITE);
    aDefAttr.ePatMix     =ROP_OVERPAINT;
    aDefAttr.ePatBgMix   =ROP_OVERPAINT;
    aDefAttr.aImgCol     =Color(COL_BLACK);
    aDefAttr.aImgBgCol   =Color(COL_WHITE);
    aDefAttr.eImgMix     =ROP_OVERPAINT;
    aDefAttr.eImgBgMix   =ROP_OVERPAINT;
    aDefAttr.nArcP       =1;
    aDefAttr.nArcQ       =1;
    aDefAttr.nArcR       =0;
    aDefAttr.nArcS       =0;
    aDefAttr.nChrAng     =0;
    aDefAttr.aChrCellSize=Size(12,12);
    aDefAttr.nChrSet     =0;
    aDefAttr.aCurPos     =Point(0,0);
    aDefAttr.eLinStyle   =PEN_SOLID;
    aDefAttr.nLinWidth   =0;
    aDefAttr.aMrkCellSize=Size(10,10);
    aDefAttr.nMrkPrec    =0x01;
    aDefAttr.nMrkSet     =0xff;
    aDefAttr.nMrkSymbol  =0x01;
    aDefAttr.bFill       =TRUE;
    aDefAttr.nStrLinWidth=0;

    aAttr=aDefAttr;

    pOrdFile=NULL;

    pVirDev = new VirtualDevice();
    pVirDev->EnableOutput(FALSE);
    rGDIMetaFile.Record(pVirDev);

    pOS2MET->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

    nStartPos=pOS2MET->Tell();
    nEndPos=pOS2MET->Seek(STREAM_SEEK_TO_END); pOS2MET->Seek(nStartPos);
    Callback(0); nLastPercent=0;

    nPos=pOS2MET->Tell();
    if ( nStartPos == nEndPos )
    {
        nEndPos = 100;
        nStartPos = 0;
    }

    for (;;) {

        nPercent=(nPos-nStartPos)*100/(nEndPos-nStartPos);
        if (nLastPercent+4<=nPercent) {
            if (Callback((USHORT)nPercent)==TRUE) break;
            nLastPercent=nPercent;
        }

        nFieldSize=ReadBigEndianWord();

        *pOS2MET >> nMagicByte;
        if (nMagicByte!=0xd3) {
            pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
            ErrorCode=7;
            break;
        }
        *pOS2MET >> nFieldType;

        pOS2MET->SeekRel(3);
        nPos+=8; nFieldSize-=8;

        if (pOS2MET->GetError()) break;
        if (pOS2MET->IsEof()) {
            pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
            ErrorCode=8;
            break;
        }

        if (nFieldType==EndDocumnMagic) break;

        ReadField(nFieldType, nFieldSize);

        nPos+=(ULONG)nFieldSize;
        if (pOS2MET->Tell()>nPos)  {
            pOS2MET->SetError(SVSTREAM_FILEFORMAT_ERROR);
            ErrorCode=9;
            break;
        }
        pOS2MET->Seek(nPos);
    }

    rGDIMetaFile.Stop();
    delete pVirDev;

    rGDIMetaFile.SetPrefMapMode( aGlobMapMode );

    if( aBoundingRect.GetWidth() && aBoundingRect.GetHeight() )
        rGDIMetaFile.SetPrefSize( aBoundingRect.GetSize() );
    else
    {
        if( aCalcBndRect.Left() || aCalcBndRect.Top() )
            rGDIMetaFile.Move( -aCalcBndRect.Left(), -aCalcBndRect.Top() );

        rGDIMetaFile.SetPrefSize( aCalcBndRect.GetSize() );
    }

    if (pOrdFile!=NULL) delete pOrdFile;

    while (pAreaStack!=NULL) {
        OSArea * p=pAreaStack;
        pAreaStack=p->pSucc;
        delete p;
    }

    while (pPathStack!=NULL) {
        OSPath * p=pPathStack;
        pPathStack=p->pSucc;
        delete p;
    }

    while (pPathList!=NULL) {
        OSPath * p=pPathList;
        pPathList=p->pSucc;
        delete p;
    }

    while (pFontList!=NULL) {
        OSFont * p=pFontList;
        pFontList=p->pSucc;
        delete p;
    }

    while (pBitmapList!=NULL) {
        OSBitmap * p=pBitmapList;
        pBitmapList=p->pSucc;
        if (p->pBMP!=NULL) delete p->pBMP;
        delete p;
    }

    while (pAttrStack!=NULL) {
        OSAttr * p=pAttrStack;
        pAttrStack=p->pSucc;
        delete p;
    }

    while (pPaletteStack!=NULL) {
        OSPalette * p=pPaletteStack;
        pPaletteStack=p->pSucc;
        if (p->p0RGB!=NULL) delete p->p0RGB;
        delete p;
    }

    pOS2MET->SetNumberFormatInt(nOrigNumberFormat);

    if (pOS2MET->GetError()) {
        OOODEBUG("Fehler Nr.:",ErrorCode);
        pOS2MET->Seek(nOrigPos);
    }
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#else
extern "C" BOOL GraphicImport(SvStream & rStream, Graphic & rGraphic,
                              PFilterCallback pCallback, void * pCallerData,
                              Config *, BOOL)
#endif
{
    OS2METReader    aOS2METReader;
    GDIMetaFile     aMTF;
    BOOL            bRet = FALSE;

    if ( &rStream == NULL && pCallerData )
    {
        SvMemoryStream  aMemStm;

        aMemStm << *(GDIMetaFile*) pCallerData;
        aMemStm.Seek( 0 );
        pCallerData = NULL;

        aOS2METReader.ReadOS2MET( aMemStm, aMTF, NULL, NULL );

        if ( !aMemStm.GetError() )
        {
            rGraphic = Graphic(aMTF);
            bRet = TRUE;
        }
    }
    else
    {
        aOS2METReader.ReadOS2MET(rStream,aMTF,pCallback,pCallerData);

        if ( !rStream.GetError() )
        {
            rGraphic=Graphic(aMTF);
            bRet = TRUE;
        }
    }

    return bRet;
}

//================== ein bischen Muell fuer Windows ==========================

#pragma hdrstop

#ifdef WIN

static HINSTANCE hDLLInst = 0;      // HANDLE der DLL

extern "C" int CALLBACK LibMain( HINSTANCE hDLL, WORD, WORD nHeap, LPSTR )
{
#ifndef WNT
    if ( nHeap )
        UnlockData( 0 );
#endif

    hDLLInst = hDLL;

    return TRUE;
}

extern "C" int CALLBACK WEP( int )
{
    return 1;
}

#endif

