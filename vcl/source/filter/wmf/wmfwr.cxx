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

#include "wmfwr.hxx"
#include <unotools/fontcvt.hxx>
#include "emfwr.hxx"
#include <rtl/crc.h>
#include <rtl/tencinfo.h>
#include <tools/bigint.hxx>
#include <tools/helpers.hxx>
#include <tools/tenccvt.hxx>
#include <osl/endian.h>
#include <vcl/dibtools.hxx>
#include <vcl/metric.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//====================== MS-Windows-defines ===============================

#define W_META_SETBKMODE            0x0102
#define W_META_SETROP2              0x0104
#define W_META_SETSTRETCHBLTMODE    0x0107
#define W_META_SETTEXTCOLOR         0x0209
#define W_META_SETWINDOWORG         0x020B
#define W_META_SETWINDOWEXT         0x020C
#define W_META_LINETO               0x0213
#define W_META_MOVETO               0x0214
#define W_META_INTERSECTCLIPRECT    0x0416
#define W_META_ARC                  0x0817
#define W_META_ELLIPSE              0x0418
#define W_META_PIE                  0x081A
#define W_META_RECTANGLE            0x041B
#define W_META_ROUNDRECT            0x061C
#define W_META_SAVEDC               0x001E
#define W_META_SETPIXEL             0x041F
#define W_META_TEXTOUT              0x0521
#define W_META_POLYGON              0x0324
#define W_META_POLYLINE             0x0325
#define W_META_ESCAPE               0x0626
#define W_META_RESTOREDC            0x0127
#define W_META_SELECTOBJECT         0x012D
#define W_META_SETTEXTALIGN         0x012E
#define W_META_CHORD                0x0830
#define W_META_EXTTEXTOUT           0x0a32
#define W_META_POLYPOLYGON          0x0538
#define W_META_STRETCHDIB           0x0f43
#define W_META_DELETEOBJECT         0x01f0
#define W_META_CREATEPENINDIRECT    0x02FA
#define W_META_CREATEFONTINDIRECT   0x02FB
#define W_META_CREATEBRUSHINDIRECT  0x02FC

#define W_TRANSPARENT     1
#define W_OPAQUE          2

#define W_R2_NOT              6
#define W_R2_XORPEN           7
#define W_R2_COPYPEN          13

#define W_TA_NOUPDATECP      0x0000
#define W_TA_LEFT            0x0000
#define W_TA_RIGHT           0x0002
#define W_TA_TOP             0x0000
#define W_TA_BOTTOM          0x0008
#define W_TA_BASELINE        0x0018
#define W_TA_RTLREADING      0x0100

#define W_SRCCOPY             0x00CC0020L
#define W_SRCPAINT            0x00EE0086L
#define W_SRCAND              0x008800C6L
#define W_SRCINVERT           0x00660046L
#define W_DSTINVERT           0x00550009L

#define W_PS_SOLID            0
#define W_PS_DASH             1
#define W_PS_DOT              2
#define W_PS_DASHDOT          3
#define W_PS_DASHDOTDOT       4
#define W_PS_NULL             5

#define W_LF_FACESIZE       32

#define W_ANSI_CHARSET          0

#define W_DEFAULT_PITCH       0x00
#define W_FIXED_PITCH         0x01
#define W_VARIABLE_PITCH      0x02

#define W_FF_DONTCARE         0x00
#define W_FF_ROMAN            0x10
#define W_FF_SWISS            0x20
#define W_FF_MODERN           0x30
#define W_FF_SCRIPT           0x40
#define W_FF_DECORATIVE       0x50

#define W_FW_DONTCARE       0
#define W_FW_THIN           100
#define W_FW_LIGHT          300
#define W_FW_NORMAL         400
#define W_FW_MEDIUM         500
#define W_FW_SEMIBOLD       600
#define W_FW_BOLD           700
#define W_FW_ULTRALIGHT     200
#define W_FW_ULTRABOLD      800
#define W_FW_BLACK          900

#define W_BS_SOLID          0
#define W_BS_HOLLOW         1

#define W_MFCOMMENT         15

#define PRIVATE_ESCAPE_UNICODE          2

//========================== Methoden von WMFWriter ==========================

void WMFWriter::MayCallback()
{
    if ( xStatusIndicator.is() )
    {
        sal_uLong nPercent;

        // Wir gehen mal einfach so davon aus, dass 16386 Actions einer Bitmap entsprechen
        // (in der Regel wird ein Metafile entweder nur Actions oder einige Bitmaps und fast
        // keine Actions enthalten. Dann ist das Verhaeltnis ziemlich unwichtig)

        nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
                *100
                /((nNumberOfBitmaps<<14)+nNumberOfActions);

        if ( nPercent >= nLastPercent + 3 )
        {
            nLastPercent = nPercent;
            if( nPercent <= 100 )
                xStatusIndicator->setValue( nPercent );
        }
    }
}

void WMFWriter::CountActionsAndBitmaps( const GDIMetaFile & rMTF )
{
    size_t nAction, nActionCount;

    nActionCount = rMTF.GetActionSize();

    for ( nAction=0; nAction < nActionCount; nAction++ )
    {
        MetaAction* pMA = rMTF.GetAction( nAction );

        switch( pMA->GetType() )
        {
            case META_BMP_ACTION:
            case META_BMPSCALE_ACTION:
            case META_BMPSCALEPART_ACTION:
            case META_BMPEX_ACTION:
            case META_BMPEXSCALE_ACTION:
            case META_BMPEXSCALEPART_ACTION:
                nNumberOfBitmaps++;
            break;
        }
        nNumberOfActions++;
    }
}


void WMFWriter::WritePointXY(const Point & rPoint)
{
    Point aPt( pVirDev->LogicToLogic(rPoint,aSrcMapMode,aTargetMapMode) );
    *pWMF << ((short)aPt.X()) << ((short)aPt.Y());
}


void WMFWriter::WritePointYX(const Point & rPoint)
{
    Point aPt( pVirDev->LogicToLogic(rPoint,aSrcMapMode,aTargetMapMode) );
    *pWMF << ((short)aPt.Y()) << ((short)aPt.X());
}


sal_Int32 WMFWriter::ScaleWidth( sal_Int32 nDX )
{
    Size aSz( pVirDev->LogicToLogic(Size(nDX,0),aSrcMapMode,aTargetMapMode) );
    return aSz.Width();
}


void WMFWriter::WriteSize(const Size & rSize)
{
    Size aSz( pVirDev->LogicToLogic(rSize,aSrcMapMode,aTargetMapMode) );
    *pWMF << ((short)aSz.Width()) << ((short)aSz.Height());
}


void WMFWriter::WriteHeightWidth(const Size & rSize)
{
    Size aSz( pVirDev->LogicToLogic(rSize,aSrcMapMode,aTargetMapMode) );
    *pWMF << ((short)aSz.Height()) << ((short)aSz.Width());
}


void WMFWriter::WriteRectangle(const Rectangle & rRect)
{
    WritePointYX(Point(rRect.Right()+1,rRect.Bottom()+1));
    WritePointYX(rRect.TopLeft());
}


void WMFWriter::WriteColor(const Color & rColor)
{
    *pWMF << (sal_uInt8) rColor.GetRed() << (sal_uInt8) rColor.GetGreen() << (sal_uInt8) rColor.GetBlue() << (sal_uInt8) 0;
}


void WMFWriter::WriteRecordHeader(sal_uInt32 nSizeWords, sal_uInt16 nType)
{
    nActRecordPos=pWMF->Tell();
    if (nSizeWords>nMaxRecordSize) nMaxRecordSize=nSizeWords;
    *pWMF << nSizeWords << nType;
}


void WMFWriter::UpdateRecordHeader()
{
    sal_uLong nPos;
    sal_uInt32 nSize;

    nPos=pWMF->Tell(); nSize=nPos-nActRecordPos;
    if ((nSize & 1)!=0) {
        *pWMF << (sal_uInt8)0;
        nPos++; nSize++;
    }
    nSize/=2;
    if (nSize>nMaxRecordSize) nMaxRecordSize=nSize;
    pWMF->Seek(nActRecordPos);
    *pWMF << nSize;
    pWMF->Seek(nPos);
}


void WMFWriter::WMFRecord_Arc(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_ARC);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}

void WMFWriter::WMFRecord_Chord(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_CHORD);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}


void WMFWriter::WMFRecord_CreateBrushIndirect(const Color& rColor)
{
    WriteRecordHeader(0x00000007,W_META_CREATEBRUSHINDIRECT);

    if( rColor==Color(COL_TRANSPARENT) )
        *pWMF << (sal_uInt16) W_BS_HOLLOW;
    else
        *pWMF << (sal_uInt16) W_BS_SOLID;

    WriteColor( rColor );
    *pWMF << (sal_uInt16) 0;
}


void WMFWriter::WMFRecord_CreateFontIndirect(const Font & rFont)
{
    sal_uInt16 nWeight,i;
    sal_uInt8 nPitchFamily;

    WriteRecordHeader(0x00000000,W_META_CREATEFONTINDIRECT);
    WriteHeightWidth(Size(rFont.GetSize().Width(),-rFont.GetSize().Height()));
    *pWMF << (short)rFont.GetOrientation() << (short)rFont.GetOrientation();

    switch (rFont.GetWeight()) {
        case WEIGHT_THIN:       nWeight=W_FW_THIN;       break;
        case WEIGHT_ULTRALIGHT: nWeight=W_FW_ULTRALIGHT; break;
        case WEIGHT_LIGHT:      nWeight=W_FW_LIGHT;      break;
        case WEIGHT_SEMILIGHT:  nWeight=W_FW_LIGHT;      break;
        case WEIGHT_NORMAL:     nWeight=W_FW_NORMAL;     break;
        case WEIGHT_MEDIUM:     nWeight=W_FW_MEDIUM;     break;
        case WEIGHT_SEMIBOLD:   nWeight=W_FW_SEMIBOLD;   break;
        case WEIGHT_BOLD:       nWeight=W_FW_BOLD;       break;
        case WEIGHT_ULTRABOLD:  nWeight=W_FW_ULTRABOLD;  break;
        case WEIGHT_BLACK:      nWeight=W_FW_BLACK;      break;
        default:                nWeight=W_FW_DONTCARE;
    }
    *pWMF << nWeight;

    if (rFont.GetItalic()==ITALIC_NONE)       *pWMF << (sal_uInt8)0; else  *pWMF << (sal_uInt8)1;
    if (rFont.GetUnderline()==UNDERLINE_NONE) *pWMF << (sal_uInt8)0; else  *pWMF << (sal_uInt8)1;
    if (rFont.GetStrikeout()==STRIKEOUT_NONE) *pWMF << (sal_uInt8)0; else  *pWMF << (sal_uInt8)1;

    CharSet     eFontNameEncoding = rFont.GetCharSet();
    sal_uInt8   nCharSet = rtl_getBestWindowsCharsetFromTextEncoding( eFontNameEncoding );
    if ( eFontNameEncoding == RTL_TEXTENCODING_SYMBOL )
        eFontNameEncoding = RTL_TEXTENCODING_MS_1252;
    if ( nCharSet == 1 )
        nCharSet = W_ANSI_CHARSET;
    *pWMF << nCharSet;

    *pWMF << (sal_uInt8)0 << (sal_uInt8)0 << (sal_uInt8)0;

    switch (rFont.GetPitch()) {
        case PITCH_FIXED:    nPitchFamily=W_FIXED_PITCH;    break;
        case PITCH_VARIABLE: nPitchFamily=W_VARIABLE_PITCH; break;
        default:             nPitchFamily=W_DEFAULT_PITCH;
    }
    switch (rFont.GetFamily()) {
        case FAMILY_DECORATIVE: nPitchFamily|=W_FF_DECORATIVE; break;
        case FAMILY_MODERN:     nPitchFamily|=W_FF_MODERN;     break;
        case FAMILY_ROMAN:      nPitchFamily|=W_FF_ROMAN;      break;
        case FAMILY_SCRIPT:     nPitchFamily|=W_FF_SCRIPT;     break;
        case FAMILY_SWISS:      nPitchFamily|=W_FF_SWISS;      break;
        default:                nPitchFamily|=W_FF_DONTCARE;
    }
    *pWMF << nPitchFamily;

    OString aFontName(OUStringToOString(rFont.GetName(), eFontNameEncoding));
    for ( i = 0; i < W_LF_FACESIZE; i++ )
    {
        sal_Char nChar = ( i < aFontName.getLength() ) ? aFontName[i] : 0;
        *pWMF << nChar;
    }
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_CreatePenIndirect(const Color& rColor, const LineInfo& rLineInfo )
{
    WriteRecordHeader(0x00000008,W_META_CREATEPENINDIRECT);
    sal_uInt16 nStyle = rColor == Color( COL_TRANSPARENT ) ? W_PS_NULL : W_PS_SOLID;
    switch( rLineInfo.GetStyle() )
    {
        case LINE_DASH :
        {
            if ( rLineInfo.GetDotCount() )
            {
                if ( !rLineInfo.GetDashCount() )
                    nStyle = W_PS_DOT;
                else
                {
                    if ( rLineInfo.GetDotCount() == 1 )
                        nStyle = W_PS_DASHDOT;
                    else
                        nStyle = W_PS_DASHDOTDOT;
                }
            }
            else
                nStyle = W_PS_DASH;
        }
        break;
        case LINE_NONE :
            nStyle = W_PS_NULL;
        break;
        default:
        break;
    }
    *pWMF << nStyle;

    WriteSize( Size( rLineInfo.GetWidth(), 0 ) );
    WriteColor( rColor );
}

void WMFWriter::WMFRecord_DeleteObject(sal_uInt16 nObjectHandle)
{
    WriteRecordHeader(0x00000004,W_META_DELETEOBJECT);
    *pWMF << nObjectHandle;
}


void WMFWriter::WMFRecord_Ellipse(const Rectangle & rRect)
{
    WriteRecordHeader(0x00000007,W_META_ELLIPSE);
    WriteRectangle(rRect);
}

void WMFWriter::WMFRecord_Escape( sal_uInt32 nEsc, sal_uInt32 nLen, const sal_Int8* pData )
{
#ifdef OSL_BIGENDIAN
    sal_uInt32 nTmp = OSL_SWAPDWORD( nEsc );
    sal_uInt32 nCheckSum = rtl_crc32( 0, &nTmp, 4 );
#else
    sal_uInt32 nCheckSum = rtl_crc32( 0, &nEsc, 4 );
#endif
    if ( nLen )
        nCheckSum = rtl_crc32( nCheckSum, pData, nLen );

    WriteRecordHeader( 3 + 9 + ( ( nLen + 1 ) >> 1 ), W_META_ESCAPE );
    *pWMF << (sal_uInt16)W_MFCOMMENT
          << (sal_uInt16)( nLen + 14 )  // we will always have a fourteen byte escape header:
          << (sal_uInt16)0x4f4f         // OO
          << (sal_uInt32)0xa2c2a        // evil magic number
          << (sal_uInt32)nCheckSum      // crc32 checksum about nEsc & pData
          << (sal_uInt32)nEsc;          // escape number
    pWMF->Write( pData, nLen );
    if ( nLen & 1 )
        *pWMF << (sal_uInt8)0;          // pad byte
}

/* if return value is true, then a complete unicode string and also a polygon replacement has been written,
    so there is no more action necessary
*/
bool WMFWriter::WMFRecord_Escape_Unicode( const Point& rPoint, const String& rUniStr, const sal_Int32* pDXAry )
{
    bool bEscapeUsed = false;

    sal_uInt32 i, nStringLen = rUniStr.Len();
    if ( nStringLen )
    {
        // first we will check if a comment is necessary
        if ( aSrcFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )     // symbol is always byte character, so there is no unicode loss
        {
            const sal_Unicode* pBuf = rUniStr.GetBuffer();
            const rtl_TextEncoding aTextEncodingOrg = aSrcFont.GetCharSet();
            OString aByteStr(OUStringToOString(rUniStr, aTextEncodingOrg));
            OUString aUniStr2(OStringToOUString(aByteStr, aTextEncodingOrg));
            const sal_Unicode* pConversion = aUniStr2.getStr();  // this is the unicode array after bytestring <-> unistring conversion
            for ( i = 0; i < nStringLen; i++ )
            {
                if ( *pBuf++ != *pConversion++ )
                    break;
            }

            if  ( i != nStringLen )                             // after conversion the characters are not original,
            {                                                   // try again, with determining a better charset from unicode char
                pBuf = rUniStr.GetBuffer();
                const sal_Unicode* pCheckChar = pBuf;
                rtl_TextEncoding aTextEncoding = getBestMSEncodingByChar(*pCheckChar); // try the first character
                for ( i = 1; i < nStringLen; i++)
                {
                    if (aTextEncoding != aTextEncodingOrg) // found something
                        break;
                    pCheckChar++;
                    aTextEncoding = getBestMSEncodingByChar(*pCheckChar); // try the next character
                }

                aByteStr = OUStringToOString(rUniStr,  aTextEncoding);
                aUniStr2 = OStringToOUString(aByteStr, aTextEncoding);
                pConversion = aUniStr2.getStr(); // this is the unicode array after bytestring <-> unistring conversion
                for ( i = 0; i < nStringLen; i++ )
                {
                    if ( *pBuf++ != *pConversion++ )
                        break;
                }
                if (i == nStringLen)
                {
                    aSrcFont.SetCharSet (aTextEncoding);
                    SetAllAttr();
                }
            }

            if ( ( i != nStringLen ) || IsStarSymbol( aSrcFont.GetName() ) )    // after conversion the characters are not original, so we
            {                                                                   // will store the unicode string and a polypoly replacement
                Color aOldFillColor( aSrcFillColor );
                Color aOldLineColor( aSrcLineColor );
                aSrcLineInfo  = LineInfo();
                aSrcFillColor = aSrcTextColor;
                aSrcLineColor = Color( COL_TRANSPARENT );
                SetLineAndFillAttr();
                pVirDev->SetFont( aSrcFont );
                std::vector<PolyPolygon> aPolyPolyVec;
                if ( pVirDev->GetTextOutlines( aPolyPolyVec, rUniStr ) )
                {
                    sal_uInt32 nDXCount = pDXAry ? nStringLen : 0;
                    sal_uInt32 nSkipActions = aPolyPolyVec.size();
                    sal_Int32 nStrmLen = 8 +
                                           + sizeof( nStringLen ) + ( nStringLen * 2 )
                                           + sizeof( nDXCount ) + ( nDXCount * 4 )
                                           + sizeof( nSkipActions );

                    SvMemoryStream aMemoryStream( nStrmLen );
                    Point aPt( pVirDev->LogicToLogic( rPoint, aSrcMapMode, aTargetMapMode ) );
                    aMemoryStream << static_cast<sal_Int32>(aPt.X())
                                  << static_cast<sal_Int32>(aPt.Y())
                                  << nStringLen;
                    for ( i = 0; i < nStringLen; i++ )
                        aMemoryStream << rUniStr.GetChar( (sal_uInt16)i );
                    aMemoryStream << nDXCount;
                    for ( i = 0; i < nDXCount; i++ )
                        aMemoryStream << pDXAry[ i ];
                    aMemoryStream << nSkipActions;
                    WMFRecord_Escape( PRIVATE_ESCAPE_UNICODE, nStrmLen, (const sal_Int8*)aMemoryStream.GetData() );

                    std::vector<PolyPolygon>::iterator aIter( aPolyPolyVec.begin() );
                    while ( aIter != aPolyPolyVec.end() )
                    {
                        PolyPolygon aPolyPoly( *aIter++ );
                        aPolyPoly.Move( rPoint.X(), rPoint.Y() );
                        WMFRecord_PolyPolygon( aPolyPoly );
                    }
                    aSrcFillColor = aOldFillColor;
                    aSrcLineColor = aOldLineColor;
                    bEscapeUsed = true;
                }
            }
        }
    }
    return bEscapeUsed;
}

void WMFWriter::WMFRecord_ExtTextOut( const Point & rPoint,
    const String & rString, const sal_Int32 * pDXAry )
{
    sal_uInt16 nOriginalTextLen = rString.Len();

    if ( (nOriginalTextLen <= 1) || (pDXAry == NULL) )
    {
        WMFRecord_TextOut(rPoint, rString);
        return;
    }
    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
    OString aByteString(OUStringToOString(rString, eChrSet));
    TrueExtTextOut(rPoint,rString,aByteString,pDXAry);
}

void WMFWriter::TrueExtTextOut( const Point & rPoint, const String & rString,
    const OString& rByteString, const sal_Int32 * pDXAry )
{
    WriteRecordHeader( 0, W_META_EXTTEXTOUT );
    WritePointYX( rPoint );
    sal_uInt16 nNewTextLen = static_cast<sal_uInt16>(rByteString.getLength());
    *pWMF << nNewTextLen << (sal_uInt16)0;
    write_uInt8s_FromOString(*pWMF, rByteString, nNewTextLen);
    if ( nNewTextLen & 1 )
        *pWMF << (sal_uInt8)0;

    sal_uInt16 nOriginalTextLen = rString.Len();
    sal_Int16* pConvertedDXAry = new sal_Int16[ nOriginalTextLen ];
    sal_Int32 j = 0;
    pConvertedDXAry[ j++ ] = (sal_Int16)ScaleWidth( pDXAry[ 0 ] );
    for (sal_uInt16 i = 1; i < ( nOriginalTextLen - 1 ); ++i)
        pConvertedDXAry[ j++ ] = (sal_Int16)ScaleWidth( pDXAry[ i ] - pDXAry[ i - 1 ] );
    pConvertedDXAry[ j ] = (sal_Int16)ScaleWidth( pDXAry[ nOriginalTextLen - 2 ] / ( nOriginalTextLen - 1 ) );

    for (sal_uInt16 i = 0; i < nOriginalTextLen; ++i)
    {
        sal_Int16 nDx = pConvertedDXAry[ i ];
        *pWMF << nDx;
        if ( nOriginalTextLen < nNewTextLen )
        {
            sal_Unicode nUniChar = rString.GetChar(i);
            OString aTemp(&nUniChar, 1, aSrcFont.GetCharSet());
            j = aTemp.getLength();
            while ( --j > 0 )
                *pWMF << (sal_uInt16)0;
        }
    }
    delete[] pConvertedDXAry;
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_LineTo(const Point & rPoint)
{
    WriteRecordHeader(0x00000005,W_META_LINETO);
    WritePointYX(rPoint);
}


void WMFWriter::WMFRecord_MoveTo(const Point & rPoint)
{
    WriteRecordHeader(0x00000005,W_META_MOVETO);
    WritePointYX(rPoint);
}


void WMFWriter::WMFRecord_Pie(const Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_PIE);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}


void WMFWriter::WMFRecord_Polygon(const Polygon & rPoly)
{
    sal_uInt16 nSize,i;

    Polygon aSimplePoly;
    if ( rPoly.HasFlags() )
        rPoly.AdaptiveSubdivide( aSimplePoly );
    else
        aSimplePoly = rPoly;
    nSize = aSimplePoly.GetSize();
    WriteRecordHeader(((sal_uLong)nSize)*2+4,W_META_POLYGON);
    *pWMF << nSize;
    for (i=0; i<nSize; i++) WritePointXY(aSimplePoly.GetPoint(i));
}


void WMFWriter::WMFRecord_PolyLine(const Polygon & rPoly)
{
    sal_uInt16 nSize,i;
    Polygon aSimplePoly;
    if ( rPoly.HasFlags() )
        rPoly.AdaptiveSubdivide( aSimplePoly );
    else
        aSimplePoly = rPoly;
    nSize=aSimplePoly.GetSize();
    WriteRecordHeader(((sal_uLong)nSize)*2+4,W_META_POLYLINE);
    *pWMF << nSize;
    for (i=0; i<nSize; i++) WritePointXY(aSimplePoly.GetPoint(i));
}


void WMFWriter::WMFRecord_PolyPolygon(const PolyPolygon & rPolyPoly)
{
    const Polygon * pPoly;
    sal_uInt16 nCount,nSize,i,j;

    nCount=rPolyPoly.Count();
    PolyPolygon aSimplePolyPoly( rPolyPoly );
    for ( i = 0; i < nCount; i++ )
    {
        if ( aSimplePolyPoly[ i ].HasFlags() )
        {
            Polygon aSimplePoly;
            aSimplePolyPoly[ i ].AdaptiveSubdivide( aSimplePoly );
            aSimplePolyPoly[ i ] = aSimplePoly;
        }
    }
    WriteRecordHeader(0,W_META_POLYPOLYGON);
    *pWMF << nCount;
    for (i=0; i<nCount; i++) *pWMF << ((sal_uInt16)(aSimplePolyPoly.GetObject(i).GetSize()));
    for (i=0; i<nCount; i++) {
        pPoly=&(aSimplePolyPoly.GetObject(i));
        nSize=pPoly->GetSize();
        for (j=0; j<nSize; j++) WritePointXY(pPoly->GetPoint(j));
    }
    UpdateRecordHeader();
}


void WMFWriter::WMFRecord_Rectangle(const Rectangle & rRect)
{
    WriteRecordHeader( 0x00000007,W_META_RECTANGLE );
    WriteRectangle( rRect );
}


void WMFWriter::WMFRecord_RestoreDC()
{
    WriteRecordHeader(0x00000004,W_META_RESTOREDC);
    *pWMF << (short)-1;
}


void WMFWriter::WMFRecord_RoundRect(const Rectangle & rRect, long nHorzRound, long nVertRound)
{
    WriteRecordHeader(0x00000009,W_META_ROUNDRECT);
    WriteHeightWidth(Size(nHorzRound,nVertRound));
    WriteRectangle(rRect);
}


void WMFWriter::WMFRecord_SaveDC()
{
    WriteRecordHeader(0x00000003,W_META_SAVEDC);
}


void WMFWriter::WMFRecord_SelectObject(sal_uInt16 nObjectHandle)
{
    WriteRecordHeader(0x00000004,W_META_SELECTOBJECT);
    *pWMF << nObjectHandle;
}


void WMFWriter::WMFRecord_SetBkMode(bool bTransparent)
{
    WriteRecordHeader(0x00000004,W_META_SETBKMODE);
    if (bTransparent) *pWMF << (sal_uInt16)W_TRANSPARENT;
    else                    *pWMF << (sal_uInt16)W_OPAQUE;
}

void WMFWriter::WMFRecord_SetStretchBltMode()
{
    WriteRecordHeader( 0x00000004, W_META_SETSTRETCHBLTMODE );
    *pWMF << (sal_uInt16) 3; // STRETCH_DELETESCANS
}

void WMFWriter::WMFRecord_SetPixel(const Point & rPoint, const Color & rColor)
{
    WriteRecordHeader(0x00000007,W_META_SETPIXEL);
    WriteColor(rColor);
    WritePointYX(rPoint);
}


void WMFWriter::WMFRecord_SetROP2(RasterOp eROP)
{
    sal_uInt16 nROP2;

    switch (eROP) {
        case ROP_INVERT: nROP2=W_R2_NOT;        break;
        case ROP_XOR:    nROP2=W_R2_XORPEN;     break;
        default:         nROP2=W_R2_COPYPEN;
    }
    WriteRecordHeader(0x00000004,W_META_SETROP2);
    *pWMF << nROP2;
}


void WMFWriter::WMFRecord_SetTextAlign(FontAlign eFontAlign, sal_uInt32 eHorTextAlign)
{
    sal_uInt16 nAlign;

    switch (eFontAlign) {
        case ALIGN_TOP:    nAlign=W_TA_TOP; break;
        case ALIGN_BOTTOM: nAlign=W_TA_BOTTOM; break;
        default:           nAlign=W_TA_BASELINE;
    }
    nAlign|=eHorTextAlign;
    nAlign|=W_TA_NOUPDATECP;

    WriteRecordHeader(0x00000004,W_META_SETTEXTALIGN);
    *pWMF << nAlign;
}


void WMFWriter::WMFRecord_SetTextColor(const Color & rColor)
{
    WriteRecordHeader(0x00000005,W_META_SETTEXTCOLOR);
    WriteColor(rColor);
}


void WMFWriter::WMFRecord_SetWindowExt(const Size & rSize)
{
    WriteRecordHeader(0x00000005,W_META_SETWINDOWEXT);
    WriteHeightWidth(rSize);
}


void WMFWriter::WMFRecord_SetWindowOrg(const Point & rPoint)
{
    WriteRecordHeader(0x00000005,W_META_SETWINDOWORG);
    WritePointYX(rPoint);
}


void WMFWriter::WMFRecord_StretchDIB( const Point & rPoint, const Size & rSize,
                                      const Bitmap & rBitmap, sal_uInt32 nROP )
{
    sal_uLong nPosAnf,nPosEnd;

    nActBitmapPercent=50;
    MayCallback();

    WriteRecordHeader(0x00000000,W_META_STRETCHDIB);

    // Die Reihenfolge im Metafile soll jetzt sein:
    // einige Parameter (laenge 22), dann die Bitmap ohne FILEHEADER.
    // Da aber *pWMF << rBitmap einen FILEHEADER der Laenge 14
    // erzeugt, schreiben wir zuerst die Bitmap an die richtige Position
    // Und ueberschreiben hinterher den FILEHEADER mit den Parametern.
    nPosAnf=pWMF->Tell(); // Position merken, wo Parameter hin sollen
    *pWMF << (sal_Int32)0 << (sal_Int32)0; // 8 bytes auffuellen (diese 8 bytes +
                                           // 14 bytes ueberfluessigen FILEHEADER
                                           // = 22 bytes Parameter)

    // write bitmap
    WriteDIB(rBitmap, *pWMF, false, true);


    // Parameter schreiben:
    nPosEnd=pWMF->Tell();
    pWMF->Seek(nPosAnf);

    // Raster-Op bestimmen, falls nichts uebergeben wurde
    if( !nROP )
    {
        switch( eSrcRasterOp )
        {
            case ROP_INVERT: nROP = W_DSTINVERT; break;
            case ROP_XOR:    nROP = W_SRCINVERT; break;
            default:         nROP = W_SRCCOPY;
        }
    }

    *pWMF << nROP <<
             (short) 0 <<
             (short) rBitmap.GetSizePixel().Height() <<
             (short) rBitmap.GetSizePixel().Width() <<
             (short) 0 <<
             (short) 0;

    WriteHeightWidth(rSize);
    WritePointYX(rPoint);
    pWMF->Seek(nPosEnd);

    UpdateRecordHeader();

    nWrittenBitmaps++;
    nActBitmapPercent=0;
}


void WMFWriter::WMFRecord_TextOut(const Point & rPoint, const String & rStr)
{
    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
    OString aString(OUStringToOString(rStr, eChrSet));
    TrueTextOut(rPoint, aString);
}

void WMFWriter::TrueTextOut(const Point & rPoint, const OString& rString)
{
    WriteRecordHeader(0,W_META_TEXTOUT);

    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(*pWMF, rString);
    sal_Int32 nLen = rString.getLength();
    if ((nLen&1)!=0) *pWMF << (sal_uInt8)0;
    WritePointYX(rPoint);
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_EndOfFile()
{
    WriteRecordHeader(0x00000003,0x0000);
}


void WMFWriter::WMFRecord_IntersectClipRect( const Rectangle& rRect )
{
    WriteRecordHeader( 0x00000007, W_META_INTERSECTCLIPRECT );
    WriteRectangle(rRect);
}


sal_uInt16 WMFWriter::AllocHandle()
{
    sal_uInt16 i;

    for (i=0; i<MAXOBJECTHANDLES; i++) {
        if (!bHandleAllocated[i]) {
            bHandleAllocated[i]=true;
            return i;
        }
    }
    bStatus=false;
    return 0xffff;
}


void WMFWriter::FreeHandle(sal_uInt16 nObjectHandle)
{
    if (nObjectHandle<MAXOBJECTHANDLES) bHandleAllocated[nObjectHandle]=false;
}


void WMFWriter::CreateSelectDeletePen( const Color& rColor, const LineInfo& rLineInfo )
{
    sal_uInt16 nOldHandle;

    nOldHandle=nDstPenHandle;
    nDstPenHandle=AllocHandle();
    WMFRecord_CreatePenIndirect( rColor, rLineInfo );
    WMFRecord_SelectObject(nDstPenHandle);
    if (nOldHandle<MAXOBJECTHANDLES) {
        WMFRecord_DeleteObject(nOldHandle);
        FreeHandle(nOldHandle);
    }
}


void WMFWriter::CreateSelectDeleteFont(const Font & rFont)
{
    sal_uInt16 nOldHandle;

    nOldHandle=nDstFontHandle;
    nDstFontHandle=AllocHandle();
    WMFRecord_CreateFontIndirect(rFont);
    WMFRecord_SelectObject(nDstFontHandle);
    if (nOldHandle<MAXOBJECTHANDLES) {
        WMFRecord_DeleteObject(nOldHandle);
        FreeHandle(nOldHandle);
    }
}


void WMFWriter::CreateSelectDeleteBrush(const Color& rColor)
{
    sal_uInt16 nOldHandle;

    nOldHandle=nDstBrushHandle;
    nDstBrushHandle=AllocHandle();
    WMFRecord_CreateBrushIndirect(rColor);
    WMFRecord_SelectObject(nDstBrushHandle);
    if (nOldHandle<MAXOBJECTHANDLES) {
        WMFRecord_DeleteObject(nOldHandle);
        FreeHandle(nOldHandle);
    }
}


void WMFWriter::SetLineAndFillAttr()
{
    if ( eDstROP2 != eSrcRasterOp )
    {
        eDstROP2=eSrcRasterOp;
        WMFRecord_SetROP2(eDstROP2);
    }
    if ( ( aDstLineColor != aSrcLineColor ) || ( aDstLineInfo != aSrcLineInfo ) )
    {
        aDstLineColor = aSrcLineColor;
        aDstLineInfo  = aSrcLineInfo;
        CreateSelectDeletePen( aDstLineColor, aDstLineInfo );
    }
    if ( aDstFillColor != aSrcFillColor )
    {
        aDstFillColor = aSrcFillColor;
        CreateSelectDeleteBrush( aDstFillColor );
    }
    if ( bDstIsClipping != bSrcIsClipping ||
        (bSrcIsClipping && aDstClipRegion!=aSrcClipRegion)) {
        bDstIsClipping=bSrcIsClipping;
        aDstClipRegion=aSrcClipRegion;
    }
}

void WMFWriter::SetAllAttr()
{
    SetLineAndFillAttr();
    if ( aDstTextColor != aSrcTextColor )
    {
        aDstTextColor = aSrcTextColor;
        WMFRecord_SetTextColor(aDstTextColor);
    }
    if ( eDstTextAlign != eSrcTextAlign || eDstHorTextAlign != eSrcHorTextAlign )
    {
        eDstTextAlign = eSrcTextAlign;
        eDstHorTextAlign = eSrcHorTextAlign;
        WMFRecord_SetTextAlign( eDstTextAlign, eDstHorTextAlign );
    }
    if ( aDstFont != aSrcFont )
    {
        pVirDev->SetFont(aSrcFont);
        if ( aDstFont.GetName() != aSrcFont.GetName() )
        {
            FontCharMap aFontCharMap;
            if ( pVirDev->GetFontCharMap( aFontCharMap ) )
            {
                if ( ( aFontCharMap.GetFirstChar() & 0xff00 ) == 0xf000 )
                    aSrcFont.SetCharSet( RTL_TEXTENCODING_SYMBOL );
                else if ( aSrcFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL )
                    aSrcFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );
            }
        }
        aDstFont = aSrcFont;
        CreateSelectDeleteFont(aDstFont);
    }
}


void WMFWriter::HandleLineInfoPolyPolygons(const LineInfo& rInfo, const basegfx::B2DPolygon& rLinePolygon)
{
    if(rLinePolygon.count())
    {
        basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolygon);
        basegfx::B2DPolyPolygon aFillPolyPolygon;

        rInfo.applyToB2DPolyPolygon(aLinePolyPolygon, aFillPolyPolygon);

        if(aLinePolyPolygon.count())
        {
            aSrcLineInfo = rInfo;
            SetLineAndFillAttr();

            for(sal_uInt32 a(0); a < aLinePolyPolygon.count(); a++)
            {
                const basegfx::B2DPolygon aCandidate(aLinePolyPolygon.getB2DPolygon(a));
                WMFRecord_PolyLine(Polygon(aCandidate));
            }
        }

        if(aFillPolyPolygon.count())
        {
            const Color aOldLineColor(aSrcLineColor);
            const Color aOldFillColor(aSrcFillColor);

            aSrcLineColor = Color( COL_TRANSPARENT );
            aSrcFillColor = aOldLineColor;
            SetLineAndFillAttr();

            for(sal_uInt32 a(0); a < aFillPolyPolygon.count(); a++)
            {
                const Polygon aPolygon(aFillPolyPolygon.getB2DPolygon(a));
                WMFRecord_Polygon(Polygon(aPolygon));
            }

            aSrcLineColor = aOldLineColor;
            aSrcFillColor = aOldFillColor;
            SetLineAndFillAttr();
        }
    }
}

void WMFWriter::WriteRecords( const GDIMetaFile & rMTF )
{
    MetaAction* pMA;

    if( bStatus )
    {
        size_t nACount = rMTF.GetActionSize();

        WMFRecord_SetStretchBltMode();

        for( size_t nA = 0; nA < nACount; nA++ )
        {
            pMA = rMTF.GetAction( nA );

            switch( pMA->GetType() )
            {
                case META_PIXEL_ACTION:
                {
                    const MetaPixelAction* pA = (const MetaPixelAction *) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_SetPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case META_POINT_ACTION:
                {
                    const MetaPointAction*  pA = (const MetaPointAction*) pMA;
                    const Point&            rPt = pA->GetPoint();
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_MoveTo( rPt);
                    WMFRecord_LineTo( rPt );
                }
                break;

                case META_LINE_ACTION:
                {
                    const MetaLineAction* pA = (const MetaLineAction *) pMA;
                    if(pA->GetLineInfo().IsDefault())
                    {
                        aSrcLineInfo = pA->GetLineInfo();
                        SetLineAndFillAttr();
                        WMFRecord_MoveTo( pA->GetStartPoint() );
                        WMFRecord_LineTo( pA->GetEndPoint() );
                    }
                    else
                    {
                        // LineInfo used; handle Dash/Dot and fat lines
                        basegfx::B2DPolygon aPolygon;
                        aPolygon.append(basegfx::B2DPoint(pA->GetStartPoint().X(), pA->GetStartPoint().Y()));
                        aPolygon.append(basegfx::B2DPoint(pA->GetEndPoint().X(), pA->GetEndPoint().Y()));
                        HandleLineInfoPolyPolygons(pA->GetLineInfo(), aPolygon);
                    }
                }
                break;

                case META_RECT_ACTION:
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Rectangle( pA->GetRect() );
                }
                break;

                case META_ROUNDRECT_ACTION:
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_RoundRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case META_ELLIPSE_ACTION:
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Ellipse( pA->GetRect() );
                }
                break;

                case META_ARC_ACTION:
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Arc( pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint() );
                }
                break;

                case META_PIE_ACTION:
                {
                    const MetaPieAction* pA = (const MetaPieAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Pie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;


                case META_CHORD_ACTION:
                {
                    const MetaChordAction* pA = (const MetaChordAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Chord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case META_POLYLINE_ACTION:
                {
                    const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pMA;
                    const Polygon&              rPoly = pA->GetPolygon();

                    if( rPoly.GetSize() )
                    {
                        if(pA->GetLineInfo().IsDefault())
                        {
                            aSrcLineInfo = pA->GetLineInfo();
                            SetLineAndFillAttr();
                            WMFRecord_PolyLine( rPoly );
                        }
                        else
                        {
                            // LineInfo used; handle Dash/Dot and fat lines
                            HandleLineInfoPolyPolygons(pA->GetLineInfo(), rPoly.getB2DPolygon());
                        }
                    }
                }
                break;

                case META_POLYGON_ACTION:
                {
                    const MetaPolygonAction* pA = (const MetaPolygonAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Polygon( pA->GetPolygon() );
                }
                break;

                case META_POLYPOLYGON_ACTION:
                {
                    const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_PolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case META_TEXTRECT_ACTION:
                {
                    const MetaTextRectAction * pA = (const MetaTextRectAction*)pMA;
                    String aTemp( pA->GetText() );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();

                    Point aPos( pA->GetRect().TopLeft() );
                    if ( !WMFRecord_Escape_Unicode( aPos, aTemp, NULL ) )
                        WMFRecord_TextOut( aPos, aTemp );
                }
                break;

                case META_TEXT_ACTION:
                {
                    const MetaTextAction * pA = (const MetaTextAction*) pMA;
                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();
                    if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, NULL ) )
                        WMFRecord_TextOut( pA->GetPoint(), aTemp );
                }
                break;

                case META_TEXTARRAY_ACTION:
                {
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pMA;

                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();
                    if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, pA->GetDXArray() ) )
                        WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pA->GetDXArray() );
                }
                break;

                case META_STRETCHTEXT_ACTION:
                {
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction *) pMA;
                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );

                    sal_uInt16 nLen,i;
                    sal_Int32 nNormSize;

                    pVirDev->SetFont( aSrcFont );
                    nLen = aTemp.Len();
                    sal_Int32* pDXAry = nLen ? new sal_Int32[ nLen ] : NULL;
                    nNormSize = pVirDev->GetTextArray( aTemp, pDXAry );
                    for ( i = 0; i < ( nLen - 1 ); i++ )
                        pDXAry[ i ] = pDXAry[ i ] * (sal_Int32)pA->GetWidth() / nNormSize;
                    if ( ( nLen <= 1 ) || ( (sal_Int32)pA->GetWidth() == nNormSize ) )
                        delete[] pDXAry, pDXAry = NULL;
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();
                    if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, pDXAry ) )
                        WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pDXAry );
                    delete[] pDXAry;
                }
                break;

                case META_BMP_ACTION:
                {
                    const MetaBmpAction* pA = (const MetaBmpAction *) pMA;
                    WMFRecord_StretchDIB( pA->GetPoint(), pA->GetBitmap().GetSizePixel(), pA->GetBitmap() );
                }
                break;

                case META_BMPSCALE_ACTION:
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pMA;
                    WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
                }
                break;

                case META_BMPSCALEPART_ACTION:
                {
                    const MetaBmpScalePartAction*   pA = (const MetaBmpScalePartAction*) pMA;
                    Bitmap                          aTmp( pA->GetBitmap() );

                    if( aTmp.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) ) )
                        WMFRecord_StretchDIB( pA->GetDestPoint(), pA->GetDestSize(), aTmp );
                }
                break;

                case META_BMPEX_ACTION:
                {
                    const MetaBmpExAction*  pA = (const MetaBmpExAction *) pMA;
                    Bitmap                  aBmp( pA->GetBitmapEx().GetBitmap() );
                    Bitmap                  aMsk( pA->GetBitmapEx().GetMask() );

                    if( !!aMsk )
                    {
                        aBmp.Replace( aMsk, COL_WHITE );
                        aMsk.Invert();
                        WMFRecord_StretchDIB( pA->GetPoint(), aMsk.GetSizePixel(), aBmp, W_SRCPAINT );
                        WMFRecord_StretchDIB( pA->GetPoint(), aBmp.GetSizePixel(), aBmp, W_SRCAND );
                    }
                    else
                        WMFRecord_StretchDIB( pA->GetPoint(), aBmp.GetSizePixel(), aBmp );
                }
                break;

                case META_BMPEXSCALE_ACTION:
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pMA;
                    Bitmap                      aBmp( pA->GetBitmapEx().GetBitmap() );
                    Bitmap                      aMsk( pA->GetBitmapEx().GetMask() );

                    if( !!aMsk )
                    {
                        aBmp.Replace( aMsk, COL_WHITE );
                        aMsk.Invert();
                        WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), aMsk, W_SRCPAINT );
                        WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), aBmp, W_SRCAND );
                    }
                    else
                        WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), aBmp );
                }
                break;

                case META_BMPEXSCALEPART_ACTION:
                {
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pMA;
                    BitmapEx                        aBmpEx( pA->GetBitmapEx() );
                    aBmpEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    Bitmap                          aBmp( aBmpEx.GetBitmap() );
                    Bitmap                          aMsk( aBmpEx.GetMask() );

                    if( !!aMsk )
                    {
                        aBmp.Replace( aMsk, COL_WHITE );
                        aMsk.Invert();
                        WMFRecord_StretchDIB( pA->GetDestPoint(), pA->GetDestSize(), aMsk, W_SRCPAINT );
                        WMFRecord_StretchDIB( pA->GetDestPoint(), pA->GetDestSize(), aBmp, W_SRCAND );
                    }
                    else
                        WMFRecord_StretchDIB( pA->GetDestPoint(), pA->GetDestSize(), aBmp );
                }
                break;

                case META_GRADIENT_ACTION:
                {
                    const MetaGradientAction*   pA = (const MetaGradientAction*) pMA;
                    GDIMetaFile                 aTmpMtf;

                    pVirDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                    WriteRecords( aTmpMtf );
                }
                break;

                case META_HATCH_ACTION:
                {
                    const MetaHatchAction*  pA = (const MetaHatchAction*) pMA;
                    GDIMetaFile             aTmpMtf;

                    pVirDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                    WriteRecords( aTmpMtf );
                }
                break;

                case META_WALLPAPER_ACTION:
                {
                    const MetaWallpaperAction*  pA = (const MetaWallpaperAction*) pMA;
                    const Color&                rColor = pA->GetWallpaper().GetColor();
                    const Color                 aOldLineColor( aSrcLineColor );
                    const Color                 aOldFillColor( aSrcFillColor );

                    aSrcLineColor = rColor;
                    aSrcFillColor = rColor;
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Rectangle( pA->GetRect() );
                    aSrcLineColor = aOldLineColor;
                    aSrcFillColor = aOldFillColor;
                }
                break;

                case META_ISECTRECTCLIPREGION_ACTION:
                {
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pMA;
                    WMFRecord_IntersectClipRect( pA->GetRect() );
                }
                break;

                case META_LINECOLOR_ACTION:
                {
                    const MetaLineColorAction* pA = (const MetaLineColorAction*) pMA;

                    if( pA->IsSetting() )
                        aSrcLineColor = pA->GetColor();
                    else
                        aSrcLineColor = Color( COL_TRANSPARENT );
                }
                break;

                case META_FILLCOLOR_ACTION:
                {
                    const MetaFillColorAction* pA = (const MetaFillColorAction*) pMA;

                    if( pA->IsSetting() )
                        aSrcFillColor = pA->GetColor();
                    else
                        aSrcFillColor = Color( COL_TRANSPARENT );
                }
                break;

                case META_TEXTCOLOR_ACTION:
                {
                    const MetaTextColorAction* pA = (const MetaTextColorAction*) pMA;
                    aSrcTextColor = pA->GetColor();
                }
                break;

                case META_TEXTFILLCOLOR_ACTION:
                {
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pMA;
                    if( pA->IsSetting() )
                        aSrcFont.SetFillColor( pA->GetColor() );
                    else
                        aSrcFont.SetFillColor( Color( COL_TRANSPARENT ) );
                }
                break;

                case META_TEXTALIGN_ACTION:
                {
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pMA;
                    eSrcTextAlign = pA->GetTextAlign();
                }
                break;

                case META_MAPMODE_ACTION:
                {
                    const MetaMapModeAction* pA = (const MetaMapModeAction*) pMA;

                    if (aSrcMapMode!=pA->GetMapMode())
                    {
                        if( pA->GetMapMode().GetMapUnit() == MAP_RELATIVE )
                        {
                            MapMode aMM = pA->GetMapMode();
                            Fraction aScaleX = aMM.GetScaleX();
                            Fraction aScaleY = aMM.GetScaleY();

                            Point aOrigin = aSrcMapMode.GetOrigin();
                            BigInt aX( aOrigin.X() );
                            aX *= BigInt( aScaleX.GetDenominator() );
                            if( aOrigin.X() >= 0 )
                                if( aScaleX.GetNumerator() >= 0 )
                                    aX += BigInt( aScaleX.GetNumerator()/2 );
                                else
                                    aX -= BigInt( (aScaleX.GetNumerator()+1)/2 );
                            else
                                if( aScaleX.GetNumerator() >= 0 )
                                    aX -= BigInt( (aScaleX.GetNumerator()-1)/2 );
                                else
                                    aX += BigInt( aScaleX.GetNumerator()/2 );
                            aX /= BigInt( aScaleX.GetNumerator() );
                            aOrigin.X() = (long)aX + aMM.GetOrigin().X();
                            BigInt aY( aOrigin.Y() );
                            aY *= BigInt( aScaleY.GetDenominator() );
                            if( aOrigin.Y() >= 0 )
                                if( aScaleY.GetNumerator() >= 0 )
                                    aY += BigInt( aScaleY.GetNumerator()/2 );
                                else
                                    aY -= BigInt( (aScaleY.GetNumerator()+1)/2 );
                            else
                                if( aScaleY.GetNumerator() >= 0 )
                                    aY -= BigInt( (aScaleY.GetNumerator()-1)/2 );
                                else
                                    aY += BigInt( aScaleY.GetNumerator()/2 );
                            aY /= BigInt( aScaleY.GetNumerator() );
                            aOrigin.Y() = (long)aY + aMM.GetOrigin().Y();
                            aSrcMapMode.SetOrigin( aOrigin );

                            aScaleX *= aSrcMapMode.GetScaleX();
                            aScaleY *= aSrcMapMode.GetScaleY();
                            aSrcMapMode.SetScaleX( aScaleX );
                            aSrcMapMode.SetScaleY( aScaleY );
                        }
                        else
                            aSrcMapMode=pA->GetMapMode();
                    }
                }
                break;

                case META_FONT_ACTION:
                {
                    const MetaFontAction* pA = (const MetaFontAction*) pMA;
                    aSrcFont = pA->GetFont();

                    if ( (aSrcFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW)
                         || (aSrcFont.GetCharSet() == RTL_TEXTENCODING_UNICODE) )
                    {
                        aSrcFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                    }
                    eSrcTextAlign = aSrcFont.GetAlign();
                    aSrcTextColor = aSrcFont.GetColor();
                    aSrcFont.SetAlign( ALIGN_BASELINE );
                    aSrcFont.SetColor( COL_WHITE );
                }
                break;

                case META_PUSH_ACTION:
                {
                    const MetaPushAction* pA = (const MetaPushAction*)pMA;

                    WMFWriterAttrStackMember* pAt = new WMFWriterAttrStackMember;
                    pAt->nFlags = pA->GetFlags();
                    pAt->aClipRegion = aSrcClipRegion;
                    pAt->aLineColor=aSrcLineColor;
                    pAt->aFillColor=aSrcFillColor;
                    pAt->eRasterOp=eSrcRasterOp;
                    pAt->aFont=aSrcFont;
                    pAt->eTextAlign=eSrcTextAlign;
                    pAt->aTextColor=aSrcTextColor;
                    pAt->aMapMode=aSrcMapMode;
                    pAt->aLineInfo=aDstLineInfo;
                    pAt->pSucc=pAttrStack;
                    pAttrStack=pAt;

                    SetAllAttr();           // update ( now all source attributes are equal to the destination attributes )
                    WMFRecord_SaveDC();

                }
                break;

                case META_POP_ACTION:
                {
                    WMFWriterAttrStackMember * pAt=pAttrStack;

                    if( pAt )
                    {
                        aDstLineInfo = pAt->aLineInfo;
                        aDstLineColor = pAt->aLineColor;
                        if ( pAt->nFlags & PUSH_LINECOLOR )
                            aSrcLineColor = pAt->aLineColor;
                        aDstFillColor = pAt->aFillColor;
                        if ( pAt->nFlags & PUSH_FILLCOLOR )
                            aSrcFillColor = pAt->aFillColor;
                        eDstROP2 = pAt->eRasterOp;
                        if ( pAt->nFlags & PUSH_RASTEROP )
                            eSrcRasterOp = pAt->eRasterOp;
                        aDstFont = pAt->aFont;
                        if ( pAt->nFlags & PUSH_FONT )
                            aSrcFont = pAt->aFont;
                        eDstTextAlign = pAt->eTextAlign;
                        if ( pAt->nFlags & ( PUSH_FONT | PUSH_TEXTALIGN ) )
                            eSrcTextAlign = pAt->eTextAlign;
                        aDstTextColor = pAt->aTextColor;
                        if ( pAt->nFlags & ( PUSH_FONT | PUSH_TEXTCOLOR ) )
                            aSrcTextColor = pAt->aTextColor;
                        if ( pAt->nFlags & PUSH_MAPMODE )
                            aSrcMapMode = pAt->aMapMode;
                        aDstClipRegion = pAt->aClipRegion;
                        if ( pAt->nFlags & PUSH_CLIPREGION )
                            aSrcClipRegion = pAt->aClipRegion;

                        WMFRecord_RestoreDC();
                        pAttrStack = pAt->pSucc;
                        delete pAt;
                    }
                }
                break;

                case META_EPS_ACTION :
                {
                    const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                    const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                    size_t nCount = aGDIMetaFile.GetActionSize();
                    for ( size_t i = 0; i < nCount; i++ )
                    {
                        const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                        if ( pMetaAct->GetType() == META_BMPSCALE_ACTION )
                        {
                            const MetaBmpScaleAction* pBmpScaleAction = (const MetaBmpScaleAction*)pMetaAct;
                            WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap() );
                            break;
                        }
                    }
                }
                break;

                case META_RASTEROP_ACTION:
                {
                    const MetaRasterOpAction* pA = (const MetaRasterOpAction*) pMA;
                    eSrcRasterOp=pA->GetRasterOp();
                }
                break;

                case META_TRANSPARENT_ACTION:
                {
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_PolyPolygon( ( (MetaTransparentAction*) pMA )->GetPolyPolygon() );
                }
                break;

                case META_FLOATTRANSPARENT_ACTION:
                {
                    const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pMA;

                    GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                    Point           aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                    const Size      aSrcSize( aTmpMtf.GetPrefSize() );
                    const Point     aDestPt( pA->GetPoint() );
                    const Size      aDestSize( pA->GetSize() );
                    const double    fScaleX = aSrcSize.Width() ? (double) aDestSize.Width() / aSrcSize.Width() : 1.0;
                    const double    fScaleY = aSrcSize.Height() ? (double) aDestSize.Height() / aSrcSize.Height() : 1.0;
                    long            nMoveX, nMoveY;

                    aSrcLineInfo = LineInfo();
                    SetAllAttr();

                    if( fScaleX != 1.0 || fScaleY != 1.0 )
                    {
                        aTmpMtf.Scale( fScaleX, fScaleY );
                        aSrcPt.X() = FRound( aSrcPt.X() * fScaleX ), aSrcPt.Y() = FRound( aSrcPt.Y() * fScaleY );
                    }

                    nMoveX = aDestPt.X() - aSrcPt.X(), nMoveY = aDestPt.Y() - aSrcPt.Y();

                    if( nMoveX || nMoveY )
                        aTmpMtf.Move( nMoveX, nMoveY );

                    WriteRecords( aTmpMtf );
                }
                break;

                case( META_LAYOUTMODE_ACTION ):
                {
                    sal_uInt32 nLayoutMode = ( (MetaLayoutModeAction*) pMA )->GetLayoutMode();
                    eSrcHorTextAlign = 0; // TA_LEFT
                    if (nLayoutMode & TEXT_LAYOUT_BIDI_RTL)
                    {
                        eSrcHorTextAlign = W_TA_RIGHT | W_TA_RTLREADING;
                    }
                    if (nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_RIGHT)
                        eSrcHorTextAlign |= W_TA_RIGHT;
                    else if (nLayoutMode & TEXT_LAYOUT_TEXTORIGIN_LEFT)
                        eSrcHorTextAlign &= ~W_TA_RIGHT;
                    break;
                }

                // Unsupported Actions
                case META_MASK_ACTION:
                case META_MASKSCALE_ACTION:
                case META_MASKSCALEPART_ACTION:
                {
                    OSL_FAIL( "Unsupported action: MetaMask...Action!" );
                }
                break;

                case META_CLIPREGION_ACTION:
                break;

                case META_ISECTREGIONCLIPREGION_ACTION:
                {
                    OSL_FAIL( "Unsupported action: MetaISectRegionClipRegionAction!" );
                }
                break;

                case META_MOVECLIPREGION_ACTION:
                {
                    OSL_FAIL( "Unsupported action: MetaMoveClipRegionAction!" );
                }
                break;

                default:
                {
                    OSL_FAIL( "Unsupported meta action!" );
                }
                break;
          }

          nWrittenActions++;
          MayCallback();

          if (pWMF->GetError())
            bStatus=false;

          if(!bStatus)
            break;
        }
    }
}

// ------------------------------------------------------------------------

void WMFWriter::WriteHeader( const GDIMetaFile &, bool bPlaceable )
{
    if( bPlaceable )
    {
        sal_uInt16  nCheckSum, nValue;
        Size    aSize( pVirDev->LogicToLogic(Size(1,1),MapMode(MAP_INCH), aTargetMapMode) );
        sal_uInt16  nUnitsPerInch = (sal_uInt16) ( ( aSize.Width() + aSize.Height() ) >> 1 );

        nCheckSum=0;
        nValue=0xcdd7;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x9ac6;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=(sal_uInt16) aTargetSize.Width();        nCheckSum^=nValue; *pWMF << nValue;
        nValue=(sal_uInt16) aTargetSize.Height();       nCheckSum^=nValue; *pWMF << nValue;
        nValue=nUnitsPerInch;                       nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        *pWMF << nCheckSum;
    }

    nMetafileHeaderPos=pWMF->Tell();
    *pWMF << (sal_uInt16)0x0001           // Typ: Datei
          << (sal_uInt16)0x0009           // Headerlaenge in Worten
          << (sal_uInt16)0x0300           // Version als BCD-Zahl
          << (sal_uInt32) 0x00000000      // Dateilaenge (ohne 1. Header), wird spaeter durch UpdateHeader() berichtigt
          << (sal_uInt16)MAXOBJECTHANDLES // Maximalezahl der gleichzeitigen Objekte
          << (sal_uInt32) 0x00000000      // Maximale Record-laenge, wird spaeter durch UpdateHeader() berichtigt
          << (sal_uInt16)0x0000;          // Reserved
}

// ------------------------------------------------------------------------

void WMFWriter::UpdateHeader()
{
    sal_uLong nPos;
    sal_uInt32 nFileSize;

    nPos=pWMF->Tell();                 // Endposition = Gesammtgroesse der Datei
    nFileSize=nPos-nMetafileHeaderPos; // Groesse des 1. Headers abziehen
    if ((nFileSize&1)!=0) {            // ggf. auf ganze Worte aufrunden
        *pWMF << (sal_uInt8)0;
        nPos++;
        nFileSize++;
    }
    nFileSize>>=1;                    // In Anzahl Worte umrechnen
    pWMF->Seek(nMetafileHeaderPos+6); // Zum Dateigroessen-Eintrag im zweiten Header
    *pWMF << nFileSize;               // Dateigroesse berichtigen
    pWMF->SeekRel(2);                 // Zum Max-Record-Laenge-Eintrag im zweiten Header
    *pWMF << nMaxRecordSize;          // und berichtigen
    pWMF->Seek(nPos);
}

// ------------------------------------------------------------------------

bool WMFWriter::WriteWMF( const GDIMetaFile& rMTF, SvStream& rTargetStream,
                            FilterConfigItem* pFConfigItem, bool bPlaceable )
{
    WMFWriterAttrStackMember * pAt;

    bEmbedEMF = true;
    bStatus=true;
    pConvert = 0;
    pVirDev = new VirtualDevice;

    pFilterConfigItem = pFConfigItem;
    if ( pFilterConfigItem )
    {
        xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            OUString aMsg;
            xStatusIndicator->start( aMsg, 100 );
        }
    }
    nLastPercent=0;

    pWMF=&rTargetStream;
    pWMF->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);

    nMaxRecordSize=0;

    aSrcMapMode=rMTF.GetPrefMapMode();

    if( bPlaceable )
    {
        aTargetMapMode = aSrcMapMode;
        aTargetSize = rMTF.GetPrefSize();
        nTargetDivisor = CalcSaveTargetMapMode(aTargetMapMode, aTargetSize);
        aTargetSize.Width() /= nTargetDivisor;
        aTargetSize.Height() /= nTargetDivisor;
    }
    else
    {
        aTargetMapMode = MapMode( MAP_INCH );

        const long      nUnit = pVirDev->LogicToPixel( Size( 1, 1 ), aTargetMapMode ).Width();
        const Fraction  aFrac( 1, nUnit );

        aTargetMapMode.SetScaleX( aFrac );
        aTargetMapMode.SetScaleY( aFrac );
        aTargetSize = pVirDev->LogicToLogic( rMTF.GetPrefSize(), aSrcMapMode, aTargetMapMode );
    }

    pVirDev->SetMapMode( aTargetMapMode );

    pAttrStack=NULL;

    for (sal_uInt16 i=0; i<MAXOBJECTHANDLES; i++)
        bHandleAllocated[i]=false;

    nDstPenHandle=0xffff;
    nDstFontHandle=0xffff;
    nDstBrushHandle=0xffff;

    nNumberOfActions=0;
    nNumberOfBitmaps=0;
    nWrittenActions=0;
    nWrittenBitmaps=0;
    nActBitmapPercent=0;

    CountActionsAndBitmaps(rMTF);

    WriteHeader(rMTF,bPlaceable);
    if( bEmbedEMF )
        WriteEmbeddedEMF( rMTF );
    WMFRecord_SetWindowOrg(Point(0,0));
    WMFRecord_SetWindowExt(rMTF.GetPrefSize());
    WMFRecord_SetBkMode( true );

    eDstROP2 = eSrcRasterOp = ROP_OVERPAINT;
    WMFRecord_SetROP2(eDstROP2);

    aDstLineInfo = LineInfo();
    aDstLineColor = aSrcLineColor = Color( COL_BLACK );
    CreateSelectDeletePen( aDstLineColor, aDstLineInfo );

    aDstFillColor = aSrcFillColor = Color( COL_WHITE );
    CreateSelectDeleteBrush( aDstFillColor );

    aDstClipRegion = aSrcClipRegion = Region();
    bDstIsClipping = bSrcIsClipping = false;

    Font aFont;
    aFont.SetCharSet( GetExtendedTextEncoding( RTL_TEXTENCODING_MS_1252 ) );
    aFont.SetColor( Color( COL_WHITE ) );
    aFont.SetAlign( ALIGN_BASELINE );
    aDstFont = aSrcFont = aFont;
    CreateSelectDeleteFont(aDstFont);

    eDstTextAlign = eSrcTextAlign = ALIGN_BASELINE;
    eDstHorTextAlign = eSrcHorTextAlign = W_TA_LEFT;
    WMFRecord_SetTextAlign( eDstTextAlign, eDstHorTextAlign );

    aDstTextColor = aSrcTextColor = Color( COL_WHITE );
    WMFRecord_SetTextColor(aDstTextColor);

    // Write records
    WriteRecords(rMTF);

    WMFRecord_EndOfFile();
    UpdateHeader();

    while(pAttrStack)
    {
        pAt=pAttrStack;
        pAttrStack=pAt->pSucc;
        delete pAt;
    }

    delete pVirDev;
    delete pConvert;

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bStatus;
}

// ------------------------------------------------------------------------

sal_uInt16 WMFWriter::CalcSaveTargetMapMode(MapMode& rMapMode,
                                        const Size& rPrefSize)
{
    Fraction    aDivFrac(2, 1);
    sal_uInt16      nDivisor = 1;

    Size aSize = pVirDev->LogicToLogic( rPrefSize, aSrcMapMode, rMapMode );

    while( nDivisor <= 64 && (aSize.Width() > 32767 || aSize.Height() > 32767) )
    {
        Fraction aFrac = rMapMode.GetScaleX();

        aFrac *= aDivFrac;
        rMapMode.SetScaleX(aFrac);
        aFrac = rMapMode.GetScaleY();
        aFrac *= aDivFrac;
        rMapMode.SetScaleY(aFrac);
        nDivisor <<= 1;
        aSize = pVirDev->LogicToLogic( rPrefSize, aSrcMapMode, rMapMode );
    }

    return nDivisor;
}

// ------------------------------------------------------------------------

void WMFWriter::WriteEmbeddedEMF( const GDIMetaFile& rMTF )
{
    SvMemoryStream aStream;
    EMFWriter aEMFWriter(aStream);

    if( aEMFWriter.WriteEMF( rMTF ) )
    {
        sal_Size nTotalSize = aStream.Tell();
        if( nTotalSize > SAL_MAX_UINT32 )
            return;
        aStream.Seek( 0 );
        sal_uInt32 nRemainingSize = static_cast< sal_uInt32 >( nTotalSize );
        sal_uInt32 nRecCounts = ( (nTotalSize - 1) / 0x2000 ) + 1;
        sal_uInt16 nCheckSum = 0, nWord;

        sal_uInt32 nPos = 0;

        while( nPos + 1 < nTotalSize )
        {
            aStream >> nWord;
            nCheckSum ^= nWord;
            nPos += 2;
        }

        nCheckSum = static_cast< sal_uInt16 >( nCheckSum * -1 );

        aStream.Seek( 0 );
        while( nRemainingSize > 0 )
        {
            sal_uInt32 nCurSize;
            if( nRemainingSize > 0x2000 )
            {
                nCurSize = 0x2000;
                nRemainingSize -= 0x2000;
            }
            else
            {
                nCurSize = nRemainingSize;
                nRemainingSize = 0;
            }
            WriteEMFRecord( aStream,
                            nCurSize,
                            nRemainingSize,
                            nTotalSize,
                            nRecCounts,
                            nCheckSum );
            nCheckSum = 0;
        }
    }
}

// ------------------------------------------------------------------------

void WMFWriter::WriteEMFRecord( SvMemoryStream& rStream, sal_uInt32 nCurSize, sal_uInt32 nRemainingSize,
                sal_uInt32 nTotalSize, sal_uInt32 nRecCounts, sal_uInt16 nCheckSum )
{
   // according to http://msdn.microsoft.com/en-us/library/dd366152%28PROT.13%29.aspx
   WriteRecordHeader( 0, W_META_ESCAPE );
   *pWMF << (sal_uInt16)W_MFCOMMENT         // same as META_ESCAPE_ENHANCED_METAFILE
          << (sal_uInt16)( nCurSize + 34 )  // we will always have a 34 byte escape header:
          << (sal_uInt32) 0x43464D57        // WMFC
          << (sal_uInt32) 0x00000001        // Comment type
          << (sal_uInt32) 0x00010000        // version
          << nCheckSum                      // check sum
          << (sal_uInt32) 0                 // flags = 0
          << nRecCounts                     // total number of records
          << nCurSize                       // size of this record's data
          << nRemainingSize                 // remaining size of data in following records, missing in MSDN documentation
          << nTotalSize;                    // total size of EMF stream

   pWMF->Write( static_cast< const sal_Char* >( rStream.GetData() ) + rStream.Tell(), nCurSize );
   rStream.SeekRel( nCurSize );
   UpdateRecordHeader();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
