/*************************************************************************
 *
 *  $RCSfile: wmfwr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2000-09-27 12:03:56 $
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

#include <vcl/salbtype.hxx>
#include "wmfwr.hxx"

//====================== MS-Windows-defines ===============================

#define W_META_SETBKCOLOR           0x0201
#define W_META_SETBKMODE            0x0102
#define W_META_SETMAPMODE           0x0103
#define W_META_SETROP2              0x0104
#define W_META_SETRELABS            0x0105
#define W_META_SETPOLYFILLMODE      0x0106
#define W_META_SETSTRETCHBLTMODE    0x0107
#define W_META_SETTEXTCHAREXTRA     0x0108
#define W_META_SETTEXTCOLOR         0x0209
#define W_META_SETTEXTJUSTIFICATION 0x020A
#define W_META_SETWINDOWORG         0x020B
#define W_META_SETWINDOWEXT         0x020C
#define W_META_SETVIEWPORTORG       0x020D
#define W_META_SETVIEWPORTEXT       0x020E
#define W_META_OFFSETWINDOWORG      0x020F
#define W_META_SCALEWINDOWEXT       0x0410
#define W_META_OFFSETVIEWPORTORG    0x0211
#define W_META_SCALEVIEWPORTEXT     0x0412
#define W_META_LINETO               0x0213
#define W_META_MOVETO               0x0214
#define W_META_EXCLUDECLIPRECT      0x0415
#define W_META_INTERSECTCLIPRECT    0x0416
#define W_META_ARC                  0x0817
#define W_META_ELLIPSE              0x0418
#define W_META_FLOODFILL            0x0419
#define W_META_PIE                  0x081A
#define W_META_RECTANGLE            0x041B
#define W_META_ROUNDRECT            0x061C
#define W_META_PATBLT               0x061D
#define W_META_SAVEDC               0x001E
#define W_META_SETPIXEL             0x041F
#define W_META_OFFSETCLIPRGN        0x0220
#define W_META_TEXTOUT              0x0521
#define W_META_BITBLT               0x0922
#define W_META_STRETCHBLT           0x0B23
#define W_META_POLYGON              0x0324
#define W_META_POLYLINE             0x0325
#define W_META_ESCAPE               0x0626
#define W_META_RESTOREDC            0x0127
#define W_META_FILLREGION           0x0228
#define W_META_FRAMEREGION          0x0429
#define W_META_INVERTREGION         0x012A
#define W_META_PAINTREGION          0x012B
#define W_META_SELECTCLIPREGION     0x012C
#define W_META_SELECTOBJECT         0x012D
#define W_META_SETTEXTALIGN         0x012E
#define W_META_DRAWTEXT             0x062F
#define W_META_CHORD                0x0830
#define W_META_SETMAPPERFLAGS       0x0231
#define W_META_EXTTEXTOUT           0x0a32
#define W_META_SETDIBTODEV          0x0d33
#define W_META_SELECTPALETTE        0x0234
#define W_META_REALIZEPALETTE       0x0035
#define W_META_ANIMATEPALETTE       0x0436
#define W_META_SETPALENTRIES        0x0037
#define W_META_POLYPOLYGON          0x0538
#define W_META_RESIZEPALETTE        0x0139
#define W_META_DIBBITBLT            0x0940
#define W_META_DIBSTRETCHBLT        0x0b41
#define W_META_DIBCREATEPATTERNBRUSH 0x0142
#define W_META_STRETCHDIB           0x0f43
#define W_META_EXTFLOODFILL         0x0548
#define W_META_RESETDC              0x014C
#define W_META_STARTDOC             0x014D
#define W_META_STARTPAGE            0x004F
#define W_META_ENDPAGE              0x0050
#define W_META_ABORTDOC             0x0052
#define W_META_ENDDOC               0x005E
#define W_META_DELETEOBJECT         0x01f0
#define W_META_CREATEPALETTE        0x00f7
#define W_META_CREATEBRUSH          0x00F8
#define W_META_CREATEPATTERNBRUSH   0x01F9
#define W_META_CREATEPENINDIRECT    0x02FA
#define W_META_CREATEFONTINDIRECT   0x02FB
#define W_META_CREATEBRUSHINDIRECT  0x02FC
#define W_META_CREATEBITMAPINDIRECT 0x02FD
#define W_META_CREATEBITMAP         0x06FE
#define W_META_CREATEREGION         0x06FF

#define W_TRANSPARENT     1
#define W_OPAQUE          2

#define W_R2_BLACK            1
#define W_R2_NOTMERGEPEN      2
#define W_R2_MASKNOTPEN       3
#define W_R2_NOTCOPYPEN       4
#define W_R2_MASKPENNOT       5
#define W_R2_NOT              6
#define W_R2_XORPEN           7
#define W_R2_NOTMASKPEN       8
#define W_R2_MASKPEN          9
#define W_R2_NOTXORPEN        10
#define W_R2_NOP              11
#define W_R2_MERGENOTPEN      12
#define W_R2_COPYPEN          13
#define W_R2_MERGEPENNOT      14
#define W_R2_MERGEPEN         15
#define W_R2_WHITE            16

#define W_TA_NOUPDATECP      0x0000
#define W_TA_UPDATECP        0x0001
#define W_TA_LEFT            0x0000
#define W_TA_RIGHT           0x0002
#define W_TA_CENTER          0x0006
#define W_TA_TOP             0x0000
#define W_TA_BOTTOM          0x0008
#define W_TA_BASELINE        0x0018

#define W_SRCCOPY             0x00CC0020L
#define W_SRCPAINT            0x00EE0086L
#define W_SRCAND              0x008800C6L
#define W_SRCINVERT           0x00660046L
#define W_SRCERASE            0x00440328L
#define W_NOTSRCCOPY          0x00330008L
#define W_NOTSRCERASE         0x001100A6L
#define W_MERGECOPY           0x00C000CAL
#define W_MERGEPAINT          0x00BB0226L
#define W_PATCOPY             0x00F00021L
#define W_PATPAINT            0x00FB0A09L
#define W_PATINVERT           0x005A0049L
#define W_DSTINVERT           0x00550009L
#define W_BLACKNESS           0x00000042L
#define W_WHITENESS           0x00FF0062L

#define W_PS_SOLID            0
#define W_PS_DASH             1
#define W_PS_DOT              2
#define W_PS_DASHDOT          3
#define W_PS_DASHDOTDOT       4
#define W_PS_NULL             5
#define W_PS_INSIDEFRAME      6

#define W_LF_FACESIZE       32

#define W_ANSI_CHARSET          0
#define W_DEFAULT_CHARSET       1
#define W_SYMBOL_CHARSET        2
#define W_SHIFTJIS_CHARSET    128
#define W_HANGEUL_CHARSET     129
#define W_CHINESEBIG5_CHARSET 136
#define W_OEM_CHARSET         255

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
#define W_FW_EXTRALIGHT     200
#define W_FW_LIGHT          300
#define W_FW_NORMAL         400
#define W_FW_MEDIUM         500
#define W_FW_SEMIBOLD       600
#define W_FW_BOLD           700
#define W_FW_EXTRABOLD      800
#define W_FW_HEAVY          900
#define W_FW_ULTRALIGHT     200
#define W_FW_REGULAR        400
#define W_FW_DEMIBOLD       600
#define W_FW_ULTRABOLD      800
#define W_FW_BLACK          900

#define W_BS_SOLID          0
#define W_BS_HOLLOW         1
#define W_BS_HATCHED        2
#define W_BS_PATTERN        3
#define W_BS_INDEXED        4
#define W_BS_DIBPATTERN     5

#define W_HS_HORIZONTAL     0
#define W_HS_VERTICAL       1
#define W_HS_FDIAGONAL      2
#define W_HS_BDIAGONAL      3
#define W_HS_CROSS          4
#define W_HS_DIAGCROSS      5

//========================== Methoden von WMFWriter ==========================

void WMFWriter::MayCallback()
{
    ULONG nPercent;

    // Wir gehen mal einfach so davon aus, dass 16386 Actions einer Bitmap entsprechen
    // (in der Regel wird ein Metafile entweder nur Actions oder einige Bitmaps und fast
    // keine Actions enthalten. Dann ist das Verhaeltnis ziemlich unwichtig)

    nPercent=((nWrittenBitmaps<<14)+(nActBitmapPercent<<14)/100+nWrittenActions)
             *100
             /((nNumberOfBitmaps<<14)+nNumberOfActions);

    if (nPercent>=nLastPercent+3) {
        nLastPercent=nPercent;
        if(pCallback!=NULL && nPercent<=100) {
            if (((*pCallback)(pCallerData,(USHORT)nPercent))==TRUE) bStatus=FALSE;
        }
    }
}


void WMFWriter::CountActionsAndBitmaps( const GDIMetaFile & rMTF )
{
    ULONG nAction, nActionCount;

    nActionCount = rMTF.GetActionCount();

    for ( nAction=0; nAction<nActionCount; nAction++ )
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
    *pWMF << (BYTE) rColor.GetRed() << (BYTE) rColor.GetGreen() << (BYTE) rColor.GetBlue() << (BYTE) 0;
}


void WMFWriter::WriteRecordHeader(ULONG nSizeWords, USHORT nType)
{
    nActRecordPos=pWMF->Tell();
    if (nSizeWords>nMaxRecordSize) nMaxRecordSize=nSizeWords;
    *pWMF << nSizeWords << nType;
}


void WMFWriter::UpdateRecordHeader()
{
    ULONG nPos,nSize;

    nPos=pWMF->Tell(); nSize=nPos-nActRecordPos;
    if ((nSize & 1)!=0) {
        *pWMF << (BYTE)0;
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
        *pWMF << (UINT16) W_BS_HOLLOW;
    else
        *pWMF << (UINT16) W_BS_SOLID;

    WriteColor( rColor );
    *pWMF << (UINT16) 0;
}


void WMFWriter::WMFRecord_CreateFontIndirect(const Font & rFont)
{
    USHORT nWeight,i;
    BYTE nCharSet,nPitchFamily;

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

    if (rFont.GetItalic()==ITALIC_NONE)       *pWMF << (BYTE)0; else  *pWMF << (BYTE)1;
    if (rFont.GetUnderline()==UNDERLINE_NONE) *pWMF << (BYTE)0; else  *pWMF << (BYTE)1;
    if (rFont.GetStrikeout()==STRIKEOUT_NONE) *pWMF << (BYTE)0; else  *pWMF << (BYTE)1;

    switch (rFont.GetCharSet())
    {
        case RTL_TEXTENCODING_SYMBOL :  nCharSet = W_SYMBOL_CHARSET; break;
        case RTL_TEXTENCODING_MS_932 :  nCharSet = 0x3c; break;
        default:                        nCharSet = W_ANSI_CHARSET;
    }
    *pWMF << nCharSet;

    *pWMF << (BYTE)0 << (BYTE)0 << (BYTE)0;

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

    ByteString aFontName( rFont.GetName(), gsl_getSystemTextEncoding() );
    for ( i = 0; i < W_LF_FACESIZE; i++ )
    {
        sal_Char nChar = ( i < aFontName.Len() ) ? aFontName.GetChar( i ) : 0;
        *pWMF << nChar;
    }
    UpdateRecordHeader();
}


void WMFWriter::WMFRecord_CreatePenIndirect(const Color& rColor)
{
    WriteRecordHeader(0x00000008,W_META_CREATEPENINDIRECT);

    if( rColor == Color( COL_TRANSPARENT ) )
        *pWMF << (UINT16) W_PS_NULL;
    else
        *pWMF << (UINT16) W_PS_SOLID;

    WriteSize( Size() );
    WriteColor(rColor);
}

void WMFWriter::WMFRecord_CreatePenIndirect(const Color& rColor, const LineInfo& rLineInfo )
{
    WriteRecordHeader(0x00000008,W_META_CREATEPENINDIRECT);

    USHORT nStyle = W_PS_SOLID;
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
                    if ( !rLineInfo.GetDotCount() == 1 )
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
    }
    *pWMF << nStyle;

    WriteSize( Size( rLineInfo.GetWidth(), 0 ) );
    WriteColor( rColor );
}

void WMFWriter::WMFRecord_DeleteObject(USHORT nObjectHandle)
{
    WriteRecordHeader(0x00000004,W_META_DELETEOBJECT);
    *pWMF << nObjectHandle;
}


void WMFWriter::WMFRecord_Ellipse(const Rectangle & rRect)
{
    WriteRecordHeader(0x00000007,W_META_ELLIPSE);
    WriteRectangle(rRect);
}


void WMFWriter::WMFRecord_ExtTextOut( const Point & rPoint, const String & rString, const long * pDXAry )
{
    sal_uInt16  nOriginalTextLen, nNewTextLen, i;
    sal_Int32   j;
    nOriginalTextLen = rString.Len();

    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
    if ( eChrSet == RTL_TEXTENCODING_DONTKNOW )
        eChrSet = gsl_getSystemTextEncoding();
    ByteString aByteString( rString, eChrSet );

    if ( ( nOriginalTextLen <= 1 ) || ( pDXAry == NULL ) )
    {
        WMFRecord_TextOut( rPoint, aByteString );
        return;
    }
    WriteRecordHeader( 0, W_META_EXTTEXTOUT );
    WritePointYX( rPoint );
    nNewTextLen = aByteString.Len();
    *pWMF << nNewTextLen << (sal_uInt16)0;

    for ( i = 0; i < nNewTextLen; i++ )
        *pWMF << (sal_uInt8)aByteString.GetChar( i );
    if ( nNewTextLen & 1 )
        *pWMF << (sal_uInt8)0;

    sal_Int16* pConvertedDXAry = new sal_Int16[ nOriginalTextLen ];
    j = 0;
    pConvertedDXAry[ j++ ] = (sal_Int16)ScaleWidth( pDXAry[ 0 ] );
    for ( i = 1; i < ( nOriginalTextLen - 1 ); i++ )
        pConvertedDXAry[ j++ ] = (sal_Int16)ScaleWidth( pDXAry[ i ] - pDXAry[ i - 1 ] );
    pConvertedDXAry[ j ] = (sal_Int16)ScaleWidth( pDXAry[ nOriginalTextLen - 2 ] / ( nOriginalTextLen - 1 ) );

    for ( i = 0; i < nOriginalTextLen; i++ )
    {
        sal_Int16 nDx = pConvertedDXAry[ i ];
        *pWMF << nDx;
        if ( nOriginalTextLen < nNewTextLen )
        {
            ByteString aTemp( rString.GetChar( i ), eChrSet );
            j = aTemp.Len();
            while ( --j > 0 )
                *pWMF << (sal_uInt16)0;
        }
    }
    delete pConvertedDXAry;
    UpdateRecordHeader();
}


void WMFWriter::WMFRecord_ExtTextOut( const Point & rPoint, const String & rString, ULONG nWidth )
{
    USHORT nLen,i;
    long * pDXAry;
    sal_Int32   nNormSize;

    pVirDev->SetFont( aSrcFont );
    nLen = rString.Len();
    pDXAry = new long[ nLen ];
    nNormSize = pVirDev->GetTextArray( rString, pDXAry );
    if ( nLen <= 1 || nNormSize ==(long)nWidth )
        WMFRecord_ExtTextOut( rPoint, rString, (const long*)NULL );
    else
    {
        for ( i = 0; i < ( nLen - 1 ); i++ )
            pDXAry[ i ] = pDXAry[ i ] * ( (long)nWidth ) / nNormSize;
        WMFRecord_ExtTextOut( rPoint, rString, pDXAry );
    }
    delete pDXAry;
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
    USHORT nSize,i;
    nSize=rPoly.GetSize();

    WriteRecordHeader(((ULONG)nSize)*2+4,W_META_POLYGON);
    *pWMF << nSize;
    for (i=0; i<nSize; i++) WritePointXY(rPoly.GetPoint(i));
}


void WMFWriter::WMFRecord_PolyLine(const Polygon & rPoly)
{
    USHORT nSize,i;
    nSize=rPoly.GetSize();

    WriteRecordHeader(((ULONG)nSize)*2+4,W_META_POLYLINE);
    *pWMF << nSize;
    for (i=0; i<nSize; i++) WritePointXY(rPoly.GetPoint(i));
}


void WMFWriter::WMFRecord_PolyPolygon(const PolyPolygon & rPolyPoly)
{
    const Polygon * pPoly;
    USHORT nCount,nSize,i,j;

    WriteRecordHeader(0,W_META_POLYPOLYGON);
    nCount=rPolyPoly.Count();
    *pWMF << nCount;
    for (i=0; i<nCount; i++) *pWMF << ((USHORT)(rPolyPoly.GetObject(i).GetSize()));
    for (i=0; i<nCount; i++) {
        pPoly=&(rPolyPoly.GetObject(i));
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


void WMFWriter::WMFRecord_SelectObject(USHORT nObjectHandle)
{
    WriteRecordHeader(0x00000004,W_META_SELECTOBJECT);
    *pWMF << nObjectHandle;
}


void WMFWriter::WMFRecord_SetBkColor(const Color & rColor)
{
    WriteRecordHeader(0x00000005,W_META_SETBKCOLOR);
    WriteColor(rColor);
}


void WMFWriter::WMFRecord_SetBkMode(BOOL bTransparent)
{
    WriteRecordHeader(0x00000004,W_META_SETBKMODE);
    if (bTransparent==TRUE) *pWMF << (USHORT)W_TRANSPARENT;
    else                    *pWMF << (USHORT)W_OPAQUE;
}

void WMFWriter::WMFRecord_SetStretchBltMode()
{
    WriteRecordHeader( 0x00000004, W_META_SETSTRETCHBLTMODE );
    *pWMF << (USHORT) 3; // STRETCH_DELETESCANS
}

void WMFWriter::WMFRecord_SetPixel(const Point & rPoint, const Color & rColor)
{
    WriteRecordHeader(0x00000007,W_META_SETPIXEL);
    WriteColor(rColor);
    WritePointYX(rPoint);
}


void WMFWriter::WMFRecord_SetROP2(RasterOp eROP)
{
    USHORT nROP2;

    switch (eROP) {
        case ROP_INVERT: nROP2=W_R2_NOT;        break;
        case ROP_XOR:    nROP2=W_R2_XORPEN;     break;
        default:         nROP2=W_R2_COPYPEN;
    }
    WriteRecordHeader(0x00000004,W_META_SETROP2);
    *pWMF << nROP2;
}


void WMFWriter::WMFRecord_SetTextAlign(FontAlign eFontAlign)
{
    USHORT nAlign;

    switch (eFontAlign) {
        case ALIGN_TOP:    nAlign=W_TA_TOP; break;
        case ALIGN_BOTTOM: nAlign=W_TA_BOTTOM; break;
        default:           nAlign=W_TA_BASELINE;
    }
    nAlign|=W_TA_LEFT;
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
                                      const Bitmap & rBitmap, ULONG nROP )
{
    ULONG nPosAnf,nPosEnd;

    nActBitmapPercent=50;
    MayCallback();

    WriteRecordHeader(0x00000000,W_META_STRETCHDIB);

    // Die Reihenfolge im Metafile soll jetzt sein:
    // einige Parameter (laenge 22), dann die Bitmap ohne FILEHEADER.
    // Da aber *pWMF << rBitmap einen FILEHEADER der Laenge 14
    // erzeugt, schreiben wir zuerst die Bitmap an die richtige Position
    // Und ueberschreiben hinterher den FILEHEADER mit den Parametern.
    nPosAnf=pWMF->Tell(); // Position merken, wo Parameter hin sollen
    *pWMF << (long)0 << (long)0; // 8 bytes auffuellen (diese 8 bytes +
                                 // 14 bytes ueberfluessigen FILEHEADER
                                 // = 22 bytes Parameter)
    *pWMF << rBitmap; // Bitmap schreiben

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


void WMFWriter::WMFRecord_TextOut(const Point & rPoint, const ByteString & rString)
{
    USHORT nLen,i;

    WriteRecordHeader(0,W_META_TEXTOUT);
    nLen=rString.Len();
    *pWMF << nLen;
    for ( i = 0; i < nLen; i++ )
        *pWMF << (BYTE)rString.GetChar( i );
    if ((nLen&1)!=0) *pWMF << (BYTE)0;
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


USHORT WMFWriter::AllocHandle()
{
    USHORT i;

    for (i=0; i<MAXOBJECTHANDLES; i++) {
        if (bHandleAllocated[i]==FALSE) {
            bHandleAllocated[i]=TRUE;
            return i;
        }
    }
    bStatus=FALSE;
    return 0xffff;
}


void WMFWriter::FreeHandle(USHORT nObjectHandle)
{
    if (nObjectHandle<MAXOBJECTHANDLES) bHandleAllocated[nObjectHandle]=FALSE;
}


void WMFWriter::CreateSelectDeletePen(const Color& rColor)
{
    USHORT nOldHandle;

    nOldHandle=nDstPenHandle;
    nDstPenHandle=AllocHandle();
    WMFRecord_CreatePenIndirect(rColor);
    WMFRecord_SelectObject(nDstPenHandle);
    if (nOldHandle<MAXOBJECTHANDLES) {
        WMFRecord_DeleteObject(nOldHandle);
        FreeHandle(nOldHandle);
    }
}


void WMFWriter::CreateSelectDeleteFont(const Font & rFont)
{
    USHORT nOldHandle;

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
    USHORT nOldHandle;

    nOldHandle=nDstBrushHandle;
    nDstBrushHandle=AllocHandle();
    WMFRecord_CreateBrushIndirect(rColor);
    WMFRecord_SelectObject(nDstBrushHandle);
    if (nOldHandle<MAXOBJECTHANDLES) {
        WMFRecord_DeleteObject(nOldHandle);
        FreeHandle(nOldHandle);
    }
}


void WMFWriter::SetAttrForLines()
{

    if (bAttrReadyForLines==TRUE) return;

    if (bDstGraphicsInvalid || eDstROP2!=eSrcRasterOp) {
        eDstROP2=eSrcRasterOp;
        WMFRecord_SetROP2(eDstROP2);
    }
    if (bDstGraphicsInvalid || aDstLineColor!=aSrcLineColor) {
        aDstLineColor=aSrcLineColor;
        CreateSelectDeletePen(aDstLineColor);
    }
    if (bDstGraphicsInvalid || aDstFillColor!=aSrcFillColor) {
        aDstFillColor=aSrcFillColor;
        CreateSelectDeleteBrush(aDstFillColor);
    }
    if (bDstGraphicsInvalid || bDstIsClipping!=bSrcIsClipping ||
        (bSrcIsClipping==TRUE && aDstClipRegion!=aSrcClipRegion)) {
        bDstIsClipping=bSrcIsClipping;
        aDstClipRegion=aSrcClipRegion;
        if (bDstGraphicsInvalid==FALSE || bSrcIsClipping==TRUE) {
            //...???...
        }
    }

    bAttrReadyForLines=TRUE;
    bAttrReadyForAreas=FALSE;
    bAttrReadyForText=FALSE;
    bDstGraphicsInvalid=FALSE;
}


void WMFWriter::SetAttrForAreas()
{
    if( aSrcFillColor == Color( COL_TRANSPARENT ) )
    {
        SetAttrForLines();
        return;
    }

    if (bAttrReadyForAreas==TRUE) return;

    if (bDstGraphicsInvalid || eDstROP2!=eSrcRasterOp) {
        eDstROP2=eSrcRasterOp;
        WMFRecord_SetROP2(eDstROP2);
    }
    if (bDstGraphicsInvalid || aDstLineColor!=aSrcLineColor) {
        aDstLineColor=aSrcLineColor;
        CreateSelectDeletePen(aDstLineColor);
    }
    if (bDstGraphicsInvalid || aDstFillColor!=aSrcFillColor) {
        aDstFillColor=aSrcFillColor;
        CreateSelectDeleteBrush(aDstFillColor);
    }
    if (bDstGraphicsInvalid || bDstIsClipping!=bSrcIsClipping ||
        (bSrcIsClipping==TRUE && aDstClipRegion!=aSrcClipRegion)) {
        bDstIsClipping=bSrcIsClipping;
        aDstClipRegion=aSrcClipRegion;
        if (bDstGraphicsInvalid==FALSE || bSrcIsClipping==TRUE) {
            //...???...
        }
    }

    bAttrReadyForAreas=TRUE;
    bAttrReadyForLines=FALSE;
    bAttrReadyForText=FALSE;
    bDstGraphicsInvalid=FALSE;
}


void WMFWriter::SetAttrForText()
{
    if (bAttrReadyForText==TRUE) return;

    if (bDstGraphicsInvalid==FALSE) SetAttrForLines();

    if (bDstTextInvalid || aDstTextColor!=aSrcFont.GetColor()) {
        aDstTextColor=aSrcFont.GetColor();
        WMFRecord_SetTextColor(aDstTextColor);
    }
    if (bDstTextInvalid || eDstTextAlign!=aSrcFont.GetAlign()) {
        eDstTextAlign=aSrcFont.GetAlign();
        WMFRecord_SetTextAlign(eDstTextAlign);
    }
    if (bDstTextInvalid || aDstFont!=aSrcFont) {
        aDstFont=aSrcFont;
        CreateSelectDeleteFont(aDstFont);
    }

    bAttrReadyForText=TRUE;
    bAttrReadyForLines=FALSE;
    bAttrReadyForAreas=FALSE;
    bDstTextInvalid=FALSE;
}


void WMFWriter::WriteRecords( const GDIMetaFile & rMTF )
{
    ULONG       nA, nACount;
    MetaAction* pMA;

    if( bStatus )
    {
        nACount = rMTF.GetActionCount();

        WMFRecord_SetStretchBltMode();

        for( nA=0; nA<nACount; nA++ )
        {
            pMA = rMTF.GetAction( nA );

            switch( pMA->GetType() )
            {
                case META_PIXEL_ACTION:
                {
                    const MetaPixelAction* pA = (const MetaPixelAction *) pMA;
                    SetAttrForLines();
                    WMFRecord_SetPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case META_POINT_ACTION:
                {
                    const MetaPointAction*  pA = (const MetaPointAction*) pMA;
                    const Point&            rPt = pA->GetPoint();
                    SetAttrForLines();
                    WMFRecord_MoveTo( rPt);
                    WMFRecord_LineTo( rPt );
                }
                break;

                case META_LINE_ACTION:
                {
                    const MetaLineAction* pA = (const MetaLineAction *) pMA;
                    const LineInfo& rLineInfo = pA->GetLineInfo();
                    SetAttrForLines();
                    if ( rLineInfo.IsDefault() )
                    {
                        WMFRecord_MoveTo( pA->GetStartPoint() );
                        WMFRecord_LineTo( pA->GetEndPoint() );
                    }
                    else
                    {
                        USHORT nCurrentHandle = AllocHandle();
                        WMFRecord_CreatePenIndirect( aSrcLineColor, rLineInfo );
                        WMFRecord_SelectObject(nCurrentHandle);
                        WMFRecord_MoveTo( pA->GetStartPoint() );
                        WMFRecord_LineTo( pA->GetEndPoint() );
                        WMFRecord_DeleteObject(nCurrentHandle);
                        FreeHandle(nCurrentHandle);
                        WMFRecord_SelectObject(nDstPenHandle);
                    }
                }
                break;

                case META_RECT_ACTION:
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_Rectangle( pA->GetRect() );
                }
                break;

                case META_ROUNDRECT_ACTION:
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_RoundRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case META_ELLIPSE_ACTION:
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_Ellipse( pA->GetRect() );
                }
                break;

                case META_ARC_ACTION:
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pMA;
                    SetAttrForLines();
                    WMFRecord_Arc( pA->GetRect(),pA->GetStartPoint(),pA->GetEndPoint() );
                }
                break;

                case META_PIE_ACTION:
                {
                    const MetaPieAction* pA = (const MetaPieAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_Pie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;


                case META_CHORD_ACTION:
                {
                    const MetaChordAction* pA = (const MetaChordAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_Chord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case META_POLYLINE_ACTION:
                {
                    const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pMA;
                    const LineInfo& rLineInfo = pA->GetLineInfo();
                    SetAttrForLines();

                    if ( rLineInfo.IsDefault() )
                        WMFRecord_PolyLine( pA->GetPolygon() );
                    else
                    {
                        USHORT nCurrentHandle = AllocHandle();
                        WMFRecord_CreatePenIndirect( aSrcLineColor, rLineInfo );
                        WMFRecord_SelectObject(nCurrentHandle);
                        WMFRecord_PolyLine( pA->GetPolygon() );
                        WMFRecord_DeleteObject(nCurrentHandle);
                        FreeHandle(nCurrentHandle);
                        WMFRecord_SelectObject(nDstPenHandle);
                    }
                }
                break;

                case META_POLYGON_ACTION:
                {
                    const MetaPolygonAction* pA = (const MetaPolygonAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_Polygon( pA->GetPolygon() );
                }
                break;

                case META_POLYPOLYGON_ACTION:
                {
                    const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pMA;
                    SetAttrForAreas();
                    WMFRecord_PolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case META_TEXT_ACTION:
                case META_TEXTRECT_ACTION:
                {
                    const MetaTextAction * pA = (const MetaTextAction*) pMA;
                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    rtl_TextEncoding eChrSet = aSrcFont.GetCharSet();
                    if ( eChrSet == RTL_TEXTENCODING_DONTKNOW )
                        eChrSet = gsl_getSystemTextEncoding();
                    ByteString aStr( aTemp, eChrSet );
                    SetAttrForText();
                    WMFRecord_TextOut( pA->GetPoint(), aStr );
                }
                break;

                case META_TEXTARRAY_ACTION:
                {
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pMA;

                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    SetAttrForText();
                    WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pA->GetDXArray() );
                }
                break;

                case META_STRETCHTEXT_ACTION:
                {
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction *) pMA;
                    String aTemp( pA->GetText(), pA->GetIndex(), pA->GetLen() );
                    SetAttrForText();
                    WMFRecord_ExtTextOut( pA->GetPoint(), aTemp, pA->GetWidth() );
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
                    bAttrReadyForLines=FALSE;
                    bAttrReadyForAreas=FALSE;
                    SetAttrForAreas();
                    WMFRecord_Rectangle( pA->GetRect() );
                    aSrcLineColor = aOldLineColor;
                    aSrcFillColor = aOldFillColor;
                    bAttrReadyForLines=FALSE;
                    bAttrReadyForAreas=FALSE;
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

                    bAttrReadyForLines=FALSE;
                    bAttrReadyForAreas=FALSE;
                }
                break;

                case META_FILLCOLOR_ACTION:
                {
                    const MetaFillColorAction* pA = (const MetaFillColorAction*) pMA;

                    if( pA->IsSetting() )
                        aSrcFillColor = pA->GetColor();
                    else
                        aSrcFillColor = Color( COL_TRANSPARENT );

                    bAttrReadyForLines=FALSE;
                    bAttrReadyForAreas=FALSE;
                }
                break;

                case META_TEXTCOLOR_ACTION:
                {
                    const MetaTextColorAction* pA = (const MetaTextColorAction*) pMA;
                    aSrcFont.SetColor( pA->GetColor() );
                    bAttrReadyForText = FALSE;
                }
                break;

                case META_TEXTFILLCOLOR_ACTION:
                {
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pMA;

                    if( pA->IsSetting() )
                        aSrcFont.SetFillColor( pA->GetColor() );
                    else
                        aSrcFont.SetFillColor( Color( COL_TRANSPARENT ) );

                    bAttrReadyForText = FALSE;
                }
                break;

                case META_TEXTALIGN_ACTION:
                {
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pMA;
                    WMFRecord_SetTextAlign( pA->GetTextAlign() );
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

                        bAttrReadyForLines=FALSE;
                        bAttrReadyForAreas=FALSE;
                        bAttrReadyForText=FALSE;
                        bDstGraphicsInvalid=TRUE;
                        bDstTextInvalid=TRUE;
                    }
                }
                break;

                case META_FONT_ACTION:
                {
                    const MetaFontAction* pA = (const MetaFontAction*) pMA;
                    aSrcFont=pA->GetFont();
                    bAttrReadyForText=FALSE;
                }
                break;

                case META_PUSH_ACTION:
                {
                    WMFWriterAttrStackMember* pAt = new WMFWriterAttrStackMember;

                    pAt->aLineColor=aSrcLineColor;
                    pAt->aFillColor=aSrcFillColor;
                    pAt->eRasterOp=eSrcRasterOp;
                    pAt->aFont=aSrcFont;
                    pAt->aMapMode=aSrcMapMode;
                    pAt->aClipRegion=aSrcClipRegion;
                    pAt->pSucc=pAttrStack;
                    pAttrStack=pAt;

                    // Fuer das SaveDC muessen wir ggf. alle Objekte
                    // sofort selektieren, damit beim RestoreDC im ::Pop
                    // alles wieder richtig restauriert wird
                    SetAttrForLines();
                    SetAttrForAreas();
                    SetAttrForText();

                    // Das machen wir nur, um die ClipRegion nach einem evtl.
                    // IntersectClipRect wieder zuruecksetzen zu koennen
                    WMFRecord_SaveDC();
                }
                break;

                case META_POP_ACTION:
                {
                    WMFWriterAttrStackMember * pAt=pAttrStack;

                    if( pAt )
                    {
                        aSrcLineColor=pAt->aLineColor;
                        aSrcFillColor=pAt->aFillColor;
                        eSrcRasterOp=pAt->eRasterOp;
                        aSrcFont=pAt->aFont;
                        aSrcMapMode=pAt->aMapMode;
                        aSrcClipRegion=pAt->aClipRegion;
                        pAttrStack=pAt->pSucc;
                        delete pAt;

                        WMFRecord_RestoreDC();

                        bAttrReadyForLines=FALSE;
                        bAttrReadyForAreas=FALSE;
                        bAttrReadyForText=FALSE;
                        bDstGraphicsInvalid=TRUE;
                    }
                }
                break;

                case META_EPS_ACTION :
                {
                    const MetaEPSAction* pA = (const MetaEPSAction*)pMA;
                    const GDIMetaFile aGDIMetaFile( pA->GetSubstitute() );

                    INT32 nCount = aGDIMetaFile.GetActionCount();
                    for ( INT32 i = 0; i < nCount; i++ )
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
                    bAttrReadyForLines=FALSE;
                    bAttrReadyForAreas=FALSE;
                    bAttrReadyForText=FALSE;
                }
                break;

                case META_TRANSPARENT_ACTION:
                {
                    SetAttrForAreas();
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

                    SetAttrForLines();
                    SetAttrForAreas();
                    SetAttrForText();

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

                // Unsupported Actions
                case META_MASK_ACTION:
                case META_MASKSCALE_ACTION:
                case META_MASKSCALEPART_ACTION:
                {
                    DBG_ERROR( "Unsupported action: MetaMask...Action!" );
                }
                break;

                case META_CLIPREGION_ACTION:
                break;

                case META_ISECTREGIONCLIPREGION_ACTION:
                {
                    DBG_ERROR( "Unsupported action: MetaISectRegionClipRegionAction!" );
                }
                break;

                case META_MOVECLIPREGION_ACTION:
                {
                    DBG_ERROR( "Unsupported action: MetaMoveClipRegionAction!" );
                }
                break;
          }

          nWrittenActions++;
          MayCallback();

          if (pWMF->GetError())
            bStatus=FALSE;

          if(bStatus==FALSE)
            break;
        }
    }
}

// ------------------------------------------------------------------------

void WMFWriter::WriteHeader( const GDIMetaFile & rMTF, BOOL bPlaceable )
{
    if( bPlaceable )
    {
        USHORT  nCheckSum, nValue;
        Size    aSize( pVirDev->LogicToLogic(Size(1,1),MapMode(MAP_INCH), aTargetMapMode) );
        USHORT  nUnitsPerInch = (USHORT) ( ( aSize.Width() + aSize.Height() ) >> 1 );

        nCheckSum=0;
        nValue=0xcdd7;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x9ac6;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=(USHORT) aTargetSize.Width();        nCheckSum^=nValue; *pWMF << nValue;
        nValue=(USHORT) aTargetSize.Height();       nCheckSum^=nValue; *pWMF << nValue;
        nValue=nUnitsPerInch;                       nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        nValue=0x0000;                              nCheckSum^=nValue; *pWMF << nValue;
        *pWMF << nCheckSum;
    }

    nMetafileHeaderPos=pWMF->Tell();
    *pWMF << (USHORT)0x0001           // Typ: Datei
          << (USHORT)0x0009           // Headerlaenge in Worten
          << (USHORT)0x0300           // Version als BCD-Zahl
          << (ULONG) 0x00000000       // Dateilaenge (ohne 1. Header), wird spaeter durch UpdateHeader() berichtigt
          << (USHORT)MAXOBJECTHANDLES // Maximalezahl der gleichzeitigen Objekte
          << (ULONG) 0x00000000       // Maximale Record-laenge, wird spaeter durch UpdateHeader() berichtigt
          << (USHORT)0x0000;          // Reserved
}

// ------------------------------------------------------------------------

void WMFWriter::UpdateHeader()
{
    ULONG nPos,nFileSize;

    nPos=pWMF->Tell();                 // Endposition = Gesammtgroesse der Datei
    nFileSize=nPos-nMetafileHeaderPos; // Groesse des 1. Headers abziehen
    if ((nFileSize&1)!=0) {            // ggf. auf ganze Worte aufrunden
        *pWMF << (BYTE)0;
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

BOOL WMFWriter::WriteWMF(const GDIMetaFile& rMTF, SvStream& rTargetStream,
                         PFilterCallback pcallback, void* pcallerdata,
                         BOOL bPlaceable)
{
    WMFWriterAttrStackMember * pAt;

    bStatus=TRUE;
    pVirDev = new VirtualDevice;
    pCallback=pcallback;
    pCallerData=pcallerdata;
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

    aSrcLineColor = Color( COL_BLACK );
    aSrcFillColor = Color( COL_WHITE );
    eSrcRasterOp=ROP_OVERPAINT;
    aSrcFont=Font();
    bSrcIsClipping=FALSE;
    aSrcClipRegion=Region();
    pAttrStack=NULL;

    aDstLineColor = Color( COL_BLACK );
    aDstFillColor = Color( COL_WHITE );
    eDstROP2=ROP_OVERPAINT;
    aDstTextColor=Color(COL_BLACK);
    eDstTextAlign=ALIGN_BASELINE;
    aDstFont=Font();
    bDstIsClipping=FALSE;
    aDstClipRegion=Region();

    for (USHORT i=0; i<MAXOBJECTHANDLES; i++)
        bHandleAllocated[i]=FALSE;

    nDstPenHandle=0xffff;
    nDstFontHandle=0xffff;
    nDstBrushHandle=0xffff;

    bDstTextInvalid=TRUE;
    bDstGraphicsInvalid=TRUE;

    bAttrReadyForLines=FALSE;
    bAttrReadyForAreas=FALSE;
    bAttrReadyForText=FALSE;

    nNumberOfActions=0;
    nNumberOfBitmaps=0;
    nWrittenActions=0;
    nWrittenBitmaps=0;
    nActBitmapPercent=0;

    CountActionsAndBitmaps(rMTF);

    WriteHeader(rMTF,bPlaceable);
    WMFRecord_SetWindowOrg(Point(0,0));
    WMFRecord_SetWindowExt(rMTF.GetPrefSize());
    WMFRecord_SetBkMode( TRUE );

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

    return bStatus;
}

// ------------------------------------------------------------------------

USHORT WMFWriter::CalcSaveTargetMapMode(MapMode& rMapMode,
                                        const Size& rPrefSize)
{
    Fraction    aDivFrac(2, 1);
    USHORT      nDivisor = 1;

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
