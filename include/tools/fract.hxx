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

#include <sal/types.h>
#include <tools/toolsdllapi.h>
#include <memory>
#include <type_traits>

class SvStream;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Fraction final
{
    struct Impl;

    std::unique_ptr<Impl> mpImpl;

    bool            HasOverflowValue();

public:
                    Fraction();
                    Fraction( const Fraction & rFrac );
                    Fraction( Fraction && rFrac );
    explicit        Fraction( double dVal );
                    Fraction( double nNum, double nDen );
                    Fraction( sal_Int64 nNum, sal_Int64 nDen );
                    // just to prevent ambiguity between the sal_Int64 and double constructors
                    template<typename T1, typename T2> Fraction(
                        T1 nNum, T2 nDen,
                        typename std::enable_if<std::is_integral<T1>::value && std::is_integral<T2>::value, int>::type = 0)
                        : Fraction( sal_Int64(nNum), sal_Int64(nDen) ) {}
                    ~Fraction();

    bool            IsValid() const;

    sal_Int32       GetNumerator() const;
    sal_Int32       GetDenominator() const;

    explicit operator sal_Int32() const;
#if SAL_TYPES_SIZEOFLONG == 8
    explicit operator long() const { return sal_Int32(*this); }
#endif
    explicit operator double() const;

    Fraction&       operator=( const Fraction& rfrFrac );
    Fraction&       operator=( Fraction&& rfrFrac );
    Fraction&       operator=( double v ) { return operator=(Fraction(v)); }

    Fraction&       operator+=( const Fraction& rfrFrac );
    Fraction&       operator-=( const Fraction& rfrFrac );
    Fraction&       operator*=( const Fraction& rfrFrac );
    Fraction&       operator/=( const Fraction& rfrFrac );
    Fraction&       operator+=( double v ) { return operator+=(Fraction(v)); }
    Fraction&       operator-=( double v ) { return operator-=(Fraction(v)); }
    Fraction&       operator*=( double v ) { return operator*=(Fraction(v)); }
    Fraction&       operator/=( double v ) { return operator/=(Fraction(v)); }

    void            ReduceInaccurate( unsigned nSignificantBits );

    TOOLS_DLLPUBLIC friend Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend bool operator==( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend bool operator!=( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend bool operator< ( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend bool operator> ( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend bool operator<=( const Fraction& rVal1, const Fraction& rVal2 );
    TOOLS_DLLPUBLIC friend bool operator>=( const Fraction& rVal1, const Fraction& rVal2 );

    TOOLS_DLLPUBLIC friend SvStream& ReadFraction( SvStream& rIStream, Fraction const & rFract );
    TOOLS_DLLPUBLIC friend SvStream& WriteFraction( SvStream& rOStream, const Fraction& rFract );
};

TOOLS_DLLPUBLIC Fraction operator+( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC Fraction operator-( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC Fraction operator*( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC Fraction operator/( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC bool operator !=( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC bool operator <=( const Fraction& rVal1, const Fraction& rVal2 );
TOOLS_DLLPUBLIC bool operator >=( const Fraction& rVal1, const Fraction& rVal2 );

inline Fraction operator+( double v1, const Fraction& rVal2 ) { return Fraction(v1) + rVal2; }
inline Fraction operator-( double v1, const Fraction& rVal2 ) { return Fraction(v1) - rVal2; }
inline Fraction operator*( double v1, const Fraction& rVal2 ) { return Fraction(v1) * rVal2; }
inline Fraction operator/( double v1, const Fraction& rVal2 ) { return Fraction(v1) / rVal2; }

inline Fraction operator+( const Fraction& rVal1, double v2 ) { return rVal1 + Fraction(v2); }
inline Fraction operator-( const Fraction& rVal1, double v2 ) { return rVal1 - Fraction(v2); }
inline Fraction operator*( const Fraction& rVal1, double v2 ) { return rVal1 * Fraction(v2); }
inline Fraction operator/( const Fraction& rVal1, double v2 ) { return rVal1 / Fraction(v2); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
