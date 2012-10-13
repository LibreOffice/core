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

#if defined UNX && ! defined QUARTZ && ! defined _COM_SUN_STAR_AWT_SYSTEMDEPENDENTXWINDOW_HPP_
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#endif

#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>

#ifdef QUARTZ
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#ifdef IOS
#include "premac.h"
#include <UIKit/UIKit.h>
#include "postmac.h"
#endif

#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

//  ----------------------------------------------------
//  class VCLXSystemDependentWindow
//  ----------------------------------------------------
VCLXSystemDependentWindow::VCLXSystemDependentWindow()
{
}

VCLXSystemDependentWindow::~VCLXSystemDependentWindow()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXSystemDependentWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XSystemDependentWindowPeer* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXSystemDependentWindow )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSystemDependentWindowPeer>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Any VCLXSystemDependentWindow::getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // TODO, check the process id
    ::com::sun::star::uno::Any aRet;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        const SystemEnvData* pSysData = ((SystemChildWindow *)pWindow)->GetSystemData();
        if( pSysData )
        {
#if (defined WNT)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32 )
            {
                 aRet <<= (sal_Int32)pSysData->hWnd;
            }
#elif (defined QUARTZ)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_MAC )
            {
                 aRet <<= (sal_IntPtr)pSysData->pView;
            }
#elif (defined IOS)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_IOS )
            {
                 aRet <<= (sal_IntPtr)pSysData->pView;
            }
#elif (defined UNX)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW )
            {
                ::com::sun::star::awt::SystemDependentXWindow aSD;
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
