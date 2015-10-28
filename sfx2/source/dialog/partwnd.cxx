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

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <sfx2/sfxsids.hrc>
#include "partwnd.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxuno.hxx>


// SfxPartChildWnd_Impl


SFX_IMPL_DOCKINGWINDOW( SfxPartChildWnd_Impl, SID_BROWSER );

SfxPartChildWnd_Impl::SfxPartChildWnd_Impl
(
    vcl::Window* pParentWnd,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
)
    : SfxChildWindow( pParentWnd, nId )
{
    // Create Window
    SetWindow(VclPtr<SfxPartDockWnd_Impl>::Create( pBindings, this, pParentWnd, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK ));
    SetAlignment(SfxChildAlignment::TOP);

    assert(pInfo);
    pInfo->nFlags |= SfxChildWindowFlags::FORCEDOCK;

    static_cast<SfxDockingWindow*>(GetWindow())->SetFloatingSize( Size( 175, 175 ) );
    GetWindow()->SetSizePixel( Size( 175, 175 ) );

    static_cast<SfxDockingWindow*>(GetWindow())->Initialize( pInfo );
    SetHideNotDelete( true );
}

SfxPartChildWnd_Impl::~SfxPartChildWnd_Impl()
{
    css::uno::Reference< css::frame::XFrame > xFrame = GetFrame();

    // If xFrame=NULL release pMgr! Because this window lives longer then the manager!
    // In these case we got a xFrame->dispose() call from outside ... and has release our
    // frame reference in our own DisposingListener.
    // But don't do it, if xFrame already exist. Then dispose() must come from inside ...
    // and we need a valid pMgr for further operations ...

    SfxPartDockWnd_Impl* pWin = static_cast<SfxPartDockWnd_Impl*>(GetWindow());

    if ( pWin && xFrame == pWin->GetBindings().GetActiveFrame() )
        pWin->GetBindings().SetActiveFrame( NULL );
}

bool SfxPartChildWnd_Impl::QueryClose()
{
    return static_cast<SfxPartDockWnd_Impl*>(GetWindow())->QueryClose();
}


// SfxPartDockWnd_Impl


SfxPartDockWnd_Impl::SfxPartDockWnd_Impl
(
    SfxBindings* pBind,
    SfxChildWindow* pChildWin,
    vcl::Window* pParent,
    WinBits nBits
)
    : SfxDockingWindow( pBind, pChildWin, pParent, nBits )
{
    css::uno::Reference < css::frame::XFrame2 > xFrame = css::frame::Frame::create(
            ::comphelper::getProcessComponentContext() );
    xFrame->initialize( VCLUnoHelper::GetInterface ( this ) );

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xLMPropSet( xFrame->getLayoutManager(), css::uno::UNO_QUERY_THROW );

        const OUString aAutomaticToolbars( "AutomaticToolbars" );
        xLMPropSet->setPropertyValue( aAutomaticToolbars, css::uno::Any( sal_False ));
    }
    catch( css::uno::RuntimeException& )
    {
        throw;
    }
    catch( css::uno::Exception& )
    {
    }

    pChildWin->SetFrame( css::uno::Reference<css::frame::XFrame>(xFrame,css::uno::UNO_QUERY_THROW) );
    if ( pBind->GetDispatcher() )
    {
        css::uno::Reference < css::frame::XFramesSupplier >
                xSupp ( pBind->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface(), css::uno::UNO_QUERY );
        if ( xSupp.is() )
            xSupp->getFrames()->append( css::uno::Reference<css::frame::XFrame>(xFrame, css::uno::UNO_QUERY_THROW) );
    }
    else {
        OSL_FAIL("Bindings without Dispatcher!");
    }
}



void SfxPartDockWnd_Impl::Resize()

/*  [Description]

    Adjusting the size of the controls wrt the new window size
*/

{
    SfxDockingWindow::Resize();
}



bool SfxPartDockWnd_Impl::QueryClose()
{
    bool bClose = true;
    SfxChildWindow* pChild = GetChildWindow_Impl();
    if( pChild )
    {
        css::uno::Reference< css::frame::XFrame > xFrame = pChild->GetFrame();
        if( xFrame.is() )
        {
            css::uno::Reference< css::frame::XController >  xCtrl = xFrame->getController();
            if( xCtrl.is() )
                bClose = xCtrl->suspend( sal_True );
        }
    }

    return bClose;
}



bool SfxPartDockWnd_Impl::Notify( NotifyEvent& rEvt )
{
    if( rEvt.GetType() == MouseNotifyEvent::GETFOCUS )
    {
        SfxChildWindow* pChild = GetChildWindow_Impl();
        if( pChild )
        {
            css::uno::Reference< css::frame::XFrame > xFrame = pChild->GetFrame();
            if( xFrame.is() )
                xFrame->activate();
        }
    }

    return SfxDockingWindow::Notify( rEvt );
}

void SfxPartDockWnd_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
