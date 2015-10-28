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


#include "impframe.hxx"
#include "objshimp.hxx"
#include <sfx2/sfxhelp.hxx>
#include "workwin.hxx"

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/event.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svl/rectitem.hxx>
#include <svl/stritem.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/syswin.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::frame::XComponentLoader;



class SfxFrameWindow_Impl : public vcl::Window
{
public:
    SfxFrame*           pFrame;

    SfxFrameWindow_Impl( SfxFrame* pF, vcl::Window& i_rContainerWindow );

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool        Notify( NotifyEvent& rEvt ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    void                DoResize();
};

SfxFrameWindow_Impl::SfxFrameWindow_Impl( SfxFrame* pF, vcl::Window& i_rContainerWindow )
        : Window( &i_rContainerWindow, WB_BORDER | WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_3DLOOK )
        , pFrame( pF )
{
}

void SfxFrameWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    SfxWorkWindow *pWorkWin = pFrame->GetWorkWindow_Impl();
    if ( pWorkWin )
        pWorkWin->DataChanged_Impl( rDCEvt );
}

bool SfxFrameWindow_Impl::Notify( NotifyEvent& rNEvt )
{
    if ( pFrame->IsClosing_Impl() || !pFrame->GetFrameInterface().is() )
        return false;

    SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
    if ( !pView || !pView->GetObjectShell() )
        return Window::Notify( rNEvt );

    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
    {
        if ( pView->GetViewShell() && !pView->GetViewShell()->GetUIActiveIPClient_Impl() && !pFrame->IsInPlace() )
        {
            OSL_TRACE("SfxFrame: GotFocus");
            pView->MakeActive_Impl( false );
        }

        // if focus was on an external window, the clipboard content might have been changed
        pView->GetBindings().Invalidate( SID_PASTE );
        pView->GetBindings().Invalidate( SID_PASTE_SPECIAL );
        return true;
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        if ( pView->GetViewShell()->KeyInput( *rNEvt.GetKeyEvent() ) )
            return true;
    }
    else if ( rNEvt.GetType() == MouseNotifyEvent::EXECUTEDIALOG /*|| rNEvt.GetType() == MouseNotifyEvent::INPUTDISABLE*/ )
    {
        pView->SetModalMode( true );
        return true;
    }
    else if ( rNEvt.GetType() == MouseNotifyEvent::ENDEXECUTEDIALOG /*|| rNEvt.GetType() == MouseNotifyEvent::INPUTENABLE*/ )
    {
        pView->SetModalMode( false );
        return true;
    }

    return Window::Notify( rNEvt );
}

bool SfxFrameWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();
    if ( nType == MouseNotifyEvent::KEYINPUT || nType == MouseNotifyEvent::KEYUP )
    {
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : NULL;
        if ( pShell && pShell->HasKeyListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
            return true;
    }
    else if ( nType == MouseNotifyEvent::MOUSEBUTTONUP || nType == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        vcl::Window* pWindow = rNEvt.GetWindow();
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : NULL;
        if ( pShell )
            if ( pWindow == pShell->GetWindow() || pShell->GetWindow()->IsChild( pWindow ) )
                if ( pShell->HasMouseClickListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
                    return true;
    }

    if ( nType == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        vcl::Window* pWindow = rNEvt.GetWindow();
        const MouseEvent* pMEvent = rNEvt.GetMouseEvent();
        Point aPos = pWindow->OutputToScreenPixel( pMEvent->GetPosPixel() );
        SfxWorkWindow *pWorkWin = pFrame->GetWorkWindow_Impl();
        if ( pWorkWin )
            pWorkWin->EndAutoShow_Impl( aPos );
    }

    return Window::PreNotify( rNEvt );
}

void SfxFrameWindow_Impl::GetFocus()
{
    if ( pFrame && !pFrame->IsClosing_Impl() && pFrame->GetCurrentViewFrame() && pFrame->GetFrameInterface().is() )
        pFrame->GetCurrentViewFrame()->MakeActive_Impl( true );
}

void SfxFrameWindow_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() || GetOutputSizePixel().Width() )
        DoResize();
}

void SfxFrameWindow_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        pFrame->pImp->bHidden = false;
        if ( pFrame->IsInPlace() )
            // TODO/MBA: workaround for bug in LayoutManager: the final resize does not get through because the
            // LayoutManager works asynchronously and between resize and time execution the DockingAcceptor was exchanged so that
            // the resize event never is sent to the component
            SetSizePixel( GetParent()->GetOutputSizePixel() );

        DoResize();
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        if ( pView )
            pView->GetBindings().GetWorkWindow_Impl()->ShowChildren_Impl();
    }

    Window::StateChanged( nStateChange );
}

void SfxFrameWindow_Impl::DoResize()
{
    if ( !pFrame->pImp->bLockResize )
        pFrame->Resize();
}

Reference < XFrame > SfxFrame::CreateBlankFrame()
{
    Reference < XFrame > xFrame;
    try
    {
        Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xFrame.set( xDesktop->findFrame( "_blank", 0 ), UNO_SET_THROW );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xFrame;
}

SfxFrame* SfxFrame::Create( SfxObjectShell& rDoc, vcl::Window& rWindow, sal_uInt16 nViewId, bool bHidden )
{
    SfxFrame* pFrame = NULL;
    try
    {
        // create and initialize new top level frame for this window
        Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference < XDesktop2 > xDesktop = Desktop::create( xContext );
        Reference < XFrame2 > xFrame = Frame::create( xContext );

        Reference< awt::XWindow2 > xWin( VCLUnoHelper::GetInterface ( &rWindow ), uno::UNO_QUERY_THROW );
        xFrame->initialize( xWin.get() );
        xDesktop->getFrames()->append( xFrame );

        if ( xWin->isActive() )
            xFrame->activate();

        // create load arguments
        Sequence< PropertyValue > aLoadArgs;
        TransformItems( SID_OPENDOC, *rDoc.GetMedium()->GetItemSet(), aLoadArgs );

        ::comphelper::NamedValueCollection aArgs( aLoadArgs );
        aArgs.put( "Model", rDoc.GetModel() );
        aArgs.put( "Hidden", bHidden );
        if ( nViewId )
            aArgs.put( "ViewId", nViewId );

        aLoadArgs = aArgs.getPropertyValues();

        // load the doc into that frame
        OUString sLoaderURL( "private:object" );
        Reference< XComponentLoader > xLoader( xFrame, UNO_QUERY_THROW );
        xLoader->loadComponentFromURL(
            sLoaderURL,
            OUString( "_self" ),
            0,
            aLoadArgs
        );

        for (   pFrame = SfxFrame::GetFirst();
                pFrame;
                pFrame = SfxFrame::GetNext( *pFrame )
            )
        {
            if ( pFrame->GetFrameInterface() == xFrame )
                break;
        }

        OSL_ENSURE( pFrame, "SfxFrame::Create: load succeeded, but no SfxFrame was created during this!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return pFrame;
}

SfxFrame* SfxFrame::Create( const Reference < XFrame >& i_rFrame )
{
    // create a new TopFrame to an external XFrame object ( wrap controller )
    ENSURE_OR_THROW( i_rFrame.is(), "NULL frame not allowed" );
    vcl::Window* pWindow = VCLUnoHelper::GetWindow( i_rFrame->getContainerWindow() );
    ENSURE_OR_THROW( pWindow, "frame without container window not allowed" );

    SfxFrame* pFrame = new SfxFrame( *pWindow, false );
    pFrame->SetFrameInterface_Impl( i_rFrame );
    return pFrame;
}

SfxFrame::SfxFrame( vcl::Window& i_rContainerWindow, bool i_bHidden )
    :SvCompatWeakBase<SfxFrame>( this )
    ,pParentFrame( NULL )
    ,pChildArr( NULL )
    ,pImp( NULL )
    ,pWindow( NULL )
{
    Construct_Impl();

    pImp->bHidden = i_bHidden;
    InsertTopFrame_Impl( this );
    pImp->pExternalContainerWindow = &i_rContainerWindow;

    pWindow = VclPtr<SfxFrameWindow_Impl>::Create( this, i_rContainerWindow );

    // always show pWindow, which is the ComponentWindow of the XFrame we live in
    // nowadays, since SfxFrames can be created with an XFrame only, hiding or showing the complete XFrame
    // is not done at level of the container window, not at SFX level. Thus, the component window can
    // always be visible.
    pWindow->Show();
}

void SfxFrame::SetPresentationMode( bool bSet )
{
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetWindow().SetBorderStyle( bSet ? WindowBorderStyle::NOBORDER : WindowBorderStyle::NORMAL );

    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue("LayoutManager");
        aValue >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
        xLayoutManager->setVisible( !bSet ); // we don't want to have ui in presentation mode

    SetMenuBarOn_Impl( !bSet );
    if ( GetWorkWindow_Impl() )
        GetWorkWindow_Impl()->SetDockingAllowed( !bSet );
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetDispatcher()->Update_Impl( true );
}

SystemWindow* SfxFrame::GetSystemWindow() const
{
    return GetTopWindow_Impl();
}

SystemWindow* SfxFrame::GetTopWindow_Impl() const
{
    if ( pImp->pExternalContainerWindow->IsSystemWindow() )
        return static_cast<SystemWindow*>( pImp->pExternalContainerWindow.get() );
    else
        return NULL;
}


bool SfxFrame::Close()
{
    delete this;
    return true;
}

void SfxFrame::LockResize_Impl( bool bLock )
{
    pImp->bLockResize = bLock;
}

void SfxFrame::SetMenuBarOn_Impl( bool bOn )
{
    pImp->bMenuBarOn = bOn;

    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue("LayoutManager");
        aValue >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
    {
        OUString aMenuBarURL( "private:resource/menubar/menubar" );

        if ( bOn )
            xLayoutManager->showElement( aMenuBarURL );
        else
            xLayoutManager->hideElement( aMenuBarURL );
    }
}

bool SfxFrame::IsMenuBarOn_Impl() const
{
    return pImp->bMenuBarOn;
}

void SfxFrame::PrepareForDoc_Impl( SfxObjectShell& i_rDoc )
{
    const ::comphelper::NamedValueCollection aDocumentArgs( i_rDoc.GetModel()->getArgs() );

    // hidden?
    OSL_ENSURE( !pImp->bHidden, "when does this happen?" );
    pImp->bHidden = aDocumentArgs.getOrDefault( "Hidden", pImp->bHidden );

    // update our descriptor
    UpdateDescriptor( &i_rDoc );

    // plugin mode
    sal_Int16 nPluginMode = aDocumentArgs.getOrDefault( "PluginMode", sal_Int16( 0 ) );
    if ( nPluginMode && ( nPluginMode != 2 ) )
        SetInPlace_Impl( true );
}

bool SfxFrame::IsMarkedHidden_Impl() const
{
    return pImp->bHidden;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
