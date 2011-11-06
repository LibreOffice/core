/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
