/*************************************************************************
 *
 *  $RCSfile: controls.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-15 11:44:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#define NOOLDSV

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
// #include <unoctrl2.hxx>

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

        // Printer Testen...
/*
        XServiceManagerRef xProv = getGlobalServiceManager();
        XServiceProviderRef xSSI = xProv->queryServiceProvider( L"stardiv.vcl.PrinterServer" );
        XPrinterServerRef xPrinterServer( xSSI->createInstance(), USR_QUERY );

        Sequence< UString > aPrinterNames = xPrinterServer->getPrinterNames();
        USHORT nPrinters = aPrinterNames.getLen();
        String aInfo( "Printers: " );
        aInfo += nPrinters;
        if ( nPrinters )
        {
            for ( USHORT n = 0; n < nPrinters; n++ )
            {
                aInfo += '\n';
                aInfo += OUStringToString( aPrinterNames.getConstArray()[n], CHARSET_SYSTEM );
            }

            XPrinterRef xPrinter = xPrinterServer->createPrinter( aPrinterNames.getConstArray()[0] );
            xPrinter->start( L"UNOPrinterTest", 1, TRUE );
            XDeviceRef xDev = xPrinter->startPage();
            XGraphicsRef xGraphics = xDev->createGraphics();
            xGraphics->drawText( 200, 200, L"Printed with UNO"  );
            xPrinter->endPage();
            xPrinter->end();
        }
        InfoBox( this, aInfo ).Execute();
*/
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

/*

            // ListBox...
            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.ListBox" );
            xModel = (XInterface*)xModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );

            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            Sequence<UString> aSeq( 7 );
            aSeq.getArray()[0] = L"Item1";
            aSeq.getArray()[1] = L"Item2";
            aSeq.getArray()[2] = L"Item3";
            aSeq.getArray()[3] = L"Item4";
            aSeq.getArray()[4] = L"Item5";
            aSeq.getArray()[5] = L"Item6";
            aSeq.getArray()[6] = L"Item7";
            xPSet->setPropertyValue( L"StringItemList", UsrAny( &aSeq, Sequence<UString>::getReflection() ) );
            xPSet->setPropertyValue( L"LineCount", UsrAny_UINT16( 4 ) );
            xPSet->setPropertyValue( L"Dropdown", UsrAny( (BOOL)TRUE ) );

            XServiceProviderRef xListBoxCtrlProv = xProv->queryServiceProvider( L"stardiv.vcl.control.ListBox" );
            XControlRef xListBox1 = (XControl*)xListBoxCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xListBox1->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xListBoxCmp1 = (XWindow*)xListBox1->queryInterface( XWindow::getSmartUik() );
            xListBoxCmp1->setPosSize( 20, 250, 200, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xListBox1 );

            // FixedText...
            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.FixedText" );
            xModel = (XInterface*)xModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            DBG_ASSERT( xModel, "No Model!" );

            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Label:" ) );
             xPSet->setPropertyValue( L"BackgroundColor", UsrAny( (UINT32)0x00888888) );

            Font_Attribs aFontAttrs;
            aFontAttrs.Italic = ITALIC_NORMAL;
            xPSet->setPropertyValue( L"Font_Attribs", UsrAny( &aFontAttrs, Font_Attribs_getReflection() ) );

            XPropertyStateRef xState = (XPropertyState*)xPSet->queryInterface( XPropertyState::getSmartUik() );
            xState->getPropertyState( L"Font_Attribs" );
            xState->getPropertyState( L"Font_Size" );

            XServiceProviderRef xFixedTextCtrlProv = xProv->queryServiceProvider( L"stardiv.vcl.control.FixedText" );
            XControlRef xFixedText1 = (XControl*)xFixedTextCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            DBG_ASSERT( xFixedText1, "No FixedText!" );
            xFixedText1->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xFixedTextCmp1 = (XWindow*)xFixedText1->queryInterface( XWindow::getSmartUik() );
            xFixedTextCmp1->setPosSize( 20, 20, 120, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xFixedText1 );

            // TabTest...
            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.GroupBox" );
            xModel = (XInterface*)xModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Radio-Test:" ) );
            XServiceProviderRef xGroupBoxCtrlProv = xProv->queryServiceProvider( L"stardiv.vcl.control.GroupBox" );
            XControlRef xGroupBox1 = (XControl*)xGroupBoxCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xGroupBox1->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xGroupBoxCmp1 = (XWindow*)xGroupBox1->queryInterface( XWindow::getSmartUik() );
            xGroupBoxCmp1->setPosSize( 30, 410, 100, 130, PosSize_POSSIZE );
            xCont->addControl( L"", xGroupBox1 );

            XServiceProviderRef xRadioButtonModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.RadioButton" );
            XServiceProviderRef xRadioButtonCtrlProv = xProv->queryServiceProvider( L"stardiv.vcl.control.RadioButton" );

            xModel = (XInterface*)xRadioButtonModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            XControlRef xT1 = (XControl*)xRadioButtonCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xT1->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xTC1 = (XWindow*)xT1->queryInterface( XWindow::getSmartUik() );
            xTC1->setPosSize( 40, 430, 80, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xT1 );
            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Radio1" ) );
            xPSet->setPropertyValue( L"State", UsrAny_UINT16( 1 ) );

            xModel = (XInterface*)xRadioButtonModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            XControlRef xT2 = (XControl*)xRadioButtonCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xT2->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xTC2 = (XWindow*)xT2->queryInterface( XWindow::getSmartUik() );
            xTC2->setPosSize( 40, 470, 80, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xT2 );
            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Radio2" ) );

            xModel = (XInterface*)xRadioButtonModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            XControlRef xT3 = (XControl*)xRadioButtonCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xT3->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xTC3 = (XWindow*)xT3->queryInterface( XWindow::getSmartUik() );
            xTC3->setPosSize( 40, 510, 80, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xT3 );
            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Radio3" ) );

            xModel = (XInterface*)xRadioButtonModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            XControlRef xT4 = (XControl*)xRadioButtonCtrlProv->createInstance()->queryInterface( XControl::getSmartUik() );
            xT4->setModel( (XControlModel*)xModel->queryInterface( XControlModel::getSmartUik() ) );
            XWindowRef xTC4 = (XWindow*)xT4->queryInterface( XWindow::getSmartUik() );
            xTC4->setPosSize( 40, 550, 80, 20, PosSize_POSSIZE );
            xCont->addControl( L"", xT4 );
            xPSet = (XPropertySet*)xModel->queryInterface( XPropertySet::getSmartUik() );
            xPSet->setPropertyValue( L"Label", UsrAny( L"Radio4 - no" ) );
            xPSet->setPropertyValue( L"Tabstop", UsrAny( (BOOL)TRUE ) );

            // TabController:
            xModelProv = xProv->queryServiceProvider( L"stardiv.vcl.controlmodel.TabController" );
            xModel = (XInterface*)xModelProv->createInstance()->queryInterface( XInterface::getSmartUik() );
            XTabControllerModelRef xTCModel = (XTabControllerModel*)xModel->queryInterface( XTabControllerModel::getSmartUik() );

            XServiceProviderRef xTCProv = xProv->queryServiceProvider( L"stardiv.vcl.control.TabController" );
            XTabControllerRef xTC = (XTabController*)xTCProv->createInstance()->queryInterface( XTabController::getSmartUik() );
            xTC->setModel( (XTabControllerModel*)xModel->queryInterface( XTabControllerModel::getSmartUik() ) );
            XUnoControlContainerRef xUCC = (XUnoControlContainer*)xCont->queryInterface( XUnoControlContainer::getSmartUik() );
            xUCC->addTabController( xTC );

            Sequence<XControlModelRef> aControls( 5 );
            aControls.getArray()[0] = xGroupBox1->getModel();
            aControls.getArray()[1] = xT1->getModel();
            aControls.getArray()[2] = xT3->getModel();
            aControls.getArray()[3] = xT2->getModel();
            aControls.getArray()[4] = xT4->getModel();
            xTCModel->setControls( aControls );

            Sequence<XControlModelRef> aGroup( 3 );
            aGroup.getArray()[0] = xT1->getModel();
            aGroup.getArray()[1] = xT3->getModel();
            aGroup.getArray()[2] = xT2->getModel();
            xTCModel->setGroup( aGroup, L"test" );
*/
            // Container anzeigen...
            // Als Child zu diesem Fenster
            xContControl->createPeer( XToolkitRef(), GetComponentInterface( TRUE ) );

            XDeviceRef xD( xContControl->getPeer(), USR_QUERY );
            xG = xD->createGraphics();
            XViewRef xV ( xDrawCtrl, USR_QUERY );
            xV->setGraphics( xG );

//      ((UnoControl*)(XControl*)xNumField))->updateFromModel();

/*
            // TEST:
            WindowDecriptor aDescr;
            aDescr.ComponentServiceName = "window";
            aDescr.Type = VCLCOMPONENTTYPE_CONTAINER;
            aDescr.Parent = GetComponentInterface( TRUE );
            aDescr.WindowAttributes = WA_SHOW|WA_BORDER;
            aDescr.Bounds = Rectangle( Point( 500, 50 ), Size( 300, 200 ) );
            XVclWindowPeerRef xSPWin = xToolkit->createComponent( aDescr );

            WindowDecriptor aDescr2;
            aDescr2.ComponentServiceName = "scrollbar";
            aDescr2.Type = VCLCOMPONENTTYPE_SIMPLE;
            aDescr2.Parent = xSPWin;
            aDescr2.WindowAttributes = WA_SHOW|WA_BORDER|WA_VSCROLL;
            aDescr2.Bounds = Rectangle( Point( 250, 0 ), Size( 50, 200 ) );
            XVclWindowPeerRef xSB = xToolkit->createComponent( aDescr2 );
*/
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

