/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxtopwindow.cxx,v $
 * $Revision: 1.13 $
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
#include <com/sun/star/awt/SystemDependentXWindow.hpp>

#if !defined(UNX) && !defined(OS2)
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#endif

#ifdef QUARTZ
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>

#include <tools/debug.hxx>

//  ----------------------------------------------------
//  class VCLXTopWindow
//  ----------------------------------------------------

void VCLXTopWindow::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXTopWindow::VCLXTopWindow(bool bWHWND)
    : m_bWHWND(bWHWND)
{
}

VCLXTopWindow::~VCLXTopWindow()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTopWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet;
    if(! m_bWHWND) {
        aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( ::com::sun::star::awt::XTopWindow*, this ) );
    }
    else {
        aRet = ::cppu::queryInterface( rType,
                                       SAL_STATIC_CAST( ::com::sun::star::awt::XTopWindow*, this ),
                                       SAL_STATIC_CAST( ::com::sun::star::awt::XSystemDependentWindowPeer*, this ) );
    }
    return (aRet.hasValue() ? aRet : VCLXContainer::queryInterface( rType ));
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXTopWindow::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OImplementationId* pId = NULL;
    static ::cppu::OImplementationId* pIdWithHandle = NULL;
    if ( m_bWHWND )
    {
        if( !pIdWithHandle )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pIdWithHandle )
            {
                static ::cppu::OImplementationId idWithHandle( sal_False );
                pIdWithHandle = &idWithHandle;
            }
        }

        return (*pIdWithHandle).getImplementationId();
    }
    else
    {
        if( !pId )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pId )
            {
                static ::cppu::OImplementationId id( sal_False );
                pId = &id;
            }
        }

        return (*pId).getImplementationId();
    }
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > VCLXTopWindow::getTypes() throw(::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OTypeCollection* pCollection = NULL;
    static ::cppu::OTypeCollection* pCollectionWithHandle = NULL;

    if ( m_bWHWND )
    {
        if( !pCollectionWithHandle )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollectionWithHandle )
            {
                static ::cppu::OTypeCollection collectionWithHandle(
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSystemDependentWindowPeer>* ) NULL ),
                VCLXContainer::getTypes() );
                pCollectionWithHandle = &collectionWithHandle;
            }
        }

        return (*pCollectionWithHandle).getTypes();
    }
    else
    {
        if( !pCollection )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pCollection )
            {
                static ::cppu::OTypeCollection collection(
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider>* ) NULL ),
                getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow>* ) NULL ),
                VCLXContainer::getTypes() );
                pCollection = &collection;
            }
        }
        return (*pCollection).getTypes();
    }
}

::com::sun::star::uno::Any VCLXTopWindow::getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    // TODO, check the process id
    ::com::sun::star::uno::Any aRet;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        const SystemEnvData* pSysData = ((SystemWindow *)pWindow)->GetSystemData();
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


void VCLXTopWindow::addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetTopWindowListeners().addInterface( rxListener );
}

void VCLXTopWindow::removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    GetTopWindowListeners().removeInterface( rxListener );
}

void VCLXTopWindow::toFront(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        ((WorkWindow*)pWindow)->ToTop( TOTOP_RESTOREWHENMIN );
}

void VCLXTopWindow::toBack(  ) throw(::com::sun::star::uno::RuntimeException)
{
/* Not possible in VCL...

    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        ((WorkWindow*)pWindow)->ToBack();
    }
*/
}

void VCLXTopWindow::setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& rxMenu ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    SystemWindow* pWindow = (SystemWindow*) GetWindow();
    if ( pWindow )
    {
        pWindow->SetMenuBar( NULL );
        if ( rxMenu.is() )
        {
            VCLXMenu* pMenu = VCLXMenu::GetImplementation( rxMenu );
            if ( pMenu && !pMenu->IsPopupMenu() )
                pWindow->SetMenuBar( (MenuBar*) pMenu->GetMenu() );
        }
    }
    mxMenuBar = rxMenu;
}




