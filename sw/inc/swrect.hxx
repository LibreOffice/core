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
#ifndef INCLUDED_SW_INC_SWRECT_HXX
#define INCLUDED_SW_INC_SWRECT_HXX

#include <ostream>

#include <sal/log.hxx>
#include <tools/gen.hxx>
#include <tools/UnitConversion.hxx>
#include "swdllapi.h"
#include "twips.hxx"

class SvStream;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

class SAL_WARN_UNUSED SW_DLLPUBLIC SwPoint
{
    SwTwips m_X, m_Y;
public:
    SwPoint() : m_X(0), m_Y(0) {}
    SwPoint(SwTwips x, SwTwips y) : m_X(x), m_Y(y) {}
    explicit SwPoint(const Point & pt) : m_X(pt.getX()), m_Y(pt.getY()) {}
    void setX(SwTwips x) { m_X = x; }
    void setY(SwTwips y) { m_Y = y; }
    SwTwips getX() const { return m_X; }
    SwTwips getY() const { return m_Y; }
    SwTwips X() const { return m_X; }
    SwTwips Y() const { return m_Y; }
    bool operator==(SwPoint const & rhs) const { return m_X == rhs.m_X && m_Y == rhs.m_Y; }
    bool operator!=(SwPoint const & rhs) const { return !operator==(rhs); }
    SwPoint& operator+=(SwPoint pt) { m_X += pt.m_X; m_Y += pt.m_Y; return *this; }
    SwPoint& operator-=(SwPoint pt) { m_X -= pt.m_X; m_Y -= pt.m_Y; return *this; }
    explicit operator Point() const { return Point(tools::Long(m_X), tools::Long(m_Y)); }
    void Move( SwTwips nHorzMove, SwTwips nVertMove ) { m_X += nHorzMove; m_Y += nVertMove; }
    void AdjustX( SwTwips nMove ) { m_X += nMove; }
    void AdjustY( SwTwips nMove ) { m_Y += nMove; }
};

inline SwPoint operator+(const SwPoint & lhs, const SwPoint & rhs)
{
    return SwPoint(lhs.X() + rhs.X(), lhs.Y() + rhs.Y());
}

inline SwPoint operator-(const SwPoint & lhs, const SwPoint & rhs)
{
    return SwPoint(lhs.X() - rhs.X(), lhs.Y() - rhs.Y());
}

class SAL_WARN_UNUSED SW_DLLPUBLIC SwSize
{
    SwTwips m_Width, m_Height;
public:
    SwSize() : m_Width(0), m_Height(0) {}
    SwSize(SwTwips width, SwTwips height) : m_Width(width), m_Height(height) {}
    explicit SwSize(Size sz) : m_Width(sz.Width()), m_Height(sz.Height()) {}
    void setWidth(SwTwips w) { m_Width = w; }
    void setHeight(SwTwips h) { m_Height = h; }
    void AdjustWidth(SwTwips w) { m_Width += w; }
    void AdjustHeight(SwTwips h) { m_Height += h; }
    SwTwips getWidth() const { return m_Width; }
    SwTwips getHeight() const { return m_Height; }
    SwTwips Width() const { return m_Width; }
    SwTwips Height() const { return m_Height; }
    bool operator==(SwSize const & rhs) const { return m_Width == rhs.m_Width && m_Height == rhs.m_Height; }
    bool operator!=(SwSize const & rhs) const { return !operator==(rhs); }
    explicit operator Size() const { return Size(tools::Long(m_Width), tools::Long(m_Height)); }
    bool IsEmpty() const { return m_Width == SwTwips(0) || m_Height == SwTwips(0); }
};

/// *Of course* Writer needs its own rectangles.
/// This is half-open so m_Point.X() + m_Size.getWidth() is *not* included.
/// Note the tools Rectangle is (usually? sometimes?) closed so there's a
/// SVRect() to subtract 1 for the conversion.
class SAL_WARN_UNUSED SW_DLLPUBLIC SwRect
{
    SwPoint m_Point;
    SwSize m_Size;

public:
    inline SwRect();
    inline SwRect( const SwRect &rRect );
    inline SwRect( const SwPoint& rLT, const SwSize&  rSize );
    inline SwRect( const SwPoint& rLT, const SwPoint& rRB );
    inline SwRect( SwTwips X, SwTwips Y, SwTwips Width, SwTwips Height );

    //SV-SS e.g. SwRect( pWin->GetClipRect() );
    explicit SwRect( const tools::Rectangle &rRect );

    //Set-Methods
    inline void Chg( const SwPoint& rNP, const SwSize &rNS );
    inline void Pos(  const SwPoint& rNew );
    inline void Pos( const SwTwips nNewX, const SwTwips nNewY );
    inline void SSize( const SwSize& rNew  );
    inline void SSize( const SwTwips nHeight, const SwTwips nWidth );
    inline void Width(  SwTwips nNew );
    inline void Height( SwTwips nNew );
    inline void Left( const SwTwips nLeft );
    inline void Right( const SwTwips nRight );
    inline void Top( const SwTwips nTop );
    inline void Bottom( const SwTwips nBottom );

    void AdjustLeft( SwTwips nMove ) { m_Point.AdjustX(nMove); }
    void AdjustRight( SwTwips nMove ) { m_Size.AdjustWidth(nMove); }
    void AdjustTop( SwTwips nMove ) { m_Point.AdjustY(nMove); }
    void AdjustBottom( SwTwips nMove ) { m_Size.AdjustHeight(nMove); }

    //Get-Methods
    inline const SwPoint &Pos()  const;
    inline const SwSize  &SSize() const;
    inline SwTwips Width()  const;
    inline SwTwips Height() const;
    inline SwTwips Left()   const;
    inline SwTwips Right()  const;
    inline SwTwips Top()    const;
    inline SwTwips Bottom() const;

    // In order to be able to access the members of Pos and SSize from the layout side.
    inline SwPoint &Pos();

    SwPoint Center() const;

    void Justify();

    SwRect &Union( const SwRect& rRect );
    SwRect &Intersection( const SwRect& rRect );

    // Same as Intersection, only assume that Rects are overlapping!
    SwRect &Intersection_( const SwRect &rRect );

    bool IsInside( const SwPoint& rPOINT ) const;
    bool IsNear(const SwPoint& rPoint, SwTwips nTolerance ) const;
    bool IsInside( const SwRect& rRect ) const;
    bool IsOver( const SwRect& rRect ) const;
    inline bool HasArea() const;
    inline bool IsEmpty() const;
    inline void Clear();

    inline SwRect &operator = ( const SwRect &rRect );

    inline bool operator == ( const SwRect& rRect ) const;
    inline bool operator != ( const SwRect& rRect ) const;

    inline SwRect &operator+=( const SwPoint &rPt );
    inline SwRect &operator-=( const SwPoint &rPt );

    //SV-SS e.g. pWin->DrawRect( aSwRect.SVRect() );
    inline tools::Rectangle  SVRect() const;

    // Output operator for debugging.
    friend SvStream& WriteSwRect( SvStream &rStream, const SwRect &rRect );
    void dumpAsXmlAttributes(xmlTextWriterPtr writer) const;

    void Top_(      const SwTwips nTop );
    void Bottom_(   const SwTwips nBottom );
    void Left_(     const SwTwips nLeft );
    void Right_(    const SwTwips nRight );
    void Width_(    const SwTwips nNew );
    void Height_(   const SwTwips nNew );
    SwTwips Top_()     const;
    SwTwips Bottom_()  const;
    SwTwips Left_()    const;
    SwTwips Right_()   const;
    SwTwips Width_()   const;
    SwTwips Height_()  const;
    void SubTop(    const SwTwips nSub );
    void AddTop(    const SwTwips nAdd );
    void AddBottom( const SwTwips nAdd );
    void AddLeft(   const SwTwips nAdd );
    void SubLeft(   const SwTwips nSub );
    void AddRight(  const SwTwips nAdd );
    void AddWidth(  const SwTwips nAdd );
    void AddHeight( const SwTwips nAdd );
    void SetPosX(   const SwTwips nNew );
    void SetPosY(   const SwTwips nNew );
    void SetLeftAndWidth( SwTwips nLeft, SwTwips nNew );
    void SetTopAndHeight( SwTwips nTop, SwTwips nNew );
    void SetRightAndWidth( SwTwips nRight, SwTwips nNew );
    void SetBottomAndHeight( SwTwips nBottom, SwTwips nNew );
    void SetUpperLeftCorner(  const SwPoint& rNew );
    void SetUpperRightCorner(  const SwPoint& rNew );
    void SetLowerLeftCorner(  const SwPoint& rNew );
    SwSize  Size_() const;
    SwPoint TopLeft()  const;
    SwPoint TopRight()  const;
    SwPoint BottomLeft()  const;
    SwPoint BottomRight()  const;
    SwSize  SwappedSize() const;
    SwTwips GetLeftDistance( SwTwips ) const;
    SwTwips GetBottomDistance( SwTwips ) const;
    SwTwips GetRightDistance( SwTwips ) const;
    SwTwips GetTopDistance( SwTwips ) const;
    bool OverStepLeft( SwTwips ) const;
    bool OverStepBottom( SwTwips ) const;
    bool OverStepTop( SwTwips ) const;
    bool OverStepRight( SwTwips ) const;
};

typedef void (SwRect::*SwRectSet)( const SwTwips nNew );
typedef SwTwips (SwRect::*SwRectGet)() const;
typedef SwPoint (SwRect::*SwRectPoint)() const;
typedef SwSize (SwRect::*SwRectSize)() const;
typedef bool (SwRect::*SwRectMax)( SwTwips ) const;
typedef SwTwips (SwRect::*SwRectDist)( SwTwips ) const;
typedef void (SwRect::*SwRectSetTwice)( SwTwips, SwTwips );
typedef void (SwRect::*SwRectSetPos)( const SwPoint& );

//  Set-Methods
inline void SwRect::Chg( const SwPoint& rNP, const SwSize &rNS )
{
    m_Point = rNP;
    m_Size = rNS;
}
inline void SwRect::Pos(  const SwPoint& rNew )
{
    m_Point = rNew;
}
inline void SwRect::Pos( const SwTwips nNewX, const SwTwips nNewY )
{
    m_Point.setX(nNewX);
    m_Point.setY(nNewY);
}
inline void SwRect::SSize( const SwSize&  rNew  )
{
    m_Size = rNew;
}
inline void SwRect::SSize( const SwTwips nNewHeight, const SwTwips nNewWidth )
{
    m_Size.setWidth(nNewWidth);
    m_Size.setHeight(nNewHeight);
}
inline void SwRect::Width(  SwTwips nNew )
{
    m_Size.setWidth(nNew);
}
inline void SwRect::Height( SwTwips nNew )
{
    m_Size.setHeight(nNew);
}
inline void SwRect::Left( const SwTwips nLeft )
{
    m_Size.AdjustWidth( m_Point.getX() - nLeft );
    m_Point.setX(nLeft);
}
inline void SwRect::Right( const SwTwips nRight )
{
    m_Size.setWidth(nRight - m_Point.getX() + SwTwips(1));
}
inline void SwRect::Top( const SwTwips nTop )
{
    m_Size.AdjustHeight( m_Point.getY() - nTop );
    m_Point.setY(nTop);
}
inline void SwRect::Bottom( const SwTwips nBottom )
{
    m_Size.setHeight(nBottom - m_Point.getY() + SwTwips(1));
}

// Get-Methods
inline const SwPoint &SwRect::Pos()  const
{
    return m_Point;
}
inline SwPoint &SwRect::Pos()
{
    return m_Point;
}
inline const SwSize  &SwRect::SSize() const
{
    return m_Size;
}
inline SwTwips SwRect::Width()  const
{
    return m_Size.Width();
}
inline SwTwips SwRect::Height() const
{
    return m_Size.Height();
}
inline SwTwips SwRect::Left()   const
{
    return m_Point.X();
}
inline SwTwips SwRect::Right()  const
{
    return m_Size.getWidth() ? m_Point.getX() + m_Size.getWidth() - SwTwips(1) : m_Point.getX();
}
inline SwTwips SwRect::Top()    const
{
    return m_Point.Y();
}
inline SwTwips SwRect::Bottom() const
{
    return m_Size.getHeight() ? m_Point.getY() + m_Size.getHeight() - SwTwips(1) : m_Point.getY();
}

// operators
inline SwRect &SwRect::operator = ( const SwRect &rRect )
{
    m_Point = rRect.m_Point;
    m_Size = rRect.m_Size;
    return *this;
}
inline bool SwRect::operator == ( const SwRect& rRect ) const
{
    return (m_Point == rRect.m_Point && m_Size == rRect.m_Size);
}
inline bool SwRect::operator != ( const SwRect& rRect ) const
{
    return (m_Point != rRect.m_Point || m_Size != rRect.m_Size);
}

inline SwRect &SwRect::operator+=( const SwPoint &rPt )
{
    m_Point += rPt;
    return *this;
}
inline SwRect &SwRect::operator-=( const SwPoint &rPt )
{
    m_Point -= rPt;
    return *this;
}

// other
inline tools::Rectangle SwRect::SVRect() const
{
    SAL_WARN_IF( IsEmpty(), "sw", "SVRect() without Width or Height" );
    return tools::Rectangle( tools::Long(m_Point.getX()), tools::Long(m_Point.getY()),
        tools::Long(m_Point.getX() + m_Size.getWidth() - SwTwips(1)),         //Right()
        tools::Long(m_Point.getY() + m_Size.getHeight() - SwTwips(1)) );      //Bottom()
}

inline bool SwRect::HasArea() const
{
    return !IsEmpty();
}
inline bool SwRect::IsEmpty() const
{
    return !(m_Size.getHeight() && m_Size.getWidth());
}
inline void SwRect::Clear()
{
    m_Point.setX(SwTwips(0));
    m_Point.setY(SwTwips(0));
    m_Size.setWidth(SwTwips(0));
    m_Size.setHeight(SwTwips(0));
}

// constructors
inline SwRect::SwRect() :
    m_Point( SwTwips(0), SwTwips(0) ),
    m_Size( SwTwips(0), SwTwips(0) )
{
}
inline SwRect::SwRect( const SwRect &rRect ) :
    m_Point( rRect.m_Point ),
    m_Size( rRect.m_Size )
{
}
inline SwRect::SwRect( const SwPoint& rLT, const SwSize&  rSize ) :
    m_Point( rLT ),
    m_Size( rSize )
{
}
inline SwRect::SwRect( const SwPoint& rLT, const SwPoint& rRB ) :
    m_Point( rLT ),
    m_Size( rRB.X() - rLT.X() + SwTwips(1), rRB.Y() - rLT.Y() + SwTwips(1) )
{
}
inline SwRect::SwRect( SwTwips X, SwTwips Y, SwTwips W, SwTwips H ) :
    m_Point( X, Y ),
    m_Size( W, H )
{
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const SwRect& rectangle )
{
    if (rectangle.IsEmpty())
        stream << "EMPTY:";
    return stream << rectangle.SSize()
                  << "@(" << rectangle.Pos() << ")";
}

// specialise the template in tools/UnitConversion.hxx
template <> constexpr auto convertTwipToMm100(SwTwips n)
{
    return o3tl::convert(tools::Long(n), o3tl::Length::twip, o3tl::Length::mm100);
}
template <> constexpr auto convertMm100ToTwip(tools::Long n)
{
    return SwTwips(o3tl::convert(n, o3tl::Length::mm100, o3tl::Length::twip));
}
template <> constexpr auto convertMm100ToTwip(sal_Int32 n)
{
    return SwTwips(o3tl::convert(n, o3tl::Length::mm100, o3tl::Length::twip));
}

#endif // INCLUDED_SW_INC_SWRECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
