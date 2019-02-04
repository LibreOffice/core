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

#include <stdlib.h>

#include <sal/log.hxx>
#include <vcl/bitmapaccess.hxx>
#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include "impvect.hxx"
#include <array>
#include <memory>

#define VECT_POLY_MAX 8192

#define VECT_FREE_INDEX 0
#define VECT_CONT_INDEX 1
#define VECT_DONE_INDEX 2

#define VECT_POLY_INLINE_INNER  1UL
#define VECT_POLY_INLINE_OUTER  2UL
#define VECT_POLY_OUTLINE_INNER 4UL
#define VECT_POLY_OUTLINE_OUTER 8UL

static void VECT_MAP( const std::unique_ptr<long []> & pMapIn, const std::unique_ptr<long []>& pMapOut, long nVal )
{
    pMapIn[nVal] = (nVal * 4) + 1;
    pMapOut[nVal] = pMapIn[nVal] + 5;
}
static constexpr long BACK_MAP( long _def_nVal )
{
    return ((_def_nVal + 2) >> 2) - 1;
}
static void VECT_PROGRESS( const Link<long, void>* pProgress, long _def_nVal )
{
    if(pProgress)
      pProgress->Call(_def_nVal);
}

class ImplVectMap;
class ImplChain;

namespace ImplVectorizer
{
    static ImplVectMap* ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor );
    static void     ImplCalculate( ImplVectMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce );
    static bool     ImplGetChain( ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain );
    static bool     ImplIsUp( ImplVectMap const * pMap, long nY, long nX );
    static void     ImplLimitPolyPoly( tools::PolyPolygon& rPolyPoly );
}

struct ChainMove { long nDX; long nDY; };

static const ChainMove aImplMove[ 8 ] =   {
                                        { 1, 0 },
                                        { 0, -1 },
                                        { -1, 0 },
                                        { 0, 1 },
                                        { 1, -1 },
                                        { -1, -1 },
                                        { -1, 1 },
                                        { 1, 1 }
                                    };

static const ChainMove aImplMoveInner[ 8 ] =  {
                                            { 0, 1 },
                                            { 1, 0 },
                                            { 0, -1 },
                                            { -1, 0 },
                                            { 0, 1 },
                                            { 1, 0 },
                                            { 0, -1 },
                                            { -1, 0 }
                                        };

static const ChainMove aImplMoveOuter[ 8 ] =  {
                                            { 0, -1 },
                                            { -1, 0 },
                                            { 0, 1 },
                                            { 1, 0 },
                                            { -1, 0 },
                                            { 0, 1 },
                                            { 1, 0 },
                                            { 0, -1 }
                                        };

struct ImplColorSet
{
    BitmapColor maColor;
    sal_uInt16      mnIndex = 0;
    bool        mbSet = false;
};

static int ImplColorSetCmpFnc( const ImplColorSet& lhs, const ImplColorSet& rhs)
{
    int             nRet;

    if( lhs.mbSet && rhs.mbSet )
    {
        const sal_uInt8 cLum1 = lhs.maColor.GetLuminance();
        const sal_uInt8 cLum2 = rhs.maColor.GetLuminance();
        nRet = ( cLum1 > cLum2 ) ? -1 : ( ( cLum1 == cLum2 ) ? 0 : 1 );
    }
    else if( lhs.mbSet )
        nRet = -1;
    else if( rhs.mbSet )
        nRet = 1;
    else
        nRet = 0;

    return nRet;
}

class ImplPointArray
{
    std::unique_ptr<Point[]> mpArray;
    sal_uLong mnSize;
    sal_uLong mnRealSize;

public:

    ImplPointArray();

    void ImplSetSize( sal_uLong nSize );
    sal_uLong ImplGetRealSize() const { return mnRealSize; }
    void ImplSetRealSize( sal_uLong nRealSize ) { mnRealSize = nRealSize; }
    void ImplCreatePoly( tools::Polygon& rPoly ) const;

    inline Point& operator[]( sal_uLong nPos );
    inline const Point& operator[]( sal_uLong nPos ) const;

};

ImplPointArray::ImplPointArray() :
    mnSize      ( 0 ),
    mnRealSize  ( 0 )

{
}

void ImplPointArray::ImplSetSize( sal_uLong nSize )
{
    const sal_uLong nTotal = nSize * sizeof( Point );

    mnSize = nSize;
    mnRealSize = 0;

    mpArray = std::make_unique<Point[]>( nTotal );
}

inline Point& ImplPointArray::operator[]( sal_uLong nPos )
{
    SAL_WARN_IF( nPos >= mnSize, "vcl", "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

inline const Point& ImplPointArray::operator[]( sal_uLong nPos ) const
{
    SAL_WARN_IF( nPos >= mnSize, "vcl", "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

void ImplPointArray::ImplCreatePoly( tools::Polygon& rPoly ) const
{
    rPoly = tools::Polygon( sal::static_int_cast<sal_uInt16>(mnRealSize), mpArray.get() );
}

class ImplVectMap
{
private:

    Scanline const        mpBuf;
    Scanline*       mpScan;
    long const            mnWidth;
    long const            mnHeight;

public:

                    ImplVectMap( long nWidth, long nHeight );
                    ~ImplVectMap();

    long     Width() const { return mnWidth; }
    long     Height() const { return mnHeight; }

    inline void     Set( long nY, long nX, sal_uInt8 cVal );
    inline sal_uInt8        Get( long nY, long nX ) const;

    inline bool     IsFree( long nY, long nX ) const;
    inline bool     IsCont( long nY, long nX ) const;
    inline bool     IsDone( long nY, long nX ) const;

};

ImplVectMap::ImplVectMap( long nWidth, long nHeight ) :
    mpBuf ( static_cast<Scanline>(rtl_allocateZeroMemory(nWidth * nHeight)) ),
    mpScan ( static_cast<Scanline*>(std::malloc(nHeight * sizeof(Scanline))) ),
    mnWidth ( nWidth ),
    mnHeight( nHeight )
{
    const long  nWidthAl = ( nWidth >> 2 ) + 1;
    Scanline    pTmp = mpBuf;

    for( long nY = 0; nY < nHeight; pTmp += nWidthAl )
        mpScan[ nY++ ] = pTmp;
}

ImplVectMap::~ImplVectMap()
{
    std::free( mpBuf );
    std::free( mpScan );
}

inline void ImplVectMap::Set( long nY, long nX, sal_uInt8 cVal )
{
    const sal_uInt8 cShift = sal::static_int_cast<sal_uInt8>(6 - ( ( nX & 3 ) << 1 ));
    auto & rPixel = mpScan[ nY ][ nX >> 2 ];
    rPixel = (rPixel & ~( 3 << cShift ) ) | ( cVal << cShift );
}

inline sal_uInt8    ImplVectMap::Get( long nY, long nX ) const
{
    return sal::static_int_cast<sal_uInt8>( ( ( mpScan[ nY ][ nX >> 2 ] ) >> ( 6 - ( ( nX & 3 ) << 1 ) ) ) & 3 );
}

inline bool ImplVectMap::IsFree( long nY, long nX ) const
{
    return( VECT_FREE_INDEX == Get( nY, nX ) );
}

inline bool ImplVectMap::IsCont( long nY, long nX ) const
{
    return( VECT_CONT_INDEX == Get( nY, nX ) );
}

inline bool ImplVectMap::IsDone( long nY, long nX ) const
{
    return( VECT_DONE_INDEX == Get( nY, nX ) );
}

class ImplChain
{
private:

    tools::Polygon  maPoly;
    Point           maStartPt;
    sal_uLong       mnArraySize;
    sal_uLong       mnCount;
    std::unique_ptr<sal_uInt8[]>
                    mpCodes;

    void            ImplGetSpace();

    void            ImplPostProcess( const ImplPointArray& rArr );

    ImplChain(const ImplChain&) = delete;
    ImplChain& operator=(const ImplChain&) = delete;

public:

                    ImplChain();

    void            ImplBeginAdd( const Point& rStartPt );
    inline void     ImplAdd( sal_uInt8 nCode );
    void            ImplEndAdd( sal_uLong nTypeFlag );

    const tools::Polygon& ImplGetPoly() const { return maPoly; }
};

ImplChain::ImplChain() :
    mnArraySize ( 1024 ),
    mnCount     ( 0 ),
    mpCodes     ( new sal_uInt8[mnArraySize] )
{
}

void ImplChain::ImplGetSpace()
{
    const sal_uLong nOldArraySize = mnArraySize;
    sal_uInt8*      pNewCodes;

    mnArraySize = mnArraySize << 1;
    pNewCodes = new sal_uInt8[ mnArraySize ];
    memcpy( pNewCodes, mpCodes.get(), nOldArraySize );
    mpCodes.reset( pNewCodes );
}

void ImplChain::ImplBeginAdd( const Point& rStartPt )
{
    maPoly = tools::Polygon();
    maStartPt = rStartPt;
    mnCount = 0;
}

inline void ImplChain::ImplAdd( sal_uInt8 nCode )
{
    if( mnCount == mnArraySize )
        ImplGetSpace();

    mpCodes[ mnCount++ ] = nCode;
}

void ImplChain::ImplEndAdd( sal_uLong nFlag )
{
    if( mnCount )
    {
        ImplPointArray aArr;

        if( nFlag & VECT_POLY_INLINE_INNER )
        {
            long nFirstX, nFirstY;
            long nLastX, nLastY;

            nFirstX = nLastX = maStartPt.X();
            nFirstY = nLastY = maStartPt.Y();
            aArr.ImplSetSize( mnCount << 1 );

            sal_uInt16 nPolyPos;
            sal_uLong i;
            for( i = 0, nPolyPos = 0; i < ( mnCount - 1 ); i++ )
            {
                const sal_uInt8             cMove = mpCodes[ i ];
                const sal_uInt8             cNextMove = mpCodes[ i + 1 ];
                const ChainMove&        rMove = aImplMove[ cMove ];
                const ChainMove&        rMoveInner = aImplMoveInner[ cMove ];
//              Point&                  rPt = aArr[ nPolyPos ];
                bool                    bDone = true;

                nLastX += rMove.nDX;
                nLastY += rMove.nDY;

                if( cMove < 4 )
                {
                    if( ( cMove == 0 && cNextMove == 3 ) ||
                        ( cMove == 3 && cNextMove == 2 ) ||
                        ( cMove == 2 && cNextMove == 1 ) ||
                        ( cMove == 1 && cNextMove == 0 ) )
                    {
                    }
                    else if( cMove == 2 && cNextMove == 3 )
                    {
                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );
                    }
                    else if( cMove == 3 && cNextMove == 0 )
                    {
                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );
                    }
                    else if( cMove == 0 && cNextMove == 1 )
                    {
                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );
                    }
                    else if( cMove == 1 && cNextMove == 2 )
                    {
                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );
                    }
                    else
                        bDone = false;
                }
                else if( cMove == 7 && cNextMove == 0 )
                {
                    aArr[ nPolyPos ].setX( nLastX - 1 );
                    aArr[ nPolyPos++ ].setY( nLastY );

                    aArr[ nPolyPos ].setX( nLastX );
                    aArr[ nPolyPos++ ].setY( nLastY + 1 );
                }
                else if( cMove == 4 && cNextMove == 1 )
                {
                    aArr[ nPolyPos ].setX( nLastX );
                    aArr[ nPolyPos++ ].setY( nLastY + 1 );

                    aArr[ nPolyPos ].setX( nLastX + 1 );
                    aArr[ nPolyPos++ ].setY( nLastY );
                }
                else
                    bDone = false;

                if( !bDone )
                {
                    aArr[ nPolyPos ].setX( nLastX + rMoveInner.nDX );
                    aArr[ nPolyPos++ ].setY( nLastY + rMoveInner.nDY );
                }
            }

            aArr[ nPolyPos ].setX( nFirstX + 1 );
            aArr[ nPolyPos++ ].setY( nFirstY + 1 );
            aArr.ImplSetRealSize( nPolyPos );
        }
        else if( nFlag & VECT_POLY_INLINE_OUTER )
        {
            long nFirstX, nFirstY;
            long nLastX, nLastY;

            nFirstX = nLastX = maStartPt.X();
            nFirstY = nLastY = maStartPt.Y();
            aArr.ImplSetSize( mnCount << 1 );

            sal_uInt16 nPolyPos;
            sal_uLong i;
            for( i = 0, nPolyPos = 0; i < ( mnCount - 1 ); i++ )
            {
                const sal_uInt8             cMove = mpCodes[ i ];
                const sal_uInt8             cNextMove = mpCodes[ i + 1 ];
                const ChainMove&        rMove = aImplMove[ cMove ];
                const ChainMove&        rMoveOuter = aImplMoveOuter[ cMove ];
//              Point&                  rPt = aArr[ nPolyPos ];
                bool                    bDone = true;

                nLastX += rMove.nDX;
                nLastY += rMove.nDY;

                if( cMove < 4 )
                {
                    if( ( cMove == 0 && cNextMove == 1 ) ||
                        ( cMove == 1 && cNextMove == 2 ) ||
                        ( cMove == 2 && cNextMove == 3 ) ||
                        ( cMove == 3 && cNextMove == 0 ) )
                    {
                    }
                    else if( cMove == 0 && cNextMove == 3 )
                    {
                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );
                    }
                    else if( cMove == 3 && cNextMove == 2 )
                    {
                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );

                        aArr[ nPolyPos ].setX( nLastX + 1 );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );
                    }
                    else if( cMove == 2 && cNextMove == 1 )
                    {
                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY + 1 );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );
                    }
                    else if( cMove == 1 && cNextMove == 0 )
                    {
                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY );

                        aArr[ nPolyPos ].setX( nLastX - 1 );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );

                        aArr[ nPolyPos ].setX( nLastX );
                        aArr[ nPolyPos++ ].setY( nLastY - 1 );
                    }
                    else
                        bDone = false;
                }
                else if( cMove == 7 && cNextMove == 3 )
                {
                    aArr[ nPolyPos ].setX( nLastX );
                    aArr[ nPolyPos++ ].setY( nLastY - 1 );

                    aArr[ nPolyPos ].setX( nLastX + 1 );
                    aArr[ nPolyPos++ ].setY( nLastY );
                }
                else if( cMove == 6 && cNextMove == 2 )
                {
                    aArr[ nPolyPos ].setX( nLastX + 1 );
                    aArr[ nPolyPos++ ].setY( nLastY );

                    aArr[ nPolyPos ].setX( nLastX );
                    aArr[ nPolyPos++ ].setY( nLastY + 1 );
                }
                else
                    bDone = false;

                if( !bDone )
                {
                    aArr[ nPolyPos ].setX( nLastX + rMoveOuter.nDX );
                    aArr[ nPolyPos++ ].setY( nLastY + rMoveOuter.nDY );
                }
            }

            aArr[ nPolyPos ].setX( nFirstX - 1 );
            aArr[ nPolyPos++ ].setY( nFirstY - 1 );
            aArr.ImplSetRealSize( nPolyPos );
        }
        else
        {
            long nLastX = maStartPt.X(), nLastY = maStartPt.Y();

            aArr.ImplSetSize( mnCount + 1 );
            aArr[ 0 ] = Point( nLastX, nLastY );

            for( sal_uLong i = 0; i < mnCount; )
            {
                const ChainMove& rMove = aImplMove[ mpCodes[ i ] ];
                aArr[ ++i ] = Point( nLastX += rMove.nDX, nLastY += rMove.nDY );
            }

            aArr.ImplSetRealSize( mnCount + 1 );
        }

        ImplPostProcess( aArr );
    }
    else
        maPoly.SetSize( 0 );
}

void ImplChain::ImplPostProcess( const ImplPointArray& rArr )
{
    ImplPointArray  aNewArr1;
    ImplPointArray  aNewArr2;
    Point*          pLast;
    Point*          pLeast;
    sal_uLong           nNewPos;
    sal_uLong           nCount = rArr.ImplGetRealSize();
    sal_uLong           n;

    // pass 1
    aNewArr1.ImplSetSize( nCount );
    pLast = &( aNewArr1[ 0 ] );
    pLast->setX( BACK_MAP( rArr[ 0 ].X() ) );
    pLast->setY( BACK_MAP( rArr[ 0 ].Y() ) );

    for( n = nNewPos = 1; n < nCount; )
    {
        const Point& rPt = rArr[ n++ ];
        const long   nX = BACK_MAP( rPt.X() );
        const long   nY = BACK_MAP( rPt.Y() );

        if( nX != pLast->X() || nY != pLast->Y() )
        {
            pLast = pLeast = &( aNewArr1[ nNewPos++ ] );
            pLeast->setX( nX );
            pLeast->setY( nY );
        }
    }

    nCount = nNewPos;
    aNewArr1.ImplSetRealSize( nCount );

    // pass 2
    aNewArr2.ImplSetSize( nCount );
    pLast = &( aNewArr2[ 0 ] );
    *pLast = aNewArr1[ 0 ];

    for( n = nNewPos = 1; n < nCount; )
    {
        pLeast = &( aNewArr1[ n++ ] );

        if( pLeast->X() == pLast->X() )
        {
            while( n < nCount && aNewArr1[ n ].X() == pLast->X() )
                pLeast = &( aNewArr1[ n++ ] );
        }
        else if( pLeast->Y() == pLast->Y() )
        {
            while( n < nCount && aNewArr1[ n ].Y() == pLast->Y() )
                pLeast = &( aNewArr1[ n++ ] );
        }

        pLast = pLeast;
        aNewArr2[ nNewPos++ ] = *pLast;
    }

    aNewArr2.ImplSetRealSize( nNewPos );
    aNewArr2.ImplCreatePoly( maPoly );
}

namespace ImplVectorizer {

bool ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                    sal_uInt8 cReduce, const Link<long,void>* pProgress )
{
    bool bRet = false;

    VECT_PROGRESS( pProgress, 0 );

    std::unique_ptr<Bitmap> xBmp(new Bitmap( rColorBmp ));
    Bitmap::ScopedReadAccess pRAcc(*xBmp);

    if( pRAcc )
    {
        tools::PolyPolygon         aPolyPoly;
        double              fPercent = 0.0;
        double              fPercentStep_2 = 0.0;
        const long          nWidth = pRAcc->Width();
        const long          nHeight = pRAcc->Height();
        const sal_uInt16        nColorCount = pRAcc->GetPaletteEntryCount();
        sal_uInt16              n;
        std::array<ImplColorSet, 256> aColorSet;

        rMtf.Clear();

        // get used palette colors and sort them from light to dark colors
        for( n = 0; n < nColorCount; n++ )
        {
            aColorSet[ n ].mnIndex = n;
            aColorSet[ n ].maColor = pRAcc->GetPaletteColor( n );
        }

        for( long nY = 0; nY < nHeight; nY++ )
        {
            Scanline pScanlineRead = pRAcc->GetScanline( nY );
            for( long nX = 0; nX < nWidth; nX++ )
                aColorSet[ pRAcc->GetIndexFromData( pScanlineRead, nX ) ].mbSet = true;
        }

        std::sort( aColorSet.begin(), aColorSet.end(), ImplColorSetCmpFnc );

        for( n = 0; n < 256; n++ )
            if( !aColorSet[ n ].mbSet )
                break;

        if( n )
            fPercentStep_2 = 45.0 / n;

        fPercent += 10.0;
        VECT_PROGRESS( pProgress, FRound( fPercent ) );

        for( sal_uInt16 i = 0; i < n; i++ )
        {
            const BitmapColor   aBmpCol( pRAcc->GetPaletteColor( aColorSet[ i ].mnIndex ) );
            const Color         aFindColor( aBmpCol.GetRed(), aBmpCol.GetGreen(), aBmpCol.GetBlue() );
            std::unique_ptr<ImplVectMap> xMap(ImplExpand( pRAcc.get(), aFindColor ));

            fPercent += fPercentStep_2;
            VECT_PROGRESS( pProgress, FRound( fPercent ) );

            if( xMap )
            {
                aPolyPoly.Clear();
                ImplCalculate( xMap.get(), aPolyPoly, cReduce );
                xMap.reset();

                if( aPolyPoly.Count() )
                {
                    ImplLimitPolyPoly( aPolyPoly );

                    aPolyPoly.Optimize( PolyOptimizeFlags::EDGES );

                    if( aPolyPoly.Count() )
                    {
                        rMtf.AddAction( new MetaLineColorAction( aFindColor, true ) );
                        rMtf.AddAction( new MetaFillColorAction( aFindColor, true ) );
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                    }
                }
            }

            fPercent += fPercentStep_2;
            VECT_PROGRESS( pProgress, FRound( fPercent ) );
        }

        if( rMtf.GetActionSize() )
        {
            MapMode         aMap( MapUnit::Map100thMM );
            ScopedVclPtrInstance< VirtualDevice > aVDev;
            const Size      aLogSize1( aVDev->PixelToLogic( Size( 1, 1 ), aMap ) );

            rMtf.SetPrefMapMode( aMap );
            rMtf.SetPrefSize( Size( nWidth + 2, nHeight + 2 ) );
            rMtf.Move( 1, 1 );
            rMtf.Scale( aLogSize1.Width(), aLogSize1.Height() );
            bRet = true;
        }
    }

    pRAcc.reset();
    xBmp.reset();
    VECT_PROGRESS( pProgress, 100 );

    return bRet;
}

void ImplLimitPolyPoly( tools::PolyPolygon& rPolyPoly )
{
    if( rPolyPoly.Count() > VECT_POLY_MAX )
    {
        tools::PolyPolygon aNewPolyPoly;
        long        nReduce = 0;
        sal_uInt16      nNewCount;

        do
        {
            aNewPolyPoly.Clear();
            nReduce++;

            for( sal_uInt16 i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
            {
                const tools::Rectangle aBound( rPolyPoly[ i ].GetBoundRect() );

                if( aBound.GetWidth() > nReduce && aBound.GetHeight() > nReduce )
                {
                    if( rPolyPoly[ i ].GetSize() )
                        aNewPolyPoly.Insert( rPolyPoly[ i ] );
                }
            }

            nNewCount = aNewPolyPoly.Count();
        }
        while( nNewCount > VECT_POLY_MAX );

        rPolyPoly = aNewPolyPoly;
    }
}

ImplVectMap* ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor )
{
    ImplVectMap* pMap = nullptr;

    if( pRAcc && pRAcc->Width() && pRAcc->Height() )
    {
        const long          nOldWidth = pRAcc->Width();
        const long          nOldHeight = pRAcc->Height();
        const long          nNewWidth = ( nOldWidth << 2 ) + 4;
        const long          nNewHeight = ( nOldHeight << 2 ) + 4;
        const BitmapColor   aTest( pRAcc->GetBestMatchingColor( rColor ) );
        std::unique_ptr<long[]> pMapIn(new long[ std::max( nOldWidth, nOldHeight ) ]);
        std::unique_ptr<long[]> pMapOut(new long[ std::max( nOldWidth, nOldHeight ) ]);
        long                nX, nY, nTmpX, nTmpY;

        pMap = new ImplVectMap( nNewWidth, nNewHeight );

        for( nX = 0; nX < nOldWidth; nX++ )
            VECT_MAP( pMapIn, pMapOut, nX );

        for( nY = 0, nTmpY = 5; nY < nOldHeight; nY++, nTmpY += 4 )
        {
            Scanline pScanlineRead = pRAcc->GetScanline( nY );
            for( nX = 0; nX < nOldWidth; )
            {
                if( pRAcc->GetPixelFromData( pScanlineRead, nX ) == aTest )
                {
                    nTmpX = pMapIn[ nX++ ];
                    nTmpY -= 3;

                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );

                    while( nX < nOldWidth && pRAcc->GetPixelFromData( pScanlineRead, nX ) == aTest )
                         nX++;

                    nTmpX = pMapOut[ nX - 1 ];
                    nTmpY -= 3;

                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );
                }
                else
                    nX++;
            }
        }

        for( nY = 0; nY < nOldHeight; nY++ )
            VECT_MAP( pMapIn, pMapOut, nY );

        for( nX = 0, nTmpX = 5; nX < nOldWidth; nX++, nTmpX += 4 )
        {
            for( nY = 0; nY < nOldHeight; )
            {
                if( pRAcc->GetPixel( nY, nX ) == aTest )
                {
                    nTmpX -= 3;
                    nTmpY = pMapIn[ nY++ ];

                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );

                    while( nY < nOldHeight && pRAcc->GetPixel( nY, nX ) == aTest )
                        nY++;

                    nTmpX -= 3;
                    nTmpY = pMapOut[ nY - 1 ];

                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );
                }
                else
                    nY++;
            }
        }
    }

    return pMap;
}

void ImplCalculate( ImplVectMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce )
{
    const long nWidth = pMap->Width(), nHeight= pMap->Height();

    for( long nY = 0; nY < nHeight; nY++ )
    {
        long    nX = 0;
        bool    bInner = true;

        while( nX < nWidth )
        {
            // skip free
            while( ( nX < nWidth ) && pMap->IsFree( nY, nX ) )
                nX++;

            if( nX == nWidth )
                break;

            if( pMap->IsCont( nY, nX ) )
            {
                // new contour
                ImplChain   aChain;
                const Point aStartPt( nX++, nY );

                // get chain code
                aChain.ImplBeginAdd( aStartPt );
                ImplGetChain( pMap, aStartPt, aChain );

                aChain.ImplEndAdd( bInner ? VECT_POLY_OUTLINE_INNER : VECT_POLY_OUTLINE_OUTER );

                const tools::Polygon& rPoly = aChain.ImplGetPoly();

                if( rPoly.GetSize() > 2 )
                {
                    if( cReduce )
                    {
                        const tools::Rectangle aBound( rPoly.GetBoundRect() );

                        if( aBound.GetWidth() > cReduce && aBound.GetHeight() > cReduce )
                            rPolyPoly.Insert( rPoly );
                    }
                    else
                        rPolyPoly.Insert( rPoly  );
                }

                // skip rest of detected contour
                while( pMap->IsCont( nY, nX ) )
                    nX++;
            }
            else
            {
                // process done segment
                const long nStartSegX = nX++;

                while( pMap->IsDone( nY, nX ) )
                    nX++;

                if( ( ( nX - nStartSegX ) == 1 ) || ( ImplIsUp( pMap, nY, nStartSegX ) != ImplIsUp( pMap, nY, nX - 1 ) ) )
                    bInner = !bInner;
            }
        }
    }
}

bool ImplGetChain(  ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain )
{
    long                nActX = rStartPt.X();
    long                nActY = rStartPt.Y();
    sal_uLong               nFound;
    sal_uLong               nLastDir = 0;
    sal_uLong               nDir;

    do
    {
        nFound = 0;

        // first try last direction
        long nTryX = nActX + aImplMove[ nLastDir ].nDX;
        long nTryY = nActY + aImplMove[ nLastDir ].nDY;

        if( pMap->IsCont( nTryY, nTryX ) )
        {
            rChain.ImplAdd( static_cast<sal_uInt8>(nLastDir) );
            nActY = nTryY;
            nActX = nTryX;
            pMap->Set( nActY, nActX, VECT_DONE_INDEX );
            nFound = 1;
        }
        else
        {
            // try other directions
            for( nDir = 0; nDir < 8; nDir++ )
            {
                // we already tried nLastDir
                if( nDir != nLastDir )
                {
                    nTryX = nActX + aImplMove[ nDir ].nDX;
                    nTryY = nActY + aImplMove[ nDir ].nDY;

                    if( pMap->IsCont( nTryY, nTryX ) )
                    {
                        rChain.ImplAdd( static_cast<sal_uInt8>(nDir) );
                        nActY = nTryY;
                        nActX = nTryX;
                        pMap->Set( nActY, nActX, VECT_DONE_INDEX );
                        nFound = 1;
                        nLastDir = nDir;
                        break;
                    }
                }
            }
        }
    }
    while( nFound );

    return true;
}

bool ImplIsUp( ImplVectMap const * pMap, long nY, long nX )
{
    if( pMap->IsDone( nY - 1, nX ) )
        return true;
    else if( pMap->IsDone( nY + 1, nX ) )
        return false;
    else if( pMap->IsDone( nY - 1, nX - 1 ) || pMap->IsDone( nY - 1, nX + 1 ) )
        return true;
    else
        return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
