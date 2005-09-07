/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvastools.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:51:06 $
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

#ifndef _COM_SUN_STAR_GEOMETRY_REALSIZE2D_HPP__
#include <com/sun/star/geometry/RealSize2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALRECTANGLE2D_HPP__
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALBEZIERSEGMENT2D_HPP__
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_AFFINEMATRIX2D_HPP_
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERSIZE2D_HPP__
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERPOINT2D_HPP__
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERRECTANGLE2D_HPP__
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP__
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POINT_HPP__
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP__
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <basegfx/tools/canvastools.hxx>

#include <limits>


using namespace ::com::sun::star;

namespace basegfx
{

    namespace unotools
    {
        namespace
        {
            uno::Sequence< geometry::RealBezierSegment2D > bezierSequenceFromB2DPolygon( const ::basegfx::B2DPolygon& rPoly )
            {
                const sal_uInt32 nNumPoints( rPoly.count() );

                uno::Sequence< geometry::RealBezierSegment2D > outputSequence( nNumPoints );
                geometry::RealBezierSegment2D* pOutput = outputSequence.getArray();

                // fill sequence from polygon
                sal_uInt32 i;
                for( i=0; i<nNumPoints; ++i )
                {
                    const ::basegfx::B2DPoint   aStartPoint( rPoly.getB2DPoint(i) );
                    const ::basegfx::B2DPoint   aEndPoint( i+1<nNumPoints ? rPoly.getB2DPoint(i+1) : aStartPoint );
                    const ::basegfx::B2DPoint   aCtrl1( rPoly.getControlPointA(i) );
                    const ::basegfx::B2DPoint   aCtrl2( rPoly.getControlPointB(i) );

                    if( aStartPoint.equal( aCtrl1 ) &&
                        aStartPoint.equal( aCtrl2 ) )
                    {
                        const double nX( aStartPoint.getX() );
                        const double nY( aStartPoint.getY() );

                        // ATTN: The following line should match the
                        // comparison below in
                        // polygonFromBezier2DSequence()!
                        pOutput[i] = geometry::RealBezierSegment2D( nX, nY,
                                                                    nX, nY,
                                                                    nX, nY );
                    }
                    else
                    {
                        pOutput[i] = geometry::RealBezierSegment2D( aStartPoint.getX(),
                                                                    aStartPoint.getY(),
                                                                    aCtrl1.getX(),
                                                                    aCtrl1.getY(),
                                                                    aCtrl2.getX(),
                                                                    aCtrl2.getY() );
                    }
                }

                return outputSequence;
            }

            uno::Sequence< geometry::RealPoint2D > pointSequenceFromB2DPolygon( const ::basegfx::B2DPolygon& rPoly )
            {
                const sal_uInt32 nNumPoints( rPoly.count() );

                uno::Sequence< geometry::RealPoint2D > outputSequence( nNumPoints );
                geometry::RealPoint2D* pOutput = outputSequence.getArray();

                // fill sequence from polygon
                sal_uInt32 i;
                for( i=0; i<nNumPoints; ++i )
                {
                    const ::basegfx::B2DPoint   aPoint( rPoly.getB2DPoint(i) );

                    pOutput[i] = geometry::RealPoint2D( aPoint.getX(),
                                                        aPoint.getY() );
                }

                return outputSequence;
            }
        }

        //---------------------------------------------------------------------------------------

        uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > bezierSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly )
        {
            const sal_uInt32 nNumPolies( rPolyPoly.count() );
            sal_uInt32 i;

            uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > outputSequence( nNumPolies );
            uno::Sequence< geometry::RealBezierSegment2D >* pOutput = outputSequence.getArray();

            for( i=0; i<nNumPolies; ++i )
            {
                pOutput[i] = bezierSequenceFromB2DPolygon( rPolyPoly.getB2DPolygon(i) );
            }

            return outputSequence;
        }

        //---------------------------------------------------------------------------------------

        uno::Sequence< uno::Sequence< geometry::RealPoint2D > > pointSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly  )
        {
            const sal_uInt32 nNumPolies( rPolyPoly.count() );
            sal_uInt32 i;

            uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( nNumPolies );
            uno::Sequence< geometry::RealPoint2D >* pOutput = outputSequence.getArray();

            for( i=0; i<nNumPolies; ++i )
            {
                pOutput[i] = pointSequenceFromB2DPolygon( rPolyPoly.getB2DPolygon(i) );
            }

            return outputSequence;
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromB2DPolygon( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                                                                const ::basegfx::B2DPolygon&                        rPoly    )
        {
            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;

            if( rPoly.areControlPointsUsed() )
            {
                uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > outputSequence( 1 );
                outputSequence[0] = bezierSequenceFromB2DPolygon( rPoly );

                xRes.set( xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence ),
                          uno::UNO_QUERY );
            }
            else
            {
                uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( 1 );
                outputSequence[0] = pointSequenceFromB2DPolygon( rPoly );

                xRes.set( xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence ),
                          uno::UNO_QUERY );
            }

            if( xRes.is() && rPoly.isClosed() )
                xRes->setClosed( 0, sal_True );

            return xRes;
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromB2DPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                                                                    const ::basegfx::B2DPolyPolygon&                    rPolyPoly    )
        {
            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;

            const sal_uInt32 nNumPolies( rPolyPoly.count() );
            sal_uInt32 i;

            if( rPolyPoly.areControlPointsUsed() )
            {
                xRes.set( xGraphicDevice->createCompatibleBezierPolyPolygon(
                              bezierSequenceSequenceFromB2DPolyPolygon( rPolyPoly ) ),
                          uno::UNO_QUERY );
            }
            else
            {
                xRes.set( xGraphicDevice->createCompatibleLinePolyPolygon(
                              pointSequenceSequenceFromB2DPolyPolygon( rPolyPoly ) ),
                          uno::UNO_QUERY );
            }

            for( i=0; i<nNumPolies; ++i )
            {
                if( rPolyPoly.getB2DPolygon(i).isClosed() )
                    xRes->setClosed( i, sal_True );
            }

            return xRes;
        }

        //---------------------------------------------------------------------------------------

        ::basegfx::B2DPolygon polygonFromPoint2DSequence( const uno::Sequence< geometry::RealPoint2D >& points )
        {
            const sal_Int32 nCurrSize( points.getLength() );

            ::basegfx::B2DPolygon aPoly;

            for( sal_Int32 nCurrPoint=0; nCurrPoint<nCurrSize; ++nCurrPoint )
                aPoly.append( b2DPointFromRealPoint2D( points[nCurrPoint] ) );

            return aPoly;
        }

        //---------------------------------------------------------------------------------------

        ::basegfx::B2DPolyPolygon polyPolygonFromPoint2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >& points )
        {
            ::basegfx::B2DPolyPolygon aRes;

            for( sal_Int32 nCurrPoly=0; nCurrPoly<points.getLength(); ++nCurrPoly )
            {
                aRes.append( polygonFromPoint2DSequence( points[nCurrPoly] ) );
            }

            return aRes;
        }

        //---------------------------------------------------------------------------------------

        ::basegfx::B2DPolygon polygonFromBezier2DSequence( const uno::Sequence< geometry::RealBezierSegment2D >& curves )
        {
            const sal_Int32 nSize( curves.getLength() );

            ::basegfx::B2DPolygon aPoly;

            for( sal_Int32 nCurrPoint=0; nCurrPoint<nSize; ++nCurrPoint )
            {
                const geometry::RealBezierSegment2D aCurrSegment( curves[nCurrPoint] );

                // ATTN: This line should match the setup in
                // bezierSequenceFromB2DPolygon()!
                if( aCurrSegment.Px == aCurrSegment.C1x &&
                    aCurrSegment.Px == aCurrSegment.C2x &&
                    aCurrSegment.Py == aCurrSegment.C1y &&
                    aCurrSegment.Py == aCurrSegment.C2y     )
                {
                    aPoly.append( ::basegfx::B2DPoint( aCurrSegment.Px,
                                                       aCurrSegment.Py ) );
                }
                else
                {
                    aPoly.append( ::basegfx::B2DPoint( aCurrSegment.Px,
                                                       aCurrSegment.Py ) );
                    aPoly.setControlPointA( nCurrPoint,
                                            ::basegfx::B2DPoint( aCurrSegment.C1x,
                                                                 aCurrSegment.C1y ) );
                    aPoly.setControlPointB( nCurrPoint,
                                            ::basegfx::B2DPoint( aCurrSegment.C2x,
                                                                 aCurrSegment.C2y ) );
                }
            }

            return aPoly;
        }

        //---------------------------------------------------------------------------------------

        ::basegfx::B2DPolyPolygon polyPolygonFromBezier2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >& curves )
        {
            ::basegfx::B2DPolyPolygon aRes;

            for( sal_Int32 nCurrPoly=0; nCurrPoly<curves.getLength(); ++nCurrPoly )
            {
                aRes.append( polygonFromBezier2DSequence( curves[nCurrPoly] ) );
            }

            return aRes;
        }

        //---------------------------------------------------------------------------------------

        ::basegfx::B2DHomMatrix& homMatrixFromAffineMatrix( ::basegfx::B2DHomMatrix&        output,
                                                            const geometry::AffineMatrix2D& input )
        {
            // ensure last row is [0,0,1] (and optimized away)
            output.identity();

            output.set(0,0, input.m00);
            output.set(0,1, input.m01);
            output.set(0,2, input.m02);
            output.set(1,0, input.m10);
            output.set(1,1, input.m11);
            output.set(1,2, input.m12);

            return output;
        }

        geometry::AffineMatrix2D& affineMatrixFromHomMatrix( geometry::AffineMatrix2D&      output,
                                                             const ::basegfx::B2DHomMatrix& input)
        {
            output.m00 = input.get(0,0);
            output.m01 = input.get(0,1);
            output.m02 = input.get(0,2);
            output.m10 = input.get(1,0);
            output.m11 = input.get(1,1);
            output.m12 = input.get(1,2);

            return output;
        }

        //---------------------------------------------------------------------------------------

        geometry::RealSize2D size2DFromB2DSize( const ::basegfx::B2DVector& rVec )
        {
            return geometry::RealSize2D( rVec.getX(),
                                         rVec.getY() );
        }

        geometry::RealPoint2D point2DFromB2DPoint( const ::basegfx::B2DPoint& rPoint )
        {
            return geometry::RealPoint2D( rPoint.getX(),
                                          rPoint.getY() );
        }

        geometry::RealRectangle2D rectangle2DFromB2DRectangle( const ::basegfx::B2DRange& rRect )
        {
            return geometry::RealRectangle2D( rRect.getMinX(),
                                              rRect.getMinY(),
                                              rRect.getMaxX(),
                                              rRect.getMaxY() );
        }

        ::basegfx::B2DVector b2DSizeFromRealSize2D( const geometry::RealSize2D& rSize )
        {
            return ::basegfx::B2DVector( rSize.Width,
                                         rSize.Height );
        }

        ::basegfx::B2DPoint b2DPointFromRealPoint2D( const geometry::RealPoint2D& rPoint )
        {
            return ::basegfx::B2DPoint( rPoint.X,
                                        rPoint.Y );
        }

        ::basegfx::B2DRange b2DRectangleFromRealRectangle2D( const geometry::RealRectangle2D& rRect )
        {
            return ::basegfx::B2DRange( rRect.X1,
                                        rRect.Y1,
                                        rRect.X2,
                                        rRect.Y2 );
        }

        geometry::IntegerSize2D integerSize2DFromB2ISize( const ::basegfx::B2IVector& rSize )
        {
            return geometry::IntegerSize2D( rSize.getX(),
                                            rSize.getY() );
        }

        geometry::IntegerPoint2D integerPoint2DFromB2IPoint( const ::basegfx::B2IPoint& rPoint )
        {
            return geometry::IntegerPoint2D( rPoint.getX(),
                                             rPoint.getY() );
        }

        geometry::IntegerRectangle2D integerRectangle2DFromB2IRectangle( const ::basegfx::B2IRange& rRectangle )
        {
            return geometry::IntegerRectangle2D( rRectangle.getMinX(), rRectangle.getMinY(),
                                                 rRectangle.getMaxX(), rRectangle.getMaxY() );
        }

        ::basegfx::B2IVector b2ISizeFromIntegerSize2D( const geometry::IntegerSize2D& rSize )
        {
            return ::basegfx::B2IVector( rSize.Width,
                                         rSize.Height );
        }

        ::basegfx::B2IPoint b2IPointFromIntegerPoint2D( const geometry::IntegerPoint2D& rPoint )
        {
            return ::basegfx::B2IPoint( rPoint.X,
                                        rPoint.Y );
        }

        ::basegfx::B2IRange b2IRectangleFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRectangle )
        {
            return ::basegfx::B2IRange( rRectangle.X1, rRectangle.Y1,
                                        rRectangle.X2, rRectangle.Y2 );
        }

        awt::Size awtSizeFromB2ISize( const ::basegfx::B2IVector& rVec )
        {
            return awt::Size( rVec.getX(),
                              rVec.getY() );
        }

        awt::Point awtPointFromB2IPoint( const ::basegfx::B2IPoint& rPoint )
        {
            return awt::Point( rPoint.getX(),
                               rPoint.getY() );
        }

        awt::Rectangle awtRectangleFromB2IRectangle( const ::basegfx::B2IRange& rRect )
        {
            OSL_ENSURE( rRect.getWidth() < ::std::numeric_limits< sal_Int32 >::max() &&
                        rRect.getWidth() > ::std::numeric_limits< sal_Int32 >::min(),
                        "awtRectangleFromB2IRectangle(): width overflow" );
            OSL_ENSURE( rRect.getHeight() < ::std::numeric_limits< sal_Int32 >::max() &&
                        rRect.getHeight() > ::std::numeric_limits< sal_Int32 >::min(),
                        "awtRectangleFromB2IRectangle(): height overflow" );

            return awt::Rectangle( rRect.getMinX(),
                                   rRect.getMinY(),
                                   static_cast< sal_Int32 >(rRect.getWidth()),
                                   static_cast< sal_Int32 >(rRect.getHeight()) );
        }

        ::basegfx::B2IVector b2ISizeFromAwtSize( const awt::Size& rSize )
        {
            return ::basegfx::B2IVector( rSize.Width,
                                         rSize.Height );
        }

        ::basegfx::B2IPoint b2IPointFromAwtPoint( const awt::Point& rPoint )
        {
            return ::basegfx::B2IPoint( rPoint.X,
                                        rPoint.Y );
        }

        ::basegfx::B2IRange b2IRectangleFromAwtRectangle( const awt::Rectangle& rRect )
        {
            return ::basegfx::B2IRange( rRect.X,
                                        rRect.Y,
                                        rRect.X + rRect.Width,
                                        rRect.Y + rRect.Height );
        }

    } // namespace bgfxtools

} // namespace canvas
