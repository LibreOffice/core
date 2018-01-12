/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SOURCE_CONTROL_THUMBPOS_HXX
#define INCLUDED_VCL_SOURCE_CONTROL_THUMBPOS_HXX

inline long ImplMulDiv( long nNumber, long nNumerator, long nDenominator )
{
    if (!nDenominator)
        return 0;
    double n = (static_cast<double>(nNumber) * static_cast<double>(nNumerator)) / static_cast<double>(nDenominator);
    return static_cast<long>(n);
}

#endif // INCLUDED_VCL_SOURCE_CONTROL_THUMBPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
