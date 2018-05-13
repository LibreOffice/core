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
#ifndef INCLUDED_TOOLS_GEN_HXX
#define INCLUDED_TOOLS_GEN_HXX

#include <tools/toolsdllapi.h>
#include <tools/Pair.hxx>

#include <limits.h>
#include <algorithm>
#include <ostream>
#include <cstdlib>

class SvStream;
namespace rtl
{
    class OString;
}

enum TriState { TRISTATE_FALSE, TRISTATE_TRUE, TRISTATE_INDET };

// Point

class Size;
class SAL_WARN_UNUSED SAL_DLLPUBLIC_EXPORT Point final : protected Pair
{
public:
                        Point() {}
                        Point( long nX, long nY ) : Pair( nX, nY ) {}

    long                X() const { return nA; }
    long                Y() const { return nB; }

    void                Move( long nHorzMove, long nVertMove );
    void                Move( Size const & s );
    long                AdjustX( long nHorzMove ) { nA += nHorzMove; return nA; }
    long                AdjustY( long nVertMove ) { nB += nVertMove; return nB; }

    void                RotateAround( long& rX, long& rY, short nOrientation ) const;
    void                RotateAround( Point&, short nOrientation ) const;

    Point&              operator += ( const Point& rPoint );
    Point&              operator -= ( const Point& rPoint );
    Point&              operator *= ( const long nVal );
    Point&              operator /= ( const long nVal );

    friend inline Point operator+( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator-( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator*( const Point &rVal1, const long nVal2 );
    friend inline Point operator/( const Point &rVal1, const long nVal2 );

    long                getX() const { return X(); }
    long                getY() const { return Y(); }
    void                setX(long nX)  { nA = nX; }
    void                setY(long nY)  { nB = nY; }

    Pair const &        toPair() const { return *this; }
    Pair &              toPair() { return *this; }

    using Pair::toString;
};

inline void Point::Move( long nHorzMove, long nVertMove )
{
    nA += nHorzMove;
    nB += nVertMove;
}

inline Point& Point::operator += ( const Point& rPoint )
{
    nA += rPoint.nA;
    nB += rPoint.nB;
    return *this;
}

inline Point& Point::operator -= ( const Point& rPoint )
{
    nA -= rPoint.nA;
    nB -= rPoint.nB;
    return *this;
}

inline Point& Point::operator *= ( const long nVal )
{
    nA *= nVal;
    nB *= nVal;
    return *this;
}

inline Point& Point::operator /= ( const long nVal )
{
    nA /= nVal;
    nB /= nVal;
    return *this;
}

inline Point operator+( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.nA+rVal2.nA, rVal1.nB+rVal2.nB );
}

inline Point operator-( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.nA-rVal2.nA, rVal1.nB-rVal2.nB );
}

inline Point operator*( const Point &rVal1, const long nVal2 )
{
    return Point( rVal1.nA*nVal2, rVal1.nB*nVal2 );
}

inline Point operator/( const Point &rVal1, const long nVal2 )
{
    return Point( rVal1.nA/nVal2, rVal1.nB/nVal2 );
}

inline bool operator ==(Point const & p1, Point const & p2)
{
    return tools::detail::equal(p1.toPair(), p2.toPair());
}

inline bool operator !=(Point const & p1, Point const & p2)
{
    return !(p1 == p2);
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Point& point )
{
    return stream << point.X() << ',' << point.Y();
}

// Size

class SAL_WARN_UNUSED Size final : protected Pair
{
public:
                    Size() {}
                    Size( long nWidth, long nHeight ) : Pair( nWidth, nHeight ) {}

    long            Width() const  { return nA; }
    long            Height() const { return nB; }

    long            AdjustWidth( long n ) { nA += n; return nA; }
    long            AdjustHeight( long n ) { nB += n; return nB; }

    long            getWidth() const { return Width(); }
    long            getHeight() const { return Height(); }
    void            setWidth(long nWidth)  { nA = nWidth; }
    void            setHeight(long nHeight)  { nB = nHeight; }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool operator ==(Size const & s1, Size const & s2)
{
    return tools::detail::equal(s1.toPair(), s2.toPair());
}

inline bool operator !=(Size const & s1, Size const & s2)
{
    return !(s1 == s2);
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Size& size )
{
    return stream << size.Width() << 'x' << size.Height();
}

inline void Point::Move( Size const & s )
{
    AdjustX(s.Width());
    AdjustY(s.Height());
}

// Range

#define RANGE_MAX   LONG_MAX

class SAL_WARN_UNUSED Range final : protected Pair
{
public:
                    Range() {}
                    Range( long nMin, long nMax ) : Pair( nMin, nMax ) {}

    long            Min() const { return nA; }
    long            Max() const { return nB; }
    long            Len() const { return nB - nA + 1; }

    long&           Min() { return nA; }
    long&           Max() { return nB; }

    bool            IsInside( long nIs ) const;

    void            Justify();

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Range::IsInside( long nIs ) const
{
    return ((nA <= nIs) && (nIs <= nB ));
}

inline void Range::Justify()
{
    if ( nA > nB )
    {
        long nHelp = nA;
        nA = nB;
        nB = nHelp;
    }
}

inline bool operator ==(Range const & r1, Range const & r2)
{
    return tools::detail::equal(r1.toPair(), r2.toPair());
}

inline bool operator !=(Range const & r1, Range const & r2)
{
    return !(r1 == r2);
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Range& range )
{
    return stream << range.Min() << '-' << range.Max();
}

// Selection

#define SELECTION_MIN   LONG_MIN
#define SELECTION_MAX   LONG_MAX

class SAL_WARN_UNUSED Selection final : protected Pair
{
public:
                    Selection() {}
                    Selection( long nPos ) : Pair( nPos, nPos ) {}
                    Selection( long nMin, long nMax ) : Pair( nMin, nMax ) {}

    long            Min() const { return nA; }
    long            Max() const { return nB; }
    long            Len() const { return nB - nA; }

    long&           Min() { return nA; }
    long&           Max() { return nB; }

    bool            IsInside( long nIs ) const;

    void            Justify();

    bool            operator !() const { return !Len(); }

    long            getMin() const { return Min(); }
    void            setMin(long nMin)  { Min() = nMin; }
    void            setMax(long nMax)  { Max() = nMax; }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Selection::IsInside( long nIs ) const
{
    return ((nA <= nIs) && (nIs < nB ));
}

inline void Selection::Justify()
{
    if ( nA > nB )
    {
        long nHelp = nA;
        nA = nB;
        nB = nHelp;
    }
}

inline bool operator ==(Selection const & s1, Selection const & s2)
{
    return tools::detail::equal(s1.toPair(), s2.toPair());
}

inline bool operator !=(Selection const & s1, Selection const & s2)
{
    return !(s1 == s2);
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Selection& selection )
{
    return stream << selection.Min() << '-' << selection.Max();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
