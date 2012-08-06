/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _MCVMATH_HXX
#define _MCVMATH_HXX

#include <tools/solar.h>

class FixCpx;

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
    friend  class FixCpx;

public:
    long            x;

public:
                    Fix() { x=0; }
                    Fix( int i ) { x=(long(i)<<FIX_POST); }
                    Fix( short l ) { x=(long(l)<<FIX_POST); }
                    Fix( sal_uInt16 l ) { x=(long(l)<<FIX_POST); }
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

    friend sal_Bool     operator> ( const Fix& a, const Fix& b ) { return a.x > b.x; }
    friend sal_Bool     operator< ( const Fix& a, const Fix& b ) { return a.x < b.x; }

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

sal_uInt16 ImpSqrt( sal_uLong nRadi );
FixCpx ImpExPI( sal_uInt16 nPhi );

#endif // _MCVMATH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
