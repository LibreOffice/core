/*************************************************************************
 *
 *  $RCSfile: unodialog.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: mm $ $Date: 2001-02-22 18:05:32 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#include <sal/main.h>

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif


#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include <svtools/unoiface.hxx> // InitExtToolkit

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/servicefactory.hxx>
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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xReturn = ::cppu::createServiceFactory();

    try
    {

    if ( xReturn.is() )
    {
        Reference< XInitialization > xInit ( xReturn, UNO_QUERY ) ;
        if ( xInit.is() )
        {
            ::rtl::OUString localRegistry = ::comphelper::getPathToUserRegistry();
            ::rtl::OUString systemRegistry = ::comphelper::getPathToSystemRegistry();

            Reference< XSimpleRegistry > xLocalRegistry   ( xReturn->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.registry.SimpleRegistry") ) ), UNO_QUERY );
            Reference< XSimpleRegistry > xSystemRegistry ( xReturn->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry") ) ), UNO_QUERY );
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
                Sequence< Any > seqAnys(2);
                seqAnys[0] <<= xLocalRegistry ;
                seqAnys[1] <<= xSystemRegistry ;

                Reference < XSimpleRegistry > xReg(
                            xReturn->createInstanceWithArguments(
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.NestedRegistry")), seqAnys ), UNO_QUERY );

                seqAnys = Sequence< Any >( 1 );
                seqAnys[0] <<= xReg;
                if ( xReg.is() )
                    xInit->initialize( seqAnys );
            }
        }
        else
        {
            xReturn = ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >();
        }
    }
    }

    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return xReturn ;
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
    //InitExtVclToolkit();
    //Application::RegisterUnoServices();

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
}

void MyWin::Paint( const Rectangle& r )
{
    static BOOL bDraw = TRUE;
    if ( bDraw && mxView.is() )
        mxView->draw( 50, 50 );
}

