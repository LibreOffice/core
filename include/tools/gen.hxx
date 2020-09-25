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

#include <limits.h>
#include <algorithm>
#include <ostream>
#include <config_options.h>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>

class SvStream;
namespace rtl
{
    class OString;
}

enum TriState { TRISTATE_FALSE, TRISTATE_TRUE, TRISTATE_INDET };

// Pair

class SAL_WARN_UNUSED Pair
{
public:
                        Pair() : nA(0), nB(0) {}
                        Pair( long _nA, long _nB ) : nA(_nA), nB(_nB) {}

    long                A() const { return nA; }
    long                B() const { return nB; }

    long&               A() { return nA; }
    long&               B() { return nB; }

    TOOLS_DLLPUBLIC rtl::OString        toString() const;

protected:
    long                nA;
    long                nB;
};

namespace tools::detail {

// Used to implement operator == for subclasses of Pair:
inline bool equal(Pair const & p1, Pair const & p2)
{
    return p1.A() == p2.A() && p1.B() == p2.B();
}

}

// Point

class Size;
class SAL_WARN_UNUSED UNLESS_MERGELIBS(SAL_DLLPUBLIC_EXPORT) Point final : protected Pair
{
public:
                        Point() {}
                        Point( long nX, long nY ) : Pair( nX, nY ) {}
                        Point(basegfx::B2DPoint const & rPoint) : Pair(rPoint.getX(), rPoint.getY()) {}

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

    operator basegfx::B2DPoint() const { return basegfx::B2DPoint(nA, nB); }

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

    bool            IsEmpty() const { return nA <= 0 || nB <= 0; }

    void extendBy(long x, long y)
    {
        nA += x;
        nB += y;
    }

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
// Rectangle

#define RECT_MAX    LONG_MAX
#define RECT_MIN    LONG_MIN

/// Note: this class is a true marvel of engineering: because the author
/// could not decide whether it's better to have a closed or half-open
/// interval, they just implemented *both* in the same class!
///
/// If you have the misfortune of having to use this class, don't immediately
/// despair but first take note that the uppercase GetWidth() / GetHeight()
/// etc. methods interpret the interval as closed, while the lowercase
/// getWidth() / getHeight() etc. methods interpret the interval as half-open.
/// Ok, now is the time for despair.
namespace tools
{
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Rectangle final
{
    static constexpr short RECT_EMPTY = -32767;
public:
                        Rectangle();
                        Rectangle( const Point& rLT, const Point& rRB );
                        Rectangle( long nLeft, long nTop,
                                   long nRight, long nBottom );
    /// Constructs an empty Rectangle, with top/left at the specified params
                        Rectangle( long nLeft, long nTop );
                        Rectangle( const Point& rLT, const Size& rSize );
                        Rectangle( basegfx::B2DRectangle const & );

    static Rectangle    Justify( const Point& rLT, const Point& rRB );

    long                Left() const    { return nLeft;   }
    long                Right() const;
    long                Top() const     { return nTop;    }
    long                Bottom() const;

    void                SetLeft(long v)    { nLeft = v;   }
    void                SetRight(long v)   { nRight = v;  }
    void                SetTop(long v)     { nTop = v;    }
    void                SetBottom(long v)  { nBottom = v; }

    inline Point        TopLeft() const;
    inline Point        TopRight() const;
    inline Point        TopCenter() const;
    inline Point        BottomLeft() const;
    inline Point        BottomRight() const;
    inline Point        BottomCenter() const;
    inline Point        LeftCenter() const;
    inline Point        RightCenter() const;
    inline Point        Center() const;

    /// Move the top and left edges by a delta, preserving width and height
    inline void         Move( long nHorzMoveDelta, long nVertMoveDelta );
    void                Move( Size const & s ) { Move(s.Width(), s.Height()); }
    long                AdjustLeft( long nHorzMoveDelta ) { nLeft += nHorzMoveDelta; return nLeft; }
    long                AdjustRight( long nHorzMoveDelta );
    long                AdjustTop( long nVertMoveDelta ) { nTop += nVertMoveDelta; return nTop; }
    long                AdjustBottom( long nVertMoveDelta );
    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );
    inline Size         GetSize() const;

    /// Returns the difference between right and left, assuming the range is inclusive.
    inline long         GetWidth() const;
    /// Returns the difference between bottom and top, assuming the range is inclusive.
    inline long         GetHeight() const;

    tools::Rectangle&          Union( const tools::Rectangle& rRect );
    tools::Rectangle&          Intersection( const tools::Rectangle& rRect );
    inline tools::Rectangle    GetUnion( const tools::Rectangle& rRect ) const;
    inline tools::Rectangle    GetIntersection( const tools::Rectangle& rRect ) const;

    void                Justify();

    bool                IsInside( const Point& rPOINT ) const;
    bool                IsInside( const tools::Rectangle& rRect ) const;
    bool                IsOver( const tools::Rectangle& rRect ) const;

    void                SetEmpty() { nRight = nBottom = RECT_EMPTY; }
    void                SetWidthEmpty() { nRight = RECT_EMPTY; }
    void                SetHeightEmpty() { nBottom = RECT_EMPTY; }
    inline bool         IsEmpty() const;
    bool                IsWidthEmpty() const { return nRight == RECT_EMPTY; }
    bool                IsHeightEmpty() const { return nBottom == RECT_EMPTY; }

    inline bool         operator == ( const tools::Rectangle& rRect ) const;
    inline bool         operator != ( const tools::Rectangle& rRect ) const;

    inline tools::Rectangle&   operator += ( const Point& rPt );
    inline tools::Rectangle&   operator -= ( const Point& rPt );

    friend inline tools::Rectangle operator + ( const tools::Rectangle& rRect, const Point& rPt );
    friend inline tools::Rectangle operator - ( const tools::Rectangle& rRect, const Point& rPt );

    // ONE
    long                getX() const { return nLeft; }
    long                getY() const { return nTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    long                getWidth() const;
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    long                getHeight() const;
    /// Set the left edge of the rectangle to x, preserving the width
    void                setX( long x );
    /// Set the top edge of the rectangle to y, preserving the height
    void                setY( long y );
    void                setWidth( long n ) { nRight = nLeft + n; }
    void                setHeight( long n ) { nBottom = nTop + n; }
    /// Returns the string representation of the rectangle, format is "x, y, width, height".
    rtl::OString        toString() const;

    /**
     * Expands the rectangle in all directions by the input value.
     */
    void expand(long nExpandBy);
    void shrink(long nShrinkBy);

    /**
     * Sanitizing variants for handling data from the outside
     */
    void                SaturatingSetSize(const Size& rSize);
    void                SaturatingSetX(long x);
    void                SaturatingSetY(long y);

    operator basegfx::B2DRectangle() const { return basegfx::B2DRectangle(nLeft, nTop, nRight, nBottom); }

private:
    long                nLeft;
    long                nTop;
    long                nRight;
    long                nBottom;
};
}

inline tools::Rectangle::Rectangle()
{
    nLeft = nTop = 0;
    nRight = nBottom = RECT_EMPTY;
}

inline tools::Rectangle::Rectangle( const Point& rLT, const Point& rRB )
{
    nLeft   = rLT.X();
    nTop    = rLT.Y();
    nRight  = rRB.X();
    nBottom = rRB.Y();
}

inline tools::Rectangle::Rectangle( long _nLeft,  long _nTop,
                             long _nRight, long _nBottom )
{
    nLeft   = _nLeft;
    nTop    = _nTop;
    nRight  = _nRight;
    nBottom = _nBottom;
}

inline tools::Rectangle::Rectangle( long _nLeft,  long _nTop )
{
    nLeft   = _nLeft;
    nTop    = _nTop;
    nRight = nBottom = RECT_EMPTY;
}

inline tools::Rectangle::Rectangle( const Point& rLT, const Size& rSize )
{
    nLeft   = rLT.X();
    nTop    = rLT.Y();
    nRight  = rSize.Width()  ? nLeft+(rSize.Width()-1) : RECT_EMPTY;
    nBottom = rSize.Height() ? nTop+(rSize.Height()-1) : RECT_EMPTY;
}

inline tools::Rectangle::Rectangle( const basegfx::B2DRectangle& rRect )
{
    nLeft   = rRect.getMinX();
    nTop    = rRect.getMinY();
    nRight  = !rRect.isEmpty() ? rRect.getMaxX() : RECT_EMPTY;
    nBottom = !rRect.isEmpty() ? rRect.getMaxY() : RECT_EMPTY;
}

inline bool tools::Rectangle::IsEmpty() const
{
    return (nRight == RECT_EMPTY) || (nBottom == RECT_EMPTY);
}

inline Point tools::Rectangle::TopLeft() const
{
    return Point( nLeft, nTop );
}

inline Point tools::Rectangle::TopRight() const
{
    return Point( (nRight == RECT_EMPTY) ? nLeft : nRight, nTop );
}

inline Point tools::Rectangle::BottomLeft() const
{
    return Point( nLeft, (nBottom == RECT_EMPTY) ? nTop : nBottom );
}

inline Point tools::Rectangle::BottomRight() const
{
    return Point( (nRight  == RECT_EMPTY) ? nLeft : nRight,
                  (nBottom == RECT_EMPTY) ? nTop  : nBottom );
}

inline Point tools::Rectangle::TopCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::min( nLeft, nRight ) + std::abs( (nRight - nLeft)/2 ),
                      std::min( nTop,  nBottom) );
}

inline Point tools::Rectangle::BottomCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::min( nLeft, nRight ) + std::abs( (nRight - nLeft)/2 ),
                      std::max( nTop,  nBottom) );
}

inline Point tools::Rectangle::LeftCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::min( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
}

inline Point tools::Rectangle::RightCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::max( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
}

inline Point tools::Rectangle::Center() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( nLeft+(nRight-nLeft)/2 , nTop+(nBottom-nTop)/2 );
}

inline void tools::Rectangle::Move( long nHorzMove, long nVertMove )
{
    nLeft += nHorzMove;
    nTop  += nVertMove;
    if ( nRight != RECT_EMPTY )
        nRight += nHorzMove;
    if ( nBottom != RECT_EMPTY )
        nBottom += nVertMove;
}

inline void tools::Rectangle::SetPos( const Point& rPoint )
{
    if ( nRight != RECT_EMPTY )
        nRight += rPoint.X() - nLeft;
    if ( nBottom != RECT_EMPTY )
        nBottom += rPoint.Y() - nTop;
    nLeft = rPoint.X();
    nTop  = rPoint.Y();
}

inline long tools::Rectangle::GetWidth() const
{
    long n;
    if ( nRight == RECT_EMPTY )
        n = 0;
    else
    {
        n = nRight - nLeft;
        if( n < 0 )
            n--;
        else
            n++;
    }

    return n;
}

inline long tools::Rectangle::GetHeight() const
{
    long n;
    if ( nBottom == RECT_EMPTY )
        n = 0;
    else
    {
        n = nBottom - nTop;
        if ( n < 0 )
            n--;
        else
            n++;
    }

    return n;
}

inline Size tools::Rectangle::GetSize() const
{
    return Size( GetWidth(), GetHeight() );
}

inline tools::Rectangle tools::Rectangle::GetUnion( const tools::Rectangle& rRect ) const
{
    tools::Rectangle aTmpRect( *this );
    return aTmpRect.Union( rRect );
}

inline tools::Rectangle tools::Rectangle::GetIntersection( const tools::Rectangle& rRect ) const
{
    tools::Rectangle aTmpRect( *this );
    return aTmpRect.Intersection( rRect );
}

inline bool tools::Rectangle::operator == ( const tools::Rectangle& rRect ) const
{
    return (nLeft   == rRect.nLeft   ) &&
           (nTop    == rRect.nTop    ) &&
           (nRight  == rRect.nRight  ) &&
           (nBottom == rRect.nBottom );
}

inline bool tools::Rectangle::operator != ( const tools::Rectangle& rRect ) const
{
    return (nLeft   != rRect.nLeft   ) ||
           (nTop    != rRect.nTop    ) ||
           (nRight  != rRect.nRight  ) ||
           (nBottom != rRect.nBottom );
}

inline tools::Rectangle& tools::Rectangle::operator +=( const Point& rPt )
{
    nLeft += rPt.X();
    nTop  += rPt.Y();
    if ( nRight != RECT_EMPTY )
        nRight += rPt.X();
    if ( nBottom != RECT_EMPTY )
        nBottom += rPt.Y();
    return *this;
}

inline tools::Rectangle& tools::Rectangle::operator -= ( const Point& rPt )
{
    nLeft -= rPt.X();
    nTop  -= rPt.Y();
    if ( nRight != RECT_EMPTY )
        nRight -= rPt.X();
    if ( nBottom != RECT_EMPTY )
        nBottom -= rPt.Y();
    return *this;
}

namespace tools
{
inline Rectangle operator + ( const Rectangle& rRect, const Point& rPt )
{
    return rRect.IsEmpty()
        ?  Rectangle( rRect.nLeft  + rPt.X(), rRect.nTop    + rPt.Y() )
        :  Rectangle( rRect.nLeft  + rPt.X(), rRect.nTop    + rPt.Y(),
                      rRect.nRight + rPt.X(), rRect.nBottom + rPt.Y() );
}

inline Rectangle operator - ( const Rectangle& rRect, const Point& rPt )
{
    return rRect.IsEmpty()
        ? Rectangle( rRect.nLeft - rPt.X(),  rRect.nTop - rPt.Y() )
        : Rectangle( rRect.nLeft - rPt.X(),  rRect.nTop - rPt.Y(),
                     rRect.nRight - rPt.X(), rRect.nBottom - rPt.Y() );
}
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const tools::Rectangle& rectangle )
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.getWidth() << 'x' << rectangle.getHeight()
                      << "@(" << rectangle.getX() << ',' << rectangle.getY() << ")";
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
