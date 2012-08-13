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
#ifndef _LINE_HXX
#define _LINE_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>

class Link;

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
};

#endif // _SV_LINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
