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


#include "winmtf.hxx"
#include <boost/scoped_array.hpp>
#include <vcl/gdimtf.hxx>
#include <svtools/wmf.hxx>
#include <rtl/crc.h>
#include <rtl/tencinfo.h>
#include <osl/endian.h>

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

//=================== Methods of WMFReader ==============================

inline Point WMFReader::ReadPoint()
{
    short nX = 0, nY = 0;
    *pWMF >> nX >> nY;
    return Point( nX, nY );
}

// ------------------------------------------------------------------------

inline Point WMFReader::ReadYX()
{
    short nX = 0, nY = 0;
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
    short nW=0, nH=0;
    *pWMF >> nH >> nW;
    return Size( nW, nH );
}

// ------------------------------------------------------------------------

void WMFReader::ReadRecordParams( sal_uInt16 nFunc )
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
            sal_uInt16 nDat = 0;
            *pWMF >> nDat;
            pOut->SetBkMode( nDat );
        }
        break;

        // !!!
        case W_META_SETMAPMODE:
        {
            sal_Int16 nMapMode = 0;
            *pWMF >> nMapMode;
            pOut->SetMapMode( nMapMode );
        }
        break;

        case W_META_SETROP2:
        {
            sal_uInt16 nROP2 = 0;
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
            short nWidth = 0, nHeight = 0;
            *pWMF >> nHeight >> nWidth;
            pOut->SetWinExt( Size( nWidth, nHeight ) );
        }
        break;

        case W_META_OFFSETWINDOWORG:
        {
            short nXAdd = 0, nYAdd = 0;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetWinOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEWINDOWEXT:
        {
            short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
            *pWMF >> nYDenom >> nYNum >> nXDenom >> nXNum;
            pOut->ScaleWinExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
        }
        break;

        case W_META_SETVIEWPORTORG:
        case W_META_SETVIEWPORTEXT:
        break;

        case W_META_OFFSETVIEWPORTORG:
        {
            short nXAdd = 0, nYAdd = 0;
            *pWMF >> nYAdd >> nXAdd;
            pOut->SetDevOrgOffset( nXAdd, nYAdd );
        }
        break;

        case W_META_SCALEVIEWPORTEXT:
        {
            short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
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
            pOut->DrawRoundRect( ReadRectangle(), Size( aSize.Width() / 2, aSize.Height() / 2 ) );
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
            Rectangle aRect( ReadRectangle() );
            aRect.Justify();
            pOut->DrawArc( aRect, aStart, aEnd );
        }
        break;

        case W_META_PIE:
        {
            Point     aEnd( ReadYX() );
            Point     aStart( ReadYX() );
            Rectangle aRect( ReadRectangle() );
            aRect.Justify();

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
            Rectangle aRect( ReadRectangle() );
            aRect.Justify();
            pOut->DrawChord( aRect, aStart, aEnd );
        }
        break;

        case W_META_POLYGON:
        {
            sal_uInt16 nPoints = 0;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for( sal_uInt16 i = 0; i < nPoints; i++ )
                aPoly[ i ] = ReadPoint();
            pOut->DrawPolygon( aPoly );
        }
        break;

        case W_META_POLYPOLYGON:
        {
            bool bRecordOk = true;
            sal_uInt16  nPoly = 0;
            Point*  pPtAry;
            // Number of polygons:
            *pWMF >> nPoly;
            // Number of points of each polygon. Determine total number of points
            boost::scoped_array<sal_uInt16> xPolygonPointCounts(new sal_uInt16[nPoly]);
            sal_uInt16* pnPoints = xPolygonPointCounts.get();
            sal_uInt16 nPoints = 0;
            for(sal_uInt16 i = 0; i < nPoly; i++ )
            {
                *pWMF >> pnPoints[i];

                if (pnPoints[i] > SAL_MAX_UINT16 - nPoints)
                {
                    bRecordOk = false;
                    break;
                }

                nPoints += pnPoints[i];
            }

            SAL_WARN_IF(!bRecordOk, "svtools", "polypolygon record has more polygons that we can handle");

            bRecordOk &= pWMF->good();

            if (!bRecordOk)
            {
                pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                break;
            }

            // Polygon points are:
            boost::scoped_array<Point> xPolygonPoints(new Point[nPoints]);
            pPtAry = xPolygonPoints.get();
            for (sal_uInt16 i = 0; i < nPoints; i++ )
                pPtAry[ i ] = ReadPoint();

            bRecordOk &= pWMF->good();

            if (!bRecordOk)
            {
                pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                break;
            }

            // Produce PolyPolygon Actions
            PolyPolygon aPolyPoly( nPoly, pnPoints, pPtAry );
            pOut->DrawPolyPolygon( aPolyPoly );
        }
        break;

        case W_META_POLYLINE:
        {
            sal_uInt16 nPoints = 0;
            *pWMF >> nPoints;
            Polygon aPoly( nPoints );
            for(sal_uInt16 i = 0; i < nPoints; i++ )
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
            sal_uInt16 nLength = 0;
            *pWMF >> nLength;
            if ( nLength )
            {
                char*   pChar = new char[ ( nLength + 1 ) &~ 1 ];
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
            sal_uInt16  nLen = 0, nOptions = 0;
            sal_Int32   nRecordPos, nRecordSize = 0, nOriginalTextLen, nNewTextLen;
            Point       aPosition;
            Rectangle   aRect;
            sal_Int32*  pDXAry = NULL;

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
                nNewTextLen = aText.Len();                                              // less character (japanese version), so the
                delete[] pChar;                                                         // dxAry will not fit

                if ( nNewTextLen )
                {
                    sal_uInt32  nMaxStreamPos = nRecordPos + ( nRecordSize << 1 );
                    sal_Int32   nDxArySize =  nMaxStreamPos - pWMF->Tell();
                    sal_Int32   nDxAryEntries = nDxArySize >> 1;
                    sal_Bool    bUseDXAry = sal_False;

                    if ( ( ( nDxAryEntries % nOriginalTextLen ) == 0 ) && ( nNewTextLen <= nOriginalTextLen ) )
                    {
                        sal_Int16 nDx = 0, nDxTmp = 0;
                        sal_uInt16 i; //needed just outside the for
                        pDXAry = new sal_Int32[ nNewTextLen ];
                        for (i = 0; i < nNewTextLen; i++ )
                        {
                            if ( pWMF->Tell() >= nMaxStreamPos )
                                break;
                            *pWMF >> nDx;
                            if ( nNewTextLen != nOriginalTextLen )
                            {
                                sal_Unicode nUniChar = aText.GetChar(i);
                                rtl::OString aTmp(&nUniChar, 1, pOut->GetCharSet());
                                if ( aTmp.getLength() > 1 )
                                {
                                    sal_Int32 nDxCount = aTmp.getLength() - 1;
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
                            bUseDXAry = sal_True;
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
            sal_Int16   nObjIndex = 0;
            *pWMF >> nObjIndex;
            pOut->SelectObject( nObjIndex );
        }
        break;

        case W_META_SETTEXTALIGN:
        {
            sal_uInt16  nAlign = 0;
            *pWMF >> nAlign;
            pOut->SetTextAlign( nAlign );
        }
        break;

        case W_META_BITBLT:
        {
            // 0-3   : nWinROP                      #93454#
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
            // 25    : bitcount

            sal_Int32   nWinROP = 0;
            sal_uInt16  nSx = 0, nSy = 0, nSxe = 0, nSye = 0, nDontKnow = 0, nWidth = 0, nHeight = 0, nBytesPerScan = 0;
            sal_uInt8   nPlanes, nBitCount;

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
                    for (sal_uInt16 y = 0; y < nHeight; y++ )
                    {
                        sal_uInt16 x = 0;
                        for (sal_uInt16 scan = 0; scan < nBytesPerScan; scan++ )
                        {
                            sal_Int8 nEightPixels = 0;
                            *pWMF >> nEightPixels;
                            for (sal_Int8 i = 7; i >= 0; i-- )
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
                    aBmpSaveList.push_back( new BSaveStruct( aBmp, aDestRect, nWinROP, pOut->GetFillStyle () ) );
                }
            }
        }
        break;

        case W_META_STRETCHBLT:
        case W_META_DIBBITBLT:
        case W_META_DIBSTRETCHBLT:
        case W_META_STRETCHDIB:
        {
            sal_Int32   nWinROP = 0;
            sal_uInt16  nSx = 0, nSy = 0, nSxe = 0, nSye = 0, nUsage = 0;
            Bitmap      aBmp;

            *pWMF >> nWinROP;

            if( nFunc == W_META_STRETCHDIB )
                *pWMF >> nUsage;

            // nSye and nSxe is the number of pixels that has to been used
            // If they are set to zero, it is as indicator not to scale the bitmap later
            //
            if( nFunc == W_META_STRETCHDIB || nFunc == W_META_STRETCHBLT || nFunc == W_META_DIBSTRETCHBLT )
                *pWMF >> nSye >> nSxe;

            // nSy and nx is the offset of the first pixel
            *pWMF >> nSy >> nSx;

            if( nFunc == W_META_STRETCHDIB || nFunc == W_META_DIBBITBLT || nFunc == W_META_DIBSTRETCHBLT )
            {
                if ( nWinROP == PATCOPY )
                    *pWMF >> nUsage;    // i don't know anything of this parameter, so its called nUsage
                                        // pOut->DrawRect( Rectangle( ReadYX(), aDestSize ), sal_False );

                Size aDestSize( ReadYXExt() );
                if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps
                {
                    Rectangle aDestRect( ReadYX(), aDestSize );
                    if ( nWinROP != PATCOPY )
                        aBmp.Read( *pWMF, sal_False );

                    // test if it is sensible to crop
                    if ( nSye && nSxe &&
                        ( ( nSx + nSxe ) <= aBmp.GetSizePixel().Width() ) &&
                            ( ( nSy + nSye <= aBmp.GetSizePixel().Height() ) ) )
                    {
                        Rectangle aCropRect( Point( nSx, nSy ), Size( nSxe, nSye ) );
                        aBmp.Crop( aCropRect );
                    }
                    aBmpSaveList.push_back( new BSaveStruct( aBmp, aDestRect, nWinROP, pOut->GetFillStyle () ) );
                }
            }
        }
        break;

        case W_META_DIBCREATEPATTERNBRUSH:
        {
            Bitmap  aBmp;
            BitmapReadAccess* pBmp;
            sal_uInt32  nRed = 0, nGreen = 0, nBlue = 0, nCount = 1;
            sal_uInt16  nFunction = 0;

            *pWMF >> nFunction >> nFunction;

            aBmp.Read( *pWMF, sal_False );
            pBmp = aBmp.AcquireReadAccess();
            if ( pBmp )
            {
                for ( sal_Int32 y = 0; y < pBmp->Height(); y++ )
                {
                    for ( sal_Int32 x = 0; x < pBmp->Width(); x++ )
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
            Color aColor( (sal_uInt8)( nRed / nCount ), (sal_uInt8)( nGreen / nCount ), (sal_uInt8)( nBlue / nCount ) );
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( aColor, sal_False ) );
        }
        break;

        case W_META_DELETEOBJECT:
        {
            sal_Int16 nIndex = 0;
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
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), sal_False ) );
        }
        break;

        case W_META_CREATEPATTERNBRUSH:
        {
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( Color( COL_WHITE ), sal_False ) );
        }
        break;

        case W_META_CREATEPENINDIRECT:
        {
            LineInfo    aLineInfo;
            sal_uInt16      nStyle = 0, nWidth = 0, nHeight = 0;

            *pWMF >> nStyle >> nWidth >> nHeight;

            if ( nWidth )
                aLineInfo.SetWidth( nWidth );

            sal_Bool bTransparent = sal_False;
            sal_uInt16 nDashCount = 0;
            sal_uInt16 nDotCount = 0;
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
                    bTransparent = sal_True;
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
            sal_uInt16  nStyle = 0;
            *pWMF >> nStyle;
            pOut->CreateObject( GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? sal_True : sal_False ) );
        }
        break;

        case W_META_CREATEFONTINDIRECT:
        {
            Size    aFontSize;
            char    lfFaceName[ LF_FACESIZE ];
            sal_Int16   lfEscapement = 0, lfOrientation = 0, lfWeight = 0;  // ( formerly sal_uInt16 )

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
                eCharSet = osl_getThreadTextEncoding();
            else
                eCharSet = rtl_getTextEncodingFromWindowsCharset( aLogFont.lfCharSet );
            if ( eCharSet == RTL_TEXTENCODING_DONTKNOW )
                eCharSet = osl_getThreadTextEncoding();
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
            sal_uInt32 nROP = 0, nOldROP = 0;
            *pWMF >> nROP;
            Size aSize = ReadYXExt();
            nOldROP = pOut->SetRasterOp( nROP );
            pOut->DrawRect( Rectangle( ReadYX(), aSize ), sal_False );
            pOut->SetRasterOp( nOldROP );
        }
        break;

        case W_META_SELECTCLIPREGION:
        {
            sal_Int16 nObjIndex = 0;
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
            // nRecSize has been checked previously to be greater than 3
            sal_uInt64 nMetaRecSize = static_cast< sal_uInt64 >( nRecSize - 2 ) * 2;
            sal_uInt64 nMetaRecEndPos = pWMF->Tell() + nMetaRecSize;

            // taking care that nRecSize does not exceed the maximal stream position
            if ( nMetaRecEndPos > nEndPos )
            {
                pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                break;
            }
            if ( nRecSize >= 4 )    // minimal escape lenght
            {
                sal_uInt16  nMode = 0, nLen = 0;
                *pWMF >> nMode
                      >> nLen;
                if ( ( nMode == W_MFCOMMENT ) && ( nLen >= 4 ) )
                {
                    sal_uInt32 nNewMagic = 0; // we have to read int32 for
                    *pWMF >> nNewMagic;   // META_ESCAPE_ENHANCED_METAFILE CommentIdentifier

                    if( nNewMagic == 0x2c2a4f4f &&  nLen >= 14 )
                    {
                        sal_uInt16 nMagic2 = 0;
                        *pWMF >> nMagic2;
                        if( nMagic2 == 0x0a ) // 2nd half of magic
                        {                     // continue with private escape
                            sal_uInt32 nCheck = 0, nEsc = 0;
                            *pWMF >> nCheck
                                  >> nEsc;

                            sal_uInt32 nEscLen = nLen - 14;
                            if ( nEscLen <= ( nRecSize * 2 ) )
                            {
#ifdef OSL_BIGENDIAN
                                sal_uInt32 nTmp = OSL_SWAPDWORD( nEsc );
                                sal_uInt32 nCheckSum = rtl_crc32( 0, &nTmp, 4 );
#else
                                sal_uInt32 nCheckSum = rtl_crc32( 0, &nEsc, 4 );
#endif
                                sal_Int8* pData = NULL;

                                if ( ( static_cast< sal_uInt64 >( nEscLen ) + pWMF->Tell() ) > nMetaRecEndPos )
                                {
                                    pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                                    break;
                                }
                                if ( nEscLen > 0 )
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
                                        {   // we will use text instead of polygons only if we have the correct font
                                            if ( aVDev.IsFontAvailable( pOut->GetFont().GetName() ) )
                                            {
                                                Point  aPt;
                                                String aString;
                                                sal_uInt32  nStringLen, nDXCount;
                                                sal_Int32* pDXAry = NULL;
                                                SvMemoryStream aMemoryStream( nEscLen );
                                                aMemoryStream.Write( pData, nEscLen );
                                                aMemoryStream.Seek( STREAM_SEEK_TO_BEGIN );
                                                //#fdo39428 SvStream no longer supports operator>>(long&)
                                                sal_Int32 nTmpX(0), nTmpY(0);
                                                aMemoryStream >> nTmpX
                                                              >> nTmpY
                                                              >> nStringLen;
                                                 aPt.X() = nTmpX;
                                                 aPt.Y() = nTmpY;

                                                if ( ( static_cast< sal_uInt64 >( nStringLen ) * sizeof( sal_Unicode ) ) < ( nEscLen - aMemoryStream.Tell() ) )
                                                {

                                                    aString = read_uInt16s_ToOUString(aMemoryStream, nStringLen);
                                                    aMemoryStream >> nDXCount;
                                                    if ( ( static_cast< sal_uInt64 >( nDXCount ) * sizeof( sal_Int32 ) ) >= ( nEscLen - aMemoryStream.Tell() ) )
                                                        nDXCount = 0;
                                                    if ( nDXCount )
                                                        pDXAry = new sal_Int32[ nDXCount ];
                                                    for  (sal_uInt32 i = 0; i < nDXCount; i++ )
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
                    else if ( (nNewMagic == static_cast< sal_uInt32 >(0x43464D57)) && (nLen >= 34) && ( (sal_Int32)(nLen + 10) <= (sal_Int32)(nRecSize * 2) ))
                    {
                        sal_uInt32 nComType = 0, nVersion = 0, nFlags = 0, nComRecCount = 0,
                                   nCurRecSize = 0, nRemainingSize = 0, nEMFTotalSize = 0;
                        sal_uInt16 nCheck = 0;

                        *pWMF >> nComType >> nVersion >> nCheck >> nFlags
                              >> nComRecCount >> nCurRecSize
                              >> nRemainingSize >> nEMFTotalSize; // the nRemainingSize is not mentioned in MSDN documentation
                                                                  // but it seems to be required to read in data produced by OLE

                        if( nComType == 0x01 && nVersion == 0x10000 && nComRecCount )
                        {
                            if( !nEMFRec )
                            {   // first EMF comment
                                nEMFRecCount    = nComRecCount;
                                nEMFSize        = nEMFTotalSize;
                                pEMFStream = new SvMemoryStream( nEMFSize );
                            }
                            else if( ( nEMFRecCount != nComRecCount ) || ( nEMFSize != nEMFTotalSize ) ) // add additional checks here
                            {
                                // total records should be the same as in previous comments
                                nEMFRecCount = 0xFFFFFFFF;
                                delete pEMFStream;
                                pEMFStream = NULL;
                            }
                            nEMFRec++;

                            if( pEMFStream && nCurRecSize + 34 > nLen )
                            {
                                nEMFRecCount = 0xFFFFFFFF;
                                delete pEMFStream;
                                pEMFStream = NULL;
                            }

                            if( pEMFStream )
                            {
                                sal_Int8* pBuf = new sal_Int8[ nCurRecSize ];
                                sal_uInt32 nCount = pWMF->Read( pBuf, nCurRecSize );
                                if( nCount == nCurRecSize )
                                    pEMFStream->Write( pBuf, nCount );
                                delete[] pBuf;
                            }
                        }
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

sal_Bool WMFReader::ReadHeader()
{
    sal_Size nStrmPos = pWMF->Tell();

    sal_uInt32 nPlaceableMetaKey(0);
    // Einlesen des METAFILEHEADER, falls vorhanden
    *pWMF >> nPlaceableMetaKey;
    if (!pWMF->good())
        return false;

    Rectangle aPlaceableBound;

    if (nPlaceableMetaKey == 0x9ac6cdd7L)
    { //TODO do some real error handling here
        sal_Int16 nVal;

        // Skip reserved bytes
        pWMF->SeekRel(2);

        // BoundRect
        *pWMF >> nVal;
        aPlaceableBound.Left() = nVal;
        *pWMF >> nVal;
        aPlaceableBound.Top() = nVal;
        *pWMF >> nVal;
        aPlaceableBound.Right() = nVal;
        *pWMF >> nVal;
        aPlaceableBound.Bottom() = nVal;

        // inch
        *pWMF >> nUnitsPerInch;

        // reserved
        pWMF->SeekRel( 4 );

        // Skip and don't check the checksum
        pWMF->SeekRel( 2 );
    }
    else
    {
        nUnitsPerInch = 96;
        pWMF->Seek( nStrmPos + 18 );    // set the streampos to the start of the the metaactions
        GetPlaceableBound( aPlaceableBound, pWMF );
        pWMF->Seek( nStrmPos );
        if ( pExternalHeader != NULL && ( pExternalHeader->mapMode == MM_ISOTROPIC
                                        || pExternalHeader->mapMode == MM_ANISOTROPIC ) )
        {
            // #n417818#: If we have an external header then overwrite the bounds!
            Rectangle aExtRect(0, 0,
                          pExternalHeader->xExt*567*nUnitsPerInch/1440/1000,
                          pExternalHeader->yExt*567*nUnitsPerInch/1440/1000);
            GetWinExtMax( aExtRect, aPlaceableBound, pExternalHeader->mapMode );
            pOut->SetMapMode( pExternalHeader->mapMode );
        }
    }

    pOut->SetUnitsPerInch( nUnitsPerInch );
    pOut->SetWinOrg( aPlaceableBound.TopLeft() );
    Size aWMFSize( labs( aPlaceableBound.GetWidth() ), labs( aPlaceableBound.GetHeight() ) );
    pOut->SetWinExt( aWMFSize );

    Size aDevExt( 10000, 10000 );
    if( ( labs( aWMFSize.Width() ) > 1 ) && ( labs( aWMFSize.Height() ) > 1 ) )
    {
        const Fraction  aFrac( 1, nUnitsPerInch );
        MapMode         aWMFMap( MAP_INCH, Point(), aFrac, aFrac );
        Size            aSize100( OutputDevice::LogicToLogic( aWMFSize, aWMFMap, MAP_100TH_MM ) );
        aDevExt = Size( labs( aSize100.Width() ), labs( aSize100.Height() ) );
    }
    pOut->SetDevExt( aDevExt );

    // Einlesen des METAHEADER
    sal_uInt32 nMetaKey(0);
    *pWMF >> nMetaKey; // Typ und Headergroesse
    if (!pWMF->good())
        return false;
    if (nMetaKey != 0x00090001)
    {
        sal_uInt16 aNextWord(0);
        *pWMF >> aNextWord;
        if (nMetaKey != 0x10000 || aNextWord != 0x09)
        {
            pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
            return false;
        }
    }

    pWMF->SeekRel( 2 ); // Version (von Windows)
    pWMF->SeekRel( 4 ); // Size (der Datei in Words)
    pWMF->SeekRel( 2 ); // NoObjects (Maximale Anzahl der gleichzeitigen Objekte)
    pWMF->SeekRel( 4 ); // MaxRecord (Groesse des groessten Records in Words)
    pWMF->SeekRel( 2 ); // NoParameters (Unused

    return pWMF->good();
}

void WMFReader::ReadWMF()
{
    sal_uInt16  nFunction;
    sal_uLong   nPos, nPercent, nLastPercent;

    nSkipActions = 0;
    nCurrentAction = 0;
    nUnicodeEscapeAction = 0;

    pEMFStream      = NULL;
    nEMFRecCount    = 0;
    nEMFRec         = 0;
    nEMFSize        = 0;

    sal_Bool bEMFAvailable = sal_False;

    pOut->SetMapMode( MM_ANISOTROPIC );
    pOut->SetWinOrg( Point() );
    pOut->SetWinExt( Size( 1, 1 ) );
    pOut->SetDevExt( Size( 10000, 10000 ) );

    nEndPos=pWMF->Seek( STREAM_SEEK_TO_END );
    pWMF->Seek( nStartPos );
    Callback( (sal_uInt16) ( nLastPercent = 0 ) );

    if ( ReadHeader( ) )
    {

        nPos = pWMF->Tell();

        if( nEndPos - nStartPos )
        {
            while( sal_True )
            {
                nCurrentAction++;
                nPercent = ( nPos - nStartPos ) * 100 / ( nEndPos - nStartPos );

                if( nLastPercent + 4 <= nPercent )
                {
                    Callback( (sal_uInt16) nPercent );
                    nLastPercent = nPercent;
                }
                *pWMF >> nRecSize >> nFunction;

                if(  pWMF->GetError()
                  || ( nRecSize < 3 )
                  || (  nRecSize  == 3
                     && nFunction == 0
                     )
                  || pWMF->IsEof()
                  )
                {
                    if( pWMF->IsEof() )
                        pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );

                    break;
                }
                if ( !bEMFAvailable )
                {
                    if(   !aBmpSaveList.empty()
                      && ( nFunction != W_META_STRETCHDIB    )
                      && ( nFunction != W_META_DIBBITBLT     )
                      && ( nFunction != W_META_DIBSTRETCHBLT )
                      )
                    {
                        pOut->ResolveBitmapActions( aBmpSaveList );
                    }

                    if ( !nSkipActions )
                        ReadRecordParams( nFunction );
                    else
                        nSkipActions--;

                    if( pEMFStream && nEMFRecCount == nEMFRec )
                    {
                        GDIMetaFile aMeta;
                        pEMFStream->Seek( 0 );
                        EnhWMFReader* pEMFReader = new EnhWMFReader ( *pEMFStream, aMeta );
                        bEMFAvailable = pEMFReader->ReadEnhWMF();
                        delete pEMFReader; // destroy first!!!

                        if( bEMFAvailable )
                        {
                            pOut->AddFromGDIMetaFile( aMeta );
                            pOut->SetrclFrame( Rectangle( Point(0, 0), aMeta.GetPrefSize()));

                            // the stream needs to be set to the wmf end position,
                            // otherwise the GfxLink that is created will be incorrect
                            // (leading to graphic loss after swapout/swapin).
                            // so we will proceed normally, but are ignoring further wmf
                            // records
                        }
                        else
                        {
                            // something went wrong
                            // continue with WMF, don't try this again
                            delete pEMFStream;
                            pEMFStream = NULL;
                        }
                    }
                }
                nPos += nRecSize * 2;
                if ( nPos <= nEndPos )
                    pWMF->Seek( nPos  );
                else
                    pWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
            }
        }
        else
            pWMF->SetError( SVSTREAM_GENERALERROR );

        if( !pWMF->GetError() && !aBmpSaveList.empty() )
            pOut->ResolveBitmapActions( aBmpSaveList );
    }
    if ( pWMF->GetError() )
        pWMF->Seek( nStartPos );
}

// ------------------------------------------------------------------------

sal_Bool WMFReader::GetPlaceableBound( Rectangle& rPlaceableBound, SvStream* pStm )
{
    sal_Bool bRet = sal_True;

    rPlaceableBound.Left()   = (sal_Int32)0x7fffffff;
    rPlaceableBound.Top()    = (sal_Int32)0x7fffffff;
    rPlaceableBound.Right()  = (sal_Int32)0x80000000;
    rPlaceableBound.Bottom() = (sal_Int32)0x80000000;

    sal_uInt32 nPos = pStm->Tell();
    sal_uInt32 nEnd = pStm->Seek( STREAM_SEEK_TO_END );

    pStm->Seek( nPos );

    if( nEnd - nPos )
    {
        sal_Int16 nMapMode = MM_ANISOTROPIC;
        sal_uInt16 nFunction;
        sal_uInt32 nRSize;

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
                    ReadYXExt(); // size
                    GetWinExtMax( ReadRectangle(), rPlaceableBound, nMapMode );
                break;

                case W_META_ARC:
                case W_META_PIE:
                case W_META_CHORD:
                    ReadYX(); // end
                    ReadYX(); // start
                    GetWinExtMax( ReadRectangle(), rPlaceableBound, nMapMode );
                break;

                case W_META_POLYGON:
                {
                    sal_uInt16 nPoints;
                    *pStm >> nPoints;
                    for(sal_uInt16 i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_POLYPOLYGON:
                {
                    bool bRecordOk = true;
                    sal_uInt16 nPoly, nPoints = 0;
                    *pStm >> nPoly;
                    for(sal_uInt16 i = 0; i < nPoly; i++ )
                    {
                        sal_uInt16 nP = 0;
                        *pStm >> nP;
                        if (nP > SAL_MAX_UINT16 - nPoints)
                        {
                            bRecordOk = false;
                            break;
                        }
                        nPoints += nP;
                    }

                    SAL_WARN_IF(!bRecordOk, "svtools", "polypolygon record has more polygons that we can handle");

                    bRecordOk &= pStm->good();

                    if (!bRecordOk)
                    {
                        pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                        bRet = sal_False;
                        break;
                    }

                    for (sal_uInt16 i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );

                    bRecordOk &= pStm->good();

                    if (!bRecordOk)
                    {
                        pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                        bRet = sal_False;
                        break;
                    }
                }
                break;

                case W_META_POLYLINE:
                {
                    sal_uInt16 nPoints;
                    *pStm >> nPoints;
                    for(sal_uInt16 i = 0; i < nPoints; i++ )
                        GetWinExtMax( ReadPoint(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_SETPIXEL:
                {
                    ReadColor();
                    GetWinExtMax( ReadYX(), rPlaceableBound, nMapMode );
                }
                break;

                case W_META_TEXTOUT:
                {
                    sal_uInt16 nLength;
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
                    sal_uInt16  nLen, nOptions;
                    sal_Int32   nRecordSize;
                    Point       aPosition;
                    Rectangle   aRect;

                    pStm->SeekRel(-6);
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
                            *pStm >> nUsage;    // i don't know anything of this parameter, so its called nUsage
                                                // pOut->DrawRect( Rectangle( ReadYX(), aDestSize ), sal_False );

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
                    sal_uInt32 nROP;
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

WMFReader::~WMFReader()
{
    if( pEMFStream )
        delete pEMFStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
