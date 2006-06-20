/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvas.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:20:57 $
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

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>
#include <canvas/canvastools.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <vcl/canvastools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>

#include <basegfx/tools/canvastools.hxx>

#include <algorithm>

#include "spritecanvas.hxx"


using namespace ::com::sun::star;

#define IMPLEMENTATION_NAME "VCLCanvas::SpriteCanvas"
#define SERVICE_NAME "com.sun.star.rendering.VCLCanvas"

namespace
{
    static ::rtl::OUString SAL_CALL getImplementationName_SpriteCanvas()
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    static uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_SpriteCanvas()
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

namespace vclcanvas
{
    SpriteCanvas::SpriteCanvas( const uno::Reference< uno::XComponentContext >& rxContext ) :
        mxComponentContext( rxContext )
    {
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


    void SAL_CALL SpriteCanvas::initialize( const uno::Sequence< uno::Any >& aArguments ) throw( uno::Exception,
                                                                                                 uno::RuntimeException)
    {
        tools::LocalGuard aGuard;

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

    ::rtl::OUString SAL_CALL SpriteCanvas::getImplementationName() throw( uno::RuntimeException )
    {
        return getImplementationName_SpriteCanvas();
    }

    sal_Bool SAL_CALL SpriteCanvas::supportsService( const ::rtl::OUString& ServiceName ) throw( uno::RuntimeException )
    {
        return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ) );
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL SpriteCanvas::getSupportedServiceNames()  throw( uno::RuntimeException )
    {
        return getSupportedServiceNames_SpriteCanvas();
    }

    ::rtl::OUString SAL_CALL SpriteCanvas::getServiceName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    }

    uno::Reference< uno::XInterface > SAL_CALL SpriteCanvas::createInstance( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::Exception )
    {
        return uno::Reference< uno::XInterface >( static_cast<cppu::OWeakObject*>(new SpriteCanvas( xContext )) );
    }

    bool SpriteCanvas::repaint( const GraphicObjectSharedPtr&   rGrf,
                                const ::Point&                  rPt,
                                const ::Size&                   rSz,
                                const GraphicAttr&              rAttr ) const
    {
        tools::LocalGuard aGuard;

        return maCanvasHelper.repaint( rGrf, rPt, rSz, rAttr );
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
}

namespace
{
    /* shared lib exports implemented with helpers */
    static struct ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            vclcanvas::SpriteCanvas::createInstance, getImplementationName_SpriteCanvas,
            getSupportedServiceNames_SpriteCanvas, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}


/* Exported UNO methods for registration and object creation.
   ==========================================================
 */
extern "C"
{
    void SAL_CALL component_getImplementationEnvironment( const sal_Char**  ppEnvTypeName,
                                                          uno_Environment**  )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL component_writeInfo( lang::XMultiServiceFactory*  xMgr,
                                           registry::XRegistryKey*      xRegistry )
    {
        return ::cppu::component_writeInfoHelper(
            xMgr, xRegistry, s_component_entries );
    }

    void * SAL_CALL component_getFactory( sal_Char const*               implName,
                                          lang::XMultiServiceFactory*   xMgr,
                                          registry::XRegistryKey*       xRegistry )
    {
        return ::cppu::component_getFactoryHelper(
            implName, xMgr, xRegistry, s_component_entries );
    }
}
