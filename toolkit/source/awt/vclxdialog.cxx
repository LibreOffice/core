/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "vclxdialog.hxx"

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>

#include <cppuhelper/typeprovider.hxx>

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>

#ifdef WNT
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#elif defined ( QUARTZ )
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

vos::IMutex& VCLXDialog::GetMutexImpl()
{
    return VCLXWindow::GetMutex();
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
        ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OClearableGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

void VCLXDialog::setHelpId( const rtl::OUString& rId ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetHelpId( rtl::OUStringToOString( rId, RTL_TEXTENCODING_UTF8 ) );
}

::rtl::OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aTitle;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

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
    ::vos::OGuard aGuard( GetMutex() );

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
