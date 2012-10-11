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
#ifndef _SV_GEN_HXX
#define _SV_GEN_HXX

#include "tools/toolsdllapi.h"
#include <tools/solar.h>

#include <limits.h>

class SvStream;

// Pair

class SAL_WARN_UNUSED Pair
{
public:
    long                nA;
    long                nB;

                        Pair();
                        Pair( long nA, long nB );

    long                A() const { return nA; }
    long                B() const { return nB; }

    long&               A() { return nA; }
    long&               B() { return nB; }

    sal_Bool                operator == ( const Pair& rPair ) const;
    sal_Bool                operator != ( const Pair& rPair ) const;

    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, Pair& rPair );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const Pair& rPair );
};

inline Pair::Pair()
{
    nA = nB = 0;
}

inline Pair::Pair( long _nA, long _nB )
{
    Pair::nA = _nA;
    Pair::nB = _nB;
}

inline sal_Bool Pair::operator == ( const Pair& rPair ) const
{
    return ((nA == rPair.nA) && (nB == rPair.nB));
}

inline sal_Bool Pair::operator != ( const Pair& rPair ) const
{
    return ((nA != rPair.nA) || (nB != rPair.nB));
}

// Point

class SAL_WARN_UNUSED Point : public Pair
{
public:
                        Point();
                        Point( long nX, long nY );

    long                X() const { return nA; }
    long                Y() const { return nB; }

    long&               X() { return nA; }
    long&               Y() { return nB; }

    void                Move( long nHorzMove, long nVertMove );
    sal_Bool                IsAbove( const Point& rPoint ) const;
    sal_Bool                IsBelow( const Point& rPoint ) const;
    sal_Bool                IsLeft( const Point& rPoint ) const;
    sal_Bool                IsRight( const Point& rPoint ) const;

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
    void                setX(long nX)  { X() = nX; }
    void                setY(long nY)  { Y() = nY; }
};

inline Point::Point()
{
}

inline Point::Point( long nX, long nY ) : Pair( nX, nY )
{
}

inline void Point::Move( long nHorzMove, long nVertMove )
{
    nA += nHorzMove;
    nB += nVertMove;
}

inline sal_Bool Point::IsAbove( const Point& rPoint ) const
{
    return (nB > rPoint.nB);
}

inline sal_Bool Point::IsBelow( const Point& rPoint ) const
{
    return (nB < rPoint.nB);
}

inline sal_Bool Point::IsLeft( const Point& rPoint ) const
{
    return (nA < rPoint.nA);
}

inline sal_Bool Point::IsRight( const Point& rPoint ) const
{
    return (nA > rPoint.nA);
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

// Size

class SAL_WARN_UNUSED Size : public Pair
{
public:
                    Size();
                    Size( long nWidth, long nHeight );

    long            Width() const  { return nA; }
    long            Height() const { return nB; }

    long&           Width()  { return nA; }
    long&           Height() { return nB; }

    long                getWidth() const { return Width(); }
    long                getHeight() const { return Height(); }
    void                setWidth(long nWidth)  { Width() = nWidth; }
    void                setHeight(long nHeight)  { Height() = nHeight; }
};

inline Size::Size()
{
}

inline Size::Size( long nWidth, long nHeight ) :
                Pair( nWidth, nHeight )
{
}

// Range

#define RANGE_MAX   LONG_MAX

class SAL_WARN_UNUSED Range : public Pair
{
public:
                    Range();
                    Range( long nMin, long nMax );

    long            Min() const { return nA; }
    long            Max() const { return nB; }
    long            Len() const { return nB - nA + 1; }

    long&           Min() { return nA; }
    long&           Max() { return nB; }

    sal_Bool            IsInside( long nIs ) const;

    void            Justify();
};

inline Range::Range()
{
}

inline Range::Range( long nMin, long nMax ) : Pair( nMin, nMax )
{
}

inline sal_Bool Range::IsInside( long nIs ) const
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

// Selection

#define SELECTION_MIN   LONG_MIN
#define SELECTION_MAX   LONG_MAX

class SAL_WARN_UNUSED Selection : public Pair
{
public:
                    Selection();
                    Selection( long nPos );
                    Selection( long nMin, long nMax );

    long            Min() const { return nA; }
    long            Max() const { return nB; }
    long            Len() const { return nB - nA; }

    long&           Min() { return nA; }
    long&           Max() { return nB; }

    sal_Bool            IsInside( long nIs ) const;

    void            Justify();

    sal_Bool            operator !() const { return !Len(); }

    long                getMin() const { return Min(); }
    long                getMax() const { return Max(); }
    void                setMin(long nMin)  { Min() = nMin; }
    void                setMax(long nMax)  { Max() = nMax; }
};

inline Selection::Selection()
{
}

inline Selection::Selection( long nPos ) : Pair( nPos, nPos )
{
}

inline Selection::Selection( long nMin, long nMax ) :
           Pair( nMin, nMax )
{
}

inline sal_Bool Selection::IsInside( long nIs ) const
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

// Rectangle

#define RECT_EMPTY  ((short)-32767)

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Rectangle
{
public:
    long                nLeft;
    long                nTop;
    long                nRight;
    long                nBottom;

                        Rectangle();
                        Rectangle( const Point& rLT, const Point& rRB );
                        Rectangle( long nLeft, long nTop,
                                   long nRight, long nBottom );
                        Rectangle( const Point& rLT, const Size& rSize );

    long                Left() const    { return nLeft;   }
    long                Right() const   { return nRight;  }
    long                Top() const     { return nTop;    }
    long                Bottom() const  { return nBottom; }

    long&               Left()          { return nLeft;   }
    long&               Right()         { return nRight;  }
    long&               Top()           { return nTop;    }
    long&               Bottom()        { return nBottom; }

    inline Point                TopLeft() const;
    Point               TopRight() const;
    Point               TopCenter() const;
    Point               BottomLeft() const;
    Point               BottomRight() const;
    Point               BottomCenter() const;
    Point               LeftCenter() const;
    Point               RightCenter() const;
    Point               Center() const;

    void                Move( long nHorzMove, long nVertMove );
    inline void         Transpose();
    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );
    inline Size         GetSize() const;

    long                GetWidth() const;
    long                GetHeight() const;

    Rectangle&          Union( const Rectangle& rRect );
    Rectangle&          Intersection( const Rectangle& rRect );
    Rectangle           GetUnion( const Rectangle& rRect ) const;
    Rectangle           GetIntersection( const Rectangle& rRect ) const;

    void                Justify();

    sal_Bool                IsInside( const Point& rPOINT ) const;
    sal_Bool                IsInside( const Rectangle& rRect ) const;
    sal_Bool                IsOver( const Rectangle& rRect ) const;

    void                SetEmpty() { nRight = nBottom = RECT_EMPTY; }
    sal_Bool                IsEmpty() const;

    sal_Bool                operator == ( const Rectangle& rRect ) const;
    sal_Bool                operator != ( const Rectangle& rRect ) const;

    Rectangle&          operator += ( const Point& rPt );
    Rectangle&          operator -= ( const Point& rPt );

    friend inline Rectangle operator + ( const Rectangle& rRect, const Point& rPt );
    friend inline Rectangle operator - ( const Rectangle& rRect, const Point& rPt );

    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, Rectangle& rRect );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const Rectangle& rRect );

    // ONE
    long                getX() const { return nLeft; }
    long                getY() const { return nTop; }
    long                getWidth() const { return nRight - nLeft; }
    long                getHeight() const { return nBottom - nTop; }
    void                setX( long n ) { nRight += n-nLeft; nLeft = n; }
    void                setY( long n ) { nBottom += n-nTop; nTop = n; }
    void                setWidth( long n ) { nRight = nLeft + n; }
    void                setHeight( long n ) { nBottom = nTop + n; }
};

inline Rectangle::Rectangle()
{
    nLeft = nTop = 0;
    nRight = nBottom = RECT_EMPTY;
}

inline Rectangle::Rectangle( const Point& rLT, const Point& rRB )
{
    nLeft   = rLT.X();
    nTop    = rLT.Y();
    nRight  = rRB.X();
    nBottom = rRB.Y();
}

inline Rectangle::Rectangle( long _nLeft,  long _nTop,
                             long _nRight, long _nBottom )
{
    nLeft   = _nLeft;
    nTop    = _nTop;
    nRight  = _nRight;
    nBottom = _nBottom;
}

inline Rectangle::Rectangle( const Point& rLT, const Size& rSize )
{
    nLeft   = rLT.X();
    nTop    = rLT.Y();
    nRight  = rSize.Width()  ? nLeft+rSize.Width()-1 : RECT_EMPTY;
    nBottom = rSize.Height() ? nTop+rSize.Height()-1 : RECT_EMPTY;
}

inline sal_Bool Rectangle::IsEmpty() const
{
    return ((nRight == RECT_EMPTY) || (nBottom == RECT_EMPTY));
}

inline Point Rectangle::TopLeft() const
{
    return Point( nLeft, nTop );
}

inline Point Rectangle::TopRight() const
{
    return Point( (nRight == RECT_EMPTY) ? nLeft : nRight, nTop );
}

inline Point Rectangle::BottomLeft() const
{
    return Point( nLeft, (nBottom == RECT_EMPTY) ? nTop : nBottom );
}

inline Point Rectangle::BottomRight() const
{
    return Point( (nRight  == RECT_EMPTY) ? nLeft : nRight,
                  (nBottom == RECT_EMPTY) ? nTop  : nBottom );
}

inline Point Rectangle::TopCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( Min( nLeft, nRight ) + Abs( (nRight - nLeft)/2 ),
                      Min( nTop,  nBottom) );
}

inline Point Rectangle::BottomCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( Min( nLeft, nRight ) + Abs( (nRight - nLeft)/2 ),
                      Max( nTop,  nBottom) );
}

inline Point Rectangle::LeftCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( Min( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
}

inline Point Rectangle::RightCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( Max( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
}

inline Point Rectangle::Center() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( nLeft+(nRight-nLeft)/2 , nTop+(nBottom-nTop)/2 );
}

inline void Rectangle::Move( long nHorzMove, long nVertMove )
{
    nLeft += nHorzMove;
    nTop  += nVertMove;
    if ( nRight != RECT_EMPTY )
        nRight += nHorzMove;
    if ( nBottom != RECT_EMPTY )
        nBottom += nVertMove;
}

void Rectangle::Transpose()
{
    if ( !IsEmpty() )
    {
        long swap( nLeft );
        nLeft = nTop;
        nTop = swap;

        swap = nRight;
        nRight = nBottom;
        nBottom = swap;
    }
}

inline void Rectangle::SetPos( const Point& rPoint )
{
    if ( nRight != RECT_EMPTY )
        nRight += rPoint.X() - nLeft;
    if ( nBottom != RECT_EMPTY )
        nBottom += rPoint.Y() - nTop;
    nLeft = rPoint.X();
    nTop  = rPoint.Y();
}

inline long Rectangle::GetWidth() const
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

inline long Rectangle::GetHeight() const
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

inline Size Rectangle::GetSize() const
{
    return Size( GetWidth(), GetHeight() );
}

inline Rectangle Rectangle::GetUnion( const Rectangle& rRect ) const
{
    Rectangle aTmpRect( *this );
    return aTmpRect.Union( rRect );
}

inline Rectangle Rectangle::GetIntersection( const Rectangle& rRect ) const
{
    Rectangle aTmpRect( *this );
    return aTmpRect.Intersection( rRect );
}

inline sal_Bool Rectangle::operator == ( const Rectangle& rRect ) const
{
    return ((nLeft   == rRect.nLeft   ) &&
            (nTop    == rRect.nTop    ) &&
            (nRight  == rRect.nRight  ) &&
            (nBottom == rRect.nBottom ));
}

inline sal_Bool Rectangle::operator != ( const Rectangle& rRect ) const
{
    return ((nLeft   != rRect.nLeft   ) ||
            (nTop    != rRect.nTop    ) ||
            (nRight  != rRect.nRight  ) ||
            (nBottom != rRect.nBottom ));
}

inline Rectangle& Rectangle::operator +=( const Point& rPt )
{
    nLeft += rPt.X();
    nTop  += rPt.Y();
    if ( nRight != RECT_EMPTY )
        nRight += rPt.X();
    if ( nBottom != RECT_EMPTY )
        nBottom += rPt.Y();
    return *this;
}

inline Rectangle& Rectangle::operator -= ( const Point& rPt )
{
    nLeft -= rPt.X();
    nTop  -= rPt.Y();
    if ( nRight != RECT_EMPTY )
        nRight -= rPt.X();
    if ( nBottom != RECT_EMPTY )
        nBottom -= rPt.Y();
    return *this;
}

inline Rectangle operator + ( const Rectangle& rRect, const Point& rPt )
{
    Rectangle aRect( rRect.nLeft  + rPt.X(), rRect.nTop    + rPt.Y(),
                     (rRect.nRight  == RECT_EMPTY) ? RECT_EMPTY : rRect.nRight + rPt.X(),
                     (rRect.nBottom == RECT_EMPTY) ? RECT_EMPTY : rRect.nBottom + rPt.Y() );
    return aRect;
}

inline Rectangle operator - ( const Rectangle& rRect, const Point& rPt )
{
    Rectangle aRect( rRect.nLeft - rPt.X(),
                     rRect.nTop - rPt.Y(),
                     (rRect.nRight  == RECT_EMPTY) ? RECT_EMPTY : rRect.nRight - rPt.X(),
                     (rRect.nBottom == RECT_EMPTY) ? RECT_EMPTY : rRect.nBottom - rPt.Y() );
    return aRect;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
