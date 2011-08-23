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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include "winmtf.hxx"
#include <rtl/crc.h>
#include <rtl/tencinfo.h>
#include <osl/endian.h>

namespace binfilter
{


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

//=================== Methoden von WMFReader ==============================

inline Point WMFReader::ReadPoint()
{
    short nX, nY;
    *pWMF >> nX >> nY;
    return Point( nX, nY );
}

// ------------------------------------------------------------------------

inline Point WMFReader::ReadYX()
{
    short nX, nY;
    *pWMF >> nY >> nX;
    return Point( nX, nY );
}

// ------------------------------------------------------------------------

Rectangle WMFReader::ReadRectangle()
{
    Point aBR, aTL;
    aBR = ReadYX();
    aTL = ReadYX();
    aBR.X()--;
    aBR.Y()--;
    return Rectangle( aTL, aBR );
}

// ------------------------------------------------------------------------

Size WMFReader::ReadYXExt()
{
    short nW, nH;
    *pWMF >> nH >> nW;
    return Size( nW, nH );
}

// ------------------------------------------------------------------------

void WMFReader::ReadRecordParams( USHORT nFunc )
{
    switch( nFunc )
    {
        case W_META_SETBKCOLOR:
        {
            pOut->SetBkColor( ReadColor() );
        }
        break;

        case W_META_SETBKMODE:
        {
            USHORT nDat;
            *pWMF >> nDat;
            pOut->SetBkMode( nDat );
        }
        break;

        // !!!
        case W_META_SETMAPMODE:
        {
            sal_Int16 nMapMode;
            *pWMF >> nMapMode;
            pOut->SetMapMode( nMapMode );
        }
        break;

        case W_META_SETROP2:
        {
            UINT16 nROP2;
            *pWMF >> nROP2;
            pOut->SetRasterOp( nROP2 );
        }
        break;

        case W_META_SETTEXTCOLOR:
        {
            pOut->SetTextColor( ReadColor() );
        }
        break;

        case W_META_SETWINDOWORG:
        {
            pOut->SetWinOrg( ReadYX() );
        }
        break;

        case W_META_SETWINDOWEXT:
        {
            short nWidth, nHeight;
            *pWMF >> nHeight >> nWidth;
            pOut->SetWinExt( Size( nWidth, nHeight ) );
        }
        break;

        case W_META_OFFSETWINDOWORG:
        {
            short nXAdd, nYAdd;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetWinOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEWINDOWEXT:
        {
            short nXNum, nXDenom, nYNum, nYDenom;
            *pWMF >> nYDenom >> nYNum >> nXDenom >> nXNum;
            pOut->ScaleWinExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
        }
        break;

        case W_META_SETVIEWPORTORG:
        case W_META_SETVIEWPORTEXT:
        break;

        case W_META_OFFSETVIEWPORTORG:
        {
            short nXAdd, nYAdd;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetDevOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEVIEWPORTEXT:
        {
            short nXNum, nXDenom, nYNum, nYDenom;
            *pWMF >> nYDenom >> nYNum >> nXDenom >> nXNum;
            pOut->ScaleDevExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
        }
        break;

        case W_META_LINETO:
        {
            pOut->LineTo( ReadYX() );
        }
        break;

        case W_META_MOVETO:
        {
            pOut->MoveTo( ReadYX() );
        }
        break;

        case W_META_INTERSECTCLIPRECT:
        {
            pOut->IntersectClipRect( ReadRectangle() );
        }
        break;

        case W_META_RECTANGLE:
        {
            pOut->DrawRect( ReadRectangle() );
        }
        break;

        case W_META_ROUNDRECT:
        {
            Size aSize( ReadYXExt() );
            pOut->DrawRoundRect( ReadRectangle(), aSize );
        }
        break;

        case W_META_ELLIPSE:
        {
            pOut->DrawEllipse( ReadRectangle() );
        }
        break;

        case W_META_ARC:
        {
            Point aEnd( ReadYX() );
            Point aStart( ReadYX() );
            pOut->DrawArc( ReadRectangle(), aStart, aEnd );
        }
        break;

        case W_META_PIE:
        {
            Point     aEnd( ReadYX() );
            Point     aStart( ReadYX() );
            Rectangle aRect( ReadRectangle() );

            // #i73608# OutputDevice deviates from WMF
            // semantics. start==end means full ellipse here.
            if( aStart == aEnd )
                pOut->DrawEllipse( aRect );
            else
                pOut->DrawPie( aRect, aStart, aEnd );
        }
        break;

        case W_META_CHORD:
        {
            Point aEnd( ReadYX() );
            Point aStart( ReadYX() );
            pOut->DrawChord( ReadRectangle(), aStart, aEnd );
        }
        break;

        case W_META_POLYGON:
        {
            USHORT i,nPoints;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for( i = 0; i < nPoints; i++ )
                aPoly[ i ] = ReadPoint();
            pOut->DrawPolygon( aPoly );
        }
        break;

        case W_META_POLYPOLYGON:
        {
            USHORT	i, nPoly, nPoints;
            USHORT*	pnPoints;
            Point*	pPtAry;
            // Anzahl der Polygone:
            *pWMF >> nPoly;
            // Anzahl der Punkte eines jeden Polygons holen, Gesammtzahl der Punkte ermitteln:
            pnPoints = new USHORT[ nPoly ];
            nPoints = 0;
            for( i = 0; i < nPoly; i++ )
            {
                *pWMF >> pnPoints[i];
                nPoints = nPoints + pnPoints[i];
            }
            // Polygonpunkte holen:
            pPtAry  = (Point*) new char[ nPoints * sizeof(Point) ];
            for ( i = 0; i < nPoints; i++ )
                pPtAry[ i ] = ReadPoint();
            // PolyPolygon Actions erzeugen
            PolyPolygon aPolyPoly( nPoly, pnPoints, pPtAry );
            pOut->DrawPolyPolygon( aPolyPoly );
            delete[] (char*) pPtAry;
            delete[] pnPoints;
        }
        break;

        case W_META_POLYLINE:
        {
            USHORT i,nPoints;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for( i = 0; i < nPoints; i++ )
                aPoly[ i ] = ReadPoint();
            pOut->DrawPolyLine( aPoly );
        }
        break;

        case W_META_SAVEDC:
        {
            pOut->Push();
        }
        break;

        case W_META_RESTOREDC:
        {
            pOut->Pop();
        }
        break;

        case W_META_SETPIXEL:
        {
            const Color aColor = ReadColor();
            pOut->DrawPixel( ReadYX(), aColor );
        }
        break;

        case W_META_OFFSETCLIPRGN:
        {
            pOut->MoveClipRegion( ReadYXExt() );
        }
        break;

        case W_META_TEXTOUT:
        {
            USHORT nLength;
            *pWMF >> nLength;
            if ( nLength )
            {
                char*	pChar = new char[ ( nLength + 1 ) &~ 1 ];
                pWMF->Read( pChar, ( nLength + 1 ) &~ 1 );
                String aText( pChar, nLength, pOut->GetCharSet() );
                delete[] pChar;
                Point aPosition( ReadYX() );
                pOut->DrawText( aPosition, aText );
            }
        }
        break;

        case W_META_EXTTEXTOUT:
        {
            sal_Int16	nDx, nDxTmp;
            sal_uInt16	i, nLen, nOptions;
            sal_Int32	nRecordPos, nRecordSize, nOriginalTextLen, nNewTextLen;
            Point		aPosition;
            Rectangle	aRect;
            sal_Int32*	pDXAry = NULL;

            pWMF->SeekRel(-6);
            nRecordPos = pWMF->Tell();
            *pWMF >> nRecordSize;
            pWMF->SeekRel(2);
            aPosition = ReadYX();
            *pWMF >> nLen >> nOptions;

            sal_Int32 nTextLayoutMode = TEXT_LAYOUT_DEFAULT;
            if ( nOptions & ETO_RTLREADING )
                nTextLayoutMode = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT;
            pOut->SetTextLayoutMode( nTextLayoutMode );
            DBG_ASSERT( ( nOptions & ( ETO_PDY | ETO_GLYPH_INDEX ) ) == 0, "SJ: ETO_PDY || ETO_GLYPH_INDEX in WMF" );

            // Nur wenn der Text auch Zeichen enthaelt, macht die Ausgabe Sinn
            if( nLen )
            {
                nOriginalTextLen = nLen;
                if( nOptions & ETO_CLIPPED )
                {
                    const Point aPt1( ReadPoint() );
                    const Point aPt2( ReadPoint() );
                    aRect = Rectangle( aPt1, aPt2 );
                }
                char* pChar = new char[ ( nOriginalTextLen + 1 ) &~ 1 ];
                pWMF->Read( pChar, ( nOriginalTextLen + 1 ) &~ 1 );
                String aText( pChar, (sal_uInt16)nOriginalTextLen, pOut->GetCharSet() );// after this conversion the text may contain
                nNewTextLen = aText.Len();										        // less character (japanese version), so the
                delete[] pChar;													        // dxAry will not fit

                if ( nNewTextLen )
                {
                    sal_uInt32	nMaxStreamPos = nRecordPos + ( nRecordSize << 1 );
                    sal_Int32	nDxArySize =  nMaxStreamPos - pWMF->Tell();
                    sal_Int32	nDxAryEntries = nDxArySize >> 1;
                    sal_Bool	bUseDXAry = FALSE;

                    if ( ( ( nDxAryEntries % nOriginalTextLen ) == 0 ) && ( nNewTextLen <= nOriginalTextLen ) )
                    {
                        pDXAry = new sal_Int32[ nNewTextLen ];
                        for ( i = 0; i < nNewTextLen; i++ )
                        {
                            if ( pWMF->Tell() >= nMaxStreamPos )
                                break;
                            *pWMF >> nDx;
                            if ( nNewTextLen != nOriginalTextLen )
                            {
                                ByteString aTmp( aText.GetChar( i ), pOut->GetCharSet() );
                                if ( aTmp.Len() > 1 )
                                {
                                    sal_Int32 nDxCount = aTmp.Len() - 1;
                                    if ( ( ( nDxCount * 2 ) + pWMF->Tell() ) > nMaxStreamPos )
                                        break;
                                    while ( nDxCount-- )
                                    {
                                        *pWMF >> nDxTmp;
                                        nDx = nDx + nDxTmp;
                                    }
                                }							
                            }
                            pDXAry[ i ] = nDx;
                        }
                        if ( i == nNewTextLen )
                            bUseDXAry = TRUE;
                    }
                    if ( pDXAry && bUseDXAry )
                        pOut->DrawText( aPosition, aText, pDXAry );
                    else
                        pOut->DrawText( aPosition, aText );
                }
            }
            delete[] pDXAry;

        }
        break;

        case W_META_SELECTOBJECT:
        {
            INT16	nObjIndex;
            *pWMF >> nObjIndex;
            pOut->SelectObject( nObjIndex );
        }
        break;

        case W_META_SETTEXTALIGN:
        {
            UINT16	nAlign;
            *pWMF >> nAlign;
            pOut->SetTextAlign( nAlign );
        }
        break;

        case W_META_BITBLT:
        {
            // 0-3   : nWinROP						#93454#
            // 4-5   : y offset of source bitmap
            // 6-7   : x offset of source bitmap
            // 8-9   : used height of source bitmap
            // 10-11 : used width  of source bitmap
            // 12-13 : destination position y (in pixel)
            // 14-15 : destination position x (in pixel)
            // 16-17 : dont know
            // 18-19 : Width Bitmap in Pixel
            // 20-21 : Height Bitmap in Pixel
            // 22-23 : bytes per scanline
            // 24    : planes
            // 25	 : bitcount

            sal_Int32	nWinROP;
            sal_uInt16	nSx, nSy, nSxe, nSye, nDontKnow, nWidth, nHeight, nBytesPerScan;
            sal_uInt8	nPlanes, nBitCount;

            *pWMF >> nWinROP
                  >> nSy >> nSx >> nSye >> nSxe;
            Point aPoint( ReadYX() );
            *pWMF >> nDontKnow >> nWidth >> nHeight >> nBytesPerScan >> nPlanes >> nBitCount;

            if ( nWidth && nHeight && ( nPlanes == 1 ) && ( nBitCount == 1 ) )
            {
                Bitmap aBmp( Size( nWidth, nHeight ), nBitCount );
                BitmapWriteAccess* pAcc;
                pAcc = aBmp.AcquireWriteAccess();
                if ( pAcc )
                {
                    sal_uInt16 y, x, scan;
                    sal_Int8 i, nEightPixels;
                    for ( y = 0; y < nHeight; y++ )
                    {
                        x = 0;
                        for ( scan = 0; scan < nBytesPerScan; scan++ )
                        {
                            *pWMF >> nEightPixels;
                            for ( i = 7; i >= 0; i-- )
                            {
                                if ( x < nWidth )
                                {
                                    pAcc->SetPixel( y, x, (nEightPixels>>i)&1 );
                                }
                                x++;
                            }
                        }
                    }
                    aBmp.ReleaseAccess( pAcc );
                    if ( nSye && nSxe && 
                        ( ( nSx + nSxe ) <= aBmp.GetSizePixel().Width() ) &&
                            ( ( nSy + nSye <= aBmp.GetSizePixel().Height() ) ) )
                    {
                        Rectangle aCropRect( Point( nSx, nSy ), Size( nSxe, nSye ) );
                        aBmp.Crop( aCropRect );
                    }
                    Rectangle aDestRect( aPoint, Size( nSxe, nSye ) );
                    aBmpSaveList.Insert( new BSaveStruct( aBmp, aDestRect, nWinROP ), LIST_APPEND );
                }
            }
        }		
        break;

        case W_META_STRETCHBLT:
        case W_META_DIBBITBLT:
        case W_META_DIBSTRETCHBLT:
        case W_META_STRETCHDIB:
        {
            sal_Int32   nWinROP;
            sal_uInt16  nSx, nSy, nSxe, nSye, nUsage;
            Bitmap		aBmp;

            *pWMF >> nWinROP;

            if( nFunc == W_META_STRETCHDIB )
                *pWMF >> nUsage;
            
            // nSye and nSxe is the number of pixels that has to been used
            if( nFunc == W_META_STRETCHDIB || nFunc == W_META_STRETCHBLT || nFunc == W_META_DIBSTRETCHBLT )
                *pWMF >> nSye >> nSxe;
            else
                nSye = nSxe = 0;    // set this to zero as indicator not to scale the bitmap later

            // nSy and nx is the offset of the first pixel
            *pWMF >> nSy >> nSx;

            if( nFunc == W_META_STRETCHDIB || nFunc == W_META_DIBBITBLT || nFunc == W_META_DIBSTRETCHBLT )
            {
                if ( nWinROP == PATCOPY )
                    *pWMF >> nUsage;	// i don't know anything of this parameter, so its called nUsage
                                        // pOut->DrawRect( Rectangle( ReadYX(), aDestSize ), FALSE );

                Size aDestSize( ReadYXExt() );
                if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps 
                {
                    Rectangle aDestRect( ReadYX(), aDestSize );
                    if ( nWinROP != PATCOPY )
                        aBmp.Read( *pWMF, FALSE );

                    // test if it is sensible to crop
                    if ( nSye && nSxe && 
                        ( ( nSx + nSxe ) <= aBmp.GetSizePixel().Width() ) &&
                            ( ( nSy + nSye <= aBmp.GetSizePixel().Height() ) ) )
                    {
                        Rectangle aCropRect( Point( nSx, nSy ), Size( nSxe, nSye ) );
                        aBmp.Crop( aCropRect );
                    }
                    aBmpSaveList.Insert( new BSaveStruct( aBmp, aDestRect, nWinROP ), LIST_APPEND );
                }
            }
        }
        break;

        case W_META_DIBCREATEPATTERNBRUSH:
        {
            Bitmap	aBmp;
            BitmapReadAccess* pBmp;
            UINT32	nRed = 0, nGreen = 0, nBlue = 0, nCount = 1;
            UINT16	nFunction;

            *pWMF >> nFunction >> nFunction;

            aBmp.Read( *pWMF, FALSE );
            pBmp = aBmp.AcquireReadAccess();
            if ( pBmp )
            {
                for ( INT32 y = 0; y < pBmp->Height(); y++ )
                {
                    for ( INT32 x = 0; x < pBmp->Width(); x++ )
                    {
                        const BitmapColor aColor( pBmp->GetColor( y, x ) );						
                        
                        nRed += aColor.GetRed();
                        nGreen += aColor.GetGreen();
                        nBlue += aColor.GetBlue();
                    }
                }
                nCount = pBmp->Height() * pBmp->Width();
                if ( !nCount )
                    nCount++;
                aBmp.ReleaseAccess( pBmp );
            }
            Color aColor( (BYTE)( nRed / nCount ), (BYTE)( nGreen / nCount ), (BYTE)( nBlue / nCount ) );
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( aColor, FALSE ) );
        }
        break;

        case W_META_DELETEOBJECT:
        {
            INT16 nIndex;
            *pWMF >> nIndex;
            pOut->DeleteObject( nIndex );
        }
        break;

        case W_META_CREATEPALETTE:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEBRUSH:
        {
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), FALSE ) );
        }
        break;

        case W_META_CREATEPATTERNBRUSH:
        {
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), FALSE ) );
        }
        break;

        case W_META_CREATEPENINDIRECT:
        {
            LineInfo	aLineInfo;
            USHORT		nStyle, nWidth, nHeight;

            *pWMF >> nStyle >> nWidth >> nHeight;

            if ( nWidth )
                aLineInfo.SetWidth( nWidth );

            BOOL bTransparent = FALSE;
            UINT16 nDashCount = 0;
            UINT16 nDotCount = 0;
            switch( nStyle )
            {
                case PS_DASHDOTDOT :
                    nDotCount++;
                case PS_DASHDOT :
                    nDashCount++;
                case PS_DOT :
                    nDotCount++;
                break;
                case PS_DASH :
                    nDashCount++;
                break;
                case PS_NULL :
                    bTransparent = TRUE;
                    aLineInfo.SetStyle( LINE_NONE );
                break;
                default :
                case PS_INSIDEFRAME :
                case PS_SOLID :
                    aLineInfo.SetStyle( LINE_SOLID );
            }
            if ( nDashCount | nDotCount )
            {
                aLineInfo.SetStyle( LINE_DASH );
                aLineInfo.SetDashCount( nDashCount );
                aLineInfo.SetDotCount( nDotCount );
            }
            pOut->CreateObject( GDI_PEN, new WinMtfLineStyle( ReadColor(), aLineInfo, bTransparent ) );
        }
        break;

        case W_META_CREATEBRUSHINDIRECT:
        {
            USHORT	nStyle;
            *pWMF >> nStyle;
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? TRUE : FALSE ) );
        }
        break;

        case W_META_CREATEFONTINDIRECT:
        {
            Size	aFontSize;
            char	lfFaceName[ LF_FACESIZE ];
            INT16	lfEscapement, lfOrientation, lfWeight;	// ( ehemals USHORT )

            LOGFONTW aLogFont;
            aFontSize = ReadYXExt();
            *pWMF >> lfEscapement >> lfOrientation >> lfWeight
                    >> aLogFont.lfItalic >> aLogFont.lfUnderline >> aLogFont.lfStrikeOut >> aLogFont.lfCharSet >> aLogFont.lfOutPrecision
                        >> aLogFont.lfClipPrecision >> aLogFont.lfQuality >> aLogFont.lfPitchAndFamily;
            pWMF->Read( lfFaceName, LF_FACESIZE );
            aLogFont.lfWidth = aFontSize.Width();
            aLogFont.lfHeight = aFontSize.Height();
            aLogFont.lfEscapement = lfEscapement;
            aLogFont.lfOrientation = lfOrientation;
            aLogFont.lfWeight = lfWeight;

            CharSet eCharSet;
            if ( ( aLogFont.lfCharSet == OEM_CHARSET ) || ( aLogFont.lfCharSet == DEFAULT_CHARSET ) )
                eCharSet = gsl_getSystemTextEncoding();
            else
                eCharSet = rtl_getTextEncodingFromWindowsCharset( aLogFont.lfCharSet );
            if ( eCharSet == RTL_TEXTENCODING_DONTKNOW )
                eCharSet = gsl_getSystemTextEncoding();
            if ( eCharSet == RTL_TEXTENCODING_SYMBOL )
                eCharSet = RTL_TEXTENCODING_MS_1252;
            aLogFont.alfFaceName = UniString( lfFaceName, eCharSet );

            pOut->CreateObject( GDI_FONT, new WinMtfFontStyle( aLogFont ) );
        }
        break;

        case W_META_CREATEBITMAPINDIRECT:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEBITMAP:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_CREATEREGION:
        {
            pOut->CreateObject( GDI_DUMMY );
        }
        break;

        case W_META_EXCLUDECLIPRECT :
        {
            pOut->ExcludeClipRect( ReadRectangle() );
        }
        break;

        case W_META_PATBLT:
        {
            UINT32 nROP, nOldROP;
            *pWMF >> nROP;
            Size aSize = ReadYXExt();
            nOldROP = pOut->SetRasterOp( nROP );
            pOut->DrawRect( Rectangle( ReadYX(), aSize ), FALSE );
            pOut->SetRasterOp( nOldROP );
        }
        break;

        case W_META_SELECTCLIPREGION:
        {
            sal_Int16 nObjIndex;
            *pWMF >> nObjIndex;
            if ( !nObjIndex )
            {
                PolyPolygon aEmptyPolyPoly;
                pOut->SetClipPath( aEmptyPolyPoly, RGN_COPY, sal_True );
            }
        }
        break;

        case W_META_ESCAPE :
        {
            if ( nRecSize >= 12 )	// minimal escape lenght
            {
                sal_uInt16	nMode, nLen, OO;
                sal_uInt32  Magic, nCheck,nEsc;
                *pWMF >> nMode
                      >> nLen
                      >> OO
                      >> Magic
                      >> nCheck
                      >> nEsc;
                if ( ( nMode == W_MFCOMMENT ) && ( nLen >= 14 ) && ( OO == 0x4f4f ) && ( Magic == 0xa2c2a ) )
                {
                    sal_uInt32 nEscLen = nLen - 14;
                    if ( nEscLen <= ( nRecSize * 2 ) )
                    {
#ifdef OSL_BIGENDIAN
                        sal_uInt32 nTmp = SWAPLONG( nEsc );
                        sal_uInt32 nCheckSum = rtl_crc32( 0, &nTmp, 4 );
#else
                        sal_uInt32 nCheckSum = rtl_crc32( 0, &nEsc, 4 );
#endif
                        sal_Int8* pData = NULL;
                        if ( nEscLen )
                        {
                            pData = new sal_Int8[ nEscLen ];
                            pWMF->Read( pData, nEscLen );
                            nCheckSum = rtl_crc32( nCheckSum, pData, nEscLen );
                        }
                        if ( nCheck == nCheckSum )
                        {
                            switch( nEsc )
                            {
                                case PRIVATE_ESCAPE_UNICODE :
                                {	// we will use text instead of polygons only if we have the correct font
                                    if ( aVDev.IsFontAvailable( pOut->GetFont().GetName() ) )
                                    {
                                        Point  aPt;
                                        String aString;
                                        sal_uInt32	i, nStringLen, nDXCount;
                                        sal_Int32* pDXAry = NULL;
                                        SvMemoryStream aMemoryStream( nEscLen );
                                        aMemoryStream.Write( pData, nEscLen );
                                        aMemoryStream.Seek( STREAM_SEEK_TO_BEGIN );
                                        aMemoryStream >> aPt.X()
                                                      >> aPt.Y()
                                                      >> nStringLen;

                                        if (nStringLen < STRING_MAXLEN)
                                        {
                                            sal_Unicode* pBuf = aString.AllocBuffer( (xub_StrLen)nStringLen );
                                            for ( i = 0; i < nStringLen; i++ )
                                                aMemoryStream >> pBuf[ i ];
                                            aMemoryStream >> nDXCount;
                                            if ( nDXCount )
                                                pDXAry = new sal_Int32[ nDXCount ];
                                            for  ( i = 0; i < nDXCount; i++ )
                                                aMemoryStream >> pDXAry[ i ];
                                            aMemoryStream >> nSkipActions;
                                            pOut->DrawText( aPt, aString, pDXAry );
                                            delete[] pDXAry;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        delete[] pData;
                    }
                }
            }
        }
        break;

        case W_META_SETRELABS:
        case W_META_SETPOLYFILLMODE:
        case W_META_SETSTRETCHBLTMODE:
        case W_META_SETTEXTCHAREXTRA:
        case W_META_SETTEXTJUSTIFICATION:
        case W_META_FLOODFILL :
        case W_META_FILLREGION:
        case W_META_FRAMEREGION:
        case W_META_INVERTREGION:
        case W_META_PAINTREGION:
        case W_META_DRAWTEXT:
        case W_META_SETMAPPERFLAGS:
        case W_META_SETDIBTODEV:
        case W_META_SELECTPALETTE:
        case W_META_REALIZEPALETTE:
        case W_META_ANIMATEPALETTE:
        case W_META_SETPALENTRIES:
        case W_META_RESIZEPALETTE:
        case W_META_EXTFLOODFILL:
        case W_META_RESETDC:
        case W_META_STARTDOC:
        case W_META_STARTPAGE:
        case W_META_ENDPAGE:
        case W_META_ABORTDOC:
        case W_META_ENDDOC:
        break;
    }
}

// ------------------------------------------------------------------------

BOOL WMFReader::ReadHeader()
{
    Rectangle	aPlaceableBound;
    sal_uInt32  nl, nStrmPos = pWMF->Tell();

    // Einlesen des METAFILEHEADER, falls vorhanden
    *pWMF >> nl;

    Size aWMFSize;
    if ( nl == 0x9ac6cdd7L )
    {
        INT16 nVal;

        // hmf (Unused) ueberlesen wir
        pWMF->SeekRel(2);

        // BoundRect
        *pWMF >> nVal; aPlaceableBound.Left() = nVal;
        *pWMF >> nVal; aPlaceableBound.Top() = nVal;
        *pWMF >> nVal; aPlaceableBound.Right() = nVal;
        *pWMF >> nVal; aPlaceableBound.Bottom() = nVal;

        // inch
        *pWMF >> nUnitsPerInch;

        // reserved
        pWMF->SeekRel( 4 );

        // checksum  pruefen wir lieber nicht
        pWMF->SeekRel( 2 );
    }
    else
    {
        nUnitsPerInch = 96;       
        pWMF->Seek( nStrmPos + 18 );    // set the streampos to the start of the the metaactions
        GetPlaceableBound( aPlaceableBound, pWMF );
        pWMF->Seek( nStrmPos );
    }

    pOut->SetWinOrg( aPlaceableBound.TopLeft() );
    aWMFSize = Size( labs( aPlaceableBound.GetWidth() ), labs( aPlaceableBound.GetHeight() ) );
    pOut->SetWinExt( aWMFSize );

    Size aDevExt( 10000, 10000 );
    if( ( labs( aWMFSize.Width() ) > 1 ) && ( labs( aWMFSize.Height() ) > 1 ) )
    {
        const Fraction	aFrac( 1, nUnitsPerInch );
        MapMode			aWMFMap( MAP_INCH, Point(), aFrac, aFrac );
        Size			aSize100( OutputDevice::LogicToLogic( aWMFSize, aWMFMap, MAP_100TH_MM ) );
        aDevExt = Size( labs( aSize100.Width() ), labs( aSize100.Height() ) );
    }
    pOut->SetDevExt( aDevExt );

    // Einlesen des METAHEADER
    *pWMF >> nl; // Typ und Headergroesse

    if( nl != 0x00090001 )
    {
        pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
        return FALSE;
    }

    pWMF->SeekRel( 2 ); // Version (von Windows)
    pWMF->SeekRel( 4 ); // Size (der Datei in Words)
    pWMF->SeekRel( 2 ); // NoObjects (Maximale Anzahl der gleichzeitigen Objekte)
    pWMF->SeekRel( 4 ); // MaxRecord (Groesse des groessten Records in Words)
    pWMF->SeekRel( 2 ); // NoParameters (Unused

    return TRUE;
}

void WMFReader::ReadWMF()
{
    USHORT	nFunction;
    ULONG	nPos, nPercent, nLastPercent;
    
    nSkipActions = 0;
    nCurrentAction = 0;
    nUnicodeEscapeAction = 0;

    pOut->SetMapMode( MM_ANISOTROPIC );
    pOut->SetWinOrg( Point() );
    pOut->SetWinExt( Size( 1, 1 ) );
    pOut->SetDevExt( Size( 10000, 10000 ) );

    nEndPos=pWMF->Seek( STREAM_SEEK_TO_END );
    pWMF->Seek( nStartPos );
    Callback( (USHORT) ( nLastPercent = 0 ) );

    if ( ReadHeader() )
    {

        nPos = pWMF->Tell();

        if( nEndPos - nStartPos )
        {
            while( TRUE )
            {
                nCurrentAction++;
                nPercent = ( nPos - nStartPos ) * 100 / ( nEndPos - nStartPos );

                if( nLastPercent + 4 <= nPercent )
                {
                    Callback( (USHORT) nPercent );
                    nLastPercent = nPercent;
                }
                *pWMF >> nRecSize >> nFunction;

                if( pWMF->GetError() || ( nRecSize < 3 ) || ( nRecSize==3 && nFunction==0 ) || pWMF->IsEof() )
                {

                    if( pWMF->IsEof() )
                        pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );

                    break;
                }
                if( aBmpSaveList.Count() &&
                    ( nFunction != W_META_STRETCHDIB ) &&
                    ( nFunction != W_META_DIBBITBLT ) &&
                    ( nFunction != W_META_DIBSTRETCHBLT ) )
                {
                    pOut->ResolveBitmapActions( aBmpSaveList );
                }
                if ( !nSkipActions )
                    ReadRecordParams( nFunction );
                else
                    nSkipActions--;
                nPos += nRecSize * 2;
                if ( nPos <= nEndPos )
                    pWMF->Seek( nPos  );
                else
                    pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );

            }
        }
        else
            pWMF->SetError( SVSTREAM_GENERALERROR );

        if( !pWMF->GetError() && aBmpSaveList.Count() )
            pOut->ResolveBitmapActions( aBmpSaveList );
    }
    if ( pWMF->GetError() )
        pWMF->Seek( nStartPos );
}

// ------------------------------------------------------------------------

static void GetWinExtMax( const Point& rSource, Rectangle& rPlaceableBound, const sal_Int16 nMapMode )
{
    Point aSource( rSource );
    if ( nMapMode == MM_HIMETRIC )
        aSource.Y() = -rSource.Y();
    if ( aSource.X() < rPlaceableBound.Left() )
        rPlaceableBound.Left() = aSource.X();
    if ( aSource.X() > rPlaceableBound.Right() )
        rPlaceableBound.Right() = aSource.X();
    if ( aSource.Y() < rPlaceableBound.Top() )
        rPlaceableBound.Top() = aSource.Y();
    if ( aSource.Y() > rPlaceableBound.Bottom() )
        rPlaceableBound.Bottom() = aSource.Y();
}

static void GetWinExtMax( const Rectangle& rSource, Rectangle& rPlaceableBound, const sal_Int16 nMapMode )
{
    GetWinExtMax( rSource.TopLeft(), rPlaceableBound, nMapMode );
    GetWinExtMax( rSource.BottomRight(), rPlaceableBound, nMapMode );
}

sal_Bool WMFReader::GetPlaceableBound( Rectangle& rPlaceableBound, SvStream* pStm )
{
    sal_Bool bRet = sal_True;

    rPlaceableBound.Left()   = (sal_Int32)0x7fffffff;
    rPlaceableBound.Top()    = (sal_Int32)0x7fffffff;
    rPlaceableBound.Right()  = (sal_Int32)0x80000000;
    rPlaceableBound.Bottom() = (sal_Int32)0x80000000;
    
    sal_Int16 nMapMode = MM_ANISOTROPIC;

    sal_uInt16 nFunction;
    sal_uInt32 nRSize;
    sal_uInt32 nPos = pStm->Tell();
    sal_uInt32 nEnd = pStm->Seek( STREAM_SEEK_TO_END );
    
    pStm->Seek( nPos );

    if( nEnd - nPos )
    {
        while( bRet )
        {
            *pStm >> nRSize >> nFunction;

            if( pStm->GetError() || ( nRSize < 3 ) || ( nRSize==3 && nFunction==0 ) || pStm->IsEof() )
            {
                if( pStm->IsEof() )
                {
                    pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    bRet = sal_False;
                }
                break;
            }
            switch( nFunction )
            {
                case W_META_SETWINDOWORG:
                {
                    Point aWinOrg;
                    aWinOrg = ReadYX();
                    rPlaceableBound.SetPos( aWinOrg );
                }
                break;

                case W_META_SETWINDOWEXT:
                {
                    Point aPos0( 0, 0 );
                    sal_Int16 nWidth, nHeight;
                    *pStm >> nHeight >> nWidth;
                    rPlaceableBound.SetSize( Size( nWidth, nHeight ) );
                }
                break;				

                case W_META_SETMAPMODE :
                    *pStm >> nMapMode;
                break;

                case W_META_MOVETO:
                case W_META_LINETO:
                    GetWinExtMax( ReadYX(), rPlaceableBound, nMapMode );
                break;

                case W_META_RECTANGLE:
                case W_META_INTERSECTCLIPRECT:
                case W_META_EXCLUDECLIPRECT :
                case W_META_ELLIPSE:
                    GetWinExtMax( ReadRectangle(), rPlaceableBound, nMapMode );
                break;

                case W_META_ROUNDRECT:
                {
                    Size aSize( ReadYXExt() );
                    GetWinExtMax( ReadRectangle(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_ARC:
                case W_META_PIE:
                case W_META_CHORD:
                {
                    Point aEnd( ReadYX() );
                    Point aStart( ReadYX() );
                    GetWinExtMax( ReadRectangle(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_POLYGON:
                {
                    USHORT i,nPoints;
                    *pStm >> nPoints;
                    for( i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_POLYPOLYGON:
                {
                    USHORT	i, nPoly, nPoints = 0;
                    *pStm >> nPoly;
                    for( i = 0; i < nPoly; i++ )
                    {
                        sal_uInt16 nP;
                        *pStm >> nP;
                        nPoints = nPoints + nP;
                    }
                    for ( i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_POLYLINE:
                {
                    USHORT i,nPoints;
                    *pStm >> nPoints;
                    for( i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_SETPIXEL:
                {
                    const Color aColor = ReadColor();
                    GetWinExtMax( ReadYX(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_TEXTOUT:
                {
                    USHORT nLength;
                    *pStm >> nLength;
                    // todo: we also have to take care of the text width
                    if ( nLength )
                    {
                        pStm->SeekRel( ( nLength + 1 ) &~ 1 );
                        GetWinExtMax( ReadYX(), rPlaceableBound, nMapMode );
                    }
                }
                break;

                case W_META_EXTTEXTOUT:
                {
                    sal_uInt16	nLen, nOptions;
                    sal_Int32	nRecordPos, nRecordSize;
                    Point		aPosition;
                    Rectangle	aRect;

                    pStm->SeekRel(-6);
                    nRecordPos = pStm->Tell();
                    *pStm >> nRecordSize;
                    pStm->SeekRel(2);
                    aPosition = ReadYX();
                    *pStm >> nLen >> nOptions;
                    // todo: we also have to take care of the text width
                    if( nLen )
                        GetWinExtMax( aPosition, rPlaceableBound, nMapMode );
                }
                break;
                case W_META_BITBLT:
                case W_META_STRETCHBLT:
                case W_META_DIBBITBLT:
                case W_META_DIBSTRETCHBLT:
                case W_META_STRETCHDIB:
                {
                    sal_Int32   nWinROP;
                    sal_uInt16  nSx, nSy, nSxe, nSye, nUsage;
                    *pStm >> nWinROP;

                    if( nFunction == W_META_STRETCHDIB )
                        *pStm >> nUsage;
            
                    // nSye and nSxe is the number of pixels that has to been used
                    if( nFunction == W_META_STRETCHDIB || nFunction == W_META_STRETCHBLT || nFunction == W_META_DIBSTRETCHBLT )
                        *pStm >> nSye >> nSxe;
                    else
                        nSye = nSxe = 0;    // set this to zero as indicator not to scale the bitmap later

                    // nSy and nx is the offset of the first pixel
                    *pStm >> nSy >> nSx;

                    if( nFunction == W_META_STRETCHDIB || nFunction == W_META_DIBBITBLT || nFunction == W_META_DIBSTRETCHBLT )
                    {
                        if ( nWinROP == PATCOPY )
                            *pStm >> nUsage;	// i don't know anything of this parameter, so its called nUsage
                                                // pOut->DrawRect( Rectangle( ReadYX(), aDestSize ), FALSE );

                        Size aDestSize( ReadYXExt() );
                        if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps 
                        {
                            Rectangle aDestRect( ReadYX(), aDestSize );
                            GetWinExtMax( aDestRect, rPlaceableBound, nMapMode );
                        }
                    }
                }
                break;

                case W_META_PATBLT:
                {
                    UINT32 nROP;
                    *pStm >> nROP;
                    Size aSize = ReadYXExt();
                    GetWinExtMax( Rectangle( ReadYX(), aSize ), rPlaceableBound, nMapMode );
                }
                break;
            }
            nPos += nRSize * 2;
             if ( nPos <= nEnd )
                 pStm->Seek( nPos );
             else
             {
                 pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                 bRet = sal_False;
             }

        }
    }
    else
    {
        pStm->SetError( SVSTREAM_GENERALERROR );
        bRet = sal_False;
    }
    return bRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
