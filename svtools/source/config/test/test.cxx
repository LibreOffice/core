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


//_________________________________________________________________________________________________________________
//  switches
//  use it to enable test scenarios
//_________________________________________________________________________________________________________________

#define TEST_DYNAMICMENUOPTIONS

#include <unotools/dynamicmenuoptions.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

using namespace ::rtl                       ;
using namespace ::osl                       ;
using namespace ::comphelper                ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::beans     ;
using namespace ::com::sun::star::registry  ;

#define ASCII( STEXT )                      OUString( RTL_CONSTASCII_USTRINGPARAM( STEXT ))

class TestApplication : public Application
{
    //*************************************************************************************************************
    // interface
    //*************************************************************************************************************
    public:
        void Main();

    //*************************************************************************************************************
    // test methods
    //*************************************************************************************************************
    private:
        void impl_testDynamicMenuOptions();

    //*************************************************************************************************************
    // helper methods
    //*************************************************************************************************************
    private:
        static Reference< XMultiServiceFactory > getUNOServiceManager();

    //*************************************************************************************************************
    // member
    //*************************************************************************************************************
    private:

};  //  class TestApplication

//_________________________________________________________________________________________________________________
//  global variables
//_________________________________________________________________________________________________________________

TestApplication aTestApplication ;

//_________________________________________________________________________________________________________________
//  main
//_________________________________________________________________________________________________________________

void TestApplication::Main()
{
    /**-***********************************************************************************************************
        initialize program
    **************************************************************************************************************/

    // Init global servicemanager and set it for external services.
    ::comphelper::setProcessServiceFactory( TestApplication::getUNOServiceManager() );
    // Control sucess of operation.
    OSL_ENSURE( !(::comphelper::getProcessServiceFactory()!=TestApplication::getUNOServiceManager()), "TestApplication::Main()\nGlobal servicemanager not right initialized.\n" );

    /**-***********************************************************************************************************
        test area
    **************************************************************************************************************/

    #ifdef TEST_DYNAMICMENUOPTIONS
        impl_testDynamicMenuOptions();
    #endif

//  Execute();
    OSL_FAIL( "Test was successful!\n" );
}

//*****************************************************************************************************************
// test configuration of dynamic menus "New" and "Wizard"
//*****************************************************************************************************************
void TestApplication::impl_testDynamicMenuOptions()
{
    SvtDynamicMenuOptions aCFG;

    // Test:
    //      read menus
    //      if( menus == empty )
    //      {
    //          fill it with samples
    //          read it again
    //      }
    //      output content

    Sequence< Sequence< PropertyValue > > lNewMenu    = aCFG.GetMenu( E_NEWMENU    );
    Sequence< Sequence< PropertyValue > > lWizardMenu = aCFG.GetMenu( E_WIZARDMENU );

    if( lNewMenu.getLength() < 1 )
    {
        aCFG.AppendItem( E_NEWMENU, ASCII("private:factory/swriter"), ASCII("new writer"), ASCII("icon_writer"), ASCII("_blank") );
        aCFG.AppendItem( E_NEWMENU, ASCII("private:factory/scalc"  ), ASCII("new calc"  ), ASCII("icon_calc"  ), ASCII("_blank") );
        aCFG.AppendItem( E_NEWMENU, ASCII("private:factory/sdraw"  ), ASCII("new draw"  ), ASCII("icon_draw"  ), ASCII("_blank") );

        lNewMenu = aCFG.GetMenu( E_NEWMENU );
    }

    if( lWizardMenu.getLength() < 1 )
    {
        aCFG.AppendItem( E_WIZARDMENU, ASCII("file://a"), ASCII("system file"), ASCII("icon_file"), ASCII("_self") );
        aCFG.AppendItem( E_WIZARDMENU, ASCII("ftp://b" ), ASCII("ftp host"   ), ASCII("icon_ftp" ), ASCII("_self") );
        aCFG.AppendItem( E_WIZARDMENU, ASCII("http://c"), ASCII("www"        ), ASCII("icon_www" ), ASCII("_self") );

        lWizardMenu = aCFG.GetMenu( E_WIZARDMENU );
    }

    sal_uInt32     nItemCount    ;
    sal_uInt32     nItem         ;
    sal_uInt32     nPropertyCount;
    sal_uInt32     nProperty     ;
    OUString       sPropertyValue;
    OUStringBuffer sOut( 5000 )  ;

    nItemCount = lNewMenu.getLength();
    for( nItem=0; nItem<nItemCount; ++nItem )
    {
        nPropertyCount = lNewMenu[nItem].getLength();
        for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            lNewMenu[nItem][nProperty].Value >>= sPropertyValue;

            sOut.appendAscii ( "New/"                            );
            sOut.append      ( (sal_Int32)nItem                  );
            sOut.appendAscii ( "/"                               );
            sOut.append      ( lNewMenu[nItem][nProperty].Name   );
            sOut.appendAscii ( " = "                             );
            sOut.append      ( sPropertyValue                    );
            sOut.appendAscii ( "\n"                              );
        }
    }

    sOut.appendAscii("\n--------------------------------------\n");

    nItemCount = lWizardMenu.getLength();
    for( nItem=0; nItem<nItemCount; ++nItem )
    {
        nPropertyCount = lNewMenu[nItem].getLength();
        for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            lWizardMenu[nItem][nProperty].Value >>= sPropertyValue;

            sOut.appendAscii ( "Wizard/"                         );
            sOut.append      ( (sal_Int32)nItem                  );
            sOut.appendAscii ( "/"                               );
            sOut.append      ( lNewMenu[nItem][nProperty].Name   );
            sOut.appendAscii ( " = "                             );
            sOut.append      ( sPropertyValue                    );
            sOut.appendAscii ( "\n"                              );
        }
    }

    OSL_FAIL( OUStringToOString( sOut.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr() );
}

//*****************************************************************************************************************
// create new uno servicemanager by using normall applicat.rdb and user.rdb of an office installation!
// Don't use this application at same time like the office!
//*****************************************************************************************************************
Reference< XMultiServiceFactory > TestApplication::getUNOServiceManager()
{
    static Reference< XMultiServiceFactory > smgr;
    if( ! smgr.is() )
    {
        Reference< XComponentContext > rCtx =
            cppu::defaultBootstrap_InitialComponentContext();
        smgr = Reference< XMultiServiceFactory > ( rCtx->getServiceManager() , UNO_QUERY );
    }
    return smgr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
