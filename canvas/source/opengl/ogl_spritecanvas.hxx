/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CANVAS_SOURCE_OPENGL_OGL_SPRITECANVAS_HXX
#define INCLUDED_CANVAS_SOURCE_OPENGL_OGL_SPRITECANVAS_HXX

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/bufferedgraphicdevicebase.hxx>

#include "ogl_spritedevicehelper.hxx"
#include "ogl_canvashelper.hxx"


namespace oglcanvas
{
    class CanvasCustomSprite;

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XSpriteCanvas,
                                             css::rendering::XGraphicDevice,
                                             css::lang::XMultiServiceFactory,
                                             css::rendering::XBufferController,
                                             css::awt::XWindowListener,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet,
                                             css::lang::XServiceName >    WindowGraphicDeviceBase_Base;
    typedef ::canvas::BufferedGraphicDeviceBase<
        ::canvas::DisambiguationHelper< WindowGraphicDeviceBase_Base >,
        SpriteDeviceHelper,
        ::osl::MutexGuard,
        ::cppu::OWeakObject >  SpriteCanvasDeviceBaseT;

    typedef ::canvas::CanvasBase< SpriteCanvasDeviceBaseT,
                                  CanvasHelper,
                                  ::osl::MutexGuard,
                                  ::cppu::OWeakObject >         SpriteCanvasBaseT;

    /** Product of this component's factory.

        The SpriteCanvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class SpriteCanvas : public SpriteCanvasBaseT
    {
    public:
        SpriteCanvas( const css::uno::Sequence<
                            css::uno::Any >&               aArguments,
                      const css::uno::Reference<
                            css::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( SpriteCanvas, WindowGraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XBufferController (partial)
        virtual sal_Bool SAL_CALL showBuffer( sal_Bool bUpdateAll ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL switchBuffer( sal_Bool bUpdateAll ) throw (css::uno::RuntimeException, std::exception) override;

        // XSpriteCanvas
        virtual css::uno::Reference< css::rendering::XAnimatedSprite > SAL_CALL createSpriteFromAnimation( const css::uno::Reference< css::rendering::XAnimation >& animation ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XAnimatedSprite > SAL_CALL createSpriteFromBitmaps( const css::uno::Sequence< css::uno::Reference< css::rendering::XBitmap > >& animationBitmaps, ::sal_Int8 interpolationMode ) throw (css::lang::IllegalArgumentException, css::rendering::VolatileContentDestroyedException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XCustomSprite > SAL_CALL createCustomSprite( const css::geometry::RealSize2D& spriteSize ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XSprite > SAL_CALL createClonedSprite( const css::uno::Reference< css::rendering::XSprite >& original ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll )
            throw (css::uno::RuntimeException,
                   std::exception) override;

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (css::uno::RuntimeException, std::exception) override;

        void show( const ::rtl::Reference< CanvasCustomSprite >& );
        void hide( const ::rtl::Reference< CanvasCustomSprite >& );

        /** Write out recorded actions
         */
        bool renderRecordedActions() const;

    private:
        css::uno::Sequence< css::uno::Any >                maArguments;
        css::uno::Reference< css::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
