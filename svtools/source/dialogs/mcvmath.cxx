/*************************************************************************
 *
 *  $RCSfile: mcvmath.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
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

#include <mcvmath.hxx>

// ---------------------------------------------------------------------
// die folgenden Tabellen enthalten     sin(phi) * 2**14
// fuer phi= 360Grad*2**-32 bis 360 Grad
// def. fuer x: phi=360Grad * 2**(x-16)
//           d.h. x =  16 -> 360 Grad
//                x = -16 -> (2**-16) * 360 Grad
//         x:         -16 ... 0 ... 15
//x=    0,     1,     2,     3,     4,     5,     6,      7,
//      8,     9,    10,    11,    12,    13,    14,     15

static const short CosTab[16] =
{
    16384, 16384, 16384, 16384, 16384, 16384, 16384,  16383,
    16379, 16364, 16305, 16069, 15137, 11585,     0, -16383
};
static const short SinTab[16]=
{
        2,     3,      6,    13,    25,     50,   101,  201,
      402,   804,   1606,  3196,  6270,  11585, 16384,    0
};

/**************************************************************************
|*
|*    ImpMultBig2()
|*
|*    Beschreibung       Multiplikation fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

//  first parameter should be the bigger one

Fix ImpMultBig2( const Fix& a, const Fix& b )
{
    Fix f;
    f.x = (((b.x+FIX_A2)>>FIX_P2)*a.x+FIX_A3)>>FIX_P3;
    return f;
}

/**************************************************************************
|*
|*    ImpMultBig2()
|*
|*    Beschreibung       Multiplikation fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

//  first parameter should be the bigger one

FixCpx ImpMultBig2( const FixCpx& ra, const FixCpx& rb )
{
    Fix rr = ImpMultBig2(ra.r,rb.r)-ImpMultBig2(ra.i,rb.i);
    Fix ii = ImpMultBig2(ra.r,rb.i)+ImpMultBig2(ra.i,rb.r);
    return FixCpx( rr,ii );
}

/**************************************************************************
|*
|*    ImpSqrt()
|*
|*    Beschreibung       Wurzelfunktion fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

USHORT ImpSqrt( ULONG nRadi )
{
    register ULONG  inf = 1;
    register ULONG  sup = nRadi;
    register USHORT sqr;

    if ( !nRadi )
        return 0;

    while ( (inf<<1) <= sup )
    {
        sup >>= 1;
        inf <<= 1;
    }
    sqr = (sup+inf) >> 1;               // Anfangswert der Iteration

    sqr = (nRadi/sqr + sqr) >> 1;       // 2 Newton-Iterationen reichen fuer
    sqr = (nRadi/sqr + sqr) >> 1;       // +- 1 Digit

    return sqr;
}

/**************************************************************************
|*
|*    ImpExPI()
|*
|*    Beschreibung       EXPI-Funktion fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

// e**(i*nPhi), Einheit nPhi: 2**16 == 360 Grad

FixCpx ImpExPI( USHORT nPhi )
{
    short i;
    FixCpx aIter(1L);                   // e**(0*i)
    FixCpx Mul;
    const char Sft=14-FIX_POST;

    for ( i = 15; i >= 0; i-- )
    {
        if ( (1L<<i) & nPhi )
        {
            Mul.r.x = CosTab[i]>>Sft;   // e**(i(phi1+phi2)) =
            Mul.i.x = SinTab[i]>>Sft;   // e**(i*phi1)) * e**(i*phi2))
            aIter  *= Mul;
        }
    }

    return aIter;
}

/**************************************************************************
|*
|*    ImpATanx2()
|*
|*    Beschreibung       ATANX2-Funktion fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

// use for x*x+y*y==1 only

static USHORT ImpATanx2( const Fix& rX, const Fix& rY )
{
    USHORT      phi0 = 0;           // result angel higher part
    USHORT      phi = 0;            // dito lower part
    long        x = rX.x;
    long        y = rY.x;
    long        z;
    const char  Sft=14-FIX_POST;
    short       i;
    FixCpx      aTry;
    FixCpx      aInc;
    FixCpx      aIter(1L);
    BOOL        Small = FALSE;

    if ( (x==0) && (y==0) )
        return 0;

    if ( y < 0)
    {
        // reduce 3. to 1. quadrant (0..90 Degree)
        phi0 += 180L * 65536L / 360L;
        // turn 180 degree
        y    *= -1;
        x    *= -1;
    }

    if ( x < 0)
    {
        // 2. to 1. q.
        phi0 += 90L * 65536L / 360L;
        // turn 90 degree clockwise
        z = y;
        y = -x;
        x = z;
    }

    for ( i = 13; i >= 0; i-- )
    {
        aInc.r.x = CosTab[i]>>Sft; // e**(i(phi1+phi2)) =
        aInc.i.x = SinTab[i]>>Sft; // e**(i*phi1)) * e**(i*phi2))
        aTry     = aIter*aInc;

        if ( Small )
        {
            // is try ok
           if ( aTry.r.x >= x )
           {
                aIter =  aTry;
                phi   += (1<<i);
            }
        }
        else
        {
            // is try ok
            if ( aTry.i.x <= y )
            {
                aIter = aTry;
                phi  += (1<<i);

                if ( i > 11 )
                    Small=TRUE;
            }
        }
    }

    return phi0+phi;
}

/**************************************************************************
|*
|*    ImpATan2()
|*
|*    Beschreibung       ATAN-Funktion fuer FixPoint-Berechnungen
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

USHORT ImpATan2( const short x, const short y )
{
    Fix rRad = ImpSqrt(ULONG(long(x)*x+long(y)*y));

    if ( !rRad.x )
        return 0;
    Fix fx = x;
    fx.DivBig( rRad );            // Normiere auf Einheitskreis
    Fix fy = y;
    fy.DivBig( rRad );

    return ImpATanx2( fx, fy );
}

/**************************************************************************
|*
|*    ImpCartToPolar()
|*
|*    Beschreibung       Koordinaaten-Wandlung
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

void ImpCartToPolar( const short x, const short y, Fix& rRad, USHORT& rPhi )
{
    rRad = Fix( ImpSqrt( ULONG( long(x)*x+long(y)*y ) ) );

    if ( !rRad.x )
        rPhi=0;
    else
    {
        // Normiere auf Einheitskreis
        Fix fx = x;
        fx.DivBig(rRad);
        Fix fy = y;
        fy.DivBig(rRad);
        rPhi = ImpATanx2(fx, fy);
    }
}

/**************************************************************************
|*
|*    ImpPolarToCart()
|*
|*    Beschreibung       Koordinaaten-Wandlung
|*    Ersterstellung     SH 01.07.93
|*    Letzte Aenderung   SH 01.07.93
|*
**************************************************************************/

void ImpPolarToCart( const Fix& rR, const USHORT Phi, short& rX, short& rY )
{
    FixCpx fc = ImpExPI( Phi );  // calculate sin() & cos()
    fc.GetReal().MultBig( rR );
    rX = long( fc.GetReal() );
    fc.GetImag().MultBig( rR );
    rY = long( fc.GetImag() );
}

