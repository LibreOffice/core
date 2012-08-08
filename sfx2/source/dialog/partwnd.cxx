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
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <sfx2/sfxsids.hrc>
#include "partwnd.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxuno.hxx>

//****************************************************************************
// SfxPartChildWnd_Impl
//****************************************************************************

SFX_IMPL_DOCKINGWINDOW( SfxPartChildWnd_Impl, SID_BROWSER );

SfxPartChildWnd_Impl::SfxPartChildWnd_Impl
(
    Window* pParentWnd,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
)
    : SfxChildWindow( pParentWnd, nId )
{
    // Create Window
    pWindow = new SfxPartDockWnd_Impl( pBindings, this, pParentWnd, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK );
    eChildAlignment = SFX_ALIGN_TOP;
    if ( pInfo )
        pInfo->nFlags |= SFX_CHILDWIN_FORCEDOCK;

    ((SfxDockingWindow*)pWindow)->SetFloatingSize( Size( 175, 175 ) );
    pWindow->SetSizePixel( Size( 175, 175 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
    SetHideNotDelete( sal_True );
}

SfxPartChildWnd_Impl::~SfxPartChildWnd_Impl()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = GetFrame();

    // If xFrame=NULL release pMgr! Because this window lives longer then the manager!
    // In these case we got a xFrame->dispose() call from outside ... and has release our
    // frame reference in our own DisposingListener.
    // But don't do it, if xFrame already exist. Then dispose() must come from inside ...
    // and we need a valid pMgr for further operations ...

    SfxPartDockWnd_Impl* pWin = (SfxPartDockWnd_Impl*) pWindow;

    if ( pWin && xFrame == pWin->GetBindings().GetActiveFrame() )
        pWin->GetBindings().SetActiveFrame( NULL );
}

sal_Bool SfxPartChildWnd_Impl::QueryClose()
{
    return ( (SfxPartDockWnd_Impl*)pWindow )->QueryClose();
}

//****************************************************************************
// SfxPartDockWnd_Impl
//****************************************************************************

SfxPartDockWnd_Impl::SfxPartDockWnd_Impl
(
    SfxBindings* pBind,
    SfxChildWindow* pChildWin,
    Window* pParent,
    WinBits nBits
)
    : SfxDockingWindow( pBind, pChildWin, pParent, nBits )
{
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame(
            ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), ::com::sun::star::uno::UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( this ) );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet(
        xFrame, ::com::sun::star::uno::UNO_QUERY );
    try
    {
        const ::rtl::OUString aLayoutManager( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ));
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xLMPropSet;

        ::com::sun::star::uno::Any a = xPropSet->getPropertyValue( aLayoutManager );
        if ( a >>= xLMPropSet )
        {
            const ::rtl::OUString aAutomaticToolbars( RTL_CONSTASCII_USTRINGPARAM( "AutomaticToolbars" ));
            xLMPropSet->setPropertyValue( aAutomaticToolbars, ::com::sun::star::uno::Any( sal_False ));
        }
    }
    catch( ::com::sun::star::uno::RuntimeException& )
    {
        throw;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    pChildWin->SetFrame( xFrame );
    if ( pBind->GetDispatcher() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                xSupp ( pBind->GetDispatcher()->GetFrame()->GetFrame().GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
        if ( xSupp.is() )
            xSupp->getFrames()->append( xFrame );
    }
    else {
        OSL_FAIL("Bindings without Dispatcher!");
    }
}

//****************************************************************************

SfxPartDockWnd_Impl::~SfxPartDockWnd_Impl()
{
}

//****************************************************************************

void SfxPartDockWnd_Impl::Resize()

/*  [Description]

    Adjusting the size of the controls wrt the new window size
*/

{
    SfxDockingWindow::Resize();
}

//****************************************************************************

sal_Bool SfxPartDockWnd_Impl::QueryClose()
{
    sal_Bool bClose = sal_True;
    SfxChildWindow* pChild = GetChildWindow_Impl();
    if( pChild )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = pChild->GetFrame();
        if( xFrame.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >  xCtrl = xFrame->getController();
            if( xCtrl.is() )
                bClose = xCtrl->suspend( sal_True );
        }
    }

    return bClose;
}

//****************************************************************************

long SfxPartDockWnd_Impl::Notify( NotifyEvent& rEvt )
{
    if( rEvt.GetType() == EVENT_GETFOCUS )
    {
        SfxChildWindow* pChild = GetChildWindow_Impl();
        if( pChild )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = pChild->GetFrame();
            if( xFrame.is() )
                xFrame->activate();
        }
    }

    return SfxDockingWindow::Notify( rEvt );
}

void SfxPartDockWnd_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
