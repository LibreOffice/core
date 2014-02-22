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


#include <mcvmath.hxx>


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
|*    Description       Multiplier for FixPoint-calculations
|*
**************************************************************************/

//  first parameter should be the larger one

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
|*    Description       SquareRoot function for FixPoint-calculations
|*
**************************************************************************/

sal_uInt16 ImpSqrt( sal_uLong nRadi )
{
    sal_uLong  inf = 1;
    sal_uLong  sup = nRadi;
    sal_uLong sqr;

    if ( !nRadi )
        return 0;

    while ( (inf<<1) <= sup )
    {
        sup >>= 1;
        inf <<= 1;
    }
    sqr = (sup+inf) >> 1;               // startvalue for iteration

    sqr = (nRadi/sqr + sqr) >> 1;       // 2 Newton-Iterations suffice for
    sqr = (nRadi/sqr + sqr) >> 1;       // +- 1 Digit

    return sal::static_int_cast< sal_uInt16 >(sqr);
}

/**************************************************************************
|*
|*    ImpExPI()
|*
|*    Description       EXPI funktion or FixPoint-calculations
|*
**************************************************************************/

// e**(i*nPhi), unit nPhi: 2**16 == 360 degrees

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
