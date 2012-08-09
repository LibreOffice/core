/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/awt/DockingEvent.hpp>
#include <com/sun/star/awt/EndDockingEvent.hpp>
#include <com/sun/star/awt/EndPopupModeEvent.hpp>
#include <com/sun/star/awt/XWindowListener2.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxpointer.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/accessibilityclient.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/button.hxx>
#include <comphelper/asyncnotification.hxx>
#include <comphelper/flagguard.hxx>
#include "stylesettings.hxx"
#include <tools/urlobj.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::awt::XWindowListener2;
using ::com::sun::star::awt::XDockableWindowListener;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::XStyleSettings;
using ::com::sun::star::lang::DisposedException;
using ::com::sun::star::style::VerticalAlignment;
using ::com::sun::star::style::VerticalAlignment_TOP;
using ::com::sun::star::style::VerticalAlignment_MIDDLE;
using ::com::sun::star::style::VerticalAlignment_BOTTOM;
using ::com::sun::star::style::VerticalAlignment_MAKE_FIXED_SIZE;

namespace WritingMode2 = ::com::sun::star::text::WritingMode2;
namespace MouseWheelBehavior = ::com::sun::star::awt::MouseWheelBehavior;

//====================================================================
//= VCLXWindowImpl
//====================================================================
class SAL_DLLPRIVATE VCLXWindowImpl
{
private:
    typedef ::std::vector< VCLXWindow::Callback >                       CallbackArray;

private:
    VCLXWindow&                         mrAntiImpl;
    ::toolkit::AccessibilityClient      maAccFactory;
    bool                                mbDisposed;
    bool                                mbDrawingOntoParent;    // no bit mask, is passed around  by reference
    sal_Bool                            mbEnableVisible;
    sal_Bool                            mbDirectVisible;

    ::osl::Mutex                        maListenerContainerMutex;
    ::cppu::OInterfaceContainerHelper   maWindow2Listeners;
    ::cppu::OInterfaceContainerHelper   maDockableWindowListeners;
    EventListenerMultiplexer            maEventListeners;
    FocusListenerMultiplexer            maFocusListeners;
    WindowListenerMultiplexer           maWindowListeners;
    KeyListenerMultiplexer              maKeyListeners;
    MouseListenerMultiplexer            maMouseListeners;
    MouseMotionListenerMultiplexer      maMouseMotionListeners;
    PaintListenerMultiplexer            maPaintListeners;
    VclContainerListenerMultiplexer     maContainerListeners;
    TopWindowListenerMultiplexer        maTopWindowListeners;

    CallbackArray                       maCallbackEvents;
    sal_uLong                               mnCallbackEventId;

public:
    bool                                mbDisposing             : 1;
    bool                                mbDesignMode            : 1;
    bool                                mbSynthesizingVCLEvent  : 1;
    bool                                mbWithDefaultProps      : 1;

    sal_uLong                               mnListenerLockLevel;
    sal_Int16                           mnWritingMode;
    sal_Int16                           mnContextWritingMode;

    UnoPropertyArrayHelper*             mpPropHelper;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >
                                        mxPointer;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                                        mxAccessibleContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >
                                        mxViewGraphics;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XStyleSettings >
                                        mxWindowStyleSettings;

public:
    bool&   getDrawingOntoParent_ref()  { return mbDrawingOntoParent; }

public:
    /** ctor
    @param _pAntiImpl
        the <type>VCLXWindow</type> instance which the object belongs to. Must
        live longer then the object just being constructed.
    */
    VCLXWindowImpl( VCLXWindow& _rAntiImpl, bool _bWithDefaultProps );

    /** synchronously mbEnableVisible
    */
    void    setEnableVisible( sal_Bool bEnableVisible ) { mbEnableVisible = bEnableVisible; }
    sal_Bool    isEnableVisible() { return mbEnableVisible; }
    /** synchronously mbDirectVisible;
    */
    void    setDirectVisible( sal_Bool bDirectVisible ) { mbDirectVisible = bDirectVisible; }
    sal_Bool    isDirectVisible() { return mbDirectVisible; }

    /** impl-version of VCLXWindow::ImplExecuteAsyncWithoutSolarLock
    */
    void    callBackAsync( const VCLXWindow::Callback& i_callback );

    /** notifies the object that its VCLXWindow is being disposed
    */
    void    disposing();

    inline ::toolkit::AccessibilityClient& getAccessibleFactory()
    {
        return maAccFactory;
    }

    Reference< XStyleSettings > getStyleSettings();

    /** returns the container of registered XWindowListener2 listeners
    */
    inline ::cppu::OInterfaceContainerHelper&   getWindow2Listeners()       { return maWindow2Listeners; }
    inline ::cppu::OInterfaceContainerHelper&   getDockableWindowListeners(){ return maDockableWindowListeners; }
    inline EventListenerMultiplexer&            getEventListeners()         { return maEventListeners; }
    inline FocusListenerMultiplexer&            getFocusListeners()         { return maFocusListeners; }
    inline WindowListenerMultiplexer&           getWindowListeners()        { return maWindowListeners; }
    inline KeyListenerMultiplexer&              getKeyListeners()           { return maKeyListeners; }
    inline MouseListenerMultiplexer&            getMouseListeners()         { return maMouseListeners; }
    inline MouseMotionListenerMultiplexer&      getMouseMotionListeners()   { return maMouseMotionListeners; }
    inline PaintListenerMultiplexer&            getPaintListeners()         { return maPaintListeners; }
    inline VclContainerListenerMultiplexer&     getContainerListeners()     { return maContainerListeners; }
    inline TopWindowListenerMultiplexer&        getTopWindowListeners()     { return maTopWindowListeners; }

    virtual ~VCLXWindowImpl();

protected:
    virtual void SAL_CALL acquire();
    virtual void SAL_CALL release();

private:
    DECL_LINK( OnProcessCallbacks, void* );

private:
    VCLXWindowImpl();                                   // never implemented
    VCLXWindowImpl( const VCLXWindowImpl& );            // never implemented
    VCLXWindowImpl& operator=( const VCLXWindowImpl& ); // never implemented
};

//--------------------------------------------------------------------
VCLXWindowImpl::VCLXWindowImpl( VCLXWindow& _rAntiImpl, bool _bWithDefaultProps )
    :mrAntiImpl( _rAntiImpl )
    ,mbDisposed( false )
    ,mbDrawingOntoParent( false )
    ,mbEnableVisible(sal_True)
    ,mbDirectVisible(sal_True)
    ,maListenerContainerMutex( )
    ,maWindow2Listeners( maListenerContainerMutex )
    ,maDockableWindowListeners( maListenerContainerMutex )
    ,maEventListeners( _rAntiImpl )
    ,maFocusListeners( _rAntiImpl )
    ,maWindowListeners( _rAntiImpl )
    ,maKeyListeners( _rAntiImpl )
    ,maMouseListeners( _rAntiImpl )
    ,maMouseMotionListeners( _rAntiImpl )
    ,maPaintListeners( _rAntiImpl )
    ,maContainerListeners( _rAntiImpl )
    ,maTopWindowListeners( _rAntiImpl )
    ,mnCallbackEventId( 0 )
    ,mbDisposing( false )
    ,mbDesignMode( false )
    ,mbSynthesizingVCLEvent( false )
    ,mbWithDefaultProps( _bWithDefaultProps )
    ,mnListenerLockLevel( 0 )
    ,mnWritingMode( WritingMode2::CONTEXT )
    ,mnContextWritingMode( WritingMode2::CONTEXT )
    ,mpPropHelper( NULL )
{
}

VCLXWindowImpl::~VCLXWindowImpl()
{
    delete mpPropHelper;
}

//--------------------------------------------------------------------
void VCLXWindowImpl::disposing()
{
    SolarMutexGuard aGuard;
    if ( mnCallbackEventId )
        Application::RemoveUserEvent( mnCallbackEventId );
    mnCallbackEventId = 0;

    mbDisposed= true;

    ::com::sun::star::lang::EventObject aEvent;
    aEvent.Source = mrAntiImpl;

    maDockableWindowListeners.disposeAndClear( aEvent );
    maEventListeners.disposeAndClear( aEvent );
    maFocusListeners.disposeAndClear( aEvent );
    maWindowListeners.disposeAndClear( aEvent );
    maKeyListeners.disposeAndClear( aEvent );
    maMouseListeners.disposeAndClear( aEvent );
    maMouseMotionListeners.disposeAndClear( aEvent );
    maPaintListeners.disposeAndClear( aEvent );
    maContainerListeners.disposeAndClear( aEvent );
    maTopWindowListeners.disposeAndClear( aEvent );

    ::toolkit::WindowStyleSettings* pStyleSettings = static_cast< ::toolkit::WindowStyleSettings* >( mxWindowStyleSettings.get() );
    if ( pStyleSettings != NULL )
        pStyleSettings->dispose();
    mxWindowStyleSettings.clear();
}

//--------------------------------------------------------------------
void VCLXWindowImpl::callBackAsync( const VCLXWindow::Callback& i_callback )
{
    DBG_TESTSOLARMUTEX();
    maCallbackEvents.push_back( i_callback );
    if ( !mnCallbackEventId )
    {
        // ensure our VCLXWindow is not destroyed while the event is underway
        mrAntiImpl.acquire();
        mnCallbackEventId = Application::PostUserEvent( LINK( this, VCLXWindowImpl, OnProcessCallbacks ) );
    }
}

//----------------------------------------------------------------------------------------------------------------------
IMPL_LINK_NOARG(VCLXWindowImpl, OnProcessCallbacks)
{
    const Reference< uno::XInterface > xKeepAlive( mrAntiImpl );

    // work on a copy of the callback array
    CallbackArray aCallbacksCopy;
    {
        SolarMutexGuard aGuard;
        aCallbacksCopy = maCallbackEvents;
        maCallbackEvents.clear();

        // we acquired our VCLXWindow once before posting the event, release this one ref now
        mrAntiImpl.release();

        if ( !mnCallbackEventId )
            // we were disposed while waiting for the mutex to lock
            return 1L;

        mnCallbackEventId = 0;
    }

    {
        SolarMutexReleaser aReleaseSolar;
        for (   CallbackArray::const_iterator loop = aCallbacksCopy.begin();
                loop != aCallbacksCopy.end();
                ++loop
            )
        {
            (*loop)();
        }
    }

    return 0L;
}

//--------------------------------------------------------------------
void SAL_CALL VCLXWindowImpl::acquire()
{
    mrAntiImpl.acquire();
}

//--------------------------------------------------------------------
void SAL_CALL VCLXWindowImpl::release()
{
    mrAntiImpl.release();
}

//--------------------------------------------------------------------
Reference< XStyleSettings > VCLXWindowImpl::getStyleSettings()
{
    SolarMutexGuard aGuard;
    if ( mbDisposed )
        throw DisposedException( ::rtl::OUString(), mrAntiImpl );
    if ( !mxWindowStyleSettings.is() )
        mxWindowStyleSettings = new ::toolkit::WindowStyleSettings( maListenerContainerMutex, mrAntiImpl );
    return mxWindowStyleSettings;
}

//====================================================================
//====================================================================

// Uses an out-parameter instead of return value, due to the object reference

void ImplInitWindowEvent( ::com::sun::star::awt::WindowEvent& rEvent, Window* pWindow )
{
    Point aPos = pWindow->GetPosPixel();
    Size aSz = pWindow->GetSizePixel();

    rEvent.X = aPos.X();
    rEvent.Y = aPos.Y();

    rEvent.Width = aSz.Width();
    rEvent.Height = aSz.Height();

    pWindow->GetBorder( rEvent.LeftInset, rEvent.TopInset, rEvent.RightInset, rEvent.BottomInset );
}

//  ----------------------------------------------------
//  class VCLXWindow
//  ----------------------------------------------------

DBG_NAME(VCLXWindow);

VCLXWindow::VCLXWindow( bool _bWithDefaultProps )
    :mpImpl( NULL )
{
    DBG_CTOR( VCLXWindow, 0 );

    mpImpl = new VCLXWindowImpl( *this, _bWithDefaultProps );
}

VCLXWindow::~VCLXWindow()
{
    DBG_DTOR( VCLXWindow, 0 );

    delete mpImpl;

    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
        GetWindow()->SetWindowPeer( NULL, NULL );
        GetWindow()->SetAccessible( NULL );
    }
}

//----------------------------------------------------------------------------------------------------------------------
void VCLXWindow::ImplExecuteAsyncWithoutSolarLock( const Callback& i_callback )
{
    mpImpl->callBackAsync( i_callback );
}

//----------------------------------------------------------------------------------------------------------------------
::toolkit::IAccessibleFactory& VCLXWindow::getAccessibleFactory()
{
    return mpImpl->getAccessibleFactory().getFactory();
}

void VCLXWindow::SetWindow( Window* pWindow )
{
    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
//        GetWindow()->DbgAssertNoEventListeners();
    }

    SetOutputDevice( pWindow );

    if ( GetWindow() )
    {
        GetWindow()->AddEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
        sal_Bool bDirectVisible = pWindow ? pWindow->IsVisible() : false;
        mpImpl->setDirectVisible( bDirectVisible );
    }
}

void VCLXWindow::suspendVclEventListening( )
{
    ++mpImpl->mnListenerLockLevel;
}

void VCLXWindow::resumeVclEventListening( )
{
    DBG_ASSERT( mpImpl->mnListenerLockLevel, "VCLXWindow::resumeVclEventListening: not suspended!" );
    --mpImpl->mnListenerLockLevel;
}

void VCLXWindow::notifyWindowRemoved( Window& _rWindow )
{
    if ( mpImpl->getContainerListeners().getLength() )
    {
        awt::VclContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Child = static_cast< XWindow* >( _rWindow.GetWindowPeer() );
        mpImpl->getContainerListeners().windowRemoved( aEvent );
    }
}

IMPL_LINK( VCLXWindow, WindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( mpImpl->mnListenerLockLevel )
        return 0L;

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow() && GetWindow(), "Window???" );
        ProcessWindowEvent( *(VclWindowEvent*)pEvent );
    }
    return 0;
}

namespace
{
    struct CallWindow2Listener
    {
        CallWindow2Listener( ::cppu::OInterfaceContainerHelper& i_rWindow2Listeners, const bool i_bEnabled, const EventObject& i_rEvent )
            :m_rWindow2Listeners( i_rWindow2Listeners )
            ,m_bEnabled( i_bEnabled )
            ,m_aEvent( i_rEvent )
        {
        }

        void operator()()
        {
            m_rWindow2Listeners.notifyEach( m_bEnabled ? &XWindowListener2::windowEnabled : &XWindowListener2::windowDisabled, m_aEvent );
        }

        ::cppu::OInterfaceContainerHelper&  m_rWindow2Listeners;
        const bool                          m_bEnabled;
        const EventObject                   m_aEvent;
    };
}

void VCLXWindow::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xThis( (::cppu::OWeakObject*)this );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_ENABLED:
        case VCLEVENT_WINDOW_DISABLED:
        {
            Callback aCallback = CallWindow2Listener(
                mpImpl->getWindow2Listeners(),
                ( VCLEVENT_WINDOW_ENABLED == rVclWindowEvent.GetId() ),
                EventObject( *this )
            );
            ImplExecuteAsyncWithoutSolarLock( aCallback );
        }
        break;

        case VCLEVENT_WINDOW_PAINT:
        {
            if ( mpImpl->getPaintListeners().getLength() )
            {
                ::com::sun::star::awt::PaintEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.UpdateRect = AWTRectangle( *(Rectangle*)rVclWindowEvent.GetData() );
                aEvent.Count = 0;
                mpImpl->getPaintListeners().windowPaint( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowMoved( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_RESIZE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowResized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_SHOW:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowShown( aEvent );
            }

            // For TopWindows this means opened...
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowOpened( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowHidden( aEvent );
            }

            // For TopWindows this means closed...
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowClosed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowActivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowDeactivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_CLOSE:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::closed, aEvent );
            }
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowClosing( aEvent );
            }
        }
        break;
        case VCLEVENT_CONTROL_GETFOCUS:
        case VCLEVENT_WINDOW_GETFOCUS:
        {
            if  (   (   rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_CONTROL_GETFOCUS
                    )
                ||  (   !rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_WINDOW_GETFOCUS
                    )
                )
            {
                if ( mpImpl->getFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
                    aEvent.Temporary = sal_False;
                    mpImpl->getFocusListeners().focusGained( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_CONTROL_LOSEFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            if  (   (   rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_CONTROL_LOSEFOCUS
                    )
                ||  (   !rVclWindowEvent.GetWindow()->IsCompoundControl()
                    &&  rVclWindowEvent.GetId() == VCLEVENT_WINDOW_LOSEFOCUS
                    )
                )
            {
                if ( mpImpl->getFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
                    aEvent.Temporary = sal_False;

                    Window* pNext = Application::GetFocusWindow();
                    if ( pNext )
                    {
                        // Don't care about internals if this control is compound
                        Window* pNextC = pNext;
                        while ( pNextC && !pNextC->IsCompoundControl() )
                            pNextC = pNextC->GetParent();
                        if ( pNextC )
                            pNext = pNextC;

                        pNext->GetComponentInterface( sal_True );
                        aEvent.NextFocus = (::cppu::OWeakObject*)pNext->GetWindowPeer();
                    }
                    mpImpl->getFocusListeners().focusLost( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MINIMIZE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowMinimized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_NORMALIZE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getTopWindowListeners().windowNormalized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            if ( mpImpl->getKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent(
                    *(KeyEvent*)rVclWindowEvent.GetData(), *this
                ) );
                mpImpl->getKeyListeners().keyPressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYUP:
        {
            if ( mpImpl->getKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent(
                    *(KeyEvent*)rVclWindowEvent.GetData(), *this
                ) );
                mpImpl->getKeyListeners().keyReleased( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_COMMAND:
        {
            CommandEvent* pCmdEvt = (CommandEvent*)rVclWindowEvent.GetData();
            if ( mpImpl->getMouseListeners().getLength() && ( pCmdEvt->GetCommand() == COMMAND_CONTEXTMENU ) )
            {
                // COMMAND_CONTEXTMENU als mousePressed mit PopupTrigger = sal_True versenden...
                Point aWhere = static_cast< CommandEvent* >( rVclWindowEvent.GetData() )->GetMousePosPixel();
                if ( !pCmdEvt->IsMouseEvent() )
                {   // for keyboard events, we set the coordinates to -1,-1. This is a slight HACK, but the current API
                    // handles a context menu command as special case of a mouse event, which is simply wrong.
                    // Without extending the API, we would not have another chance to notify listeners of a
                    // keyboard-triggered context menu request
                    aWhere = Point( -1, -1 );
                }

                MouseEvent aMEvt( aWhere, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, 0 );
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( aMEvt, *this ) );
                aEvent.PopupTrigger = sal_True;

                Callback aCallback = ::boost::bind(
                    &MouseListenerMultiplexer::mousePressed,
                    &mpImpl->getMouseListeners(),
                    aEvent
                );
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEMOVE:
        {
            MouseEvent* pMouseEvt = (MouseEvent*)rVclWindowEvent.GetData();
            if ( mpImpl->getMouseListeners().getLength() && ( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() ) )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *pMouseEvt, *this ) );

                Callback aCallback = ::boost::bind(
                    pMouseEvt->IsEnterWindow() ? &MouseListenerMultiplexer::mouseEntered : &MouseListenerMultiplexer::mouseExited,
                    &mpImpl->getMouseListeners(),
                    aEvent
                );
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }

            if ( mpImpl->getMouseMotionListeners().getLength() && !pMouseEvt->IsEnterWindow() && !pMouseEvt->IsLeaveWindow() )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *pMouseEvt, *this ) );
                aEvent.ClickCount = 0;  // #92138#
                if ( pMouseEvt->GetMode() & MOUSE_SIMPLEMOVE )
                    mpImpl->getMouseMotionListeners().mouseMoved( aEvent );
                else
                    mpImpl->getMouseMotionListeners().mouseDragged( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
        {
            if ( mpImpl->getMouseListeners().getLength() )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *(MouseEvent*)rVclWindowEvent.GetData(), *this ) );
                Callback aCallback = ::boost::bind(
                    &MouseListenerMultiplexer::mousePressed,
                    &mpImpl->getMouseListeners(),
                    aEvent
                );
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONUP:
        {
            if ( mpImpl->getMouseListeners().getLength() )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *(MouseEvent*)rVclWindowEvent.GetData(), *this ) );
                Callback aCallback = ::boost::bind(
                    &MouseListenerMultiplexer::mouseReleased,
                    &mpImpl->getMouseListeners(),
                    aEvent
                );
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_STARTDOCKING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                DockingData *pData = (DockingData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::DockingEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.TrackingRectangle = AWTRectangle( pData->maTrackRect );
                    aEvent.MousePos.X = pData->maMousePos.X();
                    aEvent.MousePos.Y = pData->maMousePos.Y();
                    aEvent.bLiveMode = pData->mbLivemode;
                    aEvent.bInteractive = pData->mbInteractive;

                    mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::startDocking, aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_DOCKING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                DockingData *pData = (DockingData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::DockingEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.TrackingRectangle = AWTRectangle( pData->maTrackRect );
                    aEvent.MousePos.X = pData->maMousePos.X();
                    aEvent.MousePos.Y = pData->maMousePos.Y();
                    aEvent.bLiveMode = pData->mbLivemode;
                    aEvent.bInteractive = pData->mbInteractive;

                    Reference< XDockableWindowListener > xFirstListener;
                    ::cppu::OInterfaceIteratorHelper aIter( mpImpl->getDockableWindowListeners() );
                    while ( aIter.hasMoreElements() && !xFirstListener.is() )
                    {
                        xFirstListener.set( aIter.next(), UNO_QUERY );
                    }

                    ::com::sun::star::awt::DockingData aDockingData =
                        xFirstListener->docking( aEvent );
                    pData->maTrackRect = VCLRectangle( aDockingData.TrackingRectangle );
                    pData->mbFloating = aDockingData.bFloating;
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_ENDDOCKING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                EndDockingData *pData = (EndDockingData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::EndDockingEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.WindowRectangle = AWTRectangle( pData->maWindowRect );
                    aEvent.bFloating = pData->mbFloating;
                    aEvent.bCancelled = pData->mbCancelled;
                    mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::endDocking, aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_PREPARETOGGLEFLOATING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                sal_Bool *p_bFloating = (sal_Bool*)rVclWindowEvent.GetData();

                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;

                Reference< XDockableWindowListener > xFirstListener;
                ::cppu::OInterfaceIteratorHelper aIter( mpImpl->getDockableWindowListeners() );
                while ( aIter.hasMoreElements() && !xFirstListener.is() )
                {
                    xFirstListener.set( aIter.next(), UNO_QUERY );
                }

                *p_bFloating = xFirstListener->prepareToggleFloatingMode( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_TOGGLEFLOATING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::toggleFloatingMode, aEvent );
            }
       }
        break;
        case VCLEVENT_WINDOW_ENDPOPUPMODE:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                EndPopupModeData *pData = (EndPopupModeData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::EndPopupModeEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FloatingPosition.X = pData->maFloatingPos.X();
                    aEvent.FloatingPosition.Y = pData->maFloatingPos.Y();
                    aEvent.bTearoff = pData->mbTearoff;
                    mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::endPopupMode, aEvent );
                }
            }
        }
        break;

    }
}

uno::Reference< accessibility::XAccessibleContext > VCLXWindow::CreateAccessibleContext()
{
    SolarMutexGuard aGuard;
    return getAccessibleFactory().createAccessibleContext( this );
}

void VCLXWindow::SetSynthesizingVCLEvent( sal_Bool _b )
{
    mpImpl->mbSynthesizingVCLEvent = _b;
}

sal_Bool VCLXWindow::IsSynthesizingVCLEvent() const
{
    return mpImpl->mbSynthesizingVCLEvent;
}

Size VCLXWindow::ImplCalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
        aSz.Width() += nLeft+nRight;
        aSz.Height() += nTop+nBottom;
    }
    return aSz;
}


// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL2( VCLXWindow, VCLXDevice )

// ::com::sun::star::lang::Component
void VCLXWindow::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->mxViewGraphics = NULL;

    if ( !mpImpl->mbDisposing )
    {
        mpImpl->mbDisposing = true;

        mpImpl->disposing();

        if ( GetWindow() )
        {
            OutputDevice* pOutDev = GetOutputDevice();
            SetWindow( NULL );  // so that handlers are logged off, if necessary (virtual)
            SetOutputDevice( pOutDev );
            DestroyOutputDevice();
        }

        // #i14103# dispose the accessible context after the window has been destroyed,
        // otherwise the old value in the child event fired in VCLXAccessibleComponent::ProcessWindowEvent()
        // for VCLEVENT_WINDOW_CHILDDESTROYED contains a reference to an already disposed accessible object
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent( mpImpl->mxAccessibleContext, ::com::sun::star::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL( "VCLXWindow::dispose: could not dispose the accessible context!" );
        }
        mpImpl->mxAccessibleContext.clear();

        mpImpl->mbDisposing = false;
    }
}

void VCLXWindow::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->getEventListeners().addInterface( rxListener );
}

void VCLXWindow::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->getEventListeners().removeInterface( rxListener );
}


// ::com::sun::star::awt::XWindow
void VCLXWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        if( Window::GetDockingManager()->IsDockable( GetWindow() ) )
            Window::GetDockingManager()->SetPosSizePixel( GetWindow() , X, Y, Width, Height, Flags );
        else
            GetWindow()->SetPosSizePixel( X, Y, Width, Height, Flags );
    }
}

::com::sun::star::awt::Rectangle VCLXWindow::getPosSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Rectangle aBounds;
    if ( GetWindow() )
    {
        if( Window::GetDockingManager()->IsDockable( GetWindow() ) )
            aBounds = AWTRectangle( Window::GetDockingManager()->GetPosSizePixel( GetWindow() ) );
        else
            aBounds = AWTRectangle( Rectangle( GetWindow()->GetPosPixel(), GetWindow()->GetSizePixel() ) );
    }

    return aBounds;
}

void VCLXWindow::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        mpImpl->setDirectVisible( bVisible );
        pWindow->Show( bVisible &&  mpImpl->isEnableVisible() );
    }
}

void VCLXWindow::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->Enable( bEnable, sal_False ); // #95824# without children!
        pWindow->EnableInput( bEnable );
    }
}

void VCLXWindow::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->GrabFocus();
}

void VCLXWindow::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->getWindowListeners().addInterface( rxListener );

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().addInterface( xListener2 );

    // #100119# Get all resize events, even if height or width 0, or invisible
    if ( GetWindow() )
        GetWindow()->EnableAllResize( sal_True );
}

void VCLXWindow::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().removeInterface( xListener2 );

    mpImpl->getWindowListeners().removeInterface( rxListener );
}

void VCLXWindow::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getFocusListeners().addInterface( rxListener );
}

void VCLXWindow::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getFocusListeners().removeInterface( rxListener );
}

void VCLXWindow::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getKeyListeners().addInterface( rxListener );
}

void VCLXWindow::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getKeyListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseMotionListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseMotionListeners().removeInterface( rxListener );
}

void VCLXWindow::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getPaintListeners().addInterface( rxListener );
}

void VCLXWindow::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    mpImpl->getPaintListeners().removeInterface( rxListener );
}

// ::com::sun::star::awt::XWindowPeer
::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > VCLXWindow::getToolkit(  ) throw(::com::sun::star::uno::RuntimeException)
{
    // no guard. nothing to guard here.
    // 82463 - 12/21/00 - fs
    return Application::GetVCLToolkit();
}

void VCLXWindow::setPointer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPointer >& rxPointer ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    VCLXPointer* pPointer = VCLXPointer::GetImplementation( rxPointer );
    if ( pPointer )
    {
        mpImpl->mxPointer = rxPointer;
        if ( GetWindow() )
            GetWindow()->SetPointer( pPointer->GetPointer() );
    }
}

void VCLXWindow::setBackground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetBackground( aColor );
        GetWindow()->SetControlBackground( aColor );

        WindowType eWinType = GetWindow()->GetType();
        if ( ( eWinType == WINDOW_WINDOW ) ||
             ( eWinType == WINDOW_WORKWINDOW ) ||
             ( eWinType == WINDOW_FLOATINGWINDOW ) )
        {
            GetWindow()->Invalidate();
        }
    }
}

void VCLXWindow::invalidate( sal_Int16 nInvalidateFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->Invalidate( (sal_uInt16) nInvalidateFlags );
}

void VCLXWindow::invalidateRect( const ::com::sun::star::awt::Rectangle& rRect, sal_Int16 nInvalidateFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->Invalidate( VCLRectangle(rRect), (sal_uInt16) nInvalidateFlags );
}


// ::com::sun::star::awt::XVclWindowPeer
sal_Bool VCLXWindow::isChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Bool bIsChild = sal_False;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window* pPeerWindow = VCLUnoHelper::GetWindow( rxPeer );
        bIsChild = pPeerWindow && pWindow->IsChild( pPeerWindow );
    }

    return bIsChild;
}

void VCLXWindow::setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->mbDesignMode = bOn;
}

sal_Bool VCLXWindow::isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return mpImpl->mbDesignMode;
}

void VCLXWindow::enableClipSiblings( sal_Bool bClip ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->EnableClipSiblings( bClip );
}

void VCLXWindow::setForeground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetControlForeground( aColor );
    }
}

void VCLXWindow::setControlFont( const ::com::sun::star::awt::FontDescriptor& rFont ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->SetControlFont( VCLUnoHelper::CreateFont( rFont, GetWindow()->GetControlFont() ) );
}

void VCLXWindow::getStyles( sal_Int16 nType, ::com::sun::star::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        const StyleSettings& rStyleSettings = GetWindow()->GetSettings().GetStyleSettings();

        switch ( nType )
        {
            case ::com::sun::star::awt::Style::FRAME:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetWindowTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetWindowColor().GetColor();
            }
            break;
            case ::com::sun::star::awt::Style::DIALOG:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetDialogTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetDialogColor().GetColor();
            }
            break;
            default: OSL_FAIL( "VCLWindow::getStyles() - unknown Type" );
        }

    }
}

namespace toolkit
{
    static void setColorSettings( Window* _pWindow, const ::com::sun::star::uno::Any& _rValue,
        void (StyleSettings::*pSetter)( const Color& ), const Color& (StyleSettings::*pGetter)( ) const )
    {
        sal_Int32 nColor = 0;
        if ( !( _rValue >>= nColor ) )
            nColor = (Application::GetSettings().GetStyleSettings().*pGetter)().GetColor();

        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();

        (aStyleSettings.*pSetter)( Color( nColor ) );

        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings, sal_True );
    }
}

// Terminated by BASEPROPERTY_NOTFOUND (or 0)
void VCLXWindow::PushPropertyIds( std::list< sal_uInt16 > &rIds,
                                  int nFirstId, ...)
{
    va_list pVarArgs;
    va_start( pVarArgs, nFirstId );

    for ( int nId = nFirstId; nId != BASEPROPERTY_NOTFOUND;
          nId = va_arg( pVarArgs, int ) )
        rIds.push_back( (sal_uInt16) nId );

    va_end( pVarArgs );
}

void VCLXWindow::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds, bool bWithDefaults )
{
    // These are common across ~all VCLXWindow derived classes
    if( bWithDefaults )
        PushPropertyIds( rIds,
                         BASEPROPERTY_ALIGN,
                         BASEPROPERTY_BACKGROUNDCOLOR,
                         BASEPROPERTY_BORDER,
                         BASEPROPERTY_BORDERCOLOR,
                         BASEPROPERTY_DEFAULTCONTROL,
                         BASEPROPERTY_ENABLED,
                         BASEPROPERTY_FONTDESCRIPTOR,
                         BASEPROPERTY_HELPTEXT,
                         BASEPROPERTY_HELPURL,
                         BASEPROPERTY_TEXT,
                         BASEPROPERTY_PRINTABLE,
                         BASEPROPERTY_ENABLEVISIBLE, // for visibility
                         BASEPROPERTY_TABSTOP,
                         0);

    // lovely hack from:
    // void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId )
    std::list< sal_uInt16 >::const_iterator iter;
    for( iter = rIds.begin(); iter != rIds.end(); ++iter) {
        if( *iter == BASEPROPERTY_FONTDESCRIPTOR )
        {
            // some properties are not included in the FontDescriptor, but everytime
            // when we have a FontDescriptor we want to have these properties too.
            // => Easier to register the here, istead everywhere where I register the FontDescriptor...

            rIds.push_back( BASEPROPERTY_TEXTCOLOR );
            rIds.push_back( BASEPROPERTY_TEXTLINECOLOR );
            rIds.push_back( BASEPROPERTY_FONTRELIEF );
            rIds.push_back( BASEPROPERTY_FONTEMPHASISMARK );
            break;
        }
    }
}

void VCLXWindow::GetPropertyIds( std::list< sal_uInt16 >& _out_rIds )
{
    return ImplGetPropertyIds( _out_rIds, mpImpl->mbWithDefaultProps );
}

::cppu::OInterfaceContainerHelper& VCLXWindow::GetContainerListeners()
{
    return mpImpl->getContainerListeners();
}

::cppu::OInterfaceContainerHelper& VCLXWindow::GetTopWindowListeners()
{
    return mpImpl->getTopWindowListeners();
}

namespace
{
    void    lcl_updateWritingMode( Window& _rWindow, const sal_Int16 _nWritingMode, const sal_Int16 _nContextWritingMode )
    {
        sal_Bool bEnableRTL = sal_False;
        switch ( _nWritingMode )
        {
        case WritingMode2::LR_TB:   bEnableRTL = sal_False; break;
        case WritingMode2::RL_TB:   bEnableRTL = sal_True; break;
        case WritingMode2::CONTEXT:
        {
            // consult our ContextWritingMode. If it has an explicit RTL/LTR value, then use
            // it. If it doesn't (but is CONTEXT itself), then just ask the parent window of our
            // own window for its RTL mode
            switch ( _nContextWritingMode )
            {
                case WritingMode2::LR_TB:   bEnableRTL = sal_False; break;
                case WritingMode2::RL_TB:   bEnableRTL = sal_True; break;
                case WritingMode2::CONTEXT:
                {
                    const Window* pParent = _rWindow.GetParent();
                    OSL_ENSURE( pParent, "lcl_updateWritingMode: cannot determine context's writing mode!" );
                    if ( pParent )
                        bEnableRTL = pParent->IsRTLEnabled();
                }
                break;
            }
        }
        break;
        default:
            OSL_FAIL( "lcl_updateWritingMode: unsupported WritingMode!" );
        }   // switch ( nWritingMode )

        _rWindow.EnableRTL( bEnableRTL );
    }
}

void VCLXWindow::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( !pWindow )
        return;

    sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

    WindowType eWinType = pWindow->GetType();
    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_REFERENCE_DEVICE:
        {
            Control* pControl = dynamic_cast< Control* >( pWindow );
            OSL_ENSURE( pControl, "VCLXWindow::setProperty( RefDevice ): need a Control for this!" );
            if ( !pControl )
                break;
            Reference< XDevice > xDevice( Value, UNO_QUERY );
            OutputDevice* pDevice = VCLUnoHelper::GetOutputDevice( xDevice );
            pControl->SetReferenceDevice( pDevice );
        }
        break;

        case BASEPROPERTY_CONTEXT_WRITING_MODE:
        {
            OSL_VERIFY( Value >>= mpImpl->mnContextWritingMode );
            if ( mpImpl->mnWritingMode == WritingMode2::CONTEXT )
                lcl_updateWritingMode( *pWindow, mpImpl->mnWritingMode, mpImpl->mnContextWritingMode );
        }
        break;

        case BASEPROPERTY_WRITING_MODE:
        {
            sal_Bool bProperType = ( Value >>= mpImpl->mnWritingMode );
            OSL_ENSURE( bProperType, "VCLXWindow::setProperty( 'WritingMode' ): illegal value type!" );
            if ( bProperType )
                lcl_updateWritingMode( *pWindow, mpImpl->mnWritingMode, mpImpl->mnContextWritingMode );
        }
        break;

        case BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR:
        {
            sal_uInt16 nWheelBehavior( MouseWheelBehavior::SCROLL_FOCUS_ONLY );
            OSL_VERIFY( Value >>= nWheelBehavior );

            AllSettings aSettings = pWindow->GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();

            sal_uInt16 nVclBehavior( MOUSE_WHEEL_FOCUS_ONLY );
            switch ( nWheelBehavior )
            {
            case MouseWheelBehavior::SCROLL_DISABLED:   nVclBehavior = MOUSE_WHEEL_DISABLE;     break;
            case MouseWheelBehavior::SCROLL_FOCUS_ONLY: nVclBehavior = MOUSE_WHEEL_FOCUS_ONLY;  break;
            case MouseWheelBehavior::SCROLL_ALWAYS:     nVclBehavior = MOUSE_WHEEL_ALWAYS;      break;
            default:
                OSL_FAIL( "VCLXWindow::setProperty( 'MouseWheelBehavior' ): illegal property value!" );
            }

            aMouseSettings.SetWheelBehavior( nVclBehavior );
            aSettings.SetMouseSettings( aMouseSettings );
            pWindow->SetSettings( aSettings, sal_True );
        }
        break;

        case BASEPROPERTY_NATIVE_WIDGET_LOOK:
        {
            sal_Bool bEnable( sal_True );
            OSL_VERIFY( Value >>= bEnable );
            pWindow->EnableNativeWidget( bEnable );
        }
        break;

        case BASEPROPERTY_PLUGINPARENT:
        {
            // set parent handle
            SetSystemParent_Impl( Value );
        }
        break;

        case BASEPROPERTY_ENABLED:
        {
            sal_Bool b = sal_Bool();
            if ( Value >>= b )
                setEnable( b );
        }
        break;
        case BASEPROPERTY_ENABLEVISIBLE:
        {
            sal_Bool b = sal_False;
            if ( Value >>= b )
            {
                if( b != mpImpl->isEnableVisible() )
                {
                    mpImpl->setEnableVisible( b );
                    pWindow->Show( b && mpImpl->isDirectVisible() );
                }
            }
        }
        break;
        case BASEPROPERTY_TEXT:
        case BASEPROPERTY_LABEL:
        case BASEPROPERTY_TITLE:
        {
            ::rtl::OUString aText;
            if ( Value >>= aText )
            {
                switch (eWinType)
                {
                    case WINDOW_OKBUTTON:
                    case WINDOW_CANCELBUTTON:
                    case WINDOW_HELPBUTTON:
                        // Standard Button: overwrite only if not empty.
                        if (!aText.isEmpty())
                            pWindow->SetText( aText );
                        break;

                    default:
                        pWindow->SetText( aText );
                        break;
                }
            }
        }
        break;
        case BASEPROPERTY_ACCESSIBLENAME:
        {
            ::rtl::OUString aText;
            if ( Value >>= aText )
                pWindow->SetAccessibleName( aText );
        }
        break;
        case BASEPROPERTY_HELPURL:
        {
            ::rtl::OUString aURL;
            if ( Value >>= aURL )
            {
                INetURLObject aHelpURL( aURL );
                if ( aHelpURL.GetProtocol() == INET_PROT_HID )
                    pWindow->SetHelpId( rtl::OUStringToOString( aHelpURL.GetURLPath(), RTL_TEXTENCODING_UTF8 ) );
                else
                    pWindow->SetHelpId( rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ) );
            }
        }
        break;
        case BASEPROPERTY_HELPTEXT:
        {
            ::rtl::OUString aHelpText;
            if ( Value >>= aHelpText )
            {
                pWindow->SetQuickHelpText( aHelpText );
            }
        }
        break;
        case BASEPROPERTY_FONTDESCRIPTOR:
        {
            if ( bVoid )
                pWindow->SetControlFont( Font() );
            else
            {
                ::com::sun::star::awt::FontDescriptor aFont;
                if ( Value >>= aFont )
                    pWindow->SetControlFont( VCLUnoHelper::CreateFont( aFont, pWindow->GetControlFont() ) );
            }
        }
        break;
        case BASEPROPERTY_FONTRELIEF:
        {
            sal_Int16 n = sal_Int16();
            if ( Value >>= n )
            {
                Font aFont = pWindow->GetControlFont();
                aFont.SetRelief( (FontRelief)n );
                pWindow->SetControlFont( aFont );
            }
        }
        break;
        case BASEPROPERTY_FONTEMPHASISMARK:
        {
            sal_Int16 n = sal_Int16();
            if ( Value >>= n )
            {
                Font aFont = pWindow->GetControlFont();
                aFont.SetEmphasisMark( n );
                pWindow->SetControlFont( aFont );
            }
        }
        break;
        case BASEPROPERTY_BACKGROUNDCOLOR:
            if ( bVoid )
            {
                switch ( eWinType )
                {
                    // set dialog color for default
                    case WINDOW_DIALOG:
                    case WINDOW_MESSBOX:
                    case WINDOW_INFOBOX:
                    case WINDOW_WARNINGBOX:
                    case WINDOW_ERRORBOX:
                    case WINDOW_QUERYBOX:
                    case WINDOW_TABPAGE:
                    {
                        Color aColor = pWindow->GetSettings().GetStyleSettings().GetDialogColor();
                        pWindow->SetBackground( aColor );
                        pWindow->SetControlBackground( aColor );
                        break;
                    }

                    case WINDOW_FIXEDTEXT:
                    case WINDOW_CHECKBOX:
                    case WINDOW_RADIOBUTTON:
                    case WINDOW_GROUPBOX:
                    case WINDOW_FIXEDLINE:
                    {
                        // support transparency only for special controls
                        pWindow->SetBackground();
                        pWindow->SetControlBackground();
                        pWindow->SetPaintTransparent( sal_True );
                        break;
                    }

                    default:
                    {
                        // default code which enables transparency for
                        // compound controls. It's not real transparency
                        // as most of these controls repaint their client
                        // area completely new.
                        if ( pWindow->IsCompoundControl() )
                            pWindow->SetBackground();
                        pWindow->SetControlBackground();
                        break;
                    }
                }
            }
            else
            {
                sal_Int32 nColor = 0;
                if ( Value >>= nColor )
                {
                    Color aColor( nColor );
                    pWindow->SetControlBackground( aColor );
                    pWindow->SetBackground( aColor );
                    switch ( eWinType )
                    {
                        // reset paint transparent mode
                        case WINDOW_FIXEDTEXT:
                        case WINDOW_CHECKBOX:
                        case WINDOW_RADIOBUTTON:
                        case WINDOW_GROUPBOX:
                        case WINDOW_FIXEDLINE:
                            pWindow->SetPaintTransparent( sal_False );
                        default: ;
                    }
                    pWindow->Invalidate();  // Invalidate if control does not respond to it
                }
            }
        break;
        case BASEPROPERTY_TEXTCOLOR:
            if ( bVoid )
            {
                pWindow->SetControlForeground();
            }
            else
            {
                sal_Int32 nColor = 0;
                if ( Value >>= nColor )
                {
                    Color aColor( nColor );
                    pWindow->SetTextColor( aColor );
                    pWindow->SetControlForeground( aColor );
                }
            }
        break;
        case BASEPROPERTY_TEXTLINECOLOR:
            if ( bVoid )
            {
                pWindow->SetTextLineColor();
            }
            else
            {
                sal_Int32 nColor = 0;
                if ( Value >>= nColor )
                {
                    Color aColor( nColor );
                    pWindow->SetTextLineColor( aColor );
                }
            }
        break;
        case BASEPROPERTY_FILLCOLOR:
            if ( bVoid )
                pWindow->SetFillColor();
            else
            {
                sal_Int32 nColor = 0;
                if ( Value >>= nColor )
                {
                    Color aColor( nColor );
                    pWindow->SetFillColor( aColor );
                }
            }
        break;
        case BASEPROPERTY_LINECOLOR:
            if ( bVoid )
                pWindow->SetLineColor();
            else
            {
                sal_Int32 nColor = 0;
                if ( Value >>= nColor )
                {
                    Color aColor( nColor );
                    pWindow->SetLineColor( aColor );
                }
            }
        break;
        case BASEPROPERTY_BORDER:
        {
            WinBits nStyle = pWindow->GetStyle();
            sal_uInt16 nBorder = 0;
            Value >>= nBorder;
            if ( !nBorder )
            {
                pWindow->SetStyle( nStyle & ~WB_BORDER );
            }
            else
            {
                pWindow->SetStyle( nStyle | WB_BORDER );
                pWindow->SetBorderStyle( nBorder );
            }
        }
        break;
        case BASEPROPERTY_TABSTOP:
        {
            WinBits nStyle = pWindow->GetStyle() & ~WB_TABSTOP;
            if ( !bVoid )
            {
                sal_Bool bTab = false;
                Value >>= bTab;
                if ( bTab )
                    nStyle |= WB_TABSTOP;
                else
                    nStyle |= WB_NOTABSTOP;
            }
            pWindow->SetStyle( nStyle );
        }
        break;
        case BASEPROPERTY_VERTICALALIGN:
        {
            VerticalAlignment eAlign = VerticalAlignment_MAKE_FIXED_SIZE;
            WinBits nStyle = pWindow->GetStyle();
            nStyle &= ~(WB_TOP|WB_VCENTER|WB_BOTTOM);
            if ( !bVoid )
                Value >>= eAlign;
            switch ( eAlign )
            {
            case VerticalAlignment_TOP:
                nStyle |= WB_TOP;
                break;
            case VerticalAlignment_MIDDLE:
                nStyle |= WB_VCENTER;
                break;
            case VerticalAlignment_BOTTOM:
                nStyle |= WB_BOTTOM;
                break;
            default: ; // for warning free code, MAKE_FIXED_SIZE
            }
            pWindow->SetStyle( nStyle );
        }
        break;
        case BASEPROPERTY_ALIGN:
        {
            sal_Int16 nAlign = PROPERTY_ALIGN_LEFT;
            switch ( eWinType )
            {
                case WINDOW_COMBOBOX:
                case WINDOW_BUTTON:
                case WINDOW_PUSHBUTTON:
                case WINDOW_OKBUTTON:
                case WINDOW_CANCELBUTTON:
                case WINDOW_HELPBUTTON:
                    nAlign = PROPERTY_ALIGN_CENTER;
                    // no break here!
                case WINDOW_FIXEDTEXT:
                case WINDOW_EDIT:
                case WINDOW_MULTILINEEDIT:
                case WINDOW_CHECKBOX:
                case WINDOW_RADIOBUTTON:
                case WINDOW_LISTBOX:
                {
                    WinBits nStyle = pWindow->GetStyle();
                    nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
                    if ( !bVoid )
                        Value >>= nAlign;
                    if ( nAlign == PROPERTY_ALIGN_LEFT )
                        nStyle |= WB_LEFT;
                    else if ( nAlign == PROPERTY_ALIGN_CENTER )
                        nStyle |= WB_CENTER;
                    else
                        nStyle |= WB_RIGHT;
                    pWindow->SetStyle( nStyle );
                }
                break;
            }
        }
        break;
        case BASEPROPERTY_MULTILINE:
        {
            if  (  ( eWinType == WINDOW_FIXEDTEXT )
                || ( eWinType == WINDOW_CHECKBOX )
                || ( eWinType == WINDOW_RADIOBUTTON )
                || ( eWinType == WINDOW_BUTTON )
                || ( eWinType == WINDOW_PUSHBUTTON )
                || ( eWinType == WINDOW_OKBUTTON )
                || ( eWinType == WINDOW_CANCELBUTTON )
                || ( eWinType == WINDOW_HELPBUTTON )
                )
            {
                WinBits nStyle = pWindow->GetStyle();
                sal_Bool bMulti = false;
                Value >>= bMulti;
                if ( bMulti )
                    nStyle |= WB_WORDBREAK;
                else
                    nStyle &= ~WB_WORDBREAK;
                pWindow->SetStyle( nStyle );
            }
        }
        break;
        case BASEPROPERTY_ORIENTATION:
        {
            switch ( eWinType )
            {
                case WINDOW_FIXEDLINE:
                {
                    sal_Int32 nOrientation = 0;
                    if ( Value >>= nOrientation )
                    {
                        WinBits nStyle = pWindow->GetStyle();
                        nStyle &= ~(WB_HORZ|WB_VERT);
                        if ( nOrientation == 0 )
                            nStyle |= WB_HORZ;
                        else
                            nStyle |= WB_VERT;

                        pWindow->SetStyle( nStyle );
                    }
                }
                break;
            }
        }
        break;
        case BASEPROPERTY_AUTOMNEMONICS:
        {
            sal_Bool bAutoMnemonics = false;
            Value >>= bAutoMnemonics;
            AllSettings aSettings = pWindow->GetSettings();
            StyleSettings aStyleSettings = aSettings.GetStyleSettings();
            if ( aStyleSettings.GetAutoMnemonic() != bAutoMnemonics )
            {
                aStyleSettings.SetAutoMnemonic( bAutoMnemonics );
                aSettings.SetStyleSettings( aStyleSettings );
                pWindow->SetSettings( aSettings );
            }
        }
        break;
        case BASEPROPERTY_MOUSETRANSPARENT:
        {
            sal_Bool bMouseTransparent = false;
            Value >>= bMouseTransparent;
            pWindow->SetMouseTransparent( bMouseTransparent );
        }
        break;
        case BASEPROPERTY_PAINTTRANSPARENT:
        {
            sal_Bool bPaintTransparent = false;
            Value >>= bPaintTransparent;
            pWindow->SetPaintTransparent( bPaintTransparent );
//                pWindow->SetBackground();
        }
        break;

        case BASEPROPERTY_REPEAT:
        {
            sal_Bool bRepeat( sal_False );
            Value >>= bRepeat;

            WinBits nStyle = pWindow->GetStyle();
            if ( bRepeat )
                nStyle |= WB_REPEAT;
            else
                nStyle &= ~WB_REPEAT;
            pWindow->SetStyle( nStyle );
        }
        break;

        case BASEPROPERTY_REPEAT_DELAY:
        {
            sal_Int32 nRepeatDelay = 0;
            if ( Value >>= nRepeatDelay )
            {
                AllSettings aSettings = pWindow->GetSettings();
                MouseSettings aMouseSettings = aSettings.GetMouseSettings();

                aMouseSettings.SetButtonRepeat( nRepeatDelay );
                aSettings.SetMouseSettings( aMouseSettings );

                pWindow->SetSettings( aSettings, sal_True );
            }
        }
        break;

        case BASEPROPERTY_SYMBOL_COLOR:
            ::toolkit::setColorSettings( pWindow, Value, &StyleSettings::SetButtonTextColor, &StyleSettings::GetButtonTextColor );
            break;

        case BASEPROPERTY_BORDERCOLOR:
            ::toolkit::setColorSettings( pWindow, Value, &StyleSettings::SetMonoColor, &StyleSettings::GetMonoColor);
            break;
        case BASEPROPERTY_DEFAULTCONTROL:
        {
            rtl::OUString aName;
            Value >>= aName;
            break;
        }
    }
}

::com::sun::star::uno::Any VCLXWindow::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        WindowType eWinType = GetWindow()->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_REFERENCE_DEVICE:
            {
                Control* pControl = dynamic_cast< Control* >( GetWindow() );
                OSL_ENSURE( pControl, "VCLXWindow::setProperty( RefDevice ): need a Control for this!" );
                if ( !pControl )
                    break;

                VCLXDevice* pDevice = new VCLXDevice;
                pDevice->SetOutputDevice( pControl->GetReferenceDevice() );
                aProp <<= Reference< XDevice >( pDevice );
            }
            break;

            case BASEPROPERTY_CONTEXT_WRITING_MODE:
                aProp <<= mpImpl->mnContextWritingMode;
                break;

            case BASEPROPERTY_WRITING_MODE:
                aProp <<= mpImpl->mnWritingMode;
                break;

            case BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR:
            {
                sal_uInt16 nVclBehavior = GetWindow()->GetSettings().GetMouseSettings().GetWheelBehavior();
                sal_Int16 nBehavior = MouseWheelBehavior::SCROLL_FOCUS_ONLY;
                switch ( nVclBehavior )
                {
                case MOUSE_WHEEL_DISABLE:       nBehavior = MouseWheelBehavior::SCROLL_DISABLED;    break;
                case MOUSE_WHEEL_FOCUS_ONLY:    nBehavior = MouseWheelBehavior::SCROLL_FOCUS_ONLY;  break;
                case MOUSE_WHEEL_ALWAYS:        nBehavior = MouseWheelBehavior::SCROLL_ALWAYS;      break;
                default:
                    OSL_FAIL( "VCLXWindow::getProperty( 'MouseWheelBehavior' ): illegal VCL value!" );
                }
                aProp <<= nBehavior;
            }
            break;

            case BASEPROPERTY_NATIVE_WIDGET_LOOK:
                aProp <<= (sal_Bool) GetWindow()->IsNativeWidgetEnabled();
                break;

            case BASEPROPERTY_ENABLED:
                aProp <<= (sal_Bool) GetWindow()->IsEnabled();
                break;

            case BASEPROPERTY_ENABLEVISIBLE:
                aProp <<= (sal_Bool) mpImpl->isEnableVisible();
                break;

            case BASEPROPERTY_HIGHCONTRASTMODE:
                aProp <<= (sal_Bool) GetWindow()->GetSettings().GetStyleSettings().GetHighContrastMode();
                break;

            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                ::rtl::OUString aText = GetWindow()->GetText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_ACCESSIBLENAME:
            {
                ::rtl::OUString aText = GetWindow()->GetAccessibleName();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_HELPTEXT:
            {
                ::rtl::OUString aText = GetWindow()->GetQuickHelpText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_HELPURL:
            {
                rtl::OUString aHelpId( rtl::OStringToOUString( GetWindow()->GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
                aProp <<= ::rtl::OUString( aHelpId );
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                Font aFont = GetWindow()->GetControlFont();
                ::com::sun::star::awt::FontDescriptor aFD = VCLUnoHelper::CreateFontDescriptor( aFont );
                aProp <<= aFD;
            }
            break;
            case BASEPROPERTY_BACKGROUNDCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlBackground().GetColor();
            break;
            case BASEPROPERTY_DISPLAYBACKGROUNDCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetDisplayBackground().GetColor().GetColor();
            break;
            case BASEPROPERTY_FONTRELIEF:
                aProp <<= (sal_Int16) GetWindow()->GetControlFont().GetRelief();
            break;
            case BASEPROPERTY_FONTEMPHASISMARK:
                aProp <<= (sal_Int16) GetWindow()->GetControlFont().GetEmphasisMark();
            break;
            case BASEPROPERTY_TEXTCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetControlForeground().GetColor();
            break;
            case BASEPROPERTY_TEXTLINECOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetTextLineColor().GetColor();
            break;
            case BASEPROPERTY_FILLCOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetFillColor().GetColor();
            break;
            case BASEPROPERTY_LINECOLOR:
                aProp <<= (sal_Int32) GetWindow()->GetLineColor().GetColor();
            break;
            case BASEPROPERTY_BORDER:
            {
                sal_Int16 nBorder = 0;
                if ( GetWindow()->GetStyle() & WB_BORDER )
                    nBorder = GetWindow()->GetBorderStyle();
                aProp <<= nBorder;
            }
            break;
            case BASEPROPERTY_TABSTOP:
                aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_TABSTOP ) ? sal_True : sal_False;
            break;
            case BASEPROPERTY_VERTICALALIGN:
            {
                WinBits nStyle = GetWindow()->GetStyle();
                if ( nStyle & WB_TOP )
                    aProp <<= VerticalAlignment_TOP;
                else if ( nStyle & WB_VCENTER )
                    aProp <<= VerticalAlignment_MIDDLE;
                else if ( nStyle & WB_BOTTOM )
                    aProp <<= VerticalAlignment_BOTTOM;
            }
            break;
            case BASEPROPERTY_ALIGN:
            {
                switch ( eWinType )
                {
                    case WINDOW_FIXEDTEXT:
                    case WINDOW_EDIT:
                    case WINDOW_MULTILINEEDIT:
                    case WINDOW_CHECKBOX:
                    case WINDOW_RADIOBUTTON:
                    case WINDOW_LISTBOX:
                    case WINDOW_COMBOBOX:
                    case WINDOW_BUTTON:
                    case WINDOW_PUSHBUTTON:
                    case WINDOW_OKBUTTON:
                    case WINDOW_CANCELBUTTON:
                    case WINDOW_HELPBUTTON:
                    {
                        WinBits nStyle = GetWindow()->GetStyle();
                        if ( nStyle & WB_LEFT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_LEFT;
                        else if ( nStyle & WB_CENTER )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_CENTER;
                        else if ( nStyle & WB_RIGHT )
                            aProp <<= (sal_Int16) PROPERTY_ALIGN_RIGHT;
                    }
                    break;
                }
            }
            case BASEPROPERTY_MULTILINE:
            {
                if  (  ( eWinType == WINDOW_FIXEDTEXT )
                    || ( eWinType == WINDOW_CHECKBOX )
                    || ( eWinType == WINDOW_RADIOBUTTON )
                    || ( eWinType == WINDOW_BUTTON )
                    || ( eWinType == WINDOW_PUSHBUTTON )
                    || ( eWinType == WINDOW_OKBUTTON )
                    || ( eWinType == WINDOW_CANCELBUTTON )
                    || ( eWinType == WINDOW_HELPBUTTON )
                    )
                    aProp <<= (sal_Bool) ( GetWindow()->GetStyle() & WB_WORDBREAK ) ? sal_True : sal_False;
            }
            break;
            case BASEPROPERTY_AUTOMNEMONICS:
            {
                sal_Bool bAutoMnemonics = GetWindow()->GetSettings().GetStyleSettings().GetAutoMnemonic();
                aProp <<= bAutoMnemonics;
            }
            break;
            case BASEPROPERTY_MOUSETRANSPARENT:
            {
                sal_Bool bMouseTransparent = GetWindow()->IsMouseTransparent();
                aProp <<= bMouseTransparent;
            }
            break;
            case BASEPROPERTY_PAINTTRANSPARENT:
            {
                sal_Bool bPaintTransparent = GetWindow()->IsPaintTransparent();
                aProp <<= bPaintTransparent;
            }
            break;

            case BASEPROPERTY_REPEAT:
                aProp <<= (sal_Bool)( 0 != ( GetWindow()->GetStyle() & WB_REPEAT ) );
                break;

            case BASEPROPERTY_REPEAT_DELAY:
            {
                sal_Int32 nButtonRepeat = GetWindow()->GetSettings().GetMouseSettings().GetButtonRepeat();
                aProp <<= (sal_Int32)nButtonRepeat;
            }
            break;

            case BASEPROPERTY_SYMBOL_COLOR:
                aProp <<= (sal_Int32)GetWindow()->GetSettings().GetStyleSettings().GetButtonTextColor().GetColor();
                break;

            case BASEPROPERTY_BORDERCOLOR:
                aProp <<= (sal_Int32)GetWindow()->GetSettings().GetStyleSettings().GetMonoColor().GetColor();
                break;
        }
    }
    return aProp;
}


// ::com::sun::star::awt::XLayoutConstrains
::com::sun::star::awt::Size VCLXWindow::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // Use this method only for those components which can be created through
    // ::com::sun::star::awt::Toolkit , but do not have an interface

    Size aSz;
    if ( GetWindow() )
    {
        WindowType nWinType = GetWindow()->GetType();
        switch ( nWinType )
        {
            case WINDOW_CONTROL:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*12;
                aSz.Height() = GetWindow()->GetTextHeight()+2*6;
            break;

            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_LONGCURRENCYBOX:
                aSz.Width() = GetWindow()->GetTextWidth( GetWindow()->GetText() )+2*2;
                aSz.Height() = GetWindow()->GetTextHeight()+2*2;
            break;
            case WINDOW_SCROLLBARBOX:
                return VCLXScrollBar::implGetMinimumSize( GetWindow() );
            default:
                aSz = GetWindow()->GetOptimalSize( WINDOWSIZE_MINIMUM );
        }
    }

    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

::com::sun::star::awt::Size VCLXWindow::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXWindow::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aNewSize( rNewSize );
    ::com::sun::star::awt::Size aMinSize = getMinimumSize();

    if ( aNewSize.Width < aMinSize.Width )
        aNewSize.Width = aMinSize.Width;
    if ( aNewSize.Height < aMinSize.Height )
        aNewSize.Height = aMinSize.Height;

    return aNewSize;
}


// ::com::sun::star::awt::XView
sal_Bool VCLXWindow::setGraphics( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& rxDevice ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( VCLUnoHelper::GetOutputDevice( rxDevice ) )
        mpImpl->mxViewGraphics = rxDevice;
    else
        mpImpl->mxViewGraphics = NULL;

    return mpImpl->mxViewGraphics.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXWindow::getGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return mpImpl->mxViewGraphics;
}

::com::sun::star::awt::Size VCLXWindow::getSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Size aSz;
    if ( GetWindow() )
        aSz = GetWindow()->GetSizePixel();
    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

void VCLXWindow::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( !pWindow )
        return;

    if ( isDesignMode() || mpImpl->isEnableVisible() )
    {
        TabPage* pTabPage = dynamic_cast< TabPage* >( pWindow );
        if ( pTabPage )
        {
            Point aPos( nX, nY );
            Size  aSize = pWindow->GetSizePixel();

            OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mpImpl->mxViewGraphics );
            aPos  = pDev->PixelToLogic( aPos );
            aSize = pDev->PixelToLogic( aSize );

            pTabPage->Draw( pDev, aPos, aSize, 0 );
            return;
        }

        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mpImpl->mxViewGraphics );
        Point aPos( nX, nY );

        if ( !pDev )
            pDev = pWindow->GetParent();

        if ( pWindow->GetParent() && !pWindow->IsSystemWindow() && ( pWindow->GetParent() == pDev ) )
        {
            // #i40647# don't draw here if this is a recursive call
            // sometimes this is called recursively, because the Update call on the parent
            // (strangely) triggers another paint. Prevent a stack overflow here
            // Yes, this is only fixing symptoms for the moment ....
            // #i40647# / 2005-01-18 / frank.schoenheit@sun.com
            if ( !mpImpl->getDrawingOntoParent_ref() )
            {
                ::comphelper::FlagGuard aDrawingflagGuard( mpImpl->getDrawingOntoParent_ref() );

                sal_Bool bWasVisible = pWindow->IsVisible();
                Point aOldPos( pWindow->GetPosPixel() );

                if ( bWasVisible && aOldPos == aPos )
                {
                    pWindow->Update();
                    return;
                }

                pWindow->SetPosPixel( aPos );

                // Update parent first to avoid painting the parent upon the update
                // of this window, as it may otherwise cause the parent
                // to hide this window again
                if( pWindow->GetParent() )
                    pWindow->GetParent()->Update();

                pWindow->Show();
                pWindow->Update();
                pWindow->SetParentUpdateMode( sal_False );
                pWindow->Hide();
                pWindow->SetParentUpdateMode( sal_True );

                pWindow->SetPosPixel( aOldPos );
                if ( bWasVisible )
                    pWindow->Show( sal_True );
            }
        }
        else if ( pDev )
        {
            Size aSz = pWindow->GetSizePixel();
            aSz = pDev->PixelToLogic( aSz );
            Point aP = pDev->PixelToLogic( aPos );

            vcl::PDFExtOutDevData* pPDFExport   = dynamic_cast<vcl::PDFExtOutDevData*>(pDev->GetExtOutDevData());
            bool bDrawSimple =    ( pDev->GetOutDevType() == OUTDEV_PRINTER )
                               || ( pDev->GetOutDevViewType() == OUTDEV_VIEWTYPE_PRINTPREVIEW )
                               || ( pPDFExport != NULL );
            if ( bDrawSimple )
            {
                pWindow->Draw( pDev, aP, aSz, WINDOW_DRAW_NOCONTROLS );
            }
            else
            {
                sal_Bool bOldNW =pWindow->IsNativeWidgetEnabled();
                if( bOldNW )
                    pWindow->EnableNativeWidget(sal_False);
                pWindow->PaintToDevice( pDev, aP, aSz );
                if( bOldNW )
                    pWindow->EnableNativeWidget(sal_True);
            }
        }
    }
}

void VCLXWindow::setZoom( float fZoomX, float /*fZoomY*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        // Fraction::Fraction takes a double, but we have a float only.
        // The implicit conversion from float to double can result in a precision loss, i.e. 1.2 is converted to
        // 1.200000000047something. To prevent this, we convert explicitly to double, and round it.
        double nZoom( fZoomX );
        nZoom = ::rtl::math::round( nZoom, 4 );
        GetWindow()->SetZoom( Fraction( nZoom ) );
    }
}

// ::com::sun::star::lang::XEventListener
void SAL_CALL VCLXWindow::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // check if it comes from our AccessibleContext
    uno::Reference< uno::XInterface > aAC( mpImpl->mxAccessibleContext, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xSource( _rSource.Source, uno::UNO_QUERY );

    if ( aAC.get() == xSource.get() )
    {   // yep, it does
        mpImpl->mxAccessibleContext = uno::Reference< accessibility::XAccessibleContext >();
    }
}

// ::com::sun::star::accessibility::XAccessible
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXWindow::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
{
    using namespace ::com::sun::star;

    SolarMutexGuard aGuard;

    // already disposed
    if( ! mpImpl )
        return uno::Reference< accessibility::XAccessibleContext >();

    if ( !mpImpl->mxAccessibleContext.is() && GetWindow() )
    {
        mpImpl->mxAccessibleContext = CreateAccessibleContext();

        // add as event listener to this component
        // in case somebody disposes it, we do not want to have a (though weak) reference to a dead
        // object
        uno::Reference< lang::XComponent > xComp( mpImpl->mxAccessibleContext, uno::UNO_QUERY );
        if ( xComp.is() )
            xComp->addEventListener( this );
    }

    return mpImpl->mxAccessibleContext;
}

// ::com::sun::star::awt::XDockable
void SAL_CALL VCLXWindow::addDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( xListener.is() )
        mpImpl->getDockableWindowListeners().addInterface( xListener );

}

void SAL_CALL VCLXWindow::removeDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    mpImpl->getDockableWindowListeners().removeInterface( xListener );
}

void SAL_CALL VCLXWindow::enableDocking( sal_Bool bEnable ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->EnableDocking( bEnable );
}

sal_Bool SAL_CALL VCLXWindow::isFloating(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if( pWindow )
        return Window::GetDockingManager()->IsFloating( pWindow );
    else
        return sal_False;
}

void SAL_CALL VCLXWindow::setFloatingMode( sal_Bool bFloating ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if( pWindow )
        Window::GetDockingManager()->SetFloatingMode( pWindow, bFloating );
}

sal_Bool SAL_CALL VCLXWindow::isLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if( pWindow )
        return Window::GetDockingManager()->IsLocked( pWindow );
    else
        return sal_False;
}

void SAL_CALL VCLXWindow::lock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if( pWindow && !Window::GetDockingManager()->IsFloating( pWindow ) )
        Window::GetDockingManager()->Lock( pWindow );
}

void SAL_CALL VCLXWindow::unlock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if( pWindow && !Window::GetDockingManager()->IsFloating( pWindow ) )
        Window::GetDockingManager()->Unlock( pWindow );
}
void SAL_CALL VCLXWindow::startPopupMode( const ::com::sun::star::awt::Rectangle& ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: remove interface in the next incompatible build
    SolarMutexGuard aGuard;

}

sal_Bool SAL_CALL VCLXWindow::isInPopupMode(  ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: remove interface in the next incompatible build
    SolarMutexGuard aGuard;
    return sal_False;
}


// ::com::sun::star::awt::XWindow2

void SAL_CALL VCLXWindow::setOutputSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window *pWindow;
    if( (pWindow = GetWindow()) != NULL )
    {
        DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >(pWindow);
        if( pDockingWindow )
            pDockingWindow->SetOutputSizePixel( VCLSize( aSize ) );
        else
            pWindow->SetOutputSizePixel( VCLSize( aSize ) );
    }
}

::com::sun::star::awt::Size SAL_CALL VCLXWindow::getOutputSize(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Window *pWindow;
    if( (pWindow = GetWindow()) != NULL )
    {
        DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >(pWindow);
        if( pDockingWindow )
            return AWTSize( pDockingWindow->GetOutputSizePixel() );
        else
            return AWTSize( pWindow->GetOutputSizePixel() );
    }
    else
        return ::com::sun::star::awt::Size();
}

sal_Bool SAL_CALL VCLXWindow::isVisible(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsVisible();
    else
        return sal_False;
}

sal_Bool SAL_CALL VCLXWindow::isActive(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsActive();
    else
        return sal_False;

}

sal_Bool SAL_CALL VCLXWindow::isEnabled(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsEnabled();
    else
        return sal_False;
}

sal_Bool SAL_CALL VCLXWindow::hasFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->HasFocus();
    else
        return sal_False;
}

// ::com::sun::star::beans::XPropertySetInfo

UnoPropertyArrayHelper *
VCLXWindow::GetPropHelper()
{
    SolarMutexGuard aGuard;
    if ( mpImpl->mpPropHelper == NULL )
    {
        std::list< sal_uInt16 > aIDs;
        GetPropertyIds( aIDs );
        mpImpl->mpPropHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return mpImpl->mpPropHelper;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL
VCLXWindow::getProperties() throw (::com::sun::star::uno::RuntimeException)
{
    return GetPropHelper()->getProperties();
}
::com::sun::star::beans::Property SAL_CALL
VCLXWindow::getPropertyByName( const ::rtl::OUString& rName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)
{
    return GetPropHelper()->getPropertyByName( rName );
}

::sal_Bool SAL_CALL
VCLXWindow::hasPropertyByName( const ::rtl::OUString& rName ) throw (::com::sun::star::uno::RuntimeException)
{
    return GetPropHelper()->hasPropertyByName( rName );
}

Reference< XStyleSettings > SAL_CALL VCLXWindow::getStyleSettings() throw (RuntimeException)
{
    return mpImpl->getStyleSettings();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
