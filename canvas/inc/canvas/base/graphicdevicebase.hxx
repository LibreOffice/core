/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX
#define INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>

#include <canvas/parametricpolypolygon.hxx>
#include <canvas/propertysethelper.hxx>


/* Definition of GraphicDeviceBase class */

namespace canvas
{
    /** Helper template base class for XGraphicDevice implementations.

        This base class provides partial implementations of the
        XGraphicDevice-related interface, such as XColorSpace.

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
        DisambiguationHelper-provided lock.  Everytime one of the methods is
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
        typedef DeviceHelper      DeviceHelperType;
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
                                         ("HardwareAcceleration",
                                          boost::bind(&DeviceHelper::isAccelerated,
                                                      boost::ref(maDeviceHelper)))
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

        virtual void disposeThis()
        {
            MutexType aGuard( BaseType::m_aMutex );

            maDeviceHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XGraphicDevice
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBufferController > SAL_CALL getBufferController(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBufferController >();
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace > SAL_CALL getDeviceColorSpace(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getColorSpace();
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

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > SAL_CALL getParametricPolyPolygonFactory(  ) throw (::com::sun::star::uno::RuntimeException)
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

        // XMultiServiceFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >()));
        }

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& aServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
        {
            return ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              Arguments));
        }

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw (::com::sun::star::uno::RuntimeException)
        {
            return ParametricPolyPolygon::getAvailableServiceNames();
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

        DeviceHelperType  maDeviceHelper;
        PropertySetHelper maPropHelper;
        bool              mbDumpScreenContent;

    private:
        GraphicDeviceBase( const GraphicDeviceBase& );
        GraphicDeviceBase& operator=( const GraphicDeviceBase& );
    };
}

#endif /* INCLUDED_CANVAS_GRAPHICDEVICEBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
