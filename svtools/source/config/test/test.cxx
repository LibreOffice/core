/*************************************************************************
 *
 *  $RCSfile: test.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-05-11 12:31:18 $
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

//_________________________________________________________________________________________________________________
//  switches
//  use it to enable test szenarios
//_________________________________________________________________________________________________________________

#define TEST_DYNAMICMENUOPTIONS

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include "dynamicmenuoptions.hxx"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/regpathhelper.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include <cppuhelper/bootstrap.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

/*
#ifndef _SVT_UNOIFACE_HXX
#include <svtools/unoiface.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
*/
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <stdio.h>

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl                       ;
using namespace ::osl                       ;
using namespace ::comphelper                ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::lang      ;
using namespace ::com::sun::star::beans     ;
using namespace ::com::sun::star::registry  ;

//_________________________________________________________________________________________________________________
//  defines
//_________________________________________________________________________________________________________________

#define ASCII( STEXT )                      OUString( RTL_CONSTASCII_USTRINGPARAM( STEXT ))

#define SERVICENAME_SIMPLEREGISTRY          OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.registry.SimpleRegistry" ))
#define SERVICENAME_NESTEDREGISTRY          OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.registry.NestedRegistry" ))

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

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
    OSL_ENSURE( sal_False, "Test was successful!\n" );
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

    OSL_ENSURE( sal_False, OUStringToOString( sOut.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ).getStr() );
}

//*****************************************************************************************************************
// create new uno servicemanager by using normall applicat.rdb and user.rdb of an office installation!
// Don't use this application at same time like the office!
//*****************************************************************************************************************
Reference< XMultiServiceFactory > TestApplication::getUNOServiceManager()
{
    // Algorithm: (create global manager one times only!)
    //      a)  get paths to user.rdb and applicat.rdb
    //      b)  open rdb files (if user.rdb not exist - create it)
    //      c)  initialize component context
    //      d)  set service manager of component context as static member

    static Reference< XMultiServiceFactory >* m_pManager = NULL;
    if( m_pManager == NULL )
    {
        // Global access, must be guarded (multithreading!).
        MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        try
        {
            // a) (getPath... comes from comphelper)
            OUString sNameUserRegistry      = getPathToUserRegistry()   ;
            OUString sNameSystemRegistry    = getPathToSystemRegistry() ;

            // b)
            Reference< XSimpleRegistry > xUserRegistry  ( ::cppu::createSimpleRegistry() );
            Reference< XSimpleRegistry > xSystemRegistry( ::cppu::createSimpleRegistry() );

            if  (
                ( xUserRegistry.is()            ==  sal_True    )   &&
                ( sNameUserRegistry.getLength() >   0           )
                )
            {
                xUserRegistry->open( sNameUserRegistry, sal_False, sal_True ); // try to open/create it writable ... (second param = sal_False)
                if( xUserRegistry->isValid() == sal_False )
                {
                    xUserRegistry->open( sNameUserRegistry, sal_True, sal_True ); // try to open/create it readonly ... (second param = sal_True)
                }
            }

            if  (
                ( xSystemRegistry.is()              ==  sal_True    )   &&
                ( sNameSystemRegistry.getLength()   >   0           )
                )
            {
                xSystemRegistry->open( sNameSystemRegistry, sal_True, sal_False );
            }

            if  (
                ( xSystemRegistry.is()          ==  sal_True    )   &&
                ( xSystemRegistry->isValid()    ==  sal_True    )
                )
            {
                Sequence< Any > lNestedParameters(1);
                lNestedParameters[0] <<= xSystemRegistry;
                if  (
                    ( xUserRegistry.is()            ==  sal_True    )   &&
                    ( xUserRegistry->isValid()      ==  sal_True    )
                    )
                {
                    lNestedParameters.realloc(2);
                    lNestedParameters[1] <<= xUserRegistry;
                }

                Reference < XSimpleRegistry > xNestedRegistry( ::cppu::createNestedRegistry() );
                Reference< XInitialization > xInit( xNestedRegistry, UNO_QUERY );
                xInit->initialize( lNestedParameters );

                // c)
                Reference< XComponentContext > xContext( ::cppu::bootstrap_InitialComponentContext( xNestedRegistry ) );
                // d)
                static Reference< XMultiServiceFactory > xManager( xContext->getServiceManager(), UNO_QUERY );
                m_pManager = &xManager;
            }
        }
        catch(...)
        {
            // If an exception is thrown - we set the return value to NULL!
            // Its neccessary, because it can be, that we have overwrite the default from beginning of this method with an "HALF-VALID" value!
            m_pManager = NULL;
        }
    }

    return *m_pManager;
}
