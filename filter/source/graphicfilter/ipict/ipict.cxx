/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <string.h>
#include <osl/thread.h>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>
#include <tools/poly.hxx>
#include <vcl/virdev.hxx>
#include <math.h>

#include "shape.hxx"

class FilterConfigItem;

namespace PictReaderInternal {
  
  class Pattern {
  public:
    
    Pattern() {
      isColor = false; isRead = false;
      penStyle=PEN_SOLID; brushStyle = BRUSH_SOLID;
      nBitCount = 64;
    }

    
    sal_uLong read(SvStream &stream);
    
    void setColor(Color &col) { isColor = true; color = col; }
    /** returns a color which can be "used" to replace the pattern,
     *     created from ForeColor and BackColor, ...
     *
     * note: maybe, we must also use some mode PatCopy, ... to define the color
     */
    Color getColor(Color bkColor=COL_WHITE, Color fgColor = COL_BLACK) const {
      if (isColor) return color;
      
      double alpha = nBitCount / 64.0;
      return Color(sal_uInt8(alpha*fgColor.GetRed()+(1.0-alpha)*bkColor.GetRed()),
           sal_uInt8(alpha*fgColor.GetGreen()+(1.0-alpha)*bkColor.GetGreen()),
           sal_uInt8(alpha*fgColor.GetBlue()+(1.0-alpha)*bkColor.GetBlue()));
    }

    
    bool isDefault() const { return isRead == false; }

    enum PenStyle { PEN_NULL, PEN_SOLID, PEN_DOT, PEN_DASH, PEN_DASHDOT };
    enum BrushStyle { BRUSH_NULL, BRUSH_SOLID, BRUSH_HORZ, BRUSH_VERT,
              BRUSH_CROSS, BRUSH_DIAGCROSS, BRUSH_UPDIAG, BRUSH_DOWNDIAG,
              BRUSH_25, BRUSH_50, BRUSH_75,
              BRUSH_BITMAP };
    
    enum PenStyle penStyle;
    enum BrushStyle brushStyle;
    short nBitCount;

    bool isColor; 
    Color color;

  protected:
    
    bool isRead;
  };

  sal_uLong Pattern::read(SvStream &stream) {
    short nx,ny;
    unsigned char nbyte[8];
    sal_uLong nHiBytes, nLoBytes;
    isColor = false;

    
    nBitCount=0;
    for (ny=0; ny<8; ny++) {
      stream.ReadChar( (char&)nbyte[ny] );
      for (nx=0; nx<8; nx++) {
    if ( (nbyte[ny] & (1<<nx)) != 0 ) nBitCount++;
      }
    }

    
    nHiBytes=(((((((sal_uLong)nbyte[0])<<8)|
         (sal_uLong)nbyte[1])<<8)|
           (sal_uLong)nbyte[2])<<8)|
      (sal_uLong)nbyte[3];
    nLoBytes=(((((((sal_uLong)nbyte[4])<<8)|
         (sal_uLong)nbyte[5])<<8)|
           (sal_uLong)nbyte[6])<<8)|
      (sal_uLong)nbyte[7];

    
    if      (nBitCount<=0)  penStyle=PEN_NULL;
    else if (nBitCount<=16) penStyle=PEN_DOT;
    else if (nBitCount<=32) penStyle=PEN_DASHDOT;
    else if (nBitCount<=48) penStyle=PEN_DASH;
    else                    penStyle=PEN_SOLID;

    
    if      (nHiBytes==0xffffffff && nLoBytes==0xffffffff) brushStyle=BRUSH_SOLID;
    else if (nHiBytes==0xff000000 && nLoBytes==0x00000000) brushStyle=BRUSH_HORZ;
    else if (nHiBytes==0x80808080 && nLoBytes==0x80808080) brushStyle=BRUSH_VERT;
    else if (nHiBytes==0xff808080 && nLoBytes==0x80808080) brushStyle=BRUSH_CROSS;
    else if (nHiBytes==0x01824428 && nLoBytes==0x10284482) brushStyle=BRUSH_DIAGCROSS;
    else if (nHiBytes==0x80402010 && nLoBytes==0x08040201) brushStyle=BRUSH_UPDIAG;
    else if (nHiBytes==0x01020408 && nLoBytes==0x10204080) brushStyle=BRUSH_DOWNDIAG;
    else if (nBitCount<=24) brushStyle=BRUSH_25;
    else if (nBitCount<=40) brushStyle=BRUSH_50;
    else if (nBitCount<=56) brushStyle=BRUSH_75;
    else                    brushStyle=BRUSH_SOLID;

    isRead = true;

    return 8;
  }
}



enum PictDrawingMethod {
    PDM_FRAME, PDM_PAINT, PDM_ERASE, PDM_INVERT, PDM_FILL,
    PDM_TEXT, PDM_UNDEFINED
};

class PictReader {
  typedef class PictReaderInternal::Pattern Pattern;
private:

    SvStream    * pPict;             
    VirtualDevice * pVirDev;         
                                     

    sal_uLong     nOrigPos;          
    sal_uInt16    nOrigNumberFormat; 
    sal_Bool      IsVersion2;        
    Rectangle     aBoundingRect;     

    Point         aPenPosition;
    Point         aTextPosition;
    Color         aActForeColor;
    Color         aActBackColor;
    Pattern       eActPenPattern;
    Pattern       eActFillPattern;
    Pattern       eActBackPattern;
    Size          nActPenSize;
 
    RasterOp      eActROP;
    PictDrawingMethod eActMethod;
    Size          aActOvalSize;
    Font          aActFont;

    Fraction        aHRes;
    Fraction        aVRes;

    sal_Bool Callback(sal_uInt16 nPercent);

    Point ReadPoint();

    Point ReadDeltaH(Point aBase);
    Point ReadDeltaV(Point aBase);

    Point ReadUnsignedDeltaH(Point aBase);
    Point ReadUnsignedDeltaV(Point aBase);

    Size ReadSize();

    Color ReadColor();

    Color ReadRGBColor();

    void ReadRectangle(Rectangle & rRect);

    sal_uLong ReadPolygon(Polygon & rPoly);

    sal_uLong ReadPixPattern(Pattern &pattern);

    Rectangle aLastRect;
    sal_uLong ReadAndDrawRect(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSameRect(PictDrawingMethod eMethod);

    Rectangle aLastRoundRect;
    sal_uLong ReadAndDrawRoundRect(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSameRoundRect(PictDrawingMethod eMethod);

    Rectangle aLastOval;
    sal_uLong ReadAndDrawOval(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSameOval(PictDrawingMethod eMethod);

    Polygon aLastPolygon;
    sal_uLong ReadAndDrawPolygon(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSamePolygon(PictDrawingMethod eMethod);

    Rectangle aLastArcRect;
    sal_uLong ReadAndDrawArc(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSameArc(PictDrawingMethod eMethod);

    sal_uLong ReadAndDrawRgn(PictDrawingMethod eMethod);
    sal_uLong ReadAndDrawSameRgn(PictDrawingMethod eMethod);

        
        bool IsInvisible(PictDrawingMethod eMethod) const {
      if (eActROP == ROP_1) return true;
      if (eMethod==PDM_FRAME && (nActPenSize.Width() == 0 || nActPenSize.Height() == 0)) return true;
      return false;
    }
    void DrawingMethod(PictDrawingMethod eMethod);

    sal_uLong ReadAndDrawText();

    sal_uLong ReadPixMapEtc(Bitmap & rBitmap, sal_Bool bBaseAddr, sal_Bool bColorTable,
                        Rectangle * pSrcRect, Rectangle * pDestRect,
                        sal_Bool bMode, sal_Bool bMaskRgn);

    void ReadHeader();
        

    sal_uLong ReadData(sal_uInt16 nOpcode);
        
        
        

    void SetLineColor( const Color& rColor );
    void SetFillColor( const Color& rColor );

  
  static rtl_TextEncoding GetTextEncoding (sal_uInt16 fId = 0xFFFF);
public:

  PictReader() { aActFont.SetCharSet(GetTextEncoding()); }

    void ReadPict( SvStream & rStreamPict, GDIMetaFile & rGDIMetaFile );
        

};



#define SETBYTE                                         \
    switch ( nPixelSize )                               \
    {                                                   \
        case 1 :                                        \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 7 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 6 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 5 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 4 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 3 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 2 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 1 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat );      \
            break;                                      \
        case 2 :                                        \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 6 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, (nDat>>4)&3);\
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, (nDat>>2)&3 );\
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat & 3);       \
            break;                                      \
        case 4 :                                        \
            pAcc->SetPixelIndex( ny, nx++, nDat >> 4 ); \
            if ( nx == nWidth ) break;                  \
            pAcc->SetPixelIndex( ny, nx++, nDat );      \
            break;                                      \
        case 8 :                                        \
            pAcc->SetPixelIndex( ny, nx++, nDat );      \
            break;                                      \
    }



#define BITMAPERROR                                     \
{                                                       \
    if ( pAcc )                                         \
        aBitmap.ReleaseAccess( pAcc );                  \
    if ( pReadAcc )                                     \
        aBitmap.ReleaseAccess( pReadAcc );              \
    return 0xffffffff;                                  \
}


rtl_TextEncoding PictReader::GetTextEncoding (sal_uInt16 fId) {
  static bool first = true;
  static rtl_TextEncoding enc = RTL_TEXTENCODING_APPLE_ROMAN;
  if (first) {
    rtl_TextEncoding def = osl_getThreadTextEncoding();
    
    switch(def) {
    case RTL_TEXTENCODING_APPLE_ROMAN:
    case RTL_TEXTENCODING_APPLE_ARABIC:
    case RTL_TEXTENCODING_APPLE_CENTEURO:
    case RTL_TEXTENCODING_APPLE_CROATIAN:
    case RTL_TEXTENCODING_APPLE_CYRILLIC:
    case RTL_TEXTENCODING_APPLE_DEVANAGARI:
    case RTL_TEXTENCODING_APPLE_FARSI:
    case RTL_TEXTENCODING_APPLE_GREEK:
    case RTL_TEXTENCODING_APPLE_GUJARATI:
    case RTL_TEXTENCODING_APPLE_GURMUKHI:
    case RTL_TEXTENCODING_APPLE_HEBREW:
    case RTL_TEXTENCODING_APPLE_ICELAND:
    case RTL_TEXTENCODING_APPLE_ROMANIAN:
    case RTL_TEXTENCODING_APPLE_THAI:
    case RTL_TEXTENCODING_APPLE_TURKISH:
    case RTL_TEXTENCODING_APPLE_UKRAINIAN:
    case RTL_TEXTENCODING_APPLE_CHINSIMP:
    case RTL_TEXTENCODING_APPLE_CHINTRAD:
    case RTL_TEXTENCODING_APPLE_JAPANESE:
    case RTL_TEXTENCODING_APPLE_KOREAN:
      enc = def; break;
    default: break;
    }
    first = false;
  }
  if (fId == 13) return RTL_TEXTENCODING_ADOBE_DINGBATS; 
  if (fId == 23) return RTL_TEXTENCODING_ADOBE_SYMBOL;
  return enc;
}

void PictReader::SetLineColor( const Color& rColor )
{
    pVirDev->SetLineColor( rColor );
}

void PictReader::SetFillColor( const Color& rColor )
{
    pVirDev->SetFillColor( rColor );
}

sal_Bool PictReader::Callback(sal_uInt16 /*nPercent*/)
{
    return sal_False;
}

Point PictReader::ReadPoint()
{
    short nx,ny;

    pPict->ReadInt16( ny ).ReadInt16( nx );

   return Point( (long)nx - aBoundingRect.Left(),
                 (long)ny - aBoundingRect.Top() );
}

Point PictReader::ReadDeltaH(Point aBase)
{
    signed char ndh;

    pPict->ReadChar( (char&)ndh );

    return Point( aBase.X() + (long)ndh, aBase.Y() );
}

Point PictReader::ReadDeltaV(Point aBase)
{
    signed char ndv;

    pPict->ReadChar( (char&)ndv );

    return Point( aBase.X(), aBase.Y() + (long)ndv );
}

Point PictReader::ReadUnsignedDeltaH(Point aBase)
{
    sal_uInt8 ndh;

    pPict->ReadUChar( ndh );

    return Point( aBase.X() + (long)ndh, aBase.Y() );
}

Point PictReader::ReadUnsignedDeltaV(Point aBase)
{
    sal_uInt8 ndv;

    pPict->ReadUChar( ndv );

    return Point( aBase.X(), aBase.Y() + (long)ndv );
}

Size PictReader::ReadSize()
{
    short nx,ny;

    pPict->ReadInt16( ny ).ReadInt16( nx );

    return Size( (long)nx, (long)ny );
}

Color PictReader::ReadColor()
{
    sal_uInt32 nCol;
    Color aCol;

    pPict->ReadUInt32( nCol );
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
    sal_uInt16 nR, nG, nB;

    pPict->ReadUInt16( nR ).ReadUInt16( nG ).ReadUInt16( nB );
    return Color( (sal_uInt8) ( nR >> 8 ), (sal_uInt8) ( nG >> 8 ), (sal_uInt8) ( nB >> 8 ) );
}


void PictReader::ReadRectangle(Rectangle & rRect)
{
    Point aTopLeft, aBottomRight;

    aTopLeft=ReadPoint();
    aBottomRight=ReadPoint();
    rRect=Rectangle(aTopLeft,aBottomRight);
}


sal_uLong PictReader::ReadPolygon(Polygon & rPoly)
{
    sal_uInt16 nSize,i;
    sal_uLong nDataSize;

    pPict->ReadUInt16( nSize );
    pPict->SeekRel(8);
    nDataSize=(sal_uLong)nSize;
    nSize=(nSize-10)/4;
    rPoly.SetSize(nSize);
    for (i=0; i<nSize; i++) rPoly.SetPoint(ReadPoint(),i);
    return nDataSize;
}

sal_uLong PictReader::ReadPixPattern(PictReader::Pattern &pattern)
{
    
    
    


    sal_uLong nDataSize;
    sal_uInt16 nPatType;
    Bitmap aBMP;

    pPict->ReadUInt16( nPatType );
    if (nPatType==1) {
            pattern.read(*pPict);
        nDataSize=ReadPixMapEtc(aBMP,sal_False,sal_True,NULL,NULL,sal_False,sal_False);
        
        if (nDataSize!=0xffffffff) nDataSize+=10;
    }
    else if (nPatType==2) {
            pattern.read(*pPict);
        
        sal_uInt16 nR, nG, nB;
        pPict->ReadUInt16( nR ).ReadUInt16( nG ).ReadUInt16( nB );
        Color col((sal_uInt8) ( nR >> 8 ), (sal_uInt8) ( nG >> 8 ), (sal_uInt8) ( nB >> 8 ) );
        pattern.setColor(col);
        nDataSize=16;
    }
    else nDataSize=0xffffffff;

    return nDataSize;
}

sal_uLong PictReader::ReadAndDrawRect(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastRect);
    ReadAndDrawSameRect(eMethod);
    return 8;
}

sal_uLong PictReader::ReadAndDrawSameRect(PictDrawingMethod eMethod)
{
    if (IsInvisible(eMethod)) return 0;
    DrawingMethod(eMethod);
    PictReaderShape::drawRectangle(pVirDev, eMethod==PDM_FRAME, aLastRect, nActPenSize);
    return 0;
}

sal_uLong PictReader::ReadAndDrawRoundRect(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastRoundRect);
    ReadAndDrawSameRoundRect(eMethod);
    return 8;
}

sal_uLong PictReader::ReadAndDrawSameRoundRect(PictDrawingMethod eMethod)
{
    if (IsInvisible(eMethod)) return 0;
    DrawingMethod(eMethod);
    PictReaderShape::drawRoundRectangle(pVirDev, eMethod==PDM_FRAME, aLastRoundRect, aActOvalSize, nActPenSize);
    return 0;
}

sal_uLong PictReader::ReadAndDrawOval(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastOval);
    ReadAndDrawSameOval(eMethod);
    return 8;
}

sal_uLong PictReader::ReadAndDrawSameOval(PictDrawingMethod eMethod)
{
    if (IsInvisible(eMethod)) return 0;
    DrawingMethod(eMethod);
    PictReaderShape::drawEllipse(pVirDev, eMethod==PDM_FRAME, aLastOval, nActPenSize);
    return 0;
}

sal_uLong PictReader::ReadAndDrawPolygon(PictDrawingMethod eMethod)
{
    sal_uLong nDataSize;
    nDataSize=ReadPolygon(aLastPolygon);
    ReadAndDrawSamePolygon(eMethod);
    return nDataSize;
}

sal_uLong PictReader::ReadAndDrawSamePolygon(PictDrawingMethod eMethod)
{
    if (IsInvisible(eMethod)) return 0;
    DrawingMethod(eMethod);
    PictReaderShape::drawPolygon(pVirDev, eMethod==PDM_FRAME, aLastPolygon, nActPenSize);
    return 0;
}


sal_uLong PictReader::ReadAndDrawArc(PictDrawingMethod eMethod)
{
    ReadRectangle(aLastArcRect);
    ReadAndDrawSameArc(eMethod);
    return 12;
}

sal_uLong PictReader::ReadAndDrawSameArc(PictDrawingMethod eMethod)
{
    short nstartAngle, narcAngle;
    double fAng1, fAng2;

    pPict->ReadInt16( nstartAngle ).ReadInt16( narcAngle );
    if (IsInvisible(eMethod)) return 4;
    DrawingMethod(eMethod);

    if (narcAngle<0) {
        nstartAngle = nstartAngle + narcAngle;
        narcAngle=-narcAngle;
    }
    fAng1=((double)nstartAngle)/180.0*3.14159265359;
    fAng2=((double)(nstartAngle+narcAngle))/180.0*3.14159265359;
    PictReaderShape::drawArc(pVirDev, eMethod==PDM_FRAME, aLastArcRect,fAng1,fAng2, nActPenSize);
    return 4;
}

sal_uLong PictReader::ReadAndDrawRgn(PictDrawingMethod eMethod)
{
    sal_uInt16 nSize;

    pPict->ReadUInt16( nSize );
    
    //
    
    
    
    
    
    

    
    
    
    
    
    ReadAndDrawSameRgn(eMethod);
    return (sal_uLong)nSize;
}

sal_uLong PictReader::ReadAndDrawSameRgn(PictDrawingMethod eMethod)
{
    if (IsInvisible(eMethod)) return 0;
    DrawingMethod(eMethod);
    
    return 0;
}

void PictReader::DrawingMethod(PictDrawingMethod eMethod)
{
    if( eActMethod==eMethod ) return;
    switch (eMethod) {
        case PDM_FRAME:
                if (eActPenPattern.isDefault())
              SetLineColor( aActForeColor );
            else
              SetLineColor(eActPenPattern.getColor(aActBackColor, aActForeColor));
            SetFillColor( Color(COL_TRANSPARENT) );
            pVirDev->SetRasterOp(eActROP);
            break;
        case PDM_PAINT:
            SetLineColor( Color(COL_TRANSPARENT) );
            if (eActPenPattern.isDefault())
              SetFillColor( aActForeColor );
            else
              SetFillColor(eActPenPattern.getColor(aActBackColor, aActForeColor));
            pVirDev->SetRasterOp(eActROP);
            break;
        case PDM_ERASE:
            SetLineColor( Color(COL_TRANSPARENT) );
            if (eActBackPattern.isDefault())
              SetFillColor( aActBackColor );
            else 
              SetFillColor(eActBackPattern.getColor(COL_BLACK, aActBackColor));
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
            case PDM_INVERT: 
            SetLineColor( Color(COL_TRANSPARENT));
            SetFillColor( Color( COL_BLACK ) );
            pVirDev->SetRasterOp(ROP_INVERT);
            break;
        case PDM_FILL:
            SetLineColor( Color(COL_TRANSPARENT) );
            if (eActFillPattern.isDefault())
              SetFillColor( aActForeColor );
            else
              SetFillColor(eActFillPattern.getColor(aActBackColor, aActForeColor));
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
        case PDM_TEXT:
            aActFont.SetColor(aActForeColor);
            aActFont.SetFillColor(aActBackColor);
            aActFont.SetTransparent(true);
            pVirDev->SetFont(aActFont);
            pVirDev->SetRasterOp(ROP_OVERPAINT);
            break;
        default:
            break;  
    }
    eActMethod=eMethod;
}

sal_uLong PictReader::ReadAndDrawText()
{
    char        nByteLen;
    sal_uInt32  nLen, nDataLen;
    sal_Char    sText[256];

    pPict->ReadChar( nByteLen ); nLen=((sal_uLong)nByteLen)&0x000000ff;
    nDataLen = nLen + 1;
    pPict->Read( &sText, nLen );

    if (IsInvisible(PDM_TEXT)) return nDataLen;
    DrawingMethod(PDM_TEXT);

    
    while ( nLen > 0 && ( (unsigned char)sText[ nLen - 1 ] ) < 32 )
            nLen--;
    sText[ nLen ] = 0;
    OUString aString( (const sal_Char*)&sText, strlen(sText), aActFont.GetCharSet());
    pVirDev->DrawText( Point( aTextPosition.X(), aTextPosition.Y() ), aString );
    return nDataLen;
}

sal_uLong PictReader::ReadPixMapEtc( Bitmap &rBitmap, sal_Bool bBaseAddr, sal_Bool bColorTable, Rectangle* pSrcRect,
                                    Rectangle* pDestRect, sal_Bool bMode, sal_Bool bMaskRgn )
{
    Bitmap              aBitmap;
    BitmapWriteAccess*  pAcc = NULL;
    BitmapReadAccess*   pReadAcc = NULL;
    sal_uInt16              ny, nx, nColTabSize;
    sal_uInt16              nRowBytes, nBndX, nBndY, nWidth, nHeight, nVersion, nPackType, nPixelType,
                        nPixelSize, nCmpCount, nCmpSize;
    sal_uInt32          nPackSize, nPlaneBytes, nHRes, nVRes;
    sal_uInt8               nDat, nRed, nGreen, nBlue, nDummy;
    sal_uLong               i, nDataSize = 0;

    
    nDataSize = 0;

    
    if ( bBaseAddr )
    {
        pPict->SeekRel( 4 );
        nDataSize += 4;
    }

    
    pPict->ReadUInt16( nRowBytes ).ReadUInt16( nBndY ).ReadUInt16( nBndX ).ReadUInt16( nHeight ).ReadUInt16( nWidth );
    nHeight = nHeight - nBndY;
    nWidth = nWidth - nBndX;

    if ( ( nRowBytes & 0x8000 ) != 0 )
    {   
        nRowBytes &= 0x3fff;
        pPict->ReadUInt16( nVersion ).ReadUInt16( nPackType ).ReadUInt32( nPackSize ).ReadUInt32( nHRes ).ReadUInt32( nVRes ).ReadUInt16( nPixelType ).                    ReadUInt16( nPixelSize ).ReadUInt16( nCmpCount ).ReadUInt16( nCmpSize ).ReadUInt32( nPlaneBytes );

        pPict->SeekRel( 8 );
        nDataSize += 46;

        sal_uInt16 nDstBitCount = nPixelSize;
        if ( nDstBitCount > 8 )
            nDstBitCount = 24;
        else if ( nDstBitCount == 2 )
            nDstBitCount = 4;
        aBitmap = Bitmap( Size( nWidth, nHeight ), nDstBitCount );

        if ( ( pAcc = aBitmap.AcquireWriteAccess() ) == NULL )
            BITMAPERROR;

        if ( bColorTable )
        {
            pPict->SeekRel( 6 );
            pPict->ReadUInt16( nColTabSize );

            if ( ++nColTabSize > 256 )
                BITMAPERROR;

            pAcc->SetPaletteEntryCount( nColTabSize );

            for ( i = 0; i < nColTabSize; i++ )
            {
                pPict->SeekRel(2);
                pPict->ReadUChar( nRed ).ReadUChar( nDummy ).ReadUChar( nGreen ).ReadUChar( nDummy ).ReadUChar( nBlue ).ReadUChar( nDummy );
                pAcc->SetPaletteColor( (sal_uInt16) i, BitmapColor( nRed, nGreen, nBlue ) );
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

    
    if ( pSrcRect != 0)
    {
        sal_uInt16  nTop, nLeft, nBottom, nRight;
        pPict->ReadUInt16( nTop ).ReadUInt16( nLeft ).ReadUInt16( nBottom ).ReadUInt16( nRight );
        *pSrcRect = Rectangle( (sal_uLong)nLeft, (sal_uLong)nTop, (sal_uLong)nRight, (sal_uLong)nBottom );
        nDataSize += 8;
    }

    
    if ( pDestRect != 0 )
    {
        Point aTL, aBR;
        aTL = ReadPoint();
        aBR = ReadPoint();
        *pDestRect = Rectangle( aTL, aBR );
        nDataSize += 8;
    }

    
    if ( bMode )
    {
        pPict->SeekRel(2);
        nDataSize += 2;
    }

    
    if ( bMaskRgn )
    {
        sal_uInt16 nSize;
        pPict->ReadUInt16( nSize );
        pPict->SeekRel( nSize - 2 );
        nDataSize += (sal_uLong)nSize;
    }



    
    if ( nPixelSize == 1 || nPixelSize == 2 || nPixelSize == 4 || nPixelSize == 8 )
    {
        sal_uInt8   nByteCountAsByte, nFlagCounterByte;
        sal_uInt16  nByteCount, nCount, nSrcBPL, nDestBPL;

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
                {
                    pPict->ReadUChar( nDat );
                    if ( nx < nWidth )
                        SETBYTE;
                }
                nDataSize += nRowBytes;
            }
            else
            {
                if ( nRowBytes > 250 )
                {
                    pPict->ReadUInt16( nByteCount );
                    nDataSize += 2 + (sal_uLong)nByteCount;
                }
                else
                {
                    pPict->ReadUChar( nByteCountAsByte );
                    nByteCount = ( (sal_uInt16)nByteCountAsByte ) & 0x00ff;
                    nDataSize += 1 + (sal_uLong)nByteCount;
                }

                while ( nByteCount )
                {
                    pPict->ReadUChar( nFlagCounterByte );
                    if ( ( nFlagCounterByte & 0x80 ) == 0 )
                    {
                        nCount = ( (sal_uInt16)nFlagCounterByte ) + 1;
                        for ( i = 0; i < nCount; i++ )
                        {
                            pPict->ReadUChar( nDat );
                            if ( nx < nWidth )
                                SETBYTE;
                        }
                        nByteCount -= 1 + nCount;
                    }
                    else
                    {
                        nCount = ( 1 - ( ( (sal_uInt16)nFlagCounterByte ) | 0xff00 ) );
                        pPict->ReadUChar( nDat );
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
        sal_uInt8   nByteCountAsByte, nFlagCounterByte;
        sal_uInt16  nByteCount, nCount, nD;
        sal_uLong   nSrcBitsPos;

        if ( nRowBytes < 2 * nWidth )
            BITMAPERROR;

        for ( ny = 0; ny < nHeight; ny++ )
        {
            nx = 0;
            if ( nRowBytes < 8 || nPackType == 1 )
            {
                for ( i = 0; i < nWidth; i++ )
                {
                    pPict->ReadUInt16( nD );
                    nRed = (sal_uInt8)( nD >> 7 );
                    nGreen = (sal_uInt8)( nD >> 2 );
                    nBlue = (sal_uInt8)( nD << 3 );
                    pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                }
                nDataSize += ( (sal_uLong)nWidth ) * 2;
            }
            else
            {
                nSrcBitsPos = pPict->Tell();
                if ( nRowBytes > 250 )
                {
                    pPict->ReadUInt16( nByteCount );
                    nByteCount += 2;
                }
                else
                {
                    pPict->ReadUChar( nByteCountAsByte );
                    nByteCount = ( (sal_uInt16)nByteCountAsByte ) & 0x00ff;
                    nByteCount++;
                }
                while ( nx != nWidth )
                {
                    pPict->ReadUChar( nFlagCounterByte );
                    if ( (nFlagCounterByte & 0x80) == 0)
                    {
                        nCount=((sal_uInt16)nFlagCounterByte)+1;
                        if ( nCount + nx > nWidth)              
                            nCount = nWidth - nx;               
                        for (i=0; i<nCount; i++)                
                        {                                       
                            pPict->ReadUInt16( nD );
                            nRed = (sal_uInt8)( nD >> 7 );
                            nGreen = (sal_uInt8)( nD >> 2 );
                            nBlue = (sal_uInt8)( nD << 3 );
                            pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                    else
                    {
                        nCount=(1-(((sal_uInt16)nFlagCounterByte)|0xff00));
                        if ( nCount + nx > nWidth )
                            nCount = nWidth - nx;
                        pPict->ReadUInt16( nD );
                        nRed = (sal_uInt8)( nD >> 7 );
                        nGreen = (sal_uInt8)( nD >> 2 );
                        nBlue = (sal_uInt8)( nD << 3 );
                        for (i=0; i<nCount; i++)
                        {
                            pAcc->SetPixel( ny, nx++, BitmapColor( nRed, nGreen, nBlue ) );
                        }
                    }
                }
                nDataSize+=(sal_uLong)nByteCount;
                pPict->Seek(nSrcBitsPos+(sal_uLong)nByteCount);
            }
        }
    }
    else if (nPixelSize==32)
    {
        sal_uInt8               nByteCountAsByte, nFlagCounterByte;
        sal_uInt16              nByteCount, nCount;
        sal_uLong               nSrcBitsPos;
        BitmapColor         aBitmapColor;
        if ( ( pReadAcc = aBitmap.AcquireReadAccess() ) == NULL )
            BITMAPERROR;
        if ( nRowBytes != 4*nWidth )
            BITMAPERROR;

        if ( nRowBytes < 8 || nPackType == 1 )
        {
            for ( ny = 0; ny < nHeight; ny++ )
            {
                if ( nRowBytes < 8 || nPackType == 1 )
                {
                    for ( nx = 0; nx < nWidth; nx++ )
                    {
                        pPict->ReadUChar( nDummy ).ReadUChar( nRed ).ReadUChar( nGreen ).ReadUChar( nBlue );
                        pAcc->SetPixel( ny, nx, BitmapColor( nRed, nGreen, nBlue) );
                    }
                    nDataSize += ( (sal_uLong)nWidth ) * 4;
                }
            }
        }
        else if ( nPackType == 2 )
        {
            for ( ny = 0; ny < nHeight; ny++ )
            {
                for ( nx = 0; nx < nWidth; nx++ )
                {
                    pPict->ReadUChar( nRed ).ReadUChar( nGreen ).ReadUChar( nBlue );
                    pAcc->SetPixel( ny, nx, BitmapColor( nRed, nGreen, nBlue ) );
                }
                nDataSize += ( (sal_uLong)nWidth ) * 3;
            }
        }
        else
        {
            if ( ( nCmpCount == 3 ) || ( nCmpCount == 4 ) )
            {
                sal_uInt8* pScanline = new sal_uInt8[ nWidth * nCmpCount ];
                for ( ny = 0; ny < nHeight; ny++ )
                {
                    nSrcBitsPos = pPict->Tell();
                    if ( nRowBytes > 250 )
                    {
                        pPict->ReadUInt16( nByteCount );
                        nByteCount += 2;
                    }
                    else
                    {
                        pPict->ReadUChar( nByteCountAsByte );
                        nByteCount = (sal_uInt8)nByteCountAsByte;
                        nByteCount++;
                    }
                    i = 0;
                    while( i < (sal_uInt32)( nWidth * nCmpCount ) )
                    {
                        pPict->ReadUChar( nFlagCounterByte );
                        if ( ( nFlagCounterByte & 0x80 ) == 0)
                        {
                            nCount = ( (sal_uInt16)nFlagCounterByte ) + 1;
                            if ( ( i + nCount ) > (sal_uInt32)( nWidth * nCmpCount ) )
                                nCount = (sal_uInt16)( nWidth * nCmpCount - i );
                            while( nCount-- )
                            {
                                pPict->ReadUChar( nDat );
                                pScanline[ i++ ] = nDat;
                            }
                        }
                        else
                        {
                            nCount = ( 1 - ( ( (sal_uInt16)nFlagCounterByte ) | 0xff00 ) );
                            if ( ( i + nCount ) > (sal_uInt32)( nWidth * nCmpCount ) )
                                nCount = (sal_uInt16)( nWidth * nCmpCount - i );
                            pPict->ReadUChar( nDat );
                            while( nCount-- )
                                pScanline[ i++ ] = nDat;
                        }
                    }
                    sal_uInt8* pTmp = pScanline;
                    if ( nCmpCount == 4 )
                        pTmp += nWidth;
                    for ( nx = 0; nx < nWidth; pTmp++ )
                        pAcc->SetPixel( ny, nx++, BitmapColor( *pTmp, pTmp[ nWidth ], pTmp[ 2 * nWidth ] ) );
                    nDataSize += (sal_uLong)nByteCount;
                    pPict->Seek( nSrcBitsPos + (sal_uLong)nByteCount );
                }
                delete[] pScanline;
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
    short y1,x1,y2,x2;

    sal_Char    sBuf[ 2 ];
    
    
    sal_uLong   nStartPos = pPict->Tell();
    
    
    

    
    
    
    
    

    
    
    

    
    
    
    
    
    int st;
    sal_uInt32 nOffset;
    int confidence[2] = { 0, 0};
    for ( st = 0; st < 3 + 513; st++ )
      {
        int actualConfid = 20; 
        pPict->ResetError();
        if (st < 2) nOffset = nStartPos+st*512;
        else if (st == 2) {
          
          int actPos = -1, actConf=0;
          if (confidence[0] > 0) { actPos = 0; actConf =  confidence[0]; }
          if (confidence[1] > 0 && confidence[1] >= actConf) actPos = 1;
          if (actPos < 0) continue;
          nOffset = nStartPos+actPos*512;
        }
        else {
          nOffset = 509+st; 
          
          pPict->Seek(nOffset+10);
          pPict->Read( sBuf, 2 );
          if (pPict->IsEof() || pPict->GetError()) break;
          if (sBuf[0] == 0x11 || (sBuf[0] == 0x00 && sBuf[1] == 0x11)) ; 
          else continue;
        }
        pPict->Seek(nOffset);

        
        pPict->SeekRel( 2 );
        pPict->ReadInt16( y1 ).ReadInt16( x1 ).ReadInt16( y2 ).ReadInt16( x2 ); 
        if (x1 > x2 || y1 > y2) continue; 
        if (x1 < -2048 || x2 > 2048 || y1 < -2048 || y2 > 2048 || 
        (x1 == x2 && y1 == y2) ) 
          actualConfid-=3;
        else if (x2 < x1+8 || y2 < y1+8) 
          actualConfid-=1;
        if (st >= 3 && actualConfid != 20) continue;
        aBoundingRect=Rectangle( x1,y1, x2, y2 );

        if (pPict->IsEof() || pPict->GetError()) continue;
        
        pPict->Read( sBuf, 2 );
        
        if ( sBuf[ 0 ] == 0x11 && sBuf[ 1 ] == 0x01 ) {
          
          if (st < 2) { confidence[st] = --actualConfid; continue; }
          IsVersion2 = sal_False; return;
        }
        if (sBuf[0] != 0x00) continue; 
        int numZero = 0;
        do
          {
        numZero++;
        pPict->SeekRel(-1);
        pPict->Read( sBuf, 2 );
          }
        while ( sBuf[0] == 0x00 && numZero < 10);
        actualConfid -= (numZero-1); 
        if (pPict->IsEof() || pPict->GetError()) continue;
        if (sBuf[0] != 0x11) continue; 
        
        if (sBuf[1] == 0x01 ) {
          
          if (st < 2) { confidence[st] = --actualConfid; continue; }
          IsVersion2 = sal_False; return;
        }
        if (sBuf[1] != 0x02 ) continue; 

        IsVersion2=sal_True;
        short   nExtVer, nReserved;
        
        pPict->SeekRel( 3 );
        pPict->ReadInt16( nExtVer ).ReadInt16( nReserved );
        if (pPict->IsEof() || pPict->GetError()) continue;

        if ( nExtVer == -2 ) 
          {
        sal_Int32 nHResFixed, nVResFixed;
        pPict->ReadInt32( nHResFixed ).ReadInt32( nVResFixed );
        pPict->ReadInt16( y1 ).ReadInt16( x1 ).ReadInt16( y2 ).ReadInt16( x2 ); 
        if (x1 > x2 || y1 > y2) continue; 
        if (st < 2 && actualConfid != 20) { confidence[st] = actualConfid; continue; }

        double fHRes = nHResFixed;
        fHRes /= 65536;
        double fVRes = nVResFixed;
        fVRes /= 65536;
        aHRes /= fHRes;
        aVRes /= fVRes;
        aBoundingRect=Rectangle( x1,y1, x2, y2 );
        pPict->SeekRel( 4 ); 
        return;
          }
        else if (nExtVer == -1 ) { 
          if (st < 2 && actualConfid != 20) { confidence[st] = actualConfid; continue; }
          pPict->SeekRel( 16); 
          pPict->SeekRel(4); 
          return;
        }
      }
    pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
}

sal_uLong PictReader::ReadData(sal_uInt16 nOpcode)
{
    sal_uInt16 nUSHORT;
    Point aPoint;
    sal_uLong nDataSize=0;
    PictDrawingMethod shapeDMethod = PDM_UNDEFINED;
    switch (nOpcode & 7) {
    case 0: shapeDMethod = PDM_FRAME; break;
    case 1: shapeDMethod = PDM_PAINT; break;
    case 2: shapeDMethod = PDM_ERASE; break;
    case 3: shapeDMethod = PDM_INVERT; break;
    case 4: shapeDMethod = PDM_FILL; break;
    default: break;
    }

    switch(nOpcode) {

    case 0x0000:   
        nDataSize=0;
        break;

    case 0x0001: { 
        Rectangle aRect;
        pPict->ReadUInt16( nUSHORT );
        nDataSize=nUSHORT;
        ReadRectangle(aRect);
        
        
        
        
               
                aRect.setWidth(aRect.getWidth()+1);
        aRect.setHeight(aRect.getHeight()+1);
        pVirDev->SetClipRegion( Region( aRect ) );
        break;
    }
    case 0x0002:   
      nDataSize=eActBackPattern.read(*pPict);
      eActMethod=PDM_UNDEFINED;
      break;

    case 0x0003:   
        pPict->ReadUInt16( nUSHORT );
        if      (nUSHORT <=    1) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT <=   12) aActFont.SetFamily(FAMILY_DECORATIVE);
        else if (nUSHORT <=   20) aActFont.SetFamily(FAMILY_ROMAN);
        else if (nUSHORT ==   21) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT ==   22) aActFont.SetFamily(FAMILY_MODERN);
        else if (nUSHORT <= 1023) aActFont.SetFamily(FAMILY_SWISS);
        else                      aActFont.SetFamily(FAMILY_ROMAN);
        aActFont.SetCharSet(GetTextEncoding(nUSHORT));
        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
        break;

    case 0x0004: {  
        char nFace;
        pPict->ReadChar( nFace );
        if ( (nFace & 0x01)!=0 ) aActFont.SetWeight(WEIGHT_BOLD);
        else                     aActFont.SetWeight(WEIGHT_NORMAL);
        if ( (nFace & 0x02)!=0 ) aActFont.SetItalic(ITALIC_NORMAL);
        else                     aActFont.SetItalic(ITALIC_NONE);
        if ( (nFace & 0x04)!=0 ) aActFont.SetUnderline(UNDERLINE_SINGLE);
        else                     aActFont.SetUnderline(UNDERLINE_NONE);
        if ( (nFace & 0x08)!=0 ) aActFont.SetOutline(true);
        else                     aActFont.SetOutline(false);
        if ( (nFace & 0x10)!=0 ) aActFont.SetShadow(true);
        else                     aActFont.SetShadow(false);
        eActMethod=PDM_UNDEFINED;
        nDataSize=1;
        break;
    }
    case 0x0005:   
        nDataSize=2;
        break;

    case 0x0006:   
        nDataSize=4;
        break;

    case 0x0007: { 
        nActPenSize=ReadSize();
        eActMethod=PDM_UNDEFINED;
        nDataSize=4;
        break;
    }
    case 0x0008:   
        pPict->ReadUInt16( nUSHORT );
        
        if (nUSHORT==23) eActROP = ROP_1;
        else {
          switch (nUSHORT & 0x0007) {
            case 0: eActROP=ROP_OVERPAINT; break; 
            case 1: eActROP=ROP_OVERPAINT; break; 
            case 2: eActROP=ROP_XOR;       break; 
            case 3: eActROP=ROP_OVERPAINT; break; 
            case 4: eActROP=ROP_INVERT;    break; 
            case 5: eActROP=ROP_OVERPAINT; break; 
            case 6: eActROP=ROP_XOR;       break; 
            case 7: eActROP=ROP_OVERPAINT; break; 
          }
        }
        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
        break;

    case 0x0009:   
      nDataSize=eActPenPattern.read(*pPict);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x000a:   
      nDataSize=eActFillPattern.read(*pPict);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x000b:   
        aActOvalSize=ReadSize();
        nDataSize=4;
        break;

    case 0x000c:   
        nDataSize=4;
        break;

    case 0x000d:   
    {
        pPict->ReadUInt16( nUSHORT );
        aActFont.SetSize( Size( 0, (long)nUSHORT ) );
        eActMethod=PDM_UNDEFINED;
        nDataSize=2;
    }
    break;

    case 0x000e:   
        aActForeColor=ReadColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=4;
        break;

    case 0x000f:   
        aActBackColor=ReadColor();
        nDataSize=4;
        break;

    case 0x0010:   
        nDataSize=8;
        break;

    case 0x0011:   
        nDataSize=1;
        break;

    case 0x0012:   
        nDataSize=ReadPixPattern(eActBackPattern);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0013:   
        nDataSize=ReadPixPattern(eActPenPattern);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0014:   
        nDataSize=ReadPixPattern(eActFillPattern);
        eActMethod=PDM_UNDEFINED;
        break;

    case 0x0015:   
        nDataSize=2;
        break;

    case 0x0016:   
        nDataSize=2;
        break;

    case 0x0017:   
    case 0x0018:   
    case 0x0019:   
        nDataSize=0;
        break;

    case 0x001a:   
        aActForeColor=ReadRGBColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=6;
        break;

    case 0x001b:   
        aActBackColor=ReadRGBColor();
        eActMethod=PDM_UNDEFINED;
        nDataSize=6;
        break;

    case 0x001c:   
        nDataSize=0;
        break;

    case 0x001d:   
        nDataSize=6;
        break;

    case 0x001e:   
        nDataSize=0;
        break;

    case 0x001f:   
        nDataSize=6;
        break;

    case 0x0020:   
        aPoint=ReadPoint(); aPenPosition=ReadPoint();
        nDataSize=8;

        if (IsInvisible(PDM_FRAME)) break;
        DrawingMethod(PDM_FRAME);
        PictReaderShape::drawLine(pVirDev, aPoint,aPenPosition, nActPenSize);
        break;

    case 0x0021:   
        aPoint=aPenPosition; aPenPosition=ReadPoint();
        nDataSize=4;

        if (IsInvisible(PDM_FRAME)) break;
        DrawingMethod(PDM_FRAME);
        PictReaderShape::drawLine(pVirDev, aPoint,aPenPosition, nActPenSize);
        break;

    case 0x0022:   
        aPoint=ReadPoint();
        aPenPosition=ReadDeltaH(aPoint);
        aPenPosition=ReadDeltaV(aPenPosition);
        nDataSize=6;

        if (IsInvisible(PDM_FRAME)) break;
        DrawingMethod(PDM_FRAME);
        PictReaderShape::drawLine(pVirDev, aPoint,aPenPosition, nActPenSize);
        break;

    case 0x0023:   
        aPoint=aPenPosition;
        aPenPosition=ReadDeltaH(aPoint);
        aPenPosition=ReadDeltaV(aPenPosition);
        nDataSize=2;

        if (IsInvisible(PDM_FRAME)) break;
        DrawingMethod(PDM_FRAME);
        PictReaderShape::drawLine(pVirDev, aPoint,aPenPosition, nActPenSize);
        break;

    case 0x0024:   
    case 0x0025:   
    case 0x0026:   
    case 0x0027:   
        pPict->ReadUInt16( nUSHORT );
        nDataSize=2+nUSHORT;
        break;

    case 0x0028:   
        aTextPosition=ReadPoint();
        nDataSize=4+ReadAndDrawText();
        break;

    case 0x0029:   
        aTextPosition=ReadUnsignedDeltaH(aTextPosition);
        nDataSize=1+ReadAndDrawText();
        break;

    case 0x002a:   
        aTextPosition=ReadUnsignedDeltaV(aTextPosition);
        nDataSize=1+ReadAndDrawText();
        break;

    case 0x002b:   
        aTextPosition=ReadUnsignedDeltaH(aTextPosition);
        aTextPosition=ReadUnsignedDeltaV(aTextPosition);
        nDataSize=2+ReadAndDrawText();
        break;

    case 0x002c: { 
        char        sFName[ 256 ], nByteLen;
        sal_uInt16  nLen;
        pPict->ReadUInt16( nUSHORT ); nDataSize=nUSHORT+2;
        pPict->ReadUInt16( nUSHORT );
        if      (nUSHORT <=    1) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT <=   12) aActFont.SetFamily(FAMILY_DECORATIVE);
        else if (nUSHORT <=   20) aActFont.SetFamily(FAMILY_ROMAN);
        else if (nUSHORT ==   21) aActFont.SetFamily(FAMILY_SWISS);
        else if (nUSHORT ==   22) aActFont.SetFamily(FAMILY_MODERN);
        else if (nUSHORT <= 1023) aActFont.SetFamily(FAMILY_SWISS);
        else                      aActFont.SetFamily(FAMILY_ROMAN);
        aActFont.SetCharSet(GetTextEncoding(nUSHORT));
        pPict->ReadChar( nByteLen ); nLen=((sal_uInt16)nByteLen)&0x00ff;
        pPict->Read( &sFName, nLen );
        sFName[ nLen ] = 0;
        OUString aString( (const sal_Char*)&sFName, strlen(sFName), osl_getThreadTextEncoding() );
        aActFont.SetName( aString );
        eActMethod=PDM_UNDEFINED;
        break;
    }
    case 0x002d:   
        nDataSize=10;
        break;

    case 0x002e:   
        pPict->ReadUInt16( nUSHORT );
        nDataSize=2+nUSHORT;
        break;

    case 0x002f:   
        pPict->ReadUInt16( nUSHORT );
        nDataSize=2+nUSHORT;
        break;

    case 0x0030:   
    case 0x0031:   
    case 0x0032:   
    case 0x0033:   
    case 0x0034:   
        nDataSize=ReadAndDrawRect(shapeDMethod);
        break;

    case 0x0035:   
    case 0x0036:   
    case 0x0037:   
        nDataSize=8;
        break;

    case 0x0038:   
    case 0x0039:   
    case 0x003a:   
    case 0x003b:   
    case 0x003c:   
        nDataSize=ReadAndDrawSameRect(shapeDMethod);
        break;

    case 0x003d:   
    case 0x003e:   
    case 0x003f:   
        nDataSize=0;
        break;

    case 0x0040:   
    case 0x0041:   
    case 0x0042:   
    case 0x0043:   
    case 0x0044:   
        nDataSize=ReadAndDrawRoundRect(shapeDMethod);
        break;

    case 0x0045:   
    case 0x0046:   
    case 0x0047:   
        nDataSize=8;
        break;

    case 0x0048:   
    case 0x0049:   
    case 0x004a:   
    case 0x004b:   
    case 0x004c:   
        nDataSize=ReadAndDrawSameRoundRect(shapeDMethod);
        break;

    case 0x004d:   
    case 0x004e:   
    case 0x004f:   
        nDataSize=0;
        break;

    case 0x0050:   
    case 0x0051:   
    case 0x0052:   
    case 0x0053:   
    case 0x0054:   
        nDataSize=ReadAndDrawOval(shapeDMethod);
        break;

    case 0x0055:   
    case 0x0056:   
    case 0x0057:   
        nDataSize=8;
        break;

    case 0x0058:   
    case 0x0059:   
    case 0x005a:   
    case 0x005b:   
    case 0x005c:   
        nDataSize=ReadAndDrawSameOval(shapeDMethod);
        break;

    case 0x005d:   
    case 0x005e:   
    case 0x005f:   
        nDataSize=0;
        break;

    case 0x0060:   
    case 0x0061:   
    case 0x0062:   
    case 0x0063:   
    case 0x0064:   
        nDataSize=ReadAndDrawArc(shapeDMethod);
        break;

    case 0x0065:   
    case 0x0066:   
    case 0x0067:   
        nDataSize=12;
        break;

    case 0x0068:   
    case 0x0069:   
    case 0x006a:   
    case 0x006b:   
    case 0x006c:   
        nDataSize=ReadAndDrawSameArc(shapeDMethod);
        break;

    case 0x006d:   
    case 0x006e:   
    case 0x006f:   
        nDataSize=4;
        break;

    case 0x0070:   
    case 0x0071:   
    case 0x0072:   
    case 0x0073:   
    case 0x0074:   
        nDataSize=ReadAndDrawPolygon(shapeDMethod);
        break;

    case 0x0075:   
    case 0x0076:   
    case 0x0077:   
        pPict->ReadUInt16( nUSHORT ); nDataSize=nUSHORT;
        break;

    case 0x0078:   
    case 0x0079:   
    case 0x007a:   
    case 0x007b:   
    case 0x007c:   
        nDataSize=ReadAndDrawSamePolygon(shapeDMethod);
        break;

    case 0x007d:   
    case 0x007e:   
    case 0x007f:   
        nDataSize=0;
        break;

    case 0x0080:   
    case 0x0081:   
    case 0x0082:   
    case 0x0083:   
    case 0x0084:   
        nDataSize=ReadAndDrawRgn(shapeDMethod);
        break;

    case 0x0085:   
    case 0x0086:   
    case 0x0087:   
        pPict->ReadUInt16( nUSHORT ); nDataSize=nUSHORT;
        break;

    case 0x0088:   
    case 0x0089:   
    case 0x008a:   
    case 0x008b:   
    case 0x008c:   
        nDataSize=ReadAndDrawSameRgn(shapeDMethod);
        break;

    case 0x008d:   
    case 0x008e:   
    case 0x008f:   
        nDataSize=0;
        break;

    case 0x0090: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_False, sal_True, &aSrcRect, &aDestRect, sal_True, sal_False);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0091: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_False, sal_True, &aSrcRect, &aDestRect, sal_True, sal_True);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0092:   
    case 0x0093:   
    case 0x0094:   
    case 0x0095:   
    case 0x0096:   
    case 0x0097:   
        pPict->ReadUInt16( nUSHORT ); nDataSize=2+nUSHORT;
        break;

    case 0x0098: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_False, sal_True, &aSrcRect, &aDestRect, sal_True, sal_False);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x0099: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_False, sal_True, &aSrcRect, &aDestRect, sal_True, sal_True);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009a: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_True, sal_False, &aSrcRect, &aDestRect, sal_True, sal_False);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009b: { 
        Bitmap aBmp;
        Rectangle aSrcRect, aDestRect;
        nDataSize=ReadPixMapEtc(aBmp, sal_True, sal_False, &aSrcRect, &aDestRect, sal_True, sal_True);
        DrawingMethod(PDM_PAINT);
        pVirDev->DrawBitmap(aDestRect.TopLeft(),aDestRect.GetSize(),aBmp);
        break;
    }
    case 0x009c:   
    case 0x009d:   
    case 0x009e:   
    case 0x009f:   
        pPict->ReadUInt16( nUSHORT ); nDataSize=2+nUSHORT;
        break;

    case 0x00a0:   
        nDataSize=2;
        break;

    case 0x00a1:   
        pPict->SeekRel(2); pPict->ReadUInt16( nUSHORT ); nDataSize=4+nUSHORT;
        break;

    default: 
        if      (nOpcode<=0x00af) { pPict->ReadUInt16( nUSHORT ); nDataSize=2+nUSHORT; }
        else if (nOpcode<=0x00cf) { nDataSize=0; }
        else if (nOpcode<=0x00fe) { sal_uInt32 nTemp; pPict->ReadUInt32( nTemp ) ; nDataSize = nTemp; nDataSize+=4; }
        
        
        else if (nOpcode==0x00ff) { nDataSize=IsVersion2 ? 2 : 0; } 
        else if (nOpcode<=0x01ff) { nDataSize=2; }
        else if (nOpcode<=0x0bfe) { nDataSize=4; }
        else if (nOpcode<=0x0bff) { nDataSize=22; }
        else if (nOpcode==0x0c00) { nDataSize=24; } 
        else if (nOpcode<=0x7eff) { nDataSize=24; }
        else if (nOpcode<=0x7fff) { nDataSize=254; }
        else if (nOpcode<=0x80ff) { nDataSize=0; }
        else                      { sal_uInt32 nTemp; pPict->ReadUInt32( nTemp ) ; nDataSize = nTemp; nDataSize+=4; }
    }

    if (nDataSize==0xffffffff) {
        pPict->SetError(SVSTREAM_FILEFORMAT_ERROR);
        return 0;
    }
    return nDataSize;
}

void PictReader::ReadPict( SvStream & rStreamPict, GDIMetaFile & rGDIMetaFile )
{
    sal_uInt16          nOpcode;
    sal_uInt8           nOneByteOpcode;
    sal_uLong           nSize, nPos, nStartPos, nEndPos, nPercent, nLastPercent;

    pPict               = &rStreamPict;
    nOrigPos            = pPict->Tell();
    nOrigNumberFormat   = pPict->GetNumberFormatInt();

    aActForeColor       = Color(COL_BLACK);
    aActBackColor       = Color(COL_WHITE);
    nActPenSize         = Size(1,1);
    eActROP             = ROP_OVERPAINT;
    eActMethod          = PDM_UNDEFINED;
    aActOvalSize        = Size(1,1);

    aActFont.SetCharSet( GetTextEncoding());
    aActFont.SetFamily(FAMILY_SWISS);
    aActFont.SetSize(Size(0,12));
    aActFont.SetAlign(ALIGN_BASELINE);

    aHRes = aVRes = Fraction( 1, 1 );

    pVirDev = new VirtualDevice();
    pVirDev->EnableOutput(false);
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
            if (Callback((sal_uInt16)nPercent)==sal_True) break;
            nLastPercent=nPercent;
        }

        if (IsVersion2 )
            pPict->ReadUInt16( nOpcode );
        else
        {
            pPict->ReadUChar( nOneByteOpcode );
            nOpcode=(sal_uInt16)nOneByteOpcode;
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

    rGDIMetaFile.SetPrefMapMode( MapMode( MAP_INCH, Point(), aHRes, aVRes ) );
    rGDIMetaFile.SetPrefSize( aBoundingRect.GetSize() );

    pPict->SetNumberFormatInt(nOrigNumberFormat);

    if (pPict->GetError()) pPict->Seek(nOrigPos);
}






#if defined(DISABLE_DYNLOADING)
#define GraphicImport iptGraphicImport
#endif

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
GraphicImport( SvStream& rIStm, Graphic & rGraphic, FilterConfigItem*, sal_Bool)
{
    GDIMetaFile aMTF;
    PictReader  aPictReader;
    sal_Bool        bRet = sal_False;

    aPictReader.ReadPict( rIStm, aMTF );

    if ( !rIStm.GetError() )
    {
        rGraphic = Graphic( aMTF );
        bRet = sal_True;
    }

    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
