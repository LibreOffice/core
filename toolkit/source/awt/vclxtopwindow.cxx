/*************************************************************************
 *
 *  $RCSfile: vclxtopwindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-20 12:22:40 $
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

#ifndef _COM_SUN_STAR_LANG_SYSTEMDEPENDENT_HPP_
#include <com/sun/star/lang/SystemDependent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SYSTEMDEPENDENTXWINDOW_HPP_
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#endif

#ifndef UNX
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
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

::com::sun::star::uno::Any VCLXTopWindow::getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException)
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
#ifdef UNX
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW )
            {
                ::com::sun::star::awt::SystemDependentXWindow aSD;
                aSD.DisplayPointer = (sal_Int64)pSysData->pDisplay;
                aSD.WindowHandle = pSysData->aWindow;
                aRet <<= aSD;
            }
#elif (defined WNT)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32 )
            {
                 aRet <<= (sal_Int32)pSysData->hWnd;
            }
#elif (defined OS2)
            if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_OS2 )
            {
                 aRet <<= (sal_Int32)pSysData->hWnd;
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




