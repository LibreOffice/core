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

#include <boost/ptr_container/ptr_vector.hpp>
#include "vclxdialog.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>

#include <cppuhelper/typeprovider.hxx>

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>

#ifdef QUARTZ
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/wrkwin.hxx>

#include "forward.hxx"

namespace layoutimpl
{

DBG_NAME( VCLXDialog )

VCLXDialog::VCLXDialog()
    : VCLXWindow()
    , VCLXTopWindow_Base( true )
    , VCLXDialog_Base()
    , Bin()
    , bRealized( false )
    , bResizeSafeguard( false )
{
    DBG_CTOR( VCLXDialog, NULL );

/*        mxLayoutUnit = uno::Reference< awt::XLayoutUnit >( new LayoutUnit() );
          assert(mxLayoutUnit.is());*/
}

VCLXDialog::~VCLXDialog()
{
    DBG_DTOR( VCLXDialog, NULL );
}

Window* VCLXDialog::GetWindowImpl()
{
    return VCLXWindow::GetWindow();
}

::cppu::OInterfaceContainerHelper& VCLXDialog::GetTopWindowListenersImpl()
{
    return GetTopWindowListeners();
}

IMPLEMENT_2_FORWARD_XINTERFACE2( VCLXDialog, VCLXWindow, Bin, VCLXDialog_Base );

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXDialog, VCLXWindow, VCLXDialog_Base );

void SAL_CALL VCLXDialog::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    {
        SolarMutexGuard aGuard;

        ::com::sun::star::lang::EventObject aDisposeEvent;
        aDisposeEvent.Source = W3K_EXPLICIT_CAST (*this);
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

void VCLXDialog::resizedCb()
{
    queueResize();
}

void SAL_CALL VCLXDialog::allocateArea( const css::awt::Rectangle &rArea )
    throw (css::uno::RuntimeException)
{
    ::com::sun::star::awt::Size reqSize = Bin::getMinimumSize();
    reqSize.Height = getHeightForWidth( rArea.Width );

    if ( !bRealized )
    {
        setPosSize( 0, 0, reqSize.Width, reqSize.Height, ::com::sun::star::awt::PosSize::SIZE );
        bRealized = true;
        setVisible( true );
    }
    else
    {
        ::com::sun::star::awt::Size curSize = getSize();
        if ( reqSize.Width > curSize.Width )
            setPosSize( 0, 0, reqSize.Width, 0, ::com::sun::star::awt::PosSize::WIDTH );
        if ( reqSize.Height > curSize.Height )
            setPosSize( 0, 0, 0, reqSize.Height, ::com::sun::star::awt::PosSize::HEIGHT );
    }

    ::com::sun::star::awt::Size size = getSize();
    maAllocation.Width = size.Width;
    maAllocation.Height = size.Height;

    Bin::allocateArea( maAllocation );
}

void VCLXDialog::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    SolarMutexClearableGuard aGuard;

    switch ( _rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_RESIZE:
            resizedCb();
        default:
            aGuard.clear();
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
            break;
    }
}

void SAL_CALL VCLXDialog::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any &Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( GetWindow() )
    {
/*        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                VCLXWindow::setProperty( PropertyName, Value );
/*        }
*/
    }
}

::com::sun::star::uno::Any SAL_CALL VCLXDialog::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aReturn;
    if ( GetWindow() )
    {
/*
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
*/
                aReturn = VCLXWindow::getProperty( PropertyName );
/*
        }
*/
    }
    return aReturn;
}

void VCLXDialog::setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

void VCLXDialog::setHelpId( const rtl::OUString& rId ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetHelpId( rtl::OUStringToOString( rId, RTL_TEXTENCODING_UTF8 ) );
}

::rtl::OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aTitle;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        Dialog* pDlg = (Dialog*) GetWindow();
        Window* pParent = pDlg->GetWindow( WINDOW_PARENTOVERLAP );
        Window* pOldParent = NULL;
        if ( pParent && !pParent->IsReallyVisible() )
        {
            pOldParent = pDlg->GetParent();
            Window* pFrame = pDlg->GetWindow( WINDOW_FRAME );
            if ( pFrame != pDlg )
                pDlg->SetParent( pFrame );
        }
        nRet = pDlg->Execute();
        if ( pOldParent )
            pDlg->SetParent( pOldParent );
    }
    return nRet;
}

void VCLXDialog::endDialog( sal_Int32 nResult ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    if ( nResult == BUTTONID_HELP )
    {
        // UGH: c&p button.cxx
        ::Window* pFocusWin = Application::GetFocusWindow();
        if ( !pFocusWin )
            pFocusWin = GetWindow();

        HelpEvent aEvt( pFocusWin->GetPointerPosPixel(), HELPMODE_CONTEXT );
        pFocusWin->RequestHelp( aEvt );
        return;
    }

    Dialog* pDlg = (Dialog*) GetWindow();
    if ( pDlg )
        pDlg->EndDialog( nResult );
}

void VCLXDialog::endExecute() throw(::com::sun::star::uno::RuntimeException)
{
    endDialog( 0 );
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
