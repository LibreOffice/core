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

#include <canvas/parametricpolypolygon.hxx>
#include <canvas/propertysethelper.hxx>
#include <canvas/verifyinput.hxx>

namespace com::sun::star::beans { class XPropertySetInfo; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::rendering { class XColorSpace; }


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
        overridden) and your implementation class. You then only have
        to implement the functionality <em>besides</em>
        XGraphicDevice. If you want to support the optional debug
        XUpdatable interface, also add that to the base classes
        (client code will call the corresponding update() method,
        whenever a burst of animations is over).

        <pre>
        Example:
        typedef ::cppu::WeakComponentImplHelper < css::rendering::XGraphicDevice,
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
        Base class to use, most probably the
        WeakComponentImplHelper template with the appropriate
        interfaces. At least XGraphicDevice should be among them (why else
        would you use this template, then?). Base class must have an
        Base( const Mutex& ) constructor (like the
        WeakComponentImplHelper template has). As the very least,
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
        typedef Mutex             MutexType;
        typedef UnambiguousBase   UnambiguousBaseType;

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

        virtual void disposeThis() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            maDeviceHelper.disposing();

            // pass on to base class
            BaseType::disposeThis();
        }

        // XGraphicDevice
        virtual css::uno::Reference< css::rendering::XBufferController > SAL_CALL getBufferController(  ) override
        {
            return css::uno::Reference< css::rendering::XBufferController >();
        }

        virtual css::uno::Reference< css::rendering::XColorSpace > SAL_CALL getDeviceColorSpace(  ) override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getColorSpace();
        }

        virtual css::geometry::RealSize2D SAL_CALL getPhysicalResolution() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalResolution();
        }

        virtual css::geometry::RealSize2D SAL_CALL getPhysicalSize() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.getPhysicalSize();
        }

        virtual css::uno::Reference< css::rendering::XLinePolyPolygon2D > SAL_CALL createCompatibleLinePolyPolygon( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& points ) override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleLinePolyPolygon( this, points );
        }

        virtual css::uno::Reference< css::rendering::XBezierPolyPolygon2D > SAL_CALL createCompatibleBezierPolyPolygon( const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& points ) override
        {
            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBezierPolyPolygon( this, points );
        }

        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL createCompatibleBitmap( const css::geometry::IntegerSize2D& size ) override
        {
            tools::verifyBitmapSize(size,
                                    OSL_THIS_FUNC,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XVolatileBitmap > SAL_CALL createVolatileBitmap( const css::geometry::IntegerSize2D& size ) override
        {
            tools::verifyBitmapSize(size,
                                    OSL_THIS_FUNC,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL createCompatibleAlphaBitmap( const css::geometry::IntegerSize2D& size ) override
        {
            tools::verifyBitmapSize(size,
                                    OSL_THIS_FUNC,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createCompatibleAlphaBitmap( this, size );
        }

        virtual css::uno::Reference< css::rendering::XVolatileBitmap > SAL_CALL createVolatileAlphaBitmap( const css::geometry::IntegerSize2D& size ) override
        {
            tools::verifyBitmapSize(size,
                                    OSL_THIS_FUNC,
                                    static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maDeviceHelper.createVolatileAlphaBitmap( this, size );
        }

        virtual css::uno::Reference< css::lang::XMultiServiceFactory > SAL_CALL getParametricPolyPolygonFactory(  ) override
        {
            return this;
        }

        virtual sal_Bool SAL_CALL hasFullScreenMode(  ) override
        {
            return false;
        }

        virtual sal_Bool SAL_CALL enterFullScreenMode( sal_Bool ) override
        {
            return false;
        }

        // XMultiServiceFactory
        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override
        {
            return css::uno::Reference< css::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              css::uno::Sequence< css::uno::Any >()));
        }

        virtual css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& aServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override
        {
            return css::uno::Reference< css::rendering::XParametricPolyPolygon2D >(
                ParametricPolyPolygon::create(this,
                                              aServiceSpecifier,
                                              Arguments));
        }

        virtual css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override
        {
            return ParametricPolyPolygon::getAvailableServiceNames();
        }


        // XUpdatable
        virtual void SAL_CALL update() override
        {
            MutexType aGuard( BaseType::m_aMutex );

            if( mbDumpScreenContent )
                maDeviceHelper.dumpScreenContent();
        }


        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertySetInfo();
        }

        virtual void SAL_CALL setPropertyValue( const OUString&                   aPropertyName,
                                                const css::uno::Any& aValue ) override
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.setPropertyValue( aPropertyName, aValue );
        }

        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& aPropertyName ) override
        {
            MutexType aGuard( BaseType::m_aMutex );
            return maPropHelper.getPropertyValue( aPropertyName );
        }

        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addPropertyChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removePropertyChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XPropertyChangeListener >& ) override
        {
        }

        virtual void SAL_CALL addVetoableChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener ) override
        {
            MutexType aGuard( BaseType::m_aMutex );
            maPropHelper.addVetoableChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void SAL_CALL removeVetoableChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XVetoableChangeListener >& ) override
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

        DeviceHelper      maDeviceHelper;
        PropertySetHelper maPropHelper;
        bool              mbDumpScreenContent;

    private:
        GraphicDeviceBase( const GraphicDeviceBase& ) = delete;
        GraphicDeviceBase& operator=( const GraphicDeviceBase& ) = delete;
    };
}

#endif // INCLUDED_CANVAS_BASE_GRAPHICDEVICEBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
