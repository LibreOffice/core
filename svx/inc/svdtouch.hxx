/*************************************************************************
 *
 *  $RCSfile: svdtouch.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:01 $
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
class OutputDevice;

FASTBOOL IsPointInsidePoly(const Polygon& rPoly, const Point& rHit);
FASTBOOL IsPointInsidePoly(const PolyPolygon& rPoly, const Point& rHit);
FASTBOOL IsPointInsidePoly(const XPolygon& rPoly, const Point& rHit, OutputDevice* pOut=NULL);
FASTBOOL IsPointInsidePoly(const XPolyPolygon& rPoly, const Point& rHit, OutputDevice* pOut=NULL);

FASTBOOL IsRectTouchesPoly(const Polygon& rPoly, const Rectangle& rHit);
FASTBOOL IsRectTouchesPoly(const PolyPolygon& rPoly, const Rectangle& rHit);
FASTBOOL IsRectTouchesPoly(const XPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut=NULL);
FASTBOOL IsRectTouchesPoly(const XPolyPolygon& rPoly, const Rectangle& rHit, OutputDevice* pOut=NULL);

FASTBOOL IsRectTouchesLine(const Point& rPt1, const Point& rPt2, const Rectangle& rHit);
FASTBOOL IsRectTouchesLine(const Polygon& rLine, const Rectangle& rHit);
FASTBOOL IsRectTouchesLine(const PolyPolygon& rLine, const Rectangle& rHit);
FASTBOOL IsRectTouchesLine(const XPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);
FASTBOOL IsRectTouchesLine(const XPolyPolygon& rLine, const Rectangle& rHit, OutputDevice* pOut=NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTOUCH_HXX

