/*************************************************************************
 *
 *  $RCSfile: ipict.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-08 15:48:29 $
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
#include <string.h>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <vcl/poly.hxx>
#include <vcl/virdev.hxx>
#include <svtools/fltcall.hxx>

#ifndef NOOLDSV
#include <vcl/gdiobj.hxx>
#else // NOOLDSV
enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
enum BrushStyle { BRUSH_NULL, BRUSH_SOLID, BRUSH_HORZ, BRUSH_VERT,
                  BRUSH_CROSS, BRUSH_DIAGCROSS, BRUSH_UPDIAG, BRUSH_DOWNDIAG,
                  BRUSH_25, BRUSH_50, BRUSH_75,
                  BRUSH_BITMAP };
#endif // NOOLDSV

//============================ PictReader ==================================

enum PictDrawingMethod {
    PDM_FRAME, PDM_PAINT, PDM_ERASE, PDM_INVERT, PDM_FILL,
    PDM_TEXT, PDM_UNDEFINED
};

class PictReader {

private:

    PFilterCallback pCallback;
    void * pCallerData;

    SvStream    * pPict;             // Die einzulesende Pict-Datei
    VirtualDevice * pVirDev;         // Hier werden die Drawing-Methoden aufgerufen.
                                     // Dabei findet ein Recording in das GDIMetaFile
                                     // statt.
    ULONG         nOrigPos;          // Anfaengliche Position in pPict
    UINT16        nOrigNumberFormat; // Anfaengliches Nummern-Format von pPict
    BOOL          IsVersion2;        // Ob es ein Version 2 Pictfile ist.
    Rectangle     aBoundingRect;     // Min/Max-Rechteck fuer die ganze Zeichnung

    Point         aPenPosition;
    Point         aTextPosition;
    Color         aActForeColor;
    Color         aActBackColor;
    PenStyle      eActPenPenStyle;
    BrushStyle    eActPenBrushStyle;
    BrushStyle    eActFillStyle;
    BrushStyle    eActBackStyle;
    USHORT        nActPenSize;
    RasterOp      eActROP;
    PictDrawingMethod eActMethod;
    Size          aActOvalSize;
    Font          aActFont;

    Fraction        aHRes;
    Fraction        aVRes;

    BOOL Callback(USHORT nPercent);

    Point ReadPoint();

    Point ReadDeltaH(Point aBase);
    Point ReadDeltaV(Point aBase);

    Point ReadUnsignedDeltaH(Point aBase);
    Point ReadUnsignedDeltaV(Point aBase);

    Size ReadSize();

    Color ReadColor();

    Color ReadRGBColor();

    void ReadRectangle(Rectangle & rRect);

    ULONG ReadPolygon(Polygon & rPoly);

    ULONG ReadPattern(PenStyle * pPenStyle, BrushStyle * pBrushStyle);

    ULONG ReadPixPattern(PenStyle * pPenStyle, BrushStyle * pBrushStyle);

    Rectangle aLastRect;
    ULONG ReadAndDrawRect(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSameRect(PictDrawingMethod eMethod);

    Rectangle aLastRoundRect;
    ULONG ReadAndDrawRoundRect(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSameRoundRect(PictDrawingMethod eMethod);

    Rectangle aLastOval;
    ULONG ReadAndDrawOval(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSameOval(PictDrawingMethod eMethod);

    Polygon aLastPolygon;
    ULONG ReadAndDrawPolygon(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSamePolygon(PictDrawingMethod eMethod);

    Rectangle aLastArcRect;
    ULONG ReadAndDrawArc(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSameArc(PictDrawingMethod eMethod);

    ULONG ReadAndDrawRgn(PictDrawingMethod eMethod);
    ULONG ReadAndDrawSameRgn(PictDrawingMethod eMethod);

    void DrawingMethod(PictDrawingMethod eMethod);

    ULONG ReadAndDrawText();

    ULONG ReadPixMapEtc(Bitmap & rBitmap, BOOL bBaseAddr, BOOL bColorTable,
                        Rectangle * pSrcRect, Rectangle * pDestRect,
                        BOOL bMode, BOOL bMaskRgn);

    void ReadHeader();
        // Liesst den Kopf der Pict-Datei, setzt IsVersion2 und aBoundingRect

    ULONG ReadData(USHORT nOpcode);
        // Liesst die Daten eines Opcodes ein und fuehrt die Operation aus.
        // Auf jeden Fall wird die Anzahl der Datenbytes zu dem Opcode
        // zurueckgeliefert.

    void SetPen( const Color& rPenColor, USHORT nWidth, PenStyle eStyle );
    void SetBrush( const Color& rColor, const Color& rBgColor, BrushStyle eStyle );

public:

    PictReader() {}

    void ReadPict( SvStream & rStreamPict, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata);
        // Liesst aus dem Stream eine Pict-Datei und fuellt das GDIMetaFile

};

//------------------------------------------------------------------------------------------------

#define SETBYTE                                         \
    switch ( nPixelSize )                               \
    {                                                   \
        case 1 :                                        \
            pAcc->SetPixel( ny, nx++, nDat >> 7 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 6 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 5 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 4 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 3 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 2 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 1 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat );           \
            break;                                      \
        case 2 :                                        \
            pAcc->SetPixel( ny, nx++, nDat >> 6 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 4 & 3);   \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat >> 2 & 3 );  \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat & 3);        \
            break;                                      \
        case 4 :                                        \
            pAcc->SetPixel( ny, nx++, nDat >> 4 );      \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixel( ny, nx++, nDat );           \
            break;                                      \
        case 8 :                                        \
            pAcc->SetPixel( ny, nx++, nDat );           \
            break;                                      \
    }

//------------------------------------------------------------------------------------------------

#define BITMAPERROR                                     \
{                                                       \
    if ( pAcc )                                         \
        aBitmap.ReleaseAccess( pAcc );                  \
    if ( pReadAcc )                                     \
        aBitmap.ReleaseAccess( pReadAcc );              \
    return 0xffffffff;                                  \
}

//=================== Methoden von PictReader ==============================

void PictReader::SetPen( const Color& rPenColor, USHORT nWidth, PenStyle eStyle )
{
    pVirDev->SetLineColor( rPenColor );
}

void PictReader::SetBrush( const Color& rColor, const Color& rBgColor, BrushStyle eStyle )
{
    pVirDev->SetFillColor( rColor );
}

BOOL PictReader::Callback(USHORT nPercent)
{
    if (pCallback!=NULL) {
        if (((*pCallback)(pCallerData,nPercent))==TRUE) {
            pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
            return TRUE;
        }
    }
    return FALSE;
}

inline Point PictReader::ReadPoint()
{
    short nx,ny;

    *pPict >> ny >> nx;

   return Point( (long) ( Fraction( (long) nx ) * aHRes ) - aBoundingRect.Left(),
                 (long) ( Fraction( (long) ny ) * aVRes ) - aBoundingRect.Top() );
}

inline Point PictReader::ReadDeltaH(Point aBase)
{
    signed char ndh;

    *pPict >> ((char&)ndh);

    return Point( aBase.X() + (long) ( Fraction( (long) ndh ) * aHRes ),
                  aBase.Y() );
}

inline Point PictReader::ReadDeltaV(Point aBase)
{
    signed char ndv;

    *pPict >> ((char&)ndv);

    return Point( aBase.X(), aBase.Y() + (long) ( Fraction( (long) ndv ) * aVRes ) );
}

inline Point PictReader::ReadUnsignedDeltaH(Point aBase)
{
    char ndh;

    *pPict >> ndh;

    return Point(aBase.X() + (long) ( Fraction( (long) ndh ) * aHRes ), aBase.Y() );
}

inline Point PictReader::ReadUnsignedDeltaV(Point aBase)
{
    char ndv;

    *pPict >> ndv;

    return Point( aBase.X(), aBase.Y() + (long) ( Fraction( (long) ndv ) * aVRes ) );
}

inline Size PictReader::ReadSize()
{
    short nx,ny;

    *pPict >> ny >> nx;

    return Size( (long) ( Fraction( (long) nx ) * aHRes ),
                 (long) ( Fraction( (long) ny ) * aVRes ) );
}

Color PictReader::ReadColor()
{
    ULONG nCol;
    Color aCol;

    *pPict >> nCol;
    switch (nCol)
    {
        case  33: aCol=Color( COL_BLACK );        break;
        case  30: aCol=Color( COL_WHITE );        break;
        case 205: aCol=Color( COL_LIGHTRED );     break;
        case 341: aCol=Color( COL_LIGHTGREEN );   break;
        case 409: aCol=Color( COL_LIGHTBLUE );    break;
        case 273: aCol=Color( COL_LIGHTCYAN );    break;
        case 137: aCol=Color( COL_LIGHTMAGENTA ); break;
        case  69: aCol=Color( COL_YELLOW );       break;
        default:  aCol=Color( COL_LIGHTGRAY );
    }
    return aCol;
}


Color PictReader::ReadRGBColor()
{
    USHORT nR, nG, nB;

    *pPict >> nR >> nG >> nB;
    return Color( (BYTE) ( nR >> 8 ), (BYTE) ( nG >> 8 ), (BYTE) ( nB >> 8 ) );
}


void PictReader::ReadRectangle(Rectangle & rRect)
{
    Point aTopLeft, aBottomRight;

    aTopLeft=ReadPoint();
    aBottomRight=ReadPoint();
    aBottomRight.X() -= 1;
    aBottomRight.Y() -= 1;
    rRect=Rectangle(aTopLeft,aBottomRight);
}


ULONG PictReader::ReadPolygon(Polygon & rPoly)
{
    USHORT nSize,i;
    ULONG nDataSize;

    *pPict >> nSize;
    pPict->SeekRel(8);
    nDataSize=(ULONG)nSize;
    nSize=(nSize-10)/4;
    rPoly.SetSize(nSize);
    for (i=0; i<nSize; i++) rPoly.SetPoint(ReadPoint(),i);
    return nDataSize;
}

ULONG PictReader::ReadPattern(PenStyle * pPenStyle, BrushStyle * pBrushStyle)
{
    short nx,ny,nBitCount;
    unsigned char nbyte[8];
    BrushStyle eBrStyle;
    PenStyle ePnStyle;
    ULONG nHiBytes, nLoBytes;

    // Anzahl der Bits im Pattern zaehlen, die auf 1 gesetzt sind:
    nBitCount=0;
    for (ny=0; ny<8; ny++) {
        *pPict >> ((char&)nbyte[ny]);
        for (nx=0; nx<8; nx++) {
            if ( (nbyte[ny] & (1<<nx)) != 0 ) nBitCount++;
        }
    }

    // Pattern in 2 Langworten unterbringen:
    nHiBytes=(((((((ULONG)nbyte[0])<<8)|
                 (ULONG)nbyte[1])<<8)|
               (ULONG)nbyte[2])<<8)|
             (ULONG)nbyte[3];
    nLoBytes=(((((((ULONG)nbyte[4])<<8)|
                 (ULONG)nbyte[5])<<8)|
               (ULONG)nbyte[6])<<8)|
             (ULONG)nbyte[7];

    // Einen PenStyle machen:
    if      (nBitCount<=0)  ePnStyle=PEN_NULL;
    else if (nBitCount<=16) ePnStyle=PEN_DOT;
    else if (nBitCount<=32) ePnStyle=PEN_DASHDOT;
    else if (nBitCount<=48) ePnStyle=PEN_DASH;
    else                    ePnStyle=PEN_SOLID;

    // Einen BrushStyle machen:
    if      (nHiBytes==0xffffffff && nLoBytes==0xffffffff) eBrStyle=BRUSH_SOLID;
    else if (nHiBytes==0xff000000 && nLoBytes==0x00000000) eBrStyle=BRUSH_HORZ;
    else if (nHiBytes==0x80808080 && nLoBytes==0x80808080) eBrStyle=BRUSH_VERT;
    else if (nHiBytes==0xff808080 && nLoBytes==0x80808080) eBrStyle=BRUSH_CROSS;
    else if (nHiBytes==0x01824428 && nLoBytes==0x10284482) eBrStyle=BRUSH_DIAGCROSS;
    else if (nHiBytes==0x80402010 && nLoBytes==0x08040201) eBrStyle=BRUSH_UPDIAG;
    else if (nHiBytes==0x01020408 && nLoBytes==0x10204080) eBrStyle=BRUSH_DOWNDIAG;
    else if (nBitCount<=24) eBrStyle=BRUSH_25;
    else if (nBitCount<=40) eBrStyle=BRUSH_50;
    else if (nBitCount<=56) eBrStyle=BRUSH_75;
    else                    eBrStyle=BRUSH_SOLID;

    if (pPenStyle!=0) *pPenStyle=ePnStyle;

    if (pBrushStyle!=0) *pBrushStyle=eBrStyle;

    return 8;
}

ULONG PictReader::ReadPixPattern(PenStyle * pPenStyle, BrushStyle * pBrushStyle)
{
    // Keine Ahnung, ob dies richtig ist, weil kein Bild gefunden, das
    // PixPatterns enthaelt. Auch hier nur der Versuch, die Groesse der Daten zu
    // ermitteln, und einfache StarView-Styles daraus zu machen. Gluecklicherweise
    // enthaelt ein PixPattern immer auch ein normales Pattern.

    ULONG nDataSize;
    USHORT nPatType;
    Bitmap aBMP;

    *pPict >> nPatType;
    if (nPatType==1) {
        ReadPattern(pPenStyle,pBrushStyle);
        nDataSize=ReadPixMapEtc(aBMP,FALSE,TRUE,NULL,NULL,FALSE,FALSE);
        if (nDataSize!=0xffffffff) nDataSize+=10;
    }
    else if (nPatType==2) {
        ReadPattern(pPenStyle,pBrushStyle);
        pPict->SeekRel(6); // RGBColor
        nDataSize=16;
    }
    else nDataSize=0xffffffff;

    return nDataSize;
}

ULONG PictReader::ReadAndDrawRect(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastRect);
    DrawingMethod(eMethod);
    pVirDev->DrawRect(aLastRect);
    return 8;
}

ULONG PictReader::ReadAndDrawSameRect(PictDrawingMethod eMethod)
{
    DrawingMethod(eMethod);
    pVirDev->DrawRect(aLastRect);
    return 0;
}

ULONG PictReader::ReadAndDrawRoundRect(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastRoundRect);
    DrawingMethod(eMethod);
    pVirDev->DrawRect(aLastRoundRect,aActOvalSize.Width(),aActOvalSize.Height());
    return 8;
}

ULONG PictReader::ReadAndDrawSameRoundRect(PictDrawingMethod eMethod)
{
    DrawingMethod(eMethod);
    pVirDev->DrawRect(aLastRoundRect,aActOvalSize.Width(),aActOvalSize.Height());
    return 0;
}

ULONG PictReader::ReadAndDrawOval(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastOval);
    DrawingMethod(eMethod);
    pVirDev->DrawEllipse(aLastOval);
    return 8;
}

ULONG PictReader::ReadAndDrawSameOval(PictDrawingMethod eMethod)
{
    DrawingMethod(eMethod);
    pVirDev->DrawEllipse(aLastOval);
    return 0;
}

ULONG PictReader::ReadAndDrawPolygon(PictDrawingMethod eMethod)
{
    ULONG nDataSize;

    nDataSize=ReadPolygon(aLastPolygon);
    DrawingMethod(eMethod);
    if (eMethod==PDM_FRAME) pVirDev->DrawPolyLine(aLastPolygon);
    else pVirDev->DrawPolygon(aLastPolygon);
    return nDataSize;
}

ULONG PictReader::ReadAndDrawSamePolygon(PictDrawingMethod eMethod)
{
    DrawingMethod(eMethod);
    if (eMethod==PDM_FRAME) pVirDev->DrawPolyLine(aLastPolygon);
    else pVirDev->DrawPolygon(aLastPolygon);
    return 0;
}


ULONG PictReader::ReadAndDrawArc(PictDrawingMethod eMethod)
{
    short nstartAngle, narcAngle;
    double fAng1, fAng2;
    Point aStartPt, aEndPt, aCenter;

    ReadRectangle(aLastArcRect);
    *pPict >> nstartAngle >> narcAngle;
    if (narcAngle<0) {
        nstartAngle+=narcAngle;
        narcAngle=-narcAngle;
    }
    fAng1=((double)nstartAngle)/180.0*3.14159265359;
    fAng2=((double)(nstartAngle+narcAngle))/180.0*3.14159265359;
    aCenter=Point((aLastArcRect.Left()+aLastArcRect.Right())/2,
                  (aLastArcRect.Top()+aLastArcRect.Bottom())/2);
    aStartPt=Point(aCenter.X()+(long)( sin(fAng2)*256.0),
                   aCenter.Y()+(long)(-cos(fAng2)*256.0));
    aEndPt=  Point(aCenter.X()+(long)( sin(fAng1)*256.0),
                   aCenter.Y()+(long)(-cos(fAng1)*256.0));
    DrawingMethod(eMethod);
    if (eMethod==PDM_FRAME) pVirDev->DrawArc(aLastArcRect,aStartPt,aEndPt);
    else pVirDev->DrawPie(aLastArcRect,aStartPt,aEndPt);
    return 12;
}

ULONG PictReader::ReadAndDrawSameArc(PictDrawingMethod eMethod)
{
    short nstartAngle, narcAngle;
    double fAng1, fAng2;
    Point aStartPt, aEndPt, aCenter;

    *pPict >> nstartAngle >> narcAngle;
    if (narcAngle<0) {
        nstartAngle+=narcAngle;
        narcAngle=-narcAngle;
    }
    fAng1=((double)nstartAngle)/180.0*3.14159265359;
    fAng2=((double)(nstartAngle+narcAngle))/180.0*3.14159265359;
    aCenter=Point((aLastArcRect.Left()+aLastArcRect.Right())/2,
                  (aLastArcRect.Top()+aLastArcRect.Bottom())/2);
    aStartPt=Point(aCenter.X()+(long)( sin(fAng2)*256.0),
                   aCenter.Y()+(long)(-cos(fAng2)*256.0));
    aEndPt=  Point(aCenter.X()+(long)( sin(fAng1)*256.0),
                   aCenter.Y()+(long)(-cos(fAng1)*256.0));
        DrawingMethod(eMethod);
    if (eMethod==PDM_FRAME) pVirDev->DrawArc(aLastArcRect,aStartPt,aEndPt);
    else pVirDev->DrawPie(aLastArcRect,aStartPt,aEndPt);
    return 4;
}

ULONG PictReader::ReadAndDrawRgn(PictDrawingMethod eMethod)
{
    USHORT nSize;

        DrawingMethod(eMethod);
    *pPict >> nSize;
    // ...???...
    return (ULONG)nSize;
}

ULONG PictReader::ReadAndDrawSameRgn(PictDrawingMethod eMethod)
{
        DrawingMethod(eMethod);
    // ...???...
    return 0;
}

void PictReader::DrawingMethod(PictDrawingMethod eMethod)
{
    if( eActMethod==eMethod ) return;
    switch (eMethod) {
        case PDM_FRAME:
            SetPen( aActForeColor, nActPenSize, eActPenPenStyle );
            SetBrush( Color(COL_TRANSPARENT), Color(COL_TRANSPARENT), BRUSH_NULL );
            pVirDev->SetRasterOp(eActROP);
            break;
        case PDM_PAINT:
            SetPen( Color(COL_TRANSPARENT), 0, PEN_NULL );
            SetBrush( aActForeColor, aActBackColor, eActPenBrushStyle );
            pVirDev->SetRasterOp(eActROP);
            break;
        case PDM_ERASE:
            SetPen( Color(COL_TRANSPARENT), 0, PEN_NULL );
            SetBrush( aActForeColor, aActBackColor, eActPenBrushStyle );
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
        case PDM_INVERT:
            SetPen( Color(COL_TRANSPARENT), 0, PEN_NULL );
            SetBrush( Color( COL_BLACK ), Color( COL_BLACK ), BRUSH_SOLID );
            pVirDev->SetRasterOp(ROP_INVERT);
            break;
        case PDM_FILL:
            SetPen( Color(COL_TRANSPARENT), 0, PEN_NULL );
            SetBrush( aActForeColor, aActBackColor, eActPenBrushStyle );
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
        case PDM_TEXT:
            aActFont.SetColor(aActForeColor);
            aActFont.SetFillColor(aActBackColor);
            aActFont.SetTransparent(TRUE);
            pVirDev->SetFont(aActFont);
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
    }
    eActMethod=eMethod;
}

ULONG PictReader::ReadAndDrawText()
{
    char        nByteLen;
    sal_uInt32  nLen, nDataLen;
    sal_Char    sText[256];

    DrawingMethod(PDM_TEXT);
    *pPict >> nByteLen; nLen=((ULONG)nByteLen)&0x000000ff;
    nDataLen = nLen + 1;
    pPict->Read( &sText, nLen );

    // Stoerende Steuerzeuichen wegnehmen:
    while ( nLen > 0 && ( (unsigned char)sText[ nLen - 1 ] ) < 32 )
            nLen--;
    sText[ nLen ] = 0;
    String aString( (const sal_Char*)&sText, gsl_getSystemTextEncoding() );
    pVirDev->DrawText( Point( aTextPosition.X(), aTextPosition.Y() ), aString );
    return nDataLen;
}

ULONG PictReader::ReadPixMapEtc( Bitmap &rBitmap, BOOL bBaseAddr, BOOL bColorTable, Rectangle* pSrcRect,
                                    Rectangle* pDestRect, BOOL bMode, BOOL bMaskRgn )
{
    Bitmap              aBitmap;
    BitmapWriteAccess*  pAcc = NULL;
    BitmapReadAccess*   pReadAcc = NULL;
    USHORT              ny, nx, nColTabSize;
    USHORT              nRowBytes, nBndX, nBndY, nWidth, nHeight, nVersion, nPackType, nPixelType,
                        nPixelSize, nCmpCount, nCmpSize;
    ULONG               nPackSize, nPlaneBytes, nHRes, nVRes;
    BYTE                nDat, nRed, nGreen, nBlue, nDummy;
    ULONG               i, nDataSize = 0;

    // In nDataSize wird mitgerechnet, wie gross die gesammten Daten sind.
    nDataSize = 0;

    // ggf. BaseAddr ueberlesen
    if ( bBaseAddr )
    {
        pPict->SeekRel( 4 );
        nDataSize += 4;
    }

    // PixMap oder Bitmap-Struktur einlesen;
    *pPict >> nRowBytes >> nBndY >> nBndX >> nHeight >> nWidth;
    nHeight -= nBndY;
    nWidth -= nBndX;

    if ( ( nRowBytes & 0x8000 ) != 0 )
    {
        nRowBytes &= 0x3fff;
        *pPict >> nVersion >> nPackType >> nPackSize >> nHRes >> nVRes >> nPixelType >>
                    nPixelSize >> nCmpCount >> nCmpSize >> nPlaneBytes;

        pPict->SeekRel( 8 );
        nDataSize += 46;
        aBitmap = Bitmap( Size( nWidth, nHeight ), ( nPixelSize > 8 ) ? 24 : nPixelSize );

        if ( ( pAcc = aBitmap.AcquireWriteAccess() ) == NULL )
            BITMAPERROR;

        if ( bColorTable )
        {
            pPict->SeekRel( 6 );
            *pPict >> nColTabSize;

            if ( ++nColTabSize > 256 )
                BITMAPERROR;

            pAcc->SetPaletteEntryCount( nColTabSize );

            for ( i = 0; i < nColTabSize; i++ )
            {
                pPict->SeekRel(2);
                *pPict >> nRed >> nDummy >> nGreen >> nDummy >> nBlue >> nDummy;
                pAcc->SetPaletteColor( (USHORT) i, BitmapColor( nRed, nGreen, nBlue ) );
            }
            nDataSize += 8 + nColTabSize * 8;
        }
    }
    else
    {
        nRowBytes &= 0x3fff;
        nVersion = 0;
        nPackType = 0;
        nPackSize = nHRes = nVRes = nPlaneBytes = 0;
        nPixelType = 0;
        nPixelSize = nCmpCount = nCmpSize = 1;
        nDataSize += 10;
        aBitmap = Bitmap( Size( nWidth, nHeight ), 1 );
        if ( ( pAcc = aBitmap.AcquireWriteAccess() ) == NULL )
            BITMAPERROR;
        pAcc->SetPaletteEntryCount( 2 );
        pAcc->SetPaletteColor( 0, BitmapColor( 0xff, 0xff, 0xff ) );
        pAcc->SetPaletteColor( 1, BitmapColor( 0, 0, 0 ) );
    }

    // ggf. Quell-Rechteck einlesen:
    if ( pSrcRect != 0)
    {
        USHORT  nTop, nLeft, nBottom, nRight;
        *pPict >> nTop >> nLeft >> nBottom >> nRight;
        *pSrcRect = Rectangle( (ULONG)nLeft, (ULONG)nTop, (ULONG)nRight, (ULONG)nBottom );
        nDataSize += 8;
    }

    // ggf. Ziel-Rechteck einlesen:
    if ( pDestRect != 0 )
    {
        Point aTL, aBR;
        aTL = ReadPoint();
        aBR = ReadPoint();
        *pDestRect = Rectangle( aTL, aBR );
        nDataSize += 8;
    }

    // ggf. Modus einlesen (bzw. ueberspringen):
    if ( bMode )
    {
        pPict->SeekRel(2);
        nDataSize += 2;
    }

    // ggf. Region einlesen (bzw. ueberspringen):
    if ( bMaskRgn )
    {
        USHORT nSize;
        *pPict >> nSize;
        pPict->SeekRel( nSize - 2 );
        nDataSize += (ULONG)nSize;
    }

//  aSMem << (nHRes/1665L) << (nVRes/1665L) << ((ULONG)0) << ((ULONG)0);

    // Lese und Schreibe Bitmap-Bits:
    if ( nPixelSize == 1 || nPixelSize == 2 || nPixelSize == 4 || nPixelSize == 8 )
    {
        BYTE    nByteCountAsByte, nFlagCounterByte;
        USHORT  nByteCount, nCount, nSrcBPL, nDestBPL;

        if      ( nPixelSize == 1 ) nSrcBPL = ( nWidth + 7 ) >> 3;
        else if ( nPixelSize == 2 ) nSrcBPL = ( nWidth + 3 ) >> 2;
        else if ( nPixelSize == 4 ) nSrcBPL = ( nWidth + 1 ) >> 1;
        else                        nSrcBPL = nWidth;
        nDestBPL = ( nSrcBPL + 3 ) & 0xfffc;
        if ( nRowBytes < nSrcBPL || nRowBytes > nDestBPL )
            BITMAPERROR;

        for ( ny = 0; ny < nHeight; ny++ )
        {
            nx = 0;
            if ( nRowBytes < 8 || nPackType == 1 )
            {
                for ( i = 0; i < nRowBytes; i++ )
                    SETBYTE;
                nDataSize += nRowBytes;
            }
            else
            {
                if ( nRowBytes > 250 )
                {
                    *pPict >> nByteCount;
                    nDataSize += 2 + (ULONG)nByteCount;
                }
                else
                {
                    *pPict >> nByteCountAsByte;
                    nByteCount = ( (USHORT)nByteCountAsByte ) & 0x00ff;
                    nDataSize += 1 + (ULONG)nByteCount;
                }

                while ( nByteCount )
                {
                    *pPict >> nFlagCounterByte;
                    if ( ( nFlagCounterByte & 0x80 ) == 0 )
                    {
                        nCount = ( (USHORT)nFlagCounterByte ) + 1;
                        for ( i = 0; i < nCount; i++ )
                        {
                            *pPict >> nDat;
                            if ( nx < nWidth )
                                SETBYTE;
                        }
                        nByteCount -= 1 + nCount;
                    }
                    else
                    {
                        nCount = ( 1 - ( ( (USHORT)nFlagCounterByte ) | 0xff00 ) ) & 0xffff;
                        *pPict >> nDat;
                        for ( i = 0; i < nCount; i++ )
                        {
                            if ( nx < nWidth )
                                SETBYTE;
                        }
                        nByteCount -= 2;
                    }
                }
            }
        }
    }
    else if ( nPixelSize == 16 )
    {
        BYTE    nByteCountAsByte, nFlagCounterByte;
        USHORT  nByteCount, nCount, nDestBPL,nD;
        ULONG   nSrcBitsPos;

        if ( nRowBytes < 2 * nWidth )
            BITMAPERROR;

        nDestBPL = ( ( 3 * nWidth ) + 0x0003 ) & 0xfffc;

        for ( ny = 0; ny < nHeight; ny++ )
        {
            nx = 0;
            if ( nRowBytes < 8 || nPackType == 1 )
            {
                for ( i = 0; i < nWidth; i++ )
                {
                    *pPict >> nD;
                    nRed = (BYTE)( nD >> 7 );
                    nGreen = (BYTE)( nD >> 2 );
                    nBlue = (BYTE)( nD << 3 );
                    pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                }
                nDataSize += ( (ULONG)nWidth ) * 2;
            }
            else
            {
                nSrcBitsPos = pPict->Tell();
                if ( nRowBytes > 250 )
                {
                    *pPict >> nByteCount;
                    nByteCount += 2;
                }
                else
                {
                    *pPict >> nByteCountAsByte;
                    nByteCount = ( (USHORT)nByteCountAsByte ) & 0x00ff;
                    nByteCount++;
                }
                while ( nx != nWidth )
                {
                    *pPict >> nFlagCounterByte;
                    if ( (nFlagCounterByte & 0x80) == 0)
                    {
                        nCount=((USHORT)nFlagCounterByte)+1;
                        if ( nCount + nx > nWidth)
                            nCount = nWidth - nx;
                        for (i=0; i<nCount; i++)
                        {
                            *pPict >> nD;
                            nRed = (BYTE)( nD >> 7 );
                            nGreen = (BYTE)( nD >> 2 );
                            nBlue = (BYTE)( nD << 3 );
                            pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                    else
                    {
                        nCount=(1-(((USHORT)nFlagCounterByte)|0xff00))&0xffff;
                        if ( nCount + nx > nWidth )
                            nCount = nWidth - nx;
                        *pPict >> nD;
                        nRed = (BYTE)( nD >> 7 );
                        nGreen = (BYTE)( nD >> 2 );
                        nBlue = (BYTE)( nD << 3 );
                        for (i=0; i<nCount; i++)
                        {
                            pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                }
                nDataSize+=(ULONG)nByteCount;
                pPict->Seek(nSrcBitsPos+(ULONG)nByteCount);
            }
        }
    }
    else if (nPixelSize==32)
    {
        BYTE                nByteCountAsByte, nFlagCounterByte;
        USHORT              nByteCount, nCount, nDestBPL,nc;
        ULONG               nSrcBitsPos;
        BitmapColor         aBitmapColor;
        if ( ( pReadAcc = aBitmap.AcquireReadAccess() ) == NULL )
            BITMAPERROR;
        if ( nRowBytes != 4*nWidth )
            BITMAPERROR;
        nDestBPL = ( ( 3 * nWidth ) + 0x0003 ) & 0xfffc;

        for ( ny = 0; ny < nHeight; ny++ )
        {
            nx = 0;
            if ( nRowBytes < 8 || nPackType == 1 )
            {
                for ( i = 0; i < nWidth; i++ )
                {
                    *pPict >> nDummy >> nRed >> nGreen >> nBlue;
                    pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue) );
                }
                nDataSize += ( (ULONG)nWidth ) * 4;
            }
            else if ( nPackType == 2 )
            {
                for ( i = 0; i < nWidth; i++ )
                {
                    *pPict >> nRed >> nGreen >> nBlue;
                    pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                }
                nDataSize += ( (ULONG)nWidth ) * 3;
            }
            else
            {
                nSrcBitsPos = pPict->Tell();
                if ( nRowBytes > 250 )
                {
                    *pPict >> nByteCount;
                    nByteCount += 2;
                }
                else
                {
                    *pPict >> nByteCountAsByte;
                    nByteCount = (BYTE)nByteCountAsByte;
                    nByteCount++;
                }
                for ( nc = 0; nc < 4; nc++ )
                {
                    nx = 0;
                    while ( nx != nWidth )
                    {
                        *pPict >> nFlagCounterByte;
                        if ( ( nFlagCounterByte & 0x80 ) == 0)
                        {
                            nCount = ( (USHORT)nFlagCounterByte ) + 1;
                            BOOL nSeekStream = 0;
                            if ( ( nCount + nx ) > nWidth )
                            {
                                nSeekStream = nCount - nWidth + nx;
                                nCount = nWidth - nx;
                            }
                            for ( i  = 0; i < nCount; i++ )
                            {
                                *pPict >> nDat;
                                switch( nc )
                                {
                                    case 0 :
                                        nx++;
                                        break;
                                    case 1 :
                                        pAcc->SetPixel( ny, nx++, BitmapColor( nDat, 0, 0 ) );
                                        break;
                                    case 2 :
                                        aBitmapColor = pReadAcc->GetPixel( ny, nx );
                                        aBitmapColor.SetGreen( nDat );
                                        pAcc->SetPixel( ny, nx++, aBitmapColor );
                                        break;
                                    case 3 :
                                        aBitmapColor = pReadAcc->GetPixel( ny, nx );
                                        aBitmapColor.SetBlue( nDat );
                                        pAcc->SetPixel( ny, nx++, aBitmapColor );
                                        break;
                                }
                            }
                            if ( nSeekStream )
                                pPict->SeekRel( nSeekStream );
                        }
                        else
                        {
                            nCount = ( 1 - ( ( (USHORT)nFlagCounterByte ) | 0xff00 ) );
                            if ( nCount + nx > nWidth )
                                nCount = nWidth - nx;
                            *pPict >> nDat;
                            for ( i = 0; i < nCount; i++ )
                            {
                                switch( nc )
                                {
                                    case 0 :
                                        nx++;
                                        break;
                                    case 1 :
                                        pAcc->SetPixel( ny, nx++, BitmapColor( nDat, 0, 0 ) );
                                        break;
                                    case 2 :
                                        aBitmapColor = pReadAcc->GetPixel( ny, nx );
                                        aBitmapColor.SetGreen( nDat );
                                        pAcc->SetPixel( ny, nx++, aBitmapColor );
                                        break;
                                    case 3 :
                                        aBitmapColor = pReadAcc->GetPixel( ny, nx );
                                        aBitmapColor.SetBlue( nDat );
                                        pAcc->SetPixel( ny, nx++, aBitmapColor );
                                        break;
                                }
                            }
                        }
                    }
                }
                nDataSize += (ULONG)nByteCount;
                pPict->Seek( nSrcBitsPos + (ULONG)nByteCount );
            }
        }
    }
    else
        BITMAPERROR;
    if ( pReadAcc )
        aBitmap.ReleaseAccess( pReadAcc );
    aBitmap.ReleaseAccess( pAcc );
    rBitmap = aBitmap;
    return nDataSize;
}

void PictReader::ReadHeader()
{
    char nC;
    short y1,x1,y2,x2;

    sal_Char    sBuf[ 3 ];
    pPict->SeekRel( 10 );
    pPict->Read( sBuf, 3 );
    if ( sBuf[ 0 ] == 0x00 && sBuf[ 1 ] == 0x11 && ( sBuf[ 2 ] == 0x01 || sBuf[ 2 ] == 0x02 ) )
        pPict->SeekRel( -13 );      // this maybe a pict from a ms document
    else
        pPict->SeekRel( 512 - 13 ); // 512 Bytes Muell am Anfang

    pPict->SeekRel(2);              // Lo-16-bits von "picture size"
    *pPict >> y1 >> x1 >> y2 >> x2; // Rahmen-Rechteck des Bildes
    aBoundingRect=Rectangle( x1,y1, --x2, --y2 );

    // Jetzt kommen x-beliebig viele Nullen
    // (in manchen Dateien tatsaechlich mehr als eine):
    do { *pPict >> nC; } while (nC==0 && pPict->IsEof()==FALSE);

    // dann sollte der Versions-Opcode 0x11 folgen, dann die Versionsnummer:
    if (nC==0x11)
    {
        *pPict >> nC;
        if ( nC == 0x01 )
            IsVersion2 = FALSE;         // Version 1
        else                            // Version 2 oder hoeher
        {
            short   nExtVer;
            ULONG   nTempX;
            ULONG   nTempY;

            // 3 Bytes ueberspringen, um auf
            // ExtVersion2 oder Version2 zu kommen
            pPict->SeekRel( 3 );
            *pPict >> nExtVer;

            // nachsehen, ob wir einen Extended-Version2-Header (==-2) haben
            // oder einen einfachen Version2-Header (==-1);
            // dementsprechend Aufloesung einlesen oder nicht
            if ( nExtVer == -2 )
            {
                // Horizontale Skalierung als Bruch ( bzgl. 72 DPI )
                *pPict >> nTempX;
                nTempX &= 0x0000ffff;

                // Vertikale Skalierung als Bruch ( bzgl. 72 DPI )
                *pPict >> nTempY;
                nTempY &= 0x0000ffff;

                // nachfolgender Code soll nicht beeinflusst werden
                pPict->SeekRel( -12 );
            }
            else
            {
                nTempX = nTempY = 72;

                // nachfolgender Code soll nicht beeinflusst werden
                pPict->SeekRel( -4 );
            }

            // gefundene Aufloesung setzen
            aHRes = Fraction( 72, nTempX );
            aVRes = Fraction( 72, nTempY );

            IsVersion2=TRUE;
        }
    }
    else {
        // Eigentlich ist dies wohl kein Pict-File, aber es gibt tatsaechlich
        // Dateien, bei denen mehr als 512 Bytes "Muell" am Anfang stehen.
        // Somit koennte es theoretisch folgende Art von Header geben:
        // <beliebig viele Bytes Muell> <Picture-Size (Lo-Bytes)> <BoundingRect>
        // <beliebig viele Nullen> <0x11> ..
        // Da aber in so einem Fall die Position von <BoundingRect> kaum auszumachen ist,
        // gehen wir nun davon aus, dass in einer Datei immer entweder genau 512 Bytes Muell
        // am Anfang sind (wie oben versucht), oder (wie normalerweise ueblich) genau eine 0 zwischen
        // Bounding-Rectangle und 0x11. Des weiteren mag es hoechstens 1024 Bytes Muell geben,
        // und das Ganze nur fuer Version 1 oder 2.
        // Somit suchen wir nun nach der Folge 0x00,0x11,0x01 oder 0x00,0x11,0x02 innerhalb der
        // "zweiten" 512 Bytes, und nehmen an, dass davor das Bounding-Rect steht, und hoffen
        // dass das alles so seine Richtigkeit hat.
        BYTE n1,n2,n3;
        USHORT i,Found;
        pPict->Seek(522);
        Found=0;
        *pPict >> n1 >> n2 >> n3;
        for (i=0; i<512; i++) {
            if (n1==0x00 && n2==0x11 && (n3==0x01 || n3==0x02)) { Found=1; break; }
            n1=n2; n2=n3; *pPict >> n3;
        }
        if (Found!=0) {
            pPict->SeekRel(-11);
            *pPict >> y1 >> x1 >> y2 >> x2;
            // Lieber nochmal nachsehen, ob das Bounding-Rectangle gut zu sein scheint:
            if (x1+10<x2 && y1+10<y2 && y1>=-2048 && x1>=-2048 && x2<=2048 && y2<=2048) {
                aBoundingRect=Rectangle( x1, y1, --x2, --y2 );
                if (n3==0x01) {
                    pPict->SeekRel(3);
                    IsVersion2=FALSE;
                }
                else {
                    pPict->SeekRel(4);
                    IsVersion2=TRUE;
                }
            }
            else pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
        }
        else pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
    }
}


ULONG PictReader::ReadData(USHORT nOpcode)
{
    USHORT nUSHORT;
    Point aPoint;
    ULONG nDataSize=0;

    switch(nOpcode) {

    case 0x0000:   // NOP
        nDataSize=0;
        break;

    case 0x0001: { // Clip
        Rectangle aRect;
        *pPict >> nUSHORT;
        nDataSize=nUSHORT;
        ReadRectangle(aRect);
        pVirDev->SetClipRegion( Region( aRect ) );
        break;
    }
    case 0x0002:   // BkPat
        nDataSize=ReadPattern(NULL,&eActBackStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0003:   // TxFont
        *pPict >> nUSHORT;
        if      (nUSHORT <=    1) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT <=   12) aActFont.SetFamily(FAMILY_DECORATIVE);
        else if (nUSHORT <=   20) aActFont.SetFamily(FAMILY_ROMAN);
        else if (nUSHORT ==   21) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT ==   22) aActFont.SetFamily(FAMILY_MODERN);
        else if (nUSHORT <= 1023) aActFont.SetFamily(FAMILY_SWISS);
        else                      aActFont.SetFamily(FAMILY_ROMAN);
        if      ( nUSHORT == 23 ) aActFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
        else    aActFont.SetCharSet( gsl_getSystemTextEncoding() );
        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
        break;

    case 0x0004: {  // TxFace
        char nFace;
        *pPict >> nFace;
        if ( (nFace & 0x01)!=0 ) aActFont.SetWeight(WEIGHT_BOLD);
        else                     aActFont.SetWeight(WEIGHT_NORMAL);
        if ( (nFace & 0x02)!=0 ) aActFont.SetItalic(ITALIC_NORMAL);
        else                     aActFont.SetItalic(ITALIC_NONE);
        if ( (nFace & 0x04)!=0 ) aActFont.SetUnderline(UNDERLINE_SINGLE);
        else                     aActFont.SetUnderline(UNDERLINE_NONE);
        if ( (nFace & 0x08)!=0 ) aActFont.SetOutline(TRUE);
        else                     aActFont.SetOutline(FALSE);
        if ( (nFace & 0x10)!=0 ) aActFont.SetShadow(TRUE);
        else                     aActFont.SetShadow(FALSE);
        eActMethod=PDM_UNDEFINED;
        nDataSize=1;
        break;
    }
    case 0x0005:   // TxMode
        nDataSize=2;
        break;

    case 0x0006:   // SpExtra
        nDataSize=4;
        break;

    case 0x0007: { // PnSize
        Size aSize;
        aSize=ReadSize();
        nActPenSize=(USHORT)((aSize.Width()+aSize.Height())/2);
        eActMethod=PDM_UNDEFINED;
        nDataSize=4;
        break;
    }
    case 0x0008:   // PnMode
        *pPict >> nUSHORT;
        switch (nUSHORT & 0x0007) {
            case 0: eActROP=ROP_OVERPAINT; break; // Copy
            case 1: eActROP=ROP_OVERPAINT; break; // Or
            case 2: eActROP=ROP_XOR;       break; // Xor
            case 3: eActROP=ROP_OVERPAINT; break; // Bic
            case 4: eActROP=ROP_INVERT;    break; // notCopy
            case 5: eActROP=ROP_OVERPAINT; break; // notOr
            case 6: eActROP=ROP_XOR;       break; // notXor
            case 7: eActROP=ROP_OVERPAINT; break; // notBic
        }
        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
        break;

    case 0x0009:   // PnPat
        nDataSize=ReadPattern(&eActPenPenStyle,&eActPenBrushStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x000a:   // FillPat
        nDataSize=ReadPattern(NULL,&eActFillStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x000b:   // OvSize
        aActOvalSize=ReadSize();
        nDataSize=4;
        break;

    case 0x000c:   // Origin
        nDataSize=4;
        break;

    case 0x000d:   // TxSize
    {
        *pPict >> nUSHORT;

        long nTemp = Max( (long) ( Fraction( (long) nUSHORT ) * aVRes ), 12L );
        aActFont.SetSize( Size( 0, nTemp ) );

        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
    }
    break;

    case 0x000e:   // FgColor
        aActForeColor=ReadColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=4;
        break;

    case 0x000f:   // BkColor
        aActBackColor=ReadColor();
        nDataSize=4;
        break;

    case 0x0010:   // TxRatio
        nDataSize=8;
        break;

    case 0x0011:   // VersionOp
        nDataSize=1;
        break;

    case 0x0012:   // BkPixPat
        nDataSize=ReadPixPattern(NULL,&eActBackStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0013:   // PnPixPat
        nDataSize=ReadPixPattern(&eActPenPenStyle,&eActPenBrushStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0014:   // FillPixPat
        nDataSize=ReadPixPattern(NULL,&eActFillStyle);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0015:   // PnLocHFrac
        nDataSize=2;
        break;

    case 0x0016:   // ChExtra
        nDataSize=2;
        break;

    case 0x0017:   // Reserved (0 Bytes)
    case 0x0018:   // Reserved (0 Bytes)
    case 0x0019:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x001a:   // RGBFgCol
        aActForeColor=ReadRGBColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=6;
        break;

    case 0x001b:   // RGBBkCol
        aActBackColor=ReadRGBColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=6;
        break;

    case 0x001c:   // HiliteMode
        nDataSize=0;
        break;

    case 0x001d:   // HiliteColor
        nDataSize=6;
        break;

    case 0x001e:   // DefHilite
        nDataSize=0;
        break;

    case 0x001f:   // OpColor
        nDataSize=6;
        break;

    case 0x0020:   // Line
        aPoint=ReadPoint(); aPenPosition=ReadPoint();
        DrawingMethod(PDM_FRAME);
        pVirDev->DrawLine(aPoint,aPenPosition);
        nDataSize=8;
        break;

    case 0x0021:   // LineFrom
        aPoint=aPenPosition; aPenPosition=ReadPoint();
        DrawingMethod(PDM_FRAME);
        pVirDev->DrawLine(aPoint,aPenPosition);
        nDataSize=4;
        break;

    case 0x0022:   // ShortLine
        aPoint=ReadPoint();
        aPenPosition=ReadDeltaH(aPoint);
        aPenPosition=ReadDeltaV(aPenPosition);
        DrawingMethod(PDM_FRAME);
        pVirDev->DrawLine(aPoint,aPenPosition);
        nDataSize=6;
        break;

    case 0x0023:   // ShortLineFrom
        aPoint=aPenPosition;
        aPenPosition=ReadDeltaH(aPoint);
        aPenPosition=ReadDeltaV(aPenPosition);
        DrawingMethod(PDM_FRAME);
        pVirDev->DrawLine(aPoint,aPenPosition);
        nDataSize=2;
        break;

    case 0x0024:   // Reserved (n Bytes)
    case 0x0025:   // Reserved (n Bytes)
    case 0x0026:   // Reserved (n Bytes)
    case 0x0027:   // Reserved (n Bytes)
        *pPict >> nUSHORT;
        nDataSize=2+nUSHORT;
        break;

    case 0x0028:   // LongText
        aTextPosition=ReadPoint();
        nDataSize=4+ReadAndDrawText();
        break;

    case 0x0029:   // DHText
        aTextPosition=ReadUnsignedDeltaH(aTextPosition);
        nDataSize=1+ReadAndDrawText();
        break;

    case 0x002a:   // DVText
        aTextPosition=ReadUnsignedDeltaV(aTextPosition);
        nDataSize=1+ReadAndDrawText();
        break;

    case 0x002b:   // DHDVText
        aTextPosition=ReadUnsignedDeltaH(aTextPosition);
        aTextPosition=ReadUnsignedDeltaV(aTextPosition);
        nDataSize=2+ReadAndDrawText();
        break;

    case 0x002c: { // fontName
        char        sFName[ 256 ], nByteLen;
        sal_uInt16  nLen;
        *pPict >> nUSHORT; nDataSize=nUSHORT+2;
        *pPict >> nUSHORT;
        if      (nUSHORT <=    1) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT <=   12) aActFont.SetFamily(FAMILY_DECORATIVE);
        else if (nUSHORT <=   20) aActFont.SetFamily(FAMILY_ROMAN);
        else if (nUSHORT ==   21) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT ==   22) aActFont.SetFamily(FAMILY_MODERN);
        else if (nUSHORT <= 1023) aActFont.SetFamily(FAMILY_SWISS);
        else                      aActFont.SetFamily(FAMILY_ROMAN);
        if (nUSHORT==23) aActFont.SetCharSet( RTL_TEXTENCODING_SYMBOL);
        else aActFont.SetCharSet( gsl_getSystemTextEncoding() );
        *pPict >> nByteLen; nLen=((USHORT)nByteLen)&0x00ff;
        pPict->Read( &sFName, nLen );
        sFName[ nLen ] = 0;
        String aString( (const sal_Char*)&sFName, gsl_getSystemTextEncoding() );
        aActFont.SetName( aString );
        eActMethod=PDM_UNDEFINED;
        break;
    }
    case 0x002d:   // lineJustify
        nDataSize=10;
        break;

    case 0x002e:   // glyphState
        *pPict >> nUSHORT;
        nDataSize=2+nUSHORT;
        break;

    case 0x002f:   // Reserved (n Bytes)
        *pPict >> nUSHORT;
        nDataSize=2+nUSHORT;
        break;

    case 0x0030:   // frameRect
        nDataSize=ReadAndDrawRect(PDM_FRAME);
        break;

    case 0x0031:   // paintRect
        nDataSize=ReadAndDrawRect(PDM_PAINT);
        break;

    case 0x0032:   // eraseRect
        nDataSize=ReadAndDrawRect(PDM_ERASE);
        break;

    case 0x0033:   // invertRect
        nDataSize=ReadAndDrawRect(PDM_INVERT);
        break;

    case 0x0034:   // fillRect
        nDataSize=ReadAndDrawRect(PDM_FILL);
        break;

    case 0x0035:   // Reserved (8 Bytes)
    case 0x0036:   // Reserved (8 Bytes)
    case 0x0037:   // Reserved (8 Bytes)
        nDataSize=8;
        break;

    case 0x0038:   // frameSameRect
        nDataSize=ReadAndDrawSameRect(PDM_FRAME);
        break;

    case 0x0039:   // paintSameRect
        nDataSize=ReadAndDrawSameRect(PDM_PAINT);
        break;

    case 0x003a:   // eraseSameRect
        nDataSize=ReadAndDrawSameRect(PDM_ERASE);
        break;

    case 0x003b:   // invertSameRect
        nDataSize=ReadAndDrawSameRect(PDM_INVERT);
        break;

    case 0x003c:   // fillSameRect
        nDataSize=ReadAndDrawSameRect(PDM_FILL);
        break;

    case 0x003d:   // Reserved (0 Bytes)
    case 0x003e:   // Reserved (0 Bytes)
    case 0x003f:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x0040:   // frameRRect
        nDataSize=ReadAndDrawRoundRect(PDM_FRAME);
        break;

    case 0x0041:   // paintRRect
        nDataSize=ReadAndDrawRoundRect(PDM_PAINT);
        break;

    case 0x0042:   // eraseRRect
        nDataSize=ReadAndDrawRoundRect(PDM_ERASE);
        break;

    case 0x0043:   // invertRRect
        nDataSize=ReadAndDrawRoundRect(PDM_INVERT);
        break;

    case 0x0044:   // fillRRect
        nDataSize=ReadAndDrawRoundRect(PDM_FILL);
        break;

    case 0x0045:   // Reserved (8 Bytes)
    case 0x0046:   // Reserved (8 Bytes)
    case 0x0047:   // Reserved (8 Bytes)
        nDataSize=8;
        break;

    case 0x0048:   // frameSameRRect
        nDataSize=ReadAndDrawSameRoundRect(PDM_FRAME);
        break;

    case 0x0049:   // paintSameRRect
        nDataSize=ReadAndDrawSameRoundRect(PDM_PAINT);
        break;

    case 0x004a:   // eraseSameRRect
        nDataSize=ReadAndDrawSameRoundRect(PDM_ERASE);
        break;

    case 0x004b:   // invertSameRRect
        nDataSize=ReadAndDrawSameRoundRect(PDM_INVERT);
        break;

    case 0x004c:   // fillSameRRect
        nDataSize=ReadAndDrawSameRoundRect(PDM_FILL);
        break;

    case 0x004d:   // Reserved (0 Bytes)
    case 0x004e:   // Reserved (0 Bytes)
    case 0x004f:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x0050:   // frameOval
        nDataSize=ReadAndDrawOval(PDM_FRAME);
        break;

    case 0x0051:   // paintOval
        nDataSize=ReadAndDrawOval(PDM_PAINT);
        break;

    case 0x0052:   // eraseOval
        nDataSize=ReadAndDrawOval(PDM_ERASE);
        break;

    case 0x0053:   // invertOval
        nDataSize=ReadAndDrawOval(PDM_INVERT);
        break;

    case 0x0054:   // fillOval
        nDataSize=ReadAndDrawOval(PDM_FILL);
        break;

    case 0x0055:   // Reserved (8 Bytes)
    case 0x0056:   // Reserved (8 Bytes)
    case 0x0057:   // Reserved (8 Bytes)
        nDataSize=8;
        break;

    case 0x0058:   // frameSameOval
        nDataSize=ReadAndDrawSameOval(PDM_FRAME);
        break;

    case 0x0059:   // paintSameOval
        nDataSize=ReadAndDrawSameOval(PDM_PAINT);
        break;

    case 0x005a:   // eraseSameOval
        nDataSize=ReadAndDrawSameOval(PDM_ERASE);
        break;

    case 0x005b:   // invertSameOval
        nDataSize=ReadAndDrawSameOval(PDM_INVERT);
        break;

    case 0x005c:   // fillSameOval
        nDataSize=ReadAndDrawSameOval(PDM_FILL);
        break;

    case 0x005d:   // Reserved (0 Bytes)
    case 0x005e:   // Reserved (0 Bytes)
    case 0x005f:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x0060:   // frameArc
        nDataSize=ReadAndDrawArc(PDM_FRAME);
        break;

    case 0x0061:   // paintArc
        nDataSize=ReadAndDrawArc(PDM_PAINT);
        break;

    case 0x0062:   // eraseArc
        nDataSize=ReadAndDrawArc(PDM_ERASE);
        break;

    case 0x0063:   // invertArc
        nDataSize=ReadAndDrawArc(PDM_INVERT);
        break;

    case 0x0064:   // fillArc
        nDataSize=ReadAndDrawArc(PDM_FILL);
        break;

    case 0x0065:   // Reserved (12 Bytes)
    case 0x0066:   // Reserved (12 Bytes)
    case 0x0067:   // Reserved (12 Bytes)
        nDataSize=12;
        break;

    case 0x0068:   // frameSameArc
        nDataSize=ReadAndDrawSameArc(PDM_FRAME);
        break;

    case 0x0069:   // paintSameArc
        nDataSize=ReadAndDrawSameArc(PDM_PAINT);
        break;

    case 0x006a:   // eraseSameArc
        nDataSize=ReadAndDrawSameArc(PDM_ERASE);
        break;

    case 0x006b:   // invertSameArc
        nDataSize=ReadAndDrawSameArc(PDM_INVERT);
        break;

    case 0x006c:   // fillSameArc
        nDataSize=ReadAndDrawSameArc(PDM_FILL);
        break;

    case 0x006d:   // Reserved (4 Bytes)
    case 0x006e:   // Reserved (4 Bytes)
    case 0x006f:   // Reserved (4 Bytes)
        nDataSize=4;
        break;

    case 0x0070:   // framePoly
        nDataSize=ReadAndDrawPolygon(PDM_FRAME);
        break;

    case 0x0071:   // paintPoly
        nDataSize=ReadAndDrawPolygon(PDM_PAINT);
        break;

    case 0x0072:   // erasePoly
        nDataSize=ReadAndDrawPolygon(PDM_ERASE);
        break;

    case 0x0073:   // invertPoly
        nDataSize=ReadAndDrawPolygon(PDM_INVERT);
        break;

    case 0x0074:   // fillPoly
        nDataSize=ReadAndDrawPolygon(PDM_FILL);
        break;

    case 0x0075:   // Reserved (Polygon-Size)
    case 0x0076:   // Reserved (Polygon-Size)
    case 0x0077:   // Reserved (Polygon-Size)
        *pPict >> nUSHORT; nDataSize=nUSHORT;
        break;

    case 0x0078:   // frameSamePoly
        nDataSize=ReadAndDrawSamePolygon(PDM_FRAME);
        break;

    case 0x0079:   // paintSamePoly
        nDataSize=ReadAndDrawSamePolygon(PDM_PAINT);
        break;

    case 0x007a:   // eraseSamePoly
        nDataSize=ReadAndDrawSamePolygon(PDM_ERASE);
        break;

    case 0x007b:   // invertSamePoly
        nDataSize=ReadAndDrawSamePolygon(PDM_INVERT);
        break;

    case 0x007c:   // fillSamePoly
        nDataSize=ReadAndDrawSamePolygon(PDM_FILL);
        break;

    case 0x007d:   // Reserved (0 Bytes)
    case 0x007e:   // Reserved (0 Bytes)
    case 0x007f:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x0080:   // frameRgn
        nDataSize=ReadAndDrawRgn(PDM_FILL);
        break;

    case 0x0081:   // paintRgn
        nDataSize=ReadAndDrawRgn(PDM_PAINT);
        break;

    case 0x0082:   // eraseRgn
        nDataSize=ReadAndDrawRgn(PDM_ERASE);
        break;

    case 0x0083:   // invertRgn
        nDataSize=ReadAndDrawRgn(PDM_INVERT);
        break;

    case 0x0084:   // fillRgn
        nDataSize=ReadAndDrawRgn(PDM_FILL);
        break;

    case 0x0085:   // Reserved (Region-Size)
    case 0x0086:   // Reserved (Region-Size)
    case 0x0087:   // Reserved (Region-Size)
        *pPict >> nUSHORT; nDataSize=nUSHORT;
        break;

    case 0x0088:   // frameSameRgn
        nDataSize=ReadAndDrawSameRgn(PDM_FRAME);
        break;

    case 0x0089:   // paintSameRgn
        nDataSize=ReadAndDrawSameRgn(PDM_PAINT);
        break;

    case 0x008a:   // eraseSameRgn
        nDataSize=ReadAndDrawSameRgn(PDM_ERASE);
        break;

    case 0x008b:   // invertSameRgn
        nDataSize=ReadAndDrawSameRgn(PDM_INVERT);
        break;

    case 0x008c:   // fillSameRgn
        nDataSize=ReadAndDrawSameRgn(PDM_FILL);
        break;

    case 0x008d:   // Reserved (0 Bytes)
    case 0x008e:   // Reserved (0 Bytes)
    case 0x008f:   // Reserved (0 Bytes)
        nDataSize=0;
        break;

    case 0x0090: { // BitsRect
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, FALSE, TRUE, &aSrcRect, &aDestRect, TRUE, FALSE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0091: { // BitsRgn
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, FALSE, TRUE, &aSrcRect, &aDestRect, TRUE, TRUE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0092:   // Reserved (n Bytes)
    case 0x0093:   // Reserved (n Bytes)
    case 0x0094:   // Reserved (n Bytes)
    case 0x0095:   // Reserved (n Bytes)
    case 0x0096:   // Reserved (n Bytes)
    case 0x0097:   // Reserved (n Bytes)
        *pPict >> nUSHORT; nDataSize=2+nUSHORT;
        break;

    case 0x0098: { // PackBitsRect
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, FALSE, TRUE, &aSrcRect, &aDestRect, TRUE, FALSE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0099: { // PackBitsRgn
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, FALSE, TRUE, &aSrcRect, &aDestRect, TRUE, TRUE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009a: { // DirectBitsRect
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, TRUE, FALSE, &aSrcRect, &aDestRect, TRUE, FALSE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009b: { // DirectBitsRgn
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, TRUE, FALSE, &aSrcRect, &aDestRect, TRUE, TRUE);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009c:   // Reserved (n Bytes)
    case 0x009d:   // Reserved (n Bytes)
    case 0x009e:   // Reserved (n Bytes)
    case 0x009f:   // Reserved (n Bytes)
        *pPict >> nUSHORT; nDataSize=2+nUSHORT;
        break;

    case 0x00a0:   // ShortComment
        nDataSize=2;
        break;

    case 0x00a1:   // LongComment
        pPict->SeekRel(2); *pPict >> nUSHORT; nDataSize=4+nUSHORT;
        break;

    default: // 0x00a2 bis 0xffff (zumeist Reserved)
        if      (nOpcode<=0x00af) { *pPict >> nUSHORT; nDataSize=2+nUSHORT; }
        else if (nOpcode<=0x00cf) { nDataSize=0; }
        else if (nOpcode<=0x00fe) { *pPict >> nDataSize; nDataSize+=4; }
        else if (nOpcode==0x00ff) { nDataSize=2; } // OpEndPic
        else if (nOpcode<=0x01ff) { nDataSize=2; }
        else if (nOpcode<=0x0bfe) { nDataSize=4; }
        else if (nOpcode<=0x0bff) { nDataSize=22; }
        else if (nOpcode==0x0c00) { nDataSize=24; } // HeaderOp
        else if (nOpcode<=0x7eff) { nDataSize=24; }
        else if (nOpcode<=0x7fff) { nDataSize=254; }
        else if (nOpcode<=0x80ff) { nDataSize=0; }
        else                      { *pPict >> nDataSize; nDataSize+=4; }
    }

    if (nDataSize==0xffffffff) {
        pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
        return 0;
    }
    return nDataSize;
}

void PictReader::ReadPict( SvStream & rStreamPict, GDIMetaFile & rGDIMetaFile, PFilterCallback pcallback, void * pcallerdata)
{
    const Fraction  aFrac72( 1, 72 );
    const MapMode   aMap72( MAP_INCH, Point(), aFrac72, aFrac72 );
    USHORT          nOpcode;
    BYTE            nOneByteOpcode;
    ULONG           nSize, nPos, nStartPos, nEndPos, nPercent, nLastPercent;

    pCallback=pcallback; pCallerData=pcallerdata;

    pPict               = &rStreamPict;
    nOrigPos            = pPict->Tell();
    nOrigNumberFormat   = pPict->GetNumberFormatInt();

    aActForeColor       = Color(COL_BLACK);
    aActBackColor       = Color(COL_WHITE);
    eActPenPenStyle     = PEN_SOLID;
    eActPenBrushStyle   = BRUSH_SOLID;
    eActFillStyle       = BRUSH_SOLID;
    eActBackStyle       = BRUSH_SOLID;
    nActPenSize         = 1;
    eActROP             = ROP_OVERPAINT;
    eActMethod          = PDM_UNDEFINED;
    aActOvalSize        = Size(1,1);

    aActFont.SetCharSet( gsl_getSystemTextEncoding() );
    aActFont.SetFamily(FAMILY_SWISS);
    aActFont.SetSize(Size(0,12));
    aActFont.SetAlign(ALIGN_BASELINE);

    aHRes = aVRes = Fraction( 1, 1 );

    pVirDev = new VirtualDevice();
    pVirDev->EnableOutput(FALSE);
    rGDIMetaFile.Record(pVirDev);

    pPict->SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);

    nStartPos=pPict->Tell();
    nEndPos=pPict->Seek(STREAM_SEEK_TO_END); pPict->Seek(nStartPos);
    Callback(0); nLastPercent=0;

    ReadHeader();

    aPenPosition=Point(-aBoundingRect.Left(),-aBoundingRect.Top());
    aTextPosition=aPenPosition;

    nPos=pPict->Tell();

    for (;;) {

        nPercent=(nPos-nStartPos)*100/(nEndPos-nStartPos);
        if (nLastPercent+4<=nPercent) {
            if (Callback((USHORT)nPercent)==TRUE) break;
            nLastPercent=nPercent;
        }

        if (IsVersion2 )
            *pPict >> nOpcode;
        else
        {
            *pPict >> nOneByteOpcode;
            nOpcode=(USHORT)nOneByteOpcode;
        }

        if (pPict->GetError())
            break;

        if (pPict->IsEof())
        {
            pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
            break;
        }

        if (nOpcode==0x00ff)
            break;

        nSize=ReadData(nOpcode);

        if ( IsVersion2 )
        {
            if ( nSize & 1 )
                nSize++;

            nPos+=2+nSize;
        }
        else
            nPos+=1+nSize;

        pPict->Seek(nPos);
    }

    rGDIMetaFile.Stop();
    delete pVirDev;

    rGDIMetaFile.SetPrefMapMode( aMap72 );
    rGDIMetaFile.SetPrefSize( aBoundingRect.GetSize() );

    pPict->SetNumberFormatInt(nOrigNumberFormat);

    if (pPict->GetError()) pPict->Seek(nOrigPos);
}

//================== GraphicImport - die exportierte Funktion ================

#ifdef WNT
extern "C" BOOL _cdecl GraphicImport( SvStream& rIStm, Graphic & rGraphic,
                            PFilterCallback pCallback, void * pCallerData,
                                FilterConfigItem*, BOOL)
#else
extern "C" BOOL GraphicImport( SvStream& rIStm, Graphic & rGraphic,
                            PFilterCallback pCallback, void * pCallerData,
                                FilterConfigItem*, BOOL)
#endif
{
    GDIMetaFile aMTF;
    PictReader  aPictReader;
    BOOL        bRet = FALSE;

    aPictReader.ReadPict( rIStm, aMTF, pCallback, pCallerData );

    if ( !rIStm.GetError() )
    {
        rGraphic = Graphic( aMTF );
        bRet = TRUE;
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
