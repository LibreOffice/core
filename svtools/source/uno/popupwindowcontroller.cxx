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

#include <cppuhelper/supportsservice.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>

#include <svtools/popupwindowcontroller.hxx>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::lang;


namespace svt
{

class PopupWindowControllerImpl
{
public:
    PopupWindowControllerImpl();
    ~PopupWindowControllerImpl();

    void SetPopupWindow( vcl::Window* pPopupWindow, ToolBox* pToolBox );
    void SetFloatingWindow();
    DECL_LINK( WindowEventListener, VclWindowEvent&, void );

private:
    VclPtr<vcl::Window> mpPopupWindow, mpFloatingWindow;
    VclPtr<ToolBox>     mpToolBox;
};

PopupWindowControllerImpl::PopupWindowControllerImpl()
{
}

PopupWindowControllerImpl::~PopupWindowControllerImpl()
{
    SetPopupWindow(nullptr,nullptr);
    SetFloatingWindow();
}

void PopupWindowControllerImpl::SetPopupWindow( vcl::Window* pPopupWindow, ToolBox* pToolBox )
{
    if( mpPopupWindow )
    {
        mpPopupWindow->RemoveEventListener( LINK( this, PopupWindowControllerImpl, WindowEventListener ) );
        mpPopupWindow.disposeAndClear();
    }
    mpPopupWindow = pPopupWindow;
    mpToolBox = pToolBox;

    if( mpPopupWindow )
    {
        mpPopupWindow->AddEventListener( LINK( this, PopupWindowControllerImpl, WindowEventListener ));
    }
}

void PopupWindowControllerImpl::SetFloatingWindow()
{
    if( mpFloatingWindow )
    {
        mpFloatingWindow->RemoveEventListener( LINK( this, PopupWindowControllerImpl, WindowEventListener ) );
        mpFloatingWindow.disposeAndClear();
    }
    mpFloatingWindow = mpPopupWindow;
    mpPopupWindow.clear();
}

IMPL_LINK( PopupWindowControllerImpl, WindowEventListener, VclWindowEvent&, rWindowEvent, void )
{
    switch( rWindowEvent.GetId() )
    {
    case VCLEVENT_WINDOW_ENDPOPUPMODE:
    {
        EndPopupModeData* pData = static_cast< EndPopupModeData* >( rWindowEvent.GetData() );
        if( pData && pData->mbTearoff )
        {
            vcl::Window::GetDockingManager()->SetFloatingMode( mpPopupWindow.get(), true );
            vcl::Window::GetDockingManager()->SetPosSizePixel( mpPopupWindow.get(),
                                                               pData->maFloatingPos.X(),
                                                               pData->maFloatingPos.Y(),
                                                               0, 0,
                                                               PosSizeFlags::Pos );
            SetFloatingWindow();
            mpFloatingWindow->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
        }
        SetPopupWindow(nullptr,nullptr);
        break;
    }
    case VCLEVENT_WINDOW_PREPARETOGGLEFLOATING:
    {
        if ( mpFloatingWindow && rWindowEvent.GetWindow() == mpFloatingWindow.get() )
        {
            bool* pData = static_cast< bool* >( rWindowEvent.GetData() );
            *pData = false;
        }
        break;
    }
    case VCLEVENT_WINDOW_CLOSE:
    {
        SetPopupWindow(nullptr,nullptr);
        SetFloatingWindow();
        break;
    }
    case VCLEVENT_WINDOW_SHOW:
    {
        if( mpPopupWindow )
        {
            if( mpToolBox )
                mpToolBox->CallEventListeners( VCLEVENT_DROPDOWN_OPEN, static_cast<void*>(mpPopupWindow) );
            mpPopupWindow->CallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
            break;
        }
        break;
    }
    case VCLEVENT_WINDOW_HIDE:
    {
        if( mpPopupWindow )
        {
            mpPopupWindow->CallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
            if( mpToolBox )
                mpToolBox->CallEventListeners( VCLEVENT_DROPDOWN_CLOSE, static_cast<void*>(mpPopupWindow) );
        }
        break;
    }
    }
}


// class PopupWindowController


PopupWindowController::PopupWindowController( const Reference< uno::XComponentContext >& rxContext,
                                              const Reference< frame::XFrame >& xFrame,
                                              const OUString& aCommandURL )
: ImplInheritanceHelper( rxContext, xFrame, aCommandURL )
, mxImpl( new PopupWindowControllerImpl() )
{
}

PopupWindowController::~PopupWindowController()
{
}

// XServiceInfo
sal_Bool SAL_CALL PopupWindowController::supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XComponent
void SAL_CALL PopupWindowController::dispose() throw (RuntimeException, std::exception)
{
    mxImpl.reset();
    svt::ToolboxController::dispose();
}

// XStatusListener
void SAL_CALL PopupWindowController::statusChanged( const frame::FeatureStateEvent& rEvent ) throw ( RuntimeException, std::exception )
{
    svt::ToolboxController::statusChanged(rEvent);
    enable( rEvent.IsEnabled );
}

Reference< awt::XWindow > SAL_CALL PopupWindowController::createPopupWindow() throw (RuntimeException, std::exception)
{
    VclPtr< ToolBox > pToolBox = dynamic_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );
    if( pToolBox )
    {
        vcl::Window* pItemWindow = pToolBox->GetItemWindow( pToolBox->GetDownItemId() );
        VclPtr<vcl::Window> pWin = createPopupWindow( pItemWindow ? pItemWindow : pToolBox );
        if( pWin )
        {
            FloatWinPopupFlags eFloatFlags = FloatWinPopupFlags::GrabFocus |
                                             FloatWinPopupFlags::AllMouseButtonClose |
                                             FloatWinPopupFlags::NoMouseUpClose;

            WinBits nWinBits;
            if ( pWin->GetType() == WINDOW_DOCKINGWINDOW )
                nWinBits = static_cast< DockingWindow* >( pWin.get() )->GetFloatStyle();
            else
                nWinBits = pWin->GetStyle();

            if ( nWinBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) )
                eFloatFlags |= FloatWinPopupFlags::AllowTearOff;

            pWin->EnableDocking();
            mxImpl->SetPopupWindow(pWin,pToolBox);
            vcl::Window::GetDockingManager()->StartPopupMode( pToolBox, pWin, eFloatFlags );
        }
    }
    return Reference< awt::XWindow >();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
