/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include "docsignature.hxx"

#define GLOBALOVERFLOW2
#include <basic/sbx.hxx>
#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <ide_pch.hxx>

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
#include <managelang.hxx>
#include <localizationmgr.hxx>
#include <helpid.hrc>

#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/minfitem.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <algorithm>
#include <memory>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

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
            RemoveWindow( pCurWin, sal_False );
        }
        break;
        case SID_BASICIDE_DELETECURRENT:
        {
            ScriptDocument aDocument( pCurWin->GetDocument() );
            String aLibName = pCurWin->GetLibName();
            String aName = pCurWin->GetName();

            if ( pCurWin->ISA( ModulWindow ) )
            {
                // module
                if ( QueryDelModule( aName, pCurWin ) )
                {
                    if ( aDocument.removeModule( aLibName, aName ) )
                    {
                        BasicIDE::MarkDocumentModified( aDocument );
                    }
                }
            }
            else
            {
                // dialog
                if ( QueryDelDialog( aName, pCurWin ) )
                {
                    if ( BasicIDE::RemoveDialog( aDocument, aLibName, aName ) )
                    {
                        RemoveWindow( pCurWin, sal_True );
                        BasicIDE::MarkDocumentModified( aDocument );
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
                sal_uInt16 nWhich = pArgs->GetWhichByPos( 0 );
                DBG_ASSERT( nWhich, "Wich fuer SearchItem ?" );
                const SfxPoolItem& rItem = pArgs->Get( nWhich );
                DBG_ASSERT( rItem.ISA( SvxSearchItem ), "Kein Searchitem!" );
                if ( rItem.ISA( SvxSearchItem ) )
                {
                    // Item wegen der Einstellungen merken...
                    IDE_DLL()->GetExtraData()->SetSearchItem( (const SvxSearchItem&)rItem );
                    sal_uInt16 nFound = 0;
                    sal_Bool bCanceled = sal_False;
                    if ( ((const SvxSearchItem&)rItem).GetCommand() == SVX_SEARCHCMD_REPLACE_ALL )
                    {
                        sal_uInt16 nActModWindows = 0;
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
                                    nFound = nFound + ((ModulWindow*)pWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem );
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
                            sal_Bool bChangeCurWindow = sal_False;
                            aIDEWindowTable.Seek( pCurWin );
                            // Erstmal beim naechsten Anfangen, ggf. spaeter von vorne
                            IDEBaseWindow* pWin = aIDEWindowTable.Next();
                            sal_Bool bSearchedFromStart = sal_False;
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
                                        bSearchedFromStart = sal_True;
                                    }
                                    else
                                        bCanceled = sal_True;
                                }

                                if ( pWin && !pWin->IsSuspended() && pWin->IsA( TYPE( ModulWindow ) ) )
                                {
                                    if ( pWin != pCurWin )
                                    {
                                        // Groesse einstellen, damit die View
                                        // gleich richtig justiert werden kann.
                                        if ( pCurWin )
                                            pWin->SetSizePixel( pCurWin->GetSizePixel() );
                                        nFound = ((ModulWindow*)pWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem, sal_True );
                                    }
                                    if ( nFound )
                                    {
                                        bChangeCurWindow = sal_True;
                                        break;
                                    }
                                }
                                if ( pWin && ( pWin != pCurWin ) )
                                    pWin = aIDEWindowTable.Next();
                                else
                                    pWin = 0;   // Dann sind wir durch...
                            }
                            if ( !nFound && bSearchedFromStart )    // Aktuelles von vorne...
                                nFound = ((ModulWindow*)pCurWin)->StartSearchAndReplace( (const SvxSearchItem&)rItem, sal_True );
                            if ( bChangeCurWindow )
                                SetCurWindow( pWin, sal_True );
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
    sal_uInt16 nSlot = rReq.GetSlot();
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
                ScriptDocument aDocument( pCurWin->GetDocument() );
                if ( aDocument.isDocument() )
                {
                    uno::Reference< task::XStatusIndicator > xStatusIndicator;

                    SFX_REQUEST_ARG( rReq, pStatusIndicatorItem, SfxUnoAnyItem, SID_PROGRESS_STATUSBAR_CONTROL, sal_False );
                    if ( pStatusIndicatorItem )
                        OSL_VERIFY( pStatusIndicatorItem->GetValue() >>= xStatusIndicator );
                    else
                    {
                        // get statusindicator
                        SfxViewFrame *pFrame_ = GetFrame();
                        if ( pFrame_ )
                        {
                            uno::Reference< task::XStatusIndicatorFactory > xStatFactory(
                                                                        pFrame_->GetFrame().GetFrameInterface(),
                                                                        uno::UNO_QUERY );
                            if( xStatFactory.is() )
                                xStatusIndicator = xStatFactory->createStatusIndicator();
                        }

                        if ( xStatusIndicator.is() )
                            rReq.AppendItem( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, uno::makeAny( xStatusIndicator ) ) );
                    }

                    aDocument.saveDocument( xStatusIndicator );
                }

                SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                if ( pBindings )
                {
                    pBindings->Invalidate( SID_DOC_MODIFIED );
                    pBindings->Invalidate( SID_SAVEDOC );
                    pBindings->Invalidate( SID_SIGNATURE );
                }
            }
        }
        break;
        case SID_SIGNATURE:
        {
            if ( pCurWin )
            {
                ::basctl::DocumentSignature aSignature( pCurWin->GetDocument() );
                if ( aSignature.supportsSignatures() )
                {
                    aSignature.signScriptingContent();
                    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                    if ( pBindings )
                        pBindings->Invalidate( SID_SIGNATURE );
                }
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
            BasicIDE::ChooseMacro( NULL, sal_False, ::rtl::OUString() );
        }
        break;
        case SID_BASICIDE_CREATEMACRO:
        case SID_BASICIDE_EDITMACRO:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxMacroInfoItem& rInfo = (const SfxMacroInfoItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO );
            BasicManager* pBasMgr = (BasicManager*)rInfo.GetBasicManager();
            DBG_ASSERT( pBasMgr, "Nichts selektiert im Basic-Baum ?" );

            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );

            StartListening( *pBasMgr, sal_True /* Nur einmal anmelden */ );
            String aLibName( rInfo.GetLib() );
            if ( !aLibName.Len() )
                aLibName = String::CreateFromAscii( "Standard" );
            StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
            if ( !pBasic )
            {
                // load module and dialog library (if not loaded)
                aDocument.loadLibraryIfExists( E_SCRIPTS, aLibName );
                aDocument.loadLibraryIfExists( E_DIALOGS, aLibName );

                // get Basic
                pBasic = pBasMgr->GetLib( aLibName );
            }
            DBG_ASSERT( pBasic, "Kein Basic!" );

            SetCurLib( aDocument, aLibName );

            if ( rReq.GetSlot() == SID_BASICIDE_CREATEMACRO )
            {
                SbModule* pModule = pBasic->FindModule( rInfo.GetModule() );
                if ( !pModule )
                {
                    if ( rInfo.GetModule().Len() || !pBasic->GetModules()->Count() )
                    {
                        String aModName = rInfo.GetModule();

                        ::rtl::OUString sModuleCode;
                        if ( aDocument.createModule( aLibName, aModName, sal_False, sModuleCode ) )
                            pModule = pBasic->FindModule( aModName );
                    }
                    else
                        pModule = (SbModule*) pBasic->GetModules()->Get(0);
                }
                DBG_ASSERT( pModule, "Kein Modul!" );
                if ( !pModule->GetMethods()->Find( rInfo.GetMethod(), SbxCLASS_METHOD ) )
                    BasicIDE::CreateMacro( pModule, rInfo.GetMethod() );
            }
            SfxViewFrame* pViewFrame = GetViewFrame();
            if ( pViewFrame )
                pViewFrame->ToTop();
            ModulWindow* pWin = FindBasWin( aDocument, aLibName, rInfo.GetModule(), sal_True );
            DBG_ASSERT( pWin, "Edit/Create Macro: Fenster wurde nicht erzeugt/gefunden!" );
            SetCurWindow( pWin, sal_True );
            pWin->EditMacro( rInfo.GetMethod() );
        }
        break;
        case SID_BASICIDE_OBJCAT:
        {
            ShowObjectDialog( sal_True, sal_True );
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
                    bool bRenameOk = false;
                    if ( pWin->IsA( TYPE( ModulWindow ) ) )
                    {
                        ModulWindow* pModWin = (ModulWindow*)pWin;
                        String aLibName = ( pModWin->GetLibName() );
                        ScriptDocument aDocument( pWin->GetDocument() );

                        if ( BasicIDE::RenameModule( pModWin, aDocument, aLibName,  aOldName, aNewName ) )
                        {
                            bRenameOk = true;
                            // Because we listen for container events for script
                            // modules, rename will delete the 'old' window
                            // pWin has been invalidated, restore now
                            pWin = FindBasWin( aDocument, aLibName, aNewName, sal_True );
                        }

                    }
                    else if ( pWin->IsA( TYPE( DialogWindow ) ) )
                    {
                        DialogWindow* pDlgWin = (DialogWindow*)pWin;
                        bRenameOk = pDlgWin->RenameDialog( aNewName );
                    }
                    if ( bRenameOk )
                    {
                        BasicIDE::MarkDocumentModified( pWin->GetDocument() );
                    }
                    else
                    {
                        // set old name in TabWriter
                        sal_uInt16 nId = (sal_uInt16)aIDEWindowTable.GetKey( pWin );
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
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            ModulWindow* pWin = FindBasWin( aDocument, rInfo.GetLib(), rInfo.GetModule(), sal_False, sal_True );
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
            const SfxUsrAnyItem& rShellItem = (const SfxUsrAnyItem&)rReq.GetArgs()->Get( SID_BASICIDE_ARG_DOCUMENT_MODEL );
            uno::Reference< frame::XModel > xModel( rShellItem.GetValue(), UNO_QUERY );
            ScriptDocument aDocument( xModel.is() ? ScriptDocument( xModel ) : ScriptDocument::getApplicationScriptDocument() );
            const SfxStringItem& rLibNameItem = (const SfxStringItem&)rReq.GetArgs()->Get( SID_BASICIDE_ARG_LIBNAME );
            String aLibName( rLibNameItem.GetValue() );

            if ( nSlot == SID_BASICIDE_LIBSELECTED )
            {
                // load module and dialog library (if not loaded)
                aDocument.loadLibraryIfExists( E_SCRIPTS, aLibName );
                aDocument.loadLibraryIfExists( E_DIALOGS, aLibName );

                // check password, if library is password protected and not verified
                sal_Bool bOK = sal_True;
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        String aPassword;
                        bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                    }
                }

                if ( bOK )
                {
                    SetCurLib( aDocument, aLibName, true, false );
                }
                else
                {
                    // alten Wert einstellen...
                    SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                    if ( pBindings )
                        pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR, sal_True, sal_False );
                }
            }
            else if ( nSlot == SID_BASICIDE_LIBREMOVED )
            {
                if ( !m_aCurLibName.Len() || ( aDocument == m_aCurDocument && aLibName == m_aCurLibName ) )
                {
                    RemoveWindows( aDocument, aLibName, sal_True );
                    if ( aDocument == m_aCurDocument && aLibName == m_aCurLibName )
                    {
                        m_aCurDocument = ScriptDocument::getApplicationScriptDocument();
                        m_aCurLibName = String();
                        // Kein UpdateWindows!
                        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
                        if ( pBindings )
                            pBindings->Invalidate( SID_BASICIDE_LIBSELECTOR );
                    }
                }
            }
            else    // Loaded...
                UpdateWindows();
        }
        break;
        case SID_BASICIDE_NEWMODULE:
        {
            ModulWindow* pWin = CreateBasWin( m_aCurDocument, m_aCurLibName, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, sal_True );
        }
        break;
        case SID_BASICIDE_NEWDIALOG:
        {
            DialogWindow* pWin = CreateDlgWin( m_aCurDocument, m_aCurLibName, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, sal_True );
        }
        break;
        case SID_BASICIDE_SBXRENAMED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
        }
        break;
        case SID_BASICIDE_SBXINSERTED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            ScriptDocument aDocument( rSbxItem.GetDocument() );
            String aLibName( rSbxItem.GetLibName() );
            String aName( rSbxItem.GetName() );
            if ( !m_aCurLibName.Len() || ( aDocument == m_aCurDocument && aLibName == m_aCurLibName ) )
            {
                IDEBaseWindow* pWin = 0;
                if ( rSbxItem.GetType() == BASICIDE_TYPE_MODULE )
                    pWin = FindBasWin( aDocument, aLibName, aName, sal_True );
                else if ( rSbxItem.GetType() == BASICIDE_TYPE_DIALOG )
                    pWin = FindDlgWin( aDocument, aLibName, aName, sal_True );
            }
        }
        break;
        case SID_BASICIDE_SBXDELETED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            ScriptDocument aDocument( rSbxItem.GetDocument() );
            IDEBaseWindow* pWin = FindWindow( aDocument, rSbxItem.GetLibName(), rSbxItem.GetName(), rSbxItem.GetType(), sal_True );
            if ( pWin )
                RemoveWindow( pWin, sal_True );
        }
        break;
        case SID_BASICIDE_SHOWSBX:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            ScriptDocument aDocument( rSbxItem.GetDocument() );
            String aLibName( rSbxItem.GetLibName() );
            String aName( rSbxItem.GetName() );
            SetCurLib( aDocument, aLibName );
            IDEBaseWindow* pWin = 0;
            if ( rSbxItem.GetType() == BASICIDE_TYPE_DIALOG )
            {
                pWin = FindDlgWin( aDocument, aLibName, aName, sal_True );
            }
            else if ( rSbxItem.GetType() == BASICIDE_TYPE_MODULE )
            {
                pWin = FindBasWin( aDocument, aLibName, aName, sal_True );
            }
            else if ( rSbxItem.GetType() == BASICIDE_TYPE_METHOD )
            {
                pWin = FindBasWin( aDocument, aLibName, aName, sal_True );
                ((ModulWindow*)pWin)->EditMacro( rSbxItem.GetMethodName() );
            }
            DBG_ASSERT( pWin, "Fenster wurde nicht erzeugt!" );
            SetCurWindow( pWin, sal_True );
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
            ::std::auto_ptr< ScriptDocument > pDocument;

            SFX_REQUEST_ARG( rReq, pDocumentItem, SfxStringItem, SID_BASICIDE_ARG_DOCUMENT, sal_False );
            if ( pDocumentItem )
            {
                String sDocumentCaption = pDocumentItem->GetValue();
                if ( sDocumentCaption.Len() )
                    pDocument.reset( new ScriptDocument( ScriptDocument::getDocumentWithURLOrCaption( sDocumentCaption ) ) );
            }

            SFX_REQUEST_ARG( rReq, pDocModelItem, SfxUsrAnyItem, SID_BASICIDE_ARG_DOCUMENT_MODEL, sal_False );
            if ( !pDocument.get() && pDocModelItem )
            {
                uno::Reference< frame::XModel > xModel( pDocModelItem->GetValue(), UNO_QUERY );
                if ( xModel.is() )
                    pDocument.reset( new ScriptDocument( xModel ) );
            }

            if ( !pDocument.get() )
                break;

            SFX_REQUEST_ARG( rReq, pLibNameItem, SfxStringItem, SID_BASICIDE_ARG_LIBNAME, sal_False );
            if ( !pLibNameItem )
                break;

            String aLibName( pLibNameItem->GetValue() );
            pDocument->loadLibraryIfExists( E_SCRIPTS, aLibName );
            SetCurLib( *pDocument, aLibName );
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
                    pWin = FindBasWin( *pDocument, aLibName, aName, sal_False );
                else if ( aType == aDlgType )
                    pWin = FindDlgWin( *pDocument, aLibName, aName, sal_False );

                if ( pWin )
                {
                    SetCurWindow( pWin, sal_True );
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
                                        pTextView->ShowCursor( sal_False, sal_True );
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
                                    Window* pWindow_ = pTextView->GetWindow();
                                    if ( pWindow_ )
                                        pWindow_->GrabFocus();
                                }
                            }
                        }
                    }
                }
            }
            rReq.Done();
        }
        break;

        case SID_BASICIDE_MANAGE_LANG:
        {
            ManageLanguageDialog aDlg( GetCurWindow(), GetCurLocalizationMgr() );
            aDlg.Execute();
            rReq.Done();
        }
        break;
    }
}

void __EXPORT BasicIDEShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nWh = aIter.FirstWhich(); 0 != nWh; nWh = aIter.NextWhich() )
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
                sal_Bool bDisable = sal_False;

                if ( pCurWin )
                {
                    if ( !pCurWin->IsModified() )
                    {
                        ScriptDocument aDocument( pCurWin->GetDocument() );
                        bDisable =  ( !aDocument.isAlive() )
                                ||  ( aDocument.isDocument() ? !aDocument.isDocumentModified() : !IsAppBasicModified() );
                    }
                }
                else
                {
                    bDisable = sal_True;
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
            case SID_SIGNATURE:
            {
                sal_uInt16 nState = 0;
                if ( pCurWin )
                {
                    ::basctl::DocumentSignature aSignature( pCurWin->GetDocument() );
                    nState = aSignature.getScriptingSignatureState();
                }
                rSet.Put( SfxUInt16Item( SID_SIGNATURE, nState ) );
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
                else if ( ( nWh == SID_BASICLOAD ) && ( StarBASIC::IsRunning() || ( pCurWin && pCurWin->IsReadOnly() ) ) )
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
                sal_uInt16 nOptions = 0;
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
                    LibraryLocation eLocation = m_aCurDocument.getLibraryLocation( m_aCurLibName );
                    aName = CreateMgrAndLibStr( m_aCurDocument.getTitle( eLocation ), m_aCurLibName );
                }
                SfxStringItem aItem( SID_BASICIDE_LIBSELECTOR, aName );
                rSet.Put( aItem );
            }
            break;
            case SID_SEARCH_ITEM:
            {
                String aSelected = GetSelectionText( sal_True );
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
                sal_Bool bModified = sal_False;

                if ( pCurWin )
                {
                    if ( pCurWin->IsModified() )
                        bModified = sal_True;
                    else
                    {
                        ScriptDocument aDocument( pCurWin->GetDocument() );
                        bModified = aDocument.isDocument() ? aDocument.isDocumentModified() : IsAppBasicModified();
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

            case SID_BASICIDE_CURRENT_LANG:
            {
                if( (pCurWin && pCurWin->IsReadOnly()) || GetCurLibName().Len() == 0 )
                    rSet.DisableItem( nWh );
                else
                {
                    String aItemStr;
                    LocalizationMgr* pCurMgr = GetCurLocalizationMgr();
                    if ( pCurMgr->isLibraryLocalized() )
                    {
                        Sequence< lang::Locale > aLocaleSeq = pCurMgr->getStringResourceManager()->getLocales();
                        const lang::Locale* pLocale = aLocaleSeq.getConstArray();
                        sal_Int32 i, nCount = aLocaleSeq.getLength();

                        // Force different results for any combination of locales and default locale
                        ::rtl::OUString aLangStr;
                        for ( i = 0;  i <= nCount;  ++i )
                        {
                            lang::Locale aLocale;
                            if( i < nCount )
                                aLocale = pLocale[i];
                            else
                                aLocale = pCurMgr->getStringResourceManager()->getDefaultLocale();

                            aLangStr += aLocale.Language;
                            aLangStr += aLocale.Country;
                            aLangStr += aLocale.Variant;
                        }
                        aItemStr = aLangStr;
                    }
                    rSet.Put( SfxStringItem( nWh, aItemStr ) );
                }
            }
            break;

            case SID_BASICIDE_MANAGE_LANG:
            {
                if( (pCurWin && pCurWin->IsReadOnly()) || GetCurLibName().Len() == 0 )
                    rSet.DisableItem( nWh );
            }
            break;
        }
    }
    if ( pCurWin )
        pCurWin->GetState( rSet );
}

sal_Bool BasicIDEShell::HasUIFeature( sal_uInt32 nFeature )
{
    sal_Bool bResult = sal_False;

    if ( (nFeature & BASICIDE_UI_FEATURE_SHOW_BROWSER) == BASICIDE_UI_FEATURE_SHOW_BROWSER )
    {
        // fade out (in) property browser in module (dialog) windows
        if ( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) && !pCurWin->IsReadOnly() )
            bResult = sal_True;
    }

    return bResult;
}

void BasicIDEShell::SetCurWindow( IDEBaseWindow* pNewWin, sal_Bool bUpdateTabBar, sal_Bool bRememberAsCurrent )
{
    // Es muss ein EditWindow am Sfx gesetzt sein, sonst kommt kein
    // Resize, also stehen die Controls auf den Wiese...
    // Sieht dann sowieso besser aus, wenn das Modul-Layout angezeigt wird...
    if ( !pNewWin && ( GetWindow() != pModulLayout ) )
    {
        pModulLayout->Show();
        AdjustPosSizePixel( Point( 0, 0 ), GetViewFrame()->GetWindow().GetOutputSizePixel() );
        SetWindow( pModulLayout );
        EnableScrollbars( sal_False );
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
                    sal_uInt16 nCurrentType = pCurWin->IsA( TYPE( ModulWindow ) ) ? BASICIDE_TYPE_MODULE : BASICIDE_TYPE_DIALOG;
                    LibInfoItem* pLibInfoItem = new LibInfoItem( pCurWin->GetDocument(), pCurWin->GetLibName(), pCurWin->GetName(), nCurrentType );
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
            sal_uLong nKey = aIDEWindowTable.GetKey( pCurWin );
            if ( pCurWin && ( pTabBar->GetPagePos( (sal_uInt16)nKey ) == TAB_PAGE_NOTFOUND ) )
                pTabBar->InsertPage( (sal_uInt16)nKey, pCurWin->GetTitle() );   // wurde neu eingeblendet
            pTabBar->SetCurPageId( (sal_uInt16)nKey );
        }
        if ( pCurWin && pCurWin->IsSuspended() )    // Wenn das Fenster im Fehlerfall angezeigt wird...
            pCurWin->SetStatus( pCurWin->GetStatus() & ~BASWIN_SUSPENDED );
        if ( pCurWin )
        {
            SetWindow( pCurWin );
            if ( pCurWin->GetDocument().isDocument() )
                SfxObjectShell::SetCurrentComponent( pCurWin->GetDocument().getDocument() );
        }
        else
        {
            SetWindow( pModulLayout );
            GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_MODULWINDOW );
            SfxObjectShell::SetCurrentComponent( NULL );
        }
        SetUndoManager( pCurWin ? pCurWin->GetUndoManager() : 0 );
        InvalidateBasicIDESlots();
        EnableScrollbars( pCurWin ? sal_True : sal_False );

        if ( m_pCurLocalizationMgr )
            m_pCurLocalizationMgr->handleTranslationbar();

        ManageToolbars();

        // fade out (in) property browser in module (dialog) windows
        UIFeatureChanged();
    }
}

void BasicIDEShell::ManageToolbars()
{
    static ::rtl::OUString aLayoutManagerName = ::rtl::OUString::createFromAscii( "LayoutManager" );
    static ::rtl::OUString aMacroBarResName =
        ::rtl::OUString::createFromAscii( "private:resource/toolbar/macrobar" );
    static ::rtl::OUString aDialogBarResName =
        ::rtl::OUString::createFromAscii( "private:resource/toolbar/dialogbar" );
    static ::rtl::OUString aInsertControlsBarResName =
        ::rtl::OUString::createFromAscii( "private:resource/toolbar/insertcontrolsbar" );
    (void)aInsertControlsBarResName;

    if( !pCurWin )
        return;

    Reference< beans::XPropertySet > xFrameProps
        ( GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
    if ( xFrameProps.is() )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
        uno::Any a = xFrameProps->getPropertyValue( aLayoutManagerName );
        a >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            if( pCurWin->IsA( TYPE( DialogWindow ) ) )
            {
                xLayoutManager->destroyElement( aMacroBarResName );

                xLayoutManager->requestElement( aDialogBarResName );
                xLayoutManager->requestElement( aInsertControlsBarResName );
            }
            else
            {
                xLayoutManager->destroyElement( aDialogBarResName );
                xLayoutManager->destroyElement( aInsertControlsBarResName );

                xLayoutManager->requestElement( aMacroBarResName );
            }
            xLayoutManager->unlock();
        }
    }
}

IDEBaseWindow* BasicIDEShell::FindApplicationWindow()
{
    return FindWindow( ScriptDocument::getApplicationScriptDocument() );
}

IDEBaseWindow* BasicIDEShell::FindWindow( const ScriptDocument& rDocument, const String& rLibName, const String& rName, sal_uInt16 nType, sal_Bool bFindSuspended )
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
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rName &&
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
        sal_Bool bAppWindowDisabled, bDispatcherLocked;
        sal_uInt16 nWaitCount;
        SfxUInt16Item *pSWActionCount, *pSWLockViewCount;
        BasicIDE::BasicStopped( &bAppWindowDisabled, &bDispatcherLocked,
                                &nWaitCount, &pSWActionCount, &pSWLockViewCount );

        nRet = pModWin->BasicBreakHdl( pBasic );

        if ( StarBASIC::IsRunning() )   // Falls abgebrochen...
        {
            if ( bAppWindowDisabled )
                Application::GetDefDialogParent()->Enable( sal_False );
        /*
            if ( bDispatcherLocked )
                SFX_APP()->LockDispatcher( sal_True );
        */
            if ( nWaitCount )
            {
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                for ( sal_uInt16 n = 0; n < nWaitCount; n++ )
                    pIDEShell->GetViewFrame()->GetWindow().EnterWait();
            }
        }
    }
    return nRet;
}

ModulWindow* BasicIDEShell::ShowActiveModuleWindow( StarBASIC* pBasic )
{
    SetCurLib( ScriptDocument::getApplicationScriptDocument(), String(), false );

    SbModule* pActiveModule = StarBASIC::GetActiveModule();
    SbClassModuleObject* pClassModuleObject = PTR_CAST(SbClassModuleObject,pActiveModule);
    if( pClassModuleObject != NULL )
        pActiveModule = pClassModuleObject->getClassModule();

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
                ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
                String aLibName = pLib->GetName();
                pWin = FindBasWin( aDocument, aLibName, pActiveModule->GetName(), sal_True );
                DBG_ASSERT( pWin, "Error/Step-Hdl: Fenster wurde nicht erzeugt/gefunden!" );
                SetCurLib( aDocument, aLibName );
                SetCurWindow( pWin, sal_True );
            }
        }
        BasicManager* pBasicMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasicMgr )
            StartListening( *pBasicMgr, sal_True /* Nur einmal anmelden */ );
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

Reference< XModel > BasicIDEShell::GetCurrentDocument() const
{
    Reference< XModel > xDocument;
    if ( pCurWin && pCurWin->GetDocument().isDocument() )
        xDocument = pCurWin->GetDocument().getDocument();
    return xDocument;
}

void __EXPORT BasicIDEShell::Activate( sal_Bool bMDI )
{
    SfxViewShell::Activate( bMDI );

    if ( bMDI )
    {
        if( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) )
            ((DialogWindow*)pCurWin)->UpdateBrowser();

        ShowObjectDialog( sal_True, sal_False );
    }
}

void __EXPORT BasicIDEShell::Deactivate( sal_Bool bMDI )
{
    // bMDI sal_True heisst, dass ein anderes MDI aktiviert wurde, bei einem
    // Deactivate durch eine MessageBox ist bMDI FALSE
    if ( bMDI )
    {
        if( pCurWin && pCurWin->IsA( TYPE( DialogWindow ) ) )
        {
            DialogWindow* pXDlgWin = (DialogWindow*)pCurWin;
            pXDlgWin->DisableBrowser();
            if( pXDlgWin->IsModified() )
                BasicIDE::MarkDocumentModified( pXDlgWin->GetDocument() );
        }

        // CanClose pruefen, damit auch beim deaktivieren der BasicIDE geprueft wird,
        // ob in einem Modul der Sourcecode zu gross ist...
        for ( sal_uLong nWin = 0; nWin < aIDEWindowTable.Count(); nWin++ )
        {
            IDEBaseWindow* pWin = aIDEWindowTable.GetObject( nWin );
            if ( /* !pWin->IsSuspended() && */ !pWin->CanClose() )
            {
                if ( m_aCurLibName.Len() && ( pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName ) )
                    SetCurLib( ScriptDocument::getApplicationScriptDocument(), String(), false );
                SetCurWindow( pWin, sal_True );
                break;
            }
        }

        ShowObjectDialog( sal_False, sal_False );
    }
}


IMPL_LINK( BasicIDEShell, AccelSelectHdl, Accelerator*, pAccel )
{
    sal_Bool bDone = sal_True;
    SfxViewFrame* pViewFrame = GetViewFrame();
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( !pDispatcher )
        return sal_False;
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
        default:    bDone = sal_False;
    }
    return bDone;
}

