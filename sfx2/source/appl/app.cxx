/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:41 $
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

#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif // UNX

#include "app.hxx"
#include "frame.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _DRAG_HXX //autogen
#include <vcl/drag.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _FILELIST_HXX //autogen
#include <so3/filelist.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _EXTATTR_HXX //autogen
#include <svtools/extattr.hxx>
#endif
#ifndef _INET_WRAPPER_HXX
#include <inet/wrapper.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/svdde.hxx>
#include <tools/urlobj.hxx>
#include <tools/tempfile.hxx>
#pragma hdrstop

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include <basic/basmgr.hxx>

#include <appuno.hxx>
#include "sfxhelp.hxx"
#include "request.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "arrdecl.hxx"
#include "progress.hxx"
#include "objsh.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "cfgmgr.hxx"
#include "fltfnc.hxx"
#include "nfltdlg.hxx"
#include "iodlg.hxx"
#include "new.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "genlink.hxx"
#include "accmgr.hxx"
#include "tbxmgr.hxx"
#include "mnumgr.hxx"
#include "topfrm.hxx"
#include "newhdl.hxx"
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "interno.hxx"
#include "ipenv.hxx"
#include "intfrm.hxx"
#include "virtmenu.hxx"
#include "module.hxx"
#include "sfxdir.hxx"
#include "event.hxx"
#include "oregdlg.hxx"
#include "appimp.hxx"

#ifdef DBG_UTIL
#include "tbxctrl.hxx"
#include "stbitem.hxx"
#include "mnuitem.hxx"
#endif

#if defined( WIN ) || defined( WNT ) || defined( OS2 )
#define DDE_AVAILABLE
#endif

#include <svtools/saveopt.hxx>
#include <svtools/undoopt.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/workingsetoptions.hxx>

// Static member
SfxApplication* SfxApplication::pApp = NULL;

SfxApplication* SfxApplication::GetOrCreate()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
#if 0                                   // SFX on demand
    if ( !pApp )
    {
        SfxApplication *pNew = new SfxApplication;
        pNew->StartUpScreen( NULL );
        SetApp( pNew );
    }
#endif
    return pApp;
}

void SfxApplication::SetApp( SfxApplication* pSfxApp )
{
    static ::osl::Mutex aProtector;
    ::osl::MutexGuard aGuard( aProtector );

    DBG_ASSERT( !pApp, "SfxApplication already created!" );
    if ( pApp )
        DELETEZ( pApp );

    pApp = pSfxApp;

    // at the moment a bug may occur when Initialize_Impl returns FALSE, but this is only temporary because all code that may cause such a
    // fault will be moved outside the SFX
    pApp->Initialize_Impl();
}

SfxApplication::SfxApplication()
    : _nFeatures( ULONG_MAX )
    , pImp( 0 )
    , pAppData_Impl( 0 )
    , pMenuMgr( 0 )
    , pAcceleratorMgr( 0 )
    , pStatusBarMgr( 0 )
    , pAppDispat( 0 )
    , bDispatcherLocked( sal_False )
    , pResMgr( 0 )
#ifdef ENABLE_INIMANAGER//MUSTINI
    , pAppIniMgr( 0 )
#endif
    , pCfgMgr( 0 )
    , pSlotPool( 0 )
    , pInterfaces( 0 )
    , bInInit( sal_False )
    , bInExit( sal_False )
    , bDowning( sal_True )
    , bCreatedExternal( sal_False )
    , pOptions( 0 )
    , pViewFrame( 0 )
    , pImageMgr( 0 )
    , nInterfaces( 0 )
{
    pImp = new SfxApplication_Impl;
    pImp->bConfigLoaded = sal_False;
    pImp->pEmptyMenu = 0;
    pImp->nDocNo = 0;
    pImp->pIntro = 0;
    pImp->pTbxCtrlFac = 0;
    pImp->pStbCtrlFac = 0;
    pImp->pViewFrames = 0;
    pImp->pObjShells = 0;
    pImp->bAutoSaveNow = sal_False;
    pImp->pTemplateDlg = 0;
    pImp->pBasicMgr = 0;
    pImp->pBasicTestWin = 0;
    pImp->pSfxResManager = 0;
    pImp->pSimpleResManager = 0;
    pImp->nWarnLevel = 0;
    pImp->pAutoSaveTimer = 0;
#ifdef ENABLE_INIMANAGER//MUSTINI
    pAppIniMgr = CreateIniManager();
    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->StartListening( *pAppIniMgr );
    pAppData_Impl->UpdateApplicationSettings( pAppIniMgr->IsDontHideDisabledEntries() );
#else
    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->UpdateApplicationSettings( SvtMenuOptions().IsEntryHidingEnabled() );
#endif
    pApp->PreInit();

    pAppData_Impl->pSaveOptions = new SvtSaveOptions;
    pAppData_Impl->pUndoOptions = new SvtUndoOptions;
    pAppData_Impl->pHelpOptions = new SvtHelpOptions;

#ifdef DDE_AVAILABLE
#ifdef PRODUCT
    InitializeDde();
#else
    if( !InitializeDde() )
    {
        ByteString aStr( "Kein DDE-Service moeglich. Fehler: " );
        if( GetDdeService() )
            aStr += ByteString::CreateFromInt32(GetDdeService()->GetError());
        else
            aStr += '?';
        DBG_ASSERT( sal_False, aStr.GetBuffer() )
    }
#endif
#endif
}

SfxApplication::~SfxApplication()
{
    DELETEZ( pAppData_Impl->pSaveOptions );
    DELETEZ( pAppData_Impl->pUndoOptions );
    DELETEZ( pAppData_Impl->pHelpOptions );
    if ( !bDowning )
        Deinitialize();
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );
    delete pImp;
    delete pAppData_Impl;
#ifdef ENABLE_INIMANAGER//MUSTINI
    SfxIniManager::Close();
#endif
    utl::ConfigManager::RemoveConfigManager();
    pApp = 0;
}

//====================================================================

class SfxResourceTimer : public Timer
{
    sal_uInt16 *pnWarnLevel;
public:
    SfxResourceTimer(sal_uInt16 *pn, sal_uInt32 nTimeOut) : pnWarnLevel(pn)
    { SetTimeout(nTimeOut); Start(); }
    virtual void Timeout() { --*pnWarnLevel; delete this; }
};

//--------------------------------------------------------------------

//====================================================================

void SfxApplication::LockDispatcher
(
    sal_Bool bLock              /*  sal_True
                                schaltet alle SfxDispatcher ein

                                sal_False
                                schaltet alle SfxDispatcher aus */
)

/*  [Beschreibung]

    Mit dieser Methode werden alle Dispatcher der Applikation global
    blockiert (bLock==sal_True) bzw. grundsaetzlich freigegeben
    (bLock==sal_False).

    Unabhaengig von diesem zentralen Schalter kann jeder Dispatcher
    einzeln gelockt sein:

        Dispatcher X    global      =>  gesamt

        gelockt         gelockt     =>  gelockt
        freigegeben     gelockt     =>  gelockt
        gelockt         freigegeben =>  gelockt
        freigegeben     freigegeben =>  freigegeben

    Wenn der aktive Dispatcher gelockt ist, werden keine Requests mehr
    dispatcht.

    [Querverweise]
    <SfxDispatcher::Lock(sal_Bool)> */

{
    bDispatcherLocked = bLock;
    if ( !bLock )
    {
        GetDispatcher_Impl()->InvalidateBindings_Impl( pAppData_Impl->bInvalidateOnUnlock );
        pAppData_Impl->bInvalidateOnUnlock = sal_False;
    }
}

//--------------------------------------------------------------------

SfxObjectShell* SfxApplication::GetActiveObjectShell() const

/*  [Beschreibung]

    Diese Methode liefert einen Zeiger auf die aktive <SfxObjectShell>-
    Instanz oder einen 0-Pointer, falls keine SfxObjectShell-Instanz
    aktiv ist.
*/

{
    if ( pViewFrame )
        return pViewFrame->GetObjectShell();
    return 0;
}

//--------------------------------------------------------------------
#if SUPD<594
sal_uInt32 SfxApplication::InsertEventHdl
(
    const GenLink&  rLink   /*  Link, der auf ein StarView-UserEvent
                                gerufen werden soll. */
)

/*  [Beschreibung]

    Diese Methode fuegt einen Handler fuer ein StarView-UserEnvent
    ein und liefert die Id fuer das Event zurueck.

    PostAppEvent() mit dieser Id ruft daher den eingefuegen Handler.

    Somit koennen verschiedenen, sich gegenseitig unbekannte Programmteile
    in derselben Applikation koexistieren und UserEvents verschicken.
*/

{
    return ( pImp->pEventHdl->Insert( new GenLink(rLink) ) ) + DYNAMIC_USERID_OFFSET;
}

//--------------------------------------------------------------------

void SfxApplication::RemoveEventHdl
(
    sal_uInt32 nId               /*  Id des StarView-UserEvents, das entfernt
                                werden soll. */
)

/*  [Beschreibung]

    Diese Methode entfernt den unter der Id nId eingefuegten Handler
    fuer StarView-UserEvents. Die Id wird damit zur Wiederverwendung
    freigegeben, darf also nicht mehr verwendet werden, bis sie durch
    ein erneutes <SfxApplication::InsertEventHdl()> wieder
    zurueckgegeben wurde.

    */

{
    delete (GenLink*) pImp->pEventHdl->Remove( nId - DYNAMIC_USERID_OFFSET );
}
#endif

//--------------------------------------------------------------------

#if SUPD<594
void SfxApplication::UserEvent
(
    sal_uInt32       nEvent          /*  Id des StarView-UserEvents */,

    void*       pEventData      /*  Event-Daten abhaengig von der Event-Id */
)

/*  [Beschreibung]

    StarView-Handler zum Ausfuehrend eines UserEvents.

    In SFx-Applikationen muessen die Event-Ids durch die Methode
    <SfxApplication::InsertEventHdl()> ermittelt werden. Sollte eine
    Subklasse von SfxApplication diese Methode ueberladen, mu"s die
    Basisimplementierung gerufen werden.

    */

{
    // z.b. ein Channel-Agent kann so Directories updaten
    if ( SID_RELOAD == nEvent && pEventData )
    {
        String aString = S2U( (const char*) pEventData );
        SFX_APP()->Broadcast( SfxDirEntryHint( 0, aString ) );
        return;
    }

    if ( nEvent >= DYNAMIC_USERID_OFFSET )
    {
        GenLink* pLink = (GenLink*) pImp->pEventHdl->Get( nEvent-DYNAMIC_USERID_OFFSET );
        if ( pLink )
        {
            pLink->Call( (SfxHint*) pEventData );
            return;
        }
    }

    if ( nEvent == ULONG_MAX )
    {
        if ( pEventData )
            DELETEZ(pAppData_Impl->pProgress);
        else
        {
            pAppData_Impl->pProgress = new SfxProgress(0, String(SfxResId(RID_PLUGIN)), 0, sal_True);
            pAppData_Impl->pProgress->Lock();
        }
    }
    else
        DBG_ERROR( "unregistered user event occured" );
}
#endif

//--------------------------------------------------------------------

sal_Bool IsTemplate_Impl( const String& aPath )
{
    INetURLObject aObj( aPath, INET_PROT_FILE );
    if ( aObj.getExtension().CompareIgnoreCaseToAscii( "vor" ) == COMPARE_EQUAL )
        return sal_True;

    SvEaMgr aMgr( aPath );
    String aType;

    if ( aMgr.GetFileType(aType) )
    {
        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4EA( aType );
        if( pFilter && pFilter->IsOwnTemplateFormat() )
            return sal_True;
    }

    return sal_False;
}

extern void FATToVFat_Impl( String& );

void SfxApplication::HandleAppEvent( const ApplicationEvent& rAppEvent )
{
    if ( rAppEvent.IsOpenEvent() )
    {
        // die Parameter enthalten die zu "offnenden Dateien
        for(sal_uInt16 i=0;i<rAppEvent.GetParamCount();i++)
        {
            // Dateiname rausholen
            String aName( rAppEvent.GetParam(i) );
            if ( COMPARE_EQUAL == aName.CompareToAscii("/userid:",8) )
                continue;
#ifdef WNT
            FATToVFat_Impl( aName );
#endif
            SfxStringItem aFileName( SID_FILE_NAME, aName );

#ifdef APPEVENT_DBG
            aStream << "Open: " << (const char *)aFileName.GetValue();
#endif
            // Art, Existens und Groesse
            INetURLObject aURL( aFileName.GetValue(), INET_PROT_FILE );
            sal_Bool bIsFileURL = INET_PROT_FILE == aURL.GetProtocol();

            // ist ein oeffnen grundsaetzlich moeglich
            if ( TRUE ) // (pb)
            {
                // ist es eine Vorlage?
                if ( bIsFileURL && IsTemplate_Impl( aURL.GetMainURL() ) )
                {
#ifdef APPEVENT_DBG
                    aStream << " Neues Dokument aus Vorlage angelegt\n";
#endif
                    // neue Datei aus der Vorlage erzeugen
                    pAppDispat->Execute( SID_NEWDOC, SFX_CALLMODE_SYNCHRON, &aFileName, 0L );
                }
                else
                {
#ifdef APPEVENT_DBG
                    aStream << " Neues Dokument geoeffnet\n";
#endif
                    // ::com::sun::star::util::URL "offnen
                    if ( !DocAlreadyLoaded( aFileName.GetValue(), sal_True, sal_True, sal_False ) )
                    {
                        SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_False );
                        SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
                        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:OpenEvent") );
                        pAppDispat->Execute( SID_OPENDOC,
                                SFX_CALLMODE_SYNCHRON, &aTargetName,
                                &aFileName, &aNewView, &aReferer, 0L );
                    }
                }
            }
            else
            {
                // ACHTUNG: keine Fehlermeldung bei '.' (unterdr"uckt OpenClients)
                HACK(Fehlermeldung fehlt);
#ifdef APPEVENT_DBG
                aStream << " FEHLER\n";
#endif
            }
        }
    }
    else if(rAppEvent.IsPrintEvent() )
    {
        // "uber die Parameter iterieren (zu druckende Dateien + Druckername)
        SfxStringItem aPrinterName(SID_PRINTER_NAME, String());
        for (sal_uInt16 i=0;i<rAppEvent.GetParamCount();i++)
        {
            // Druckername?
            String aArg(rAppEvent.GetParam(i));
            if(aArg.Len()>1 && *aArg.GetBuffer()=='@')
            {
                aPrinterName.SetValue( aArg.Copy(1) );
                continue;
            }

            // Datei "offnen -- immer neue ::com::sun::star::sdbcx::View erzeugen
            SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
            SfxStringItem aFileName( SID_FILE_NAME, aArg );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, sal_True);
            SfxBoolItem aHidden(SID_HIDDEN, sal_True);
            SfxBoolItem aSilent(SID_SILENT, sal_True);
            const SfxPoolItem *pRet = pAppDispat->Execute( SID_OPENDOC,
                    SFX_CALLMODE_SYNCHRON, &aTargetName,
                    &aFileName, &aNewView, &aHidden, &aSilent, 0L );
            if ( !pRet )
                continue;

            // die neue ::com::sun::star::sdbcx::View des Dokuments ermitteln
            const SfxViewFrameItem *pFrameItem =
                PTR_CAST(SfxViewFrameItem, pRet);
            if ( pFrameItem && pFrameItem->GetFrame() )
            {
                // "uber die ::com::sun::star::sdbcx::View drucken
                SfxViewFrame *pFrame = pFrameItem->GetFrame();
                SfxBoolItem aSilent( SID_SILENT, sal_True );
                pFrame->GetDispatcher()->Execute( SID_PRINTDOC,
                        SFX_CALLMODE_SYNCHRON,
                        &aPrinterName, &aSilent, 0L );
                pFrame->GetFrame()->DoClose();
            }
        }
    }
}

//--------------------------------------------------------------------

long SfxAppFocusChanged_Impl( void* pObj, void* pArg )
{
    SfxApplication *pApp = SFX_APP();
    if ( pApp && !pApp->IsDowning() )
    {
        Help* pHelp = Application::GetHelp();
        Window* pFocusWindow = Application::GetFocusWindow();
        if ( pHelp && pFocusWindow )
        {
            sal_uInt32 nId = pFocusWindow->GetHelpId();
            while ( !nId && pFocusWindow )
            {
                pFocusWindow = pFocusWindow->GetParent();
                nId = pFocusWindow ? pFocusWindow->GetHelpId() : 0;
            }
            ((SfxHelp_Impl*)pHelp)->SlotExecutedOrFocusChanged(
                nId, sal_False, SvtHelpOptions().IsHelpAgentAutoStartMode() );
        }
    }

    return 0;
}

void SfxApplication::FocusChanged()
{
    static AsynchronLink *pFocusCallback = new AsynchronLink( Link( 0, SfxAppFocusChanged_Impl ) );
    pFocusCallback->Call( this, sal_True );
}

//--------------------------------------------------------------------

const String& SfxApplication::GetLastDir_Impl() const

/*  [Beschreibung]

    Interne Methode, mit der im SFx das zuletzt mit der Methode
    <SfxApplication::SetLastDir_Impl()> gesetzte Verzeichnis
    zurueckgegeben wird.

    Dieses ist i.d.R. das zuletzt durch den SfxFileDialog
    angesprochene Verzeichnis.

    [Querverweis]
    <SfxApplication::SetLastDir_Impl()>
*/

{
    return pAppData_Impl->aLastDir;
}

const String& SfxApplication::GetLastSaveDirectory() const

/*  [Beschreibung]

    Wie <SfxApplication::GetLastDir_Impl()>, nur extern

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    return GetLastDir_Impl();
}

//--------------------------------------------------------------------

void SfxApplication::SetLastDir_Impl
(
    const String&   rNewDir     /*  kompletter Verzeichnis-Pfad als String */
    )

/*  [Beschreibung]

    Interne Methode, mit der ein Verzeichnis-Pfad gesetzt wird, der
    zuletzt (z.B. durch den SfxFileDialog) angesprochen wurde.

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    pAppData_Impl->aLastDir = rNewDir;
}

//--------------------------------------------------------------------

const String& SfxApplication::GetLastFilter_Impl() const
{
    return pAppData_Impl->aLastFilter;
}

//--------------------------------------------------------------------

void SfxApplication::SetLastFilter_Impl( const String &rNewFilter )
{
    pAppData_Impl->aLastFilter = rNewFilter;
}

//--------------------------------------------------------------------
#if SUPD<606
SfxDispatcher& SfxApplication::GetDispatcher()
{
    return pViewFrame? *pViewFrame->GetDispatcher(): *pAppDispat;
}

SfxBindings& SfxApplication::GetBindings() const
{
    DBG_ASSERT( pViewFrame, "No ViewFrame available!" );
    return pViewFrame? pViewFrame->GetBindings(): SfxViewFrame::GetFirst( 0, 0, sal_False )->GetBindings();
}
#endif

SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pViewFrame? pViewFrame->GetDispatcher(): pAppDispat;
}

//--------------------------------------------------------------------
void SfxApplication::SetViewFrame( SfxViewFrame *pFrame )
{
    if( pFrame && !pFrame->IsSetViewFrameAllowed_Impl() )
        return;

    if ( pFrame != pViewFrame )
    {
        SfxInPlaceFrame *pOld = PTR_CAST( SfxInPlaceFrame, pViewFrame );
        SfxInPlaceFrame *pNew = PTR_CAST( SfxInPlaceFrame, pFrame );
        FASTBOOL bTaskActivate = !pNew;
        SfxViewFrame *pContainer = pViewFrame;
        while ( pContainer && pContainer->GetParentViewFrame_Impl() )
            pContainer = pContainer->GetParentViewFrame_Impl();

        BOOL bDocWinActivate = pContainer && pFrame &&
                ( pContainer->GetTopViewFrame() == pFrame || pFrame->GetTopViewFrame() == pContainer );

        if ( bTaskActivate )
        {
            if ( pViewFrame )
            {
                // BeamerConfig sichern
                pViewFrame->GetFrame()->Deactivate_Impl();

                // DeactivateEvent f"ur den alten ViewFrame verschicken
                NotifyEvent( SfxEventHint( SFX_EVENT_DEACTIVATEDOC, pViewFrame->GetObjectShell() ) );

                // Ggf. auch InPlaceDeactivate
                // Daf"ur den aktiven ContainerFrame suchen
                if ( pOld )
                {
                    // Falls aktiver IPClient, diesen deaktivieren
                    SvInPlaceClient *pCli = pContainer->GetViewShell() ? pContainer->GetViewShell()->GetIPClient() : NULL;
                    if ( pCli && pCli->GetProtocol().IsUIActive() )
                    {
                        if ( bDocWinActivate )
                        {
                            pCli->GetIPObj()->GetIPEnv()->DoShowUITools( sal_False );
                            pCli->GetProtocol().DocWinActivate( sal_False );
                        }
                        else
                            pCli->GetProtocol().TopWinActivate( sal_False );
                    }
                }
            }
        }

        SfxViewFrame *pOldContainerFrame = pViewFrame;
        if( pOldContainerFrame )
        {
            // Wenn der alte Frame ein IPFrame ist, mu\s dessen ContainerDokument aktiviert werden,
            // der IPFrame wurde schon im Top/DocWinDeactivate
            if ( pOld )
                pOldContainerFrame = pOld->GetParentViewFrame_Impl();

            if ( bTaskActivate && pOldContainerFrame != pViewFrame )
                NotifyEvent( SfxEventHint( SFX_EVENT_DEACTIVATEDOC, pOldContainerFrame->GetObjectShell() ) );

            pOldContainerFrame->DoDeactivate( bTaskActivate, pFrame );
        }

        pViewFrame = pFrame;

        // Jetzt ist der ViewFrame gesetzt, das TopWindow kann abgefragt werden
        SfxWorkWindow* pWork = pViewFrame ? pViewFrame->GetFrame()->GetWorkWindow_Impl() : NULL;
        Window* pWin = pWork ? pWork->GetTopWindow() : NULL;
        Application::SetDefDialogParent( pWin );

        const SfxObjectShell* pSh = pViewFrame ? pViewFrame->GetObjectShell() : 0;
        if ( !pSh )
        {
            // Wenn es ein Dokument gibt, wird die BaseURL im Activate gesetzt
#if SUPD<613//MUSTINI
            INetURLObject aObject( GetIniManager()->Get( SFX_KEY_WORK_PATH ), INET_PROT_FILE );
#else
            INetURLObject aObject( SvtPathOptions().GetWorkPath(), INET_PROT_FILE );
#endif
            aObject.setFinalSlash();
            INetURLObject::SetBaseURL( aObject.GetMainURL() );
        }

        // Activate mit sal_True auch wenn die zu aktivierenden Bindings gerade
        // keinen Dispatcher haben
        if ( pViewFrame && bTaskActivate )
            pViewFrame->GetObjectShell()->PostActivateEvent_Impl();

        if( pViewFrame )
            pViewFrame->DoActivate(
                bTaskActivate ||
                pViewFrame->GetBindings().GetDispatcher_Impl() !=
                pViewFrame->GetDispatcher(), pOldContainerFrame );

        if( pOldContainerFrame && pOldContainerFrame->GetProgress() )
            pOldContainerFrame->GetProgress()->Suspend();

        // Beim Browsen kann es passieren, da\s gerade keine ViewShell da ist
        if ( pViewFrame && !pViewFrame->GetViewShell() )
            return;

        if ( pViewFrame )
        {
            if ( pNew )
            {
                pNew->GetEnv_Impl()->ActivateConfig();
            }
            else
            {
                SfxObjectShell *pObjSh = pViewFrame->GetObjectShell();
                if ( pObjSh->GetConfigManager())
                {
                    pObjSh->GetConfigManager()->ActivateTask( pViewFrame );
                    pObjSh->GetConfigManager()->SetParent(pAppData_Impl->pAppCfg);
                    pObjSh->GetConfigManager()->Activate(pCfgMgr);
                }
                else
                {
                    pAppData_Impl->pAppCfg->ActivateTask( pViewFrame );
                    pAppData_Impl->pAppCfg->Activate(pCfgMgr);
                }

                SfxDispatcher* pDisp = pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);

                SfxProgress *pProgress = pViewFrame->GetProgress();
                if ( !pProgress )
                    pProgress = pAppData_Impl->pProgress;
                if ( pProgress )
                {
                    if( pProgress->IsSuspended() )
                        pProgress->Resume();
                    else
                        pProgress->SetState( pProgress->GetState() );
                }

                // Falls aktiver IPClient, diesen aktivieren
                SvInPlaceClient *pCli = pViewFrame->GetViewShell()->GetIPClient();
                if ( pCli && pCli->GetProtocol().IsUIActive() )
                {
                    if ( bDocWinActivate )
                    {
                        pCli->GetIPObj()->GetIPEnv()->DoShowUITools( sal_True );
                        pCli->GetProtocol().DocWinActivate( sal_True );
                    }
                    else
                        pCli->GetProtocol().TopWinActivate( sal_True );
                }
            }
        }
        else
        {
            pCfgMgr->ActivateTask( NULL );
            pAppData_Impl->pAppCfg->Activate( pCfgMgr );
        }
    }
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------

sal_uInt32 SfxApplication::DetectFilter( const String &rFileName,
                                    const SfxFilter **ppFilter,
                                    sal_uInt16 nFilterClass )
{
    SfxMedium aSfxMedium(rFileName,(STREAM_READ | STREAM_SHARE_DENYNONE),sal_False);

    return DetectFilter(aSfxMedium, ppFilter, nFilterClass );
}

//-------------------------------------------------------------------------

sal_uInt32 SfxApplication::DetectFilter(
    SfxMedium& rMedium,const SfxFilter **ppFilter, sal_uInt16 nFilterClass )
{
    const SfxFilter *pFilter=0;
    SvEaMgr aMgr( rMedium.GetName() );
    String aType;
    if ( !SfxObjectFactory::HasObjectFactories() )
        return 1; HACK(Error-Code verwenden) ;

    SfxFilterMatcher rMatcher( SfxObjectFactory::GetDefaultFactory().GetFilterContainer()  );
    if( aMgr.GetFileType( aType ))
        pFilter = rMatcher.GetFilter4EA( aType );

    if( !pFilter)
    {
        if ( !rMedium.IsRemote() )
        {
            SvStorageRef aStor = rMedium.GetStorage();
            if ( !aStor.Is() )
                return ERRCODE_IO_GENERAL;
            pFilter = rMatcher.GetFilter4ClipBoardId(aStor->GetFormat());
        }
        else
        {
            // Finden anhand der Extension
            pFilter = rMatcher.GetFilter4Extension(
                rMedium.GetURLObject().GetName() );
            if ( pFilter && pFilter->UsesStorage() )
                pFilter = 0;
        }
    }

    if(pFilter)
        *ppFilter=pFilter;

    return pFilter? 0: 1; HACK(Error-Code verwenden)
}



//--------------------------------------------------------------------

SfxNewFileDialog*  SfxApplication::CreateNewDialog()
{
    return new SfxNewFileDialog(GetTopWindow(), SFXWB_DOCINFO | SFXWB_PREVIEW );
}

//--------------------------------------------------------------------

const SfxFilter* SfxApplication::GetFilter
(
    const SfxObjectFactory &rFact,
    const String &rFilterName
    )   const
{
    DBG_ASSERT( rFilterName.Search( ':' ) == STRING_NOTFOUND,
                "SfxApplication::GetFilter erwartet unqualifizierte Namen" );
    return rFact.GetFilterContainer()->GetFilter4FilterName(rFilterName);
}

//--------------------------------------------------------------------

short SfxApplication::QuerySave_Impl( SfxObjectShell& rDoc, sal_Bool bAutoSave )
{
    if ( !rDoc.IsModified() )
        return RET_NO;

    String aMsg( SfxResId( STR_ISMODIFIED ) );
    aMsg.SearchAndReplaceAscii( "%1", rDoc.GetTitle() );

    SfxFrame *pFrame = SfxViewFrame::GetFirst(&rDoc)->GetFrame();
    pFrame->Appear();

    WinBits nBits = WB_YES_NO_CANCEL;
    nBits |= bAutoSave ? WB_DEF_YES : WB_DEF_NO;
    QueryBox aBox( &pFrame->GetWindow(), nBits, aMsg );

    if ( bAutoSave )
        aBox.SetText( String( SfxResId( STR_AUTOSAVE ) ) );

    return aBox.Execute();
}

//--------------------------------------------------------------------

sal_Bool Drop1_Impl( sal_uInt16 nSID, const String &rFile, sal_Bool bHidden,
                 SfxExecuteItem *&rpExecItem, SfxExecuteItem *&rpPrintItem )
{
    // Parameter bestimmen
    SfxStringItem aFileNameItem(SID_FILE_NAME, rFile);
    SfxBoolItem aHiddenItem(SID_HIDDEN, bHidden);
    SfxStringItem aRefererItem(SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
    SfxExecuteItem* pOld = rpExecItem;
    if( !rpPrintItem )
        rpExecItem = new SfxExecuteItem(
            SID_SUBREQUEST, nSID, SFX_CALLMODE_SYNCHRON,
            &aFileNameItem, &aHiddenItem, &aRefererItem,
            (SfxPoolItem*)rpExecItem, 0L );
    else
        rpExecItem = new SfxExecuteItem(
            SID_SUBREQUEST, nSID, SFX_CALLMODE_SYNCHRON,
                &aFileNameItem, &aHiddenItem, &aRefererItem, rpPrintItem,
            (SfxPoolItem*)rpExecItem, 0L );
    delete pOld;
    return sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::Drop_Impl( sal_uInt16 nSID, DropEvent& rEvt )

/*  [Beschreibung]

    F"uhrt 'nSID' mit den Files aus, die im DragServer stehen.
*/

{
    // Actions bestimmen
    sal_uInt16 n2ndSID = 0;     // nach SID_OPENDOC auszuf"uhren
    sal_Bool bHidden = sal_False;   // unsichtbar "offnen
    switch ( nSID )
    {
        case SID_OPENDOC:
        case SID_OPENURL:
            break;

        case SID_PRINTDOC:
        case SID_PRINTDOCDIRECT:
            n2ndSID = SID_PRINTDOCDIRECT;
            bHidden = sal_True;
            break;

        case SID_NEWDOC:
        case SID_NEWDOCDIRECT:
            nSID = SID_NEWDOC;
            break;

        default:
            // unbekannte SID
            return sal_False;
    }

    // "uber die Items im DragServer iterieren
    const sal_uInt16 nCount = DragServer::GetItemCount();
    sal_Bool bSuccess = sal_False;

    SfxExecuteItem* pExecItem = 0;
    SfxExecuteItem* pPrintItem = n2ndSID ? new SfxExecuteItem(
        SID_AFTEROPENEVENT, n2ndSID, SFX_CALLMODE_SYNCHRON ) : 0;

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        // Format erkennen
        String aFile;
        INetBookmark aBmk;
        if ( DragServer::HasFormat(i, FORMAT_FILE_LIST) )
        {
            // SvData basteln
            SvData aData( FORMAT_FILE_LIST );
            SvDataObjectRef xDataObj = SvDataObject::PasteDragServer( rEvt );
            xDataObj->GetData( &aData );

            // Daten holen
            FileList aFileList;
            FileList* pFileList = &aFileList;
            aData.GetData( (SvDataCopyStream**)&pFileList, pFileList->Type() );
            for ( sal_uInt16 n = (sal_uInt16)aFileList.Count(); n--; )
                Drop1_Impl( nSID, aFileList.GetFile(n), bHidden,
                            pExecItem, pPrintItem );
        }
        else if ( DragServer::HasFormat(i, FORMAT_FILE) )
        {
               String aFile = DragServer::PasteFile(i);
            Drop1_Impl( nSID, aFile, bHidden, pExecItem, pPrintItem );
        }
        else if ( aBmk.PasteDragServer(i) )
        {
            // Format via ::com::sun::star::text::Bookmark rausholen
            Drop1_Impl( nSID, aBmk.GetURL(), bHidden, pExecItem, pPrintItem );
        }
    }

    if( pExecItem )
    {
        // Fuer Mac muss erstes Execute Asyncron kommen
        pExecItem->SetCallMode( SFX_CALLMODE_ASYNCHRON );
        pViewFrame->GetDispatcher()->Execute( *pExecItem );
        delete pExecItem;
    }


    // scheinbar annehmen, sonst kommt zweites Drop im falschen Window
    return DROP_COPY == rEvt.GetAction();
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::QueryDrop_Impl( sal_uInt16 nSID, DropEvent& rEvt )

/*  [Beschreibung]

    QueryDrop-Handler; wird in der Regel aus dem QueryDrop() an den
    Windows gerufen;
    er returned sal_True, wenn FORMAT_FILE im DragServer vorliegt.
*/
{
    if ( nSID == SID_OPENDOC || nSID == SID_OPENURL ||
         nSID == SID_PRINTDOC || nSID == SID_PRINTDOCDIRECT ||
         nSID == SID_NEWDOC || nSID == SID_NEWDOCDIRECT )
    {
        const sal_uInt16 nCount = DragServer::GetItemCount();
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            if ( INetBookmark::DragServerHasFormat( i ) ||
                 DragServer::HasFormat(i, FORMAT_FILE) ||
                 DragServer::HasFormat(i, FORMAT_FILE_LIST) )
            {
                // if ( rEvt.IsDefaultAction() )
                {
                    rEvt.SetAction( DROP_COPY );
                    return sal_True;
                }
                if ( rEvt.GetAction() == DROP_COPY )
                    return sal_True;
            }
        }
    }
    return sal_False;
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::Drop( DropEvent& rEvt )

/*  [Beschreibung]

    Dieser Drop-Handler kann von den Applikationen, die i.d.R. keine
    Ableitug vom SfxApplicationWindow haben, "uberladen werden. Er wird in
    der Regel aus dem Drop() an den Windows gerufen.

    In der Basisimplementierung wird versucht, alle Elemente im DragServer
    als Datei zu oeffnen, indem sie als Event ueber den Dispatcher verschickt
    werden.
*/

{
    return Drop_Impl( SID_OPENDOC, rEvt );
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::QueryDrop( DropEvent& rEvt )

/*  [Beschreibung]

    Dieser QueryDrop-Handler kann von den Applikationen, die i.d.R. keine
    Ableitug vom SfxApplicationWindow haben, "uberladen werden. Er wird in
    der Regel aus dem QueryDrop() an den Windows gerufen.

    Die Basisimplementierung returned sal_True, wenn FORMAT_FILE im DragServer
    vorliegt.
*/

{
    return QueryDrop_Impl( SID_OPENDOC, rEvt );
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::IsInException() const
{
    return pAppData_Impl->bInException;
}

//--------------------------------------------------------------------

sal_uInt16 SfxApplication::Exception( sal_uInt16 nError )
{
    if ( pAppData_Impl->bInException )
        Application::Abort( pImp->aDoubleExceptionString );

    pAppData_Impl->bInException = sal_True;

    if( SfxNewHdl::Get() )
    {
        SfxNewHdl::Get()->FlushWarnMem();
        SfxNewHdl::Get()->FlushExceptMem();
    }

#ifndef TF_UCB
        // Flush all CHAOS data.
    CntSystem::Flush();
#endif

#if SUPD<613//MUSTINI
    INetURLObject aSaveObj( pAppIniMgr->Get( SFX_KEY_BACKUP_PATH ), INET_PROT_FILE );
    if ( Application::IsInExecute() )
    {
        SfxObjectShell *pIter, *pNext;
        sal_uInt16 n = 0;
        for(pIter = SfxObjectShell::GetFirst(); pIter; pIter = pNext)
        {
            pNext = SfxObjectShell::GetNext(*pIter);
            if( pIter->IsModified() && pIter->GetName().CompareToAscii("BasicIDE") != COMPARE_EQUAL && !pIter->IsLoading() )
            {
                //try
                {
                    // backup unsaved document
                    SFX_ITEMSET_ARG( pIter->GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, sal_False );
                    SfxRequest aReq(SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, pIter->GetPool());

                    sal_Bool bHadName = pIter->HasName();
                    INetURLObject aOldURL = pIter->GetMedium()->GetURLObject();
                    String aOldName = pIter->GetTitle();

                    const SfxFilter *pFilter = pIter->GetMedium()->GetFilter();
                    const SfxFilter *pOrigFilter = pFilter;
                    if ( !pFilter || ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) || !( pFilter->GetFilterFlags() & SFX_FILTER_EXPORT ) )
                        // packed files must be saved with default format, but remember original filter !
                        pFilter = pIter->GetFactory().GetFilter(0);

                    String aSaveName, aSavePath = aSaveObj.GetMainURL();
                    String aFilterName;
                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetName();
                        TempFile aTempFile( &aSavePath );
                        aSaveName = aTempFile.GetName();
                    }
                    else
                    {
                        String aExt( DEFINE_CONST_UNICODE( ".sav" ) );
                        TempFile aTempFile( DEFINE_CONST_UNICODE( "exc" ), &aExt, &aSavePath );
                        aSaveName = aTempFile.GetName();
                    }

                    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aSaveName ) );
                    aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
                    if ( pPassItem )
                        aReq.AppendItem( *pPassItem );

                    pIter->ExecuteSlot(aReq);

                    String aEntry( aSaveName );
                    aEntry += DEFINE_CONST_UNICODE(";");
                    aEntry += pOrigFilter ? pOrigFilter->GetName() : aFilterName;
                    aEntry += DEFINE_CONST_UNICODE(";");

                    if ( bHadName && INET_PROT_FILE == aOldURL.GetProtocol() )
                    {
                        aEntry += DEFINE_CONST_UNICODE("url;"),
                        aEntry += aOldURL.GetMainURL();
                    }
                    else
                    {
                        aEntry += DEFINE_CONST_UNICODE("title;"),
                        aEntry += aOldName;
                    }
                    pAppIniMgr->Set( aEntry, SFX_GROUP_WORKINGSET_IMPL, DEFINE_CONST_UNICODE("Recover"), n++ );
                }
                /*catch ( ::Exception & )
                {
                }*/
            }
        }
        pAppIniMgr->Flush();

        if ( ( nError & EXC_MAJORTYPE ) != EXC_DISPLAY && ( nError & EXC_MAJORTYPE ) != EXC_REMOTE )
        {
            Window *pTopWindow = GetTopWindow(); // GCC needs temporary
            WarningBox( pTopWindow, SfxResId(STR_RECOVER_PREPARED) ).Execute();
        }
    }
    else
        pAppIniMgr->Flush();
#else//MUSTINI
    INetURLObject aSaveObj( SvtPathOptions().GetBackupPath(), INET_PROT_FILE );
    // save all modified documents and close all documents
    // Do it only, if it's allowed! Ask configuration for right flag.
    if(
        ( Application::IsInExecute()                                    ) &&
        ( pAppData_Impl->pSaveOptions->IsSaveWorkingSet() == sal_True   )
    )
    {
        SfxObjectShell *pIter, *pNext;
        sal_uInt16 n = 0;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > seqWindowList;
        for(pIter = SfxObjectShell::GetFirst(); pIter; pIter = pNext)
        {
            pNext = SfxObjectShell::GetNext(*pIter);
            if( pIter->IsModified() && pIter->GetName().CompareToAscii("BasicIDE") != COMPARE_EQUAL && !pIter->IsLoading() )
            {
                //try
                {
                    // backup unsaved document
                    SFX_ITEMSET_ARG( pIter->GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, sal_False );
                    SfxRequest aReq(SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, pIter->GetPool());

                    sal_Bool bHadName = pIter->HasName();
                    INetURLObject aOldURL = pIter->GetMedium()->GetURLObject();
                    String aOldName = pIter->GetTitle();

                    const SfxFilter *pFilter = pIter->GetMedium()->GetFilter();
                    const SfxFilter *pOrigFilter = pFilter;
                    if ( !pFilter || ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) || !( pFilter->GetFilterFlags() & SFX_FILTER_EXPORT ) )
                        // packed files must be saved with default format, but remember original filter !
                        pFilter = pIter->GetFactory().GetFilter(0);

                    String aSaveName, aSavePath = aSaveObj.GetMainURL();
                    String aFilterName;
                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetName();
                        TempFile aTempFile( &aSavePath );
                        aSaveName = aTempFile.GetName();
                    }
                    else
                    {
                        String aExt( DEFINE_CONST_UNICODE( ".sav" ) );
                        TempFile aTempFile( DEFINE_CONST_UNICODE( "exc" ), &aExt, &aSavePath );
                        aSaveName = aTempFile.GetName();
                    }

                    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aSaveName ) );
                    aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
                    if ( pPassItem )
                        aReq.AppendItem( *pPassItem );

                    pIter->ExecuteSlot(aReq);

                    String aEntry( n );
                    aEntry += aSaveName;
                    aEntry += DEFINE_CONST_UNICODE(";");
                    aEntry += pOrigFilter ? pOrigFilter->GetName() : aFilterName;
                    aEntry += DEFINE_CONST_UNICODE(";");

                    if ( bHadName && INET_PROT_FILE == aOldURL.GetProtocol() )
                    {
                        aEntry += DEFINE_CONST_UNICODE("url;"),
                        aEntry += aOldURL.GetMainURL();
                    }
                    else
                    {
                        aEntry += DEFINE_CONST_UNICODE("title;"),
                        aEntry += aOldName;
                    }

                    seqWindowList.realloc(n+1);
                    seqWindowList[n] = aEntry;
                    ++n;
                }
                /*catch ( ::Exception & )
                {
                }*/
            }
        }

        SvtWorkingSetOptions().SetWindowList( seqWindowList );

        if ( ( nError & EXC_MAJORTYPE ) != EXC_DISPLAY && ( nError & EXC_MAJORTYPE ) != EXC_REMOTE )
        {
            Window *pTopWindow = GetTopWindow(); // GCC needs temporary
            WarningBox( pTopWindow, SfxResId(STR_RECOVER_PREPARED) ).Execute();
        }
    }
#endif//MUSTINI

#if SUPD<613//MUSTINI
/*TODO: We need a new key to save informations for SenCrashMail feature.*/
    sal_Bool bSendMail = (sal_uInt16) pAppIniMgr->ReadKey( DEFINE_CONST_UNICODE("Common"), DEFINE_CONST_UNICODE("SendCrashMail") ).ToInt32();
    if ( !pAppData_Impl->bBean && bSendMail )
    {
        String aInfo = System::GetSummarySystemInfos();
        if ( aInfo.Len() )
        {
            TempFile aTempFile( aSaveObj.GetMainURL() );
            String aFileName = aTempFile.GetName();
            SvFileStream aStr( aFileName, STREAM_STD_READWRITE );
            aStr.WriteByteString(aInfo);
            aStr << "\n<Build>\n";
            aStr << BUILD;
            aStr << '\n';
            aStr << "</Build>\n";
            aStr << "\n<Plattform>\n";
#ifdef WNT
            ByteString aPlattform( "wntmsci3" );
#elif defined ( C50 )
#   if defined ( SPARC )
            ByteString aPlattform( "unxsols2" );
#   elif defined ( INTEL )
            ByteString aPlattform( "unxsoli2" );
#   endif
#elif defined ( C52 )
#   if defined ( SPARC )
            ByteString aPlattform( "unxsols3" );
#   elif defined ( INTEL )
            ByteString aPlattform( "unxsoli3" );
#   endif
#elif GLIBC == 2
            ByteString aPlattform( "unxlngi2" );
#elif defined ( SPARC ) && defined ( GCC )
            ByteString aPlattform( "unxsogs" );
#endif
#ifndef DBG_UTIL
            aPlattform += ".pro";
#endif
            aStr << aPlattform.GetBuffer();
            aStr << '\n';
            aStr << "</Plattform>\n";
            aStr << "\n<OfficeLanguage>\n";
            aStr.WriteByteString( ByteString(Application::GetAppInternational().GetLanguage()) );
            aStr << '\n';
            aStr << "</OfficeLanguage>\n";
            aStr << "\n<ExceptionType>\n";
            aStr << nError;
            aStr << '\n';
            aStr << "</ExceptionType>\n";
            aStr.Close();

            pAppIniMgr->WriteKey( pAppIniMgr->GetGroupName( SFX_GROUP_WORKINGSET_IMPL ),
                                  DEFINE_CONST_UNICODE("Info"), aFileName );
            pAppIniMgr->Flush();
        }
    }
#endif//MUSTINI
    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_USER:
            if( nError == EXC_OUTOFMEMORY )
                Application::Abort( pImp->aMemExceptionString );
            break;

        case EXC_RSCNOTLOADED:
            Application::Abort( pImp->aResExceptionString );
            break;

        case EXC_SYSOBJNOTCREATED:
            Application::Abort( pImp->aSysResExceptionString );
            break;
    }

    pAppData_Impl->bInException = sal_False;
    return 0;
}

//---------------------------------------------------------------------

ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
{
    String aMgrName = String::CreateFromAscii( pPrefix );
    aMgrName += String::CreateFromInt32(SOLARUPD); // aktuelle Versionsnummer
    return ResMgr::CreateResMgr(U2S(aMgrName));
}

//---------------------------------------------------------------------

SimpleResMgr* SfxApplication::CreateSimpleResManager()
{
    SimpleResMgr    *pRet;
    ::rtl::OUString sAppName;

    if ( ::vos::OStartupInfo().getExecutableFile(sAppName) != ::vos::OStartupInfo::E_None )
    {
        sAppName = ::rtl::OUString();
    }

    LanguageType nType = Application::GetAppInternational().GetLanguage();
    if ( nType == LANGUAGE_SYSTEM )
        nType = System::GetLanguage();

    String sTemp( sAppName );
    pRet = new SimpleResMgr( CREATEVERSIONRESMGR_NAME(sfx),
                             nType, &sTemp, 0 );

    return pRet;
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
    if ( !pImp->pSfxResManager )
    {
        pImp->pSfxResManager = CreateResManager("sfx");
#if 0                                   // SFX on demand
        if ( !Resource::GetResManager() )
            Resource::SetResManager( pImp->pSfxResManager );
#endif
    }

    return pImp->pSfxResManager;
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetLabelResManager() const
{
    return pAppData_Impl->pLabelResMgr;
}

//--------------------------------------------------------------------

SimpleResMgr* SfxApplication::GetSimpleResManager()
{
    if ( !pImp->pSimpleResManager )
    {
        pImp->pSimpleResManager = CreateSimpleResManager();
    }
    return pImp->pSimpleResManager;
}

//------------------------------------------------------------------------

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress  /*  zu startender <SfxProgress> oder 0, falls
                                der Progress zurueckgesetzt werden soll */
)

/*  [Beschreibung]

    Interne Methode zum setzen oder zuruecksetzen des Progress-Modes
    fuer die gesamte Applikation.
*/

{
    DBG_ASSERT( ( !pAppData_Impl->pProgress && pProgress ) ||
                ( pAppData_Impl->pProgress && !pProgress ),
                "Progress acitivation/deacitivation mismatch" );

    if ( pAppData_Impl->pProgress && pProgress )
    {
        pAppData_Impl->pProgress->Suspend();
        pAppData_Impl->pProgress->UnLock();
        delete pAppData_Impl->pProgress;
    }

    pAppData_Impl->pProgress = pProgress;
}

//------------------------------------------------------------------------

sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pAppData_Impl->aIndexBitSet.GetFreeIndex()+1;
}

//------------------------------------------------------------------------

void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
}

//--------------------------------------------------------------------

void SfxApplication::EnterAsynchronCall_Impl()
{
    ++pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

void SfxApplication::LeaveAsynchronCall_Impl()
{
    --pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

FASTBOOL SfxApplication::IsInAsynchronCall_Impl() const
{
    return pAppData_Impl->nAsynchronCalls > 0;
}

//--------------------------------------------------------------------

Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    return pWork ? pWork->GetWindow() : NULL;
}

//--------------------------------------------------------------------

void SfxApplication::SetTopWindow( WorkWindow *pWindow )
{
/*
    sal_Bool bMode = sal_True;
    if ( !pWindow || pWindow == GetAppWindow() )
        bMode = sal_False;

    SfxPlugInFrame* pPlug = PTR_CAST( SfxPlugInFrame,
        GetViewFrame()->GetTopViewFrame() );
    if ( pPlug )
        pPlug->GetEnv_Impl()->SetPresentationMode( bMode, pWindow );
    else
        GetAppWindow()->SetPresentationMode( bMode, pWindow );
*/
}

sal_Bool SfxApplication::IsPlugin()
{
/*  Reference < XPluginInstance > xPlugin ( pImp->xFrame, UNO_QUERY );
    return xPlugin.is();*/

    // Set default return value if method failed.
    sal_Bool bReturn = sal_False;
    // Get Desktop to get a list of all current tasks on it.
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTasksSupplier > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( !(xDesktop.is()==sal_False), "SfxFrame::IsPlugin_Impl()Can't get reference to desktop service!\n" );
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > xList = xDesktop->getTasks()->createEnumeration();
    while( xList->hasMoreElements() == sal_True )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTask > xTask;
        xList->nextElement() >>= xTask;
        ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XPluginInstance > xPlugIn( xTask, ::com::sun::star::uno::UNO_QUERY );
        if( xPlugIn.is() == sal_True )
        {
            bReturn = sal_True;
            break;
        }
    }
    return bReturn;
}

//--------------------------------------------------------------------

const Accelerator& SfxApplication::GetAccelerator_Impl() const
{
    return *GetAcceleratorManager()->GetAccel();
}

//--------------------------------------------------------------------

SvVerbList* SfxApplication::GetVerbList_Impl() const
{
    if ( !pAppData_Impl->pVerbs )
    {
        // globale Verb-List anlegen
        pAppData_Impl->pVerbs = new SvVerbList;
        pAppData_Impl->pVerbs->Append( SvVerb( 0, String( SfxResId( STR_EDITOBJECT ) ) ) );
        pAppData_Impl->pVerbs->Append( SvVerb( 1, String( SfxResId( STR_OPENOBJECT ) ) ) );
        pAppData_Impl->pVerbs->Append( SvVerb( 2, DEFINE_CONST_UNICODE(STARAPP_VERB), sal_True, sal_False ) );
    }

    return pAppData_Impl->pVerbs;
}

//--------------------------------------------------------------------

/*  [Beschreibung]

*/

String SfxApplication::LocalizeDBName
(
    SfxDBNameConvert eConvert,
    const String& rList,
    char aDel
) const
{
/*    String  aActName;
    String  aResult;
    String  aNationalName = SfxResId(STR_ADDRESS_NAME);
    String  aIniName( "Address" );
    sal_uInt16  nCnt = rList.GetTokenCount( aDel );

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
    {
        aActName = rList.GetToken( i, aDel );

        if( eConvert == INI2NATIONAL )
        {
            if( aActName == aIniName )
                aResult += aNationalName;
            else
                aResult += aActName;
        }
        else
        {
            if( aActName == aNationalName )
                aResult += aIniName;
            else
                aResult += aActName;
        }

        aResult += aDel;
    }

    aResult.EraseTrailingChars( aDel );

    return aResult;*/

    return rList;
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK( SfxApplication, CookieAlertHdl_Impl, void*, EMPTYARG )
{
    return 0;
}

//--------------------------------------------------------------------

void SfxApplication::SetUserEMailAddress( const String &rEMail )
{
    DBG_ERROR( "Obsolete call!" );
}

//-------------------------------------------------------------------------

void SfxApplication::SetDefFocusWindow( Window *pWin )

/*  [Beschreibung]

    Mit dieser Methode wird das Window gesetzt, auf das beim n"achsten
    <SfxApplication::GrabFocus()> der Focus gesetzt werden soll.

    Ein 'SetDefFocusWindow()' wirkt f"ur genau einen einzigen Aufruf von
    'SfxApplication::GrabFocus()'.

    Damit kann z.B. das in verschiedenen Situationen von Windows kommende
    Focus-Setzen auf MDIWindows verhindert werden.
*/

{
    pAppData_Impl->pDefFocusWin = pWin;
}

//-------------------------------------------------------------------------

void SfxApplication::GrabFocus( Window *pAlternate )

/*  [Beschreibung]

    Mit dieser Methode wird der Focus auf das zuvor mit der Methode
    <SfxApplicaton::SetDefFocusWindow()> gesetzte Window gegrabt. Ist
    keins mehr gesetzt oder wurde es bereits verwendet, wird der Focus
    auf 'pAlternate' gesetzt. Ein 'SetDefFocusWindow()' wirkt f"ur genau
    ein einziges 'SfxApplication::GrabFocus()'.
*/

{
    Window *pWin = pAppData_Impl->pDefFocusWin
                        ? pAppData_Impl->pDefFocusWin
                        : pAlternate;
    pWin->GrabFocus();
    pAppData_Impl->pDefFocusWin = 0;
}

#if 0
SfxFrame* SfxApplication::GetTargetFrame( const SfxItemSet* pSet, sal_Bool& rbOwner  )
{
    return GetTargetFrame_Impl( pSet, rbOwner  );
}
#endif

SfxStatusBarManager* SfxApplication::GetStatusBarManager() const
{
    if ( !pViewFrame )
        return NULL;

    SfxViewFrame *pTop = pViewFrame;
    while ( pTop->GetParentViewFrame_Impl() )
        pTop = pTop->GetParentViewFrame_Impl();

    return pTop->GetFrame()->GetWorkWindow_Impl()->GetStatusBarManager_Impl();
}

SfxViewFrame* SfxApplication::GetViewFrame()
{
    return pViewFrame;
}

UniqueIndex* SfxApplication::GetEventHandler_Impl()
{
    return pImp->pEventHdl;
}

SfxTbxCtrlFactArr_Impl&     SfxApplication::GetTbxCtrlFactories_Impl() const
{
    return *pImp->pTbxCtrlFac;
}

SfxStbCtrlFactArr_Impl&     SfxApplication::GetStbCtrlFactories_Impl() const
{
    return *pImp->pStbCtrlFac;
}

SfxMenuCtrlFactArr_Impl&    SfxApplication::GetMenuCtrlFactories_Impl() const
{
    return *pImp->pMenuCtrlFac;
}

SfxViewFrameArr_Impl&       SfxApplication::GetViewFrames_Impl() const
{
    return *pImp->pViewFrames;
}

SfxViewShellArr_Impl&       SfxApplication::GetViewShells_Impl() const
{
    return *pImp->pViewShells;
}

SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
{
    return *pImp->pObjShells;
}

#if SUPD>605
void SfxApplication::Invalidate( USHORT nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}
#endif
