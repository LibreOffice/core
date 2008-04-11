/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxwindow.cxx,v $
 * $Revision: 1.86 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/Style.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/awt/DockingEvent.hpp>
#include <com/sun/star/awt/EndDockingEvent.hpp>
#include <com/sun/star/awt/EndPopupModeEvent.hpp>
#include <com/sun/star/awt/XWindowListener2.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxpointer.hxx>
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
#include <comphelper/asyncnotification.hxx>
#include <toolkit/helper/solarrelease.hxx>

#include <toolkit/helper/unopropertyarrayhelper.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::awt::XWindowListener2;
using ::com::sun::star::style::VerticalAlignment;
using ::com::sun::star::style::VerticalAlignment_TOP;
using ::com::sun::star::style::VerticalAlignment_MIDDLE;
using ::com::sun::star::style::VerticalAlignment_BOTTOM;
using ::com::sun::star::style::VerticalAlignment_MAKE_FIXED_SIZE;


//#define SYNCHRON_NOTIFICATION
    // define this for notifying mouse events synchronously when they happen
    // disadvantage: potential of deadlocks, since this means that the
    // SolarMutex is locked when the listener is called
    // See http://www.openoffice.org/issues/show_bug.cgi?id=40583 for an example
    // deadlock
//#define THREADED_NOTIFICATION
    // define this for notifying mouse events asynchronously, in a dedicated thread
    // This is what I'd like to use. However, there's some Windows API code
    // which doesn't like being called in the non-main thread, and we didn't
    // find out which one :(
    // See http://www.openoffice.org/issues/show_bug.cgi?id=47502 for an example
    // of a bug triggered by asynchronous notification in a foreign thread

// If none of the above is defined, then mouse events are notified asynchronously
// in the main thread, using PostUserEvent.
// disadvantage: The event we're posting is delayed until the next event
// reschedule. Normally, this is virtually immediately, but there's no guarantee
// ....

//====================================================================
//= VCLXWindowImpl
//====================================================================
namespace
{
    enum MouseEventType
    {
        EVENT_MOUSE_PRESSED     = 0,
        EVENT_MOUSE_RELEASED    = 1,
        EVENT_MOUSE_ENTERED     = 2,
        EVENT_MOUSE_EXITED      = 3,

        META_FIRST_MOUSE_EVENT  = 0,
        META_LAST_MOUSE_EVENT   = 3
    };

    enum PlainEventType
    {
        EVENT_WINDOW_ENABLED    = 4,
        EVENT_WINDOW_DISABLED   = 5,

        META_FIRST_PLAIN_EVENT  = 4,
        META_LAST_PLAIN_EVENT   = 5
    };

#if OSL_DEBUG_LEVEL > 0
    static void checkEventDefinitions()
    {
        OSL_ENSURE( (int)META_LAST_MOUSE_EVENT < (int)META_FIRST_PLAIN_EVENT, "checkEventDefinitions: invalid event definitions!" );
    }
    #define DBG_CHECK_EVENTS()  checkEventDefinitions()
#else
    #define DBG_CHECK_EVENTS()
#endif

    struct AnyWindowEvent : public ::comphelper::AnyEvent
    {
    private:
        awt::MouseEvent     m_aMouseEvent;
        lang::EventObject   m_aPlainEvent;

        sal_Int32           m_nEventType;

    public:
        AnyWindowEvent( const awt::MouseEvent& _rEvent, MouseEventType _nType )
            :comphelper::AnyEvent()
            ,m_aMouseEvent( _rEvent )
            ,m_nEventType( static_cast< sal_Int32 >( _nType ) )
        {
            DBG_CHECK_EVENTS();
        }

        AnyWindowEvent( const lang::EventObject& _rEvent, PlainEventType _nType )
            :comphelper::AnyEvent()
            ,m_aPlainEvent( _rEvent )
            ,m_nEventType( static_cast< sal_Int32 >( _nType ) )
        {
            DBG_CHECK_EVENTS();
        }

        bool    isMouseEvent() const
        {
            return ( META_FIRST_MOUSE_EVENT <= m_nEventType ) && ( m_nEventType <= META_LAST_MOUSE_EVENT );
        }

        bool    isPlainEvent() const
        {
            return ( META_FIRST_PLAIN_EVENT <= m_nEventType ) && ( m_nEventType <= META_LAST_PLAIN_EVENT );
        }

        const awt::MouseEvent& getMouseEvent() const
        {
            OSL_ENSURE( isMouseEvent(), "AnyWindowEvent::getMouseEvent: no mouse event!" );
            return m_aMouseEvent;
        }

        MouseEventType getMouseEventType() const
        {
            OSL_ENSURE( isMouseEvent(), "AnyWindowEvent::getMouseEventType: no mouse event!" );
            return static_cast< MouseEventType >( m_nEventType );
        }

        const lang::EventObject& getPlainEvent() const
        {
            OSL_ENSURE( isPlainEvent(), "AnyWindowEvent::getPlainEvent: no plain event!" );
            return m_aPlainEvent;
        }

        PlainEventType getPlainEventType() const
        {
            OSL_ENSURE( isPlainEvent(), "AnyWindowEvent::getPlainEventType: no mouse event!" );
            return static_cast< PlainEventType >( m_nEventType );
        }
    };
}

class SAL_DLLPRIVATE VCLXWindowImpl : public ::comphelper::IEventProcessor
{
private:
    typedef ::std::vector< ::rtl::Reference< ::comphelper::AnyEvent > >
        EventArray;

private:
    oslInterlockedCount                 m_refCount;
    VCLXWindow&                         mrAntiImpl;
    ::vos::IMutex&                      mrMutex;
    ::toolkit::AccessibilityClient      maAccFactory;
    bool                                mbDisposed;
    ::osl::Mutex                        maListenerContainerMutex;
    ::cppu::OInterfaceContainerHelper   maWindow2Listeners;

#ifdef THREADED_NOTIFICATION
    ::rtl::Reference< ::comphelper::AsyncEventNotifier >
                                        mpAsyncNotifier;
#else
#if !defined( SYNCHRON_NOTIFICATION )
    EventArray                          maEvents;
    sal_Int32                           mnEventId;
#endif
#endif

public:
    /** ctor
    @param _pAntiImpl
        the <type>VCLXWindow</type> instance which the object belongs to. Must
        live longer then the object just being constructed.
    */
    VCLXWindowImpl( VCLXWindow& _rAntiImpl, ::vos::IMutex& _rMutex );

    /** asynchronously notifies a mouse event to the VCLXWindow's XMouseListeners
    */
    void    notifyMouseEvent( const awt::MouseEvent& _rMouseEvent, MouseEventType _nType );

    /** asynchronously notifies an event described by an EventObject to the respective listeners
    */
    void    notifyPlainEvent( const lang::EventObject& _rPlainEvent, PlainEventType _nType );

    /** notifies the object that its VCLXWindow is being disposed
    */
    void    disposing();

    inline ::toolkit::AccessibilityClient& getAccessibleFactory()
    {
        return maAccFactory;
    }

    /** returns the container of registered XWindowListener2 listeners
    */
    inline ::cppu::OInterfaceContainerHelper&
            getWindow2Listeners() { return maWindow2Listeners; }

    virtual void SAL_CALL acquire();
    virtual void SAL_CALL release();

protected:
    virtual ~VCLXWindowImpl();

    // IEventProcessor
    virtual void processEvent( const ::comphelper::AnyEvent& _rEvent );

#if !defined( SYNCHRON_NOTIFICATION ) && !defined( THREADED_NOTIFICATION )
private:
    DECL_LINK( OnProcessEvent, void* );
#endif

private:
    /** notifies an arbitrary event
        @param _rEvent
            the event to notify
        @param _rGuard
            a guard currentl guarding our mutex, which is released for the actual notification
    */
    void impl_notifyAnyEvent(
        const ::rtl::Reference< ::comphelper::AnyEvent >& _rEvent,
        ::vos::OClearableGuard& _rGuard
    );

private:
    /** determines whether the instance is already disposed
        @precond
            m_aMutex must be acquired
    */
    inline bool impl_isDisposed()
    {
        return mbDisposed;
    }

private:
    VCLXWindowImpl();                                   // never implemented
    VCLXWindowImpl( const VCLXWindowImpl& );            // never implemented
    VCLXWindowImpl& operator=( const VCLXWindowImpl& ); // never implemented
};

//--------------------------------------------------------------------
VCLXWindowImpl::VCLXWindowImpl( VCLXWindow& _rAntiImpl, ::vos::IMutex& _rMutex )
    :m_refCount( 0 )
    ,mrAntiImpl( _rAntiImpl )
    ,mrMutex( _rMutex )
    ,mbDisposed( false )
    ,maListenerContainerMutex( )
    ,maWindow2Listeners( maListenerContainerMutex )
#ifdef THREADED_NOTIFICATION
    ,mpAsyncNotifier( NULL )
#else
#ifndef SYNCHRON_NOTIFICATION
    ,mnEventId( 0 )
#endif
#endif
{
}

VCLXWindowImpl::~VCLXWindowImpl()
{
}

//--------------------------------------------------------------------
void VCLXWindowImpl::disposing()
{
    ::vos::OGuard aGuard( mrMutex );
#ifdef THREADED_NOTIFICATION
    if ( mpAsyncNotifier.is() )
    {
        mpAsyncNotifier->removeEventsForProcessor( this );
        mpAsyncNotifier->dispose();
        mpAsyncNotifier = NULL;
    }
#else
#ifndef SYNCHRON_NOTIFICATION
    if ( mnEventId )
        Application::RemoveUserEvent( mnEventId );
    mnEventId = 0;
#endif
#endif
    mbDisposed= true;
}

//--------------------------------------------------------------------
void VCLXWindowImpl::impl_notifyAnyEvent( const ::rtl::Reference< ::comphelper::AnyEvent >& _rEvent, ::vos::OClearableGuard& _rGuard )
{
#ifdef THREADED_NOTIFICATION
    (void)_rGuard;
    if ( !mpAsyncNotifier.is() )
    {
        mpAsyncNotifier = new ::comphelper::AsyncEventNotifier;
        mpAsyncNotifier->create();
    }
    mpAsyncNotifier->addEvent( _rEvent, this );

#else   // #ifdef THREADED_NOTIFICATION

#ifdef SYNCHRON_NOTIFICATION
    _rGuard.clear();
    processEvent( *_rEvent );
#else   // #ifdef SYNCHRON_NOTIFICATION
    (void)_rGuard;
    maEvents.push_back( _rEvent );
    if ( !mnEventId )
        mnEventId = Application::PostUserEvent( LINK( this, VCLXWindowImpl, OnProcessEvent ) );
#endif  // #ifdef SYNCHRON_NOTIFICATION

#endif  // // #ifdef THREADED_NOTIFICATION
}

//--------------------------------------------------------------------
void VCLXWindowImpl::notifyMouseEvent( const awt::MouseEvent& _rMouseEvent, MouseEventType _nType )
{
    ::vos::OClearableGuard aGuard( mrMutex );
    if ( mrAntiImpl.GetMouseListeners().getLength() )
        impl_notifyAnyEvent( new AnyWindowEvent( _rMouseEvent, _nType ), aGuard );
}

//--------------------------------------------------------------------
void VCLXWindowImpl::notifyPlainEvent( const lang::EventObject& _rPlainEvent, PlainEventType _nType )
{
    ::vos::OClearableGuard aGuard( mrMutex );
    if ( maWindow2Listeners.getLength() )
        impl_notifyAnyEvent( new AnyWindowEvent( _rPlainEvent, _nType ), aGuard );
}

#if !defined( SYNCHRON_NOTIFICATION ) && !defined( THREADED_NOTIFICATION )
//--------------------------------------------------------------------
IMPL_LINK( VCLXWindowImpl, OnProcessEvent, void*, EMPTYARG )
{
    // work on a copy of the events array
    EventArray aEventsCopy;
    {
        ::vos::OGuard aGuard( mrMutex );
        aEventsCopy = maEvents;
        maEvents.clear();

        if ( !mnEventId )
            // we were disposed while waiting for the mutex to lock
            return 1L;

        mnEventId = 0;
    }

    {
        ::toolkit::ReleaseSolarMutex aReleaseSolar;
        for (   EventArray::const_iterator loop = aEventsCopy.begin();
                loop != aEventsCopy.end();
                ++loop
            )
        {
            processEvent( *(*loop) );
        }
    }

    return 0L;
}
#endif

//--------------------------------------------------------------------
void VCLXWindowImpl::processEvent( const ::comphelper::AnyEvent& _rEvent )
{
    ::vos::OGuard aGuard( mrMutex );
    if ( impl_isDisposed() )
        // while we were waiting for our mutex, another thread disposed us
        return;

    const AnyWindowEvent& rEventDescriptor( static_cast< const AnyWindowEvent& >( _rEvent ) );
    if ( rEventDescriptor.isMouseEvent() )
    {
        const awt::MouseEvent& rEvent( rEventDescriptor.getMouseEvent() );
        switch ( rEventDescriptor.getMouseEventType() )
        {
        case EVENT_MOUSE_PRESSED:
            mrAntiImpl.GetMouseListeners().mousePressed( rEvent );
            break;
        case EVENT_MOUSE_RELEASED:
            mrAntiImpl.GetMouseListeners().mouseReleased( rEvent );
            break;
        case EVENT_MOUSE_ENTERED:
            mrAntiImpl.GetMouseListeners().mouseEntered( rEvent );
            break;
        case EVENT_MOUSE_EXITED:
            mrAntiImpl.GetMouseListeners().mouseExited( rEvent );
            break;
        default:
            DBG_ERROR( "VCLXWindowImpl::processEvent: what kind of event *is* this (1)?" );
            break;
        }
    }
    else if ( rEventDescriptor.isPlainEvent() )
    {
        const lang::EventObject& rEvent( rEventDescriptor.getPlainEvent() );
        switch ( rEventDescriptor.getPlainEventType() )
        {
        case EVENT_WINDOW_ENABLED:
            maWindow2Listeners.notifyEach( &XWindowListener2::windowEnabled, rEvent );
            break;
        case EVENT_WINDOW_DISABLED:
            maWindow2Listeners.notifyEach( &XWindowListener2::windowDisabled, rEvent );
            break;
        default:
            DBG_ERROR( "VCLXWindowImpl::processEvent: what kind of event *is* this (2)?" );
            break;
        }
    }
    else
        DBG_ERROR( "VCLXWindowImpl::processEvent: what kind of event *is* this (3)?" );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXWindowImpl::acquire()
{
    osl_incrementInterlockedCount( &m_refCount );
}

//--------------------------------------------------------------------
void SAL_CALL VCLXWindowImpl::release()
{
    if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        delete this;
}

//====================================================================
//====================================================================

// Mit Out-Parameter besser als Rueckgabewert, wegen Ref-Objekt...

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

void ImplInitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent, const KeyEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.GetKeyCode().IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.GetKeyCode().IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.GetKeyCode().IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.KeyCode = rEvt.GetKeyCode().GetCode();
    rEvent.KeyChar = rEvt.GetCharCode();
    rEvent.KeyFunc = sal::static_int_cast< sal_Int16 >(
        rEvt.GetKeyCode().GetFunction());
}

void ImplInitMouseEvent( awt::MouseEvent& rEvent, const MouseEvent& rEvt )
{
    rEvent.Modifiers = 0;
    if ( rEvt.IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( rEvt.IsMod1() )
    rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( rEvt.IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;

    rEvent.Buttons = 0;
    if ( rEvt.IsLeft() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::LEFT;
    if ( rEvt.IsRight() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::RIGHT;
    if ( rEvt.IsMiddle() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::MIDDLE;

    rEvent.X = rEvt.GetPosPixel().X();
    rEvent.Y = rEvt.GetPosPixel().Y();
    rEvent.ClickCount = rEvt.GetClicks();
    rEvent.PopupTrigger = sal_False;
}

//  ----------------------------------------------------
//  class VCLXWindow
//  ----------------------------------------------------

DBG_NAME(VCLXWindow);

VCLXWindow::VCLXWindow( bool bWithDefaultProps )
    : maEventListeners( *this ),
      maFocusListeners( *this ),
      maWindowListeners( *this ),
      maKeyListeners( *this ),
      maMouseListeners( *this ),
      maMouseMotionListeners( *this ),
      maPaintListeners( *this ),
      maContainerListeners( *this ),
      maTopWindowListeners( *this ),
      mnListenerLockLevel( 0 ),
      mpImpl( NULL ),
      mpPropHelper( NULL ),
      mbDisposing( false ),
      mbDesignMode( false ),
      mbSynthesizingVCLEvent( false ),
      mbWithDefaultProps( !!bWithDefaultProps ),
      mbDrawingOntoParent( false )
{
    DBG_CTOR( VCLXWindow, 0 );

    mpImpl = new VCLXWindowImpl( *this, GetMutex() );
    mpImpl->acquire();

    mbDisposing = sal_False;
    mbDesignMode = sal_False;
    mbSynthesizingVCLEvent = sal_False;
}

VCLXWindow::~VCLXWindow()
{
    DBG_DTOR( VCLXWindow, 0 );

    delete mpPropHelper;

    if ( GetWindow() )
    {
        GetWindow()->RemoveEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
        GetWindow()->SetWindowPeer( NULL, NULL );
        GetWindow()->SetAccessible( NULL );
    }
}

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
        GetWindow()->AddEventListener( LINK( this, VCLXWindow, WindowEventListener ) );
}

void VCLXWindow::suspendVclEventListening( )
{
    ++mnListenerLockLevel;
}

void VCLXWindow::resumeVclEventListening( )
{
    DBG_ASSERT( mnListenerLockLevel, "VCLXWindow::resumeVclEventListening: not suspended!" );
    --mnListenerLockLevel;
}

IMPL_LINK( VCLXWindow, WindowEventListener, VclSimpleEvent*, pEvent )
{
    if ( mnListenerLockLevel )
        return 0L;

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow() && GetWindow(), "Window???" );
        ProcessWindowEvent( *(VclWindowEvent*)pEvent );
    }
    return 0;
}

void VCLXWindow::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xThis( (::cppu::OWeakObject*)this );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_ENABLED:
        case VCLEVENT_WINDOW_DISABLED:
        {
            bool bEnabled = ( VCLEVENT_WINDOW_ENABLED == rVclWindowEvent.GetId() );
            EventObject aEvent( *this );
            mpImpl->notifyPlainEvent( aEvent,
                bEnabled ? EVENT_WINDOW_ENABLED : EVENT_WINDOW_DISABLED );
        }
        break;

        case VCLEVENT_WINDOW_PAINT:
        {
            if ( GetPaintListeners().getLength() )
            {
                ::com::sun::star::awt::PaintEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.UpdateRect = AWTRectangle( *(Rectangle*)rVclWindowEvent.GetData() );
                aEvent.Count = 0;
                GetPaintListeners().windowPaint( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowMoved( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_RESIZE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowResized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_SHOW:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowShown( aEvent );
            }

            // For TopWindows this means opened...
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowOpened( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            if ( GetWindowListeners().getLength() )
            {
                ::com::sun::star::awt::WindowEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitWindowEvent( aEvent, rVclWindowEvent.GetWindow() );
                GetWindowListeners().windowHidden( aEvent );
            }

            // For TopWindows this means closed...
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowClosed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowActivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowDeactivated( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_CLOSE:
        {
            if ( mxDockableWindowListener.is() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mxDockableWindowListener->closed( aEvent );
            }
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowClosing( aEvent );
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
                if ( GetFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
                    aEvent.Temporary = sal_False;
                    GetFocusListeners().focusGained( aEvent );
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
                if ( GetFocusListeners().getLength() )
                {
                    ::com::sun::star::awt::FocusEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FocusFlags = rVclWindowEvent.GetWindow()->GetGetFocusFlags();
                    aEvent.Temporary = sal_False;

                    Window* pNext = Application::GetFocusWindow();
                    if ( pNext )
                    {
                        // Bei zusammengesetzten Controls interessiert sich keiner fuer das Innenleben:
                        Window* pNextC = pNext;
                        while ( pNextC && !pNextC->IsCompoundControl() )
                            pNextC = pNextC->GetParent();
                        if ( pNextC )
                            pNext = pNextC;

                        pNext->GetComponentInterface( sal_True );
                        aEvent.NextFocus = (::cppu::OWeakObject*)pNext->GetWindowPeer();
                    }
                    GetFocusListeners().focusLost( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MINIMIZE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowMinimized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_NORMALIZE:
        {
            if ( GetTopWindowListeners().getLength() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTopWindowListeners().windowNormalized( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYINPUT:
        {
            if ( GetKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitKeyEvent( aEvent, *(KeyEvent*)rVclWindowEvent.GetData() );
                GetKeyListeners().keyPressed( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_KEYUP:
        {
            if ( GetKeyListeners().getLength() )
            {
                ::com::sun::star::awt::KeyEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitKeyEvent( aEvent, *(KeyEvent*)rVclWindowEvent.GetData() );
                GetKeyListeners().keyReleased( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_COMMAND:
        {
            CommandEvent* pCmdEvt = (CommandEvent*)rVclWindowEvent.GetData();
            if ( GetMouseListeners().getLength() && ( pCmdEvt->GetCommand() == COMMAND_CONTEXTMENU ) )
            {
                // COMMAND_CONTEXTMENU als mousePressed mit PopupTrigger = sal_True versenden...
                Point aWhere = static_cast< CommandEvent* >( rVclWindowEvent.GetData() )->GetMousePosPixel();
                if ( !pCmdEvt->IsMouseEvent() )
                {   // for keyboard events, we set the coordinates to -1,-1. This is a slight HACK, but the current API
                    // handles a context menu command as special case of a mouse event, which is simply wrong.
                    // Without extending the API, we would not have another chance to notify listeners of a
                    // keyboard-triggered context menu request
                    // 102205 - 16.08.2002 - fs@openoffice.org
                    aWhere = Point( -1, -1 );
                }

                MouseEvent aMEvt( aWhere, 1, MOUSE_SIMPLECLICK, MOUSE_LEFT, 0 );
                awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, aMEvt );
                aEvent.PopupTrigger = sal_True;
                mpImpl->notifyMouseEvent( aEvent, EVENT_MOUSE_PRESSED );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEMOVE:
        {
            MouseEvent* pMouseEvt = (MouseEvent*)rVclWindowEvent.GetData();
            if ( GetMouseListeners().getLength() && ( pMouseEvt->IsEnterWindow() || pMouseEvt->IsLeaveWindow() ) )
            {
                awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *pMouseEvt );

                mpImpl->notifyMouseEvent(
                    aEvent,
                    pMouseEvt->IsEnterWindow() ? EVENT_MOUSE_ENTERED : EVENT_MOUSE_EXITED
                );
            }

            if ( GetMouseMotionListeners().getLength() && !pMouseEvt->IsEnterWindow() && !pMouseEvt->IsLeaveWindow() )
            {
                awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *pMouseEvt );
                aEvent.ClickCount = 0;  // #92138#

                if ( pMouseEvt->GetMode() & MOUSE_SIMPLEMOVE )
                    GetMouseMotionListeners().mouseMoved( aEvent );
                else
                    GetMouseMotionListeners().mouseDragged( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONDOWN:
        {
            if ( GetMouseListeners().getLength() )
            {
                awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *(MouseEvent*)rVclWindowEvent.GetData() );
                mpImpl->notifyMouseEvent( aEvent, EVENT_MOUSE_PRESSED );
            }
        }
        break;
        case VCLEVENT_WINDOW_MOUSEBUTTONUP:
        {
            if ( GetMouseListeners().getLength() )
            {
                awt::MouseEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                ImplInitMouseEvent( aEvent, *(MouseEvent*)rVclWindowEvent.GetData() );
                mpImpl->notifyMouseEvent( aEvent, EVENT_MOUSE_RELEASED );
            }
        }
        break;
        case VCLEVENT_WINDOW_STARTDOCKING:
        {
            if ( mxDockableWindowListener.is() )
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

                    mxDockableWindowListener->startDocking( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_DOCKING:
        {
            if ( mxDockableWindowListener.is() )
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
                    ::com::sun::star::awt::DockingData aDockingData =
                        mxDockableWindowListener->docking( aEvent );
                    pData->maTrackRect = VCLRectangle( aDockingData.TrackingRectangle );
                    pData->mbFloating = aDockingData.bFloating;
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_ENDDOCKING:
        {
            if ( mxDockableWindowListener.is() )
            {
                EndDockingData *pData = (EndDockingData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::EndDockingEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.WindowRectangle = AWTRectangle( pData->maWindowRect );
                    aEvent.bFloating = pData->mbFloating;
                    aEvent.bCancelled = pData->mbCancelled;
                    mxDockableWindowListener->endDocking( aEvent );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_PREPARETOGGLEFLOATING:
        {
            if ( mxDockableWindowListener.is() )
            {
                BOOL *p_bFloating = (BOOL*)rVclWindowEvent.GetData();

                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                *p_bFloating = mxDockableWindowListener->prepareToggleFloatingMode( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_TOGGLEFLOATING:
        {
            if ( mxDockableWindowListener.is() )
            {
                ::com::sun::star::lang::EventObject aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                mxDockableWindowListener->toggleFloatingMode( aEvent );
            }
        }
        break;
        case VCLEVENT_WINDOW_ENDPOPUPMODE:
        {
            if ( mxDockableWindowListener.is() )
            {
                EndPopupModeData *pData = (EndPopupModeData*)rVclWindowEvent.GetData();

                if( pData )
                {
                    ::com::sun::star::awt::EndPopupModeEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.FloatingPosition.X = pData->maFloatingPos.X();
                    aEvent.FloatingPosition.Y = pData->maFloatingPos.Y();
                    aEvent.bTearoff = pData->mbTearoff;
                    mxDockableWindowListener->endPopupMode( aEvent );
                }
            }
        }
        break;

    }
}

uno::Reference< accessibility::XAccessibleContext > VCLXWindow::CreateAccessibleContext()
{
    ::vos::OGuard aGuard( GetMutex() );
    return getAccessibleFactory().createAccessibleContext( this );
}

/*
void VCLXWindow::FillAccessibleStateSet( AccessibleStateSetHelper& rStateSet )
{
}
*/

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


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XComponent*, (::com::sun::star::awt::XWindow*)this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindow*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XVclWindowPeer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XView*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::accessibility::XAccessible*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::beans::XPropertySetInfo*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XWindow2*, this ),
                                           SAL_STATIC_CAST( ::com::sun::star::awt::XDockableWindow*, this ) );
    return (aRet.hasValue() ? aRet : VCLXDevice::queryInterface( rType ));
}

// ::com::sun::star::lang::XUnoTunnel
IMPL_XUNOTUNNEL2( VCLXWindow, VCLXDevice )

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXWindow )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow2>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XView>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindow>* ) NULL ),
    VCLXDevice::getTypes()
IMPL_XTYPEPROVIDER_END


// ::com::sun::star::lang::Component
void VCLXWindow::dispose(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    mxViewGraphics = NULL;

    if ( !mbDisposing )
    {
        mbDisposing = sal_True;

        ::com::sun::star::lang::EventObject aObj;
        aObj.Source = static_cast< ::cppu::OWeakObject* >( this );

        maEventListeners.disposeAndClear( aObj );
        maFocusListeners.disposeAndClear( aObj );
        maWindowListeners.disposeAndClear( aObj );
        maKeyListeners.disposeAndClear( aObj );
        maMouseListeners.disposeAndClear( aObj );
        maMouseMotionListeners.disposeAndClear( aObj );
        maPaintListeners.disposeAndClear( aObj );
        maContainerListeners.disposeAndClear( aObj );
        maTopWindowListeners.disposeAndClear( aObj );

        if ( mpImpl )
        {
            mpImpl->disposing();
            mpImpl->release();
            mpImpl = NULL;
        }

        if ( GetWindow() )
        {
            OutputDevice* pOutDev = GetOutputDevice();
            SetWindow( NULL );  // Damit ggf. Handler abgemeldet werden (virtuell).
            SetOutputDevice( pOutDev );
            DestroyOutputDevice();
        }

        // #i14103# dispose the accessible context after the window has been destroyed,
        // otherwise the old value in the child event fired in VCLXAccessibleComponent::ProcessWindowEvent()
        // for VCLEVENT_WINDOW_CHILDDESTROYED contains a reference to an already disposed accessible object
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xComponent( mxAccessibleContext, ::com::sun::star::uno::UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const ::com::sun::star::uno::Exception& )
        {
            DBG_ERROR( "VCLXWindow::dispose: could not dispose the accessible context!" );
        }
        mxAccessibleContext.clear();

        mbDisposing = sal_False;
    }
}

void VCLXWindow::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().addInterface( rxListener );
}

void VCLXWindow::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetEventListeners().removeInterface( rxListener );
}


// ::com::sun::star::awt::XWindow
void VCLXWindow::setPosSize( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, sal_Int16 Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
/*
        if ( bVisible )
        {
            // #57167# TopWindows mit unsichtbaren Parent anzeigen...
            ::com::sun::star::uno::Any aTest = queryInterface( ::getCppuType(  (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >*)  0 ) );
            if ( aTest.hasValue() )
            {
                Window* pParent = pWindow->GetWindow( WINDOW_PARENTOVERLAP );
                if ( pParent && !pParent->IsReallyVisible() )
                    pWindow->SetParent( pWindow->GetWindow( WINDOW_FRAME ) );
            }
        }
*/
        pWindow->Show( bVisible );
    }
}

void VCLXWindow::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->Enable( bEnable, FALSE ); // #95824# without children!
        pWindow->EnableInput( bEnable );
    }
}

void VCLXWindow::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->GrabFocus();
}

void VCLXWindow::addWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetWindowListeners().addInterface( rxListener );

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().addInterface( xListener2 );

    // #100119# Get all resize events, even if height or width 0, or invisible
    if ( GetWindow() )
        GetWindow()->EnableAllResize( TRUE );
}

void VCLXWindow::removeWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Reference< XWindowListener2 > xListener2( rxListener, UNO_QUERY );
    if ( xListener2.is() )
        mpImpl->getWindow2Listeners().removeInterface( xListener2 );

    GetWindowListeners().removeInterface( rxListener );
}

void VCLXWindow::addFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().addInterface( rxListener );
}

void VCLXWindow::removeFocusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetFocusListeners().removeInterface( rxListener );
}

void VCLXWindow::addKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().addInterface( rxListener );
}

void VCLXWindow::removeKeyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetKeyListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseListeners().removeInterface( rxListener );
}

void VCLXWindow::addMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().addInterface( rxListener );
}

void VCLXWindow::removeMouseMotionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseMotionListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetMouseMotionListeners().removeInterface( rxListener );
}

void VCLXWindow::addPaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().addInterface( rxListener );
}

void VCLXWindow::removePaintListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPaintListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetPaintListeners().removeInterface( rxListener );
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
    ::vos::OGuard aGuard( GetMutex() );

    VCLXPointer* pPointer = VCLXPointer::GetImplementation( rxPointer );
    if ( pPointer )
    {
        mxPointer = rxPointer;
        if ( GetWindow() )
            GetWindow()->SetPointer( pPointer->GetPointer() );
    }
}

void VCLXWindow::setBackground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( (sal_uInt16) nInvalidateFlags );
}

void VCLXWindow::invalidateRect( const ::com::sun::star::awt::Rectangle& rRect, sal_Int16 nInvalidateFlags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->Invalidate( VCLRectangle(rRect), (sal_uInt16) nInvalidateFlags );
}


// ::com::sun::star::awt::XVclWindowPeer
sal_Bool VCLXWindow::isChild( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& rxPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    mbDesignMode = bOn;
}

sal_Bool VCLXWindow::isDesignMode(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mbDesignMode;
}

void VCLXWindow::enableClipSiblings( sal_Bool bClip ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->EnableClipSiblings( bClip );
}

void VCLXWindow::setForeground( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Color aColor( (sal_uInt32)nColor );
        GetWindow()->SetControlForeground( aColor );
    }
}

void VCLXWindow::setControlFont( const ::com::sun::star::awt::FontDescriptor& rFont ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetControlFont( VCLUnoHelper::CreateFont( rFont, GetWindow()->GetControlFont() ) );
}

void VCLXWindow::getStyles( sal_Int16 nType, ::com::sun::star::awt::FontDescriptor& Font, sal_Int32& ForegroundColor, sal_Int32& BackgroundColor ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
            default: DBG_ERROR( "VCLWindow::getStyles() - unknown Type" );
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
        _pWindow->SetSettings( aSettings, TRUE );
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
                         BASEPROPERTY_TABSTOP,
                         0);

    // lovely hack from:
    // void UnoControlModel::ImplRegisterProperty( sal_uInt16 nPropId )
    std::list< sal_uInt16 >::const_iterator iter;
    for( iter = rIds.begin(); iter != rIds.end(); iter++) {
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

void VCLXWindow::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        WindowType eWinType = pWindow->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_WHEELWITHOUTFOCUS:
            {
                sal_Bool bWheelOnHover( sal_True );
                if ( Value >>= bWheelOnHover )
                {
                    AllSettings aSettings = pWindow->GetSettings();
                    MouseSettings aMouseSettings = aSettings.GetMouseSettings();

                    aMouseSettings.SetNoWheelActionWithoutFocus( !bWheelOnHover );
                    aSettings.SetMouseSettings( aMouseSettings );

                    pWindow->SetSettings( aSettings, TRUE );
                }
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
                            if (aText.getLength())
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
                    String aHelpURL(  aURL );
                    String aPattern( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
                    if ( aHelpURL.CompareIgnoreCaseToAscii( aPattern, aPattern.Len() ) == COMPARE_EQUAL )
                    {
                        String aID = aHelpURL.Copy( aPattern.Len() );
                        pWindow->SetHelpId( aID.ToInt32() );
                    }
                    else
                    {
                        pWindow->SetSmartHelpId( SmartId( aHelpURL ) );
                    }
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
                            pWindow->SetPaintTransparent( TRUE );
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
                                pWindow->SetPaintTransparent( FALSE );
                            default: ;
                        }
                        pWindow->Invalidate();  // Falls das Control nicht drauf reagiert
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
                sal_Bool bRepeat( FALSE );
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

                    pWindow->SetSettings( aSettings, TRUE );
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
}

::com::sun::star::uno::Any VCLXWindow::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        WindowType eWinType = GetWindow()->GetType();
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_WHEELWITHOUTFOCUS:
            {
                sal_Bool bWheelOnHover = !GetWindow()->GetSettings().GetMouseSettings().GetNoWheelActionWithoutFocus();
                aProp <<= bWheelOnHover;
            }
            break;

            case BASEPROPERTY_NATIVE_WIDGET_LOOK:
                aProp <<= (sal_Bool) GetWindow()->IsNativeWidgetEnabled();
                break;

            case BASEPROPERTY_ENABLED:
                aProp <<= (sal_Bool) GetWindow()->IsEnabled();
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
                SmartId aSmartId = GetWindow()->GetSmartHelpId();
                if( aSmartId.HasString() )
                {
                    String aStrHelpId = aSmartId.GetStr();
                    aProp <<= ::rtl::OUString( aStrHelpId );
                }
                else
                {
                    ::rtl::OUStringBuffer aURL;
                    aURL.appendAscii( "HID:" );
                    aURL.append( (sal_Int32) GetWindow()->GetHelpId() );
                    aProp <<= aURL.makeStringAndClear();
                }
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
    ::vos::OGuard aGuard( GetMutex() );

    // Diese Methode sollte nur fuer Componenten gerufen werden, die zwar
    // ueber das ::com::sun::star::awt::Toolkit erzeugt werden koennen, aber fuer die es
    // kein Interface gibt.

    Size aSz;
    if ( GetWindow() )
    {
        WindowType nWinType = GetWindow()->GetType();
        switch ( nWinType )
        {
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                aSz = Size( 250, 100 );
            break;

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
            case WINDOW_SCROLLBAR:
            {
                long n = GetWindow()->GetSettings().GetStyleSettings().GetScrollBarSize();
                aSz = Size( n, n );
            }
            break;
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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    if ( VCLUnoHelper::GetOutputDevice( rxDevice ) )
        mxViewGraphics = rxDevice;
    else
        mxViewGraphics = NULL;

    return mxViewGraphics.is();
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics > VCLXWindow::getGraphics(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    return mxViewGraphics;
}

::com::sun::star::awt::Size VCLXWindow::getSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    if ( GetWindow() )
        aSz = GetWindow()->GetSizePixel();
    return ::com::sun::star::awt::Size( aSz.Width(), aSz.Height() );
}

namespace
{
    class FlagGuard
    {
    private:
        sal_Bool&   m_rFlag;

    public:
        FlagGuard( sal_Bool& _rFlag )
            :m_rFlag( _rFlag )
        {
            m_rFlag = sal_True;
        }
        ~FlagGuard()
        {
            m_rFlag = sal_False;
        }
    };
}

void VCLXWindow::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( !pWindow )
        return;

    if ( pWindow )
    {
        TabPage* pTabPage = dynamic_cast< TabPage* >( pWindow );
        if ( pTabPage )
        {
            Point aPos( nX, nY );
            Size  aSize = pWindow->GetSizePixel();

            OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mxViewGraphics );
            aPos  = pDev->PixelToLogic( aPos );
            aSize = pDev->PixelToLogic( aSize );

            pTabPage->Draw( pDev, aPos, aSize, 0 );
            return;
        }

        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( mxViewGraphics );
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
            if ( !mbDrawingOntoParent )
            {
                FlagGuard aDrawingflagGuard( mbDrawingOntoParent );

                BOOL bWasVisible = pWindow->IsVisible();
                Point aOldPos( pWindow->GetPosPixel() );

                if ( bWasVisible && aOldPos == aPos )
                {
                    pWindow->Update();
                    return;
                }

                pWindow->SetPosPixel( aPos );

                // Erstmal ein Update auf den Parent, damit nicht beim Update
                // auf dieses Fenster noch ein Paint vom Parent abgearbeitet wird,
                // wo dann ggf. dieses Fenster sofort wieder gehidet wird.
                if( pWindow->GetParent() )
                    pWindow->GetParent()->Update();

                pWindow->Show();
                pWindow->Update();
                pWindow->SetParentUpdateMode( sal_False );
                pWindow->Hide();
                pWindow->SetParentUpdateMode( sal_True );

                pWindow->SetPosPixel( aOldPos );
                if ( bWasVisible )
                    pWindow->Show( TRUE );
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
                               || ( pPDFExport && ! pPDFExport->GetIsExportFormFields() );
            if ( bDrawSimple )
            {
                pWindow->Draw( pDev, aP, aSz, WINDOW_DRAW_NOCONTROLS );
            }
            else
            {
                BOOL bOldNW =pWindow->IsNativeWidgetEnabled();
                if( bOldNW )
                    pWindow->EnableNativeWidget(FALSE);
                pWindow->PaintToDevice( pDev, aP, aSz );
                if( bOldNW )
                    pWindow->EnableNativeWidget(TRUE);
            }
        }
    }
}

void VCLXWindow::setZoom( float fZoomX, float /*fZoomY*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
        GetWindow()->SetZoom( Fraction( fZoomX ) );
}

// ::com::sun::star::lang::XEventListener
void SAL_CALL VCLXWindow::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    // check if it comes from our AccessibleContext
    uno::Reference< uno::XInterface > aAC( mxAccessibleContext, uno::UNO_QUERY );
    uno::Reference< uno::XInterface > xSource( _rSource.Source, uno::UNO_QUERY );

    if ( aAC.get() == xSource.get() )
    {   // yep, it does
        mxAccessibleContext = uno::Reference< accessibility::XAccessibleContext >();
    }
}

// ::com::sun::star::accessibility::XAccessible
::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXWindow::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
{
    using namespace ::com::sun::star;

    ::vos::OGuard aGuard( GetMutex() );

    if ( !mxAccessibleContext.is() && GetWindow() )
    {
        mxAccessibleContext = CreateAccessibleContext();

        // add as event listener to this component
        // in case somebody disposes it, we do not want to have a (though weak) reference to a dead
        // object
        uno::Reference< lang::XComponent > xComp( mxAccessibleContext, uno::UNO_QUERY );
        if ( xComp.is() )
            xComp->addEventListener( this );
    }

    return mxAccessibleContext;
}

// ::com::sun::star::awt::XDockable
void SAL_CALL VCLXWindow::addDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( !mxDockableWindowListener.is() )
        mxDockableWindowListener = xListener;
    //else
    //    throw too_many_listeners_exception

}

void SAL_CALL VCLXWindow::removeDockableWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDockableWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if( mxDockableWindowListener == xListener )
        mxDockableWindowListener.clear();
}

void SAL_CALL VCLXWindow::enableDocking( sal_Bool bEnable ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->EnableDocking( bEnable );
}

sal_Bool SAL_CALL VCLXWindow::isFloating(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if( pWindow )
        return Window::GetDockingManager()->IsFloating( pWindow );
    else
        return FALSE;
}

void SAL_CALL VCLXWindow::setFloatingMode( sal_Bool bFloating ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if( pWindow )
        Window::GetDockingManager()->SetFloatingMode( pWindow, bFloating );
}

sal_Bool SAL_CALL VCLXWindow::isLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if( pWindow )
        return Window::GetDockingManager()->IsLocked( pWindow );
    else
        return FALSE;
}

void SAL_CALL VCLXWindow::lock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if( pWindow && !Window::GetDockingManager()->IsFloating( pWindow ) )
        Window::GetDockingManager()->Lock( pWindow );
}

void SAL_CALL VCLXWindow::unlock(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if( pWindow && !Window::GetDockingManager()->IsFloating( pWindow ) )
        Window::GetDockingManager()->Unlock( pWindow );
}
void SAL_CALL VCLXWindow::startPopupMode( const ::com::sun::star::awt::Rectangle& ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: remove interface in the next incompatible build
    ::vos::OGuard aGuard( GetMutex() );

}

sal_Bool SAL_CALL VCLXWindow::isInPopupMode(  ) throw (::com::sun::star::uno::RuntimeException)
{
    // TODO: remove interface in the next incompatible build
    ::vos::OGuard aGuard( GetMutex() );
    return FALSE;
}


// ::com::sun::star::awt::XWindow2

void SAL_CALL VCLXWindow::setOutputSize( const ::com::sun::star::awt::Size& aSize ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
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
    ::vos::OGuard aGuard( GetMutex() );
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
    ::vos::OGuard aGuard( GetMutex() );
    if( GetWindow() )
        return GetWindow()->IsVisible();
    else
        return FALSE;
}

sal_Bool SAL_CALL VCLXWindow::isActive(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    if( GetWindow() )
        return GetWindow()->IsActive();
    else
        return FALSE;

}

sal_Bool SAL_CALL VCLXWindow::isEnabled(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    if( GetWindow() )
        return GetWindow()->IsEnabled();
    else
        return FALSE;
}

sal_Bool SAL_CALL VCLXWindow::hasFocus(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    if( GetWindow() )
        return GetWindow()->HasFocus();
    else
        return FALSE;
}

// ::com::sun::star::beans::XPropertySetInfo

UnoPropertyArrayHelper *
VCLXWindow::GetPropHelper()
{
    ::vos::OGuard aGuard( GetMutex() );
    if (mpPropHelper == NULL)
    {
        std::list< sal_uInt16 > aIDs;
        GetPropertyIds( aIDs );
        mpPropHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return mpPropHelper;
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
