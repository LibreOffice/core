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



#include <stdio.h>
#include <unistd.h>

#include "tools/testtoolloader.hxx"

#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/unowrap.hxx"

#include "padialog.hxx"
#include "helper.hxx"
#include "desktopcontext.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "comphelper/processfactory.hxx"
#include "ucbhelper/contentbroker.hxx"
#include "ucbhelper/configurationkeys.hxx"
#include "unotools/configmgr.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"

using namespace padmin;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace comphelper;

// -----------------------------------------------------------------------

class MyApp : public Application
{
public:
    void            Main();
    virtual sal_uInt16  Exception( sal_uInt16 nError );

    static void ReadStringHook( String& );
};

MyApp aMyApp;

void MyApp::ReadStringHook( String& rStr )
{
    static String maProduct;
    if( ! maProduct.Len() )
    {
        Any aRet = utl::ConfigManager::GetDirectConfigProperty( utl::ConfigManager::PRODUCTNAME );
        OUString aProd;
        aRet >>= aProd;
        maProduct = String( aProd );
    }
    rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", maProduct );
};


// -----------------------------------------------------------------------

sal_uInt16 MyApp::Exception( sal_uInt16 nError )
{
    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_RSCNOTLOADED:
            Abort( String::CreateFromAscii( "Error: could not load language resources.\nPlease check your installation.\n" ) );
            break;
    }
    return 0;
}

void MyApp::Main()
{
    PADialog* pPADialog;

    EnableAutoHelpId();

    //-------------------------------------------------
    // create the global service-manager
    //-------------------------------------------------
    Reference< XMultiServiceFactory > xFactory;
    try
    {
        Reference< XComponentContext > xCtx = defaultBootstrap_InitialComponentContext();
        xFactory = Reference< XMultiServiceFactory >(  xCtx->getServiceManager(), UNO_QUERY );
        if( xFactory.is() )
            setProcessServiceFactory( xFactory );
    }
    catch( com::sun::star::uno::Exception& rExc)
    {
    }

    if( ! xFactory.is() )
    {
        fprintf( stderr, "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }

    // Detect desktop environment - need to do this as early as possible
    com::sun::star::uno::setCurrentContext(
        new DesktopContext( com::sun::star::uno::getCurrentContext() ) );

    /*
     *  Create UCB.
     */
    Sequence< Any > aArgs( 2 );
    aArgs[ 0 ] <<= OUString::createFromAscii( UCB_CONFIGURATION_KEY1_LOCAL );
    aArgs[ 1 ] <<= OUString::createFromAscii( UCB_CONFIGURATION_KEY2_OFFICE );
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bSuccess =
#endif
        ::ucbhelper::ContentBroker::initialize( xFactory, aArgs );

#if OSL_DEBUG_LEVEL > 1
    if ( !bSuccess )
    {
        fprintf( stderr, "Error creating UCB, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }
#endif

    /*
     * Initialize the Java UNO AccessBridge if accessibility is turned on
     */

    if( Application::GetSettings().GetMiscSettings().GetEnableATToolSupport() )
    {
        sal_Bool bQuitApp (sal_False);
        if( !InitAccessBridge( true, bQuitApp ) )
            if( bQuitApp )
                return;
    }

    // initialize test-tool library (if available)
    tools::InitTestToolLib();

    ResMgr::SetReadStringHook( MyApp::ReadStringHook );

    pPADialog = PADialog::Create( NULL , sal_False );
    Application::SetDisplayName( pPADialog->GetText() );
    pPADialog->SetIcon(501);
    pPADialog->Execute();
    delete pPADialog;

    tools::DeInitTestToolLib();

    /*
     *  clean up UCB
     */
    ::ucbhelper::ContentBroker::deinitialize();

}
