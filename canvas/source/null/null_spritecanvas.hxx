/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: null_spritecanvas.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:49:55 $
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

#ifndef _NULLCANVAS_SPRITECANVAS_HXX_
#define _NULLCANVAS_SPRITECANVAS_HXX_

#include <rtl/ref.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBufferController.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XParametricPolyPolygon2DFactory.hpp>

#include <cppuhelper/compbase11.hxx>
#include <comphelper/uno3.hxx>

#include <canvas/base/spritecanvasbase.hxx>
#include <canvas/base/basemutexhelper.hxx>
#include <canvas/base/windowgraphicdevicebase.hxx>

#include "null_spritecanvashelper.hxx"
#include "null_devicehelper.hxx"
#include "null_usagecounter.hxx"


namespace nullcanvas
{
    typedef ::cppu::WeakComponentImplHelper11< ::com::sun::star::rendering::XSpriteCanvas,
                                                ::com::sun::star::rendering::XIntegerBitmap,
                                                ::com::sun::star::rendering::XGraphicDevice,
                                               ::com::sun::star::rendering::XParametricPolyPolygon2DFactory,
                                               ::com::sun::star::rendering::XBufferController,
                                               ::com::sun::star::rendering::XColorSpace,
                                               ::com::sun::star::awt::XWindow,
                                               ::com::sun::star::beans::XPropertySet,
                                                ::com::sun::star::lang::XInitialization,
                                                    ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::lang::XServiceName >   WindowGraphicDeviceBase_Base;
    typedef ::canvas::WindowGraphicDeviceBase< ::canvas::BaseMutexHelper< WindowGraphicDeviceBase_Base >,
                                               DeviceHelper,
                                               ::osl::MutexGuard,
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
                                        ::osl::MutexGuard,
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
                         private UsageCounter< SpriteCanvas >
    {
    public:
        SpriteCanvas( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::uno::XComponentContext >& rxContext );

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

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw( ::com::sun::star::uno::Exception,
                                                                                                                                   ::com::sun::star::uno::RuntimeException);
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // component factory
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) throw ( ::com::sun::star::uno::Exception );

     private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxComponentContext;
    };

    typedef ::rtl::Reference< SpriteCanvas > SpriteCanvasRef;
    typedef ::rtl::Reference< SpriteCanvas > DeviceRef;
}

#endif
