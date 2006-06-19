/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pamain.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:16:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>

#ifndef _TOOLS_TESTTOOLLOADER_HXX_
#include <tools/testtoolloader.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _VCL_UNOWRAP_HXX
#include <vcl/unowrap.hxx>
#endif
#ifndef _PAD_PADIALOG_HXX_
#include <padialog.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif

#ifndef _PADMIN_DESKTOPCONTEXT_HXX_
#include <desktopcontext.hxx>
#endif

#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include <cppuhelper/bootstrap.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif

#ifndef _UCBHELPER_CONFIGURATIONKEYS_HXX_
#include <ucbhelper/configurationkeys.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

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
    virtual USHORT  Exception( USHORT nError );
};

MyApp aMyApp;


// -----------------------------------------------------------------------

USHORT MyApp::Exception( USHORT nError )
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
        ::ucb::ContentBroker::initialize( xFactory, aArgs );

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
        BOOL bQuitApp;
        if( !InitAccessBridge( true, bQuitApp ) )
            if( bQuitApp )
                return;
    }

    // initialize test-tool library (if available)
    tools::InitTestToolLib();

    pPADialog = PADialog::Create( NULL , FALSE );
    Application::SetDisplayName( pPADialog->GetText() );
    pPADialog->SetIcon(501);
    pPADialog->Execute();
    delete pPADialog;

    tools::DeInitTestToolLib();

    /*
     *  clean up UCB
     */
    ::ucb::ContentBroker::deinitialize();

}
