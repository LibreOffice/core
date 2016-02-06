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
#include <vcl/bitmapaccess.hxx>
#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <impvect.hxx>
#include <memory>

#define VECT_POLY_MAX 8192

#define VECT_FREE_INDEX 0
#define VECT_CONT_INDEX 1
#define VECT_DONE_INDEX 2

#define VECT_POLY_INLINE_INNER  1UL
#define VECT_POLY_INLINE_OUTER  2UL
#define VECT_POLY_OUTLINE_INNER 4UL
#define VECT_POLY_OUTLINE_OUTER 8UL

#define VECT_MAP( _def_pIn, _def_pOut, _def_nVal )  _def_pOut[_def_nVal]=(_def_pIn[_def_nVal]=((_def_nVal)*4L)+1L)+5L;
#define BACK_MAP( _def_nVal )                       ((((_def_nVal)+2)>>2)-1)
#define VECT_PROGRESS( _def_pProgress, _def_nVal ) \
  if(_def_pProgress)      \
      (_def_pProgress->Call(_def_nVal));

class ImplVectMap;
class ImplChain;

namespace ImplVectorizer
{
    ImplVectMap* ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor );
    void     ImplCalculate( ImplVectMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce, BmpVectorizeFlags nFlags );
    bool     ImplGetChain( ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain );
    bool     ImplIsUp( ImplVectMap* pMap, long nY, long nX );
    void     ImplLimitPolyPoly( tools::PolyPolygon& rPolyPoly );
}

struct ChainMove { long nDX; long nDY; };

static const ChainMove aImplMove[ 8 ] =   {
                                        { 1L, 0L },
                                        { 0L, -1L },
                                        { -1L, 0L },
                                        { 0L, 1L },
                                        { 1L, -1L },
                                        { -1, -1L },
                                        { -1L, 1L },
                                        { 1L, 1L }
                                    };

static const ChainMove aImplMoveInner[ 8 ] =  {
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L },
                                            { -1L, 0L }
                                        };

static const ChainMove aImplMoveOuter[ 8 ] =  {
                                            { 0L, -1L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L }
                                        };

struct ImplColorSet
{
    BitmapColor maColor;
    sal_uInt16      mnIndex;
    bool        mbSet;
};

extern "C" int SAL_CALL ImplColorSetCmpFnc( const void* p1, const void* p2 )
{
    ImplColorSet const * pSet1 = static_cast<ImplColorSet const *>(p1);
    ImplColorSet const * pSet2 = static_cast<ImplColorSet const *>(p2);
    int             nRet;

    if( pSet1->mbSet && pSet2->mbSet )
    {
        const sal_uInt8 cLum1 = pSet1->maColor.GetLuminance();
        const sal_uInt8 cLum2 = pSet2->maColor.GetLuminance();
        nRet = ( ( cLum1 > cLum2 ) ? -1 : ( ( cLum1 == cLum2 ) ? 0 : 1 ) );
    }
    else if( pSet1->mbSet )
        nRet = -1;
    else if( pSet2->mbSet )
        nRet = 1;
    else
        nRet = 0;

    return nRet;
}

class ImplPointArray
{
    Point* mpArray;
    sal_uLong mnSize;
    sal_uLong mnRealSize;

public:

    ImplPointArray();
   ~ImplPointArray();

    void ImplSetSize( sal_uLong nSize );
    sal_uLong ImplGetRealSize() const { return mnRealSize; }
    void ImplSetRealSize( sal_uLong nRealSize ) { mnRealSize = nRealSize; }
    void ImplCreatePoly( tools::Polygon& rPoly ) const;

    inline Point& operator[]( sal_uLong nPos );
    inline const Point& operator[]( sal_uLong nPos ) const;

};

ImplPointArray::ImplPointArray() :
    mpArray     ( nullptr ),
    mnSize      ( 0UL ),
    mnRealSize  ( 0UL )

{
}

ImplPointArray::~ImplPointArray()
{
    if( mpArray )
        rtl_freeMemory( mpArray );
}

void ImplPointArray::ImplSetSize( sal_uLong nSize )
{
    const sal_uLong nTotal = nSize * sizeof( Point );

    mnSize = nSize;
    mnRealSize = 0UL;

    if( mpArray )
        rtl_freeMemory( mpArray );

    mpArray = static_cast<Point*>(rtl_allocateMemory( nTotal ));
    memset( mpArray, 0, nTotal );
}

inline Point& ImplPointArray::operator[]( sal_uLong nPos )
{
    DBG_ASSERT( nPos < mnSize, "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

inline const Point& ImplPointArray::operator[]( sal_uLong nPos ) const
{
    DBG_ASSERT( nPos < mnSize, "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

void ImplPointArray::ImplCreatePoly( tools::Polygon& rPoly ) const
{
    rPoly = tools::Polygon( sal::static_int_cast<sal_uInt16>(mnRealSize), mpArray );
}

class ImplVectMap
{
private:

    Scanline        mpBuf;
    Scanline*       mpScan;
    long            mnWidth;
    long            mnHeight;

public:

                    ImplVectMap( long nWidth, long nHeight );
                    ~ImplVectMap();

    inline long     Width() const { return mnWidth; }
    inline long     Height() const { return mnHeight; }

    inline void     Set( long nY, long nX, sal_uInt8 cVal );
    inline sal_uInt8        Get( long nY, long nX ) const;

    inline bool     IsFree( long nY, long nX ) const;
    inline bool     IsCont( long nY, long nX ) const;
    inline bool     IsDone( long nY, long nX ) const;

};

ImplVectMap::ImplVectMap( long nWidth, long nHeight ) :
    mnWidth ( nWidth ),
    mnHeight( nHeight )
{
    const long  nWidthAl = ( nWidth >> 2L ) + 1L;
    const long  nSize = nWidthAl * nHeight;
    Scanline    pTmp = mpBuf = static_cast<Scanline>(rtl_allocateMemory( nSize ));

    memset( mpBuf, 0, nSize );
    mpScan = static_cast<Scanline*>(rtl_allocateMemory( nHeight * sizeof( Scanline ) ));

    for( long nY = 0L; nY < nHeight; pTmp += nWidthAl )
        mpScan[ nY++ ] = pTmp;
}

ImplVectMap::~ImplVectMap()
{
    rtl_freeMemory( mpBuf );
    rtl_freeMemory( mpScan );
}

inline void ImplVectMap::Set( long nY, long nX, sal_uInt8 cVal )
{
    const sal_uInt8 cShift = sal::static_int_cast<sal_uInt8>(6 - ( ( nX & 3 ) << 1 ));
    ( ( mpScan[ nY ][ nX >> 2 ] ) &= ~( 3 << cShift ) ) |= ( cVal << cShift );
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

    tools::Polygon maPoly;
    Point           maStartPt;
    sal_uLong           mnArraySize;
    sal_uLong           mnCount;
    long            mnResize;
    sal_uInt8*          mpCodes;

    void            ImplGetSpace();

    void            ImplPostProcess( const ImplPointArray& rArr );

public:

                    ImplChain( sal_uLong nInitCount = 1024UL, long nResize = -1L );
                    ~ImplChain();

    void            ImplBeginAdd( const Point& rStartPt );
    inline void     ImplAdd( sal_uInt8 nCode );
    void            ImplEndAdd( sal_uLong nTypeFlag );

    const tools::Polygon& ImplGetPoly() const { return maPoly; }
};

ImplChain::ImplChain( sal_uLong nInitCount, long nResize ) :
    mnArraySize ( nInitCount ),
    mnCount     ( 0UL ),
    mnResize    ( nResize )
{
    DBG_ASSERT( nInitCount && nResize, "ImplChain::ImplChain(): invalid parameters!" );
    mpCodes = new sal_uInt8[ mnArraySize ];
}

ImplChain::~ImplChain()
{
    delete[] mpCodes;
}

void ImplChain::ImplGetSpace()
{
    const sal_uLong nOldArraySize = mnArraySize;
    sal_uInt8*      pNewCodes;

    mnArraySize = ( mnResize < 0L ) ? ( mnArraySize << 1UL ) : ( mnArraySize + (sal_uLong) mnResize );
    pNewCodes = new sal_uInt8[ mnArraySize ];
    memcpy( pNewCodes, mpCodes, nOldArraySize );
    delete[] mpCodes;
    mpCodes = pNewCodes;
}

void ImplChain::ImplBeginAdd( const Point& rStartPt )
{
    maPoly = tools::Polygon();
    maStartPt = rStartPt;
    mnCount = 0UL;
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

            sal_uInt16 i, nPolyPos;
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
                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;
                    }
                    else if( cMove == 3 && cNextMove == 0 )
                    {
                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;
                    }
                    else if( cMove == 0 && cNextMove == 1 )
                    {
                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;
                    }
                    else if( cMove == 1 && cNextMove == 2 )
                    {
                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;
                    }
                    else
                        bDone = false;
                }
                else if( cMove == 7 && cNextMove == 0 )
                {
                    aArr[ nPolyPos ].X() = nLastX - 1;
                    aArr[ nPolyPos++ ].Y() = nLastY;

                    aArr[ nPolyPos ].X() = nLastX;
                    aArr[ nPolyPos++ ].Y() = nLastY + 1;
                }
                else if( cMove == 4 && cNextMove == 1 )
                {
                    aArr[ nPolyPos ].X() = nLastX;
                    aArr[ nPolyPos++ ].Y() = nLastY + 1;

                    aArr[ nPolyPos ].X() = nLastX + 1;
                    aArr[ nPolyPos++ ].Y() = nLastY;
                }
                else
                    bDone = false;

                if( !bDone )
                {
                    aArr[ nPolyPos ].X() = nLastX + rMoveInner.nDX;
                    aArr[ nPolyPos++ ].Y() = nLastY + rMoveInner.nDY;
                }
            }

            aArr[ nPolyPos ].X() = nFirstX + 1L;
            aArr[ nPolyPos++ ].Y() = nFirstY + 1L;
            aArr.ImplSetRealSize( nPolyPos );
        }
        else if( nFlag & VECT_POLY_INLINE_OUTER )
        {
            long nFirstX, nFirstY;
            long nLastX, nLastY;

            nFirstX = nLastX = maStartPt.X();
            nFirstY = nLastY = maStartPt.Y();
            aArr.ImplSetSize( mnCount << 1 );

            sal_uInt16 i, nPolyPos;
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
                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;
                    }
                    else if( cMove == 3 && cNextMove == 2 )
                    {
                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;

                        aArr[ nPolyPos ].X() = nLastX + 1;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;
                    }
                    else if( cMove == 2 && cNextMove == 1 )
                    {
                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY + 1;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;
                    }
                    else if( cMove == 1 && cNextMove == 0 )
                    {
                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY;

                        aArr[ nPolyPos ].X() = nLastX - 1;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;

                        aArr[ nPolyPos ].X() = nLastX;
                        aArr[ nPolyPos++ ].Y() = nLastY - 1;
                    }
                    else
                        bDone = false;
                }
                else if( cMove == 7 && cNextMove == 3 )
                {
                    aArr[ nPolyPos ].X() = nLastX;
                    aArr[ nPolyPos++ ].Y() = nLastY - 1;

                    aArr[ nPolyPos ].X() = nLastX + 1;
                    aArr[ nPolyPos++ ].Y() = nLastY;
                }
                else if( cMove == 6 && cNextMove == 2 )
                {
                    aArr[ nPolyPos ].X() = nLastX + 1;
                    aArr[ nPolyPos++ ].Y() = nLastY;

                    aArr[ nPolyPos ].X() = nLastX;
                    aArr[ nPolyPos++ ].Y() = nLastY + 1;
                }
                else
                    bDone = false;

                if( !bDone )
                {
                    aArr[ nPolyPos ].X() = nLastX + rMoveOuter.nDX;
                    aArr[ nPolyPos++ ].Y() = nLastY + rMoveOuter.nDY;
                }
            }

            aArr[ nPolyPos ].X() = nFirstX - 1L;
            aArr[ nPolyPos++ ].Y() = nFirstY - 1L;
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
    pLast->X() = BACK_MAP( rArr[ 0 ].X() );
    pLast->Y() = BACK_MAP( rArr[ 0 ].Y() );

    for( n = nNewPos = 1; n < nCount; )
    {
        const Point& rPt = rArr[ n++ ];
        const long   nX = BACK_MAP( rPt.X() );
        const long   nY = BACK_MAP( rPt.Y() );

        if( nX != pLast->X() || nY != pLast->Y() )
        {
            pLast = pLeast = &( aNewArr1[ nNewPos++ ] );
            pLeast->X() = nX;
            pLeast->Y() = nY;
        }
    }

    aNewArr1.ImplSetRealSize( nCount = nNewPos );

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

        aNewArr2[ nNewPos++ ] = *( pLast = pLeast );
    }

    aNewArr2.ImplSetRealSize( nNewPos );
    aNewArr2.ImplCreatePoly( maPoly );
}

namespace ImplVectorizer {

bool ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                                    sal_uInt8 cReduce, BmpVectorizeFlags nFlags, const Link<long,void>* pProgress )
{
    bool bRet = false;

    VECT_PROGRESS( pProgress, 0 );

    std::unique_ptr<Bitmap> xBmp(new Bitmap( rColorBmp ));
    BitmapReadAccess*   pRAcc = xBmp->AcquireReadAccess();

    if( pRAcc )
    {
        tools::PolyPolygon         aPolyPoly;
        double              fPercent = 0.0;
        double              fPercentStep_2 = 0.0;
        const long          nWidth = pRAcc->Width();
        const long          nHeight = pRAcc->Height();
        const sal_uInt16        nColorCount = pRAcc->GetPaletteEntryCount();
        sal_uInt16              n;
        ImplColorSet*       pColorSet = reinterpret_cast<ImplColorSet*>(new sal_uInt8[ 256 * sizeof( ImplColorSet ) ]);

        memset( pColorSet, 0, 256 * sizeof( ImplColorSet ) );
        rMtf.Clear();

        // get used palette colors and sort them from light to dark colors
        for( n = 0; n < nColorCount; n++ )
        {
            pColorSet[ n ].mnIndex = n;
            pColorSet[ n ].maColor = pRAcc->GetPaletteColor( n );
        }

        for( long nY = 0L; nY < nHeight; nY++ )
            for( long nX = 0L; nX < nWidth; nX++ )
                pColorSet[ pRAcc->GetPixel( nY, nX ).GetIndex() ].mbSet = true;

        qsort( pColorSet, 256, sizeof( ImplColorSet ), ImplColorSetCmpFnc );

        for( n = 0; n < 256; n++ )
            if( !pColorSet[ n ].mbSet )
                break;

        if( n )
            fPercentStep_2 = 45.0 / n;

        VECT_PROGRESS( pProgress, FRound( fPercent += 10.0 ) );

        for( sal_uInt16 i = 0; i < n; i++ )
        {
            const BitmapColor   aBmpCol( pRAcc->GetPaletteColor( pColorSet[ i ].mnIndex ) );
            const Color         aFindColor( aBmpCol.GetRed(), aBmpCol.GetGreen(), aBmpCol.GetBlue() );
            std::unique_ptr<ImplVectMap> xMap(ImplExpand( pRAcc, aFindColor ));

            VECT_PROGRESS( pProgress, FRound( fPercent += fPercentStep_2 ) );

            if( xMap )
            {
                aPolyPoly.Clear();
                ImplCalculate( xMap.get(), aPolyPoly, cReduce, nFlags );
                xMap.reset();

                if( aPolyPoly.Count() )
                {
                    ImplLimitPolyPoly( aPolyPoly );

                    if( nFlags & BmpVectorizeFlags::ReduceEdges )
                        aPolyPoly.Optimize( PolyOptimizeFlags::EDGES );

                    if( aPolyPoly.Count() )
                    {
                        rMtf.AddAction( new MetaLineColorAction( aFindColor, true ) );
                        rMtf.AddAction( new MetaFillColorAction( aFindColor, true ) );
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                    }
                }
            }

            VECT_PROGRESS( pProgress, FRound( fPercent += fPercentStep_2 ) );
        }

        delete[] reinterpret_cast<sal_uInt8*>(pColorSet);

        if( rMtf.GetActionSize() )
        {
            MapMode         aMap( MAP_100TH_MM );
            ScopedVclPtrInstance< VirtualDevice > aVDev;
            const Size      aLogSize1( aVDev->PixelToLogic( Size( 1, 1 ), aMap ) );

            rMtf.SetPrefMapMode( aMap );
            rMtf.SetPrefSize( Size( nWidth + 2, nHeight + 2 ) );
            rMtf.Move( 1, 1 );
            rMtf.Scale( aLogSize1.Width(), aLogSize1.Height() );
            bRet = true;
        }
    }

    Bitmap::ReleaseAccess( pRAcc );
    xBmp.reset();
    VECT_PROGRESS( pProgress, 100 );

    return bRet;
}

bool ImplVectorize( const Bitmap& rMonoBmp,
                                    tools::PolyPolygon& rPolyPoly,
                                    BmpVectorizeFlags nFlags )
{
    std::unique_ptr<Bitmap> xBmp(new Bitmap( rMonoBmp ));
    BitmapReadAccess*   pRAcc;
    bool                bRet = false;

    if( xBmp->GetBitCount() > 1 )
        xBmp->Convert( BMP_CONVERSION_1BIT_THRESHOLD );

    pRAcc = xBmp->AcquireReadAccess();
    std::unique_ptr <ImplVectMap> xMap(ImplExpand( pRAcc, COL_BLACK ));
    Bitmap::ReleaseAccess( pRAcc );
    xBmp.reset();

    if( xMap )
    {
        rPolyPoly.Clear();
        ImplCalculate( xMap.get(), rPolyPoly, 0, nFlags );
        xMap.reset();
        ImplLimitPolyPoly( rPolyPoly );

        if( nFlags & BmpVectorizeFlags::ReduceEdges )
            rPolyPoly.Optimize( PolyOptimizeFlags::EDGES );

        // #i14895#:setting the correct direction for polygons
        // that represent holes and non-holes; non-hole polygons
        // need to have a right orientation, holes need to have a
        // left orientation in order to be treated correctly by
        // several external tools like Flash viewers
        sal_Int32   nFirstPoly = -1;
        sal_uInt16  nCurPoly( 0 ), nCount( rPolyPoly.Count() );

        for( ; nCurPoly < nCount; ++nCurPoly )
        {
            const tools::Polygon& rPoly = rPolyPoly.GetObject( nCurPoly );
            const sal_uInt16    nSize( rPoly.GetSize() );
            sal_uInt16          nDepth( 0 ), i( 0 );
            const bool          bRight( rPoly.IsRightOrientated() );

            for( ; i < nCount; ++i )
                if( ( i != nCurPoly ) && rPolyPoly.GetObject( i ).IsInside( rPoly[ 0 ] ) )
                    ++nDepth;

            const bool bHole( ( nDepth & 0x0001 ) == 1 );

            if( nSize && ( ( !bRight && !bHole ) || ( bRight && bHole ) ) )
            {
                tools::Polygon aNewPoly( nSize );
                sal_uInt16  nPrim( 0 ), nSec( nSize - 1 );

                if( rPoly.HasFlags() )
                {
                    while( nPrim < nSize )
                    {
                        aNewPoly.SetPoint( rPoly.GetPoint( nSec ), nPrim );
                        aNewPoly.SetFlags( nPrim++, rPoly.GetFlags( nSec-- ) );
                    }
                }
                else
                    while( nPrim < nSize )
                        aNewPoly.SetPoint( rPoly.GetPoint( nSec-- ), nPrim++ );

                rPolyPoly.Replace( aNewPoly, nCurPoly );
            }

            if( ( 0 == nDepth ) && ( -1 == nFirstPoly ) )
                nFirstPoly = nCurPoly;
        }

        // put outmost polygon to the front
        if( nFirstPoly > 0 )
        {
            const tools::Polygon aFirst( rPolyPoly.GetObject( static_cast< sal_uInt16 >( nFirstPoly ) ) );

            rPolyPoly.Remove( static_cast< sal_uInt16 >( nFirstPoly ) );
            rPolyPoly.Insert( aFirst, 0 );
        }

        bRet = true;
    }

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
                const Rectangle aBound( rPolyPoly[ i ].GetBoundRect() );

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
        const long          nNewWidth = ( nOldWidth << 2L ) + 4L;
        const long          nNewHeight = ( nOldHeight << 2L ) + 4L;
        const BitmapColor   aTest( pRAcc->GetBestMatchingColor( rColor ) );
        std::unique_ptr<long[]> pMapIn(new long[ std::max( nOldWidth, nOldHeight ) ]);
        std::unique_ptr<long[]> pMapOut(new long[ std::max( nOldWidth, nOldHeight ) ]);
        long                nX, nY, nTmpX, nTmpY;

        pMap = new ImplVectMap( nNewWidth, nNewHeight );

        for( nX = 0L; nX < nOldWidth; nX++ )
            VECT_MAP( pMapIn, pMapOut, nX );

        for( nY = 0L, nTmpY = 5L; nY < nOldHeight; nY++, nTmpY += 4L )
        {
            for( nX = 0L; nX < nOldWidth; )
            {
                if( pRAcc->GetPixel( nY, nX ) == aTest )
                {
                    nTmpX = pMapIn[ nX++ ];
                    nTmpY -= 3L;

                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );

                    while( nX < nOldWidth && pRAcc->GetPixel( nY, nX ) == aTest )
                         nX++;

                    nTmpX = pMapOut[ nX - 1L ];
                    nTmpY -= 3L;

                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY++, nTmpX, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );
                }
                else
                    nX++;
            }
        }

        for( nY = 0L; nY < nOldHeight; nY++ )
            VECT_MAP( pMapIn, pMapOut, nY );

        for( nX = 0L, nTmpX = 5L; nX < nOldWidth; nX++, nTmpX += 4L )
        {
            for( nY = 0L; nY < nOldHeight; )
            {
                if( pRAcc->GetPixel( nY, nX ) == aTest )
                {
                    nTmpX -= 3L;
                    nTmpY = pMapIn[ nY++ ];

                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX++, VECT_CONT_INDEX );
                    pMap->Set( nTmpY, nTmpX, VECT_CONT_INDEX );

                    while( nY < nOldHeight && pRAcc->GetPixel( nY, nX ) == aTest )
                        nY++;

                    nTmpX -= 3L;
                    nTmpY = pMapOut[ nY - 1L ];

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

void ImplCalculate( ImplVectMap* pMap, tools::PolyPolygon& rPolyPoly, sal_uInt8 cReduce, BmpVectorizeFlags nFlags )
{
    const long nWidth = pMap->Width(), nHeight= pMap->Height();

    for( long nY = 0L; nY < nHeight; nY++ )
    {
        long    nX = 0L;
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

                if( nFlags & BmpVectorizeFlags::Inner )
                    aChain.ImplEndAdd( bInner ? VECT_POLY_INLINE_INNER : VECT_POLY_INLINE_OUTER );
                else
                    aChain.ImplEndAdd( bInner ? VECT_POLY_OUTLINE_INNER : VECT_POLY_OUTLINE_OUTER );

                const tools::Polygon& rPoly = aChain.ImplGetPoly();

                if( rPoly.GetSize() > 2 )
                {
                    if( cReduce )
                    {
                        const Rectangle aBound( rPoly.GetBoundRect() );

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

                if( ( ( nX - nStartSegX ) == 1L ) || ( ImplIsUp( pMap, nY, nStartSegX ) != ImplIsUp( pMap, nY, nX - 1L ) ) )
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
    sal_uLong               nLastDir = 0UL;
    sal_uLong               nDir;

    do
    {
        nFound = 0UL;

        // first try last direction
        long nTryX = nActX + aImplMove[ nLastDir ].nDX;
        long nTryY = nActY + aImplMove[ nLastDir ].nDY;

        if( pMap->IsCont( nTryY, nTryX ) )
        {
            rChain.ImplAdd( (sal_uInt8) nLastDir );
            pMap->Set( nActY = nTryY, nActX = nTryX, VECT_DONE_INDEX );
            nFound = 1UL;
        }
        else
        {
            // try other directions
            for( nDir = 0UL; nDir < 8UL; nDir++ )
            {
                // we already tried nLastDir
                if( nDir != nLastDir )
                {
                    nTryX = nActX + aImplMove[ nDir ].nDX;
                    nTryY = nActY + aImplMove[ nDir ].nDY;

                    if( pMap->IsCont( nTryY, nTryX ) )
                    {
                        rChain.ImplAdd( (sal_uInt8) nDir );
                        pMap->Set( nActY = nTryY, nActX = nTryX, VECT_DONE_INDEX );
                        nFound = 1UL;
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

bool ImplIsUp( ImplVectMap* pMap, long nY, long nX )
{
    if( pMap->IsDone( nY - 1L, nX ) )
        return true;
    else if( pMap->IsDone( nY + 1L, nX ) )
        return false;
    else if( pMap->IsDone( nY - 1L, nX - 1L ) || pMap->IsDone( nY - 1L, nX + 1L ) )
        return true;
    else
        return false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
