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

#include <cstdlib>
#include <memory>
#include <optional>
#include <o3tl/safeint.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/crc.h>
#include <rtl/tencinfo.h>
#include <sal/log.hxx>
#include <osl/endian.h>
#include <vcl/gdimtf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/wmfexternal.hxx>
#include <tools/fract.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/BitmapTools.hxx>
#include <osl/thread.h>

namespace
{
    // MS Windows defines
    enum WMFRecords
    {
        W_META_EOF                  = 0x0000,
        W_META_SETBKCOLOR           = 0x0201,
        W_META_SETBKMODE            = 0x0102,
        W_META_SETMAPMODE           = 0x0103,
        W_META_SETROP2              = 0x0104,
        W_META_SETRELABS            = 0x0105,
        W_META_SETPOLYFILLMODE      = 0x0106,
        W_META_SETSTRETCHBLTMODE    = 0x0107,
        W_META_SETTEXTCHAREXTRA     = 0x0108,
        W_META_SETTEXTCOLOR         = 0x0209,
        W_META_SETTEXTJUSTIFICATION = 0x020A,
        W_META_SETWINDOWORG         = 0x020B,
        W_META_SETWINDOWEXT         = 0x020C,
        W_META_SETVIEWPORTORG       = 0x020D,
        W_META_SETVIEWPORTEXT       = 0x020E,
        W_META_OFFSETWINDOWORG      = 0x020F,
        W_META_SCALEWINDOWEXT       = 0x0410,
        W_META_OFFSETVIEWPORTORG    = 0x0211,
        W_META_SCALEVIEWPORTEXT     = 0x0412,
        W_META_LINETO               = 0x0213,
        W_META_MOVETO               = 0x0214,
        W_META_EXCLUDECLIPRECT      = 0x0415,
        W_META_INTERSECTCLIPRECT    = 0x0416,
        W_META_ARC                  = 0x0817,
        W_META_ELLIPSE              = 0x0418,
        W_META_FLOODFILL            = 0x0419,
        W_META_PIE                  = 0x081A,
        W_META_RECTANGLE            = 0x041B,
        W_META_ROUNDRECT            = 0x061C,
        W_META_PATBLT               = 0x061D,
        W_META_SAVEDC               = 0x001E,
        W_META_SETPIXEL             = 0x041F,
        W_META_OFFSETCLIPRGN        = 0x0220,
        W_META_TEXTOUT              = 0x0521,
        W_META_BITBLT               = 0x0922,
        W_META_STRETCHBLT           = 0x0B23,
        W_META_POLYGON              = 0x0324,
        W_META_POLYLINE             = 0x0325,
        W_META_ESCAPE               = 0x0626,
        W_META_RESTOREDC            = 0x0127,
        W_META_FILLREGION           = 0x0228,
        W_META_FRAMEREGION          = 0x0429,
        W_META_INVERTREGION         = 0x012A,
        W_META_PAINTREGION          = 0x012B,
        W_META_SELECTCLIPREGION     = 0x012C,
        W_META_SELECTOBJECT         = 0x012D,
        W_META_SETTEXTALIGN         = 0x012E,
        W_META_DRAWTEXT             = 0x062F,
        W_META_CHORD                = 0x0830,
        W_META_SETMAPPERFLAGS       = 0x0231,
        W_META_EXTTEXTOUT           = 0x0a32,
        W_META_SETDIBTODEV          = 0x0d33,
        W_META_SELECTPALETTE        = 0x0234,
        W_META_REALIZEPALETTE       = 0x0035,
        W_META_ANIMATEPALETTE       = 0x0436,
        W_META_SETPALENTRIES        = 0x0037,
        W_META_POLYPOLYGON          = 0x0538,
        W_META_RESIZEPALETTE        = 0x0139,
        W_META_DIBBITBLT            = 0x0940,
        W_META_DIBSTRETCHBLT        = 0x0b41,
        W_META_DIBCREATEPATTERNBRUSH = 0x0142,
        W_META_STRETCHDIB           = 0x0f43,
        W_META_EXTFLOODFILL         = 0x0548,
        W_META_RESETDC              = 0x014C,
        W_META_STARTDOC             = 0x014D,
        W_META_STARTPAGE            = 0x004F,
        W_META_ENDPAGE              = 0x0050,
        W_META_ABORTDOC             = 0x0052,
        W_META_ENDDOC               = 0x005E,
        W_META_DELETEOBJECT         = 0x01f0,
        W_META_CREATEPALETTE        = 0x00f7,
        W_META_CREATEBRUSH          = 0x00F8,
        W_META_CREATEPATTERNBRUSH   = 0x01F9,
        W_META_CREATEPENINDIRECT    = 0x02FA,
        W_META_CREATEFONTINDIRECT   = 0x02FB,
        W_META_CREATEBRUSHINDIRECT  = 0x02FC,
        W_META_CREATEBITMAPINDIRECT = 0x02FD,
        W_META_CREATEBITMAP         = 0x06FE,
        W_META_CREATEREGION         = 0x06FF
    };

    void GetWinExtMax(const Point& rSource, tools::Rectangle& rPlaceableBound, const sal_Int16 nMapMode)
    {
        Point aSource(rSource);
        if (nMapMode == MM_HIMETRIC)
            aSource.setY( -rSource.Y() );
        if (aSource.X() < rPlaceableBound.Left())
            rPlaceableBound.SetLeft( aSource.X() );
        if (aSource.X() > rPlaceableBound.Right())
            rPlaceableBound.SetRight( aSource.X() );
        if (aSource.Y() < rPlaceableBound.Top())
            rPlaceableBound.SetTop( aSource.Y() );
        if (aSource.Y() > rPlaceableBound.Bottom())
            rPlaceableBound.SetBottom( aSource.Y() );
    }

    void GetWinExtMax(const tools::Rectangle& rSource, tools::Rectangle& rPlaceableBound, const sal_Int16 nMapMode)
    {
        GetWinExtMax(rSource.TopLeft(), rPlaceableBound, nMapMode);
        GetWinExtMax(rSource.BottomRight(), rPlaceableBound, nMapMode);
    }

    const char *
    record_type_name(sal_uInt16 nRecType)
    {
    #ifndef SAL_LOG_INFO
        (void) nRecType;
        return "";
    #else
        switch( nRecType )
        {
        case W_META_SETBKCOLOR: return "META_SETBKCOLOR";
        case W_META_SETBKMODE: return "META_SETBKMODE";
        case W_META_SETMAPMODE: return "META_SETMAPMODE";
        case W_META_SETROP2: return "META_SETROP2";
        case W_META_SETRELABS: return "META_SETRELABS";
        case W_META_SETPOLYFILLMODE: return "META_SETPOLYFILLMODE";
        case W_META_SETSTRETCHBLTMODE: return "META_SETSTRETCHBLTMODE";
        case W_META_SETTEXTCHAREXTRA: return "META_SETTEXTCHAREXTRA";
        case W_META_SETTEXTCOLOR: return "META_SETTEXTCOLOR";
        case W_META_SETTEXTJUSTIFICATION: return "META_SETTEXTJUSTIFICATION";
        case W_META_SETWINDOWORG: return "META_SETWINDOWORG";
        case W_META_SETWINDOWEXT: return "META_SETWINDOWEXT";
        case W_META_SETVIEWPORTORG: return "META_SETVIEWPORTORG";
        case W_META_SETVIEWPORTEXT: return "META_SETVIEWPORTEXT";
        case W_META_OFFSETWINDOWORG: return "META_OFFSETWINDOWORG";
        case W_META_SCALEWINDOWEXT: return "META_SCALEWINDOWEXT";
        case W_META_OFFSETVIEWPORTORG: return "META_OFFSETVIEWPORTORG";
        case W_META_SCALEVIEWPORTEXT: return "META_SCALEVIEWPORTEXT";
        case W_META_LINETO: return "META_LINETO";
        case W_META_MOVETO: return "META_MOVETO";
        case W_META_EXCLUDECLIPRECT: return "META_EXCLUDECLIPRECT";
        case W_META_INTERSECTCLIPRECT: return "META_INTERSECTCLIPRECT";
        case W_META_ARC: return "META_ARC";
        case W_META_ELLIPSE: return "META_ELLIPSE";
        case W_META_FLOODFILL: return "META_FLOODFILL";
        case W_META_PIE: return "META_PIE";
        case W_META_RECTANGLE: return "META_RECTANGLE";
        case W_META_ROUNDRECT: return "META_ROUNDRECT";
        case W_META_PATBLT: return "META_PATBLT";
        case W_META_SAVEDC: return "META_SAVEDC";
        case W_META_SETPIXEL: return "META_SETPIXEL";
        case W_META_OFFSETCLIPRGN: return "META_OFFSETCLIPRGN";
        case W_META_TEXTOUT: return "META_TEXTOUT";
        case W_META_BITBLT: return "META_BITBLT";
        case W_META_STRETCHBLT: return "META_STRETCHBLT";
        case W_META_POLYGON: return "META_POLYGON";
        case W_META_POLYLINE: return "META_POLYLINE";
        case W_META_ESCAPE: return "META_ESCAPE";
        case W_META_RESTOREDC: return "META_RESTOREDC";
        case W_META_FILLREGION: return "META_FILLREGION";
        case W_META_FRAMEREGION: return "META_FRAMEREGION";
        case W_META_INVERTREGION: return "META_INVERTREGION";
        case W_META_PAINTREGION: return "META_PAINTREGION";
        case W_META_SELECTCLIPREGION: return "META_SELECTCLIPREGION";
        case W_META_SELECTOBJECT: return "META_SELECTOBJECT";
        case W_META_SETTEXTALIGN: return "META_SETTEXTALIGN";
        case W_META_DRAWTEXT: return "META_DRAWTEXT";
        case W_META_CHORD: return "META_CHORD";
        case W_META_SETMAPPERFLAGS: return "META_SETMAPPERFLAGS";
        case W_META_EXTTEXTOUT: return "META_EXTTEXTOUT";
        case W_META_SETDIBTODEV: return "META_SETDIBTODEV";
        case W_META_SELECTPALETTE: return "META_SELECTPALETTE";
        case W_META_REALIZEPALETTE: return "META_REALIZEPALETTE";
        case W_META_ANIMATEPALETTE: return "META_ANIMATEPALETTE";
        case W_META_SETPALENTRIES: return "META_SETPALENTRIES";
        case W_META_POLYPOLYGON: return "META_POLYPOLYGON";
        case W_META_RESIZEPALETTE: return "META_RESIZEPALETTE";
        case W_META_DIBBITBLT: return "META_DIBBITBLT";
        case W_META_DIBSTRETCHBLT: return "META_DIBSTRETCHBLT";
        case W_META_DIBCREATEPATTERNBRUSH: return "META_DIBCREATEPATTERNBRUSH";
        case W_META_STRETCHDIB: return "META_STRETCHDIB";
        case W_META_EXTFLOODFILL: return "META_EXTFLOODFILL";
        case W_META_RESETDC: return "META_RESETDC";
        case W_META_STARTDOC: return "META_STARTDOC";
        case W_META_STARTPAGE: return "META_STARTPAGE";
        case W_META_ENDPAGE: return "META_ENDPAGE";
        case W_META_ABORTDOC: return "META_ABORTDOC";
        case W_META_ENDDOC: return "META_ENDDOC";
        case W_META_DELETEOBJECT: return "META_DELETEOBJECT";
        case W_META_CREATEPALETTE: return "META_CREATEPALETTE";
        case W_META_CREATEBRUSH: return "META_CREATEBRUSH";
        case W_META_CREATEPATTERNBRUSH: return "META_CREATEPATTERNBRUSH";
        case W_META_CREATEPENINDIRECT: return "META_CREATEPENINDIRECT";
        case W_META_CREATEFONTINDIRECT: return "META_CREATEFONTINDIRECT";
        case W_META_CREATEBRUSHINDIRECT: return "META_CREATEBRUSHINDIRECT";
        case W_META_CREATEBITMAPINDIRECT: return "META_CREATEBITMAPINDIRECT";
        case W_META_CREATEBITMAP: return "META_CREATEBITMAP";
        case W_META_CREATEREGION: return "META_CREATEREGION";
        default:
            // Yes, return a pointer to a static buffer. This is a very
            // local debugging output function, so no big deal.
            static char buffer[11];
            sprintf(buffer, "0x%08" SAL_PRIxUINT32, sal_uInt32(nRecType));
            return buffer;
        }
    #endif
    }

}

namespace emfio
{
    inline Point WmfReader::ReadPoint()
    {
        short nX = 0, nY = 0;
        mpInputStream->ReadInt16( nX ).ReadInt16( nY );
        return Point( nX, nY );
    }

    inline Point WmfReader::ReadYX()
    {
        short nX = 0, nY = 0;
        mpInputStream->ReadInt16( nY ).ReadInt16( nX );
        return Point( nX, nY );
    }

    tools::Rectangle WmfReader::ReadRectangle()
    {
        Point aBR, aTL;
        aBR = ReadYX();
        aTL = ReadYX();
        aBR.AdjustX( -1 );
        aBR.AdjustY( -1 );
        if (aTL.X() > aBR.X() || aTL.Y() > aBR.Y())
        {
            SAL_WARN("emfio", "broken rectangle");
            return tools::Rectangle::Justify(aTL, aBR);
        }
        return tools::Rectangle( aTL, aBR );
    }

    Size WmfReader::ReadYXExt()
    {
        short nW=0, nH=0;
        mpInputStream->ReadInt16( nH ).ReadInt16( nW );
        return Size( nW, nH );
    }

    void WmfReader::ReadRecordParams( sal_uInt32 nRecordSize, sal_uInt16 nFunc )
    {
        SAL_INFO("emfio", "\t" << record_type_name(nFunc));
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
                mpInputStream->ReadUInt16( nDat );
                SetBkMode( static_cast<BkMode>(nDat) );
            }
            break;

            // !!!
            case W_META_SETMAPMODE:
            {
                sal_Int16 nMapMode = 0;
                mpInputStream->ReadInt16( nMapMode );
                SetMapMode( nMapMode );
            }
            break;

            case W_META_SETROP2:
            {
                sal_uInt16 nROP2 = 0;
                mpInputStream->ReadUInt16( nROP2 );
                SetRasterOp( static_cast<WMFRasterOp>(nROP2) );
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
                mpInputStream->ReadInt16( nHeight ).ReadInt16( nWidth );
                SetWinExt( Size( nWidth, nHeight ) );
            }
            break;

            case W_META_OFFSETWINDOWORG:
            {
                short nXAdd = 0, nYAdd = 0;
                mpInputStream->ReadInt16( nYAdd ).ReadInt16( nXAdd );
                SetWinOrgOffset( nXAdd, nYAdd );
            }
            break;

            case W_META_SCALEWINDOWEXT:
            {
                short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
                mpInputStream->ReadInt16( nYDenom ).ReadInt16( nYNum ).ReadInt16( nXDenom ).ReadInt16( nXNum );
                if (!nYDenom || !nXDenom)
                {
                    mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                ScaleWinExt( static_cast<double>(nXNum) / nXDenom, static_cast<double>(nYNum) / nYDenom );
            }
            break;

            case W_META_SETVIEWPORTORG:
            case W_META_SETVIEWPORTEXT:
            break;

            case W_META_OFFSETVIEWPORTORG:
            {
                short nXAdd = 0, nYAdd = 0;
                mpInputStream->ReadInt16( nYAdd ).ReadInt16( nXAdd );
                SetDevOrgOffset( nXAdd, nYAdd );
            }
            break;

            case W_META_SCALEVIEWPORTEXT:
            {
                short nXNum = 0, nXDenom = 0, nYNum = 0, nYDenom = 0;
                mpInputStream->ReadInt16( nYDenom ).ReadInt16( nYNum ).ReadInt16( nXDenom ).ReadInt16( nXNum );
                if (!nYDenom || !nXDenom)
                {
                    mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                ScaleDevExt( static_cast<double>(nXNum) / nXDenom, static_cast<double>(nYNum) / nYDenom );
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
                mpInputStream->ReadUInt16(nPoints);

                if (nPoints > mpInputStream->remainingSize() / (2 * sizeof(sal_uInt16)))
                {
                    bRecordOk = false;
                }
                else
                {
                    tools::Polygon aPoly(nPoints);
                    for (sal_uInt16 i(0); i < nPoints && mpInputStream->good(); ++i)
                        aPoly[ i ] = ReadPoint();
                    DrawPolygon(aPoly, false/*bRecordPath*/);
                }

                SAL_WARN_IF(!bRecordOk, "emfio", "polygon record has more points than we can handle");

                bRecordOk &= mpInputStream->good();

                if (!bRecordOk)
                {
                    mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
            }
            break;

            case W_META_POLYPOLYGON:
            {
                sal_uInt16 nPolyCount(0);
                // Number of polygons:
                mpInputStream->ReadUInt16( nPolyCount );
                if (nPolyCount && mpInputStream->good())
                {
                    bool bRecordOk = true;
                    if (nPolyCount > mpInputStream->remainingSize() / sizeof(sal_uInt16))
                    {
                        break;
                    }

                    // Number of points of each polygon. Determine total number of points
                    std::unique_ptr<sal_uInt16[]> xPolygonPointCounts(new sal_uInt16[nPolyCount]);
                    sal_uInt16* pnPoints = xPolygonPointCounts.get();
                    tools::PolyPolygon aPolyPoly(nPolyCount);
                    sal_uInt16 nPoints = 0;
                    for (sal_uInt16 a = 0; a < nPolyCount && mpInputStream->good(); ++a)
                    {
                        mpInputStream->ReadUInt16( pnPoints[a] );

                        if (pnPoints[a] > SAL_MAX_UINT16 - nPoints)
                        {
                            bRecordOk = false;
                            break;
                        }

                        nPoints += pnPoints[a];
                    }

                    SAL_WARN_IF(!bRecordOk, "emfio", "polypolygon record has more polygons than we can handle");

                    bRecordOk &= mpInputStream->good();

                    if (!bRecordOk)
                    {
                        mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                        break;
                    }

                    // Polygon points are:
                    for (sal_uInt16 a = 0; a < nPolyCount && mpInputStream->good(); ++a)
                    {
                        const sal_uInt16 nPointCount(pnPoints[a]);

                        if (nPointCount > mpInputStream->remainingSize() / (2 * sizeof(sal_uInt16)))
                        {
                            bRecordOk = false;
                            break;
                        }

                        std::unique_ptr<Point[]> xPolygonPoints(new Point[nPointCount]);
                        Point* pPtAry = xPolygonPoints.get();

                        for(sal_uInt16 b(0); b < nPointCount && mpInputStream->good(); ++b)
                        {
                            pPtAry[b] = ReadPoint();
                        }

                        aPolyPoly.Insert( tools::Polygon(nPointCount, pPtAry) );
                    }

                    bRecordOk &= mpInputStream->good();

                    if (!bRecordOk)
                    {
                        mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
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
                mpInputStream->ReadUInt16(nPoints);

                if (nPoints > mpInputStream->remainingSize() / (2 * sizeof(sal_uInt16)))
                {
                    bRecordOk = false;
                }
                else
                {
                    tools::Polygon aPoly(nPoints);
                    for (sal_uInt16 i(0); i < nPoints && mpInputStream->good(); ++i)
                        aPoly[ i ] = ReadPoint();
                    DrawPolyLine( aPoly );
                }

                SAL_WARN_IF(!bRecordOk, "emfio", "polyline record has more points than we can handle");

                bRecordOk &= mpInputStream->good();

                if (!bRecordOk)
                {
                    mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
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
                sal_Int16 nSavedDC(0);
                mpInputStream->ReadInt16( nSavedDC );
                SAL_INFO( "emfio", "\t\t SavedDC: " << nSavedDC );
                Pop( nSavedDC );
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
                //record is Recordsize, RecordFunction, StringLength, <String>, YStart, XStart
                const sal_uInt32 nNonStringLen = sizeof(sal_uInt32) + 4 * sizeof(sal_uInt16);
                const sal_uInt32 nRecSize = mnRecSize * 2;

                if (nRecSize < nNonStringLen)
                {
                    SAL_WARN("emfio", "W_META_TEXTOUT too short");
                    break;
                }

                sal_uInt16 nLength = 0;
                mpInputStream->ReadUInt16(nLength);
                sal_uInt16 nStoredLength = (nLength + 1) &~ 1;

                if (nRecSize - nNonStringLen < nStoredLength)
                {
                    SAL_WARN("emfio", "W_META_TEXTOUT too short, truncating string");
                    nLength = nStoredLength = nRecSize - nNonStringLen;
                }

                if (nLength)
                {
                    std::vector<char> aChars(nStoredLength);
                    nLength = std::min<sal_uInt16>(nLength, mpInputStream->ReadBytes(aChars.data(), aChars.size()));
                    OUString aText(aChars.data(), nLength, GetCharSet());
                    Point aPosition( ReadYX() );
                    DrawText( aPosition, aText );
                }
            }
            break;

            case W_META_EXTTEXTOUT:
            {
                //record is Recordsize, RecordFunction, Y, X, StringLength, options, maybe rectangle, <String>
                sal_uInt32 nNonStringLen = sizeof(sal_uInt32) + 5 * sizeof(sal_uInt16);
                const sal_uInt32 nRecSize = mnRecSize * 2;

                if (nRecSize < nNonStringLen)
                {
                    SAL_WARN("emfio", "W_META_EXTTEXTOUT too short");
                    break;
                }

                auto nRecordPos = mpInputStream->Tell() - 6;
                Point aPosition = ReadYX();
                sal_uInt16 nLen = 0, nOptions = 0;
                mpInputStream->ReadUInt16( nLen ).ReadUInt16( nOptions );
                SAL_INFO( "emfio", "\t\t\t Pos: " << aPosition.getX() << ":" << aPosition.getY() << " String Length: " << nLen << " Options: " << nOptions );
                tools::Rectangle aRect;
                if ( ( nOptions & ETO_CLIPPED ) || ( nOptions & ETO_OPAQUE ) )
                {
                    nNonStringLen += 2 * sizeof(sal_uInt16);

                    if (nRecSize < nNonStringLen)
                    {
                        SAL_WARN("emfio", "W_META_TEXTOUT too short");
                        break;
                    }
                    const Point aTopLeft = ReadPoint();
                    const Point aBottomRight = ReadPoint();
                    aRect = tools::Rectangle( aTopLeft, aBottomRight );
                    if ( nOptions & ETO_OPAQUE )
                        DrawRectWithBGColor( aRect );
                    SAL_INFO( "emfio", "\t\t\t Rectangle : " << aTopLeft.getX() << ":" << aTopLeft.getY() << ", " << aBottomRight.getX() << ":" << aBottomRight.getY() );
                }

                vcl::text::ComplexTextLayoutFlags nTextLayoutMode = vcl::text::ComplexTextLayoutFlags::Default;
                if ( nOptions & ETO_RTLREADING )
                    nTextLayoutMode = vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::TextOriginLeft;
                SetTextLayoutMode( nTextLayoutMode );
                SAL_WARN_IF( ( nOptions & ( ETO_PDY | ETO_GLYPH_INDEX ) ) != 0, "emfio", "SJ: ETO_PDY || ETO_GLYPH_INDEX in WMF" );

                // output only makes sense if the text contains characters
                if (nLen)
                {
                    sal_Int32 nOriginalTextLen = nLen;
                    sal_Int32 nOriginalBlockLen = ( nOriginalTextLen + 1 ) &~ 1;

                    auto nMaxStreamPos = nRecordPos + nRecSize;
                    auto nRemainingSize = std::min(mpInputStream->remainingSize(), nMaxStreamPos - mpInputStream->Tell());
                    if (nRemainingSize < o3tl::make_unsigned(nOriginalBlockLen))
                    {
                        SAL_WARN("emfio", "exttextout record claimed more data than the stream can provide");
                        nOriginalTextLen = nOriginalBlockLen = nRemainingSize;
                    }

                    std::vector<char> pChar(nOriginalBlockLen);
                    mpInputStream->ReadBytes(pChar.data(), nOriginalBlockLen);
                    OUString aText(pChar.data(), nOriginalTextLen, GetCharSet()); // after this conversion the text may contain
                    sal_Int32 nNewTextLen = aText.getLength();                         // less character (japanese version), so the
                                                                                       // dxAry will not fit
                    if ( nNewTextLen )
                    {
                        if ( nOptions & ETO_CLIPPED )
                        {
                            Push(); // Save the current clip. It will be restored after text drawing
                            IntersectClipRect( aRect );
                        }
                        SAL_INFO( "emfio", "\t\t\t Text : " << aText );
                        std::vector<sal_Int32> aDXAry;
                        std::unique_ptr<tools::Long[]> pDYAry;
                        auto nDxArySize =  nMaxStreamPos - mpInputStream->Tell();
                        auto nDxAryEntries = nDxArySize >> 1;
                        bool        bUseDXAry = false;

                        if ( ( ( nDxAryEntries % nOriginalTextLen ) == 0 ) && ( nNewTextLen <= nOriginalTextLen ) )
                        {
                            sal_Int32 i; // needed just outside the for
                            aDXAry.resize( nNewTextLen );
                            if ( nOptions & ETO_PDY )
                            {
                                pDYAry.reset(new tools::Long[ nNewTextLen ]);
                            }
                            for (i = 0; i < nNewTextLen; i++ )
                            {
                                if ( mpInputStream->Tell() >= nMaxStreamPos )
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
                                    if ( ( mpInputStream->Tell() + 2 ) > nMaxStreamPos )
                                        break;
                                    sal_Int16 nDxTmp = 0;
                                    mpInputStream->ReadInt16(nDxTmp);
                                    nDx += nDxTmp;
                                    if ( nOptions & ETO_PDY )
                                    {
                                        if ( ( mpInputStream->Tell() + 2 ) > nMaxStreamPos )
                                            break;
                                        sal_Int16 nDyTmp = 0;
                                        mpInputStream->ReadInt16(nDyTmp);
                                        nDy += nDyTmp;
                                    }
                                }

                                aDXAry[ i ] = nDx;
                                if ( nOptions & ETO_PDY )
                                {
                                    pDYAry[i] = nDy;
                                }
                            }
                            if ( i == nNewTextLen )
                                bUseDXAry = true;
                        }
                        if ( bUseDXAry )
                            DrawText( aPosition, aText, &aDXAry, pDYAry.get() );
                        else
                            DrawText( aPosition, aText );
                        if ( nOptions & ETO_CLIPPED )
                            Pop();
                    }
                }
            }
            break;

            case W_META_SELECTOBJECT:
            case W_META_SELECTPALETTE:
            {
                sal_uInt16   nObjIndex = 0;
                mpInputStream->ReadUInt16( nObjIndex );
                SelectObject( nObjIndex );
            }
            break;

            case W_META_SETTEXTALIGN:
            {
                sal_uInt16  nAlign = 0;
                mpInputStream->ReadUInt16( nAlign );
                SetTextAlign( nAlign );
            }
            break;

            case W_META_BITBLT:
            case W_META_STRETCHBLT:
            {
                sal_uInt32  nRasterOperation = 0;
                sal_Int16   nSrcHeight = 0, nSrcWidth = 0, nYSrc, nXSrc, nSye, nSxe, nBitmapType, nWidth, nHeight, nBytesPerScan;
                sal_uInt8   nPlanes, nBitCount;
                const bool bNoSourceBitmap = ( nRecordSize == ( static_cast< sal_uInt32 >( nFunc ) >> 8 ) + 3 );

                mpInputStream->ReadUInt32( nRasterOperation );
                SAL_INFO("emfio", "\t\t Raster operation: 0x" << std::hex << nRasterOperation << std::dec << ", No source bitmap: " << bNoSourceBitmap);

                if( nFunc == W_META_STRETCHBLT )
                    mpInputStream->ReadInt16( nSrcHeight ).ReadInt16( nSrcWidth );

                mpInputStream->ReadInt16( nYSrc ).ReadInt16( nXSrc );
                if ( bNoSourceBitmap )
                    mpInputStream->SeekRel( 2 ); // Skip Reserved 2 bytes (it must be ignored)
                mpInputStream->ReadInt16( nSye ).ReadInt16( nSxe );
                Point aPoint( ReadYX() ); // The upper-left corner of the destination rectangle.
                mpInputStream->ReadInt16( nBitmapType ).ReadInt16( nWidth ).ReadInt16( nHeight ).ReadInt16( nBytesPerScan ).ReadUChar( nPlanes ).ReadUChar( nBitCount );

                SAL_INFO("emfio", "\t\t Bitmap type:" << nBitmapType << " Width:" << nWidth << " Height:" << nHeight << " WidthBytes:" << nBytesPerScan << " Planes: " << static_cast< sal_uInt16 >( nPlanes ) << " BitCount: " << static_cast< sal_uInt16 >( nBitCount ) );
                if (!mpInputStream->good() || bNoSourceBitmap || nBitCount == 4 || nBitCount == 8 || nPlanes != 1)
                {
                    SAL_WARN("emfio", "\t\t TODO The unsupported Bitmap record. Please fill a bug.");
                    break;
                }
                const vcl::PixelFormat ePixelFormat = vcl::bitDepthToPixelFormat( nBitCount );
                bool bOk = nWidth > 0 && nHeight > 0 && nBytesPerScan > 0 && ePixelFormat != vcl::PixelFormat::INVALID;
                if (bOk)
                {
                    // must be enough data to fulfil the request
                    bOk = o3tl::make_unsigned( nBytesPerScan ) <= mpInputStream->remainingSize() / nHeight;
                }
                if (bOk)
                {
                    // scanline must be large enough to provide all pixels
                    bOk = nBytesPerScan >= nWidth * nBitCount / 8;
                }
                if (bOk)
                {
                    std::unique_ptr< sal_uInt8[] > pData;
                    pData.reset( new sal_uInt8[ nHeight * nBytesPerScan ] );
                    mpInputStream->ReadBytes( pData.get(), nHeight * nBytesPerScan );
                    BitmapEx aBitmap = vcl::bitmap::CreateFromData( pData.get(), nWidth, nHeight, nBytesPerScan, ePixelFormat, true );
                    if ( nSye && nSxe &&
                         ( nXSrc + nSxe <= nWidth ) &&
                         ( nYSrc + nSye <= nHeight ) )
                    {
                        tools::Rectangle aCropRect( Point( nXSrc, nYSrc ), Size( nSxe, nSye ) );
                        aBitmap.Crop( aCropRect );
                    }
                    tools::Rectangle aDestRect( aPoint, Size( nSxe, nSye ) );
                    maBmpSaveList.emplace_back(aBitmap, aDestRect, nRasterOperation);
                }
            }
            break;

            case W_META_DIBBITBLT:
            case W_META_DIBSTRETCHBLT:
            case W_META_STRETCHDIB:
            {
                sal_uInt32  nRasterOperation = 0;
                sal_uInt16  nColorUsage = 0;
                sal_Int16   nSrcHeight = 0, nSrcWidth = 0, nYSrc = 0, nXSrc = 0;
                Bitmap      aBmp;
                const bool bNoSourceBitmap = ( nFunc != W_META_STRETCHDIB ) && ( nRecordSize == ( ( static_cast< sal_uInt32 >( nFunc ) >> 8 ) + 3 ) );

                mpInputStream->ReadUInt32( nRasterOperation );
                SAL_INFO("emfio", "\t\t Raster operation: 0x" << std::hex << nRasterOperation << std::dec << ", No source bitmap: " << bNoSourceBitmap);
                if( nFunc == W_META_STRETCHDIB )
                    mpInputStream->ReadUInt16( nColorUsage );

                // nSrcHeight and nSrcWidth is the number of pixels that has to been used
                // If they are set to zero, it is as indicator not to scale the bitmap later
                if( nFunc == W_META_DIBSTRETCHBLT ||
                    nFunc == W_META_STRETCHDIB )
                    mpInputStream->ReadInt16( nSrcHeight ).ReadInt16( nSrcWidth );

                // nYSrc and nXSrc is the offset of the first pixel
                mpInputStream->ReadInt16( nYSrc ).ReadInt16( nXSrc );

                if ( bNoSourceBitmap )
                    mpInputStream->SeekRel( 2 ); // Skip Reserved 2 bytes (it must be ignored)

                Size aDestSize( ReadYXExt() );
                if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps
                {
                    tools::Rectangle aDestRect( ReadYX(), aDestSize );
                    if ( !bNoSourceBitmap )
                    {
                        // tdf#142625 Read the DIBHeader and check if bitmap is supported
                        // If bitmap is not supported don't run ReadDIB, as it will interrupt image processing
                        const auto nOldPos(mpInputStream->Tell());
                        sal_uInt32 nHeaderSize(0);
                        mpInputStream->ReadUInt32( nHeaderSize );
                        if ( nHeaderSize == 0xC ) // BitmapCoreHeader
                            mpInputStream->SeekRel( 6 ); // skip Width (16), Height (16), Planes (16)
                        else
                            mpInputStream->SeekRel( 10 ); // skip Width (32), Height (32), Planes (16)
                        sal_uInt16 nBitCount(0);
                        mpInputStream->ReadUInt16( nBitCount );
                        if ( nBitCount == 0 ) // TODO Undefined BitCount (JPEG/PNG), which are not supported
                            break;
                        mpInputStream->Seek(nOldPos);

                        if ( !ReadDIB( aBmp, *mpInputStream, false ) )
                            SAL_WARN( "emfio", "\tTODO Read DIB failed. Interrupting processing whole image. Please report bug report." );
                    }
                    // test if it is sensible to crop
                    if ( nSrcHeight && nSrcWidth &&
                            ( nXSrc + nSrcWidth <= aBmp.GetSizePixel().Width() ) &&
                            ( nYSrc + nSrcHeight <= aBmp.GetSizePixel().Height() ) )
                    {
                        tools::Rectangle aCropRect( Point( nXSrc, nYSrc ), Size( nSrcWidth, nSrcHeight ) );
                        aBmp.Crop( aCropRect );
                    }

                    maBmpSaveList.emplace_back(aBmp, aDestRect, nRasterOperation);
                }
            }
            break;

            case W_META_DIBCREATEPATTERNBRUSH:
            {
                Bitmap  aBmp;
                sal_uInt32 nRed(0), nGreen(0), nBlue(0), nCount(1);
                sal_uInt16 nStyle(0), nColorUsage(0);

                mpInputStream->ReadUInt16( nStyle ).ReadUInt16( nColorUsage );
                SAL_INFO( "emfio", "\t\t Style:" << nStyle << ", ColorUsage: " << nColorUsage );
                if ( nStyle == BS_PATTERN ) // TODO tdf#142625 Add support for pattern
                {
                    SAL_WARN( "emfio", "\tTODO: Pattern brush style is not supported." );
                    CreateObject();
                    break;
                }
                if ( !ReadDIB( aBmp, *mpInputStream, false ) )
                    SAL_WARN( "emfio", "\tTODO Read DIB failed. Interrupting processing whole image. Please report bug report." );
                if ( !aBmp.IsEmpty() )
                {
                    Bitmap::ScopedReadAccess pBmp(aBmp);
                    for ( tools::Long y = 0; y < pBmp->Height(); y++ )
                    {
                        for ( tools::Long x = 0; x < pBmp->Width(); x++ )
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
                }
                Color aColor( static_cast<sal_uInt8>( nRed / nCount ), static_cast<sal_uInt8>( nGreen / nCount ), static_cast<sal_uInt8>( nBlue / nCount ) );
                CreateObject(std::make_unique<WinMtfFillStyle>( aColor, false ));
            }
            break;

            case W_META_DELETEOBJECT:
            {
                sal_uInt16 nIndex = 0;
                mpInputStream->ReadUInt16( nIndex );
                DeleteObject( nIndex );
            }
            break;

            case W_META_CREATEPALETTE:
            {
                sal_uInt16 nStart = 0;
                sal_uInt16 nNumberOfEntries = 0;
                mpInputStream->ReadUInt16( nStart );
                mpInputStream->ReadUInt16( nNumberOfEntries );

                SAL_INFO("emfio", "\t\t Start 0x" << std::hex << nStart << std::dec << ", Number of entries: " << nNumberOfEntries);
                sal_uInt32 nPalleteEntry;
                std::vector< Color > aPaletteColors;
                for (sal_uInt16 i = 0; i < nNumberOfEntries; ++i)
                {
                    //PALETTEENTRY: Values, Blue, Green, Red
                    mpInputStream->ReadUInt32( nPalleteEntry );
                    SAL_INFO("emfio", "\t\t " << i << ". Palette entry: " << std::setw(10) << std::showbase <<std::hex << nPalleteEntry << std::dec );
                    aPaletteColors.push_back(Color(static_cast<sal_uInt8>(nPalleteEntry), static_cast<sal_uInt8>(nPalleteEntry >> 8), static_cast<sal_uInt8>(nPalleteEntry >> 16)));
                }
                CreateObject(std::make_unique<WinMtfPalette>( aPaletteColors ));
            }
            break;

            case W_META_CREATEBRUSH:
            {
                SAL_WARN( "emfio", "TODO: Not implemented. Please fill the bug report" );
                CreateObject(std::make_unique<WinMtfFillStyle>( COL_WHITE, false ));
            }
            break;

            case W_META_CREATEPATTERNBRUSH:
            {
                SAL_WARN( "emfio", "TODO: Not implemented. Please fill the bug report" );
                CreateObject(std::make_unique<WinMtfFillStyle>( COL_WHITE, false ));
            }
            break;

            case W_META_CREATEPENINDIRECT:
            {
                LineInfo   aLineInfo;
                sal_uInt16 nStyle = 0;
                sal_uInt16 nWidth = 0;
                sal_uInt16 nHeight = 0;

                mpInputStream->ReadUInt16(nStyle);
                mpInputStream->ReadUInt16(nWidth);
                mpInputStream->ReadUInt16(nHeight);

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
                CreateObject(std::make_unique<WinMtfLineStyle>( ReadColor(), aLineInfo, bTransparent ));
            }
            break;

            case W_META_CREATEBRUSHINDIRECT:
            {
                sal_uInt16  nBrushStyle = 0;
                mpInputStream->ReadUInt16( nBrushStyle );
                CreateObject(std::make_unique<WinMtfFillStyle>( ReadColor(), ( nBrushStyle == BS_NULL ) ));
                SAL_WARN_IF( (nBrushStyle != BS_SOLID) && (nBrushStyle != BS_NULL), "emfio", "TODO: Brush style not implemented. Please fill the bug report" );
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
                mpInputStream->ReadInt16( lfEscapement );
                mpInputStream->ReadInt16( lfOrientation );
                mpInputStream->ReadInt16( lfWeight );
                mpInputStream->ReadUChar( aLogFont.lfItalic );
                mpInputStream->ReadUChar( aLogFont.lfUnderline );
                mpInputStream->ReadUChar( aLogFont.lfStrikeOut );
                mpInputStream->ReadUChar( aLogFont.lfCharSet );
                mpInputStream->ReadUChar( aLogFont.lfOutPrecision );
                mpInputStream->ReadUChar( aLogFont.lfClipPrecision );
                mpInputStream->ReadUChar( aLogFont.lfQuality );
                mpInputStream->ReadUChar( aLogFont.lfPitchAndFamily );
                size_t nRet = mpInputStream->ReadBytes( lfFaceName, LF_FACESIZE );
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

                CreateObject(std::make_unique<WinMtfFontStyle>( aLogFont ));
            }
            break;

            case W_META_CREATEBITMAPINDIRECT:
            {
                SAL_WARN( "emfio", "TODO: W_META_CREATEBITMAPINDIRECT is not implemented. Please fill the bug report" );
                CreateObject();
            }
            break;

            case W_META_CREATEBITMAP:
            {
                SAL_WARN( "emfio", "TODO: W_META_CREATEBITMAP is not implemented. Please fill the bug report" );
                CreateObject();
            }
            break;

            case W_META_CREATEREGION:
            {
                SAL_WARN( "emfio", "TODO: W_META_CREATEREGION is not implemented. Please fill the bug report" );
                CreateObject();
            }
            break;

            case W_META_EXCLUDECLIPRECT :
            {
                SAL_WARN( "emfio", "TODO:  Not working correctly. Please fill the bug report" );
                ExcludeClipRect( ReadRectangle() );
            }
            break;

            case W_META_PATBLT:
            {
                sal_uInt32 nROP = 0;
                mpInputStream->ReadUInt32( nROP );
                Size aSize = ReadYXExt();
                WMFRasterOp nOldROP = SetRasterOp( static_cast<WMFRasterOp>(nROP) );
                DrawRect( tools::Rectangle( ReadYX(), aSize ), false );
                SetRasterOp( nOldROP );
            }
            break;

            case W_META_SELECTCLIPREGION:
            {
                sal_uInt16 nObjIndex = 0;
                mpInputStream->ReadUInt16( nObjIndex );
                SAL_WARN( "emfio", "TODO: W_META_SELECTCLIPREGION is not implemented. Please fill the bug report" );
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
                sal_uInt64 nMetaRecEndPos = mpInputStream->Tell() + nMetaRecSize;

                // taking care that mnRecSize does not exceed the maximal stream position
                if ( nMetaRecEndPos > mnEndPos )
                {
                    mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    break;
                }
                if (mnRecSize >= 4 )    // minimal escape length
                {
                    sal_uInt16  nMode = 0, nLen = 0;
                    mpInputStream->ReadUInt16( nMode )
                         .ReadUInt16( nLen );
                    if ( ( nMode == W_MFCOMMENT ) && ( nLen >= 4 ) )
                    {
                        sal_uInt32 nNewMagic = 0; // we have to read int32 for
                        mpInputStream->ReadUInt32( nNewMagic );   // META_ESCAPE_ENHANCED_METAFILE CommentIdentifier

                        if( nNewMagic == 0x2c2a4f4f &&  nLen >= 14 )
                        {
                            sal_uInt16 nMagic2 = 0;
                            mpInputStream->ReadUInt16( nMagic2 );
                            if( nMagic2 == 0x0a ) // 2nd half of magic
                            {                     // continue with private escape
                                sal_uInt32 nCheck = 0, nEsc = 0;
                                mpInputStream->ReadUInt32( nCheck )
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

                                    if ( ( static_cast< sal_uInt64 >( nEscLen ) + mpInputStream->Tell() ) > nMetaRecEndPos )
                                    {
                                        mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                                        break;
                                    }
                                    if ( nEscLen > 0 )
                                    {
                                        pData.reset(new sal_Int8[ nEscLen ]);
                                        mpInputStream->ReadBytes(pData.get(), nEscLen);
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
                                                    sal_uInt32  nStringLen, nDXCount;
                                                    std::vector<sal_Int32> aDXAry;
                                                    SvMemoryStream aMemoryStream( nEscLen );
                                                    aMemoryStream.WriteBytes(pData.get(), nEscLen);
                                                    aMemoryStream.Seek( STREAM_SEEK_TO_BEGIN );
                                                    sal_Int32 nTmpX(0), nTmpY(0);
                                                    aMemoryStream.ReadInt32( nTmpX )
                                                                 .ReadInt32( nTmpY )
                                                                 .ReadUInt32( nStringLen );
                                                    aPt.setX( nTmpX );
                                                    aPt.setY( nTmpY );

                                                    if ( ( static_cast< sal_uInt64 >( nStringLen ) * sizeof( sal_Unicode ) ) < ( nEscLen - aMemoryStream.Tell() ) )
                                                    {
                                                        OUString aString = read_uInt16s_ToOUString(aMemoryStream, nStringLen);
                                                        aMemoryStream.ReadUInt32( nDXCount );
                                                        if ( ( static_cast< sal_uInt64 >( nDXCount ) * sizeof( sal_Int32 ) ) >= ( nEscLen - aMemoryStream.Tell() ) )
                                                            nDXCount = 0;
                                                        if ( nDXCount )
                                                            aDXAry.resize(nDXCount);
                                                        for  (sal_uInt32 i = 0; i < nDXCount; i++ )
                                                        {
                                                            sal_Int32 val;
                                                            aMemoryStream.ReadInt32( val);
                                                            aDXAry[ i ] = val;
                                                        }
                                                        aMemoryStream.ReadUInt32(mnSkipActions);
                                                        DrawText( aPt, aString, aDXAry.empty() ? nullptr : &aDXAry );
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else if ( (nNewMagic == static_cast< sal_uInt32 >(0x43464D57)) && (nLen >= 34) && ( static_cast<sal_Int32>(nLen + 10) <= static_cast<sal_Int32>(mnRecSize * 2) ))
                        {
                            sal_uInt32 nComType = 0, nVersion = 0, nFlags = 0, nComRecCount = 0,
                                       nCurRecSize = 0, nRemainingSize = 0, nEMFTotalSize = 0;
                            sal_uInt16 nCheck = 0;

                            mpInputStream->ReadUInt32( nComType ).ReadUInt32( nVersion ).ReadUInt16( nCheck ).ReadUInt32( nFlags )
                                 .ReadUInt32( nComRecCount ).ReadUInt32( nCurRecSize )
                                 .ReadUInt32( nRemainingSize ).ReadUInt32( nEMFTotalSize ); // the nRemainingSize is not mentioned in MSDN documentation
                                                                      // but it seems to be required to read in data produced by OLE

                            if( nComType == 0x01 && nVersion == 0x10000 && nComRecCount )
                            {
                                if( !mnEMFRec)
                                {   // first EMF comment
                                    mnEMFRecCount = nComRecCount;
                                    mnEMFSize = nEMFTotalSize;
                                    if (mnEMFSize > mpInputStream->remainingSize())
                                    {
                                        SAL_WARN("emfio", "emf size claims to be larger than remaining data");
                                        mpEMFStream.reset();
                                    }
                                    else
                                        mpEMFStream = std::vector<sal_uInt8>();
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

                                if (mpEMFStream && nCurRecSize > mpInputStream->remainingSize())
                                {
                                    SAL_WARN("emfio", "emf record size claims to be larger than remaining data");
                                    mnEMFRecCount = 0xFFFFFFFF;
                                    mpEMFStream.reset();
                                }

                                if (mpEMFStream)
                                {
                                    std::vector<sal_Int8> aBuf(nCurRecSize);
                                    sal_uInt32 nCount = mpInputStream->ReadBytes(aBuf.data(), nCurRecSize);
                                    if( nCount == nCurRecSize )
                                    {
                                        mpEMFStream->insert(mpEMFStream->end(), aBuf.begin(), aBuf.end());
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

            case W_META_SETPOLYFILLMODE:
            {
                sal_uInt32 nFillMode  = 0;
                mpInputStream->ReadUInt32( nFillMode );
                setPolyFillMode( static_cast<PolyFillMode>(nFillMode) );
                SAL_INFO("emfio", "\t\tPolyFillMode: 0x" << std::hex << nFillMode << std::dec);
                CreateObject();
            }
            break;

            case W_META_SETRELABS:
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
            {
                SAL_WARN("emfio", "TODO: WMF record not implemented: " << record_type_name(nFunc));
            }
            break;

            default:
            {
                SAL_WARN("emfio", "Unknown Meta Action: 0x" << std::hex << nFunc << std::dec);
            }
        }

        // tdf#127471
        maScaledFontHelper.applyAlternativeFontScale();
    }

    const tools::Long   aMaxWidth = 1024;

    bool WmfReader::ReadHeader()
    {
        sal_uInt64 const nStrmPos = mpInputStream->Tell();

        sal_uInt32 nPlaceableMetaKey(0);
        // if available read the METAFILEHEADER
        mpInputStream->ReadUInt32( nPlaceableMetaKey );
        if (!mpInputStream->good())
            return false;

        tools::Rectangle aPlaceableBound;

        bool bPlaceable = nPlaceableMetaKey == 0x9ac6cdd7L;

        SAL_INFO("emfio", "Placeable: \"" << (bPlaceable ? "yes" : "no") << "\"");

        if (bPlaceable)
        {
            //TODO do some real error handling here
            sal_Int16 nVal(0);

            // Skip reserved bytes
            mpInputStream->SeekRel(2);

            // BoundRect
            // These are simply ignored for now
            mpInputStream->ReadInt16( nVal );
            aPlaceableBound.SetLeft( nVal );
            mpInputStream->ReadInt16( nVal );
            aPlaceableBound.SetTop( nVal );
            mpInputStream->ReadInt16( nVal );
            aPlaceableBound.SetRight( nVal );
            mpInputStream->ReadInt16( nVal );
            aPlaceableBound.SetBottom( nVal );

            // inch
            mpInputStream->ReadUInt16( mnUnitsPerInch );

            // reserved
            mpInputStream->SeekRel( 4 );

            // Skip and don't check the checksum
            mpInputStream->SeekRel( 2 );

            // Skip wmf header
            mpInputStream->Seek( nStrmPos + 40 );    // set the streampos to the start of the metaactions
            GetPlaceableBound( aPlaceableBound, mpInputStream );
            // Go back to the place after placeable header
            mpInputStream->Seek( nStrmPos + 22);
        }
        else
        {
            // Default is 1440, but it is set to 96 to show the wmf larger
            mnUnitsPerInch = 96;

            if (mpExternalHeader != nullptr
                && mpExternalHeader->xExt > 0
                && mpExternalHeader->yExt > 0
                && (mpExternalHeader->mapMode == MM_ISOTROPIC || mpExternalHeader->mapMode == MM_ANISOTROPIC))
            {
                // #n417818#: If we have an external header then overwrite the bounds!
                tools::Rectangle aExtRect(0, 0,
                    o3tl::convert(mpExternalHeader->xExt, o3tl::Length::mm100, o3tl::Length::px),
                    o3tl::convert(mpExternalHeader->yExt, o3tl::Length::mm100, o3tl::Length::px));
                aPlaceableBound = aExtRect;

                SAL_INFO("emfio", "External header size "
                    " left: " << aPlaceableBound.Left() << " top: " << aPlaceableBound.Top()
                    << " right: " << aPlaceableBound.Right() << " bottom: " << aPlaceableBound.Bottom());

                SetMapMode(mpExternalHeader->mapMode);
            }
            else
            {
                mpInputStream->Seek(nStrmPos + 18);    // set the streampos to the start of the metaactions
                GetPlaceableBound(aPlaceableBound, mpInputStream);

                // The image size is not known so normalize the calculated bounds so that the
                // resulting image is not too big
                if (aPlaceableBound.GetWidth() > aMaxWidth)
                {
                    const double fMaxWidth = static_cast<double>(aMaxWidth);
                    double fRatio = aPlaceableBound.GetWidth() / fMaxWidth;

                    // changing mnUnitsPerInch as a tool to scale wmf
                    mnUnitsPerInch *= fRatio;

                    SAL_INFO("emfio", "Placeable bounds "
                        " left: " << aPlaceableBound.Left() << " top: " << aPlaceableBound.Top()
                        << " right: " << aPlaceableBound.Right() << " bottom: " << aPlaceableBound.Bottom());
                }
            }

            mpInputStream->Seek( nStrmPos );
        }

        SetWinOrg( aPlaceableBound.TopLeft() );
        Size aWMFSize(
            std::abs( aPlaceableBound.GetWidth() ), std::abs( aPlaceableBound.GetHeight() ) );
        SetWinExt( aWMFSize );

        SAL_INFO("emfio", "WMF size  w: " << aWMFSize.Width()    << " h: " << aWMFSize.Height());

        Size aDevExt( 10000, 10000 );
        if( ( std::abs( aWMFSize.Width() ) > 1 ) && ( std::abs( aWMFSize.Height() ) > 1 ) )
        {
            const Fraction  aFrac( 1, mnUnitsPerInch);
            MapMode         aWMFMap( MapUnit::MapInch, Point(), aFrac, aFrac );
            Size            aSize100(OutputDevice::LogicToLogic(aWMFSize, aWMFMap, MapMode(MapUnit::Map100thMM)));
            aDevExt = Size( std::abs( aSize100.Width() ), std::abs( aSize100.Height() ) );
        }
        SetDevExt( aDevExt );

        SAL_INFO("emfio", "Dev size  w: " << aDevExt.Width()    << " h: " << aDevExt.Height());

        // read the METAHEADER
        sal_uInt32 nMetaKey(0);
        mpInputStream->ReadUInt32( nMetaKey ); // type and headersize
        if (!mpInputStream->good())
            return false;
        if (nMetaKey != 0x00090001)
        {
            sal_uInt16 aNextWord(0);
            mpInputStream->ReadUInt16( aNextWord );
            if (nMetaKey != 0x10000 || aNextWord != 0x09)
            {
                mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );
                return false;
            }
        }

        mpInputStream->SeekRel( 2 ); // Version (of Windows)
        mpInputStream->SeekRel( 4 ); // Size (of file in words)
        mpInputStream->SeekRel( 2 ); // NoObjects (maximum number of simultaneous objects)
        mpInputStream->SeekRel( 4 ); // MaxRecord (size of largest record in words)
        mpInputStream->SeekRel( 2 ); // NoParameters (Unused

        return mpInputStream->good();
    }

    void WmfReader::ReadWMF()
    {
        sal_uInt16  nFunction;

        mnSkipActions = 0;

        mpEMFStream.reset();
        mnEMFRecCount = 0;
        mnEMFRec = 0;
        mnEMFSize = 0;

        SetMapMode( MM_ANISOTROPIC );
        SetWinOrg( Point() );
        SetWinExt( Size( 1, 1 ) );
        SetDevExt( Size( 10000, 10000 ) );

        mnEndPos=mpInputStream->TellEnd();
        mpInputStream->Seek( mnStartPos );

        if ( ReadHeader( ) )
        {
            auto nPos = mpInputStream->Tell();

            if( mnEndPos - mnStartPos )
            {
                bool bEMFAvailable = false;
                while( !mpInputStream->eof() )
                {
                    mpInputStream->ReadUInt32(mnRecSize).ReadUInt16( nFunction );

                    if (
                         !mpInputStream->good() ||
                         (mnRecSize < 3) ||
                         (mnRecSize == 3 && nFunction == W_META_EOF)
                       )
                    {
                        if( mpInputStream->eof() )
                            mpInputStream->SetError( SVSTREAM_FILEFORMAT_ERROR );

                        break;
                    }

                    const sal_uInt32 nAvailableBytes = mnEndPos - nPos;
                    const sal_uInt32 nMaxPossibleRecordSize = nAvailableBytes/2;
                    if (mnRecSize > nMaxPossibleRecordSize)
                    {
                        mpInputStream->SetError(SVSTREAM_FILEFORMAT_ERROR);
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
                            ReadRecordParams( mnRecSize, nFunction );
                        else
                            mnSkipActions--;

                        if(mpEMFStream && mnEMFRecCount == mnEMFRec)
                        {
                            GDIMetaFile aMeta;
                            SvMemoryStream aStream(mpEMFStream->data(), mpEMFStream->size(), StreamMode::STD_READ);
                            std::unique_ptr<EmfReader> pEMFReader(std::make_unique<EmfReader>(aStream, aMeta));
                            pEMFReader->SetEnableEMFPlus(mbEnableEMFPlus);
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

                    nPos += mnRecSize * 2;
                    mpInputStream->Seek(nPos);
                }
            }
            else
                mpInputStream->SetError( SVSTREAM_GENERALERROR );

            if( !mpInputStream->GetError() && !maBmpSaveList.empty() )
                ResolveBitmapActions( maBmpSaveList );
        }
        if ( mpInputStream->GetError() )
            mpInputStream->Seek( mnStartPos );
    }

    void WmfReader::GetPlaceableBound( tools::Rectangle& rPlaceableBound, SvStream* pStm )
    {
        bool bRet = true;

        tools::Rectangle aBound;
        aBound.SetLeft( RECT_MAX );
        aBound.SetTop( RECT_MAX );
        aBound.SetRight( RECT_MIN );
        aBound.SetBottom( RECT_MIN );
        bool bBoundsDetermined = false;

        auto nPos = pStm->Tell();
        auto nEnd = nPos + pStm->remainingSize();

        Point aWinOrg(0,0);
        std::optional<Size>  aWinExt;

        Point aViewportOrg(0,0);
        std::optional<Size>  aViewportExt;

        if (nEnd - nPos)
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
                else if (pStm->eof() || nRSize < 3)
                {
                    pStm->SetError( SVSTREAM_FILEFORMAT_ERROR );
                    bRet = false;
                    break;
                }
                else if ( nRSize == 3 && nFunction == W_META_EOF )
                {
                    break;
                }
                switch( nFunction )
                {
                    case W_META_EOF:
                    {
                        return;
                    }

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

                        bRecordOk &= pStm->good();

                        SAL_WARN_IF(!bRecordOk, "emfio", "polyline record claimed more points than the stream can provide");

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

                        SAL_WARN_IF(!bRecordOk, "emfio", "polypolygon record has more polygons than we can handle");

                        bRecordOk &= pStm->good();

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

                        SAL_WARN_IF(!bRecordOk, "emfio", "polypolygon record claimed more points than the stream can provide");

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

                        SAL_WARN_IF(!bRecordOk, "emfio", "polyline record claimed more points than the stream can provide");

                        bRecordOk &= pStm->good();

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
                        sal_uInt16 nLength(0);
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
                        sal_uInt16 nLen(0), nOptions;
                        Point aPosition = ReadYX();
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
                    case W_META_DIBBITBLT:
                    case W_META_DIBSTRETCHBLT:
                    case W_META_STRETCHBLT:
                    case W_META_STRETCHDIB:
                    {
                        sal_uInt32 nRasterOperation;
                        sal_Int16 nYSrc, nXSrc;
                        sal_uInt16 nColorUsage;
                        pStm->ReadUInt32( nRasterOperation );

                        if( nFunction == W_META_STRETCHDIB )
                            pStm->ReadUInt16( nColorUsage );

                        if( nFunction == W_META_DIBSTRETCHBLT ||
                            nFunction == W_META_STRETCHBLT ||
                            nFunction == W_META_STRETCHDIB )
                        {
                            sal_Int16 nSrcHeight, nSrcWidth;
                            pStm->ReadInt16( nSrcHeight ).ReadInt16( nSrcWidth );
                        }

                        // nYSrc and nXSrc is the offset of the first pixel
                        pStm->ReadInt16( nYSrc ).ReadInt16( nXSrc );

                        const bool bNoSourceBitmap = ( nFunction != W_META_STRETCHDIB ) && ( nRSize == ( ( static_cast< sal_uInt32 >( nFunction ) >> 8 ) + 3 ) );
                        if ( bNoSourceBitmap )
                            mpInputStream->SeekRel( 2 ); // Skip Reserved 2 bytes (it must be ignored)

                        Size aDestSize( ReadYXExt() );
                        if ( aDestSize.Width() && aDestSize.Height() )  // #92623# do not try to read buggy bitmaps
                        {
                            tools::Rectangle aDestRect( ReadYX(), aDestSize );
                            GetWinExtMax( aDestRect, aBound, nMapMode );
                            bBoundsDetermined = true;
                        }
                    }
                    break;

                    case W_META_PATBLT:
                    {
                        sal_uInt32 nROP(0);
                        pStm->ReadUInt32( nROP );
                        Size aSize = ReadYXExt();
                        GetWinExtMax( tools::Rectangle( ReadYX(), aSize ), aBound, nMapMode );
                        bBoundsDetermined = true;
                    }
                    break;
                }

                const auto nAvailableBytes = nEnd - nPos;
                const auto nMaxPossibleRecordSize = nAvailableBytes/2;
                if (nRSize <= nMaxPossibleRecordSize)
                {
                    nPos += nRSize * 2;
                    pStm->Seek(nPos);
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

        if (!bRet)
            return;

        if (aWinExt)
        {
            rPlaceableBound = tools::Rectangle(aWinOrg, *aWinExt);
            SAL_INFO("emfio", "Window dimension "
                       " left: " << rPlaceableBound.Left()  << " top: " << rPlaceableBound.Top()
                    << " right: " << rPlaceableBound.Right() << " bottom: " << rPlaceableBound.Bottom());
        }
        else if (aViewportExt)
        {
            rPlaceableBound = tools::Rectangle(aViewportOrg, *aViewportExt);
            SAL_INFO("emfio", "Viewport dimension "
                       " left: " << rPlaceableBound.Left()  << " top: " << rPlaceableBound.Top()
                    << " right: " << rPlaceableBound.Right() << " bottom: " << rPlaceableBound.Bottom());
        }
        else if (bBoundsDetermined)
        {
            rPlaceableBound = aBound;
            SAL_INFO("emfio", "Determined dimension "
                       " left: " << rPlaceableBound.Left()  << " top: " << rPlaceableBound.Top()
                    << " right: " << rPlaceableBound.Right() << " bottom: " << rPlaceableBound.Bottom());
        }
        else
        {
            rPlaceableBound.SetLeft( 0 );
            rPlaceableBound.SetTop( 0 );
            rPlaceableBound.SetRight( aMaxWidth );
            rPlaceableBound.SetBottom( aMaxWidth );
            SAL_INFO("emfio", "Default dimension "
                       " left: " << rPlaceableBound.Left()  << " top: " << rPlaceableBound.Top()
                    << " right: " << rPlaceableBound.Right() << " bottom: " << rPlaceableBound.Bottom());
        }
    }

    WmfReader::WmfReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile, const WmfExternal* pExternalHeader)
        : MtfTools(rGDIMetaFile, rStreamWMF)
        , mnUnitsPerInch(96)
        , mnRecSize(0)
        , mnEMFRecCount(0)
        , mnEMFRec(0)
        , mnEMFSize(0)
        , mnSkipActions(0)
        , mpExternalHeader(pExternalHeader)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
