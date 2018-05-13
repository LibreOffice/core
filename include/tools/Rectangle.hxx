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

#ifndef INCLUDED_TOOLS_RECTANGLE_HXX
#define INCLUDED_TOOLS_RECTANGLE_HXX

#include <tools/toolsdllapi.h>
#include <tools/Pair.hxx>
#include <tools/gen.hxx>

#include <limits.h>
#include <algorithm>
#include <ostream>
#include <cstdlib>

#define RECT_MAX LONG_MAX
#define RECT_MIN LONG_MIN

namespace tools
{
/**
    Note: this class is a true marvel of engineering: because the author
    could not decide whether it's better to have a closed or half-open
    interval, they just implemented *both* in the same class!

    If you have the misfortune of having to use this class, don't immediately
    despair but first take note that the uppercase GetWidth() / GetHeight()
    etc. methods interpret the interval as closed, while the lowercase
    getWidth() / getHeight() etc. methods interpret the interval as half-open.
    Ok, now is the time for despair.
 */
class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Rectangle final
{
    static constexpr short RECT_EMPTY = -32767;

public:
    Rectangle();
    Rectangle(const Point& rLT, const Point& rRB);
    Rectangle(long mnLeft, long mnTop, long mnRight, long mnBottom);
    /** Constructs an empty Rectangle, with top/left at the specified params
     */
    Rectangle(long mnLeft, long mnTop);
    Rectangle(const Point& rLT, const Size& rSize);

    long Left() const { return mnLeft; }
    long Right() const { return mnRight; }
    long Top() const { return mnTop; }
    long Bottom() const { return mnBottom; }

    void SetLeft(long v) { mnLeft = v; }
    void SetRight(long v) { mnRight = v; }
    void SetTop(long v) { mnTop = v; }
    void SetBottom(long v) { mnBottom = v; }

    inline Point TopLeft() const;
    inline Point TopRight() const;
    inline Point TopCenter() const;
    inline Point BottomLeft() const;
    inline Point BottomRight() const;
    inline Point BottomCenter() const;
    inline Point LeftCenter() const;
    inline Point RightCenter() const;
    inline Point Center() const;

    /** Move the top and left edges by a delta, preserving width and height

        @param nHorzMoveDelta delta to move horizontally
        @param nVertMoveDelta delta to move vertically
     */
    inline void Move(long nHorzMoveDelta, long nVertMoveDelta);

    /** Move the top and left edges by a delta, preserving width and height

        @param s size to horizontally and vertically
     */
    void Move(Size const& s) { Move(s.Width(), s.Height()); }

    /** Adjust the left edge out by increased delta

        @param nHorzMoveDelta left delta
     */
    long AdjustLeft(long nHorzMoveDelta)
    {
        mnLeft += nHorzMoveDelta;
        return mnLeft;
    }

    /** Adjust the right edge out by increased delta

        @param nHorzMoveDelta right delta
     */
    long AdjustRight(long nHorzMoveDelta)
    {
        mnRight += nHorzMoveDelta;
        return mnRight;
    }

    /** Adjust the top edge out by increased delta

        @param nHorzMoveDelta top delta
     */
    long AdjustTop(long nVertMoveDelta)
    {
        mnTop += nVertMoveDelta;
        return mnTop;
    }

    /** Adjust the bottom edge out by increased delta

        @param nHorzMoveDelta bottom delta
     */
    long AdjustBottom(long nVertMoveDelta)
    {
        mnBottom += nVertMoveDelta;
        return mnBottom;
    }

    /** Move the rectangle, top left corner to position

        @param rPoint top left corner's new position
     */
    inline void SetPos(const Point& rPoint);

    /** Increase the size of the rectangle

        @param rSize size to increase rectangle
     */
    void SetSize(const Size& rSize);

    /** Get size of rectangle

        @returns rectangle size
     */
    inline Size GetSize() const;

    /** Returns the difference between right and left, assuming the range is inclusive.

        @returns rectangle width
     */
    inline long GetWidth() const;

    /** Returns the difference between bottom and top, assuming the range is inclusive.

       @returns rectangle height
     */
    inline long GetHeight() const;

    /** Make this rectangle be the union of this rectangles and another rectangle to make a
        larger rectangle

        @attention if one rectangle is empty, then return the non-empty rectangle,
            otherwise if the two rectangles do not intersect returns an empty rectangle
            and makes this rectangle empty

        @param rRect 2nd rectangle with which to form union

        @returns rectangle that forms the union of the two rectangles
     */
    Rectangle& Union(const Rectangle& rRect);

    /** Make this rectangle be the intersection of this rectangle and another rectangle

        @attention if one rectangle is empty, then return the non-empty rectangle,
            otherwise if the two rectangles do not intersect returns an empty rectangle
            and makes this rectangle empty

        @param rRect 2nd rectangle with which to intersect

        @returns rectangle that forms the intersection of the two rectangles, or empty
            rectangle if they do not intersect
     */
    Rectangle& Intersection(const Rectangle& rRect);

    /** Return the union of this rectangles and another rectangle to make a larger rectangle

        @attention if one rectangle is empty, then return the non-empty rectangle,
            otherwise if the two rectangles do not intersect returns an empty rectangle

        @param rRect 2nd rectangle with which to form union

        @returns rectangle that forms the union of the two rectangles
     */
    inline Rectangle GetUnion(const Rectangle& rRect) const;

    /** Return the intersection of this rectangle and another rectangle

        @attention if one rectangle is empty, then return the non-empty rectangle,
            otherwise if the two rectangles do not intersect returns an empty rectangle

        @param rRect 2nd rectangle with which to intersect

        @returns rectangle that forms the intersection of the two rectangles, or empty
            rectangle if they do not intersect
     */
    inline Rectangle GetIntersection(const Rectangle& rRect) const;

    /** Make sure that left side is less than right side, and top is less than bottom
     */
    void Justify();

    /** Test whether a point is inside this rectangle

        @param point to test

        @retval true point is in rectangle
        @retval false point is not in rectangle
     */
    bool IsInside(const Point& rPOINT) const;

    /** Test whether a rectangle is fully inside this rectangle

        @param rectangle to test

        @retval true rectangle is fully inside this rectangle
        @retval false rectangle is not fully inside this rectangle (may partially overlap)
     */
    bool IsInside(const Rectangle& rRect) const;

    /** Test whether a rectangle overlaps this rectangle

        @param point to test

        @retval true rectangle overlaps this rectangle
        @retval false rectangle does not overlap this rectangle
     */
    bool IsOver(const Rectangle& rRect) const;

    void SetEmpty() { mnRight = mnBottom = RECT_EMPTY; }
    void SetWidthEmpty() { mnRight = RECT_EMPTY; }
    void SetHeightEmpty() { mnBottom = RECT_EMPTY; }
    inline bool IsEmpty() const;
    bool IsWidthEmpty() const { return mnRight == RECT_EMPTY; }
    bool IsHeightEmpty() const { return mnBottom == RECT_EMPTY; }

    inline bool operator==(const Rectangle& rRect) const;
    inline bool operator!=(const Rectangle& rRect) const;

    inline Rectangle& operator+=(const Point& rPt);
    inline Rectangle& operator-=(const Point& rPt);

    friend inline Rectangle operator+(const Rectangle& rRect, const Point& rPt);
    friend inline Rectangle operator-(const Rectangle& rRect, const Point& rPt);

    TOOLS_DLLPUBLIC friend SvStream& ReadRectangle(SvStream& rIStream, Rectangle& rRect);
    TOOLS_DLLPUBLIC friend SvStream& WriteRectangle(SvStream& rOStream, const Rectangle& rRect);

    long getX() const { return mnLeft; }
    long getY() const { return mnTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    long getWidth() const { return mnRight - mnLeft; }
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    long getHeight() const { return mnBottom - mnTop; }
    /// Set the left edge of the rectangle to x, preserving the width
    void setX(long x)
    {
        mnRight += x - mnLeft;
        mnLeft = x;
    }

    /// Set the top edge of the rectangle to y, preserving the height
    void setY(long y)
    {
        mnBottom += y - mnTop;
        mnTop = y;
    }

    void setWidth(long n) { mnRight = mnLeft + n; }
    void setHeight(long n) { mnBottom = mnTop + n; }
    /// Returns the string representation of the rectangle, format is "x, y, width, height".
    rtl::OString toString() const;

    /// Expands the rectangle in all directions by the input value.
    inline void expand(long nExpandBy);
    inline void shrink(long nShrinkBy);

    /// Sanitizing variants for handling data from the outside
    void SaturatingSetSize(const Size& rSize);
    void SaturatingSetX(long x);
    void SaturatingSetY(long y);

private:
    long mnLeft;
    long mnTop;
    long mnRight;
    long mnBottom;
};

inline Rectangle::Rectangle()
{
    mnLeft = mnTop = 0;
    mnRight = mnBottom = RECT_EMPTY;
}

inline Rectangle::Rectangle(const Point& rLT, const Point& rRB)
{
    mnLeft = rLT.X();
    mnTop = rLT.Y();
    mnRight = rRB.X();
    mnBottom = rRB.Y();
}

inline Rectangle::Rectangle(long _mnLeft, long _mnTop, long _mnRight, long _mnBottom)
{
    mnLeft = _mnLeft;
    mnTop = _mnTop;
    mnRight = _mnRight;
    mnBottom = _mnBottom;
}

inline Rectangle::Rectangle(long _mnLeft, long _mnTop)
{
    mnLeft = _mnLeft;
    mnTop = _mnTop;
    mnRight = mnBottom = RECT_EMPTY;
}

inline Rectangle::Rectangle(const Point& rLT, const Size& rSize)
{
    mnLeft = rLT.X();
    mnTop = rLT.Y();
    mnRight = rSize.Width() ? mnLeft + (rSize.Width() - 1) : RECT_EMPTY;
    mnBottom = rSize.Height() ? mnTop + (rSize.Height() - 1) : RECT_EMPTY;
}

inline bool Rectangle::IsEmpty() const
{
    return (mnRight == RECT_EMPTY) || (mnBottom == RECT_EMPTY);
}

inline Point Rectangle::TopLeft() const { return Point(mnLeft, mnTop); }

inline Point Rectangle::TopRight() const
{
    return Point((mnRight == RECT_EMPTY) ? mnLeft : mnRight, mnTop);
}

inline Point Rectangle::BottomLeft() const
{
    return Point(mnLeft, (mnBottom == RECT_EMPTY) ? mnTop : mnBottom);
}

inline Point Rectangle::BottomRight() const
{
    return Point((mnRight == RECT_EMPTY) ? mnLeft : mnRight,
                 (mnBottom == RECT_EMPTY) ? mnTop : mnBottom);
}

inline Point Rectangle::TopCenter() const
{
    if (IsEmpty())
        return Point(mnLeft, mnTop);
    else
        return Point(std::min(mnLeft, mnRight) + std::abs((mnRight - mnLeft) / 2),
                     std::min(mnTop, mnBottom));
}

inline Point Rectangle::BottomCenter() const
{
    if (IsEmpty())
        return Point(mnLeft, mnTop);
    else
        return Point(std::min(mnLeft, mnRight) + std::abs((mnRight - mnLeft) / 2),
                     std::max(mnTop, mnBottom));
}

inline Point Rectangle::LeftCenter() const
{
    if (IsEmpty())
        return Point(mnLeft, mnTop);
    else
        return Point(std::min(mnLeft, mnRight), mnTop + (mnBottom - mnTop) / 2);
}

inline Point Rectangle::RightCenter() const
{
    if (IsEmpty())
        return Point(mnLeft, mnTop);
    else
        return Point(std::max(mnLeft, mnRight), mnTop + (mnBottom - mnTop) / 2);
}

inline Point Rectangle::Center() const
{
    if (IsEmpty())
        return Point(mnLeft, mnTop);
    else
        return Point(mnLeft + (mnRight - mnLeft) / 2, mnTop + (mnBottom - mnTop) / 2);
}

inline void Rectangle::Move(long nHorzMove, long nVertMove)
{
    mnLeft += nHorzMove;
    mnTop += nVertMove;
    if (mnRight != RECT_EMPTY)
        mnRight += nHorzMove;
    if (mnBottom != RECT_EMPTY)
        mnBottom += nVertMove;
}

inline void Rectangle::SetPos(const Point& rPoint)
{
    if (mnRight != RECT_EMPTY)
        mnRight += rPoint.X() - mnLeft;

    if (mnBottom != RECT_EMPTY)
        mnBottom += rPoint.Y() - mnTop;

    mnLeft = rPoint.X();
    mnTop = rPoint.Y();
}

inline long Rectangle::GetWidth() const
{
    long n;
    if (mnRight == RECT_EMPTY)
        n = 0;
    else
    {
        n = mnRight - mnLeft;
        if (n < 0)
            n--;
        else
            n++;
    }

    return n;
}

inline long Rectangle::GetHeight() const
{
    long n;
    if (mnBottom == RECT_EMPTY)
        n = 0;
    else
    {
        n = mnBottom - mnTop;
        if (n < 0)
            n--;
        else
            n++;
    }

    return n;
}

inline Size Rectangle::GetSize() const { return Size(GetWidth(), GetHeight()); }

inline Rectangle Rectangle::GetUnion(const Rectangle& rRect) const
{
    Rectangle aTmpRect(*this);
    return aTmpRect.Union(rRect);
}

inline Rectangle Rectangle::GetIntersection(const Rectangle& rRect) const
{
    Rectangle aTmpRect(*this);
    return aTmpRect.Intersection(rRect);
}

inline bool Rectangle::operator==(const Rectangle& rRect) const
{
    return (mnLeft == rRect.mnLeft) && (mnTop == rRect.mnTop) && (mnRight == rRect.mnRight)
           && (mnBottom == rRect.mnBottom);
}

inline bool Rectangle::operator!=(const Rectangle& rRect) const
{
    return (mnLeft != rRect.mnLeft) || (mnTop != rRect.mnTop) || (mnRight != rRect.mnRight)
           || (mnBottom != rRect.mnBottom);
}

inline Rectangle& Rectangle::operator+=(const Point& rPt)
{
    mnLeft += rPt.X();
    mnTop += rPt.Y();
    if (mnRight != RECT_EMPTY)
        mnRight += rPt.X();
    if (mnBottom != RECT_EMPTY)
        mnBottom += rPt.Y();
    return *this;
}

inline Rectangle& Rectangle::operator-=(const Point& rPt)
{
    mnLeft -= rPt.X();
    mnTop -= rPt.Y();
    if (mnRight != RECT_EMPTY)
        mnRight -= rPt.X();
    if (mnBottom != RECT_EMPTY)
        mnBottom -= rPt.Y();
    return *this;
}

inline Rectangle operator+(const Rectangle& rRect, const Point& rPt)
{
    return rRect.IsEmpty() ? Rectangle(rRect.mnLeft + rPt.X(), rRect.mnTop + rPt.Y())
                           : Rectangle(rRect.mnLeft + rPt.X(), rRect.mnTop + rPt.Y(),
                                       rRect.mnRight + rPt.X(), rRect.mnBottom + rPt.Y());
}

inline Rectangle operator-(const Rectangle& rRect, const Point& rPt)
{
    return rRect.IsEmpty() ? Rectangle(rRect.mnLeft - rPt.X(), rRect.mnTop - rPt.Y())
                           : Rectangle(rRect.mnLeft - rPt.X(), rRect.mnTop - rPt.Y(),
                                       rRect.mnRight - rPt.X(), rRect.mnBottom - rPt.Y());
}

inline void Rectangle::expand(long nExpandBy)
{
    mnLeft -= nExpandBy;
    mnTop -= nExpandBy;
    mnRight += nExpandBy;
    mnBottom += nExpandBy;
}

inline void Rectangle::shrink(long nShrinkBy)
{
    mnLeft += nShrinkBy;
    mnTop += nShrinkBy;
    mnRight -= nShrinkBy;
    mnBottom -= nShrinkBy;
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const Rectangle& rectangle)
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.getWidth() << 'x' << rectangle.getHeight() << "@("
                      << rectangle.getX() << ',' << rectangle.getY() << ")";
}
} // namespace tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
