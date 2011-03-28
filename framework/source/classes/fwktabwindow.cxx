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

// autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <classes/fwktabwindow.hxx>
#include "framework.hrc"
#include <classes/fwkresid.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XContainerWindowEventHandler.hpp>
#include <com/sun/star/awt/XContainerWindowProvider.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/stream.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>
#include <vcl/msgbox.hxx>

const ::rtl::OUString SERVICENAME_WINPROVIDER
                                        = ::rtl::OUString::createFromAscii("com.sun.star.awt.ContainerWindowProvider");
const ::rtl::OUString EXTERNAL_EVENT    = ::rtl::OUString::createFromAscii("external_event");
const ::rtl::OUString BACK_METHOD       = ::rtl::OUString::createFromAscii("back");
const ::rtl::OUString INITIALIZE_METHOD = ::rtl::OUString::createFromAscii("initialize");
const ::rtl::OUString OK_METHOD         = ::rtl::OUString::createFromAscii("ok");

using namespace ::com::sun::star;

namespace framework
{

// class FwkTabControl ---------------------------------------------------
FwkTabControl::FwkTabControl( Window* pParent, const ResId& rResId ) :

    TabControl( pParent, rResId )
{
}

// -----------------------------------------------------------------------

void FwkTabControl::BroadcastEvent( sal_uLong nEvent )
{
    if ( VCLEVENT_TABPAGE_ACTIVATE == nEvent || VCLEVENT_TABPAGE_DEACTIVATE == nEvent )
        ImplCallEventListeners( nEvent, (void*)(sal_uIntPtr)GetCurPageId() );
    else
    {
        DBG_ERRORFILE( "FwkTabControl::BroadcastEvent(): illegal event" );
    }
}

// class FwkTabPage ------------------------------------------------

FwkTabPage::FwkTabPage(
               Window* pParent, const rtl::OUString& rPageURL,
               const css::uno::Reference< css::awt::XContainerWindowEventHandler >& rEventHdl,
               const css::uno::Reference< css::awt::XContainerWindowProvider >& rProvider ) :

    TabPage( pParent, WB_DIALOGCONTROL | WB_TABSTOP | WB_CHILDDLGCTRL ),

    m_sPageURL          ( rPageURL ),
    m_xEventHdl         ( rEventHdl ),
    m_xWinProvider      ( rProvider )

{
}

// -----------------------------------------------------------------------

FwkTabPage::~FwkTabPage()
{
    Hide();
    DeactivatePage();
}

// -----------------------------------------------------------------------

void FwkTabPage::CreateDialog()
{
    try
    {
        uno::Reference< uno::XInterface > xHandler;
        if ( m_xEventHdl.is() )
      xHandler = m_xEventHdl;

        uno::Reference< awt::XWindowPeer > xParent( VCLUnoHelper::GetInterface( this ), uno::UNO_QUERY );
        m_xPage = uno::Reference < awt::XWindow >(
            m_xWinProvider->createContainerWindow(
                m_sPageURL, rtl::OUString(), xParent, xHandler ), uno::UNO_QUERY );

        uno::Reference< awt::XControl > xPageControl( m_xPage, uno::UNO_QUERY );
        if ( xPageControl.is() )
        {
            uno::Reference< awt::XWindowPeer > xWinPeer( xPageControl->getPeer() );
            if ( xWinPeer.is() )
            {
                Window* pWindow = VCLUnoHelper::GetWindow( xWinPeer );
                if ( pWindow )
                    pWindow->SetStyle( pWindow->GetStyle() | WB_DIALOGCONTROL | WB_CHILDDLGCTRL );
            }
        }

        CallMethod( INITIALIZE_METHOD );
    }
    catch ( lang::IllegalArgumentException& )
    {
        DBG_ERRORFILE( "FwkTabPage::CreateDialog(): illegal argument" );
    }
    catch ( uno::Exception& )
    {
        DBG_ERRORFILE( "FwkTabPage::CreateDialog(): exception of XDialogProvider2::createContainerWindow()" );
    }
}

// -----------------------------------------------------------------------

sal_Bool FwkTabPage::CallMethod( const rtl::OUString& rMethod )
{
    sal_Bool bRet = sal_False;
    if ( m_xEventHdl.is() )
    {
        try
        {
            bRet = m_xEventHdl->callHandlerMethod( m_xPage, uno::makeAny( rMethod ), EXTERNAL_EVENT );
        }
        catch ( uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void FwkTabPage::DeactivatePage()
{
    TabPage::DeactivatePage();

    if ( m_xPage.is() )
        m_xPage->setVisible( sal_False );
}

// -----------------------------------------------------------------------

void FwkTabPage::Resize()
{
    if ( m_xPage.is () )
    {
        Size  aSize = GetSizePixel ();
        Point aPos  = GetPosPixel  ();

        m_xPage->setPosSize( 0, 0, aSize.Width()-1 , aSize.Height()-1, awt::PosSize::POSSIZE );
    }
}

// class FwkTabWindow ---------------------------------------------

FwkTabWindow::FwkTabWindow( Window* pParent ) :

    Window( pParent, FwkResId( WIN_TABWINDOW ) ),

    m_aTabCtrl  ( this, FwkResId( TC_TABCONTROL ) )
{
    uno::Reference < lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
    m_xWinProvider = uno::Reference < awt::XContainerWindowProvider >(
        xFactory->createInstance( SERVICENAME_WINPROVIDER ), uno::UNO_QUERY );

    SetPaintTransparent(true);

    m_aTabCtrl.SetActivatePageHdl( LINK( this, FwkTabWindow, ActivatePageHdl ) );
    m_aTabCtrl.SetDeactivatePageHdl( LINK( this, FwkTabWindow, DeactivatePageHdl ) );
    m_aTabCtrl.Show();
}

// -----------------------------------------------------------------------

FwkTabWindow::~FwkTabWindow()
{
    ClearEntryList();
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------

IMPL_LINK( FwkTabWindow, ActivatePageHdl, TabControl *, EMPTYARG )
{
    const sal_uInt16 nId = m_aTabCtrl.GetCurPageId();
    FwkTabPage* pTabPage = static_cast< FwkTabPage* >( m_aTabCtrl.GetTabPage( nId ) );
    if ( !pTabPage )
    {
        TabEntry* pEntry = FindEntry( nId );
        if ( pEntry )
        {
            pTabPage = new FwkTabPage( &m_aTabCtrl, pEntry->m_sPageURL, pEntry->m_xEventHdl, m_xWinProvider );
            pEntry->m_pPage = pTabPage;
            m_aTabCtrl.SetTabPage( nId, pTabPage );
            pTabPage->Show();
            pTabPage->ActivatePage();
        }
    } else {
        pTabPage->ActivatePage();
    }
    m_aTabCtrl.BroadcastEvent( VCLEVENT_TABPAGE_ACTIVATE );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( FwkTabWindow, DeactivatePageHdl, TabControl *, EMPTYARG )
{
    m_aTabCtrl.BroadcastEvent( VCLEVENT_TABPAGE_DEACTIVATE );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( FwkTabWindow, CloseHdl, PushButton *, EMPTYARG )
{
//    Close();
    return 0;
}

// -----------------------------------------------------------------------

void FwkTabWindow::AddEventListener( const Link& rEventListener )
{
    m_aTabCtrl.AddEventListener( rEventListener );
}

void FwkTabWindow::RemoveEventListener( const Link& rEventListener )
{
    m_aTabCtrl.RemoveEventListener( rEventListener );
}

// -----------------------------------------------------------------------

FwkTabPage* FwkTabWindow::AddTabPage( sal_Int32 nIndex, const uno::Sequence< beans::NamedValue >& rProperties )
{
    ::rtl::OUString sTitle, sToolTip, sPageURL;
    uno::Reference< css::awt::XContainerWindowEventHandler > xEventHdl;
    uno::Reference< graphic::XGraphic > xImage;
    bool bDisabled = false;

    sal_Int32 i = 0, nLen = rProperties.getLength();
    for ( i = 0; i < nLen; ++i )
    {
        beans::NamedValue aValue = rProperties[i];
        ::rtl::OUString sName = aValue.Name;

        if ( sName.equalsAscii("Title") )
            aValue.Value >>= sTitle;
        else if ( sName.equalsAscii("ToolTip") )
            aValue.Value >>= sToolTip;
        else if ( sName.equalsAscii("PageURL") )
            aValue.Value >>= sPageURL;
        else if ( sName.equalsAscii("EventHdl") )
            aValue.Value >>= xEventHdl;
        else if ( sName.equalsAscii("Image") )
            aValue.Value >>= xImage;
        else if ( sName.equalsAscii("Disabled") )
            aValue.Value >>= bDisabled;
    }

    TabEntry* pEntry = new TabEntry( nIndex, sPageURL, xEventHdl );
    m_TabList.push_back( pEntry );
    sal_uInt16 nIdx = static_cast< sal_uInt16 >( nIndex );
    m_aTabCtrl.InsertPage( nIdx, sTitle );
    if ( sToolTip.getLength() > 0 )
        m_aTabCtrl.SetHelpText( nIdx, sToolTip );
    if ( xImage.is() )
        m_aTabCtrl.SetPageImage( nIdx, Image( xImage ) );
    if ( bDisabled )
        m_aTabCtrl.EnablePage( nIdx, false );

    return pEntry->m_pPage;
}

// -----------------------------------------------------------------------

void FwkTabWindow::ActivatePage( sal_Int32 nIndex )
{
    m_aTabCtrl.SetCurPageId( static_cast< sal_uInt16 >( nIndex ) );
    ActivatePageHdl( &m_aTabCtrl );
}

// -----------------------------------------------------------------------

void FwkTabWindow::RemovePage( sal_Int32 nIndex )
{
    TabEntry* pEntry = FindEntry(nIndex);
    if ( pEntry )
    {
        m_aTabCtrl.RemovePage( static_cast< sal_uInt16 >( nIndex ) );
        if (RemoveEntry(nIndex))
            delete pEntry;
    }
}

// -----------------------------------------------------------------------
void FwkTabWindow::Resize()
{
    Size aPageSize = GetSizePixel();
    m_aTabCtrl.SetTabPageSizePixel( aPageSize );
}

} // namespace framework

