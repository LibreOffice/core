/*************************************************************************
 *
 *  $RCSfile: appinit.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cd $ $Date: 2001-08-21 16:00:55 $
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

#include "appinit.hxx"
#include "cmdlineargs.hxx"
#include "officeacceptthread.hxx"

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_BOOTSTRAP_HXX_
#include <cppuhelper/bootstrap.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COMPHELPER_REGPATHHELPER_HXX_
#include <comphelper/regpathhelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _UCBHELPER_CONFIGURATIONKEYS_HXX_
#include <ucbhelper/configurationkeys.hxx>
#endif

#include <cppuhelper/bootstrap.hxx>

#include <rtl/logfile.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/svapp.hxx>
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif

#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))
#define DEFINE_CONST_OUSTRING(CONSTASCII)       OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

#define DESKTOP_TEMPDIRNAME                     "soffice.tmp"

using namespace rtl;
using namespace vos;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;

extern desktop::CommandLineArgs*        GetCommandLineArgs();
extern desktop::OOfficeAcceptorThread*  pOfficeAcceptThread;

static String aCurrentTempURL;
static String aCurrentTempBase;


// -----------------------------------------------------------------------------

static bool configureUcb(bool bServer, rtl::OUString const & rPortalConnect)
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (sb93797) ::configureUcb" );
    Reference< XMultiServiceFactory >
        xServiceFactory( comphelper::getProcessServiceFactory() );
    if (!xServiceFactory.is())
    {
        DBG_ERROR("configureUcb(): No XMultiServiceFactory");
        return false;
    }

    rtl::OUString aPipe;
    vos::OSecurity().getUserIdent(aPipe);

    rtl::OUStringBuffer aPortal;
    if (rPortalConnect.getLength() != 0)
    {
        aPortal.append(sal_Unicode(','));
        aPortal.append(rPortalConnect);
    }

    Sequence< Any > aArgs(6);
    aArgs[0]
        <<= rtl::OUString::createFromAscii(bServer ?
                                               UCB_CONFIGURATION_KEY1_SERVER :
                                               UCB_CONFIGURATION_KEY1_LOCAL);
    aArgs[1]
        <<= rtl::OUString::createFromAscii(UCB_CONFIGURATION_KEY2_OFFICE);
    aArgs[2] <<= rtl::OUString::createFromAscii("PIPE");
    aArgs[3] <<= aPipe;
    aArgs[4] <<= rtl::OUString::createFromAscii("PORTAL");
    aArgs[5] <<= aPortal.makeStringAndClear();

    return ::ucb::ContentBroker::initialize( xServiceFactory, aArgs ) != false;
}


Reference< XMultiServiceFactory > createApplicationServiceManager()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createApplicationServiceManager" );

    try
    {
        Reference<XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
        Reference<XMultiServiceFactory> xMS(xComponentContext->getServiceManager(), UNO_QUERY);

        return xMS;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return Reference< XMultiServiceFactory >();
}

void destroyApplicationServiceManager( Reference< XMultiServiceFactory >& xSMgr )
{
    Reference< XPropertySet > xProps( xSMgr, UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            Reference< XComponent > xComp;
            if (xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComp )
            {
                xComp->dispose();
            }
        }
        catch ( UnknownPropertyException& )
        {
        }
    }
}

void registerServices( Reference< XMultiServiceFactory >& xSMgr )
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::registerServices" );

    // read command line parameters
    ::rtl::OUString conDcp;
    ::rtl::OUString aClientDisplay;
    ::rtl::OUString aUserDir;
    ::rtl::OUString aTmpString;
    sal_Bool        bHeadlessMode = sal_False;

    // interpret command line arguments
    CommandLineArgs* pCmdLine = GetCommandLineArgs();

    // read accept string from configuration
    if ( !Application::IsRemoteServer() )
        conDcp = SvtStartOptions().GetConnectionURL();

    if ( pCmdLine->GetAcceptString( aTmpString ))
        conDcp = aTmpString;
    pCmdLine->GetUserDir( aUserDir );

    if ( Application::IsRemoteServer() )
    {
        bHeadlessMode   = pCmdLine->IsHeadless();
        pCmdLine->GetClientDisplay( aClientDisplay );
    }

    if ( bHeadlessMode )
        Application::EnableHeadlessMode();

    if ( conDcp.getLength() > 0 )
    {
        // accept incoming connections (scripting and one rvp)
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::OOfficeAcceptorThread::OOfficeAcceptorThread" );
        pOfficeAcceptThread = new OOfficeAcceptorThread( xSMgr, conDcp, bHeadlessMode, aClientDisplay, aUserDir );
        pOfficeAcceptThread->create();
    }

    // improves parallel processing on Sun ONE Webtop
    // servicemanager up -> copy user installation
    if ( Application::IsRemoteServer() )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) createInstance com.sun.star.portal.InstallUser" );
        Any aAny;
        Reference <XInterface> xRef =  xSMgr->createInstanceWithArguments(
            OUString::createFromAscii( "com.sun.star.portal.InstallUser" ),
            Sequence<Any>( &aAny, 1 ) );
    }

    ::rtl::OUString aPortalConnect;
    sal_Bool        bServer = pCmdLine->IsServer();

    pCmdLine->GetPortalConnectString( aPortalConnect );
    if ( !configureUcb( bServer, aPortalConnect ))
    {
        DBG_ERROR( "Can't configure UCB" );
        exit(-1);
    }

//  UCB_Helper::Initialize(); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    createTemporaryDirectory();
}

void createTemporaryDirectory()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createTemporaryDirectory" );

    // remove possible old directory and base directory
    SvtPathOptions      aOpt;
    SvtInternalOptions  aInternalOpt;

    // set temp base directory
    ::rtl::OUString aTempBaseURL( aOpt.GetTempPath() );
    sal_Int32 nIndex = aTempBaseURL.lastIndexOf( '/' );
    if ( nIndex != aTempBaseURL.getLength()-1 )
        aTempBaseURL += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ));
    aTempBaseURL += OUString( RTL_CONSTASCII_USTRINGPARAM( DESKTOP_TEMPDIRNAME ));

    String aOldTempURL = aInternalOpt.GetCurrentTempURL();
    if ( aOldTempURL.Len() > 0 )
    {
        // remove old temporary directory
        ::utl::UCBContentHelper::Kill( aOldTempURL );

        // remove old temporary base directory
        ::utl::UCBContentHelper::Kill( aTempBaseURL );
    }

    // set new temporary base directory
    aCurrentTempBase = aTempBaseURL;

    String aRet;
    ::rtl::OUString aTempPath( aTempBaseURL );

    // create new current temporary directory
    ::utl::LocalFileHelper::ConvertURLToPhysicalName( aCurrentTempBase, aRet );
    ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
    aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );

    // set new current temporary directory
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTempPath, aRet );
    aInternalOpt.SetCurrentTempURL( aRet );
    aCurrentTempURL = aRet;
}

void removeTemporaryDirectory()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::removeTemporaryDirectory" );

    // remove current base and current temporary directory
    if ( aCurrentTempURL.Len() > 0 && aCurrentTempBase.Len() > 0 )
    {
        if ( ::utl::UCBContentHelper::Kill( aCurrentTempURL ) )
        {
            SvtInternalOptions().SetCurrentTempURL( String() );
            ::utl::UCBContentHelper::Kill( aCurrentTempBase );
        }
    }
}
