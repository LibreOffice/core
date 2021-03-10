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

#include <sal/config.h>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>

#if defined ( MACOSX )
#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#endif

#include <vcl/sysdata.hxx>
#include <comphelper/sequence.hxx>

#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Any;
using ::com::sun::star::lang::IndexOutOfBoundsException;


css::uno::Any VCLXTopWindow::getWindowHandle( const css::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType )
{
    SolarMutexGuard aGuard;

    // TODO, check the process id
    css::uno::Any aRet;
    vcl::Window* pWindow = VCLXContainer::GetWindow();
    if ( pWindow )
    {
        const SystemEnvData* pSysData = static_cast<SystemWindow *>(pWindow)->GetSystemData();
        if( pSysData )
        {
#if defined (_WIN32)
            if( SystemType == css::lang::SystemDependent::SYSTEM_WIN32 )
            {
                 aRet <<= reinterpret_cast<sal_IntPtr>(pSysData->hWnd);
            }
#elif defined(MACOSX)
            if( SystemType == css::lang::SystemDependent::SYSTEM_MAC )
            {
                 aRet <<= reinterpret_cast<sal_IntPtr>(pSysData->mpNSView);
            }
#elif defined(ANDROID)
            // Nothing
            (void) SystemType;
#elif defined(IOS)
            // Nothing
            (void) SystemType;
#elif defined(UNX)
            if( SystemType == css::lang::SystemDependent::SYSTEM_XWINDOW )
            {
                css::awt::SystemDependentXWindow aSD;
                aSD.DisplayPointer = sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(pSysData->pDisplay));
                aSD.WindowHandle = pSysData->GetWindowHandle(pWindow->ImplGetFrame());
                aRet <<= aSD;
            }
#endif
        }
    }
    return aRet;
}

void VCLXTopWindow::addTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener )
{
    SolarMutexGuard aGuard;

    GetTopWindowListeners().addInterface( rxListener );
}

void VCLXTopWindow::removeTopWindowListener( const css::uno::Reference< css::awt::XTopWindowListener >& rxListener )
{
    SolarMutexGuard aGuard;

    GetTopWindowListeners().removeInterface( rxListener );
}

void VCLXTopWindow::toFront(  )
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = VCLXContainer::GetWindow();
    if (pWindow)
        pWindow->ToTop( ToTopFlags::RestoreWhenMin );
}

void VCLXTopWindow::toBack(  )
{
}

void VCLXTopWindow::setMenuBar( const css::uno::Reference< css::awt::XMenuBar >& rxMenu )
{
    SolarMutexGuard aGuard;

    vcl::Window* pWindow = VCLXContainer::GetWindow();
    if ( pWindow )
    {
        SystemWindow* pSystemWindow = static_cast<SystemWindow*>( pWindow );
        pSystemWindow->SetMenuBar( nullptr );
        if ( rxMenu.is() )
        {
            VCLXMenu* pMenu = comphelper::getUnoTunnelImplementation<VCLXMenu>( rxMenu );
            if ( pMenu && !pMenu->IsPopupMenu() )
                pSystemWindow->SetMenuBar( static_cast<MenuBar*>( pMenu->GetMenu() ));
        }
    }
}


sal_Bool SAL_CALL VCLXTopWindow::getIsMaximized()
{
    SolarMutexGuard aGuard;

    const WorkWindow* pWindow = VCLXContainer::GetAsDynamic<WorkWindow>();
    if ( !pWindow )
        return false;

    return pWindow->IsMaximized();
}


void SAL_CALL VCLXTopWindow::setIsMaximized( sal_Bool _ismaximized )
{
    SolarMutexGuard aGuard;

    WorkWindow* pWindow = VCLXContainer::GetAsDynamic<WorkWindow>();
    if ( !pWindow )
        return;

    pWindow->Maximize( _ismaximized );
}


sal_Bool SAL_CALL VCLXTopWindow::getIsMinimized()
{
    SolarMutexGuard aGuard;

    const WorkWindow* pWindow = VCLXContainer::GetAsDynamic<WorkWindow>();
    if ( !pWindow )
        return false;

    return pWindow->IsMinimized();
}


void SAL_CALL VCLXTopWindow::setIsMinimized( sal_Bool _isMinimized )
{
    SolarMutexGuard aGuard;

    WorkWindow* pWindow = VCLXContainer::GetAsDynamic<WorkWindow>();
    if ( !pWindow )
        return;

    _isMinimized ? pWindow->Minimize() : pWindow->Restore();
}


::sal_Int32 SAL_CALL VCLXTopWindow::getDisplay()
{
    SolarMutexGuard aGuard;

    const SystemWindow* pWindow = VCLXContainer::GetAsDynamic<SystemWindow>();
    if ( !pWindow )
        return 0;

    return pWindow->GetScreenNumber();
}


void SAL_CALL VCLXTopWindow::setDisplay( ::sal_Int32 _display )
{
    SolarMutexGuard aGuard;

    if ( ( _display < 0 ) || ( _display >= static_cast<sal_Int32>(Application::GetScreenCount()) ) )
        throw IndexOutOfBoundsException();

    SystemWindow* pWindow = VCLXContainer::GetAsDynamic<SystemWindow>();
    if ( !pWindow )
        return;

    pWindow->SetScreenNumber( _display );
}




void VCLXTopWindow::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXTopWindow::VCLXTopWindow()
{
}

VCLXTopWindow::~VCLXTopWindow()
{
}

// css::uno::XInterface
css::uno::Any VCLXTopWindow::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet( VCLXTopWindow_XBase::queryInterface( rType ) );

    if (!aRet.hasValue())
        aRet = VCLXTopWindow_XBase::queryInterface( rType );
    if ( !aRet.hasValue() )
        aRet = VCLXContainer::queryInterface( rType );

    return aRet;
}

css::uno::Sequence< sal_Int8 > VCLXTopWindow::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > VCLXTopWindow::getTypes()
{
    return ::comphelper::concatSequences( VCLXTopWindow_XBase::getTypes(), VCLXContainer::getTypes() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
