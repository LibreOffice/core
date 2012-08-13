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
#ifndef _FRACT_HXX
#define _FRACT_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

class SvStream;

class TOOLS_DLLPUBLIC Fraction
{
private:
    long            nNumerator;
    long            nDenominator;

public:
                    Fraction() { nNumerator = 0; nDenominator = 1; }
                    Fraction( const Fraction & rFrac );
                    Fraction( long nNum, long nDen=1 );
                    Fraction( double dVal );

    bool            IsValid() const;

    long            GetNumerator() const { return nNumerator; }
    long            GetDenominator() const { return nDenominator; }

    operator        long() const;
    operator        double() const;

    Fraction&       operator=( const Fraction& rfrFrac );

    Fraction&       operator+=( const Fraction& rfrFrac );
    Fraction&       operator-=( const Fraction& rfrFrac );
    Fraction&       operator*=( const Fraction& rfrFrac );
    Fraction&       operator/=( const Fraction& rfrFrac );

    void            ReduceInaccurate( unsigned nSignificantBits );

    friend inline   Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend          bool operator==( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   bool operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          bool operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          bool operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   bool operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   bool operator>=( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend SvStream& operator>>( SvStream& rIStream, Fraction& rFract );
    TOOLS_DLLPUBLIC friend SvStream& operator<<( SvStream& rOStream, const Fraction& rFract );
};

inline Fraction::Fraction( const Fraction& rFrac )
{
    nNumerator   = rFrac.nNumerator;
    nDenominator = rFrac.nDenominator;
}

inline Fraction& Fraction::operator=( const Fraction& rFrac )
{
    nNumerator   = rFrac.nNumerator;
    nDenominator = rFrac.nDenominator;
    return *this;
}

inline bool Fraction::IsValid() const
{
    return (nDenominator > 0);
}

inline Fraction::operator long() const
{
    if ( nDenominator > 0 )
        return (nNumerator / nDenominator);
    else
        return 0;
}

inline Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg += rVal2;
    return aErg;
}

inline Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg -= rVal2;
    return aErg;
}

inline Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg *= rVal2;
    return aErg;
}

inline Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 )
{
    Fraction aErg( rVal1 );
    aErg /= rVal2;
    return aErg;
}

inline bool operator !=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 == rVal2);
}

inline bool operator <=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 > rVal2);
}

inline bool operator >=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 < rVal2);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
