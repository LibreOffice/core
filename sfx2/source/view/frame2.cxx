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
#include <workwin.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfx2/viewsh.hxx>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/event.hxx>
#include <vcl/syswin.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using ::com::sun::star::frame::XComponentLoader;

class SfxFrameWindow_Impl : public vcl::Window
{
    DECL_LINK(ModalHierarchyHdl, bool, void);
public:
    SfxFrame*           m_pFrame;

    SfxFrameWindow_Impl( SfxFrame* pF, vcl::Window& i_rContainerWindow );

    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool        EventNotify( NotifyEvent& rEvt ) override;
    virtual void        Resize() override;
    virtual void        GetFocus() override;
    virtual void        dispose() override;
    void                DoResize();
};

SfxFrameWindow_Impl::SfxFrameWindow_Impl(SfxFrame* pF, vcl::Window& i_rContainerWindow)
    : Window(&i_rContainerWindow, WB_BORDER | WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_3DLOOK)
    , m_pFrame(pF)
{
    i_rContainerWindow.SetModalHierarchyHdl(LINK(this, SfxFrameWindow_Impl, ModalHierarchyHdl));
}

void SfxFrameWindow_Impl::dispose()
{
    GetParent()->SetModalHierarchyHdl(Link<bool, void>());
    vcl::Window::dispose();
}

void SfxFrameWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    // tdf#131613 the printers changing has no effect on window layout
    if (rDCEvt.GetType() == DataChangedEventType::PRINTER)
        return;
    SfxWorkWindow *pWorkWin = m_pFrame->GetWorkWindow_Impl();
    if ( pWorkWin )
        pWorkWin->DataChanged_Impl();
}

bool SfxFrameWindow_Impl::EventNotify( NotifyEvent& rNEvt )
{
    if ( m_pFrame->IsClosing_Impl() || !m_pFrame->GetFrameInterface().is() )
        return false;

    SfxViewFrame* pView = m_pFrame->GetCurrentViewFrame();
    if ( !pView || !pView->GetObjectShell() )
        return Window::EventNotify( rNEvt );

    if ( rNEvt.GetType() == NotifyEventType::GETFOCUS )
    {
        if ( pView->GetViewShell() && !pView->GetViewShell()->GetUIActiveIPClient_Impl() && !m_pFrame->IsInPlace() )
        {
            SAL_INFO("sfx", "SfxFrame: GotFocus");
            pView->MakeActive_Impl( false );
        }

        // if focus was on an external window, the clipboard content might have been changed
        pView->GetBindings().Invalidate( SID_PASTE );
        pView->GetBindings().Invalidate( SID_PASTE_SPECIAL );
        return true;
    }
    else if( rNEvt.GetType() == NotifyEventType::KEYINPUT )
    {
        if ( pView->GetViewShell()->KeyInput( *rNEvt.GetKeyEvent() ) )
            return true;
    }

    return Window::EventNotify( rNEvt );
}

IMPL_LINK(SfxFrameWindow_Impl, ModalHierarchyHdl, bool, bSetModal, void)
{
    SfxViewFrame* pView = m_pFrame->GetCurrentViewFrame();
    if (!pView || !pView->GetObjectShell())
        return;
    pView->SetModalMode(bSetModal);
}

bool SfxFrameWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    NotifyEventType nType = rNEvt.GetType();
    if ( nType == NotifyEventType::KEYINPUT || nType == NotifyEventType::KEYUP )
    {
        SfxViewFrame* pView = m_pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : nullptr;
        if ( pShell && pShell->HasKeyListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
            return true;
    }
    else if ( nType == NotifyEventType::MOUSEBUTTONUP || nType == NotifyEventType::MOUSEBUTTONDOWN )
    {
        vcl::Window* pWindow = rNEvt.GetWindow();
        SfxViewFrame* pView = m_pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : nullptr;
        if ( pShell )
            if ( pWindow == pShell->GetWindow() || pShell->GetWindow()->IsChild( pWindow ) )
                if ( pShell->HasMouseClickListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
                    return true;
    }

    if ( nType == NotifyEventType::MOUSEBUTTONDOWN )
    {
        vcl::Window* pWindow = rNEvt.GetWindow();
        const MouseEvent* pMEvent = rNEvt.GetMouseEvent();
        Point aPos = pWindow->OutputToScreenPixel( pMEvent->GetPosPixel() );
        SfxWorkWindow *pWorkWin = m_pFrame->GetWorkWindow_Impl();
        if ( pWorkWin )
            pWorkWin->EndAutoShow_Impl( aPos );
    }

    return Window::PreNotify( rNEvt );
}

void SfxFrameWindow_Impl::GetFocus()
{
    if ( m_pFrame && !m_pFrame->IsClosing_Impl() &&
         m_pFrame->GetCurrentViewFrame() &&
         m_pFrame->GetFrameInterface().is() )
        m_pFrame->GetCurrentViewFrame()->MakeActive_Impl( true );
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
        m_pFrame->m_pImpl->bHidden = false;
        if ( m_pFrame->IsInPlace() )
            // TODO/MBA: workaround for bug in LayoutManager: the final resize does not get through because the
            // LayoutManager works asynchronously and between resize and time execution the DockingAcceptor was exchanged so that
            // the resize event never is sent to the component
            SetSizePixel( GetParent()->GetOutputSizePixel() );

        DoResize();
        SfxViewFrame* pView = m_pFrame->GetCurrentViewFrame();
        if ( pView )
            pView->GetBindings().GetWorkWindow_Impl()->ShowChildren_Impl();
    }

    Window::StateChanged( nStateChange );
}

void SfxFrameWindow_Impl::DoResize()
{
    if ( !m_pFrame->m_pImpl->bLockResize )
        m_pFrame->Resize();
}

Reference < XFrame > SfxFrame::CreateBlankFrame()
{
    Reference < XFrame > xFrame;
    try
    {
        Reference < XDesktop2 > xDesktop = Desktop::create( ::comphelper::getProcessComponentContext() );
        xFrame.set( xDesktop->findFrame( u"_blank"_ustr, 0 ), UNO_SET_THROW );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }
    return xFrame;
}

SfxFrame* SfxFrame::CreateHidden( SfxObjectShell const & rDoc, vcl::Window& rWindow, SfxInterfaceId nViewId )
{
    SfxFrame* pFrame = nullptr;
    try
    {
        // create and initialize new top level frame for this window
        Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        Reference < XDesktop2 > xDesktop = Desktop::create( xContext );
        Reference < XFrame2 > xFrame = Frame::create( xContext );

        Reference< awt::XWindow2 > xWin( VCLUnoHelper::GetInterface ( &rWindow ), uno::UNO_QUERY_THROW );
        xFrame->initialize( xWin );
        xDesktop->getFrames()->append( xFrame );

        if ( xWin->isActive() )
            xFrame->activate();

        // create load arguments
        Sequence< PropertyValue > aLoadArgs;
        TransformItems( SID_OPENDOC, rDoc.GetMedium()->GetItemSet(), aLoadArgs );

        ::comphelper::NamedValueCollection aArgs( aLoadArgs );
        aArgs.put( u"Model"_ustr, rDoc.GetModel() );
        aArgs.put( u"Hidden"_ustr, true );
        if ( nViewId != SFX_INTERFACE_NONE )
            aArgs.put( u"ViewId"_ustr, static_cast<sal_uInt16>(nViewId) );

        aLoadArgs = aArgs.getPropertyValues();

        // load the doc into that frame
        Reference< XComponentLoader > xLoader( xFrame, UNO_QUERY_THROW );
        xLoader->loadComponentFromURL(
            u"private:object"_ustr,
            u"_self"_ustr,
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
        DBG_UNHANDLED_EXCEPTION("sfx.view");
    }

    return pFrame;
}

SfxFrame* SfxFrame::Create( const Reference < XFrame >& i_rFrame )
{
    // create a new TopFrame to an external XFrame object ( wrap controller )
    ENSURE_OR_THROW( i_rFrame.is(), "NULL frame not allowed" );
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( i_rFrame->getContainerWindow() );
    ENSURE_OR_THROW( pWindow, "frame without container window not allowed" );

    SfxFrame* pFrame = new SfxFrame( *pWindow );
    pFrame->SetFrameInterface_Impl( i_rFrame );
    return pFrame;
}

SfxFrame::SfxFrame( vcl::Window& i_rContainerWindow )
    :SvCompatWeakBase<SfxFrame>( this )
    ,m_pWindow( nullptr )
{
    Construct_Impl();

    m_pImpl->bHidden = false;
    InsertTopFrame_Impl( this );
    m_pImpl->pExternalContainerWindow = &i_rContainerWindow;

    m_pWindow = VclPtr<SfxFrameWindow_Impl>::Create( this, i_rContainerWindow );

    // always show pWindow, which is the ComponentWindow of the XFrame we live in
    // nowadays, since SfxFrames can be created with an XFrame only, hiding or showing the complete XFrame
    // is not done at level of the container window, not at SFX level. Thus, the component window can
    // always be visible.
    m_pWindow->Show();
}

void SfxFrame::SetPresentationMode( bool bSet )
{
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetWindow().SetBorderStyle( bSet ? WindowBorderStyle::NOBORDER : WindowBorderStyle::NORMAL );

    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
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
    if ( m_pImpl->pExternalContainerWindow->IsSystemWindow() )
        return static_cast<SystemWindow*>( m_pImpl->pExternalContainerWindow.get() );
    else
        return nullptr;
}


void SfxFrame::LockResize_Impl( bool bLock )
{
    m_pImpl->bLockResize = bLock;
}

void SfxFrame::SetMenuBarOn_Impl( bool bOn )
{
    m_pImpl->bMenuBarOn = bOn;

    Reference< css::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< css::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue(u"LayoutManager"_ustr);
        aValue >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
    {
        OUString aMenuBarURL( u"private:resource/menubar/menubar"_ustr );

        if ( bOn )
            xLayoutManager->showElement( aMenuBarURL );
        else
            xLayoutManager->hideElement( aMenuBarURL );
    }
}

bool SfxFrame::IsMenuBarOn_Impl() const
{
    return m_pImpl->bMenuBarOn;
}

void SfxFrame::PrepareForDoc_Impl( const SfxObjectShell& i_rDoc )
{
    const ::comphelper::NamedValueCollection aDocumentArgs( i_rDoc.GetModel()->getArgs2( { u"Hidden"_ustr, u"PluginMode"_ustr } ) );

    // hidden?
    OSL_ENSURE( !m_pImpl->bHidden, "when does this happen?" );
    m_pImpl->bHidden = aDocumentArgs.getOrDefault( u"Hidden"_ustr, m_pImpl->bHidden );

    // update our descriptor
    UpdateDescriptor( &i_rDoc );

    // plugin mode
    sal_Int16 nPluginMode = aDocumentArgs.getOrDefault( u"PluginMode"_ustr, sal_Int16( 0 ) );
    if ( nPluginMode && ( nPluginMode != 2 ) )
        m_pImpl->bInPlace = true;
}

bool SfxFrame::IsMarkedHidden_Impl() const
{
    return m_pImpl->bHidden;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
