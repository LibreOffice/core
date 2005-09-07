/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spiralwipe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:59:24 $
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
#include "spiralwipe.hxx"
#include "transitiontools.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/numeric/ftools.hxx"


namespace presentation {
namespace internal {

SpiralWipe::SpiralWipe( sal_Int32 nElements, bool flipOnYAxis )
    : m_elements(nElements),
      m_sqrtElements( static_cast<sal_Int32>(
                          sqrt( static_cast<double>(nElements) ) ) ),
      m_flipOnYAxis(flipOnYAxis)
{
}

::basegfx::B2DPolyPolygon SpiralWipe::calcNegSpiral( double t ) const
{
    const double area = (t * m_elements);
    const double e = (sqrt(area) / 2.0);
    const sal_Int32 edge = (static_cast<sal_Int32>(e) * 2);

    ::basegfx::B2DHomMatrix aTransform;
    aTransform.translate( -0.5, -0.5 );
    const double edge_ = ::basegfx::pruneScaleValue(
        static_cast<double>(edge) / m_sqrtElements );
    aTransform.scale( edge_, edge_ );
    aTransform.translate( 0.5, 0.5 );
    ::basegfx::B2DPolygon poly( createUnitRect() );
    poly.transform( aTransform );
    ::basegfx::B2DPolyPolygon res(poly);

    if (! ::basegfx::fTools::equalZero( 1.0 - t )) {
        const sal_Int32 edge1 = (edge + 1);
        sal_Int32 len = static_cast<sal_Int32>( (e - (edge /2)) * edge1 * 4 );
        double w = F_PI2;
        while (len > 0) {
            const sal_Int32 alen = (len > edge1 ? edge1 : len);
            len -= alen;
            poly = createUnitRect();
            ::basegfx::B2DHomMatrix aTransform;
            aTransform.scale(
                ::basegfx::pruneScaleValue( static_cast<double>(alen) / m_sqrtElements ),
                ::basegfx::pruneScaleValue( 1.0 / m_sqrtElements ) );
            aTransform.translate(
                - ::basegfx::pruneScaleValue(
                    static_cast<double>(edge / 2) / m_sqrtElements ),
                ::basegfx::pruneScaleValue(
                    static_cast<double>(edge / 2) / m_sqrtElements ) );
            aTransform.rotate( w );
            w -= F_PI2;
            aTransform.translate( 0.5, 0.5 );
            poly.transform( aTransform );
            res.append(poly);
        }
    }

    return res;
}

::basegfx::B2DPolyPolygon SpiralWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolyPolygon innerSpiral( calcNegSpiral( 1.0 - t ) );
    innerSpiral.flip();
    res.append(innerSpiral);
    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

::basegfx::B2DPolyPolygon BoxSnakesWipe::operator () ( double t )
{
    ::basegfx::B2DPolyPolygon res( createUnitRect() );
    ::basegfx::B2DPolyPolygon innerSpiral( calcNegSpiral( 1.0 - t ) );
    innerSpiral.flip();

    if (m_fourBox) {
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( 0.5, 0.5 );
        innerSpiral.transform( aTransform );
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
        innerSpiral = flipOnYAxis(innerSpiral);
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
    }
    else {
        ::basegfx::B2DHomMatrix aTransform;
        aTransform.scale( 1.0, 0.5 );
        innerSpiral.transform( aTransform );
        res.append(innerSpiral);
        res.append( flipOnXAxis(innerSpiral) );
    }

    return m_flipOnYAxis ? flipOnYAxis(res) : res;
}

}
}
