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

#include <cppuhelper/compbase8.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/disambiguationhelper.hxx>
#include <canvas/base/bufferedgraphicdevicebase.hxx>

#include "ogl_spritedevicehelper.hxx"
#include "ogl_canvashelper.hxx"


namespace oglcanvas
{
    class CanvasCustomSprite;

    typedef ::cppu::WeakComponentImplHelper8< ::com::sun::star::rendering::XSpriteCanvas,
                                                ::com::sun::star::rendering::XGraphicDevice,
                                                ::com::sun::star::lang::XMultiServiceFactory,
                                                ::com::sun::star::rendering::XBufferController,
                                                ::com::sun::star::awt::XWindowListener,
                                                ::com::sun::star::util::XUpdatable,
                                                ::com::sun::star::beans::XPropertySet,
                                                ::com::sun::star::lang::XServiceName >    WindowGraphicDeviceBase_Base;
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
        SpriteCanvas( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any >&               aArguments,
                      const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext >& rxContext );

        void initialize();

        /// Dispose all internal references
        virtual void disposeThis() SAL_OVERRIDE;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        //                                    Classname     Base doing refcounting        Base implementing the XComponent interface
        //                                       |                 |                            |
        //                                       V                 V                            V
        DECLARE_UNO3_XCOMPONENT_AGG_DEFAULTS( SpriteCanvas, WindowGraphicDeviceBase_Base, ::cppu::WeakComponentImplHelperBase )

        // XBufferController (partial)
        virtual sal_Bool SAL_CALL showBuffer( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL switchBuffer( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

        // XSpriteCanvas
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromAnimation( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimation >& animation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromBitmaps( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > >& animationBitmaps, ::sal_Int8 interpolationMode ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCustomSprite > SAL_CALL createCustomSprite( const ::com::sun::star::geometry::RealSize2D& spriteSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite > SAL_CALL createClonedSprite( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >& original ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll )
            throw (::com::sun::star::uno::RuntimeException,
                   std::exception) SAL_OVERRIDE;

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void show( const ::rtl::Reference< CanvasCustomSprite >& );
        void hide( const ::rtl::Reference< CanvasCustomSprite >& );

        /** Write out recorded actions
         */
        bool renderRecordedActions() const;

    private:
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >                maArguments;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
