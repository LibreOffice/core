/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: spritecanvas.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/servicedecl.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/tools/canvastools.hxx>

#include <algorithm>

#include "spritecanvas.hxx"


using namespace ::com::sun::star;

#define SERVICE_NAME "com.sun.star.rendering.VCLCanvas"

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Sequence< uno::Any >&                aArguments,
                                const uno::Reference< uno::XComponentContext >& rxContext ) :
        mxComponentContext( rxContext )
    {
        // #i64742# Only call initialize when not in probe mode
        if( aArguments.getLength() != 0 )
            initialize( aArguments );
    }

    void SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments )
    {
        tools::LocalGuard aGuard;

        OSL_TRACE( "SpriteCanvas created" );

        // add our own property to GraphicDevice
        maPropHelper.addProperties(
            ::canvas::PropertySetHelper::MakeMap
            ("UnsafeScrolling",
             boost::bind(&SpriteCanvasHelper::isUnsafeScrolling,
                         boost::ref(maCanvasHelper)),
             boost::bind(&SpriteCanvasHelper::enableUnsafeScrolling,
                         boost::ref(maCanvasHelper),
                         _1))
            ("SpriteBounds",
             boost::bind(&SpriteCanvasHelper::isSpriteBounds,
                         boost::ref(maCanvasHelper)),
             boost::bind(&SpriteCanvasHelper::enableSpriteBounds,
                         boost::ref(maCanvasHelper),
                         _1)));

        VERBOSE_TRACE( "VCLSpriteCanvas::initialize called" );

        CHECK_AND_THROW( aArguments.getLength() >= 1,
                         "SpriteCanvas::initialize: wrong number of arguments" );

        // We expect a single Any here, containing a pointer to a valid
        // VCL window, on which to output
        if( aArguments.getLength() >= 1 &&
            aArguments[0].getValueTypeClass() == uno::TypeClass_HYPER )
        {
            sal_Int64 nWindowPtr = 0;
            aArguments[0] >>= nWindowPtr;
            Window* pOutputWindow = reinterpret_cast<Window*>(nWindowPtr);

            CHECK_AND_THROW( pOutputWindow != NULL,
                             "SpriteCanvas::initialize: invalid Window pointer" );

            // setup helper
            maDeviceHelper.init( *pOutputWindow,
                                 *this );
            maCanvasHelper.init( *this,
                                 maDeviceHelper.getBackBuffer(),
                                 false,   // no OutDev state preservation
                                 false ); // no alpha on surface
            maCanvasHelper.setRedrawManager( maRedrawManager );
        }
    }

    SpriteCanvas::~SpriteCanvas()
    {
        OSL_TRACE( "SpriteCanvas destroyed" );
    }


    void SAL_CALL SpriteCanvas::disposing()
    {
        tools::LocalGuard aGuard;

        mxComponentContext.clear();

        // forward to parent
        SpriteCanvasBaseT::disposing();
    }

    ::sal_Bool SAL_CALL SpriteCanvas::showBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::showBuffer( bUpdateAll );
    }

    ::sal_Bool SAL_CALL SpriteCanvas::switchBuffer( ::sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : SpriteCanvasBaseT::switchBuffer( bUpdateAll );
    }

    sal_Bool SAL_CALL SpriteCanvas::updateScreen( sal_Bool bUpdateAll ) throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

        // avoid repaints on hidden window (hidden: not mapped to
        // screen). Return failure, since the screen really has _not_
        // been updated (caller should try again later)
        return !mbIsVisible ? false : maCanvasHelper.updateScreen(bUpdateAll,
                                                                  mbSurfaceDirty);
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    bool SpriteCanvas::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const rendering::ViewState&     viewState,
                                const rendering::RenderState&   renderState,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        tools::LocalGuard aGuard;

        return maCanvasHelper.repaint( rGrf, viewState, renderState, rPt, rSz, rAttr );
    }

    OutputDevice* SpriteCanvas::getOutDev() const
    {
        tools::LocalGuard aGuard;

        return maDeviceHelper.getOutDev();
    }

    BackBufferSharedPtr SpriteCanvas::getBackBuffer() const
    {
        tools::LocalGuard aGuard;

        return maDeviceHelper.getBackBuffer();
    }

    uno::Reference< beans::XPropertySetInfo > SAL_CALL SpriteCanvas::getPropertySetInfo() throw (uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        return maPropHelper.getPropertySetInfo();
    }

    void SAL_CALL SpriteCanvas::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                                  const uno::Any&        aValue ) throw (beans::UnknownPropertyException,
                                                                                         beans::PropertyVetoException,
                                                                                         lang::IllegalArgumentException,
                                                                                         lang::WrappedTargetException,
                                                                                         uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        maPropHelper.setPropertyValue( aPropertyName, aValue );
    }

    uno::Any SAL_CALL SpriteCanvas::getPropertyValue( const ::rtl::OUString& aPropertyName ) throw (beans::UnknownPropertyException,
                                                                                                    lang::WrappedTargetException,
                                                                                                    uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        return maPropHelper.getPropertyValue( aPropertyName );
    }

    void SAL_CALL SpriteCanvas::addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException,
                                                                                                                                      lang::WrappedTargetException,
                                                                                                                                      uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        maPropHelper.addPropertyChangeListener( aPropertyName,
                                                xListener );
    }

    void SAL_CALL SpriteCanvas::removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException,
                                                                                                                                         lang::WrappedTargetException,
                                                                                                                                         uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        maPropHelper.removePropertyChangeListener( aPropertyName,
                                                   xListener );
    }

    void SAL_CALL SpriteCanvas::addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                           const uno::Reference< beans::XVetoableChangeListener >& xListener ) throw (beans::UnknownPropertyException,
                                                                                                                                      lang::WrappedTargetException,
                                                                                                                                      uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        maPropHelper.addVetoableChangeListener( aPropertyName,
                                                xListener );
    }

    void SAL_CALL SpriteCanvas::removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                                              const uno::Reference< beans::XVetoableChangeListener >& xListener ) throw (beans::UnknownPropertyException,
                                                                                                                                         lang::WrappedTargetException,
                                                                                                                                         uno::RuntimeException)
    {
        tools::LocalGuard aGuard;
        maPropHelper.removeVetoableChangeListener( aPropertyName,
                                                   xListener );
    }

    namespace sdecl = comphelper::service_decl;
#if defined (__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
    sdecl::class_<SpriteCanvas, sdecl::with_args<true> > serviceImpl;
    const sdecl::ServiceDecl vclCanvasDecl(
        serviceImpl,
#else
    const sdecl::ServiceDecl vclCanvasDecl(
        sdecl::class_<SpriteCanvas, sdecl::with_args<true> >(),
#endif
        "com.sun.star.comp.rendering.VCLCanvas",
        SERVICE_NAME );
}

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(vclcanvas::vclCanvasDecl)
