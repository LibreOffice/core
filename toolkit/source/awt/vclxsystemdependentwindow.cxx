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

#include <com/sun/star/lang/SystemDependent.hpp>

#if defined UNX && ! defined MACOSX
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#endif

#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#ifdef MACOSX
#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#endif

#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>


//  class VCLXSystemDependentWindow

VCLXSystemDependentWindow::VCLXSystemDependentWindow()
{
}

VCLXSystemDependentWindow::~VCLXSystemDependentWindow()
{
}

// css::uno::XInterface
css::uno::Any VCLXSystemDependentWindow::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XSystemDependentWindowPeer* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXSystemDependentWindow )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXSystemDependentWindow::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XSystemDependentWindowPeer>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

css::uno::Any VCLXSystemDependentWindow::getWindowHandle( const css::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType )
{
    SolarMutexGuard aGuard;

    // TODO, check the process id
    css::uno::Any aRet;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        const SystemEnvData* pSysData = static_cast<SystemChildWindow *>(pWindow.get())->GetSystemData();
        if( pSysData )
        {
#if defined(_WIN32)
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
                aSD.WindowHandle = pSysData->aWindow;
                aRet <<= aSD;
            }
#endif
        }
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
