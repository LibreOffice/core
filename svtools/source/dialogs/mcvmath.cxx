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


#include "mcvmath.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
