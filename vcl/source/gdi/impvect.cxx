/*************************************************************************
 *
 *  $RCSfile: impvect.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:38 $
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

#define _SV_IMPVECT_CXX

#include <stdlib.h>
#include <tools/new.hxx>
#ifndef _SV_BMPACC_HXX
#include <bmpacc.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_VECTORIZ_HXX
#include <impvect.hxx>
#endif

// !!! ggf. einkommentieren, um Bitmaps zu erzeugen (nur, wenn File mit Debug uebersetzt wurde)
// #define DEBUG_BMPOUTPUT

#if defined DEBUG && defined DEBUG_BMPOUTPUT
#define DBG_BMP 1
#else
#undef  DBG_BMP
#endif

#ifdef DBG_BMP
#include <tools/stream.hxx>
#endif

// -----------
// - Defines -
// -----------

#define VECT_POLY_MAX 8192

// -----------------------------------------------------------------------------

#define VECT_FREE_INDEX 0
#define VECT_CONT_INDEX 1
#define VECT_DONE_INDEX 2

// -----------------------------------------------------------------------------

#define VECT_POLY_INLINE_INNER  1UL
#define VECT_POLY_INLINE_OUTER  2UL
#define VECT_POLY_OUTLINE_INNER 4UL
#define VECT_POLY_OUTLINE_OUTER 8UL

// -----------------------------------------------------------------------------

#define VECT_MAP( _def_pIn, _def_pOut, _def_nVal )  _def_pOut[_def_nVal]=(_def_pIn[_def_nVal]=((_def_nVal)*4L)+1L)+5L;
#define BACK_MAP( _def_nVal )                       ((((_def_nVal)+2)>>2)-1)
#define VECT_PROGRESS( _def_pProgress, _def_nVal )  if(_def_pProgress&&_def_pProgress->IsSet())(_def_pProgress->Call((void*)_def_nVal));

// -----------
// - statics -
// -----------

struct ChainMove { long nDX; long nDY; };

static ChainMove aImplMove[ 8 ] =   {
                                        { 1L, 0L },
                                        { 0L, -1L },
                                        { -1L, 0L },
                                        { 0L, 1L },
                                        { 1L, -1L },
                                        { -1, -1L },
                                        { -1L, 1L },
                                        { 1L, 1L }
                                    };

static ChainMove aImplMoveInner[ 8 ] =  {
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L },
                                            { -1L, 0L }
                                        };

static ChainMove aImplMoveOuter[ 8 ] =  {
                                            { 0L, -1L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { -1L, 0L },
                                            { 0L, 1L },
                                            { 1L, 0L },
                                            { 0L, -1L }
                                        };

// ----------------
// - ImplColorSet -
// ----------------

struct ImplColorSet
{
    BitmapColor maColor;
    USHORT      mnIndex;
    BOOL        mbSet;

    BOOL        operator<( const ImplColorSet& rSet ) const;
    BOOL        operator>( const ImplColorSet& rSet ) const;
};

// ----------------------------------------------------------------------------

inline BOOL ImplColorSet::operator<( const ImplColorSet& rSet ) const
{
    return( mbSet && ( !rSet.mbSet || ( maColor.GetLuminance() > rSet.maColor.GetLuminance() ) ) );
}

// ----------------------------------------------------------------------------

inline BOOL ImplColorSet::operator>( const ImplColorSet& rSet ) const
{
    return( !mbSet || ( rSet.mbSet && maColor.GetLuminance() < rSet.maColor.GetLuminance() ) );
}

// ----------------------------------------------------------------------------

extern "C" int __LOADONCALLAPI ImplColorSetCmpFnc( const void* p1, const void* p2 )
{
    ImplColorSet*   pSet1 = (ImplColorSet*) p1;
    ImplColorSet*   pSet2 = (ImplColorSet*) p2;
    int             nRet;

    if( pSet1->mbSet && pSet2->mbSet )
    {
        const BYTE cLum1 = pSet1->maColor.GetLuminance();
        const BYTE cLum2 = pSet2->maColor.GetLuminance();
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

// ------------------
// - ImplPointArray -
// ------------------

#ifdef WIN
typedef Point* huge HPPoint;
#else
typedef Point*      HPPoint;
#endif

class ImplPointArray
{
    HPPoint             mpArray;
    ULONG               mnSize;
    ULONG               mnRealSize;

public:

                        ImplPointArray();
                        ~ImplPointArray();

    void                ImplSetSize( ULONG nSize );

    ULONG               ImplGetRealSize() const { return mnRealSize; }
    void                ImplSetRealSize( ULONG nRealSize ) { mnRealSize = nRealSize; }

    inline Point&       operator[]( ULONG nPos );
    inline const Point& operator[]( ULONG nPos ) const;

    void                ImplCreatePoly( Polygon& rPoly ) const;
};

// -----------------------------------------------------------------------------

ImplPointArray::ImplPointArray() :
    mnSize      ( 0UL ),
    mnRealSize  ( 0UL ),
    mpArray     ( NULL )

{
}

// -----------------------------------------------------------------------------

ImplPointArray::~ImplPointArray()
{
    if( mpArray )
        SvMemFree( mpArray );
}

// -----------------------------------------------------------------------------

void ImplPointArray::ImplSetSize( ULONG nSize )
{
    const ULONG nTotal = nSize * sizeof( Point );

    mnSize = nSize;
    mnRealSize = 0UL;

    if( mpArray )
        SvMemFree( mpArray );

    mpArray = (HPPoint) SvMemAlloc( nTotal );
    HMEMSET( (HPBYTE) mpArray, 0, nTotal );
}

// -----------------------------------------------------------------------------

inline Point& ImplPointArray::operator[]( ULONG nPos )
{
    DBG_ASSERT( nPos < mnSize, "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

// -----------------------------------------------------------------------------

inline const Point& ImplPointArray::operator[]( ULONG nPos ) const
{
    DBG_ASSERT( nPos < mnSize, "ImplPointArray::operator[]: nPos out of range!" );
    return mpArray[ nPos ];
}

// -----------------------------------------------------------------------------

void ImplPointArray::ImplCreatePoly( Polygon& rPoly ) const
{
    rPoly.SetSize( (USHORT) mnRealSize );
    HMEMCPY( rPoly.ImplGetPointAry(), mpArray, mnRealSize * sizeof( Point ) );
}

// ---------------
// - ImplVectMap -
// ---------------

class ImplVectMap
{
private:

    Scanline        mpBuf;
    Scanline*       mpScan;
    long            mnWidth;
    long            mnHeight;

                    ImplVectMap() {};

public:

                    ImplVectMap( long nWidth, long nHeight );
                    ~ImplVectMap();

    inline long     Width() const { return mnWidth; }
    inline long     Height() const { return mnHeight; }

    inline void     Set( long nY, long nX, BYTE cVal );
    inline BYTE     Get( long nY, long nX ) const;

    inline BOOL     IsFree( long nY, long nX ) const;
    inline BOOL     IsCont( long nY, long nX ) const;
    inline BOOL     IsDone( long nY, long nX ) const;

#ifdef DBG_BMP
    Bitmap          GetBitmap() const;
#endif // DBG_BMP
};

// -----------------------------------------------------------------------------

ImplVectMap::ImplVectMap( long nWidth, long nHeight ) :
    mnWidth ( nWidth ),
    mnHeight( nHeight )
{
    const long  nWidthAl = ( nWidth >> 2L ) + 1L;
    const long  nSize = nWidthAl * nHeight;
    Scanline    pTmp = mpBuf = (Scanline) SvMemAlloc( nSize );

    HMEMSET( mpBuf, 0, nSize );
    mpScan = (Scanline*) SvMemAlloc( nHeight * sizeof( Scanline ) );

    for( long nY = 0L; nY < nHeight; pTmp += nWidthAl )
        mpScan[ nY++ ] = pTmp;
}

// -----------------------------------------------------------------------------


ImplVectMap::~ImplVectMap()
{
    SvMemFree( mpBuf );
    SvMemFree( mpScan );
}

// -----------------------------------------------------------------------------

inline void ImplVectMap::Set( long nY, long nX, BYTE cVal )
{
    const BYTE cShift = 6 - ( ( nX & 3 ) << 1 );
    ( ( mpScan[ nY ][ nX >> 2 ] ) &= ~( 3 << cShift ) ) |= ( cVal << cShift );
}

// -----------------------------------------------------------------------------

inline BYTE ImplVectMap::Get( long nY, long nX ) const
{
    return( ( ( mpScan[ nY ][ nX >> 2 ] ) >> ( 6 - ( ( nX & 3 ) << 1 ) ) ) & 3 );
}

// -----------------------------------------------------------------------------

inline BOOL ImplVectMap::IsFree( long nY, long nX ) const
{
    return( VECT_FREE_INDEX == Get( nY, nX ) );
}

// -----------------------------------------------------------------------------

inline BOOL ImplVectMap::IsCont( long nY, long nX ) const
{
    return( VECT_CONT_INDEX == Get( nY, nX ) );
}

// -----------------------------------------------------------------------------

inline BOOL ImplVectMap::IsDone( long nY, long nX ) const
{
    return( VECT_DONE_INDEX == Get( nY, nX ) );
}

// -----------------------------------------------------------------------------

#ifdef DBG_BMP
Bitmap ImplVectMap::GetBitmap() const
{
    Bitmap              aBmp( Size( mnWidth, mnHeight ), 4 );
    BitmapWriteAccess*  pAcc = aBmp.AcquireWriteAccess();

    if( pAcc )
    {
        for( long nY = 0L; nY < mnHeight; nY++ )
        {
            for( long nX = 0L; nX < mnWidth; nX++ )
            {
                switch( Get( nY, nX ) )
                {
                    case( VECT_FREE_INDEX ): pAcc->SetPixel( nY, nX, 15 ); break;
                    case( VECT_CONT_INDEX ): pAcc->SetPixel( nY, nX, 0 ); break;
                    case( VECT_DONE_INDEX ): pAcc->SetPixel( nY, nX, 2); break;
                }
            }
        }

        aBmp.ReleaseAccess( pAcc );
    }

    return aBmp;
}
#endif // DBG_BMP

// -------------
// - ImplChain -
// -------------

class ImplChain
{
private:

    Polygon         maPoly;
    Point           maStartPt;
    ULONG           mnArraySize;
    ULONG           mnCount;
    long            mnResize;
    BYTE*           mpCodes;

    void            ImplGetSpace();

    void            ImplCreate();
    void            ImplCreateInner();
    void            ImplCreateOuter();
    void            ImplPostProcess( const ImplPointArray& rArr );

public:

                    ImplChain( ULONG nInitCount = 1024UL, long nResize = -1L );
                    ~ImplChain();

    void            ImplBeginAdd( const Point& rStartPt );
    inline void     ImplAdd( BYTE nCode );
    void            ImplEndAdd( ULONG nTypeFlag );

    const Polygon&  ImplGetPoly() { return maPoly; }
};

// -----------------------------------------------------------------------------

ImplChain::ImplChain( ULONG nInitCount, long nResize ) :
    mnCount     ( 0UL ),
    mnArraySize ( nInitCount ),
    mnResize    ( nResize )
{
    DBG_ASSERT( nInitCount && nResize, "ImplChain::ImplChain(): invalid parameters!" );
    mpCodes = new BYTE[ mnArraySize ];
}

// -----------------------------------------------------------------------------

ImplChain::~ImplChain()
{
    delete[] mpCodes;
}

// -----------------------------------------------------------------------------

void ImplChain::ImplGetSpace()
{
    const ULONG nOldArraySize = mnArraySize;
    BYTE*       pNewCodes;

    mnArraySize = ( mnResize < 0L ) ? ( mnArraySize << 1UL ) : ( mnArraySize + (ULONG) mnResize );
    pNewCodes = new BYTE[ mnArraySize ];
    HMEMCPY( pNewCodes, mpCodes, nOldArraySize );
    delete[] mpCodes;
    mpCodes = pNewCodes;
}

// -----------------------------------------------------------------------------

void ImplChain::ImplBeginAdd( const Point& rStartPt )
{
    maPoly = Polygon();
    maStartPt = rStartPt;
    mnCount = 0UL;
}

// -----------------------------------------------------------------------------

inline void ImplChain::ImplAdd( BYTE nCode )
{
    if( mnCount == mnArraySize )
        ImplGetSpace();

    mpCodes[ mnCount++ ] = nCode;
}

// -----------------------------------------------------------------------------

void ImplChain::ImplEndAdd( ULONG nFlag )
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

            USHORT i, nPolyPos;
            for( i = 0, nPolyPos = 0; i < ( mnCount - 1 ); i++ )
            {
                const BYTE              cMove = mpCodes[ i ];
                const BYTE              cNextMove = mpCodes[ i + 1 ];
                const ChainMove&        rMove = aImplMove[ cMove ];
                const ChainMove&        rMoveInner = aImplMoveInner[ cMove ];
                Point&                  rPt = aArr[ nPolyPos ];
                BOOL                    bDone = TRUE;

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
                        bDone = FALSE;
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
                    bDone = FALSE;

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

            USHORT i, nPolyPos;
            for( i = 0, nPolyPos = 0; i < ( mnCount - 1 ); i++ )
            {
                const BYTE              cMove = mpCodes[ i ];
                const BYTE              cNextMove = mpCodes[ i + 1 ];
                const ChainMove&        rMove = aImplMove[ cMove ];
                const ChainMove&        rMoveOuter = aImplMoveOuter[ cMove ];
                Point&                  rPt = aArr[ nPolyPos ];
                BOOL                    bDone = TRUE;

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
                        bDone = FALSE;
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
                    bDone = FALSE;

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

            for( ULONG i = 0; i < mnCount; )
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

// -----------------------------------------------------------------------------

void ImplChain::ImplPostProcess( const ImplPointArray& rArr )
{
    ImplPointArray  aNewArr1;
    ImplPointArray  aNewArr2;
    Point*          pLast;
    Point*          pLeast;
    ULONG           nNewPos;
    ULONG           nCount = rArr.ImplGetRealSize();
    ULONG           n;

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

// ------------------
// - ImplVectorizer -
// ------------------

ImplVectorizer::ImplVectorizer()
{
}

// -----------------------------------------------------------------------------

ImplVectorizer::~ImplVectorizer()
{
}

// -----------------------------------------------------------------------------

BOOL ImplVectorizer::ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                                    BYTE cReduce, ULONG nFlags, const Link* pProgress )
{
    BOOL bRet = FALSE;

    VECT_PROGRESS( pProgress, 0 );

    Bitmap*             pBmp = new Bitmap( rColorBmp );
    BitmapReadAccess*   pRAcc = pBmp->AcquireReadAccess();

    if( pRAcc )
    {
        PolyPolygon         aPolyPoly;
        double              fPercent = 0.0;
        double              fPercentStep_2 = 0.0;
        const long          nWidth = pRAcc->Width();
        const long          nHeight = pRAcc->Height();
        const USHORT        nColorCount = pRAcc->GetPaletteEntryCount();
        USHORT              n;
        ImplColorSet*       pColorSet = (ImplColorSet*) new BYTE[ 256 * sizeof( ImplColorSet ) ];

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
                pColorSet[ pRAcc->GetPixel( nY, nX ).GetIndex() ].mbSet = 1;

        qsort( pColorSet, 256, sizeof( ImplColorSet ), ImplColorSetCmpFnc );

        for( n = 0; n < 256; n++ )
            if( !pColorSet[ n ].mbSet )
                break;

        if( n )
            fPercentStep_2 = 45.0 / n;

        VECT_PROGRESS( pProgress, FRound( fPercent += 10.0 ) );

        for( USHORT i = 0; i < n; i++ )
        {
            const BitmapColor   aBmpCol( pRAcc->GetPaletteColor( pColorSet[ i ].mnIndex ) );
            const Color         aFindColor( aBmpCol.GetRed(), aBmpCol.GetGreen(), aBmpCol.GetBlue() );
            const BYTE          cLum = aFindColor.GetLuminance();
            ImplVectMap*        pMap = ImplExpand( pRAcc, aFindColor );

            VECT_PROGRESS( pProgress, FRound( fPercent += fPercentStep_2 ) );

            if( pMap )
            {
                aPolyPoly.Clear();
                ImplCalculate( pMap, aPolyPoly, cReduce, nFlags );
                delete pMap;

                if( aPolyPoly.Count() )
                {
                    ImplLimitPolyPoly( aPolyPoly );

                    if( nFlags & BMP_VECTORIZE_REDUCE_EDGES )
                        aPolyPoly.Optimize( POLY_OPTIMIZE_EDGES );

                    if( aPolyPoly.Count() )
                    {
                        rMtf.AddAction( new MetaLineColorAction( aFindColor, TRUE ) );
                        rMtf.AddAction( new MetaFillColorAction( aFindColor, TRUE ) );
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                    }
                }
            }

            VECT_PROGRESS( pProgress, FRound( fPercent += fPercentStep_2 ) );
        }

        delete[] (BYTE*) pColorSet;

        if( rMtf.GetActionCount() )
        {
            MapMode         aMap( MAP_100TH_MM );
            VirtualDevice   aVDev;
            const Size      aLogSize1( aVDev.PixelToLogic( Size( 1, 1 ), aMap ) );

            rMtf.SetPrefMapMode( aMap );
            rMtf.SetPrefSize( Size( nWidth + 2, nHeight + 2 ) );
            rMtf.Move( 1, 1 );
            rMtf.Scale( aLogSize1.Width(), aLogSize1.Height() );
            bRet = TRUE;
        }
    }

    pBmp->ReleaseAccess( pRAcc );
    delete pBmp;
    VECT_PROGRESS( pProgress, 100 );

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL ImplVectorizer::ImplVectorize( const Bitmap& rMonoBmp,
                                    PolyPolygon& rPolyPoly,
                                    ULONG nFlags, const Link* pProgress )
{
    Bitmap*             pBmp = new Bitmap( rMonoBmp );
    BitmapReadAccess*   pRAcc;
    ImplVectMap*        pMap;
    BOOL                bRet = FALSE;

    VECT_PROGRESS( pProgress, 10 );

    if( pBmp->GetBitCount() > 1 )
        pBmp->Convert( BMP_CONVERSION_1BIT_THRESHOLD );

    VECT_PROGRESS( pProgress, 30 );

    pRAcc = pBmp->AcquireReadAccess();
    pMap = ImplExpand( pRAcc, COL_BLACK );
    pBmp->ReleaseAccess( pRAcc );
    delete pBmp;

    VECT_PROGRESS( pProgress, 60 );

    if( pMap )
    {
        rPolyPoly.Clear();
        ImplCalculate( pMap, rPolyPoly, 0, nFlags );
        delete pMap;
        ImplLimitPolyPoly( rPolyPoly );

        if( nFlags & BMP_VECTORIZE_REDUCE_EDGES )
            rPolyPoly.Optimize( POLY_OPTIMIZE_EDGES );

        bRet = TRUE;
    }

    VECT_PROGRESS( pProgress, 100 );

    return bRet;
}

// -----------------------------------------------------------------------------

void ImplVectorizer::ImplLimitPolyPoly( PolyPolygon& rPolyPoly )
{
    if( rPolyPoly.Count() > VECT_POLY_MAX )
    {
        PolyPolygon aNewPolyPoly;
        long        nReduce = 0;
        USHORT      nNewCount;

        do
        {
            aNewPolyPoly.Clear();
            nReduce++;

            for( USHORT i = 0, nCount = rPolyPoly.Count(); i < nCount; i++ )
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

// -----------------------------------------------------------------------------

ImplVectMap* ImplVectorizer::ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor )
{
    ImplVectMap* pMap = NULL;

       if( pRAcc && pRAcc->Width() && pRAcc->Height() )
    {
        const long          nOldWidth = pRAcc->Width();
        const long          nOldHeight = pRAcc->Height();
        const long          nNewWidth = ( nOldWidth << 2L ) + 4L;
        const long          nNewHeight = ( nOldHeight << 2L ) + 4L;
        const BitmapColor   aTest( pRAcc->GetBestMatchingColor( rColor ) );
        long*               pMapIn = new long[ Max( nOldWidth, nOldHeight ) ];
        long*               pMapOut = new long[ Max( nOldWidth, nOldHeight ) ];
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

        // cleanup
        delete[] pMapIn;
        delete[] pMapOut;
    }

    return pMap;
}

// -----------------------------------------------------------------------------

void ImplVectorizer::ImplCalculate( ImplVectMap* pMap, PolyPolygon& rPolyPoly, BYTE cReduce, ULONG nFlags )
{
    const long nWidth = pMap->Width(), nHeight= pMap->Height();

#ifdef DBG_BMP
    if( pMap )
    {
        SvFileStream aOStm( "d:\\cont.bmp", STREAM_WRITE | STREAM_TRUNC );
        aOStm << pMap->GetBitmap();
    }
#endif // DBG_BMP

    for( long nY = 0L; nY < nHeight; nY++ )
    {
        long    nX = 0L;
        BOOL    bInner = TRUE;

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

                if( nFlags & BMP_VECTORIZE_INNER )
                    aChain.ImplEndAdd( bInner ? VECT_POLY_INLINE_INNER : VECT_POLY_INLINE_OUTER );
                else
                    aChain.ImplEndAdd( bInner ? VECT_POLY_OUTLINE_INNER : VECT_POLY_OUTLINE_OUTER );

                const Polygon& rPoly = aChain.ImplGetPoly();

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

#ifdef DBG_BMP
    if( pMap )
    {
        SvFileStream aOStm( "d:\\vect.bmp", STREAM_WRITE | STREAM_TRUNC );
        aOStm << pMap->GetBitmap();
    }
#endif // DBG_BMP
}

// -----------------------------------------------------------------------------

BOOL ImplVectorizer::ImplGetChain(  ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain )
{
    long                nActX = rStartPt.X();
    long                nActY = rStartPt.Y();
    long                nTryX;
    long                nTryY;
    ULONG               nFound;
    ULONG               nLastDir = 0UL;
    ULONG               nDir;

    do
    {
        nFound = 0UL;

        // first try last direction
        nTryX = nActX + aImplMove[ nLastDir ].nDX;
        nTryY = nActY + aImplMove[ nLastDir ].nDY;

        if( pMap->IsCont( nTryY, nTryX ) )
        {
            rChain.ImplAdd( (BYTE) nLastDir );
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
                        rChain.ImplAdd( (BYTE) nDir );
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

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL ImplVectorizer::ImplIsUp( ImplVectMap* pMap, long nY, long nX ) const
{
    if( pMap->IsDone( nY - 1L, nX ) )
        return TRUE;
    else if( pMap->IsDone( nY + 1L, nX ) )
        return FALSE;
    else if( pMap->IsDone( nY - 1L, nX - 1L ) || pMap->IsDone( nY - 1L, nX + 1L ) )
        return TRUE;
    else
        return FALSE;
}
