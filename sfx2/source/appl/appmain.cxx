/*************************************************************************
 *
 *  $RCSfile: appmain.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-04 17:34:22 $
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

//#define TF_NEWDESKTOP

#define _SDINTERN_HXX

#pragma hdrstop

#ifndef _PVER_HXX //autogen
#include <svtools/pver.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _CSTITEM_HXX //autogen
#include <svtools/cstitem.hxx>
#endif
#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/itempool.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/helpopt.hxx>

#include "appimp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "docfac.hxx"
#include "app.hxx"
#include "arrdecl.hxx"
#include "dispatch.hxx"
#include "sfxresid.hxx"
#include "interno.hxx"
#include "fcontnr.hxx"
#include "viewsh.hxx"
#include "picklist.hxx"
#include "intro.hxx"
#include "msgpool.hxx"
#include "cfgmgr.hxx"
#include "accmgr.hxx"
#include "mnumgr.hxx"
#include "stbmgr.hxx"
#include "imgmgr.hxx"
#include "eacopier.hxx"
#include "appuno.hxx"
#include "objuno.hxx"
#include "app.hrc"
#include "docfile.hxx"
#include "inimgr.hxx"

#ifdef WNT
#include <tools/svwin.h>
#endif

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif


//===================================================================

/*DBG_NAME(SfxAppMainIntro);
DBG_NAME(SfxAppMainSO_Init);
DBG_NAME(SfxAppMainAppRes);
DBG_NAME(SfxAppMainInit0);
DBG_NAME(SfxAppMainCreateAppWin);
DBG_NAME(SfxAppMainInit1);
DBG_NAME(SfxAppMainCfgMgr);
DBG_NAME(SfxAppMainInitController);
DBG_NAME(SfxAppMainInitException);
DBG_NAME(SfxAppMainRegisterIF);
DBG_NAME(SfxAppMainInit);
DBG_NAME(SfxAppMainLoadBasMgr);
DBG_NAME(SfxAppMainSbxInit);*/
DBG_NAME(SfxAppMainNewMenu);
DBG_NAME(SfxAppMainBmkMenu);
DBG_NAME(SfxAppMainWizMenu);
DBG_NAME(SfxAppMainOLEReg);
DBG_NAME(SfxAppMainCHAOSReg);
/*DBG_NAME(SfxAppMainInitDispatcher);
DBG_NAME(SfxAppMainLoadConfig);
DBG_NAME(SfxAppMainInitAppWin);
DBG_NAME(SfxAppMainAppEvents);*/

//===================================================================

#define SFX_TEMPNAMEBASE_DIR    "soffice.tmp"
#define SFX_KEY_TEMPNAMEBASE    "Temp-Dir"

//===================================================================

#pragma code_seg("STATICS")
static SfxVoidItem aStaticDefault(1);
#pragma code_seg()

static SfxPoolItem* aStaticDefaults[1] =
{
    &aStaticDefault
};

#ifdef TF_POOLABLE
static SfxItemInfo __READONLY_DATA aItemInfos[] =
{
    { 0, 0 }
};
#endif

static SfxEA_Copier* pEA_Copier = 0;

//===================================================================

typedef Link* LinkPtr;
SV_DECL_PTRARR(SfxInitLinkList, LinkPtr, 4, 4);

TYPEINIT1(SfxSysChangeHint, SfxHint);
TYPEINIT2(SfxApplication,SfxShell,SfxBroadcaster);

//--------------------------------------------------------------------

BOOL SfxApplication::HasFeature( ULONG nFeature ) const

/** [Description]

    Determines if 'nFeature' is supported by this running application.
    If more than one feature is given (by arithmetic 'or') and at least
    one of them is supported, it will return TRUE.

    The supported features are set in the ctor of SfxApplication.
*/

{
    return 0 != ( nFeature & _nFeatures );
}

//--------------------------------------------------------------------
void SfxApplication::Init
(
)

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application:a:Main() gerufen,
    bevor Execute() ausgef"uhrt wird und
    - das Intro bereits angezeigt ist,
    - das Applikationsfenster exisitiert, aber noch hidden ist,
    - die Bindings bereits existieren (Controller sind anmeldbar),
    - der Ini- und Config-Manager bereits existiert,
    - die Standard-Controller bereits exisitieren,
    - die SFx-Shells ihre Interfaces bereits registriert haben.

    [Querverweise]
    <SfxApplication::Exit()>
    <SfxApplication::OpenClients()>
*/
{
#ifdef DDE_AVAILABLE
#ifdef PRODUCT
    InitializeDde();
#else
    if( !InitializeDde() )
    {
        ByteString aStr( "Kein DDE-Service moeglich. Fehler: " );
        if( GetDdeService() )
            aStr += GetDdeService()->GetError();
        else
            aStr += '?';
        DBG_ASSERT( sal_False, aStr.GetBuffer() )
    }
#endif
#endif
}

//--------------------------------------------------------------------

void SfxApplication::Exit()

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application::Main() gerufen,
    nachdem Execute() beendet ist und
    - die Konfiguration (SfxConfigManager) bereits gespeichert wurde,
    - die Fensterpostionen etc. in den SfxIniManager geschrieben wurden,
    - das Applikationsfenster noch existiert, aber hidden ist
    - s"amtliche Dokumente und deren Views bereits geschlossen sind.
    - Dispatcher, Bindings etc. bereits zerst"ort sind

    [Querverweise]
    <SfxApplication::Init(int,char*[])>
*/

{
}

//---------------------------------------------------------------------------

void SfxApplication::PreInit( )
{
    // in Tools das kopieren von EAs ermoeglichen
    if ( !pEA_Copier )
        pEA_Copier = new SfxEA_Copier;
}

USHORT SfxApplication::ParseCommandLine_Impl()
{
    USHORT nEvents = 0;                 // return value ( event mask )

    BOOL   bPrintEvent = FALSE;
    BOOL   bOpenEvent  = TRUE;
    USHORT nCount = Application::GetCommandLineParamCount();
    for( USHORT i=0; i < nCount; i++ )
    {
        String aArg = Application::GetCommandLineParam( i );

        if ( aArg.EqualsIgnoreCaseAscii("-minimized") == sal_True )
            pAppData_Impl->bMinimized = TRUE;
        if ( aArg.EqualsIgnoreCaseAscii("-invisible") == sal_True )
            pAppData_Impl->bInvisible = TRUE;
        if ( aArg.EqualsIgnoreCaseAscii("-embedding") == sal_True )
            pAppData_Impl->nAppEvent |= DISPATCH_SERVER;
        if ( aArg.EqualsIgnoreCaseAscii("-bean") == sal_True )
        {
            pAppData_Impl->bBean = TRUE;
            pAppData_Impl->bInvisible = TRUE;
        }
        if ( aArg.EqualsIgnoreCaseAscii("-plugin") == sal_True )
        {
            pAppData_Impl->bBean = TRUE;
            pAppData_Impl->bInvisible = TRUE;
            pAppData_Impl->bPlugged = TRUE;
        }
        if ( aArg.CompareIgnoreCaseToAscii("-ucb=",
                                           RTL_CONSTASCII_LENGTH("-ucb="))
                 == COMPARE_EQUAL )
            pAppData_Impl->aUcbUrl
                = aArg.Copy(RTL_CONSTASCII_LENGTH("-ucb="));

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
            // Dies wird als Dateiname interpretiert
            if ( bOpenEvent )
            {
                // Open Event anhaengen
                if ( pAppData_Impl->aOpenList.Len() )
                    pAppData_Impl->aOpenList += APPEVENT_PARAM_DELIMITER;
                pAppData_Impl->aOpenList += aArg;
            }
            else if ( bPrintEvent )
            {
                // Print Event anhaengen
                if( pAppData_Impl->aPrintList.Len() )
                    pAppData_Impl->aPrintList += APPEVENT_PARAM_DELIMITER;
                pAppData_Impl->aPrintList += aArg;
            }
        }
    }

    if ( pAppData_Impl->aOpenList.Len() )
        nEvents |= DISPATCH_OPEN;

    if ( pAppData_Impl->aPrintList.Len() )
        nEvents |= DISPATCH_PRINT;

    return nEvents;
}

//---------------------------------------------------------------------------
void SfxApplication::StartUpScreen( const char* pLabelPrefix )
{
    // Label-DLL mit diversen Resourcen fuer OEM-Ver. etc. (Intro, Titel, About)
    pAppData_Impl->bBean = FALSE;
    pAppData_Impl->nAppEvent = ParseCommandLine_Impl();
    if ( pLabelPrefix )
    {
        // versuchen, die Label-DLL zu erzeugen
        pAppData_Impl->pLabelResMgr = CreateResManager( pLabelPrefix );

        // keine separate Label-DLL vorhanden?
        if ( !pAppData_Impl->pLabelResMgr )
            // dann den ResMgr vom Executable verwenden
            pAppData_Impl->pLabelResMgr = new ResMgr;

        // Intro nur anzeigen, wenn normaler Start (kein Print/Server etc.)
        if ( !pAppData_Impl->bInvisible && !pAppData_Impl->bMinimized &&
             pAppData_Impl->nAppEvent != DISPATCH_PRINT && pAppData_Impl->nAppEvent != DISPATCH_SERVER )
        {
            // und es nicht ausgeschaltet ist
            String aIntroIni = pAppIniMgr->Get(SFX_KEY_SHOW_INTRO);
            ULONG nDisplayTime = ULONG(aIntroIni.ToInt32());

            if ( 2 == nDisplayTime || aIntroIni.Len() < 1 )
            {
                // 2 oder 'kein Ini-Eintrag' bedeutet
                // Einstellungen aus dem System holen
                AllSettings aSettings = Application::GetSettings();
                Application::MergeSystemSettings( aSettings );
                nDisplayTime = aSettings.GetStyleSettings().GetLogoDisplayTime();
            }

            if ( nDisplayTime != LOGO_DISPLAYTIME_NOLOGO )
            {
                USHORT nResId = RID_DEFAULTINTRO;
                if ( Application::IsRemoteServer() )
                    nResId = RID_DEFAULTINTRO_PORTAL;
                // die ggf. im System eingestellte Zeit wird hier nicht ber"ucksichtigt
                pImp->pIntro = new IntroWindow_Impl(
                    Bitmap( ResId( nResId, pAppData_Impl->pLabelResMgr ) ) );
            }
        }
    }
    else
    {
        pAppData_Impl->bBean = TRUE;
        pAppData_Impl->bInvisible = TRUE;
    }

    // merken, falls Applikation normal gestartet wurde
    if ( 0 == pAppData_Impl->nAppEvent || DISPATCH_OPEN == pAppData_Impl->nAppEvent )
        pAppData_Impl->bDirectAliveCount = TRUE;
}

void SfxApplication::Main( )
{
}

//--------------------------------------------------------------------
#if defined( MAC )
    void InstallAppleScriptHdl();
#endif

//-------------------------------------------------------------------------

IMPL_LINK( SfxApplication, LateInitNewMenu_Impl, void*, pvoid)
{
    DBG_PROFSTART(SfxAppMainNewMenu);
    pAppData_Impl->GetPopupMenu( SID_NEWDOCDIRECT );
    DBG_PROFSTOP(SfxAppMainNewMenu);
    return 0;
}

IMPL_LINK( SfxApplication, LateInitWizMenu_Impl, void*, pvoid)
{
    DBG_PROFSTART(SfxAppMainWizMenu);
    pAppData_Impl->GetPopupMenu( SID_AUTOPILOTMENU );
    DBG_PROFSTOP(SfxAppMainWizMenu);
    return 0;
}

//-------------------------------------------------------------------------

void SfxApplication::InsertLateInitHdl(const Link& rLink)
{
    if ( Application::IsInExecute() )
        Application::PostUserEvent( rLink );
    else
    {
        if ( !pAppData_Impl->pInitLinkList )
            pAppData_Impl->pInitLinkList = new SfxInitLinkList;

        Link *pLink = new Link;
        *pLink = rLink;
        USHORT nCount = ( USHORT ) pAppData_Impl->pInitLinkList->Count();
        pAppData_Impl->pInitLinkList->Insert(pLink, nCount);
    }
}

void SfxApplication::ForcePendingInitFactories()
{
    List& rList = Get_Impl()->aPendingInitFactories;
    USHORT nPos = (USHORT) rList.Count();
#if LATEINIT
    DBG_ASSERT( !nPos, "Filter nicht im LateInit" );
#endif
    while( nPos = rList.Count() )
    {
        SfxObjectFactory* pFac = (SfxObjectFactory*)rList.Remove( --nPos );
        pFac->DoInitFactory();
    }
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxApplication, LateInitTimerHdl_Impl, void*, pvoid)
{
    // Ersten Link aus der Liste holen und ausf"uhren
    Link *pLink = (*pAppData_Impl->pInitLinkList)[0];
    pLink->Call(0);

    // Link entfernen
    pAppData_Impl->pInitLinkList->Remove(0);
    delete pLink;

    // Timer wieder starten, wenn noch weitere Links da sind
    if ( pAppData_Impl->pInitLinkList->Count() )
        pAppData_Impl->aLateInitTimer.Start();
    else
    {
        // LateInit ist fertig
        DELETEZ (pAppData_Impl->pInitLinkList);
        pAppIniMgr->ResetLock();
    }
    return 0;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxApplication, StartWelcomeScreenHdl_Impl, void*, EMPTYARG )
{
    // User-Event kommt zu schnell, also 0,5s warten, wenn er nicht sowieso schon da ist.
    static Timer* pTimer = NULL;
    if ( !pTimer && !SFX_APP()->GetHelpPI() )
    {
        pTimer = new Timer;
        pTimer->SetTimeout( 500 );
        pTimer->SetTimeoutHdl( LINK( this, SfxApplication, StartWelcomeScreenHdl_Impl ) );
        pTimer->Start();
        return 0;
    }

    delete pTimer;
    pTimer = NULL;

    if ( SvtHelpOptions().IsWelcomeScreen() )
    {
/*
        ULONG nId = 0;
        Config aConfig( SfxHelp_Impl::GetHelpAgentConfig() );
        ImplSetLanguageGroup( aConfig, String::CreateFromAscii("WelcomeScreen"), TRUE );
        USHORT nTips = aConfig.GetKeyCount();
        for ( USHORT nTip = 0; nTip < nTips; nTip++ )
        {
            if ( (BOOL)(USHORT)(aConfig.ReadKey( nTip ).ToInt32()) )
            {
                nId = aConfig.GetKeyName( nTip ).ToInt32();
                break;
            }
        }

        if ( nId )
        {
            SetChildWindow( SID_TIPWINDOW, TRUE );

            // Das muss irgendwie in den ctor des Fensters
            String aHFile( "tips.svh" );
            SfxHelp::ShowHelp( nId, TRUE, aHFile, TRUE );
            SfxHelpPI* pHelpPI = SFX_APP()->GetHelpPI();
            if ( pHelpPI )
            {
                pHelpPI->SetWelcomeMode( TRUE );
                aConfig.WriteKey( nId, (USHORT)0 ); // gleich als gelesen flaggen
            }
        }
 */
    }
    return 1;
}

//-------------------------------------------------------------------------

SfxFilterMatcher& SfxApplication::GetFilterMatcher()
{
    if( !pAppData_Impl->pMatcher )
    {
        pAppData_Impl->pMatcher = new SfxFilterMatcher( TRUE );
        URIHelper::SetMaybeFileHdl( STATIC_LINK(
            pAppData_Impl->pMatcher, SfxFilterMatcher, MaybeFileHdl_Impl ) );
    }
    return *pAppData_Impl->pMatcher;
}

//-------------------------------------------------------------------------

BOOL SfxApplication::IsStandalone() const
{
    // Wenn ohne UI gestartet, nat"urlich niemals integrierter Desktop
    if ( pAppData_Impl->bBean )
        return TRUE;

    String aExt = SFX_INIMANAGER()->Get( String::CreateFromAscii( "Common"), 0, 0, String::CreateFromAscii("FullScreenDesktop") );
    if ( aExt.Len() )
    {
        // INI-Eintrag auswerten
        return !( (BOOL) (USHORT) aExt.ToInt32() );
    }
    else
    {
        // Defaults: bei AppServer kein integrierter Desktop, bei FatOffice
        // unter WNT integriert, ansonsten nicht
        if ( Application::IsRemoteServer() )
            return TRUE;
#ifdef WNT
        return FALSE;
#else
        return TRUE;
#endif
    }
}
