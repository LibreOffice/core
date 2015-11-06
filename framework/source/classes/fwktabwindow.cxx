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

// autogen include statement, do not remove

#include <classes/fwktabwindow.hxx>
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/ContainerWindowProvider.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>
#include <vcl/msgbox.hxx>

const char EXTERNAL_EVENT[] = "external_event";
const char INITIALIZE_METHOD[] = "initialize";

using namespace ::com::sun::star;

namespace framework
{

// class FwkTabControl ---------------------------------------------------
FwkTabControl::FwkTabControl(vcl::Window* pParent)
    : TabControl(pParent)
{
}

void FwkTabControl::BroadcastEvent( sal_uLong nEvent )
{
    if ( VCLEVENT_TABPAGE_ACTIVATE == nEvent || VCLEVENT_TABPAGE_DEACTIVATE == nEvent )
        CallEventListeners( nEvent, reinterpret_cast<void*>(GetCurPageId()) );
    else
    {
        SAL_WARN( "fwk", "FwkTabControl::BroadcastEvent(): illegal event" );
    }
}

// class FwkTabPage ------------------------------------------------

FwkTabPage::FwkTabPage(
               vcl::Window* pParent, const OUString& rPageURL,
               const css::uno::Reference< css::awt::XContainerWindowEventHandler >& rEventHdl,
               const css::uno::Reference< css::awt::XContainerWindowProvider >& rProvider ) :

    TabPage( pParent, WB_DIALOGCONTROL | WB_TABSTOP | WB_CHILDDLGCTRL ),

    m_sPageURL          ( rPageURL ),
    m_xEventHdl         ( rEventHdl ),
    m_xWinProvider      ( rProvider )

{
}

FwkTabPage::~FwkTabPage()
{
    disposeOnce();
}

void FwkTabPage::dispose()
{
    Hide();
    DeactivatePage();
    TabPage::dispose();
}

void FwkTabPage::CreateDialog()
{
    try
    {
        uno::Reference< uno::XInterface > xHandler;
        if ( m_xEventHdl.is() )
      xHandler = m_xEventHdl;

        uno::Reference< awt::XWindowPeer > xParent( VCLUnoHelper::GetInterface( this ), uno::UNO_QUERY );
        m_xPage.set( m_xWinProvider->createContainerWindow( m_sPageURL, OUString(), xParent, xHandler ),
                     uno::UNO_QUERY );

        uno::Reference< awt::XControl > xPageControl( m_xPage, uno::UNO_QUERY );
        if ( xPageControl.is() )
        {
            uno::Reference< awt::XWindowPeer > xWinPeer( xPageControl->getPeer() );
            if ( xWinPeer.is() )
            {
                vcl::Window* pWindow = VCLUnoHelper::GetWindow( xWinPeer );
                if ( pWindow )
                    pWindow->SetStyle( pWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
            }
        }

        CallMethod( INITIALIZE_METHOD );
    }
    catch ( const lang::IllegalArgumentException& )
    {
        SAL_WARN( "fwk", "FwkTabPage::CreateDialog(): illegal argument" );
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "fwk", "FwkTabPage::CreateDialog(): exception of XDialogProvider2::createContainerWindow()" );
    }
}

bool FwkTabPage::CallMethod( const OUString& rMethod )
{
    bool bRet = false;
    if ( m_xEventHdl.is() )
    {
        try
        {
            bRet = m_xEventHdl->callHandlerMethod( m_xPage, uno::makeAny( rMethod ), EXTERNAL_EVENT );
        }
        catch ( const uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return bRet;
}

void FwkTabPage::ActivatePage()
{
    TabPage::ActivatePage();

    if ( !m_xPage.is() )
        CreateDialog();

    if ( m_xPage.is() )
    {
        Resize ();
        m_xPage->setVisible( sal_True );
    }
}

void FwkTabPage::DeactivatePage()
{
    TabPage::DeactivatePage();

    if ( m_xPage.is() )
        m_xPage->setVisible( sal_False );
}

void FwkTabPage::Resize()
{
    if ( m_xPage.is () )
    {
        Size aSize = GetSizePixel();

        m_xPage->setPosSize( 0, 0, aSize.Width()-1 , aSize.Height()-1, awt::PosSize::POSSIZE );
    }
}

// class FwkTabWindow ---------------------------------------------
FwkTabWindow::FwkTabWindow( vcl::Window* pParent )
    : Window(pParent)
    , m_aTabCtrl(VclPtr<FwkTabControl>::Create(this))
{
    m_xWinProvider = awt::ContainerWindowProvider::create( ::comphelper::getProcessComponentContext() );

    SetPaintTransparent(true);

    m_aTabCtrl->SetActivatePageHdl( LINK( this, FwkTabWindow, ActivatePageHdl ) );
    m_aTabCtrl->SetDeactivatePageHdl( LINK( this, FwkTabWindow, DeactivatePageHdl ) );
    m_aTabCtrl->Show();
}

FwkTabWindow::~FwkTabWindow()
{
    disposeOnce();
}

void FwkTabWindow::dispose()
{
    ClearEntryList();
    m_aTabCtrl.disposeAndClear();
    vcl::Window::dispose();
}

void FwkTabWindow::ClearEntryList()
{
    TabEntryList::const_iterator pIt;
    for (  pIt  = m_TabList.begin();
           pIt != m_TabList.end();
         ++pIt )
    {
        delete *pIt;
    }

    m_TabList.clear();
}

bool FwkTabWindow::RemoveEntry( sal_Int32 nIndex )
{
    TabEntryList::iterator pIt;
    for (  pIt  = m_TabList.begin();
           pIt != m_TabList.end();
         ++pIt )
    {
        if ( (*pIt)->m_nIndex == nIndex )
            break;
    }

    // remove entry from vector
    if ( pIt != m_TabList.end())
    {
        m_TabList.erase(pIt);
        return true;
    }
    else
        return false;
}

TabEntry* FwkTabWindow::FindEntry( sal_Int32 nIndex ) const
{
    TabEntry* pEntry = NULL;

    TabEntryList::const_iterator pIt;
    for (  pIt  = m_TabList.begin();
           pIt != m_TabList.end();
         ++pIt )
    {
        if ( (*pIt)->m_nIndex == nIndex )
        {
            pEntry = *pIt;
            break;
        }
    }

    return pEntry;
}

IMPL_LINK_NOARG_TYPED(FwkTabWindow, ActivatePageHdl, TabControl*, void)
{
    const sal_uInt16 nId = m_aTabCtrl->GetCurPageId();
    FwkTabPage* pTabPage = static_cast< FwkTabPage* >( m_aTabCtrl->GetTabPage( nId ) );
    if ( !pTabPage )
    {
        TabEntry* pEntry = FindEntry( nId );
        if ( pEntry )
        {
            pTabPage = VclPtr<FwkTabPage>::Create( m_aTabCtrl.get(), pEntry->m_sPageURL, pEntry->m_xEventHdl, m_xWinProvider );
            pEntry->m_pPage = pTabPage;
            m_aTabCtrl->SetTabPage( nId, pTabPage );
            pTabPage->Show();
            pTabPage->ActivatePage();
        }
    } else {
        pTabPage->ActivatePage();
    }
    m_aTabCtrl->BroadcastEvent( VCLEVENT_TABPAGE_ACTIVATE );
}

IMPL_LINK_NOARG_TYPED(FwkTabWindow, DeactivatePageHdl, TabControl *, bool)
{
    m_aTabCtrl->BroadcastEvent( VCLEVENT_TABPAGE_DEACTIVATE );
    return true;
}

void FwkTabWindow::AddEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    m_aTabCtrl->AddEventListener( rEventListener );
}

void FwkTabWindow::RemoveEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    m_aTabCtrl->RemoveEventListener( rEventListener );
}

FwkTabPage* FwkTabWindow::AddTabPage( sal_Int32 nIndex, const uno::Sequence< beans::NamedValue >& rProperties )
{
    OUString sTitle, sToolTip, sPageURL;
    uno::Reference< css::awt::XContainerWindowEventHandler > xEventHdl;
    uno::Reference< graphic::XGraphic > xImage;
    bool bDisabled = false;

    sal_Int32 i = 0, nLen = rProperties.getLength();
    for ( i = 0; i < nLen; ++i )
    {
        beans::NamedValue aValue = rProperties[i];
        OUString sName = aValue.Name;

        if ( sName == "Title" )
            aValue.Value >>= sTitle;
        else if ( sName == "ToolTip" )
            aValue.Value >>= sToolTip;
        else if ( sName == "PageURL" )
            aValue.Value >>= sPageURL;
        else if ( sName == "EventHdl" )
            aValue.Value >>= xEventHdl;
        else if ( sName == "Image" )
            aValue.Value >>= xImage;
        else if ( sName == "Disabled" )
            aValue.Value >>= bDisabled;
    }

    TabEntry* pEntry = new TabEntry( nIndex, sPageURL, xEventHdl );
    m_TabList.push_back( pEntry );
    sal_uInt16 nIdx = static_cast< sal_uInt16 >( nIndex );
    m_aTabCtrl->InsertPage( nIdx, sTitle );
    if ( !sToolTip.isEmpty() )
        m_aTabCtrl->SetHelpText( nIdx, sToolTip );
    if ( xImage.is() )
        m_aTabCtrl->SetPageImage( nIdx, Image( xImage ) );
    if ( bDisabled )
        m_aTabCtrl->EnablePage( nIdx, false );

    return pEntry->m_pPage;
}

void FwkTabWindow::ActivatePage( sal_Int32 nIndex )
{
    m_aTabCtrl->SetCurPageId( static_cast< sal_uInt16 >( nIndex ) );
    ActivatePageHdl( m_aTabCtrl.get() );
}

void FwkTabWindow::RemovePage( sal_Int32 nIndex )
{
    TabEntry* pEntry = FindEntry(nIndex);
    if ( pEntry )
    {
        m_aTabCtrl->RemovePage( static_cast< sal_uInt16 >( nIndex ) );
        if (RemoveEntry(nIndex))
            delete pEntry;
    }
}

void FwkTabWindow::Resize()
{
    Size aPageSize = GetSizePixel();
    m_aTabCtrl->SetTabPageSizePixel( aPageSize );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
