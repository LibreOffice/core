/*************************************************************************
 *
 *  $RCSfile: salmathutils.cxx,v $
 *
 *  $Revision: 1.2 $
 *  last change: $Author: bmahbod $ $Date: 2001-03-12 23:15:32 $
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

#define _SV_SALMATHUTILS_CXX

#ifndef _SV_SALMATHUTILS_HXX
    #include <salmathutils.hxx>
#endif

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

