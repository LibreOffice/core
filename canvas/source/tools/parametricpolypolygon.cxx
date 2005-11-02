/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parametricpolypolygon.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:54:50 $
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

#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>

#include <rtl/math.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/tools/tools.hxx>

#include <limits>

#include <canvas/parametricpolypolygon.hxx>


using namespace ::com::sun::star;

namespace canvas
{
    ParametricPolyPolygon* ParametricPolyPolygon::createLinearHorizontalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< double >&                      leftColor,
        const uno::Sequence< double >&                      rightColor )
    {
        // TODO(P2): hold gradient brush statically, and only setup
        // the colors
        return new ParametricPolyPolygon( rDevice, GRADIENT_LINEAR, leftColor, rightColor );
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createAxialHorizontalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< double >&                      middleColor,
        const uno::Sequence< double >&                      endColor )
    {
        // TODO(P2): hold gradient brush statically, and only setup
        // the colors
        return new ParametricPolyPolygon( rDevice, GRADIENT_AXIAL, endColor, middleColor );
    }

    namespace
    {
        double calcAspectRatio( const geometry::RealRectangle2D& rBoundRect )
        {
            const double nWidth( rBoundRect.X2 - rBoundRect.X1 );
            const double nHeight( rBoundRect.Y2 - rBoundRect.Y1 );

            return ::basegfx::fTools::equalZero( nHeight ) ? 1.0 : fabs( nWidth / nHeight );
        }
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createEllipticalGradient(
        const uno::Reference< rendering::XGraphicDevice >&  rDevice,
        const uno::Sequence< double >&                      centerColor,
        const uno::Sequence< double >&                      endColor,
        const geometry::RealRectangle2D&                    boundRect )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            rDevice,
            ::basegfx::tools::createPolygonFromCircle(
                ::basegfx::B2DPoint( 0.5, 0.5), 0.5 ),
            GRADIENT_ELLIPTICAL,
            endColor, centerColor,
            calcAspectRatio( boundRect ) );
    }

    ParametricPolyPolygon* ParametricPolyPolygon::createRectangularGradient( const uno::Reference< rendering::XGraphicDevice >& rDevice,
                                                                             const uno::Sequence< double >&                     centerColor,
                                                                             const uno::Sequence< double >&                     endColor,
                                                                             const geometry::RealRectangle2D&                   boundRect )
    {
        // TODO(P2): hold gradient polygon statically, and only setup
        // the colors
        return new ParametricPolyPolygon(
            rDevice,
            ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle( 0.0, 0.0, 1.0, 1.0 ) ),
            GRADIENT_RECTANGULAR,
            endColor, centerColor,
            calcAspectRatio( boundRect ) );
    }

    void SAL_CALL ParametricPolyPolygon::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        mxDevice.clear();
    }

    uno::Reference< rendering::XPolyPolygon2D > SAL_CALL ParametricPolyPolygon::getOutline( double t ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO(F1): outline NYI
        return uno::Reference< rendering::XPolyPolygon2D >();
    }

    uno::Sequence< double > SAL_CALL ParametricPolyPolygon::getColor( double t ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO(F1): color NYI
        return uno::Sequence< double >();
    }

    uno::Sequence< double > SAL_CALL ParametricPolyPolygon::getPointColor( const geometry::RealPoint2D& point ) throw (lang::IllegalArgumentException, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // TODO(F1): point color NYI
        return uno::Sequence< double >();
    }

    uno::Reference< rendering::XColorSpace > SAL_CALL ParametricPolyPolygon::getColorSpace() throw (uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return mxDevice.is() ? mxDevice->getDeviceColorSpace() : uno::Reference< rendering::XColorSpace >();
    }

#define IMPLEMENTATION_NAME "Canvas::ParametricPolyPolygon"
#define SERVICE_NAME "com.sun.star.rendering.ParametricPolyPolygon"

    ::rtl::OUString SAL_CALL ParametricPolyPolygon::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL ParametricPolyPolygon::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL ParametricPolyPolygon::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

    ParametricPolyPolygon::~ParametricPolyPolygon()
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  const ::basegfx::B2DPolygon&                          rGradientPoly,
                                                  GradientType                                          eType,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor1,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor2 ) :
        ParametricPolyPolygon_Base( m_aMutex ),
        mxDevice( rDevice ),
        maValues( rGradientPoly,
                  rColor1,
                  rColor2,
                  1.0,
                  eType )
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  const ::basegfx::B2DPolygon&                          rGradientPoly,
                                                  GradientType                                          eType,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor1,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor2,
                                                  double                                                nAspectRatio ) :
        ParametricPolyPolygon_Base( m_aMutex ),
        mxDevice( rDevice ),
        maValues( rGradientPoly,
                  rColor1,
                  rColor2,
                  nAspectRatio,
                  eType )
    {
    }

    ParametricPolyPolygon::ParametricPolyPolygon( const uno::Reference< rendering::XGraphicDevice >&    rDevice,
                                                  GradientType                                          eType,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor1,
                                                  const ::com::sun::star::uno::Sequence< double >&      rColor2 ) :
        ParametricPolyPolygon_Base( m_aMutex ),
        mxDevice( rDevice ),
        maValues( ::basegfx::B2DPolygon(),
                  rColor1,
                  rColor2,
                  1.0,
                  eType )
    {
    }

    ParametricPolyPolygon::Values ParametricPolyPolygon::getValues() const
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return maValues;
    }

}
