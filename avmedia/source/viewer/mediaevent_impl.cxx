/*************************************************************************
 *
 *  $RCSfile: mediaevent_impl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: ka $ $Date: 2004-08-23 09:04:42 $
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

void SAL_CALL MediaEventListenersImpl::disposing( const ::com::sun::star::lang::EventObject& Source )
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
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), e.ClickCount, 0,
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
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), e.ClickCount, 0,
                                ( ( e.Buttons & 1 ) ? MOUSE_LEFT : 0 ) |
                                ( ( e.Buttons & 2 ) ? MOUSE_RIGHT : 0 ) |
                                ( ( e.Buttons & 4 ) ? MOUSE_MIDDLE : 0 ),
                                e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, reinterpret_cast< ::Window* >( mpNotifyWindow ), &aVCLMouseEvt );
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseEntered( const ::com::sun::star::awt::MouseEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const ::vos::OGuard aAppGuard( Application::GetSolarMutex() );

    if( mpNotifyWindow )
    {
    }
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::mouseExited( const ::com::sun::star::awt::MouseEvent& e )
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

void SAL_CALL MediaEventListenersImpl::focusGained( const ::com::sun::star::awt::FocusEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
}

// ---------------------------------------------------------------------

void SAL_CALL MediaEventListenersImpl::focusLost( const ::com::sun::star::awt::FocusEvent& e )
    throw (::com::sun::star::uno::RuntimeException)
{
}

} // namespace priv
} // namespace avemdia

