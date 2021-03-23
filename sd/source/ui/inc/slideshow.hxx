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

#pragma once

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ref.hxx>

#include <tools/link.hxx>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <editeng/unoipset.hxx>

#include <memory>

namespace com::sun::star {
    namespace drawing {
        class XDrawPage;
    }
    namespace animations {
        class XAnimationNode;
    }
}
class SdDrawDocument;
class KeyEvent;
class OutputDevice;
class Size;
namespace vcl { class Window; }
class SfxRequest;
class WorkWindow;
class CommandSwipeData;
class CommandLongPressData;
struct ImplSVEvent;

// TODO: Remove
#define PAGE_NO_END         65535

/* Definition of SlideShow class */

namespace sd
{

class SlideshowImpl;
class View;
class ViewShell;
class ViewShellBase;
struct PresentationSettingsEx;
class FrameView;

enum AnimationMode
{
    ANIMATIONMODE_SHOW,
    ANIMATIONMODE_PREVIEW
};

typedef ::cppu::WeakComponentImplHelper< css::presentation::XPresentation2, css::lang::XServiceInfo > SlideshowBase;

class SlideShow : private ::cppu::BaseMutex, public SlideshowBase
{
public:
    /// used by the model to create a slideshow for it
    static rtl::Reference< SlideShow > Create( SdDrawDocument* pDoc );

    // static helper api
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument const * pDocument );
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument const & rDocument );
    static rtl::Reference< SlideShow > GetSlideShow( ViewShellBase const & rBase );

    static css::uno::Reference< css::presentation::XSlideShowController > GetSlideShowController(ViewShellBase const & rBase );

    static bool StartPreview( ViewShellBase const & rBase,
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode );

    static void Stop( ViewShellBase const & rBase );

    /// returns true if there is a running presentation for the given ViewShellBase
    static bool IsRunning( ViewShellBase const & rBase );

    /// returns true if there is a running presentation inside the given ViewShell
    /// returns false even if there is a running presentation but in another ViewShell
    static bool IsRunning( ViewShell& rViewShell );

    // helper api

    void startPreview(
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode );

    // uno api

        virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XPresentation
    virtual void SAL_CALL start(  ) override;
    virtual void SAL_CALL end() override;
    virtual void SAL_CALL rehearseTimings(  ) override;

    // XPresentation2
    virtual void SAL_CALL startWithArguments(const css::uno::Sequence< css::beans::PropertyValue >& Arguments) override;
    virtual sal_Bool SAL_CALL isRunning(  ) override;
    virtual css::uno::Reference< css::presentation::XSlideShowController > SAL_CALL getController(  ) override;

    // legacy api

    // actions
    void jumpToPageNumber( sal_Int32 nPage );               // a.k.a. FuSlideShow::JumpToPage()
    void jumpToPageIndex( sal_Int32 nIndex );
    void jumpToBookmark( const OUString& sBookmark );            // a.k.a. FuSlideShow::JumpToBookmark()

    /** sets or clears the pause state of the running slideshow.
        !!!! This should only be called by the SdShowWindow !!!!*/
    void pause( bool bPause );
    bool swipe(const CommandSwipeData &rSwipeData);
    bool longpress(const CommandLongPressData& rLongPressData);

    // settings
    bool isFullScreen() const;                          // a.k.a. FuSlideShow::IsFullScreen()
    OutputDevice* getShowWindow();                      // a.k.a. FuSlideShow::GetShowWindow()
    int getAnimationMode() const;                       // a.k.a. FuSlideShow::GetAnimationMode()
    sal_Int32 getCurrentPageNumber() const;             // a.k.a. FuSlideShow::GetCurrentPage()

    // events
    void resize( const Size &rSize );
    // return false if the activate failed. callers should call end in response to failure
    bool activate(ViewShellBase& rBase);
    void deactivate();
    void paint();

    bool keyInput(const KeyEvent& rKEvt);

    bool dependsOn( ViewShellBase const * pViewShellBase );

    static sal_Int32 GetDisplay();

    bool IsExitAfterPresenting() const;
    void SetExitAfterPresenting(bool bExit);

private:
    SlideShow( SdDrawDocument* pDoc );

    DECL_LINK( StartInPlacePresentationConfigurationHdl, void *, void );
    void StartInPlacePresentationConfigurationCallback();

    void StartInPlacePresentation();
    void StartFullscreenPresentation();

    /// @throws css::uno::RuntimeException
    void ThrowIfDisposed() const;

    void CreateController( ViewShell* pViewSh, ::sd::View* pView, vcl::Window* pParentWindow );
    WorkWindow *GetWorkWindow();

    SlideShow(const SlideShow&) = delete;
    SlideShow& operator=( const SlideShow& ) = delete;

    SvxItemPropertySet  maPropSet;

    rtl::Reference< SlideshowImpl > mxController;
    /** This flag is used together with mxController.is() to prevent
        multiple instances of the slide show for one document.  The flag
        covers the time before mxController is set.
    */
    bool mbIsInStartup;
    SdDrawDocument* mpDoc;

    std::shared_ptr< PresentationSettingsEx > mxCurrentSettings;

    ViewShellBase* mpCurrentViewShellBase;
    ViewShellBase* mpFullScreenViewShellBase;
    FrameView* mpFullScreenFrameView;
    ImplSVEvent * mnInPlaceConfigEvent;
};

namespace slideshowhelp
{
    void ShowSlideShow(SfxRequest const & rReq, SdDrawDocument &rDoc);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
