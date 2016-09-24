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

#include "mediaevent_impl.hxx"
#include "mediawindow_impl.hxx"
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>

using namespace ::com::sun::star;

namespace avmedia { namespace priv {

MediaEventListenersImpl::MediaEventListenersImpl( vcl::Window& rEventWindow ) :
    mpNotifyWindow( &rEventWindow )
{
}


MediaEventListenersImpl::~MediaEventListenersImpl()
{
}


void MediaEventListenersImpl::cleanUp()
{
    Application::RemoveMouseAndKeyEvents( reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ) );
    mpNotifyWindow = nullptr;
}


void SAL_CALL MediaEventListenersImpl::disposing( const css::lang::EventObject& )
    throw (css::uno::RuntimeException, std::exception)
{
}


void SAL_CALL MediaEventListenersImpl::keyPressed( const css::awt::KeyEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        vcl::KeyCode aVCLKeyCode( e.KeyCode,
                                  ( ( e.Modifiers & 1 ) ? KEY_SHIFT : 0 ) |
                                  ( ( e.Modifiers & 2 ) ? KEY_MOD1 : 0 ) |
                                  ( ( e.Modifiers & 4 ) ? KEY_MOD2 : 0 ) );
        KeyEvent aVCLKeyEvt( e.KeyChar, aVCLKeyCode );

        Application::PostKeyEvent( VCLEVENT_WINDOW_KEYINPUT, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLKeyEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::keyReleased( const css::awt::KeyEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        vcl::KeyCode aVCLKeyCode( e.KeyCode,
                                  ( ( e.Modifiers & 1 ) ? KEY_SHIFT : 0 ) |
                                  ( ( e.Modifiers & 2 ) ? KEY_MOD1 : 0 ) |
                                  ( ( e.Modifiers & 4 ) ? KEY_MOD2 : 0 ) );
        KeyEvent aVCLKeyEvt( e.KeyChar, aVCLKeyCode );
        Application::PostKeyEvent( VCLEVENT_WINDOW_KEYUP, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLKeyEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::mousePressed( const css::awt::MouseEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ),
                                 sal::static_int_cast< sal_uInt16 >(e.ClickCount),
                                 MouseEventModifiers::NONE,
                                ( ( e.Buttons & 1 ) ? MOUSE_LEFT : 0 ) |
                                ( ( e.Buttons & 2 ) ? MOUSE_RIGHT : 0 ) |
                                ( ( e.Buttons & 4 ) ? MOUSE_MIDDLE : 0 ),
                                e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLMouseEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::mouseReleased( const css::awt::MouseEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ),
                                 sal::static_int_cast< sal_uInt16 >(e.ClickCount),
                                 MouseEventModifiers::NONE,
                                ( ( e.Buttons & 1 ) ? MOUSE_LEFT : 0 ) |
                                ( ( e.Buttons & 2 ) ? MOUSE_RIGHT : 0 ) |
                                ( ( e.Buttons & 4 ) ? MOUSE_MIDDLE : 0 ),
                                e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEBUTTONUP, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLMouseEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::mouseEntered( const css::awt::MouseEvent& )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
    }
}


void SAL_CALL MediaEventListenersImpl::mouseExited( const css::awt::MouseEvent& )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
    }
}


void SAL_CALL MediaEventListenersImpl::mouseDragged( const css::awt::MouseEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), 0, MouseEventModifiers::NONE, e.Buttons, e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLMouseEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::mouseMoved( const css::awt::MouseEvent& e )
    throw (css::uno::RuntimeException, std::exception)
{
    const ::osl::MutexGuard aGuard( maMutex );
    const SolarMutexGuard aAppGuard;

    if( mpNotifyWindow )
    {
        MouseEvent aVCLMouseEvt( Point( e.X, e.Y ), 0, MouseEventModifiers::NONE, e.Buttons, e.Modifiers );
        Application::PostMouseEvent( VCLEVENT_WINDOW_MOUSEMOVE, reinterpret_cast< vcl::Window* >( mpNotifyWindow.get() ), &aVCLMouseEvt );
    }
}


void SAL_CALL MediaEventListenersImpl::focusGained( const css::awt::FocusEvent& )
    throw (css::uno::RuntimeException, std::exception)
{
}


void SAL_CALL MediaEventListenersImpl::focusLost( const css::awt::FocusEvent& )
    throw (css::uno::RuntimeException, std::exception)
{
}


} // namespace priv
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
