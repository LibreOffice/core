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
    aArgs[0] <<= rtl::OUString::createFromAscii( UCB_CONFIGURATION_KEY1_LOCAL );
    aArgs[1] <<= rtl::OUString::createFromAscii( UCB_CONFIGURATION_KEY2_OFFICE );

    ::ucbhelper::ContentBroker::initialize( xMS, aArgs );

    aMyApp.Main();
    DeInitVCL();
    return 0;
}
