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


#include "quartz/salmathutils.hxx"

#include <stdlib.h>

// =======================================================================

// =======================================================================

#define Swap( x, y ) { x ^= y; y ^= x; x ^= y; }

// =======================================================================

// =======================================================================

// Storage free swapping using XOR

void CSwap ( char &rX, char &rY )
{
    Swap( rX, rY );
} // CSwap

// -----------------------------------------------------------------------

// Storage free swapping using XOR

void UCSwap ( unsigned char &rX, unsigned char &rY )
{
    Swap( rX, rY );
} // UCSwap

// -----------------------------------------------------------------------

// Storage free swapping using XOR

void SSwap ( short &rX, short &rY )
{
    Swap( rX, rY );
} // SSwap

// -----------------------------------------------------------------------

// Storage free swapping using XOR

void USSwap ( unsigned short &rX, unsigned short &rY )
{
    Swap( rX, rY );
} // USSwap

// -----------------------------------------------------------------------

// Storage free swapping using XOR

void LSwap ( long &rX, long &rY )
{
    Swap( rX, rY );
} // LSwap

// -----------------------------------------------------------------------

// Storage free swapping using XOR

void ULSwap ( unsigned long &rX, unsigned long &rY )
{
    Swap( rX, rY );
} // ULSwap

// =======================================================================

// =======================================================================

// -----------------------------------------------------------------------
//
// This way of measuring distance is also called the "Manhattan distance."
// Manhattan distance takes advantage of the fact that the sum of the
// lengths of the three components of a 3D vector is a rough approxima-
// tion of the vector's length.
//
// -----------------------------------------------------------------------

unsigned long  Euclidian2Norm ( const LRectCoorVector  pVec )
{
    unsigned long  ndist = 0;

    if ( pVec )
    {
        long           nDX  = 0;
        long           nDY  = 0;
        long           nDZ  = 0;
        unsigned long  nMax = 0;
        unsigned long  nMed = 0;
        unsigned long  nMin = 0;

        // Find |x'-x|, |y'-y|, and |z'-z| from (x,y,z) and (x',y',z')

        nDX = pVec[1].x - pVec[0].x;
        nDY = pVec[1].y - pVec[0].y;
        nDZ = pVec[1].z - pVec[0].z;

        nMax = (unsigned long)abs( nDX );
        nMed = (unsigned long)abs( nDY );
        nMin = (unsigned long)abs( nDZ );

        // Sort them (3 compares, 0-3 swaps)

        if ( nMax < nMed )
        {
            Swap( nMax, nMed );
        } // if

        if ( nMax < nMin )
        {
            Swap( nMax, nMin );
        } // if

        // Approximate Euclidian distance:
        //
        //    d = max + (11/32)*med + (1/4)*min
        //
        // with +/- 8% error, where the exact formulae for d is
        //
        //  || (x',y',z') - (x,y,z) || = { |x'-x|^2 + |y'-y|^2 + |z'-z|^2 }^(1/2)

        ndist = nMax + ( nMin >> 2UL )
                     + ( ( ( nMed << 3UL ) + ( nMed << 1UL ) + nMed ) >> 5UL );
    } // if

    return ndist;
} // RGBDistance

// =======================================================================

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
