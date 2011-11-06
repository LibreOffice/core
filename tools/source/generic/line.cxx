/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _LINE_CXX
#include <tools/link.hxx>
#include <tools/line.hxx>
#include <tools/debug.hxx>

#include <cstdlib>
#include <math.h>

// --------
// - Line -
// --------

double Line::GetLength() const
{
    return hypot( maStart.X() - maEnd.X(), maStart.Y() - maEnd.Y() );
}

// ------------------------------------------------------------------------

sal_Bool Line::Intersection( const Line& rLine, Point& rIntersection ) const
{
    double  fX, fY;
    sal_Bool    bRet;

    if( Intersection( rLine, fX, fY ) )
    {
        rIntersection.X() = FRound( fX );
        rIntersection.Y() = FRound( fY );
        bRet = sal_True;
    }
    else
        bRet = sal_False;

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool Line::Intersection( const Line& rLine, double& rIntersectionX, double& rIntersectionY ) const
{
    const double    fAx = maEnd.X() - maStart.X();
    const double    fAy = maEnd.Y() - maStart.Y();
    const double    fBx = rLine.maStart.X() - rLine.maEnd.X();
    const double    fBy = rLine.maStart.Y() - rLine.maEnd.Y();
    const double    fDen = fAy * fBx - fAx * fBy;
    sal_Bool            bOk = sal_False;

    if( fDen != 0. )
    {
        const double    fCx = maStart.X() - rLine.maStart.X();
        const double    fCy = maStart.Y() - rLine.maStart.Y();
        const double    fA = fBy * fCx - fBx * fCy;
        const sal_Bool      bGreater = ( fDen > 0. );

        bOk = sal_True;

        if ( bGreater )
        {
            if ( ( fA < 0. ) || ( fA > fDen ) )
                bOk = sal_False;
        }
        else if ( ( fA > 0. ) || ( fA < fDen ) )
            bOk = sal_False;

        if ( bOk )
        {
            const double fB = fAx * fCy - fAy * fCx;

            if ( bGreater )
            {
                if ( ( fB < 0. ) || ( fB > fDen ) )
                    bOk = sal_False;
            }
            else if ( ( fB > 0. ) || ( fB < fDen ) )
                bOk = sal_False;

            if( bOk )
            {
                const double fAlpha = fA / fDen;

                rIntersectionX = ( maStart.X() + fAlpha * fAx );
                rIntersectionY = ( maStart.Y() + fAlpha * fAy );
            }
        }
    }

    return bOk;
}

// ------------------------------------------------------------------------

sal_Bool Line::Intersection( const Rectangle& rRect, Line& rIntersection ) const
{
    const sal_Bool  bStartInside = rRect.IsInside( maStart );
    const sal_Bool  bEndInside = rRect.IsInside( maEnd );
    sal_Bool        bRet = sal_True;

    if( bStartInside && bEndInside )
    {
        // line completely inside rect
        rIntersection.maStart = maStart;
        rIntersection.maEnd = maEnd;
    }
    else
    {
        // calculate intersections
        const Point aTL( rRect.TopLeft() ), aTR( rRect.TopRight() );
        const Point aBR( rRect.BottomRight() ), aBL( rRect.BottomLeft() );
        Point       aIntersect1, aIntersect2;
        Point*      pCurIntersection = &aIntersect1;

        if( Intersection( Line( aTL, aTR ), *pCurIntersection ) )
            pCurIntersection = &aIntersect2;

        if( Intersection( Line( aTR, aBR ), *pCurIntersection ) )
            pCurIntersection = ( pCurIntersection == &aIntersect1 ) ? &aIntersect2 : NULL;

        if( pCurIntersection && Intersection( Line( aBR, aBL ), *pCurIntersection ) )
            pCurIntersection = ( pCurIntersection == &aIntersect1 ) ? &aIntersect2 : NULL;

        if( pCurIntersection && Intersection( Line( aBL, aTL ), *pCurIntersection ) )
            pCurIntersection = ( pCurIntersection == &aIntersect1 ) ? &aIntersect2 : NULL;

        if( !pCurIntersection )
        {
            // two intersections
            rIntersection.maStart = aIntersect1;
            rIntersection.maEnd = aIntersect2;
        }
        else if( pCurIntersection == &aIntersect2 )
        {
            // one intersection
            rIntersection.maStart = aIntersect1;

            if( ( maStart != aIntersect1 ) && bStartInside )
                rIntersection.maEnd = maStart;
            else if( ( maEnd != aIntersect1 ) && bEndInside )
                rIntersection.maEnd = maEnd;
            else
                rIntersection.maEnd = rIntersection.maStart;
        }
        else
            bRet = sal_False;
    }

    return bRet;
}

// ------------------------------------------------------------------------

Point Line::NearestPoint( const Point& rPoint ) const
{
    Point aRetPt;

    if ( maStart != maEnd )
    {
        const double    fDistX = maEnd.X() - maStart.X();
        const double    fDistY = maStart.Y() - maEnd.Y();
        const double    fTau = ( ( maStart.Y() - rPoint.Y() ) * fDistY -
                                 ( maStart.X() - rPoint.X() ) * fDistX ) /
                               ( fDistX * fDistX + fDistY * fDistY );

        if( fTau < 0.0 )
            aRetPt = maStart;
        else if( fTau <= 1.0 )
        {
            aRetPt.X() = FRound( maStart.X() + fTau * fDistX );
            aRetPt.Y() = FRound( maStart.Y() - fTau * fDistY );
        }
        else
            aRetPt = maEnd;
    }
    else
        aRetPt = maStart;

    return aRetPt;
}

// ------------------------------------------------------------------------

double Line::GetDistance( const double& rPtX, const double& rPtY ) const
{
    double fDist;

    if( maStart != maEnd )
    {
        const double    fDistX = maEnd.X() - maStart.X();
        const double    fDistY = maEnd.Y() - maStart.Y();
        const double    fACX = maStart.X() - rPtX;
        const double    fACY = maStart.Y() - rPtY;
        const double    fL2 = fDistX * fDistX + fDistY * fDistY;
        const double    fR = ( fACY * -fDistY - fACX * fDistX ) / fL2;
        const double    fS = ( fACY * fDistX - fACX * fDistY ) / fL2;

        if( fR < 0.0 )
        {
            fDist = hypot( maStart.X() - rPtX, maStart.Y() - rPtY );

            if( fS < 0.0 )
                fDist *= -1.0;
        }
        else if( fR <= 1.0 )
            fDist = fS * sqrt( fL2 );
        else
        {
            fDist = hypot( maEnd.X() - rPtX, maEnd.Y() - rPtY );

            if( fS < 0.0 )
                fDist *= -1.0;
        }
    }
    else
        fDist = hypot( maStart.X() - rPtX, maStart.Y() - rPtY );

    return fDist;
}

// ------------------------------------------------------------------------

void Line::Enum( const Link& rEnumLink )
{
    DBG_ASSERT( rEnumLink.IsSet(), "This call doesn't make any sense with !rEnumLink.IsSet()" );

    Point   aEnum;
    long    nX;
    long    nY;

    if( maStart.X() == maEnd.X() )
    {
        const long nEndY = maEnd.Y();

        nX = maStart.X();
        nY = maStart.Y();

        if( nEndY > nY )
        {
            while( nY <= nEndY )
            {
                aEnum.X() = nX;
                aEnum.Y() = nY++;
                rEnumLink.Call( &aEnum );
            }
        }
        else
        {
            while( nY >= nEndY )
            {
                aEnum.X() = nX;
                aEnum.Y() = nY--;
                rEnumLink.Call( &aEnum );
            }
        }
    }
    else if( maStart.Y() == maEnd.Y() )
    {
        const long nEndX = maEnd.X();

        nX = maStart.X();
        nY = maStart.Y();

        if( nEndX > nX )
        {
            while( nX <= nEndX )
            {
                aEnum.X() = nX++;
                aEnum.Y() = nY;
                rEnumLink.Call( &aEnum );
            }
        }
        else
        {
            while( nX >= nEndX )
            {
                aEnum.X() = nX--;
                aEnum.Y() = nY;
                rEnumLink.Call( &aEnum );
            }
        }
    }
    else
    {
        const long  nDX = labs( maEnd.X() - maStart.X() );
        const long  nDY = labs( maEnd.Y() - maStart.Y() );
        const long  nStartX = maStart.X();
        const long  nStartY = maStart.Y();
        const long  nEndX = maEnd.X();
        const long  nEndY = maEnd.Y();
        const long  nXInc = ( nStartX < nEndX ) ? 1L : -1L;
        const long  nYInc = ( nStartY < nEndY ) ? 1L : -1L;

        if( nDX >= nDY )
        {
            const long  nDYX = ( nDY - nDX ) << 1;
            const long  nDY2 = nDY << 1;
            long        nD = nDY2 - nDX;

            for( nX = nStartX, nY = nStartY; nX != nEndX; nX += nXInc )
            {
                aEnum.X() = nX;
                aEnum.Y() = nY;
                rEnumLink.Call( &aEnum );

                if( nD < 0L )
                    nD += nDY2;
                else
                    nD += nDYX, nY += nYInc;
            }
        }
        else
        {
            const long  nDYX = ( nDX - nDY ) << 1;
            const long  nDY2 = nDX << 1;
            long        nD = nDY2 - nDY;

            for( nX = nStartX, nY = nStartY; nY != nEndY; nY += nYInc )
            {
                aEnum.X() = nX;
                aEnum.Y() = nY;
                rEnumLink.Call( &aEnum );

                if( nD < 0L )
                    nD += nDY2;
                else
                    nD += nDYX, nX += nXInc;
            }
        }

        // last point
        aEnum.X() = nEndX;
        aEnum.Y() = nEndY;
        rEnumLink.Call( &aEnum );
    }
}
