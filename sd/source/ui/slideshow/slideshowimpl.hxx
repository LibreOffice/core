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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWIMPL_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESHOW_SLIDESHOWIMPL_HXX

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/propertysetmixin.hxx"
#include <com/sun/star/awt/XActivateListener.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include "com/sun/star/presentation/XShapeEventListener.hpp"
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XPointer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <toolkit/helper/vclunohelper.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <vcl/help.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/saveopt.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <svx/svdpagv.hxx>
#include <svx/fmshell.hxx>

#include <svx/svxids.hrc>
#include "sdmod.hxx"
#include "cusshow.hxx"
#include "ViewShellBase.hxx"
#include "PresentationViewShell.hxx"
#include "ViewShell.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"

#include "showwindow.hxx"

#include "optsitem.hxx"
#include "FrameView.hxx"
#include "DrawDocShell.hxx"

#include "app.hrc"

#include "slideshow.hxx"

class SfxViewFrame;
class SfxRequest;
class ShowWindow;

namespace sd
{
class SlideShowView;
class AnimationSlideController;
class PaneHider;

struct PresentationSettingsEx : public PresentationSettings
{
    bool mbRehearseTimings;
    bool mbPreview;
    VclPtr<vcl::Window> mpParentWindow;
    css::uno::Reference< css::drawing::XDrawPage > mxStartPage;
    css::uno::Reference< css::animations::XAnimationNode > mxAnimationNode;

    PresentationSettingsEx( const PresentationSettingsEx& );
    PresentationSettingsEx( PresentationSettings& );

    void SetArguments( const css::uno::Sequence< css::beans::PropertyValue >& rArguments ) throw (css::lang::IllegalArgumentException, std::exception);

    void SetPropertyValue( const OUString& rProperty, const css::uno::Any& rValue ) throw (css::lang::IllegalArgumentException, std::exception);
};

struct WrappedShapeEventImpl
{
    css::presentation::ClickAction meClickAction;
    sal_Int32 mnVerb;
    OUString maStrBookmark;
    WrappedShapeEventImpl() : meClickAction( css::presentation::ClickAction_NONE ), mnVerb( 0 ) {};
};

typedef std::shared_ptr< WrappedShapeEventImpl > WrappedShapeEventImplPtr;
typedef std::map< css::uno::Reference< css::drawing::XShape >, WrappedShapeEventImplPtr > WrappedShapeEventImplMap;

class SlideShowListenerProxy : private ::cppu::BaseMutex,
        public ::cppu::WeakImplHelper< css::presentation::XSlideShowListener, css::presentation::XShapeEventListener >
{
public:
    SlideShowListenerProxy( const rtl::Reference< SlideshowImpl >& xController, const css::uno::Reference< css::presentation::XSlideShow >& xSlideShow );
    virtual ~SlideShowListenerProxy();

    void addAsSlideShowListener();
    void removeAsSlideShowListener();

    void addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener );
    void removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener );

    void addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape );
    void removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape );

    // css::animations::XAnimationListener
    virtual void SAL_CALL beginEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL repeat( const css::uno::Reference< css::animations::XAnimationNode >& Node, ::sal_Int32 Repeat ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::presentation::XSlideShowListener:
    virtual void SAL_CALL paused() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL resumed() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideTransitionStarted() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideTransitionEnded() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideAnimationsEnded() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL hyperLinkClicked(const OUString & hyperLink) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::lang::XEventListener:
    virtual void SAL_CALL disposing(const css::lang::EventObject & Source) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // css::presentation::XShapeEventListener:
    virtual void SAL_CALL click(const css::uno::Reference< css::drawing::XShape > & xShape, const css::awt::MouseEvent & aOriginalEvent) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    ::cppu::OInterfaceContainerHelper maListeners;

    rtl::Reference< SlideshowImpl > mxController;
    css::uno::Reference< css::presentation::XSlideShow > mxSlideShow;
};

typedef ::cppu::WeakComponentImplHelper< css::presentation::XSlideShowController, css::container::XIndexAccess > SlideshowImplBase;

class SlideshowImpl : private ::cppu::BaseMutex, public SlideshowImplBase
{
friend class SlideShow;
friend class SlideShowView;

public:
    explicit SlideshowImpl( const css::uno::Reference< css::presentation::XPresentation2 >& xPresentation, ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc, vcl::Window* pParentWindow);

    // css::presentation::XSlideShowController:
    virtual sal_Bool SAL_CALL getAlwaysOnTop() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setAlwaysOnTop( sal_Bool _alwaysontop ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getMouseVisible() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setMouseVisible( sal_Bool _mousevisible ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getUsePen() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setUsePen( sal_Bool _usepen ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getPenColor() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPenColor( ::sal_Int32 _pencolor ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getPenWidth() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPenWidth( double dStrokeWidth ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL setEraseAllInk( bool bEraseAllInk ) throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isRunning(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getSlideCount(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getSlideByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoNextEffect(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoPreviousEffect(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoFirstSlide(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoNextSlide(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoPreviousSlide(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoLastSlide(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoBookmark( const OUString& Bookmark ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoSlide( const css::uno::Reference< css::drawing::XDrawPage >& Page ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL gotoSlideIndex( ::sal_Int32 Index ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL stopSound(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL pause(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL resume(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isPaused(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL blankScreen( ::sal_Int32 Color ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL activate(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL deactivate(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isActive(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getCurrentSlide(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCurrentSlideIndex(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getNextSlideIndex(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isEndless(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isFullScreen(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual css::uno::Reference< css::presentation::XSlideShow > SAL_CALL getSlideShow(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // will be called from the SlideShowListenerProxy when this event is fired from the XSlideShow
    void slideEnded(const bool bReverse);
    void hyperLinkClicked(const OUString & hyperLink) throw (css::uno::RuntimeException, std::exception);
    void click(const css::uno::Reference< css::drawing::XShape > & xShape, const css::awt::MouseEvent & aOriginalEvent);
    bool swipe(const CommandSwipeData &rSwipeData);
    bool longpress(const CommandLongPressData& rLongPressData);

    /// ends the presentation async
    void endPresentation();

    ViewShell* getViewShell() const { return mpViewShell; }

    void paint( const Rectangle& rRect );
    bool keyInput(const KeyEvent& rKEvt);
    void mouseButtonUp(const MouseEvent& rMEvt);

private:
    SlideshowImpl(SlideshowImpl &) SAL_DELETED_FUNCTION;
    void operator =(SlideshowImpl &) SAL_DELETED_FUNCTION;

    virtual ~SlideshowImpl();

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // internal
    bool startShow( PresentationSettingsEx* pPresSettings );
    bool startPreview(
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode,
        vcl::Window* pParent );

        /** forces an async call to update in the main thread */
    void startUpdateTimer();

    double update();

    void createSlideList( bool bAll, const OUString& rPresSlide );

    void displayCurrentSlide (const bool bSkipAllMainSequenceEffects = false);

    void displaySlideNumber( sal_Int32 nSlide );
    void displaySlideIndex( sal_Int32 nIndex );
    sal_Int32 getCurrentSlideNumber();
    sal_Int32 getFirstSlideNumber();
    sal_Int32 getLastSlideNumber();
    inline bool isInputFreezed() const { return mbInputFreeze; }

    void jumpToBookmark( const OUString& sBookmark );

    void hideChildWindows();
    void showChildWindows();

    void resize( const Size& rSize );

    void setActiveXToolbarsVisible( bool bVisible );

    DECL_LINK_TYPED( updateHdl, Timer *, void );
    DECL_LINK_TYPED( PostYieldListener, LinkParamNone*, void );
    DECL_LINK_TYPED( ReadyForNextInputHdl, Timer *, void );
    DECL_LINK_TYPED( endPresentationHdl, void*, void );
    DECL_LINK_TYPED( ContextMenuSelectHdl, Menu *, bool );
    DECL_LINK_TYPED( ContextMenuHdl, void*, void );
    DECL_LINK_TYPED( deactivateHdl, Timer *, void );
    DECL_LINK_TYPED( EventListenerHdl, VclSimpleEvent&, void );

    // helper
    void receiveRequest(SfxRequest& rReq);

    /** called only by the slideshow view when the first paint event occurs.
        This actually starts the slideshow. */
    void onFirstPaint();

    long getRestoreSlide() const { return mnRestoreSlide; }

private:
    bool startShowImpl(
        const css::uno::Sequence< css::beans::PropertyValue >& aProperties );

    SfxViewFrame* getViewFrame() const;
    SfxDispatcher* getDispatcher() const;
    SfxBindings* getBindings() const;

    sal_Int32 getSlideNumberForBookmark( const OUString& rStrBookmark );

    void removeShapeEvents();
    void registerShapeEvents( sal_Int32 nSlideNumber );
    void registerShapeEvents( css::uno::Reference< css::drawing::XShapes >& xShapes ) throw (css::uno::Exception, std::exception);

    static css::uno::Reference< css::presentation::XSlideShow > createSlideShow();

    static void setAutoSaveState( bool bOn );
    void gotoPreviousSlide (const bool bSkipAllMainSequenceEffects);

    /** Called by PostYieldListener and updateHdl handlers this method is
        responsible to call the slideshow update() method and, depending on
        its return value, wait for a certain amount of time before another
        call to update() is scheduled.
    */
    sal_Int32 updateSlideShow();

    css::uno::Reference< css::presentation::XSlideShow > mxShow;
    rtl::Reference<sd::SlideShowView> mxView;
    css::uno::Reference< css::frame::XModel > mxModel;

    Timer maUpdateTimer;
    Timer maInputFreezeTimer;
    Timer maDeactivateTimer;

    ::sd::View* mpView;
    ViewShell* mpViewShell;
    DrawDocShell* mpDocSh;
    SdDrawDocument* mpDoc;

    SfxItemSet*     mpNewAttr;
    VclPtr<vcl::Window>    mpParentWindow;
    VclPtr<ShowWindow>     mpShowWindow;
    VclPtr<PushButton>     mpTimeButton;

    std::shared_ptr< AnimationSlideController > mpSlideController;

    long            mnRestoreSlide;
    Point           maSlideOrigin;
    Point           maPopupMousePos;
    Size            maSlideSize;
    Size            maPresSize;
    AnimationMode   meAnimationMode;
    OUString        maCharBuffer;
    Pointer         maOldPointer;
    Pointer         maPencil;
    std::vector< VclPtr< ::sd::Window> > maDrawModeWindows;
    VclPtr< ::sd::Window>   mpOldActiveWindow;
    Link<StarBASIC*,bool>   maStarBASICGlobalErrorHdl;
    unsigned long   mnChildMask;
    bool            mbGridVisible;
    bool            mbBordVisible;
    bool            mbSlideBorderVisible;
    bool            mbSetOnlineSpelling;
    bool            mbDisposed;
    bool            mbAutoSaveWasOn;
    bool            mbRehearseTimings;
    bool            mbDesignMode;
    bool            mbIsPaused;
    bool            mbWasPaused;        // used to cache pause state during context menu
    bool            mbInputFreeze;
    bool        mbActive;

    PresentationSettings maPresSettings;
    sal_Int32       mnUserPaintColor;

    bool            mbUsePen;
    double          mdUserPaintStrokeWidth;

    /// used in updateHdl to prevent recursive calls
    sal_Int32       mnEntryCounter;

    sal_Int32       mnLastSlideNumber;
    WrappedShapeEventImplMap    maShapeEventMap;

    OUString msOnClick;
    OUString msBookmark;
    OUString msVerb;

    css::uno::Reference< css::drawing::XDrawPage > mxPreviewDrawPage;
    css::uno::Reference< css::animations::XAnimationNode > mxPreviewAnimationNode;

    css::uno::Reference< css::media::XPlayer > mxPlayer;

    ::std::unique_ptr<PaneHider> mpPaneHider;

    ImplSVEvent * mnEndShowEvent;
    ImplSVEvent * mnContextMenuEvent;

    css::uno::Reference< css::presentation::XPresentation2 > mxPresentation;
    ::rtl::Reference< SlideShowListenerProxy > mxListenerProxy;
};

} // namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
