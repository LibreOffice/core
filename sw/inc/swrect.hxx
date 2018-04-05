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

class SvStream;

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
    inline SwRect( const SwRect &rRect );
    inline SwRect( const Point& rLT, const Size&  rSize );
    inline SwRect( const Point& rLT, const Point& rRB );
    inline SwRect( sal_Int64 X, sal_Int64 Y, sal_Int64 Width, sal_Int64 Height );

    //SV-SS e.g. SwRect( pWin->GetClipRect() );
    SwRect( const tools::Rectangle &rRect );

    //Set-Methods
    inline void Chg( const Point& rNP, const Size &rNS );
    inline void Pos(  const Point& rNew );
    inline void Pos( const sal_Int64 nNewX, const sal_Int64 nNewY );
    inline void SSize( const Size&  rNew  );
    inline void SSize( const sal_Int64 nHeight, const sal_Int64 nWidth );
    inline void Width(  sal_Int64 nNew );
    inline void Height( sal_Int64 nNew );
    inline void Left( const sal_Int64 nLeft );
    inline void Right( const sal_Int64 nRight );
    inline void Top( const sal_Int64 nTop );
    inline void Bottom( const sal_Int64 nBottom );

    //Get-Methods
    inline const Point &Pos()  const;
    inline const Size  &SSize() const;
    inline sal_Int64 Width()  const;
    inline sal_Int64 Height() const;
    inline sal_Int64 Left()   const;
    inline sal_Int64 Right()  const;
    inline sal_Int64 Top()    const;
    inline sal_Int64 Bottom() const;

    // In order to be able to access the members of Pos and SSize from the layout side.
    inline Point &Pos();
    inline Size  &SSize();

    Point Center() const;

    void Justify();

    SwRect &Union( const SwRect& rRect );
    SwRect &Intersection( const SwRect& rRect );

    // Same as Intersection, only assume that Rects are overlapping!
    SwRect &Intersection_( const SwRect &rRect );

    bool IsInside( const Point& rPOINT ) const;
    bool IsNear(const Point& rPoint, sal_Int64 nTolerance ) const;
    bool IsInside( const SwRect& rRect ) const;
    bool IsOver( const SwRect& rRect ) const;
    inline bool HasArea() const;
    inline bool IsEmpty() const;
    inline void Clear();

    inline SwRect &operator = ( const SwRect &rRect );

    inline bool operator == ( const SwRect& rRect ) const;
    inline bool operator != ( const SwRect& rRect ) const;

    inline SwRect &operator+=( const Point &rPt );
    inline SwRect &operator-=( const Point &rPt );

    //SV-SS e.g. pWin->DrawRect( aSwRect.SVRect() );
    inline tools::Rectangle  SVRect() const;

    // Output operator for debugging.
    friend SvStream& WriteSwRect( SvStream &rStream, const SwRect &rRect );


    void Top_(      const sal_Int64 nTop );
    void Bottom_(   const sal_Int64 nBottom );
    void Left_(     const sal_Int64 nLeft );
    void Right_(    const sal_Int64 nRight );
    void Width_(    const sal_Int64 nNew );
    void Height_(   const sal_Int64 nNew );
    sal_Int64 Top_()     const;
    sal_Int64 Bottom_()  const;
    sal_Int64 Left_()    const;
    sal_Int64 Right_()   const;
    sal_Int64 Width_()   const;
    sal_Int64 Height_()  const;
    void SubTop(    const sal_Int64 nSub );
    void AddBottom( const sal_Int64 nAdd );
    void SubLeft(   const sal_Int64 nSub );
    void AddRight(  const sal_Int64 nAdd );
    void AddWidth(  const sal_Int64 nAdd );
    void AddHeight( const sal_Int64 nAdd );
    void SetPosX(   const sal_Int64 nNew );
    void SetPosY(   const sal_Int64 nNew );
    void SetLeftAndWidth( sal_Int64 nLeft, sal_Int64 nNew );
    void SetTopAndHeight( sal_Int64 nTop, sal_Int64 nNew );
    void SetRightAndWidth( sal_Int64 nRight, sal_Int64 nNew );
    void SetBottomAndHeight( sal_Int64 nBottom, sal_Int64 nNew );
    void SetUpperLeftCorner(  const Point& rNew );
    void SetUpperRightCorner(  const Point& rNew );
    void SetLowerLeftCorner(  const Point& rNew );
    const Size  Size_() const;
    const Point TopLeft()  const;
    const Point TopRight()  const;
    const Point BottomLeft()  const;
    const Point BottomRight()  const;
    const Size  SwappedSize() const;
    sal_Int64 GetLeftDistance( sal_Int64 ) const;
    sal_Int64 GetBottomDistance( sal_Int64 ) const;
    sal_Int64 GetRightDistance( sal_Int64 ) const;
    sal_Int64 GetTopDistance( sal_Int64 ) const;
    bool OverStepLeft( sal_Int64 ) const;
    bool OverStepBottom( sal_Int64 ) const;
    bool OverStepTop( sal_Int64 ) const;
    bool OverStepRight( sal_Int64 ) const;
};

typedef void (SwRect:: *SwRectSet)( const sal_Int64 nNew );
typedef sal_Int64 (SwRect:: *SwRectGet)() const;
typedef const Point (SwRect:: *SwRectPoint)() const;
typedef const Size (SwRect:: *SwRectSize)() const;
typedef bool (SwRect:: *SwRectMax)( sal_Int64 ) const;
typedef sal_Int64 (SwRect:: *SwRectDist)( sal_Int64 ) const;
typedef void (SwRect:: *SwRectSetTwice)( sal_Int64, sal_Int64 );
typedef void (SwRect:: *SwRectSetPos)( const Point& );

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
inline void SwRect::Pos( const sal_Int64 nNewX, const sal_Int64 nNewY )
{
    m_Point.setX(nNewX);
    m_Point.setY(nNewY);
}
inline void SwRect::SSize( const Size&  rNew  )
{
    m_Size = rNew;
}
inline void SwRect::SSize( const sal_Int64 nNewHeight, const sal_Int64 nNewWidth )
{
    m_Size.setWidth(nNewWidth);
    m_Size.setHeight(nNewHeight);
}
inline void SwRect::Width(  sal_Int64 nNew )
{
    m_Size.setWidth(nNew);
}
inline void SwRect::Height( sal_Int64 nNew )
{
    m_Size.setHeight(nNew);
}
inline void SwRect::Left( const sal_Int64 nLeft )
{
    m_Size.AdjustWidth( m_Point.getX() - nLeft );
    m_Point.setX(nLeft);
}
inline void SwRect::Right( const sal_Int64 nRight )
{
    m_Size.setWidth(nRight - m_Point.getX() + 1);
}
inline void SwRect::Top( const sal_Int64 nTop )
{
    m_Size.AdjustHeight( m_Point.getY() - nTop );
    m_Point.setY(nTop);
}
inline void SwRect::Bottom( const sal_Int64 nBottom )
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
inline Size  &SwRect::SSize()
{
    return m_Size;
}
inline sal_Int64 SwRect::Width()  const
{
    return m_Size.Width();
}
inline sal_Int64 SwRect::Height() const
{
    return m_Size.Height();
}
inline sal_Int64 SwRect::Left()   const
{
    return m_Point.X();
}
inline sal_Int64 SwRect::Right()  const
{
    return m_Size.getWidth() ? m_Point.getX() + m_Size.getWidth() - 1 : m_Point.getX();
}
inline sal_Int64 SwRect::Top()    const
{
    return m_Point.Y();
}
inline sal_Int64 SwRect::Bottom() const
{
    return m_Size.getHeight() ? m_Point.getY() + m_Size.getHeight() - 1 : m_Point.getY();
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
    SAL_WARN_IF( IsEmpty(), "sw", "SVRect() without Width or Height" );
    return tools::Rectangle( m_Point.getX(), m_Point.getY(),
        m_Point.getX() + (m_Size.getWidth() - 1),         //Right()
        m_Point.getY() + (m_Size.getHeight() - 1) );      //Bottom()
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
inline SwRect::SwRect( const SwRect &rRect ) :
    m_Point( rRect.m_Point ),
    m_Size( rRect.m_Size )
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
inline SwRect::SwRect( sal_Int64 X, sal_Int64 Y, sal_Int64 W, sal_Int64 H ) :
    m_Point( X, Y ),
    m_Size( W, H )
{
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const SwRect& rectangle )
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.SSize()
                      << "@(" << rectangle.Pos() << ")";
}

#endif // INCLUDED_SW_INC_SWRECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
