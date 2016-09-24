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
#include <tools/helpers.hxx>

#include <cstdlib>
#include <math.h>

namespace tools
{

double Line::GetLength() const
{
    return hypot( maStart.X() - maEnd.X(), maStart.Y() - maEnd.Y() );
}

bool Line::Intersection( const Line& rLine, Point& rIntersection ) const
{
    double  fX, fY;
    bool    bRet;

    if( Intersection( rLine, fX, fY ) )
    {
        rIntersection.X() = FRound( fX );
        rIntersection.Y() = FRound( fY );
        bRet = true;
    }
    else
        bRet = false;

    return bRet;
}

bool Line::Intersection( const tools::Line& rLine, double& rIntersectionX, double& rIntersectionY ) const
{
    const double    fAx = maEnd.X() - maStart.X();
    const double    fAy = maEnd.Y() - maStart.Y();
    const double    fBx = rLine.maStart.X() - rLine.maEnd.X();
    const double    fBy = rLine.maStart.Y() - rLine.maEnd.Y();
    const double    fDen = fAy * fBx - fAx * fBy;
    bool            bOk = false;

    if( fDen != 0. )
    {
        const double    fCx = maStart.X() - rLine.maStart.X();
        const double    fCy = maStart.Y() - rLine.maStart.Y();
        const double    fA = fBy * fCx - fBx * fCy;
        const bool      bGreater = ( fDen > 0. );

        bOk = true;

        if ( bGreater )
        {
            if ( ( fA < 0. ) || ( fA > fDen ) )
                bOk = false;
        }
        else if ( ( fA > 0. ) || ( fA < fDen ) )
            bOk = false;

        if ( bOk )
        {
            const double fB = fAx * fCy - fAy * fCx;

            if ( bGreater )
            {
                if ( ( fB < 0. ) || ( fB > fDen ) )
                    bOk = false;
            }
            else if ( ( fB > 0. ) || ( fB < fDen ) )
                bOk = false;

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

} //namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
