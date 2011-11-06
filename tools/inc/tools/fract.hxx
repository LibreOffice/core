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


#ifndef _FRACT_HXX
#define _FRACT_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

class SvStream;
// ------------
// - Fraction -
// ------------

class TOOLS_DLLPUBLIC Fraction
{
private:
    long            nNumerator;
    long            nDenominator;

public:
                    Fraction() { nNumerator = 0; nDenominator = 1; }
                    Fraction( const Fraction & rFrac );
                    Fraction( long nNum, long nDen=1 );
                    Fraction( long nN1, long nN2, long nD1, long nD2 );
                    Fraction( double dVal );

    sal_Bool            IsValid() const;

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
#ifdef __BORLANDC__
    friend          Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
    friend          Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
    friend          Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
    friend          Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );

    friend          sal_Bool operator==( const Fraction& rVal1, const Fraction& rVal2 );
    friend          sal_Bool operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    friend          sal_Bool operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    friend          sal_Bool operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    friend          sal_Bool operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    friend          sal_Bool operator>=( const Fraction& rVal1, const Fraction& rVal2 );
#else
    friend inline   Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend          sal_Bool operator==( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   sal_Bool operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          sal_Bool operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          sal_Bool operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   sal_Bool operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   sal_Bool operator>=( const Fraction& rVal1, const Fraction& rVal2 );
#endif
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

inline sal_Bool Fraction::IsValid() const
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

inline sal_Bool operator !=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 == rVal2);
}

inline sal_Bool operator <=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 > rVal2);
}

inline sal_Bool operator >=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 < rVal2);
}

#endif // _FRACT_HXX
