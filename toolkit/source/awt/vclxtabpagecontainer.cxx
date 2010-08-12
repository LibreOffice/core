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
#include "precompiled_toolkit.hxx"

#include <toolkit/awt/vclxtabpagecontainer.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;
//  ----------------------------------------------------
//  class VCLXTabPageContainer
//  ----------------------------------------------------
void VCLXTabPageContainer::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXTabPageContainer::VCLXTabPageContainer() :
    m_aTabPageListeners( *this )
{
}

VCLXTabPageContainer::~VCLXTabPageContainer()
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s", __FUNCTION__);
#endif
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTabPageContainer::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                            SAL_STATIC_CAST( ::com::sun::star::awt::tab::XTabPageContainer*, this ) );
                            //SAL_STATIC_CAST( ::com::sun::star::awt::tab::XTabPageModel*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTabPageContainer )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainer>* ) NULL ),
    //getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageModel>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void SAL_CALL VCLXTabPageContainer::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

::com::sun::star::awt::DeviceInfo VCLXTabPageContainer::getInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}


void SAL_CALL VCLXTabPageContainer::setProperty(
    const ::rtl::OUString& PropertyName,
    const ::com::sun::star::uno::Any& Value )
throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TabPage* pTabPage = (TabPage*)GetWindow();
    if ( pTabPage )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;
        VCLXWindow::setProperty( PropertyName, Value );
    }
}
::sal_Int16 SAL_CALL VCLXTabPageContainer::getActiveTabPageID() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nActiveTabPageId;
}
void SAL_CALL VCLXTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (::com::sun::star::uno::RuntimeException)
{
    m_nActiveTabPageId = _activetabpageid;
}
::sal_Int32 SAL_CALL VCLXTabPageContainer::getTabPageCount(  ) throw (::com::sun::star::uno::RuntimeException)
{
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    if ( pTabCtrl )
        return pTabCtrl -> GetPageCount();
    else
        return 0;
}
::sal_Bool SAL_CALL VCLXTabPageContainer::isTabPageActive( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException)
{
    if(m_nActiveTabPageId == tabPageIndex)
        return true;
    else
        return false;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPage( ::sal_Int16 /*tabPageIndex*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPageByID( ::sal_Int16 tabPageID ) throw (::com::sun::star::uno::RuntimeException)
{
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    if ( pTabCtrl )
    {
        TabPage* pTabPage = pTabCtrl->GetTabPage(tabPageID);
    }
    //methode um aus tabpage ein XTabpage zuerzeugen fehlt - Xcontainer methoden vielleicht nutzen
    //da er mit Controls arbeitet und XTabPage ist Control
    return NULL;
}
void SAL_CALL VCLXTabPageContainer::addTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aTabPageListeners.addInterface( listener );
}
void SAL_CALL VCLXTabPageContainer::removeTabPageListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException)
{
    m_aTabPageListeners.removeInterface( listener );
}

//// ::com::sun::star::awt::XControlContainer
void SAL_CALL VCLXTabPageContainer::setStatusText( const ::rtl::OUString& StatusText ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)StatusText;
}
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > > SAL_CALL VCLXTabPageContainer::getControls(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}
uno::Reference< awt::XControl > VCLXTabPageContainer::getControl( const ::rtl::OUString& rName ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    (void)rName;
    return uno::Reference< awt::XControl >();
}

void SAL_CALL VCLXTabPageContainer::addControl( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)Name;
    (void)Control;
}
void SAL_CALL VCLXTabPageContainer::removeControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)Control;
}
