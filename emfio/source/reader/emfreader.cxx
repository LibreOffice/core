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

#include <emfreader.hxx>
#include <osl/endian.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcl/dibtools.hxx>
#include <o3tl/safeint.hxx>
#include <tools/stream.hxx>
#include <memory>

#ifdef DBG_UTIL
#include <vcl/pngwrite.hxx>
#endif

using namespace std;

// GDI-Array

#define EMR_HEADER                      1
#define EMR_POLYBEZIER                  2
#define EMR_POLYGON                     3
#define EMR_POLYLINE                    4
#define EMR_POLYBEZIERTO                5
#define EMR_POLYLINETO                  6
#define EMR_POLYPOLYLINE                7
#define EMR_POLYPOLYGON                 8
#define EMR_SETWINDOWEXTEX              9
#define EMR_SETWINDOWORGEX              10
#define EMR_SETVIEWPORTEXTEX            11
#define EMR_SETVIEWPORTORGEX            12
#define EMR_SETBRUSHORGEX               13
#define EMR_EOF                         14
#define EMR_SETPIXELV                   15
#define EMR_SETMAPPERFLAGS              16
#define EMR_SETMAPMODE                  17
#define EMR_SETBKMODE                   18
#define EMR_SETPOLYFILLMODE             19
#define EMR_SETROP2                     20
#define EMR_SETSTRETCHBLTMODE           21
#define EMR_SETTEXTALIGN                22
#define EMR_SETCOLORADJUSTMENT          23
#define EMR_SETTEXTCOLOR                24
#define EMR_SETBKCOLOR                  25
#define EMR_OFFSETCLIPRGN               26
#define EMR_MOVETOEX                    27
#define EMR_SETMETARGN                  28
#define EMR_EXCLUDECLIPRECT             29
#define EMR_INTERSECTCLIPRECT           30
#define EMR_SCALEVIEWPORTEXTEX          31
#define EMR_SCALEWINDOWEXTEX            32
#define EMR_SAVEDC                      33
#define EMR_RESTOREDC                   34
#define EMR_SETWORLDTRANSFORM           35
#define EMR_MODIFYWORLDTRANSFORM        36
#define EMR_SELECTOBJECT                37
#define EMR_CREATEPEN                   38
#define EMR_CREATEBRUSHINDIRECT         39
#define EMR_DELETEOBJECT                40
#define EMR_ANGLEARC                    41
#define EMR_ELLIPSE                     42
#define EMR_RECTANGLE                   43
#define EMR_ROUNDRECT                   44
#define EMR_ARC                         45
#define EMR_CHORD                       46
#define EMR_PIE                         47
#define EMR_SELECTPALETTE               48
#define EMR_CREATEPALETTE               49
#define EMR_SETPALETTEENTRIES           50
#define EMR_RESIZEPALETTE               51
#define EMR_REALIZEPALETTE              52
#define EMR_EXTFLOODFILL                53
#define EMR_LINETO                      54
#define EMR_ARCTO                       55
#define EMR_POLYDRAW                    56
#define EMR_SETARCDIRECTION             57
#define EMR_SETMITERLIMIT               58
#define EMR_BEGINPATH                   59
#define EMR_ENDPATH                     60
#define EMR_CLOSEFIGURE                 61
#define EMR_FILLPATH                    62
#define EMR_STROKEANDFILLPATH           63
#define EMR_STROKEPATH                  64
#define EMR_FLATTENPATH                 65
#define EMR_WIDENPATH                   66
#define EMR_SELECTCLIPPATH              67
#define EMR_ABORTPATH                   68

#define EMR_COMMENT                     70          // Contains arbitrary private data.
// Comment Identifiers:
#define EMR_COMMENT_EMFPLUS             0x2B464D45  // Contains embedded EMF+ records.
#define EMR_COMMENT_EMFSPOOL            0x00000000  // Contains embedded EMFSPOOL records.
#define EMR_COMMENT_PUBLIC              0x43494447  // Specify extensions to EMF processing.

#define EMR_FILLRGN                     71
#define EMR_FRAMERGN                    72
#define EMR_INVERTRGN                   73
#define EMR_PAINTRGN                    74
#define EMR_EXTSELECTCLIPRGN            75
#define EMR_BITBLT                      76
#define EMR_STRETCHBLT                  77
#define EMR_MASKBLT                     78
#define EMR_PLGBLT                      79
#define EMR_SETDIBITSTODEVICE           80
#define EMR_STRETCHDIBITS               81
#define EMR_EXTCREATEFONTINDIRECTW      82
#define EMR_EXTTEXTOUTA                 83
#define EMR_EXTTEXTOUTW                 84
#define EMR_POLYBEZIER16                85
#define EMR_POLYGON16                   86
#define EMR_POLYLINE16                  87
#define EMR_POLYBEZIERTO16              88
#define EMR_POLYLINETO16                89
#define EMR_POLYPOLYLINE16              90
#define EMR_POLYPOLYGON16               91
#define EMR_POLYDRAW16                  92
#define EMR_CREATEMONOBRUSH             93
#define EMR_CREATEDIBPATTERNBRUSHPT     94
#define EMR_EXTCREATEPEN                95
#define EMR_POLYTEXTOUTA                96
#define EMR_POLYTEXTOUTW                97

// WINDOWS VERSION >= 0x400
#define EMR_SETICMMODE                  98
#define EMR_CREATECOLORSPACE            99
#define EMR_SETCOLORSPACE              100
#define EMR_DELETECOLORSPACE           101
#define EMR_GLSRECORD                  102
#define EMR_GLSBOUNDEDRECORD           103
#define EMR_PIXELFORMAT                104

// WINDOWS VERSION >= 0x500
#define EMR_DRAWESCAPE                 105
#define EMR_EXTESCAPE                  106
#define EMR_STARTDOC                   107
#define EMR_SMALLTEXTOUT               108
#define EMR_FORCEUFIMAPPING            109
#define EMR_NAMEDESCAPE                110
#define EMR_COLORCORRECTPALETTE        111
#define EMR_SETICMPROFILEA             112
#define EMR_SETICMPROFILEW             113
#define EMR_ALPHABLEND                 114
#define EMR_ALPHADIBBLEND              115
#define EMR_TRANSPARENTBLT             116
#define EMR_TRANSPARENTDIB             117
#define EMR_GRADIENTFILL               118
#define EMR_SETLINKEDUFIS              119
#define EMR_SETTEXTJUSTIFICATION       120

namespace
{

const char *
record_type_name(sal_uInt32 nRecType)
{
#ifndef SAL_LOG_INFO
    (void) nRecType;
    return "";
#else
    switch( nRecType )
    {
    case EMR_HEADER: return "HEADER";
    case EMR_POLYBEZIER: return "POLYBEZIER";
    case EMR_POLYGON: return "POLYGON";
    case EMR_POLYLINE: return "POLYLINE";
    case EMR_POLYBEZIERTO: return "POLYBEZIERTO";
    case EMR_POLYLINETO: return "POLYLINETO";
    case EMR_POLYPOLYLINE: return "POLYPOLYLINE";
    case EMR_POLYPOLYGON: return "POLYPOLYGON";
    case EMR_SETWINDOWEXTEX: return "SETWINDOWEXTEX";
    case EMR_SETWINDOWORGEX: return "SETWINDOWORGEX";
    case EMR_SETVIEWPORTEXTEX: return "SETVIEWPORTEXTEX";
    case EMR_SETVIEWPORTORGEX: return "SETVIEWPORTORGEX";
    case EMR_SETBRUSHORGEX: return "SETBRUSHORGEX";
    case EMR_EOF: return "EOF";
    case EMR_SETPIXELV: return "SETPIXELV";
    case EMR_SETMAPPERFLAGS: return "SETMAPPERFLAGS";
    case EMR_SETMAPMODE: return "SETMAPMODE";
    case EMR_SETBKMODE: return "SETBKMODE";
    case EMR_SETPOLYFILLMODE: return "SETPOLYFILLMODE";
    case EMR_SETROP2: return "SETROP2";
    case EMR_SETSTRETCHBLTMODE: return "SETSTRETCHBLTMODE";
    case EMR_SETTEXTALIGN: return "SETTEXTALIGN";
    case EMR_SETCOLORADJUSTMENT: return "SETCOLORADJUSTMENT";
    case EMR_SETTEXTCOLOR: return "SETTEXTCOLOR";
    case EMR_SETBKCOLOR: return "SETBKCOLOR";
    case EMR_OFFSETCLIPRGN: return "OFFSETCLIPRGN";
    case EMR_MOVETOEX: return "MOVETOEX";
    case EMR_SETMETARGN: return "SETMETARGN";
    case EMR_EXCLUDECLIPRECT: return "EXCLUDECLIPRECT";
    case EMR_INTERSECTCLIPRECT: return "INTERSECTCLIPRECT";
    case EMR_SCALEVIEWPORTEXTEX: return "SCALEVIEWPORTEXTEX";
    case EMR_SCALEWINDOWEXTEX: return "SCALEWINDOWEXTEX";
    case EMR_SAVEDC: return "SAVEDC";
    case EMR_RESTOREDC: return "RESTOREDC";
    case EMR_SETWORLDTRANSFORM: return "SETWORLDTRANSFORM";
    case EMR_MODIFYWORLDTRANSFORM: return "MODIFYWORLDTRANSFORM";
    case EMR_SELECTOBJECT: return "SELECTOBJECT";
    case EMR_CREATEPEN: return "CREATEPEN";
    case EMR_CREATEBRUSHINDIRECT: return "CREATEBRUSHINDIRECT";
    case EMR_DELETEOBJECT: return "DELETEOBJECT";
    case EMR_ANGLEARC: return "ANGLEARC";
    case EMR_ELLIPSE: return "ELLIPSE";
    case EMR_RECTANGLE: return "RECTANGLE";
    case EMR_ROUNDRECT: return "ROUNDRECT";
    case EMR_ARC: return "ARC";
    case EMR_CHORD: return "CHORD";
    case EMR_PIE: return "PIE";
    case EMR_SELECTPALETTE: return "SELECTPALETTE";
    case EMR_CREATEPALETTE: return "CREATEPALETTE";
    case EMR_SETPALETTEENTRIES: return "SETPALETTEENTRIES";
    case EMR_RESIZEPALETTE: return "RESIZEPALETTE";
    case EMR_REALIZEPALETTE: return "REALIZEPALETTE";
    case EMR_EXTFLOODFILL: return "EXTFLOODFILL";
    case EMR_LINETO: return "LINETO";
    case EMR_ARCTO: return "ARCTO";
    case EMR_POLYDRAW: return "POLYDRAW";
    case EMR_SETARCDIRECTION: return "SETARCDIRECTION";
    case EMR_SETMITERLIMIT: return "SETMITERLIMIT";
    case EMR_BEGINPATH: return "BEGINPATH";
    case EMR_ENDPATH: return "ENDPATH";
    case EMR_CLOSEFIGURE: return "CLOSEFIGURE";
    case EMR_FILLPATH: return "FILLPATH";
    case EMR_STROKEANDFILLPATH: return "STROKEANDFILLPATH";
    case EMR_STROKEPATH: return "STROKEPATH";
    case EMR_FLATTENPATH: return "FLATTENPATH";
    case EMR_WIDENPATH: return "WIDENPATH";
    case EMR_SELECTCLIPPATH: return "SELECTCLIPPATH";
    case EMR_ABORTPATH: return "ABORTPATH";
    case EMR_COMMENT: return "COMMENT";
    case EMR_FILLRGN: return "FILLRGN";
    case EMR_FRAMERGN: return "FRAMERGN";
    case EMR_INVERTRGN: return "INVERTRGN";
    case EMR_PAINTRGN: return "PAINTRGN";
    case EMR_EXTSELECTCLIPRGN: return "EXTSELECTCLIPRGN";
    case EMR_BITBLT: return "BITBLT";
    case EMR_STRETCHBLT: return "STRETCHBLT";
    case EMR_MASKBLT: return "MASKBLT";
    case EMR_PLGBLT: return "PLGBLT";
    case EMR_SETDIBITSTODEVICE: return "SETDIBITSTODEVICE";
    case EMR_STRETCHDIBITS: return "STRETCHDIBITS";
    case EMR_EXTCREATEFONTINDIRECTW: return "EXTCREATEFONTINDIRECTW";
    case EMR_EXTTEXTOUTA: return "EXTTEXTOUTA";
    case EMR_EXTTEXTOUTW: return "EXTTEXTOUTW";
    case EMR_POLYBEZIER16: return "POLYBEZIER16";
    case EMR_POLYGON16: return "POLYGON16";
    case EMR_POLYLINE16: return "POLYLINE16";
    case EMR_POLYBEZIERTO16: return "POLYBEZIERTO16";
    case EMR_POLYLINETO16: return "POLYLINETO16";
    case EMR_POLYPOLYLINE16: return "POLYPOLYLINE16";
    case EMR_POLYPOLYGON16: return "POLYPOLYGON16";
    case EMR_POLYDRAW16: return "POLYDRAW16";
    case EMR_CREATEMONOBRUSH: return "CREATEMONOBRUSH";
    case EMR_CREATEDIBPATTERNBRUSHPT: return "CREATEDIBPATTERNBRUSHPT";
    case EMR_EXTCREATEPEN: return "EXTCREATEPEN";
    case EMR_POLYTEXTOUTA: return "POLYTEXTOUTA";
    case EMR_POLYTEXTOUTW: return "POLYTEXTOUTW";
    case EMR_SETICMMODE: return "SETICMMODE";
    case EMR_CREATECOLORSPACE: return "CREATECOLORSPACE";
    case EMR_SETCOLORSPACE: return "SETCOLORSPACE";
    case EMR_DELETECOLORSPACE: return "DELETECOLORSPACE";
    case EMR_GLSRECORD: return "GLSRECORD";
    case EMR_GLSBOUNDEDRECORD: return "GLSBOUNDEDRECORD";
    case EMR_PIXELFORMAT: return "PIXELFORMAT";
    case EMR_DRAWESCAPE: return "DRAWESCAPE";
    case EMR_EXTESCAPE: return "EXTESCAPE";
    case EMR_STARTDOC: return "STARTDOC";
    case EMR_SMALLTEXTOUT: return "SMALLTEXTOUT";
    case EMR_FORCEUFIMAPPING: return "FORCEUFIMAPPING";
    case EMR_NAMEDESCAPE: return "NAMEDESCAPE";
    case EMR_COLORCORRECTPALETTE: return "COLORCORRECTPALETTE";
    case EMR_SETICMPROFILEA: return "SETICMPROFILEA";
    case EMR_SETICMPROFILEW: return "SETICMPROFILEW";
    case EMR_ALPHABLEND: return "ALPHABLEND";
    case EMR_ALPHADIBBLEND: return "ALPHADIBBLEND";
    case EMR_TRANSPARENTBLT: return "TRANSPARENTBLT";
    case EMR_TRANSPARENTDIB: return "TRANSPARENTDIB";
    case EMR_GRADIENTFILL: return "GRADIENTFILL";
    case EMR_SETLINKEDUFIS: return "SETLINKEDUFIS";
    case EMR_SETTEXTJUSTIFICATION: return "SETTEXTJUSTIFICATION";
    default:
        // Yes, return a pointer to a static buffer. This is a very
        // local debugging output function, so no big deal.
        static char buffer[11];
        sprintf(buffer, "0x%08" SAL_PRIxUINT32, nRecType);
        return buffer;
    }
#endif
}

struct BLENDFUNCTION
{
    unsigned char aBlendOperation;
    unsigned char aBlendFlags;
    unsigned char aSrcConstantAlpha;
    unsigned char aAlphaFormat;

    friend SvStream& operator>>(SvStream& rInStream, BLENDFUNCTION& rBlendFun);
};

SvStream& operator>>(SvStream& rInStream, BLENDFUNCTION& rBlendFun)
{
    rInStream.ReadUChar(rBlendFun.aBlendOperation);
    rInStream.ReadUChar(rBlendFun.aBlendFlags);
    rInStream.ReadUChar(rBlendFun.aSrcConstantAlpha);
    rInStream.ReadUChar(rBlendFun.aAlphaFormat);
    return rInStream;
}

bool ImplReadRegion( tools::PolyPolygon& rPolyPoly, SvStream& rStream, sal_uInt32 nLen )
{
    if (nLen == 0)
        return false;

    sal_uInt32 nHdSize, nType, nCount, nRgnSize, i;
    rStream.ReadUInt32(nHdSize);
    rStream.ReadUInt32(nType);
    rStream.ReadUInt32(nCount);
    rStream.ReadUInt32(nRgnSize);

    if (!rStream.good() || nCount == 0 || nType != RDH_RECTANGLES)
        return false;

    sal_uInt32 nSize;
    if (o3tl::checked_multiply<sal_uInt32>(nCount, 16, nSize))
        return false;
    if (o3tl::checked_add<sal_uInt32>(nSize, nHdSize - 16, nSize))
        return false;
    if (nLen < nSize)
        return false;

    sal_Int32 nx1, ny1, nx2, ny2;
    for (i = 0; i < nCount; i++)
    {
        rStream.ReadInt32(nx1);
        rStream.ReadInt32(ny1);
        rStream.ReadInt32(nx2);
        rStream.ReadInt32(ny2);

        tools::Rectangle aRectangle(Point(nx1, ny1), Point(nx2, ny2));

        tools::Polygon aPolygon(aRectangle);
        tools::PolyPolygon aPolyPolyOr1(aPolygon);
        tools::PolyPolygon aPolyPolyOr2(rPolyPoly);
        rPolyPoly.GetUnion(aPolyPolyOr1, aPolyPolyOr2);
        rPolyPoly = aPolyPolyOr2;
    }
    return true;
}

} // anonymous namespace

namespace emfio
{
    EmfReader::EmfReader(SvStream& rStream,GDIMetaFile& rGDIMetaFile)
        : MtfTools(rGDIMetaFile, rStream)
        , mnRecordCount(0)
        , mbRecordPath(false)
        , mbEMFPlus(false)
        ,mbEMFPlusDualMode(false)
    {
    }

    EmfReader::~EmfReader()
    {
    }

    void EmfReader::ReadEMFPlusComment(sal_uInt32 length, bool& bHaveDC)
    {
        if (!mbEMFPlus)
        {
            PassEMFPlusHeaderInfo();

    #if OSL_DEBUG_LEVEL > 1
            // debug code - write the stream to debug file /tmp/emf-stream.emf
            sal_uInt64 const pos = mpInputStream->Tell();
            mpInputStream->Seek(0);
            SvFileStream file( OUString( "/tmp/emf-stream.emf" ), StreamMode::WRITE | StreamMode::TRUNC );

            mpInputStream->WriteStream(file);
            file.Flush();
            file.Close();

            mpInputStream->Seek( pos );
    #endif

        }

        mbEMFPlus = true;
        sal_uInt64 const pos = mpInputStream->Tell();
        void *buffer = malloc( length );
        PassEMFPlus( buffer, mpInputStream->ReadBytes(buffer, length) );
        free( buffer );
        mpInputStream->Seek( pos );

        bHaveDC = false;

        // skip in SeekRel if impossibly unavailable
        sal_uInt32 nRemainder = length;

        const size_t nRequiredHeaderSize = 12;
        while (nRemainder >= nRequiredHeaderSize)
        {
            sal_uInt16 type(0), flags(0);
            sal_uInt32 size(0), dataSize(0);

            mpInputStream->ReadUInt16( type ).ReadUInt16( flags ).ReadUInt32( size ).ReadUInt32( dataSize );
            nRemainder -= nRequiredHeaderSize;

            SAL_INFO ("emfio", "\t\tEMF+ record type: " << std::hex << type << std::dec);

            // Get Device Context
            // TODO We should use  EmfPlusRecordType::GetDC instead
            if( type == 0x4004 )
            {
                bHaveDC = true;
                SAL_INFO ("emfio", "\t\tEMF+ lock DC (device context)");
            }

            // look for the "dual mode" in header
            // it indicates that either EMF or EMF+ records should be processed
            // 0x4001    = EMF+ header
            // flags & 1 = dual mode active
            if ( type == 0x4001 && flags & 1 )
            {
                mbEMFPlusDualMode = true;
            }

            // Get the length of the remaining data of this record based
            // on the alleged size
            sal_uInt32 nRemainingRecordData = size >= nRequiredHeaderSize ?
                size-nRequiredHeaderSize : 0;
            // clip to available size
            nRemainingRecordData = std::min(nRemainingRecordData, nRemainder);
            mpInputStream->SeekRel(nRemainingRecordData);
            nRemainder -= nRemainingRecordData;
        }
        mpInputStream->SeekRel(nRemainder);
    }

    // these are referenced from inside the templates
    static SvStream& operator >> (SvStream& rStream, sal_Int16 &n)
    {
        return rStream.ReadInt16(n);
    }

    static SvStream& operator >> (SvStream& rStream, sal_Int32 &n)
    {
        return rStream.ReadInt32(n);
    }

    /**
     * Reads polygons from the stream.
     * The \<class T> parameter is for the type of the points (sal_uInt32 or sal_uInt16).
     * skipFirst: if the first point read is the 0th point or the 1st point in the array.
     * */
    template <class T>
    tools::Polygon EmfReader::ReadPolygonWithSkip(const bool skipFirst, sal_uInt32 nNextPos)
    {
        sal_uInt32 nPoints(0), nStartIndex(0);
        mpInputStream->SeekRel( 16 );
        mpInputStream->ReadUInt32( nPoints );
        if (skipFirst)
        {
            nPoints ++;
            nStartIndex ++;
        }

        return ReadPolygon<T>(nStartIndex, nPoints, nNextPos);
    }

    /**
     * Reads polygons from the stream.
     * The \<class T> parameter is for the type of the points
     * nStartIndex: which is the starting index in the polygon of the first point read
     * nPoints: number of points
     * mpInputStream: the stream containing the polygons
     * */
    template <class T>
    tools::Polygon EmfReader::ReadPolygon(sal_uInt32 nStartIndex, sal_uInt32 nPoints, sal_uInt32 nNextPos)
    {
        bool bRecordOk = nPoints <= SAL_MAX_UINT16;
        SAL_WARN_IF(!bRecordOk, "emfio", "polygon record has more polygons than we can handle");
        if (!bRecordOk || !nPoints)
            return tools::Polygon();

        auto nRemainingSize = std::min(nNextPos - mpInputStream->Tell(), mpInputStream->remainingSize());
        auto nMaxPossiblePoints = nRemainingSize / (sizeof(T) * 2);
        auto nPointCount = nPoints - nStartIndex;
        if (nPointCount > nMaxPossiblePoints)
        {
            SAL_WARN("emfio", "polygon claims more points than record can provide, truncating");
            nPoints = nMaxPossiblePoints + nStartIndex;
        }

        tools::Polygon aPolygon(nPoints);
        for (sal_uInt32 i = nStartIndex ; i < nPoints && mpInputStream->good(); i++ )
        {
            T nX, nY;
            *mpInputStream >> nX >> nY;
            if (!mpInputStream->good())
            {
                SAL_WARN("emfio", "short read on polygon, truncating");
                aPolygon.SetSize(i);
                break;
            }
            aPolygon[ i ] = Point( nX, nY );
        }

        return aPolygon;
    }

    /**
     * Reads a polyline from the WMF file and draws it
     * The \<class T> parameter refers to the type of the points. (e.g. sal_uInt16 or sal_uInt32)
     * */
    template <class T>
    void EmfReader::ReadAndDrawPolyLine(sal_uInt32 nNextPos)
    {
        sal_uInt32  nPoints;
        sal_uInt32  i, nNumberOfPolylines( 0 ), nCount( 0 );
        mpInputStream->SeekRel( 0x10 ); // TODO Skipping Bounds. A 128-bit WMF RectL object (specifies the bounding rectangle in device units.)
        mpInputStream->ReadUInt32( nNumberOfPolylines );
        mpInputStream->ReadUInt32( nCount ); // total number of points in all polylines
        const auto nEndPos = std::min(nNextPos, mnEndPos);
        if (mpInputStream->Tell() >= nEndPos)
            return;

        // taking the amount of points of each polygon, retrieving the total number of points
        if ( mpInputStream->good() &&
             ( nNumberOfPolylines < SAL_MAX_UINT32 / sizeof( sal_uInt16 ) ) &&
             ( nNumberOfPolylines * sizeof( sal_uInt16 ) ) <= ( nEndPos - mpInputStream->Tell() )
           )
        {
            std::unique_ptr< sal_uInt32[] > pnPolylinePointCount( new sal_uInt32[ nNumberOfPolylines ] );
            for ( i = 0; i < nNumberOfPolylines && mpInputStream->good(); i++ )
            {
                mpInputStream->ReadUInt32( nPoints );
                pnPolylinePointCount[ i ] = nPoints;
            }
            // Get polyline points:
            for ( i = 0; ( i < nNumberOfPolylines ) && mpInputStream->good(); i++ )
            {
                tools::Polygon aPolygon = ReadPolygon<T>(0, pnPolylinePointCount[i], nNextPos);
                DrawPolyLine(aPolygon, false, mbRecordPath);
            }
        }
    }

    /**
     * Reads a poly polygon from the WMF file and draws it.
     * The \<class T> parameter refers to the type of the points. (e.g. sal_uInt16 or sal_uInt32)
     * */
    template <class T>
    void EmfReader::ReadAndDrawPolyPolygon(sal_uInt32 nNextPos)
    {
        sal_uInt32 nPoly(0), nGesPoints(0), nReadPoints(0);
        mpInputStream->SeekRel( 0x10 );
        // Number of polygons
        mpInputStream->ReadUInt32( nPoly ).ReadUInt32( nGesPoints );
        const auto nEndPos = std::min(nNextPos, mnEndPos);
        if (mpInputStream->Tell() >= nEndPos)
            return;
        if (!mpInputStream->good())
            return;
        //check against numeric overflowing
        if (nGesPoints >= SAL_MAX_UINT32 / sizeof(Point))
            return;
        if (nPoly >= SAL_MAX_UINT32 / sizeof(sal_uInt16))
            return;
        if (nPoly * sizeof(sal_uInt16) > nEndPos - mpInputStream->Tell())
            return;

        // Get number of points in each polygon
        std::vector<sal_uInt16> aPoints(nPoly);
        for (sal_uInt32 i = 0; i < nPoly && mpInputStream->good(); ++i)
        {
            sal_uInt32 nPoints(0);
            mpInputStream->ReadUInt32( nPoints );
            aPoints[i] = static_cast<sal_uInt16>(nPoints);
        }
        if ( mpInputStream->good() && ( nGesPoints * (sizeof(T)+sizeof(T)) ) <= ( nEndPos - mpInputStream->Tell() ) )
        {
            // Get polygon points
            tools::PolyPolygon aPolyPoly(nPoly);
            for (sal_uInt32 i = 0; i < nPoly && mpInputStream->good(); ++i)
            {
                const sal_uInt16 nPointCount(aPoints[i]);
                std::vector<Point> aPtAry(nPointCount);
                for (sal_uInt16 j = 0; j < nPointCount && mpInputStream->good(); ++j)
                {
                    T nX(0), nY(0);
                    *mpInputStream >> nX >> nY;
                    aPtAry[j] = Point( nX, nY );
                    ++nReadPoints;
                }

                aPolyPoly.Insert(tools::Polygon(aPtAry.size(), aPtAry.data()));
            }

            DrawPolyPolygon(aPolyPoly, mbRecordPath);
        }

        OSL_ENSURE(nReadPoints == nGesPoints, "The number Points processed from EMR_POLYPOLYGON is unequal imported number (!)");
    }

    bool EmfReader::ReadEnhWMF()
    {
        sal_uInt32  nStretchBltMode = 0;
        sal_uInt32  nNextPos(0),
                    nW(0), nH(0), nColor(0), nIndex(0),
                    nDat32(0), nNom1(0), nDen1(0), nNom2(0), nDen2(0);
        sal_Int32   nX32(0), nY32(0), nx32(0), ny32(0);

        bool    bStatus = ReadHeader();
        bool    bHaveDC = false;

        static bool bEnableEMFPlus = ( getenv( "EMF_PLUS_DISABLE" ) == nullptr );

        while( bStatus && mnRecordCount-- && mpInputStream->good())
        {
            sal_uInt32  nRecType(0), nRecSize(0);
            mpInputStream->ReadUInt32(nRecType).ReadUInt32(nRecSize);

            if ( !mpInputStream->good() || ( nRecSize < 8 ) || ( nRecSize & 3 ) )     // Parameters are always divisible by 4
            {
                bStatus = false;
                break;
            }

            auto nCurPos = mpInputStream->Tell();

            if (mnEndPos < nCurPos - 8)
            {
                bStatus = false;
                break;
            }

            const sal_uInt32 nMaxPossibleRecSize = mnEndPos - (nCurPos - 8);
            if (nRecSize > nMaxPossibleRecSize)
            {
                bStatus = false;
                break;
            }

            nNextPos = nCurPos + (nRecSize - 8);

            if(  !maBmpSaveList.empty()
              && ( nRecType != EMR_STRETCHBLT )
              && ( nRecType != EMR_STRETCHDIBITS )
              ) {
                ResolveBitmapActions( maBmpSaveList );
            }

            bool bFlag = false;

            SAL_INFO ("emfio", "0x" << std::hex << (nNextPos - nRecSize) <<  "-0x" << nNextPos << " " << record_type_name(nRecType) << " size: " <<  nRecSize << std::dec);

            if( bEnableEMFPlus && nRecType == EMR_COMMENT ) {
                sal_uInt32 length;

                mpInputStream->ReadUInt32( length );

                SAL_INFO("emfio", "\tGDI comment, length: " << length);

                if( mpInputStream->good() && length >= 4 && length <= mpInputStream->remainingSize() ) {
                    sal_uInt32 nCommentId;

                    mpInputStream->ReadUInt32( nCommentId );

                    SAL_INFO ("emfio", "\t\tbegin " << static_cast<char>(nCommentId & 0xff) << static_cast<char>((nCommentId & 0xff00) >> 8) << static_cast<char>((nCommentId & 0xff0000) >> 16) << static_cast<char>((nCommentId & 0xff000000) >> 24) << " id: 0x" << std::hex << nCommentId << std::dec);

                    if( nCommentId == EMR_COMMENT_EMFPLUS && nRecSize >= 12 )
                    {
                        // [MS-EMF] 2.3.3: DataSize includes both CommentIdentifier and CommentRecordParm fields.
                        // We have already read 4-byte CommentIdentifier, so reduce length appropriately
                        ReadEMFPlusComment( length-4, bHaveDC );
                    }
                    else if( nCommentId == EMR_COMMENT_PUBLIC && nRecSize >= 12 )
                    {
                        // TODO: ReadGDIComment()
                    }
                    else if( nCommentId == EMR_COMMENT_EMFSPOOL && nRecSize >= 12 )
                    {
                        // TODO Implement reading EMFSPOOL comment

                    }
                    else
                    {
                        SAL_INFO ("emfio", "\t\tunknown id: 0x" << std::hex << nCommentId << std::dec);
                    }
                }
            }
            else if ( !bHaveDC && mbEMFPlusDualMode && nRecType != EMR_HEADER && nRecType != EMR_EOF )
            {
                // skip content (EMF record) in dual mode
                // we process only EMR_COMMENT (see above) to access EMF+ data
                // with 2 exceptions, according to EMF+ specification:
                // EMR_HEADER and EMR_EOF
                // if a device context is given (bHaveDC) process the following EMF record, too.
            }
            else if( !mbEMFPlus || bHaveDC || nRecType == EMR_EOF )
            {
                switch( nRecType )
                {
                    case EMR_POLYBEZIERTO :
                        DrawPolyBezier(ReadPolygonWithSkip<sal_Int32>(true, nNextPos), true, mbRecordPath);
                    break;
                    case EMR_POLYBEZIER :
                        DrawPolyBezier(ReadPolygonWithSkip<sal_Int32>(false, nNextPos), false, mbRecordPath);
                    break;

                    case EMR_POLYGON :
                        DrawPolygon(ReadPolygonWithSkip<sal_Int32>(false, nNextPos), mbRecordPath);
                    break;

                    case EMR_POLYLINETO :
                        DrawPolyLine(ReadPolygonWithSkip<sal_Int32>(true, nNextPos), true, mbRecordPath);
                    break;

                    case EMR_POLYLINE :
                        DrawPolyLine(ReadPolygonWithSkip<sal_Int32>(false, nNextPos), false, mbRecordPath);
                    break;

                    case EMR_POLYPOLYLINE :
                        ReadAndDrawPolyLine<sal_Int32>(nNextPos);
                    break;

                    case EMR_POLYPOLYGON :
                        ReadAndDrawPolyPolygon<sal_Int32>(nNextPos);
                    break;

                    case EMR_SETWINDOWEXTEX :
                    {
                        sal_Int32 w = 0, h = 0;
                        mpInputStream->ReadInt32( w ).ReadInt32( h );
                        SetWinExt( Size( w, h ), true);
                    }
                    break;

                    case EMR_SETWINDOWORGEX :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        SetWinOrg( Point( nX32, nY32 ), true);
                    }
                    break;

                    case EMR_SCALEWINDOWEXTEX :
                    {
                        mpInputStream->ReadUInt32( nNom1 ).ReadUInt32( nDen1 ).ReadUInt32( nNom2 ).ReadUInt32( nDen2 );
                        if (nDen1 != 0 && nDen2 != 0)
                            ScaleWinExt( static_cast<double>(nNom1) / nDen1, static_cast<double>(nNom2) / nDen2 );
                        else
                            SAL_WARN("vcl.emf", "ignoring bogus divide by zero");
                    }
                    break;

                    case EMR_SETVIEWPORTORGEX :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        SetDevOrg( Point( nX32, nY32 ) );
                    }
                    break;

                    case EMR_SCALEVIEWPORTEXTEX :
                    {
                        mpInputStream->ReadUInt32( nNom1 ).ReadUInt32( nDen1 ).ReadUInt32( nNom2 ).ReadUInt32( nDen2 );
                        if (nDen1 != 0 && nDen2 != 0)
                            ScaleDevExt( static_cast<double>(nNom1) / nDen1, static_cast<double>(nNom2) / nDen2 );
                        else
                            SAL_WARN("vcl.emf", "ignoring bogus divide by zero");
                    }
                    break;

                    case EMR_SETVIEWPORTEXTEX :
                    {
                        sal_Int32 w = 0, h = 0;
                        mpInputStream->ReadInt32( w ).ReadInt32( h );
                        SetDevExt( Size( w, h ) );
                    }
                    break;

                    case EMR_EOF :
                        mnRecordCount = 0;
                    break;

                    case EMR_SETPIXELV :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        DrawPixel( Point( nX32, nY32 ), ReadColor() );
                    }
                    break;

                    case EMR_SETMAPMODE :
                    {
                        sal_uInt32 nMapMode;
                        mpInputStream->ReadUInt32( nMapMode );
                        SetMapMode( nMapMode );
                    }
                    break;

                    case EMR_SETBKMODE :
                    {
                        mpInputStream->ReadUInt32( nDat32 );
                        SetBkMode( static_cast<BkMode>(nDat32) );
                    }
                    break;

                    case EMR_SETPOLYFILLMODE :
                    break;

                    case EMR_SETROP2 :
                    {
                        mpInputStream->ReadUInt32( nDat32 );
                        SetRasterOp( static_cast<WMFRasterOp>(nDat32) );
                    }
                    break;

                    case EMR_SETSTRETCHBLTMODE :
                    {
                        mpInputStream->ReadUInt32( nStretchBltMode );
                    }
                    break;

                    case EMR_SETTEXTALIGN :
                    {
                        mpInputStream->ReadUInt32( nDat32 );
                        SetTextAlign( nDat32 );
                    }
                    break;

                    case EMR_SETTEXTCOLOR :
                    {
                        SetTextColor( ReadColor() );
                    }
                    break;

                    case EMR_SETBKCOLOR :
                    {
                        SetBkColor( ReadColor() );
                    }
                    break;

                    case EMR_OFFSETCLIPRGN :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        MoveClipRegion( Size( nX32, nY32 ) );
                    }
                    break;

                    case EMR_MOVETOEX :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        MoveTo( Point( nX32, nY32 ), mbRecordPath);
                    }
                    break;

                    case EMR_INTERSECTCLIPRECT :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 );
                        IntersectClipRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                    }
                    break;

                    case EMR_SAVEDC :
                    {
                        Push();
                    }
                    break;

                    case EMR_RESTOREDC :
                    {
                        Pop();
                    }
                    break;

                    case EMR_SETWORLDTRANSFORM :
                    {
                        XForm aTempXForm;
                        *mpInputStream >> aTempXForm;
                        SetWorldTransform( aTempXForm );
                    }
                    break;

                    case EMR_MODIFYWORLDTRANSFORM :
                    {
                        sal_uInt32  nMode;
                        XForm   aTempXForm;
                        *mpInputStream >> aTempXForm;
                        mpInputStream->ReadUInt32( nMode );
                        ModifyWorldTransform( aTempXForm, nMode );
                    }
                    break;

                    case EMR_SELECTOBJECT :
                    {
                        mpInputStream->ReadUInt32( nIndex );
                        SelectObject( nIndex );
                    }
                    break;

                    case EMR_CREATEPEN :
                    {
                        mpInputStream->ReadUInt32( nIndex );
                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        {

                            LineInfo    aLineInfo;
                            sal_uInt32      nStyle;
                            Size        aSize;
                            // #fdo39428 Remove SvStream operator>>(long&)
                            sal_Int32 nTmpW(0), nTmpH(0);

                            mpInputStream->ReadUInt32( nStyle ).ReadInt32( nTmpW ).ReadInt32( nTmpH );
                            aSize.setWidth( nTmpW );
                            aSize.setHeight( nTmpH );

                            if ( aSize.Width() )
                                aLineInfo.SetWidth( aSize.Width() );

                            bool bTransparent = false;
                            switch( nStyle & PS_STYLE_MASK )
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
                                case PS_INSIDEFRAME :
                                case PS_SOLID :
                                default :
                                    aLineInfo.SetStyle( LineStyle::Solid );
                            }
                            switch( nStyle & PS_ENDCAP_STYLE_MASK )
                            {
                                case PS_ENDCAP_ROUND :
                                    if ( aSize.Width() )
                                    {
                                        aLineInfo.SetLineCap( css::drawing::LineCap_ROUND );
                                        break;
                                    }
                                    [[fallthrough]];
                                case PS_ENDCAP_SQUARE :
                                    if ( aSize.Width() )
                                    {
                                        aLineInfo.SetLineCap( css::drawing::LineCap_SQUARE );
                                        break;
                                    }
                                    [[fallthrough]];
                                case PS_ENDCAP_FLAT :
                                default :
                                    aLineInfo.SetLineCap( css::drawing::LineCap_BUTT );
                            }
                            switch( nStyle & PS_JOIN_STYLE_MASK )
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
                            CreateObjectIndexed(nIndex, std::make_unique<WinMtfLineStyle>( ReadColor(), aLineInfo, bTransparent ));
                        }
                    }
                    break;

                    case EMR_EXTCREATEPEN :
                    {
                        sal_Int32   elpHatch;
                        sal_uInt32  offBmi, cbBmi, offBits, cbBits, nStyle, nWidth, nBrushStyle, elpNumEntries;
                        Color       aColorRef;

                        mpInputStream->ReadUInt32( nIndex );
                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        {
                            mpInputStream->ReadUInt32( offBmi ).ReadUInt32( cbBmi ).ReadUInt32( offBits ).ReadUInt32( cbBits ). ReadUInt32( nStyle ).ReadUInt32( nWidth ).ReadUInt32( nBrushStyle );
                            aColorRef = ReadColor();
                            mpInputStream->ReadInt32( elpHatch ).ReadUInt32( elpNumEntries );

                            LineInfo    aLineInfo;
                            if ( nWidth )
                                aLineInfo.SetWidth( nWidth );

                            bool bTransparent = false;

                            switch( nStyle & PS_STYLE_MASK )
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

                                case PS_INSIDEFRAME :
                                case PS_SOLID :
                                default :
                                    aLineInfo.SetStyle( LineStyle::Solid );
                            }
                            switch( nStyle & PS_ENDCAP_STYLE_MASK )
                            {
                                case PS_ENDCAP_ROUND :
                                    if ( aLineInfo.GetWidth() )
                                    {
                                        aLineInfo.SetLineCap( css::drawing::LineCap_ROUND );
                                        break;
                                    }
                                    [[fallthrough]];
                                case PS_ENDCAP_SQUARE :
                                    if ( aLineInfo.GetWidth() )
                                    {
                                        aLineInfo.SetLineCap( css::drawing::LineCap_SQUARE );
                                        break;
                                    }
                                    [[fallthrough]];
                                case PS_ENDCAP_FLAT :
                                default :
                                    aLineInfo.SetLineCap( css::drawing::LineCap_BUTT );
                            }
                            switch( nStyle & PS_JOIN_STYLE_MASK )
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
                            CreateObjectIndexed(nIndex, std::make_unique<WinMtfLineStyle>( aColorRef, aLineInfo, bTransparent ));
                        }
                    }
                    break;

                    case EMR_CREATEBRUSHINDIRECT :
                    {
                        sal_uInt32  nStyle;
                        mpInputStream->ReadUInt32( nIndex );
                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        {
                            mpInputStream->ReadUInt32( nStyle );
                            CreateObjectIndexed(nIndex, std::make_unique<WinMtfFillStyle>( ReadColor(), ( nStyle == BS_HOLLOW ) ));
                        }
                    }
                    break;

                    case EMR_DELETEOBJECT :
                    {
                        mpInputStream->ReadUInt32( nIndex );
                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                            DeleteObject( nIndex );
                    }
                    break;

                    case EMR_ELLIPSE :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 );
                        DrawEllipse( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                    }
                    break;

                    case EMR_RECTANGLE :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 );
                        DrawRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                    }
                    break;

                    case EMR_ROUNDRECT :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 ).ReadUInt32( nW ).ReadUInt32( nH );
                        Size aSize( Size( nW, nH ) );
                        DrawRoundRect( ReadRectangle( nX32, nY32, nx32, ny32 ), aSize );
                    }
                    break;

                    case EMR_ARC :
                    {
                        sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 ).ReadUInt32( nStartX ).ReadUInt32( nStartY ).ReadUInt32( nEndX ).ReadUInt32( nEndY );
                        DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                    }
                    break;

                    case EMR_CHORD :
                    {
                        sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 ).ReadUInt32( nStartX ).ReadUInt32( nStartY ).ReadUInt32( nEndX ).ReadUInt32( nEndY );
                        DrawChord( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                    }
                    break;

                    case EMR_PIE :
                    {
                        sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 ).ReadUInt32( nStartX ).ReadUInt32( nStartY ).ReadUInt32( nEndX ).ReadUInt32( nEndY );
                        const tools::Rectangle aRect( ReadRectangle( nX32, nY32, nx32, ny32 ));

                        // #i73608# OutputDevice deviates from WMF
                        // semantics. start==end means full ellipse here.
                        if( nStartX == nEndX && nStartY == nEndY )
                            DrawEllipse( aRect );
                        else
                            DrawPie( aRect, Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                    }
                    break;

                    case EMR_LINETO :
                    {
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 );
                        LineTo( Point( nX32, nY32 ), mbRecordPath);
                    }
                    break;

                    case EMR_ARCTO :
                    {
                        sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                        mpInputStream->ReadInt32( nX32 ).ReadInt32( nY32 ).ReadInt32( nx32 ).ReadInt32( ny32 ).ReadUInt32( nStartX ).ReadUInt32( nStartY ).ReadUInt32( nEndX ).ReadUInt32( nEndY );
                        DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ), true );
                    }
                    break;

                    case EMR_BEGINPATH :
                    {
                        ClearPath();
                        mbRecordPath = true;
                    }
                    break;

                    case EMR_ABORTPATH :
                        ClearPath();
                        [[fallthrough]];
                    case EMR_ENDPATH :
                        mbRecordPath = false;
                    break;

                    case EMR_CLOSEFIGURE :
                        ClosePath();
                    break;

                    case EMR_FILLPATH :
                        StrokeAndFillPath( false, true );
                    break;

                    case EMR_STROKEANDFILLPATH :
                        StrokeAndFillPath( true, true );
                    break;

                    case EMR_STROKEPATH :
                        StrokeAndFillPath( true, false );
                    break;

                    case EMR_SELECTCLIPPATH :
                    {
                        sal_Int32 nClippingMode(0);
                        mpInputStream->ReadInt32(nClippingMode);
                        SetClipPath(GetPathObj(), nClippingMode, true);
                    }
                    break;

                    case EMR_EXTSELECTCLIPRGN :
                    {
                        sal_Int32 nClippingMode(0), cbRgnData(0);
                        mpInputStream->ReadInt32(cbRgnData);
                        mpInputStream->ReadInt32(nClippingMode);

                        // This record's region data should be ignored if mode
                        // is RGN_COPY - see EMF spec section 2.3.2.2
                        if (nClippingMode == RGN_COPY)
                        {
                            SetDefaultClipPath();
                        }
                        else
                        {
                            tools::PolyPolygon aPolyPoly;
                            if (cbRgnData)
                                ImplReadRegion(aPolyPoly, *mpInputStream, nRecSize);
                            SetClipPath(aPolyPoly, nClippingMode, false);
                        }

                    }
                    break;

                    case EMR_ALPHABLEND:
                    {
                        sal_Int32 xDest(0), yDest(0), cxDest(0), cyDest(0);

                        BLENDFUNCTION aFunc;
                        sal_Int32 xSrc(0), ySrc(0), cxSrc(0), cySrc(0);
                        XForm xformSrc;
                        sal_uInt32 BkColorSrc(0), iUsageSrc(0), offBmiSrc(0);
                        sal_uInt32 cbBmiSrc(0), offBitsSrc(0), cbBitsSrc(0);

                        sal_uInt32   nStart = mpInputStream->Tell() - 8;
                        mpInputStream->SeekRel( 0x10 );

                        mpInputStream->ReadInt32( xDest ).ReadInt32( yDest ).ReadInt32( cxDest ).ReadInt32( cyDest );
                        *mpInputStream >> aFunc;
                        mpInputStream->ReadInt32( xSrc ).ReadInt32( ySrc );
                        *mpInputStream >> xformSrc;
                        mpInputStream->ReadUInt32( BkColorSrc ).ReadUInt32( iUsageSrc ).ReadUInt32( offBmiSrc ).ReadUInt32( cbBmiSrc )
                                   .ReadUInt32( offBitsSrc ).ReadUInt32( cbBitsSrc ).ReadInt32( cxSrc ).ReadInt32( cySrc ) ;

                        if ( (cbBitsSrc > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc) ||
                             cxDest == SAL_MAX_INT32 || cyDest == SAL_MAX_INT32 )
                        {
                            bStatus = false;
                        }
                        else
                        {
                            tools::Rectangle aRect(Point(xDest, yDest), Size(cxDest + 1, cyDest + 1));

                            const sal_uInt32 nSourceSize = cbBmiSrc + cbBitsSrc + 14;
                            bool bSafeRead = nSourceSize <= (mnEndPos - mnStartPos);
                            sal_uInt32 nDeltaToDIB5HeaderSize(0);
                            const bool bReadAlpha(0x01 == aFunc.aAlphaFormat);
                            if (bSafeRead && bReadAlpha)
                            {
                                // we need to read alpha channel data if AlphaFormat of BLENDFUNCTION is
                                // AC_SRC_ALPHA (==0x01). To read it, create a temp DIB-File which is ready
                                // for DIB-5 format
                                const sal_uInt32 nHeaderSize = getDIBV5HeaderSize();
                                if (cbBmiSrc > nHeaderSize)
                                    bSafeRead = false;
                                else
                                    nDeltaToDIB5HeaderSize = nHeaderSize - cbBmiSrc;
                            }
                            if (bSafeRead)
                            {
                                const sal_uInt32 nTargetSize(cbBmiSrc + nDeltaToDIB5HeaderSize + cbBitsSrc + 14);
                                char* pBuf = new char[ nTargetSize ];
                                SvMemoryStream aTmp( pBuf, nTargetSize, StreamMode::READ | StreamMode::WRITE );

                                aTmp.ObjectOwnsMemory( true );

                                // write BM-Header (14 bytes)
                                aTmp.WriteUChar( 'B' )
                                    .WriteUChar( 'M' )
                                    .WriteUInt32( cbBitsSrc )
                                    .WriteUInt16( 0 )
                                    .WriteUInt16( 0 )
                                    .WriteUInt32( cbBmiSrc + nDeltaToDIB5HeaderSize + 14 );

                                // copy DIBInfoHeader from source (cbBmiSrc bytes)
                                mpInputStream->Seek( nStart + offBmiSrc );
                                mpInputStream->ReadBytes(pBuf + 14, cbBmiSrc);

                                if (bReadAlpha)
                                {
                                    // need to add values for all stuff that DIBV5Header is bigger
                                    // than DIBInfoHeader, all values are correctly initialized to zero,
                                    // so we can use memset here
                                    memset(pBuf + cbBmiSrc + 14, 0, nDeltaToDIB5HeaderSize);
                                }

                                // copy bitmap data from source (offBitsSrc bytes)
                                mpInputStream->Seek( nStart + offBitsSrc );
                                mpInputStream->ReadBytes(pBuf + 14 + nDeltaToDIB5HeaderSize + cbBmiSrc, cbBitsSrc);
                                aTmp.Seek( 0 );

                                // prepare to read and fill BitmapEx
                                BitmapEx aBitmapEx;

                                if(bReadAlpha)
                                {
                                    Bitmap aBitmap;
                                    AlphaMask aAlpha;

                                    if(ReadDIBV5(aBitmap, aAlpha, aTmp))
                                    {
                                        aBitmapEx = BitmapEx(aBitmap, aAlpha);
                                    }
                                }
                                else
                                {
                                    Bitmap aBitmap;

                                    if(ReadDIB(aBitmap, aTmp, true))
                                    {
                                        if(0xff != aFunc.aSrcConstantAlpha)
                                        {
                                            // add const alpha channel
                                            aBitmapEx = BitmapEx(
                                                aBitmap,
                                                AlphaMask(aBitmap.GetSizePixel(), &aFunc.aSrcConstantAlpha));
                                        }
                                        else
                                        {
                                            // just use Bitmap
                                            aBitmapEx = BitmapEx(aBitmap);
                                        }
                                    }
                                }

                                if(!aBitmapEx.IsEmpty())
                                {
                                    // test if it is sensible to crop
                                    if ( ( cxSrc > 0 ) && ( cySrc > 0 ) &&
                                        ( xSrc >= 0 ) && ( ySrc >= 0 ) &&
                                            ( xSrc + cxSrc < aBitmapEx.GetSizePixel().Width() ) &&
                                                ( ySrc + cySrc < aBitmapEx.GetSizePixel().Height() ) )
                                    {
                                        const tools::Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );

                                        aBitmapEx.Crop( aCropRect );
                                    }

    #ifdef DBG_UTIL
                                    static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore

                                    if(bDoSaveForVisualControl)
                                    {
                                        SvFileStream aNew("c:\\metafile_content.png", StreamMode::WRITE|StreamMode::TRUNC);
                                        vcl::PNGWriter aPNGWriter(aBitmapEx);
                                        aPNGWriter.Write(aNew);
                                    }
    #endif
                                    maBmpSaveList.emplace_back(new BSaveStruct(aBitmapEx, aRect, SRCAND|SRCINVERT));
                                }
                            }
                        }
                    }
                    break;

                    case EMR_BITBLT :   // PASSTHROUGH INTENDED
                    case EMR_STRETCHBLT :
                    {
                        sal_Int32   xDest, yDest, cxDest, cyDest, xSrc, ySrc, cxSrc, cySrc;
                        sal_uInt32  dwRop, iUsageSrc, offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc;
                        XForm   xformSrc;

                        sal_uInt32  nStart = mpInputStream->Tell() - 8;

                        mpInputStream->SeekRel( 0x10 );
                        mpInputStream->ReadInt32( xDest ).ReadInt32( yDest ).ReadInt32( cxDest ).ReadInt32( cyDest ).ReadUInt32( dwRop ).ReadInt32( xSrc ).ReadInt32( ySrc )
                                >> xformSrc;
                        mpInputStream->ReadUInt32( nColor ).ReadUInt32( iUsageSrc ).ReadUInt32( offBmiSrc ).ReadUInt32( cbBmiSrc )
                                   .ReadUInt32( offBitsSrc ).ReadUInt32( cbBitsSrc );

                        if ( nRecType == EMR_STRETCHBLT )
                            mpInputStream->ReadInt32( cxSrc ).ReadInt32( cySrc );
                        else
                            cxSrc = cySrc = 0;

                        Bitmap      aBitmap;
                        tools::Rectangle   aRect( Point( xDest, yDest ), Size( cxDest, cyDest ) );

                        if (!mpInputStream->good() || (cbBitsSrc > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc))
                            bStatus = false;
                        else
                        {
                            sal_uInt32 nSize = cbBmiSrc + cbBitsSrc + 14;
                            if ( nSize <= ( mnEndPos - mnStartPos ) )
                            {
                                char* pBuf = new char[ nSize ];
                                SvMemoryStream aTmp( pBuf, nSize, StreamMode::READ | StreamMode::WRITE );
                                aTmp.ObjectOwnsMemory( true );
                                aTmp.WriteUChar( 'B' )
                                    .WriteUChar( 'M' )
                                    .WriteUInt32( cbBitsSrc )
                                    .WriteUInt16( 0 )
                                    .WriteUInt16( 0 )
                                    .WriteUInt32( cbBmiSrc + 14 );
                                mpInputStream->Seek( nStart + offBmiSrc );
                                mpInputStream->ReadBytes(pBuf + 14, cbBmiSrc);
                                mpInputStream->Seek( nStart + offBitsSrc );
                                mpInputStream->ReadBytes(pBuf + 14 + cbBmiSrc, cbBitsSrc);
                                aTmp.Seek( 0 );
                                ReadDIB(aBitmap, aTmp, true);

                                // test if it is sensible to crop
                                if ( (cxSrc > 0) && (cySrc > 0) &&
                                     (xSrc >= 0) && (ySrc >= 0) &&
                                     (aBitmap.GetSizePixel().Width() >= cxSrc) &&
                                     (xSrc <= aBitmap.GetSizePixel().Width() - cxSrc) &&
                                     (aBitmap.GetSizePixel().Height() >= cySrc) &&
                                     (ySrc <= aBitmap.GetSizePixel().Height() - cySrc) )
                                {
                                    tools::Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );
                                    aBitmap.Crop( aCropRect );
                                }

                                maBmpSaveList.emplace_back(new BSaveStruct(aBitmap, aRect, dwRop));
                            }
                        }
                    }
                    break;

                    case EMR_STRETCHDIBITS :
                    {
                        sal_Int32   xDest, yDest, xSrc, ySrc, cxSrc, cySrc, cxDest, cyDest;
                        sal_uInt32  offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, iUsageSrc, dwRop;
                        sal_uInt32  nStart = mpInputStream->Tell() - 8;

                        mpInputStream->SeekRel( 0x10 );
                        mpInputStream->ReadInt32( xDest )
                             .ReadInt32( yDest )
                             .ReadInt32( xSrc )
                             .ReadInt32( ySrc )
                             .ReadInt32( cxSrc )
                             .ReadInt32( cySrc )
                             .ReadUInt32( offBmiSrc )
                             .ReadUInt32( cbBmiSrc )
                             .ReadUInt32( offBitsSrc )
                             .ReadUInt32( cbBitsSrc )
                             .ReadUInt32( iUsageSrc )
                             .ReadUInt32( dwRop )
                             .ReadInt32( cxDest )
                             .ReadInt32( cyDest );

                        if (!mpInputStream->good() ||
                            ((SAL_MAX_UINT32 - 14) < cbBitsSrc) ||
                            ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc))
                        {
                            bStatus = false;
                        }
                        else
                        {
                            Bitmap aBitmap;
                            tools::Rectangle aRect(xDest, yDest);
                            aRect.SaturatingSetSize(Size(cxDest, cyDest));

                            sal_uInt32 nSize = cbBmiSrc + cbBitsSrc + 14;
                            if ( nSize <= ( mnEndPos - mnStartPos ) )
                            {
                                char* pBuf = new char[ nSize ];
                                SvMemoryStream aTmp( pBuf, nSize, StreamMode::READ | StreamMode::WRITE );
                                aTmp.ObjectOwnsMemory( true );
                                aTmp.WriteUChar( 'B' )
                                   .WriteUChar( 'M' )
                                   .WriteUInt32( cbBitsSrc )
                                   .WriteUInt16( 0 )
                                   .WriteUInt16( 0 )
                                   .WriteUInt32( cbBmiSrc + 14 );
                                mpInputStream->Seek( nStart + offBmiSrc );
                                mpInputStream->ReadBytes(pBuf + 14, cbBmiSrc);
                                mpInputStream->Seek( nStart + offBitsSrc );
                                mpInputStream->ReadBytes(pBuf + 14 + cbBmiSrc, cbBitsSrc);
                                aTmp.Seek( 0 );
                                ReadDIB(aBitmap, aTmp, true);

                                // test if it is sensible to crop
                                if ( (cxSrc > 0) && (cySrc > 0) &&
                                     (xSrc >= 0) && (ySrc >= 0) &&
                                     (aBitmap.GetSizePixel().Width() >= cxSrc) &&
                                     (xSrc <= aBitmap.GetSizePixel().Width() - cxSrc) &&
                                     (aBitmap.GetSizePixel().Height() >= cySrc) &&
                                     (ySrc <= aBitmap.GetSizePixel().Height() - cySrc) )
                                {
                                    tools::Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );
                                    aBitmap.Crop( aCropRect );
                                }
                                maBmpSaveList.emplace_back(new BSaveStruct(aBitmap, aRect, dwRop));
                            }
                        }
                    }
                    break;

                    case EMR_EXTCREATEFONTINDIRECTW :
                    {
                        mpInputStream->ReadUInt32( nIndex );
                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        {
                            LOGFONTW aLogFont;
                            mpInputStream->ReadInt32( aLogFont.lfHeight )
                                 .ReadInt32( aLogFont.lfWidth )
                                 .ReadInt32( aLogFont.lfEscapement )
                                 .ReadInt32( aLogFont.lfOrientation )
                                 .ReadInt32( aLogFont.lfWeight )
                                 .ReadUChar( aLogFont.lfItalic )
                                 .ReadUChar( aLogFont.lfUnderline )
                                 .ReadUChar( aLogFont.lfStrikeOut )
                                 .ReadUChar( aLogFont.lfCharSet )
                                 .ReadUChar( aLogFont.lfOutPrecision )
                                 .ReadUChar( aLogFont.lfClipPrecision )
                                 .ReadUChar( aLogFont.lfQuality )
                                 .ReadUChar( aLogFont.lfPitchAndFamily );

                            sal_Unicode lfFaceName[LF_FACESIZE+1];
                            lfFaceName[LF_FACESIZE] = 0;
                            for (int i = 0; i < LF_FACESIZE; ++i)
                            {
                                sal_uInt16 nChar(0);
                                mpInputStream->ReadUInt16(nChar);
                                lfFaceName[i] = nChar;
                            }
                            aLogFont.alfFaceName = OUString( lfFaceName );

                            // #i123216# Not used in the test case of #121382# (always identity in XForm), also
                            // no hints in ms docu if FontSize should be scaled with WT. Using with the example
                            // from #i123216# creates errors, so removing.

                            // // #i121382# Need to apply WorldTransform to FontHeight/Width; this should be completely
                            // // changed to basegfx::B2DHomMatrix instead of 'struct XForm', but not now due to time
                            // // constraints and dangers
                            // const XForm& rXF = GetWorldTransform();
                            // const basegfx::B2DHomMatrix aWT(rXF.eM11, rXF.eM21, rXF.eDx, rXF.eM12, rXF.eM22, rXF.eDy);
                            // const basegfx::B2DVector aTransVec(aWT * basegfx::B2DVector(aLogFont.lfWidth, aLogFont.lfHeight));
                            // aLogFont.lfWidth = aTransVec.getX();
                            // aLogFont.lfHeight = aTransVec.getY();
                            if (mpInputStream->good() && aLogFont.lfHeight != SAL_MIN_INT32 && aLogFont.lfWidth != SAL_MIN_INT32)
                            {
                                CreateObjectIndexed(nIndex, std::make_unique<WinMtfFontStyle>( aLogFont ));
                            }
                        }
                    }
                    break;

                    case EMR_EXTTEXTOUTA :
                        bFlag = true;
                        [[fallthrough]];
                    case EMR_EXTTEXTOUTW :
                    {
                        sal_Int32   nLeft, nTop, nRight, nBottom, ptlReferenceX, ptlReferenceY, nGfxMode, nXScale, nYScale;
                        sal_uInt32  nOffString, nOptions, offDx;
                        sal_Int32   nLen;

                        nCurPos = mpInputStream->Tell() - 8;

                        mpInputStream->ReadInt32( nLeft ).ReadInt32( nTop ).ReadInt32( nRight ).ReadInt32( nBottom ).ReadInt32( nGfxMode ).ReadInt32( nXScale ).ReadInt32( nYScale )
                           .ReadInt32( ptlReferenceX ).ReadInt32( ptlReferenceY ).ReadInt32( nLen ).ReadUInt32( nOffString ).ReadUInt32( nOptions );

                        mpInputStream->SeekRel( 0x10 );
                        mpInputStream->ReadUInt32( offDx );

                        ComplexTextLayoutFlags nTextLayoutMode = ComplexTextLayoutFlags::Default;
                        if ( nOptions & ETO_RTLREADING )
                            nTextLayoutMode = ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::TextOriginLeft;
                        SetTextLayoutMode( nTextLayoutMode );
                        SAL_WARN_IF( ( nOptions & ( ETO_PDY | ETO_GLYPH_INDEX ) ) != 0, "emfio", "SJ: ETO_PDY || ETO_GLYPH_INDEX in EMF" );

                        Point aPos( ptlReferenceX, ptlReferenceY );
                        bool bLenSane = nLen > 0 && nLen < static_cast<sal_Int32>( SAL_MAX_UINT32 / sizeof(sal_Int32) );
                        bool bOffStringSane = nOffString <= mnEndPos - nCurPos;
                        if (bLenSane && bOffStringSane)
                        {
                            mpInputStream->Seek( nCurPos + nOffString );
                            OUString aText;
                            if ( bFlag )
                            {
                                if ( nLen <= static_cast<sal_Int32>( mnEndPos - mpInputStream->Tell() ) )
                                {
                                    std::unique_ptr<sal_Char[]> pBuf(new sal_Char[ nLen ]);
                                    mpInputStream->ReadBytes(pBuf.get(), nLen);
                                    aText = OUString(pBuf.get(), nLen, GetCharSet());
                                }
                            }
                            else
                            {
                                if ( ( nLen * sizeof(sal_Unicode) ) <= ( mnEndPos - mpInputStream->Tell() ) )
                                {
                                    aText = read_uInt16s_ToOUString(*mpInputStream, nLen);
                                }
                            }

                            std::unique_ptr<long[]> pDXAry, pDYAry;

                            sal_Int32 nDxSize;
                            bool bOverflow = o3tl::checked_multiply<sal_Int32>(nLen, (nOptions & ETO_PDY) ? 8 : 4, nDxSize);
                            if (!bOverflow && offDx && ((nCurPos + offDx + nDxSize) <= nNextPos ) && nNextPos <= mnEndPos)
                            {
                                mpInputStream->Seek( nCurPos + offDx );
                                pDXAry.reset( new long[aText.getLength()] );
                                if (nOptions & ETO_PDY)
                                {
                                    pDYAry.reset( new long[aText.getLength()] );
                                }

                                for (sal_Int32 i = 0; i < aText.getLength(); ++i)
                                {
                                    sal_Int32 nDxCount = 1;
                                    if (aText.getLength() != nLen)
                                    {
                                        sal_Unicode cUniChar = aText[i];
                                        OString aTmp(&cUniChar, 1, GetCharSet());
                                        if (aTmp.getLength() > 1)
                                        {
                                            nDxCount = aTmp.getLength();
                                        }
                                    }

                                    sal_Int32 nDx = 0, nDy = 0;
                                    while (nDxCount--)
                                    {
                                        sal_Int32 nDxTmp = 0;
                                        mpInputStream->ReadInt32(nDxTmp);
                                        nDx += nDxTmp;
                                        if (nOptions & ETO_PDY)
                                        {
                                            sal_Int32 nDyTmp = 0;
                                            mpInputStream->ReadInt32(nDyTmp);
                                            nDy += nDyTmp;
                                        }
                                    }

                                    pDXAry[i] = nDx;
                                    if (nOptions & ETO_PDY)
                                    {
                                        pDYAry[i] = nDy;
                                    }
                                }
                            }
                            DrawText(aPos, aText, pDXAry.get(), pDYAry.get(), mbRecordPath, nGfxMode);
                        }
                    }
                    break;

                    case EMR_POLYBEZIERTO16 :
                        DrawPolyBezier(ReadPolygonWithSkip<sal_Int16>(true, nNextPos), true, mbRecordPath);
                    break;

                    case EMR_POLYBEZIER16 :
                        DrawPolyBezier(ReadPolygonWithSkip<sal_Int16>(false, nNextPos), false, mbRecordPath);
                    break;

                    case EMR_POLYGON16 :
                        DrawPolygon(ReadPolygonWithSkip<sal_Int16>(false, nNextPos), mbRecordPath);
                    break;

                    case EMR_POLYLINETO16 :
                        DrawPolyLine(ReadPolygonWithSkip<sal_Int16>(true, nNextPos), true, mbRecordPath);
                    break;

                    case EMR_POLYLINE16 :
                        DrawPolyLine(ReadPolygonWithSkip<sal_Int16>(false, nNextPos), false, mbRecordPath);
                    break;

                    case EMR_POLYPOLYLINE16 :
                        ReadAndDrawPolyLine<sal_Int16>(nNextPos);
                    break;

                    case EMR_POLYPOLYGON16 :
                        ReadAndDrawPolyPolygon<sal_Int16>(nNextPos);
                    break;

                    case EMR_FILLRGN :
                    {
                        sal_uInt32 nLen;
                        tools::PolyPolygon aPolyPoly;
                        mpInputStream->SeekRel( 0x10 );
                        mpInputStream->ReadUInt32( nLen ).ReadUInt32( nIndex );

                        if ( ImplReadRegion( aPolyPoly, *mpInputStream, nRecSize ) )
                        {
                            Push();
                            SelectObject( nIndex );
                            DrawPolyPolygon( aPolyPoly );
                            Pop();
                        }
                    }
                    break;

                    case EMR_CREATEDIBPATTERNBRUSHPT :
                    {
                        sal_uInt32  nStart = mpInputStream->Tell() - 8;
                        Bitmap aBitmap;

                        mpInputStream->ReadUInt32( nIndex );

                        if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        {
                            sal_uInt32 usage, offBmi, cbBmi, offBits, cbBits;

                            mpInputStream->ReadUInt32( usage );
                            mpInputStream->ReadUInt32( offBmi );
                            mpInputStream->ReadUInt32( cbBmi );
                            mpInputStream->ReadUInt32( offBits );
                            mpInputStream->ReadUInt32( cbBits );

                            if ( (cbBits > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBits < cbBmi) )
                               bStatus = false;
                            else if ( offBmi )
                            {
                                sal_uInt32  nSize = cbBmi + cbBits + 14;
                                if ( nSize <= ( mnEndPos - mnStartPos ) )
                                {
                                    char*   pBuf = new char[ nSize ];

                                    SvMemoryStream aTmp( pBuf, nSize, StreamMode::READ | StreamMode::WRITE );
                                    aTmp.ObjectOwnsMemory( true );
                                    aTmp.WriteUChar( 'B' )
                                        .WriteUChar( 'M' )
                                        .WriteUInt32( cbBits )
                                        .WriteUInt16( 0 )
                                        .WriteUInt16( 0 )
                                        .WriteUInt32( cbBmi + 14 );
                                    mpInputStream->Seek( nStart + offBmi );
                                    mpInputStream->ReadBytes(pBuf + 14, cbBmi);
                                    mpInputStream->Seek( nStart + offBits );
                                    mpInputStream->ReadBytes(pBuf + 14 + cbBmi, cbBits);
                                    aTmp.Seek( 0 );
                                    ReadDIB(aBitmap, aTmp, true);
                                }
                            }
                        }

                        CreateObjectIndexed(nIndex, std::make_unique<WinMtfFillStyle>( aBitmap ));
                    }
                    break;

                    case EMR_MASKBLT :                  SAL_INFO("emfio", "not implemented 'MaskBlt'");                   break;
                    case EMR_PLGBLT :                   SAL_INFO("emfio", "not implemented 'PlgBlt'");                    break;
                    case EMR_SETDIBITSTODEVICE :        SAL_INFO("emfio", "not implemented 'SetDIBitsToDevice'");         break;
                    case EMR_FRAMERGN :                 SAL_INFO("emfio", "not implemented 'FrameRgn'");                  break;
                    case EMR_INVERTRGN :                SAL_INFO("emfio", "not implemented 'InvertRgn'");                 break;
                    case EMR_PAINTRGN :                 SAL_INFO("emfio", "not implemented 'PaintRgn'");                  break;
                    case EMR_FLATTENPATH :              SAL_INFO("emfio", "not implemented 'FlattenPath'");               break;
                    case EMR_WIDENPATH :                SAL_INFO("emfio", "not implemented 'WidenPath'");                 break;
                    case EMR_POLYDRAW :                 SAL_INFO("emfio", "not implemented 'Polydraw'");                  break;
                    case EMR_SETARCDIRECTION :          SAL_INFO("emfio", "not implemented 'SetArcDirection'");           break;
                    case EMR_SETPALETTEENTRIES :        SAL_INFO("emfio", "not implemented 'SetPaletteEntries'");         break;
                    case EMR_RESIZEPALETTE :            SAL_INFO("emfio", "not implemented 'ResizePalette'");             break;
                    case EMR_EXTFLOODFILL :             SAL_INFO("emfio", "not implemented 'ExtFloodFill'");              break;
                    case EMR_ANGLEARC :                 SAL_INFO("emfio", "not implemented 'AngleArc'");                  break;
                    case EMR_SETCOLORADJUSTMENT :       SAL_INFO("emfio", "not implemented 'SetColorAdjustment'");        break;
                    case EMR_POLYDRAW16 :               SAL_INFO("emfio", "not implemented 'PolyDraw16'");                break;
                    case EMR_POLYTEXTOUTA :             SAL_INFO("emfio", "not implemented 'PolyTextOutA'");              break;
                    case EMR_POLYTEXTOUTW :             SAL_INFO("emfio", "not implemented 'PolyTextOutW'");              break;
                    case EMR_CREATECOLORSPACE :         SAL_INFO("emfio", "not implemented 'CreateColorSpace'");          break;
                    case EMR_SETCOLORSPACE :            SAL_INFO("emfio", "not implemented 'SetColorSpace'");             break;
                    case EMR_DELETECOLORSPACE :         SAL_INFO("emfio", "not implemented 'DeleteColorSpace'");          break;
                    case EMR_GLSRECORD :                SAL_INFO("emfio", "not implemented 'GlsRecord'");                 break;
                    case EMR_GLSBOUNDEDRECORD :         SAL_INFO("emfio", "not implemented 'GlsBoundRecord'");            break;
                    case EMR_PIXELFORMAT :              SAL_INFO("emfio", "not implemented 'PixelFormat'");               break;
                    case EMR_DRAWESCAPE :               SAL_INFO("emfio", "not implemented 'DrawEscape'");                break;
                    case EMR_EXTESCAPE :                SAL_INFO("emfio", "not implemented 'ExtEscape'");                 break;
                    case EMR_STARTDOC :                 SAL_INFO("emfio", "not implemented 'StartDoc'");                  break;
                    case EMR_SMALLTEXTOUT :             SAL_INFO("emfio", "not implemented 'SmallTextOut'");              break;
                    case EMR_FORCEUFIMAPPING :          SAL_INFO("emfio", "not implemented 'ForceUFIMapping'");           break;
                    case EMR_NAMEDESCAPE :              SAL_INFO("emfio", "not implemented 'NamedEscape'");               break;
                    case EMR_COLORCORRECTPALETTE :      SAL_INFO("emfio", "not implemented 'ColorCorrectPalette'");       break;
                    case EMR_SETICMPROFILEA :           SAL_INFO("emfio", "not implemented 'SetICMProfileA'");            break;
                    case EMR_SETICMPROFILEW :           SAL_INFO("emfio", "not implemented 'SetICMProfileW'");            break;
                    case EMR_TRANSPARENTBLT :           SAL_INFO("emfio", "not implemented 'TransparenBlt'");             break;
                    case EMR_TRANSPARENTDIB :           SAL_INFO("emfio", "not implemented 'TransparenDib'");             break;
                    case EMR_GRADIENTFILL :             SAL_INFO("emfio", "not implemented 'GradientFill'");              break;
                    case EMR_SETLINKEDUFIS :            SAL_INFO("emfio", "not implemented 'SetLinkedUFIS'");             break;

                    case EMR_SETMAPPERFLAGS :           SAL_INFO("emfio", "not implemented 'SetMapperFlags'");            break;
                    case EMR_SETICMMODE :               SAL_INFO("emfio", "not implemented 'SetICMMode'");                break;
                    case EMR_CREATEMONOBRUSH :          SAL_INFO("emfio", "not implemented 'CreateMonoBrush'");           break;
                    case EMR_SETBRUSHORGEX :            SAL_INFO("emfio", "not implemented 'SetBrushOrgEx'");             break;
                    case EMR_SETMETARGN :               SAL_INFO("emfio", "not implemented 'SetMetArgn'");                break;
                    case EMR_SETMITERLIMIT :            SAL_INFO("emfio", "not implemented 'SetMiterLimit'");             break;
                    case EMR_EXCLUDECLIPRECT :          SAL_INFO("emfio", "not implemented 'ExcludeClipRect'");           break;
                    case EMR_REALIZEPALETTE :           SAL_INFO("emfio", "not implemented 'RealizePalette'");            break;
                    case EMR_SELECTPALETTE :            SAL_INFO("emfio", "not implemented 'SelectPalette'");             break;
                    case EMR_CREATEPALETTE :            SAL_INFO("emfio", "not implemented 'CreatePalette'");             break;
                    case EMR_ALPHADIBBLEND :            SAL_INFO("emfio", "not implemented 'AlphaDibBlend'");             break;
                    case EMR_SETTEXTJUSTIFICATION :     SAL_INFO("emfio", "not implemented 'SetTextJustification'");      break;

                    case EMR_COMMENT :
                    case EMR_HEADER :               // has already been read at ReadHeader()
                    break;

                    default :                           SAL_INFO("emfio", "Unknown Meta Action");                                     break;
                }
            }
            mpInputStream->Seek( nNextPos );
        }
        if( !maBmpSaveList.empty() )
            ResolveBitmapActions( maBmpSaveList );

        if ( bStatus )
            mpInputStream->Seek(mnEndPos);

        return bStatus;
    };

    bool EmfReader::ReadHeader()
    {
        // Spare me the METAFILEHEADER here
        // Reading the METAHEADER - EMR_HEADER ([MS-EMF] section 2.3.4.2 EMR_HEADER Record Types)
        sal_uInt32 nType(0), nHeaderSize(0);
        mpInputStream->ReadUInt32(nType).ReadUInt32(nHeaderSize);
        if (nType != 0x00000001)
        {
            // per [MS-EMF] 2.3.4.2 EMF Header Record Types, type MUST be 0x00000001
            SAL_WARN("emfio", "EMF header type is not set to 0x00000001 - possibly corrupted file?");
            return false;
        }

        // Start reading the EMR_HEADER Header object

        // bound size (RectL object, see [MS-WMF] section 2.2.2.19)
        tools::Rectangle rclBounds = ReadRectangle(); // rectangle in logical units

        // picture frame size (RectL object)
        tools::Rectangle rclFrame = ReadRectangle(); // rectangle in device units 1/100th mm

        sal_uInt32 nSignature(0);
        mpInputStream->ReadUInt32(nSignature);

        // nSignature MUST be the ASCII characters "FME", see [WS-EMF] 2.2.9 Header Object
        // and 2.1.14 FormatSignature Enumeration
        if (nSignature != 0x464d4520)
        {
            SAL_WARN("emfio", "EMF\t\tSignature is not 0x464d4520 (\"FME\") - possibly corrupted file?");
            return false;
        }

        sal_uInt32 nVersion(0);
        mpInputStream->ReadUInt32(nVersion);  // according to [WS-EMF] 2.2.9, this SHOULD be 0x0001000, however
                                       // Microsoft note that not even Windows checks this...
        if (nVersion != 0x00010000)
        {
            SAL_WARN("emfio", "EMF\t\tThis really should be 0x00010000, though not absolutely essential...");
        }

        mpInputStream->ReadUInt32(mnEndPos); // size of metafile
        mnEndPos += mnStartPos;

        sal_uInt32 nStrmPos = mpInputStream->Tell(); // checking if mnEndPos is valid
        sal_uInt32 nActualFileSize = nStrmPos + mpInputStream->remainingSize();

        if ( nActualFileSize < mnEndPos )
        {
            SAL_WARN("emfio", "EMF\t\tEMF Header object records number of bytes as " << mnEndPos
                                << ", however the file size is actually " << nActualFileSize
                                << " bytes. Possible file corruption?");
            mnEndPos = nActualFileSize;
        }

        mpInputStream->ReadInt32(mnRecordCount);

        if (mnRecordCount <= 0)
        {
            SAL_WARN("emfio", "EMF\t\tEMF Header object shows record counter as <= 0! This shouldn't "
                                "be possible... indicator of possible file corruption?");
            return false;
        }

        // the number of "handles", or graphics objects used in the metafile

        sal_uInt16 nHandlesCount;
        mpInputStream->ReadUInt16(nHandlesCount);

        // the next 2 bytes are reserved, but according to [MS-EMF] section 2.2.9
        // it MUST be 0x000 and MUST be ignored... the thing is, having such a specific
        // value is actually pretty useful in checking if there is possible corruption

        sal_uInt16 nReserved(0);
        mpInputStream->ReadUInt16(nReserved);

        if ( nReserved != 0x0000 )
        {
            SAL_WARN("emfio", "EMF\t\tEMF Header object's reserved field is NOT 0x0000... possible "
                                "corruption?");
        }

        // The next 4 bytes specifies the number of characters in the metafile description.
        // The 4 bytes after that specific the offset from this record that contains the
        // metafile description... zero means no description string.
        // For now, we ignore it.

        mpInputStream->SeekRel(0x8);

        sal_uInt32 nPalEntries(0);
        mpInputStream->ReadUInt32(nPalEntries);
        sal_Int32 nPixX(0), nPixY(0), nMillX(0), nMillY(0);
        mpInputStream->ReadInt32(nPixX);
        mpInputStream->ReadInt32(nPixY);
        mpInputStream->ReadInt32(nMillX);
        mpInputStream->ReadInt32(nMillY);

        SetrclFrame(rclFrame);
        SetrclBounds(rclBounds);
        SetRefPix(Size( nPixX, nPixY ) );
        SetRefMill(Size( nMillX, nMillY ) );

        return checkSeek(*mpInputStream, mnStartPos + nHeaderSize);
    }

    tools::Rectangle EmfReader::ReadRectangle()
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        mpInputStream->ReadInt32(nLeft);
        mpInputStream->ReadInt32(nTop);
        mpInputStream->ReadInt32(nRight);
        mpInputStream->ReadInt32(nBottom);
        return tools::Rectangle(nLeft, nTop, nRight, nBottom);
    }

    tools::Rectangle EmfReader::ReadRectangle( sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 )
    {
        Point aTL(x1, y1);
        Point aBR(o3tl::saturating_add<sal_Int32>(x2, -1), o3tl::saturating_add<sal_Int32>(y2, -1));
        return tools::Rectangle(aTL, aBR);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
