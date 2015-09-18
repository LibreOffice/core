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
#include <cppuhelper/queryinterface.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>

#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


namespace svt
{

class PopupWindowControllerImpl
{
public:
    PopupWindowControllerImpl();
    ~PopupWindowControllerImpl();

    void SetPopupWindow( vcl::Window* pPopupWindow, ToolBox* pToolBox );
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );

private:
    VclPtr<vcl::Window> mpPopupWindow;
    VclPtr<ToolBox>     mpToolBox;
};

PopupWindowControllerImpl::PopupWindowControllerImpl()
: mpPopupWindow( 0 )
, mpToolBox( 0 )
{
}

PopupWindowControllerImpl::~PopupWindowControllerImpl()
{
    if( mpPopupWindow )
        SetPopupWindow(0,0);
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

IMPL_LINK_TYPED( PopupWindowControllerImpl, WindowEventListener, VclWindowEvent&, rWindowEvent, void )
{
    switch( rWindowEvent.GetId() )
    {
    case VCLEVENT_WINDOW_CLOSE:
    case VCLEVENT_WINDOW_ENDPOPUPMODE:
        SetPopupWindow(0,0);
        break;

    case VCLEVENT_WINDOW_SHOW:
    {
        if( mpPopupWindow )
        {
            if( mpToolBox )
                mpToolBox->CallEventListeners( VCLEVENT_DROPDOWN_OPEN, static_cast<void*>(mpPopupWindow) );
            mpPopupWindow->CallEventListeners( VCLEVENT_WINDOW_GETFOCUS, 0 );

            svtools::ToolbarMenu* pToolbarMenu = dynamic_cast< svtools::ToolbarMenu* >( mpPopupWindow.get() );
            if( pToolbarMenu )
                pToolbarMenu->highlightFirstEntry();
            break;
        }
        break;
    }
    case VCLEVENT_WINDOW_HIDE:
    {
        if( mpPopupWindow )
        {
            mpPopupWindow->CallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS, 0 );
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
: svt::ToolboxController( rxContext, xFrame, aCommandURL )
, mxImpl( new PopupWindowControllerImpl() )
{
}

PopupWindowController::~PopupWindowController()
{
}

// XInterface
Any SAL_CALL PopupWindowController::queryInterface( const Type& aType )
throw (RuntimeException, std::exception)
{
    Any a( ToolboxController::queryInterface( aType ) );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

void SAL_CALL PopupWindowController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL PopupWindowController::release() throw ()
{
    ToolboxController::release();
}

// XServiceInfo
sal_Bool SAL_CALL PopupWindowController::supportsService( const OUString& ServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XInitialization
void SAL_CALL PopupWindowController::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize( aArguments );
    if( !m_aCommandURL.isEmpty() )
        addStatusListener( m_aCommandURL );
}

// XComponent
void SAL_CALL PopupWindowController::dispose() throw (RuntimeException, std::exception)
{
    if( !m_aCommandURL.isEmpty() )
        removeStatusListener( m_aCommandURL );

    svt::ToolboxController::dispose();
}


// XStatusListener
void SAL_CALL PopupWindowController::statusChanged( const frame::FeatureStateEvent& rEvent ) throw ( RuntimeException, std::exception )
{
    svt::ToolboxController::statusChanged(rEvent);
    enable( rEvent.IsEnabled );
}

// XToolbarController
void SAL_CALL PopupWindowController::execute( sal_Int16 KeyModifier ) throw (RuntimeException, std::exception)
{
    svt::ToolboxController::execute( KeyModifier );
}

void SAL_CALL PopupWindowController::click() throw (RuntimeException, std::exception)
{
    svt::ToolboxController::click();
}

void SAL_CALL PopupWindowController::doubleClick() throw (RuntimeException, std::exception)
{
    svt::ToolboxController::doubleClick();
}

Reference< awt::XWindow > SAL_CALL PopupWindowController::createPopupWindow() throw (RuntimeException, std::exception)
{
    VclPtr< ToolBox > pToolBox = dynamic_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );
    if( pToolBox )
    {
        vcl::Window* pItemWindow = pToolBox->GetItemWindow( pToolBox->GetDownItemId() );
        vcl::Window* pWin = createPopupWindow( pItemWindow ? pItemWindow : pToolBox );
        if( pWin )
        {
            pWin->EnableDocking();
            mxImpl->SetPopupWindow(pWin,pToolBox);
            vcl::Window::GetDockingManager()->StartPopupMode( pToolBox, pWin,
                                                           FloatWinPopupFlags::GrabFocus |
                                                           FloatWinPopupFlags::NoFocusClose |
                                                           FloatWinPopupFlags::AllMouseButtonClose |
                                                           FloatWinPopupFlags::NoMouseUpClose );
        }
    }
    return Reference< awt::XWindow >();
}

Reference< awt::XWindow > SAL_CALL PopupWindowController::createItemWindow( const Reference< awt::XWindow >& /*Parent*/ )
    throw (RuntimeException, std::exception)
{
    return Reference< awt::XWindow >();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
