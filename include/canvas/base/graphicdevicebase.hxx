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

#ifndef INCLUDED_CANVAS_BASE_GRAPHICDEVICEBASE_HXX
#define INCLUDED_CANVAS_BASE_GRAPHICDEVICEBASE_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>

#include <canvas/parametricpolypolygon.hxx>
#include <canvas/propertysethelper.hxx>
#include <canvas/verifyinput.hxx>


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
        typedef ::cppu::WeakComponentImplHelper5< css::rendering::XGraphicDevice,
                                                  css::rendering::XColorSpace,
                                                  css::rendering::XPropertySet,
                                                  css::lang::XServiceInfo,
                                                  css::lang::XServiceName > GraphicDeviceBase_Base;
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
        DisambiguationHelper-provided lock. Every time one of the methods is
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
              class UnambiguousBase=css::uno::XInterface > class GraphicDeviceBase :
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
                 [this] () { return this->maDeviceHelper.isAccelerated(); } )
                ("DeviceHandle",
                 [this] () { return this->maDeviceHelper.getDeviceHandle(); } )
                ("SurfaceHandle",
                 [this] () { return this->maDeviceHelper.getSurfaceHandle(); } )
                ("DumpScreenContent",
                 [this] () { return this->getDumpScreenContent(); },
                 [this] (css::uno::Any const& rAny) { this->setDumpScreenContent(rAny); }));
        }

        virtual void disposeThis() SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            maDeviceHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XGraphicDevice
        virtual css::uno::Reference< css::rendering::XBufferController > SAL_CALL getBufferController(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            return css::uno::Reference< css::rendering::XBufferController >();
        }

        virtual css::uno::Reference< css::rendering::XColorSpace > SAL_CALL getDeviceColorSpace(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getColorSpace();
        }

        virtual css::geometry::RealSize2D SAL_CALL getPhysicalResolution() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalResolution();
        }

        virtual css::geometry::RealSize2D SAL_CALL getPhysicalSize() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalSize();
        }

        virtual css::uno::Reference< css::rendering::XLinePolyPolygon2D > SAL_CALL createCompatibleLinePolyPolygon( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& points ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleLinePolyPolygon( this, points );
        }

        virtual css::uno::Reference< css::rendering::XBezierPolyPolygon2D > SAL_CALL createCompatibleBezierPolyPolygon( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& points ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBezierPolyPolygon( this, points );
        }

        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL createCompatibleBitmap( const css::geometry::IntegerSize2D& size )
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XVolatileBitmap > SAL_CALL createVolatileBitmap( const css::geometry::IntegerSize2D& size ) throw (css::lang::IllegalArgumentException,
                                                                                                                                                                                               css::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL createCompatibleAlphaBitmap( const css::geometry::IntegerSize2D& size )
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleAlphaBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XVolatileBitmap > SAL_CALL createVolatileAlphaBitmap( const css::geometry::IntegerSize2D& size ) throw (css::lang::IllegalArgumentException,
                                                                                                                                                                                                    css::uno::RuntimeException) SAL_OVERRIDE
        {
            tools::verifyBitmapSize(size,
                                    BOOST_CURRENT_FUNCTION,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileAlphaBitmap( this, size );
        }

        virtual css::uno::Reference< css::lang::XMultiServiceFactory > SAL_CALL getParametricPolyPolygonFactory(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            return this;
        }

        virtual sal_Bool SAL_CALL hasFullScreenMode(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            return sal_False;
        }

        virtual sal_Bool SAL_CALL enterFullScreenMode( sal_Bool ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            return false;
        }

        // XMultiServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw (css::uno::Exception, css::uno::RuntimeException) SAL_OVERRIDE
        {
            return css::uno::Reference< css::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              css::uno::Sequence< css::uno::Any >()));
        }

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& aServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw (css::uno::Exception, css::uno::RuntimeException) SAL_OVERRIDE
        {
            return css::uno::Reference< css::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              Arguments));
        }

        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            return ParametricPolyPolygon::getAvailableServiceNames();
        }


        // XUpdatable
        virtual void SAL_CALL update() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );

            if( mbDumpScreenContent )
                maDeviceHelper.dumpScreenContent();
        }


        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertySetInfo();
        }

        virtual void SAL_CALL setPropertyValue( const OUString&                   aPropertyName,
                                                const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException,
                                                                                     css::beans::PropertyVetoException,
                                                                                     css::lang::IllegalArgumentException,
                                                                                     css::lang::WrappedTargetException,
                                                                                     css::uno::RuntimeException,
                                                                                     std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.setPropertyValue( aPropertyName, aValue );
        }

        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& aPropertyName ) throw (css::beans::UnknownPropertyException,
                                                                                                css::lang::WrappedTargetException,
                                                                                                css::uno::RuntimeException,
                                                                                                std::exception) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertyValue( aPropertyName );
        }

        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException,
                                                                                                                                                                        css::lang::WrappedTargetException,
                                                                                                                                                                        css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addPropertyChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removePropertyChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XPropertyChangeListener >& ) throw (css::beans::UnknownPropertyException,
                                                                                                                                                                           css::lang::WrappedTargetException,
                                                                                                                                                                           css::uno::RuntimeException) SAL_OVERRIDE
        {
        }

        virtual void SAL_CALL addVetoableChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener ) throw (css::beans::UnknownPropertyException,
                                                                                                                                                                        css::lang::WrappedTargetException,
                                                                                                                                                                        css::uno::RuntimeException) SAL_OVERRIDE
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addVetoableChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removeVetoableChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XVetoableChangeListener >& ) throw (css::beans::UnknownPropertyException,
                                                                                                                                                                           css::lang::WrappedTargetException,
                                                                                                                                                                           css::uno::RuntimeException) SAL_OVERRIDE
        {
        }

    protected:
        ~GraphicDeviceBase() {} // we're a ref-counted UNO class. _We_ destroy ourselves.

        css::uno::Any getDumpScreenContent() const
        {
            return css::uno::makeAny( mbDumpScreenContent );
        }

        void setDumpScreenContent( const css::uno::Any& rAny )
        {
            // TODO(Q1): this was mbDumpScreenContent =
            // rAny.get<bool>(), only that gcc3.3 wouldn't eat it
            rAny >>= mbDumpScreenContent;
        }

        DeviceHelperType  maDeviceHelper;
        PropertySetHelper maPropHelper;
        bool              mbDumpScreenContent;

    private:
        GraphicDeviceBase( const GraphicDeviceBase& ) = delete;
        GraphicDeviceBase& operator=( const GraphicDeviceBase& ) = delete;
    };
}

#endif // INCLUDED_CANVAS_BASE_GRAPHICDEVICEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
