/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: line.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:13:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _LINE_HXX
#define _LINE_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

class Link;

// --------
// - Line -
// --------

class TOOLS_DLLPUBLIC Line
{
private:

    Point           maStart;
    Point           maEnd;

public:
                    Line() {};
                    Line( const Point& rStartPt, const Point& rEndPt ) : maStart( rStartPt ), maEnd( rEndPt ) {}

    void            SetStart( const Point& rStartPt ) { maStart = rStartPt; }
    const Point&    GetStart() const  { return maStart; }

    void            SetEnd( const Point& rEndPt ) { maEnd = rEndPt; }
    const Point&    GetEnd() const { return maEnd; }

    long            Left() const { return ( maStart.X() < maEnd.X() ) ? maStart.X() : maEnd.X(); }
    long            Top() const { return ( maStart.Y() < maEnd.Y() ) ? maStart.Y() : maEnd.Y(); }
    long            Right() const { return ( maStart.X() > maEnd.X() ) ? maStart.X() : maEnd.X(); }
    long            Bottom() const { return ( maStart.Y() > maEnd.Y() ) ? maStart.Y() : maEnd.Y(); }

    double          GetLength() const;

    BOOL            Intersection( const Line& rLine, double& rIntersectionX, double& rIntersectionY ) const;
    BOOL            Intersection( const Line& rLine, Point& rIntersection ) const;
    BOOL            Intersection( const Rectangle& rRect, Line& rIntersection ) const;

    double          GetDistance( const double& rPtX, const double& rPtY ) const;
    double          GetDistance( const Point& rPoint ) const { return( GetDistance( rPoint.X(), rPoint.Y() ) ); }

    Point           NearestPoint( const Point& rPoint ) const;

    void            Enum( const Link& rEnumLink );
};

#endif // _SV_LINE_HXX
