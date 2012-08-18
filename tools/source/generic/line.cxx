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

#include <tools/link.hxx>
#include <tools/line.hxx>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>

#include <cstdlib>
#include <math.h>

double Line::GetLength() const
{
    return hypot( maStart.X() - maEnd.X(), maStart.Y() - maEnd.Y() );
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
