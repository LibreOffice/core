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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <tools/debug.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>


#include <stardiv/uno/awt/window.hxx>
#include <stardiv/uno/awt/vclwin.hxx>
#include <stardiv/uno/awt/printer.hxx>
#include <stardiv/uno/repos/serinfo.hxx>
#include <stardiv/uno/lang/factory.hxx>


#include <unoctrl.hxx>

#include <svtools/unoiface.hxx> // InitExtToolkit

UsrAny UsrAny_UINT16( UINT16 n )
{
    UsrAny aVal;
    aVal.setUINT16( n );
    return aVal;
}

// -----------------------------------------------------------------------

class MyApp : public Application
{
public:
    void        Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
    XControlRef             xCtrl;
    XPropertySetRef         xCtrlModel;
    XControlContainerRef    xCont;
    XPropertySetRef         xContModel;
    XControlRef             xDrawCtrl;
    XGraphicsRef            xG;

public:
                MyWin( Window* pParent, WinBits nWinStyle );
                ~MyWin();

    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        Resize();
    void        Paint( const Rectangle& r );
};

// -----------------------------------------------------------------------

void MyApp::Main()
{
    createAndSetDefaultServiceManager();

    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( "Uno-Controls - Workbench" );
    aMainWin.Show();
    Execute();
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
}

// -----------------------------------------------------------------------
MyWin::~MyWin()
{
    XComponentRef xC1( xCont, USR_QUERY );
    xC1->dispose();
    XComponentRef xC2( xContModel, USR_QUERY );
    xC2->dispose();
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.GetClicks() == 2 )
    {
        XViewRef xV( xDrawCtrl, USR_QUERY );
        XWindowRef xC( xDrawCtrl, USR_QUERY );
        xV->draw( xC->getPosSize().Left(), xC->getPosSize().Top() );
    }
    else if( rMEvt.GetClicks() == 1 )
    {
        if( xContModel )
        {
            static INT32 nColor = 0x001788ab;
             xContModel->setPropertyValue( L"BackgroundColor", UsrAny( nColor ) );
            nColor += 0x00111111;
            nColor &= 0xFFFFFF;
        }
        else
        {
            XServiceManagerRef xProv = getGlobalServiceManager();

            XMultiServiceFactoryRef xMSF = NAMESPACE_USR( getProcessServiceManager )();

            XServiceProviderRef xSSI = xProv->queryServiceProvider( L"stardiv.vcl.VclToolkit" );
            XToolkitRef xToolkit( xMSF->createInstance( L"stardiv.vcl.VclToolkit" ), USR_QUERY );
            DBG_ASSERT( xToolkit, "No Toolkit!" );

            // Uno Container + Model erzeugen
            XServiceProviderRef xModelProv; // = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.ControlContainer" );
            XInterfaceRef xCMRef( xMSF->createInstance(L"stardiv.vcl.controlmodel.ControlContainer") );
            xCMRef->queryInterface( XPropertySet::getSmartUik(), xContModel );
             xContModel->setPropertyValue( L"Border", UsrAny_UINT16( 1 ) );
             xContModel->setPropertyValue( L"BackgroundColor", UsrAny( (UINT32)0x00CCCCCC) );

            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.control.ControlContainer" );
            XInterfaceRef xCCRef( xModelProv->createInstance(), USR_QUERY );
            xCCRef->queryInterface( XControlContainer::getSmartUik(), xCont );

            XControlModelRef xCM( xContModel, USR_QUERY );
            XControlRef xContControl( xCont, USR_QUERY );
            xContControl->setModel( xCM );
            XWindowRef xContComp( xCont, USR_QUERY );
            xContComp->setPosSize( 0, 0, 500, 600, PosSize_POSSIZE );

            // Zwei EditControls auf einem Model...
//          xModelProv = xProv->queryServiceProvider( L"stardiv.uno.awt.UnoControlEditModel" );
            XInterfaceRef xModel( xMSF->createInstance(L"stardiv.uno.awt.UnoControlEditModel") );
            DBG_ASSERT( xModel, "No Model!" );

            XPropertySetRef xPSet( xModel, USR_QUERY );
            xPSet->setPropertyValue( L"Text", UsrAny( UString( L"Hallo!" ) ) );
            xPSet->setPropertyValue( L"FontName", UsrAny( UString( L"Times New Roman" ) ) );
//          xPSet->setPropertyValue( L"FontWeight", UsrAny( 200 ) );

            XServiceProviderRef xEditCtrlProv = xProv->queryServiceProvider( L"stardiv.uno.awt.UnoControlEdit" );
            // Edit1
            XControlRef xEdit1( xMSF->createInstance(L"stardiv.uno.awt.UnoControlEdit"), USR_QUERY );
            xModel->queryInterface( XControlModel::getSmartUik(), xCM );
            xEdit1->setModel( xCM );
            XWindowRef xEditCmp1( xEdit1, USR_QUERY );
            xEditCmp1->setPosSize( 50, 50, 100, 60, PosSize_POSSIZE );
            xCont->addControl( L"", xEdit1 );
            // Edit2
            XControlRef xEdit2( xMSF->createInstance(L"stardiv.uno.awt.UnoControlEdit"), USR_QUERY );
            xModel->queryInterface( XControlModel::getSmartUik(), xCM );
            xEdit2->setModel( xCM );
            XWindowRef xEditCmp2( xEdit2, USR_QUERY );
            XLayoutConstrainsRef xL( xEdit2, USR_QUERY );
            if ( xL.is() )
            {
                Size aSz = xL->getPreferredSize();
                xEditCmp2->setPosSize( 100, 180, aSz.Width(), aSz.Height(), PosSize_POSSIZE );
            }
            else
                xEditCmp2->setPosSize( 100, 180, 100, 40, PosSize_POSSIZE );
            xCont->addControl( L"", xEdit2 );

            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.NumericField" );
            XInterfaceRef xNumModel( xModelProv->createInstance(), USR_QUERY );
            DBG_ASSERT( xNumModel, "No Model!" );


            XServiceProviderRef xNumFieldProv = xProv->queryServiceProvider( L"stardiv.vcl.control.NumericField" );
            XControlRef xNumField( xNumFieldProv->createInstance(), USR_QUERY );
            xNumModel->queryInterface( XControlModel::getSmartUik(), xCM );
            xNumField->setModel( xCM );
            XWindowRef xNumFieldWin( xNumField, USR_QUERY );
            xNumFieldWin->setPosSize( 50, 250, 100, 60, PosSize_POSSIZE );
            xCont->addControl( L"", xNumField );

            XMultiPropertySetRef xPSet2( xNumModel, USR_QUERY );
            Sequence<UString> Names( 2 );
            Names.getArray()[0] = L"Value";
            Names.getArray()[1] = L"ValueMin";
            Sequence<UsrAny> Values( 2 );
            Values.getArray()[0] = UsrAny( -2000000.0 );
            Values.getArray()[1] = UsrAny( -2000000.0 );
            xPSet2->setPropertyValues( Names, Values );


            // Button...
            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.Button" );
            xModelProv->createInstance()->queryInterface( XInterface::getSmartUik(), xModel );
            DBG_ASSERT( xModel, "No Model!" );

            xModel->queryInterface( XPropertySet::getSmartUik(), xPSet );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Press!" ) );

            XServiceProviderRef xButtonCtrlProv = xProv->queryServiceProvider( L"stardiv.vcl.control.Button" );
            XControlRef xButton1( xButtonCtrlProv->createInstance(), USR_QUERY );
            xModel->queryInterface( XControlModel::getSmartUik(), xCM );
            xButton1->setModel( xCM );
            XWindowRef xButtonCmp1( xButton1, USR_QUERY );
            xButtonCmp1->setPosSize( 170, 80, 80, 80, PosSize_POSSIZE );
            // Unsichtbar, nur im MBDown mit DrawRoutine bei Doppel-Klick...
            xButtonCmp1->setVisible( FALSE );
            xDrawCtrl = xButton1;

            xCont->addControl( L"", xButton1 );

            // Container anzeigen...
            // Als Child zu diesem Fenster
            xContControl->createPeer( XToolkitRef(), GetComponentInterface( TRUE ) );

            XDeviceRef xD( xContControl->getPeer(), USR_QUERY );
            xG = xD->createGraphics();
            XViewRef xV ( xDrawCtrl, USR_QUERY );
            xV->setGraphics( xG );

        }
        return;
    }
    WorkWindow::MouseButtonDown( rMEvt );
}


// -----------------------------------------------------------------------

void MyWin::Resize()
{
    WorkWindow::Resize();
}

void MyWin::Paint( const Rectangle& r )
{
    // Muss ueber PaintListener geschehen...
    if ( xDrawCtrl.is() )
    {
        XViewRef xV( xDrawCtrl, USR_QUERY );
        XWindowRef xC( xDrawCtrl, USR_QUERY );
        xV->draw( xC->getPosSize().Left(), xC->getPosSize().Top() );
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
