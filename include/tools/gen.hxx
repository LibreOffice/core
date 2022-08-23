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

#pragma once

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
    constexpr Pair() : mnA(0), mnB(0) {}
    constexpr Pair( tools::Long nA, tools::Long nB ) : mnA(nA), mnB(nB) {}

    tools::Long                A() const { return mnA; }
    tools::Long                B() const { return mnB; }

    tools::Long&               A() { return mnA; }
    tools::Long&               B() { return mnB; }

    TOOLS_DLLPUBLIC rtl::OString        toString() const;

    // Compute value usable as hash.
    TOOLS_DLLPUBLIC size_t     GetHashValue() const;

protected:
    tools::Long                mnA;
    tools::Long                mnB;
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

    constexpr tools::Long      X() const { return mnA; }
    constexpr tools::Long      Y() const { return mnB; }

    void                Move( tools::Long nHorzMove, tools::Long nVertMove );
    void                Move( Size const & s );
    tools::Long                AdjustX( tools::Long nHorzMove ) { mnA += nHorzMove; return mnA; }
    tools::Long                AdjustY( tools::Long nVertMove ) { mnB += nVertMove; return mnB; }

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
    void                setX(tools::Long nX)  { mnA = nX; }
    void                setY(tools::Long nY)  { mnB = nY; }

    Pair const &        toPair() const { return *this; }
    Pair &              toPair() { return *this; }

    // Scales relative to 0,0
    constexpr inline Point scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                 sal_Int64 nMulY, sal_Int64 nDivY) const;

    using Pair::toString;
    using Pair::GetHashValue;
};

inline void Point::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    mnA += nHorzMove;
    mnB += nVertMove;
}

inline Point& Point::operator += ( const Point& rPoint )
{
    mnA += rPoint.mnA;
    mnB += rPoint.mnB;
    return *this;
}

inline Point& Point::operator -= ( const Point& rPoint )
{
    mnA -= rPoint.mnA;
    mnB -= rPoint.mnB;
    return *this;
}

inline Point& Point::operator *= ( const tools::Long nVal )
{
    mnA *= nVal;
    mnB *= nVal;
    return *this;
}

inline Point& Point::operator /= ( const tools::Long nVal )
{
    mnA /= nVal;
    mnB /= nVal;
    return *this;
}

inline Point operator+( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.mnA+rVal2.mnA, rVal1.mnB+rVal2.mnB );
}

inline Point operator-( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.mnA-rVal2.mnA, rVal1.mnB-rVal2.mnB );
}

inline Point operator*( const Point &rVal1, const tools::Long nVal2 )
{
    return Point( rVal1.mnA*nVal2, rVal1.mnB*nVal2 );
}

inline Point operator/( const Point &rVal1, const tools::Long nVal2 )
{
    return Point( rVal1.mnA/nVal2, rVal1.mnB/nVal2 );
}

inline bool operator ==(Point const & p1, Point const & p2)
{
    return tools::detail::equal(p1.toPair(), p2.toPair());
}

inline bool operator !=(Point const & p1, Point const & p2)
{
    return !(p1 == p2);
}

constexpr inline Point Point::scale(sal_Int64 nMulX, sal_Int64 nDivX, sal_Int64 nMulY, sal_Int64 nDivY) const
{
    return Point(o3tl::convert(getX(), nMulX, nDivX),
                 o3tl::convert(getY(), nMulY, nDivY));
}

namespace o3tl
{

constexpr Point convert(const Point& rPoint, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rPoint.scale(num, den, num, den);
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

    constexpr tools::Long  Width() const  { return mnA; }
    constexpr tools::Long  Height() const { return mnB; }

    tools::Long            AdjustWidth( tools::Long n ) { mnA += n; return mnA; }
    tools::Long            AdjustHeight( tools::Long n ) { mnB += n; return mnB; }

    constexpr tools::Long  getWidth() const { return Width(); }
    constexpr tools::Long  getHeight() const { return Height(); }
    void            setWidth(tools::Long nWidth)  { mnA = nWidth; }
    void            setHeight(tools::Long nHeight)  { mnB = nHeight; }

    bool            IsEmpty() const { return mnA <= 0 || mnB <= 0; }

    void extendBy(tools::Long x, tools::Long y)
    {
        mnA += x;
        mnB += y;
    }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
    using Pair::GetHashValue;

    Size&              operator += ( const Size& rSize );
    Size&              operator -= ( const Size& rSize );
    Size&              operator *= ( const tools::Long nVal );
    Size&              operator /= ( const tools::Long nVal );

    friend inline Size operator+( const Size &rVal1, const Size &rVal2 );
    friend inline Size operator-( const Size &rVal1, const Size &rVal2 );
    friend inline Size operator*( const Size &rVal1, const tools::Long nVal2 );
    friend inline Size operator/( const Size &rVal1, const tools::Long nVal2 );

    constexpr inline Size scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                sal_Int64 nMulY, sal_Int64 nDivY) const;

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
    mnA += rSize.mnA;
    mnB += rSize.mnB;
    return *this;
}

inline Size& Size::operator -= ( const Size& rSize )
{
    mnA -= rSize.mnA;
    mnB -= rSize.mnB;
    return *this;
}

inline Size& Size::operator *= ( const tools::Long nVal )
{
    mnA *= nVal;
    mnB *= nVal;
    return *this;
}

inline Size& Size::operator /= ( const tools::Long nVal )
{
    mnA /= nVal;
    mnB /= nVal;
    return *this;
}

inline Size operator+( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.mnA+rVal2.mnA, rVal1.mnB+rVal2.mnB );
}

inline Size operator-( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.mnA-rVal2.mnA, rVal1.mnB-rVal2.mnB );
}

inline Size operator*( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.mnA*nVal2, rVal1.mnB*nVal2 );
}

inline Size operator/( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.mnA/nVal2, rVal1.mnB/nVal2 );
}

constexpr inline Size Size::scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                  sal_Int64 nMulY, sal_Int64 nDivY) const
{
    return Size(o3tl::convert(Width(), nMulX, nDivX),
                o3tl::convert(Height(), nMulY, nDivY));
}

namespace o3tl
{

constexpr Size convert(const Size& rSize, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rSize.scale(num, den, num, den);
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

    tools::Long            Min() const { return mnA; }
    tools::Long            Max() const { return mnB; }
    tools::Long            Len() const { return mnB - mnA + 1; }

    tools::Long&           Min() { return mnA; }
    tools::Long&           Max() { return mnB; }

    bool            Contains( tools::Long nIs ) const;

    void            Normalize();

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Range::Contains( tools::Long nIs ) const
{
    return ((mnA <= nIs) && (nIs <= mnB ));
}

inline void Range::Normalize()
{
    if ( mnA > mnB )
        std::swap(mnA, mnB);
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

    tools::Long            Min() const { return mnA; }
    tools::Long            Max() const { return mnB; }
    tools::Long            Len() const { return mnB - mnA; }

    tools::Long&           Min() { return mnA; }
    tools::Long&           Max() { return mnB; }

    bool            Contains( tools::Long nIs ) const;

    void            Normalize();

    bool            operator !() const { return !Len(); }

    tools::Long            getMin() const { return Min(); }
    void            setMin(tools::Long nMin)  { Min() = nMin; }
    void            setMax(tools::Long nMax)  { Max() = nMax; }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Selection::Contains( tools::Long nIs ) const
{
    return ((mnA <= nIs) && (nIs < mnB ));
}

inline void Selection::Normalize()
{
    if ( mnA > mnB )
        std::swap(mnA, mnB);
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
/// etc. methods interpret the interval as closed. To use the half open versions,
/// use GetOpenWidth() / GetOpenHeight().
///
/// If you want to work with Size, you must use the closed interval functions!
/// And don't add GetOpenSize() / setSize; this will probably just introduce
/// bugs, especially when used in combination with list-initialization.
///
/// (Eventually you might notice, that the same engineer was also working on
/// Qt at some point; see documentation on QRect::bottom / QRect::right ;-).
namespace tools
{
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Rectangle final
{
    static constexpr short RECT_EMPTY = -32767;
public:
    constexpr Rectangle() = default;
    constexpr Rectangle( const Point& rLT, const Point& rRB );
    constexpr Rectangle( tools::Long mnLeft, tools::Long mnTop,
                         tools::Long mnRight, tools::Long mnBottom );
    /// Constructs an empty Rectangle, with top/left at the specified params
    constexpr Rectangle( tools::Long mnLeft, tools::Long mnTop );
    /// Constructs a closed interval rectangle
    constexpr Rectangle( const Point& rLT, const Size& rSize );

    constexpr inline static Rectangle Normalize(const Point& rLT, const Point& rRB);

    constexpr tools::Long Left() const { return mnLeft; }
    constexpr tools::Long Right() const { return IsWidthEmpty() ? mnLeft : mnRight; }
    constexpr tools::Long Top() const { return mnTop; }
    constexpr tools::Long Bottom() const { return IsHeightEmpty() ? mnTop : mnBottom; }

    constexpr void SetLeft(tools::Long v) { mnLeft = v; }
    constexpr void SetRight(tools::Long v) { mnRight = v; }
    constexpr void SetTop(tools::Long v) { mnTop = v; }
    constexpr void SetBottom(tools::Long v) { mnBottom = v; }

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
    tools::Long         AdjustLeft( tools::Long nHorzMoveDelta ) { mnLeft += nHorzMoveDelta; return mnLeft; }
    tools::Long         AdjustRight( tools::Long nHorzMoveDelta );
    tools::Long         AdjustTop( tools::Long nVertMoveDelta ) { mnTop += nVertMoveDelta; return mnTop; }
    tools::Long         AdjustBottom( tools::Long nVertMoveDelta );
    /// Set the left edge of the rectangle to x, preserving the width
    inline void SetPosX(tools::Long x);
    /// Set the top edge of the rectangle to y, preserving the height
    inline void SetPosY(tools::Long y);
    inline void         SetPos( const Point& rPoint );
    inline void SetWidth(tools::Long);
    inline void SetHeight(tools::Long);
    inline void SetSize(const Size&);

    constexpr Point GetPos() const { return TopLeft(); }
    constexpr Size GetSize() const { return { GetWidth(), GetHeight() }; }

    /// Returns the difference between right and left, assuming the range is inclusive.
    constexpr inline tools::Long GetWidth() const;
    /// Returns the difference between bottom and top, assuming the range is inclusive.
    constexpr inline tools::Long GetHeight() const;

    tools::Rectangle&          Union( const tools::Rectangle& rRect );
    tools::Rectangle&          Intersection( const tools::Rectangle& rRect );
    inline tools::Rectangle    GetUnion( const tools::Rectangle& rRect ) const;
    inline tools::Rectangle    GetIntersection( const tools::Rectangle& rRect ) const;

    void                Normalize();

    bool                Contains( const Point& rPOINT ) const;
    bool                Contains( const tools::Rectangle& rRect ) const;
    bool                Overlaps( const tools::Rectangle& rRect ) const;

    void                SetEmpty() { mnRight = mnBottom = RECT_EMPTY; }
    void                SetWidthEmpty() { mnRight = RECT_EMPTY; }
    void                SetHeightEmpty() { mnBottom = RECT_EMPTY; }
    constexpr bool IsEmpty() const { return IsWidthEmpty() || IsHeightEmpty(); }
    constexpr bool IsWidthEmpty() const { return mnRight == RECT_EMPTY; }
    constexpr bool IsHeightEmpty() const { return mnBottom == RECT_EMPTY; }

    inline bool         operator == ( const tools::Rectangle& rRect ) const;
    inline bool         operator != ( const tools::Rectangle& rRect ) const;

    inline tools::Rectangle&   operator += ( const Point& rPt );
    inline tools::Rectangle&   operator -= ( const Point& rPt );

    friend inline tools::Rectangle operator + ( const tools::Rectangle& rRect, const Point& rPt );
    friend inline tools::Rectangle operator - ( const tools::Rectangle& rRect, const Point& rPt );

    tools::Long         getX() const { return mnLeft; }
    tools::Long         getY() const { return mnTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    tools::Long getOpenWidth() const { return Right() - Left(); }
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    tools::Long getOpenHeight() const { return Bottom() - Top(); }
    void                setWidth( tools::Long n ) { mnRight = mnLeft + n; }
    void                setHeight( tools::Long n ) { mnBottom = mnTop + n; }
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
    void                SaturatingSetPosX(tools::Long x);
    void                SaturatingSetPosY(tools::Long y);

    // Scales relative to 0,0
    constexpr inline tools::Rectangle scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                            sal_Int64 nMulY, sal_Int64 nDivY) const;

private:
    tools::Long mnLeft = 0;
    tools::Long mnTop = 0;
    tools::Long mnRight = RECT_EMPTY;
    tools::Long mnBottom = RECT_EMPTY;
};
}

constexpr inline tools::Rectangle::Rectangle( const Point& rLT, const Point& rRB )
    : Rectangle(rLT.X(), rLT.Y(), rRB.X(), rRB.Y())
{}

constexpr inline tools::Rectangle::Rectangle( tools::Long nLeft,  tools::Long nTop,
                             tools::Long nRight, tools::Long nBottom )
    : mnLeft( nLeft )
    , mnTop( nTop )
    , mnRight( nRight )
    , mnBottom( nBottom )
{}

constexpr inline tools::Rectangle::Rectangle( tools::Long nLeft,  tools::Long nTop )
    : mnLeft(nLeft)
    , mnTop(nTop)
{}

constexpr inline tools::Rectangle::Rectangle( const Point& rLT, const Size& rSize )
    : mnLeft( rLT.X())
    , mnTop( rLT.Y())
    , mnRight(rSize.Width() ? mnLeft + (rSize.Width() + (rSize.Width() > 0 ? -1 : 1)) : RECT_EMPTY)
    , mnBottom(rSize.Height() ? mnTop + (rSize.Height() + (rSize.Height() > 0 ? -1 : 1)) : RECT_EMPTY)
{}

constexpr inline tools::Rectangle tools::Rectangle::Normalize(const Point& rLT, const Point& rRB)
{
    const std::pair<tools::Long, tools::Long> aLeftRight = std::minmax(rLT.X(), rRB.X());
    const std::pair<tools::Long, tools::Long> aTopBottom = std::minmax(rLT.Y(), rRB.Y());
    return { aLeftRight.first, aTopBottom.first, aLeftRight.second, aTopBottom.second };
}

inline void tools::Rectangle::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    mnLeft += nHorzMove;
    mnTop  += nVertMove;
    if (!IsWidthEmpty())
        mnRight += nHorzMove;
    if (!IsHeightEmpty())
        mnBottom += nVertMove;
}

inline void tools::Rectangle::SetPosX(tools::Long x)
{
    if (!IsWidthEmpty())
        mnRight += x - mnLeft;
    mnLeft = x;
}

inline void tools::Rectangle::SetPosY(tools::Long y)
{
    if (!IsHeightEmpty())
        mnBottom += y - mnTop;
    mnTop = y;
}

inline void tools::Rectangle::SetPos( const Point& rPoint )
{
    SetPosX(rPoint.X());
    SetPosY(rPoint.Y());
}

inline void tools::Rectangle::SetWidth(tools::Long nWidth)
{
    if (nWidth < 0)
        mnRight = mnLeft + nWidth + 1;
    else if (nWidth > 0)
        mnRight = mnLeft + nWidth - 1;
    else
        SetWidthEmpty();
}

inline void tools::Rectangle::SetHeight(tools::Long nHeight)
{
    if (nHeight < 0)
        mnBottom = mnTop + nHeight + 1;
    else if (nHeight > 0)
        mnBottom = mnTop + nHeight - 1;
    else
        SetHeightEmpty();
}

inline void tools::Rectangle::SetSize(const Size& rSize)
{
    SetWidth(rSize.Width());
    SetHeight(rSize.Height());
}

constexpr inline tools::Long tools::Rectangle::GetWidth() const
{
    tools::Long n = 0;

    if (!IsWidthEmpty())
    {
        n = mnRight - mnLeft;
        if (n < 0)
            n--;
        else
            n++;
    }

    return n;
}

constexpr inline tools::Long tools::Rectangle::GetHeight() const
{
    tools::Long n = 0;

    if (!IsHeightEmpty())
    {
        n = mnBottom - mnTop;
        if (n < 0)
            n--;
        else
            n++;
    }

    return n;
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
    return (mnLeft   == rRect.mnLeft   ) &&
           (mnTop    == rRect.mnTop    ) &&
           (mnRight  == rRect.mnRight  ) &&
           (mnBottom == rRect.mnBottom );
}

inline bool tools::Rectangle::operator != ( const tools::Rectangle& rRect ) const
{
    return (mnLeft   != rRect.mnLeft   ) ||
           (mnTop    != rRect.mnTop    ) ||
           (mnRight  != rRect.mnRight  ) ||
           (mnBottom != rRect.mnBottom );
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

constexpr inline tools::Rectangle tools::Rectangle::scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                                          sal_Int64 nMulY, sal_Int64 nDivY) const
{
    // 1. Create an empty rectangle with correct left and top
    tools::Rectangle aRect(o3tl::convert(Left(), nMulX, nDivX),
                           o3tl::convert(Top(), nMulY, nDivY));
    // 2. If source has width/height, set respective right and bottom
    if (!IsWidthEmpty())
        aRect.SetRight(o3tl::convert(Right(), nMulX, nDivX));
    if (!IsHeightEmpty())
        aRect.SetBottom(o3tl::convert(Bottom(), nMulY, nDivY));
    return aRect;
}

namespace o3tl
{

constexpr tools::Rectangle convert(const tools::Rectangle& rRectangle, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rRectangle.scale(num, den, num, den);
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
                      << "@(" << rectangle.getX() << ',' << rectangle.getY() << ")";
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
