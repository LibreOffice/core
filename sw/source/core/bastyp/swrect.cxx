/*************************************************************************
 *
 *  $RCSfile: swrect.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:16 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef PRODUCT
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#endif
#include <stdlib.h>
#include "swrect.hxx"


/*************************************************************************
|*
|*  SwRect::SwRect()
|*
|*  Ersterstellung      MA 02. Feb. 93
|*  Letzte Aenderung    MA 05. Sep. 93
|*
|*************************************************************************/



SwRect::SwRect( const Rectangle &rRect ) :
    nX( rRect.Left() ),
    nY( rRect.Top() )
{
    nWidth  = rRect.Right() == RECT_EMPTY ? 0 :
                            rRect.Right()  - rRect.Left() +1;
    nHeight = rRect.Bottom() == RECT_EMPTY ? 0 :
                            rRect.Bottom() - rRect.Top() + 1;
}

/*************************************************************************
|*
|*  SwRect::Center()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/
Point SwRect::Center() const
{
    return Point( Left() + Width()  / 2,
                  Top()  + Height() / 2 );

/*  Wer ruft schon ein Center auf ein "falsches" Rechteck?
    const long nRight = Right();
    const long nBottom= Bottom();
    return Point( min( Left(), nRight ) + long(abs( (nRight - Left())/2) ),
                  min( Top(),  nBottom) + long(abs( (nBottom - Top())/2)));
*/
}

/*************************************************************************
|*
|*  SwRect::Union()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/



SwRect& SwRect::Union( const SwRect& rRect )
{
    if ( Top() > rRect.Top() )
        Top( rRect.Top() );
    if ( Left() > rRect.Left() )
        Left( rRect.Left() );
    register long n = rRect.Right();
    if ( Right() < n )
        Right( n );
    n = rRect.Bottom();
    if ( Bottom() < n )
        Bottom( n );
    return *this;
}
/*************************************************************************
|*
|*  SwRect::Intersection(), _Intersection()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 05. Sep. 93
|*
|*************************************************************************/



SwRect& SwRect::Intersection( const SwRect& rRect )
{
    //Hat das Teil ueberhaupt Gemeinsamkeiten mit mir?
    if ( IsOver( rRect ) )
    {
        //Bestimmung der kleineren  rechten sowie unteren und
        //           der groesseren linken  sowie oberen Kante.
        if ( Left() < rRect.Left() )
            Left( rRect.Left() );
        if ( Top() < rRect.Top() )
            Top( rRect.Top() );
        register long n = rRect.Right();
        if ( Right() > n )
            Right( n );
        n = rRect.Bottom();
        if ( Bottom() > n )
            Bottom( n );
    }
    else
        //Def.: Bei einer leeren Intersection wird nur die SSize genullt.
        nHeight = nWidth = 0;

    return *this;
}



SwRect& SwRect::_Intersection( const SwRect& rRect )
{
    //Bestimmung der kleineren  rechten sowie unteren und
    //           der groesseren linken  sowie oberen Kante.
    if ( Left() < rRect.Left() )
        Left( rRect.Left() );
    if ( Top() < rRect.Top() )
        Top( rRect.Top() );
    register long n = rRect.Right();
    if ( Right() > n )
        Right( n );
    n = rRect.Bottom();
    if ( Bottom() > n )
        Bottom( n );

    return *this;
}
/*************************************************************************
|*
|*  SwRect::IsInside()
|*
|*  Ersterstellung      MA 27. Jan. 93
|*  Letzte Aenderung    MA 27. Jan. 93
|*
|*************************************************************************/



BOOL SwRect::IsInside( const SwRect& rRect ) const
{
    const long nRight  = Right();
    const long nBottom = Bottom();
    const long nrRight = rRect.Right();
    const long nrBottom= rRect.Bottom();
    return (Left() <= rRect.Left()) && (rRect.Left()<= nRight)  &&
           (Left() <= nrRight)      && (nrRight     <= nRight)  &&
           (Top()  <= rRect.Top())  && (rRect.Top() <= nBottom) &&
           (Top()  <= nrBottom)     && (nrBottom    <= nBottom);
}



BOOL SwRect::IsInside( const Point& rPoint ) const
{
    return    (Left()  <= rPoint.X())
           && (Top()   <= rPoint.Y())
           && (Right() >= rPoint.X())
           && (Bottom()>= rPoint.Y());
}
/* -----------------------------11.04.00 15:46--------------------------------
    mouse moving of table borders
 ---------------------------------------------------------------------------*/
BOOL SwRect::IsNear( const Point& rPoint, long nTolerance ) const
{
    return    IsInside(rPoint) ||
        (((Left() - nTolerance)  <= rPoint.X())
           && ((Top()  - nTolerance)  <= rPoint.Y())
           && ((Right() + nTolerance) >= rPoint.X())
           && ((Bottom()  + nTolerance)>= rPoint.Y()));
}

/*************************************************************************
|*
|*  SwRect::IsOver()
|*
|*  Ersterstellung      MA 25. Feb. 94
|*  Letzte Aenderung    MA 27. Jun. 96
|*
|*************************************************************************/



BOOL SwRect::IsOver( const SwRect& rRect ) const
{
    return    (Top()   <= rRect.Bottom())
           && (Left()  <= rRect.Right())
           && (Right() >= rRect.Left())
           && (Bottom()>= rRect.Top()) ? TRUE : FALSE;
}

/*************************************************************************
|*
|*  SwRect::Justify()
|*
|*  Ersterstellung      MA 10. Oct. 94
|*  Letzte Aenderung    MA 23. Oct. 96
|*
|*************************************************************************/



void SwRect::Justify()
{
    if ( nHeight < 0 )
    {
        nY = nY + nHeight + 1;
        nHeight = -nHeight;
    }
    if ( nWidth < 0 )
    {
        nX = nX + nWidth + 1;
        nWidth = -nWidth;
    }
}


#ifndef PRODUCT
/*************************************************************************
 *                  operator<<( ostream&, SwRect&)
 *************************************************************************/



SvStream &operator<<( SvStream &rStream, const SwRect &rRect )
{
    rStream << '[' << rRect.Top()   << '/' << rRect.Left()
            << ',' << rRect.Width() << 'x' << rRect.Height() << "] ";
    return rStream;
}
#endif


