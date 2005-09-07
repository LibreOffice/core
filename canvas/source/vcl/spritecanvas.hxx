/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spritecanvas.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:23:37 $
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

#ifndef _VCLCANVAS_SPRITECANVAS_HXX_
#define _VCLCANVAS_SPRITECANVAS_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP_
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>
#include <canvas/bitmapcanvasbase.hxx>

#include <memory>

#include "redrawmanager.hxx"
#include "spritesurface.hxx"
#include "canvashelper.hxx"
#include "backbuffer.hxx"
#include "impltools.hxx"
#include "repainttarget.hxx"

class OutputDevice;
class Point;

namespace com { namespace sun { namespace star { namespace uno
{
    class XComponentContext;
    class RuntimeException;
} } } }


namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper6< ::com::sun::star::rendering::XSpriteCanvas,
                                               ::com::sun::star::rendering::XIntegerBitmap,
                                              ::com::sun::star::awt::XWindow,
                                               ::com::sun::star::lang::XInitialization,
                                               ::com::sun::star::lang::XServiceInfo,
                                               ::com::sun::star::lang::XServiceName >                   CanvasBase_Base;
    typedef ::canvas::internal::BitmapCanvasBase< CanvasBase_Base, CanvasHelper, tools::LocalGuard >    SpriteCanvas_Base;

    class SpriteCanvas : public SpriteCanvas_Base,
                         public SpriteSurface,
                         public RepaintTarget
    {
    public:
        typedef ::rtl::Reference< SpriteCanvas > ImplRef;

        SpriteCanvas( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XSpriteCanvas
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromAnimation( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimation >& animation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > SAL_CALL createSpriteFromBitmaps( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap > >& animationBitmaps,
                                                                                                                                           sal_Int8                                                                                                                   interpolationMode ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCustomSprite > SAL_CALL createCustomSprite( const ::com::sun::star::geometry::RealSize2D& spriteSize ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite > SAL_CALL createClonedSprite( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >& original ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL updateScreen( sal_Bool bUpdateAll ) throw (::com::sun::star::uno::RuntimeException);

        // XComponent (comes implicitely with XWindow)
        virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

        // XWindow
        virtual void SAL_CALL setPosSize( ::sal_Int32 X, ::sal_Int32 Y, ::sal_Int32 Width, ::sal_Int32 Height, ::sal_Int16 Flags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Rectangle SAL_CALL getPosSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setVisible( ::sal_Bool Visible ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setEnable( ::sal_Bool Enable ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw( ::com::sun::star::uno::Exception,
                                                                                                                                   ::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()  throw( ::com::sun::star::uno::RuntimeException );

        // XServiceName
        virtual ::rtl::OUString SAL_CALL getServiceName(  ) throw (::com::sun::star::uno::RuntimeException);

        // factory
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) throw ( ::com::sun::star::uno::Exception );

        // SpriteSurface
        virtual void showSprite( const Sprite::ImplRef& sprite );
        virtual void hideSprite( const Sprite::ImplRef& sprite );
        virtual void moveSprite( const Sprite::ImplRef& sprite,
                                 const Point&           rOldPos,
                                 const Point&           rNewPos,
                                 const Size&            rSpriteSize );
        virtual void updateSprite( const Sprite::ImplRef&   sprite,
                                   const Point&             rPos,
                                   const Rectangle&         rUpdateArea );

        // RepaintTarget
        virtual bool repaint( const GraphicObjectSharedPtr& rGrf,
                              const ::Point&                rPt,
                              const ::Size&                 rSz,
                              const GraphicAttr&            rAttr ) const;

    protected:
        ~SpriteCanvas(); // we're a ref-counted UNO class. _We_ destroy ourselves.

    private:
        // default: disabled copy/assignment
        SpriteCanvas(const SpriteCanvas&);
        SpriteCanvas& operator=( const SpriteCanvas& );

        /// Current bounds of the owning Window
        ::com::sun::star::awt::Rectangle                                                maBounds;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    mxComponentContext;
        WindowGraphicDevice::ImplRef                                                    mxDevice;
        BackBufferSharedPtr                                                             mpBackBuffer;
        ::std::auto_ptr< RedrawManager >                                                mpRedrawManager;    // handles smooth screen updates for us

        /// True, if the window this canvas is contained in, is visible
        bool                                                                            mbIsVisible;
    };
}

#endif
