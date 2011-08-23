/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifdef DBG_UTIL
#endif
#include <stdlib.h>
#include "swrect.hxx"


/*************************************************************************
|*
|*	SwRect::SwRect()
|*
|*	Ersterstellung		MA 02. Feb. 93
|*	Letzte Aenderung	MA 05. Sep. 93
|*
|*************************************************************************/
#include <tools/debug.hxx>
namespace binfilter {
/*N*/ SwRect::SwRect( const Rectangle &rRect ) :
/*N*/ 	nX( rRect.Left() ),
/*N*/ 	nY( rRect.Top() )
/*N*/ {
/*N*/ 	nWidth  = rRect.Right() == RECT_EMPTY ? 0 :
/*N*/ 							rRect.Right()  - rRect.Left() +1;
/*N*/ 	nHeight = rRect.Bottom() == RECT_EMPTY ? 0 :
/*N*/ 							rRect.Bottom() - rRect.Top() + 1;
/*N*/ }

/*************************************************************************
|*
|*	SwRect::Center()
|*
|*	Ersterstellung		MA 27. Jan. 93
|*	Letzte Aenderung	MA 27. Jan. 93
|*
|*************************************************************************/
/*N*/ Point SwRect::Center() const
/*N*/ {
/*N*/ 	return Point( Left() + Width()  / 2,
/*N*/ 				  Top()  + Height() / 2 );

/*  Wer ruft schon ein Center auf ein "falsches" Rechteck?
    const long nRight = Right();
    const long nBottom= Bottom();
    return Point( min( Left(), nRight ) + long(abs( (nRight - Left())/2) ),
                  min( Top(),  nBottom) + long(abs( (nBottom - Top())/2)));
*/
/*N*/ }

/*************************************************************************
|*
|*	SwRect::Union()
|*
|*	Ersterstellung		MA 27. Jan. 93
|*	Letzte Aenderung	MA 27. Jan. 93
|*
|*************************************************************************/



/*N*/ SwRect& SwRect::Union( const SwRect& rRect )
/*N*/ {
/*N*/ 	if ( Top() > rRect.Top() )
/*N*/ 		Top( rRect.Top() );
/*N*/ 	if ( Left() > rRect.Left() )
/*N*/ 		Left( rRect.Left() );
/*N*/ 	register long n = rRect.Right();
/*N*/ 	if ( Right() < n )
/*N*/ 		Right( n );
/*N*/ 	n = rRect.Bottom();
/*N*/ 	if ( Bottom() < n )
/*N*/ 		Bottom( n );
/*N*/ 	return *this;
/*N*/ }
/*************************************************************************
|*
|*	SwRect::Intersection(), _Intersection()
|*
|*	Ersterstellung		MA 27. Jan. 93
|*	Letzte Aenderung	MA 05. Sep. 93
|*
|*************************************************************************/



/*N*/ SwRect& SwRect::Intersection( const SwRect& rRect )
/*N*/ {
/*N*/ 	//Hat das Teil ueberhaupt Gemeinsamkeiten mit mir?
/*N*/ 	if ( IsOver( rRect ) )
/*N*/ 	{
/*N*/ 		//Bestimmung der kleineren  rechten sowie unteren und
/*N*/ 		//           der groesseren linken  sowie oberen Kante.
/*N*/ 		if ( Left() < rRect.Left() )
/*N*/ 			Left( rRect.Left() );
/*N*/ 		if ( Top() < rRect.Top() )
/*?*/ 			Top( rRect.Top() );
/*N*/ 		register long n = rRect.Right();
/*N*/ 		if ( Right() > n )
/*N*/ 			Right( n );
/*N*/ 		n = rRect.Bottom();
/*N*/ 		if ( Bottom() > n )
/*?*/ 			Bottom( n );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		//Def.: Bei einer leeren Intersection wird nur die SSize genullt.
/*N*/ 		nHeight = nWidth = 0;
/*N*/ 
/*N*/ 	return *this;
/*N*/ }



/*N*/ SwRect& SwRect::_Intersection( const SwRect& rRect )
/*N*/ {
/*N*/ 	//Bestimmung der kleineren  rechten sowie unteren und
/*N*/ 	//           der groesseren linken  sowie oberen Kante.
/*N*/ 	if ( Left() < rRect.Left() )
/*N*/ 		Left( rRect.Left() );
/*N*/ 	if ( Top() < rRect.Top() )
/*N*/ 		Top( rRect.Top() );
/*N*/ 	register long n = rRect.Right();
/*N*/ 	if ( Right() > n )
/*N*/ 		Right( n );
/*N*/ 	n = rRect.Bottom();
/*N*/ 	if ( Bottom() > n )
/*N*/ 		Bottom( n );
/*N*/ 
/*N*/ 	return *this;
/*N*/ }
/*************************************************************************
|*
|*	SwRect::IsInside()
|*
|*	Ersterstellung		MA 27. Jan. 93
|*	Letzte Aenderung	MA 27. Jan. 93
|*
|*************************************************************************/



/*N*/ BOOL SwRect::IsInside( const SwRect& rRect ) const
/*N*/ {
/*N*/ 	const long nRight  = Right();
/*N*/ 	const long nBottom = Bottom();
/*N*/ 	const long nrRight = rRect.Right();
/*N*/ 	const long nrBottom= rRect.Bottom();
/*N*/ 	return (Left() <= rRect.Left()) && (rRect.Left()<= nRight)  &&
/*N*/ 		   (Left() <= nrRight)		&& (nrRight		<= nRight)  &&
/*N*/ 		   (Top()  <= rRect.Top())	&& (rRect.Top() <= nBottom) &&
/*N*/ 		   (Top()  <= nrBottom)		&& (nrBottom	<= nBottom);
/*N*/ }



/*N*/ BOOL SwRect::IsInside( const Point& rPoint ) const
/*N*/ {
/*N*/ 	return    (Left()  <= rPoint.X())
/*N*/ 		   && (Top()   <= rPoint.Y())
/*N*/ 		   && (Right() >= rPoint.X())
/*N*/ 		   && (Bottom()>= rPoint.Y());
/*N*/ }
/* -----------------------------11.04.00 15:46--------------------------------
    mouse moving of table borders
 ---------------------------------------------------------------------------*/

/*************************************************************************
|*
|*	SwRect::IsOver()
|*
|*	Ersterstellung		MA 25. Feb. 94
|*	Letzte Aenderung	MA 27. Jun. 96
|*
|*************************************************************************/



/*N*/ BOOL SwRect::IsOver( const SwRect& rRect ) const
/*N*/ {
/*N*/ 	return	  (Top()   <= rRect.Bottom())
/*N*/ 		   && (Left()  <= rRect.Right())
/*N*/ 		   && (Right() >= rRect.Left())
/*N*/ 		   && (Bottom()>= rRect.Top()) ? TRUE : FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwRect::Justify()
|*
|*	Ersterstellung		MA 10. Oct. 94
|*	Letzte Aenderung	MA 23. Oct. 96
|*
|*************************************************************************/



/*N*/ void SwRect::Justify()
/*N*/ {
/*N*/ 	if ( nHeight < 0 )
/*N*/ 	{
/*N*/ 		nY = nY + nHeight + 1;
/*N*/ 		nHeight = -nHeight;
/*N*/ 	}
/*N*/ 	if ( nWidth < 0 )
/*N*/ 	{
/*N*/ 		nX = nX + nWidth + 1;
/*N*/ 		nWidth = -nWidth;
/*N*/ 	}
/*N*/ }


#ifdef VERTICAL_LAYOUT

// Similiar to the inline methods, but we need the function pointers

/*N*/ void SwRect::_Width( const long nNew ) { nWidth = nNew; }
/*N*/ void SwRect::_Height( const long nNew ) { nHeight = nNew; }
/*N*/ void SwRect::_Left( const long nLeft ){ nWidth += nX - nLeft; nX = nLeft; }
/*N*/ void SwRect::_Right( const long nRight ){ nWidth = nRight - nX; }
/*N*/ void SwRect::_Top( const long nTop ){ nHeight += nY - nTop; nY = nTop; }
/*N*/ void SwRect::_Bottom( const long nBottom ){ nHeight = nBottom - nY; }
/*N*/ 
/*N*/ long SwRect::_Width() const{ return nWidth; }
/*N*/ long SwRect::_Height() const{ return nHeight; }
/*N*/ long SwRect::_Left() const{ return nX; }
/*N*/ long SwRect::_Right() const{ return nX + nWidth; }
/*N*/ long SwRect::_Top() const{ return nY; }
/*N*/ long SwRect::_Bottom() const{ return nY + nHeight; }

/*N*/ void SwRect::AddWidth( const long nAdd ) { nWidth += nAdd; }
/*N*/ void SwRect::AddHeight( const long nAdd ) { nHeight += nAdd; }
/*N*/ void SwRect::SubLeft( const long nSub ){ nWidth += nSub; nX -= nSub; }
/*N*/ void SwRect::AddRight( const long nAdd ){ nWidth += nAdd; }
/*N*/ void SwRect::SubTop( const long nSub ){ nHeight += nSub; nY -= nSub; }
/*N*/ void SwRect::AddBottom( const long nAdd ){ nHeight += nAdd; }
/*N*/ void SwRect::SetPosX( const long nNew ){ nX = nNew; }
/*N*/ void SwRect::SetPosY( const long nNew ){ nY = nNew; }
/*N*/ const Size  SwRect::_Size() const { return SSize(); }
/*N*/ const Size  SwRect::SwappedSize() const { return Size( nHeight, nWidth ); }
/*N*/ const Point SwRect::TopLeft() const { return Pos(); }
/*N*/ const Point SwRect::TopRight() const { return Point( nX + nWidth, nY ); }
/*N*/ const Point SwRect::BottomLeft() const { return Point( nX, nY + nHeight ); }
/*N*/ const Point SwRect::BottomRight() const
/*N*/     { return Point( nX + nWidth, nY + nHeight ); }
/*N*/ long SwRect::GetLeftDistance( long nLimit ) const { return nX - nLimit; }
/*N*/ long SwRect::GetBottomDistance( long nLim ) const { return nLim - nY - nHeight;}
/*N*/ long SwRect::GetTopDistance( long nLimit ) const { return nY - nLimit; }
/*N*/ long SwRect::GetRightDistance( long nLim ) const { return nLim - nX - nWidth; }
/*N*/ BOOL SwRect::OverStepLeft( long nLimit ) const
/*N*/     { return nLimit > nX && nX + nWidth > nLimit; }
/*N*/ BOOL SwRect::OverStepBottom( long nLimit ) const
/*N*/     { return nLimit > nY && nY + nHeight > nLimit; }
/*N*/ BOOL SwRect::OverStepTop( long nLimit ) const
/*N*/     { return nLimit > nY && nY + nHeight > nLimit; }
/*N*/ BOOL SwRect::OverStepRight( long nLimit ) const
/*N*/     { return nLimit > nX && nX + nWidth > nLimit; }
/*N*/ void SwRect::SetLeftAndWidth( long nLeft, long nNew )
/*N*/     { nX = nLeft; nWidth = nNew; }
/*N*/ void SwRect::SetTopAndHeight( long nTop, long nNew )
/*N*/     { nY = nTop; nHeight = nNew; }
/*N*/ void SwRect::SetRightAndWidth( long nRight, long nNew )
/*N*/     { nX = nRight - nNew; nWidth = nNew; }
/*N*/ void SwRect::SetBottomAndHeight( long nBottom, long nNew )
/*N*/     { nY = nBottom - nNew; nHeight = nNew; }
/*N*/ void SwRect::SetUpperLeftCorner(  const Point& rNew )
/*N*/     { nX = rNew.nA; nY = rNew.nB; }
/*N*/ void SwRect::SetUpperRightCorner(  const Point& rNew )
/*N*/     { nX = rNew.nA - nWidth; nY = rNew.nB; }
/*N*/ void SwRect::SetLowerLeftCorner(  const Point& rNew )
/*N*/     { nX = rNew.nA; nY = rNew.nB - nHeight; }
#endif

#ifdef DBG_UTIL
/*************************************************************************
 *					operator<<( ostream&, SwRect&)
 *************************************************************************/



/*N*/ SvStream &operator<<( SvStream &rStream, const SwRect &rRect )
/*N*/ {
/*N*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	rStream << '[' << rRect.Top()   << '/' << rRect.Left()
/*N*/ 	return rStream;
/*N*/ }
#endif


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
