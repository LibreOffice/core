/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implncvt.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_vcl.hxx"
#include <vcl/salbtype.hxx>
#ifndef _SV_IMPLNCVT_HXX
#include "implncvt.hxx"
#endif

// -----------
// - Defines -
// -----------

#define CURVE_LEFT          1
#define CURVE_RIGHT         2
#define CURVE_STRAIGHTON    3

// -----------------
// - ImplFloatPoint
// -----------------

struct ImplFloatPoint
{
    double          fX;
    double          fY;

    inline          ImplFloatPoint() {}
    inline          ImplFloatPoint( const Point& rPoint ) { fX = rPoint.X(); fY = rPoint.Y(); }
    inline          ImplFloatPoint( double _fX, double _fY ) { fX = _fX; fY = _fY; }
    inline          ImplFloatPoint( const ImplFloatPoint& rPoint ) { fX = rPoint.fX; fY = rPoint.fY; }
    inline          ~ImplFloatPoint() {}

    void            operator+=( const ImplFloatPoint& rPoint ) { fX += rPoint.fX; fY += rPoint.fY; }
    void            operator-=( const ImplFloatPoint& rPoint ) { fX -= rPoint.fX; fY -= rPoint.fY; }
    void            operator*=( const double& rD ) { fX *= rD; fY *= rD; }
    BOOL            operator==( const ImplFloatPoint& rPoint ) const { return ( ( rPoint.fX == fX ) && ( rPoint.fY == fY ) ); } const
    void            operator=( const Point rPoint ) { fX = rPoint.X(); fY = rPoint.Y(); }

    ImplFloatPoint  GetOVec( const ImplFloatPoint& rPoint ) const;
    ImplFloatPoint  GetNVec( const ImplFloatPoint& rPoint ) const;
};

// -----------------------------------------------------------------------------

ImplFloatPoint ImplFloatPoint::GetOVec( const ImplFloatPoint& rPoint ) const
{
    double fxt = rPoint.fX - fX;
    double fyt = rPoint.fY - fY;
    double fL;

    if( fyt != 0.0 )
    {
        fyt = -fxt / fyt;
        fL = sqrt( 1 + fyt * fyt );

        return ImplFloatPoint( 1.0 / fL, fyt / fL );
    }
    else
        return ImplFloatPoint( fyt, ( fxt > 0.0 ) ? 1.0 : -1.0 );
};

// -----------------------------------------------------------------------------

ImplFloatPoint ImplFloatPoint::GetNVec( const ImplFloatPoint& rPoint ) const
{
    const double fxt = rPoint.fX - fX;
    const double fyt = rPoint.fY - fY;
    const double fL = hypot( fxt, fyt );

    return ImplFloatPoint( fxt / fL, fyt / fL );
};

// --------------------
// - ImplLineConverter
// --------------------

ImplLineConverter::ImplLineConverter( const Polygon& rPolygon, const LineInfo& rLineInfo, const Point* pRefPoint ) :
    mbRefPoint      ( FALSE ),
    mfWidthHalf     ( rLineInfo.GetWidth() >> 1 ),
    maLineInfo      ( rLineInfo ),
    mpFloat0        ( new ImplFloatPoint[ 6 ] ),
    mpFloat1        ( new ImplFloatPoint[ 6 ] ),
    mnLines         ( 0 ),
    mpFloatPoint    ( NULL )
{
    UINT16  nIndex, nPolySize = rPolygon.GetSize();
    if ( nPolySize )
    {
        if( rPolygon.GetFlags( 0 ) == POLY_NORMAL )
        {
            mpFloatPoint = new ImplFloatPoint[ nPolySize ];
            mpFloatPoint[ 0 ] = rPolygon[ 0 ];

            nIndex = 0;

            while( ++nIndex < nPolySize )   // doppelte Punkte eliminieren und ein FloatPointArray anlegen
            {
                if( rPolygon.GetFlags( nIndex ) == POLY_NORMAL )
                {
                    double nxt = mpFloatPoint[ mnLines ].fX;
                    double nyt = mpFloatPoint[ mnLines ].fY;

                    if ( ( nxt == rPolygon[ nIndex ].X() ) && ( nyt == rPolygon[ nIndex ].Y() ) )
                        continue;

                    mpFloatPoint[ ++mnLines ] = rPolygon[ nIndex ];
                }
                else
                {
                    DBG_ERROR( "Bezier points not supported!" );
                }
            }
            mbClosed = ( mpFloatPoint[ 0 ] == mpFloatPoint[ mnLines ] ) ;

            if ( ( mnLines == 1 ) && ( maLineInfo.GetStyle() == LINE_DASH ) )
            {
                BOOL bX = mpFloatPoint[ 0 ].fY == mpFloatPoint[ 1 ].fY;
                BOOL bY = mpFloatPoint[ 0 ].fX == mpFloatPoint[ 1 ].fX;
                mbRefPoint = pRefPoint && ( bX || bY );
                if ( mbRefPoint )
                {
                    if ( !maLineInfo.GetDashCount() )
                    {
                        maLineInfo.SetDashCount( maLineInfo.GetDotCount() );
                        maLineInfo.SetDashLen( maLineInfo.GetDotLen() );
                        maLineInfo.SetDotCount( 0 );
                    }
                    INT32 nDistance = maLineInfo.GetDistance();
                    INT32 nDashLen = maLineInfo.GetDashCount() * ( maLineInfo.GetDashLen() + nDistance );
                    INT32 nDotLen = maLineInfo.GetDotCount() * ( maLineInfo.GetDotLen() + nDistance );
                    if ( bX )
                    {
                        if ( mpFloatPoint[ 1 ].fX > mpFloatPoint[ 0 ].fX )
                        {
                            ImplFloatPoint aFloat = mpFloatPoint[ 0 ];
                            mpFloatPoint[ 0 ] = mpFloatPoint[ 1 ];
                            mpFloatPoint[ 1 ] = aFloat;
                        }
                        mnRefDistance = (INT32)mpFloatPoint[ mnLines ].fX - pRefPoint->X();
                    }
                    else
                    {
                        if ( mpFloatPoint[ 1 ].fY > mpFloatPoint[ 0 ].fY )
                        {
                            ImplFloatPoint aFloat = mpFloatPoint[ 0 ];
                            mpFloatPoint[ 0 ] = mpFloatPoint[ 1 ];
                            mpFloatPoint[ 1 ] = aFloat;
                        }
                        mnRefDistance = (INT32)mpFloatPoint[ mnLines ].fY - pRefPoint->Y();
                    }

//                  mnRefDistance = ( (INT32)mpFloatPoint[ mnLines ].fX - pRefPoint->X() ) +
//                                      ( (INT32)mpFloatPoint[ mnLines ].fY - pRefPoint->Y() );

                    mnRefDistance = mnRefDistance % ( nDashLen + nDotLen );
                    if ( mnRefDistance < 0 )
                        mnRefDistance = ( nDashLen + nDotLen ) + mnRefDistance;
                }
            }
        }
    }
};

//------------------------------------------------------------------------

ImplLineConverter::~ImplLineConverter()
{
    delete[] mpFloat0;
    delete[] mpFloat1;
    delete[] mpFloatPoint;
};

//------------------------------------------------------------------------

const Polygon* ImplLineConverter::ImplGetFirst()
{
    mnFloat1Points = 0;
    mnLinesAvailable = mnLines;

    if ( mnLines )
    {
        if ( maLineInfo.GetStyle() == LINE_DASH )
        {
            mnDashCount = maLineInfo.GetDashCount();
            mnDotCount = maLineInfo.GetDotCount();
            mfDashDotLenght = mnDashCount ? maLineInfo.GetDashLen() : maLineInfo.GetDotLen();

            if ( mbRefPoint )
            {
                INT32 nDistance = maLineInfo.GetDistance();
                INT32 nDashLen = maLineInfo.GetDashLen() + nDistance;
                INT32 nDashesLen = maLineInfo.GetDashCount() * nDashLen;
                INT32 nDotLen = maLineInfo.GetDotLen() + nDistance;

                if ( mnRefDistance >= nDashesLen )
                {
                    // get dotcount
                    if ( nDotLen )
                    {
                        INT32 nLen = ( mnRefDistance - nDashesLen ) % nDotLen;
                        if ( nLen >= maLineInfo.GetDotLen() )
                        {
                            mnDotCount -= 1 + ( mnRefDistance - nDashesLen ) / nDotLen;
                            if ( mnDotCount )
                                mnDashCount = 0;
                            else
                                mnDotCount = maLineInfo.GetDotCount();
                            mfDashDotLenght = 0.0;
                            mfDistanceLenght = ( maLineInfo.GetDotLen() + nDistance ) - nLen;
                        }
                        else
                        {
                            mnDashCount = 0;
                            mfDashDotLenght = maLineInfo.GetDotLen() - nLen;
                            mnDotCount -= ( mnRefDistance - nDashesLen ) / nDotLen;
                        }
                    }
                }
                else
                {
                    if ( nDashLen )
                    {
                        // get dashcount
                        INT32 nLen = mnRefDistance % nDashLen;
                        if ( nLen >= maLineInfo.GetDashLen() )
                        {
                            mfDashDotLenght = 0.0;
                            mfDistanceLenght = ( maLineInfo.GetDashLen() + nDistance ) - nLen;
                            mnDashCount -= 1 + ( mnRefDistance / nDashLen );
                        }
                        else
                        {
                            mfDashDotLenght = maLineInfo.GetDashLen() - nLen;
                            mnDashCount -= ( mnRefDistance / nDashLen );
                        }
                    }
                }
                if ( ! ( mnDashCount | mnDotCount ) )
                {
                    mnDashCount = maLineInfo.GetDashCount();
                    mnDotCount = maLineInfo.GetDotCount();
                }
                if ( ( mfDashDotLenght == 0.0 ) && ( mfDistanceLenght == 0.0 ) )
                    mfDistanceLenght = maLineInfo.GetDistance();
            }
        }
    }
    return ImplGetNext();
};

//------------------------------------------------------------------------

const Polygon* ImplLineConverter::ImplGetNext()
{
    while( mnFloat1Points || mnLinesAvailable )
    {
        if ( maLineInfo.GetWidth() > 1 )
        {
            if ( !mnFloat1Points )
            {
                ImplFloatPoint aPointA( mpFloatPoint[ mnLinesAvailable-- ] );
                ImplFloatPoint aPointB( mpFloatPoint[ mnLinesAvailable ] );
                ImplFloatPoint aOVecAB( aPointA.GetOVec( aPointB ) );
                ImplFloatPoint aN1Vec( aPointA.GetNVec( aPointB ) );
                aN1Vec *= mfWidthHalf;

                if ( !mbClosed && ( ( mnLinesAvailable + 1 ) ==  mnLines ) )
                    aPointA -= aN1Vec;

                aOVecAB *= mfWidthHalf;
                mpFloat0[ 0 ] = aPointA;
                mpFloat0[ 0 ] -= aOVecAB;
                mpFloat0[ 3 ] = aPointA;
                mpFloat0[ 3 ] += aOVecAB;
                mpFloat0[ 1 ] = aPointB;
                mpFloat0[ 1 ] -= aOVecAB;
                mpFloat0[ 2 ] = aPointB;
                mpFloat0[ 2 ] += aOVecAB;

                double f1D = ( aN1Vec.fX == 0 ) ? 1 : ( aN1Vec.fY / aN1Vec.fX );
                double f2D = -f1D;

                mnFloat0Points = 4;

                int nDirection;

                BOOL bContinues = ( mnLinesAvailable || mbClosed );
                if ( bContinues )
                {
                    ImplFloatPoint aPointC;

                    if ( mnLinesAvailable )
                        aPointC = mpFloatPoint[  mnLinesAvailable - 1 ];
                    else
                        aPointC = mpFloatPoint[ mnLines - 1 ];

                    ImplFloatPoint aOVecBC( aPointB.GetOVec( aPointC ) );
                    aOVecBC *= mfWidthHalf;
                    ImplFloatPoint aPointR0( aPointB );
                    aPointR0 -= aOVecBC;
                    ImplFloatPoint aPointR1( aPointB );
                    aPointR1 += aOVecBC;
                    ImplFloatPoint aN2Vec( aPointB.GetNVec( aPointC ) );
                    aN2Vec *= mfWidthHalf;

                    f2D = ( fabs( aN2Vec.fX ) < 0.00000001 ) ? 1 : ( aN2Vec.fY / aN2Vec.fX );
                    if ( fabs( f1D - f2D ) < 0.00000001 )
                        nDirection = CURVE_STRAIGHTON;
                    else
                    {
                        if ( ( aN1Vec.fX * aN2Vec.fY - aN1Vec.fY * aN2Vec.fX ) > 0 )
                            nDirection = CURVE_LEFT;
                        else
                            nDirection = CURVE_RIGHT;
                    }
                    if ( nDirection != CURVE_STRAIGHTON )
                    {
                        double fWidth;
                        ImplFloatPoint aDestPoint;
                        if ( hypot( aPointR0.fX - aPointA.fX, aPointR0.fY - aPointA.fY ) > hypot( aPointR1.fX - aPointA.fX, aPointR1.fY - aPointA.fY ) )
                            aDestPoint = aPointR0;
                        else
                            aDestPoint = aPointR1;

                        UINT16  nFirst = 0;
                        if ( aN1Vec.fY > 0 )
                        {
                            if ( nDirection != CURVE_RIGHT )
                                nFirst++;
                        }
                        else
                        {
                            if ( nDirection == CURVE_RIGHT )
                                nFirst++;
                        }
                        fWidth = hypot( mpFloat0[ 1 + nFirst ].fX - aDestPoint.fX, mpFloat0[ 1 + nFirst ].fY - aDestPoint.fY );
                        fWidth = sqrt( fWidth * fWidth / 2 );
                        if ( fWidth > mfWidthHalf )
                        {
                            // Spitzer Winkel :
                            mnFloat0Points = 6;
                            mpFloat0[ 4 + nFirst ^ 1 ] = aDestPoint;
                            aDestPoint -= aN2Vec;
                            mpFloat0[ 4 + nFirst ] = aDestPoint;
                            mpFloat0[ 1 + nFirst ] += aN1Vec;
                        }
                        else
                        {
                            // Stumpferwinkel : Schnittpunkt wird berechnet
                            mnFloat0Points = 5;
                            ImplFloatPoint aSourcePoint;
                            double fX = 0;
                            double fY;
                            double fBDest = 0;
                            double fBSource = 0;
                            aSourcePoint = mpFloat0[ 1 + nFirst ];

                            int nValid = 0;

                            if ( fabs( aN2Vec.fX ) < 0.00000001 )
                            {
                                fX = aDestPoint.fX;
                                nValid = 1;
                            }
                            else
                                fBDest = aDestPoint.fY - ( aN2Vec.fY / aN2Vec.fX * aDestPoint.fX );

                            if ( fabs( aN1Vec.fX ) < 0.000000001 )
                            {
                                fX = aSourcePoint.fX;
                                nValid = 2;
                            }
                            else
                                fBSource = aSourcePoint.fY - ( aN1Vec.fY / aN1Vec.fX * aSourcePoint.fX );

                            if ( !nValid )
                                fX = ( fBSource - fBDest ) / ( aN2Vec.fY / aN2Vec.fX - aN1Vec.fY / aN1Vec.fX );
                            if ( nValid < 2 )
                                fY = aN1Vec.fY / aN1Vec.fX * fX + fBSource;
                            else
                                fY = aN2Vec.fY / aN2Vec.fX * fX + fBDest;

                            mpFloat0[ 1 + nFirst ].fX = fX;
                            mpFloat0[ 1 + nFirst ].fY = fY;
                            mpFloat0[ 4 ] = aDestPoint;
                        }
                    }
                    else if ( ( aN1Vec.fX - aN2Vec.fX + aN1Vec.fY - aN2Vec.fY ) != 0 )  // besitzt zweiter Richtungsvektor die gleiche Steigung aber andere
                        bContinues = FALSE;                                             // Richtung, dann wird hinten noch eine halbe Linienbreite angehaengt
                }
                if ( !bContinues )
                {
                    mpFloat0[ 1 ] += aN1Vec;
                    mpFloat0[ 2 ] += aN1Vec;
                }
            }
            else
            {
                mnFloat0Points = mnFloat1Points;
                ImplFloatPoint* pTemp = mpFloat1;
                mpFloat1 = mpFloat0;
                mpFloat0 = pTemp;
            }
            if ( maLineInfo.GetStyle() == LINE_DASH )
            {
                double fLenghtDone = 0;
                double fLenght = ( mfDashDotLenght > 0.0 ) ? mfDashDotLenght : mfDistanceLenght;

                double fDistance;

                fDistance = hypot( mpFloat0[ 0 ].fX - mpFloat0[ 1 ].fX, mpFloat0[ 0 ].fY - mpFloat0[ 1 ].fY );
                if ( mnFloat0Points == 5 )
                {
                    double fDist = hypot( mpFloat0[ 2 ].fX - mpFloat0[ 3 ].fX, mpFloat0[ 2 ].fY - mpFloat0[ 3 ].fY );
                    if ( fDist < fDistance )
                        fDistance = fDist;
                }

                if ( fDistance > fLenght )
                {
                    fLenghtDone = fLenght;

                    ImplFloatPoint aNVec( mpFloat0[ 0 ].GetNVec( mpFloat0[ 1 ] ) );
                    aNVec *= fLenght;
                    mnFloat1Points = mnFloat0Points;
                    ImplFloatPoint* pTemp = mpFloat1;
                    mpFloat1 = mpFloat0;
                    mpFloat0 = pTemp;
                    mnFloat0Points = 4;
                    mpFloat0[ 0 ] = mpFloat0[ 1 ] = mpFloat1[ 0 ];
                    mpFloat0[ 1 ] += aNVec;
                     mpFloat0[ 2 ] = mpFloat0[ 3 ] = mpFloat1[ 3 ];
                    mpFloat0[ 2 ] += aNVec;

                    mpFloat1[ 0 ] = mpFloat0[ 1 ];
                    mpFloat1[ 3 ] = mpFloat0[ 2 ];
                }
                else
                {
                    mnFloat1Points = 0;
                    fLenghtDone = fDistance;
                }

                if ( mfDashDotLenght > 0.0 )
                {                                       // Ein Dash oder Dot wurde erzeugt
                    mfDashDotLenght -= fLenghtDone;
                    if ( mfDashDotLenght == 0.0 )
                    {                                   // Komplett erzeugt
                        if ( mnDashCount )
                            mnDashCount--;
                        else
                            mnDotCount--;

                        if ( ! ( mnDashCount | mnDotCount ) )
                        {
                            mnDashCount = maLineInfo.GetDashCount();
                            mnDotCount = maLineInfo.GetDotCount();
                        }
                        mfDistanceLenght = maLineInfo.GetDistance();
                    }
                }
                else
                {                                       // Das erzeugte Polygon muessen wir ignorieren
                    mfDistanceLenght -= fLenghtDone;
                    if ( mfDistanceLenght ==  0.0 )
                        mfDashDotLenght = ( mnDashCount ) ? maLineInfo.GetDashLen() : maLineInfo.GetDotLen();
                    continue;
                }
            }
            maPolygon.SetSize( (UINT16)mnFloat0Points );
              UINT16 i = 0;
            maPolygon[ i++ ] = Point( FRound( mpFloat0[ 0 ].fX ), FRound( mpFloat0[ 0 ].fY ) );
            maPolygon[ i++ ] = Point( FRound( mpFloat0[ 1 ].fX ), FRound( mpFloat0[ 1 ].fY ) );
            if ( mnFloat0Points > 4 )
                maPolygon[ i++ ] = Point( FRound( mpFloat0[ 4 ].fX ), FRound( mpFloat0[ 4 ].fY ) );
            if ( mnFloat0Points > 5 )
                maPolygon[ i++ ] = Point( FRound( mpFloat0[ 5 ].fX ), FRound( mpFloat0[ 5 ].fY ) );
            maPolygon[ i++ ] = Point( FRound( mpFloat0[ 2 ].fX ), FRound( mpFloat0[ 2 ].fY ) );
            maPolygon[ i ] = Point( FRound( mpFloat0[ 3 ].fX ), FRound( mpFloat0[ 3 ].fY ) );

        }
        else
        {
            if ( !mnFloat1Points )
            {
                mpFloat0[ 0 ] = mpFloatPoint[ mnLinesAvailable-- ];
                mpFloat0[ 1 ] = mpFloatPoint[ mnLinesAvailable ];
            }
            else
            {
                mpFloat0[ 0 ] = mpFloat1[ 0 ];
                mpFloat0[ 1 ] = mpFloat1[ 1 ];
            }
            if ( maLineInfo.GetStyle() == LINE_DASH )
            {
                double fLenghtDone = 0;
                double fLenght = ( mfDashDotLenght > 0.0 ) ? mfDashDotLenght : mfDistanceLenght;
                double fDistance;
                fDistance = hypot( mpFloat0[ 0 ].fX - mpFloat0[ 1 ].fX, mpFloat0[ 0 ].fY - mpFloat0[ 1 ].fY );
                if ( fDistance > fLenght )
                {
                    fLenghtDone = fLenght;
                    ImplFloatPoint aNVec( mpFloat0[ 0 ].GetNVec( mpFloat0[ 1 ] ) );
                    aNVec *= fLenght;
                    mpFloat1[ 1 ] = mpFloat0[ 1 ];
                    mpFloat0[ 1 ] = mpFloat0[ 0 ];
                    mpFloat0[ 1 ] += aNVec;
                    mpFloat1[ 0 ] = mpFloat0[ 1 ];
                    mnFloat1Points = 2;
                }
                else
                {
                    mnFloat1Points = 0;
                    fLenghtDone = fDistance;
                }
                if ( mfDashDotLenght > 0.0 )
                {                                       // Ein Dash oder Dot wurde erzeugt
                    mfDashDotLenght -= fLenghtDone;
                    if ( mfDashDotLenght == 0.0 )
                    {                                   // Komplett erzeugt
                        if ( mnDashCount )
                            mnDashCount--;
                        else
                            mnDotCount--;

                        if ( ! ( mnDashCount | mnDotCount ) )
                        {
                            mnDashCount = maLineInfo.GetDashCount();
                            mnDotCount = maLineInfo.GetDotCount();
                        }
                        mfDistanceLenght = maLineInfo.GetDistance();
                    }
                }
                else
                {                                       // Das erzeugte Polygon muessen wir ignorieren
                    mfDistanceLenght -= fLenghtDone;
                    if ( mfDistanceLenght ==  0.0 )
                        mfDashDotLenght = ( mnDashCount ) ? maLineInfo.GetDashLen() : maLineInfo.GetDotLen();
                    continue;
                }
            }
            maPolygon.SetSize( 2 );
            maPolygon[ 0 ] = Point( (long)mpFloat0[ 0 ].fX, (long)mpFloat0[ 0 ].fY );
            maPolygon[ 1 ] = Point( (long)mpFloat0[ 1 ].fX, (long)mpFloat0[ 1 ].fY );
        }
        return &maPolygon;
    }
    return NULL;
};
