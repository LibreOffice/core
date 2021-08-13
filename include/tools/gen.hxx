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
#include <o3tl/unit_conversion.hxx>

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
    constexpr Pair() : nA(0), nB(0) {}
    constexpr Pair( tools::Long _nA, tools::Long _nB ) : nA(_nA), nB(_nB) {}

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
    constexpr Point() {}
    constexpr Point( tools::Long nX, tools::Long nY ) : Pair( nX, nY ) {}

    constexpr tools::Long      X() const { return nA; }
    constexpr tools::Long      Y() const { return nB; }

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

    constexpr tools::Long      getX() const { return X(); }
    constexpr tools::Long      getY() const { return Y(); }
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

namespace o3tl
{

constexpr Point convert(const Point& rPoint, o3tl::Length eFrom, o3tl::Length eTo)
{
    return Point(
            o3tl::convert(rPoint.getX(), eFrom, eTo),
            o3tl::convert(rPoint.getY(), eFrom, eTo));
}

} // end o3tl

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
    constexpr Size() {}
    constexpr Size( tools::Long nWidth, tools::Long nHeight ) : Pair( nWidth, nHeight ) {}

    constexpr tools::Long  Width() const  { return nA; }
    constexpr tools::Long  Height() const { return nB; }

    tools::Long            AdjustWidth( tools::Long n ) { nA += n; return nA; }
    tools::Long            AdjustHeight( tools::Long n ) { nB += n; return nB; }

    constexpr tools::Long  getWidth() const { return Width(); }
    constexpr tools::Long  getHeight() const { return Height(); }
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

    Size&              operator += ( const Size& rSize );
    Size&              operator -= ( const Size& rSize );
    Size&              operator *= ( const tools::Long nVal );
    Size&              operator /= ( const tools::Long nVal );

    friend inline Size operator+( const Size &rVal1, const Size &rVal2 );
    friend inline Size operator-( const Size &rVal1, const Size &rVal2 );
    friend inline Size operator*( const Size &rVal1, const tools::Long nVal2 );
    friend inline Size operator/( const Size &rVal1, const tools::Long nVal2 );

};

inline bool operator ==(Size const & s1, Size const & s2)
{
    return tools::detail::equal(s1.toPair(), s2.toPair());
}

inline bool operator !=(Size const & s1, Size const & s2)
{
    return !(s1 == s2);
}

inline Size& Size::operator += ( const Size& rSize )
{
    nA += rSize.nA;
    nB += rSize.nB;
    return *this;
}

inline Size& Size::operator -= ( const Size& rSize )
{
    nA -= rSize.nA;
    nB -= rSize.nB;
    return *this;
}

inline Size& Size::operator *= ( const tools::Long nVal )
{
    nA *= nVal;
    nB *= nVal;
    return *this;
}

inline Size& Size::operator /= ( const tools::Long nVal )
{
    nA /= nVal;
    nB /= nVal;
    return *this;
}

inline Size operator+( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.nA+rVal2.nA, rVal1.nB+rVal2.nB );
}

inline Size operator-( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.nA-rVal2.nA, rVal1.nB-rVal2.nB );
}

inline Size operator*( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.nA*nVal2, rVal1.nB*nVal2 );
}

inline Size operator/( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.nA/nVal2, rVal1.nB/nVal2 );
}

namespace o3tl
{

constexpr Size convert(const Size& rSize, o3tl::Length eFrom, o3tl::Length eTo)
{
        return Size(
            o3tl::convert(rSize.Width(), eFrom, eTo),
            o3tl::convert(rSize.Height(), eFrom, eTo));
}

} // end o3tl

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
    constexpr Range() {}
    constexpr Range( tools::Long nMin, tools::Long nMax ) : Pair( nMin, nMax ) {}

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
    constexpr Rectangle() = default;
    constexpr Rectangle( const Point& rLT, const Point& rRB );
    constexpr Rectangle( tools::Long nLeft, tools::Long nTop,
                         tools::Long nRight, tools::Long nBottom );
    /// Constructs an empty Rectangle, with top/left at the specified params
    constexpr Rectangle( tools::Long nLeft, tools::Long nTop );
    constexpr Rectangle( const Point& rLT, const Size& rSize );

    static Rectangle    Justify( const Point& rLT, const Point& rRB );

    constexpr tools::Long Left() const { return nLeft; }
    constexpr tools::Long Right() const { return IsWidthEmpty() ? nLeft : nRight; }
    constexpr tools::Long Top() const { return nTop; }
    constexpr tools::Long Bottom() const { return IsHeightEmpty() ? nTop : nBottom; }

    constexpr void SetLeft(tools::Long v) { nLeft = v; }
    constexpr void SetRight(tools::Long v) { nRight = v; }
    constexpr void SetTop(tools::Long v) { nTop = v; }
    constexpr void SetBottom(tools::Long v) { nBottom = v; }

    constexpr Point TopLeft() const { return { Left(), Top() }; }
    constexpr Point TopRight() const { return { Right(), Top() }; }
    constexpr Point TopCenter() const { return { (Left() + Right()) / 2, Top() }; }
    constexpr Point BottomLeft() const { return { Left(), Bottom() }; }
    constexpr Point BottomRight() const { return { Right(), Bottom() }; }
    constexpr Point BottomCenter() const { return { (Left() + Right()) / 2, Bottom() }; }
    constexpr Point LeftCenter() const { return { Left(), (Top() + Bottom()) / 2 }; }
    constexpr Point RightCenter() const { return { Right(), (Top() + Bottom()) / 2 }; }
    constexpr Point Center() const { return { (Left() + Right()) / 2, (Top() + Bottom()) / 2 }; }

    /// Move the top and left edges by a delta, preserving width and height
    inline void         Move( tools::Long nHorzMoveDelta, tools::Long nVertMoveDelta );
    void                Move( Size const & s ) { Move(s.Width(), s.Height()); }
    tools::Long         AdjustLeft( tools::Long nHorzMoveDelta ) { nLeft += nHorzMoveDelta; return nLeft; }
    tools::Long         AdjustRight( tools::Long nHorzMoveDelta );
    tools::Long         AdjustTop( tools::Long nVertMoveDelta ) { nTop += nVertMoveDelta; return nTop; }
    tools::Long         AdjustBottom( tools::Long nVertMoveDelta );
    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );

    constexpr Size GetSize() const { return { GetWidth(), GetHeight() }; }

    /// Returns the difference between right and left, assuming the range is inclusive.
    constexpr tools::Long GetWidth() const
    {
        tools::Long n = 0;

        if (!IsWidthEmpty())
        {
            n = nRight - nLeft;
            if (n < 0)
                n--;
            else
                n++;
        }

        return n;
    }

    /// Returns the difference between bottom and top, assuming the range is inclusive.
    constexpr tools::Long GetHeight() const
    {
        tools::Long n = 0;

        if (!IsHeightEmpty())
        {
            n = nBottom - nTop;
            if (n < 0)
                n--;
            else
                n++;
        }

        return n;
    }

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
    constexpr bool IsEmpty() const { return IsWidthEmpty() || IsHeightEmpty(); }
    constexpr bool IsWidthEmpty() const { return nRight == RECT_EMPTY; }
    constexpr bool IsHeightEmpty() const { return nBottom == RECT_EMPTY; }

    inline bool         operator == ( const tools::Rectangle& rRect ) const;
    inline bool         operator != ( const tools::Rectangle& rRect ) const;

    inline tools::Rectangle&   operator += ( const Point& rPt );
    inline tools::Rectangle&   operator -= ( const Point& rPt );

    friend inline tools::Rectangle operator + ( const tools::Rectangle& rRect, const Point& rPt );
    friend inline tools::Rectangle operator - ( const tools::Rectangle& rRect, const Point& rPt );

    // ONE
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    tools::Long getWidth() const { return Right() - Left(); }
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    tools::Long getHeight() const { return Bottom() - Top(); }
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
    tools::Long nLeft = 0;
    tools::Long nTop = 0;
    tools::Long nRight = RECT_EMPTY;
    tools::Long nBottom = RECT_EMPTY;
};
}

constexpr inline tools::Rectangle::Rectangle( const Point& rLT, const Point& rRB )
    : Rectangle(rLT.X(), rLT.Y(), rRB.X(), rRB.Y())
{}

constexpr inline tools::Rectangle::Rectangle( tools::Long _nLeft,  tools::Long _nTop,
                             tools::Long _nRight, tools::Long _nBottom )
    : nLeft( _nLeft )
    , nTop( _nTop )
    , nRight( _nRight )
    , nBottom( _nBottom )
{}

constexpr inline tools::Rectangle::Rectangle( tools::Long _nLeft,  tools::Long _nTop )
    : nLeft(_nLeft)
    , nTop(_nTop)
{}

constexpr inline tools::Rectangle::Rectangle( const Point& rLT, const Size& rSize )
    : nLeft( rLT.X())
    , nTop( rLT.Y())
    , nRight( rSize.Width()  ? nLeft+(rSize.Width()-1) : RECT_EMPTY )
    , nBottom( rSize.Height() ? nTop+(rSize.Height()-1) : RECT_EMPTY )
{}

inline void tools::Rectangle::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    nLeft += nHorzMove;
    nTop  += nVertMove;
    if (!IsWidthEmpty())
        nRight += nHorzMove;
    if (!IsHeightEmpty())
        nBottom += nVertMove;
}

inline void tools::Rectangle::SetPos( const Point& rPoint )
{
    if (!IsWidthEmpty())
        nRight += rPoint.X() - nLeft;
    if (!IsHeightEmpty())
        nBottom += rPoint.Y() - nTop;
    nLeft = rPoint.X();
    nTop  = rPoint.Y();
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
    Move(rPt.X(), rPt.Y());
    return *this;
}

inline tools::Rectangle& tools::Rectangle::operator -= ( const Point& rPt )
{
    Move(-rPt.X(), -rPt.Y());
    return *this;
}

namespace tools
{
inline Rectangle operator + ( const Rectangle& rRect, const Point& rPt )
{
    return Rectangle{ rRect }.operator+=(rPt);
}

inline Rectangle operator - ( const Rectangle& rRect, const Point& rPt )
{
    return Rectangle{ rRect }.operator-=(rPt);
}

}

namespace o3tl
{

constexpr tools::Rectangle convert(const tools::Rectangle& rRectangle, o3tl::Length eFrom, o3tl::Length eTo)
{
    return tools::Rectangle(o3tl::convert(rRectangle.TopLeft(), eFrom, eTo),
                            o3tl::convert(rRectangle.GetSize(), eFrom, eTo));
}

} // end o3tl

namespace tools
{
template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const tools::Rectangle& rectangle )
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.GetWidth() << 'x' << rectangle.GetHeight()
                      << "@(" << rectangle.Left() << ',' << rectangle.Top() << ")";
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
