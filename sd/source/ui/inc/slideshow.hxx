/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: slideshow.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _SD_SLIDESHOW_HXX
#define _SD_SLIDESHOW_HXX

#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ref.hxx>

#include <tools/link.hxx>

#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

#include <svx/unoipset.hxx>

#include <memory>
#include <boost/shared_ptr.hpp>

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
class HelpEvent;
class MouseEvent;
class Size;
class CommandEvent;
class Rectangle;
class Window;
class SfxRequest;

// TODO: Remove
#define PAGE_NO_END         65535
#define PAGE_NO_SOFTEND     (PAGE_NO_END - 1)
#define PAGE_NO_PAUSE       (PAGE_NO_SOFTEND - 1)
#define PAGE_NO_FIRSTDEF    PAGE_NO_PAUSE // immer mit anpassen

/* Definition of SlideShow class */

namespace sd
{

class SlideshowImpl;
class ShowWindow;
class Window;
class View;
class ViewShell;
struct PresentationSettings;
class EffectSequenceHelper;
class ViewShellBase;
struct PresentationSettingsEx;
class FrameView;

enum AnimationMode
{
    ANIMATIONMODE_SHOW,
    ANIMATIONMODE_VIEW,
    ANIMATIONMODE_PREVIEW
};

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::presentation::XPresentation2, ::com::sun::star::lang::XServiceInfo > SlideshowBase;

class SlideShow : private ::cppu::BaseMutex, public SlideshowBase
{
public:
    /// used by the model to create a slideshow for it
    static rtl::Reference< SlideShow > Create( SdDrawDocument* pDoc );

    // static helper api
    static rtl::Reference< SlideShow > GetSlideShow( SdDrawDocument* pDocument );
    static rtl::Reference< SlideShow > GetSlideShow( ViewShellBase& rBase );

    static ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowController > GetSlideShowController(ViewShellBase& rBase );

    static bool StartPreview( ViewShellBase& rBase,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent = 0 );

    static void Stop( ViewShellBase& rBase );

    /// returns true if there is a running presentation for the given ViewShellBase
    static bool IsRunning( ViewShellBase& rBase );

    /// returns true if there is a running presentation inside the given ViewShell
    /// returns false even if there is a running presentation but in another ViewShell
    static bool IsRunning( ViewShell& rViewShell );

    // helper api

    bool startPreview(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent = 0 );

    // uno api

        virtual void SAL_CALL disposing (void);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XPresentation
    virtual void SAL_CALL start(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL end(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rehearseTimings(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPresentation2
    virtual void SAL_CALL startWithArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isRunning(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowController > SAL_CALL getController(  ) throw (::com::sun::star::uno::RuntimeException);

    // legacy api

    // actions
    void jumpToPageNumber( sal_Int32 nPage );               // a.k.a. FuSlideShow::JumpToPage()
    void jumpToPageIndex( sal_Int32 nIndex );
    void jumpToBookmark( const ::rtl::OUString& sBookmark );            // a.k.a. FuSlideShow::JumpToBookmark()

    /** sets or clears the pause state of the running slideshow.
        !!!! This should only be called by the SdShowWindow !!!!*/
    bool pause( bool bPause );


    // settings
    bool isFullScreen();                                // a.k.a. FuSlideShow::IsFullScreen()
    bool isAlwaysOnTop();                               // a.k.a. FuSlideShow::IsAlwaysOnTop();
    ShowWindow* getShowWindow();                        // a.k.a. FuSlideShow::GetShowWindow()
    int getAnimationMode();                             // a.k.a. FuSlideShow::GetAnimationMode()
    sal_Int32 getCurrentPageNumber();                   // a.k.a. FuSlideShow::GetCurrentPage()
    sal_Int32 getCurrentPageIndex();
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

private:
    SlideShow( SdDrawDocument* pDoc );

    DECL_LINK( StartInPlacePresentationConfigurationHdl, void * );
    void StartInPlacePresentationConfigurationCallback();

    void StartInPlacePresentation();
    void StartFullscreenPresentation();

    void ThrowIfDisposed() throw (::com::sun::star::uno::RuntimeException);

    void CreateController( ViewShell* pViewSh, ::sd::View* pView, ::Window* pParentWindow );

    // default: disabled copy/assignment
    SlideShow(const SlideShow&);
    SlideShow& operator=( const SlideShow& );

    SvxItemPropertySet  maPropSet;

    rtl::Reference< SlideshowImpl > mxController;
    SdDrawDocument* mpDoc;

    boost::shared_ptr< PresentationSettingsEx > mxCurrentSettings;

    ViewShellBase* mpCurrentViewShellBase;
    ViewShellBase* mpFullScreenViewShellBase;
    FrameView* mpFullScreenFrameView;
    sal_Int32   mnInPlaceConfigEvent;
};

}

#endif /* _SD_SLIDESHOW_HXX */
