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
#include "precompiled_svtools.hxx"

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

sal_uInt16 ImpSqrt( sal_uLong nRadi )
{
    register sal_uLong  inf = 1;
    register sal_uLong  sup = nRadi;
    register sal_uLong sqr;

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

    return sal::static_int_cast< sal_uInt16 >(sqr);
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

FixCpx ImpExPI( sal_uInt16 nPhi )
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

static sal_uInt16 ImpATanx2( const Fix& rX, const Fix& rY )
{
    sal_uInt16      phi0 = 0;           // result angel higher part
    sal_uInt16      phi = 0;            // dito lower part
    long        x = rX.x;
    long        y = rY.x;
    long        z;
    const char  Sft=14-FIX_POST;
    short       i;
    FixCpx      aTry;
    FixCpx      aInc;
    FixCpx      aIter(1L);
    sal_Bool        Small = sal_False;

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
                    Small=sal_True;
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

sal_uInt16 ImpATan2( const short x, const short y )
{
    Fix rRad = ImpSqrt(sal_uLong(long(x)*x+long(y)*y));

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

void ImpCartToPolar( const short x, const short y, Fix& rRad, sal_uInt16& rPhi )
{
    rRad = Fix( ImpSqrt( sal_uLong( long(x)*x+long(y)*y ) ) );

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

void ImpPolarToCart( const Fix& rR, const sal_uInt16 Phi, short& rX, short& rY )
{
    FixCpx fc = ImpExPI( Phi );  // calculate sin() & cos()
    fc.GetReal().MultBig( rR );
    rX = sal::static_int_cast< short >(long( fc.GetReal() ));
    fc.GetImag().MultBig( rR );
    rY = sal::static_int_cast< short >(long( fc.GetImag() ));
}

