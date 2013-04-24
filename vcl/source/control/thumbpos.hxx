/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _VCL_THUMBPOS_HXX
#define _VCL_THUMBPOS_HXX

inline long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    if (!nDenominator)
        return 0;
    double n = ((double)nNumber * (double)nNumerator) / (double)nDenominator;
    return (long)n;
}

#endif // _VCL_THUMBPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
