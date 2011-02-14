/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _LINE_HXX
#define _LINE_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>

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

    sal_Bool            Intersection( const Line& rLine, double& rIntersectionX, double& rIntersectionY ) const;
    sal_Bool            Intersection( const Line& rLine, Point& rIntersection ) const;
    sal_Bool            Intersection( const Rectangle& rRect, Line& rIntersection ) const;

    double          GetDistance( const double& rPtX, const double& rPtY ) const;
    double          GetDistance( const Point& rPoint ) const { return( GetDistance( rPoint.X(), rPoint.Y() ) ); }

    Point           NearestPoint( const Point& rPoint ) const;

    void            Enum( const Link& rEnumLink );
};

#endif // _SV_LINE_HXX
