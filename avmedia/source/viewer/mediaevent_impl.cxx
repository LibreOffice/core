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

#include "mediaevent_impl.hxx"
#include "mediawindow_impl.hxx"
#include <osl/mutex.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

using namespace ::com::sun::star;

/* Definition of MediaWindowImpl class */

namespace avmedia { namespace priv {
// ---------------------------
// - MediaEventListenersImpl -
// ---------------------------

MediaEventListenersImpl::MediaEventListenersImpl( Window& rEventWindow ) :
    mpNotifyWindow( &rEventWindow )
{
}

// ---------------------------------------------------------------------

MediaEventListenersImpl::~MediaEventListenersImpl()
{
}

// ---------------------------------------------------------------------

void MediaEventListenersImpl::cleanUp()
{
    Application::RemoveMouseAndKeyEvents( reinterpret_cast< ::Window* >( mpNotifyWindow ) );
    mpNotifyWindow = NULL;
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
    throw (::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::keyPressed( const ::com::sun::star::awt::KeyEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        KeyCode aVCLKeyCode( e.KeyCode,
                            ( ( e.Modifiers & 1 ) ? KEY_SHIFT : 0 ) |
                            ( ( e.Modifiers & 2 ) ? KEY_MOD1 : 0 ) |
                            ( ( e.Modifiers & 4 ) ? KEY_MOD2 : 0 ) );
        KeyEvent aVCLKeyEvt( e.KeyChar, aVCLKeyCode );

        Application::PostKeyEvent( VCLEVENT_WINDOW_KEYINPUT, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLKeyEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::keyReleased( const ::com::sun::star::awt::KeyEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        KeyCode aVCLKeyCode( e.KeyCode,
                            ( ( e.Modifiers & 1 ) ? KEY_SHIFT : 0 ) |
                            ( ( e.Modifiers & 2 ) ? KEY_MOD1 : 0 ) |
                            ( ( e.Modifiers & 4 ) ? KEY_MOD2 : 0 ) );
        KeyEvent aVCLKeyEvt( e.KeyChar, aVCLKeyCode );
        Application::PostKeyEvent( VCLEVENT_WINDOW_KEYUP, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLKeyEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mousePressed( const ::com::sun::star::awt::MouseEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ),
                                 sal::static_int_cast< sal_uInt16 >(e.ClickCount),
                                 0,
                                ( ( e.Buttons & 1 ) ? MOUSE_LEFT : 0 ) |
                                ( ( e.Buttons & 2 ) ? MOUSE_RIGHT : 0 ) |
                                ( ( e.Buttons & 4 ) ? MOUSE_MIDDLE : 0 ),
                                e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLMouseEvt );
    }
}

// ----------------------------------------------gvd-----------------------

void SAL_CALL MediaEventListenersImpl::mouseReleased( const ::com::sun::star::awt::MouseEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ),
                                 sal::static_int_cast< sal_uInt16 >(e.ClickCount),
                                 0,
                                ( ( e.Buttons & 1 ) ? MOUSE_LEFT : 0 ) |
                                ( ( e.Buttons & 2 ) ? MOUSE_RIGHT : 0 ) |
                                ( ( e.Buttons & 4 ) ? MOUSE_MIDDLE : 0 ),
                                e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLMouseEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseEntered( const ::com::sun::star::awt::MouseEvent& /* e */ )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseExited( const ::com::sun::star::awt::MouseEvent& /* e */ )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseDragged( const ::com::sun::star::awt::MouseEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), 0, 0, e.Buttons, e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLMouseEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseMoved( const ::com::sun::star::awt::MouseEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), 0, 0, e.Buttons, e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLMouseEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::focusGained( const ::com::sun::star::awt::FocusEvent& /* e */ )
    throw (::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::focusLost( const ::com::sun::star::awt::FocusEvent& /* e */ )
    throw (::com::sun::star::uno::RuntimeException)
{
}

} // namespace priv
} // namespace avemdia

