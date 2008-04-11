/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: graphicdevicebase.hxx,v $
 * $Revision: 1.7 $
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

#ifndef INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX
#define INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>

#include <canvas/parametricpolypolygon.hxx>
#include <canvas/propertysethelper.hxx>


/* Definition of GraphicDeviceBase class */

namespace canvas
{
    /** Helper template base class for XGraphicDevice implementations.

        This base class provides partial implementations of the
        XGraphicDevice-related interface, such as XBufferController,
        XParametricPolyPolygon2DFactory and XColorSpace.

        This template basically interposes itself between the full
        interface you implement (i.e. not restricted to XGraphicDevice
        etc.). The problem with UNO partial interface implementation
        actually is, that you cannot do it the plain way, since
        deriving from a common base subclass always introduces the
        whole set of pure virtuals, that your baseclass helper just
        overrided) and your implementation class. You then only have
        to implement the functionality <em>besides</em>
        XGraphicDevice. If you want to support the optional debug
        XUpdatable interface, also add that to the base classes
        (client code will call the corresponding update() method,
        whenever a burst of animations is over).

        <pre>
        Example:
        typedef ::cppu::WeakComponentImplHelper5< ::com::sun::star::rendering::XGraphicDevice,
                                                  ::com::sun::star::rendering::XBufferController,
                                                  ::com::sun::star::rendering::XColorSpace,
                                                  ::com::sun::star::rendering::XPropertySet,
                                                  ::com::sun::star::lang::XServiceInfo,
                                                  ::com::sun::star::lang::XServiceName > GraphicDeviceBase_Base;
        typedef ::canvas::internal::GraphicDeviceBase< GraphicDeviceBase, DeviceHelper > ExampleDevice_Base;

        class ExampleDevice : public ExampleDevice_Base
        {
        };
        </pre>

        @tpl Base
        Base class to use, most probably one of the
        WeakComponentImplHelperN templates with the appropriate
        interfaces. At least XGraphicDevice should be among them (why else
        would you use this template, then?). Base class must have an
        Base( const Mutex& ) constructor (like the
        WeakComponentImplHelperN templates have). As the very least,
        the base class must be derived from uno::XInterface, as some
        error reporting mechanisms rely on that.

        @tpl DeviceHelper
        Device helper implementation for the backend in question. This
        object will be held as a member of this template class, and
        basically gets forwarded all XGraphicDevice API calls that
        could not be handled generically.

        @tpl Mutex
        Lock strategy to use. Defaults to using the
        BaseMutexHelper-provided lock.  Everytime one of the methods is
        entered, an object of type Mutex is created with m_aMutex as
        the sole parameter, and destroyed again when the method scope
        is left.

        @tpl UnambiguousBase
        Optional unambiguous base class for XInterface of Base. It's
        sometimes necessary to specify this parameter, e.g. if Base
        derives from multiple UNO interface (were each provides its
        own version of XInterface, making the conversion ambiguous)
     */
    template< class Base,
              class DeviceHelper,
              class Mutex=::osl::MutexGuard,
              class UnambiguousBase=::com::sun::star::uno::XInterface > class GraphicDeviceBase :
        public Base
    {
    public:
        typedef Base              BaseType;
        typedef DeviceHelper      HelperType;
        typedef Mutex             MutexType;
        typedef UnambiguousBase   UnambiguousBaseType;
        typedef GraphicDeviceBase ThisType;

        typedef ::rtl::Reference< GraphicDeviceBase > Reference;

        GraphicDeviceBase() :
            maDeviceHelper(),
            maPropHelper(),
            mbDumpScreenContent(false)
        {
            maPropHelper.initProperties( PropertySetHelper::MakeMap
                                         ("DeviceHandle",
                                          boost::bind(&DeviceHelper::getDeviceHandle,
                                                      boost::ref(maDeviceHelper)))
                                         ("SurfaceHandle",
                                          boost::bind(&DeviceHelper::getSurfaceHandle,
                                                      boost::ref(maDeviceHelper)))
                                         ("DumpScreenContent",
                                          boost::bind(&ThisType::getDumpScreenContent,
                                                      this),
                                          boost::bind(&ThisType::setDumpScreenContent,
                                                      this,
                                                      _1)));
        }

#if defined __SUNPRO_CC
        using Base::disposing;
#endif
        virtual void SAL_CALL disposing()
        {
            MutexType aGuard( BaseType::m_aMutex );

            maDeviceHelper.disposing();

            // pass on to base class
            cppu::WeakComponentImplHelperBase::disposing();
        }

        // XGraphicDevice
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBufferController > SAL_CALL getBufferController(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return this;
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace > SAL_CALL getDeviceColorSpace(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return this;
        }

        virtual ::com::sun::star::geometry::RealSize2D SAL_CALL getPhysicalResolution(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalResolution();
        }

        virtual ::com::sun::star::geometry::RealSize2D SAL_CALL getPhysicalSize(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalSize();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XLinePolyPolygon2D > SAL_CALL createCompatibleLinePolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& points ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleLinePolyPolygon( this, points );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBezierPolyPolygon2D > SAL_CALL createCompatibleBezierPolyPolygon( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& points ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBezierPolyPolygon( this, points );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > SAL_CALL createCompatibleBitmap( const ::com::sun::star::geometry::IntegerSize2D& size ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                         ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBitmap( this, size );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > SAL_CALL createVolatileBitmap( const ::com::sun::star::geometry::IntegerSize2D& size ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                               ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileBitmap( this, size );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > SAL_CALL createCompatibleAlphaBitmap( const ::com::sun::star::geometry::IntegerSize2D& size ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                              ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleAlphaBitmap( this, size );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XVolatileBitmap > SAL_CALL createVolatileAlphaBitmap( const ::com::sun::star::geometry::IntegerSize2D& size ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                    ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileAlphaBitmap( this, size );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2DFactory > SAL_CALL getParametricPolyPolygonFactory(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return this;
        }

        virtual ::sal_Bool SAL_CALL hasFullScreenMode(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.hasFullScreenMode();
        }

        virtual ::sal_Bool SAL_CALL enterFullScreenMode( ::sal_Bool bEnter ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.enterFullScreenMode( bEnter );
        }


        // XBufferController
        virtual ::sal_Int32 SAL_CALL createBuffers( ::sal_Int32 nBuffers ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                  ::com::sun::star::uno::RuntimeException)
        {
            tools::verifyRange( nBuffers, (sal_Int32)1 );

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createBuffers( nBuffers );
        }

        virtual void SAL_CALL destroyBuffers(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            maDeviceHelper.destroyBuffers();
        }

        virtual ::sal_Bool SAL_CALL showBuffer( ::sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.showBuffer( bUpdateAll );
        }

        virtual ::sal_Bool SAL_CALL switchBuffer( ::sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.switchBuffer( bUpdateAll );
        }


        // XColorSpace
        virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            // Dummy for now
            return 0;
        }

        virtual ::com::sun::star::rendering::ColorProfile SAL_CALL getICCProfile(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            // Dummy for now
            return ::com::sun::star::rendering::ColorProfile();
        }

        virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            // Dummy for now
            return ::rtl::OUString();
        }

        virtual ::com::sun::star::rendering::ColorSpaceType SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            // Dummy for now
            return 0;
        }


        // XParametricPolyPolygon2DFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createLinearHorizontalGradient( const ::com::sun::star::uno::Sequence< double >& leftColor, const ::com::sun::star::uno::Sequence< double >& rightColor ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                                                                    ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::createLinearHorizontalGradient( this,
                                                                       leftColor,
                                                                       rightColor ) );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createAxialHorizontalGradient( const ::com::sun::star::uno::Sequence< double >& middleColor, const ::com::sun::star::uno::Sequence< double >& endColor ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                                                                   ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::createAxialHorizontalGradient( this,
                                                                      middleColor,
                                                                      endColor ) );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createEllipticalGradient( const ::com::sun::star::uno::Sequence< double >& centerColor, const ::com::sun::star::uno::Sequence< double >& endColor, const ::com::sun::star::geometry::RealRectangle2D& boundRect ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                                                                                                                            ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::createEllipticalGradient( this,
                                                                 centerColor,
                                                                 endColor,
                                                                 boundRect ) );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createRectangularGradient( const ::com::sun::star::uno::Sequence< double >& centerColor, const ::com::sun::star::uno::Sequence< double >& endColor, const ::com::sun::star::geometry::RealRectangle2D& boundRect ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                                                                                                                                             ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::createRectangularGradient( this,
                                                                  centerColor,
                                                                  endColor,
                                                                  boundRect ) );
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createVerticalLinesHatch( const ::com::sun::star::uno::Sequence< double >& /*leftColor*/,
                                                                                                                                             const ::com::sun::star::uno::Sequence< double >& /*rightColor*/ ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                      ::com::sun::star::uno::RuntimeException)
        {
            // TODO(F1): hatch factory NYI
            return ::com::sun::star::uno::Reference<
                        ::com::sun::star::rendering::XParametricPolyPolygon2D >();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createOrthogonalLinesHatch( const ::com::sun::star::uno::Sequence< double >& /*leftTopColor*/,
                                                                                                                                               const ::com::sun::star::uno::Sequence< double >& /*rightBottomColor*/ ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                              ::com::sun::star::uno::RuntimeException)
        {
            // TODO(F1): hatch factory NYI
            return ::com::sun::star::uno::Reference<
                        ::com::sun::star::rendering::XParametricPolyPolygon2D >();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createThreeCrossingLinesHatch( const ::com::sun::star::uno::Sequence< double >& /*startColor*/,
                                                                                                                                                  const ::com::sun::star::uno::Sequence< double >& /*endColor*/ ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                         ::com::sun::star::uno::RuntimeException)
        {
            // TODO(F1): hatch factory NYI
            return ::com::sun::star::uno::Reference<
                        ::com::sun::star::rendering::XParametricPolyPolygon2D >();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D > SAL_CALL createFourCrossingLinesHatch( const ::com::sun::star::uno::Sequence< double >& /*startColor*/,
                                                                                                                                                 const ::com::sun::star::uno::Sequence< double >& /*endColor*/ ) throw (::com::sun::star::lang::IllegalArgumentException,
                                                                                                                                                                                                                        ::com::sun::star::uno::RuntimeException)
        {
            // TODO(F1): hatch factory NYI
            return ::com::sun::star::uno::Reference<
                        ::com::sun::star::rendering::XParametricPolyPolygon2D >();
        }


        // XUpdatable
        virtual void SAL_CALL update() throw (com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            if( mbDumpScreenContent )
                maDeviceHelper.dumpScreenContent();
        }


        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertySetInfo();
        }

        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString&            aPropertyName,
                                                const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                  ::com::sun::star::beans::PropertyVetoException,
                                                                                                  ::com::sun::star::lang::IllegalArgumentException,
                                                                                                  ::com::sun::star::lang::WrappedTargetException,
                                                                                                  ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.setPropertyValue( aPropertyName, aValue );
        }

        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                    ::com::sun::star::lang::WrappedTargetException,
                                                                                                                    ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertyValue( aPropertyName );
        }

        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                        ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                        ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addPropertyChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                           ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                           ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.removePropertyChangeListener( aPropertyName,
                                                       xListener );
        }

        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                        ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                        ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addVetoableChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                           ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                           ::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.removeVetoableChangeListener( aPropertyName,
                                                       xListener );
        }

    protected:
        ~GraphicDeviceBase() {} // we're a ref-counted UNO class. _We_ destroy ourselves.

        ::com::sun::star::uno::Any getDumpScreenContent() const
        {
            return ::com::sun::star::uno::makeAny( mbDumpScreenContent );
        }

        void setDumpScreenContent( const ::com::sun::star::uno::Any& rAny )
        {
            // TODO(Q1): this was mbDumpScreenContent =
            // rAny.get<bool>(), only that gcc3.3 wouldn't eat it
            rAny >>= mbDumpScreenContent;
        }

        HelperType         maDeviceHelper;
        PropertySetHelper  maPropHelper;
        bool               mbDumpScreenContent;

    private:
        GraphicDeviceBase( const GraphicDeviceBase& );
        GraphicDeviceBase& operator=( const GraphicDeviceBase& );
    };
}

#endif /* INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX */
