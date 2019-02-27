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

#include <toolkit/awt/vclxtabpagecontainer.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <sal/log.hxx>
#include <vcl/image.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <helper/tkresmgr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::view;

//  class VCLXTabPageContainer

void VCLXTabPageContainer::GetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    VCLXWindow::ImplGetPropertyIds( rIds );
}

VCLXTabPageContainer::VCLXTabPageContainer() :
    m_aTabPageListeners( *this )
{
}

VCLXTabPageContainer::~VCLXTabPageContainer()
{
    SAL_INFO("toolkit", __FUNCTION__);
}

void SAL_CALL VCLXTabPageContainer::draw( sal_Int32 nX, sal_Int32 nY )
{
    SolarMutexGuard aGuard;
    VclPtr<TabControl> pTabControl = GetAs<TabControl>();
    if ( pTabControl )
    {
        TabPage *pTabPage = pTabControl->GetTabPage( sal::static_int_cast< sal_uInt16 >(  pTabControl->GetCurPageId( ) ) );
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if (pTabPage && pDev)
        {
            ::Point aPos( nX, nY );
            ::Size  aSize = pTabPage->GetSizePixel();

            aPos  = pDev->PixelToLogic( aPos );
            aSize = pDev->PixelToLogic( aSize );

            pTabPage->Draw( pDev, aPos, aSize, DrawFlags::NONE );
        }
    }

    VCLXWindow::draw( nX, nY );
}

void SAL_CALL VCLXTabPageContainer::setProperty(const OUString& PropertyName,   const Any& Value )
{
    SolarMutexGuard aGuard;
    VclPtr<TabControl> pTabPage = GetAs<TabControl>();
    if ( pTabPage )
        VCLXWindow::setProperty( PropertyName, Value );
}

::sal_Int16 SAL_CALL VCLXTabPageContainer::getActiveTabPageID()
{
    VclPtr<TabControl> pTabCtrl = GetAs<TabControl>();
    return pTabCtrl ? pTabCtrl->GetCurPageId( ) : 0;
}

void SAL_CALL VCLXTabPageContainer::setActiveTabPageID( ::sal_Int16 _activetabpageid )
{
    VclPtr<TabControl> pTabCtrl = GetAs<TabControl>();
    if ( pTabCtrl )
        pTabCtrl->SelectTabPage(_activetabpageid);
}

::sal_Int16 SAL_CALL VCLXTabPageContainer::getTabPageCount(  )
{
    VclPtr<TabControl> pTabCtrl = GetAs<TabControl>();
    return pTabCtrl ? pTabCtrl->GetPageCount() : 0;
}

sal_Bool SAL_CALL VCLXTabPageContainer::isTabPageActive( ::sal_Int16 tabPageIndex )
{
    return (getActiveTabPageID() == tabPageIndex);
}

Reference< css::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPage( ::sal_Int16 tabPageIndex )
{
    return (tabPageIndex >= 0 && tabPageIndex < static_cast<sal_Int16>(m_aTabPages.size())) ? m_aTabPages[tabPageIndex] : nullptr;
}

Reference< css::awt::tab::XTabPage > SAL_CALL VCLXTabPageContainer::getTabPageByID( ::sal_Int16 tabPageID )
{
    SolarMutexGuard aGuard;
    Reference< css::awt::tab::XTabPage > xTabPage;
    for(const auto& rTabPage : m_aTabPages)
    {
        Reference< awt::XControl > xControl(rTabPage,UNO_QUERY );
        Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
        if ( tabPageID == xP->getTabPageID() )
        {
            xTabPage = rTabPage;
            break;
        }
    }
    return xTabPage;
}

void SAL_CALL VCLXTabPageContainer::addTabPageContainerListener( const Reference< css::awt::tab::XTabPageContainerListener >& listener )
{
    m_aTabPageListeners.addInterface( listener );
}

void SAL_CALL VCLXTabPageContainer::removeTabPageContainerListener( const Reference< css::awt::tab::XTabPageContainerListener >& listener )
{
    m_aTabPageListeners.removeInterface( listener );
}

void VCLXTabPageContainer::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    SolarMutexClearableGuard aGuard;
    VclPtr<TabControl> pTabControl = GetAs<TabControl>();
    if ( pTabControl )
    {
        switch ( _rVclWindowEvent.GetId() )
        {
            case VclEventId::TabpageActivate:
            {
                sal_uLong page = reinterpret_cast<sal_uLong>(_rVclWindowEvent.GetData());
                awt::tab::TabPageActivatedEvent aEvent(nullptr,page);
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
void SAL_CALL VCLXTabPageContainer::disposing( const css::lang::EventObject& /*Source*/ )
{
}
void SAL_CALL VCLXTabPageContainer::elementInserted( const css::container::ContainerEvent& Event )
{
    SolarMutexGuard aGuard;
    VclPtr<TabControl> pTabCtrl = GetAs<TabControl>();
    Reference< css::awt::tab::XTabPage > xTabPage(Event.Element,uno::UNO_QUERY);
    if ( !pTabCtrl || !xTabPage.is() )
        return;

    Reference< awt::XControl > xControl(xTabPage,UNO_QUERY );
    Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
    sal_Int16 nPageID = xP->getTabPageID();

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xControl->getPeer());
    TabPage* pPage = static_cast<TabPage*>(pWindow.get());
    pTabCtrl->InsertPage(nPageID,pPage->GetText());

    pPage->Hide();
    pTabCtrl->SetTabPage(nPageID,pPage);
    pTabCtrl->SetHelpText(nPageID,xP->getToolTip());
    pTabCtrl->SetPageImage(nPageID,TkResMgr::getImageFromURL(xP->getImageURL()));
    pTabCtrl->SelectTabPage(nPageID);
    pTabCtrl->EnablePage(nPageID,xP->getEnabled());
    m_aTabPages.push_back(xTabPage);

}
void SAL_CALL VCLXTabPageContainer::elementRemoved( const css::container::ContainerEvent& Event )
{
    SolarMutexGuard aGuard;
    VclPtr<TabControl> pTabCtrl = GetAs<TabControl>();
    Reference< css::awt::tab::XTabPage > xTabPage(Event.Element,uno::UNO_QUERY);
    if ( pTabCtrl && xTabPage.is() )
    {
        Reference< awt::XControl > xControl(xTabPage,UNO_QUERY );
        Reference< awt::tab::XTabPageModel > xP( xControl->getModel(), UNO_QUERY );
        pTabCtrl->RemovePage(xP->getTabPageID());
        m_aTabPages.erase(::std::remove(m_aTabPages.begin(),m_aTabPages.end(),xTabPage));
    }
}
void SAL_CALL VCLXTabPageContainer::elementReplaced( const css::container::ContainerEvent& /*Event*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
