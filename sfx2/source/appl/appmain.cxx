/*************************************************************************
 *
 *  $RCSfile: appmain.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:56:25 $
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
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
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
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <svtools/startoptions.hxx>
#endif
#include <svtools/itempool.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/helpopt.hxx>
#include <vos/process.hxx>

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
#include "intro.hxx"
#include "msgpool.hxx"
#include "cfgmgr.hxx"
#include "accmgr.hxx"
#include "mnumgr.hxx"
#include "stbmgr.hxx"
#include "imgmgr.hxx"
#include "appuno.hxx"
#include "objuno.hxx"
#include "app.hrc"
#include "docfile.hxx"
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif

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
}

USHORT SfxApplication::ParseCommandLine_Impl()
{
    USHORT nEvents = 0;                 // return value ( event mask )

    BOOL   bPrintEvent = FALSE;
    BOOL   bOpenEvent  = TRUE;

    ::vos::OExtCommandLine aCmdLine;
    USHORT nCount = aCmdLine.getCommandArgCount();
    for( USHORT i=0; i < nCount; i++ )
    {
        String aArg;
        ::rtl::OUString aDummy;
        aCmdLine.getCommandArg( i, aDummy );
        aArg = aDummy;

        if ( aArg.EqualsIgnoreCaseAscii("-minimized") == sal_True )
            pAppData_Impl->bMinimized = TRUE;
        else if ( aArg.EqualsIgnoreCaseAscii("-invisible") == sal_True )
            pAppData_Impl->bInvisible = TRUE;
        else if ( aArg.EqualsIgnoreCaseAscii("-embedding") == sal_True )
            pAppData_Impl->nAppEvent |= DISPATCH_SERVER;
        else if ( aArg.EqualsIgnoreCaseAscii("-bean") == sal_True )
        {
            pAppData_Impl->bBean = TRUE;
            pAppData_Impl->bInvisible = TRUE;
        }
        else if ( aArg.EqualsIgnoreCaseAscii("-plugin") == sal_True )
        {
            pAppData_Impl->bBean = TRUE;
            pAppData_Impl->bInvisible = TRUE;
            pAppData_Impl->bPlugged = TRUE;
        }
        else if ( aArg.EqualsIgnoreCaseAscii("-server") )
            pAppData_Impl->bServer = true;
        else if ( aArg.CompareIgnoreCaseToAscii("-portal,",
                                                RTL_CONSTASCII_LENGTH(
                                                    "-portal,"))
                      == COMPARE_EQUAL )
            pAppData_Impl->aPortalConnect
                = aArg.Copy(RTL_CONSTASCII_LENGTH("-portal,"));

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
void SfxApplication::InitLabelResMgr( const char* pLabelPrefix )
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
    if ( !SfxViewFrame::GetFirst( 0,0,FALSE ) )
    {
        pAppData_Impl->aLateInitTimer.Start();
        return 0;
    }

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
#if SUPD<613//MUSTINI
        pAppIniMgr->ResetLock();
#endif
    }
    return 0;
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

SfxFilterMatcher& SfxApplication::GetFilterMatcher()
{
    if( !pAppData_Impl->pMatcher )
    {
        pAppData_Impl->pMatcher = new SfxFilterMatcher();
        URIHelper::SetMaybeFileHdl( STATIC_LINK(
            pAppData_Impl->pMatcher, SfxFilterMatcher, MaybeFileHdl_Impl ) );
    }
    return *pAppData_Impl->pMatcher;
}

//-------------------------------------------------------------------------

BOOL SfxApplication::IsStandalone() const
{
#if SUPD<613//MUSTINI
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
#else//MUSTINI
    return FALSE;
#endif
}
