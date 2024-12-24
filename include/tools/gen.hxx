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
#include <o3tl/unit_conversion.hxx>

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

class TOOLS_DLLPUBLIC PointTemplateBase : protected Pair
{
friend class RectangleTemplateBase;
public:
    PointTemplateBase() = default;
protected:
    constexpr PointTemplateBase( tools::Long nX, tools::Long nY ) : Pair( nX, nY ) {}
    // Rotate parameter point using This as origin; store result back into parameter point
    void                RotateAround( tools::Long& rX, tools::Long& rY, Degree10 nOrientation ) const;
    void                RotateAround( PointTemplateBase&, Degree10 nOrientation ) const;

public:
    constexpr tools::Long      X() const { return mnA; }
    constexpr tools::Long      Y() const { return mnB; }
};

template<class PointT, class SizeT>
class PointTemplate : public PointTemplateBase
{
public:
    using SizeType = SizeT;

    constexpr PointTemplate() {}
    constexpr PointTemplate( tools::Long nX, tools::Long nY ) : PointTemplateBase( nX, nY ) {}

    void Move( tools::Long nHorzMove, tools::Long nVertMove )
    {
        mnA += nHorzMove;
        mnB += nVertMove;
    }
    void Move( SizeT const & s )
    {
        AdjustX(s.Width());
        AdjustY(s.Height());
    }

    tools::Long                AdjustX( tools::Long nHorzMove ) { mnA += nHorzMove; return mnA; }
    tools::Long                AdjustY( tools::Long nVertMove ) { mnB += nVertMove; return mnB; }

    void                RotateAround( tools::Long& rX, tools::Long& rY, Degree10 nOrientation ) const
    { PointTemplateBase::RotateAround(rX, rY, nOrientation); }
    void                RotateAround( PointT& p, Degree10 nOrientation ) const
    { PointTemplateBase::RotateAround(p, nOrientation); }

    PointT& operator+=( const PointT& rPoint )
    {
        mnA += rPoint.mnA;
        mnB += rPoint.mnB;
        return static_cast<PointT&>(*this);
    }
    PointT& operator-=( const PointT& rPoint )
    {
        mnA -= rPoint.mnA;
        mnB -= rPoint.mnB;
        return static_cast<PointT&>(*this);
    }
    PointT& operator*=( const tools::Long nVal )
    {
        mnA *= nVal;
        mnB *= nVal;
        return static_cast<PointT&>(*this);
    }
    PointT& operator/=( const tools::Long nVal )
    {
        mnA /= nVal;
        mnB /= nVal;
        return static_cast<PointT&>(*this);
    }

    constexpr tools::Long      getX() const { return X(); }
    constexpr tools::Long      getY() const { return Y(); }
    void                setX(tools::Long nX)  { mnA = nX; }
    void                setY(tools::Long nY)  { mnB = nY; }

    Pair const &        toPair() const { return *this; }
    Pair &              toPair() { return *this; }

    // Scales relative to 0,0
    constexpr PointT scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                 sal_Int64 nMulY, sal_Int64 nDivY) const
    {
        return PointT(o3tl::convert(getX(), nMulX, nDivX),
                    o3tl::convert(getY(), nMulY, nDivY));
    }

    using Pair::toString;
    using Pair::GetHashValue;
};

class Size;
class AbsoluteScreenPixelSize;
class AbsoluteScreenPixelPoint;
namespace tools { class Rectangle; }
class AbsoluteScreenPixelRectangle;

class SAL_WARN_UNUSED Point : public PointTemplate<::Point, ::Size>
{
public:
    constexpr Point() {}
    constexpr Point( tools::Long nX, tools::Long nY ) : PointTemplate( nX, nY ) {}
    // TODO delete this to expose more problems
    constexpr explicit Point(const AbsoluteScreenPixelPoint&);
};

// A point relative to top-level parent or screen, in screen pixels
class SAL_WARN_UNUSED AbsoluteScreenPixelPoint : public PointTemplate<AbsoluteScreenPixelPoint, AbsoluteScreenPixelSize> {
public:
    constexpr AbsoluteScreenPixelPoint() {}
    constexpr AbsoluteScreenPixelPoint( tools::Long nX, tools::Long nY ) : PointTemplate( nX, nY ) {}
    constexpr explicit AbsoluteScreenPixelPoint(const Point & pt) : PointTemplate(pt.X(), pt.Y()) {}
};

inline Point operator+( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.X()+rVal2.X(), rVal1.Y()+rVal2.Y() );
}
inline AbsoluteScreenPixelPoint operator+( const AbsoluteScreenPixelPoint &rVal1, const AbsoluteScreenPixelPoint &rVal2 )
{
    return AbsoluteScreenPixelPoint( rVal1.X()+rVal2.X(), rVal1.Y()+rVal2.Y() );
}

inline Point operator-( const Point &rVal1, const Point &rVal2 )
{
    return Point( rVal1.X()-rVal2.X(), rVal1.Y()-rVal2.Y() );
}
inline AbsoluteScreenPixelPoint operator-( const AbsoluteScreenPixelPoint &rVal1, const AbsoluteScreenPixelPoint &rVal2 )
{
    return AbsoluteScreenPixelPoint( rVal1.X()-rVal2.X(), rVal1.Y()-rVal2.Y() );
}

inline Point operator*( const Point &rVal1, const tools::Long nVal2 )
{
    return Point( rVal1.X()*nVal2, rVal1.Y()*nVal2 );
}
inline AbsoluteScreenPixelPoint operator*( const AbsoluteScreenPixelPoint &rVal1, const tools::Long nVal2 )
{
    return AbsoluteScreenPixelPoint( rVal1.X()*nVal2, rVal1.Y()*nVal2 );
}

inline Point operator/( const Point &rVal1, const tools::Long nVal2 )
{
    return Point( rVal1.X()/nVal2, rVal1.Y()/nVal2 );
}
inline AbsoluteScreenPixelPoint operator/( const AbsoluteScreenPixelPoint &rVal1, const tools::Long nVal2 )
{
    return AbsoluteScreenPixelPoint( rVal1.X()/nVal2, rVal1.Y()/nVal2 );
}

inline bool operator ==(Point const & p1, Point const & p2)
{
    return tools::detail::equal(p1.toPair(), p2.toPair());
}
inline bool operator ==(AbsoluteScreenPixelPoint const & p1, AbsoluteScreenPixelPoint const & p2)
{
    return tools::detail::equal(p1.toPair(), p2.toPair());
}

inline bool operator !=(Point const & p1, Point const & p2)
{
    return !(p1 == p2);
}
inline bool operator !=(AbsoluteScreenPixelPoint const & p1, AbsoluteScreenPixelPoint const & p2)
{
    return !(p1 == p2);
}


constexpr Point::Point(const AbsoluteScreenPixelPoint& p) : Point(p.X(), p.Y()) {}

namespace o3tl
{
template <class PointT,
          std::enable_if_t<std::is_base_of_v<PointTemplate<PointT, typename PointT::SizeType>, PointT>, int> = 0>
constexpr auto convert(const PointT& rPoint, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rPoint.scale(num, den, num, den);
}
} // namespace o3tl

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const PointTemplateBase& point )
{
    return stream << point.X() << ',' << point.Y();
}

// Size

class SizeTemplateBase : protected Pair
{
public:
    constexpr SizeTemplateBase() = default;
    constexpr SizeTemplateBase( tools::Long nWidth, tools::Long nHeight ) : Pair( nWidth, nHeight ) {}

    constexpr tools::Long  Width() const  { return mnA; }
    constexpr tools::Long  Height() const { return mnB; }

};

template<class SizeT>
class SizeTemplate : public SizeTemplateBase
{
public:
    constexpr SizeTemplate() {}
    constexpr SizeTemplate( tools::Long nWidth, tools::Long nHeight ) : SizeTemplateBase( nWidth, nHeight ) {}

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

    SizeT&              operator += ( const SizeT& rSize )
    {
        mnA += rSize.mnA;
        mnB += rSize.mnB;
        return static_cast<SizeT&>(*this);
    }
    SizeT&              operator -= ( const SizeT& rSize )
    {
        mnA -= rSize.mnA;
        mnB -= rSize.mnB;
        return static_cast<SizeT&>(*this);
    }
    SizeT&              operator *= ( const tools::Long nVal )
    {
        mnA *= nVal;
        mnB *= nVal;
        return static_cast<SizeT&>(*this);
    }
    SizeT&              operator /= ( const tools::Long nVal )
    {
        mnA /= nVal;
        mnB /= nVal;
        return static_cast<SizeT&>(*this);
    }

    constexpr SizeT scale(sal_Int64 nMulX, sal_Int64 nDivX,
                        sal_Int64 nMulY, sal_Int64 nDivY) const
    {
        return SizeT(o3tl::convert(Width(), nMulX, nDivX),
                    o3tl::convert(Height(), nMulY, nDivY));
    }
};

class SAL_WARN_UNUSED Size : public SizeTemplate<::Size>
{
public:
    constexpr Size() {}
    constexpr Size( tools::Long nWidth, tools::Long nHeight ) : SizeTemplate( nWidth, nHeight ) {}
    // TODO delete to find more problems
    constexpr explicit Size(const AbsoluteScreenPixelSize& pt);
};

// Screen pixels
class SAL_WARN_UNUSED AbsoluteScreenPixelSize : public SizeTemplate<AbsoluteScreenPixelSize>
{
public:
    constexpr AbsoluteScreenPixelSize() {}
    constexpr AbsoluteScreenPixelSize( tools::Long nWidth, tools::Long nHeight ) : SizeTemplate( nWidth, nHeight ) {}
    constexpr explicit AbsoluteScreenPixelSize(const Size & pt) : SizeTemplate(pt.Width(), pt.Height()) {}
};

constexpr Size::Size(const AbsoluteScreenPixelSize& pt) : SizeTemplate(pt.Width(), pt.Height()) {}

inline bool operator ==(Size const & s1, Size const & s2)
{
    return tools::detail::equal(s1.toPair(), s2.toPair());
}
inline bool operator ==(AbsoluteScreenPixelSize const & s1, AbsoluteScreenPixelSize const & s2)
{
    return tools::detail::equal(s1.toPair(), s2.toPair());
}

inline bool operator !=(Size const & s1, Size const & s2)
{
    return !(s1 == s2);
}
inline bool operator !=(AbsoluteScreenPixelSize const & s1, AbsoluteScreenPixelSize const & s2)
{
    return !(s1 == s2);
}

inline Size operator+( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.Width()+rVal2.Width(), rVal1.Height()+rVal2.Height() );
}
inline AbsoluteScreenPixelSize operator+( const AbsoluteScreenPixelSize &rVal1, const AbsoluteScreenPixelSize &rVal2 )
{
    return AbsoluteScreenPixelSize( rVal1.Width()+rVal2.Width(), rVal1.Height()+rVal2.Height() );
}

inline Size operator-( const Size &rVal1, const Size &rVal2 )
{
    return Size( rVal1.Width()-rVal2.Width(), rVal1.Height()-rVal2.Height() );
}
inline AbsoluteScreenPixelSize operator-( const AbsoluteScreenPixelSize &rVal1, const AbsoluteScreenPixelSize &rVal2 )
{
    return AbsoluteScreenPixelSize( rVal1.Width()-rVal2.Width(), rVal1.Height()-rVal2.Height() );
}

inline Size operator*( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.Width()*nVal2, rVal1.Height()*nVal2 );
}
inline AbsoluteScreenPixelSize operator*( const AbsoluteScreenPixelSize &rVal1, const tools::Long nVal2 )
{
    return AbsoluteScreenPixelSize( rVal1.Width()*nVal2, rVal1.Height()*nVal2 );
}

inline Size operator/( const Size &rVal1, const tools::Long nVal2 )
{
    return Size( rVal1.Width()/nVal2, rVal1.Height()/nVal2 );
}
inline AbsoluteScreenPixelSize operator/( const AbsoluteScreenPixelSize &rVal1, const tools::Long nVal2 )
{
    return AbsoluteScreenPixelSize( rVal1.Width()/nVal2, rVal1.Height()/nVal2 );
}

namespace o3tl
{

template <class SizeT,
          std::enable_if_t<std::is_base_of_v<SizeTemplate<SizeT>, SizeT>, int> = 0>
constexpr auto convert(const SizeT& rSize, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rSize.scale(num, den, num, den);
}

} // end o3tl

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const SizeTemplateBase& size )
{
    return stream << size.Width() << 'x' << size.Height();
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

class TOOLS_DLLPUBLIC RectangleTemplateBase
{
public:
    static constexpr short RECT_EMPTY = -32767;

    constexpr RectangleTemplateBase() = default;
    constexpr RectangleTemplateBase( tools::Long nLeft, tools::Long nTop,
                         tools::Long nRight, tools::Long nBottom )
        : mnLeft( nLeft ), mnTop( nTop ), mnRight( nRight ), mnBottom( nBottom )
    {}
    /// Constructs an empty Rectangle, with top/left at the specified params
    constexpr RectangleTemplateBase( tools::Long nLeft, tools::Long nTop )
        : mnLeft(nLeft), mnTop(nTop)
    {}
    /// Constructs a closed interval rectangle
    constexpr RectangleTemplateBase( const PointTemplateBase& rLT, const SizeTemplateBase& rSize )
        : mnLeft( rLT.X())
        , mnTop( rLT.Y())
        , mnRight(rSize.Width() ? mnLeft + (rSize.Width() + (rSize.Width() > 0 ? -1 : 1)) : RECT_EMPTY)
        , mnBottom(rSize.Height() ? mnTop + (rSize.Height() + (rSize.Height() > 0 ? -1 : 1)) : RECT_EMPTY)
    {}

    constexpr tools::Long Left() const { return mnLeft; }
    constexpr tools::Long Right() const { return IsWidthEmpty() ? mnLeft : mnRight; }
    constexpr tools::Long Top() const { return mnTop; }
    constexpr tools::Long Bottom() const { return IsHeightEmpty() ? mnTop : mnBottom; }

    constexpr void SetLeft(tools::Long v) { mnLeft = v; }
    constexpr void SetRight(tools::Long v) { mnRight = v; }
    constexpr void SetTop(tools::Long v) { mnTop = v; }
    constexpr void SetBottom(tools::Long v) { mnBottom = v; }

    void                SetEmpty() { mnRight = mnBottom = RECT_EMPTY; }
    constexpr bool IsEmpty() const { return IsWidthEmpty() || IsHeightEmpty(); }
    constexpr bool IsWidthEmpty() const { return mnRight == RECT_EMPTY; }
    constexpr bool IsHeightEmpty() const { return mnBottom == RECT_EMPTY; }
    void                SetWidthEmpty() { mnRight = RECT_EMPTY; }
    void                SetHeightEmpty() { mnBottom = RECT_EMPTY; }

    tools::Long         getX() const { return mnLeft; }
    tools::Long         getY() const { return mnTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    tools::Long getOpenWidth() const { return Right() - Left(); }
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    tools::Long getOpenHeight() const { return Bottom() - Top(); }
    void                setWidth( tools::Long n ) { mnRight = mnLeft + n; }
    void                setHeight( tools::Long n ) { mnBottom = mnTop + n; }

    /// Returns the difference between right and left, assuming the range is inclusive.
    constexpr tools::Long GetWidth() const
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
    /// Returns the difference between bottom and top, assuming the range is inclusive.
    constexpr tools::Long GetHeight() const
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


    tools::Long         AdjustLeft( tools::Long nHorzMoveDelta ) { mnLeft += nHorzMoveDelta; return mnLeft; }
    tools::Long         AdjustRight( tools::Long nHorzMoveDelta );
    tools::Long         AdjustTop( tools::Long nVertMoveDelta ) { mnTop += nVertMoveDelta; return mnTop; }
    tools::Long         AdjustBottom( tools::Long nVertMoveDelta );
    /// Set the left edge of the rectangle to x, preserving the width
    void SetPosX(tools::Long x)
    {
        if (!IsWidthEmpty())
            mnRight += x - mnLeft;
        mnLeft = x;
    }
    /// Set the top edge of the rectangle to y, preserving the height
    void SetPosY(tools::Long y)
    {
        if (!IsHeightEmpty())
            mnBottom += y - mnTop;
        mnTop = y;
    }

    void                SaturatingSetPosX(tools::Long x);
    void                SaturatingSetPosY(tools::Long y);

    void SetWidth(tools::Long nWidth)
    {
        if (nWidth < 0)
            mnRight = mnLeft + nWidth + 1;
        else if (nWidth > 0)
            mnRight = mnLeft + nWidth - 1;
        else
            SetWidthEmpty();
    }
    void SetHeight(tools::Long nHeight)
    {
        if (nHeight < 0)
            mnBottom = mnTop + nHeight + 1;
        else if (nHeight > 0)
            mnBottom = mnTop + nHeight - 1;
        else
            SetHeightEmpty();
    }

    void                Normalize();

    /**
     * Expands the rectangle in all directions by the input value.
     */
    void expand(tools::Long nExpandBy);
    void shrink(tools::Long nShrinkBy);

    /// Move the top and left edges by a delta, preserving width and height
    void Move( tools::Long nHorzMoveDelta, tools::Long nVertMoveDelta )
    {
        mnLeft += nHorzMoveDelta;
        mnTop  += nVertMoveDelta;
        if (!IsWidthEmpty())
            mnRight += nHorzMoveDelta;
        if (!IsHeightEmpty())
            mnBottom += nVertMoveDelta;
    }

    /// Returns the string representation of the rectangle, format is "x, y, width, height".
    rtl::OString        toString() const;

protected:
    void SaturatingSetSize(const SizeTemplateBase& rSize);
    void Union( const RectangleTemplateBase& rRect );
    void Intersection( const RectangleTemplateBase& rRect );
    bool Contains( const PointTemplateBase& rPOINT ) const;
    bool Contains( const RectangleTemplateBase& rRect ) const;
    bool Overlaps( const RectangleTemplateBase& rRect ) const;

    tools::Long mnLeft = 0;
    tools::Long mnTop = 0;
    tools::Long mnRight = RECT_EMPTY;
    tools::Long mnBottom = RECT_EMPTY;
};

template<class RectangleT, class PointT, class SizeT>
class RectangleTemplate : public RectangleTemplateBase
{
friend class ::tools::Rectangle;
friend class AbsoluteScreenPixelRectangle;
public:
    using PointType = PointT;
    using SizeType = SizeT;

public:
    constexpr RectangleTemplate() = default;
    constexpr RectangleTemplate( const PointT& rLT, const PointT& rRB )
        : RectangleTemplate(rLT.X(), rLT.Y(), rRB.X(), rRB.Y()) {}
    constexpr RectangleTemplate( tools::Long nLeft, tools::Long nTop,
                         tools::Long nRight, tools::Long nBottom )
        : RectangleTemplateBase(nLeft, nTop, nRight, nBottom )
    {}
    /// Constructs an empty Rectangle, with top/left at the specified params
    constexpr RectangleTemplate( tools::Long nLeft, tools::Long nTop )
        : RectangleTemplateBase(nLeft, nTop)
    {}
    /// Constructs a closed interval rectangle
    constexpr RectangleTemplate( const PointT& rLT, const SizeT& rSize )
        : RectangleTemplateBase( rLT, rSize )
    {}

    using RectangleTemplateBase::Normalize;
    constexpr static RectangleT Normalize(const PointT& rLT, const PointT& rRB)
    {
        const std::pair<tools::Long, tools::Long> aLeftRight = std::minmax(rLT.X(), rRB.X());
        const std::pair<tools::Long, tools::Long> aTopBottom = std::minmax(rLT.Y(), rRB.Y());
        return { aLeftRight.first, aTopBottom.first, aLeftRight.second, aTopBottom.second };
    }

    constexpr PointT TopLeft() const { return { Left(), Top() }; }
    constexpr PointT TopRight() const { return { Right(), Top() }; }
    constexpr PointT TopCenter() const { return { (Left() + Right()) / 2, Top() }; }
    constexpr PointT BottomLeft() const { return { Left(), Bottom() }; }
    constexpr PointT BottomRight() const { return { Right(), Bottom() }; }
    constexpr PointT BottomCenter() const { return { (Left() + Right()) / 2, Bottom() }; }
    constexpr PointT LeftCenter() const { return { Left(), (Top() + Bottom()) / 2 }; }
    constexpr PointT RightCenter() const { return { Right(), (Top() + Bottom()) / 2 }; }
    constexpr PointT Center() const { return { (Left() + Right()) / 2, (Top() + Bottom()) / 2 }; }

    using RectangleTemplateBase::Move;
    void                Move( SizeT const & s ) { Move(s.Width(), s.Height()); }
    void SetPos( const PointT& rPoint )
    {
        SetPosX(rPoint.X());
        SetPosY(rPoint.Y());
    }
    void SetSize(const SizeT& rSize)
    {
        SetWidth(rSize.Width());
        SetHeight(rSize.Height());
    }

    constexpr PointT GetPos() const { return TopLeft(); }
    constexpr SizeT GetSize() const { return { GetWidth(), GetHeight() }; }

    RectangleT&          Union( const RectangleTemplate& rRect ) { RectangleTemplateBase::Union(rRect); return static_cast<RectangleT&>(*this); }
    RectangleT&          Intersection( const RectangleTemplate& rRect ) { RectangleTemplateBase::Intersection(rRect); return static_cast<RectangleT&>(*this); }
    RectangleT    GetUnion( const RectangleT& rRect ) const
    {
        RectangleT aTmpRect( rRect );
        return aTmpRect.Union( *this );
    }
    RectangleT    GetIntersection( const RectangleT& rRect ) const
    {
        RectangleT aTmpRect( rRect );
        return aTmpRect.Intersection( *this );
    }

    bool                Contains( const PointT& rPt ) const { return RectangleTemplateBase::Contains(rPt); }
    bool                Contains( const RectangleT& rRect ) const { return RectangleTemplateBase::Contains(rRect); }
    bool                Overlaps( const RectangleT& rRect ) const { return RectangleTemplateBase::Overlaps(rRect); }

    bool         operator == ( const RectangleTemplate& rRect ) const
    {
        return (mnLeft   == rRect.mnLeft   ) &&
               (mnTop    == rRect.mnTop    ) &&
               (mnRight  == rRect.mnRight  ) &&
               (mnBottom == rRect.mnBottom );
    }
    bool         operator != ( const RectangleTemplate& rRect ) const
    {
        return (mnLeft   != rRect.mnLeft   ) ||
               (mnTop    != rRect.mnTop    ) ||
               (mnRight  != rRect.mnRight  ) ||
               (mnBottom != rRect.mnBottom );
    }

    RectangleT&   operator += ( const PointT& rPt )
    {
        Move(rPt.X(), rPt.Y());
        return static_cast<RectangleT&>(*this);
    }
    RectangleT&   operator -= ( const PointT& rPt )
    {
        Move(-rPt.X(), -rPt.Y());
        return static_cast<RectangleT&>(*this);
    }

    RectangleT operator+( const Point& rPt ) const
    {
        RectangleT aTmp(mnLeft, mnTop, mnRight, mnBottom);
        aTmp += rPt;
        return aTmp;
    }
    RectangleT operator-( const Point& rPt ) const
    {
        RectangleT aTmp(mnLeft, mnTop, mnRight, mnBottom);
        aTmp -= rPt;
        return aTmp;
    }

    /**
     * Sanitizing variants for handling data from the outside
     */
    void                SaturatingSetSize(const SizeT& rSize) { RectangleTemplateBase::SaturatingSetSize(rSize); }

    // Scales relative to 0,0
    constexpr RectangleT scale(sal_Int64 nMulX, sal_Int64 nDivX,
                                            sal_Int64 nMulY, sal_Int64 nDivY) const
    {
        // 1. Create an empty rectangle with correct left and top
        RectangleT aRect(o3tl::convert(Left(), nMulX, nDivX),
                               o3tl::convert(Top(), nMulY, nDivY));
        // 2. If source has width/height, set respective right and bottom
        if (!IsWidthEmpty())
            aRect.SetRight(o3tl::convert(Right(), nMulX, nDivX));
        if (!IsHeightEmpty())
            aRect.SetBottom(o3tl::convert(Bottom(), nMulY, nDivY));
        return aRect;
    }
};

namespace tools
{
class SAL_WARN_UNUSED Rectangle final : public RectangleTemplate<Rectangle, Point, Size>
{
public:
    using RectangleTemplate::RectangleTemplate;
    // TODO remove this to find more issues
    constexpr Rectangle(const AbsoluteScreenPixelPoint& pt, const Size& sz) : RectangleTemplate(Point(pt.X(), pt.Y()), sz) {}
    // TODO remove this to find more issues
    constexpr Rectangle(const Point& pt, const AbsoluteScreenPixelSize& sz) : RectangleTemplate(pt, Size(sz.Width(), sz.Height())) {}
    // TODO remove this to find more issues
    constexpr explicit Rectangle(const AbsoluteScreenPixelRectangle & r);
};

} // namespace tools


// A rectangle relative to top-level screen, in screen pixels
class SAL_WARN_UNUSED AbsoluteScreenPixelRectangle : public RectangleTemplate<AbsoluteScreenPixelRectangle, AbsoluteScreenPixelPoint, AbsoluteScreenPixelSize> {
public:
    using RectangleTemplate::RectangleTemplate;
    // TODO remove
    constexpr explicit AbsoluteScreenPixelRectangle(const tools::Rectangle & r) : RectangleTemplate(r.mnLeft, r.mnTop, r.mnRight, r.mnBottom) {}
    // TODO remove
    constexpr AbsoluteScreenPixelRectangle(const AbsoluteScreenPixelPoint& pt, const Size& sz) : RectangleTemplate(pt, AbsoluteScreenPixelSize(sz.Width(), sz.Height())) {}
};

namespace tools
{
    constexpr Rectangle::Rectangle(const AbsoluteScreenPixelRectangle & r) : RectangleTemplate(r.mnLeft, r.mnTop, r.mnRight, r.mnBottom) {}
}

namespace o3tl
{

template <class RectangleT,
          std::enable_if_t<std::is_base_of_v<RectangleTemplate<RectangleT, typename RectangleT::PointType, typename RectangleT::SizeType>, RectangleT>, int> = 0>
constexpr auto convert(const RectangleT& rRectangle, o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto [num, den] = o3tl::getConversionMulDiv(eFrom, eTo);
    return rRectangle.scale(num, den, num, den);
}

} // end o3tl

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const RectangleTemplateBase& rectangle )
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.GetWidth() << 'x' << rectangle.GetHeight()
                      << "@(" << rectangle.getX() << ',' << rectangle.getY() << ")";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
