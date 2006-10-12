/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdtouch.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:53:41 $
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
class XPolygon;
class XPolyPolygon;
class Rectangle;
//BFS09class OutputDevice;

sal_Bool IsPointInsidePoly(const Polygon& rPoly, const Point& rHit);
sal_Bool IsPointInsidePoly(const PolyPolygon& rPoly, const Point& rHit);
//BFS09sal_Bool IsPointInsidePoly(const XPolygon& rPoly, const Point& rHit, OutputDevice* pOut=NULL);
//BFS09sal_Bool IsPointInsidePoly(const XPolyPolygon& rPoly, const Point& rHit, OutputDevice* pOut=NULL);
sal_Bool IsPointInsidePoly(const XPolygon& rPoly, const Point& rHit);
sal_Bool IsPointInsidePoly(const XPolyPolygon& rPoly, const Point& rHit);

sal_Bool IsRectTouchesPoly(const Polygon& rPoly, const Rectangle& rHit);
sal_Bool IsRectTouchesPoly(const PolyPolygon& rPoly, const Rectangle& rHit);
//BFS09sal_Bool IsRectTouchesPoly(const XPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut=NULL);
//BFS09sal_Bool IsRectTouchesPoly(const XPolyPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut=NULL);
sal_Bool IsRectTouchesPoly(const XPolygon& rPoly, const Rectangle& rHit);
sal_Bool IsRectTouchesPoly(const XPolyPolygon& rPoly, const Rectangle& rHit);

bool IsRectTouchesLine(const Point& rPt1, const Point& rPt2, const Rectangle& rHit);
bool IsRectTouchesLine(const Polygon& rLine, const Rectangle& rHit);
bool IsRectTouchesLine(const PolyPolygon& rLine, const Rectangle& rHit);
//BFS09sal_Bool IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);
//BFS09sal_Bool IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);
bool IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit);
bool IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTOUCH_HXX

