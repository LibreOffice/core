/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: cd $ $Date: 2001-07-06 15:53:14 $
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

#include "app.hxx"
#include "wrapper.hxx"
#include "intro.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <offmgr/app.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/sfxuno.hxx>
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#include <setup2/installer.hxx>
#include <svtools/pathoptions.hxx>
#include "svtools/cjkoptions.hxx"
#include <unotools/configmgr.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/bitmap.hxx>
#include <sfx2/sfx.hrc>

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#define DEFINE_CONST_UNICODE(CONSTASCII)        UniString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII##))

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

void PreloadConfigTrees()
{
    // these tree are preloaded to get a faster startup for the office
    Sequence <rtl::OUString> aPreloadPathList(6);
    aPreloadPathList[0] =  rtl::OUString::createFromAscii("org.openoffice.Office.Common");
    aPreloadPathList[1] =  rtl::OUString::createFromAscii("org.openoffice.ucb.Configuration");
    aPreloadPathList[2] =  rtl::OUString::createFromAscii("org.openoffice.Office.Writer");
    aPreloadPathList[3] =  rtl::OUString::createFromAscii("org.openoffice.Office.WriterWeb");
    aPreloadPathList[4] =  rtl::OUString::createFromAscii("org.openoffice.Office.Calc");
    aPreloadPathList[5] =  rtl::OUString::createFromAscii("org.openoffice.Office.Impress");

    Reference< XMultiServiceFactory > xProvider(
            ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider")), UNO_QUERY);

    Any aValue;
    aValue <<= aPreloadPathList;

    Reference < com::sun::star::beans::XPropertySet > (xProvider, UNO_QUERY)->setPropertyValue(rtl::OUString::createFromAscii("PrefetchNodes"), aValue );
}


void ReplaceStringHookProc( UniString& rStr )
{
    static String aBrandName;
    static String aVersion;
    static String aExtension;

    static int nAll = 0, nPro = 0;

    if ( !aBrandName.Len() )
    {
        Any aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTNAME );
        rtl::OUString aTmp;
        aRet >>= aTmp;
        aBrandName = aTmp;

        aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTVERSION );
        aRet >>= aTmp;
        aVersion = aTmp;

        aRet = ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::PRODUCTEXTENSION );
        aRet >>= aTmp;
        aExtension = aTmp;
    }

    nAll++;
    if ( rStr.SearchAscii( "%PRODUCT" ) != STRING_NOTFOUND )
    {
        nPro++;
        rStr.SearchAndReplaceAllAscii( "%PRODUCTNAME", aBrandName );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTVERSION", aVersion );
        rStr.SearchAndReplaceAllAscii( "%PRODUCTEXTENSION", aExtension );
    }
}

Desktop aDesktop;

Desktop::Desktop() : m_pLabelResMgr( 0 ), m_pIntro( 0 )
{
}

void Desktop::Main()
{
    // ----  Startup screen ----
    OpenStartupScreen( "iso" );

    ResMgr::SetReadStringHook( ReplaceStringHookProc );
    SetAppName( DEFINE_CONST_UNICODE("soffice") );

#ifdef TIMEBOMB
    Date aDate;
    Date aFinalDate( 31, 10, 2001 );
    if ( aFinalDate < aDate )
    {
        String aMsg;
        aMsg += DEFINE_CONST_UNICODE("This Early Access Version has expired!\n");
        InfoBox aBox( NULL, aMsg );
        aBox.Execute();
        return;
    }
#endif

    bool bTerminate = false;
    int nParamCount = GetCommandLineParamCount();

    for( int nActParam = 0; nActParam < nParamCount ; nActParam++ )
    {
        String sActParam = GetCommandLineParam( nActParam );
        if( sActParam.EqualsIgnoreCaseAscii("-terminate_after_init") )
        {
            bTerminate = true;
            break;
        }
    }

//  Read the common configuration items for optimization purpose
//  do not do it if terminate flag was specified, to avoid exception
    if( !bTerminate )
    {
        try
        {
            PreloadConfigTrees();
        }
        catch(com::sun::star::uno::Exception &e)
        {
            bTerminate = true;
            rtl::OUString sError = rtl::OUString::createFromAscii("Unable to retrieve application configuration data: ");
            sError += e.Message;
            Application::Abort(sError);
        }
    }

//  The only step that should be done if terminate flag was specified
//  Typically called by the plugin only
    Installer* pInstaller = new Installer;
    pInstaller->InitializeInstallation( Application::GetAppFileName() );
    delete pInstaller;

    if( !bTerminate )
    {
        SvtPathOptions* pPathOptions = new SvtPathOptions;
        SvtCJKOptions* pCJKOPptions = new SvtCJKOptions(sal_True);
        RegisterServices();
        OfficeWrapper* pWrapper = new OfficeWrapper( ::comphelper::getProcessServiceFactory() );
//      Reference < XComponent > xWrapper( ::utl::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.office.OfficeWrapper" ) ), UNO_QUERY );

        // Post user event to startup first application component window
        Application::PostUserEvent( LINK( this, Desktop, OpenClients ) );
        SfxApplicationClass::Main();
//      xWrapper->dispose();

        if( pWrapper!=NULL)
        {
            delete pWrapper;
            pWrapper=NULL;
        }

        delete pCJKOPptions;
        delete pPathOptions;
    }

    // instead of removing of the configManager jast let it commit all the changes
    utl::ConfigManager::GetConfigManager()->StoreConfigItems();

    //utl::ConfigManager::RemoveConfigManager();
}

void Desktop::SystemSettingsChanging( AllSettings& rSettings, Window* pFrame )
{
    OFF_APP()->SystemSettingsChanging( rSettings, pFrame );
}

IMPL_LINK( Desktop, OpenClients, void*, pvoid )
{
    SFX_APP()->OpenClients();
    CloseStartupScreen();

    return 0;
}

void Desktop::OpenStartupScreen( const char* pLabelPrefix )
{
    if ( pLabelPrefix && !Application::IsRemoteServer() )
    {
        // versuchen, die Label-DLL zu erzeugen
        String aMgrName = String::CreateFromAscii( pLabelPrefix );
        aMgrName += String::CreateFromInt32(SOLARUPD); // aktuelle Versionsnummer
        m_pLabelResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));

        // keine separate Label-DLL vorhanden?
        if ( !m_pLabelResMgr )
        {
            // dann den ResMgr vom Executable verwenden
            m_pLabelResMgr = new ResMgr;
        }

        // Intro nur anzeigen, wenn normaler Start (kein Print/Server etc.)
        ParseCommandLine();
        if ( !m_bInvisible && !m_bMinimized &&
             m_nAppEvents != DISPATCH_PRINT && m_nAppEvents != DISPATCH_SERVER )
        {
            const USHORT nResId = RID_DEFAULTINTRO;
            ResId aIntroBmpRes( nResId, m_pLabelResMgr );
            m_pIntro = new IntroWindow_Impl( aIntroBmpRes );
        }
    }
}

void Desktop::CloseStartupScreen()
{
    delete m_pIntro;
    m_pIntro = 0;
}

void Desktop::ParseCommandLine()
{
    m_nAppEvents = 0;
    m_bMinimized = 0;
    m_bInvisible = 0;

    BOOL   bPrintEvent = FALSE;
    BOOL   bOpenEvent  = TRUE;

    ::vos::OExtCommandLine aCmdLine;
    sal_uInt32 nCount = aCmdLine.getCommandArgCount();
    for( sal_uInt32 i=0; i < nCount; i++ )
    {
        String aArg;
        ::rtl::OUString aDummy;
        aCmdLine.getCommandArg( i, aDummy );
        aArg = aDummy;

        if ( aArg.EqualsIgnoreCaseAscii("-minimized") == sal_True )
            m_bMinimized = sal_True;
        else if ( aArg.EqualsIgnoreCaseAscii("-invisible") == sal_True )
            m_bInvisible = sal_True;
        else if ( aArg.EqualsIgnoreCaseAscii("-embedding") == sal_True )
            m_nAppEvents |= DISPATCH_SERVER;
        else if ( aArg.EqualsIgnoreCaseAscii("-bean") == sal_True )
            m_bInvisible = sal_True;
        else if ( aArg.EqualsIgnoreCaseAscii("-plugin") == sal_True )
            m_bInvisible  = sal_True;

        const xub_Unicode* pArg = aArg.GetBuffer();
        // Erstmal nur mit -, da unter Unix Dateinmane auch mit Slasch anfangen koennen
        if ( (*pArg == '-') /* || (*pArg == '/') */ )
        {
            pArg++;

            // Ein Schalter
            if ( (*pArg == 'p') || (*pArg == 'P') )
            {
                bPrintEvent = TRUE;
                bOpenEvent = FALSE;    // Ab hier keine OpenEvents mehr
            }
        }
        else
        {
            if ( bOpenEvent )
            {
                // Dies wird als Dateiname interpretiert
                m_nAppEvents |= DISPATCH_OPEN;
            }
            else if ( bPrintEvent )
            {
                // Print Event anhaengen
                m_nAppEvents |= DISPATCH_PRINT;
            }
        }
    }
}
