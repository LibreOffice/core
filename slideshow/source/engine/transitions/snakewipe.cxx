/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: snakewipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:58:54 $
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

#include "canvas/debug.hxx"
#include "snakewipe.hxx"
#include "transitiontools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/point/b2dpoint.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"


namespace presentation {
namespace internal {

SnakeWipe::SnakeWipe( sal_Int32 nElements, bool diagonal, bool flipOnYAxis )
    : m_sqrtElements( static_cast<sal_Int32>(
                          sqrt( static_cast<double>(nElements) ) ) ),
      m_elementEdge( 1.0 / m_sqrtElements ),
      m_diagonal(diagonal),
      m_flipOnYAxis(flipOnYAxis)
{
}

::basegfx::B2DPolyPolygon SnakeWipe::calcSnake( double t ) const
{
    ::basegfx::B2DPolyPolygon res;
    const double area = (t * m_sqrtElements * m_sqrtElements);
    const sal_Int32 line_ = (static_cast<sal_Int32>(area) / m_sqrtElements);
    const double line = ::basegfx::pruneScaleValue(
        static_cast<double>(line_) / m_sqrtElements );
    const double col = ::basegfx::pruneScaleValue(
        (area - (line_ * m_sqrtElements)) / m_sqrtElements );

    if (! ::basegfx::fTools::equalZero( line )) {
        ::basegfx::B2DPolygon poly;
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, line ) );
        poly.append( ::basegfx::B2DPoint( 1.0, line ) );
        poly.append( ::basegfx::B2DPoint( 1.0, 0.0 ) );
        poly.setClosed(true);
        res.append(poly);
    }
    if (! ::basegfx::fTools::equalZero( col ))
    {
        double offset = 0.0;
        if ((line_ & 1) == 1) {
            // odd line: => right to left
            offset = (1.0 - col);
        }
        ::basegfx::B2DPolygon poly;
        poly.append( ::basegfx::B2DPoint( offset, line ) );
        poly.append( ::basegfx::B2DPoint( offset,
                                          line + m_elementEdge ) );
        poly.append( ::basegfx::B2DPoint( offset + col,
                                          line + m_elementEdge ) );
        poly.append( ::basegfx::B2DPoint( offset + col, line ) );
        poly.setClosed(true);
        res.append(poly);
    }

    return res;
}

::basegfx::B2DPolyPolygon SnakeWipe::calcHalfDiagonalSnake(
    double t, bool in ) const
{
    ::basegfx::B2DPolyPolygon res;

    if (in) {
        const double sqrtArea2 = sqrt( t * m_sqrtElements * m_sqrtElements );
        const double edge = ::basegfx::pruneScaleValue(
            static_cast<double>( static_cast<sal_Int32>(sqrtArea2) ) /
            m_sqrtElements );

        ::basegfx::B2DPolygon poly;
        if (! ::basegfx::fTools::equalZero( edge )) {
            poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
            poly.append( ::basegfx::B2DPoint( 0.0, edge ) );
            poly.append( ::basegfx::B2DPoint( edge, 0.0 ) );
            poly.setClosed(true);
            res.append(poly);
        }
        const double a = (M_SQRT1_2 / m_sqrtElements);
        const double d = (sqrtArea2 - static_cast<sal_Int32>(sqrtArea2));
        const double len = (t * M_SQRT2 * d);
        const double height = ::basegfx::pruneScaleValue( M_SQRT1_2 / m_sqrtElements );
        poly.clear();
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, 0.0 ) );
        poly.setClosed(true);
        ::basegfx::B2DHomMatrix aTransform;
        if ((static_cast<sal_Int32>(sqrtArea2) & 1) == 1) {
            // odd line
            aTransform.rotate( M_PI_2 + M_PI_4 );
            aTransform.translate( edge + m_elementEdge, 0.0 );
        }
        else {
            aTransform.translate( -a, 0.0 );
            aTransform.rotate( -M_PI_4 );
            aTransform.translate( 0.0, edge );
        }
        poly.transform( aTransform );
        res.append(poly);
    }
    else // out
    {
        const double sqrtArea2 = sqrt( t * m_sqrtElements * m_sqrtElements );
        const double edge = ::basegfx::pruneScaleValue(
            static_cast<double>( static_cast<sal_Int32>(sqrtArea2) ) /
            m_sqrtElements );

        ::basegfx::B2DPolygon poly;
        if (! ::basegfx::fTools::equalZero( edge )) {
            poly.append( ::basegfx::B2DPoint( 0.0, 1.0 ) );
            poly.append( ::basegfx::B2DPoint( edge, 1.0 ) );
            poly.append( ::basegfx::B2DPoint( 1.0, edge ) );
            poly.append( ::basegfx::B2DPoint( 1.0, 0.0 ) );
            poly.setClosed(true);
            res.append(poly);
        }
        const double a = (M_SQRT1_2 / m_sqrtElements);
        const double d = (sqrtArea2 - static_cast<sal_Int32>(sqrtArea2));
        const double len = ((1.0 - t) * M_SQRT2 * d);
        const double height = ::basegfx::pruneScaleValue( M_SQRT1_2 / m_sqrtElements );
        poly.clear();
        poly.append( ::basegfx::B2DPoint( 0.0, 0.0 ) );
        poly.append( ::basegfx::B2DPoint( 0.0, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, height ) );
        poly.append( ::basegfx::B2DPoint( len + a, 0.0 ) );
        poly.setClosed(true);
        ::basegfx::B2DHomMatrix aTransform;
        if ((static_cast<sal_Int32>(sqrtArea2) & 1) == 1) {
            // odd line
            aTransform.translate( 0.0, -height );
            aTransform.rotate( M_PI_2 + M_PI_4 );
            aTransform.translate( 1.0, edge );
        }
        else {
            aTransform.rotate( -M_PI_4 );
            aTransform.translate( edge, 1.0 );
        }
        poly.transform( aTransform );
        res.append(poly);
    }

    return res;
}

::basegfx::B2DPolyPolygon SnakeWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    if (m_diagonal)
    {
        if (t >= 0.5) {
            res.append( calcHalfDiagonalSnake( 1.0, true ) );
            res.append( calcHalfDiagonalSnake( 2.0 * (t - 0.5), false ) );
        }
        else
            res.append( calcHalfDiagonalSnake( 2.0 * t, true ) );
    }
    else
        res = calcSnake(t);

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

::basegfx::B2DPolyPolygon ParallelSnakesWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res;
    if (m_diagonal)
    {
        OSL_ASSERT( m_opposite );
        ::basegfx::B2DPolyPolygon half(
            calcHalfDiagonalSnake( t, false /* out */ ) );
        // flip on x axis and rotate 90 degrees:
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( 1.0, -1.0 );
        aTransform.translate( -0.5, 0.5 );
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        half.flip();
        res.append( half );
        // rotate 180 degrees:
        aTransform.identity();
        aTransform.translate( -0.5, -0.5 );
        aTransform.rotate( M_PI );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        res.append( half );
    }
    else
    {
        ::basegfx::B2DPolyPolygon half( calcSnake( t / 2.0 ) );
        // rotate 90 degrees:
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.translate( -0.5, -0.5 );
        aTransform.rotate( M_PI_2 );
        aTransform.translate( 0.5, 0.5 );
        half.transform( aTransform );
        res.append( flipOnYAxis(half) );
        res.append( m_opposite ? flipOnXAxis(half) : half );
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}
