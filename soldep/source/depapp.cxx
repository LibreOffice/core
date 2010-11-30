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

// -----------------------------------------------------------------------
#include "depapp.hxx"
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/configurationkeys.hxx>

void MyApp::Main()
{
#if defined(WNT)
    pDebugFile = fopen( "fprintf.out", "w" );
#endif

    pMyApp = GetpApp();
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    pAppWindow = &aMainWin;
   // pToolBarFrame = new FloatingWindow( aMainWin, WB_STDWORK );
    //pToolBox = new ToolBox(pToolBarFrame,DtSodResId(TID_SOLDEP_MAIN));

    pSolDep = new SolDep( &aMainWin );
    pSolDep->Init();
    aMainWin.SetText( String::CreateFromAscii( SOLDEPL_NAME ));
    pSolDep->Hide();
    aMainWin.Show();
    Help aHelp;
    SetHelp(&aHelp);
    aHelp.EnableContextHelp();
    aHelp.EnableQuickHelp();
    Execute();
    delete pResMgr;
    delete pSolDep;
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )//,aToolBox( this, DtSodResId(TID_SOLDEP_MAIN)),
        // depper aTaskBarFrame(this, 0)
{
//depper    aTaskBarFrame.EnableAlwaysOnTop();

//depper    aMenuBar.InsertItem( 1, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Source" ) ) );
//depper    aMenuBar.InsertItem( 2, XubString( RTL_CONSTASCII_USTRINGPARAM( "~Exit" ) ) );
//depper    SetMenuBar( &aMenuBar );
//depper    aToolBox.SetPosSizePixel( Point( 0,0 ), Size( 1100,35 ));
//    aToolBox.Show();
}

// -----------------------------------------------------------------------

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    ((MyApp*)GetpApp())->GetSolDep()->Resize();
    WorkWindow::Resize();
}

SAL_IMPLEMENT_MAIN()
{
    //Reference< XMultiServiceFactory > xMS;

    // for this to work make sure an <appname>.ini file is available, you can just copy soffice.ini
    Reference< XComponentContext > xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();

    //xMS = cppu::createRegistryServiceFactory(
    //              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "applicat.rdb" ) ), sal_True );

    Reference< XMultiServiceFactory > xMS( xComponentContext->getServiceManager(), UNO_QUERY);

    InitVCL( xMS );
    ::comphelper::setProcessServiceFactory(xMS);
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > aArgs(2);
    aArgs[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY1_LOCAL ));
    aArgs[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( UCB_CONFIGURATION_KEY2_OFFICE ));

    ::ucbhelper::ContentBroker::initialize( xMS, aArgs );

    aMyApp.Main();
    DeInitVCL();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
