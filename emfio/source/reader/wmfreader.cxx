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

#include <wmfreader.hxx>
#include <emfreader.hxx>

#include <memory>
#include <boost/optional.hpp>
//#include <vcl/gdimtf.hxx>
//#include <vcl/wmf.hxx>
#include <rtl/crc.h>
#include <rtl/tencinfo.h>
#include <osl/endian.h>
#include <vcl/svapp.hxx>
#include <vcl/dibtools.hxx>
#include <tools/fract.hxx>
#include <o3tl/make_unique.hxx>
#include <vcl/bitmapaccess.hxx>
#include <osl/thread.h>

// MS Windows defines

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

namespace
{
    static void GetWinExtMax(const Point& rSource, tools::Rectangle& rPlaceableBound, const sal_Int16 nMapMode)
    {
        Point aSource(rSource);
        if (nMapMode == MM_HIMETRIC)
            aSource.Y() = -rSource.Y();
        if (aSource.X() < rPlaceableBound.Left())
            rPlaceableBound.Left() = aSource.X();
        if (aSource.X() > rPlaceableBound.Right())
            rPlaceableBound.Right() = aSource.X();
        if (aSource.Y() < rPlaceableBound.Top())
            rPlaceableBound.Top() = aSource.Y();
        if (aSource.Y() > rPlaceableBound.Bottom())
            rPlaceableBound.Bottom() = aSource.Y();
    }

    static void GetWinExtMax(const tools::Rectangle& rSource, tools::Rectangle& rPlaceableBound, const sal_Int16 nMapMode)
    {
        GetWinExtMax(rSource.TopLeft(), rPlaceableBound, nMapMode);
        GetWinExtMax(rSource.BottomRight(), rPlaceableBound, nMapMode);
    }

}

namespace emfio
{
    inline Point WmfReader::ReadPoint()
    {
        short nX = 0, nY = 0;
        mpWMF->ReadInt16( nX ).ReadInt16( nY );
        return Point( nX, nY );
    }

    inline Point WmfReader::ReadYX()
    {
        short nX = 0, nY = 0;
        mpWMF->ReadInt16( nY ).ReadInt16( nX );
        return Point( nX, nY );
    }

    tools::Rectangle WmfReader::ReadRectangle()
    {
        Point aBR, aTL;
        aBR = ReadYX();
        aTL = ReadYX();
        aBR.X()--;
        aBR.Y()--;
        return tools::Rectangle( aTL, aBR );
    }

    Size WmfReader::ReadYXExt()
    {
        short nW=0, nH=0;
        mpWMF->ReadInt16( nH ).ReadInt16( nW );
        return Size( nW, nH );
    }

    void WmfReader::ReadRecordParams( sal_uInt16 nFunc )
    {
        switch( nFunc )
        {
            case W_META_SETBKCOLOR:
            {
                SetBkColor( ReadColor() );
            }
            break;

            case W_META_SETBKMODE:
            {
                sal_uInt16 nDat = 0;
                mpWMF->ReadUInt16( nDat );
                SetBkMode( static_cast<BkMode>(nDat) );
            }
            break;

            // !!!
            case W_META_SETMAPMODE:
            {
                sal_Int16 nMapMode = 0;
                mpWMF->ReadInt16( nMapMode );
                SetMapMode( nMapMode );
            }
            break;

            case W_META_SETROP2:
            {
                sal_uInt16 nROP2 = 0;
                mpWMF->ReadUInt16( nROP2 );
                SetRasterOp( (WMFRasterOp)nROP2 );
            }
            break;

            case W_META_SETTEXTCOLOR:
            {
                SetTextColor( ReadColor() );
            }
            break;

            case W_META_SETWINDOWORG:
            {
                SetWinOrg( ReadYX() );
            }
            break;

            case W_META_SETWINDOWEXT:
            {
                short nWidth = 0, nHeight = 0;
                mpWMF->ReadInt16( nHeight ).ReadInt16( nWidth );
                SetWinExt( Size( nWidth, nHeight ) );
            }
            break;

            case W_META_OFFSETWINDOWORG:
            {
                short nXAdd = 0, nYAdd = 0;
                mpWMF->ReadInt16( nYAdd ).ReadInt16( nXAdd );
                SetWinOrgOffset( nXAdd, nYAdd );
            }
            break;

            case W_META_SCALEWINDOWEXT:
            {
                short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
                mpWMF->ReadInt16( nYDenom ).ReadInt16( nYNum ).ReadInt16( nXDenom ).ReadInt16( nXNum );
                if (!nYDenom || !nXDenom)
                {
                    mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                ScaleWinExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
            }
            break;

            case W_META_SETVIEWPORTORG:
            case W_META_SETVIEWPORTEXT:
            break;

            case W_META_OFFSETVIEWPORTORG:
            {
                short nXAdd = 0, nYAdd = 0;
                mpWMF->ReadInt16( nYAdd ).ReadInt16( nXAdd );
                SetDevOrgOffset( nXAdd, nYAdd );
            }
            break;

            case W_META_SCALEVIEWPORTEXT:
            {
                short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
                mpWMF->ReadInt16( nYDenom ).ReadInt16( nYNum ).ReadInt16( nXDenom ).ReadInt16( nXNum );
                if (!nYDenom || !nXDenom)
                {
                    mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                ScaleDevExt( (double)nXNum / nXDenom, (double)nYNum / nYDenom );
            }
            break;

            case W_META_LINETO:
            {
                LineTo( ReadYX() );
            }
            break;

            case W_META_MOVETO:
            {
                MoveTo( ReadYX() );
            }
            break;

            case W_META_INTERSECTCLIPRECT:
            {
                IntersectClipRect( ReadRectangle() );
            }
            break;

            case W_META_RECTANGLE:
            {
                DrawRect( ReadRectangle() );
            }
            break;

            case W_META_ROUNDRECT:
            {
                Size aSize( ReadYXExt() );
                DrawRoundRect( ReadRectangle(), Size( aSize.Width() / 2, aSize.Height() / 2 ) );
            }
            break;

            case W_META_ELLIPSE:
            {
                DrawEllipse( ReadRectangle() );
            }
            break;

            case W_META_ARC:
            {
                Point aEnd( ReadYX() );
                Point aStart( ReadYX() );
                tools::Rectangle aRect( ReadRectangle() );
                aRect.Justify();
                DrawArc( aRect, aStart, aEnd );
            }
            break;

            case W_META_PIE:
            {
                Point     aEnd( ReadYX() );
                Point     aStart( ReadYX() );
                tools::Rectangle aRect( ReadRectangle() );
                aRect.Justify();

                // #i73608# OutputDevice deviates from WMF
                // semantics. start==end means full ellipse here.
                if( aStart == aEnd )
                    DrawEllipse( aRect );
                else
                    DrawPie( aRect, aStart, aEnd );
            }
            break;

            case W_META_CHORD:
            {
                Point aEnd( ReadYX() );
                Point aStart( ReadYX() );
                tools::Rectangle aRect( ReadRectangle() );
                aRect.Justify();
                DrawChord( aRect, aStart, aEnd );
            }
            break;

            case W_META_POLYGON:
            {
                bool bRecordOk = true;

                sal_uInt16 nPoints(0);
                mpWMF->ReadUInt16(nPoints);

                if (nPoints > mpWMF->remainingSize() / (2 * sizeof(sal_uInt16)))
                {
                    bRecordOk = false;
                }
                else
                {
                    tools::Polygon aPoly(nPoints);
                    for (sal_uInt16 i(0); i < nPoints && mpWMF->good(); ++i)
                        aPoly[ i ] = ReadPoint();
                    DrawPolygon(aPoly, false/*bRecordPath*/);
                }

                SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polygon record has more points than we can handle");

                bRecordOk &= mpWMF->good();

                if (!bRecordOk)
                {
                    mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
            }
            break;

            case W_META_POLYPOLYGON:
            {
                sal_uInt16 nPolyCount(0);
                // Number of polygons:
                mpWMF->ReadUInt16( nPolyCount );
                if (nPolyCount && mpWMF->good())
                {
                    bool bRecordOk = true;
                    if (nPolyCount > mpWMF->remainingSize() / sizeof(sal_uInt16))
                    {
                        break;
                    }

                    // Number of points of each polygon. Determine total number of points
                    std::unique_ptr<sal_uInt16[]> xPolygonPointCounts(new sal_uInt16[nPolyCount]);
                    sal_uInt16* pnPoints = xPolygonPointCounts.get();
                    tools::PolyPolygon aPolyPoly(nPolyCount, nPolyCount);
                    sal_uInt16 nPoints = 0;
                    for (sal_uInt16 a = 0; a < nPolyCount && mpWMF->good(); ++a)
                    {
                        mpWMF->ReadUInt16( pnPoints[a] );

                        if (pnPoints[a] > SAL_MAX_UINT16 - nPoints)
                        {
                            bRecordOk = false;
                            break;
                        }

                        nPoints += pnPoints[a];
                    }

                    SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polypolygon record has more polygons than we can handle");

                    bRecordOk &= mpWMF->good();

                    if (!bRecordOk)
                    {
                        mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                        break;
                    }

                    // Polygon points are:
                    for (sal_uInt16 a = 0; a < nPolyCount && mpWMF->good(); ++a)
                    {
                        const sal_uInt16 nPointCount(pnPoints[a]);

                        if (nPointCount > mpWMF->remainingSize() / (2 * sizeof(sal_uInt16)))
                        {
                            bRecordOk = false;
                            break;
                        }

                        std::unique_ptr<Point[]> xPolygonPoints(new Point[nPointCount]);
                        Point* pPtAry = xPolygonPoints.get();

                        for(sal_uInt16 b(0); b < nPointCount && mpWMF->good(); ++b)
                        {
                            pPtAry[b] = ReadPoint();
                        }

                        aPolyPoly.Insert( tools::Polygon(nPointCount, pPtAry) );
                    }

                    bRecordOk &= mpWMF->good();

                    if (!bRecordOk)
                    {
                        mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                        break;
                    }

                    DrawPolyPolygon( aPolyPoly );
                }
            }
            break;

            case W_META_POLYLINE:
            {
                bool bRecordOk = true;

                sal_uInt16 nPoints(0);
                mpWMF->ReadUInt16(nPoints);

                if (nPoints > mpWMF->remainingSize() / (2 * sizeof(sal_uInt16)))
                {
                    bRecordOk = false;
                }
                else
                {
                    tools::Polygon aPoly(nPoints);
                    for (sal_uInt16 i(0); i < nPoints && mpWMF->good(); ++i)
                        aPoly[ i ] = ReadPoint();
                    DrawPolyLine( aPoly );
                }

                SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polyline record has more points than we can handle");

                bRecordOk &= mpWMF->good();

                if (!bRecordOk)
                {
                    mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
            }
            break;

            case W_META_SAVEDC:
            {
                Push();
            }
            break;

            case W_META_RESTOREDC:
            {
                Pop();
            }
            break;

            case W_META_SETPIXEL:
            {
                const Color aColor = ReadColor();
                DrawPixel( ReadYX(), aColor );
            }
            break;

            case W_META_OFFSETCLIPRGN:
            {
                MoveClipRegion( ReadYXExt() );
            }
            break;

            case W_META_TEXTOUT:
            {
                sal_uInt16 nLength = 0;
                mpWMF->ReadUInt16( nLength );
                if ( nLength )
                {
                    std::unique_ptr<char[]> pChar(new char[ ( nLength + 1 ) &~ 1 ]);
                    mpWMF->ReadBytes(pChar.get(), (nLength + 1) &~ 1);
                    OUString aText( pChar.get(), nLength, GetCharSet() );
                    pChar.reset();
                    Point aPosition( ReadYX() );
                    DrawText( aPosition, aText );
                }
            }
            break;

            case W_META_EXTTEXTOUT:
            {
                mpWMF->SeekRel(-6);
                sal_Int32 nRecordPos = mpWMF->Tell(), nRecordSize = 0;
                mpWMF->ReadInt32( nRecordSize );
                mpWMF->SeekRel(2);
                Point aPosition = ReadYX();
                sal_uInt16 nLen = 0, nOptions = 0;
                mpWMF->ReadUInt16( nLen ).ReadUInt16( nOptions );

                ComplexTextLayoutFlags nTextLayoutMode = ComplexTextLayoutFlags::Default;
                if ( nOptions & ETO_RTLREADING )
                    nTextLayoutMode = ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::TextOriginLeft;
                SetTextLayoutMode( nTextLayoutMode );
                SAL_WARN_IF( ( nOptions & ( ETO_PDY | ETO_GLYPH_INDEX ) ) != 0, "vcl.wmf", "SJ: ETO_PDY || ETO_GLYPH_INDEX in WMF" );

                // output only makes sense if the text contains characters
                if( nLen )
                {
                    sal_Int32 nOriginalTextLen = nLen;
                    sal_Int32 nOriginalBlockLen = ( nOriginalTextLen + 1 ) &~ 1;
                    tools::Rectangle aRect;
                    if( nOptions & ETO_CLIPPED )
                    {
                        const Point aPt1( ReadPoint() );
                        const Point aPt2( ReadPoint() );
                        aRect = tools::Rectangle( aPt1, aPt2 );
                    }

                    auto nRemainingSize = mpWMF->remainingSize();
                    if (nRemainingSize < static_cast<sal_uInt32>(nOriginalBlockLen))
                    {
                        SAL_WARN("vcl.wmf", "exttextout record claimed more data than the stream can provide");
                        nOriginalTextLen = nOriginalBlockLen = nRemainingSize;
                    }

                    std::unique_ptr<char[]> pChar(new char[nOriginalBlockLen]);
                    mpWMF->ReadBytes(pChar.get(), nOriginalBlockLen);
                    OUString aText(pChar.get(), nOriginalTextLen, GetCharSet()); // after this conversion the text may contain
                    sal_Int32 nNewTextLen = aText.getLength();                         // less character (japanese version), so the
                                                                                       // dxAry will not fit
                    if ( nNewTextLen )
                    {
                        std::unique_ptr<long[]> pDXAry, pDYAry;
                        sal_uInt32  nMaxStreamPos = nRecordPos + ( nRecordSize << 1 );
                        sal_Int32   nDxArySize =  nMaxStreamPos - mpWMF->Tell();
                        sal_Int32   nDxAryEntries = nDxArySize >> 1;
                        bool        bUseDXAry = false;

                        if ( ( ( nDxAryEntries % nOriginalTextLen ) == 0 ) && ( nNewTextLen <= nOriginalTextLen ) )
                        {
                            sal_Int32 i; // needed just outside the for
                            pDXAry.reset(new long[ nNewTextLen ]);
                            if ( nOptions & ETO_PDY )
                            {
                                pDYAry.reset(new long[ nNewTextLen ]);
                            }
                            for (i = 0; i < nNewTextLen; i++ )
                            {
                                if ( mpWMF->Tell() >= nMaxStreamPos )
                                    break;
                                sal_Int32 nDxCount = 1;
                                if ( nNewTextLen != nOriginalTextLen )
                                {
                                    sal_Unicode cUniChar = aText[i];
                                    OString aTmp(&cUniChar, 1, GetCharSet());
                                    if ( aTmp.getLength() > 1 )
                                    {
                                        nDxCount = aTmp.getLength();
                                    }
                                }

                                sal_Int16 nDx = 0, nDy = 0;
                                while ( nDxCount-- )
                                {
                                    if ( ( mpWMF->Tell() + 2 ) > nMaxStreamPos )
                                        break;
                                    sal_Int16 nDxTmp = 0;
                                    mpWMF->ReadInt16(nDxTmp);
                                    nDx += nDxTmp;
                                    if ( nOptions & ETO_PDY )
                                    {
                                        if ( ( mpWMF->Tell() + 2 ) > nMaxStreamPos )
                                            break;
                                        sal_Int16 nDyTmp = 0;
                                        mpWMF->ReadInt16(nDyTmp);
                                        nDy += nDyTmp;
                                    }
                                }

                                pDXAry[ i ] = nDx;
                                if ( nOptions & ETO_PDY )
                                {
                                    pDYAry[i] = nDy;
                                }
                            }
                            if ( i == nNewTextLen )
                                bUseDXAry = true;
                        }
                        if ( pDXAry && bUseDXAry )
                            DrawText( aPosition, aText, pDXAry.get(), pDYAry.get() );
                        else
                            DrawText( aPosition, aText );
                    }
                }
            }
            break;

            case W_META_SELECTOBJECT:
            {
                sal_Int16   nObjIndex = 0;
                mpWMF->ReadInt16( nObjIndex );
                SelectObject( nObjIndex );
            }
            break;

            case W_META_SETTEXTALIGN:
            {
                sal_uInt16  nAlign = 0;
                mpWMF->ReadUInt16( nAlign );
                SetTextAlign( nAlign );
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
                // 16-17 : don't know
                // 18-19 : Width Bitmap in Pixel
                // 20-21 : Height Bitmap in Pixel
                // 22-23 : bytes per scanline
                // 24    : planes
                // 25    : bitcount

                sal_Int32   nWinROP = 0;
                sal_uInt16  nSx = 0, nSy = 0, nSxe = 0, nSye = 0, nDontKnow = 0, nWidth = 0, nHeight = 0, nBytesPerScan = 0;
                sal_uInt8   nPlanes, nBitCount;

                mpWMF->ReadInt32( nWinROP )
                     .ReadUInt16( nSy ).ReadUInt16( nSx ).ReadUInt16( nSye ).ReadUInt16( nSxe );
                Point aPoint( ReadYX() );
                mpWMF->ReadUInt16( nDontKnow ).ReadUInt16( nWidth ).ReadUInt16( nHeight ).ReadUInt16( nBytesPerScan ).ReadUChar( nPlanes ).ReadUChar( nBitCount );

                bool bOk = nWidth && nHeight && nPlanes == 1 && nBitCount == 1;
                if (bOk)
                {
                    bOk = nBytesPerScan <= mpWMF->remainingSize() / nHeight;
                }
                if (bOk)
                {
                    Bitmap aBmp( Size( nWidth, nHeight ), nBitCount );
                    Bitmap::ScopedWriteAccess pAcc(aBmp);
                    if ( pAcc )
                    {
                        for (sal_uInt16 y = 0; y < nHeight && mpWMF->good(); ++y)
                        {
                            sal_uInt16 x = 0;
                            for (sal_uInt16 scan = 0; scan < nBytesPerScan; scan++ )
                            {
                                sal_Int8 nEightPixels = 0;
                                mpWMF->ReadSChar( nEightPixels );
                                for (sal_Int8 i = 7; i >= 0; i-- )
                                {
                                    if ( x < nWidth )
                                    {
                                        pAcc->SetPixelIndex( y, x, (nEightPixels>>i)&1 );
                                    }
                                    x++;
                                }
                            }
                        }
                        pAcc.reset();
                        if ( nSye && nSxe &&
                            ( ( nSx + nSxe ) <= aBmp.GetSizePixel().Width() ) &&
                                ( ( nSy + nSye <= aBmp.GetSizePixel().Height() ) ) )
                        {
                            tools::Rectangle aCropRect( Point( nSx, nSy ), Size( nSxe, nSye ) );
                            aBmp.Crop( aCropRect );
                        }
                        tools::Rectangle aDestRect( aPoint, Size( nSxe, nSye ) );
                        maBmpSaveList.emplace_back(new BSaveStruct(aBmp, aDestRect, nWinROP));
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

                mpWMF->ReadInt32( nWinROP );

                if( nFunc == W_META_STRETCHDIB )
                    mpWMF->ReadUInt16( nUsage );

                // nSye and nSxe is the number of pixels that has to been used
                // If they are set to zero, it is as indicator not to scale the bitmap later

                if( nFunc == W_META_STRETCHDIB || nFunc == W_META_STRETCHBLT || nFunc == W_META_DIBSTRETCHBLT )
                    mpWMF->ReadUInt16( nSye ).ReadUInt16( nSxe );

                // nSy and nx is the offset of the first pixel
                mpWMF->ReadUInt16( nSy ).ReadUInt16( nSx );

                if( nFunc == W_META_STRETCHDIB || nFunc == W_META_DIBBITBLT || nFunc == W_META_DIBSTRETCHBLT )
                {
                    if ( nWinROP == PATCOPY )
                        mpWMF->ReadUInt16( nUsage );    // i don't know anything of this parameter, so its called nUsage
                                            // DrawRect( Rectangle( ReadYX(), aDestSize ), false );

                    Size aDestSize( ReadYXExt() );
                    if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps
                    {
                        tools::Rectangle aDestRect( ReadYX(), aDestSize );
                        if ( nWinROP != PATCOPY )
                            ReadDIB(aBmp, *mpWMF, false);

                        // test if it is sensible to crop
                        if ( nSye && nSxe &&
                            ( ( nSx + nSxe ) <= aBmp.GetSizePixel().Width() ) &&
                                ( ( nSy + nSye <= aBmp.GetSizePixel().Height() ) ) )
                        {
                            tools::Rectangle aCropRect( Point( nSx, nSy ), Size( nSxe, nSye ) );
                            aBmp.Crop( aCropRect );
                        }
                        maBmpSaveList.emplace_back(new BSaveStruct(aBmp, aDestRect, nWinROP));
                    }
                }
            }
            break;

            case W_META_DIBCREATEPATTERNBRUSH:
            {
                Bitmap  aBmp;
                sal_uInt32  nRed = 0, nGreen = 0, nBlue = 0, nCount = 1;
                sal_uInt16  nFunction = 0;

                mpWMF->ReadUInt16( nFunction ).ReadUInt16( nFunction );

                ReadDIB(aBmp, *mpWMF, false);
                Bitmap::ScopedReadAccess pBmp(aBmp);
                if ( pBmp )
                {
                    for ( long y = 0; y < pBmp->Height(); y++ )
                    {
                        for ( long x = 0; x < pBmp->Width(); x++ )
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
                    pBmp.reset();
                }
                Color aColor( (sal_uInt8)( nRed / nCount ), (sal_uInt8)( nGreen / nCount ), (sal_uInt8)( nBlue / nCount ) );
                CreateObject(o3tl::make_unique<WinMtfFillStyle>( aColor, false ));
            }
            break;

            case W_META_DELETEOBJECT:
            {
                sal_Int16 nIndex = 0;
                mpWMF->ReadInt16( nIndex );
                DeleteObject( nIndex );
            }
            break;

            case W_META_CREATEPALETTE:
            {
                CreateObject();
            }
            break;

            case W_META_CREATEBRUSH:
            {
                CreateObject(o3tl::make_unique<WinMtfFillStyle>( Color( COL_WHITE ), false ));
            }
            break;

            case W_META_CREATEPATTERNBRUSH:
            {
                CreateObject(o3tl::make_unique<WinMtfFillStyle>( Color( COL_WHITE ), false ));
            }
            break;

            case W_META_CREATEPENINDIRECT:
            {
                LineInfo   aLineInfo;
                sal_uInt16 nStyle = 0;
                sal_uInt16 nWidth = 0;
                sal_uInt16 nHeight = 0;

                mpWMF->ReadUInt16(nStyle);
                mpWMF->ReadUInt16(nWidth);
                mpWMF->ReadUInt16(nHeight);

                if (nWidth > 0)
                    aLineInfo.SetWidth(nWidth);

                bool bTransparent = false;

                switch( nStyle & 0xFF )
                {
                    case PS_DASHDOTDOT :
                        aLineInfo.SetStyle( LineStyle::Dash );
                        aLineInfo.SetDashCount( 1 );
                        aLineInfo.SetDotCount( 2 );
                    break;
                    case PS_DASHDOT :
                        aLineInfo.SetStyle( LineStyle::Dash );
                        aLineInfo.SetDashCount( 1 );
                        aLineInfo.SetDotCount( 1 );
                    break;
                    case PS_DOT :
                        aLineInfo.SetStyle( LineStyle::Dash );
                        aLineInfo.SetDashCount( 0 );
                        aLineInfo.SetDotCount( 1 );
                    break;
                    case PS_DASH :
                        aLineInfo.SetStyle( LineStyle::Dash );
                        aLineInfo.SetDashCount( 1 );
                        aLineInfo.SetDotCount( 0 );
                    break;
                    case PS_NULL :
                        bTransparent = true;
                        aLineInfo.SetStyle( LineStyle::NONE );
                    break;
                    default :
                    case PS_INSIDEFRAME :
                    case PS_SOLID :
                        aLineInfo.SetStyle( LineStyle::Solid );
                }
                switch( nStyle & 0xF00 )
                {
                    case PS_ENDCAP_ROUND :
                        aLineInfo.SetLineCap( css::drawing::LineCap_ROUND );
                    break;
                    case PS_ENDCAP_SQUARE :
                        aLineInfo.SetLineCap( css::drawing::LineCap_SQUARE );
                    break;
                    case PS_ENDCAP_FLAT :
                    default :
                        aLineInfo.SetLineCap( css::drawing::LineCap_BUTT );
                }
                switch( nStyle & 0xF000 )
                {
                    case PS_JOIN_ROUND :
                        aLineInfo.SetLineJoin ( basegfx::B2DLineJoin::Round );
                    break;
                    case PS_JOIN_MITER :
                        aLineInfo.SetLineJoin ( basegfx::B2DLineJoin::Miter );
                    break;
                    case PS_JOIN_BEVEL :
                        aLineInfo.SetLineJoin ( basegfx::B2DLineJoin::Bevel );
                    break;
                    default :
                        aLineInfo.SetLineJoin ( basegfx::B2DLineJoin::NONE );
                }
                CreateObject(o3tl::make_unique<WinMtfLineStyle>( ReadColor(), aLineInfo, bTransparent ));
            }
            break;

            case W_META_CREATEBRUSHINDIRECT:
            {
                sal_uInt16  nStyle = 0;
                mpWMF->ReadUInt16( nStyle );
                CreateObject(o3tl::make_unique<WinMtfFillStyle>( ReadColor(), ( nStyle == BS_HOLLOW ) ));
            }
            break;

            case W_META_CREATEFONTINDIRECT:
            {
                Size aFontSize;
                char lfFaceName[LF_FACESIZE+1];
                sal_Int16 lfEscapement = 0;
                sal_Int16 lfOrientation = 0;
                sal_Int16 lfWeight = 0;

                LOGFONTW aLogFont;
                aFontSize = ReadYXExt();
                mpWMF->ReadInt16( lfEscapement );
                mpWMF->ReadInt16( lfOrientation );
                mpWMF->ReadInt16( lfWeight );
                mpWMF->ReadUChar( aLogFont.lfItalic );
                mpWMF->ReadUChar( aLogFont.lfUnderline );
                mpWMF->ReadUChar( aLogFont.lfStrikeOut );
                mpWMF->ReadUChar( aLogFont.lfCharSet );
                mpWMF->ReadUChar( aLogFont.lfOutPrecision );
                mpWMF->ReadUChar( aLogFont.lfClipPrecision );
                mpWMF->ReadUChar( aLogFont.lfQuality );
                mpWMF->ReadUChar( aLogFont.lfPitchAndFamily );
                size_t nRet = mpWMF->ReadBytes( lfFaceName, LF_FACESIZE );
                lfFaceName[nRet] = 0;
                aLogFont.lfWidth = aFontSize.Width();
                aLogFont.lfHeight = aFontSize.Height();
                aLogFont.lfEscapement = lfEscapement;
                aLogFont.lfOrientation = lfOrientation;
                aLogFont.lfWeight = lfWeight;

                rtl_TextEncoding eCharSet;
                if ( ( aLogFont.lfCharSet == OEM_CHARSET ) || ( aLogFont.lfCharSet == DEFAULT_CHARSET ) )
                    eCharSet = osl_getThreadTextEncoding();
                else
                    eCharSet = rtl_getTextEncodingFromWindowsCharset( aLogFont.lfCharSet );
                if ( eCharSet == RTL_TEXTENCODING_DONTKNOW )
                    eCharSet = osl_getThreadTextEncoding();
                if ( eCharSet == RTL_TEXTENCODING_SYMBOL )
                    eCharSet = RTL_TEXTENCODING_MS_1252;
                aLogFont.alfFaceName = OUString( lfFaceName, strlen(lfFaceName), eCharSet );

                CreateObject(o3tl::make_unique<WinMtfFontStyle>( aLogFont ));
            }
            break;

            case W_META_CREATEBITMAPINDIRECT:
            {
                CreateObject();
            }
            break;

            case W_META_CREATEBITMAP:
            {
                CreateObject();
            }
            break;

            case W_META_CREATEREGION:
            {
                CreateObject();
            }
            break;

            case W_META_EXCLUDECLIPRECT :
            {
                ExcludeClipRect( ReadRectangle() );
            }
            break;

            case W_META_PATBLT:
            {
                sal_uInt32 nROP = 0;
                WMFRasterOp nOldROP = WMFRasterOp::NONE;
                mpWMF->ReadUInt32( nROP );
                Size aSize = ReadYXExt();
                nOldROP = SetRasterOp( (WMFRasterOp)nROP );
                DrawRect( tools::Rectangle( ReadYX(), aSize ), false );
                SetRasterOp( nOldROP );
            }
            break;

            case W_META_SELECTCLIPREGION:
            {
                sal_Int16 nObjIndex = 0;
                mpWMF->ReadInt16( nObjIndex );
                if ( !nObjIndex )
                {
                    tools::PolyPolygon aEmptyPolyPoly;
                    SetClipPath( aEmptyPolyPoly, RGN_COPY, true );
                }
            }
            break;

            case W_META_ESCAPE :
            {
                // mnRecSize has been checked previously to be greater than 3
                sal_uInt64 nMetaRecSize = static_cast< sal_uInt64 >(mnRecSize - 2 ) * 2;
                sal_uInt64 nMetaRecEndPos = mpWMF->Tell() + nMetaRecSize;

                // taking care that mnRecSize does not exceed the maximal stream position
                if ( nMetaRecEndPos > mnEndPos )
                {
                    mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                if (mnRecSize >= 4 )    // minimal escape length
                {
                    sal_uInt16  nMode = 0, nLen = 0;
                    mpWMF->ReadUInt16( nMode )
                         .ReadUInt16( nLen );
                    if ( ( nMode == W_MFCOMMENT ) && ( nLen >= 4 ) )
                    {
                        sal_uInt32 nNewMagic = 0; // we have to read int32 for
                        mpWMF->ReadUInt32( nNewMagic );   // META_ESCAPE_ENHANCED_METAFILE CommentIdentifier

                        if( nNewMagic == 0x2c2a4f4f &&  nLen >= 14 )
                        {
                            sal_uInt16 nMagic2 = 0;
                            mpWMF->ReadUInt16( nMagic2 );
                            if( nMagic2 == 0x0a ) // 2nd half of magic
                            {                     // continue with private escape
                                sal_uInt32 nCheck = 0, nEsc = 0;
                                mpWMF->ReadUInt32( nCheck )
                                     .ReadUInt32( nEsc );

                                sal_uInt32 nEscLen = nLen - 14;
                                if ( nEscLen <= (mnRecSize * 2 ) )
                                {
    #ifdef OSL_BIGENDIAN
                                    sal_uInt32 nTmp = OSL_SWAPDWORD( nEsc );
                                    sal_uInt32 nCheckSum = rtl_crc32( 0, &nTmp, 4 );
    #else
                                    sal_uInt32 nCheckSum = rtl_crc32( 0, &nEsc, 4 );
    #endif
                                    std::unique_ptr<sal_Int8[]> pData;

                                    if ( ( static_cast< sal_uInt64 >( nEscLen ) + mpWMF->Tell() ) > nMetaRecEndPos )
                                    {
                                        mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                                        break;
                                    }
                                    if ( nEscLen > 0 )
                                    {
                                        pData.reset(new sal_Int8[ nEscLen ]);
                                        mpWMF->ReadBytes(pData.get(), nEscLen);
                                        nCheckSum = rtl_crc32( nCheckSum, pData.get(), nEscLen );
                                    }
                                    if ( nCheck == nCheckSum )
                                    {
                                        switch( nEsc )
                                        {
                                            case PRIVATE_ESCAPE_UNICODE :
                                            {
                                                // we will use text instead of polygons only if we have the correct font
                                                if ( Application::GetDefaultDevice()->IsFontAvailable( GetFont().GetFamilyName() ) )
                                                {
                                                    Point  aPt;
                                                    OUString aString;
                                                    sal_uInt32  nStringLen, nDXCount;
                                                    std::unique_ptr<long[]> pDXAry;
                                                    SvMemoryStream aMemoryStream( nEscLen );
                                                    aMemoryStream.WriteBytes(pData.get(), nEscLen);
                                                    aMemoryStream.Seek( STREAM_SEEK_TO_BEGIN );
                                                    sal_Int32 nTmpX(0), nTmpY(0);
                                                    aMemoryStream.ReadInt32( nTmpX )
                                                                 .ReadInt32( nTmpY )
                                                                 .ReadUInt32( nStringLen );
                                                     aPt.X() = nTmpX;
                                                     aPt.Y() = nTmpY;

                                                    if ( ( static_cast< sal_uInt64 >( nStringLen ) * sizeof( sal_Unicode ) ) < ( nEscLen - aMemoryStream.Tell() ) )
                                                    {

                                                        aString = read_uInt16s_ToOUString(aMemoryStream, nStringLen);
                                                        aMemoryStream.ReadUInt32( nDXCount );
                                                        if ( ( static_cast< sal_uInt64 >( nDXCount ) * sizeof( sal_Int32 ) ) >= ( nEscLen - aMemoryStream.Tell() ) )
                                                            nDXCount = 0;
                                                        if ( nDXCount )
                                                            pDXAry.reset(new long[ nDXCount ]);
                                                        for  (sal_uInt32 i = 0; i < nDXCount; i++ )
                                                        {
                                                            sal_Int32 val;
                                                            aMemoryStream.ReadInt32( val);
                                                            pDXAry[ i ] = val;
                                                        }
                                                        aMemoryStream.ReadUInt32(mnSkipActions);
                                                        DrawText( aPt, aString, pDXAry.get() );
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else if ( (nNewMagic == static_cast< sal_uInt32 >(0x43464D57)) && (nLen >= 34) && ( (sal_Int32)(nLen + 10) <= (sal_Int32)(mnRecSize * 2) ))
                        {
                            sal_uInt32 nComType = 0, nVersion = 0, nFlags = 0, nComRecCount = 0,
                                       nCurRecSize = 0, nRemainingSize = 0, nEMFTotalSize = 0;
                            sal_uInt16 nCheck = 0;

                            mpWMF->ReadUInt32( nComType ).ReadUInt32( nVersion ).ReadUInt16( nCheck ).ReadUInt32( nFlags )
                                 .ReadUInt32( nComRecCount ).ReadUInt32( nCurRecSize )
                                 .ReadUInt32( nRemainingSize ).ReadUInt32( nEMFTotalSize ); // the nRemainingSize is not mentioned in MSDN documentation
                                                                      // but it seems to be required to read in data produced by OLE

                            if( nComType == 0x01 && nVersion == 0x10000 && nComRecCount )
                            {
                                if( !mnEMFRec)
                                {   // first EMF comment
                                    mnEMFRecCount = nComRecCount;
                                    mnEMFSize = nEMFTotalSize;
                                    if (mnEMFSize > mpWMF->remainingSize())
                                    {
                                        SAL_WARN("vcl.wmf", "emf size claims to be larger than remaining data");
                                        mpEMFStream.reset();
                                    }
                                    else
                                        mpEMFStream = o3tl::make_unique<SvMemoryStream>(mnEMFSize, 0);
                                }
                                else if( (mnEMFRecCount != nComRecCount ) || (mnEMFSize != nEMFTotalSize ) ) // add additional checks here
                                {
                                    // total records should be the same as in previous comments
                                    mnEMFRecCount = 0xFFFFFFFF;
                                    mpEMFStream.reset();
                                }
                                mnEMFRec++;

                                if (mpEMFStream && nCurRecSize + 34 > nLen)
                                {
                                    mnEMFRecCount = 0xFFFFFFFF;
                                    mpEMFStream.reset();
                                }

                                if (mpEMFStream && nCurRecSize > mpWMF->remainingSize())
                                {
                                    SAL_WARN("vcl.wmf", "emf record size claims to be larger than remaining data");
                                    mnEMFRecCount = 0xFFFFFFFF;
                                    mpEMFStream.reset();
                                }

                                if (mpEMFStream)
                                {
                                    std::vector<sal_Int8> aBuf(nCurRecSize);
                                    sal_uInt32 nCount = mpWMF->ReadBytes(aBuf.data(), nCurRecSize);
                                    if( nCount == nCurRecSize )
                                        mpEMFStream->WriteBytes(aBuf.data(), nCount);
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

    static const long   aMaxWidth = 1024;

    bool WmfReader::ReadHeader()
    {
        sal_uInt64 const nStrmPos = mpWMF->Tell();

        sal_uInt32 nPlaceableMetaKey(0);
        // if available read the METAFILEHEADER
        mpWMF->ReadUInt32( nPlaceableMetaKey );
        if (!mpWMF->good())
            return false;

        tools::Rectangle aPlaceableBound;

        bool bPlaceable = nPlaceableMetaKey == 0x9ac6cdd7L;

        SAL_INFO("vcl.wmf", "Placeable: \"" << (bPlaceable ? "yes" : "no") << "\"");

        if (bPlaceable)
        {
            //TODO do some real error handling here
            sal_Int16 nVal;

            // Skip reserved bytes
            mpWMF->SeekRel(2);

            // BoundRect
            mpWMF->ReadInt16( nVal );
            aPlaceableBound.Left() = nVal;
            mpWMF->ReadInt16( nVal );
            aPlaceableBound.Top() = nVal;
            mpWMF->ReadInt16( nVal );
            aPlaceableBound.Right() = nVal;
            mpWMF->ReadInt16( nVal );
            aPlaceableBound.Bottom() = nVal;

            // inch
            mpWMF->ReadUInt16( mnUnitsPerInch );

            // reserved
            mpWMF->SeekRel( 4 );

            // Skip and don't check the checksum
            mpWMF->SeekRel( 2 );
        }
        else
        {
            mnUnitsPerInch = 96;


            mpWMF->Seek( nStrmPos + 18 );    // set the streampos to the start of the metaactions
            GetPlaceableBound( aPlaceableBound, mpWMF );

            // The image size is not known so normalize the calculated bounds so that the
            // resulting image is not too big
            const double fMaxWidth = static_cast<double>(aMaxWidth);
            if (aPlaceableBound.GetWidth() > aMaxWidth)
            {
                double fRatio = aPlaceableBound.GetWidth() / fMaxWidth;

                aPlaceableBound = tools::Rectangle(
                                    aPlaceableBound.Left()   / fRatio,
                                    aPlaceableBound.Top()    / fRatio,
                                    aPlaceableBound.Right()  / fRatio,
                                    aPlaceableBound.Bottom() / fRatio);

                SAL_INFO("vcl.wmf", "Placeable bounds "
                        " t: " << aPlaceableBound.Left()  << " l: " << aPlaceableBound.Top()
                    << " b: " << aPlaceableBound.Right() << " r: " << aPlaceableBound.Bottom());
            }

            mpWMF->Seek( nStrmPos );
        }

        SetWinOrg( aPlaceableBound.TopLeft() );
        Size aWMFSize( labs( aPlaceableBound.GetWidth() ), labs( aPlaceableBound.GetHeight() ) );
        SetWinExt( aWMFSize );

        SAL_INFO("vcl.wmf", "WMF size  w: " << aWMFSize.Width()    << " h: " << aWMFSize.Height());

        Size aDevExt( 10000, 10000 );
        if( ( labs( aWMFSize.Width() ) > 1 ) && ( labs( aWMFSize.Height() ) > 1 ) )
        {
            const Fraction  aFrac( 1, mnUnitsPerInch);
            MapMode         aWMFMap( MapUnit::MapInch, Point(), aFrac, aFrac );
            Size            aSize100( OutputDevice::LogicToLogic( aWMFSize, aWMFMap, MapUnit::Map100thMM ) );
            aDevExt = Size( labs( aSize100.Width() ), labs( aSize100.Height() ) );
        }
        SetDevExt( aDevExt );

        SAL_INFO("vcl.wmf", "Dev size  w: " << aDevExt.Width()    << " h: " << aDevExt.Height());

        // read the METAHEADER
        sal_uInt32 nMetaKey(0);
        mpWMF->ReadUInt32( nMetaKey ); // type and headersize
        if (!mpWMF->good())
            return false;
        if (nMetaKey != 0x00090001)
        {
            sal_uInt16 aNextWord(0);
            mpWMF->ReadUInt16( aNextWord );
            if (nMetaKey != 0x10000 || aNextWord != 0x09)
            {
                mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                return false;
            }
        }

        mpWMF->SeekRel( 2 ); // Version (of Windows)
        mpWMF->SeekRel( 4 ); // Size (of file in words)
        mpWMF->SeekRel( 2 ); // NoObjects (maximum number of simultaneous objects)
        mpWMF->SeekRel( 4 ); // MaxRecord (size of largest record in words)
        mpWMF->SeekRel( 2 ); // NoParameters (Unused

        return mpWMF->good();
    }

    void WmfReader::ReadWMF()
    {
        sal_uInt16  nFunction;
        sal_uLong   nPos;

        mnSkipActions = 0;
        mnCurrentAction = 0;

        mpEMFStream.reset();
        mnEMFRecCount = 0;
        mnEMFRec = 0;
        mnEMFSize = 0;

        SetMapMode( MM_ANISOTROPIC );
        SetWinOrg( Point() );
        SetWinExt( Size( 1, 1 ) );
        SetDevExt( Size( 10000, 10000 ) );

        mnEndPos=mpWMF->Seek( STREAM_SEEK_TO_END );
        mpWMF->Seek( mnStartPos );

        if ( ReadHeader( ) )
        {
            nPos = mpWMF->Tell();

            if( mnEndPos - mnStartPos )
            {
               bool bEMFAvailable = false;
                while( true )
                {
                    mnCurrentAction++;
                    mpWMF->ReadUInt32(mnRecSize).ReadUInt16( nFunction );

                    if(  mpWMF->GetError()
                      || (mnRecSize < 3 )
                      || (mnRecSize == 3
                         && nFunction == 0
                         )
                      || mpWMF->IsEof()
                      )
                    {
                        if( mpWMF->IsEof() )
                            mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );

                        break;
                    }
                    if ( !bEMFAvailable )
                    {
                        if(   !maBmpSaveList.empty()
                          && ( nFunction != W_META_STRETCHDIB    )
                          && ( nFunction != W_META_DIBBITBLT     )
                          && ( nFunction != W_META_DIBSTRETCHBLT )
                          )
                        {
                            ResolveBitmapActions( maBmpSaveList );
                        }

                        if ( !mnSkipActions)
                            ReadRecordParams( nFunction );
                        else
                            mnSkipActions--;

                        if(mpEMFStream && mnEMFRecCount == mnEMFRec)
                        {
                            GDIMetaFile aMeta;
                            mpEMFStream->Seek( 0 );
                            std::unique_ptr<EmfReader> pEMFReader(o3tl::make_unique<EmfReader>( *mpEMFStream, aMeta ));
                            bEMFAvailable = pEMFReader->ReadEnhWMF();
                            pEMFReader.reset(); // destroy first!!!

                            if( bEMFAvailable )
                            {
                                AddFromGDIMetaFile( aMeta );
                                SetrclFrame( tools::Rectangle( Point(0, 0), aMeta.GetPrefSize()));

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
                                mpEMFStream.reset();
                            }
                        }
                    }

                    const sal_uInt32 nAvailableBytes = mnEndPos - nPos;
                    const sal_uInt32 nMaxPossibleRecordSize = nAvailableBytes/2;

                    if (mnRecSize <= nMaxPossibleRecordSize)
                    {
                        nPos += mnRecSize * 2;
                        mpWMF->Seek(nPos);
                    }
                    else
                        mpWMF->SetError( SVSTREAM_FILEFORMAT_ERROR );
                }
            }
            else
                mpWMF->SetError( SVSTREAM_GENERALERROR );

            if( !mpWMF->GetError() && !maBmpSaveList.empty() )
                ResolveBitmapActions( maBmpSaveList );
        }
        if ( mpWMF->GetError() )
            mpWMF->Seek( mnStartPos );
    }

    void WmfReader::GetPlaceableBound( tools::Rectangle& rPlaceableBound, SvStream* pStm )
    {
        bool bRet = true;

        tools::Rectangle aBound;
        aBound.Left()   = RECT_MAX;
        aBound.Top()    = RECT_MAX;
        aBound.Right()  = RECT_MIN;
        aBound.Bottom() = RECT_MIN;
        bool bBoundsDetermined = false;

        sal_uInt32 nPos = pStm->Tell();
        sal_uInt32 nEnd = pStm->Seek( STREAM_SEEK_TO_END );

        pStm->Seek( nPos );

        Point aWinOrg(0,0);
        boost::optional<Size>  aWinExt;

        Point aViewportOrg(0,0);
        boost::optional<Size>  aViewportExt;

        if( nEnd - nPos )
        {
            sal_Int16 nMapMode = MM_ANISOTROPIC;
            sal_uInt16 nFunction;
            sal_uInt32 nRSize;

            while( bRet )
            {
                pStm->ReadUInt32( nRSize ).ReadUInt16( nFunction );

                if( pStm->GetError() )
                {
                    bRet = false;
                    break;
                }
                else if ( nRSize==3 && nFunction==0 )
                {
                    break;
                }
                else if ( nRSize < 3 || pStm->IsEof() )
                {
                    pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    bRet = false;
                    break;
                }
                switch( nFunction )
                {
                    case W_META_SETWINDOWORG:
                    {
                        aWinOrg = ReadYX();
                    }
                    break;

                    case W_META_SETWINDOWEXT:
                    {
                        sal_Int16 nWidth(0), nHeight(0);
                        pStm->ReadInt16(nHeight);
                        pStm->ReadInt16(nWidth);
                        aWinExt = Size(nWidth, nHeight);
                    }
                    break;

                    case W_META_SETVIEWPORTORG:
                    {
                        aViewportOrg = ReadYX();
                    }
                    break;

                    case W_META_SETVIEWPORTEXT:
                    {
                        sal_Int16 nWidth(0), nHeight(0);
                        pStm->ReadInt16(nHeight);
                        pStm->ReadInt16(nWidth);
                        aViewportExt = Size(nWidth, nHeight);
                    }
                    break;

                    case W_META_SETMAPMODE :
                        pStm->ReadInt16( nMapMode );
                    break;

                    case W_META_MOVETO:
                    case W_META_LINETO:
                        GetWinExtMax( ReadYX(), aBound, nMapMode );
                        bBoundsDetermined = true;
                    break;

                    case W_META_RECTANGLE:
                    case W_META_INTERSECTCLIPRECT:
                    case W_META_EXCLUDECLIPRECT :
                    case W_META_ELLIPSE:
                        GetWinExtMax( ReadRectangle(), aBound, nMapMode );
                        bBoundsDetermined = true;
                    break;

                    case W_META_ROUNDRECT:
                        ReadYXExt(); // size
                        GetWinExtMax( ReadRectangle(), aBound, nMapMode );
                        bBoundsDetermined = true;
                    break;

                    case W_META_ARC:
                    case W_META_PIE:
                    case W_META_CHORD:
                        ReadYX(); // end
                        ReadYX(); // start
                        GetWinExtMax( ReadRectangle(), aBound, nMapMode );
                        bBoundsDetermined = true;
                    break;

                    case W_META_POLYGON:
                    {
                        bool bRecordOk = true;

                        sal_uInt16 nPoints(0);
                        pStm->ReadUInt16( nPoints );

                        if (nPoints > pStm->remainingSize() / (2 * sizeof(sal_uInt16)))
                        {
                            bRecordOk = false;
                        }
                        else
                        {
                            for(sal_uInt16 i = 0; i < nPoints; i++ )
                            {
                                GetWinExtMax( ReadPoint(), aBound, nMapMode );
                                bBoundsDetermined = true;
                            }
                        }

                        SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polyline record claimed more points than the stream can provide");

                        if (!bRecordOk)
                        {
                            pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                            bRet = false;
                            break;
                        }
                    }
                    break;

                    case W_META_POLYPOLYGON:
                    {
                        bool bRecordOk = true;
                        sal_uInt16 nPoly(0), nPoints(0);
                        pStm->ReadUInt16(nPoly);
                        if (nPoly > pStm->remainingSize() / sizeof(sal_uInt16))
                        {
                            bRecordOk = false;
                        }
                        else
                        {
                            for(sal_uInt16 i = 0; i < nPoly; i++ )
                            {
                                sal_uInt16 nP = 0;
                                pStm->ReadUInt16( nP );
                                if (nP > SAL_MAX_UINT16 - nPoints)
                                {
                                    bRecordOk = false;
                                    break;
                                }
                                nPoints += nP;
                            }
                        }

                        SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polypolygon record has more polygons than we can handle");

                        bRecordOk = bRecordOk && pStm->good();

                        if (!bRecordOk)
                        {
                            pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                            bRet = false;
                            break;
                        }

                        if (nPoints > pStm->remainingSize() / (2 * sizeof(sal_uInt16)))
                        {
                            bRecordOk = false;
                        }
                        else
                        {
                            for (sal_uInt16 i = 0; i < nPoints; i++ )
                            {
                                GetWinExtMax( ReadPoint(), aBound, nMapMode );
                                bBoundsDetermined = true;
                            }
                        }

                        SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polypolygon record claimed more points than the stream can provide");

                        bRecordOk &= pStm->good();

                        if (!bRecordOk)
                        {
                            pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                            bRet = false;
                            break;
                        }
                    }
                    break;

                    case W_META_POLYLINE:
                    {
                        bool bRecordOk = true;

                        sal_uInt16 nPoints(0);
                        pStm->ReadUInt16(nPoints);
                        if (nPoints > pStm->remainingSize() / (2 * sizeof(sal_uInt16)))
                        {
                            bRecordOk = false;
                        }
                        else
                        {
                            for (sal_uInt16 i = 0; i < nPoints; ++i)
                            {
                                GetWinExtMax( ReadPoint(), aBound, nMapMode );
                                bBoundsDetermined = true;
                            }
                        }

                        SAL_WARN_IF(!bRecordOk, "vcl.wmf", "polyline record claimed more points than the stream can provide");

                        if (!bRecordOk)
                        {
                            pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                            bRet = false;
                            break;
                        }
                    }
                    break;

                    case W_META_SETPIXEL:
                    {
                        ReadColor();
                        GetWinExtMax( ReadYX(), aBound, nMapMode );
                        bBoundsDetermined = true;
                    }
                    break;

                    case W_META_TEXTOUT:
                    {
                        sal_uInt16 nLength;
                        pStm->ReadUInt16( nLength );
                        // todo: we also have to take care of the text width
                        if ( nLength )
                        {
                            pStm->SeekRel( ( nLength + 1 ) &~ 1 );
                            GetWinExtMax( ReadYX(), aBound, nMapMode );
                            bBoundsDetermined = true;
                        }
                    }
                    break;

                    case W_META_EXTTEXTOUT:
                    {
                        sal_uInt16  nLen, nOptions;
                        Point       aPosition;

                        aPosition = ReadYX();
                        pStm->ReadUInt16( nLen ).ReadUInt16( nOptions );
                        // todo: we also have to take care of the text width
                        if( nLen )
                        {
                            GetWinExtMax( aPosition, aBound, nMapMode );
                            bBoundsDetermined = true;
                        }
                    }
                    break;
                    case W_META_BITBLT:
                    case W_META_STRETCHBLT:
                    case W_META_DIBBITBLT:
                    case W_META_DIBSTRETCHBLT:
                    case W_META_STRETCHDIB:
                    {
                        sal_Int32   nWinROP;
                        sal_uInt16  nSx, nSy, nUsage;
                        pStm->ReadInt32( nWinROP );

                        if( nFunction == W_META_STRETCHDIB )
                            pStm->ReadUInt16( nUsage );

                        // nSye and nSxe is the number of pixels that has to been used
                        if( nFunction == W_META_STRETCHDIB || nFunction == W_META_STRETCHBLT || nFunction == W_META_DIBSTRETCHBLT )
                        {
                            sal_uInt16 nSxe, nSye;
                            pStm->ReadUInt16( nSye ).ReadUInt16( nSxe );
                        }

                        // nSy and nx is the offset of the first pixel
                        pStm->ReadUInt16( nSy ).ReadUInt16( nSx );

                        if( nFunction == W_META_STRETCHDIB || nFunction == W_META_DIBBITBLT || nFunction == W_META_DIBSTRETCHBLT )
                        {
                            if ( nWinROP == PATCOPY )
                                pStm->ReadUInt16( nUsage );    // i don't know anything of this parameter, so its called nUsage
                                                    // DrawRect( Rectangle( ReadYX(), aDestSize ), false );

                            Size aDestSize( ReadYXExt() );
                            if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps
                            {
                                tools::Rectangle aDestRect( ReadYX(), aDestSize );
                                GetWinExtMax( aDestRect, aBound, nMapMode );
                                bBoundsDetermined = true;
                            }
                        }
                    }
                    break;

                    case W_META_PATBLT:
                    {
                        sal_uInt32 nROP;
                        pStm->ReadUInt32( nROP );
                        Size aSize = ReadYXExt();
                        GetWinExtMax( tools::Rectangle( ReadYX(), aSize ), aBound, nMapMode );
                        bBoundsDetermined = true;
                    }
                    break;
                }

                const sal_uInt32 nAvailableBytes = nEnd - nPos;
                const sal_uInt32 nMaxPossibleRecordSize = nAvailableBytes/2;
                if (nRSize <= nMaxPossibleRecordSize)
                {
                    nPos += nRSize * 2;
                    pStm->Seek( nPos );
                }
                else
                {
                    pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    bRet = false;
                }
            }
        }
        else
        {
            pStm->SetError( SVSTREAM_GENERALERROR );
            bRet = false;
        }

        if (bRet)
        {
            if (aWinExt)
            {
                rPlaceableBound = tools::Rectangle(aWinOrg, *aWinExt);
                SAL_INFO("vcl.wmf", "Window dimension "
                           " t: " << rPlaceableBound.Left()  << " l: " << rPlaceableBound.Top()
                        << " b: " << rPlaceableBound.Right() << " r: " << rPlaceableBound.Bottom());
            }
            else if (aViewportExt)
            {
                rPlaceableBound = tools::Rectangle(aViewportOrg, *aViewportExt);
                SAL_INFO("vcl.wmf", "Viewport dimension "
                           " t: " << rPlaceableBound.Left()  << " l: " << rPlaceableBound.Top()
                        << " b: " << rPlaceableBound.Right() << " r: " << rPlaceableBound.Bottom());
            }
            else if (bBoundsDetermined)
            {
                rPlaceableBound = aBound;
                SAL_INFO("vcl.wmf", "Determined dimension "
                           " t: " << rPlaceableBound.Left()  << " l: " << rPlaceableBound.Top()
                        << " b: " << rPlaceableBound.Right() << " r: " << rPlaceableBound.Bottom());
            }
            else
            {
                rPlaceableBound.Left() = 0;
                rPlaceableBound.Top() = 0;
                rPlaceableBound.Right() = aMaxWidth;
                rPlaceableBound.Bottom() = aMaxWidth;
                SAL_INFO("vcl.wmf", "Default dimension "
                           " t: " << rPlaceableBound.Left()  << " l: " << rPlaceableBound.Top()
                        << " b: " << rPlaceableBound.Right() << " r: " << rPlaceableBound.Bottom());
            }
        }
    }

    WmfReader::WmfReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile)
        : MtfTools(rGDIMetaFile, rStreamWMF)
        , mnUnitsPerInch(96)
        , mnRecSize(0)
        , mpEMFStream()
        , mnEMFRecCount(0)
        , mnEMFRec(0)
        , mnEMFSize(0)
        , mnSkipActions(0)
        , mnCurrentAction(0)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
