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
#include <toolkit/helper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uuid.h>
#include <rtl/ustrbuf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>
#include <comphelper/asyncnotification.hxx>
#include <comphelper/flagguard.hxx>
#include "stylesettings.hxx"
#include <tools/urlobj.hxx>

#include "helper/accessibilityclient.hxx"
#include "helper/unopropertyarrayhelper.hxx"

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


//= VCLXWindowImpl

class VCLXWindowImpl
{
private:
    typedef ::std::vector< VCLXWindow::Callback >                       CallbackArray;

private:
    VCLXWindow&                         mrAntiImpl;
    ::toolkit::AccessibilityClient      maAccFactory;
    bool                                mbDisposed;
    bool                                mbDrawingOntoParent;    // no bit mask, is passed around  by reference
    bool                            mbEnableVisible;
    bool                            mbDirectVisible;

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
    ImplSVEvent *                       mnCallbackEventId;

public:
    bool                                mbDisposing             : 1;
    bool                                mbDesignMode            : 1;
    bool                                mbSynthesizingVCLEvent  : 1;
    bool                                mbWithDefaultProps      : 1;

    sal_uLong                               mnListenerLockLevel;
    sal_Int16                           mnWritingMode;
    sal_Int16                           mnContextWritingMode;

    UnoPropertyArrayHelper*             mpPropHelper;

    css::uno::Reference< css::awt::XPointer >
                                        mxPointer;
    css::uno::Reference< css::accessibility::XAccessibleContext >
                                        mxAccessibleContext;
    css::uno::Reference< css::awt::XGraphics >
                                        mxViewGraphics;
    css::uno::Reference< css::awt::XStyleSettings >
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

    VCLXWindowImpl( const VCLXWindowImpl& ) = delete;
    const VCLXWindowImpl& operator=(const VCLXWindowImpl&) = delete;

    /** synchronously mbEnableVisible
    */
    void    setEnableVisible( bool bEnableVisible ) { mbEnableVisible = bEnableVisible; }
    bool    isEnableVisible() { return mbEnableVisible; }
    /** synchronously mbDirectVisible;
    */
    void    setDirectVisible( bool bDirectVisible ) { mbDirectVisible = bDirectVisible; }
    bool    isDirectVisible() { return mbDirectVisible; }

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

    ~VCLXWindowImpl();

private:
    DECL_LINK_TYPED( OnProcessCallbacks, void*, void );
};


VCLXWindowImpl::VCLXWindowImpl( VCLXWindow& _rAntiImpl, bool _bWithDefaultProps )
    :mrAntiImpl( _rAntiImpl )
    ,mbDisposed( false )
    ,mbDrawingOntoParent( false )
    ,mbEnableVisible(true)
    ,mbDirectVisible(true)
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
    ,mnCallbackEventId( nullptr )
    ,mbDisposing( false )
    ,mbDesignMode( false )
    ,mbSynthesizingVCLEvent( false )
    ,mbWithDefaultProps( _bWithDefaultProps )
    ,mnListenerLockLevel( 0 )
    ,mnWritingMode( WritingMode2::CONTEXT )
    ,mnContextWritingMode( WritingMode2::CONTEXT )
    ,mpPropHelper( nullptr )
{
}

VCLXWindowImpl::~VCLXWindowImpl()
{
    delete mpPropHelper;
}


void VCLXWindowImpl::disposing()
{
    SolarMutexGuard aGuard;
    if ( mnCallbackEventId )
        Application::RemoveUserEvent( mnCallbackEventId );
    mnCallbackEventId = nullptr;

    mbDisposed= true;

    css::lang::EventObject aEvent;
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
    if ( pStyleSettings != nullptr )
        pStyleSettings->dispose();
    mxWindowStyleSettings.clear();
}


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


IMPL_LINK_NOARG_TYPED(VCLXWindowImpl, OnProcessCallbacks, void*, void)
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
            return;

        mnCallbackEventId = nullptr;
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
}

Reference< XStyleSettings > VCLXWindowImpl::getStyleSettings()
{
    SolarMutexGuard aGuard;
    if ( mbDisposed )
        throw DisposedException( OUString(), mrAntiImpl );
    if ( !mxWindowStyleSettings.is() )
        mxWindowStyleSettings = new ::toolkit::WindowStyleSettings( maListenerContainerMutex, mrAntiImpl );
    return mxWindowStyleSettings;
}


// Uses an out-parameter instead of return value, due to the object reference

void ImplInitWindowEvent( css::awt::WindowEvent& rEvent, vcl::Window* pWindow )
{
    Point aPos = pWindow->GetPosPixel();
    Size aSz = pWindow->GetSizePixel();

    rEvent.X = aPos.X();
    rEvent.Y = aPos.Y();

    rEvent.Width = aSz.Width();
    rEvent.Height = aSz.Height();

    pWindow->GetBorder( rEvent.LeftInset, rEvent.TopInset, rEvent.RightInset, rEvent.BottomInset );
}

VCLXWindow::VCLXWindow( bool _bWithDefaultProps )
    :mpImpl( nullptr )
{
    mpImpl = new VCLXWindowImpl( *this, _bWithDefaultProps );
}

VCLXWindow::~VCLXWindow()
{
    delete mpImpl;

    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
        GetWindow()->SetWindowPeer( nullptr, nullptr );
        GetWindow()->SetAccessible( nullptr );
    }
}


void VCLXWindow::ImplExecuteAsyncWithoutSolarLock( const Callback& i_callback )
{
    mpImpl->callBackAsync( i_callback );
}


::toolkit::IAccessibleFactory& VCLXWindow::getAccessibleFactory()
{
    return mpImpl->getAccessibleFactory().getFactory();
}

void VCLXWindow::SetWindow( const VclPtr<vcl::Window> &pWindow )
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
        bool bDirectVisible = pWindow && pWindow->IsVisible();
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

void VCLXWindow::notifyWindowRemoved( vcl::Window& _rWindow )
{
    if ( mpImpl->getContainerListeners().getLength() )
    {
        awt::VclContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Child = static_cast< XWindow* >( _rWindow.GetWindowPeer() );
        mpImpl->getContainerListeners().windowRemoved( aEvent );
    }
}

IMPL_LINK_TYPED( VCLXWindow, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if ( mpImpl->mnListenerLockLevel )
        return;

    DBG_ASSERT( rEvent.GetWindow() && GetWindow(), "Window???" );
    ProcessWindowEvent( rEvent );
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
    css::uno::Reference< css::uno::XInterface > xThis( static_cast<cppu::OWeakObject*>(this) );

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
                css::awt::PaintEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                aEvent.UpdateRect = AWTRectangle( *static_cast<Rectangle*>(rVclWindowEvent.GetData()) );
                aEvent.Count = 0;
                mpImpl->getPaintListeners().windowPaint( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                css::awt::WindowEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowMoved( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_RESIZE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                css::awt::WindowEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowResized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_SHOW:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                css::awt::WindowEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowShown( aEvent );
            }

            // For TopWindows this means opened...
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowOpened( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            if ( mpImpl->getWindowListeners().getLength() )
            {
                css::awt::WindowEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                mpImpl->getWindowListeners().windowHidden( aEvent );
            }

            // For TopWindows this means closed...
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowClosed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowActivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowDeactivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_CLOSE:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::closed, aEvent );
            }
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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
                    css::awt::FocusEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.FocusFlags = static_cast<sal_Int16>(rVclWindowEvent.GetWindow()->GetGetFocusFlags());
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
                    css::awt::FocusEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                    aEvent.FocusFlags = static_cast<sal_Int16>(rVclWindowEvent.GetWindow()->GetGetFocusFlags());
                    aEvent.Temporary = sal_False;

                    vcl::Window* pNext = Application::GetFocusWindow();
                    if ( pNext )
                    {
                        // Don't care about internals if this control is compound
                        vcl::Window* pNextC = pNext;
                        while ( pNextC && !pNextC->IsCompoundControl() )
                            pNextC = pNextC->GetParent();
                        if ( pNextC )
                            pNext = pNextC;

                        pNext->GetComponentInterface();
                        aEvent.NextFocus = static_cast<cppu::OWeakObject*>(pNext->GetWindowPeer());
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
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowMinimized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_NORMALIZE:
        {
            if ( mpImpl->getTopWindowListeners().getLength() )
            {
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getTopWindowListeners().windowNormalized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            if ( mpImpl->getKeyListeners().getLength() )
            {
                css::awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent(
                    *static_cast<KeyEvent*>(rVclWindowEvent.GetData()), *this
                ) );
                mpImpl->getKeyListeners().keyPressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYUP:
        {
            if ( mpImpl->getKeyListeners().getLength() )
            {
                css::awt::KeyEvent aEvent( VCLUnoHelper::createKeyEvent(
                    *static_cast<KeyEvent*>(rVclWindowEvent.GetData()), *this
                ) );
                mpImpl->getKeyListeners().keyReleased( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_COMMAND:
        {
            CommandEvent* pCmdEvt = static_cast<CommandEvent*>(rVclWindowEvent.GetData());
            if ( mpImpl->getMouseListeners().getLength() && ( pCmdEvt->GetCommand() == CommandEventId::ContextMenu ) )
            {
                // CommandEventId::ContextMenu als mousePressed mit PopupTrigger = sal_True versenden...
                Point aWhere = static_cast< CommandEvent* >( rVclWindowEvent.GetData() )->GetMousePosPixel();
                if ( !pCmdEvt->IsMouseEvent() )
                {   // for keyboard events, we set the coordinates to -1,-1. This is a slight HACK, but the current API
                    // handles a context menu command as special case of a mouse event, which is simply wrong.
                    // Without extending the API, we would not have another chance to notify listeners of a
                    // keyboard-triggered context menu request
                    aWhere = Point( -1, -1 );
                }

                MouseEvent aMEvt( aWhere, 1, MouseEventModifiers::SIMPLECLICK, MOUSE_LEFT, 0 );
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( aMEvt, *this ) );
                aEvent.PopupTrigger = sal_True;

                Callback aCallback = [ this, aEvent ]()
                                     { this->mpImpl->getMouseListeners().mousePressed( aEvent ); };

                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEMOVE:
        {
            MouseEvent* pMouseEvt = static_cast<MouseEvent*>(rVclWindowEvent.GetData());
            if ( mpImpl->getMouseListeners().getLength() && ( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() ) )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *pMouseEvt, *this ) );
                bool const isEnter(pMouseEvt->IsEnterWindow());
                Callback aCallback = [ this, isEnter, aEvent ]()
                     { MouseListenerMultiplexer& rMouseListeners = this->mpImpl->getMouseListeners();
                       (isEnter)
                           ? rMouseListeners.mouseEntered(aEvent)
                           : rMouseListeners.mouseExited(aEvent); };

                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }

            if ( mpImpl->getMouseMotionListeners().getLength() && !pMouseEvt->IsEnterWindow() && !pMouseEvt->IsLeaveWindow() )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *pMouseEvt, *this ) );
                aEvent.ClickCount = 0;
                if ( pMouseEvt->GetMode() & MouseEventModifiers::SIMPLEMOVE )
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
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *static_cast<MouseEvent*>(rVclWindowEvent.GetData()), *this ) );
                Callback aCallback = [ this, aEvent ]()
                                     { this->mpImpl->getMouseListeners().mousePressed( aEvent ); };
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONUP:
        {
            if ( mpImpl->getMouseListeners().getLength() )
            {
                awt::MouseEvent aEvent( VCLUnoHelper::createMouseEvent( *static_cast<MouseEvent*>(rVclWindowEvent.GetData()), *this ) );

                Callback aCallback = [ this, aEvent ]()
                                     { this->mpImpl->getMouseListeners().mouseReleased( aEvent ); };
                ImplExecuteAsyncWithoutSolarLock( aCallback );
            }
        }
        break;
        case VCLEVENT_WINDOW_STARTDOCKING:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                DockingData *pData = static_cast<DockingData*>(rVclWindowEvent.GetData());

                if( pData )
                {
                    css::awt::DockingEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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
                DockingData *pData = static_cast<DockingData*>(rVclWindowEvent.GetData());

                if( pData )
                {
                    css::awt::DockingEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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

                    css::awt::DockingData aDockingData =
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
                EndDockingData *pData = static_cast<EndDockingData*>(rVclWindowEvent.GetData());

                if( pData )
                {
                    css::awt::EndDockingEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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
                sal_Bool *p_bFloating = static_cast<sal_Bool*>(rVclWindowEvent.GetData());

                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);

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
                css::lang::EventObject aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
                mpImpl->getDockableWindowListeners().notifyEach( &XDockableWindowListener::toggleFloatingMode, aEvent );
            }
       }
        break;
        case VCLEVENT_WINDOW_ENDPOPUPMODE:
        {
            if ( mpImpl->getDockableWindowListeners().getLength() )
            {
                EndPopupModeData *pData = static_cast<EndPopupModeData*>(rVclWindowEvent.GetData());

                if( pData )
                {
                    css::awt::EndPopupModeEvent aEvent;
                    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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

void VCLXWindow::SetSynthesizingVCLEvent( bool _b )
{
    mpImpl->mbSynthesizingVCLEvent = _b;
}

bool VCLXWindow::IsSynthesizingVCLEvent() const
{
    return mpImpl->mbSynthesizingVCLEvent;
}

Size VCLXWindow::ImplCalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Int32 nLeft, nTop, nRight, nBottom;
        pWindow->GetBorder( nLeft, nTop, nRight, nBottom );
        aSz.Width() += nLeft+nRight;
        aSz.Height() += nTop+nBottom;
    }
    return aSz;
}


// css::lang::XUnoTunnel
sal_Int64 VCLXWindow::getSomething( const css::uno::Sequence< sal_Int8 >& rIdentifier ) throw(css::uno::RuntimeException, std::exception)
{
    if( ( rIdentifier.getLength() == 16 ) && ( 0 == memcmp( VCLXWindow::GetUnoTunnelId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
    {
        return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(this));
    }
    return VCLXDevice::getSomething( rIdentifier );
}
namespace
{
    class theVCLXWindowUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theVCLXWindowUnoTunnelId> {};
}
const css::uno::Sequence< sal_Int8 >& VCLXWindow::GetUnoTunnelId() throw()
{
    return theVCLXWindowUnoTunnelId::get().getSeq();
}
VCLXWindow* VCLXWindow::GetImplementation( const css::uno::Reference< css::uno::XInterface >& rxIFace )
{
    css::uno::Reference< css::lang::XUnoTunnel > xUT( rxIFace, css::uno::UNO_QUERY );
    return xUT.is() ? reinterpret_cast<VCLXWindow*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( VCLXWindow::GetUnoTunnelId() ))) : nullptr;
}


// css::lang::Component
void VCLXWindow::dispose(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->mxViewGraphics = nullptr;

    if ( !mpImpl->mbDisposing )
    {
        mpImpl->mbDisposing = true;

        mpImpl->disposing();

        if ( GetWindow() )
        {
            VclPtr<OutputDevice> pOutDev = GetOutputDevice();
            SetWindow( nullptr );  // so that handlers are logged off, if necessary (virtual)
            SetOutputDevice( nullptr );
            pOutDev.disposeAndClear();
        }

        // #i14103# dispose the accessible context after the window has been destroyed,
        // otherwise the old value in the child event fired in VCLXAccessibleComponent::ProcessWindowEvent()
        // for VCLEVENT_WINDOW_CHILDDESTROYED contains a reference to an already disposed accessible object
        try
        {
            css::uno::Reference< css::lang::XComponent > xComponent( mpImpl->mxAccessibleContext, css::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const css::uno::Exception& )
        {
            OSL_FAIL( "VCLXWindow::dispose: could not dispose the accessible context!" );
        }
        mpImpl->mxAccessibleContext.clear();

        mpImpl->mbDisposing = false;
    }
}

void VCLXWindow::addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->getEventListeners().addInterface( rxListener );
}

void VCLXWindow::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->getEventListeners().removeInterface( rxListener );
}


// css::awt::XWindow
void VCLXWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        if( vcl::Window::GetDockingManager()->IsDockable( GetWindow() ) )
            vcl::Window::GetDockingManager()->SetPosSizePixel( GetWindow() , X, Y, Width, Height, static_cast<PosSizeFlags>(Flags) );
        else
            GetWindow()->setPosSizePixel( X, Y, Width, Height, static_cast<PosSizeFlags>(Flags) );
    }
}

css::awt::Rectangle VCLXWindow::getPosSize(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    css::awt::Rectangle aBounds;
    if ( GetWindow() )
    {
        if( vcl::Window::GetDockingManager()->IsDockable( GetWindow() ) )
            aBounds = AWTRectangle( vcl::Window::GetDockingManager()->GetPosSizePixel( GetWindow() ) );
        else
            aBounds = AWTRectangle( Rectangle( GetWindow()->GetPosPixel(), GetWindow()->GetSizePixel() ) );
    }

    return aBounds;
}

void VCLXWindow::setVisible( sal_Bool bVisible ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        mpImpl->setDirectVisible( bVisible );
        pWindow->Show( bVisible &&  mpImpl->isEnableVisible() );
    }
}

void VCLXWindow::setEnable( sal_Bool bEnable ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->Enable( bEnable, false ); // #95824# without children!
        pWindow->EnableInput( bEnable );
    }
}

void VCLXWindow::setFocus(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->GrabFocus();
}

void VCLXWindow::addWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->getWindowListeners().addInterface( rxListener );

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().addInterface( xListener2 );

    // #100119# Get all resize events, even if height or width 0, or invisible
    if ( GetWindow() )
        GetWindow()->EnableAllResize();
}

void VCLXWindow::removeWindowListener( const css::uno::Reference< css::awt::XWindowListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().removeInterface( xListener2 );

    mpImpl->getWindowListeners().removeInterface( rxListener );
}

void VCLXWindow::addFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getFocusListeners().addInterface( rxListener );
}

void VCLXWindow::removeFocusListener( const css::uno::Reference< css::awt::XFocusListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getFocusListeners().removeInterface( rxListener );
}

void VCLXWindow::addKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getKeyListeners().addInterface( rxListener );
}

void VCLXWindow::removeKeyListener( const css::uno::Reference< css::awt::XKeyListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getKeyListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseListener( const css::uno::Reference< css::awt::XMouseListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseMotionListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseMotionListener( const css::uno::Reference< css::awt::XMouseMotionListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getMouseMotionListeners().removeInterface( rxListener );
}

void VCLXWindow::addPaintListener( const css::uno::Reference< css::awt::XPaintListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getPaintListeners().addInterface( rxListener );
}

void VCLXWindow::removePaintListener( const css::uno::Reference< css::awt::XPaintListener >& rxListener ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    mpImpl->getPaintListeners().removeInterface( rxListener );
}

// css::awt::XWindowPeer
css::uno::Reference< css::awt::XToolkit > VCLXWindow::getToolkit(  ) throw(css::uno::RuntimeException, std::exception)
{
    // no guard. nothing to guard here.
    // 82463 - 12/21/00 - fs
    return Application::GetVCLToolkit();
}

void VCLXWindow::setPointer( const css::uno::Reference< css::awt::XPointer >& rxPointer ) throw(css::uno::RuntimeException, std::exception)
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

void VCLXWindow::setBackground( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception)
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

void VCLXWindow::invalidate( sal_Int16 nInvalidateFlags ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->Invalidate( static_cast<InvalidateFlags>(nInvalidateFlags) );
}

void VCLXWindow::invalidateRect( const css::awt::Rectangle& rRect, sal_Int16 nInvalidateFlags ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->Invalidate( VCLRectangle(rRect), static_cast<InvalidateFlags>(nInvalidateFlags) );
}


// css::awt::XVclWindowPeer
sal_Bool VCLXWindow::isChild( const css::uno::Reference< css::awt::XWindowPeer >& rxPeer ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    bool bIsChild = false;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        vcl::Window* pPeerWindow = VCLUnoHelper::GetWindow( rxPeer );
        bIsChild = pPeerWindow && pWindow->IsChild( pPeerWindow );
    }

    return bIsChild;
}

void VCLXWindow::setDesignMode( sal_Bool bOn ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->mbDesignMode = bOn;
}

sal_Bool VCLXWindow::isDesignMode(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return mpImpl->mbDesignMode;
}

void VCLXWindow::enableClipSiblings( sal_Bool bClip ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->EnableClipSiblings( bClip );
}

void VCLXWindow::setForeground( sal_Int32 nColor ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetControlForeground( aColor );
    }
}

void VCLXWindow::setControlFont( const css::awt::FontDescriptor& rFont ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
        GetWindow()->SetControlFont( VCLUnoHelper::CreateFont( rFont, GetWindow()->GetControlFont() ) );
}

void VCLXWindow::getStyles( sal_Int16 nType, css::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        const StyleSettings& rStyleSettings = GetWindow()->GetSettings().GetStyleSettings();

        switch ( nType )
        {
            case css::awt::Style::FRAME:
            {
                Font = VCLUnoHelper::CreateFontDescriptor( rStyleSettings.GetAppFont() );
                ForegroundColor = rStyleSettings.GetWindowTextColor().GetColor();
                BackgroundColor = rStyleSettings.GetWindowColor().GetColor();
            }
            break;
            case css::awt::Style::DIALOG:
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
    static void setColorSettings( vcl::Window* _pWindow, const css::uno::Any& _rValue,
        void (StyleSettings::*pSetter)( const Color& ), const Color& (StyleSettings::*pGetter)( ) const )
    {
        sal_Int32 nColor = 0;
        if ( !( _rValue >>= nColor ) )
            nColor = (Application::GetSettings().GetStyleSettings().*pGetter)().GetColor();

        AllSettings aSettings = _pWindow->GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();

        (aStyleSettings.*pSetter)( Color( nColor ) );

        aSettings.SetStyleSettings( aStyleSettings );
        _pWindow->SetSettings( aSettings, true );
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
            // some properties are not included in the FontDescriptor, but every time
            // when we have a FontDescriptor we want to have these properties too.
            // => Easier to register the here, instead everywhere where I register the FontDescriptor...

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
    void    lcl_updateWritingMode( vcl::Window& _rWindow, const sal_Int16 _nWritingMode, const sal_Int16 _nContextWritingMode )
    {
        bool bEnableRTL = false;
        switch ( _nWritingMode )
        {
        case WritingMode2::LR_TB:   bEnableRTL = false; break;
        case WritingMode2::RL_TB:   bEnableRTL = true; break;
        case WritingMode2::CONTEXT:
        {
            // consult our ContextWritingMode. If it has an explicit RTL/LTR value, then use
            // it. If it doesn't (but is CONTEXT itself), then just ask the parent window of our
            // own window for its RTL mode
            switch ( _nContextWritingMode )
            {
                case WritingMode2::LR_TB:   bEnableRTL = false; break;
                case WritingMode2::RL_TB:   bEnableRTL = true; break;
                case WritingMode2::CONTEXT:
                {
                    const vcl::Window* pParent = _rWindow.GetParent();
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

void VCLXWindow::setProperty( const OUString& PropertyName, const css::uno::Any& Value )
    throw(css::uno::RuntimeException,
          std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( !pWindow )
        return;

    bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

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
            bool bProperType = ( Value >>= mpImpl->mnWritingMode );
            OSL_ENSURE( bProperType, "VCLXWindow::setProperty( 'WritingMode' ): illegal value type!" );
            if ( bProperType )
                lcl_updateWritingMode( *pWindow, mpImpl->mnWritingMode, mpImpl->mnContextWritingMode );
        }
        break;

        case BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR:
        {
            sal_uInt16 nWheelBehavior( css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY );
            OSL_VERIFY( Value >>= nWheelBehavior );

            AllSettings aSettings = pWindow->GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();

            MouseWheelBehaviour nVclBehavior( MouseWheelBehaviour::FocusOnly );
            switch ( nWheelBehavior )
            {
            case css::awt::MouseWheelBehavior::SCROLL_DISABLED:   nVclBehavior = MouseWheelBehaviour::Disable;     break;
            case css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY: nVclBehavior = MouseWheelBehaviour::FocusOnly;  break;
            case css::awt::MouseWheelBehavior::SCROLL_ALWAYS:     nVclBehavior = MouseWheelBehaviour::ALWAYS;      break;
            default:
                OSL_FAIL( "VCLXWindow::setProperty( 'MouseWheelBehavior' ): illegal property value!" );
            }

            aMouseSettings.SetWheelBehavior( nVclBehavior );
            aSettings.SetMouseSettings( aMouseSettings );
            pWindow->SetSettings( aSettings, true );
        }
        break;

        case BASEPROPERTY_NATIVE_WIDGET_LOOK:
        {
            bool bEnable( true );
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
            bool b = bool();
            if ( Value >>= b )
                setEnable( b );
        }
        break;
        case BASEPROPERTY_ENABLEVISIBLE:
        {
            bool b = false;
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
            OUString aText;
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
            OUString aText;
            if ( Value >>= aText )
                pWindow->SetAccessibleName( aText );
        }
        break;
        case BASEPROPERTY_HELPURL:
        {
            OUString aURL;
            if ( Value >>= aURL )
            {
                INetURLObject aHelpURL( aURL );
                if ( aHelpURL.GetProtocol() == INetProtocol::Hid )
                    pWindow->SetHelpId( OUStringToOString( aHelpURL.GetURLPath(), RTL_TEXTENCODING_UTF8 ) );
                else
                    pWindow->SetHelpId( OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ) );
            }
        }
        break;
        case BASEPROPERTY_HELPTEXT:
        {
            OUString aHelpText;
            if ( Value >>= aHelpText )
            {
                pWindow->SetQuickHelpText( aHelpText );
            }
        }
        break;
        case BASEPROPERTY_FONTDESCRIPTOR:
        {
            if ( bVoid )
                pWindow->SetControlFont( vcl::Font() );
            else
            {
                css::awt::FontDescriptor aFont;
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
                vcl::Font aFont = pWindow->GetControlFont();
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
                vcl::Font aFont = pWindow->GetControlFont();
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
                        pWindow->SetPaintTransparent( true );
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
                            pWindow->SetPaintTransparent( false );
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
            sal_uInt16 nTmp = 0;
            Value >>= nTmp;
            WindowBorderStyle nBorder = static_cast<WindowBorderStyle>(nTmp);
            if ( !bool(nBorder) )
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
                bool bTab = false;
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
                    SAL_FALLTHROUGH;
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
                bool bMulti = false;
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
            bool bAutoMnemonics = false;
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
            bool bMouseTransparent = false;
            Value >>= bMouseTransparent;
            pWindow->SetMouseTransparent( bMouseTransparent );
        }
        break;
        case BASEPROPERTY_PAINTTRANSPARENT:
        {
            bool bPaintTransparent = false;
            Value >>= bPaintTransparent;
            pWindow->SetPaintTransparent( bPaintTransparent );
//                pWindow->SetBackground();
        }
        break;

        case BASEPROPERTY_REPEAT:
        {
            bool bRepeat( false );
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

                pWindow->SetSettings( aSettings, true );
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
            OUString aName;
            Value >>= aName;
            break;
        }
    }
}

css::uno::Any VCLXWindow::getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    if ( GetWindow() )
    {
        WindowType eWinType = GetWindow()->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_REFERENCE_DEVICE:
            {
                VclPtr<Control> pControl = GetAsDynamic<Control >();
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
                MouseWheelBehaviour nVclBehavior = GetWindow()->GetSettings().GetMouseSettings().GetWheelBehavior();
                sal_uInt16 nBehavior = css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY;
                switch ( nVclBehavior )
                {
                case MouseWheelBehaviour::Disable:       nBehavior = css::awt::MouseWheelBehavior::SCROLL_DISABLED;    break;
                case MouseWheelBehaviour::FocusOnly:     nBehavior = css::awt::MouseWheelBehavior::SCROLL_FOCUS_ONLY;  break;
                case MouseWheelBehaviour::ALWAYS:        nBehavior = css::awt::MouseWheelBehavior::SCROLL_ALWAYS;      break;
                default:
                    OSL_FAIL( "VCLXWindow::getProperty( 'MouseWheelBehavior' ): illegal VCL value!" );
                }
                aProp <<= nBehavior;
            }
            break;

            case BASEPROPERTY_NATIVE_WIDGET_LOOK:
                aProp <<= GetWindow()->IsNativeWidgetEnabled();
                break;

            case BASEPROPERTY_ENABLED:
                aProp <<= GetWindow()->IsEnabled();
                break;

            case BASEPROPERTY_ENABLEVISIBLE:
                aProp <<= mpImpl->isEnableVisible();
                break;

            case BASEPROPERTY_HIGHCONTRASTMODE:
                aProp <<= GetWindow()->GetSettings().GetStyleSettings().GetHighContrastMode();
                break;

            case BASEPROPERTY_TEXT:
            case BASEPROPERTY_LABEL:
            case BASEPROPERTY_TITLE:
            {
                OUString aText = GetWindow()->GetText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_ACCESSIBLENAME:
            {
                OUString aText = GetWindow()->GetAccessibleName();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_HELPTEXT:
            {
                OUString aText = GetWindow()->GetQuickHelpText();
                aProp <<= aText;
            }
            break;
            case BASEPROPERTY_HELPURL:
            {
                OUString aHelpId( OStringToOUString( GetWindow()->GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
                aProp <<= OUString( aHelpId );
            }
            break;
            case BASEPROPERTY_FONTDESCRIPTOR:
            {
                vcl::Font aFont = GetWindow()->GetControlFont();
                css::awt::FontDescriptor aFD = VCLUnoHelper::CreateFontDescriptor( aFont );
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
                WindowBorderStyle nBorder = WindowBorderStyle::NONE;
                if ( GetWindow()->GetStyle() & WB_BORDER )
                    nBorder = GetWindow()->GetBorderStyle();
                aProp <<= static_cast<sal_uInt16>(nBorder);
            }
            break;
            case BASEPROPERTY_TABSTOP:
                aProp <<= ( GetWindow()->GetStyle() & WB_TABSTOP ) != 0;
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
                    aProp <<= ( GetWindow()->GetStyle() & WB_WORDBREAK ) != 0;
            }
            break;
            case BASEPROPERTY_AUTOMNEMONICS:
            {
                bool bAutoMnemonics = GetWindow()->GetSettings().GetStyleSettings().GetAutoMnemonic();
                aProp <<= bAutoMnemonics;
            }
            break;
            case BASEPROPERTY_MOUSETRANSPARENT:
            {
                bool bMouseTransparent = GetWindow()->IsMouseTransparent();
                aProp <<= bMouseTransparent;
            }
            break;
            case BASEPROPERTY_PAINTTRANSPARENT:
            {
                bool bPaintTransparent = GetWindow()->IsPaintTransparent();
                aProp <<= bPaintTransparent;
            }
            break;

            case BASEPROPERTY_REPEAT:
                aProp <<= ( 0 != ( GetWindow()->GetStyle() & WB_REPEAT ) );
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


// css::awt::XLayoutConstrains
css::awt::Size VCLXWindow::getMinimumSize(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    // Use this method only for those components which can be created through
    // css::awt::Toolkit , but do not have an interface

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
                aSz = GetWindow()->get_preferred_size();
        }
    }

    return css::awt::Size( aSz.Width(), aSz.Height() );
}

css::awt::Size VCLXWindow::getPreferredSize(  ) throw(css::uno::RuntimeException, std::exception)
{
    return getMinimumSize();
}

css::awt::Size VCLXWindow::calcAdjustedSize( const css::awt::Size& rNewSize ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    css::awt::Size aNewSize( rNewSize );
    css::awt::Size aMinSize = getMinimumSize();

    if ( aNewSize.Width < aMinSize.Width )
        aNewSize.Width = aMinSize.Width;
    if ( aNewSize.Height < aMinSize.Height )
        aNewSize.Height = aMinSize.Height;

    return aNewSize;
}


// css::awt::XView
sal_Bool VCLXWindow::setGraphics( const css::uno::Reference< css::awt::XGraphics >& rxDevice ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( VCLUnoHelper::GetOutputDevice( rxDevice ) )
        mpImpl->mxViewGraphics = rxDevice;
    else
        mpImpl->mxViewGraphics = nullptr;

    return mpImpl->mxViewGraphics.is();
}

css::uno::Reference< css::awt::XGraphics > VCLXWindow::getGraphics(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    return mpImpl->mxViewGraphics;
}

css::awt::Size VCLXWindow::getSize(  ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Size aSz;
    if ( GetWindow() )
        aSz = GetWindow()->GetSizePixel();
    return css::awt::Size( aSz.Width(), aSz.Height() );
}

void VCLXWindow::draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( !pWindow )
        return;

    if ( isDesignMode() || mpImpl->isEnableVisible() )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mpImpl->mxViewGraphics );
        if (!pDev)
            pDev = pWindow->GetParent();
        TabPage* pTabPage = dynamic_cast< TabPage* >( pWindow );
        if ( pTabPage )
        {
            Point aPos( nX, nY );
            Size  aSize = pWindow->GetSizePixel();

            aPos  = pDev->PixelToLogic( aPos );
            aSize = pDev->PixelToLogic( aSize );

            pTabPage->Draw( pDev, aPos, aSize, DrawFlags::NONE );
            return;
        }

        Point aPos( nX, nY );

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

                bool bWasVisible = pWindow->IsVisible();
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
                pWindow->SetParentUpdateMode( false );
                pWindow->Hide();
                pWindow->SetParentUpdateMode( true );

                pWindow->SetPosPixel( aOldPos );
                if ( bWasVisible )
                    pWindow->Show();
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
                               || ( pPDFExport != nullptr );
            if ( bDrawSimple )
            {
                pWindow->Draw( pDev, aP, aSz, DrawFlags::NoControls );
            }
            else
            {
                bool bOldNW =pWindow->IsNativeWidgetEnabled();
                if( bOldNW )
                    pWindow->EnableNativeWidget(false);
                pWindow->PaintToDevice( pDev, aP, aSz );
                if( bOldNW )
                    pWindow->EnableNativeWidget();
            }
        }
    }
}

void VCLXWindow::setZoom( float fZoomX, float /*fZoomY*/ ) throw(css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
        // Fraction::Fraction takes a double, but we have a float only.
        // The implicit conversion from float to double can result in a precision loss, i.e. 1.2 is converted to
        // 1.200000000047something. To prevent this, we convert explicitly to double, and round it.
        double nZoom( fZoomX );
        Fraction aZoom(::rtl::math::round(nZoom, 4));
        aZoom.ReduceInaccurate(10); // to avoid runovers and BigInt mapping
        GetWindow()->SetZoom(aZoom);
    }
}

// css::lang::XEventListener
void SAL_CALL VCLXWindow::disposing( const css::lang::EventObject& _rSource ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    // check if it comes from our AccessibleContext
    uno::Reference< uno::XInterface > aAC( mpImpl->mxAccessibleContext, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xSource( _rSource.Source, uno::UNO_QUERY );

    if ( aAC.get() == xSource.get() )
    {   // yep, it does
        mpImpl->mxAccessibleContext.clear();
    }
}

// css::accessibility::XAccessible
css::uno::Reference< css::accessibility::XAccessibleContext > VCLXWindow::getAccessibleContext(  ) throw (css::uno::RuntimeException, std::exception)
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

// css::awt::XDockable
void SAL_CALL VCLXWindow::addDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if ( xListener.is() )
        mpImpl->getDockableWindowListeners().addInterface( xListener );

}

void SAL_CALL VCLXWindow::removeDockableWindowListener( const css::uno::Reference< css::awt::XDockableWindowListener >& xListener ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    mpImpl->getDockableWindowListeners().removeInterface( xListener );
}

void SAL_CALL VCLXWindow::enableDocking( sal_Bool bEnable ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->EnableDocking( bEnable );
}

sal_Bool SAL_CALL VCLXWindow::isFloating(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if( pWindow )
        return vcl::Window::GetDockingManager()->IsFloating( pWindow );
    else
        return sal_False;
}

void SAL_CALL VCLXWindow::setFloatingMode( sal_Bool bFloating ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if( pWindow )
        vcl::Window::GetDockingManager()->SetFloatingMode( pWindow, bFloating );
}

sal_Bool SAL_CALL VCLXWindow::isLocked(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if( pWindow )
        return vcl::Window::GetDockingManager()->IsLocked( pWindow );
    else
        return sal_False;
}

void SAL_CALL VCLXWindow::lock(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if( pWindow && !vcl::Window::GetDockingManager()->IsFloating( pWindow ) )
        vcl::Window::GetDockingManager()->Lock( pWindow );
}

void SAL_CALL VCLXWindow::unlock(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = GetWindow();
    if( pWindow && !vcl::Window::GetDockingManager()->IsFloating( pWindow ) )
        vcl::Window::GetDockingManager()->Unlock( pWindow );
}
void SAL_CALL VCLXWindow::startPopupMode( const css::awt::Rectangle& ) throw (css::uno::RuntimeException, std::exception)
{
    // TODO: remove interface in the next incompatible build
    SolarMutexGuard aGuard;

}

sal_Bool SAL_CALL VCLXWindow::isInPopupMode(  ) throw (css::uno::RuntimeException, std::exception)
{
    // TODO: remove interface in the next incompatible build
    SolarMutexGuard aGuard;
    return sal_False;
}


// css::awt::XWindow2

void SAL_CALL VCLXWindow::setOutputSize( const css::awt::Size& aSize ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    vcl::Window *pWindow;
    if( (pWindow = GetWindow()) != nullptr )
    {
        DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >(pWindow);
        if( pDockingWindow )
            pDockingWindow->SetOutputSizePixel( VCLSize( aSize ) );
        else
            pWindow->SetOutputSizePixel( VCLSize( aSize ) );
    }
}

css::awt::Size SAL_CALL VCLXWindow::getOutputSize(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    vcl::Window *pWindow;
    if( (pWindow = GetWindow()) != nullptr )
    {
        DockingWindow *pDockingWindow = dynamic_cast< DockingWindow* >(pWindow);
        if( pDockingWindow )
            return AWTSize( pDockingWindow->GetOutputSizePixel() );
        else
            return AWTSize( pWindow->GetOutputSizePixel() );
    }
    else
        return css::awt::Size();
}

sal_Bool SAL_CALL VCLXWindow::isVisible(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsVisible();
    else
        return sal_False;
}

sal_Bool SAL_CALL VCLXWindow::isActive(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsActive();
    else
        return sal_False;

}

sal_Bool SAL_CALL VCLXWindow::isEnabled(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->IsEnabled();
    else
        return sal_False;
}

sal_Bool SAL_CALL VCLXWindow::hasFocus(  ) throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if( GetWindow() )
        return GetWindow()->HasFocus();
    else
        return sal_False;
}

// css::beans::XPropertySetInfo

UnoPropertyArrayHelper *
VCLXWindow::GetPropHelper()
{
    SolarMutexGuard aGuard;
    if ( mpImpl->mpPropHelper == nullptr )
    {
        std::list< sal_uInt16 > aIDs;
        GetPropertyIds( aIDs );
        mpImpl->mpPropHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return mpImpl->mpPropHelper;
}

css::uno::Sequence< css::beans::Property > SAL_CALL
VCLXWindow::getProperties() throw (css::uno::RuntimeException, std::exception)
{
    return GetPropHelper()->getProperties();
}
css::beans::Property SAL_CALL
VCLXWindow::getPropertyByName( const OUString& rName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception)
{
    return GetPropHelper()->getPropertyByName( rName );
}

sal_Bool SAL_CALL
VCLXWindow::hasPropertyByName( const OUString& rName ) throw (css::uno::RuntimeException, std::exception)
{
    return GetPropHelper()->hasPropertyByName( rName );
}

Reference< XStyleSettings > SAL_CALL VCLXWindow::getStyleSettings() throw (RuntimeException, std::exception)
{
    return mpImpl->getStyleSettings();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
