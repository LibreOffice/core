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

#include <toolkit/awt/vclxtabpagecontainer.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/tkresmgr.hxx>
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

void SAL_CALL VCLXTabPageContainer::draw( sal_Int32 nX, sal_Int32 nY ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    TabControl* pTabControl = (TabControl*)GetWindow();
    if ( pTabControl )
    {
        TabPage *pTabPage = pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >(  pTabControl->GetCurPageId( ) ) );
        if ( pTabPage )
        {
            ::Point aPos( nX, nY );
            ::Size  aSize = pTabPage->GetSizePixel();

            OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
            aPos  = pDev->PixelToLogic( aPos );
            aSize = pDev->PixelToLogic( aSize );

            pTabPage->Draw( pDev, aPos, aSize, 0 );
        }
    }

    VCLXWindow::draw( nX, nY );
}

::com::sun::star::awt::DeviceInfo VCLXTabPageContainer::getInfo() throw(RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();
    return aInfo;
}

void SAL_CALL VCLXTabPageContainer::setProperty(const ::rtl::OUString& PropertyName,   const Any& Value ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    TabControl* pTabPage = (TabControl*)GetWindow();
    if ( pTabPage )
    {
        VCLXWindow::setProperty( PropertyName, Value );
    }
}
::sal_Int16 SAL_CALL VCLXTabPageContainer::getActiveTabPageID() throw (RuntimeException)
{
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    return pTabCtrl != NULL ? pTabCtrl->GetCurPageId( ) : 0;
}
void SAL_CALL VCLXTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (RuntimeException)
{
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    if ( pTabCtrl )
        pTabCtrl->SelectTabPage(_activetabpageid);
}
::sal_Int16 SAL_CALL VCLXTabPageContainer::getTabPageCount(  ) throw (RuntimeException)
{
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    return pTabCtrl != NULL ? pTabCtrl->GetPageCount() : 0;
}
::sal_Bool SAL_CALL VCLXTabPageContainer::isTabPageActive( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    return (getActiveTabPageID() == tabPageIndex);
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPage( ::sal_Int16 tabPageIndex ) throw (RuntimeException)
{
    return (tabPageIndex >= 0 && tabPageIndex < static_cast<sal_Int16>(m_aTabPages.size())) ? m_aTabPages[tabPageIndex] : NULL;
}
Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPageByID( ::sal_Int16 tabPageID ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference< ::com::sun::star::awt::tab::XTabPage > xTabPage;
    ::std::vector< Reference< ::com::sun::star::awt::tab::XTabPage > >::iterator aIter = m_aTabPages.begin();
    ::std::vector< Reference< ::com::sun::star::awt::tab::XTabPage > >::iterator aEnd = m_aTabPages.end();
    for(;aIter != aEnd;++aIter)
    {
        Reference< awt::XControl > xControl(*aIter,UNO_QUERY );
        Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
        if ( tabPageID == xP->getTabPageID() )
        {
            xTabPage = *aIter;
            break;
        }
    }
    return xTabPage;
}
void SAL_CALL VCLXTabPageContainer::addTabPageContainerListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    m_aTabPageListeners.addInterface( listener );
}
void SAL_CALL VCLXTabPageContainer::removeTabPageContainerListener( const Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (RuntimeException)
{
    m_aTabPageListeners.removeInterface( listener );
}

void VCLXTabPageContainer::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    SolarMutexClearableGuard aGuard;
    TabControl* pTabControl = static_cast< TabControl* >( GetWindow() );
    if ( pTabControl )
    {
        switch ( _rVclWindowEvent.GetId() )
        {
            case VCLEVENT_TABPAGE_ACTIVATE:
            {
                sal_uLong page = (sal_uLong)_rVclWindowEvent.GetData();
                awt::tab::TabPageActivatedEvent aEvent(NULL,page);
                m_aTabPageListeners.tabPageActivated(aEvent);
                break;
            }
            default:
                aGuard.clear();
                VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
                break;
        }
    }
}
void SAL_CALL VCLXTabPageContainer::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}
void SAL_CALL VCLXTabPageContainer::elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    Reference< ::com::sun::star::awt::tab::XTabPage > xTabPage(Event.Element,uno::UNO_QUERY);
    if ( pTabCtrl && xTabPage.is() )
    {
        Reference< awt::XControl > xControl(xTabPage,UNO_QUERY );
        Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
        sal_Int16 nPageID = xP->getTabPageID();

        Window* pWindow = VCLUnoHelper::GetWindow(xControl->getPeer());
        TabPage* pPage = (TabPage*)pWindow;
        pTabCtrl->InsertPage(nPageID,pPage->GetText());

        pPage->Hide();
        pTabCtrl->SetTabPage(nPageID,pPage);
        pTabCtrl->SetHelpText(nPageID,xP->getToolTip());
        pTabCtrl->SetPageImage(nPageID,TkResMgr::getImageFromURL(xP->getImageURL()));
        pTabCtrl->SelectTabPage(nPageID);
        m_aTabPages.push_back(xTabPage);
    }
}
void SAL_CALL VCLXTabPageContainer::elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    TabControl* pTabCtrl = (TabControl*)GetWindow();
    Reference< ::com::sun::star::awt::tab::XTabPage > xTabPage(Event.Element,uno::UNO_QUERY);
    if ( pTabCtrl && xTabPage.is() )
    {
        Reference< awt::XControl > xControl(xTabPage,UNO_QUERY );
        Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
        pTabCtrl->RemovePage(xP->getTabPageID());
        m_aTabPages.erase(::std::remove(m_aTabPages.begin(),m_aTabPages.end(),xTabPage));
    }
}
void SAL_CALL VCLXTabPageContainer::elementReplaced( const ::com::sun::star::container::ContainerEvent& /*Event*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
