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
#include <cstdlib>
#include <cassert>

class SvStream;
namespace rtl
{
    class OString;
}

enum TriState { TRISTATE_FALSE, TRISTATE_TRUE, TRISTATE_INDET };

/** the min/max values we can use without overflowing width/height calculations in Rectangle*/
#define RECT_MAX    (SAL_MAX_INT32/2)
#define RECT_MIN    (SAL_MIN_INT32/2)

// Pair

class SAL_WARN_UNUSED Pair
{
public:
                        Pair() : nA(0), nB(0) {}
                        Pair( sal_Int32 _nA, sal_Int32 _nB ) : nA(_nA), nB(_nB) {}

    sal_Int32           A() const { return nA; }
    sal_Int32           B() const { return nB; }

    sal_Int32&          A() { return nA; }
    sal_Int32&          B() { return nB; }

    TOOLS_DLLPUBLIC rtl::OString        toString() const;
    TOOLS_DLLPUBLIC friend SvStream&    ReadPair( SvStream& rIStream, Pair& rPair );
    TOOLS_DLLPUBLIC friend SvStream&    WritePair( SvStream& rOStream, const Pair& rPair );

protected:
      inline void checkRectFieldInRange(sal_Int32 x)
      {
          assert( (x <= RECT_MAX) && (x >= RECT_MIN) );
      }
    sal_Int32           nA;
    sal_Int32           nB;
};

namespace tools { namespace detail {

// Used to implement operator == for subclasses of Pair:
inline bool equal(Pair const & p1, Pair const & p2)
{
    return p1.A() == p2.A() && p1.B() == p2.B();
}

} }

// Point

class Size;
class SAL_WARN_UNUSED SAL_DLLPUBLIC_EXPORT Point final : protected Pair
{
public:
                        Point() {}
                        Point( sal_Int32 nX, sal_Int32 nY ) : Pair( nX, nY )
                        {
                            checkRectFieldInRange(nX);
                            checkRectFieldInRange(nY);
                        }

    sal_Int32           X() const { return nA; }
    sal_Int32           Y() const { return nB; }

    sal_Int32&          X() { return nA; }
    sal_Int32&          Y() { return nB; }

    void                Move( sal_Int32 nHorzMove, sal_Int32 nVertMove );
    void                Move( Size const & s );
    sal_Int32           AdjustX( sal_Int32 nHorzMove ) { nA += nHorzMove; return nA; }
    sal_Int32           AdjustY( sal_Int32 nVertMove ) { nB += nVertMove; return nB; }

    void                RotateAround( sal_Int32& rX, sal_Int32& rY, short nOrientation ) const;
    void                RotateAround( Point&, short nOrientation ) const;

    Point&              operator += ( const Point& rPoint );
    Point&              operator -= ( const Point& rPoint );
    Point&              operator *= ( const sal_Int32 nVal );
    Point&              operator /= ( const sal_Int32 nVal );

    friend inline Point operator+( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator-( const Point &rVal1, const Point &rVal2 );
    friend inline Point operator*( const Point &rVal1, const sal_Int32 nVal2 );
    friend inline Point operator/( const Point &rVal1, const sal_Int32 nVal2 );

    sal_Int32           getX() const { return X(); }
    sal_Int32           getY() const { return Y(); }
    void                setX(sal_Int32 nX)  { nA = nX; }
    void                setY(sal_Int32 nY)  { nB = nY; }

    Pair const &        toPair() const { return *this; }
    Pair &              toPair() { return *this; }

    using Pair::toString;
};

inline void Point::Move( sal_Int32 nHorzMove, sal_Int32 nVertMove )
{
    nA += nHorzMove;
    nB += nVertMove;
    checkRectFieldInRange(nA);
    checkRectFieldInRange(nB);
}

inline Point& Point::operator += ( const Point& rPoint )
{
    nA += rPoint.nA;
    nB += rPoint.nB;
    checkRectFieldInRange(nA);
    checkRectFieldInRange(nB);
    return *this;
}

inline Point& Point::operator -= ( const Point& rPoint )
{
    nA -= rPoint.nA;
    nB -= rPoint.nB;
    checkRectFieldInRange(nA);
    checkRectFieldInRange(nB);
    return *this;
}

inline Point& Point::operator *= ( const sal_Int32 nVal )
{
    nA *= nVal;
    nB *= nVal;
    checkRectFieldInRange(nA);
    checkRectFieldInRange(nB);
    return *this;
}

inline Point& Point::operator /= ( const sal_Int32 nVal )
{
    nA /= nVal;
    nB /= nVal;
    checkRectFieldInRange(nA);
    checkRectFieldInRange(nB);
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

inline Point operator*( const Point &rVal1, const sal_Int32 nVal2 )
{
    return Point( rVal1.nA*nVal2, rVal1.nB*nVal2 );
}

inline Point operator/( const Point &rVal1, const sal_Int32 nVal2 )
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
                    Size( sal_Int32 nWidth, sal_Int32 nHeight ) : Pair( nWidth, nHeight ) {}

    sal_Int32       Width() const  { return nA; }
    sal_Int32       Height() const { return nB; }

    sal_Int32       AdjustWidth( sal_Int32 n ) { nA += n; return nA; }
    sal_Int32       AdjustHeight( sal_Int32 n ) { nB += n; return nB; }

    sal_Int32       getWidth() const { return Width(); }
    sal_Int32       getHeight() const { return Height(); }
    void            setWidth(sal_Int32 nWidth)  { nA = nWidth; }
    void            setHeight(sal_Int32 nHeight)  { nB = nHeight; }

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

#define RANGE_MAX   SAL_MAX_INT32

class SAL_WARN_UNUSED Range final : protected Pair
{
public:
                    Range() {}
                    Range( sal_Int32 nMin, sal_Int32 nMax ) : Pair( nMin, nMax )
                    {
                        checkRectFieldInRange(nA);
                        checkRectFieldInRange(nB);
                    }

    sal_Int32       Min() const { return nA; }
    sal_Int32       Max() const { return nB; }
    sal_Int32       Len() const { return nB - nA + 1; }

    sal_Int32&      Min() { return nA; }
    sal_Int32&      Max() { return nB; }

    bool            IsInside( sal_Int32 nIs ) const;

    void            Justify();

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Range::IsInside( sal_Int32 nIs ) const
{
    return ((nA <= nIs) && (nIs <= nB ));
}

inline void Range::Justify()
{
    if ( nA > nB )
    {
        sal_Int32 nHelp = nA;
        nA = nB;
        nB = nHelp;
        checkRectFieldInRange(nA);
        checkRectFieldInRange(nB);
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

#define SELECTION_MIN   SAL_MIN_INT32
#define SELECTION_MAX   SAL_MAX_INT32

class SAL_WARN_UNUSED Selection final : protected Pair
{
public:
                    Selection() {}
                    Selection( sal_Int32 nPos ) : Pair( nPos, nPos ) {}
                    Selection( sal_Int32 nMin, sal_Int32 nMax ) : Pair( nMin, nMax ) {}

    sal_Int32       Min() const { return nA; }
    sal_Int32       Max() const { return nB; }
    sal_Int32       Len() const { return nB - nA; }

    sal_Int32&      Min() { return nA; }
    sal_Int32&      Max() { return nB; }

    bool            IsInside( sal_Int32 nIs ) const;

    void            Justify();

    bool            operator !() const { return !Len(); }

    sal_Int32       getMin() const { return Min(); }
    void            setMin(sal_Int32 nMin)  { Min() = nMin; }
    void            setMax(sal_Int32 nMax)  { Max() = nMax; }

    Pair const &    toPair() const { return *this; }
    Pair &          toPair() { return *this; }

    using Pair::toString;
};

inline bool Selection::IsInside( sal_Int32 nIs ) const
{
    return ((nA <= nIs) && (nIs < nB ));
}

inline void Selection::Justify()
{
    if ( nA > nB )
    {
        sal_Int32 nHelp = nA;
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
namespace details
{
    inline void checkRectFieldInRange(sal_Int32 x)
    {
        assert( (x <= RECT_MAX) && (x >= RECT_MIN) );
    }
} // details namespace

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Rectangle final
{
    static constexpr sal_Int32 RECT_EMPTY = -32767;
public:
                        Rectangle();
                        Rectangle( const Point& rLT, const Point& rRB );
                        Rectangle( sal_Int32 nLeft, sal_Int32 nTop,
                                   sal_Int32 nRight, sal_Int32 nBottom );
    /// Constructs an empty Rectangle, with top/left at the specified params
                        Rectangle( sal_Int32 nLeft, sal_Int32 nTop );
                        Rectangle( const Point& rLT, const Size& rSize );

    sal_Int32           Left() const    { return nLeft;   }
    sal_Int32           Right() const   { return nRight;  }
    sal_Int32           Top() const     { return nTop;    }
    sal_Int32           Bottom() const  { return nBottom; }

    void                SetLeft(sal_Int32 v)    { nLeft = v;   }
    void                SetRight(sal_Int32 v)   { nRight = v;  }
    void                SetTop(sal_Int32 v)     { nTop = v;    }
    void                SetBottom(sal_Int32 v)  { nBottom = v; }

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
    inline void         Move( sal_Int32 nHorzMoveDelta, sal_Int32 nVertMoveDelta );
    void                Move( Size const & s ) { Move(s.Width(), s.Height()); }
    sal_Int32           AdjustLeft( sal_Int32 nHorzMoveDelta ) { nLeft += nHorzMoveDelta; return nLeft; }
    sal_Int32           AdjustRight( sal_Int32 nHorzMoveDelta ) { nRight += nHorzMoveDelta; return nRight; }
    sal_Int32           AdjustTop( sal_Int32 nVertMoveDelta ) { nTop += nVertMoveDelta; return nTop; }
    sal_Int32           AdjustBottom( sal_Int32 nVertMoveDelta ) { nBottom += nVertMoveDelta; return nBottom; }

    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );
    inline Size         GetSize() const;

    /// Returns the difference between right and left, assuming the range is inclusive.
    inline sal_Int32    GetWidth() const;
    /// Returns the difference between bottom and top, assuming the range is inclusive.
    inline sal_Int32    GetHeight() const;

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

    TOOLS_DLLPUBLIC friend SvStream&    ReadRectangle( SvStream& rIStream, tools::Rectangle& rRect );
    TOOLS_DLLPUBLIC friend SvStream&    WriteRectangle( SvStream& rOStream, const tools::Rectangle& rRect );

    // ONE
    sal_Int32           getX() const { return nLeft; }
    sal_Int32           getY() const { return nTop; }
    /// Returns the difference between right and left, assuming the range includes one end, but not the other.
    sal_Int32           getWidth() const { return nRight - nLeft; }
    /// Returns the difference between bottom and top, assuming the range includes one end, but not the other.
    sal_Int32           getHeight() const { return nBottom - nTop; }
    /// Set the left edge of the rectangle to x, preserving the width
    void                setX( sal_Int32 x ) { nRight  += x - nLeft; nLeft = x; checkFieldsInRange(); }
    /// Set the top edge of the rectangle to y, preserving the height
    void                setY( sal_Int32 y ) { nBottom += y - nTop;  nTop  = y; checkFieldsInRange(); }
    void                setWidth( sal_Int32 n ) { nRight = nLeft + n; checkFieldsInRange(); }
    void                setHeight( sal_Int32 n ) { nBottom = nTop + n; checkFieldsInRange(); }
    /// Returns the string representation of the rectangle, format is "x, y, width, height".
    rtl::OString        toString() const;

    /**
     * Expands the rectangle in all directions by the input value.
     */
    inline void expand(sal_Int32 nExpandBy);
    inline void shrink(sal_Int32 nShrinkBy);

    /**
     * Sanitizing variants for handling data from the outside
     */
    void                SaturatingSetSize(const Size& rSize);

private:
    void checkFieldsInRange()
    {
        details::checkRectFieldInRange(nLeft);
        details::checkRectFieldInRange(nTop);
        details::checkRectFieldInRange(nRight);
        details::checkRectFieldInRange(nBottom);
    }
    sal_Int32           nLeft;
    sal_Int32           nTop;
    sal_Int32           nRight;
    sal_Int32           nBottom;
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
    checkFieldsInRange();
}

inline tools::Rectangle::Rectangle( sal_Int32 _nLeft,  sal_Int32 _nTop,
                             sal_Int32 _nRight, sal_Int32 _nBottom )
{
    nLeft   = _nLeft;
    nTop    = _nTop;
    nRight  = _nRight;
    nBottom = _nBottom;
    checkFieldsInRange();
}

inline tools::Rectangle::Rectangle( sal_Int32 _nLeft,  sal_Int32 _nTop )
{
    nLeft   = _nLeft;
    nTop    = _nTop;
    nRight = nBottom = RECT_EMPTY;
    checkFieldsInRange();
}

inline tools::Rectangle::Rectangle( const Point& rLT, const Size& rSize )
{
    nLeft   = rLT.X();
    nTop    = rLT.Y();
    nRight  = rSize.Width()  ? nLeft+(rSize.Width()-1) : RECT_EMPTY;
    nBottom = rSize.Height() ? nTop+(rSize.Height()-1) : RECT_EMPTY;
    checkFieldsInRange();
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

inline void tools::Rectangle::Move( sal_Int32 nHorzMove, sal_Int32 nVertMove )
{
    nLeft += nHorzMove;
    nTop  += nVertMove;
    if ( nRight != RECT_EMPTY )
        nRight += nHorzMove;
    if ( nBottom != RECT_EMPTY )
        nBottom += nVertMove;
    checkFieldsInRange();
}

inline void tools::Rectangle::SetPos( const Point& rPoint )
{
    if ( nRight != RECT_EMPTY )
        nRight += rPoint.X() - nLeft;
    if ( nBottom != RECT_EMPTY )
        nBottom += rPoint.Y() - nTop;
    nLeft = rPoint.X();
    nTop  = rPoint.Y();
    checkFieldsInRange();
}

inline sal_Int32 tools::Rectangle::GetWidth() const
{
    sal_Int32 n;
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

inline sal_Int32 tools::Rectangle::GetHeight() const
{
    sal_Int32 n;
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
    checkFieldsInRange();
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
    checkFieldsInRange();
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

inline void tools::Rectangle::expand(sal_Int32 nExpandBy)
{
    nLeft   -= nExpandBy;
    nTop    -= nExpandBy;
    nRight  += nExpandBy;
    nBottom += nExpandBy;
    checkFieldsInRange();
}

inline void tools::Rectangle::shrink(sal_Int32 nShrinkBy)
{
    nLeft   += nShrinkBy;
    nTop    += nShrinkBy;
    nRight  -= nShrinkBy;
    nBottom -= nShrinkBy;
    checkFieldsInRange();
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

inline SvStream& ReadPair( SvStream& rIStream, Point& v ) { return ReadPair(rIStream, v.toPair()); }
inline SvStream& WritePair( SvStream& rOStream, const Point& v ) { return WritePair(rOStream, v.toPair()); }
inline SvStream& ReadPair( SvStream& rIStream, Size& v ) { return ReadPair(rIStream, v.toPair()); }
inline SvStream& WritePair( SvStream& rOStream, const Size& v ) { return WritePair(rOStream, v.toPair()); }
inline SvStream& ReadPair( SvStream& rIStream, Range& v ) { return ReadPair(rIStream, v.toPair()); }
inline SvStream& WritePair( SvStream& rOStream, const Range& v ) { return WritePair(rOStream, v.toPair()); }
inline SvStream& ReadPair( SvStream& rIStream, Selection& v ) { return ReadPair(rIStream, v.toPair()); }
inline SvStream& WritePair( SvStream& rOStream, const Selection& v ) { return WritePair(rOStream, v.toPair()); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
