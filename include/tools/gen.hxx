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
#include <ostream>
#include <cstdlib>

class SvStream;

enum TriState { TRISTATE_FALSE, TRISTATE_TRUE, TRISTATE_INDET };

// Pair

class SAL_WARN_UNUSED Pair
{
public:
                        Pair();
                        Pair( long nA, long nB );

    long                A() const { return nA; }
    long                B() const { return nB; }

    long&               A() { return nA; }
    long&               B() { return nB; }

    bool                operator == ( const Pair& rPair ) const;
    bool                operator != ( const Pair& rPair ) const;

    TOOLS_DLLPUBLIC friend SvStream&    ReadPair( SvStream& rIStream, Pair& rPair );
    TOOLS_DLLPUBLIC friend SvStream&    WritePair( SvStream& rOStream, const Pair& rPair );

protected:
    long                nA;
    long                nB;
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

inline bool Pair::operator == ( const Pair& rPair ) const
{
    return ((nA == rPair.nA) && (nB == rPair.nB));
}

inline bool Pair::operator != ( const Pair& rPair ) const
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
    bool                IsAbove( const Point& rPoint ) const;
    bool                IsBelow( const Point& rPoint ) const;
    bool                IsLeft( const Point& rPoint ) const;
    bool                IsRight( const Point& rPoint ) const;

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

inline bool Point::IsAbove( const Point& rPoint ) const
{
    return (nB > rPoint.nB);
}

inline bool Point::IsBelow( const Point& rPoint ) const
{
    return (nB < rPoint.nB);
}

inline bool Point::IsLeft( const Point& rPoint ) const
{
    return (nA < rPoint.nA);
}

inline bool Point::IsRight( const Point& rPoint ) const
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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Point& point )
{
    return stream << point.X() << ',' << point.Y();
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

    long            getWidth() const { return Width(); }
    long            getHeight() const { return Height(); }
    void            setWidth(long nWidth)  { Width() = nWidth; }
    void            setHeight(long nHeight)  { Height() = nHeight; }
};

inline Size::Size()
{
}

inline Size::Size( long nWidth, long nHeight ) :
                Pair( nWidth, nHeight )
{
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Size& size )
{
    return stream << size.Width() << 'x' << size.Height();
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

    bool            IsInside( long nIs ) const;

    void            Justify();
};

inline Range::Range()
{
}

inline Range::Range( long nMin, long nMax ) : Pair( nMin, nMax )
{
}

inline bool Range::IsInside( long nIs ) const
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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Range& range )
{
    return stream << range.Min() << '-' << range.Max();
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

    bool            IsInside( long nIs ) const;

    void            Justify();

    bool            operator !() const { return !Len(); }

    long            getMin() const { return Min(); }
    long            getMax() const { return Max(); }
    void            setMin(long nMin)  { Min() = nMin; }
    void            setMax(long nMax)  { Max() = nMax; }
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

inline bool Selection::IsInside( long nIs ) const
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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Selection& selection )
{
    return stream << selection.Min() << '-' << selection.Max();
}
// Rectangle

#define RECT_EMPTY  ((short)-32767)

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Rectangle
{
public:
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

    inline Point        TopLeft() const;
    inline Point        TopRight() const;
    inline Point        TopCenter() const;
    inline Point        BottomLeft() const;
    inline Point        BottomRight() const;
    inline Point        BottomCenter() const;
    inline Point        LeftCenter() const;
    inline Point        RightCenter() const;
    inline Point        Center() const;

    inline void         Move( long nHorzMove, long nVertMove );
    inline void         Transpose();
    inline void         SetPos( const Point& rPoint );
    void                SetSize( const Size& rSize );
    inline Size         GetSize() const;

    inline long         GetWidth() const;
    inline long         GetHeight() const;

    Rectangle&          Union( const Rectangle& rRect );
    Rectangle&          Intersection( const Rectangle& rRect );
    inline Rectangle    GetUnion( const Rectangle& rRect ) const;
    inline Rectangle    GetIntersection( const Rectangle& rRect ) const;

    void                Justify();

    bool                IsInside( const Point& rPOINT ) const;
    bool                IsInside( const Rectangle& rRect ) const;
    bool                IsOver( const Rectangle& rRect ) const;

    void                SetEmpty() { nRight = nBottom = RECT_EMPTY; }
    inline bool         IsEmpty() const;

    inline bool         operator == ( const Rectangle& rRect ) const;
    inline bool         operator != ( const Rectangle& rRect ) const;

    inline Rectangle&   operator += ( const Point& rPt );
    inline Rectangle&   operator -= ( const Point& rPt );

    friend inline Rectangle operator + ( const Rectangle& rRect, const Point& rPt );
    friend inline Rectangle operator - ( const Rectangle& rRect, const Point& rPt );

    TOOLS_DLLPUBLIC friend SvStream&    ReadRectangle( SvStream& rIStream, Rectangle& rRect );
    TOOLS_DLLPUBLIC friend SvStream&    WriteRectangle( SvStream& rOStream, const Rectangle& rRect );

    // ONE
    long                getX() const { return nLeft; }
    long                getY() const { return nTop; }
    long                getWidth() const { return nRight - nLeft; }
    long                getHeight() const { return nBottom - nTop; }
    void                setX( long n ) { nRight += n-nLeft; nLeft = n; }
    void                setY( long n ) { nBottom += n-nTop; nTop = n; }
    void                setWidth( long n ) { nRight = nLeft + n; }
    void                setHeight( long n ) { nBottom = nTop + n; }

private:
    long                nLeft;
    long                nTop;
    long                nRight;
    long                nBottom;
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

inline bool Rectangle::IsEmpty() const
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
        return Point( std::min( nLeft, nRight ) + std::abs( (nRight - nLeft)/2 ),
                      std::min( nTop,  nBottom) );
}

inline Point Rectangle::BottomCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::min( nLeft, nRight ) + std::abs( (nRight - nLeft)/2 ),
                      std::max( nTop,  nBottom) );
}

inline Point Rectangle::LeftCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::min( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
}

inline Point Rectangle::RightCenter() const
{
    if ( IsEmpty() )
        return Point( nLeft, nTop );
    else
        return Point( std::max( nLeft, nRight ), nTop + (nBottom - nTop)/2 );
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

inline bool Rectangle::operator == ( const Rectangle& rRect ) const
{
    return ((nLeft   == rRect.nLeft   ) &&
            (nTop    == rRect.nTop    ) &&
            (nRight  == rRect.nRight  ) &&
            (nBottom == rRect.nBottom ));
}

inline bool Rectangle::operator != ( const Rectangle& rRect ) const
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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Rectangle& rectangle )
{
    if (rectangle.IsEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.getWidth() << 'x' << rectangle.getHeight()
                      << "@(" << rectangle.getX() << ',' << rectangle.getY() << ")";
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
