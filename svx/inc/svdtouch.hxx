/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svdtouch.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SVDTOUCH_HXX
#define _SVDTOUCH_HXX

#include <tools/solar.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

class Point;
class Polygon;
class PolyPolygon;
class Rectangle;

bool IsPointInsidePoly(const Polygon& rPoly, const Point& rHit);
bool IsPointInsidePoly(const PolyPolygon& rPoly, const Point& rHit);

bool IsRectTouchesLine(const Point& rPt1, const Point& rPt2, const Rectangle& rHit);
bool IsRectTouchesLine(const Polygon& rLine, const Rectangle& rHit);
bool IsRectTouchesLine(const PolyPolygon& rLine, const Rectangle& rHit);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTOUCH_HXX

