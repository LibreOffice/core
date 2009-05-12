/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unodialog.cxx,v $
 * $Revision: 1.15 $
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
#include <tools/svwin.h>
#include <sal/main.h>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>


#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include <svtools/unoiface.hxx> // InitExtToolkit
#include <comphelper/processfactory.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/regpathhelper.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::connection;
using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;



::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > createApplicationServiceManager()
{
    try
    {
        ::rtl::OUString localRegistry = ::comphelper::getPathToUserRegistry();
        ::rtl::OUString systemRegistry = ::comphelper::getPathToSystemRegistry();

        Reference< XSimpleRegistry > xLocalRegistry( ::cppu::createSimpleRegistry() );
        Reference< XSimpleRegistry > xSystemRegistry( ::cppu::createSimpleRegistry() );
        if ( xLocalRegistry.is() && (localRegistry.getLength() > 0) )
        {
            try
            {
                xLocalRegistry->open( localRegistry, sal_False, sal_True);
            }
            catch ( InvalidRegistryException& )
            {
            }

            if ( !xLocalRegistry->isValid() )
                xLocalRegistry->open(localRegistry, sal_True, sal_True);
        }

        if ( xSystemRegistry.is() && (systemRegistry.getLength() > 0) )
            xSystemRegistry->open( systemRegistry, sal_True, sal_False);

        if ( (xLocalRegistry.is() && xLocalRegistry->isValid()) &&
             (xSystemRegistry.is() && xSystemRegistry->isValid()) )
        {
            Reference < XSimpleRegistry > xReg( ::cppu::createNestedRegistry() );
            Sequence< Any > seqAnys(2);
            seqAnys[0] <<= xLocalRegistry ;
            seqAnys[1] <<= xSystemRegistry ;
            Reference< XInitialization > xInit( xReg, UNO_QUERY );
            xInit->initialize( seqAnys );

            Reference< XComponentContext > xContext( ::cppu::bootstrap_InitialComponentContext( xReg ) );
            return Reference< XMultiServiceFactory >( xContext->getServiceManager(), UNO_QUERY );
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return ::cppu::createServiceFactory();
}


// -----------------------------------------------------------------------
void Main( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );

SAL_IMPLEMENT_MAIN()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMSF = createApplicationServiceManager();
    //SVInit( xMSF );
    ::Main( xMSF );
    //SVDeinit();
    return NULL;
}

/*
class MyApp : public Application
{
public:
    void        Main()
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMSF = createApplicationServiceManager();
        ::Main( xMSF );
    }
};

MyApp aMyApp;
*/


class MyWin : public WorkWindow
{
private:
    uno::Reference< awt::XView > mxView;

public:
    MyWin() : WorkWindow( NULL, WB_APP|WB_STDWORK ) {;}
    void        Paint( const Rectangle& r );
    void        SetXView(  uno::Reference< awt::XView > xV  ) { mxView = xV; }
};


// -----------------------------------------------------------------------

void Main( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMSF )
{
    ::comphelper::setProcessServiceFactory( xMSF );

    //uno::Reference< awt::XToolkit> xToolkit( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.ExtToolkit" ) ) ), uno::UNO_QUERY );
    uno::Reference< awt::XToolkit> xToolkit( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.Toolkit" ) ) ), uno::UNO_QUERY );

    // Create a DialogModel
    uno::Reference< container::XNameContainer > xC( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), uno::UNO_QUERY );
    uno::Reference< lang::XMultiServiceFactory >  xModFact( xC, uno::UNO_QUERY );

    // Create a ButtonModel
    uno::Reference< awt::XControlModel > xCtrl1( xModFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlButtonModel" ) ) ), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xPSet( xCtrl1, uno::UNO_QUERY );
    uno::Any aValue;
    aValue <<= (sal_Int32) 10;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 10;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 40;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 12;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Test!" ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Label" ) ), aValue );
    uno::Any aAny;
    aAny <<= xCtrl1;
    xC->insertByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Control1" ) ), aAny );

    uno::Reference< beans::XPropertySet > xDlgPSet( xC, uno::UNO_QUERY );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Test-Dialog" ) );
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), aValue );
    aValue <<= (sal_Int32) 200;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 200;
    xDlgPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );

    // Create a Dialog
    uno::Reference< awt::XControl > xDlg( xMSF->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialog" ) ) ), uno::UNO_QUERY );
    uno::Reference< awt::XControlModel > xDlgMod( xC, uno::UNO_QUERY );
    xDlg->setModel( xDlgMod );

     // Create a EditModel
    uno::Reference< awt::XControlModel > xCtrl2( xModFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlEditModel" ) ) ), uno::UNO_QUERY );
    xPSet = uno::Reference< beans::XPropertySet >( xCtrl2, uno::UNO_QUERY );
    aValue <<= (sal_Int32) 10;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );
    aValue <<= (sal_Int32) 40;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ), aValue );
    aValue <<= (sal_Int32) 80;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Width" ) ), aValue );
    aValue <<= (sal_Int32) 12;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Height" ) ), aValue );
    aValue <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Text..." ) );
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Text" ) ), aValue );
    aAny <<= xCtrl2;
    xC->insertByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Control2" ) ), aAny );

    // test if listener works...
    aValue <<= (sal_Int32) 20;
    xPSet->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ), aValue );

    MyWin * pWindow;
    ::osl::Guard< vos::IMutex > aVclGuard( Application::GetSolarMutex() );
    pWindow = new MyWin();
    pWindow->Show();

    xDlg->setDesignMode( sal_True );

    uno::Reference< awt::XWindow > xWindow( xDlg, uno::UNO_QUERY );
    xWindow->setVisible( sal_False );

    xDlg->createPeer( xToolkit, pWindow->GetComponentInterface() );

    uno::Reference< awt::XView > xView( xDlg, uno::UNO_QUERY );
    pWindow->SetXView( xView );

    uno::Reference< awt::XDialog > xD( xDlg, uno::UNO_QUERY );

    //static BOOL bExecute = FALSE;
    //if ( bExecute )
        xD->execute();
    //Execute();
    Reference< XComponent > xDT( xD, uno::UNO_QUERY );
    xDT->dispose();
    delete pWindow;

    Reference< XComponent > xT( xToolkit, uno::UNO_QUERY );
    xT->dispose();

    Reference< beans::XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY );
    if (xProps.is())
    {
        try
        {
            Reference< lang::XComponent > xComp;
            if (xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xComp)
            {
                xComp->dispose();
            }
        }
        catch (beans::UnknownPropertyException &)
        {
        }
    }
}

void MyWin::Paint( const Rectangle& r )
{
    static BOOL bDraw = TRUE;
    if ( bDraw && mxView.is() )
        mxView->draw( 50, 50 );
}

