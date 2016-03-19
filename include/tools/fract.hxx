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

class SvStream;

// This class uses the platform defined type 'long' as valid values but do all
// calculations using sal_Int64 with checks for 'long' overflows.
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Fraction
{
    struct Impl;

    std::unique_ptr<Impl> mpImpl;

    bool            HasOutsideOf32bitValue();

public:
                    Fraction();
                    Fraction( const Fraction & rFrac );
                    Fraction( Fraction && rFrac );
                    Fraction( sal_Int32 nNum, sal_Int32 nDen );
                    Fraction( double dVal );
                    ~Fraction();

    bool            IsOkay() const;

    sal_Int64       GetNumerator() const;
    sal_Int64       GetDenominator() const;

    operator        double() const;

    Fraction&       operator=( const Fraction& rfrFrac );
    Fraction&       operator=( Fraction&& rfrFrac );

    Fraction&       operator+=( const Fraction& rfrFrac );
    Fraction&       operator-=( const Fraction& rfrFrac );
    Fraction&       operator*=( const Fraction& rfrFrac );
    Fraction&       operator/=( const Fraction& rfrFrac );

    void            ReduceInaccurate( size_t nSignificantBits );

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
