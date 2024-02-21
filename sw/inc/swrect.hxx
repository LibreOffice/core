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

#include <ostream>

#include <sal/log.hxx>
#include <tools/gen.hxx>
#include "swdllapi.h"

class SvStream;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

/// *Of course* Writer needs its own rectangles.
/// This is half-open so m_Point.X() + m_Size.getWidth() is *not* included.
/// Note the tools Rectangle is (usually? sometimes?) closed so there's a
/// SVRect() to subtract 1 for the conversion.
class SAL_WARN_UNUSED SwRect
{
    Point m_Point;
    Size m_Size;

public:
    inline SwRect();
    inline SwRect( const SwRect &rRect ) = default;
    inline SwRect( const Point& rLT, const Size&  rSize );
    inline SwRect( const Point& rLT, const Point& rRB );
    inline SwRect( tools::Long X, tools::Long Y, tools::Long Width, tools::Long Height );

    //SV-SS e.g. SwRect( pWin->GetClipRect() );
    explicit SwRect( const tools::Rectangle &rRect );

    //Set-Methods
    inline void Chg( const Point& rNP, const Size &rNS );
    inline void Pos(  const Point& rNew );
    inline void Pos( const tools::Long nNewX, const tools::Long nNewY );
    inline void SSize( const Size&  rNew  );
    inline void SSize( const tools::Long nHeight, const tools::Long nWidth );
    inline void Width(  tools::Long nNew );
    inline void Height( tools::Long nNew );
    inline void Left( const tools::Long nLeft );
    inline void Right( const tools::Long nRight );
    inline void Top( const tools::Long nTop );
    inline void Bottom( const tools::Long nBottom );

    //Get-Methods
    inline const Point &Pos()  const;
    inline const Size  &SSize() const;
    inline tools::Long Width()  const;
    inline tools::Long Height() const;
    inline tools::Long Left()   const;
    inline tools::Long Right()  const;
    inline tools::Long Top()    const;
    inline tools::Long Bottom() const;

    // In order to be able to access the members of Pos and SSize from the layout side.
    inline Point &Pos();

    Point Center() const;

    void Justify();

    SwRect &Union( const SwRect& rRect );
    SwRect &Intersection( const SwRect& rRect );

    SwRect GetUnion( const SwRect& rRect ) const;
    SwRect GetIntersection( const SwRect& rRect ) const;

    // Same as Intersection, only assume that Rects are overlapping!
    SwRect &Intersection_( const SwRect &rRect );

    bool Contains( const Point& rPOINT ) const;
    bool Contains( const SwRect& rRect ) const;
    bool Overlaps( const SwRect& rRect ) const;
    bool IsNear(const Point& rPoint, tools::Long nTolerance ) const;
    inline bool HasArea() const;
    inline bool IsEmpty() const;
    inline void Clear();

    SwRect &operator = ( const SwRect &rRect ) = default;

    inline bool operator == ( const SwRect& rRect ) const;
    inline bool operator != ( const SwRect& rRect ) const;

    inline SwRect &operator+=( const Point &rPt );
    inline SwRect &operator-=( const Point &rPt );

    //SV-SS e.g. pWin->DrawRect( aSwRect.SVRect() );
    inline tools::Rectangle  SVRect() const;

    // Output operator for debugging.
    friend SvStream& WriteSwRect( SvStream &rStream, const SwRect &rRect );
    void dumpAsXmlAttributes(xmlTextWriterPtr writer) const;

    void Top_(      const tools::Long nTop );
    void Bottom_(   const tools::Long nBottom );
    void Left_(     const tools::Long nLeft );
    void Right_(    const tools::Long nRight );
    void Width_(    const tools::Long nNew );
    void Height_(   const tools::Long nNew );
    tools::Long Top_()     const;
    tools::Long Bottom_()  const;
    tools::Long Left_()    const;
    tools::Long Right_()   const;
    tools::Long Width_()   const;
    tools::Long Height_()  const;
    void SubTop(    const tools::Long nSub );
    void AddTop(    const tools::Long nAdd );
    void AddBottom( const tools::Long nAdd );
    void AddLeft(   const tools::Long nAdd );
    void SubLeft(   const tools::Long nSub );
    void AddRight(  const tools::Long nAdd );
    void AddWidth(  const tools::Long nAdd );
    void AddHeight( const tools::Long nAdd );
    void SetPosX(   const tools::Long nNew );
    void SetPosY(   const tools::Long nNew );
    void SetLeftAndWidth( tools::Long nLeft, tools::Long nNew );
    void SetTopAndHeight( tools::Long nTop, tools::Long nNew );
    void SetRightAndWidth( tools::Long nRight, tools::Long nNew );
    void SetBottomAndHeight( tools::Long nBottom, tools::Long nNew );
    void SetUpperLeftCorner(  const Point& rNew );
    void SetUpperRightCorner(  const Point& rNew );
    void SetLowerLeftCorner(  const Point& rNew );
    Size  Size_() const;
    Point TopLeft()  const;
    Point TopRight()  const;
    Point BottomLeft()  const;
    Point BottomRight()  const;
    Size  SwappedSize() const;
    tools::Long GetLeftDistance( tools::Long ) const;
    tools::Long GetBottomDistance( tools::Long ) const;
    tools::Long GetRightDistance( tools::Long ) const;
    tools::Long GetTopDistance( tools::Long ) const;
    bool OverStepLeft( tools::Long ) const;
    bool OverStepBottom( tools::Long ) const;
    bool OverStepTop( tools::Long ) const;
    bool OverStepRight( tools::Long ) const;
};

typedef void (SwRect::*SwRectSet)( const tools::Long nNew );
typedef tools::Long (SwRect::*SwRectGet)() const;
typedef Point (SwRect::*SwRectPoint)() const;
typedef Size (SwRect::*SwRectSize)() const;
typedef bool (SwRect::*SwRectMax)( tools::Long ) const;
typedef tools::Long (SwRect::*SwRectDist)( tools::Long ) const;
typedef void (SwRect::*SwRectSetTwice)( tools::Long, tools::Long );
typedef void (SwRect::*SwRectSetPos)( const Point& );

//  Set-Methods
inline void SwRect::Chg( const Point& rNP, const Size &rNS )
{
    m_Point = rNP;
    m_Size = rNS;
}
inline void SwRect::Pos(  const Point& rNew )
{
    m_Point = rNew;
}
inline void SwRect::Pos( const tools::Long nNewX, const tools::Long nNewY )
{
    m_Point.setX(nNewX);
    m_Point.setY(nNewY);
}
inline void SwRect::SSize( const Size&  rNew  )
{
    m_Size = rNew;
}
inline void SwRect::SSize( const tools::Long nNewHeight, const tools::Long nNewWidth )
{
    m_Size.setWidth(nNewWidth);
    m_Size.setHeight(nNewHeight);
}
inline void SwRect::Width(  tools::Long nNew )
{
    m_Size.setWidth(nNew);
}
inline void SwRect::Height( tools::Long nNew )
{
    m_Size.setHeight(nNew);
}
inline void SwRect::Left( const tools::Long nLeft )
{
    m_Size.AdjustWidth( m_Point.getX() - nLeft );
    m_Point.setX(nLeft);
}
inline void SwRect::Right( const tools::Long nRight )
{
    m_Size.setWidth(nRight - m_Point.getX() + 1);
}
inline void SwRect::Top( const tools::Long nTop )
{
    m_Size.AdjustHeight( m_Point.getY() - nTop );
    m_Point.setY(nTop);
}
inline void SwRect::Bottom( const tools::Long nBottom )
{
    m_Size.setHeight(nBottom - m_Point.getY() + 1);
}

// Get-Methods
inline const Point &SwRect::Pos()  const
{
    return m_Point;
}
inline Point &SwRect::Pos()
{
    return m_Point;
}
inline const Size  &SwRect::SSize() const
{
    return m_Size;
}
inline tools::Long SwRect::Width()  const
{
    return m_Size.Width();
}
inline tools::Long SwRect::Height() const
{
    return m_Size.Height();
}
inline tools::Long SwRect::Left()   const
{
    return m_Point.X();
}
inline tools::Long SwRect::Right()  const
{
    return m_Size.getWidth() ? m_Point.getX() + m_Size.getWidth() - 1 : m_Point.getX();
}
inline tools::Long SwRect::Top()    const
{
    return m_Point.Y();
}
inline tools::Long SwRect::Bottom() const
{
    return m_Size.getHeight() ? m_Point.getY() + m_Size.getHeight() - 1 : m_Point.getY();
}

inline Point SwRect::TopLeft() const
{
    return Point( Left(), Top());
}
inline Point SwRect::TopRight() const
{
    return Point( Right(), Top());
}
inline Point SwRect::BottomLeft() const
{
    return Point( Left(), Bottom());
}
inline Point SwRect::BottomRight() const
{
    return Point( Right(), Bottom());
}

inline bool SwRect::operator == ( const SwRect& rRect ) const
{
    return (m_Point == rRect.m_Point && m_Size == rRect.m_Size);
}
inline bool SwRect::operator != ( const SwRect& rRect ) const
{
    return (m_Point != rRect.m_Point || m_Size != rRect.m_Size);
}

inline SwRect &SwRect::operator+=( const Point &rPt )
{
    m_Point += rPt;
    return *this;
}
inline SwRect &SwRect::operator-=( const Point &rPt )
{
    m_Point -= rPt;
    return *this;
}

// other
inline tools::Rectangle SwRect::SVRect() const
{
    SAL_INFO_IF( IsEmpty(), "sw.core", "SVRect() without Width or Height" );
    return tools::Rectangle( m_Point.getX(), m_Point.getY(),
        m_Point.getX() + m_Size.getWidth() - 1,         //Right()
        m_Point.getY() + m_Size.getHeight() - 1 );      //Bottom()
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
    m_Point.setX(0);
    m_Point.setY(0);
    m_Size.setWidth(0);
    m_Size.setHeight(0);
}

// constructors
inline SwRect::SwRect() :
    m_Point( 0, 0 ),
    m_Size( 0, 0 )
{
}
inline SwRect::SwRect( const Point& rLT, const Size&  rSize ) :
    m_Point( rLT ),
    m_Size( rSize )
{
}
inline SwRect::SwRect( const Point& rLT, const Point& rRB ) :
    m_Point( rLT ),
    m_Size( rRB.X() - rLT.X() + 1, rRB.Y() - rLT.Y() + 1 )
{
}
inline SwRect::SwRect( tools::Long X, tools::Long Y, tools::Long W, tools::Long H ) :
    m_Point( X, Y ),
    m_Size( W, H )
{
}

inline Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );
}

inline bool SwRect::Contains( const SwRect& rRect ) const
{
    const tools::Long nRight  = Right();
    const tools::Long nBottom = Bottom();
    const tools::Long nrRight = rRect.Right();
    const tools::Long nrBottom= rRect.Bottom();
    return (Left() <= rRect.Left()) && (rRect.Left()<= nRight)  &&
           (Left() <= nrRight)      && (nrRight     <= nRight)  &&
           (Top()  <= rRect.Top())  && (rRect.Top() <= nBottom) &&
           (Top()  <= nrBottom)     && (nrBottom    <= nBottom);
}

inline bool SwRect::Contains( const Point& rPoint ) const
{
    return (Left()  <= rPoint.X()) &&
           (Top()   <= rPoint.Y()) &&
           (Right() >= rPoint.X()) &&
           (Bottom()>= rPoint.Y());
}

// mouse moving of table borders
inline bool SwRect::IsNear( const Point& rPoint, tools::Long nTolerance ) const
{
    bool bIsNearby = (((Left()   - nTolerance) <= rPoint.X()) &&
                      ((Top()    - nTolerance) <= rPoint.Y()) &&
                      ((Right()  + nTolerance) >= rPoint.X()) &&
                      ((Bottom() + nTolerance) >= rPoint.Y()));
    return Contains(rPoint) || bIsNearby;
}

inline bool SwRect::Overlaps( const SwRect& rRect ) const
{
    return (Top()   <= rRect.Bottom()) &&
           (Left()  <= rRect.Right())  &&
           (Right() >= rRect.Left())   &&
           (Bottom()>= rRect.Top());
}

inline SwRect SwRect::GetUnion( const SwRect& rRect ) const
{
    return IsEmpty() ? rRect : rRect.IsEmpty() ? *this : SwRect(
        Point( std::min( Left(), rRect.Left()),
               std::min( Top(), rRect.Top())),
        Point( std::max( Right(), rRect.Right()),
               std::max( Bottom(), rRect.Bottom())));
}

inline SwRect SwRect::GetIntersection( const SwRect& rRect ) const
{
    return Overlaps( rRect )
        ? SwRect(
            Point( std::max( Left(), rRect.Left()),
                   std::max( Top(), rRect.Top())),
            Point( std::min( Right(), rRect.Right()),
                   std::min( Bottom(), rRect.Bottom())))
        : SwRect();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
