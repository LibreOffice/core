/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
|*    ImpSqrt()
|*
|*    Beschreibung       Wurzelfunktion fuer FixPoint-Berechnungen
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
