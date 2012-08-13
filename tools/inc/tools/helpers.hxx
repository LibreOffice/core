/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _TOOLS_HELPERS_HXX
#define _TOOLS_HELPERS_HXX

inline long MinMax( long nVal, long nMin, long nMax )
{
    return nVal >= nMin ? ( nVal <= nMax ? nVal : nMax ) : nMin;
}

inline long AlignedWidth4Bytes( long nWidthBits )
{
    return ( ( nWidthBits + 31 ) >> 5 ) << 2;
}

inline long FRound( double fVal )
{
    return fVal > 0.0 ? static_cast<long>( fVal + 0.5 ) : -static_cast<long>( -fVal + 0.5 );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
