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


#include "winmtf.hxx"
#include <osl/endian.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <boost/bind.hpp>
#include <vcl/dibtools.hxx>

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

#define EMR_GDICOMMENT                  70
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


#ifdef OSL_BIGENDIAN
// currently unused
static float GetSwapFloat( SvStream& rSt )
{
    float   fTmp;
    sal_Int8* pPtr = (sal_Int8*)&fTmp;
    rSt >> pPtr[3] >> pPtr[2] >> pPtr[1] >> pPtr[0];    // Little Endian <-> Big Endian switch
    return fTmp;
}
#endif

struct BLENDFUNCTION{
    unsigned char aBlendOperation;
    unsigned char aBlendFlags;
    unsigned char aSrcConstantAlpha;
    unsigned char aAlphaFormat;

    friend SvStream& operator>>( SvStream& rIn, BLENDFUNCTION& rBlendFun );
};

SvStream& operator>>( SvStream& rIn, BLENDFUNCTION& rBlendFun )
{
    rIn >> rBlendFun.aBlendOperation >> rBlendFun.aBlendFlags >>
             rBlendFun.aSrcConstantAlpha >> rBlendFun.aAlphaFormat;
    return rIn;
}
SvStream& operator>>( SvStream& rIn, XForm& rXForm )
{
    if ( sizeof( float ) != 4 )
    {
        OSL_FAIL( "EnhWMFReader::sizeof( float ) != 4" );
        rXForm = XForm();
    }
    else
    {
#ifdef OSL_BIGENDIAN
    rXForm.eM11 = GetSwapFloat( rIn );
    rXForm.eM12 = GetSwapFloat( rIn );
    rXForm.eM21 = GetSwapFloat( rIn );
    rXForm.eM22 = GetSwapFloat( rIn );
    rXForm.eDx = GetSwapFloat( rIn );
    rXForm.eDy = GetSwapFloat( rIn );
#else
    rIn >> rXForm.eM11 >> rXForm.eM12 >> rXForm.eM21 >> rXForm.eM22
            >> rXForm.eDx >> rXForm.eDy;
#endif
    }
    return rIn;
}

static bool ImplReadRegion( PolyPolygon& rPolyPoly, SvStream& rSt, sal_uInt32 nLen )
{
    bool bOk = false;
    if ( nLen )
    {
        sal_uInt32 nHdSize, nType, nCount, nRgnSize, i;
        rSt >> nHdSize
            >> nType
            >> nCount
            >> nRgnSize;

        if ( nCount && ( nType == RDH_RECTANGLES ) &&
                ( nLen >= ( ( nCount << 4 ) + ( nHdSize - 16 ) ) ) )
        {
            sal_Int32 nx1, ny1, nx2, ny2;

            for ( i = 0; i < nCount; i++ )
            {
                rSt >> nx1 >> ny1 >> nx2 >> ny2;

                Rectangle aRect( Point( nx1, ny1 ), Point( nx2, ny2 ) );
                Polygon aPolygon( aRect );
                PolyPolygon aPolyPolyOr1( aPolygon );
                PolyPolygon aPolyPolyOr2( rPolyPoly );
                rPolyPoly.GetUnion( aPolyPolyOr1, aPolyPolyOr2 );
                rPolyPoly = aPolyPolyOr2;
            }
            bOk = true;
        }
    }
    return bOk;
}

#if OSL_DEBUG_LEVEL > 1
void dumpWords( SvStream& s, int i )
{
    sal_uInt32 pos = s.Tell();
    sal_Int16 data;
    for( ; i > 0; i -- ) {
        s >> data;
        SAL_INFO("vcl.emf", "\t\t\tdata: " << std::hex << data << std::dec);
    }
    s.Seek (pos);
};
#endif

void EnhWMFReader::ReadEMFPlusComment(sal_uInt32 length, sal_Bool& bHaveDC)
{
    if (!bEMFPlus) {
        pOut->PassEMFPlusHeaderInfo();

#if OSL_DEBUG_LEVEL > 1
        // debug code - write the stream to debug file /tmp/emf-stream.emf
        int pos = pWMF->Tell();
        pWMF->Seek(0);
        SvFileStream file( OUString( "/tmp/emf-stream.emf" ), STREAM_WRITE | STREAM_TRUNC );

        *pWMF >> file;
        file.Flush();
        file.Close();

        pWMF->Seek( pos );
#endif

    }
    bEMFPlus = true;

    sal_Size pos = pWMF->Tell();
    void *buffer = malloc( length );
    pOut->PassEMFPlus( buffer, pWMF->Read( buffer, length ) );
    free( buffer );
    pWMF->Seek( pos );

    bHaveDC = false;

    OSL_ASSERT(length >= 4);
    // reduce by 32bit length itself, skip in SeekRel if
    // impossibly unavailable
    sal_uInt32 nRemainder = length >= 4 ? length-4 : length;

    const size_t nRequiredHeaderSize = 12;
    while (nRemainder >= nRequiredHeaderSize)
    {
        sal_uInt16 type(0), flags(0);
        sal_uInt32 size(0), dataSize(0);

        *pWMF >> type >> flags >> size >> dataSize;
        nRemainder -= nRequiredHeaderSize;

        SAL_INFO ("vcl.emf", "\t\tEMF+ record type: " << std::hex << type << std::dec);

        // GetDC
        if( type == 16388 ) {
            bHaveDC = true;
            SAL_INFO ("vcl.emf", "\t\tEMF+ lock DC (device context)");
        }

        // Get the length of the remaining data of this record based
        // on the alleged size
        sal_uInt32 nRemainingRecordData = size >= nRequiredHeaderSize ?
            size-nRequiredHeaderSize : 0;
        // clip to available size
        nRemainingRecordData = std::min(nRemainingRecordData, nRemainder);
        pWMF->SeekRel(nRemainingRecordData);
        nRemainder -= nRemainingRecordData;
    }
    pWMF->SeekRel(nRemainder);
}

/**
 * Reads polygons from the stream.
 * The <class T> parameter is for the type of the points (sal_uInt32 or sal_uInt16).
 * The <class Drawer> parameter is a boost binding for the method that will draw the polygon.
 * skipFirst: if the first point read is the 0th point or the 1st point in the array.
 * */
template <class T, class Drawer>
void EnhWMFReader::ReadAndDrawPolygon(Drawer drawer, const sal_Bool skipFirst)
{
    sal_uInt32 nPoints(0), nStartIndex(0);
    pWMF->SeekRel( 16 );
    *pWMF >> nPoints;
    if (skipFirst)
    {
        nPoints ++;
        nStartIndex ++;
    }

    Polygon aPolygon = ReadPolygon<T>(nStartIndex, nPoints);
    drawer(pOut, aPolygon, skipFirst, bRecordPath);
}


/**
 * Reads polygons from the stream.
 * The <class T> parameter is for the type of the points
 * nStartIndex: which is the starting index in the polygon of the first point read
 * nPoints: number of points
 * pWMF: the stream containings the polygons
 * */
template <class T>
Polygon EnhWMFReader::ReadPolygon(sal_uInt32 nStartIndex, sal_uInt32 nPoints)
{
    Polygon aPolygon(nPoints);
    for (sal_uInt16 i = nStartIndex ; i < nPoints && pWMF->good(); i++ )
    {
        T nX, nY;
        *pWMF >> nX >> nY;
        if (!pWMF->good())
            break;
        aPolygon[ i ] = Point( nX, nY );
    }

    return aPolygon;
}

/**
 * Reads a polyline from the WMF file and draws it
 * The <class T> parameter refers to the type of the points. (e.g. sal_uInt16 or sal_uInt32)
 * */
template <class T>
void EnhWMFReader::ReadAndDrawPolyLine()
{
    sal_uInt32  nPoints;
    sal_Int32   i, nPoly(0), nGesPoints(0);
    pWMF->SeekRel( 0x10 );
    // Number of Polygons:
    *pWMF >> nPoly >> nGesPoints;

    // taking the amount of points of each polygon, retrieving the total number of points
    if ( pWMF->good() &&
         ( static_cast< sal_uInt32 >(nPoly) < SAL_MAX_UINT32 / sizeof(sal_uInt16) ) &&
         ( static_cast< sal_uInt32 >( nPoly ) * sizeof(sal_uInt16) ) <= ( nEndPos - pWMF->Tell() )
       )
    {
        sal_uInt16* pnPoints = new sal_uInt16[ nPoly ];
        for ( i = 0; i < nPoly && pWMF->good(); i++ )
        {
            *pWMF >> nPoints;
            pnPoints[ i ] = (sal_uInt16)nPoints;
        }
        // Get polygon points:
        for ( i = 0; ( i < nPoly ) && pWMF->good(); i++ )
        {
            Polygon aPolygon = ReadPolygon<T>(0, pnPoints[i]);
            pOut->DrawPolyLine( aPolygon, sal_False, bRecordPath );
        }
        delete[] pnPoints;
    }
}

/**
 * Reads a poly polygon from the WMF file and draws it.
 * The <class T> parameter refers to the type of the points. (e.g. sal_uInt16 or sal_uInt32)
 * */
template <class T>
void EnhWMFReader::ReadAndDrawPolyPolygon()
{
    sal_uInt32 nPoly(0), nGesPoints(0), nReadPoints(0);
    pWMF->SeekRel( 0x10 );
    // Number of polygons
    *pWMF >> nPoly >> nGesPoints;
    if ( pWMF->good() &&
        ( nGesPoints < SAL_MAX_UINT32 / sizeof(Point) ) && //check against numeric overflowing
        ( nPoly < SAL_MAX_UINT32 / sizeof(sal_uInt16) ) &&
        ( (  nPoly * sizeof( sal_uInt16 ) ) <= ( nEndPos - pWMF->Tell() ) ))
    {
        // Get number of points in each polygon
        sal_uInt16 * pnPoints = new sal_uInt16[ nPoly ];
        for (sal_uInt32 i = 0; i < nPoly && pWMF->good(); ++i)
        {
            sal_uInt32 nPoints(0);
            *pWMF >> nPoints;
            pnPoints[ i ] = (sal_uInt16)nPoints;
        }
        if ( pWMF->good() && ( nGesPoints * (sizeof(T)+sizeof(T)) ) <= ( nEndPos - pWMF->Tell() ) )
        {
            // Get polygon points
            PolyPolygon aPolyPoly(nPoly, nPoly);
            for (sal_uInt32 i = 0; i < nPoly && pWMF->good(); ++i)
            {
                const sal_uInt16 nPointCount(pnPoints[i]);
                Point* pPtAry = new Point[nPointCount];
                for (sal_uInt16 j = 0; j < nPointCount && pWMF->good(); ++j)
                {
                    T nX(0), nY(0);
                    *pWMF >> nX >> nY;
                    pPtAry[ j ] = Point( nX, nY );
                    ++nReadPoints;
                }

                aPolyPoly.Insert(Polygon(nPointCount, pPtAry));
                delete[] pPtAry;
            }

            pOut->DrawPolyPolygon( aPolyPoly, bRecordPath );
        }
        delete[] pnPoints;

        OSL_ENSURE(nReadPoints == nGesPoints, "The number Points processed from EMR_POLYPOLYGON is unequal imported number (!)");

    }
}

sal_Bool EnhWMFReader::ReadEnhWMF()
{
    sal_uInt32  nStretchBltMode = 0;
    sal_uInt32  nRecType(0), nRecSize(0), nNextPos(0),
                nW(0), nH(0), nColor(0), nIndex(0),
                nDat32(0), nNom1(0), nDen1(0), nNom2(0), nDen2(0);
    sal_Int32   nX32(0), nY32(0), nx32(0), ny32(0);

    sal_Bool    bFlag(sal_False), bStatus = ReadHeader();
    sal_Bool    bHaveDC = false;

    static bool bEnableEMFPlus = ( getenv( "EMF_PLUS_DISABLE" ) == NULL );

    while( bStatus && nRecordCount-- && pWMF->good())
    {
        *pWMF >> nRecType >> nRecSize;

        if ( !pWMF->good() || ( nRecSize < 8 ) || ( nRecSize & 3 ) )     // Parameters are always divisible by 4
        {
            bStatus = sal_False;
            break;
        }

        nNextPos = pWMF->Tell() + ( nRecSize - 8 );

        if ( !pWMF->good() || nNextPos > nEndPos )
        {
            bStatus = sal_False;
            break;
        }

        if(  !aBmpSaveList.empty()
          && ( nRecType != EMR_STRETCHBLT )
          && ( nRecType != EMR_STRETCHDIBITS )
          ) {
            pOut->ResolveBitmapActions( aBmpSaveList );
        }

        bFlag = sal_False;

        SAL_INFO ("vcl.emf", "0x" << std::hex << (nNextPos - nRecSize) <<  "-0x" << nNextPos << " record type: " << std::dec << nRecType << " size: " <<  nRecSize << std::dec);

        if( bEnableEMFPlus && nRecType == EMR_GDICOMMENT ) {
            sal_uInt32 length;

            *pWMF >> length;

            SAL_INFO("vcl.emf", "\tGDI comment");
            SAL_INFO("vcl.emf", "\t\tlength: " << length);

            if( pWMF->good() && length >= 4 ) {
                sal_uInt32 id;

                *pWMF >> id;

                SAL_INFO ("vcl.emf", "\t\tbegin " << (char)(id & 0xff) << (char)((id & 0xff00) >> 8) << (char)((id & 0xff0000) >> 16) << (char)((id & 0xff000000) >> 24) << " id: 0x" << std::hex << id << std::dec);

                // EMF+ comment (FIXME: BE?)
                if( id == 0x2B464D45 && nRecSize >= 12 )
                    ReadEMFPlusComment( length, bHaveDC );
                // GDIC comment, doesn't do anything useful yet
                else if( id == 0x43494447 && nRecSize >= 12 ) {
                    // TODO: ReadGDIComment()
                } else {
                    SAL_INFO ("vcl.emf", "\t\tunknown id: 0x" << std::hex << id << std::dec);
                }
            }
        }
        else if( !bEMFPlus || bHaveDC || nRecType == EMR_EOF )
        {
            switch( nRecType )
            {
                case EMR_POLYBEZIERTO :
                    ReadAndDrawPolygon<sal_Int32>(boost::bind(&WinMtfOutput::DrawPolyBezier, _1, _2, _3, _4), sal_True);
                break;
                case EMR_POLYBEZIER :
                    ReadAndDrawPolygon<sal_Int32>(boost::bind(&WinMtfOutput::DrawPolyBezier, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYGON :
                    ReadAndDrawPolygon<sal_Int32>(boost::bind(&WinMtfOutput::DrawPolygon, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYLINETO :
                    ReadAndDrawPolygon<sal_Int32>(boost::bind(&WinMtfOutput::DrawPolyLine, _1, _2, _3, _4), sal_True);
                break;
                case EMR_POLYLINE :
                    ReadAndDrawPolygon<sal_Int32>(boost::bind(&WinMtfOutput::DrawPolyLine, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYPOLYLINE :
                    ReadAndDrawPolyLine<sal_Int32>();
                break;

                case EMR_POLYPOLYGON :
                    ReadAndDrawPolyPolygon<sal_Int32>();
                break;

                case EMR_SETWINDOWEXTEX :
                {                                                       // #75383#
                    *pWMF >> nW >> nH;
                    pOut->SetWinExt( Size( nW, nH ), true);
                }
                break;

                case EMR_SETWINDOWORGEX :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->SetWinOrg( Point( nX32, nY32 ), true);
                }
                break;

                case EMR_SCALEWINDOWEXTEX :
                {
                    *pWMF >> nNom1 >> nDen1 >> nNom2 >> nDen2;
                    pOut->ScaleWinExt( (double)nNom1 / nDen1, (double)nNom2 / nDen2 );
                }
                break;

                case EMR_SETVIEWPORTORGEX :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->SetDevOrg( Point( nX32, nY32 ) );
                }
                break;

                case EMR_SCALEVIEWPORTEXTEX :
                {
                    *pWMF >> nNom1 >> nDen1 >> nNom2 >> nDen2;
                    pOut->ScaleDevExt( (double)nNom1 / nDen1, (double)nNom2 / nDen2 );
                }
                break;

                case EMR_SETVIEWPORTEXTEX :
                {
                    *pWMF >> nW >> nH;
                    pOut->SetDevExt( Size( nW, nH ) );
                }
                break;

                case EMR_EOF :
                    nRecordCount = 0;           // #76846#
                break;

                case EMR_SETPIXELV :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->DrawPixel( Point( nX32, nY32 ), ReadColor() );
                }
                break;

                case EMR_SETMAPMODE :
                {
                    sal_uInt32 nMapMode;
                    *pWMF >> nMapMode;
                    pOut->SetMapMode( nMapMode );
                }
                break;

                case EMR_SETBKMODE :
                {
                    *pWMF >> nDat32;
                    pOut->SetBkMode( nDat32 );
                }
                break;

                case EMR_SETPOLYFILLMODE :
                break;

                case EMR_SETROP2 :
                {
                    *pWMF >> nDat32;
                    pOut->SetRasterOp( nDat32 );
                }
                break;

                case EMR_SETSTRETCHBLTMODE :
                {
                    *pWMF >> nStretchBltMode;
                }
                break;

                case EMR_SETTEXTALIGN :
                {
                    *pWMF >> nDat32;
                    pOut->SetTextAlign( nDat32 );
                }
                break;

                case EMR_SETTEXTCOLOR :
                {
                    pOut->SetTextColor( ReadColor() );
                }
                break;

                case EMR_SETBKCOLOR :
                {
                    pOut->SetBkColor( ReadColor() );
                }
                break;

                case EMR_OFFSETCLIPRGN :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->MoveClipRegion( Size( nX32, nY32 ) );
                }
                break;

                case EMR_MOVETOEX :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->MoveTo( Point( nX32, nY32 ), bRecordPath );
                }
                break;

                case EMR_INTERSECTCLIPRECT :
                {
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                    pOut->IntersectClipRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                }
                break;

                case EMR_SAVEDC :
                {
                    pOut->Push();
                }
                break;

                case EMR_RESTOREDC :
                {
                    pOut->Pop();
                }
                break;

                case EMR_SETWORLDTRANSFORM :
                {
                    XForm aTempXForm;
                    *pWMF >> aTempXForm;
                    pOut->SetWorldTransform( aTempXForm );
                }
                break;

                case EMR_MODIFYWORLDTRANSFORM :
                {
                    sal_uInt32  nMode;
                    XForm   aTempXForm;
                    *pWMF >> aTempXForm >> nMode;
                    pOut->ModifyWorldTransform( aTempXForm, nMode );
                }
                break;

                case EMR_SELECTOBJECT :
                {
                    *pWMF >> nIndex;
                    pOut->SelectObject( nIndex );
                }
                break;

                case EMR_CREATEPEN :
                {
                    *pWMF >> nIndex;
                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    {

                        LineInfo    aLineInfo;
                        sal_uInt32      nStyle;
                        Size        aSize;
                        // #fdo39428 Remove SvStream operator>>(long&)
                        sal_Int32 nTmpW(0), nTmpH(0);

                        *pWMF >> nStyle >> nTmpW >> nTmpH;
                        aSize.Width() = nTmpW;
                        aSize.Height() = nTmpH;

                        if ( aSize.Width() )
                            aLineInfo.SetWidth( aSize.Width() );

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
                        pOut->CreateObject( nIndex, GDI_PEN, new WinMtfLineStyle( ReadColor(), aLineInfo, bTransparent ) );
                    }
                }
                break;

                case EMR_EXTCREATEPEN :
                {
                    sal_Int32   elpHatch;
                    sal_uInt32  offBmi, cbBmi, offBits, cbBits, nStyle, nWidth, nBrushStyle, elpNumEntries;
                    Color       aColorRef;

                    *pWMF >> nIndex;
                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    {
                        *pWMF >> offBmi >> cbBmi >> offBits >> cbBits >>  nStyle >> nWidth >> nBrushStyle;
                         aColorRef = ReadColor();
                         *pWMF >> elpHatch >> elpNumEntries;

                        LineInfo    aLineInfo;
                        if ( nWidth )
                            aLineInfo.SetWidth( nWidth );

                        sal_Bool bTransparent = sal_False;
                        sal_uInt16 nDashCount = 0;
                        sal_uInt16 nDotCount = 0;

                        switch( nStyle & PS_STYLE_MASK )
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
                        pOut->CreateObject( nIndex, GDI_PEN, new WinMtfLineStyle( aColorRef, aLineInfo, bTransparent ) );
                    }
                }
                break;

                case EMR_CREATEBRUSHINDIRECT :
                {
                    sal_uInt32  nStyle;
                    *pWMF >> nIndex;
                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    {
                        *pWMF >> nStyle;
                        pOut->CreateObject( nIndex, GDI_BRUSH, new WinMtfFillStyle( ReadColor(), ( nStyle == BS_HOLLOW ) ? sal_True : sal_False ) );
                    }
                }
                break;

                case EMR_DELETEOBJECT :
                {
                    *pWMF >> nIndex;
                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                        pOut->DeleteObject( nIndex );
                }
                break;

                case EMR_ELLIPSE :
                {
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                    pOut->DrawEllipse( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                }
                break;

                case EMR_RECTANGLE :
                {
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32;
                    pOut->DrawRect( ReadRectangle( nX32, nY32, nx32, ny32 ) );
                }
                break;

                case EMR_ROUNDRECT :
                {
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nW >> nH;
                    Size aSize( Size( nW, nH ) );
                    pOut->DrawRoundRect( ReadRectangle( nX32, nY32, nx32, ny32 ), aSize );
                }
                break;

                case EMR_ARC :
                {
                    sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                    pOut->DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                }
                break;

                case EMR_CHORD :
                {
                    sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                    pOut->DrawChord( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                }
                break;

                case EMR_PIE :
                {
                    sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                    const Rectangle aRect( ReadRectangle( nX32, nY32, nx32, ny32 ));

                    // #i73608# OutputDevice deviates from WMF
                    // semantics. start==end means full ellipse here.
                    if( nStartX == nEndX && nStartY == nEndY )
                        pOut->DrawEllipse( aRect );
                    else
                        pOut->DrawPie( aRect, Point( nStartX, nStartY ), Point( nEndX, nEndY ) );
                }
                break;

                case EMR_LINETO :
                {
                    *pWMF >> nX32 >> nY32;
                    pOut->LineTo( Point( nX32, nY32 ), bRecordPath );
                }
                break;

                case EMR_ARCTO :
                {
                    sal_uInt32 nStartX, nStartY, nEndX, nEndY;
                    *pWMF >> nX32 >> nY32 >> nx32 >> ny32 >> nStartX >> nStartY >> nEndX >> nEndY;
                    pOut->DrawArc( ReadRectangle( nX32, nY32, nx32, ny32 ), Point( nStartX, nStartY ), Point( nEndX, nEndY ), sal_True );
                }
                break;

                case EMR_BEGINPATH :
                {
                    pOut->ClearPath();
                    bRecordPath = sal_True;
                }
                break;

                case EMR_ABORTPATH :
                    pOut->ClearPath();
                case EMR_ENDPATH :
                    bRecordPath = sal_False;
                break;

                case EMR_CLOSEFIGURE :
                    pOut->ClosePath();
                break;

                case EMR_FILLPATH :
                    pOut->StrokeAndFillPath( sal_False, sal_True );
                break;

                case EMR_STROKEANDFILLPATH :
                    pOut->StrokeAndFillPath( sal_True, sal_True );
                break;

                case EMR_STROKEPATH :
                    pOut->StrokeAndFillPath( sal_True, sal_False );
                break;

                case EMR_SELECTCLIPPATH :
                {
                    sal_Int32 nClippingMode;
                    *pWMF >> nClippingMode;
                    pOut->SetClipPath( pOut->GetPathObj(), nClippingMode, sal_True );
                }
                break;

                case EMR_EXTSELECTCLIPRGN :
                {
                    sal_Int32 iMode, cbRgnData;
                    *pWMF >> cbRgnData
                          >> iMode;

                    PolyPolygon aPolyPoly;
                    if ( cbRgnData )
                        ImplReadRegion( aPolyPoly, *pWMF, nRecSize );
                    pOut->SetClipPath( aPolyPoly, iMode, sal_True );
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

                    sal_uInt32   nStart = pWMF->Tell() - 8;
                    pWMF->SeekRel( 0x10 );

                    *pWMF >> xDest >> yDest >> cxDest >> cyDest >> aFunc >> xSrc >> ySrc
                            >> xformSrc >> BkColorSrc >> iUsageSrc >> offBmiSrc >> cbBmiSrc
                                >> offBitsSrc >> cbBitsSrc >>cxSrc>>cySrc ;

                    sal_uInt32  dwRop = SRCAND|SRCINVERT;

                    Bitmap      aBitmap;
                    Rectangle   aRect( Point( xDest, yDest ), Size( cxDest+1, cyDest+1 ) );

                    if ( (cbBitsSrc > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc) )
                        bStatus = sal_False;
                    else
                    {
                        sal_uInt32 nSize = cbBmiSrc + cbBitsSrc + 14;
                        if ( nSize <= ( nEndPos - nStartPos ) )
                        {
                            char* pBuf = new char[ nSize ];
                            SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                            aTmp.ObjectOwnsMemory( sal_True );
                            aTmp << (sal_uInt8)'B'
                                 << (sal_uInt8)'M'
                                 << (sal_uInt32)cbBitsSrc
                                 << (sal_uInt16)0
                                 << (sal_uInt16)0
                                 << (sal_uInt32)cbBmiSrc + 14;
                            pWMF->Seek( nStart + offBmiSrc );
                            pWMF->Read( pBuf + 14, cbBmiSrc );
                            pWMF->Seek( nStart + offBitsSrc );
                            pWMF->Read( pBuf + 14 + cbBmiSrc, cbBitsSrc );
                            aTmp.Seek( 0 );
                            ReadDIB(aBitmap, aTmp, true);

                            // test if it is sensible to crop
                            if ( ( cxSrc > 0 ) && ( cySrc > 0 ) &&
                                ( xSrc >= 0 ) && ( ySrc >= 0 ) &&
                                    ( xSrc + cxSrc <= aBitmap.GetSizePixel().Width() ) &&
                                        ( ySrc + cySrc <= aBitmap.GetSizePixel().Height() ) )
                            {
                                Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );
                                aBitmap.Crop( aCropRect );
                            }
                            aBmpSaveList.push_back( new BSaveStruct( aBitmap, aRect, dwRop, pOut->GetFillStyle () ) );
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

                    sal_uInt32  nStart = pWMF->Tell() - 8;

                    pWMF->SeekRel( 0x10 );
                    *pWMF >> xDest >> yDest >> cxDest >> cyDest >> dwRop >> xSrc >> ySrc
                            >> xformSrc >> nColor >> iUsageSrc >> offBmiSrc >> cbBmiSrc
                                >> offBitsSrc >> cbBitsSrc;

                    if ( nRecType == EMR_STRETCHBLT )
                        *pWMF >> cxSrc >> cySrc;
                    else
                        cxSrc = cySrc = 0;

                    Bitmap      aBitmap;
                    Rectangle   aRect( Point( xDest, yDest ), Size( cxDest, cyDest ) );

                    cxDest = abs( (int)cxDest );        // sj: i37894, size can be negative
                    cyDest = abs( (int)cyDest );        // and also 122889

                    if ( (cbBitsSrc > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc) )
                        bStatus = sal_False;
                    else
                    {
                        sal_uInt32 nSize = cbBmiSrc + cbBitsSrc + 14;
                        if ( nSize <= ( nEndPos - nStartPos ) )
                        {
                            char* pBuf = new char[ nSize ];
                            SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                            aTmp.ObjectOwnsMemory( sal_True );
                            aTmp << (sal_uInt8)'B'
                                 << (sal_uInt8)'M'
                                 << (sal_uInt32)cbBitsSrc
                                 << (sal_uInt16)0
                                 << (sal_uInt16)0
                                 << (sal_uInt32)cbBmiSrc + 14;
                            pWMF->Seek( nStart + offBmiSrc );
                            pWMF->Read( pBuf + 14, cbBmiSrc );
                            pWMF->Seek( nStart + offBitsSrc );
                            pWMF->Read( pBuf + 14 + cbBmiSrc, cbBitsSrc );
                            aTmp.Seek( 0 );
                            ReadDIB(aBitmap, aTmp, true);

                            // test if it is sensible to crop
                            if ( ( cxSrc > 0 ) && ( cySrc > 0 ) &&
                                ( xSrc >= 0 ) && ( ySrc >= 0 ) &&
                                    ( xSrc + cxSrc <= aBitmap.GetSizePixel().Width() ) &&
                                        ( ySrc + cySrc <= aBitmap.GetSizePixel().Height() ) )
                            {
                                Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );
                                aBitmap.Crop( aCropRect );
                            }
                            aBmpSaveList.push_back( new BSaveStruct( aBitmap, aRect, dwRop, pOut->GetFillStyle () ) );
                        }
                    }
                }
                break;

                case EMR_STRETCHDIBITS :
                {
                    sal_Int32   xDest, yDest, xSrc, ySrc, cxSrc, cySrc, cxDest, cyDest;
                    sal_uInt32  offBmiSrc, cbBmiSrc, offBitsSrc, cbBitsSrc, iUsageSrc, dwRop;
                    sal_uInt32  nStart = pWMF->Tell() - 8;

                    pWMF->SeekRel( 0x10 );
                    *pWMF >> xDest
                          >> yDest
                          >> xSrc
                          >> ySrc
                          >> cxSrc
                          >> cySrc
                          >> offBmiSrc
                          >> cbBmiSrc
                          >> offBitsSrc
                          >> cbBitsSrc
                          >> iUsageSrc
                          >> dwRop
                          >> cxDest
                          >> cyDest;

                    Bitmap      aBitmap;
                    Rectangle   aRect( Point( xDest, yDest ), Size( cxDest, cyDest ) );

                    cxDest = abs( (int)cxDest );        // sj: i37894, size can be negative
                    cyDest = abs( (int)cyDest );        // and also 122889

                    if (  ((SAL_MAX_UINT32 - 14)             < cbBitsSrc)
                       || ((SAL_MAX_UINT32 - 14) - cbBitsSrc < cbBmiSrc )
                       )
                    {
                        bStatus = sal_False;
                    }
                    else
                    {
                        sal_uInt32 nSize = cbBmiSrc + cbBitsSrc + 14;
                        if ( nSize <= ( nEndPos - nStartPos ) )
                        {
                            char* pBuf = new char[ nSize ];
                            SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                            aTmp.ObjectOwnsMemory( sal_True );
                            aTmp << (sal_uInt8)'B'
                                << (sal_uInt8)'M'
                                << (sal_uInt32)cbBitsSrc
                                << (sal_uInt16)0
                                << (sal_uInt16)0
                                << (sal_uInt32)cbBmiSrc + 14;
                            pWMF->Seek( nStart + offBmiSrc );
                            pWMF->Read( pBuf + 14, cbBmiSrc );
                            pWMF->Seek( nStart + offBitsSrc );
                            pWMF->Read( pBuf + 14 + cbBmiSrc, cbBitsSrc );
                            aTmp.Seek( 0 );
                            ReadDIB(aBitmap, aTmp, true);

                            // test if it is sensible to crop
                            if ( ( cxSrc > 0 ) && ( cySrc > 0 ) &&
                                ( xSrc >= 0 ) && ( ySrc >= 0 ) &&
                                    ( xSrc + cxSrc <= aBitmap.GetSizePixel().Width() ) &&
                                        ( ySrc + cySrc <= aBitmap.GetSizePixel().Height() ) )
                            {
                                Rectangle aCropRect( Point( xSrc, ySrc ), Size( cxSrc, cySrc ) );
                                aBitmap.Crop( aCropRect );
                            }
                        aBmpSaveList.push_back( new BSaveStruct( aBitmap, aRect, dwRop, pOut->GetFillStyle () ) );
                        }
                    }
                }
                break;

                case EMR_EXTCREATEFONTINDIRECTW :
                {
                    *pWMF >> nIndex;
                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    {
                        LOGFONTW aLogFont;
                        *pWMF >> aLogFont.lfHeight
                              >> aLogFont.lfWidth
                              >> aLogFont.lfEscapement
                              >> aLogFont.lfOrientation
                              >> aLogFont.lfWeight
                              >> aLogFont.lfItalic
                              >> aLogFont.lfUnderline
                              >> aLogFont.lfStrikeOut
                              >> aLogFont.lfCharSet
                              >> aLogFont.lfOutPrecision
                              >> aLogFont.lfClipPrecision
                              >> aLogFont.lfQuality
                              >> aLogFont.lfPitchAndFamily;

                        sal_Unicode lfFaceName[ LF_FACESIZE ];

                        for ( int i = 0; i < LF_FACESIZE; i++ )
                        {
                            sal_uInt16 nChar;
                            *pWMF >> nChar;
                            lfFaceName[ i ] = nChar;
                        }
                        aLogFont.alfFaceName = OUString( lfFaceName );

                        // #i121382# Need to apply WorldTransform to FontHeight/Width; this should be completely
                        // changed to basegfx::B2DHomMatrix instead of 'struct XForm', but not now due to time
                        // constraints and dangers
                        const XForm& rXF = pOut->GetWorldTransform();
                        const basegfx::B2DHomMatrix aWT(rXF.eM11, rXF.eM21, rXF.eDx, rXF.eM12, rXF.eM22, rXF.eDy);
                        const basegfx::B2DVector aTransVec(aWT * basegfx::B2DVector(aLogFont.lfWidth, aLogFont.lfHeight));
                        aLogFont.lfWidth = aTransVec.getX();
                        aLogFont.lfHeight = aTransVec.getY();

                        pOut->CreateObject( nIndex, GDI_FONT, new WinMtfFontStyle( aLogFont ) );
                    }
                }
                break;

                case EMR_EXTTEXTOUTA :
                    bFlag = sal_True;
                case EMR_EXTTEXTOUTW :
                {
                    sal_Int32   nLeft, nTop, nRight, nBottom, ptlReferenceX, ptlReferenceY, nGfxMode, nXScale, nYScale;
                    sal_uInt32  nCurPos, nOffString, nOptions, offDx;
                    sal_Int32   nLen;
                    sal_Int32*  pDX = NULL;

                    nCurPos = pWMF->Tell() - 8;

                    *pWMF >> nLeft >> nTop >> nRight >> nBottom >> nGfxMode >> nXScale >> nYScale
                        >> ptlReferenceX >> ptlReferenceY >> nLen >> nOffString >> nOptions;

                    pWMF->SeekRel( 0x10 );
                    *pWMF >> offDx;

                    sal_Int32 nTextLayoutMode = TEXT_LAYOUT_DEFAULT;
                    if ( nOptions & ETO_RTLREADING )
                        nTextLayoutMode = TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT;
                    pOut->SetTextLayoutMode( nTextLayoutMode );
                    DBG_ASSERT( ( nOptions & ( ETO_PDY | ETO_GLYPH_INDEX ) ) == 0, "SJ: ETO_PDY || ETO_GLYPH_INDEX in EMF" );

                    Point aPos( ptlReferenceX, ptlReferenceY );
                    if ( nLen && nLen < static_cast<sal_Int32>( SAL_MAX_UINT32 / sizeof(sal_Int32) ) )
                    {
                        if ( offDx && (( nCurPos + offDx + nLen * 4 ) <= nNextPos ) )
                        {
                            pWMF->Seek( nCurPos + offDx );
                            if ( ( nLen * sizeof(sal_uInt32) ) <= ( nEndPos - pWMF->Tell() ) )
                            {
                                pDX = new sal_Int32[ nLen ];
                                sal_Int32 i;
                                for ( i = 0; i < nLen; i++ )
                                    *pWMF >> pDX[ i ];
                            }
                        }
                        pWMF->Seek( nCurPos + nOffString );
                        OUString aText;
                        if ( bFlag )
                        {
                            if ( nLen <= static_cast<sal_Int32>( nEndPos - pWMF->Tell() ) )
                            {
                                sal_Char* pBuf = new sal_Char[ nLen ];
                                pWMF->Read( pBuf, nLen );
                                aText = OUString( pBuf, (sal_uInt16)nLen, pOut->GetCharSet() );
                                delete[] pBuf;

                                if ( aText.getLength() != nLen )
                                {
                                    sal_uInt16 i, j;
                                    sal_Int32* pOldDx = pDX;
                                    pDX = new sal_Int32[ aText.getLength() ];
                                    for ( i = 0, j = 0; i < aText.getLength(); i++ )
                                    {
                                        sal_Unicode cUniChar = aText[i];
                                        OString aCharacter(&cUniChar, 1, pOut->GetCharSet());
                                        pDX[ i ] = 0;
                                        for (sal_Int32 k = 0; ( k < aCharacter.getLength() ) && ( j < nLen ) && ( i < aText.getLength() ); ++k)
                                            pDX[ i ] += pOldDx[ j++ ];
                                    }
                                    delete[] pOldDx;
                                }
                            }
                        }
                        else
                        {
                            if ( ( nLen * sizeof(sal_Unicode) ) <= ( nEndPos - pWMF->Tell() ) )
                            {
                                sal_Unicode* pBuf = new sal_Unicode[ nLen ];
                                pWMF->Read( pBuf, nLen << 1 );
#ifdef OSL_BIGENDIAN
                                sal_Char nTmp, *pTmp = (sal_Char*)( pBuf + nLen );
                                while ( pTmp-- != (sal_Char*)pBuf )
                                {
                                    nTmp = *pTmp--;
                                    pTmp[ 1 ] = *pTmp;
                                    *pTmp = nTmp;
                                }
#endif
                                aText = OUString(pBuf, nLen);
                                delete[] pBuf;
                            }
                        }
                        pOut->DrawText( aPos, aText, pDX, bRecordPath, nGfxMode );
                    }
                    delete[] pDX;
                }
                break;

                case EMR_POLYBEZIERTO16 :
                    ReadAndDrawPolygon<sal_Int16>(boost::bind(&WinMtfOutput::DrawPolyBezier, _1, _2, _3, _4), sal_True);
                    break;
                case EMR_POLYBEZIER16 :
                    ReadAndDrawPolygon<sal_Int16>(boost::bind(&WinMtfOutput::DrawPolyBezier, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYGON16 :
                    ReadAndDrawPolygon<sal_Int16>(boost::bind(&WinMtfOutput::DrawPolygon, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYLINETO16 :
                    ReadAndDrawPolygon<sal_Int16>(boost::bind(&WinMtfOutput::DrawPolyLine, _1, _2, _3, _4), sal_True);
                    break;
                case EMR_POLYLINE16 :
                    ReadAndDrawPolygon<sal_Int16>(boost::bind(&WinMtfOutput::DrawPolyLine, _1, _2, _3, _4), sal_False);
                break;

                case EMR_POLYPOLYLINE16 :
                    ReadAndDrawPolyLine<sal_Int16>();
                    break;

                case EMR_POLYPOLYGON16 :
                    ReadAndDrawPolyPolygon<sal_Int16>();
                break;

                case EMR_FILLRGN :
                {
                    sal_uInt32 nLen;
                    PolyPolygon aPolyPoly;
                    pWMF->SeekRel( 0x10 );
                    *pWMF >> nLen >> nIndex;

                    if ( ImplReadRegion( aPolyPoly, *pWMF, nRecSize ) )
                    {
                        pOut->Push();
                        pOut->SelectObject( nIndex );
                        pOut->DrawPolyPolygon( aPolyPoly, sal_False );
                        pOut->Pop();
                    }
                }
                break;

                case EMR_CREATEDIBPATTERNBRUSHPT :
                {
                    sal_uInt32  nStart = pWMF->Tell() - 8;
                    Bitmap aBitmap;

                    *pWMF >> nIndex;

                    if ( ( nIndex & ENHMETA_STOCK_OBJECT ) == 0 )
                    {
                        sal_uInt32 usage, offBmi, cbBmi, offBits, cbBits;

                        *pWMF >> usage;
                        *pWMF >> offBmi;
                        *pWMF >> cbBmi;
                        *pWMF >> offBits;
                        *pWMF >> cbBits;

                        if ( (cbBits > (SAL_MAX_UINT32 - 14)) || ((SAL_MAX_UINT32 - 14) - cbBits < cbBmi) )
                           bStatus = sal_False;
                        else if ( offBmi )
                        {
                            sal_uInt32  nSize = cbBmi + cbBits + 14;
                            if ( nSize <= ( nEndPos - nStartPos ) )
                            {
                                char*   pBuf = new char[ nSize ];

                                SvMemoryStream aTmp( pBuf, nSize, STREAM_READ | STREAM_WRITE );
                                aTmp.ObjectOwnsMemory( sal_True );
                                aTmp << (sal_uInt8)'B'
                                     << (sal_uInt8)'M'
                                     << (sal_uInt32)cbBits
                                     << (sal_uInt16)0
                                     << (sal_uInt16)0
                                     << (sal_uInt32)cbBmi + 14;
                                pWMF->Seek( nStart + offBmi );
                                pWMF->Read( pBuf + 14, cbBmi );
                                pWMF->Seek( nStart + offBits );
                                pWMF->Read( pBuf + 14 + cbBmi, cbBits );
                                aTmp.Seek( 0 );
                                ReadDIB(aBitmap, aTmp, true);
                            }
                        }
                    }

                    pOut->CreateObject( nIndex, GDI_BRUSH, new WinMtfFillStyle( aBitmap ) );
                }
                break;

                default :                           WinMtfAssertHandler( "Unknown Meta Action" );       break;
                case EMR_MASKBLT :                  WinMtfAssertHandler( "MaskBlt" );                   break;
                case EMR_PLGBLT :                   WinMtfAssertHandler( "PlgBlt" );                    break;
                case EMR_SETDIBITSTODEVICE :        WinMtfAssertHandler( "SetDIBitsToDevice" );         break;
                case EMR_FRAMERGN :                 WinMtfAssertHandler( "FrameRgn" );                  break;
                case EMR_INVERTRGN :                WinMtfAssertHandler( "InvertRgn" );                 break;
                case EMR_PAINTRGN :                 WinMtfAssertHandler( "PaintRgn" );                  break;
                case EMR_FLATTENPATH :              WinMtfAssertHandler( "FlattenPath" );               break;
                case EMR_WIDENPATH :                WinMtfAssertHandler( "WidenPath" );                 break;
                case EMR_POLYDRAW :                 WinMtfAssertHandler( "Polydraw" );                  break;
                case EMR_SETARCDIRECTION :          WinMtfAssertHandler( "SetArcDirection" );           break;
                case EMR_SETPALETTEENTRIES :        WinMtfAssertHandler( "SetPaletteEntries" );         break;
                case EMR_RESIZEPALETTE :            WinMtfAssertHandler( "ResizePalette" );             break;
                case EMR_EXTFLOODFILL :             WinMtfAssertHandler( "ExtFloodFill" );              break;
                case EMR_ANGLEARC :                 WinMtfAssertHandler( "AngleArc" );                  break;
                case EMR_SETCOLORADJUSTMENT :       WinMtfAssertHandler( "SetColorAdjustment" );        break;
                case EMR_POLYDRAW16 :               WinMtfAssertHandler( "PolyDraw16" );                break;
                case EMR_POLYTEXTOUTA :             WinMtfAssertHandler( "PolyTextOutA" );              break;
                case EMR_POLYTEXTOUTW :             WinMtfAssertHandler( "PolyTextOutW" );              break;
                case EMR_CREATECOLORSPACE :         WinMtfAssertHandler( "CreateColorSpace" );          break;
                case EMR_SETCOLORSPACE :            WinMtfAssertHandler( "SetColorSpace" );             break;
                case EMR_DELETECOLORSPACE :         WinMtfAssertHandler( "DeleteColorSpace" );          break;
                case EMR_GLSRECORD :                WinMtfAssertHandler( "GlsRecord" );                 break;
                case EMR_GLSBOUNDEDRECORD :         WinMtfAssertHandler( "GlsBoundRecord" );            break;
                case EMR_PIXELFORMAT :              WinMtfAssertHandler( "PixelFormat" );               break;
                case EMR_DRAWESCAPE :               WinMtfAssertHandler( "DrawEscape" );                break;
                case EMR_EXTESCAPE :                WinMtfAssertHandler( "ExtEscape" );                 break;
                case EMR_STARTDOC :                 WinMtfAssertHandler( "StartDoc" );                  break;
                case EMR_SMALLTEXTOUT :             WinMtfAssertHandler( "SmallTextOut" );              break;
                case EMR_FORCEUFIMAPPING :          WinMtfAssertHandler( "ForceUFIMapping" );           break;
                case EMR_NAMEDESCAPE :              WinMtfAssertHandler( "NamedEscape" );               break;
                case EMR_COLORCORRECTPALETTE :      WinMtfAssertHandler( "ColorCorrectPalette" );       break;
                case EMR_SETICMPROFILEA :           WinMtfAssertHandler( "SetICMProfileA" );            break;
                case EMR_SETICMPROFILEW :           WinMtfAssertHandler( "SetICMProfileW" );            break;
                case EMR_TRANSPARENTBLT :           WinMtfAssertHandler( "TransparenBlt" );             break;
                case EMR_TRANSPARENTDIB :           WinMtfAssertHandler( "TransparenDib" );             break;
                case EMR_GRADIENTFILL :             WinMtfAssertHandler( "GradientFill" );              break;
                case EMR_SETLINKEDUFIS :            WinMtfAssertHandler( "SetLinkedUFIS" );             break;

                case EMR_SETMAPPERFLAGS :           WinMtfAssertHandler( "SetMapperFlags", 0 );         break;
                case EMR_SETICMMODE :               WinMtfAssertHandler( "SetICMMode", 0 );             break;
                case EMR_CREATEMONOBRUSH :          WinMtfAssertHandler( "CreateMonoBrush", 0 );        break;
                case EMR_SETBRUSHORGEX :            WinMtfAssertHandler( "SetBrushOrgEx", 0 );          break;
                case EMR_SETMETARGN :               WinMtfAssertHandler( "SetMetArgn", 0 );             break;
                case EMR_SETMITERLIMIT :            WinMtfAssertHandler( "SetMiterLimit", 0 );          break;
                case EMR_EXCLUDECLIPRECT :          WinMtfAssertHandler( "ExcludeClipRect", 0 );        break;
                case EMR_REALIZEPALETTE :           WinMtfAssertHandler( "RealizePalette", 0 );         break;
                case EMR_SELECTPALETTE :            WinMtfAssertHandler( "SelectPalette", 0 );          break;
                case EMR_CREATEPALETTE :            WinMtfAssertHandler( "CreatePalette", 0 );          break;
                case EMR_ALPHADIBBLEND :            WinMtfAssertHandler( "AlphaDibBlend", 0 );          break;
                case EMR_SETTEXTJUSTIFICATION :     WinMtfAssertHandler( "SetTextJustification", 0 );   break;

                case EMR_GDICOMMENT :
                case EMR_HEADER :               // has already been read at ReadHeader()
                break;
            }
        }
        pWMF->Seek( nNextPos );
    }
    if( !aBmpSaveList.empty() )
        pOut->ResolveBitmapActions( aBmpSaveList );

    if ( bStatus )
        pWMF->Seek(nEndPos);

    return bStatus;
};


sal_Bool EnhWMFReader::ReadHeader()
{
    sal_uInt32      nType, nSignature, nVersion;
    sal_uInt32      nHeaderSize, nPalEntries;
    sal_Int32       nLeft, nTop, nRight, nBottom;

    // Spare me the METAFILEHEADER here
    // Reading the METAHEADER - EMR_HEADER ([MS-EMF] section 2.3.4.2 EMR_HEADER Record Types)
    *pWMF >> nType >> nHeaderSize;
    if ( nType != 1 ) { // per [MS-EMF] 2.3.4.2 EMF Header Record Types, type MUST be 0x00000001
        SAL_WARN("vcl.emf", "EMF header type is not set to 0x00000001 - possibly corrupted file?");
        return sal_False;
    }

    // Start reading the EMR_HEADER Header object

    // bound size (RectL object, see [MS-WMF] section 2.2.2.19)
    Rectangle rclBounds;    // rectangle in logical units
    *pWMF >> nLeft >> nTop >> nRight >> nBottom;
    rclBounds.Left() = nLeft;
    rclBounds.Top() = nTop;
    rclBounds.Right() = nRight;
    rclBounds.Bottom() = nBottom;

    // picture frame size (RectL object)
    Rectangle rclFrame;     // rectangle in device units 1/100th mm
    *pWMF >> nLeft >> nTop >> nRight >> nBottom;
    rclFrame.Left() = nLeft;
    rclFrame.Top() = nTop;
    rclFrame.Right() = nRight;
    rclFrame.Bottom() = nBottom;

    *pWMF >> nSignature;

    // nSignature MUST be the ASCII characters "FME", see [WS-EMF] 2.2.9 Header Object
    // and 2.1.14 FormatSignature Enumeration
    if ( nSignature != 0x464d4520 ) {
        SAL_WARN("vcl.emf", "EMF\t\tSignature is not 0x464d4520 (\"FME\") - possibly corrupted file?");
        return sal_False;
    }

    *pWMF >> nVersion;  // according to [WS-EMF] 2.2.9, this SHOULD be 0x0001000, however
                        // Microsoft note that not even Windows checks this...
    if ( nVersion != 0x00010000 ) {
        SAL_WARN("vcl.emf", "EMF\t\tThis really should be 0x00010000, though not absolutely essential...");
    }

    *pWMF >> nEndPos;                                   // size of metafile
    nEndPos += nStartPos;

    sal_uInt32 nStrmPos = pWMF->Tell();                 // checking if nEndPos is valid
    pWMF->Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nActualFileSize = pWMF->Tell();

    if ( nActualFileSize < nEndPos ) {
        SAL_WARN("vcl.emf", "EMF\t\tEMF Header object records number of bytes as " << nEndPos
                            << ", however the file size is actually " << nActualFileSize
                            << " bytes. Possible file corruption?");
        nEndPos = nActualFileSize;
    }
    pWMF->Seek( nStrmPos );

    *pWMF >> nRecordCount;

    if ( !nRecordCount ) {
        SAL_WARN("vcl.emf", "EMF\t\tEMF Header object shows record counter as 0! This shouldn't "
                            "be possible... indicator of possible file corruption?");
        return sal_False;
    }

    // the number of "handles", or graphics objects used in the metafile

    sal_uInt16 nHandlesCount;
    *pWMF >> nHandlesCount;

    // the next 2 bytes are reserved, but according to [MS-EMF] section 2.2.9
    // it MUST be 0x000 and MUST be ignored... the thing is, having such a specific
    // value is actually pretty useful in checking if there is possible corruption

    sal_uInt16 nReserved;
    *pWMF >> nReserved;

    if ( nReserved != 0x0000 ) {
        SAL_WARN("vcl.emf", "EMF\t\tEMF Header object's reserved field is NOT 0x0000... possible "
                            "corruption?");
    }

    // The next 4 bytes specifies the number of characters in the metafile description.
    // The 4 bytes after that specific the offset from this record that contains the
    // metafile description... zero means no description string.
    // For now, we ignore it.

    pWMF->SeekRel( 0x8 );

    sal_Int32 nPixX, nPixY, nMillX, nMillY;
    *pWMF >> nPalEntries >> nPixX >> nPixY >> nMillX >> nMillY;

    pOut->SetrclFrame( rclFrame );
    pOut->SetrclBounds( rclBounds );
    pOut->SetRefPix( Size( nPixX, nPixY ) );
    pOut->SetRefMill( Size( nMillX, nMillY ) );

    pWMF->Seek( nStartPos + nHeaderSize );
    return sal_True;
}


Rectangle  EnhWMFReader::ReadRectangle( sal_Int32 x1, sal_Int32 y1, sal_Int32 x2, sal_Int32 y2 )
{
    Point aTL ( Point( x1, y1 ) );
    Point aBR( Point( --x2, --y2 ) );
    return Rectangle( aTL, aBR );
}

EnhWMFReader::~EnhWMFReader()
{

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
