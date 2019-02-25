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

#include <memory>
#include <sal/config.h>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XShapeEventListener.hpp>

#include <drawdoc.hxx>

#include "showwindow.hxx"

#include <slideshow.hxx>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace media { class XPlayer; } } } }
namespace sd { class DrawDocShell; }
namespace sd { class ViewShell; }

class SfxBindings;
class SfxDispatcher;
class SfxViewFrame;
class StarBASIC;

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
    explicit PresentationSettingsEx( PresentationSettings const & );

    /// @throws css::lang::IllegalArgumentException
    void SetArguments( const css::uno::Sequence< css::beans::PropertyValue >& rArguments );

    /// @throws css::lang::IllegalArgumentException
    void SetPropertyValue( const OUString& rProperty, const css::uno::Any& rValue );
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
    virtual ~SlideShowListenerProxy() override;

    void addAsSlideShowListener();
    void removeAsSlideShowListener();

    void addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener );
    void removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener );

    void addShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape );
    void removeShapeEventListener( const css::uno::Reference< css::drawing::XShape >& xShape );

    // css::animations::XAnimationListener
    virtual void SAL_CALL beginEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) override;
    virtual void SAL_CALL endEvent( const css::uno::Reference< css::animations::XAnimationNode >& Node ) override;
    virtual void SAL_CALL repeat( const css::uno::Reference< css::animations::XAnimationNode >& Node, ::sal_Int32 Repeat ) override;

    // css::presentation::XSlideShowListener:
    virtual void SAL_CALL paused() override;
    virtual void SAL_CALL resumed() override;
    virtual void SAL_CALL slideTransitionStarted() override;
    virtual void SAL_CALL slideTransitionEnded() override;
    virtual void SAL_CALL slideAnimationsEnded() override;
    virtual void SAL_CALL slideEnded(sal_Bool bReverse) override;
    virtual void SAL_CALL hyperLinkClicked(const OUString & hyperLink) override;

    // css::lang::XEventListener:
    virtual void SAL_CALL disposing(const css::lang::EventObject & Source) override;

    // css::presentation::XShapeEventListener:
    virtual void SAL_CALL click(const css::uno::Reference< css::drawing::XShape > & xShape, const css::awt::MouseEvent & aOriginalEvent) override;

private:
    ::comphelper::OInterfaceContainerHelper2 maListeners;
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
    virtual sal_Bool SAL_CALL getAlwaysOnTop() override;
    virtual void SAL_CALL setAlwaysOnTop( sal_Bool _alwaysontop ) override;
    virtual sal_Bool SAL_CALL getMouseVisible() override;
    virtual void SAL_CALL setMouseVisible( sal_Bool _mousevisible ) override;
    virtual sal_Bool SAL_CALL getUsePen() override;
    virtual void SAL_CALL setUsePen( sal_Bool _usepen ) override;
    virtual ::sal_Int32 SAL_CALL getPenColor() override;
    virtual void SAL_CALL setPenColor( ::sal_Int32 _pencolor ) override;
    virtual double SAL_CALL getPenWidth() override;
    virtual void SAL_CALL setPenWidth( double dStrokeWidth ) override;
    /// @throws css::uno::RuntimeException
    void setEraseAllInk( bool bEraseAllInk );
    virtual sal_Bool SAL_CALL isRunning(  ) override;
    virtual ::sal_Int32 SAL_CALL getSlideCount(  ) override;
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getSlideByIndex( ::sal_Int32 Index ) override;
    virtual void SAL_CALL addSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener ) override;
    virtual void SAL_CALL removeSlideShowListener( const css::uno::Reference< css::presentation::XSlideShowListener >& Listener ) override;
    virtual void SAL_CALL gotoNextEffect(  ) override;
    virtual void SAL_CALL gotoPreviousEffect(  ) override;
    virtual void SAL_CALL gotoFirstSlide(  ) override;
    virtual void SAL_CALL gotoNextSlide(  ) override;
    virtual void SAL_CALL gotoPreviousSlide(  ) override;
    virtual void SAL_CALL gotoLastSlide(  ) override;
    virtual void SAL_CALL gotoBookmark( const OUString& Bookmark ) override;
    virtual void SAL_CALL gotoSlide( const css::uno::Reference< css::drawing::XDrawPage >& Page ) override;
    virtual void SAL_CALL gotoSlideIndex( ::sal_Int32 Index ) override;
    virtual void SAL_CALL stopSound(  ) override;
    virtual void SAL_CALL pause(  ) override;
    virtual void SAL_CALL resume(  ) override;
    virtual sal_Bool SAL_CALL isPaused(  ) override;
    virtual void SAL_CALL blankScreen( ::sal_Int32 Color ) override;
    virtual void SAL_CALL activate(  ) override;
    virtual void SAL_CALL deactivate(  ) override;
    virtual sal_Bool SAL_CALL isActive(  ) override;
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL getCurrentSlide(  ) override;
    virtual ::sal_Int32 SAL_CALL getCurrentSlideIndex(  ) override;
    virtual ::sal_Int32 SAL_CALL getNextSlideIndex(  ) override;
    virtual sal_Bool SAL_CALL isEndless(  ) override;
    virtual sal_Bool SAL_CALL isFullScreen(  ) override;
    virtual css::uno::Reference< css::presentation::XSlideShow > SAL_CALL getSlideShow(  ) override;

    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // will be called from the SlideShowListenerProxy when this event is fired from the XSlideShow
    void slideEnded(const bool bReverse);
    /// @throws css::uno::RuntimeException
    void hyperLinkClicked(const OUString & hyperLink);
    void click(const css::uno::Reference< css::drawing::XShape > & xShape);
    bool swipe(const CommandSwipeData &rSwipeData);
    bool longpress(const CommandLongPressData& rLongPressData);

    /// ends the presentation async
    void endPresentation();

    ViewShell* getViewShell() const { return mpViewShell; }

    void paint();
    bool keyInput(const KeyEvent& rKEvt);
    void mouseButtonUp(const MouseEvent& rMEvt);

private:
    SlideshowImpl(SlideshowImpl const &) = delete;
    void operator =(SlideshowImpl const &) = delete;

    virtual ~SlideshowImpl() override;

    // override WeakComponentImplHelperBase::disposing()
    // This function is called upon disposing the component,
    // if your component needs special work when it becomes
    // disposed, do it here.
    virtual void SAL_CALL disposing() override;

    // internal
    bool startShow( PresentationSettingsEx const * pPresSettings );
    bool startPreview(
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode,
        vcl::Window* pParent );

        /** forces an async call to update in the main thread */
    void startUpdateTimer();

    void update();

    void createSlideList( bool bAll, const OUString& rPresSlide );

    void displayCurrentSlide (const bool bSkipAllMainSequenceEffects = false);

    void displaySlideNumber( sal_Int32 nSlide );
    void displaySlideIndex( sal_Int32 nIndex );
    sal_Int32 getCurrentSlideNumber();
    bool isInputFreezed() const { return mbInputFreeze; }

    void jumpToBookmark( const OUString& sBookmark );

    void hideChildWindows();
    void showChildWindows();

    void resize( const Size& rSize );

    void setActiveXToolbarsVisible( bool bVisible );

    DECL_LINK( updateHdl, Timer *, void );
    DECL_LINK( ReadyForNextInputHdl, Timer *, void );
    DECL_LINK( endPresentationHdl, void*, void );
    DECL_LINK( ContextMenuSelectHdl, Menu *, bool );
    DECL_LINK( ContextMenuHdl, void*, void );
    DECL_LINK( deactivateHdl, Timer *, void );
    DECL_LINK( EventListenerHdl, VclSimpleEvent&, void );

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
    /// @throws css::uno::Exception
    void registerShapeEvents( css::uno::Reference< css::drawing::XShapes > const & xShapes );

    static css::uno::Reference< css::presentation::XSlideShow > createSlideShow();

    static void setAutoSaveState( bool bOn );
    void gotoPreviousSlide (const bool bSkipAllMainSequenceEffects);

    /** Called by our maUpdateTimer's updateHdl handler this method is
        responsible to call the slideshow update() method and, depending on
        its return value, wait for a certain amount of time before another
        call to update() is scheduled.
    */
    void updateSlideShow();

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

    VclPtr<vcl::Window>    mpParentWindow;
    VclPtr<sd::ShowWindow>     mpShowWindow;

    std::shared_ptr< AnimationSlideController > mpSlideController;

    long            mnRestoreSlide;
    Point           maPopupMousePos;
    Size            maPresSize;
    AnimationMode   meAnimationMode;
    OUString        maCharBuffer;
    VclPtr< ::sd::Window>   mpOldActiveWindow;
    Link<StarBASIC*,bool>   maStarBASICGlobalErrorHdl;
    unsigned long   mnChildMask;
    bool            mbDisposed;
    bool            mbAutoSaveWasOn;
    bool            mbRehearseTimings;
    bool            mbIsPaused;
    bool            mbWasPaused;        // used to cache pause state during context menu
    bool            mbInputFreeze;
    bool            mbActive;

    PresentationSettings maPresSettings;
    sal_Int32       mnUserPaintColor;

    bool            mbUsePen;
    double          mdUserPaintStrokeWidth;

    WrappedShapeEventImplMap    maShapeEventMap;

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
