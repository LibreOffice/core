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

#ifndef INCLUDED_SVTOOLS_SOURCE_DIALOGS_MCVMATH_HXX
#define INCLUDED_SVTOOLS_SOURCE_DIALOGS_MCVMATH_HXX

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


// - Fix -


class Fix
{
private:
    friend  class FixCpx;

public:
    long            x;

public:
                    Fix() { x=0; }
    explicit        Fix( int i ) { x=(long(i)<<FIX_POST); }
    explicit        Fix( short l ) { x=(long(l)<<FIX_POST); }
    explicit        Fix( sal_uInt16 l ) { x=(long(l)<<FIX_POST); }
    explicit        Fix( long l ) { x=(l<<FIX_POST); }
                    Fix( long Z, long N ) { x=(Z<<FIX_POST)/N; }

    enum class Bits { Bits };

    Fix(long bits, Bits): x(bits) {}

    void            operator+= ( const Fix& a ) { x+=a.x; }
    void            operator-= ( const Fix& a ) { x-=a.x; }
    void            operator*= ( const Fix& a ) { x=(x*a.x+FIX_ADD)>>FIX_POST; }
    void            operator/= ( const Fix& a ) { x=(x<<FIX_POST)/a.x; }
    friend Fix      operator-  ( const Fix& a );

    friend bool     operator> ( const Fix& a, const Fix& b ) { return a.x > b.x; }
    friend bool     operator< ( const Fix& a, const Fix& b ) { return a.x < b.x; }

    operator        long() const    { return (x+FIX_ADD) >> FIX_POST; }
    operator        double() const  { return double(x)/(1<<FIX_POST); }

    friend Fix      operator+ ( const Fix& a, const Fix& b );
    friend Fix      operator- ( const Fix& a, const Fix& b );
    friend Fix      operator* ( const Fix& a, const Fix& b );
    friend Fix      operator/ ( const Fix& a, const Fix& b );

    friend FixCpx   operator-( const FixCpx& a );
};


// - FixCpx -


class FixCpx
{
public:
    Fix             r;
    Fix             i;

public:
                    FixCpx()               : r(), i() {}
           explicit FixCpx( Fix a )        : r( a ), i() {}
                    FixCpx( Fix a, Fix b ) : r( a ), i( b ) {}

    void            operator*= ( const FixCpx& ra );

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
    return Fix(a.x+b.x, Fix::Bits::Bits);
}

inline Fix operator- ( const Fix& a, const Fix& b )
{
    return Fix(a.x-b.x, Fix::Bits::Bits);
}

inline Fix operator* ( const Fix& a, const Fix& b )
{
    return Fix((a.x*b.x+FIX_ADD)>>FIX_POST, Fix::Bits::Bits);
}

inline Fix operator/ ( const Fix& a, const Fix& b )
{
    return Fix((a.x<<FIX_POST)/b.x, Fix::Bits::Bits);
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



sal_uInt16 ImpSqrt( sal_uLong nRadi );

#endif // INCLUDED_SVTOOLS_SOURCE_DIALOGS_MCVMATH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
