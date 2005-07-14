/*************************************************************************
 *
 *  $RCSfile: slideshowimpl.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:45:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SD_SLIDESHOWIMPL_HXX_
#define _SD_SLIDESHOWIMPL_HXX_

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
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
#ifndef _COM_SUN_STAR_PRESENTATION_XSHAPEEVENTLISTENER_HPP_
#include <com/sun/star/presentation/XShapeEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATIONNODESUPPLIER_HPP_
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XMANAGER_HPP_
#include <com/sun/star/media/XManager.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
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

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif

#ifndef SD_SHOW_WINDOW_HXX
#include "ShowWindow.hxx"
#endif

#ifndef _SD_OPTSITEM_HXX
#include "optsitem.hxx"
#endif

#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
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

#include "slideshow.hxx"

class SfxViewFrame;
class SfxRequest;

namespace sd
{
class SlideShowView;
class AnimationPageList;
class PaneHider;

struct WrappedShapeEventImpl
{
    ::com::sun::star::presentation::ClickAction meClickAction;
    sal_Int32 mnVerb;
    ::rtl::OUString maStrBookmark;
    WrappedShapeEventImpl() : meClickAction( ::com::sun::star::presentation::ClickAction_NONE ), mnVerb( 0 ) {};
};

typedef boost::shared_ptr< WrappedShapeEventImpl > WrappedShapeEventImplPtr;
typedef std::map< ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >, WrappedShapeEventImplPtr > WrappedShapeEventImplMap;


typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::presentation::XShapeEventListener,
        ::com::sun::star::presentation::XSlideShowListener > SlideshowImpl_base;

class SlideshowImpl : public ::comphelper::OBaseMutex, public SlideshowImpl_base
{
    friend class Slideshow;
public:
    SlideshowImpl( ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc );
    ~SlideshowImpl();

    bool startShow( PresentationSettings* pPresSettings );
    bool startPreview(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent = 0 );

    void stopShow();

    double update();
    /** forces an async call to update in the main thread */
    void startUpdateTimer();
    void paint( const Rectangle& rRect );
    bool keyInput(const KeyEvent& rKEvt);
    void mouseButtonUp(const MouseEvent& rMEvt);

    void createPageList( bool bAll, bool bStartWithActualPage, const String& rPresPage );

    /** called from the ::com::sun::star::presentation::XSlideShowListener implementation when a slide is changed or
        the end of the presentation is reached */
    void slideChange();

    void jumpToPageNumber( sal_Int32 nPage );
    void jumpToPageIndex( sal_Int32 nIndex );
    sal_Int32 getCurrentPageNumber();
    sal_Int32 getCurrentPageIndex();
    sal_Int32 getFirstPageNumber();
    sal_Int32 getLastPageNumber();
    bool isEndless();
    bool isDrawingPossible();
    inline bool isInputFreezed() const;

    void activate();
    void deactivate();

    void hideChildWindows();
    void showChildWindows();

    void resize( const Size& rSize );

    DECL_LINK( updateHdl, Timer* );
    DECL_LINK( ReadyForNextInputHdl, Timer* );
    DECL_LINK( endPresentationHdl, void* );

    // XShapeEventListener
    virtual void SAL_CALL click( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, ::sal_Int32 nSlideIndex, const ::com::sun::star::awt::MouseEvent& aOriginalEvent ) throw (::com::sun::star::uno::RuntimeException);

    // XSlideShowListener
    virtual void SAL_CALL slideChange( sal_Int32 nOldSlideIndex, sal_Int32 nNewSlideIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL showEnded( ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // helper
    void gotoPreviousSlide();
    void gotoNextSlide();
    void gotoFirstSlide();
    void gotoLastSlide();
    void endPresentation();
    void gotoSlideIndex( sal_Int32 nPageIndex );
    void enablePen();

    bool pause( bool bPause );

    void receiveRequest(SfxRequest& rReq);

    /** called only by the slideshow view when the first paint event occurs.
        This actually starts the slideshow. */
    void onFirstPaint();

    ShowWindow* getShowWindow() const { return mpShowWindow; }

private:
    bool startShowImpl(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > >& aSlides,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > >& aRootNodes,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProperties );

    SfxViewFrame* getViewFrame() const;

    sal_Int32 getPageNumberForBookmark( const rtl::OUString& rStrBookmark );

    void removeShapeEvents();
    void registerShapeEvents( sal_Int32 nPageNumber );
    void registerShapeEvents( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes ) throw (::com::sun::star::uno::Exception);

    // default: disabled copy/assignment
    SlideshowImpl(const SlideshowImpl&);
    SlideshowImpl& operator=( const SlideshowImpl& );

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShow > createSlideShow() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShow > mxShow;
    comphelper::ImplementationReference< ::sd::SlideShowView, ::com::sun::star::presentation::XSlideShowView > mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;

    Timer maUpdateTimer;
    Timer maInputFreezeTimer;

    ::sd::View* mpView;
    ViewShell* mpViewShell;
    DrawDocShell* mpDocSh;
    SdDrawDocument* mpDoc;

    SfxItemSet*     mpNewAttr;
    ShowWindow*     mpShowWindow;
    SvtSaveOptions* mpSaveOptions;
    PushButton*     mpTimeButton;

    boost::shared_ptr< AnimationPageList > mpAnimationPageList;

    long            mnRestorePage;
    Point           maPageOrigin;
    Size            maPageSize;
    Size            maPresSize;
    AnimationMode   meAnimationMode;
    String          maCharBuffer;
    Pointer         maOldPointer;
    Pointer         maPencil;
    std::vector< ::sd::Window* > maDrawModeWindows;
    ::sd::Window*   mpOldActiveWindow;
    Link            maStarBASICGlobalErrorHdl;
    unsigned long   mnChildMask;
    bool            mbGridVisible;
    bool            mbBordVisible;
    bool            mbPageBorderVisible;
    bool            mbSetOnlineSpelling;
    bool            mbDisposed;
    bool            mbMouseIsDrawing;
    bool            mbAutoSaveSuppressed;
    bool            mbRehearseTimings;
    bool            mbDesignMode;
    bool            mbIsPaused;
    bool            mbInputFreeze;

    PresentationSettings maPresSettings;

    /// used in updateHdl to prevent recursive calls
    sal_Int32       mnEntryCounter;

    sal_Int32       mnLastPageNumber;
    WrappedShapeEventImplMap    maShapeEventMap;

    ::rtl::OUString msOnClick;
    ::rtl::OUString msBookmark;
    ::rtl::OUString msVerb;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > mxPreviewDrawPage;
    ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode > mxPreviewAnimationNode;

    ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > mxPlayer;
    ::com::sun::star::uno::Reference< ::com::sun::star::media::XManager > mxManager;

    ::std::auto_ptr<PaneHider> mpPaneHider;

    ULONG   mnEndShowEvent;
};

class SlideShowImplGuard
{
public:
    SlideShowImplGuard( SlideshowImpl* pImpl );
    ~SlideShowImplGuard();

private:
    SlideshowImpl* mpImpl;
};

bool SlideshowImpl::isInputFreezed() const
{
    return mbInputFreeze;
}

} // namespace ::sd

#endif
