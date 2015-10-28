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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SLIDESHOW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SLIDESHOW_HXX

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ref.hxx>

#include <tools/link.hxx>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <editeng/unoipset.hxx>

#include <memory>

namespace com { namespace sun { namespace star {

    namespace drawing {
        class XDrawPage;
    }
    namespace animations {
        class XAnimationNode;
    }
} } }

class SdDrawDocument;
class KeyEvent;
class OutputDevice;
class Size;
class Rectangle;
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
class Window;
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
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument* pDocument );
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument& rDocument );
    static rtl::Reference< SlideShow > GetSlideShow( ViewShellBase& rBase );

    static css::uno::Reference< css::presentation::XSlideShowController > GetSlideShowController(ViewShellBase& rBase );

    static bool StartPreview( ViewShellBase& rBase,
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode,
        vcl::Window* pParent = 0 );

    static void Stop( ViewShellBase& rBase );

    /// returns true if there is a running presentation for the given ViewShellBase
    static bool IsRunning( ViewShellBase& rBase );

    /// returns true if there is a running presentation inside the given ViewShell
    /// returns false even if there is a running presentation but in another ViewShell
    static bool IsRunning( ViewShell& rViewShell );

    // helper api

    bool startPreview(
        const css::uno::Reference< css::drawing::XDrawPage >& xDrawPage,
        const css::uno::Reference< css::animations::XAnimationNode >& xAnimationNode,
        vcl::Window* pParent = 0 );

    // uno api

        virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XPresentation
    virtual void SAL_CALL start(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL end()
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL rehearseTimings(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XPresentation2
    virtual void SAL_CALL startWithArguments(const css::uno::Sequence< css::beans::PropertyValue >& Arguments)
        throw (css::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL isRunning(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::presentation::XSlideShowController > SAL_CALL getController(  ) throw (css::uno::RuntimeException, std::exception) override;

    // legacy api

    // actions
    void jumpToPageNumber( sal_Int32 nPage );               // a.k.a. FuSlideShow::JumpToPage()
    void jumpToPageIndex( sal_Int32 nIndex );
    void jumpToBookmark( const OUString& sBookmark );            // a.k.a. FuSlideShow::JumpToBookmark()

    /** sets or clears the pause state of the running slideshow.
        !!!! This should only be called by the SdShowWindow !!!!*/
    bool pause( bool bPause );
    bool swipe(const CommandSwipeData &rSwipeData);
    bool longpress(const CommandLongPressData& rLongPressData);

    // settings
    bool isFullScreen();                                // a.k.a. FuSlideShow::IsFullScreen()
    bool isAlwaysOnTop();                               // a.k.a. FuSlideShow::IsAlwaysOnTop();
    OutputDevice* getShowWindow();                      // a.k.a. FuSlideShow::GetShowWindow()
    int getAnimationMode();                             // a.k.a. FuSlideShow::GetAnimationMode()
    sal_Int32 getCurrentPageNumber();                   // a.k.a. FuSlideShow::GetCurrentPage()
    sal_Int32 getFirstPageNumber();
    sal_Int32 getLastPageNumber();
    bool isEndless();
    bool isDrawingPossible();

    // events
    void resize( const Size &rSize );
    void activate(ViewShellBase& rBase);
    void deactivate(ViewShellBase& rBase);
    void paint( const Rectangle& rRect );

    bool keyInput(const KeyEvent& rKEvt);

    void receiveRequest(SfxRequest& rReq);

    bool dependsOn( ViewShellBase* pViewShellBase );

    static sal_Int32 GetDisplay();

    bool IsExitAfterPresenting() const;
    void SetExitAfterPresenting(bool bExit);

private:
    SlideShow( SdDrawDocument* pDoc );

    DECL_LINK_TYPED( StartInPlacePresentationConfigurationHdl, void *, void );
    void StartInPlacePresentationConfigurationCallback();

    void StartInPlacePresentation();
    void StartFullscreenPresentation();

    void ThrowIfDisposed() const throw (css::uno::RuntimeException);

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
    void ShowSlideShow(SfxRequest& rReq, SdDrawDocument &rDoc);
}

}

#endif // INCLUDED_SD_SOURCE_UI_INC_SLIDESHOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
