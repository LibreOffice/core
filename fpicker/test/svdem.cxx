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

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <osl/file.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <rtl/ustring>
#include <sal/types.h>
#include <osl/diagnose.h>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>

#include <sal/main.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include "../source/office/iodlg.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


// Forward declaration
void Main();

SAL_IMPLEMENT_MAIN()
{
    Reference< XMultiServiceFactory > xMS = cppu::createRegistryServiceFactory( OUString( "applicat.rdb" ), sal_True );

    css::uno::Reference< css::uno::XComponentContext > xComponentContext = cppu::defaultBootstrap_InitialComponentContext();
    css::uno::Reference< css::lang::XMultiServiceFactory> xMSch;
    xMSch.set(xComponentContext->getServiceManager(), css::uno::UNO_QUERY);
    comphelper::setProcessServiceFactory(xMSch);

    comphelper::setProcessServiceFactory(xMS);

    InitVCL();
    ::Main();
    DeInitVCL();

    return 0;
}



class MyWin : public WorkWindow
{
public:
                MyWin( vcl::Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();

private:
    OKButton    aOKBtn;

                    DECL_LINK( Test, PushButton* );

};



void Main()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText(OUString("VCL - Workbench"));
    aMainWin.Show();

    Application::Execute();
}



MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle )
    :WorkWindow( pParent, nWinStyle )
    ,aOKBtn(this)
{
    aOKBtn.SetPosSizePixel( Point( 10, 10 ), Size( 140, 140 ) );
    aOKBtn.SetClickHdl( LINK( this, MyWin, Test ) );
    aOKBtn.Show();
}



void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}



void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}



void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}



void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}



void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}



void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}



void MyWin::Resize()
{
    WorkWindow::Resize();
}



IMPL_LINK_TYPED( MyWin, Test, PushButton*, pBtn, void )
{
printf("Test\n");
    if ( pBtn == &aOKBtn )
    {
        ScopedVclPtrInstance<SvtFileDialog> pDlg(this,SFXWB_PATHDIALOG);
        pDlg->Execute();
        printf("ok\n");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
