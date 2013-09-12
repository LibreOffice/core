/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "basidesh.hrc"
#include "helpid.hrc"

#include "baside2.hxx"
#include "baside3.hxx"
#include "basobj.hxx"
#include "docsignature.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "localizationmgr.hxx"
#include "managelang.hxx"

#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <svl/intitem.hxx>
#include <svl/srchitem.hxx>
#include <svl/visitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/msgbox.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

void Shell::ExecuteCurrent( SfxRequest& rReq )
{
    if ( !pCurWin )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_BASICIDE_HIDECURPAGE:
        {
            pCurWin->StoreData();
            RemoveWindow( pCurWin, false );
        }
        break;
        case SID_BASICIDE_RENAMECURRENT:
        {
            pTabBar->StartEditMode( pTabBar->GetCurPageId() );
        }
        break;
        case FID_SEARCH_NOW:
        {
            if (!pCurWin->HasActiveEditor())
                break;
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            SfxItemSet const& rArgs = *rReq.GetArgs();
            // unfortunately I don't know the ID:
            sal_uInt16 nWhich = rArgs.GetWhichByPos( 0 );
            DBG_ASSERT( nWhich, "Wich fuer SearchItem ?" );
            SfxPoolItem const& rItem = rArgs.Get(nWhich);
            DBG_ASSERT(dynamic_cast<SvxSearchItem const*>(&rItem), "no searchitem!");
            SvxSearchItem const& rSearchItem = static_cast<SvxSearchItem const&>(rItem);
            // memorize item because of the adjustments...
            GetExtraData()->SetSearchItem(rSearchItem);
            sal_Int32 nFound = 0;
            bool bCanceled = false;
            if (rSearchItem.GetCommand() == SVX_SEARCHCMD_REPLACE_ALL)
            {
                sal_uInt16 nActModWindows = 0;
                for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
                {
                    BaseWindow* pWin = it->second;
                    if (pWin->HasActiveEditor())
                        nActModWindows++;
                }

                if ( nActModWindows <= 1 || ( !rSearchItem.GetSelection() && QueryBox( pCurWin, WB_YES_NO|WB_DEF_YES, IDEResId(RID_STR_SEARCHALLMODULES).toString() ).Execute() == RET_YES ) )
                {
                    for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
                    {
                        BaseWindow* pWin = it->second;
                        nFound += pWin->StartSearchAndReplace(rSearchItem);
                    }
                }
                else
                    nFound = pCurWin->StartSearchAndReplace(rSearchItem);

                OUString aReplStr(IDE_RESSTR(RID_STR_SEARCHREPLACES));
                aReplStr = aReplStr.replaceAll("XX", OUString::number(nFound));
                InfoBox( pCurWin, aReplStr ).Execute();
            }
            else
            {
                nFound = pCurWin->StartSearchAndReplace(rSearchItem);
                if ( !nFound && !rSearchItem.GetSelection() )
                {
                    // search other modules...
                    bool bChangeCurWindow = false;
                    WindowTableIt it;
                    for (it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
                        if (it->second == pCurWin)
                            break;
                    if (it != aWindowTable.end())
                        ++it;
                    BaseWindow* pWin = it != aWindowTable.end() ? it->second : 0;

                    bool bSearchedFromStart = false;
                    while ( !nFound && !bCanceled && ( pWin || !bSearchedFromStart ) )
                    {
                        if ( !pWin )
                        {
                            SfxViewFrame* pViewFrame = GetViewFrame();
                            SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow( SID_SEARCH_DLG ) : NULL;
                            Window* pParent = pChildWin ? pChildWin->GetWindow() : NULL;
                            QueryBox aQuery(pParent, WB_YES_NO|WB_DEF_YES, IDE_RESSTR(RID_STR_SEARCHFROMSTART));
                            if ( aQuery.Execute() == RET_YES )
                            {
                                it = aWindowTable.begin();
                                if ( it != aWindowTable.end() )
                                    pWin = it->second;
                                bSearchedFromStart = true;
                            }
                            else
                                bCanceled = true;
                        }

                        if (pWin && pWin->HasActiveEditor())
                        {
                            if ( pWin != pCurWin )
                            {
                                if ( pCurWin )
                                    pWin->SetSizePixel( pCurWin->GetSizePixel() );
                                nFound = pWin->StartSearchAndReplace(rSearchItem, true);
                            }
                            if ( nFound )
                            {
                                bChangeCurWindow = true;
                                break;
                            }
                        }
                        if ( pWin && ( pWin != pCurWin ) )
                        {
                            if ( it != aWindowTable.end() )
                                ++it;
                            pWin = it != aWindowTable.end() ? it->second : 0;
                        }
                        else
                            pWin = 0;
                    }
                    if ( !nFound && bSearchedFromStart )
                        nFound = pCurWin->StartSearchAndReplace(rSearchItem, true);
                    if ( bChangeCurWindow )
                        SetCurWindow( pWin, true );
                }
                if ( !nFound && !bCanceled )
                    InfoBox( pCurWin, IDEResId(RID_STR_SEARCHNOTFOUND).toString() ).Execute();
            }

            rReq.Done();
        }
        break;
        case SID_UNDO:
        case SID_REDO:
            if ( GetUndoManager() && pCurWin->AllowUndo() )
                GetViewFrame()->ExecuteSlot( rReq );
            break;
        default:
            pCurWin->ExecuteCommand( rReq );
    }
}

//  no matter who's at the top, influence on the shell:
void Shell::ExecuteGlobal( SfxRequest& rReq )
{
    sal_uInt16 nSlot = rReq.GetSlot();
    switch ( nSlot )
    {
        case SID_BASICSTOP:
        {
            // maybe do not simply stop if on breakpoint!
            if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin))
                pMCurWin->BasicStop();
            StopBasic();
        }
        break;

        case SID_SAVEDOC:
        {
            if ( pCurWin )
            {
                // rewrite date into the BASIC
                StoreAllWindowData();

                // document basic
                ScriptDocument aDocument( pCurWin->GetDocument() );
                if ( aDocument.isDocument() )
                {
                    uno::Reference< task::XStatusIndicator > xStatusIndicator;

                    SFX_REQUEST_ARG( rReq, pStatusIndicatorItem, SfxUnoAnyItem,
                        SID_PROGRESS_STATUSBAR_CONTROL, false );
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

                if (SfxBindings* pBindings = GetBindingsPtr())
                {
                    pBindings->Invalidate( SID_DOC_MODIFIED );
                    pBindings->Invalidate( SID_SAVEDOC );
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
                Organize( rTabId.GetValue() );
            }
            else
                Organize( 0 );
        }
        break;
        case SID_BASICIDE_CHOOSEMACRO:
        {
            ChooseMacro( NULL, false, OUString() );
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

            StartListening( *pBasMgr, true /* log on only once */ );
            OUString aLibName( rInfo.GetLib() );
            if ( aLibName.isEmpty() )
                aLibName = "Standard" ;
            StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
            if ( !pBasic )
            {
                // load module and dialog library (if not loaded)
                aDocument.loadLibraryIfExists( E_SCRIPTS, aLibName );
                aDocument.loadLibraryIfExists( E_DIALOGS, aLibName );

                // get Basic
                pBasic = pBasMgr->GetLib( aLibName );
            }
            DBG_ASSERT( pBasic, "No Basic!" );

            SetCurLib( aDocument, aLibName );

            if ( pBasic && rReq.GetSlot() == SID_BASICIDE_CREATEMACRO )
            {
                SbModule* pModule = pBasic->FindModule( rInfo.GetModule() );
                if ( !pModule )
                {
                    if ( !rInfo.GetModule().isEmpty() || !pBasic->GetModules()->Count() )
                    {
                        OUString aModName = rInfo.GetModule();

                        OUString sModuleCode;
                        if ( aDocument.createModule( aLibName, aModName, false, sModuleCode ) )
                            pModule = pBasic->FindModule( aModName );
                    }
                    else
                        pModule = (SbModule*) pBasic->GetModules()->Get(0);
                }
                DBG_ASSERT( pModule, "Kein Modul!" );
                if ( pModule && !pModule->GetMethods()->Find( rInfo.GetMethod(), SbxCLASS_METHOD ) )
                    CreateMacro( pModule, rInfo.GetMethod() );
            }
            SfxViewFrame* pViewFrame = GetViewFrame();
            if ( pViewFrame )
                pViewFrame->ToTop();
            ModulWindow* pWin = FindBasWin( aDocument, aLibName, rInfo.GetModule(), true );
            DBG_ASSERT( pWin, "Edit/Create Macro: Fenster wurde nicht erzeugt/gefunden!" );
            SetCurWindow( pWin, true );
            pWin->EditMacro( rInfo.GetMethod() );
        }
        break;

        case SID_BASICIDE_OBJCAT:
            // toggling object catalog
            aObjectCatalog.Show(!aObjectCatalog.IsVisible());
            if (pLayout)
                pLayout->ArrangeWindows();
            // refresh the button state
            if (SfxBindings* pBindings = GetBindingsPtr())
                pBindings->Invalidate(SID_BASICIDE_OBJCAT);
            break;

        case SID_BASICIDE_NAMECHANGEDONTAB:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxUInt16Item &rTabId = (const SfxUInt16Item&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID );
            const SfxStringItem &rModName = (const SfxStringItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_MODULENAME );
            if ( aWindowTable.find( rTabId.GetValue() ) !=  aWindowTable.end() )
            {
                BaseWindow* pWin = aWindowTable[ rTabId.GetValue() ];
                OUString aNewName( rModName.GetValue() );
                OUString aOldName( pWin->GetName() );
                if ( aNewName != aOldName )
                {
                    bool bRenameOk = false;
                    if (ModulWindow* pModWin = dynamic_cast<ModulWindow*>(pWin))
                    {
                        OUString aLibName = pModWin->GetLibName();
                        ScriptDocument aDocument( pWin->GetDocument() );

                        if (RenameModule(pModWin, aDocument, aLibName, aOldName, aNewName))
                        {
                            bRenameOk = true;
                            // Because we listen for container events for script
                            // modules, rename will delete the 'old' window
                            // pWin has been invalidated, restore now
                            pWin = FindBasWin( aDocument, aLibName, aNewName, true );
                        }

                    }
                    else if (DialogWindow* pDlgWin = dynamic_cast<DialogWindow*>(pWin))
                    {
                        bRenameOk = pDlgWin->RenameDialog( aNewName );
                    }
                    if ( bRenameOk )
                    {
                        MarkDocumentModified( pWin->GetDocument() );
                    }
                    else
                    {
                        // set old name in TabWriter
                        sal_uInt16 nId = GetWindowId( pWin );
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
            ModulWindow* pWin = FindBasWin( aDocument, rInfo.GetLib(), rInfo.GetModule(), false, true );
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
            for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
            {
                BaseWindow* pWin = it->second;
                if (!pWin->IsSuspended() && dynamic_cast<ModulWindow*>(pWin))
                {
                    if ( rReq.GetSlot() == SID_BASICIDE_STOREALLMODULESOURCES )
                        pWin->StoreData();
                    else
                        pWin->UpdateData();
                }
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
            OUString aLibName( rLibNameItem.GetValue() );

            if ( nSlot == SID_BASICIDE_LIBSELECTED )
            {
                // load module and dialog library (if not loaded)
                aDocument.loadLibraryIfExists( E_SCRIPTS, aLibName );
                aDocument.loadLibraryIfExists( E_DIALOGS, aLibName );

                // check password, if library is password protected and not verified
                bool bOK = true;
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
                    {
                        OUString aPassword;
                        bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
                    }
                }

                if ( bOK )
                {
                    SetCurLib( aDocument, aLibName, true, false );
                }
                else
                {
                    // adjust old value...
                    if (SfxBindings* pBindings = GetBindingsPtr())
                        pBindings->Invalidate(SID_BASICIDE_LIBSELECTOR, true, false);
                }
            }
            else if ( nSlot == SID_BASICIDE_LIBREMOVED )
            {
                if ( m_aCurLibName.isEmpty() || ( aDocument == m_aCurDocument && aLibName == m_aCurLibName ) )
                {
                    RemoveWindows( aDocument, aLibName, true );
                    if ( aDocument == m_aCurDocument && aLibName == m_aCurLibName )
                    {
                        m_aCurDocument = ScriptDocument::getApplicationScriptDocument();
                        m_aCurLibName = String();
                        // no UpdateWindows!
                        if (SfxBindings* pBindings = GetBindingsPtr())
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
            SetCurWindow( pWin, true );
        }
        break;
        case SID_BASICIDE_NEWDIALOG:
        {
            DialogWindow* pWin = CreateDlgWin( m_aCurDocument, m_aCurLibName, String() );
            DBG_ASSERT( pWin, "New Module: Konnte Fenster nicht erzeugen!" );
            SetCurWindow( pWin, true );
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
            OUString aLibName( rSbxItem.GetLibName() );
            OUString aName( rSbxItem.GetName() );
            if ( m_aCurLibName.isEmpty() || ( aDocument == m_aCurDocument && aLibName == m_aCurLibName ) )
            {
                if ( rSbxItem.GetType() == TYPE_MODULE )
                    FindBasWin( aDocument, aLibName, aName, true );
                else if ( rSbxItem.GetType() == TYPE_DIALOG )
                    FindDlgWin( aDocument, aLibName, aName, true );
            }
        }
        break;
        case SID_BASICIDE_SBXDELETED:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            ScriptDocument aDocument( rSbxItem.GetDocument() );
            BaseWindow* pWin = FindWindow( aDocument, rSbxItem.GetLibName(), rSbxItem.GetName(), rSbxItem.GetType(), true );
            if ( pWin )
                RemoveWindow( pWin, true );
        }
        break;
        case SID_BASICIDE_SHOWSBX:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = (const SbxItem&)rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX );
            ScriptDocument aDocument( rSbxItem.GetDocument() );
            OUString aLibName( rSbxItem.GetLibName() );
            OUString aName( rSbxItem.GetName() );
            SetCurLib( aDocument, aLibName );
            BaseWindow* pWin = 0;
            if ( rSbxItem.GetType() == TYPE_DIALOG )
            {
                pWin = FindDlgWin( aDocument, aLibName, aName, true );
            }
            else if ( rSbxItem.GetType() == TYPE_MODULE )
            {
                pWin = FindBasWin( aDocument, aLibName, aName, true );
            }
            else if ( rSbxItem.GetType() == TYPE_METHOD )
            {
                pWin = FindBasWin( aDocument, aLibName, aName, true );
                static_cast<ModulWindow*>(pWin)->EditMacro( rSbxItem.GetMethodName() );
            }
            DBG_ASSERT( pWin, "Fenster wurde nicht erzeugt!" );
            SetCurWindow( pWin, true );
            pTabBar->MakeVisible( pTabBar->GetCurPageId() );
        }
        break;
        case SID_BASICIDE_SHOWWINDOW:
        {
            ::std::auto_ptr< ScriptDocument > pDocument;

            SFX_REQUEST_ARG( rReq, pDocumentItem, SfxStringItem, SID_BASICIDE_ARG_DOCUMENT, false );
            if ( pDocumentItem )
            {
                OUString sDocumentCaption = pDocumentItem->GetValue();
                if ( !sDocumentCaption.isEmpty() )
                    pDocument.reset( new ScriptDocument( ScriptDocument::getDocumentWithURLOrCaption( sDocumentCaption ) ) );
            }

            SFX_REQUEST_ARG( rReq, pDocModelItem, SfxUsrAnyItem, SID_BASICIDE_ARG_DOCUMENT_MODEL, false );
            if ( !pDocument.get() && pDocModelItem )
            {
                uno::Reference< frame::XModel > xModel( pDocModelItem->GetValue(), UNO_QUERY );
                if ( xModel.is() )
                    pDocument.reset( new ScriptDocument( xModel ) );
            }

            if ( !pDocument.get() )
                break;

            SFX_REQUEST_ARG( rReq, pLibNameItem, SfxStringItem, SID_BASICIDE_ARG_LIBNAME, false );
            if ( !pLibNameItem )
                break;

            OUString aLibName( pLibNameItem->GetValue() );
            pDocument->loadLibraryIfExists( E_SCRIPTS, aLibName );
            SetCurLib( *pDocument, aLibName );
            SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_BASICIDE_ARG_NAME, false );
            if ( pNameItem )
            {
                OUString aName( pNameItem->GetValue() );
                OUString aModType( "Module" );
                OUString aDlgType( "Dialog" );
                OUString aType( aModType );
                SFX_REQUEST_ARG( rReq, pTypeItem, SfxStringItem, SID_BASICIDE_ARG_TYPE, false );
                if ( pTypeItem )
                    aType = pTypeItem->GetValue();

                BaseWindow* pWin = 0;
                if ( aType == aModType )
                    pWin = FindBasWin( *pDocument, aLibName, aName, false );
                else if ( aType == aDlgType )
                    pWin = FindDlgWin( *pDocument, aLibName, aName, false );

                if ( pWin )
                {
                    SetCurWindow( pWin, true );
                    if ( pTabBar )
                        pTabBar->MakeVisible( pTabBar->GetCurPageId() );

                    if (ModulWindow* pModWin = dynamic_cast<ModulWindow*>(pWin))
                    {
                        SFX_REQUEST_ARG( rReq, pLineItem, SfxUInt32Item, SID_BASICIDE_ARG_LINE, false );
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
                                        pTextView->ShowCursor( false, true );
                                        pModWin->GetEditVScrollBar().SetThumbPos( pTextView->GetStartDocPos().Y() );
                                    }
                                    sal_uInt16 nCol1 = 0, nCol2 = 0;
                                    SFX_REQUEST_ARG( rReq, pCol1Item, SfxUInt16Item, SID_BASICIDE_ARG_COLUMN1, false );
                                    if ( pCol1Item )
                                    {
                                        nCol1 = pCol1Item->GetValue();
                                        if ( nCol1 > 0 )
                                            --nCol1;
                                        nCol2 = nCol1;
                                    }
                                    SFX_REQUEST_ARG( rReq, pCol2Item, SfxUInt16Item, SID_BASICIDE_ARG_COLUMN2, false );
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
            ManageLanguageDialog aDlg(pCurWin, m_pCurLocalizationMgr);
            aDlg.Execute();
            rReq.Done();
        }
        break;

        default:
            if (pLayout)
                pLayout->ExecuteGlobal(rReq);
            if (pCurWin)
                pCurWin->ExecuteGlobal(rReq);
            break;
    }
}

void Shell::GetState(SfxItemSet &rSet)
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
                bool bDisable = false;

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
                    bDisable = true;
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
                    DocumentSignature aSignature( pCurWin->GetDocument() );
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
            case SID_BASICIDE_OBJCAT:
                if (pLayout)
                    rSet.Put(SfxBoolItem(nWh, aObjectCatalog.IsVisible()));
                else
                    rSet.Put(SfxVisibilityItem(nWh, false));
                break;
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
                if (!dynamic_cast<ModulWindow*>(pCurWin))
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
                if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin))
                {
                    if (StarBASIC::IsRunning() && !pMCurWin->GetBasicStatus().bIsInReschedule)
                        rSet.DisableItem(nWh);
                }
                else
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICCOMPILE:
            {
                if (StarBASIC::IsRunning() || !dynamic_cast<ModulWindow*>(pCurWin))
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_BASICSTOP:
            {
                // stop is always possible when some Basic is running...
                if (!StarBASIC::IsRunning())
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_CHOOSE_CONTROLS:
            case SID_DIALOG_TESTMODE:
            case SID_INSERT_FORM_RADIO:
            case SID_INSERT_FORM_CHECK:
            case SID_INSERT_FORM_LIST:
            case SID_INSERT_FORM_COMBO:
            case SID_INSERT_FORM_VSCROLL:
            case SID_INSERT_FORM_HSCROLL:
            case SID_INSERT_FORM_SPIN:
            {
                if (!dynamic_cast<DialogWindow*>(pCurWin))
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
                OUString aName;
                if ( !m_aCurLibName.isEmpty() )
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
                OUString aSelected = GetSelectionText(true);
                SvxSearchItem& rItem = GetExtraData()->GetSearchItem();
                rItem.SetSearchString( aSelected );
                rSet.Put( rItem );
            }
            break;
            case SID_BASICIDE_STAT_DATE:
            {
                OUString aDate( "Datum?!" );
                SfxStringItem aItem( SID_BASICIDE_STAT_DATE, aDate );
                rSet.Put( aItem );
            }
            break;
            case SID_DOC_MODIFIED:
            {
                bool bModified = false;

                if ( pCurWin )
                {
                    if ( pCurWin->IsModified() )
                        bModified = true;
                    else
                    {
                        ScriptDocument aDocument( pCurWin->GetDocument() );
                        bModified = aDocument.isDocument() ? aDocument.isDocumentModified() : IsAppBasicModified();
                    }
                }

                SfxBoolItem aItem(SID_DOC_MODIFIED, bModified);
                rSet.Put( aItem );
            }
            break;
            case SID_BASICIDE_STAT_TITLE:
            {
                if ( pCurWin )
                {
                    OUString aTitle = pCurWin->CreateQualifiedName();
                    SfxStringItem aItem( SID_BASICIDE_STAT_TITLE, aTitle );
                    rSet.Put( aItem );
                }
            }
            break;
            // are interpreted by the controller:
            case SID_ATTR_SIZE:
            case SID_ATTR_INSERT:
            break;
            case SID_UNDO:
            case SID_REDO:
            {
                if( GetUndoManager() )  // recursive GetState else
                    GetViewFrame()->GetSlotState( nWh, NULL, &rSet );
            }
            break;
            case SID_BASICIDE_CURRENT_LANG:
            {
                if( (pCurWin && pCurWin->IsReadOnly()) || GetCurLibName().isEmpty() )
                    rSet.DisableItem( nWh );
                else
                {
                    OUString aItemStr;
                    boost::shared_ptr<LocalizationMgr> pCurMgr(GetCurLocalizationMgr());
                    if ( pCurMgr->isLibraryLocalized() )
                    {
                        Sequence< lang::Locale > aLocaleSeq = pCurMgr->getStringResourceManager()->getLocales();
                        const lang::Locale* pLocale = aLocaleSeq.getConstArray();
                        sal_Int32 i, nCount = aLocaleSeq.getLength();

                        // Force different results for any combination of locales and default locale
                        OUString aLangStr;
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
                if( (pCurWin && pCurWin->IsReadOnly()) || GetCurLibName().isEmpty() )
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_GOTOLINE:
            {
                // if this is not a module window hide the
                // setting, doesn't make sense for example if the
                // dialog editor is open
                if (pCurWin && !dynamic_cast<ModulWindow*>(pCurWin))
                {
                    rSet.DisableItem( nWh );
                    rSet.Put(SfxVisibilityItem(nWh, false));
                }
                break;
            }
            default:
                if (pLayout)
                    pLayout->GetState(rSet, nWh);
        }
    }
    if ( pCurWin )
        pCurWin->GetState( rSet );
}

sal_Bool Shell::HasUIFeature( sal_uInt32 nFeature )
{
    bool bResult = false;

    if ( (nFeature & BASICIDE_UI_FEATURE_SHOW_BROWSER) == BASICIDE_UI_FEATURE_SHOW_BROWSER )
    {
        // fade out (in) property browser in module (dialog) windows
        if (dynamic_cast<DialogWindow*>(pCurWin) && !pCurWin->IsReadOnly())
            bResult = true;
    }

    return bResult;
}

void Shell::SetCurWindow( BaseWindow* pNewWin, bool bUpdateTabBar, bool bRememberAsCurrent )
{
    if ( pNewWin != pCurWin )
    {
        pCurWin = pNewWin;
        if (pLayout)
            pLayout->Deactivating();
        if (pCurWin)
        {
            if (pCurWin->GetType() == TYPE_MODULE)
                pLayout = pModulLayout.get();
            else
                pLayout = pDialogLayout.get();
            AdjustPosSizePixel(Point(0, 0), GetViewFrame()->GetWindow().GetOutputSizePixel());
            pLayout->Activating(*pCurWin);
            GetViewFrame()->GetWindow().SetHelpId(pCurWin->GetHid());
            if (bRememberAsCurrent)
                pCurWin->InsertLibInfo();
            if (GetViewFrame()->GetWindow().IsVisible()) // SFX will do it later otherwise
                pCurWin->Show();
            pCurWin->Init();
            if (!GetExtraData()->ShellInCriticalSection())
            {
                Window* pFrameWindow = &GetViewFrame()->GetWindow();
                Window* pFocusWindow = Application::GetFocusWindow();
                while ( pFocusWindow && ( pFocusWindow != pFrameWindow ) )
                    pFocusWindow = pFocusWindow->GetParent();
                if ( pFocusWindow ) // Focus in BasicIDE
                    pNewWin->GrabFocus();
            }
        }
        else
        {
            SetWindow(pLayout);
            pLayout = 0;
        }
        if ( bUpdateTabBar )
        {
            sal_uLong nKey = GetWindowId( pCurWin );
            if ( pCurWin && ( pTabBar->GetPagePos( (sal_uInt16)nKey ) == TAB_PAGE_NOTFOUND ) )
                pTabBar->InsertPage( (sal_uInt16)nKey, pCurWin->GetTitle() );   // has just been faded in
            pTabBar->SetCurPageId( (sal_uInt16)nKey );
        }
        if ( pCurWin && pCurWin->IsSuspended() )    // if the window is shown in the case of an error...
            pCurWin->SetStatus( pCurWin->GetStatus() & ~BASWIN_SUSPENDED );
        if ( pCurWin )
        {
            SetWindow( pCurWin );
            if ( pCurWin->GetDocument().isDocument() )
                SfxObjectShell::SetCurrentComponent( pCurWin->GetDocument().getDocument() );
        }
        else if (pLayout)
        {
            SetWindow(pLayout);
            GetViewFrame()->GetWindow().SetHelpId( HID_BASICIDE_MODULWINDOW );
            SfxObjectShell::SetCurrentComponent(0);
        }
        aObjectCatalog.SetCurrentEntry(pCurWin);
        SetUndoManager( pCurWin ? pCurWin->GetUndoManager() : 0 );
        InvalidateBasicIDESlots();
        EnableScrollbars(pCurWin != 0);

        if ( m_pCurLocalizationMgr )
            m_pCurLocalizationMgr->handleTranslationbar();

        ManageToolbars();

        // fade out (in) property browser in module (dialog) windows
        UIFeatureChanged();
    }
}

void Shell::ManageToolbars()
{
    static OUString aLayoutManagerName( "LayoutManager" );
    static OUString aMacroBarResName( "private:resource/toolbar/macrobar" );
    static OUString aDialogBarResName( "private:resource/toolbar/dialogbar" );
    static OUString aInsertControlsBarResName( "private:resource/toolbar/insertcontrolsbar" );
    static OUString aFormControlsBarResName( "private:resource/toolbar/formcontrolsbar" );
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
            if (dynamic_cast<DialogWindow*>(pCurWin))
            {
                xLayoutManager->destroyElement( aMacroBarResName );

                xLayoutManager->requestElement( aDialogBarResName );
                xLayoutManager->requestElement( aInsertControlsBarResName );
                xLayoutManager->requestElement( aFormControlsBarResName );
            }
            else
            {
                xLayoutManager->destroyElement( aDialogBarResName );
                xLayoutManager->destroyElement( aInsertControlsBarResName );
                xLayoutManager->destroyElement( aFormControlsBarResName );

                xLayoutManager->requestElement( aMacroBarResName );
            }
            xLayoutManager->unlock();
        }
    }
}

BaseWindow* Shell::FindApplicationWindow()
{
    return FindWindow( ScriptDocument::getApplicationScriptDocument() );
}

BaseWindow* Shell::FindWindow(
    ScriptDocument const& rDocument,
    OUString const& rLibName, OUString const& rName,
    ItemType eType, bool bFindSuspended
)
{
    for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
    {
        BaseWindow* const pWin = it->second;
        if (pWin->Is(rDocument, rLibName, rName, eType, bFindSuspended))
            return pWin;
    }
    return 0;
}

long Shell::CallBasicErrorHdl( StarBASIC* pBasic )
{
    long nRet = 0;
    ModulWindow* pModWin = ShowActiveModuleWindow( pBasic );
    if ( pModWin )
        nRet = pModWin->BasicErrorHdl( pBasic );
    return nRet;
}

long Shell::CallBasicBreakHdl( StarBASIC* pBasic )
{
    long nRet = 0;
    ModulWindow* pModWin = ShowActiveModuleWindow( pBasic );
    if ( pModWin )
    {
        bool bAppWindowDisabled, bDispatcherLocked;
        sal_uInt16 nWaitCount;
        SfxUInt16Item *pSWActionCount, *pSWLockViewCount;
        BasicStopped( &bAppWindowDisabled, &bDispatcherLocked,
                                &nWaitCount, &pSWActionCount, &pSWLockViewCount );

        nRet = pModWin->BasicBreakHdl( pBasic );

        if ( StarBASIC::IsRunning() )   // if cancelled...
        {
            if ( bAppWindowDisabled )
                Application::GetDefDialogParent()->Enable(false);

            if ( nWaitCount )
            {
                Shell* pShell = GetShell();
                for ( sal_uInt16 n = 0; n < nWaitCount; n++ )
                    pShell->GetViewFrame()->GetWindow().EnterWait();
            }
        }
    }
    return nRet;
}

ModulWindow* Shell::ShowActiveModuleWindow( StarBASIC* pBasic )
{
    SetCurLib( ScriptDocument::getApplicationScriptDocument(), OUString(), false );

    SbModule* pActiveModule = StarBASIC::GetActiveModule();
    if (SbClassModuleObject* pCMO = dynamic_cast<SbClassModuleObject*>(pActiveModule))
        pActiveModule = pCMO->getClassModule();

    DBG_ASSERT( pActiveModule, "Kein aktives Modul im ErrorHdl?!" );
    if ( pActiveModule )
    {
        ModulWindow* pWin = 0;
        SbxObject* pParent = pActiveModule->GetParent();
        if (StarBASIC* pLib = dynamic_cast<StarBASIC*>(pParent))
        {
            if (BasicManager* pBasMgr = FindBasicManager(pLib))
            {
                ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
                OUString aLibName = pLib->GetName();
                pWin = FindBasWin( aDocument, aLibName, pActiveModule->GetName(), true );
                DBG_ASSERT( pWin, "Error/Step-Hdl: Fenster wurde nicht erzeugt/gefunden!" );
                SetCurLib( aDocument, aLibName );
                SetCurWindow( pWin, true );
            }
        }
        else
            DBG_ASSERT(false, "No BASIC!");
        if (BasicManager* pBasicMgr = FindBasicManager(pBasic))
            StartListening( *pBasicMgr, true /* log on only once */ );
        return pWin;
    }
    return 0;
}

void Shell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    // not if iconified because the whole text would be displaced then at restore
    if ( GetViewFrame()->GetWindow().GetOutputSizePixel().Height() == 0 )
        return;

    Size aSz( rSize );
    Size aScrollBarBoxSz( aScrollBarBox.GetSizePixel() );
    aSz.Height() -= aScrollBarBoxSz.Height();

    Size aOutSz( aSz );
    aSz.Width() -= aScrollBarBoxSz.Width();
    aScrollBarBox.SetPosPixel( Point( rSize.Width() - aScrollBarBoxSz.Width(), rSize.Height() - aScrollBarBoxSz.Height() ) );
    aVScrollBar.SetPosSizePixel( Point( rPos.X()+aSz.Width(), rPos.Y() ), Size( aScrollBarBoxSz.Width(), aSz.Height() ) );
    if ( bTabBarSplitted )
    {
        // SplitSize is 0 at a resize!
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

    if (pLayout)
        pLayout->SetPosSizePixel(rPos, dynamic_cast<DialogWindow*>(pCurWin) ? aSz : aOutSz);
}

Reference< XModel > Shell::GetCurrentDocument() const
{
    Reference< XModel > xDocument;
    if ( pCurWin && pCurWin->GetDocument().isDocument() )
        xDocument = pCurWin->GetDocument().getDocument();
    return xDocument;
}

void Shell::Activate( sal_Bool bMDI )
{
    SfxViewShell::Activate( bMDI );

    if ( bMDI )
    {
        if (DialogWindow* pDCurWin = dynamic_cast<DialogWindow*>(pCurWin))
            pDCurWin->UpdateBrowser();
    }
}

void Shell::Deactivate( sal_Bool bMDI )
{
    // bMDI == true means that another MDI has been activated; in case of a
    // deactivate due to a MessageBox bMDI is false
    if ( bMDI )
    {
        if (DialogWindow* pXDlgWin = dynamic_cast<DialogWindow*>(pCurWin))
        {
            pXDlgWin->DisableBrowser();
            if( pXDlgWin->IsModified() )
                MarkDocumentModified( pXDlgWin->GetDocument() );
        }

        // test CanClose to also test during deactivating the BasicIDE whether
        // the sourcecode is too large in one of the modules...
        for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
        {
            BaseWindow* pWin = it->second;
            if ( /* !pWin->IsSuspended() && */ !pWin->CanClose() )
            {
                if ( !m_aCurLibName.isEmpty() && ( pWin->IsDocument( m_aCurDocument ) || pWin->GetLibName() != m_aCurLibName ) )
                    SetCurLib( ScriptDocument::getApplicationScriptDocument(), OUString(), false );
                SetCurWindow( pWin, true );
                break;
            }
        }
    }
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
