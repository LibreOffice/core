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

#include <tools/long.hxx>
#include <tools/degree.hxx>
#include <limits.h>
#include <algorithm>
#include <ostream>
#include <config_options.h>

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
                        Pair( tools::Long _nA, tools::Long _nB ) : nA(_nA), nB(_nB) {}

    tools::Long                A() const { return nA; }
    tools::Long                B() const { return nB; }

    tools::Long&               A() { return nA; }
    tools::Long&               B() { return nB; }

    TOOLS_DLLPUBLIC rtl::OString        toString() const;

protected:
    tools::Long                nA;
    tools::Long                nB;
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
                        Point( tools::Long nX, tools::Long nY ) : Pair( nX, nY ) {}

    tools::Long                X() const { return nA; }
    tools::Long                Y() const { return nB; }

    void                Move( tools::Long nHorzMove, tools::Long nVertMove );
    void                Move( Size const & s );
    tools::Long                AdjustX( tools::Long nHorzMove ) { nA += nHorzMove; return nA; }
    tools::Long                AdjustY( tools::Long nVertMove ) { nB += nVertMove; return nB; }

    void                RotateAround( tools::Long& rX, tools::Long& rY, Degree10 nOrientation ) const;
    void                RotateAround( Point&, Degree10 nOrientation ) const;

    Point&              operator += ( const Point& rPoint );
    Point&              operator -= ( const Point& rPoint );
    Point&              operator *= ( const tools::Long nVal );
    Point&              operator /= ( const tools::Long nVal );

    friend inline Point operator+( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator-( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator*( const Point &rVal1, const tools::Long nVal2 );
    friend inline Point operator/( const Point &rVal1, const tools::Long nVal2 );

    tools::Long                getX() const { return X(); }
    tools::Long                getY() const { return Y(); }
    void                setX(tools::Long nX)  { nA = nX; }
    void                setY(tools::Long nY)  { nB = nY; }

    Pair const &        toPair() const { return *this; }
    Pair &              toPair() { return *this; }

    using Pair::toString;
};

inline void Point::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

inline Point& Point::operator *= ( const tools::Long nVal )
{
    nA *= nVal;
    nB *= nVal;
    return *this;
}

inline Point& Point::operator /= ( const tools::Long nVal )
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

inline Point operator*( const Point &rVal1, const tools::Long nVal2 )
{
    return Point( rVal1.nA*nVal2, rVal1.nB*nVal2 );
}

inline Point operator/( const Point &rVal1, const tools::Long nVal2 )
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
                    Size( tools::Long nWidth, tools::Long nHeight ) : Pair( nWidth, nHeight ) {}

    tools::Long            Width() const  { return nA; }
    tools::Long            Height() const { return nB; }

    tools::Long            AdjustWidth( tools::Long n ) { nA += n; return nA; }
    tools::Long            AdjustHeight( tools::Long n ) { nB += n; return nB; }

    tools::Long            getWidth() const { return Width(); }
    tools::Long            getHeight() const { return Height(); }
    void            setWidth(tools::Long nWidth)  { nA = nWidth; }
    void            setHeight(tools::Long nHeight)  { nB = nHeight; }

    bool            IsEmpty() const { return nA <= 0 || nB <= 0; }

    void extendBy(tools::Long x, tools::Long y)
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
                    Range( tools::Long nMin, tools::Long nMax ) : Pair( nMin, nMax ) {}

    tools::Long            Min() const { return nA; }
    tools::Long            Max() const { return nB; }
    tools::Long            Len() const { return nB - nA + 1; }

    tools::Long&           Min() { return nA; }
    tools::Long&           Max() { return nB; }

    bool            IsInside( tools::Long nIs ) const;

    void            Justify();

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Range::IsInside( tools::Long nIs ) const
{
    return ((nA <= nIs) && (nIs <= nB ));
}

inline void Range::Justify()
{
    if ( nA > nB )
    {
        tools::Long nHelp = nA;
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
                    Selection( tools::Long nPos ) : Pair( nPos, nPos ) {}
                    Selection( tools::Long nMin, tools::Long nMax ) : Pair( nMin, nMax ) {}

    tools::Long            Min() const { return nA; }
    tools::Long            Max() const { return nB; }
    tools::Long            Len() const { return nB - nA; }

    tools::Long&           Min() { return nA; }
    tools::Long&           Max() { return nB; }

    bool            IsInside( tools::Long nIs ) const;

    void            Justify();

    bool            operator !() const { return !Len(); }

    tools::Long            getMin() const { return Min(); }
    void            setMin(tools::Long nMin)  { Min() = nMin; }
    void            setMax(tools::Long nMax)  { Max() = nMax; }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Selection::IsInside( tools::Long nIs ) const
{
    return ((nA <= nIs) && (nIs < nB ));
}

inline void Selection::Justify()
{
    if ( nA > nB )
    {
        tools::Long nHelp = nA;
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
    constexpr           Rectangle( tools::Long nLeft, tools::Long nTop,
                                   tools::Long nRight, tools::Long nBottom );
    /// Constructs an empty Rectangle, with top/left at the specified params
                        Rectangle( tools::Long nLeft, tools::Long nTop );
                        Rectangle( const Point& rLT, const Size& rSize );

    static Rectangle    Justify( const Point& rLT, const Point& rRB );

    tools::Long         Left() const    { return nLeft;   }
    tools::Long         Right() const;
    tools::Long         Top() const     { return nTop;    }
    tools::Long         Bottom() const;

    void                SetLeft(tools::Long v)    { nLeft = v;   }
    void                SetRight(tools::Long v)   { nRight = v;  }
    void                SetTop(tools::Long v)     { nTop = v;    }
    void                SetBottom(tools::Long v)  { nBottom = v; }

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
    inline void         Move( tools::Long nHorzMoveDelta, tools::Long nVertMoveDelta );
    void                Move( Size const & s ) { Move(s.Width(), s.Height()); }
    tools::Long         AdjustLeft( tools::Long nHorzMoveDelta ) { nLeft += nHorzMoveDelta; return nLeft; }
    tools::Long         AdjustRight( tools::Long nHorzMoveDelta );
    tools::Long         AdjustTop( tools::Long nVertMoveDelta ) { nTop += nVertMoveDelta; return nTop; }
    tools::Long         AdjustBottom( tools::Long nVertMoveDelta );
    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );
    inline Size         GetSize() const;

    /// Returns the difference between right and left, assuming the range is inclusive.
    inline tools::Long         GetWidth() const;
    /// Returns the difference between bottom and top, assuming the range is inclusive.
    inline tools::Long         GetHeight() const;

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
    tools::Long         getX() const { return nLeft; }
    tools::Long         getY() const { return nTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    tools::Long         getWidth() const;
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    tools::Long         getHeight() const;
    /// Set the left edge of the rectangle to x, preserving the width
    void                setX( tools::Long x );
    /// Set the top edge of the rectangle to y, preserving the height
    void                setY( tools::Long y );
    void                setWidth( tools::Long n ) { nRight = nLeft + n; }
    void                setHeight( tools::Long n ) { nBottom = nTop + n; }
    /// Returns the string representation of the rectangle, format is "x, y, width, height".
    rtl::OString        toString() const;

    /**
     * Expands the rectangle in all directions by the input value.
     */
    void expand(tools::Long nExpandBy);
    void shrink(tools::Long nShrinkBy);

    /**
     * Sanitizing variants for handling data from the outside
     */
    void                SaturatingSetSize(const Size& rSize);
    void                SaturatingSetX(tools::Long x);
    void                SaturatingSetY(tools::Long y);

private:
    tools::Long                nLeft;
    tools::Long                nTop;
    tools::Long                nRight;
    tools::Long                nBottom;
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

constexpr inline tools::Rectangle::Rectangle( tools::Long _nLeft,  tools::Long _nTop,
                             tools::Long _nRight, tools::Long _nBottom )
    : nLeft( _nLeft )
    , nTop( _nTop )
    , nRight( _nRight )
    , nBottom( _nBottom )
{}

inline tools::Rectangle::Rectangle( tools::Long _nLeft,  tools::Long _nTop )
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

inline void tools::Rectangle::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

inline tools::Long tools::Rectangle::GetWidth() const
{
    tools::Long n;
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

inline tools::Long tools::Rectangle::GetHeight() const
{
    tools::Long n;
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

namespace tools
{
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
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
