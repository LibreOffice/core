/*************************************************************************
 *
 *  $RCSfile: sfxhelp.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: as $ $Date: 2001-08-30 13:42:14 $
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

#include "sfxhelp.hxx"

#pragma hdrstop

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <toolkit/helper/vclunohelper.hxx>

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include <svtools/pathoptions.hxx>
#include <rtl/ustring.hxx>

#define _SVSTDARR_ULONGSSORT
#include <svtools/svstdarr.hxx>     // SvUShorts

#include "sfxsids.hrc"
#include "app.hxx"
#include "viewfrm.hxx"
#include "msgpool.hxx"
#include "newhelp.hxx"
#include "objsh.hxx"
#include "docfac.hxx"
#include "sfxresid.hxx"
#include "helper.hxx"
#include "app.hrc"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::rtl;

#define ERROR_TAG   String( DEFINE_CONST_UNICODE("Error: ") )
#define PATH_TAG    String( DEFINE_CONST_UNICODE("\nPath: ") )

// -----------------------------------------------------------------------

#define STARTERLIST 0

void AppendConfigToken_Impl( String& rURL, sal_Bool bQuestionMark )
{
    Any aLocale = ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
    ::rtl::OUString aLocaleStr;
    if ( !( aLocale >>= aLocaleStr ) )
        aLocaleStr = ::rtl::OUString( DEFINE_CONST_UNICODE("en") );

    SvtHelpOptions aHelpOpt;
    if ( bQuestionMark )
        rURL += '?';
    else
        rURL += '&';
    rURL += DEFINE_CONST_UNICODE("Language=");
    rURL += String( aLocaleStr );
    rURL += DEFINE_CONST_UNICODE("&System=");
    rURL += aHelpOpt.GetSystem();
}

// -----------------------------------------------------------------------

class SfxHelpOptions_Impl : public utl::ConfigItem
{
    SvULongsSort*   pIds;
public:

                    SfxHelpOptions_Impl();
                    ~SfxHelpOptions_Impl();
    BOOL            HasId( ULONG nId )
                    { USHORT nDummy; return pIds->Seek_Entry( nId, &nDummy ); }
};

static Sequence< OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "HelpAgentStarterList",
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------

SfxHelpOptions_Impl::SfxHelpOptions_Impl()
    : ConfigItem( OUString::createFromAscii("Office.SFX/Help") )
    , pIds( 0 )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
            if ( pValues[nProp].hasValue() )
            {
                switch ( nProp )
                {
                    case STARTERLIST :
                    {
                        ::rtl::OUString aCodedList;
                        if ( pValues[nProp] >>= aCodedList )
                        {
                            String aTmp( aCodedList );
                            USHORT nCount = aTmp.GetTokenCount( ',' );
                            pIds = new SvULongsSort();
                            for ( USHORT n=0; n<nCount; n++ )
                                pIds->Insert( (ULONG) aTmp.GetToken( n, ',' ).ToInt64() );
                        }

                        else
                            DBG_ERROR( "Wrong Type!" );
                        break;
                    }

                    default:
                        DBG_ERROR( "Wrong Property!" );
                        break;
                }
            }
        }
    }
}

SfxHelpOptions_Impl::~SfxHelpOptions_Impl()
{
    delete pIds;
}

// class SfxHelp_Impl ----------------------------------------------------

class SfxHelp_Impl
{
private:
    sal_Bool                m_bIsDebug;
    SfxHelpOptions_Impl*    m_pOpt;

public:
    SfxHelp_Impl( const String& rPath, sal_Bool bDebug );
    ~SfxHelp_Impl();

    String                  GetHelpText( ULONG nHelpId, const String& rModule );
    SfxHelpOptions_Impl*    GetOptions();
};

SfxHelp_Impl::SfxHelp_Impl( const String& rPath, sal_Bool bDebug ) :

    m_bIsDebug  ( bDebug ),
    m_pOpt      ( 0 )

{
}

SfxHelp_Impl::~SfxHelp_Impl()
{
    delete m_pOpt;
}

SfxHelpOptions_Impl* SfxHelp_Impl::GetOptions()
{
    if ( !m_pOpt )
        m_pOpt = new SfxHelpOptions_Impl;
    return m_pOpt;
}

String SfxHelp_Impl::GetHelpText( ULONG nHelpId, const String& rModule )
{
    // create help url
    String aHelpURL = SfxHelp::CreateHelpURL( nHelpId, rModule );
    // added 'active' parameter
    aHelpURL += '&';
    aHelpURL += DEFINE_CONST_UNICODE("Active=true");
    // load help string
    return SfxContentHelper::GetActiveHelpString( aHelpURL );
}

// class SfxHelp ---------------------------------------------------------

SfxHelp::SfxHelp() :

    pImp     ( NULL ),
    bIsDebug( sal_False )

{
    char* pEnv = getenv( "help_debug" );
    if ( pEnv )
        bIsDebug = sal_True;

    Any aLocale = ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
    ::rtl::OUString aLocaleStr;
    if ( !( aLocale >>= aLocaleStr ) )
        aLocaleStr = ::rtl::OUString( DEFINE_CONST_UNICODE("en") );
    sal_Int32 nSepPos = aLocaleStr.indexOf( '_' );
    if ( nSepPos != -1 )
    {
        aLanguageStr = aLocaleStr.copy( 0, nSepPos );
        aCountryStr = aLocaleStr.copy( nSepPos+1 );
    }
    else
    {
        nSepPos = aLocaleStr.indexOf( '-' );
        if ( nSepPos != -1 )
        {
            aLanguageStr = aLocaleStr.copy( 0, nSepPos );
            aCountryStr = aLocaleStr.copy( nSepPos+1 );
        }
        else
        {
            aLanguageStr = aLocaleStr;
        }
    }

    INetURLObject aPath( SvtPathOptions().GetHelpPath(), INET_PROT_FILE );
    aPath.insertName( aLanguageStr );
    pImp = new SfxHelp_Impl( aPath.GetMainURL(), bIsDebug );
}

SfxHelp::~SfxHelp()
{
    delete pImp;
}

String SfxHelp::GetHelpModuleName_Impl( ULONG nHelpId )
{
    String aModuleName;
    SfxViewFrame *pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
    {
        // Wenn es ein Slot ist, kann es sein, da\s internes InPlace vorliegt
        // und eine Container-SlotId gefragt ist
        if (nHelpId >= (ULONG) SID_SFX_START && nHelpId <= (ULONG) SHRT_MAX)
        {
            if ( pViewFrame->GetParentViewFrame_Impl() )
            {
                // Ist es ein ContainerSlot ?
                const SfxSlot* pSlot = SFX_APP()->GetSlotPool(pViewFrame).GetSlot( (USHORT) nHelpId );
                if ( !pSlot || pSlot->IsMode( SFX_SLOT_CONTAINER ) )
                    pViewFrame = pViewFrame->GetParentViewFrame_Impl();
            }
        }

        if( pViewFrame->GetObjectShell() )
            aModuleName = String::CreateFromAscii( pViewFrame->GetObjectShell()->GetFactory().GetShortName() );
    }

    // cut sub factoryname, if necessary
    xub_StrLen nPos = aModuleName.Search( '/' );
    if ( nPos != STRING_NOTFOUND )
        aModuleName.Erase( nPos );

    return aModuleName;
}

String SfxHelp::CreateHelpURL_Impl( ULONG nHelpId, const String& rModuleName )
{
    String aModuleName( rModuleName );
    if ( aModuleName.Len() == 0 )
    {
        // no active module (quicklaunch?) -> detect default module
        SvtModuleOptions aModOpt;
        if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
            aModuleName = DEFINE_CONST_UNICODE("swriter");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
            aModuleName = DEFINE_CONST_UNICODE("scalc");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
            aModuleName = DEFINE_CONST_UNICODE("simpress");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
            aModuleName = DEFINE_CONST_UNICODE("sdraw");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
            aModuleName = DEFINE_CONST_UNICODE("smath");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
            aModuleName = DEFINE_CONST_UNICODE("schart");
        else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SBASIC ) )
            aModuleName = DEFINE_CONST_UNICODE("sbasic");
        else
        {
            DBG_ERRORFILE( "no installed module found" );
        }
    }

    // build up the help URL
    String aHelpURL;
    if ( aTicket.Len() )
    {
        // if there is a ticket, we are inside a plugin, so a special Help URL must be sent
        aHelpURL = DEFINE_CONST_UNICODE("vnd.sun.star.cmd:help?");
        aHelpURL += DEFINE_CONST_UNICODE("HELP_Request_Mode=contextIndex&HELP_Session_Mode=context&HELP_CallMode=portal&HELP_Device=html");

        if ( !nHelpId )
        {
            aHelpURL += DEFINE_CONST_UNICODE("&startId=go");
        }
        else
        {
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_ContextID=-");
            aHelpURL += String::CreateFromInt64( nHelpId );
        }

        aHelpURL += DEFINE_CONST_UNICODE("&HELP_ProgramID=");
        aHelpURL += aModuleName;
        aHelpURL += DEFINE_CONST_UNICODE("&HELP_User=");
        aHelpURL += aUser;
        aHelpURL += DEFINE_CONST_UNICODE("&HELP_Ticket=");
        aHelpURL += aTicket;
        aHelpURL += DEFINE_CONST_UNICODE("&HELP_Language=");
        aHelpURL += aLanguageStr;
        if ( aCountryStr.Len() )
        {
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_Country=");
            aHelpURL += aCountryStr;
        }
    }
    else
    {
        aHelpURL = String::CreateFromAscii("vnd.sun.star.help://");
        aHelpURL += aModuleName;

        if ( !nHelpId )
        {
            aHelpURL += String( DEFINE_CONST_UNICODE("/start") );
        }
        else
        {
            aHelpURL += '/';
            aHelpURL += String::CreateFromInt64( nHelpId );
        }

        AppendConfigToken_Impl( aHelpURL, sal_True );
    }

    return aHelpURL;
}

BOOL SfxHelp::Start( const String& rURL, const Window* pWindow )
{
    Reference < XDispatchProvider > xFrame;
    Reference < XTasksSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XTask > xActiveTask = xDesktop->getActiveTask();

    Sequence < PropertyValue > aProps;
    sal_Int32 nFlag = FrameSearchFlag::ALL;
    sal_Bool bHelpTaskExists = sal_False;
    if ( aTicket.Len() )
    {
        xFrame = Reference < XDispatchProvider > ( xActiveTask, UNO_QUERY );
        nFlag  = FrameSearchFlag::TASKS | FrameSearchFlag::CREATE;
    }
    else
    {
        if ( xActiveTask.is() )
        {
            // try to find the help frame
            ::rtl::OUString aTarget = ::rtl::OUString( DEFINE_CONST_UNICODE("OFFICE_HELP") );
            xFrame = Reference < XDispatchProvider > (
                xActiveTask->findFrame( aTarget, FrameSearchFlag::GLOBAL ), UNO_QUERY );
        }
        // otherwise the URL can be dispatched to the help frame
        if ( !xFrame.is() )
        {
            Reference < XFrame > xFrameFinder( xDesktop, UNO_QUERY );
            Reference < XFrame > xTask = xFrameFinder->findFrame( DEFINE_CONST_UNICODE( "_blank" ), 0 );
            xTask->setName( DEFINE_CONST_OUSTRING("OFFICE_HELP_TASK") );
            xFrame = Reference < XDispatchProvider >( xTask, UNO_QUERY );
            Window* pWin = VCLUnoHelper::GetWindow( xTask->getContainerWindow() );
            pWin->SetText( String( SfxResId( STR_HELP_WINDOW_TITLE ) ) );
            SfxHelpWindow_Impl* pHlpWin = new SfxHelpWindow_Impl( xTask, pWin, WB_DOCKBORDER );
            pHlpWin->Show();
            Reference< ::com::sun::star::awt::XWindow > xWindow = VCLUnoHelper::GetInterface( pHlpWin );
            xWindow->setPosSize( 50, 50, 300, 200, ::com::sun::star::awt::PosSize::SIZE );
            if ( !xTask->setComponent( xWindow, Reference < XController >() ) )
                return FALSE;
            else
            {
                pHlpWin->setContainerWindow( xTask->getContainerWindow() );
                pHlpWin->SetHelpURL( rURL );
                xTask->getContainerWindow()->setVisible( sal_True );
            }
        }
        else
            bHelpTaskExists = sal_True;
    }

    if ( xFrame.is() )
    {
        ::com::sun::star::util::URL aURL;
        aURL.Complete = rURL;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );
        Reference < XDispatch > xDispatch = xFrame->queryDispatch( aURL,
                                DEFINE_CONST_UNICODE("OFFICE_HELP"), nFlag );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, aProps );

        if ( bHelpTaskExists )
        {
            // bring the help task to front
            Reference < XFrame > xFrameFinder( xDesktop, UNO_QUERY );
            Reference < XFrame > xTask = xFrameFinder->findFrame( DEFINE_CONST_UNICODE("OFFICE_HELP_TASK"), FrameSearchFlag::TASKS );
            if ( xTask.is() )
            {
                Reference < ::com::sun::star::awt::XTopWindow > xTopWin( xTask->getContainerWindow(), UNO_QUERY );
                if ( xTopWin.is() )
                    xTopWin->toFront();
            }
        }

        return TRUE;
    }
    else
        return FALSE;
}

BOOL SfxHelp::Start( ULONG nHelpId, const Window* pWindow )
{
    String aHelpModuleName( GetHelpModuleName_Impl( nHelpId ) );
    String aHelpURL = CreateHelpURL( nHelpId, aHelpModuleName );
    if ( pWindow && SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
    {
        // no help found -> try with parent help id.
        Window* pParent = pWindow->GetParent();
        while ( pParent )
        {
            nHelpId = pParent->GetHelpId();
            aHelpURL = CreateHelpURL( nHelpId, aHelpModuleName );

            if ( !SfxContentHelper::IsHelpErrorDocument( aHelpURL ) )
                break;
            else
            {
                pParent = pParent->GetParent();
                if ( !pParent )
                    // create help url of start page ( helpid == 0 -> start page)
                    aHelpURL = CreateHelpURL( 0, aHelpModuleName );
            }
        }
    }

    return Start( aHelpURL, pWindow );
}

XubString SfxHelp::GetHelpText( ULONG nHelpId, const Window* pWindow )
{
    String aModuleName = GetHelpModuleName_Impl( nHelpId );
    String aHelpText = pImp->GetHelpText( nHelpId, aModuleName );
    ULONG nNewHelpId = 0;

    if ( pWindow && aHelpText.Len() == 0 )
    {
        // no help text found -> try with parent help id.
        Window* pParent = pWindow->GetParent();
        while ( pParent )
        {
            nNewHelpId = pParent->GetHelpId();
            aHelpText = pImp->GetHelpText( nNewHelpId, aModuleName );

            if ( aHelpText.Len() > 0 )
                pParent = NULL;
            else
                pParent = pParent->GetParent();
        }

        if ( bIsDebug && aHelpText.Len() == 0 )
            nNewHelpId = 0;
    }

    if ( bIsDebug )
    {
        aHelpText += DEFINE_CONST_UNICODE("\n\n");
        aHelpText += aModuleName;
        aHelpText += DEFINE_CONST_UNICODE(" - ");
        aHelpText += String::CreateFromInt64( nHelpId );
        if ( nNewHelpId )
        {
            aHelpText += DEFINE_CONST_UNICODE(" - ");
            aHelpText += String::CreateFromInt64( nNewHelpId );
        }
    }

    return aHelpText;
}

String SfxHelp::CreateHelpURL( ULONG nHelpId, const String& rModuleName )
{
    String aURL;
    SfxHelp* pHelp = SAL_STATIC_CAST( SfxHelp*, Application::GetHelp() );
    if ( pHelp )
        aURL = pHelp->CreateHelpURL_Impl( nHelpId, rModuleName );
    return aURL;
}

void SfxHelp::OpenHelpAgent( SfxFrame *pFrame, ULONG nHelpId )
{
    if ( SvtHelpOptions().IsHelpAgentAutoStartMode() )
    {
        SfxHelp* pHelp = SAL_STATIC_CAST( SfxHelp*, Application::GetHelp() );
        if ( pHelp )
        {
            SfxHelpOptions_Impl *pOpt = pHelp->pImp->GetOptions();
            if ( !pOpt->HasId( nHelpId ) )
                return;

            try
            {
                URL aURL;
                aURL.Complete = pHelp->CreateHelpURL_Impl( nHelpId, pHelp->GetHelpModuleName_Impl( nHelpId ) );
                Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
                xTrans->parseStrict(aURL);

                Reference< XDispatchProvider > xDispProv( pFrame->GetTopFrame()->GetFrameInterface(), UNO_QUERY );
                Reference< XDispatch > xHelpDispatch;
                if (xDispProv.is())
                    xHelpDispatch = xDispProv->queryDispatch(aURL, ::rtl::OUString::createFromAscii("_helpagent"), FrameSearchFlag::PARENT | FrameSearchFlag::SELF);

                DBG_ASSERT( xHelpDispatch.is(), "OpenHelpAgent: could not get a dispatcher!" );
                if ( xHelpDispatch.is() )
                    xHelpDispatch->dispatch( aURL, Sequence< PropertyValue >() );
            }
            catch( const Exception& )
            {
                DBG_ASSERT( sal_False, "OpenHelpAgent: caught an exception while executing the dispatch!" );
            }
        }
    }
}

