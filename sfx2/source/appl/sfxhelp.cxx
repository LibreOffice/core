 /*************************************************************************
 *
 *  $RCSfile: sfxhelp.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:27:42 $
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

#include <algorithm>

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
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
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
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif

#include <svtools/pathoptions.hxx>
#include <rtl/ustring.hxx>
#include <osl/process.h>

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_ULONGSSORT
#include <svtools/svstdarr.hxx>

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

#define ERROR_TAG   String( DEFINE_CONST_UNICODE("Error: ") )
#define PATH_TAG    String( DEFINE_CONST_UNICODE("\nPath: ") )

// -----------------------------------------------------------------------

#define STARTERLIST 0

void AppendConfigToken_Impl( String& rURL, sal_Bool bQuestionMark )
{
    // this completes a help url with the system parameters "Language" and "System"
    // detect installed locale
    Any aLocale =
        ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
    ::rtl::OUString aLocaleStr;
    if ( !( aLocale >>= aLocaleStr ) )
        // fallback is english
        aLocaleStr = ::rtl::OUString( DEFINE_CONST_UNICODE("en") );

    // query part exists?
    if ( bQuestionMark )
        // no, so start with '?'
        rURL += '?';
    else
        // yes, so only append with '&'
        rURL += '&';

    // set parameters
    rURL += DEFINE_CONST_UNICODE("Language=");
    rURL += String( aLocaleStr );
    rURL += DEFINE_CONST_UNICODE("&System=");
    rURL += SvtHelpOptions().GetSystem();
}

// -----------------------------------------------------------------------

sal_Bool GetHelpAnchor_Impl( const String& _rURL, String& _rAnchor )
{
    sal_Bool bRet = sal_False;
    ::rtl::OUString sAnchor;

    try
    {
        ::ucb::Content aCnt( INetURLObject( _rURL ).GetMainURL( INetURLObject::NO_DECODE ),
                             Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        if ( ( aCnt.getPropertyValue( ::rtl::OUString::createFromAscii( "AnchorName" ) ) >>= sAnchor ) )
        {

            if ( sAnchor.getLength() > 0 )
            {
                _rAnchor = String( sAnchor );
                bRet = sal_True;
            }
        }
        else
        {
            DBG_ERRORFILE( "Property 'AnchorName' is missing" );
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------

class SfxHelpOptions_Impl : public utl::ConfigItem
{
private:
    SvULongsSort*   m_pIds;

public:
                    SfxHelpOptions_Impl();
                    ~SfxHelpOptions_Impl();

    BOOL            HasId( ULONG nId ) { USHORT nDummy; return m_pIds ? m_pIds->Seek_Entry( nId, &nDummy ) : FALSE; }
};

static Sequence< ::rtl::OUString > GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "HelpAgentStarterList",
    };

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< ::rtl::OUString > aNames( nCount );
    ::rtl::OUString* pNames = aNames.getArray();
    ::rtl::OUString* pEnd   = pNames + aNames.getLength();
    int i = 0;
    for ( ; pNames != pEnd; ++pNames )
        *pNames = ::rtl::OUString::createFromAscii( aPropNames[i++] );

    return aNames;
}

// -----------------------------------------------------------------------

SfxHelpOptions_Impl::SfxHelpOptions_Impl()
    : ConfigItem( ::rtl::OUString::createFromAscii("Office.SFX/Help") )
    , m_pIds( NULL )
{
    Sequence< ::rtl::OUString > aNames = GetPropertyNames();
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
                            m_pIds = new SvULongsSort();
                            for ( USHORT n=0; n<nCount; n++ )
                                m_pIds->Insert( (ULONG) aTmp.GetToken( n, ',' ).ToInt64() );
                        }
                        else
                            DBG_ERRORFILE( "Wrong property type!" );

                        break;
                    }

                    default:
                        DBG_ERRORFILE( "Wrong property!" );
                        break;
                }
            }
        }
    }
}

SfxHelpOptions_Impl::~SfxHelpOptions_Impl()
{
    delete m_pIds;
}

// class SfxHelp_Impl ----------------------------------------------------

class SfxHelp_Impl
{
private:
    sal_Bool                            m_bIsDebug;     // environment variable "help_debug=1"
    SfxHelpOptions_Impl*                m_pOpt;         // the options
    ::std::vector< ::rtl::OUString >    m_aModulesList; // list of all installed modules

public:
    SfxHelp_Impl( sal_Bool bDebug );
    ~SfxHelp_Impl();

    SfxHelpOptions_Impl*    GetOptions();
    String                  GetHelpText( ULONG nHelpId, const String& rModule );    // get "Active Help"
    sal_Bool                HasModule( const ::rtl::OUString& rModule ) const;      // module installed
    sal_Bool                IsHelpInstalled() const;                                // module list not empty
};

SfxHelp_Impl::SfxHelp_Impl( sal_Bool bDebug ) :

    m_bIsDebug      ( bDebug ),
    m_pOpt          ( NULL )

{
    // fill modules list
    // create the help url (empty, without module and helpid)
    String sHelpURL( DEFINE_CONST_UNICODE("vnd.sun.star.help://") );
    AppendConfigToken_Impl( sHelpURL, sal_True );
    // open ucb content and get the list of the help modules
    // the list contains strings with three tokens "ui title \t type \t url"
    Sequence< ::rtl::OUString > aAllModulesList = SfxContentHelper::GetResultSet( sHelpURL );
    sal_Int32 nLen = aAllModulesList.getLength();
    m_aModulesList.reserve( nLen + 1 );
    const ::rtl::OUString* pBegin = aAllModulesList.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + nLen;
    for ( ; pBegin != pEnd; ++pBegin )
    {
        // get one module string
        String sModule( *pBegin );
        // extract the url
        String sURL = sModule.GetToken( 2, '\t' );
        // insert the module (the host part of the "vnd.sun.star.help" url)
        m_aModulesList.push_back( ::rtl::OUString( INetURLObject( sURL ).GetHost() ) );
    }
}

SfxHelp_Impl::~SfxHelp_Impl()
{
    delete m_pOpt;
}

String SfxHelp_Impl::GetHelpText( ULONG nHelpId, const String& rModule )
{
    // create help url
    String aHelpURL = SfxHelp::CreateHelpURL( nHelpId, rModule );
    // added 'active' parameter
    aHelpURL.Insert( String( DEFINE_CONST_UNICODE("&Active=true") ), aHelpURL.SearchBackward( '#' ) );
    // load help string
    return SfxContentHelper::GetActiveHelpString( aHelpURL );
}

SfxHelpOptions_Impl* SfxHelp_Impl::GetOptions()
{
    // create if not exists
    if ( !m_pOpt )
        m_pOpt = new SfxHelpOptions_Impl;
    return m_pOpt;
}

sal_Bool SfxHelp_Impl::HasModule( const ::rtl::OUString& rModule ) const
{
    return ( ::std::find( m_aModulesList.begin(), m_aModulesList.end(), rModule ) != m_aModulesList.end() );
}

sal_Bool SfxHelp_Impl::IsHelpInstalled() const
{
    return ( m_aModulesList.begin() != m_aModulesList.end() );
}

// class SfxHelp ---------------------------------------------------------

SfxHelp::SfxHelp() :

    bIsDebug( sal_False ),
    pImp    ( NULL )

{
    // read the environment variable "HELP_DEBUG"
    // if it's set, you will see debug output on active help
    {
        ::rtl::OUString sHelpDebug;
        ::rtl::OUString sEnvVarName( RTL_CONSTASCII_USTRINGPARAM( "HELP_DEBUG" ) );
        osl_getEnvironment( sEnvVarName.pData, &sHelpDebug.pData );
        bIsDebug = ( 0 != sHelpDebug.getLength() );
    }

    pImp = new SfxHelp_Impl( bIsDebug );

    Any aLocale =
        ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
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
}

SfxHelp::~SfxHelp()
{
    delete pImp;
}

String GetFactoryName_Impl( const SfxViewFrame* pFrame )
{
    // module name == short name of the factory (e.g. "swriter", "scalc",...)
    String aName = String::CreateFromAscii( pFrame->GetObjectShell()->GetFactory().GetShortName() );
    // cut sub factory, if exists (e.g. "swriter/web")
    xub_StrLen nPos = aName.Search( '/' );
    if ( nPos != STRING_NOTFOUND )
        aName.Erase( nPos );
    return aName;
}

String SfxHelp::GetHelpModuleName_Impl( ULONG nHelpId )
{
    String aModuleName;
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if ( pViewFrame )
    {
        SfxViewFrame* pParentViewFrame = pViewFrame->GetParentViewFrame_Impl();

        // Wenn es ein Slot ist, kann es sein, da\s internes InPlace vorliegt
        // und eine Container-SlotId gefragt ist
        if ( nHelpId >= (ULONG) SID_SFX_START && nHelpId <= (ULONG) SHRT_MAX && pParentViewFrame )
        {
            // Ist es ein ContainerSlot ?
            const SfxSlot* pSlot = SFX_APP()->GetSlotPool(pViewFrame).GetSlot( (USHORT) nHelpId );
            if ( !pSlot || pSlot->IsMode( SFX_SLOT_CONTAINER ) )
                pViewFrame = pParentViewFrame;
        }

        if ( pViewFrame->GetObjectShell() )
        {
            aModuleName = GetFactoryName_Impl( pViewFrame );
            // help for module installed? If help isn't installed, module search will be disabled.
            sal_Bool bHasModule = pImp->IsHelpInstalled() ? pImp->HasModule( aModuleName ) : sal_True;
            // if not, search through the shell hierachy for an installed module
            while ( !bHasModule && pParentViewFrame && pParentViewFrame->GetObjectShell() )
            {
                aModuleName = GetFactoryName_Impl( pParentViewFrame );
                bHasModule = pImp->HasModule( aModuleName );
                pParentViewFrame = pParentViewFrame->GetParentViewFrame_Impl();
            }
        }
    }

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
            // no help id -> start page
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_ContextID=start");
        }
        else
        {
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_ContextID=");
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
        sal_Bool bHasAnchor = sal_False;
        String aAnchor;
        aHelpURL = String::CreateFromAscii("vnd.sun.star.help://");
        aHelpURL += aModuleName;

        if ( !nHelpId )
            aHelpURL += String::CreateFromAscii("/start");
        else
        {
            aHelpURL += '/';
            aHelpURL += String::CreateFromInt64( nHelpId );

            String aTempURL = aHelpURL;
            AppendConfigToken_Impl( aTempURL, sal_True );
            bHasAnchor = GetHelpAnchor_Impl( aTempURL, aAnchor );
        }

        AppendConfigToken_Impl( aHelpURL, sal_True );

        if ( bHasAnchor )
        {
            aHelpURL += '#';
            aHelpURL += aAnchor;
        }
    }

    return aHelpURL;
}

BOOL SfxHelp::Start( const String& rURL, const Window* pWindow )
{
    String aHelpURL( rURL );
    INetURLObject aObj( aHelpURL );
    ::rtl::OUString sHelpKeyword;
    if ( aObj.GetProtocol() != INET_PROT_VND_SUN_STAR_HELP )
    {
        aHelpURL = CreateHelpURL_Impl( 0, GetHelpModuleName_Impl( 0 ) );
        sHelpKeyword = ::rtl::OUString( rURL );
    }
    Reference < XDispatchProvider > xFrame;
    Reference < XFramesSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XFrame > xActiveTask = xDesktop->getActiveFrame();

    Sequence < PropertyValue > aArgs;
    if ( sHelpKeyword.getLength() > 0 )
    {
        aArgs.realloc( 1 );
        aArgs[0].Name = String( DEFINE_CONST_UNICODE("HelpKeyword") );
        aArgs[0].Value <<= sHelpKeyword;
    }
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
                pHlpWin->SetHelpURL( aHelpURL );
                xFrame = Reference < XDispatchProvider >( pHlpWin->getTextFrame(), UNO_QUERY );
                xTask->getContainerWindow()->setVisible( sal_True );
            }
        }
        else
            bHelpTaskExists = sal_True;
    }

    if ( xFrame.is() )
    {
        ::com::sun::star::util::URL aURL;
        aURL.Complete = aHelpURL;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );
        Reference < XDispatch > xDispatch =
            xFrame->queryDispatch( aURL, DEFINE_CONST_UNICODE("OFFICE_HELP"), nFlag );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, aArgs );

        if ( bHelpTaskExists )
        {
            // bring the help task to front
            Reference < XFrame > xFrameFinder( xDesktop, UNO_QUERY );
            Reference < XFrame > xTask = xFrameFinder->findFrame(
                DEFINE_CONST_UNICODE("OFFICE_HELP_TASK"), FrameSearchFlag::TASKS );
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
                Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" ) ), UNO_QUERY );
                xTrans->parseStrict(aURL);

                Reference< XDispatchProvider > xDispProv( pFrame->GetTopFrame()->GetFrameInterface(), UNO_QUERY );
                Reference< XDispatch > xHelpDispatch;
                if ( xDispProv.is() )
                    xHelpDispatch = xDispProv->queryDispatch(
                        aURL, ::rtl::OUString::createFromAscii("_helpagent"),
                        FrameSearchFlag::PARENT | FrameSearchFlag::SELF );

                DBG_ASSERT( xHelpDispatch.is(), "OpenHelpAgent: could not get a dispatcher!" );
                if ( xHelpDispatch.is() )
                    xHelpDispatch->dispatch( aURL, Sequence< PropertyValue >() );
            }
            catch( const Exception& )
            {
                DBG_ERRORFILE( "OpenHelpAgent: caught an exception while executing the dispatch!" );
            }
        }
    }
}

