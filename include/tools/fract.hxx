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
#ifndef INCLUDED_TOOLS_FRACT_HXX
#define INCLUDED_TOOLS_FRACT_HXX

#include <boost/rational.hpp>
#include <sal/log.hxx>
#include <tools/toolsdllapi.h>

class SvStream;

// This class uses the platform defined type 'long' as valid values but do all
// calculations using sal_Int64 with checks for 'long' overflows.
class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Fraction
{
private:
    bool                        valid;
    boost::rational<sal_Int64>  value;

    bool            HasOverflowValue();

public:
                    Fraction() { valid = true; }
                    Fraction( const Fraction & rFrac );
                    Fraction( long nNum, long nDen=1 );
                    Fraction( double dVal );

    bool            IsValid() const;

    long            GetNumerator() const;
    long            GetDenominator() const;

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

    TOOLS_DLLPUBLIC friend SvStream& ReadFraction( SvStream& rIStream, Fraction& rFract );
    TOOLS_DLLPUBLIC friend SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract );
};

inline Fraction::Fraction( const Fraction& rFrac )
{
    valid = rFrac.valid;
    if ( valid )
        value.assign( rFrac.value.numerator(), rFrac.value.denominator() );
}

inline long Fraction::GetNumerator() const
{
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'GetNumerator()' on invalid fraction" );
        return 0;
    }
    return value.numerator();
}

inline long Fraction::GetDenominator() const {
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'GetDenominator()' on invalid fraction" );
        return -1;
    }
    return value.denominator();
}

inline Fraction& Fraction::operator=( const Fraction& rFrac )
{
    if ( this != &rFrac ) {
        valid = rFrac.valid;
        if ( valid )
            value.assign( rFrac.value.numerator(), rFrac.value.denominator() );
    }
    return *this;
}

inline bool Fraction::IsValid() const
{
    return valid;
}

inline Fraction::operator long() const
{
    if ( !valid ) {
        SAL_WARN( "tools.fraction", "'operator long()' on invalid fraction" );
        return 0;
    }
    return boost::rational_cast<long>(value);
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
