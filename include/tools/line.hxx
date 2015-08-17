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
#ifndef INCLUDED_TOOLS_LINE_HXX
#define INCLUDED_TOOLS_LINE_HXX

#include <tools/toolsdllapi.h>
#include <tools/gen.hxx>

namespace tools
{

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

    double          GetLength() const;

    bool            Intersection( const tools::Line& rLine, double& rIntersectionX, double& rIntersectionY ) const;
    bool            Intersection( const tools::Line& rLine, Point& rIntersection ) const;

    double          GetDistance( const double& rPtX, const double& rPtY ) const;
    double          GetDistance( const Point& rPoint ) const { return( GetDistance( rPoint.X(), rPoint.Y() ) ); }
};

} // namespace tools

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
