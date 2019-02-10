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

#include <sal/config.h>
#include <osl/diagnose.h>

#include <algorithm>

#include "wmfwr.hxx"
#include <unotools/fontcvt.hxx>
#include "emfwr.hxx"
#include <rtl/crc.h>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <tools/bigint.hxx>
#include <tools/helpers.hxx>
#include <tools/tenccvt.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <osl/endian.h>
#include <vcl/dibtools.hxx>
#include <vcl/metric.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <memory>
#include <vcl/fontcharmap.hxx>

// MS Windows defines

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

WMFWriter::WMFWriter()
    : bStatus(false)
    , nLastPercent(0)
    , pWMF(nullptr)
    , pVirDev(nullptr)
    , nMetafileHeaderPos(0)
    , nMaxRecordSize(0)
    , nActRecordPos(0)
    , eSrcRasterOp(RasterOp::OverPaint)
    , eSrcTextAlign(ALIGN_BASELINE)
    , pAttrStack(nullptr)
    , eSrcHorTextAlign(W_TA_LEFT)
    , eDstROP2(RasterOp::OverPaint)
    , eDstTextAlign(ALIGN_BASELINE)
    , eDstHorTextAlign(W_TA_LEFT)
    , bHandleAllocated{}
    , nDstPenHandle(0)
    , nDstFontHandle(0)
    , nDstBrushHandle(0)
    , nNumberOfActions(0)
    , nNumberOfBitmaps(0)
    , nWrittenActions(0)
    , nWrittenBitmaps(0)
    , nActBitmapPercent(0)
    , bEmbedEMF(false)
{
}

void WMFWriter::MayCallback()
{
    if ( xStatusIndicator.is() )
    {
        sal_uLong nPercent;

        // we simply assume that 16386 actions match to a bitmap
        // (normally a metafile either contains only actions or some bitmaps and
        // almost no actions. In which case the ratio is less important)

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
            case MetaActionType::BMP:
            case MetaActionType::BMPSCALE:
            case MetaActionType::BMPSCALEPART:
            case MetaActionType::BMPEX:
            case MetaActionType::BMPEXSCALE:
            case MetaActionType::BMPEXSCALEPART:
                nNumberOfBitmaps++;
            break;
            default: break;
        }
        nNumberOfActions++;
    }
}

void WMFWriter::WritePointXY(const Point & rPoint)
{
    Point aPt( OutputDevice::LogicToLogic(rPoint,aSrcMapMode,aTargetMapMode) );
    pWMF->WriteInt16( aPt.X() ).WriteInt16( aPt.Y() );
}

void WMFWriter::WritePointYX(const Point & rPoint)
{
    Point aPt( OutputDevice::LogicToLogic(rPoint,aSrcMapMode,aTargetMapMode) );
    pWMF->WriteInt16( aPt.Y() ).WriteInt16( aPt.X() );
}

sal_Int32 WMFWriter::ScaleWidth( sal_Int32 nDX )
{
    Size aSz( OutputDevice::LogicToLogic(Size(nDX,0),aSrcMapMode,aTargetMapMode) );
    return aSz.Width();
}

void WMFWriter::WriteSize(const Size & rSize)
{
    Size aSz( OutputDevice::LogicToLogic(rSize,aSrcMapMode,aTargetMapMode) );
    pWMF->WriteInt16( aSz.Width() ).WriteInt16( aSz.Height() );
}

void WMFWriter::WriteHeightWidth(const Size & rSize)
{
    Size aSz( OutputDevice::LogicToLogic(rSize,aSrcMapMode,aTargetMapMode) );
    pWMF->WriteInt16( aSz.Height() ).WriteInt16( aSz.Width() );
}

void WMFWriter::WriteRectangle(const tools::Rectangle & rRect)
{
    WritePointYX(Point(rRect.Right()+1,rRect.Bottom()+1));
    WritePointYX(rRect.TopLeft());
}

void WMFWriter::WriteColor(const Color & rColor)
{
    pWMF->WriteUChar( rColor.GetRed() ).WriteUChar( rColor.GetGreen() ).WriteUChar( rColor.GetBlue() ).WriteUChar( 0 );
}

void WMFWriter::WriteRecordHeader(sal_uInt32 nSizeWords, sal_uInt16 nType)
{
    nActRecordPos=pWMF->Tell();
    if (nSizeWords>nMaxRecordSize) nMaxRecordSize=nSizeWords;
    pWMF->WriteUInt32( nSizeWords ).WriteUInt16( nType );
}

void WMFWriter::UpdateRecordHeader()
{
    sal_uLong nPos;
    sal_uInt32 nSize;

    nPos=pWMF->Tell(); nSize=nPos-nActRecordPos;
    if ((nSize & 1)!=0) {
        pWMF->WriteUChar( 0 );
        nPos++; nSize++;
    }
    nSize/=2;
    if (nSize>nMaxRecordSize) nMaxRecordSize=nSize;
    pWMF->Seek(nActRecordPos);
    pWMF->WriteUInt32( nSize );
    pWMF->Seek(nPos);
}

void WMFWriter::WMFRecord_Arc(const tools::Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_ARC);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}

void WMFWriter::WMFRecord_Chord(const tools::Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_CHORD);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}

void WMFWriter::WMFRecord_CreateBrushIndirect(const Color& rColor)
{
    WriteRecordHeader(0x00000007,W_META_CREATEBRUSHINDIRECT);

    if( rColor==COL_TRANSPARENT )
        pWMF->WriteUInt16( W_BS_HOLLOW );
    else
        pWMF->WriteUInt16( W_BS_SOLID );

    WriteColor( rColor );
    pWMF->WriteUInt16( 0 );
}

void WMFWriter::WMFRecord_CreateFontIndirect(const vcl::Font & rFont)
{
    sal_uInt16 nWeight,i;
    sal_uInt8 nPitchFamily;

    WriteRecordHeader(0x00000000,W_META_CREATEFONTINDIRECT);
    WriteHeightWidth(Size(rFont.GetFontSize().Width(),-rFont.GetFontSize().Height()));
    pWMF->WriteInt16( rFont.GetOrientation() ).WriteInt16( rFont.GetOrientation() );

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
    pWMF->WriteUInt16( nWeight );

    if (rFont.GetItalic()==ITALIC_NONE)       pWMF->WriteUChar( 0 ); else  pWMF->WriteUChar( 1 );
    if (rFont.GetUnderline()==LINESTYLE_NONE) pWMF->WriteUChar( 0 ); else  pWMF->WriteUChar( 1 );
    if (rFont.GetStrikeout()==STRIKEOUT_NONE) pWMF->WriteUChar( 0 ); else  pWMF->WriteUChar( 1 );

    rtl_TextEncoding  eFontNameEncoding = rFont.GetCharSet();
    sal_uInt8         nCharSet = rtl_getBestWindowsCharsetFromTextEncoding( eFontNameEncoding );
    if ( eFontNameEncoding == RTL_TEXTENCODING_SYMBOL )
        eFontNameEncoding = RTL_TEXTENCODING_MS_1252;
    if ( nCharSet == 1 )
        nCharSet = W_ANSI_CHARSET;
    pWMF->WriteUChar( nCharSet );

    pWMF->WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 );

    switch (rFont.GetPitch()) {
        case PITCH_FIXED:    nPitchFamily=W_FIXED_PITCH;    break;
        case PITCH_VARIABLE: nPitchFamily=W_VARIABLE_PITCH; break;
        default:             nPitchFamily=W_DEFAULT_PITCH;
    }
    switch (rFont.GetFamilyType()) {
        case FAMILY_DECORATIVE: nPitchFamily|=W_FF_DECORATIVE; break;
        case FAMILY_MODERN:     nPitchFamily|=W_FF_MODERN;     break;
        case FAMILY_ROMAN:      nPitchFamily|=W_FF_ROMAN;      break;
        case FAMILY_SCRIPT:     nPitchFamily|=W_FF_SCRIPT;     break;
        case FAMILY_SWISS:      nPitchFamily|=W_FF_SWISS;      break;
        default:                nPitchFamily|=W_FF_DONTCARE;
    }
    pWMF->WriteUChar( nPitchFamily );

    OString aFontName(OUStringToOString(rFont.GetFamilyName(), eFontNameEncoding));
    for ( i = 0; i < W_LF_FACESIZE; i++ )
    {
        sal_Char nChar = ( i < aFontName.getLength() ) ? aFontName[i] : 0;
        pWMF->WriteChar( nChar );
    }
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_CreatePenIndirect(const Color& rColor, const LineInfo& rLineInfo )
{
    WriteRecordHeader(0x00000008,W_META_CREATEPENINDIRECT);
    sal_uInt16 nStyle = rColor == COL_TRANSPARENT ? W_PS_NULL : W_PS_SOLID;
    switch( rLineInfo.GetStyle() )
    {
        case LineStyle::Dash :
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
        case LineStyle::NONE :
            nStyle = W_PS_NULL;
        break;
        default:
        break;
    }
    pWMF->WriteUInt16( nStyle );

    WriteSize( Size( rLineInfo.GetWidth(), 0 ) );
    WriteColor( rColor );
}

void WMFWriter::WMFRecord_DeleteObject(sal_uInt16 nObjectHandle)
{
    WriteRecordHeader(0x00000004,W_META_DELETEOBJECT);
    pWMF->WriteUInt16( nObjectHandle );
}

void WMFWriter::WMFRecord_Ellipse(const tools::Rectangle & rRect)
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
    pWMF->WriteUInt16( W_MFCOMMENT )
         .WriteUInt16( nLen + 14 )      // we will always have a fourteen byte escape header:
         .WriteUInt16( 0x4f4f )         // OO
         .WriteUInt32( 0xa2c2a )        // evil magic number
         .WriteUInt32( nCheckSum )      // crc32 checksum about nEsc & pData
         .WriteUInt32( nEsc );          // escape number
    pWMF->WriteBytes( pData, nLen );
    if ( nLen & 1 )
        pWMF->WriteUChar( 0 );          // pad byte
}

/* if return value is true, then a complete unicode string and also a polygon replacement has been written,
    so there is no more action necessary
*/
bool WMFWriter::WMFRecord_Escape_Unicode( const Point& rPoint, const OUString& rUniStr, const long* pDXAry )
{
    bool bEscapeUsed = false;

    sal_uInt32 i, nStringLen = rUniStr.getLength();
    if ( nStringLen )
    {
        // first we will check if a comment is necessary
        if ( aSrcFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL )     // symbol is always byte character, so there is no unicode loss
        {
            const sal_Unicode* pBuf = rUniStr.getStr();
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
                pBuf = rUniStr.getStr();
                const sal_Unicode* pCheckChar = pBuf;
                rtl_TextEncoding aTextEncoding = getBestMSEncodingByChar(*pCheckChar); // try the first character
                if (aTextEncoding == RTL_TEXTENCODING_DONTKNOW) {
                    aTextEncoding = aTextEncodingOrg;
                }
                for ( i = 1; i < nStringLen; i++)
                {
                    if (aTextEncoding != aTextEncodingOrg) // found something
                        break;
                    pCheckChar++;
                    aTextEncoding = getBestMSEncodingByChar(*pCheckChar); // try the next character
                    if (aTextEncoding == RTL_TEXTENCODING_DONTKNOW) {
                        aTextEncoding = aTextEncodingOrg;
                    }
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

            if ( ( i != nStringLen ) || IsStarSymbol( aSrcFont.GetFamilyName() ) )    // after conversion the characters are not original, so we
            {                                                                   // will store the unicode string and a polypoly replacement
                Color aOldFillColor( aSrcFillColor );
                Color aOldLineColor( aSrcLineColor );
                aSrcLineInfo  = LineInfo();
                aSrcFillColor = aSrcTextColor;
                aSrcLineColor = COL_TRANSPARENT;
                SetLineAndFillAttr();
                pVirDev->SetFont( aSrcFont );
                std::vector<tools::PolyPolygon> aPolyPolyVec;
                if ( pVirDev->GetTextOutlines( aPolyPolyVec, rUniStr ) )
                {
                    sal_uInt32 nDXCount = pDXAry ? nStringLen : 0;
                    sal_uInt32 nSkipActions = aPolyPolyVec.size();
                    sal_Int32 nStrmLen = 8 +
                                           + sizeof( nStringLen ) + ( nStringLen * 2 )
                                           + sizeof( nDXCount ) + ( nDXCount * 4 )
                                           + sizeof( nSkipActions );

                    SvMemoryStream aMemoryStream( nStrmLen );
                    Point aPt( OutputDevice::LogicToLogic( rPoint, aSrcMapMode, aTargetMapMode ) );
                    aMemoryStream.WriteInt32( aPt.X() )
                                 .WriteInt32( aPt.Y() )
                                 .WriteUInt32( nStringLen );
                    for ( i = 0; i < nStringLen; i++ )
                        aMemoryStream.WriteUInt16( rUniStr[ i ] );
                    aMemoryStream.WriteUInt32( nDXCount );
                    for ( i = 0; i < nDXCount; i++ )
                        aMemoryStream.WriteInt32( pDXAry[ i ] );
                    aMemoryStream.WriteUInt32( nSkipActions );
                    WMFRecord_Escape( PRIVATE_ESCAPE_UNICODE, nStrmLen, static_cast<const sal_Int8*>(aMemoryStream.GetData()) );

                    for ( const auto& rPolyPoly : aPolyPolyVec )
                    {
                        tools::PolyPolygon aPolyPoly( rPolyPoly );
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

void WMFWriter::WMFRecord_ExtTextOut( const Point& rPoint,
                                      const OUString& rString,
                                      const long* pDXAry )
{
    sal_Int32 nOriginalTextLen = rString.getLength();

    if ( (nOriginalTextLen <= 1) || (pDXAry == nullptr) )
    {
        WMFRecord_TextOut(rPoint, rString);
        return;
    }
    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
    OString aByteString(OUStringToOString(rString, eChrSet));
    TrueExtTextOut(rPoint, rString, aByteString, pDXAry);
}

void WMFWriter::TrueExtTextOut( const Point& rPoint, const OUString& rString,
                                const OString& rByteString, const long* pDXAry )
{
    WriteRecordHeader( 0, W_META_EXTTEXTOUT );
    WritePointYX( rPoint );
    sal_uInt16 nNewTextLen = static_cast<sal_uInt16>(rByteString.getLength());
    pWMF->WriteUInt16( nNewTextLen ).WriteUInt16( 0 );
    write_uInt8s_FromOString(*pWMF, rByteString, nNewTextLen);
    if ( nNewTextLen & 1 )
        pWMF->WriteUChar( 0 );

    sal_Int32 nOriginalTextLen = rString.getLength();
    std::unique_ptr<sal_Int16[]> pConvertedDXAry(new sal_Int16[ nOriginalTextLen ]);
    sal_Int32 j = 0;
    pConvertedDXAry[ j++ ] = static_cast<sal_Int16>(ScaleWidth( pDXAry[ 0 ] ));
    for (sal_Int32 i = 1; i < ( nOriginalTextLen - 1 ); ++i)
        pConvertedDXAry[ j++ ] = static_cast<sal_Int16>(ScaleWidth( pDXAry[ i ] - pDXAry[ i - 1 ] ));
    pConvertedDXAry[ j ] = static_cast<sal_Int16>(ScaleWidth( pDXAry[ nOriginalTextLen - 2 ] / ( nOriginalTextLen - 1 ) ));

    for (sal_Int32 i = 0; i < nOriginalTextLen; ++i)
    {
        sal_Int16 nDx = pConvertedDXAry[ i ];
        pWMF->WriteInt16( nDx );
        if ( nOriginalTextLen < nNewTextLen )
        {
            sal_Unicode nUniChar = rString[i];
            OString aTemp(&nUniChar, 1, aSrcFont.GetCharSet());
            j = aTemp.getLength();
            while ( --j > 0 )
                pWMF->WriteUInt16( 0 );
        }
    }
    pConvertedDXAry.reset();
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

void WMFWriter::WMFRecord_Pie(const tools::Rectangle & rRect, const Point & rStartPt, const Point & rEndPt)
{
    WriteRecordHeader(0x0000000b,W_META_PIE);
    WritePointYX(rEndPt);
    WritePointYX(rStartPt);
    WriteRectangle(rRect);
}

void WMFWriter::WMFRecord_Polygon(const tools::Polygon & rPoly)
{
    tools::Polygon aSimplePoly;
    if ( rPoly.HasFlags() )
        rPoly.AdaptiveSubdivide( aSimplePoly );
    else
        aSimplePoly = rPoly;
    const sal_uInt16 nSize = aSimplePoly.GetSize();
    WriteRecordHeader(static_cast<sal_uInt32>(nSize)*2+4,W_META_POLYGON);
    pWMF->WriteUInt16( nSize );
    for (sal_uInt16 i=0; i<nSize; ++i)
        WritePointXY(aSimplePoly.GetPoint(i));
}

void WMFWriter::WMFRecord_PolyLine(const tools::Polygon & rPoly)
{
    tools::Polygon aSimplePoly;
    if ( rPoly.HasFlags() )
        rPoly.AdaptiveSubdivide( aSimplePoly );
    else
        aSimplePoly = rPoly;
    const sal_uInt16 nSize = aSimplePoly.GetSize();
    WriteRecordHeader(static_cast<sal_uInt32>(nSize)*2+4,W_META_POLYLINE);
    pWMF->WriteUInt16( nSize );
    for (sal_uInt16 i=0; i<nSize; ++i)
        WritePointXY(aSimplePoly.GetPoint(i));
}

void WMFWriter::WMFRecord_PolyPolygon(const tools::PolyPolygon & rPolyPoly)
{
    const tools::Polygon * pPoly;
    sal_uInt16 nCount,nSize,i,j;

    nCount=rPolyPoly.Count();
    tools::PolyPolygon aSimplePolyPoly( rPolyPoly );
    for ( i = 0; i < nCount; i++ )
    {
        if ( aSimplePolyPoly[ i ].HasFlags() )
        {
            tools::Polygon aSimplePoly;
            aSimplePolyPoly[ i ].AdaptiveSubdivide( aSimplePoly );
            aSimplePolyPoly[ i ] = aSimplePoly;
        }
    }
    WriteRecordHeader(0,W_META_POLYPOLYGON);
    pWMF->WriteUInt16( nCount );
    for (i=0; i<nCount; i++) pWMF->WriteUInt16( aSimplePolyPoly.GetObject(i).GetSize() );
    for (i=0; i<nCount; i++) {
        pPoly=&(aSimplePolyPoly.GetObject(i));
        nSize=pPoly->GetSize();
        for (j=0; j<nSize; j++) WritePointXY(pPoly->GetPoint(j));
    }
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_Rectangle(const tools::Rectangle & rRect)
{
    WriteRecordHeader( 0x00000007,W_META_RECTANGLE );
    WriteRectangle( rRect );
}

void WMFWriter::WMFRecord_RestoreDC()
{
    WriteRecordHeader(0x00000004,W_META_RESTOREDC);
    pWMF->WriteInt16( -1 );
}

void WMFWriter::WMFRecord_RoundRect(const tools::Rectangle & rRect, long nHorzRound, long nVertRound)
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
    pWMF->WriteUInt16( nObjectHandle );
}

void WMFWriter::WMFRecord_SetBkMode(bool bTransparent)
{
    WriteRecordHeader(0x00000004,W_META_SETBKMODE);
    if (bTransparent) pWMF->WriteUInt16( W_TRANSPARENT );
    else                    pWMF->WriteUInt16( W_OPAQUE );
}

void WMFWriter::WMFRecord_SetStretchBltMode()
{
    WriteRecordHeader( 0x00000004, W_META_SETSTRETCHBLTMODE );
    pWMF->WriteUInt16( 3 ); // STRETCH_DELETESCANS
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
        case RasterOp::Invert: nROP2=W_R2_NOT;        break;
        case RasterOp::Xor:    nROP2=W_R2_XORPEN;     break;
        default:               nROP2=W_R2_COPYPEN;
    }
    WriteRecordHeader(0x00000004,W_META_SETROP2);
    pWMF->WriteUInt16( nROP2 );
}

void WMFWriter::WMFRecord_SetTextAlign(FontAlign eFontAlign, sal_uInt16 eHorTextAlign)
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
    pWMF->WriteUInt16( nAlign );
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

    // The sequence in the metafile should be:
    // some parameters (length 22), then the bitmap without FILEHEADER.
    // As *pWMF << rBitmap generates a FILEHEADER of size 14,
    // we first write the bitmap at the right position
    // and overwrite later the FILEHEADER with the parameters.
    nPosAnf=pWMF->Tell(); // remember position, where parameters should be stored
    pWMF->WriteInt32( 0 ).WriteInt32( 0 ); // replenish 8 bytes (these 8 bytes +
                                           // 14 bytes superfluous FILEHEADER
                                           // = 22 bytes parameter)

    // write bitmap
    WriteDIB(rBitmap, *pWMF, false, true);

    // write the parameters:
    nPosEnd=pWMF->Tell();
    pWMF->Seek(nPosAnf);

    // determine raster-op, if nothing was passed
    if( !nROP )
    {
        switch( eSrcRasterOp )
        {
            case RasterOp::Invert: nROP = W_DSTINVERT; break;
            case RasterOp::Xor:    nROP = W_SRCINVERT; break;
            default:               nROP = W_SRCCOPY;
        }
    }

    pWMF->WriteUInt32( nROP ).
        WriteInt16( 0 ).
        WriteInt16( rBitmap.GetSizePixel().Height() ).
        WriteInt16( rBitmap.GetSizePixel().Width() ).
        WriteInt16( 0 ).
        WriteInt16( 0 );

    WriteHeightWidth(rSize);
    WritePointYX(rPoint);
    pWMF->Seek(nPosEnd);

    UpdateRecordHeader();

    nWrittenBitmaps++;
    nActBitmapPercent=0;
}

void WMFWriter::WMFRecord_TextOut(const Point & rPoint, const OUString & rStr)
{
    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
    OString aString(OUStringToOString(rStr, eChrSet));
    TrueTextOut(rPoint, aString);
}

void WMFWriter::TrueTextOut(const Point & rPoint, const OString& rString)
{
    WriteRecordHeader(0,W_META_TEXTOUT);

    write_uInt16_lenPrefixed_uInt8s_FromOString(*pWMF, rString);
    sal_Int32 nLen = rString.getLength();
    if ((nLen&1)!=0) pWMF->WriteUChar( 0 );
    WritePointYX(rPoint);
    UpdateRecordHeader();
}

void WMFWriter::WMFRecord_IntersectClipRect( const tools::Rectangle& rRect )
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

void WMFWriter::CreateSelectDeleteFont(const vcl::Font & rFont)
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
        if ( aDstFont.GetFamilyName() != aSrcFont.GetFamilyName() )
        {
            FontCharMapRef xFontCharMap;
            if ( pVirDev->GetFontCharMap( xFontCharMap ) )
            {
                if ( ( xFontCharMap->GetFirstChar() & 0xff00 ) == 0xf000 )
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

            for(auto const& rB2DPolygon : aLinePolyPolygon)
            {
                WMFRecord_PolyLine( tools::Polygon(rB2DPolygon) );
            }
        }

        if(aFillPolyPolygon.count())
        {
            const Color aOldLineColor(aSrcLineColor);
            const Color aOldFillColor(aSrcFillColor);

            aSrcLineColor = COL_TRANSPARENT;
            aSrcFillColor = aOldLineColor;
            SetLineAndFillAttr();

            for(auto const& rB2DPolygon : aFillPolyPolygon)
            {
                WMFRecord_Polygon( tools::Polygon(rB2DPolygon) );
            }

            aSrcLineColor = aOldLineColor;
            aSrcFillColor = aOldFillColor;
            SetLineAndFillAttr();
        }
    }
}

void WMFWriter::WriteRecords( const GDIMetaFile & rMTF )
{
    if( bStatus )
    {
        size_t nACount = rMTF.GetActionSize();

        WMFRecord_SetStretchBltMode();

        for( size_t nA = 0; nA < nACount; nA++ )
        {
            MetaAction* pMA = rMTF.GetAction( nA );

            switch( pMA->GetType() )
            {
                case MetaActionType::PIXEL:
                {
                    const MetaPixelAction* pA = static_cast<const MetaPixelAction *>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_SetPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case MetaActionType::POINT:
                {
                    const MetaPointAction*  pA = static_cast<const MetaPointAction*>(pMA);
                    const Point&            rPt = pA->GetPoint();
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_MoveTo( rPt);
                    WMFRecord_LineTo( rPt );
                }
                break;

                case MetaActionType::LINE:
                {
                    const MetaLineAction* pA = static_cast<const MetaLineAction *>(pMA);
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

                case MetaActionType::RECT:
                {
                    const MetaRectAction* pA = static_cast<const MetaRectAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Rectangle( pA->GetRect() );
                }
                break;

                case MetaActionType::ROUNDRECT:
                {
                    const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_RoundRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case MetaActionType::ELLIPSE:
                {
                    const MetaEllipseAction* pA = static_cast<const MetaEllipseAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Ellipse( pA->GetRect() );
                }
                break;

                case MetaActionType::ARC:
                {
                    const MetaArcAction* pA = static_cast<const MetaArcAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Arc( pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint() );
                }
                break;

                case MetaActionType::PIE:
                {
                    const MetaPieAction* pA = static_cast<const MetaPieAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Pie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case MetaActionType::CHORD:
                {
                    const MetaChordAction* pA = static_cast<const MetaChordAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Chord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case MetaActionType::POLYLINE:
                {
                    const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pMA);
                    const tools::Polygon& rPoly = pA->GetPolygon();

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

                case MetaActionType::POLYGON:
                {
                    const MetaPolygonAction* pA = static_cast<const MetaPolygonAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_Polygon( pA->GetPolygon() );
                }
                break;

                case MetaActionType::POLYPOLYGON:
                {
                    const MetaPolyPolygonAction* pA = static_cast<const MetaPolyPolygonAction*>(pMA);
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_PolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case MetaActionType::TEXTRECT:
                {
                    const MetaTextRectAction * pA = static_cast<const MetaTextRectAction*>(pMA);
                    OUString aTemp( pA->GetText() );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();

                    Point aPos( pA->GetRect().TopLeft() );
                    if ( !WMFRecord_Escape_Unicode( aPos, aTemp, nullptr ) )
                        WMFRecord_TextOut( aPos, aTemp );
                }
                break;

                case MetaActionType::TEXT:
                {
                    const MetaTextAction * pA = static_cast<const MetaTextAction*>(pMA);
                    OUString aTemp = pA->GetText().copy( pA->GetIndex(), std::min<sal_Int32>(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();
                    if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, nullptr ) )
                        WMFRecord_TextOut( pA->GetPoint(), aTemp );
                }
                break;

                case MetaActionType::TEXTARRAY:
                {
                    const MetaTextArrayAction* pA = static_cast<const MetaTextArrayAction*>(pMA);

                    OUString aTemp = pA->GetText().copy( pA->GetIndex(), std::min<sal_Int32>(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );
                    aSrcLineInfo = LineInfo();
                    SetAllAttr();
                    if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, pA->GetDXArray() ) )
                        WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pA->GetDXArray() );
                }
                break;

                case MetaActionType::STRETCHTEXT:
                {
                    const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction *>(pMA);
                    OUString aTemp = pA->GetText().copy( pA->GetIndex(), std::min<sal_Int32>(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) );

                    pVirDev->SetFont( aSrcFont );
                    const sal_Int32 nLen = aTemp.getLength();
                    std::unique_ptr<long[]> pDXAry(nLen ? new long[ nLen ] : nullptr);
                    const sal_Int32 nNormSize = pVirDev->GetTextArray( aTemp, pDXAry.get() );
                    if (nLen && nNormSize == 0)
                    {
                        OSL_FAIL("Impossible div by 0 action: MetaStretchTextAction!");
                    }
                    else
                    {
                        for ( sal_Int32 i = 0; i < ( nLen - 1 ); i++ )
                            pDXAry[ i ] = pDXAry[ i ] * static_cast<sal_Int32>(pA->GetWidth()) / nNormSize;
                        if ( ( nLen <= 1 ) || ( static_cast<sal_Int32>(pA->GetWidth()) == nNormSize ) )
                            pDXAry.reset();
                        aSrcLineInfo = LineInfo();
                        SetAllAttr();
                        if ( !WMFRecord_Escape_Unicode( pA->GetPoint(), aTemp, pDXAry.get() ) )
                            WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pDXAry.get() );
                    }
                }
                break;

                case MetaActionType::BMP:
                {
                    const MetaBmpAction* pA = static_cast<const MetaBmpAction *>(pMA);
                    WMFRecord_StretchDIB( pA->GetPoint(), pA->GetBitmap().GetSizePixel(), pA->GetBitmap() );
                }
                break;

                case MetaActionType::BMPSCALE:
                {
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pMA);
                    WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), pA->GetBitmap() );
                }
                break;

                case MetaActionType::BMPSCALEPART:
                {
                    const MetaBmpScalePartAction*   pA = static_cast<const MetaBmpScalePartAction*>(pMA);
                    Bitmap                          aTmp( pA->GetBitmap() );

                    if( aTmp.Crop( tools::Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) ) )
                        WMFRecord_StretchDIB( pA->GetDestPoint(), pA->GetDestSize(), aTmp );
                }
                break;

                case MetaActionType::BMPEX:
                {
                    const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction *>(pMA);
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

                case MetaActionType::BMPEXSCALE:
                {
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pMA);
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

                case MetaActionType::BMPEXSCALEPART:
                {
                    const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pMA);
                    BitmapEx                        aBmpEx( pA->GetBitmapEx() );
                    aBmpEx.Crop( tools::Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
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

                case MetaActionType::GRADIENT:
                {
                    const MetaGradientAction*   pA = static_cast<const MetaGradientAction*>(pMA);
                    GDIMetaFile                 aTmpMtf;

                    pVirDev->AddGradientActions( pA->GetRect(), pA->GetGradient(), aTmpMtf );
                    WriteRecords( aTmpMtf );
                }
                break;

                case MetaActionType::HATCH:
                {
                    const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pMA);
                    GDIMetaFile             aTmpMtf;

                    pVirDev->AddHatchActions( pA->GetPolyPolygon(), pA->GetHatch(), aTmpMtf );
                    WriteRecords( aTmpMtf );
                }
                break;

                case MetaActionType::WALLPAPER:
                {
                    const MetaWallpaperAction*  pA = static_cast<const MetaWallpaperAction*>(pMA);
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

                case MetaActionType::ISECTRECTCLIPREGION:
                {
                    const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pMA);
                    WMFRecord_IntersectClipRect( pA->GetRect() );
                }
                break;

                case MetaActionType::LINECOLOR:
                {
                    const MetaLineColorAction* pA = static_cast<const MetaLineColorAction*>(pMA);

                    if( pA->IsSetting() )
                        aSrcLineColor = pA->GetColor();
                    else
                        aSrcLineColor = COL_TRANSPARENT;
                }
                break;

                case MetaActionType::FILLCOLOR:
                {
                    const MetaFillColorAction* pA = static_cast<const MetaFillColorAction*>(pMA);

                    if( pA->IsSetting() )
                        aSrcFillColor = pA->GetColor();
                    else
                        aSrcFillColor = COL_TRANSPARENT;
                }
                break;

                case MetaActionType::TEXTCOLOR:
                {
                    const MetaTextColorAction* pA = static_cast<const MetaTextColorAction*>(pMA);
                    aSrcTextColor = pA->GetColor();
                }
                break;

                case MetaActionType::TEXTFILLCOLOR:
                {
                    const MetaTextFillColorAction* pA = static_cast<const MetaTextFillColorAction*>(pMA);
                    if( pA->IsSetting() )
                        aSrcFont.SetFillColor( pA->GetColor() );
                    else
                        aSrcFont.SetFillColor( COL_TRANSPARENT );
                }
                break;

                case MetaActionType::TEXTALIGN:
                {
                    const MetaTextAlignAction* pA = static_cast<const MetaTextAlignAction*>(pMA);
                    eSrcTextAlign = pA->GetTextAlign();
                }
                break;

                case MetaActionType::MAPMODE:
                {
                    const MetaMapModeAction* pA = static_cast<const MetaMapModeAction*>(pMA);

                    if (aSrcMapMode!=pA->GetMapMode())
                    {
                        if( pA->GetMapMode().GetMapUnit() == MapUnit::MapRelative )
                        {
                            const MapMode& aMM = pA->GetMapMode();
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
                            aOrigin.setX( static_cast<long>(aX) + aMM.GetOrigin().X() );
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
                            aOrigin.setY( static_cast<long>(aY) + aMM.GetOrigin().Y() );
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

                case MetaActionType::FONT:
                {
                    const MetaFontAction* pA = static_cast<const MetaFontAction*>(pMA);
                    aSrcFont = pA->GetFont();

                    if ( (aSrcFont.GetCharSet() == RTL_TEXTENCODING_DONTKNOW)
                         || (aSrcFont.GetCharSet() == RTL_TEXTENCODING_UNICODE) )
                    {
                        aSrcFont.SetCharSet( RTL_TEXTENCODING_MS_1252 );
                    }
                    eSrcTextAlign = aSrcFont.GetAlignment();
                    aSrcTextColor = aSrcFont.GetColor();
                    aSrcFont.SetAlignment( ALIGN_BASELINE );
                    aSrcFont.SetColor( COL_WHITE );
                }
                break;

                case MetaActionType::PUSH:
                {
                    const MetaPushAction* pA = static_cast<const MetaPushAction*>(pMA);

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

                case MetaActionType::POP:
                {
                    WMFWriterAttrStackMember * pAt=pAttrStack;

                    if( pAt )
                    {
                        aDstLineInfo = pAt->aLineInfo;
                        aDstLineColor = pAt->aLineColor;
                        if ( pAt->nFlags & PushFlags::LINECOLOR )
                            aSrcLineColor = pAt->aLineColor;
                        aDstFillColor = pAt->aFillColor;
                        if ( pAt->nFlags & PushFlags::FILLCOLOR )
                            aSrcFillColor = pAt->aFillColor;
                        eDstROP2 = pAt->eRasterOp;
                        if ( pAt->nFlags & PushFlags::RASTEROP )
                            eSrcRasterOp = pAt->eRasterOp;
                        aDstFont = pAt->aFont;
                        if ( pAt->nFlags & PushFlags::FONT )
                            aSrcFont = pAt->aFont;
                        eDstTextAlign = pAt->eTextAlign;
                        if ( pAt->nFlags & ( PushFlags::FONT | PushFlags::TEXTALIGN ) )
                            eSrcTextAlign = pAt->eTextAlign;
                        aDstTextColor = pAt->aTextColor;
                        if ( pAt->nFlags & ( PushFlags::FONT | PushFlags::TEXTCOLOR ) )
                            aSrcTextColor = pAt->aTextColor;
                        if ( pAt->nFlags & PushFlags::MAPMODE )
                            aSrcMapMode = pAt->aMapMode;
                        aDstClipRegion = pAt->aClipRegion;
                        if ( pAt->nFlags & PushFlags::CLIPREGION )
                            aSrcClipRegion = pAt->aClipRegion;

                        WMFRecord_RestoreDC();
                        pAttrStack = pAt->pSucc;
                        delete pAt;
                    }
                }
                break;

                case MetaActionType::EPS :
                {
                    const MetaEPSAction* pA = static_cast<const MetaEPSAction*>(pMA);
                    const GDIMetaFile& aGDIMetaFile( pA->GetSubstitute() );

                    size_t nCount = aGDIMetaFile.GetActionSize();
                    for ( size_t i = 0; i < nCount; i++ )
                    {
                        const MetaAction* pMetaAct = aGDIMetaFile.GetAction( i );
                        if ( pMetaAct->GetType() == MetaActionType::BMPSCALE )
                        {
                            const MetaBmpScaleAction* pBmpScaleAction = static_cast<const MetaBmpScaleAction*>(pMetaAct);
                            WMFRecord_StretchDIB( pA->GetPoint(), pA->GetSize(), pBmpScaleAction->GetBitmap() );
                            break;
                        }
                    }
                }
                break;

                case MetaActionType::RASTEROP:
                {
                    const MetaRasterOpAction* pA = static_cast<const MetaRasterOpAction*>(pMA);
                    eSrcRasterOp=pA->GetRasterOp();
                }
                break;

                case MetaActionType::Transparent:
                {
                    aSrcLineInfo = LineInfo();
                    SetLineAndFillAttr();
                    WMFRecord_PolyPolygon( static_cast<const MetaTransparentAction*>(pMA)->GetPolyPolygon() );
                }
                break;

                case MetaActionType::FLOATTRANSPARENT:
                {
                    const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pMA);

                    GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                    Point           aSrcPt( aTmpMtf.GetPrefMapMode().GetOrigin() );
                    const Size      aSrcSize( aTmpMtf.GetPrefSize() );
                    const Point     aDestPt( pA->GetPoint() );
                    const Size      aDestSize( pA->GetSize() );
                    const double    fScaleX = aSrcSize.Width() ? static_cast<double>(aDestSize.Width()) / aSrcSize.Width() : 1.0;
                    const double    fScaleY = aSrcSize.Height() ? static_cast<double>(aDestSize.Height()) / aSrcSize.Height() : 1.0;
                    long            nMoveX, nMoveY;

                    aSrcLineInfo = LineInfo();
                    SetAllAttr();

                    if( fScaleX != 1.0 || fScaleY != 1.0 )
                    {
                        aTmpMtf.Scale( fScaleX, fScaleY );
                        aSrcPt.setX( FRound( aSrcPt.X() * fScaleX ) );
                        aSrcPt.setY( FRound( aSrcPt.Y() * fScaleY ) );
                    }

                    nMoveX = aDestPt.X() - aSrcPt.X();
                    nMoveY = aDestPt.Y() - aSrcPt.Y();

                    if( nMoveX || nMoveY )
                        aTmpMtf.Move( nMoveX, nMoveY );

                    WriteRecords( aTmpMtf );
                }
                break;

                case MetaActionType::LAYOUTMODE:
                {
                    ComplexTextLayoutFlags nLayoutMode = static_cast<const MetaLayoutModeAction*>(pMA)->GetLayoutMode();
                    eSrcHorTextAlign = 0; // TA_LEFT
                    if ((nLayoutMode & ComplexTextLayoutFlags::BiDiRtl) != ComplexTextLayoutFlags::Default)
                    {
                        eSrcHorTextAlign = W_TA_RIGHT | W_TA_RTLREADING;
                    }
                    if ((nLayoutMode & ComplexTextLayoutFlags::TextOriginRight) != ComplexTextLayoutFlags::Default)
                        eSrcHorTextAlign |= W_TA_RIGHT;
                    else if ((nLayoutMode & ComplexTextLayoutFlags::TextOriginLeft) != ComplexTextLayoutFlags::Default)
                        eSrcHorTextAlign &= ~W_TA_RIGHT;
                    break;
                }

                case MetaActionType::CLIPREGION:
                case MetaActionType::TEXTLANGUAGE:
                case MetaActionType::COMMENT:
                    // Explicitly ignored cases
                break;

                default:
                    // TODO: Implement more cases as necessary. Let's not bother with a warning.
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

void WMFWriter::WriteHeader( bool bPlaceable )
{
    if( bPlaceable )
    {
        sal_uInt16  nCheckSum, nValue;
        Size    aSize( OutputDevice::LogicToLogic(Size(1,1),MapMode(MapUnit::MapInch), aTargetMapMode) );
        sal_uInt16  nUnitsPerInch = static_cast<sal_uInt16>( ( aSize.Width() + aSize.Height() ) >> 1 );

        nCheckSum=0;
        nValue=0xcdd7;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x9ac6;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x0000;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x0000;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x0000;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=static_cast<sal_uInt16>(aTargetSize.Width());        nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=static_cast<sal_uInt16>(aTargetSize.Height());       nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=nUnitsPerInch;                       nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x0000;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        nValue=0x0000;                              nCheckSum^=nValue; pWMF->WriteUInt16( nValue );
        pWMF->WriteUInt16( nCheckSum );
    }

    nMetafileHeaderPos=pWMF->Tell();
    pWMF->WriteUInt16( 0x0001 )           // type: file
         .WriteUInt16( 0x0009 )           // header length in words
         .WriteUInt16( 0x0300 )           // Version as BCD number
         .WriteUInt32( 0x00000000 )      // file length (without 1st header), is later corrected by UpdateHeader()
         .WriteUInt16( MAXOBJECTHANDLES ) // maxmimum number of simultaneous objects
         .WriteUInt32( 0x00000000 )      // maximum record length, is later corrected by UpdateHeader()
         .WriteUInt16( 0x0000 );          // reserved
}

void WMFWriter::UpdateHeader()
{
    sal_uLong nPos;
    sal_uInt32 nFileSize;

    nPos=pWMF->Tell();                 // endposition = total size of file
    nFileSize=nPos-nMetafileHeaderPos; // subtract size of 1st header
    if ((nFileSize&1)!=0) {            // if needed round to words
        pWMF->WriteUChar( 0 );
        nPos++;
        nFileSize++;
    }
    nFileSize>>=1;                    // convert to number of words
    pWMF->Seek(nMetafileHeaderPos+6); // to filesize entry in second header
    pWMF->WriteUInt32( nFileSize );               // rectify file size
    pWMF->SeekRel(2);                 // to max-record-length-entry in second header
    pWMF->WriteUInt32( nMaxRecordSize );          // and rectify
    pWMF->Seek(nPos);
}

bool WMFWriter::WriteWMF( const GDIMetaFile& rMTF, SvStream& rTargetStream,
                            FilterConfigItem const * pFConfigItem, bool bPlaceable )
{
    WMFWriterAttrStackMember * pAt;

    bEmbedEMF = true;
    bStatus=true;
    pVirDev = VclPtr<VirtualDevice>::Create();

    if (pFConfigItem)
    {
        xStatusIndicator = pFConfigItem->GetStatusIndicator();
        if ( xStatusIndicator.is() )
        {
            xStatusIndicator->start( OUString(), 100 );
        }
    }
    nLastPercent=0;

    pWMF=&rTargetStream;
    pWMF->SetEndian(SvStreamEndian::LITTLE);

    nMaxRecordSize=0;

    aSrcMapMode=rMTF.GetPrefMapMode();

    if( bPlaceable )
    {
        aTargetMapMode = aSrcMapMode;
        aTargetSize = rMTF.GetPrefSize();
        sal_uInt16 nTargetDivisor = CalcSaveTargetMapMode(aTargetMapMode, aTargetSize);
        aTargetSize.setWidth( aTargetSize.Width() / nTargetDivisor );
        aTargetSize.setHeight( aTargetSize.Height() / nTargetDivisor );
    }
    else
    {
        aTargetMapMode = MapMode( MapUnit::MapInch );

        const long      nUnit = pVirDev->LogicToPixel( Size( 1, 1 ), aTargetMapMode ).Width();
        const Fraction  aFrac( 1, nUnit );

        aTargetMapMode.SetScaleX( aFrac );
        aTargetMapMode.SetScaleY( aFrac );
        aTargetSize = OutputDevice::LogicToLogic( rMTF.GetPrefSize(), aSrcMapMode, aTargetMapMode );
    }

    pVirDev->SetMapMode( aTargetMapMode );

    pAttrStack=nullptr;

    for (bool & rn : bHandleAllocated)
        rn=false;

    nDstPenHandle=0xffff;
    nDstFontHandle=0xffff;
    nDstBrushHandle=0xffff;

    nNumberOfActions=0;
    nNumberOfBitmaps=0;
    nWrittenActions=0;
    nWrittenBitmaps=0;
    nActBitmapPercent=0;

    CountActionsAndBitmaps(rMTF);

    WriteHeader(bPlaceable);
    if( bEmbedEMF )
        WriteEmbeddedEMF( rMTF );
    WMFRecord_SetWindowOrg(Point(0,0));
    WMFRecord_SetWindowExt(rMTF.GetPrefSize());
    WMFRecord_SetBkMode( true );

    eDstROP2 = eSrcRasterOp = RasterOp::OverPaint;
    WMFRecord_SetROP2(eDstROP2);

    aDstLineInfo = LineInfo();
    aDstLineColor = aSrcLineColor = COL_BLACK;
    CreateSelectDeletePen( aDstLineColor, aDstLineInfo );

    aDstFillColor = aSrcFillColor = COL_WHITE;
    CreateSelectDeleteBrush( aDstFillColor );

    aDstClipRegion = aSrcClipRegion = vcl::Region();

    vcl::Font aFont;
    aFont.SetCharSet( GetExtendedTextEncoding( RTL_TEXTENCODING_MS_1252 ) );
    aFont.SetColor( COL_WHITE );
    aFont.SetAlignment( ALIGN_BASELINE );
    aDstFont = aSrcFont = aFont;
    CreateSelectDeleteFont(aDstFont);

    eDstTextAlign = eSrcTextAlign = ALIGN_BASELINE;
    eDstHorTextAlign = eSrcHorTextAlign = W_TA_LEFT;
    WMFRecord_SetTextAlign( eDstTextAlign, eDstHorTextAlign );

    aDstTextColor = aSrcTextColor = COL_WHITE;
    WMFRecord_SetTextColor(aDstTextColor);

    // Write records
    WriteRecords(rMTF);

    WriteRecordHeader(0x00000003,0x0000); // end of file
    UpdateHeader();

    while(pAttrStack)
    {
        pAt=pAttrStack;
        pAttrStack=pAt->pSucc;
        delete pAt;
    }

    pVirDev.disposeAndClear();

    if ( xStatusIndicator.is() )
        xStatusIndicator->end();

    return bStatus;
}

sal_uInt16 WMFWriter::CalcSaveTargetMapMode(MapMode& rMapMode,
                                        const Size& rPrefSize)
{
    Fraction    aDivFrac(2, 1);
    sal_uInt16      nDivisor = 1;

    Size aSize = OutputDevice::LogicToLogic( rPrefSize, aSrcMapMode, rMapMode );

    while( nDivisor <= 64 && (aSize.Width() > 32767 || aSize.Height() > 32767) )
    {
        Fraction aFrac = rMapMode.GetScaleX();

        aFrac *= aDivFrac;
        rMapMode.SetScaleX(aFrac);
        aFrac = rMapMode.GetScaleY();
        aFrac *= aDivFrac;
        rMapMode.SetScaleY(aFrac);
        nDivisor <<= 1;
        aSize = OutputDevice::LogicToLogic( rPrefSize, aSrcMapMode, rMapMode );
    }

    return nDivisor;
}

void WMFWriter::WriteEmbeddedEMF( const GDIMetaFile& rMTF )
{
    SvMemoryStream aStream;
    EMFWriter aEMFWriter(aStream);

    if( !aEMFWriter.WriteEMF( rMTF ) )
        return;

    sal_uInt64 const nTotalSize = aStream.Tell();
    if( nTotalSize > SAL_MAX_UINT32 )
        return;
    aStream.Seek( 0 );
    sal_uInt32 nRemainingSize = static_cast< sal_uInt32 >( nTotalSize );
    sal_uInt32 nRecCounts = ( (nTotalSize - 1) / 0x2000 ) + 1;
    sal_uInt16 nCheckSum = 0, nWord;

    sal_uInt32 nPos = 0;

    while( nPos + 1 < nTotalSize )
    {
        aStream.ReadUInt16( nWord );
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

void WMFWriter::WriteEMFRecord( SvMemoryStream& rStream, sal_uInt32 nCurSize, sal_uInt32 nRemainingSize,
                sal_uInt32 nTotalSize, sal_uInt32 nRecCounts, sal_uInt16 nCheckSum )
{
   // according to http://msdn.microsoft.com/en-us/library/dd366152%28PROT.13%29.aspx
   WriteRecordHeader( 0, W_META_ESCAPE );
   pWMF->WriteUInt16( W_MFCOMMENT )         // same as META_ESCAPE_ENHANCED_METAFILE
         .WriteUInt16( nCurSize + 34 )     // we will always have a 34 byte escape header:
         .WriteUInt32( 0x43464D57 )        // WMFC
         .WriteUInt32( 0x00000001 )        // Comment type
         .WriteUInt32( 0x00010000 )        // version
         .WriteUInt16( nCheckSum )                      // check sum
         .WriteUInt32( 0 )                 // flags = 0
         .WriteUInt32( nRecCounts )                     // total number of records
         .WriteUInt32( nCurSize )                       // size of this record's data
         .WriteUInt32( nRemainingSize )                 // remaining size of data in following records, missing in MSDN documentation
         .WriteUInt32( nTotalSize );                    // total size of EMF stream

   pWMF->WriteBytes(static_cast<const sal_Char*>(rStream.GetData()) + rStream.Tell(), nCurSize);
   rStream.SeekRel( nCurSize );
   UpdateRecordHeader();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
