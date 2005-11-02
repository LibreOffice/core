/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parametricpolypolygon.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:40:39 $
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

#ifndef INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX
#define INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP_
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPARAMETRICPOLYPOLYGON2DFACTORY_HPP_
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#include <boost/utility.hpp>

namespace basegfx
{
    class B2DPolygon;
    class B2DHomMatrix;
}


/* Definition of ParametricPolyPolygon class */

namespace canvas
{
    typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::rendering::XParametricPolyPolygon2D,
                                                   ::com::sun::star::lang::XServiceInfo > ParametricPolyPolygon_Base;

    class ParametricPolyPolygon : public ::comphelper::OBaseMutex,
                                  public ParametricPolyPolygon_Base,
                                  private ::boost::noncopyable
    {
    public:
        enum GradientType
        {
            GRADIENT_LINEAR,
            GRADIENT_AXIAL,
            GRADIENT_ELLIPTICAL,
            GRADIENT_RECTANGULAR
        };

        /** Structure of defining values for the ParametricPolyPolygon

            This is used to copy the state of the
            ParametricPolyPolygon atomically.
         */
        struct Values
        {
            Values( const ::basegfx::B2DPolygon&                        rGradientPoly,
                    const ::com::sun::star::uno::Sequence< double >&    rColor1,
                    const ::com::sun::star::uno::Sequence< double >&    rColor2,
                    double                                              nAspectRatio,
                    GradientType                                        eType ) :
                maGradientPoly( rGradientPoly ),
                mnAspectRatio( nAspectRatio ),
                maColor1( rColor1 ),
                maColor2( rColor2 ),
                meType( eType )
            {
            }

            /// Polygonal gradient shape (ignored for linear and axial gradient)
            const ::basegfx::B2DPolygon                         maGradientPoly;

            /// Aspect ratio of gradient, affects scaling of innermost gradient polygon
            const double                                        mnAspectRatio;

            /// First gradient color
            const ::com::sun::star::uno::Sequence< double >     maColor1;

            /// Second gradient color
            const ::com::sun::star::uno::Sequence< double >     maColor2;

            /// Type of gradient to render (as e.g. linear grads are not represented by maGradientPoly)
            const GradientType                                  meType;
        };

        static ParametricPolyPolygon* createLinearHorizontalGradient( const ::com::sun::star::uno::Reference<
                                                                          ::com::sun::star::rendering::XGraphicDevice >&    rDevice,
                                                                      const ::com::sun::star::uno::Sequence< double >&  leftColor,
                                                                      const ::com::sun::star::uno::Sequence< double >&  rightColor );
        static ParametricPolyPolygon* createAxialHorizontalGradient( const ::com::sun::star::uno::Reference<
                                                                          ::com::sun::star::rendering::XGraphicDevice >&    rDevice,
                                                                     const ::com::sun::star::uno::Sequence< double >&   middleColor,
                                                                     const ::com::sun::star::uno::Sequence< double >&   endColor );
        static ParametricPolyPolygon* createEllipticalGradient( const ::com::sun::star::uno::Reference<
                                                                          ::com::sun::star::rendering::XGraphicDevice >&    rDevice,
                                                                const ::com::sun::star::uno::Sequence< double >&        centerColor,
                                                                const ::com::sun::star::uno::Sequence< double >&        endColor,
                                                                const ::com::sun::star::geometry::RealRectangle2D&      boundRect );
        static ParametricPolyPolygon* createRectangularGradient( const ::com::sun::star::uno::Reference<
                                                                          ::com::sun::star::rendering::XGraphicDevice >&    rDevice,
                                                                 const ::com::sun::star::uno::Sequence< double >&       centerColor,
                                                                 const ::com::sun::star::uno::Sequence< double >&       endColor,
                                                                 const ::com::sun::star::geometry::RealRectangle2D&     boundRect );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XParametricPolyPolygon2D
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D > SAL_CALL getOutline( double t ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getColor( double t ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL getPointColor( const ::com::sun::star::geometry::RealPoint2D& point ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace > SAL_CALL getColorSpace() throw (::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        /// Query all defining values of this object atomically
        Values getValues() const;

    protected:
        ~ParametricPolyPolygon(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:

        /// Private, because objects can only be created from the static factories
        ParametricPolyPolygon( const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::rendering::XGraphicDevice >&   rDevice,
                               const ::basegfx::B2DPolygon&                     rGradientPoly,
                               GradientType                                     eType,
                               const ::com::sun::star::uno::Sequence< double >& rColor1,
                               const ::com::sun::star::uno::Sequence< double >& rColor2 );
        ParametricPolyPolygon( const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::rendering::XGraphicDevice >&   rDevice,
                               const ::basegfx::B2DPolygon&                     rGradientPoly,
                               GradientType                                     eType,
                               const ::com::sun::star::uno::Sequence< double >& rColor1,
                               const ::com::sun::star::uno::Sequence< double >& rColor2,
                               double                                           nAspectRatio );
        ParametricPolyPolygon( const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::rendering::XGraphicDevice >&   rDevice,
                               GradientType                                     eType,
                               const ::com::sun::star::uno::Sequence< double >& rColor1,
                               const ::com::sun::star::uno::Sequence< double >& rColor2 );

        ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XGraphicDevice >    mxDevice;

        /// All defining values of this object
        const Values                                         maValues;
    };
}

#endif /* INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX */
