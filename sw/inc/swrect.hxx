/*************************************************************************
 *
 *  $RCSfile: swrect.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWRECT_HXX
#define _SWRECT_HXX

#include "errhdl.hxx"

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
class SvStream;


class SwRect
{
    long nX;
    long nY;
    long nWidth;
    long nHeight;

public:
    inline SwRect();
    inline SwRect( const SwRect &rRect );
    inline SwRect( const Point& rLT, const Size&  rSize );
    inline SwRect( const Point& rLT, const Point& rRB );
    inline SwRect( long X, long Y, long Width, long Height );

        //SV-SS z.B. SwRect( pWin->GetClipRect() );
    SwRect( const Rectangle &rRect );

    //Set-Methoden
    inline void Chg( const Point& rNP, const Size &rNS );
    inline void Pos(  const Point& rNew );
    inline void Pos( const long nNewX, const long nNewY );
    inline void SSize( const Size&  rNew  );
    inline void SSize( const long nHeight, const long nWidth );
    inline void Width(  long nNew );
    inline void Height( long nNew );
    inline void Left( const long nLeft );
    inline void Right( const long nRight );
    inline void Top( const long nTop );
    inline void Bottom( const long nBottom );

    //Get-Methoden
    inline const Point &Pos()  const;
    inline const Size  &SSize() const;
    inline long Width()  const;
    inline long Height() const;
    inline long Left()   const;
    inline long Right()  const;
    inline long Top()    const;
    inline long Bottom() const;

    //Damit Layoutseitig per Memberpointer auf die Member von Pos und SSize
    //zugegriffen werden kann.
    inline Point &Pos();
    inline Size  &SSize();

    Point Center() const;

    void Justify();

           SwRect &Union( const SwRect& rRect );
           SwRect &Intersection( const SwRect& rRect );
   //Wie Intersection nur wird davon ausgegangen, dass die Rects ueberlappen!
           SwRect &_Intersection( const SwRect &rRect );
    inline SwRect  GetUnion( const SwRect& rRect ) const;
    inline SwRect  GetIntersection( const SwRect& rRect ) const;

           BOOL IsInside( const Point& rPOINT ) const;
           BOOL IsNear(const Point& rPoint, long nTolerance ) const;
           BOOL IsInside( const SwRect& rRect ) const;
           BOOL IsOver( const SwRect& rRect ) const;
    inline BOOL HasArea() const;
    inline BOOL IsEmpty() const;
    inline void Clear();

    inline SwRect &operator = ( const SwRect &rRect );

    inline BOOL operator == ( const SwRect& rRect ) const;
    inline BOOL operator != ( const SwRect& rRect ) const;

    inline SwRect &operator+=( const Point &rPt );
    inline SwRect &operator-=( const Point &rPt );

    inline SwRect &operator+=( const Size &rSz );
    inline SwRect &operator-=( const Size &rSz );

    //SV-SS z.B. pWin->DrawRect( aSwRect.SVRect() );
    inline Rectangle  SVRect() const;

    //Zortech wuerde hier fehlerhaften Code erzeugen.
//  inline operator SRectangle()  const;
//  inline operator Rectangle() const { return Rectangle( aPos, aSize ); }

    // Ausgabeoperator fuer die Debugging-Gemeinde
    friend SvStream &operator<<( SvStream &rStream, const SwRect &rRect );
};

// Implementation in in swrect.cxx
extern SvStream &operator<<( SvStream &rStream, const SwRect &rRect );



//---------------------------------- Set-Methoden
inline void SwRect::Chg( const Point& rNP, const Size &rNS )
{
    nX      = rNP.X();      nY      = rNP.Y();
    nWidth  = rNS.Width();  nHeight = rNS.Height();
}
inline void SwRect::Pos(  const Point& rNew )
{
    nX = rNew.X(); nY = rNew.Y();
}
inline void SwRect::Pos( const long nNewX, const long nNewY )
{
    nX = nNewX; nY = nNewY;
}
inline void SwRect::SSize( const Size&  rNew  )
{
    nWidth = rNew.Width(); nHeight = rNew.Height();
}
inline void SwRect::SSize( const long nNewHeight, const long nNewWidth )
{
    nWidth = nNewWidth; nHeight = nNewHeight;
}
inline void SwRect::Width(  long nNew )
{
    nWidth = nNew;
}
inline void SwRect::Height( long nNew )
{
    nHeight = nNew;
}
inline void SwRect::Left( const long nLeft )
{
    nWidth += nX - nLeft;
    nX = nLeft;
}
inline void SwRect::Right( const long nRight )
{
    nWidth = nRight - nX + 1;
}
inline void SwRect::Top( const long nTop )
{
    nHeight += nY - nTop;
    nY = nTop;
}
inline void SwRect::Bottom( const long nBottom )
{
    nHeight = nBottom - nY + 1;
}

//----------------------------------- Get-Methoden
inline const Point &SwRect::Pos()  const
{
    return *(Point*)(&nX);
}
inline Point &SwRect::Pos()
{
    return *(Point*)(&nX);
}
inline const Size  &SwRect::SSize() const
{
    return *(Size*)(&nWidth);
}
inline Size  &SwRect::SSize()
{
    return *(Size*)(&nWidth);
}
inline long SwRect::Width()  const
{
    return nWidth;
}
inline long SwRect::Height() const
{
    return nHeight;
}
inline long SwRect::Left()   const
{
    return nX;
}
inline long SwRect::Right()  const
{
    return nWidth ? nX + nWidth - 1 : nX;
}
inline long SwRect::Top()    const
{
    return nY;
}
inline long SwRect::Bottom() const
{
    return nHeight ? nY + nHeight - 1 : nY;
}

//----------------------------------- operatoren
inline SwRect &SwRect::operator = ( const SwRect &rRect )
{
    nX = rRect.Left();
    nY = rRect.Top();
    nWidth  = rRect.Width();
    nHeight = rRect.Height();
    return *this;
}
inline BOOL SwRect::operator == ( const SwRect& rRect ) const
{
    return (nX      == rRect.Left()  &&
            nY      == rRect.Top()   &&
            nWidth  == rRect.Width() &&
            nHeight == rRect.Height());
}
inline BOOL SwRect::operator != ( const SwRect& rRect ) const
{
    return (nX      != rRect.Left()  ||
            nY      != rRect.Top()   ||
            nWidth  != rRect.Width() ||
            nHeight != rRect.Height());
}

inline SwRect &SwRect::operator+=( const Point &rPt )
{
    nX += rPt.X(); nY += rPt.Y();
    return *this;
}
inline SwRect &SwRect::operator-=( const Point &rPt )
{
    nX -= rPt.X(); nY -= rPt.Y();
    return *this;
}

inline SwRect &SwRect::operator+=( const Size &rSz )
{
    nWidth += rSz.Width(); nHeight += rSz.Height();
    return *this;
}
inline SwRect &SwRect::operator-=( const Size &rSz )
{
    nWidth -= rSz.Width(); nHeight -= rSz.Height();
    return *this;
}


//--------------------------- Sonstiges
inline Rectangle SwRect::SVRect() const
{
    ASSERT( nWidth && nHeight, "SVRect() ohne Widht oder Height" );
    return Rectangle( nX, nY,
                      nX + nWidth - 1,      //Right()
                      nY + nHeight - 1 );   //Bottom()
}

inline SwRect SwRect::GetUnion( const SwRect& rRect ) const
{
    return SwRect( *this ).Union( rRect );
}
inline SwRect SwRect::GetIntersection( const SwRect& rRect ) const
{
    return SwRect( *this ).Intersection( rRect );
}

inline BOOL SwRect::HasArea() const
{
    return nHeight && nWidth;
}
inline BOOL SwRect::IsEmpty() const
{
    return !(nHeight && nWidth);
}
inline void SwRect::Clear()
{
    nX = nY = nWidth = nHeight = 0;
}

//-------------------------- CToren
inline SwRect::SwRect() :
    nX( 0 ),
    nY( 0 ),
    nWidth ( 0 ),
    nHeight( 0 )
{
}
inline SwRect::SwRect( const SwRect &rRect ) :
    nX( rRect.Left() ),
    nY( rRect.Top()  ),
    nWidth ( rRect.Width()  ),
    nHeight( rRect.Height() )
{
}
inline SwRect::SwRect( const Point& rLT, const Size&  rSize ) :
    nX( rLT.X() ),
    nY( rLT.Y() ),
    nWidth ( rSize.Width() ),
    nHeight( rSize.Height())
{
}
inline SwRect::SwRect( const Point& rLT, const Point& rRB ) :
    nX( rLT.X() ),
    nY( rLT.Y() ),
    nWidth ( rRB.X() - rLT.X() + 1 ),
    nHeight( rRB.Y() - rLT.Y() + 1 )
{
}
inline SwRect::SwRect( long X, long Y, long Width, long Height ) :
    nX( X ),
    nY( Y ),
    nWidth ( Width ),
    nHeight( Height )
{
}


#endif  //_SWRECT_HXX
