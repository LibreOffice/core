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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"


#include <com/sun/star/lang/SystemDependent.hpp>

#if defined UNX && ! defined QUARTZ && ! defined _COM_SUN_STAR_AWT_SYSTEMDEPENDENTXWINDOW_HPP_
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#endif

#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/helper/macros.hxx>
#include <cppuhelper/typeprovider.hxx>

#ifdef WNT
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#elif defined ( QUARTZ )
#include "premac.h"
#include <Cocoa/Cocoa.h>
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
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XSystemDependentWindowPeer*, this ) );
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
#elif (defined OS2)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_OS2 )
            {
                 aRet <<= (sal_Int32)pSysData->hWnd;
            }
#elif (defined QUARTZ)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_MAC )
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
