/*************************************************************************
 *
 *  $RCSfile: basides1.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:02:12 $
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

#define _SOLAR__PRIVATE 1

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
#include <sbxitem.hxx>
#include <helpid.hrc>

#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_MINFITEM_HXX //autogen
#include <sfx2/minfitem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif

#include <algorithm>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


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
            if ( pCurWin->ISA( ModulWindow ) )
            {
                // module
                SfxObjectShell* pShell = pCurWin->GetShell();
                String aLibName = pCurWin->GetLibName();
                String aName = pCurWin->GetName();
                if ( QueryDelModule( aName, pCurWin ) )
                {
                    try
                    {
                        BasicIDE::RemoveModule( pShell, aLibName, aName );
                        RemoveWindow( pCurWin, TRUE );
                        BasicIDE::MarkDocShellModified( pShell );
                    }
                    catch ( container::NoSuchElementException& e )
                    {
                        ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                        DBG_ERROR( aBStr.GetBuffer() );
                    }
                }
            }
            else
            {
                // dialog
                SfxObjectShell* pShell = pCurWin->GetShell();
                String aLibName = pCurWin->GetLibName();
                String aName = pCurWin->GetName();
                if ( QueryDelDialog( aName, pCurWin ) )
                {
                    try
                    {
                        BasicIDE::RemoveDialog( pShell, aLibName, aName );
                        RemoveWindow( pCurWin, TRUE );
                        BasicIDE::MarkDocShellModified( pShell );
                    }
                    catch ( container::NoSuchElementException& e )
                    {
                        ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                        DBG_ERROR( aBStr.GetBuffer() );
                    }
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
                        aReplStr.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "XX" ) ), String::CreateFromInt32( nFound ) );
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
                                    SfxViewFrame* pViewFrame = GetViewFrame();
                                    SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow( SID_SEARCH_DLG ) : NULL;
                                    Window* pParent = pChildWin ? pChildWin->GetWindow() : NULL;
                                    QueryBox aQuery( pParent, WB_YES_NO|WB_DEF_YES, String( IDEResId( RID_STR_SEARCHFROMSTART ) ) );
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
        case FID_SEARCH_OFF:
        {
            if ( pCurWin && pCurWin->ISA( ModulWindow ) )
                pCurWin->GrabFocus();
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

                // document basic
                SfxObjectShell* pShell = pCurWin->GetShell();
                if ( pShell )
                    pShell->ExecuteSlot( rReq );

                SfxBindings &rBindings = BasicIDE::GetBindings();
                rBindings.Invalidate( SID_DOC_MODIFIED );
                rBindings.Invalidate( SID_SAVEDOC );
            }
        }
        break;
        case SID_BASICIDE_MODULEDLG:
        {
            if ( rReq.GetArgs() )
            {
                const SfxUInt16Item &rTabId = (const SfxUInt16Item&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID );
                BasicIDE::Organize( rTabId.GetValue() );
            }
            else
                BasicIDE::Organize( 0 );
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
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            StartListening( *pBasMgr, TRUE /* Nur einmal anmelden */ );
            String aLibName( rInfo.GetLib() );
            if ( !aLibName.Len() )
                aLibName = String::CreateFromAscii( "Standard" );
            StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
            if ( !pBasic )
            {
                // LoadOnDemand
                ::rtl::OUString aOULibName( aLibName );

                // load module library (if not loaded)
                Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
                    xModLibContainer->loadLibrary( aOULibName );

                // load dialog library (if not loaded)
                Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );
                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                    xDlgLibContainer->loadLibrary( aOULibName );

                // get Basic
                pBasic = pBasMgr->GetLib( aLibName );
            }
            DBG_ASSERT( pBasic, "Kein Basic!" );

            SetCurLib( pShell, aLibName );

            if ( rReq.GetSlot() == SID_BASICIDE_CREATEMACRO )
            {
                SbModule* pModule = pBasic->FindModule( rInfo.GetModule() );
                if ( !pModule )
                {
                    if ( rInfo.GetModule().Len() || !pBasic->GetModules()->Count() )
                    {
                        String aModName = rInfo.GetModule();

                        try
                        {
                            ::rtl::OUString aModule = BasicIDE::CreateModule( pShell, aLibName, aModName );
                            pModule = pBasic->FindModule( aModName );
                        }
                        catch ( container::ElementExistException& e )
                        {
                            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                            DBG_ERROR( aBStr.GetBuffer() );
                        }
                        catch ( container::NoSuchElementException& e )
                        {
                            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                            DBG_ERROR( aBStr.GetBuffer() );
                        }
                    }
                    else
                        pModule = (SbModule*) pBasic->GetModules()->Get(0);
                }
                DBG_ASSERT( pModule, "Kein Modul!" );
                if ( !pModule->GetMethods()->Find( rInfo.GetMethod(), SbxCLASS_METHOD ) )
                    BasicIDE::CreateMacro( pModule, rInfo.GetMethod() );
            }
            BasicIDE::Appear();
            ModulWindow* pWin = FindBasWin( pShell, aLibName, rInfo.GetModule(), TRUE );
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
        case SID_BASICIDE_NAMECHANGEDONTAB:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxUInt16Item &rTabId = (const SfxUInt16Item&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID );
            const SfxStringItem &rModName = (const SfxStringItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MODULENAME );
            IDEBaseWindow* pWin = aIDEWindowTable.Get( rTabId.GetValue() );
            DBG_ASSERT( pWin, "Window nicht im Liste, aber in TabBar ?" );
            if ( pWin )
            {
                String aNewName( rModName.GetValue() );
                String aOldName( pWin->GetName() );

                if ( aNewName != aOldName )
                {
                    if ( ( pWin->IsA( TYPE( ModulWindow ) ) && ((ModulWindow*)pWin)->RenameModule( aNewName ) )
                         || ( pWin->IsA( TYPE( DialogWindow ) ) && ((DialogWindow*)pWin)->RenameDialog( aNewName ) ) )
                    {
                        BasicIDE::MarkDocShellModified( pWin->GetShell() );
                    }
                    else
                    {
                        // set old name in TabWriter
                        USHORT nId = (USHORT)aIDEWindowTable.GetKey( pWin );
                        DBG_ASSERT( nId, "No entry in Tabbar!" );
                        if ( nId )
                            pTabBar->SetPageText( nId, aOldName );
                    }
                }

                // set focus to current window
                pWin->GrabFocus();
            }
        }
        break;
        case SID_BASICIDE_STOREMODULESOURCE:
        case SID_BASICIDE_UPDATEMODULESOURCE:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxMacroInfoItem& rInfo = (const SfxMacroInfoItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO );
            BasicManager* pBasMgr = (BasicManager*)rInfo.GetBasicManager();
            DBG_ASSERT( pBasMgr, "Store source: Kein BasMgr?" );
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            ModulWindow* pWin = FindBasWin( pShell, rInfo.GetLib(), rInfo.GetModule(), FALSE, TRUE );
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
            const SfxObjectShellItem& rShellItem = (const SfxObjectShellItem&)rReq.GetArgs()->Get( SID_BASICIDE_ARG_SHELL );
            SfxObjectShell* pShell = rShellItem.GetObjectShell();
            const SfxStringItem& rLibNameItem = (const SfxStringItem&)rReq.GetArgs()->Get( SID_BASICIDE_ARG_LIBNAME );
            String aLibName( rLibNameItem.GetValue() );

            if ( nSlot == SID_BASICIDE_LIBSELECTED )
            {
                ::rtl::OUString aOULibName( aLibName );

                // load module library (if not loaded)
                Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
                    xModLibContainer->loadLibrary( aOULibName );

                // load dialog library (if not loaded)
                Reference< script::XLibraryContainer > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                    xDlgLibContainer->loadLibrary( aOULibName );

                // check password, if library is password protected and not verified
                BOOL bOK = TRUE;
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    {
                        String aPassword;
                        bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                    }
                }

                if ( bOK )
                {
                    SetCurLib( pShell, aLibName, true, false );
                }
                else
                {
                    // alten Wert einstellen...
                    BasicIDE::GetBindings().Invalidate( SID_BASICIDE_LIBSELECTOR, TRUE, FALSE );
                }
            }
            else if ( nSlot == SID_BASICIDE_LIBREMOVED )
            {
                if ( !m_aCurLibName.Len() || ( pShell == m_pCurShell && aLibName == m_aCurLibName ) )
                {
                    RemoveWindows( pShell, aLibName, TRUE );
                    if ( pShell == m_pCurShell && aLibName == m_aCurLibName )
                    {
                        m_pCurShell = 0;
                        m_aCurLibName = String();
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
            ModulWindow* pWin = CreateBasWin( m_pCurShell, m_aCurLibName, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_NEWDIALOG:
        {
            DialogWindow* pWin = CreateDlgWin( m_pCurShell, m_aCurLibName, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_SBXRENAMED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
        }
        break;
        case SID_BASICIDE_SBXINSERTED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SfxObjectShell* pShell = rSbxItem.GetShell();
            String aLibName( rSbxItem.GetLibName() );
            String aName( rSbxItem.GetName() );
            if ( !m_aCurLibName.Len() || ( pShell == m_pCurShell && aLibName == m_aCurLibName ) )
            {
                IDEBaseWindow* pWin = 0;
                if ( rSbxItem.GetType() == BASICIDE_TYPE_MODULE )
                    pWin = FindBasWin( pShell, aLibName, aName, TRUE );
                else if ( rSbxItem.GetType() == BASICIDE_TYPE_DIALOG )
                    pWin = FindDlgWin( pShell, aLibName, aName, TRUE );
            }
        }
        break;
        case SID_BASICIDE_SBXDELETED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            IDEBaseWindow* pWin = FindWindow( rSbxItem.GetShell(), rSbxItem.GetLibName(), rSbxItem.GetName(), rSbxItem.GetType(), TRUE );
            if ( pWin )
                RemoveWindow( pWin, TRUE );
        }
        break;
        case SID_BASICIDE_SHOWSBX:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            SfxObjectShell* pShell = rSbxItem.GetShell();
            String aLibName( rSbxItem.GetLibName() );
            String aName( rSbxItem.GetName() );
            SetCurLib( pShell, aLibName );
            IDEBaseWindow* pWin = 0;
            if ( rSbxItem.GetType() == BASICIDE_TYPE_DIALOG )
            {
                pWin = FindDlgWin( pShell, aLibName, aName, TRUE );
            }
            else if ( rSbxItem.GetType() == BASICIDE_TYPE_MODULE )
            {
                pWin = FindBasWin( pShell, aLibName, aName, TRUE );
            }
            else if ( rSbxItem.GetType() == BASICIDE_TYPE_METHOD )
            {
                pWin = FindBasWin( pShell, aLibName, aName, TRUE );
                ((ModulWindow*)pWin)->EditMacro( rSbxItem.GetMethodName() );
            }
            DBG_ASSERT( pWin, "Fenster wurde nicht erzeugt!" );
            SetCurWindow( pWin, TRUE );
            pTabBar->MakeVisible( pTabBar->GetCurPageId() );
        }
        break;
        case SID_SHOW_PROPERTYBROWSER:
        {
            GetViewFrame()->ChildWindowExecute( rReq );
            rReq.Done();
        }
        break;
        case SID_BASICIDE_SHOWWINDOW:
        {
            SfxObjectShell* pShell = 0;
            String aDocument;

            SFX_REQUEST_ARG( rReq, pDocumentItem, SfxStringItem, SID_BASICIDE_ARG_DOCUMENT, sal_False );
            if ( pDocumentItem )
                aDocument = pDocumentItem->GetValue();
            if ( aDocument.Len() != 0 )
            {
                SfxViewFrame* pView = SfxViewFrame::GetFirst();
                while ( pView )
                {
                    SfxObjectShell* pObjShell = pView->GetObjectShell();
                    if ( pObjShell )
                    {
                        SfxMedium* pMedium = pObjShell->GetMedium();
                        if ( ( pMedium && aDocument == pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) ) ||
                                aDocument == pObjShell->GetTitle( SFX_TITLE_FILENAME ) )
                        {
                            pShell = pObjShell;
                            break;
                        }
                    }
                    pView = SfxViewFrame::GetNext( *pView );
                }
            }

            SFX_REQUEST_ARG( rReq, pLibNameItem, SfxStringItem, SID_BASICIDE_ARG_LIBNAME, sal_False );
            if ( pLibNameItem )
            {
                String aLibName( pLibNameItem->GetValue() );
                Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
                    xModLibContainer->loadLibrary( aLibName );
                SetCurLib( pShell, aLibName );
                SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_BASICIDE_ARG_NAME, sal_False );
                if ( pNameItem )
                {
                    String aName( pNameItem->GetValue() );
                    String aModType( String::CreateFromAscii( "Module" ) );
                    String aDlgType( String::CreateFromAscii( "Dialog" ) );
                    String aType( aModType );
                    SFX_REQUEST_ARG( rReq, pTypeItem, SfxStringItem, SID_BASICIDE_ARG_TYPE, sal_False );
                    if ( pTypeItem )
                        aType = pTypeItem->GetValue();

                    IDEBaseWindow* pWin = 0;
                    if ( aType == aModType )
                        pWin = FindBasWin( pShell, aLibName, aName, FALSE );
                    else if ( aType == aDlgType )
                        pWin = FindDlgWin( pShell, aLibName, aName, FALSE );

                    if ( pWin )
                    {
                        SetCurWindow( pWin, TRUE );
                        if ( pTabBar )
                            pTabBar->MakeVisible( pTabBar->GetCurPageId() );

                        if ( pWin->ISA( ModulWindow ) )
                        {
                            ModulWindow* pModWin = (ModulWindow*)pWin;
                            SFX_REQUEST_ARG( rReq, pLineItem, SfxUInt32Item, SID_BASICIDE_ARG_LINE, sal_False );
                            if ( pLineItem )
                            {
                                pModWin->AssertValidEditEngine();
                                TextView* pTextView = pModWin->GetEditView();
                                if ( pTextView )
                                {
                                    TextEngine* pTextEngine = pTextView->GetTextEngine();
                                    if ( pTextEngine )
                                    {
                                        sal_uInt32 nLine = pLineItem->GetValue();
                                        sal_uInt32 nLineCount = 0;
                                        for ( sal_uInt32 i = 0, nCount = pTextEngine->GetParagraphCount(); i < nCount; ++i )
                                            nLineCount += pTextEngine->GetLineCount( i );
                                        if ( nLine > nLineCount )
                                            nLine = nLineCount;
                                        if ( nLine > 0 )
                                            --nLine;

                                        // scroll window and set selection
                                        long nVisHeight = pModWin->GetOutputSizePixel().Height();
                                        long nTextHeight = pTextEngine->GetTextHeight();
                                        if ( nTextHeight > nVisHeight )
                                        {
                                            long nMaxY = nTextHeight - nVisHeight;
                                            long nOldY = pTextView->GetStartDocPos().Y();
                                            long nNewY = nLine * pTextEngine->GetCharHeight() - nVisHeight / 2;
                                            nNewY = ::std::min( nNewY, nMaxY );
                                            pTextView->Scroll( 0, -( nNewY - nOldY ) );
                                            pTextView->ShowCursor( FALSE, TRUE );
                                            pModWin->GetEditVScrollBar().SetThumbPos( pTextView->GetStartDocPos().Y() );
                                        }
                                        sal_uInt16 nCol1 = 0, nCol2 = 0;
                                        SFX_REQUEST_ARG( rReq, pCol1Item, SfxUInt16Item, SID_BASICIDE_ARG_COLUMN1, sal_False );
                                        if ( pCol1Item )
                                        {
                                            nCol1 = pCol1Item->GetValue();
                                            if ( nCol1 > 0 )
                                                --nCol1;
                                            nCol2 = nCol1;
                                        }
                                        SFX_REQUEST_ARG( rReq, pCol2Item, SfxUInt16Item, SID_BASICIDE_ARG_COLUMN2, sal_False );
                                        if ( pCol2Item )
                                        {
                                            nCol2 = pCol2Item->GetValue();
                                            if ( nCol2 > 0 )
                                                --nCol2;
                                        }
                                        TextSelection aSel( TextPaM( nLine, nCol1 ), TextPaM( nLine, nCol2 ) );
                                        pTextView->SetSelection( aSel );
                                        pTextView->ShowCursor();
                                        Window* pWindow = pTextView->GetWindow();
                                        if ( pWindow )
                                            pWindow->GrabFocus();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            rReq.Done();
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
                BOOL bDisable = FALSE;

                if ( pCurWin )
                {
                    if ( !pCurWin->IsModified() )
                    {
                        SfxObjectShell* pShell = pCurWin->GetShell();
                        if ( pShell )
                        {
                            // document
                            if ( !pShell->IsModified() )
                                bDisable = TRUE;
                        }
                        else
                        {
                            // application
                            if ( !IsAppBasicModified() )
                                bDisable = TRUE;
                        }
                    }
                }
                else
                {
                    bDisable = TRUE;
                }

                if ( bDisable )
                    rSet.DisableItem( nWh );
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
            case SID_BASICIDE_MANAGEBRKPNTS:
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
                String aName;
                if ( m_aCurLibName.Len() )
                {
                    LibraryLocation eLocation = BasicIDE::GetLibraryLocation( m_pCurShell, m_aCurLibName );
                    aName = CreateMgrAndLibStr( BasicIDE::GetTitle( m_pCurShell, eLocation ), m_aCurLibName );
                }
                SfxStringItem aItem( SID_BASICIDE_LIBSELECTOR, aName );
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
                BOOL bModified = FALSE;

                if ( pCurWin )
                {
                    if ( pCurWin->IsModified() )
                        bModified = TRUE;

                    SfxObjectShell* pShell = pCurWin->GetShell();
                    if ( pShell )
                    {
                        // document
                        if ( pShell->IsModified() )
                            bModified = TRUE;
                    }
                    else
                    {
                        // application
                        if ( IsAppBasicModified() )
                            bModified = TRUE;
                    }
                }

                if ( bModified )
                    aModifiedMarker = '*';

                SfxStringItem aItem( SID_DOC_MODIFIED, aModifiedMarker );
                rSet.Put( aItem );
            }
            break;
            case SID_BASICIDE_STAT_TITLE:
            {
                if ( pCurWin )
                {
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
            case SID_SHOW_PROPERTYBROWSER:
            {
                if ( GetViewFrame()->KnowsChildWindow( nWh ) )
                    rSet.Put( SfxBoolItem( nWh, GetViewFrame()->HasChildWindow( nWh ) ) );
                else
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
    if ( pCurWin )
        pCurWin->GetState( rSet );
}

BOOL BasicIDEShell::HasUIFeature( ULONG nFeature )
{
    BOOL bResult = FALSE;

    if ( (nFeature & BASICIDE_UI_FEATURE_SHOW_BROWSER) == BASICIDE_UI_FEATURE_SHOW_BROWSER )
    {
        // fade out (in) property browser in module (dialog) windows
        if ( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) && !pCurWin->IsReadOnly() )
            bResult = TRUE;
    }

    return bResult;
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
            }
            else
            {
                pModulLayout->Hide();
                GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_DIALOGWINDOW );
            }

            if ( bRememberAsCurrent )
            {
                BasicIDEData* pData = IDE_DLL()->GetExtraData();
                if ( pData )
                {
                    USHORT nCurrentType = pCurWin->IsA( TYPE( ModulWindow ) ) ? BASICIDE_TYPE_MODULE : BASICIDE_TYPE_DIALOG;
                    LibInfoItem* pLibInfoItem = new LibInfoItem( pCurWin->GetShell(), pCurWin->GetLibName(), pCurWin->GetName(), nCurrentType );
                    pData->GetLibInfos().InsertInfo( pLibInfoItem );
                }
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
            SfxObjectShell::SetWorkingDocument( pCurWin->GetShell() );
        }
        else
        {
            SetWindow( pModulLayout );
            GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_MODULWINDOW );
            SfxObjectShell::SetWorkingDocument( NULL );
        }
        SetUndoManager( pCurWin ? pCurWin->GetUndoManager() : 0 );
        InvalidateBasicIDESlots();
        SetMDITitle();
        EnableScrollbars( pCurWin ? TRUE : FALSE );

        // fade out (in) property browser in module (dialog) windows
        UIFeatureChanged();
    }
}

IDEBaseWindow* BasicIDEShell::FindWindow( SfxObjectShell* pShell, const String& rLibName, const String& rName, USHORT nType, BOOL bFindSuspended )
{
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin )
    {
        if ( !pWin->IsSuspended() || bFindSuspended )
        {
            if ( !rLibName.Len() || !rName.Len() || nType == BASICIDE_TYPE_UNKNOWN )
            {
                // return any non-suspended window
                return pWin;
            }
            else if ( pWin->GetShell() == pShell && pWin->GetLibName() == rLibName && pWin->GetName() == rName &&
                      ( ( pWin->IsA( TYPE( ModulWindow ) )  && nType == BASICIDE_TYPE_MODULE ) ||
                        ( pWin->IsA( TYPE( DialogWindow ) ) && nType == BASICIDE_TYPE_DIALOG ) ) )
            {
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
                Application::GetDefDialogParent()->Enable( FALSE );
            if ( bDispatcherLocked )
                SFX_APP()->LockDispatcher( TRUE );

            if ( nWaitCount )
            {
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                for ( USHORT n = 0; n < nWaitCount; n++ )
                    pIDEShell->GetViewFrame()->GetWindow().EnterWait();
            }
        }
    }
    return nRet;
}

ModulWindow* BasicIDEShell::ShowActiveModuleWindow( StarBASIC* pBasic )
{
    SetCurLib( 0, String(), false );

    SbModule* pActiveModule = StarBASIC::GetActiveModule();
    DBG_ASSERT( pActiveModule, "Kein aktives Modul im ErrorHdl?!" );
    if ( pActiveModule )
    {
        ModulWindow* pWin = 0;
        SbxObject* pParent = pActiveModule->GetParent();
        DBG_ASSERT( pParent && pParent->ISA( StarBASIC ), "Kein BASIC!" );
        StarBASIC* pLib = static_cast< StarBASIC* >( pParent );
        if ( pLib )
        {
            BasicManager* pBasMgr = BasicIDE::FindBasicManager( pLib );
            if ( pBasMgr )
            {
                SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
                pWin = FindBasWin( pShell, pLib->GetName(), pActiveModule->GetName(), TRUE );
                DBG_ASSERT( pWin, "Error/Step-Hdl: Fenster wurde nicht erzeugt/gefunden!" );
                SetCurWindow( pWin, TRUE );
            }
        }
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
                BasicIDE::MarkDocShellModified( pXDlgWin->GetShell() );
        }

        // CanClose pruefen, damit auch beim deaktivieren der BasicIDE geprueft wird,
        // ob in einem Modul der Sourcecode zu gross ist...
        for ( ULONG nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
        {
            IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
            if ( /* !pWin->IsSuspended() && */ !pWin->CanClose() )
            {
                if ( m_aCurLibName.Len() && ( pWin->GetShell() != m_pCurShell || pWin->GetLibName() != m_aCurLibName ) )
                    SetCurLib( 0, String(), false );
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
    SfxViewFrame* pViewFrame = GetViewFrame();
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( !pDispatcher )
        return FALSE;
    switch ( pAccel->GetCurKeyCode().GetCode() )
    {
        case KEY_F5:
            if ( pAccel->GetCurKeyCode().IsShift() )
                pDispatcher->Execute( SID_BASICSTOP, SFX_CALLMODE_SYNCHRON );
            else
                pDispatcher->Execute( SID_BASICRUN, SFX_CALLMODE_SYNCHRON );
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
            if ( pAccel->GetCurKeyCode().IsShift() )
                pDispatcher->Execute( SID_BASICIDE_TOGGLEBRKPNTENABLED, SFX_CALLMODE_SYNCHRON );
            else
                pDispatcher->Execute( SID_BASICIDE_TOGGLEBRKPNT, SFX_CALLMODE_SYNCHRON );
        break;
        default:    bDone = FALSE;
    }
    return bDone;
}

