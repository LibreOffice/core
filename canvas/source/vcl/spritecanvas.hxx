/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spritecanvas.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _VCLCANVAS_SPRITECANVAS_HXX_
#define _VCLCANVAS_SPRITECANVAS_HXX_

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>

#include <cppuhelper/compbase10.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/windowgraphicdevicebase.hxx>

#include "spritecanvashelper.hxx"
#include "impltools.hxx"
#include "devicehelper.hxx"
#include "repainttarget.hxx"


namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper10< ::com::sun::star::rendering::XSpriteCanvas,
                                                ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::rendering::XGraphicDevice,
                                               ::com::sun::star::rendering::XParametricPolyPolygon2DFactory,
                                               ::com::sun::star::rendering::XBufferController,
                                               ::com::sun::star::rendering::XColorSpace,
                                               ::com::sun::star::awt::XWindowListener,
                                               ::com::sun::star::util::XUpdatable,
                                               ::com::sun::star::beans::XPropertySet,
                                               ::com::sun::star::lang::XServiceName >   WindowGraphicDeviceBase_Base;
    typedef ::canvas::WindowGraphicDeviceBase< ::canvas::BaseMutexHelper< WindowGraphicDeviceBase_Base >,
                                               DeviceHelper,
                                               tools::LocalGuard,
                                               ::cppu::OWeakObject >    SpriteCanvasBase_Base;
    /** Mixin SpriteSurface

        Have to mixin the SpriteSurface before deriving from
        ::canvas::SpriteCanvasBase, as this template should already
        implement some of those interface methods.

        The reason why this appears kinda convoluted is the fact that
        we cannot specify non-IDL types as WeakComponentImplHelperN
        template args, and furthermore, don't want to derive
        ::canvas::SpriteCanvasBase directly from
        ::canvas::SpriteSurface (because derivees of
        ::canvas::SpriteCanvasBase have to explicitely forward the
        XInterface methods (e.g. via DECLARE_UNO3_AGG_DEFAULTS)
        anyway). Basically, ::canvas::CanvasCustomSpriteBase should
        remain a base class that provides implementation, not to
        enforce any specific interface on its derivees.
     */
    class SpriteCanvasBaseSpriteSurface_Base : public SpriteCanvasBase_Base,
                                               public ::canvas::SpriteSurface
    {
    };

    typedef ::canvas::SpriteCanvasBase< SpriteCanvasBaseSpriteSurface_Base,
                                        SpriteCanvasHelper,
                                        tools::LocalGuard,
                                        ::cppu::OWeakObject >           SpriteCanvasBaseT;

    /** Product of this component's factory.

        The SpriteCanvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class SpriteCanvas : public SpriteCanvasBaseT,
                         public RepaintTarget
    {
    public:
        SpriteCanvas( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any >&               aArguments,
                      const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );

        /// For resource tracking
        ~SpriteCanvas();

#if defined __SUNPRO_CC
        using SpriteCanvasBaseT::disposing;
#endif

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( SpriteCanvas, WindowGraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase );

        // XBufferController (partial)
        virtual ::sal_Bool SAL_CALL showBuffer( ::sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL switchBuffer( ::sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException);

        // XSpriteCanvas (partial)
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException);

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // RepaintTarget
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const;

        /// Retrieve real output device for this Canvas
        OutputDevice* getOutDev() const;

        /// Get backbuffer for this canvas
        BackBufferSharedPtr getBackBuffer() const;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString&            aPropertyName,
                                                const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                  ::com::sun::star::beans::PropertyVetoException,
                                                                                                  ::com::sun::star::lang::IllegalArgumentException,
                                                                                                  ::com::sun::star::lang::WrappedTargetException,
                                                                                                  ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                    ::com::sun::star::lang::WrappedTargetException,
                                                                                                                    ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                        ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                        ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                           ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                           ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                         const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                        ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                        ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException,
                                                                                                                                                                           ::com::sun::star::lang::WrappedTargetException,
                                                                                                                                                                           ::com::sun::star::uno::RuntimeException);

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
    typedef ::rtl::Reference< SpriteCanvas > DeviceRef;

}

#endif
