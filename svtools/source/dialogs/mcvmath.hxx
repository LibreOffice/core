/*************************************************************************
 *
 *  $RCSfile: mcvmath.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _MCVMATH_HXX
#define _MCVMATH_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class FixCpx;
class ColWheel;

// No of fractal bits
// allowed range 0..14, must be even
#define FIX_POST 14

// scale for ...Big() -Functions
#if (FIX_POST>=4)
#define FIX_P2  4
#define FIX_P3  (FIX_POST-FIX_P2)
#else
#define FIX_P2  0
#define FIX_P3  FIX_POST
#endif

#if (FIX_POST>=1)
#define FIX_ADD (1<<(FIX_POST-1))
#else
#define FIX_ADD 0
#endif

#if (FIX_P2>=1)
#define FIX_A2 (1<<(FIX_P2-1))
#else
#define FIX_A2 0
#endif

#if (FIX_P3>=1)
#define FIX_A3 (1<<(FIX_P3-1))
#else
#define FIX_A3 0
#endif

// -------
// - Fix -
// -------

class Fix
{
private:
    friend  FixCpx;
    friend  ColWheel;

//  friend  Fix ImpMultBig2( const Fix& a, const Fix& b );

public:
    long            x;

public:
                    Fix() { x=0; }
                    Fix( int i ) { x=(long(i)<<FIX_POST); }
                    Fix( short l ) { x=(long(l)<<FIX_POST); }
                    Fix( USHORT l ) { x=(long(l)<<FIX_POST); }
                    Fix( long l ) { x=(l<<FIX_POST); }
                    Fix( long Z, long N ) { x=(Z<<FIX_POST)/N; }

    void            SetInternVal( long nVal ) { x=nVal; }
    long            GetInternVal() const { return x; }

    void            operator+= ( const Fix& a ) { x+=a.x; }
    void            operator-= ( const Fix& a ) { x-=a.x; }
    void            operator*= ( const Fix& a ) { x=(x*a.x+FIX_ADD)>>FIX_POST; }
    void            operator/= ( const Fix& a ) { x=(x<<FIX_POST)/a.x; }
    friend Fix      operator-  ( const Fix& a );

    void            MultBig( const Fix& a )
                        { x=((((a.x+FIX_A2)>>FIX_P2)*x+FIX_A3)>>FIX_P3); }
    void            DivBig( const Fix& a )
                        { x=((x<<FIX_P3)/a.x)<<FIX_P2; }

    friend BOOL     operator> ( const Fix& a, const Fix& b ) { return a.x > b.x; }
    friend BOOL     operator< ( const Fix& a, const Fix& b ) { return a.x < b.x; }

    operator        long() const    { return (x+FIX_ADD) >> FIX_POST; }
    operator        double() const  { return double(x)/(1<<FIX_POST); }

    friend Fix      operator+ ( const Fix& a, const Fix& b );
    friend Fix      operator- ( const Fix& a, const Fix& b );
    friend Fix      operator* ( const Fix& a, const Fix& b );
    friend Fix      operator/ ( const Fix& a, const Fix& b );

    friend FixCpx   operator-( const FixCpx& a );
};

// ----------
// - FixCpx -
// ----------

class FixCpx
{
//  friend  FixCpx ImpMultBig2( const FixCpx& ra, const FixCpx& rb );

public:
    Fix             r;
    Fix             i;

public:
                    FixCpx()               : r(), i() {}
                    FixCpx( Fix a )        : r( a ), i() {}
                    FixCpx( Fix a, Fix b ) : r( a ), i( b ) {}

    Fix&            GetReal() { return r; }
    Fix&            GetImag() { return i; }

    void            operator*= ( const FixCpx& ra );
    void            MultBig( const FixCpx& ra, const FixCpx& rb );

    friend FixCpx   operator+ ( const FixCpx& a, const FixCpx& b );
    friend FixCpx   operator- ( const FixCpx& a, const FixCpx& b );
    friend FixCpx   operator* ( const FixCpx& a, const FixCpx& b );
    friend FixCpx   operator/ ( const FixCpx& a, const FixCpx& b );
    friend FixCpx   operator- ( const FixCpx& a );
};

inline Fix operator- ( const Fix& a )
{
    Fix f;
    f.x = -a.x;
    return f;
}

inline Fix operator+ ( const Fix& a, const Fix& b )
{
    long l = a.x+b.x;
    return *((Fix*)&l);
}

inline Fix operator- ( const Fix& a, const Fix& b )
{
    long l = a.x-b.x;
    return *((Fix*)&l);
}

inline Fix operator* ( const Fix& a, const Fix& b )
{
    long l=(a.x*b.x+FIX_ADD)>>FIX_POST;
    return *((Fix*)&l);
}

inline Fix operator/ ( const Fix& a, const Fix& b )
{
    long l=(a.x<<FIX_POST)/b.x;
    return *((Fix*)&l);
}

inline FixCpx operator- ( const FixCpx& a )
{
    FixCpx fc;

    fc.r.x = -a.r.x;
    fc.i.x = -a.i.x;
    return fc;
}

inline FixCpx operator+ ( const FixCpx& a, const FixCpx& b )
{
    return FixCpx( a.r+b.r, a.i+b.i );
}

inline FixCpx operator- ( const FixCpx& a, const FixCpx& b )
{
    return FixCpx( a.r-b.r, a.i-b.i );
}

inline void FixCpx::operator*= ( const FixCpx& ra )
{
    Fix rr = ra.r*r-ra.i*i;
    i = ra.r*i+ra.i*r;
    r = rr;
}

inline FixCpx operator* ( const FixCpx& a, const FixCpx& b )
{
    return FixCpx( a.r*b.r-a.i*b.i, a.r*b.i+a.i*b.r );
}

inline FixCpx operator/ ( const FixCpx& a, const FixCpx& b )
{
    return FixCpx( (a.r*b.r+a.i*b.i)/(b.r*b.r+b.i*b.i),
                   (b.r*a.r-a.r*b.i)/(b.r*b.r+b.i*b.i) );
}

// -----------------------------------------------------------------------

Fix ImpMultBig2( const Fix& a, const Fix& b );
FixCpx ImpMultBig2( const FixCpx& ra, const FixCpx& rb );

void ImpCartToPolar( const short x, const short y, Fix& rRad, USHORT& rPhi );
void ImpPolarToCart( const Fix& rR, const USHORT Phi, short& rX, short& rY );

USHORT ImpSqrt( ULONG nRadi );
USHORT ImpATan2( const short x, const short y );
FixCpx ImpExPI( USHORT nPhi );

#endif // _MCVMATH_HXX
