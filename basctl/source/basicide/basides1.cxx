/*************************************************************************
 *
 *  $RCSfile: basides1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:36 $
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


#define GLOBALOVERFLOW2

#pragma hdrstop


#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#define _SVSTDARR_STRINGS
#include <svtools/svstdarr.hxx>
#include <ide_pch.hxx>
#pragma hdrstop

#define __PRIVATE 1

// #define _SVX_NOIDERESIDS

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS
#define SI_NOITEMS
#define SI_NODRW
#define _VCTRLS_HXX

#include <basidesh.hrc>
#include <basidesh.hxx>
#include <baside2.hxx>
#include <baside3.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <vcsbx.hxx>
#include <sbxitem.hxx>
#include <vcl/rcid.h>
#include <helpid.hrc>

#ifndef _SFX_MINFITEM_HXX //autogen
#include <sfx2/minfitem.hxx>
#endif


class SvxSearchItem;

//  Egal was, einfach ans aktuelle Fenster:
void __EXPORT BasicIDEShell::ExecuteCurrent( SfxRequest& rReq )
{
    if ( !pCurWin )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_BASICIDE_HIDECURPAGE:
        {
            pCurWin->StoreData();
            RemoveWindow( pCurWin, FALSE );
        }
        break;
        case SID_BASICIDE_DELETECURRENT:
        {
            StarBASIC* pBasic = pCurWin->GetBasic();
            DBG_ASSERT( pBasic, "Aktuellen Fenster ohne Basic ?!" );
            if ( pCurWin->ISA( ModulWindow ) )
            {
                SbModule* pModule = ((ModulWindow*)pCurWin)->GetModule();
                DBG_ASSERT( pModule, "DELETECURRENT: Modul nicht gefunden!" );
                if ( QueryDelModule( pModule->GetName(), pCurWin ) )
                {
                    pBasic->GetModules()->Remove( pModule );
                    RemoveWindow( pCurWin, TRUE );
                    BasicIDE::MarkDocShellModified( pBasic );

                }
            }
            else
            {
                SbxObject* pDlg = ((DialogWindow*)pCurWin)->GetDialog();
                DBG_ASSERT( pDlg, "DELETECURRENT: Dialog nicht gefunden!" );
                if ( QueryDelDialog( pDlg->GetName(), pCurWin ) )
                {
                    pBasic->GetObjects()->Remove( pDlg );
                    RemoveWindow( pCurWin, TRUE );
                    BasicIDE::MarkDocShellModified( pBasic );
                }
            }
        }
        break;
        case SID_BASICIDE_RENAMECURRENT:
        {
            pTabBar->StartEditMode( pTabBar->GetCurPageId() );
        }
        break;
        case FID_SEARCH_NOW:
        {
            if ( pCurWin->ISA( ModulWindow ) )
            {
                DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
                const SfxItemSet* pArgs = rReq.GetArgs();
                // Leider kenne ich die ID nicht:
                USHORT nWhich = pArgs->GetWhichByPos( 0 );
                DBG_ASSERT( nWhich, "Wich fuer SearchItem ?" );
                const SfxPoolItem& rItem = pArgs->Get( nWhich );
                DBG_ASSERT( rItem.ISA( SvxSearchItem ), "Kein Searchitem!" );
                if ( rItem.ISA( SvxSearchItem ) )
                {
                    // Item wegen der Einstellungen merken...
                    IDE_DLL()->GetExtraData()->SetSearchItem( (const SvxSearchItem&)rItem );
                    USHORT nFound = 0;
                    BOOL bCanceled = FALSE;
                    if ( ((const SvxSearchItem&)rItem).GetCommand() == SVX_SEARCHCMD_REPLACE_ALL )
                    {
                        USHORT nActModWindows = 0;
                        IDEBaseWindow* pWin = aIDEWindowTable.First();
                        while ( pWin )
                        {
                            if ( !pWin->IsSuspended() && pWin->IsA( TYPE( ModulWindow ) ) )
                                nActModWindows++;
                            pWin = aIDEWindowTable.Next();
                        }

                        if ( ( nActModWindows <= 1 ) || ( !((const SvxSearchItem&)rItem).GetSelection() && QueryBox( pCurWin, WB_YES_NO|WB_DEF_YES, String( IDEResId( RID_STR_SEARCHALLMODULES ) ) ).Execute() == RET_YES ) )
                        {
                            pWin = aIDEWindowTable.First();
                            while ( pWin )
                            {
                                if ( !pWin->IsSuspended() && pWin->IsA( TYPE( ModulWindow ) ) )
                                    nFound += ((ModulWindow*)pWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem );
                                pWin = aIDEWindowTable.Next();
                            }
                        }
                        else
                            nFound = ((ModulWindow*)pCurWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem );

                        IDEResId nId( RID_STR_SEARCHREPLACES );
                        String aReplStr( nId );
                        aReplStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), nFound );
                        InfoBox( pCurWin, aReplStr ).Execute();
                    }
                    else
                    {
                        nFound = ((ModulWindow*)pCurWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem );
                        if ( !nFound && !((const SvxSearchItem&)rItem).GetSelection() )
                        {
                            // Andere Module durchsuchen...
                            BOOL bChangeCurWindow = FALSE;
                            aIDEWindowTable.Seek( pCurWin );
                            // Erstmal beim naechsten Anfangen, ggf. spaeter von vorne
                            IDEBaseWindow* pWin = aIDEWindowTable.Next();
                            BOOL bSearchedFromStart = FALSE;
                            while ( !nFound && !bCanceled && ( pWin || !bSearchedFromStart ) )
                            {
                                if ( !pWin )
                                {
                                    QueryBox aQuery( pCurWin, WB_YES_NO|WB_DEF_YES, String( IDEResId( RID_STR_SEARCHFROMSTART ) ) );
                                    if ( aQuery.Execute() == RET_YES )
                                    {
                                        pWin = aIDEWindowTable.First();
                                        bSearchedFromStart = TRUE;
                                    }
                                    else
                                        bCanceled = TRUE;
                                }

                                if ( pWin && !pWin->IsSuspended() && pWin->IsA( TYPE( ModulWindow ) ) )
                                {
                                    if ( pWin != pCurWin )
                                    {
                                        // Groesse einstellen, damit die View
                                        // gleich richtig justiert werden kann.
                                        if ( pCurWin )
                                            pWin->SetSizePixel( pCurWin->GetSizePixel() );
                                        nFound = ((ModulWindow*)pWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem, TRUE );
                                    }
                                    if ( nFound )
                                    {
                                        bChangeCurWindow = TRUE;
                                        break;
                                    }
                                }
                                if ( pWin && ( pWin != pCurWin ) )
                                    pWin = aIDEWindowTable.Next();
                                else
                                    pWin = 0;   // Dann sind wir durch...
                            }
                            if ( !nFound && bSearchedFromStart )    // Aktuelles von vorne...
                                nFound = ((ModulWindow*)pCurWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem, TRUE );
                            if ( bChangeCurWindow )
                                SetCurWindow( pWin, TRUE );
                        }
                        if ( !nFound && !bCanceled )
                            InfoBox( pCurWin, String( IDEResId( RID_STR_SEARCHNOTFOUND ) ) ).Execute();
                    }

                    rReq.Done();
                }
            }
        }
        break;
        case SID_UNDO:
        case SID_REDO:
        {
            if ( GetUndoManager() && pCurWin->AllowUndo() )
            {
                GetViewFrame()->ExecuteSlot( rReq );
            }
        }
        break;
        default:
        {
            pCurWin->ExecuteCommand( rReq );
        }
    }
}

//  Egal, wer oben, Einfluss auf die Shell:
void __EXPORT BasicIDEShell::ExecuteGlobal( SfxRequest& rReq )
{
    USHORT nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_BASICSTOP:
        {
            // Evtl. nicht einfach anhalten, falls auf Brechpunkt!
            if ( pCurWin && pCurWin->IsA( TYPE( ModulWindow ) ) )
                ((ModulWindow*)pCurWin)->BasicStop();
            BasicIDE::StopBasic();
        }
        break;

        case SID_SAVEDOC:
        {
            if ( pCurWin )
            {
                // Daten ins BASIC zurueckschreiben
                StoreAllWindowData();

                // Doc-Basic?
                SfxApplication* pSfxApp = SFX_APP();
                BasicManager* pAppBasMgr = pSfxApp->GetBasicManager();
                BasicManager *pBasMgr = BasicIDE::FindBasicManager( pCurWin->GetBasic() );
                if ( pBasMgr != pAppBasMgr )
                {
                    // Doc-Basic suchen
                    BOOL bFound = FALSE;
                    for ( SfxObjectShell *pDocSh = SfxObjectShell::GetFirst();
                          !bFound && pDocSh;
                          pDocSh = SfxObjectShell::GetNext(*pDocSh) )
                    {
                        if ( pDocSh->GetBasicManager() == pBasMgr )
                        {
                            // auch das Doc speichern
                            pDocSh->ExecuteSlot( rReq );
                            bFound = TRUE;
                        }
                    }
                    DBG_ASSERT( bFound, "unknown basic" );
                }

                SfxBindings &rBindings = BasicIDE::GetBindings();
                rBindings.Invalidate( SID_DOC_MODIFIED );
                rBindings.Invalidate( SID_SAVEDOC );
            }
        }
        break;
        case SID_BASICIDE_MODULEDLG:
        {
            BasicIDE::Organize();
        }
        break;
        case SID_BASICIDE_CHOOSEMACRO:
        {
            BasicIDE::ChooseMacro();
        }
        break;
        case SID_BASICIDE_CREATEMACRO:
        case SID_BASICIDE_EDITMACRO:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxMacroInfoItem& rInfo = (const SfxMacroInfoItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO );
            BasicManager* pBasMgr = (BasicManager*)rInfo.GetBasicManager();
            DBG_ASSERT( pBasMgr, "Nichts selektiert im Basic-Baum ?" );
            StartListening( *pBasMgr, TRUE /* Nur einmal anmelden */ );
            StarBASIC* pBasic = pBasMgr->GetLib( rInfo.GetLib() );
            if ( !pBasic )
            {
                // LoadOnDemand
                USHORT nLib = pBasMgr->GetLibId( rInfo.GetLib() );
                pBasMgr->LoadLib( nLib );
                pBasic = pBasMgr->GetLib( nLib );
            }
            if ( !pBasic )
                pBasic = pBasMgr->GetLib( 0 );
            DBG_ASSERT( pBasic, "Kein Basic!" );
            // Evtl pCurBasic umschalten:
            if ( pCurBasic && ( pBasic != pCurBasic ) )
                SetCurBasic( pBasic );
            if ( rReq.GetSlot() == SID_BASICIDE_CREATEMACRO )
            {
                SbModule* pModule = pBasic->FindModule( rInfo.GetModule() );
                if ( !pModule )
                {
                    if ( rInfo.GetModule().Len() || !pBasic->GetModules()->Count() )
                        pModule = BasicIDE::CreateModule( pBasic, rInfo.GetModule() );
                    else
                        pModule = (SbModule*) pBasic->GetModules()->Get(0);
                }
                DBG_ASSERT( pModule, "Kein Modul!" );
                if ( !pModule->GetMethods()->Find( rInfo.GetMethod(), SbxCLASS_METHOD ) )
                    BasicIDE::CreateMacro( pModule, rInfo.GetMethod() );
            }
            BasicIDE::Appear();
            ModulWindow* pWin = FindBasWin( pBasic, rInfo.GetModule(), TRUE );
            DBG_ASSERT( pWin, "Edit/Create Macro: Fenster wurde nicht erzeugt/gefunden!" );
            SetCurWindow( pWin, TRUE );
            pWin->EditMacro( rInfo.GetMethod() );
        }
        break;
        case SID_BASICIDE_OBJCAT:
        {
            ShowObjectDialog( TRUE, TRUE );
        }
        break;
        case SID_BASICIDE_LIBSELECTOR:
        {
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            // ! Pruefen, ob diese Routine ueberhaupt gebraucht wird !
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // String fuer aktuelle einstellung:
            String aQName;
            if ( pCurBasic )
            {
                BasicManager* pBasMgr = BasicIDE::FindBasicManager( pCurBasic );
                if ( pBasMgr )
                    aQName = CreateMgrAndLibStr( BasicIDE::FindTitle( pBasMgr ), pCurBasic->GetName() );
            }

            PopupMenu aPopup;
            SvStrings* pStrings = BasicIDE::CreateBasicLibBoxEntries();
            for ( USHORT n = 0; n < pStrings->Count(); n++ )
            {
                String* pStr = pStrings->GetObject( n );
                aPopup.InsertItem( n+1, *pStr );    // +1, damit keine 0
                if ( *pStr == aQName )
                    aPopup.CheckItem( n+1, TRUE );
            }
            if ( !pCurBasic )
                aPopup.CheckItem( 1, TRUE );

            // (pb -> mt) we need a window here
            Window& rWin = GetViewFrame()->GetWindow();
            USHORT nId = aPopup.Execute( &rWin, rWin.GetPointerPosPixel() );
            if ( nId )
            {
                USHORT i = nId-1;
                String aLib;
                if ( i )    // sonst 'Alle', also leer
                    aLib = *pStrings->GetObject( i );
                SfxStringItem aLibName( SID_BASICIDE_ARG_LIBNAME, aLib );
                SfxViewFrame* pCurFrame = SfxViewFrame::Current();
                DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
                SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_LIBSELECTED,
                                          SFX_CALLMODE_SYNCHRON, &aLibName, 0L );
                }
            }
            pStrings->DeleteAndDestroy( 0, pStrings->Count() );
            delete pStrings;
        }
        break;
        case SID_BASICIDE_NAMECHANGEDONTAB:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxUInt16Item &rTabId = (const SfxUInt16Item&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID );
            const SfxStringItem &rModName = (const SfxStringItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MODULENAME );
            IDEBaseWindow* pWin = aIDEWindowTable.Get( rTabId.GetValue() );
            DBG_ASSERT( pWin, "Window nicht im Liste, aber in TabBar ?" );

            // Focus wieder auf "Arbeitsflaeche"
            pWin->GrabFocus();

            if ( pWin->IsA( TYPE( ModulWindow ) ) )
            {
                ModulWindow* pEditWin = (ModulWindow*)pWin;
                pEditWin->RenameModule( rModName.GetValue() );
            }
            else
            {
                DialogWindow* pViewWin = (DialogWindow*)pWin;
                pViewWin->RenameDialog( rModName.GetValue() );
            }
            BasicIDE::MarkDocShellModified( pWin->GetBasic() );
        }
        break;
        case SID_BASICIDE_STOREMODULESOURCE:
        case SID_BASICIDE_UPDATEMODULESOURCE:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxMacroInfoItem& rInfo = (const SfxMacroInfoItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO );
            BasicManager* pBasMgr = (BasicManager*)rInfo.GetBasicManager();
            DBG_ASSERT( pBasMgr, "Store source: Kein BasMgr?" );
            StarBASIC* pBasic = pBasMgr->GetLib( rInfo.GetLib() );
            DBG_ASSERT( pBasic, "Lib im BasicManager nicht gefunden!" );
            ModulWindow* pWin = FindBasWin( pBasic, rInfo.GetModule(), FALSE, TRUE );
            if ( pWin )
            {
                if ( rReq.GetSlot() == SID_BASICIDE_STOREMODULESOURCE )
                    pWin->StoreData();
                else
                    pWin->UpdateData();
            }
        }
        break;
        case SID_BASICIDE_STOREALLMODULESOURCES:
        case SID_BASICIDE_UPDATEALLMODULESOURCES:
        {
            IDEBaseWindow* pWin = aIDEWindowTable.First();
            while ( pWin )
            {
                if ( !pWin->IsSuspended() && pWin->IsA( TYPE( ModulWindow ) ) )
                {
                    if ( rReq.GetSlot() == SID_BASICIDE_STOREALLMODULESOURCES )
                        pWin->StoreData();
                    else
                        pWin->UpdateData();
                }
                pWin = aIDEWindowTable.Next();
            }
        }
        break;
        case SID_BASICIDE_LIBSELECTED:
        case SID_BASICIDE_LIBREMOVED:
        case SID_BASICIDE_LIBLOADED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxStringItem &rLibName = (const SfxStringItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_LIBNAME );
            String aBasMgrAndLib( rLibName.GetValue() );

            StarBASIC* pLib = 0;
            BasicManager* pMgr = 0;
            if ( aBasMgrAndLib.Len() )
            {
                String aBasMgr( GetMgrFromMgrAndLib( aBasMgrAndLib ) );
                String aLib( GetLibFromMgrAndLib( aBasMgrAndLib ) );

                pMgr = BasicIDE::FindBasicManager( aBasMgr );
                if ( !pMgr )    // Doc schon weg...
                    return;
                pLib = pMgr->GetLib( aLib );
                if ( !pLib && ( nSlot == SID_BASICIDE_LIBSELECTED ) )
                {
                    // LoadOnDemand
                    USHORT nLib = pMgr->GetLibId( aLib );
                    pMgr->LoadLib( nLib );
                    pLib = pMgr->GetLib( nLib );
                    if ( !pLib )
                    {
                        ErrorBox( pCurWin, WB_OK|WB_DEF_OK, String( IDEResId( RID_STR_ERROROPENLIB ) ) ).Execute();
                        // Hat leider keine Auswirkung, weil alter Wert => Kein StateChanged...
                        BasicIDE::GetBindings().Invalidate( SID_BASICIDE_LIBSELECTOR );
                        return;
                    }
                }
            }
            if ( nSlot == SID_BASICIDE_LIBSELECTED )
            {
                // Keine Abfrage, ob pCurBasic == pLib,
                // falls welche ausgeblendet waren.
                BOOL bSet = TRUE;
                if ( pLib )
                {
                    USHORT nLib = pMgr->GetLibId( pLib );
                    if ( pMgr->HasPassword( nLib ) &&
                            !pMgr->IsPasswordVerified( nLib ) )
                    {
                        bSet = QueryPassword( pMgr, nLib );
                    }
                }
                if ( bSet )
                    SetCurBasic( pLib );
                else    // alten Wert einstellen...
                    BasicIDE::GetBindings().Invalidate( SID_BASICIDE_LIBSELECTOR, TRUE, FALSE );
            }
            else if ( nSlot == SID_BASICIDE_LIBREMOVED )
            {
                if ( !pCurBasic || ( pLib == pCurBasic ) )
                {
                    RemoveWindows( pLib, TRUE );
                    if ( pCurBasic == pLib )
                    {
                        pCurBasic = 0;
                        // Kein UpdateWindows!
                        BasicIDE::GetBindings().Invalidate( SID_BASICIDE_LIBSELECTOR );
                    }
                }
            }
            else    // Loaded...
                UpdateWindows();
        }
        break;
        case SID_BASICIDE_NEWMODULE:
        {
            StarBASIC* pBasic = pCurBasic;
            if ( !pBasic )
                pBasic = SFX_APP()->GetBasicManager()->GetLib( 0 );
            ModulWindow* pWin = CreateBasWin( pBasic, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_NEWDIALOG:
        {
            StarBASIC* pBasic = pCurBasic;
            if ( !pBasic )
                pBasic = SFX_APP()->GetBasicManager()->GetLib( 0 );
            DialogWindow* pWin = CreateDlgWin( pBasic, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_SBXRENAMED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SbxObject* pSbxObject = (SbxObject*)rSbxItem.GetSbx();
            IDEBaseWindow* pWin = FindWindow( pSbxObject, FALSE );
            if ( pWin )
            {
                // TabWriter updaten
                USHORT nId = (USHORT)aIDEWindowTable.GetKey( pWin );
                DBG_ASSERT( nId, "Kein Eintrag in der Tabbar!" );
                if ( nId )
                    pTabBar->SetPageText( nId, pSbxObject->GetName() );
                if( pWin->IsA( TYPE( DialogWindow ) ) )
                    ((DialogWindow*)pWin)->UpdateBrowser();
            }
        }
        break;
        case SID_BASICIDE_SBXINSERTED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SbxObject* pSbxObject = (SbxObject*)rSbxItem.GetSbx();
            StarBASIC* pBasic = BasicIDE::FindBasic( pSbxObject );
            DBG_ASSERT( pBasic, "Basic fuer das Object nicht gefunden!" );
            if ( !pCurBasic || ( pBasic == pCurBasic ) )
            {
                IDEBaseWindow* pWin = 0;
                if ( pSbxObject->ISA( SbModule ) )
                    pWin = FindBasWin( pBasic, pSbxObject->GetName(), TRUE );
                else if ( pSbxObject->GetSbxId() == SBXID_DIALOG )
                    pWin = FindDlgWin( pBasic, pSbxObject->GetName(), TRUE );
            }
        }
        break;
        case SID_BASICIDE_SBXDELETED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SbxObject* pSbxObject = (SbxObject*)rSbxItem.GetSbx();
            IDEBaseWindow* pWin = FindWindow( pSbxObject, TRUE );
            if ( pWin )
                RemoveWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_SHOWSBX:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SbxVariable* pSbx = (SbxVariable*)rSbxItem.GetSbx();
            StarBASIC* pBasic = BasicIDE::FindBasic( pSbx );
            DBG_ASSERT( pBasic, "Basic nicht gefunden!" );
            if ( pCurBasic && ( pCurBasic != pBasic ) )
                SetCurBasic( pBasic );
            IDEBaseWindow* pWin = 0;
            if ( pSbx->GetSbxId() == SBXID_DIALOG )
                pWin = FindDlgWin( pBasic, pSbx->GetName(), TRUE );
            else if ( pSbx->ISA( SbModule ) )
                pWin = FindBasWin( pBasic, pSbx->GetName(), TRUE );
            else if ( pSbx->ISA( SbMethod ) )
            {
                pWin = FindBasWin( pBasic, pSbx->GetParent()->GetName(), TRUE );
                ((ModulWindow*)pWin)->EditMacro( pSbx->GetName() );
            }
            DBG_ASSERT( pWin, "Fenster wurde nicht erzeugt!" );
            SetCurWindow( pWin, TRUE );
        }
        break;
    }
}

void __EXPORT BasicIDEShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( USHORT nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
    {
        switch ( nWh )
        {
            case SID_DOCINFO:
            {
                rSet.DisableItem( nWh );
            }
            break;
            case SID_SAVEDOC:
            {
/*
                BasicManager* pAppBasMgr = SFX_APP()->GetBasicManager();
                // Wenn nicht modifiert, sowieso disablen...
                if ( pCurWin && ( pCurWin->IsModified() ) &&
                    ( BasicIDE::FindBasicManager( pCurWin->GetBasic() ) == pAppBasMgr ) )
                {
                    break;  // Also enabled
                }
                if ( !pAppBasMgr->IsModified() )
                    rSet.DisableItem( nWh );
                // ansonsten disablen, wenn ein anderes Basic dargestellt wird.
                else if ( pCurBasic && ( BasicIDE::FindBasicManager( pCurBasic ) != pAppBasMgr ) )
                    rSet.DisableItem( nWh );
                else if ( !pCurBasic && pCurWin )
                {
                    StarBASIC* pBasic = pCurWin->GetBasic();
                    BasicManager* pMgr = BasicIDE::FindBasicManager( pBasic );
                    if ( pMgr != pAppBasMgr )
                        rSet.DisableItem( nWh );
                }
*/

                BasicManager* pMgr = pCurWin ? BasicIDE::FindBasicManager( pCurWin->GetBasic() ) : 0;
                if ( !pCurWin || !pMgr ||
                    ( !pCurWin->IsModified() && !pMgr->IsModified() ) )
                {
                    rSet.DisableItem( nWh );
                }
            }
            break;
            case SID_NEWWINDOW:
            case SID_SAVEASDOC:
            {
                rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICIDE_MODULEDLG:
            {
                if ( StarBASIC::IsRunning() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICIDE_CHOOSEMACRO:
            case SID_BASICIDE_OBJCAT:
            case SID_BASICIDE_SHOWSBX:
            case SID_BASICIDE_CREATEMACRO:
            case SID_BASICIDE_EDITMACRO:
            case SID_BASICIDE_NAMECHANGEDONTAB:
            {
                ;
            }
            break;

            case SID_BASICIDE_ADDWATCH:
            case SID_BASICIDE_REMOVEWATCH:
            case SID_BASICLOAD:
            case SID_BASICSAVEAS:
            case SID_BASICIDE_MATCHGROUP:
            {
                if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) )
                    rSet.DisableItem( nWh );
                else if ( ( nWh == SID_BASICLOAD ) && StarBASIC::IsRunning() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICRUN:
            case SID_BASICSTEPINTO:
            case SID_BASICSTEPOVER:
            case SID_BASICSTEPOUT:
            case SID_BASICIDE_TOGGLEBRKPNT:
            {
                if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) )
                    rSet.DisableItem( nWh );
                else if ( StarBASIC::IsRunning() && !((ModulWindow*)pCurWin)->GetBasicStatus().bIsInReschedule )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICCOMPILE:
            {
                if ( !pCurWin || !pCurWin->IsA( TYPE( ModulWindow ) ) || StarBASIC::IsRunning() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICSTOP:
            {
                // Stop immermoeglich, wenn irgendein Basic lauft...
                if ( !StarBASIC::IsRunning() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_CHOOSE_CONTROLS:
            case SID_DIALOG_TESTMODE:
            {
                if( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_SHOW_FORMS:
            case SID_SHOW_HIDDEN:
            {
                rSet.DisableItem( nWh );
            }
            break;
            case SID_SEARCH_OPTIONS:
            {
                USHORT nOptions = 0;
                if( pCurWin )
                    nOptions = pCurWin->GetSearchOptions();
                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS, nOptions ) );
            }
            break;
            case SID_BASICIDE_LIBSELECTOR:
            {
                String aQName;
                if ( pCurBasic )
                {
                    BasicManager* pBasMgr = BasicIDE::FindBasicManager( pCurBasic );
                    if ( pBasMgr ) // 0 bei CloseDoc!
                        aQName = CreateMgrAndLibStr( BasicIDE::FindTitle( pBasMgr ), pCurBasic->GetName() );
                }
                SfxStringItem aItem( SID_BASICIDE_LIBSELECTOR, aQName );
                rSet.Put( aItem );
            }
            break;
            case SID_SEARCH_ITEM:
            {
                String aSelected = GetSelectionText( TRUE );
                SvxSearchItem& rItem = IDE_DLL()->GetExtraData()->GetSearchItem();
                rItem.SetSearchString( aSelected );
                rSet.Put( rItem );
            }
            break;
            case SID_BASICIDE_STAT_DATE:
            {
                String aDate;
                aDate = String( RTL_CONSTASCII_USTRINGPARAM( "Datum?!" ) );
                SfxStringItem aItem( SID_BASICIDE_STAT_DATE, aDate );
                rSet.Put( aItem );
            }
            break;
            case SID_DOC_MODIFIED:
            {
                String aModifiedMarker;
                BasicManager* pBasMgr = pCurWin ? BasicIDE::FindBasicManager( pCurWin->GetBasic() ) : 0;
                if ( pCurWin && ( pCurWin->IsModified() || ( pBasMgr && pBasMgr->IsModified() ) ) )
                {
                    aModifiedMarker = '*';
                }
                SfxStringItem aItem( SID_DOC_MODIFIED, aModifiedMarker );
                rSet.Put( aItem );
            }
            break;
            case SID_BASICIDE_STAT_TITLE:
            {
                if ( pCurWin )
                {
/*
                    String aTitle;
                    StarBASIC* pBasic = pCurWin->GetBasic();
                    BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
                    DBG_ASSERT( pBasMgr, "BasMgr?!" );
                    aTitle += BasicIDE::FindTitle( pBasMgr );
                    aTitle += '.';
                    aTitle += pBasic->GetName();
                    String aExtraTitle = pCurWin->GetTitle();
                    if ( aExtraTitle.Len() )
                    {
                        aTitle += '.';
                        aTitle += aExtraTitle;
                    }
*/
                    String aTitle = pCurWin->CreateQualifiedName();
                    SfxStringItem aItem( SID_BASICIDE_STAT_TITLE, aTitle );
                    rSet.Put( aItem );
                }
            }
            break;
            // Werden vom Controller ausgewertet:
            case SID_ATTR_SIZE:
            case SID_ATTR_INSERT:
            break;
            case SID_UNDO:
            case SID_REDO:
            {
                if( GetUndoManager() )  // sonst rekursives GetState
                    GetViewFrame()->GetSlotState( nWh, NULL, &rSet );
            }
            break;
        }
    }
    if ( pCurWin )
        pCurWin->GetState( rSet );
}

void BasicIDEShell::SetCurWindow( IDEBaseWindow* pNewWin, BOOL bUpdateTabBar, BOOL bRememberAsCurrent )
{
    // Es muss ein EditWindow am Sfx gesetzt sein, sonst kommt kein
    // Resize, also stehen die Controls auf den Wiese...
    // Sieht dann sowieso besser aus, wenn das Modul-Layout angezeigt wird...
    if ( !pNewWin && ( GetWindow() != pModulLayout ) )
    {
        pModulLayout->Show();
        AdjustPosSizePixel( Point( 0, 0 ), GetViewFrame()->GetWindow().GetOutputSizePixel() );
        SetWindow( pModulLayout );
        EnableScrollbars( FALSE );
        aVScrollBar.Hide();
    }

    if ( pNewWin != pCurWin )
    {
        IDEBaseWindow* pPrevCurWin = pCurWin;
        pCurWin = pNewWin;
        if ( pPrevCurWin )
        {
            pPrevCurWin->Hide();
            pPrevCurWin->Deactivating();
//          pPrevCurWin->GetLayoutWindow()->Hide();
            if( pPrevCurWin->IsA( TYPE( DialogWindow ) ) )
            {
                ((DialogWindow*)pPrevCurWin)->DisableBrowser();
            }
            else
            {
                pModulLayout->SetModulWindow( NULL );
            }
        }
        if ( pCurWin )
        {
            AdjustPosSizePixel( Point( 0, 0 ), GetViewFrame()->GetWindow().GetOutputSizePixel() );
            if( pCurWin->IsA( TYPE( ModulWindow ) ) )
            {
                GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_MODULWINDOW );
                pModulLayout->SetModulWindow( (ModulWindow*)pCurWin );
                pModulLayout->Show();

                if ( bRememberAsCurrent )
                {
                    StarBASIC* pLib = ((ModulWindow*)pCurWin)->GetBasic();
                    LibInfo* pLibInf =  IDE_DLL()->GetExtraData()->GetLibInfos().GetInfo( pLib, TRUE );
                    pLibInf->aCurrentModule = ((ModulWindow*)pCurWin)->GetModule()->GetName();
                }
            }
            else
            {
                pModulLayout->Hide();
                GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_DIALOGWINDOW );
            }
            if ( GetViewFrame()->GetWindow().IsVisible() ) // sonst macht es spaeter der SFX
                pCurWin->Show();

            pCurWin->Init();

            if ( !IDE_DLL()->GetExtraData()->ShellInCriticalSection() )
            {
                Window* pFrameWindow = &GetViewFrame()->GetWindow();
                Window* pFocusWindow = Application::GetFocusWindow();
                while ( pFocusWindow && ( pFocusWindow != pFrameWindow ) )
                    pFocusWindow = pFocusWindow->GetParent();
                if ( pFocusWindow ) // Focus in BasicIDE
                    pNewWin->GrabFocus();
            }
            if( pCurWin->IsA( TYPE( DialogWindow ) ) )
                ((DialogWindow*)pCurWin)->UpdateBrowser();
        }
        if ( bUpdateTabBar )
        {
            ULONG nKey = aIDEWindowTable.GetKey( pCurWin );
            if ( pCurWin && ( pTabBar->GetPagePos( (USHORT)nKey ) == TAB_PAGE_NOTFOUND ) )
                pTabBar->InsertPage( (USHORT)nKey, pCurWin->GetTitle() );   // wurde neu eingeblendet
            pTabBar->SetCurPageId( (USHORT)nKey );
        }
        if ( pCurWin && pCurWin->IsSuspended() )    // Wenn das Fenster im Fehlerfall angezeigt wird...
            pCurWin->SetStatus( pCurWin->GetStatus() & ~BASWIN_SUSPENDED );
        if ( pCurWin )
        {
            SetWindow( pCurWin );
        }
        else
        {
            SetWindow( pModulLayout );
            GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_MODULWINDOW );
        }
        SetUndoManager( pCurWin ? pCurWin->GetUndoManager() : 0 );
        InvalidateBasicIDESlots();
        SetMDITitle();
        EnableScrollbars( pCurWin ? TRUE : FALSE );

    }
}

IDEBaseWindow* BasicIDEShell::FindWindow( const SbxObject* pObj, BOOL bFindSuspended )
{
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin )
    {
        if ( !pWin->IsSuspended() || bFindSuspended )
        {
            if ( !pObj )    // irgendein nicht suspendiertes
                return pWin;
            else if ( pWin->IsA( TYPE( ModulWindow ) ) )
            {
                if ( ((ModulWindow*)pWin)->GetModule() == pObj )
                    return pWin;
            }
            else
            {
                if ( ((DialogWindow*)pWin)->GetDialog() == pObj )
                    return pWin;
            }
        }
        pWin = aIDEWindowTable.Next();
    }
    return 0;
}

long BasicIDEShell::CallBasicErrorHdl( StarBASIC* pBasic )
{
    long nRet = 0;
    ModulWindow* pModWin = ShowActiveModuleWindow( pBasic );
    if ( pModWin )
        nRet = pModWin->BasicErrorHdl( pBasic );
    return nRet;
}

long BasicIDEShell::CallBasicBreakHdl( StarBASIC* pBasic )
{
    long nRet = 0;
    ModulWindow* pModWin = ShowActiveModuleWindow( pBasic );
    if ( pModWin )
    {
        BOOL bAppWindowDisabled, bDispatcherLocked;
        USHORT nWaitCount;
        SfxUInt16Item *pSWActionCount, *pSWLockViewCount;
        BasicIDE::BasicStopped( &bAppWindowDisabled, &bDispatcherLocked,
                                &nWaitCount, &pSWActionCount, &pSWLockViewCount );

        nRet = pModWin->BasicBreakHdl( pBasic );

        if ( StarBASIC::IsRunning() )   // Falls abgebrochen...
        {
            if ( bAppWindowDisabled )
                Application::GetDefModalDialogParent()->Enable( FALSE );
            if ( bDispatcherLocked )
                SFX_APP()->LockDispatcher( TRUE );

            for ( USHORT n = 0; n < nWaitCount; n++ )
                Application::EnterWait();
        }
    }
    return nRet;
}

ModulWindow* BasicIDEShell::ShowActiveModuleWindow( StarBASIC* pBasic )
{
    if ( pCurBasic && ( pBasic != pCurBasic ) )
        SetCurBasic( 0, FALSE );

    SbModule* pActiveModule = StarBASIC::GetActiveModule();
    DBG_ASSERT( pActiveModule, "Kein aktives Modul im ErrorHdl?!" );
    if ( pActiveModule )
    {
        SbxObject* pParent = pActiveModule->GetParent();
        DBG_ASSERT( pParent && pParent->ISA( StarBASIC ), "Kein BASIC!" );
        ModulWindow* pWin = FindBasWin( (StarBASIC*)pParent, pActiveModule->GetName(), TRUE );
        DBG_ASSERT( pWin, "Error/Step-Hdl: Fenster wurde nicht erzeugt/gefunden!" );
        SetCurWindow( pWin, TRUE );
        BasicManager* pBasicMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasicMgr )
            StartListening( *pBasicMgr, TRUE /* Nur einmal anmelden */ );
        return pWin;
    }
    return 0;
}

void __EXPORT BasicIDEShell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    // Nicht wenn minimiert, weil dann bei Restore der Text verschoben ist.
    if ( GetViewFrame()->GetWindow().GetOutputSizePixel().Height() == 0 )
        return;

    Size aSz( rSize );
//  long nScrollbarWidthPixel = aVScrollBar.GetSizePixel().Width();
    Size aScrollBarBoxSz( aScrollBarBox.GetSizePixel() );
    aSz.Height() -= aScrollBarBoxSz.Height();
#ifndef VCL
#ifdef MAC
    aSz.Height()++;
#endif
#endif
    Size aOutSz( aSz );
    aSz.Width() -= aScrollBarBoxSz.Width();
    aScrollBarBox.SetPosPixel( Point( rSize.Width() - aScrollBarBoxSz.Width(), rSize.Height() - aScrollBarBoxSz.Height() ) );
    aVScrollBar.SetPosSizePixel( Point( rPos.X()+aSz.Width(), rPos.Y() ), Size( aScrollBarBoxSz.Width(), aSz.Height() ) );
    if ( bTabBarSplitted )
    {
        // SplitSize ist beim Resize 0 !
        long nSplitPos = pTabBar->GetSizePixel().Width();
        if ( nSplitPos > aSz.Width() )
            nSplitPos = aSz.Width();
        pTabBar->SetPosSizePixel( Point( rPos.X(), rPos.Y()+aSz.Height() ), Size( nSplitPos, aScrollBarBoxSz.Height() ) );
        long nScrlStart = rPos.X() + nSplitPos;
        aHScrollBar.SetPosSizePixel( Point( nScrlStart, rPos.Y()+aSz.Height() ), Size( aSz.Width() - nScrlStart + 1, aScrollBarBoxSz.Height() ) );
        aHScrollBar.Update();
    }
    else
    {
        aHScrollBar.SetPosSizePixel( Point( rPos.X()+ aSz.Width()/2 - 1, rPos.Y()+aSz.Height() ), Size( aSz.Width()/2 + 2, aScrollBarBoxSz.Height() ) );
        pTabBar->SetPosSizePixel( Point( rPos.X(), rPos.Y()+aSz.Height() ), Size( aSz.Width()/2, aScrollBarBoxSz.Height() ) );
    }

    Window* pEdtWin = pCurWin ? pCurWin->GetLayoutWindow() : pModulLayout;
    if ( pEdtWin )
    {
        if( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) )
            pEdtWin->SetPosSizePixel( rPos, aSz );  // Ohne ScrollBar
        else
            pEdtWin->SetPosSizePixel( rPos, aOutSz );
    }
}

void __EXPORT BasicIDEShell::Activate( BOOL bMDI )
{
    if ( bMDI )
    {
        if( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) )
            ((DialogWindow*)pCurWin)->UpdateBrowser();

        // Accelerator hauptsaechlich aus kompatibilitaetsgruenden in
        // ExtraData statt in ViewShell
        Accelerator* pAccel = IDE_DLL()->GetExtraData()->GetAccelerator();
        DBG_ASSERT( pAccel, "Accel?!" );
        pAccel->SetSelectHdl( LINK( this, BasicIDEShell, AccelSelectHdl ) );
        Application::InsertAccel( pAccel );

        ShowObjectDialog( TRUE, FALSE );
    }
}

void __EXPORT BasicIDEShell::Deactivate( BOOL bMDI )
{
    // bMDI TRUE heisst, dass ein anderes MDI aktiviert wurde, bei einem
    // Deactivate durch eine MessageBox ist bMDI FALSE
    if ( bMDI )
    {
        if( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) )
        {
            DialogWindow* pXDlgWin = (DialogWindow*)pCurWin;
            pXDlgWin->DisableBrowser();
            if( pXDlgWin->IsModified() )
                BasicIDE::MarkDocShellModified( pXDlgWin->GetBasic() );
        }

        // CanClose pruefen, damit auch beim deaktivieren der BasicIDE geprueft wird,
        // ob in einem Modul der Sourcecode zu gross ist...
        for ( ULONG nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
        {
            IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
            if ( /* !pWin->IsSuspended() && */ !pWin->CanClose() )
            {
                if ( pCurBasic && ( pWin->GetBasic() != pCurBasic ) )
                    SetCurBasic( 0, FALSE );
                SetCurWindow( pWin, TRUE );
                break;
            }
        }

        Accelerator* pAccel = IDE_DLL()->GetExtraData()->GetAccelerator();
        DBG_ASSERT( pAccel, "Accel?!" );
        pAccel->SetSelectHdl( Link() );
        Application::RemoveAccel( pAccel );

        ShowObjectDialog( FALSE, FALSE );
    }
}


IMPL_LINK( BasicIDEShell, AccelSelectHdl, Accelerator*, pAccel )
{
    BOOL bDone = TRUE;
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( !pDispatcher )
        return FALSE;
    switch ( pAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_F5:
            if ( pAccel->GetCurKeyCode().IsShift() || pAccel->GetCurKeyCode().IsMod2() )
                pDispatcher->Execute( SID_BASICSTOP, SFX_CALLMODE_SYNCHRON );
            if ( !pAccel->GetCurKeyCode().IsMod2() )
            {
                // Muss asynchron sein, damit ggf. STOP wirken kan (Reschedule).
                pDispatcher->Execute( SID_BASICRUN, SFX_CALLMODE_ASYNCHRON );
            }
        break;
        case KEY_F7:
            pDispatcher->Execute( SID_BASICIDE_ADDWATCH, SFX_CALLMODE_SYNCHRON );
        break;
        case KEY_F8:
            if ( pAccel->GetCurKeyCode().IsShift() )
                pDispatcher->Execute( SID_BASICSTEPOVER, SFX_CALLMODE_SYNCHRON );
            else
                pDispatcher->Execute( SID_BASICSTEPINTO, SFX_CALLMODE_SYNCHRON );
        break;
        case KEY_F9:
            pDispatcher->Execute( SID_BASICIDE_TOGGLEBRKPNT, SFX_CALLMODE_SYNCHRON );
        break;
        default:    bDone = FALSE;
    }
    return bDone;
}

