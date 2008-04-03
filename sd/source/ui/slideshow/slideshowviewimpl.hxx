/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshowviewimpl.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:15:51 $
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

#ifndef _SD_SLIDESHOWVIEWIMPL_HXX_
#define _SD_SLIDESHOWVIEWIMPL_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef COMPHELPER_INC_COMPHELPER_LISTENERNOTIFICATION_HXX
#include <comphelper/listenernotification.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_WINDOWEVENT_HPP_
#include <com/sun/star/awt/WindowEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPAINTLISTENER_HPP_
#include <com/sun/star/awt/XPaintListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XPOINTER_HPP_
#include <com/sun/star/awt/XPointer.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XSLIDESHOW_HPP_
#include <com/sun/star/presentation/XSlideShow.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XSLIDESHOWVIEW_HPP_
#include <com/sun/star/presentation/XSlideShowView.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_XSLIDESHOWLISTENER_HPP_
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODESUPPLIER_HPP_
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif

#ifndef _COMPHELPER_IMPLEMENTATIONREFERENCE_HXX
#include <comphelper/implementationreference.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif

#ifndef _SV_HELP_HXX //autogen
#include <vcl/help.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SVX_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SDMOD_HXX
#include "sdmod.hxx"
#endif

#ifndef _SD_CUSSHOW_HXX
#include "cusshow.hxx"
#endif

#ifndef SD_VIEW_SHELL_BASE_HXX
#include "ViewShellBase.hxx"
#endif

#ifndef SD_PRESENTATION_VIEW_SHELL_HXX
#include "PresentationViewShell.hxx"
#endif

#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif

#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif

#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef SD_SHOW_WINDOW_HXX
#include "showwindow.hxx"
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#ifndef SD_FRAME_VIEW_HXX
#include "FrameView.hxx"
#endif

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif

#ifndef _SD_APP_HRC_
#include "app.hrc"
#endif

namespace sd
{

struct WrappedMouseEvent : public ::com::sun::star::lang::EventObject
{
    enum EventType
    {
        PRESSED,
        RELEASED,
        ENTERED,
        EXITED
    };

    EventType       meType;
    ::com::sun::star::awt::MouseEvent   maEvent;
};

struct WrappedMouseMotionEvent : public ::com::sun::star::lang::EventObject
{
    enum EventType
    {
        DRAGGED,
        MOVED
    };

    EventType       meType;
    ::com::sun::star::awt::MouseEvent   maEvent;
};

///////////////////////////////////////////////////////////////////////
// SlideShowViewListeners
///////////////////////////////////////////////////////////////////////

typedef std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener > > ViewListenerVector;
class SlideShowViewListeners
{
public:
    SlideShowViewListeners( ::osl::Mutex& rMutex );

    void    addListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener );
    void    removeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& _rxListener );
    bool    notify( const ::com::sun::star::lang::EventObject& _rEvent ) throw( com::sun::star::uno::Exception );
    void    disposing( const ::com::sun::star::lang::EventObject& _rEventSource );

protected:
    ViewListenerVector maListeners;
    ::osl::Mutex& mrMutex;
};

typedef ::std::auto_ptr< SlideShowViewListeners >   SlideShowViewListenersPtr;

///////////////////////////////////////////////////////////////////////
// SlideShowViewPaintListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XPaintListener,
                                                ::com::sun::star::awt::PaintEvent >         SlideShowViewPaintListeners_Base;

class SlideShowViewPaintListeners : public SlideShowViewPaintListeners_Base
{
public:
    SlideShowViewPaintListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rListener, const ::com::sun::star::awt::PaintEvent& rEvent ) throw( ::com::sun::star::uno::Exception );
};
typedef ::std::auto_ptr< SlideShowViewPaintListeners >  SlideShowViewPaintListenersPtr;

///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XMouseListener, WrappedMouseEvent > SlideShowViewMouseListeners_Base;

class SlideShowViewMouseListeners : public SlideShowViewMouseListeners_Base
{
public:
    SlideShowViewMouseListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >&  rListener,
                             const WrappedMouseEvent&                   rEvent ) throw( ::com::sun::star::uno::Exception );
};

typedef ::std::auto_ptr< SlideShowViewMouseListeners >  SlideShowViewMouseListenersPtr;


///////////////////////////////////////////////////////////////////////
// SlideShowViewMouseMotionListeners
///////////////////////////////////////////////////////////////////////

typedef ::comphelper::OListenerContainerBase< ::com::sun::star::awt::XMouseMotionListener,
                                                WrappedMouseMotionEvent > SlideShowViewMouseMotionListeners_Base;

class SlideShowViewMouseMotionListeners : public SlideShowViewMouseMotionListeners_Base
{
public:
    SlideShowViewMouseMotionListeners( ::osl::Mutex& rMutex );

protected:
    virtual bool implTypedNotify( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >&    rListener,
                             const WrappedMouseMotionEvent&                 rEvent ) throw( ::com::sun::star::uno::Exception );
};
typedef ::std::auto_ptr< SlideShowViewMouseMotionListeners >    SlideShowViewMouseMotionListenersPtr;


///////////////////////////////////////////////////////////////////////
// SlideShowView
///////////////////////////////////////////////////////////////////////

class ShowWindow;
class SlideshowImpl;

typedef ::cppu::WeakComponentImplHelper4< ::com::sun::star::presentation::XSlideShowView,
                                            ::com::sun::star::awt::XWindowListener,
                                            ::com::sun::star::awt::XMouseListener,
                                            ::com::sun::star::awt::XMouseMotionListener > SlideShowView_Base;

class SlideShowView : public ::comphelper::OBaseMutex,
                    public SlideShowView_Base
{
public:
    SlideShowView( ShowWindow&     rOutputWindow,
                   SdDrawDocument* pDoc,
                   AnimationMode   eAnimationMode,
                   SlideshowImpl*  pSlideShow,
                   bool            bFullScreen );

    void ignoreNextMouseReleased() { mbMousePressedEaten = true; }

    /// Dispose all internal references
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    /// Disposing our broadcaster
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& ) throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL paint( const ::com::sun::star::awt::PaintEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // XSlideShowView methods
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTransformationChangedListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTransformationChangedListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMouseCursor( sal_Int16 nPointerShape ) throw (::com::sun::star::uno::RuntimeException);

    // XWindowListener methods
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // XMouseListener implementation
    virtual void SAL_CALL mousePressed( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseEntered( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseExited( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    // XMouseMotionListener implementation
    virtual void SAL_CALL mouseDragged( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseMoved( const ::com::sun::star::awt::MouseEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    using cppu::WeakComponentImplHelperBase::disposing;

protected:
    ~SlideShowView() {}

private:
    void init();

    void updateimpl( ::osl::ClearableMutexGuard& rGuard, SlideshowImpl* pSlideShow );

    ::cppcanvas::SpriteCanvasSharedPtr                                              mpCanvas;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              mxWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >          mxWindowPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >             mxPointer;
    SlideshowImpl*                          mpSlideShow;
    ShowWindow&                             mrOutputWindow;
    SlideShowViewListenersPtr               mpViewListeners;
    SlideShowViewPaintListenersPtr          mpPaintListeners;
    SlideShowViewMouseListenersPtr          mpMouseListeners;
    SlideShowViewMouseMotionListenersPtr    mpMouseMotionListeners;
    SdDrawDocument*                         mpDoc;
    bool                                    mbIsMouseMotionListener;
    Rectangle                               maPresentationArea;
    AnimationMode                           meAnimationMode;
    bool                                    mbFirstPaint;
    bool                                    mbFullScreen;
    bool                                    mbMousePressedEaten;
};


} // namespace ::sd

#endif
