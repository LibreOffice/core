/*************************************************************************
 *
 *  $RCSfile: svdtouch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 10:24:59 $
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

sal_Bool IsRectTouchesLine(const Point& rPt1, const Point& rPt2, const Rectangle& rHit);
sal_Bool IsRectTouchesLine(const Polygon& rLine, const Rectangle& rHit);
sal_Bool IsRectTouchesLine(const PolyPolygon& rLine, const Rectangle& rHit);
//BFS09sal_Bool IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);
//BFS09sal_Bool IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);
sal_Bool IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit);
sal_Bool IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTOUCH_HXX

