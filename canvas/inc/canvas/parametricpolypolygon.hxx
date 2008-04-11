/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: parametricpolypolygon.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX
#define INCLUDED_CANVAS_PARAMETRICPOLYPOLYGON_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

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
