/*************************************************************************
 *
 *  $RCSfile: sfxhelp.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: pb $ $Date: 2001-03-22 14:25:49 $
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

#ifndef INCLUDED_SVTOOLS_HELPOPT_HXX
#include <svtools/helpopt.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include "sfxsids.hrc"
#include "app.hxx"
#include "viewfrm.hxx"
#include "msgpool.hxx"
#include "newhelp.hxx"
#include "objsh.hxx"
#include "docfac.hxx"
#include "sfxresid.hxx"
#include "app.hrc"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

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

SfxHelp_Impl::SfxHelp_Impl() :

    bIsDebug( sal_False )

{
    char* pEnv = getenv( "help_debug" );
    if ( pEnv )
        bIsDebug = sal_True;
}

SfxHelp_Impl::~SfxHelp_Impl()
{
}

String SfxHelp_Impl::GetHelpModuleName( ULONG nHelpId )
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

BOOL SfxHelp_Impl::Start( ULONG nHelpId )
{
    Reference < XTasksSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

    Reference < XTask > xActiveTask = xDesktop->getActiveTask();
    if ( !xActiveTask.is() )
        return FALSE;

    // try to find the help frame
    ::rtl::OUString aTarget = ::rtl::OUString( DEFINE_CONST_UNICODE("OFFICE_HELP") );
    Reference < XDispatchProvider > xFrame( xActiveTask->findFrame( aTarget, FrameSearchFlag::GLOBAL ), UNO_QUERY );
    Sequence < PropertyValue > aProps;
    sal_Int32 nFlag = FrameSearchFlag::GLOBAL;
    String aHelpModuleName = GetHelpModuleName( nHelpId );

    // build up the help URL
    sal_Bool bNewWin = !( xFrame.is() );
    String aHelpURL;
    if ( aTicket.Len() )
    {
        // if there is a ticket, we are inside a plugin, so a special Help URL must be sent
        aHelpURL = DEFINE_CONST_UNICODE("vnd.sun.star.cmd:help?");
        aHelpURL += DEFINE_CONST_UNICODE("HELP_Request_Mode=contextIndex&HELP_Session_Mode=context&HELP_CallMode=portal&HELP_Device=html");

        if ( !nHelpId || bNewWin )
        {
            aHelpURL += DEFINE_CONST_UNICODE("&startId=go");
        }
        else
        {
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_ContextID=-");
            aHelpURL += String::CreateFromInt32( nHelpId );
        }

        aHelpURL += DEFINE_CONST_UNICODE("&HELP_ProgramID=");
        aHelpURL += GetHelpModuleName( nHelpId );
        aHelpURL += DEFINE_CONST_UNICODE("&HELP_User=");
        aHelpURL += aUser;
        aHelpURL += DEFINE_CONST_UNICODE("&HELP_Ticket=");
        aHelpURL += aTicket;

        Any aLocale = ::utl::ConfigManager::GetConfigManager()->GetDirectConfigProperty( ::utl::ConfigManager::LOCALE );
        ::rtl::OUString aLocaleStr;
        if ( !( aLocale >>= aLocaleStr ) )
            aLocaleStr = ::rtl::OUString( DEFINE_CONST_UNICODE("en") );
        String aLanguage;
        String aCountry;
        sal_Int32 nSepPos = aLocaleStr.indexOf( '_' );
        if ( nSepPos != -1 )
        {
            aLanguage = aLocaleStr.copy( 0, nSepPos );
            aCountry = aLocaleStr.copy( nSepPos+1 );
        }
        else
        {
            nSepPos = aLocaleStr.indexOf( '-' );
            if ( nSepPos != -1 )
            {
                aLanguage = aLocaleStr.copy( 0, nSepPos );
                aCountry = aLocaleStr.copy( nSepPos+1 );
            }
            else
            {
                aLanguage = aLocaleStr;
            }
        }

        aHelpURL += DEFINE_CONST_UNICODE("&HELP_Language=");
        aHelpURL += aLanguage;
        if ( aCountry.Len() )
        {
            aHelpURL += DEFINE_CONST_UNICODE("&HELP_Country=");
            aHelpURL += aCountry;
        }

        xFrame = Reference < XDispatchProvider > ( xActiveTask, UNO_QUERY );
    }
    else
    {
        aHelpURL = String::CreateFromAscii("vnd.sun.star.help://");
        aHelpURL += aHelpModuleName;

        if ( !nHelpId || bNewWin )
        {
            aHelpURL += String( DEFINE_CONST_UNICODE("/start") );
        }
        else
        {
            aHelpURL += '/';
            aHelpURL += String::CreateFromInt32( nHelpId );
        }

        AppendConfigToken_Impl( aHelpURL, sal_True );

        // otherwise the URL can be dispatched to the help frame
        if ( !xFrame.is() )
        {
            Reference < XFrame > xTask = xActiveTask->findFrame( DEFINE_CONST_UNICODE( "_blank" ), 0 );
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
                pHlpWin->SetFactory( aHelpModuleName, sal_True );
                xTask->getContainerWindow()->setVisible( sal_True );
            }
        }
    }

    if ( xFrame.is() )
    {
        ::com::sun::star::util::URL aURL;
        aURL.Complete = aHelpURL;
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );
        Reference < XDispatch > xDispatch = xFrame->queryDispatch( aURL,
                                DEFINE_CONST_UNICODE("OFFICE_HELP"), FrameSearchFlag::ALL );
        if ( xDispatch.is() )
        {
            xDispatch->dispatch( aURL, aProps );
        }

        return TRUE;
    }
    else
        return FALSE;
}

XubString SfxHelp_Impl::GetHelpText( ULONG nHelpId )
{
    XubString aHelpText;
    if ( bIsDebug )
    {
        aHelpText = GetHelpModuleName( nHelpId );
        aHelpText += DEFINE_CONST_UNICODE(" - ");
        aHelpText += String::CreateFromInt32( nHelpId );
    }

    return aHelpText;
}

/*-----------------22.11.2000 10:59-----------------
 * old code
BOOL SfxHelp_Impl::ImplStart( ULONG nHelpId, BOOL bCheckHelpFile, BOOL bChangeHelpFile, BOOL bHelpAgent )
{
    static BOOL bInHelpRequest = FALSE;
    if ( bInHelpRequest || !nHelpId || ( nHelpId == SID_EXTENDEDHELP ) )
    {
        if ( bInHelpRequest )
            Sound::Beep();
        return FALSE;
    }


    if ( Help::IsRightHelp() )
    {
        if( ImplGetHelpMode() & HELPTEXTMODE_NOHELPAGENT )
            return FALSE;

        if ( ( nHelpId == HELP_INDEX ) || !CheckHelpFile( TRUE ) )
            return FALSE;

        bInHelpRequest = TRUE;
        SetCurrentHelpFile( nHelpId );
        StartHelpPI( nHelpId, FALSE, FALSE );
        bInHelpRequest = FALSE;
        return TRUE;
    }

    bInHelpRequest = TRUE;

    if( ImplGetHelpMode() & HELPTEXTMODE_NOCONTEXTHELP )
    {
        if( nHelpId < 20000 || nHelpId > 20006 )
            nHelpId = HELP_INDEX;
    }

    if ( bChangeHelpFile )
        SetCurrentHelpFile( nHelpId );

    BOOL bDone = FALSE;
    if ( !bCheckHelpFile || CheckHelpFile( TRUE ) )
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        if ( !bHelpAgent || ( nHelpId == HELP_INDEX ) )
//              !Application::IsInModalMode() &&
//              Application::GetAppWindow()->IsEnabled() &&
//              !SearchFocusWindowParent() &&   // kein Dialog aktiv
//              ( !pFrame || !pFrame->GetObjectShell()->IsInModalMode() ) )
        {
            SfxHelpViewShell* pViewShell = GetHelpViewShell( TRUE );
            if ( pViewShell )
                bDone = pViewShell->ShowHelp( GetHelpFile(), nHelpId );
        }
        else
        {
            StartHelpPI( nHelpId, TRUE, FALSE );
            SfxHelpPI* pHelpPI= SFX_APP()->GetHelpPI();
            if ( pHelpPI )
            {
                if ( !pHelpPI->IsFloatingMode() )
                {
                    pHelpPI->SetFloatingMode( TRUE );
                    bForcedFloatingPI = TRUE;
                }
                if ( pHelpPI->GetFloatingWindow() )
                    pHelpPI->GetFloatingWindow()->ToTop();
                CheckPIPosition();
                bDone = TRUE;
            }
        }
    }
    bInHelpRequest = FALSE;
    return bDone;
    return FALSE;
}
*/

struct HelpFileInfo;
class SfxHelp
{
public:
    static  BOOL            ShowHelp( ULONG nId, BOOL bShowInHelpAgent, const char* pFileName = 0, BOOL bQuiet = FALSE );
    static  BOOL            ShowHelp( const String& rKeyword, BOOL bShowInHelpAgent, const char* pFileName = 0 );
    static  void            ShowHint( ULONG nId );
    static  void            SetCustomHelpFile( const String& rName );
    static  USHORT          GetHelpFileInfoCount();
    static  HelpFileInfo*   GetHelpFileInfo( USHORT n );
};

BOOL SfxHelp::ShowHelp( ULONG nId, BOOL bShowInHelpAgent, const char* pFileName, BOOL bQuiet )
{
    return FALSE;
}

BOOL SfxHelp::ShowHelp( const String& rKeyword, BOOL bShowInHelpAgent, const char* pFileName )
{
    return FALSE;
}

void SfxHelp::ShowHint( ULONG nId )
{
}

void SfxHelp::SetCustomHelpFile( const String& rName )
{
}

USHORT SfxHelp::GetHelpFileInfoCount()
{
    return 0;
}

HelpFileInfo* SfxHelp::GetHelpFileInfo( USHORT n )
{
    return NULL;
}

class SfxHelpPI
{
public:
    void            LoadTopic( const String& rFileName, ULONG nId );
    void            LoadTopic( ULONG nId );
    void            LoadTopic( const String& rKeyword );
    void            ResetTopic();
    BOOL            Close();
//    BOOL            IsConstructed() const { return ( pHelpPI != 0 ); }
    String          GetExtraInfo() const;
//    HelpPI*         GetHelpPI() const { return pHelpPI; }
//    virtual void    FillInfo( SfxChildWinInfo& ) const;
    void            SetTip( ULONG nId );
    void            SetTipText( const String& rText );
//    BOOL            IsInShowMe() const { return bInShowMe; }
//    BOOL            IsTopicJustRequested() const { return aTopicJustRequestedTimer.IsActive(); }
//    void            SetTopicJustRequested( BOOL bOn ) { if( bOn )
//                                                            aTopicJustRequestedTimer.Start();
//                                                        else
//                                                            aTopicJustRequestedTimer.Stop(); }
};

void SfxHelpPI::LoadTopic( const String& rFileName, ULONG nId ) {}
void SfxHelpPI::LoadTopic( ULONG nId ) {}
void SfxHelpPI::LoadTopic( const String& rKeyword ) {}
void SfxHelpPI::ResetTopic() {}
//    BOOL            IsConstructed() const { return ( pHelpPI != 0 ); }
//    HelpPI*         GetHelpPI() const { return pHelpPI; }
//    virtual void    FillInfo( SfxChildWinInfo& ) const;
void SfxHelpPI::SetTip( ULONG nId ) {}
void SfxHelpPI::SetTipText( const String& rText ) {}

class SfxHelpPIWrapper
{
public:
    static USHORT GetChildWindowId();
};

USHORT SfxHelpPIWrapper::GetChildWindowId()
{
    return 0;
}

class SfxHelpTipsWrapper
{
public:
    static USHORT GetChildWindowId();
};

USHORT SfxHelpTipsWrapper::GetChildWindowId()
{
    return 0;
}
