/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fract.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:11:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FRACT_HXX
#define _FRACT_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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

    BOOL            IsValid() const;

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

    friend          BOOL operator==( const Fraction& rVal1, const Fraction& rVal2 );
    friend          BOOL operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    friend          BOOL operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    friend          BOOL operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    friend          BOOL operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    friend          BOOL operator>=( const Fraction& rVal1, const Fraction& rVal2 );
#else
    friend inline   Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend          BOOL operator==( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   BOOL operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          BOOL operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend          BOOL operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   BOOL operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    friend inline   BOOL operator>=( const Fraction& rVal1, const Fraction& rVal2 );
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

inline BOOL Fraction::IsValid() const
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

inline BOOL operator !=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 == rVal2);
}

inline BOOL operator <=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 > rVal2);
}

inline BOOL operator >=( const Fraction& rVal1, const Fraction& rVal2 )
{
    return !(rVal1 < rVal2);
}

#endif // _FRACT_HXX
