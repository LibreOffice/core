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

// header for Point, Rectangle
// header for Polygon
// header for DBG_ASSERT
// header for XPolygon, XPolyPolygon
#ifndef _XPOLY_HXX
#include <bf_svx/xpoly.hxx>
#endif
// header for Line
#ifndef _SV_LINE_HXX
#include <tools/line.hxx>
#endif
// for performance measurement
#ifndef	_RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

// Note: Enable the following to skip points in the resulting spline
// poly-polygon, if they have equal x-values rather than identical points.
// Unitl now, I think there are situations where the output might differ, if you
// do so, so it's not enabled by default.

// #define SPLINE_OPTIMIZE_POINTS

#include "calculat.hxx"

#include <algorithm>
#include <functional>
namespace binfilter {

using namespace ::std;

    
/*N*/ void SchCalculationHelper::IntersectPolygonWithRectangle( const XPolygon& rPolygon, const Rectangle& rRectangle, XPolyPolygon& aResult )
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( context, "sch", "bm93744", "SchCalculationHelper::IntersectPolygonWithRectangle");
/*N*/ 
/*N*/ 	aResult.Clear();
/*N*/ 
/*N*/  	if( rRectangle.IsInside( rPolygon.GetBoundRect() ) )
/*N*/ 	{
/*N*/ 		aResult.Insert( rPolygon );
/*N*/         OSL_TRACE( "IntersectPolygonWithRectangle: result has %d polygons", aResult.Count() );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/     Point aFrom;
/*N*/     Point aTo;
/*N*/ 	USHORT nCount = rPolygon.GetPointCount();
/*N*/ 
/*N*/     // set last point to a position outside the rectangle, such that the first
/*N*/     // time clip2d returns true, the comparison to last will always yield false
/*N*/     Point aLast (rRectangle.TopLeft());
/*N*/     aLast.Move (-1, -1);
/*N*/     XPolygon aCurrentPoly;
/*N*/     USHORT nIdx = 0;
/*N*/ 
/*N*/ 	for (USHORT i=1; i<nCount; i++)
/*N*/ 	{
/*N*/ 		aFrom = rPolygon[i-1];
/*N*/ 		aTo = rPolygon[i];
/*N*/ 		if (clip2d (aFrom, aTo, rRectangle))
/*N*/ 		{
/*N*/             // compose an XPolygon of as many consecutive points as possible
/*N*/             if (aFrom == aLast)
/*N*/             {
/*N*/                 if (aTo != aFrom)
/*N*/                     aCurrentPoly.Insert (nIdx++, aTo, XPOLY_NORMAL);
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 // create an XPolygon and put it into the XPolyPolygon
/*N*/                 if (aCurrentPoly.GetPointCount() > 0)
/*N*/                     aResult.Insert (aCurrentPoly, XPOLYPOLY_APPEND);
/*N*/ 
/*N*/                 // start new sequence
/*N*/ 				aCurrentPoly.SetPointCount (0);
/*N*/                 aCurrentPoly.Insert (0, aFrom, XPOLY_NORMAL);
/*N*/                 nIdx = 1;
/*N*/                 if (aTo != aFrom)
/*N*/                     aCurrentPoly.Insert (nIdx++, aTo, XPOLY_NORMAL);
/*N*/             }
/*N*/ 
/*N*/             aLast = aTo;
/*N*/         }
/*N*/     }
/*N*/     if (aCurrentPoly.GetPointCount() > 0)
/*N*/         aResult.Insert (aCurrentPoly, XPOLYPOLY_APPEND);
/*N*/ 
/*N*/     OSL_TRACE( "IntersectPolygonWithRectangle: result has %d polygons", aResult.Count() );
/*N*/ }





/*N*/ BOOL	SchCalculationHelper::clip2d	(Point & rPoint0, 
/*N*/ 										Point & rPoint1,
/*N*/ 										const Rectangle & rRectangle)
/*N*/ {
/*N*/ 	//	Direction vector of the line.						
/*N*/ 	Point	aD = rPoint1 - rPoint0;
/*N*/ 
/*N*/ 	if (aD.X()==0 && aD.Y()==0 && rRectangle.IsInside (rPoint0))
/*N*/ 	{
/*N*/ 		//	Degenerate case of a zero length line.
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	Values of the line parameter where the line enters resp. leaves the rectangle.
/*N*/ 		double	fTE = 0,
/*N*/ 				fTL = 1;
/*N*/ 				
/*N*/ 		//	Test wether at least a part lies in the four half-planes with respect to 
/*N*/ 		//	the rectangles four edges.
/*N*/ 		if (CLIPt (aD.X(), rRectangle.Left() - rPoint0.X(), fTE, fTL))
/*N*/ 			if (CLIPt (-aD.X(), rPoint0.X() - rRectangle.Right(), fTE, fTL))
/*N*/ 				if (CLIPt (aD.Y(), rRectangle.Top() - rPoint0.Y(), fTE, fTL))
/*N*/ 					if (CLIPt (-aD.Y(), rPoint0.Y() - rRectangle.Bottom(), fTE, fTL))
/*N*/ 					{
/*N*/ 						//	At least a part is visible.
/*N*/ 						if (fTL < 1)
/*N*/ 						{
/*N*/ 							//	::com::pute the new end point.
/*N*/ 							rPoint1.X() = (long)(rPoint0.X() + fTL * aD.X() + 0.5);
/*N*/ 							rPoint1.Y() = (long)(rPoint0.Y() + fTL * aD.Y() + 0.5);
/*N*/ 						}
/*N*/ 						if (fTE > 0)
/*N*/ 						{
/*N*/ 							//	::com::pute the new starting point.
/*N*/ 							rPoint0.X() = (long)(rPoint0.X() + fTE * aD.X() + 0.5);
/*N*/ 							rPoint0.Y() = (long)(rPoint0.Y() + fTE * aD.Y() + 0.5);
/*N*/ 						}
/*N*/ 						return TRUE;
/*N*/ 					}
/*N*/ 					
/*N*/ 		//	Line is not visible.
/*N*/ 		return FALSE;
/*N*/ 	}
/*N*/ }




/*N*/ BOOL	SchCalculationHelper::CLIPt	(double fDenom, 
/*N*/ 									double fNum, 
/*N*/ 									double & fTE, 
/*N*/ 									double & fTL)
/*N*/ {
/*N*/ 	double	fT;
/*N*/ 	
/*N*/ 	if (fDenom > 0)				//	Intersection enters: PE
/*N*/ 	{
/*N*/ 		fT = fNum / fDenom;		//	Parametric value at the intersection.
/*N*/ 		if (fT > fTL)			//	fTE and fTL crossover
/*N*/ 			return FALSE;		//	  therefore reject the line.
/*N*/ 		else if (fT > fTE)		//	A new fTE has been found.
/*N*/ 			fTE = fT;
/*N*/ 	}
/*N*/ 	else if (fDenom < 0)		//	Intersection leaves: PL
/*N*/ 	{
/*N*/ 		fT = fNum / fDenom;		//	Parametric Value at the intersection.
/*N*/ 		if (fT < fTE)			//	fTE and fTL crossover
/*N*/ 			return FALSE;		//	  therefore reject the line.
/*N*/ 		else if (fT < fTL)		//	A new fTL has been found.
/*N*/ 			fTL = fT;
/*N*/ 	}
/*N*/ 	else if (fNum > 0)
/*N*/ 		return FALSE;			//	Line lies on the outside of the edge.
/*N*/ 	
/*N*/ 	return TRUE;
/*N*/ }


// --------------------------------------------------------------------------------

// Calculation of Splines


// ----------------------------------------

}
