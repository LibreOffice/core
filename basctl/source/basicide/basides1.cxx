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

#include <memory>
#include "basidesh.hrc"
#include "helpid.hrc"

#include "baside2.hxx"
#include "baside3.hxx"
#include "docsignature.hxx"
#include "iderdll.hxx"
#include "iderdll2.hxx"
#include "localizationmgr.hxx"
#include "managelang.hxx"

#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <sfx2/childwin.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/dinfdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
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
            DBG_ASSERT( nWhich, "Which for SearchItem?" );
            SfxPoolItem const& rItem = rArgs.Get(nWhich);
            DBG_ASSERT(dynamic_cast<SvxSearchItem const*>(&rItem), "no searchitem!");
            SvxSearchItem const& rSearchItem = static_cast<SvxSearchItem const&>(rItem);
            // memorize item because of the adjustments...
            GetExtraData()->SetSearchItem(rSearchItem);
            sal_Int32 nFound = 0;
            if (rSearchItem.GetCommand() == SvxSearchCmd::REPLACE_ALL)
            {
                sal_uInt16 nActModWindows = 0;
                for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
                {
                    BaseWindow* pWin = it->second;
                    if (pWin->HasActiveEditor())
                        nActModWindows++;
                }

                if ( nActModWindows <= 1 || ( !rSearchItem.GetSelection() && ScopedVclPtrInstance<QueryBox>(pCurWin, WB_YES_NO|WB_DEF_YES, IDEResId(RID_STR_SEARCHALLMODULES))->Execute() == RET_YES ) )
                {
                    for (WindowTableIt it = aWindowTable.begin(); it != aWindowTable.end(); ++it)
                    {
                        BaseWindow* pWin = it->second;
                        nFound += pWin->StartSearchAndReplace(rSearchItem);
                    }
                }
                else
                    nFound = pCurWin->StartSearchAndReplace(rSearchItem);

                OUString aReplStr(IDEResId(RID_STR_SEARCHREPLACES));
                aReplStr = aReplStr.replaceAll("XX", OUString::number(nFound));
                ScopedVclPtrInstance<InfoBox>(pCurWin, aReplStr)->Execute();
            }
            else
            {
                bool bCanceled = false;
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
                    BaseWindow* pWin = it != aWindowTable.end() ? it->second.get() : nullptr;

                    bool bSearchedFromStart = false;
                    while ( !nFound && !bCanceled && ( pWin || !bSearchedFromStart ) )
                    {
                        if ( !pWin )
                        {
                            SfxViewFrame* pViewFrame = GetViewFrame();
                            SfxChildWindow* pChildWin = pViewFrame ? pViewFrame->GetChildWindow( SID_SEARCH_DLG ) : nullptr;
                            vcl::Window* pParent = pChildWin ? pChildWin->GetWindow() : nullptr;
                            ScopedVclPtrInstance< QueryBox > aQuery(pParent, WB_YES_NO|WB_DEF_YES, IDEResId(RID_STR_SEARCHFROMSTART));
                            if ( aQuery->Execute() == RET_YES )
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
                            pWin = it != aWindowTable.end() ? it->second.get() : nullptr;
                        }
                        else
                            pWin = nullptr;
                    }
                    if ( !nFound && bSearchedFromStart )
                        nFound = pCurWin->StartSearchAndReplace(rSearchItem, true);
                    if ( bChangeCurWindow )
                        SetCurWindow( pWin, true );
                }
                if ( !nFound && !bCanceled )
                    ScopedVclPtrInstance<InfoBox>(pCurWin, IDEResId(RID_STR_SEARCHNOTFOUND))->Execute();
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
            if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin.get()))
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

                    const SfxUnoAnyItem* pStatusIndicatorItem = rReq.GetArg<SfxUnoAnyItem>(SID_PROGRESS_STATUSBAR_CONTROL);
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
                            rReq.AppendItem( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, uno::Any( xStatusIndicator ) ) );
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
                const SfxUInt16Item &rTabId = static_cast<const SfxUInt16Item&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID ));
                Organize( rTabId.GetValue() );
            }
            else
                Organize( 0 );
        }
        break;
        case SID_BASICIDE_CHOOSEMACRO:
        {
            ChooseMacro( nullptr, false, OUString() );
        }
        break;
        case SID_BASICIDE_CREATEMACRO:
        case SID_BASICIDE_EDITMACRO:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxMacroInfoItem& rInfo = static_cast<const SfxMacroInfoItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO ));
            BasicManager* pBasMgr = const_cast<BasicManager*>(rInfo.GetBasicManager());
            DBG_ASSERT( pBasMgr, "Nothing selected in basic tree?" );

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
                    if ( !rInfo.GetModule().isEmpty() || pBasic->GetModules().empty() )
                    {
                        const OUString& aModName = rInfo.GetModule();

                        OUString sModuleCode;
                        if ( aDocument.createModule( aLibName, aModName, false, sModuleCode ) )
                            pModule = pBasic->FindModule( aModName );
                    }
                    else
                        pModule = pBasic->GetModules().front().get();
                }
                DBG_ASSERT( pModule, "No Module!" );
                if ( pModule && !pModule->GetMethods()->Find( rInfo.GetMethod(), SbxClassType::Method ) )
                    CreateMacro( pModule, rInfo.GetMethod() );
            }
            SfxViewFrame* pViewFrame = GetViewFrame();
            if ( pViewFrame )
                pViewFrame->ToTop();
            VclPtr<ModulWindow> pWin = FindBasWin( aDocument, aLibName, rInfo.GetModule(), true );
            DBG_ASSERT( pWin, "Edit/Create Macro: Window was not created/found!" );
            SetCurWindow( pWin, true );
            pWin->EditMacro( rInfo.GetMethod() );
        }
        break;

        case SID_BASICIDE_OBJCAT:
            // toggling object catalog
            aObjectCatalog->Show(!aObjectCatalog->IsVisible());
            if (pLayout)
                pLayout->ArrangeWindows();
            // refresh the button state
            if (SfxBindings* pBindings = GetBindingsPtr())
                pBindings->Invalidate(SID_BASICIDE_OBJCAT);
            break;

        case SID_BASICIDE_NAMECHANGEDONTAB:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SfxUInt16Item &rTabId = static_cast<const SfxUInt16Item&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_TABID ));
            const SfxStringItem &rModName = static_cast<const SfxStringItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_MODULENAME ));
            if ( aWindowTable.find( rTabId.GetValue() ) !=  aWindowTable.end() )
            {
                VclPtr<BaseWindow> pWin = aWindowTable[ rTabId.GetValue() ];
                const OUString& aNewName( rModName.GetValue() );
                OUString aOldName( pWin->GetName() );
                if ( aNewName != aOldName )
                {
                    bool bRenameOk = false;
                    if (ModulWindow* pModWin = dynamic_cast<ModulWindow*>(pWin.get()))
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
                    else if (DialogWindow* pDlgWin = dynamic_cast<DialogWindow*>(pWin.get()))
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
            const SfxMacroInfoItem& rInfo = static_cast<const SfxMacroInfoItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_MACROINFO ));
            BasicManager* pBasMgr = const_cast<BasicManager*>(rInfo.GetBasicManager());
            DBG_ASSERT( pBasMgr, "Store source: No BasMgr?" );
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            VclPtr<ModulWindow> pWin = FindBasWin( aDocument, rInfo.GetLib(), rInfo.GetModule(), false, true );
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
            const SfxUsrAnyItem& rShellItem = static_cast<const SfxUsrAnyItem&>(rReq.GetArgs()->Get( SID_BASICIDE_ARG_DOCUMENT_MODEL ));
            uno::Reference< frame::XModel > xModel( rShellItem.GetValue(), UNO_QUERY );
            ScriptDocument aDocument( xModel.is() ? ScriptDocument( xModel ) : ScriptDocument::getApplicationScriptDocument() );
            const SfxStringItem& rLibNameItem = static_cast<const SfxStringItem&>(rReq.GetArgs()->Get( SID_BASICIDE_ARG_LIBNAME ));
            const OUString& aLibName( rLibNameItem.GetValue() );

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
                        pBindings->Invalidate(SID_BASICIDE_LIBSELECTOR, true);
                }
            }
            else if ( nSlot == SID_BASICIDE_LIBREMOVED )
            {
                if ( m_aCurLibName.isEmpty() || ( aDocument == m_aCurDocument && aLibName == m_aCurLibName ) )
                {
                    RemoveWindows( aDocument, aLibName );
                    if ( aDocument == m_aCurDocument && aLibName == m_aCurLibName )
                    {
                        m_aCurDocument = ScriptDocument::getApplicationScriptDocument();
                        m_aCurLibName.clear();
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
            VclPtr<ModulWindow> pWin = CreateBasWin( m_aCurDocument, m_aCurLibName, OUString() );
            DBG_ASSERT( pWin, "New Module: Could not create window!" );
            SetCurWindow( pWin, true );
        }
        break;
        case SID_BASICIDE_NEWDIALOG:
        {
            VclPtr<DialogWindow> pWin = CreateDlgWin( m_aCurDocument, m_aCurLibName, OUString() );
            DBG_ASSERT( pWin, "New Module: Could not create window!" );
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
            const SbxItem& rSbxItem = static_cast<const SbxItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX ));
            const ScriptDocument& aDocument( rSbxItem.GetDocument() );
            const OUString& aLibName( rSbxItem.GetLibName() );
            const OUString& aName( rSbxItem.GetName() );
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
            const SbxItem& rSbxItem = static_cast<const SbxItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX ));
            const ScriptDocument& aDocument( rSbxItem.GetDocument() );
            VclPtr<BaseWindow> pWin = FindWindow( aDocument, rSbxItem.GetLibName(), rSbxItem.GetName(), rSbxItem.GetType(), true );
            if ( pWin )
                RemoveWindow( pWin, true );
        }
        break;
        case SID_BASICIDE_SHOWSBX:
        {
            DBG_ASSERT( rReq.GetArgs(), "arguments expected" );
            const SbxItem& rSbxItem = static_cast<const SbxItem&>(rReq.GetArgs()->Get(SID_BASICIDE_ARG_SBX ));
            const ScriptDocument& aDocument( rSbxItem.GetDocument() );
            const OUString& aLibName( rSbxItem.GetLibName() );
            const OUString& aName( rSbxItem.GetName() );
            SetCurLib( aDocument, aLibName );
            BaseWindow* pWin = nullptr;
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
            DBG_ASSERT( pWin, "Window was not created!" );
            SetCurWindow( pWin, true );
            pTabBar->MakeVisible( pTabBar->GetCurPageId() );
        }
        break;
        case SID_BASICIDE_SHOWWINDOW:
        {
            std::unique_ptr< ScriptDocument > pDocument;

            const SfxStringItem* pDocumentItem = rReq.GetArg<SfxStringItem>(SID_BASICIDE_ARG_DOCUMENT);
            if ( pDocumentItem )
            {
                OUString sDocumentCaption = pDocumentItem->GetValue();
                if ( !sDocumentCaption.isEmpty() )
                    pDocument.reset( new ScriptDocument( ScriptDocument::getDocumentWithURLOrCaption( sDocumentCaption ) ) );
            }

            const SfxUsrAnyItem* pDocModelItem = rReq.GetArg<SfxUsrAnyItem>(SID_BASICIDE_ARG_DOCUMENT_MODEL);
            if ( !pDocument.get() && pDocModelItem )
            {
                uno::Reference< frame::XModel > xModel( pDocModelItem->GetValue(), UNO_QUERY );
                if ( xModel.is() )
                    pDocument.reset( new ScriptDocument( xModel ) );
            }

            if ( !pDocument.get() )
                break;

            const SfxStringItem* pLibNameItem = rReq.GetArg<SfxStringItem>(SID_BASICIDE_ARG_LIBNAME);
            if ( !pLibNameItem )
                break;

            OUString aLibName( pLibNameItem->GetValue() );
            pDocument->loadLibraryIfExists( E_SCRIPTS, aLibName );
            SetCurLib( *pDocument, aLibName );
            const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_BASICIDE_ARG_NAME);
            if ( pNameItem )
            {
                OUString aName( pNameItem->GetValue() );
                OUString aModType( "Module" );
                OUString aType( aModType );
                const SfxStringItem* pTypeItem = rReq.GetArg<SfxStringItem>(SID_BASICIDE_ARG_TYPE);
                if ( pTypeItem )
                    aType = pTypeItem->GetValue();

                BaseWindow* pWin = nullptr;
                if ( aType == aModType )
                    pWin = FindBasWin( *pDocument, aLibName, aName );
                else if ( aType == "Dialog" )
                    pWin = FindDlgWin( *pDocument, aLibName, aName );

                if ( pWin )
                {
                    SetCurWindow( pWin, true );
                    if ( pTabBar )
                        pTabBar->MakeVisible( pTabBar->GetCurPageId() );

                    if (ModulWindow* pModWin = dynamic_cast<ModulWindow*>(pWin))
                    {
                        const SfxUInt32Item* pLineItem = rReq.GetArg<SfxUInt32Item>(SID_BASICIDE_ARG_LINE);
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
                                        nNewY = std::min( nNewY, nMaxY );
                                        pTextView->Scroll( 0, -( nNewY - nOldY ) );
                                        pTextView->ShowCursor( false );
                                        pModWin->GetEditVScrollBar().SetThumbPos( pTextView->GetStartDocPos().Y() );
                                    }
                                    sal_uInt16 nCol1 = 0, nCol2 = 0;
                                    const SfxUInt16Item* pCol1Item = rReq.GetArg<SfxUInt16Item>(SID_BASICIDE_ARG_COLUMN1);
                                    if ( pCol1Item )
                                    {
                                        nCol1 = pCol1Item->GetValue();
                                        if ( nCol1 > 0 )
                                            --nCol1;
                                        nCol2 = nCol1;
                                    }
                                    const SfxUInt16Item* pCol2Item = rReq.GetArg<SfxUInt16Item>(SID_BASICIDE_ARG_COLUMN2);
                                    if ( pCol2Item )
                                    {
                                        nCol2 = pCol2Item->GetValue();
                                        if ( nCol2 > 0 )
                                            --nCol2;
                                    }
                                    TextSelection aSel( TextPaM( nLine, nCol1 ), TextPaM( nLine, nCol2 ) );
                                    pTextView->SetSelection( aSel );
                                    pTextView->ShowCursor();
                                    vcl::Window* pWindow_ = pTextView->GetWindow();
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
            ScopedVclPtrInstance< ManageLanguageDialog > aDlg(pCurWin, m_pCurLocalizationMgr);
            aDlg->Execute();
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
    for ( sal_uInt16 nWh = aIter.FirstWhich(); nWh != 0; nWh = aIter.NextWhich() )
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
                SignatureState nState = SignatureState::NOSIGNATURES;
                if ( pCurWin )
                {
                    DocumentSignature aSignature( pCurWin->GetDocument() );
                    nState = aSignature.getScriptingSignatureState();
                }
                rSet.Put( SfxUInt16Item( SID_SIGNATURE, static_cast<sal_uInt16>(nState) ) );
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
                    rSet.Put(SfxBoolItem(nWh, aObjectCatalog->IsVisible()));
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
                if (!dynamic_cast<ModulWindow*>(pCurWin.get()))
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
                if (ModulWindow* pMCurWin = dynamic_cast<ModulWindow*>(pCurWin.get()))
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
                if (StarBASIC::IsRunning() || !dynamic_cast<ModulWindow*>(pCurWin.get()))
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
            case SID_INSERT_SELECT:
            case SID_INSERT_PUSHBUTTON:
            case SID_INSERT_RADIOBUTTON:
            case SID_INSERT_CHECKBOX:
            case SID_INSERT_LISTBOX:
            case SID_INSERT_COMBOBOX:
            case SID_INSERT_GROUPBOX:
            case SID_INSERT_EDIT:
            case SID_INSERT_FIXEDTEXT:
            case SID_INSERT_IMAGECONTROL:
            case SID_INSERT_PROGRESSBAR:
            case SID_INSERT_HSCROLLBAR:
            case SID_INSERT_VSCROLLBAR:
            case SID_INSERT_HFIXEDLINE:
            case SID_INSERT_VFIXEDLINE:
            case SID_INSERT_DATEFIELD:
            case SID_INSERT_TIMEFIELD:
            case SID_INSERT_NUMERICFIELD:
            case SID_INSERT_CURRENCYFIELD:
            case SID_INSERT_FORMATTEDFIELD:
            case SID_INSERT_PATTERNFIELD:
            case SID_INSERT_FILECONTROL:
            case SID_INSERT_SPINBUTTON:
            case SID_INSERT_TREECONTROL:
            case SID_INSERT_FORM_RADIO:
            case SID_INSERT_FORM_CHECK:
            case SID_INSERT_FORM_LIST:
            case SID_INSERT_FORM_COMBO:
            case SID_INSERT_FORM_VSCROLL:
            case SID_INSERT_FORM_HSCROLL:
            case SID_INSERT_FORM_SPIN:
            {
                if (!dynamic_cast<DialogWindow*>(pCurWin.get()))
                    rSet.DisableItem( nWh );
            }
            break;
            case SID_SEARCH_OPTIONS:
            {
                SearchOptionFlags nOptions = SearchOptionFlags::NONE;
                if( pCurWin )
                    nOptions = pCurWin->GetSearchOptions();
                rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS, static_cast<sal_uInt16>(nOptions) ) );
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
                SfxStringItem aItem( SID_BASICIDE_STAT_DATE, "Datum?!" );
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
                    GetViewFrame()->GetSlotState( nWh, nullptr, &rSet );
            }
            break;
            case SID_BASICIDE_CURRENT_LANG:
            {
                if( (pCurWin && pCurWin->IsReadOnly()) || GetCurLibName().isEmpty() )
                    rSet.DisableItem( nWh );
                else
                {
                    OUString aItemStr;
                    std::shared_ptr<LocalizationMgr> pCurMgr(GetCurLocalizationMgr());
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

                            aLangStr += aLocale.Language + aLocale.Country + aLocale.Variant;
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
                if (pCurWin && !dynamic_cast<ModulWindow*>(pCurWin.get()))
                {
                    rSet.DisableItem( nWh );
                    rSet.Put(SfxVisibilityItem(nWh, false));
                }
                break;
            }
            case SID_BASICIDE_HIDECURPAGE:
            {
                if (pTabBar->GetPageCount() == 0)
                    rSet.DisableItem(nWh);
            }
            break;
            case SID_BASICIDE_DELETECURRENT:
            case SID_BASICIDE_RENAMECURRENT:
            {
                if (pTabBar->GetPageCount() == 0 || StarBASIC::IsRunning())
                    rSet.DisableItem(nWh);
                else if (m_aCurDocument.isInVBAMode())
                {
                    // disable to delete or rename object modules in IDE
                    BasicManager* pBasMgr = m_aCurDocument.getBasicManager();
                    StarBASIC* pBasic = pBasMgr ? pBasMgr->GetLib(m_aCurLibName) : nullptr;
                    if (pBasic && dynamic_cast<ModulWindow*>(pCurWin.get()))
                    {
                        SbModule* pActiveModule = pBasic->FindModule( pCurWin->GetName() );
                        if ( pActiveModule && ( pActiveModule->GetModuleType() == script::ModuleType::DOCUMENT ) )
                            rSet.DisableItem(nWh);
                    }
                }
            }
            SAL_FALLTHROUGH;

            case SID_BASICIDE_NEWMODULE:
            case SID_BASICIDE_NEWDIALOG:
            {
                Reference< script::XLibraryContainer2 > xModLibContainer( m_aCurDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
                Reference< script::XLibraryContainer2 > xDlgLibContainer( m_aCurDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
                if ( ( xModLibContainer.is() && xModLibContainer->hasByName( m_aCurLibName ) && xModLibContainer->isLibraryReadOnly( m_aCurLibName ) ) ||
                     ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( m_aCurLibName ) && xDlgLibContainer->isLibraryReadOnly( m_aCurLibName ) ) )
                    rSet.DisableItem(nWh);
            }
            break;
            default:
                if (pLayout)
                    pLayout->GetState(rSet, nWh);
        }
    }
    if ( pCurWin )
        pCurWin->GetState( rSet );
}

bool Shell::HasUIFeature(SfxShellFeature nFeature) const
{
    assert((nFeature & ~SfxShellFeature::BasicMask) == SfxShellFeature::NONE);
    bool bResult = false;

    if (nFeature & SfxShellFeature::BasicShowBrowser)
    {
        // fade out (in) property browser in module (dialog) windows
        if (dynamic_cast<DialogWindow*>(pCurWin.get()) && !pCurWin->IsReadOnly())
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
                vcl::Window* pFrameWindow = &GetViewFrame()->GetWindow();
                vcl::Window* pFocusWindow = Application::GetFocusWindow();
                while ( pFocusWindow && ( pFocusWindow != pFrameWindow ) )
                    pFocusWindow = pFocusWindow->GetParent();
                if ( pFocusWindow ) // Focus in BasicIDE
                    pCurWin->GrabFocus();
            }
        }
        else
        {
            SetWindow(pLayout);
            pLayout = nullptr;
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
            SfxObjectShell::SetCurrentComponent(nullptr);
        }
        aObjectCatalog->SetCurrentEntry(pCurWin);
        SetUndoManager( pCurWin ? pCurWin->GetUndoManager() : nullptr );
        InvalidateBasicIDESlots();
        InvalidateControlSlots();
        EnableScrollbars(pCurWin != nullptr);

        if ( m_pCurLocalizationMgr )
            m_pCurLocalizationMgr->handleTranslationbar();

        ManageToolbars();

        // fade out (in) property browser in module (dialog) windows
        UIFeatureChanged();
    }
}

void Shell::ManageToolbars()
{
    static const char aMacroBarResName[] = "private:resource/toolbar/macrobar";
    static const char aDialogBarResName[] = "private:resource/toolbar/dialogbar";
    static const char aInsertControlsBarResName[] = "private:resource/toolbar/insertcontrolsbar";
    static const char aFormControlsBarResName[] = "private:resource/toolbar/formcontrolsbar";

    if( !pCurWin )
        return;

    Reference< beans::XPropertySet > xFrameProps
        ( GetViewFrame()->GetFrame().GetFrameInterface(), uno::UNO_QUERY );
    if ( xFrameProps.is() )
    {
        Reference< css::frame::XLayoutManager > xLayoutManager;
        uno::Any a = xFrameProps->getPropertyValue( "LayoutManager" );
        a >>= xLayoutManager;
        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            if (dynamic_cast<DialogWindow*>(pCurWin.get()))
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

VclPtr<BaseWindow> Shell::FindApplicationWindow()
{
    return FindWindow( ScriptDocument::getApplicationScriptDocument(), "", "", TYPE_UNKNOWN );
}

VclPtr<BaseWindow> Shell::FindWindow(
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
    return nullptr;
}

bool Shell::CallBasicErrorHdl( StarBASIC* pBasic )
{
    bool bRet = false;
    VclPtr<ModulWindow> pModWin = ShowActiveModuleWindow( pBasic );
    if ( pModWin )
        bRet = pModWin->BasicErrorHdl( pBasic );
    return bRet;
}

BasicDebugFlags Shell::CallBasicBreakHdl( StarBASIC* pBasic )
{
    BasicDebugFlags nRet = BasicDebugFlags::NONE;
    VclPtr<ModulWindow> pModWin = ShowActiveModuleWindow( pBasic );
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

VclPtr<ModulWindow> Shell::ShowActiveModuleWindow( StarBASIC* pBasic )
{
    SetCurLib( ScriptDocument::getApplicationScriptDocument(), OUString(), false );

    SbModule* pActiveModule = StarBASIC::GetActiveModule();
    if (SbClassModuleObject* pCMO = dynamic_cast<SbClassModuleObject*>(pActiveModule))
        pActiveModule = pCMO->getClassModule();

    DBG_ASSERT( pActiveModule, "No active module in ErrorHdl!?" );
    if ( pActiveModule )
    {
        VclPtr<ModulWindow> pWin;
        SbxObject* pParent = pActiveModule->GetParent();
        if (StarBASIC* pLib = dynamic_cast<StarBASIC*>(pParent))
        {
            if (BasicManager* pBasMgr = FindBasicManager(pLib))
            {
                ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
                OUString aLibName = pLib->GetName();
                pWin = FindBasWin( aDocument, aLibName, pActiveModule->GetName(), true );
                DBG_ASSERT( pWin, "Error/Step-Hdl: Window was not created/found!" );
                SetCurLib( aDocument, aLibName );
                SetCurWindow( pWin, true );
            }
        }
        else
            SAL_WARN( "basctl.basicide", "No BASIC!");
        if (BasicManager* pBasicMgr = FindBasicManager(pBasic))
            StartListening( *pBasicMgr, true /* log on only once */ );
        return pWin;
    }
    return nullptr;
}

void Shell::AdjustPosSizePixel( const Point &rPos, const Size &rSize )
{
    // not if iconified because the whole text would be displaced then at restore
    if ( GetViewFrame()->GetWindow().GetOutputSizePixel().Height() == 0 )
        return;

    Size aTabBarSize;
    aTabBarSize.Height() = GetViewFrame()->GetWindow().GetFont().GetFontHeight() + 4;
    aTabBarSize.Width() = rSize.Width();

    Size aSz( rSize );
    Size aScrollBarBoxSz( aScrollBarBox->GetSizePixel() );
    aSz.Height() -= aScrollBarBoxSz.Height();
    aSz.Height() -= aTabBarSize.Height();

    Size aOutSz( aSz );
    aSz.Width() -= aScrollBarBoxSz.Width();
    aScrollBarBox->SetPosPixel( Point( rSize.Width() - aScrollBarBoxSz.Width(), rSize.Height() - aScrollBarBoxSz.Height() ) );
    aVScrollBar->SetPosSizePixel( Point( rPos.X()+aSz.Width(), rPos.Y() ), Size( aScrollBarBoxSz.Width(), aSz.Height() ) );
    aHScrollBar->SetPosSizePixel( Point( rPos.X(), rPos.Y()+aSz.Height() ), Size( aSz.Width(), aScrollBarBoxSz.Height() ) );
    pTabBar->SetPosSizePixel( Point( rPos.X(), rPos.Y()+aScrollBarBoxSz.Height()+aSz.Height()), aTabBarSize );

    if (pLayout)
        pLayout->SetPosSizePixel(rPos, dynamic_cast<DialogWindow*>(pCurWin.get()) ? aSz : aOutSz);
}

Reference< XModel > Shell::GetCurrentDocument() const
{
    Reference< XModel > xDocument;
    if ( pCurWin && pCurWin->GetDocument().isDocument() )
        xDocument = pCurWin->GetDocument().getDocument();
    return xDocument;
}

void Shell::Activate( bool bMDI )
{
    SfxViewShell::Activate( bMDI );

    if ( bMDI )
    {
        if (DialogWindow* pDCurWin = dynamic_cast<DialogWindow*>(pCurWin.get()))
            pDCurWin->UpdateBrowser();
    }
}

void Shell::Deactivate( bool bMDI )
{
    // bMDI == true means that another MDI has been activated; in case of a
    // deactivate due to a MessageBox bMDI is false
    if ( bMDI )
    {
        if (DialogWindow* pXDlgWin = dynamic_cast<DialogWindow*>(pCurWin.get()))
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
