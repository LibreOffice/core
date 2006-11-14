/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdtouch.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:54:41 $
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

#ifndef _SVDTOUCH_HXX
#define _SVDTOUCH_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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

