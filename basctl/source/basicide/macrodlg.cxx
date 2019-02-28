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


#include "macrodlg.hxx"
#include <basidesh.hxx>
#include <strings.hrc>

#include <iderdll.hxx>
#include "iderdll2.hxx"

#include "moduldlg.hxx"
#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <com/sun/star/script/XLibraryContainer2.hpp>

#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/request.hxx>
#include <vcl/weld.hxx>

#include <map>

namespace basctl
{

using std::map;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

MacroChooser::MacroChooser(weld::Window* pParnt, const Reference< frame::XFrame >& xDocFrame)
    : SfxDialogController(pParnt, "modules/BasicIDE/ui/basicmacrodialog.ui", "BasicMacroDialog")
    , m_xDocumentFrame(xDocFrame)
    // the Sfx doesn't ask the BasicManager whether modified or not
    // => start saving in case of a change without a into the BasicIDE.
    , bForceStoreBasic(false)
    , nMode(All)
    , m_xMacroNameEdit(m_xBuilder->weld_entry("macronameedit"))
    , m_xMacroFromTxT(m_xBuilder->weld_label("macrofromft"))
    , m_xMacrosSaveInTxt(m_xBuilder->weld_label("macrotoft"))
    , m_xBasicBox(new SbTreeListBox(m_xBuilder->weld_tree_view("libraries"), m_xDialog.get()))
    , m_xBasicBoxIter(m_xBasicBox->make_iterator())
    , m_xMacrosInTxt(m_xBuilder->weld_label("existingmacrosft"))
    , m_xMacroBox(m_xBuilder->weld_tree_view("macros"))
    , m_xMacroBoxIter(m_xMacroBox->make_iterator())
    , m_xRunButton(m_xBuilder->weld_button("ok"))
    , m_xCloseButton(m_xBuilder->weld_button("close"))
    , m_xAssignButton(m_xBuilder->weld_button("assign"))
    , m_xEditButton(m_xBuilder->weld_button("edit"))
    , m_xDelButton(m_xBuilder->weld_button("delete"))
    , m_xNewButton(m_xBuilder->weld_button("new"))
    , m_xOrganizeButton(m_xBuilder->weld_button("organize"))
    , m_xNewLibButton(m_xBuilder->weld_button("newlibrary"))
    , m_xNewModButton(m_xBuilder->weld_button("newmodule"))
{
    m_xBasicBox->set_size_request(m_xBasicBox->get_approximate_digit_width() * 30, m_xBasicBox->get_height_rows(18));
    m_xMacroBox->set_size_request(m_xMacroBox->get_approximate_digit_width() * 30, m_xMacroBox->get_height_rows(18));

    m_aMacrosInTxtBaseStr = m_xMacrosInTxt->get_label();

    m_xRunButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xCloseButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xAssignButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xEditButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xDelButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xNewButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xOrganizeButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );

    // Buttons only for MacroChooser::Recording
    m_xNewLibButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xNewModButton->connect_clicked( LINK( this, MacroChooser, ButtonHdl ) );
    m_xNewLibButton->hide();       // default
    m_xNewModButton->hide();       // default
    m_xMacrosSaveInTxt->hide();    // default

    m_xMacroNameEdit->connect_changed( LINK( this, MacroChooser, EditModifyHdl ) );

    m_xBasicBox->connect_changed( LINK( this, MacroChooser, BasicSelectHdl ) );

    m_xMacroBox->connect_row_activated( LINK( this, MacroChooser, MacroDoubleClickHdl ) );
    m_xMacroBox->connect_changed( LINK( this, MacroChooser, MacroSelectHdl ) );

    m_xBasicBox->SetMode( BrowseMode::Modules );

    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );

    m_xBasicBox->ScanAllEntries();
}

MacroChooser::~MacroChooser()
{
    if (bForceStoreBasic)
    {
        SfxGetpApp()->SaveBasicAndDialogContainer();
        bForceStoreBasic = false;
    }
}

void MacroChooser::StoreMacroDescription()
{
    m_xBasicBox->get_selected(m_xBasicBoxIter.get());
    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
    OUString aMethodName;
    if (m_xMacroBox->get_selected(m_xMacroBoxIter.get()))
        aMethodName = m_xMacroBox->get_text(*m_xMacroBoxIter);
    else
        aMethodName = m_xMacroNameEdit->get_text();
    if ( !aMethodName.isEmpty() )
    {
        aDesc.SetMethodName( aMethodName );
        aDesc.SetType( OBJ_TYPE_METHOD );
    }

    if (ExtraData* pData = basctl::GetExtraData())
        pData->SetLastEntryDescriptor( aDesc );
}

void MacroChooser::RestoreMacroDescription()
{
    EntryDescriptor aDesc;
    if (Shell* pShell = GetShell())
    {
        if (BaseWindow* pCurWin = pShell->GetCurWindow())
            aDesc = pCurWin->CreateEntryDescriptor();
    }
    else
    {
        if (ExtraData* pData = basctl::GetExtraData())
            aDesc = pData->GetLastEntryDescriptor();
    }

    m_xBasicBox->SetCurrentEntry( aDesc );

    OUString aLastMacro( aDesc.GetMethodName() );
    if (!aLastMacro.isEmpty())
    {
        // find entry in macro box
        auto nIndex = m_xMacroBox->find_text(aLastMacro);
        if (nIndex != -1)
            m_xMacroBox->select(nIndex);
        else
        {
            m_xMacroNameEdit->set_text(aLastMacro);
            m_xMacroNameEdit->select_region(0, 0);
        }
    }
}

short MacroChooser::run()
{
    RestoreMacroDescription();
    m_xRunButton->grab_focus();

    // #104198 Check if "wrong" document is active
    bool bSelectedEntry = m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
    EntryDescriptor aDesc(m_xBasicBox->GetEntryDescriptor(bSelectedEntry ? m_xBasicBoxIter.get() : nullptr));
    const ScriptDocument& rSelectedDoc(aDesc.GetDocument());

    // App Basic is always ok, so only check if shell was found
    if( rSelectedDoc.isDocument() && !rSelectedDoc.isActive() )
    {
        // Search for the right entry
        bool bValidIter = m_xBasicBox->get_iter_first(*m_xBasicBoxIter);
        while (bValidIter)
        {
            EntryDescriptor aCmpDesc(m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get()));
            const ScriptDocument& rCmpDoc( aCmpDesc.GetDocument() );
            if (rCmpDoc.isDocument() && rCmpDoc.isActive())
            {
                std::unique_ptr<weld::TreeIter> xEntry(m_xBasicBox->make_iterator());
                m_xBasicBox->copy_iterator(*m_xBasicBoxIter, *xEntry);
                std::unique_ptr<weld::TreeIter> xLastValid(m_xBasicBox->make_iterator());
                bool bValidEntryIter = true;
                do
                {
                    m_xBasicBox->copy_iterator(*xEntry, *xLastValid);
                    bValidEntryIter = m_xBasicBox->iter_children(*xEntry);
                }
                while (bValidEntryIter);
                m_xBasicBox->set_cursor(*xLastValid);
            }
            bValidIter = m_xBasicBox->iter_next_sibling(*m_xBasicBoxIter);
        }
    }

    CheckButtons();
    UpdateFields();

    if ( StarBASIC::IsRunning() )
        m_xCloseButton->grab_focus();

    return SfxDialogController::run();
}

void MacroChooser::EnableButton(weld::Button& rButton, bool bEnable)
{
    if ( bEnable )
    {
        if (nMode == ChooseOnly || nMode == Recording)
            rButton.set_sensitive(&rButton == m_xRunButton.get());
        else
            rButton.set_sensitive(true);
    }
    else
        rButton.set_sensitive(false);
}

SbMethod* MacroChooser::GetMacro()
{
    if (!m_xBasicBox->get_cursor(m_xBasicBoxIter.get()))
        return nullptr;
    SbModule* pModule = m_xBasicBox->FindModule(m_xBasicBoxIter.get());
    if (!pModule)
        return nullptr;
    if (!m_xMacroBox->get_selected(m_xMacroBoxIter.get()))
        return nullptr;
    OUString aMacroName(m_xMacroBox->get_text(*m_xMacroBoxIter));
    return pModule->FindMethod(aMacroName, SbxClassType::Method);
}

void MacroChooser::DeleteMacro()
{
    SbMethod* pMethod = GetMacro();
    DBG_ASSERT( pMethod, "DeleteMacro: No Macro !" );
    if (pMethod && QueryDelMacro(pMethod->GetName(), m_xDialog.get()))
    {
        if (SfxDispatcher* pDispatcher = GetDispatcher())
            pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );

        // mark current doc as modified:
        StarBASIC* pBasic = FindBasic(pMethod);
        assert(pBasic && "Basic?!");
        BasicManager* pBasMgr = FindBasicManager( pBasic );
        DBG_ASSERT( pBasMgr, "BasMgr?" );
        ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
        if ( aDocument.isDocument() )
        {
            aDocument.setDocumentModified();
            if (SfxBindings* pBindings = GetBindingsPtr())
                pBindings->Invalidate( SID_SAVEDOC );
        }

        SbModule* pModule = pMethod->GetModule();
        assert(pModule && "DeleteMacro: No Module?!");
        OUString aSource( pModule->GetSource32() );
        sal_uInt16 nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1 );
        pModule->SetSource32( aSource );

        // update module in library
        OUString aLibName = pBasic->GetName();
        OUString aModName = pModule->GetName();
        OSL_VERIFY( aDocument.updateModule( aLibName, aModName, aSource ) );

        bool bSelected = m_xMacroBox->get_selected(m_xMacroBoxIter.get());
        DBG_ASSERT(bSelected, "DeleteMacro: Entry ?!");
        m_xMacroBox->remove(*m_xMacroBoxIter);
        bForceStoreBasic = true;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    SbMethod* pMethod = nullptr;
    m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    OSL_ENSURE( aDocument.isAlive(), "MacroChooser::CreateMacro: no document!" );
    if ( !aDocument.isAlive() )
        return nullptr;

    OUString aLibName( aDesc.GetLibName() );

    if ( aLibName.isEmpty() )
        aLibName = "Standard" ;

    aDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
        xModLibContainer->loadLibrary( aOULibName );
    Reference< script::XLibraryContainer > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ) );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
        xDlgLibContainer->loadLibrary( aOULibName );

    BasicManager* pBasMgr = aDocument.getBasicManager();
    StarBASIC* pBasic = pBasMgr ? pBasMgr->GetLib( aLibName ) : nullptr;
    if ( pBasic )
    {
        SbModule* pModule = nullptr;
        OUString aModName( aDesc.GetName() );
        if ( !aModName.isEmpty() )
        {
            // extract the module name from the string like "Sheet1 (Example1)"
            if( aDesc.GetLibSubName() == IDEResId(RID_STR_DOCUMENT_OBJECTS) )
            {
                aModName = aModName.getToken( 0, ' ' );
            }
            pModule = pBasic->FindModule( aModName );
        }
        else if ( !pBasic->GetModules().empty() )
            pModule = pBasic->GetModules().front().get();

        // Retain the desired macro name before the macro dialog box is forced to close
        // by opening the module name dialog window when no module exists in the current library.
        OUString aSubName = m_xMacroNameEdit->get_text();

        if ( !pModule )
        {
            pModule = createModImpl(m_xDialog.get(), aDocument, *m_xBasicBox, aLibName, aModName, false);
        }

        DBG_ASSERT( !pModule || !pModule->FindMethod( aSubName, SbxClassType::Method ), "Macro exists already!" );
        pMethod = pModule ? basctl::CreateMacro( pModule, aSubName ) : nullptr;
    }

    return pMethod;
}

void MacroChooser::SaveSetCurEntry(weld::TreeView& rBox, const weld::TreeIter& rEntry)
{
    // the edit would be killed by the highlight otherwise:

    OUString aSaveText(m_xMacroNameEdit->get_text());
    int nStartPos, nEndPos;
    m_xMacroNameEdit->get_selection_bounds(nStartPos, nEndPos);

    rBox.set_cursor(rEntry);

    m_xMacroNameEdit->set_text(aSaveText);
    m_xMacroNameEdit->select_region(nStartPos, nEndPos);
}

void MacroChooser::CheckButtons()
{
    const bool bCurEntry = m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(bCurEntry ? m_xBasicBoxIter.get() : nullptr);
    const bool bMacroEntry = m_xMacroBox->get_selected(nullptr);
    SbMethod* pMethod = GetMacro();

    // check, if corresponding libraries are readonly
    bool bReadOnly = false;
    sal_uInt16 nDepth = bCurEntry ? m_xBasicBox->get_iter_depth(*m_xBasicBoxIter) : 0;
    if ( nDepth == 1 || nDepth == 2 )
    {
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        const OUString& aOULibName( aDesc.GetLibName() );
        Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
        if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
        {
            bReadOnly = true;
        }
    }

    if (nMode != Recording)
    {
        // Run...
        bool bEnable = pMethod != nullptr;
        if (nMode != ChooseOnly && StarBASIC::IsRunning())
            bEnable = false;
        EnableButton(*m_xRunButton, bEnable);
    }

    // organising still possible?

    // Assign...
    EnableButton(*m_xAssignButton, pMethod != nullptr);

    // Edit...
    EnableButton(*m_xEditButton, bMacroEntry);

    // Organizer...
    EnableButton(*m_xOrganizeButton, !StarBASIC::IsRunning() && nMode == All);

    // m_xDelButton/m_xNewButton ->...
    bool bProtected = bCurEntry && m_xBasicBox->IsEntryProtected(m_xBasicBoxIter.get());
    bool bShare = ( aDesc.GetLocation() == LIBRARY_LOCATION_SHARE );
    bool bEnable = !StarBASIC::IsRunning() && nMode == All && !bProtected && !bReadOnly && !bShare;
    EnableButton(*m_xDelButton, bEnable);
    EnableButton(*m_xNewButton, bEnable);
    if (nMode == All)
    {
        if (pMethod)
        {
            m_xDelButton->show();
            m_xNewButton->hide();
        }
        else
        {
            m_xNewButton->show();
            m_xDelButton->hide();
        }
    }

    if (nMode == Recording)
    {
        // save button
        m_xRunButton->set_sensitive(!bProtected && !bReadOnly && !bShare);
        // new library button
        m_xNewLibButton->set_sensitive(!bShare);
        // new module button
        m_xNewModButton->set_sensitive(!bProtected && !bReadOnly && !bShare);
    }
}

IMPL_LINK_NOARG(MacroChooser, MacroDoubleClickHdl, weld::TreeView&, void)
{
    SbMethod* pMethod = GetMacro();
    SbModule* pModule = pMethod ? pMethod->GetModule() : nullptr;
    StarBASIC* pBasic = pModule ? static_cast<StarBASIC*>(pModule->GetParent()) : nullptr;
    BasicManager* pBasMgr = pBasic ? FindBasicManager(pBasic) : nullptr;
    ScriptDocument aDocument(ScriptDocument::getDocumentForBasicManager(pBasMgr));
    if (aDocument.isDocument() && !aDocument.allowMacros())
    {
        std::unique_ptr<weld::MessageDialog> xError(
            Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                                             VclButtonsType::Ok, IDEResId(RID_STR_CANNOTRUNMACRO)));
        xError->run();
        return;
    }

    StoreMacroDescription();
    if (nMode == Recording)
    {
        if (pMethod && !QueryReplaceMacro(pMethod->GetName(), m_xDialog.get()))
            return;
    }

    m_xDialog->response(Macro_OkRun);
}

IMPL_LINK_NOARG(MacroChooser, MacroSelectHdl, weld::TreeView&, void)
{
    UpdateFields();
    CheckButtons();
}

IMPL_LINK_NOARG(MacroChooser, BasicSelectHdl, weld::TreeView&, void)
{
    m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
    SbModule* pModule = m_xBasicBox->FindModule(m_xBasicBoxIter.get());
    m_xMacroBox->clear();
    if (pModule)
    {
        m_xMacrosInTxt->set_label(m_aMacrosInTxtBaseStr + " " + pModule->GetName());

        // The macros should be called in the same order that they
        // are written down in the module.

        map< sal_uInt16, SbMethod* > aMacros;
        size_t nMacroCount = pModule->GetMethods()->Count();
        for ( size_t iMeth = 0; iMeth  < nMacroCount; iMeth++ )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pModule->GetMethods()->Get( iMeth ));
            if( pMethod->IsHidden() )
                continue;
            DBG_ASSERT( pMethod, "Method not found! (NULL)" );
            sal_uInt16 nStart, nEnd;
            pMethod->GetLineRange( nStart, nEnd );
            aMacros.emplace( nStart, pMethod );
        }

        m_xMacroBox->freeze();
        for (auto const& macro : aMacros)
            m_xMacroBox->append_text(macro.second->GetName());
        m_xMacroBox->thaw();

        if (m_xMacroBox->n_children())
        {
            m_xMacroBox->get_iter_first(*m_xMacroBoxIter);
            m_xMacroBox->set_cursor(*m_xMacroBoxIter);
        }
    }

    UpdateFields();
    CheckButtons();
}

IMPL_LINK_NOARG(MacroChooser, EditModifyHdl, weld::Entry&, void)
{
    // select the module in which the macro is put at "new",
    // if BasicManager or Lib is selecting
    if (m_xBasicBox->get_cursor(m_xBasicBoxIter.get()))
    {
        sal_uInt16 nDepth = m_xBasicBox->get_iter_depth(*m_xBasicBoxIter);
        if (nDepth == 1 && m_xBasicBox->IsEntryProtected(m_xBasicBoxIter.get()))
        {
            // then put to the respective Std-Lib...
            m_xBasicBox->iter_parent(*m_xBasicBoxIter);
            m_xBasicBox->iter_children(*m_xBasicBoxIter);
        }
        if (nDepth < 2)
        {
            std::unique_ptr<weld::TreeIter> xNewEntry(m_xBasicBox->make_iterator());
            m_xBasicBox->copy_iterator(*m_xBasicBoxIter, *xNewEntry);
            bool bCurEntry = true;
            do
            {
                bCurEntry = m_xBasicBox->iter_children(*m_xBasicBoxIter);
                if (bCurEntry)
                {
                    m_xBasicBox->copy_iterator(*m_xBasicBoxIter, *xNewEntry);
                    nDepth = m_xBasicBox->get_iter_depth(*m_xBasicBoxIter);
                }
            }
            while (bCurEntry && (nDepth < 2));
            SaveSetCurEntry(m_xBasicBox->get_widget(), *xNewEntry);
        }
        auto nCount = m_xMacroBox->n_children();
        if (nCount)
        {
            OUString aEdtText(m_xMacroNameEdit->get_text());
            bool bFound = false;
            bool bValidIter = m_xMacroBox->get_iter_first(*m_xMacroBoxIter);
            while (bValidIter)
            {
                if (m_xMacroBox->get_text(*m_xMacroBoxIter).equalsIgnoreAsciiCase(aEdtText))
                {
                    SaveSetCurEntry(*m_xMacroBox, *m_xMacroBoxIter);
                    bFound = true;
                    break;
                }
                bValidIter = m_xMacroBox->iter_next_sibling(*m_xMacroBoxIter);
            }
            if (!bFound)
            {
                bValidIter = m_xMacroBox->get_selected(m_xMacroBoxIter.get());
                // if the entry exists ->Select ->Description...
                if (bValidIter)
                    m_xMacroBox->unselect(*m_xMacroBoxIter);
            }
        }
    }

    CheckButtons();
}

IMPL_LINK(MacroChooser, ButtonHdl, weld::Button&, rButton, void)
{
    // apart from New/Record the Description is done by LoseFocus
    if (&rButton == m_xRunButton.get())
    {
        StoreMacroDescription();

        // #116444# check security settings before macro execution
        if (nMode == All)
        {
            SbMethod* pMethod = GetMacro();
            SbModule* pModule = pMethod ? pMethod->GetModule() : nullptr;
            StarBASIC* pBasic = pModule ? static_cast<StarBASIC*>(pModule->GetParent()) : nullptr;
            BasicManager* pBasMgr = pBasic ? FindBasicManager(pBasic) : nullptr;
            if ( pBasMgr )
            {
                ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
                if ( aDocument.isDocument() && !aDocument.allowMacros() )
                {
                    std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                                VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_CANNOTRUNMACRO)));
                    xError->run();
                    return;
                }
            }
        }
        else if (nMode == Recording )
        {
            if ( !IsValidSbxName(m_xMacroNameEdit->get_text()) )
            {
                std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                            VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
                xError->run();
                m_xMacroNameEdit->select_region(0, -1);
                m_xMacroNameEdit->grab_focus();
                return;
            }

            SbMethod* pMethod = GetMacro();
            if (pMethod && !QueryReplaceMacro(pMethod->GetName(), m_xDialog.get()))
                return;
        }

        m_xDialog->response(Macro_OkRun);
    }
    else if (&rButton == m_xCloseButton.get())
    {
        StoreMacroDescription();
        m_xDialog->response(Macro_Close);
    }
    else if (&rButton == m_xEditButton.get() || &rButton == m_xDelButton.get() || &rButton == m_xNewButton.get())
    {
        m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        const OUString& aLib( aDesc.GetLibName() );
        OUString aMod( aDesc.GetName() );
        // extract the module name from the string like "Sheet1 (Example1)"
        if( aDesc.GetLibSubName() == IDEResId(RID_STR_DOCUMENT_OBJECTS) )
        {
            aMod = aMod.getToken( 0, ' ' );
        }
        const OUString& aSub( aDesc.GetMethodName() );
        SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLib, aMod, aSub, OUString() );
        if (&rButton == m_xEditButton.get())
        {
            if (m_xMacroBox->get_selected(m_xMacroBoxIter.get()))
                aInfoItem.SetMethod(m_xMacroBox->get_text(*m_xMacroBoxIter));
            StoreMacroDescription();
            SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
            SfxGetpApp()->ExecuteSlot( aRequest );

            if (SfxDispatcher* pDispatcher = GetDispatcher())
            {
                pDispatcher->ExecuteList(SID_BASICIDE_EDITMACRO,
                        SfxCallMode::ASYNCHRON, { &aInfoItem });
            }
            m_xDialog->response(Macro_Edit);
        }
        else
        {
            if (&rButton == m_xDelButton.get())
            {
                DeleteMacro();
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                {
                    pDispatcher->ExecuteList( SID_BASICIDE_UPDATEMODULESOURCE,
                                  SfxCallMode::SYNCHRON, { &aInfoItem });
                }
                CheckButtons();
                UpdateFields();
                //if ( m_xMacroBox->GetCurEntry() )    // OV-Bug ?
                //  m_xMacroBox->Select( m_xMacroBox->GetCurEntry() );
            }
            else
            {
                if ( !IsValidSbxName(m_xMacroNameEdit->get_text()) )
                {
                    std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_xDialog.get(),
                                                                VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
                    xError->run();
                    m_xMacroNameEdit->select_region(0, -1);
                    m_xMacroNameEdit->grab_focus();
                    return;
                }
                SbMethod* pMethod = CreateMacro();
                if ( pMethod )
                {
                    aInfoItem.SetMethod( pMethod->GetName() );
                    aInfoItem.SetModule( pMethod->GetModule()->GetName() );
                    aInfoItem.SetLib( pMethod->GetModule()->GetParent()->GetName() );
                    SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
                    SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
                    SfxGetpApp()->ExecuteSlot( aRequest );

                    if (SfxDispatcher* pDispatcher = GetDispatcher())
                    {
                        pDispatcher->ExecuteList(SID_BASICIDE_EDITMACRO,
                                SfxCallMode::ASYNCHRON, { &aInfoItem });
                    }
                    StoreMacroDescription();
                    m_xDialog->response(Macro_New);
                }
            }
        }
    }
    else if (&rButton == m_xAssignButton.get())
    {
        m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        const OUString& aLib( aDesc.GetLibName() );
        const OUString& aMod( aDesc.GetName() );
        OUString aSub( m_xMacroNameEdit->get_text() );
        SbMethod* pMethod = GetMacro();
        DBG_ASSERT( pBasMgr, "BasMgr?" );
        DBG_ASSERT( pMethod, "Method?" );
        OUString aComment( GetInfo( pMethod ) );
        SfxMacroInfoItem aItem( SID_MACROINFO, pBasMgr, aLib, aMod, aSub, aComment );
        SfxAllItemSet Args( SfxGetpApp()->GetPool() );

        SfxAllItemSet aInternalSet(SfxGetpApp()->GetPool());
        if (m_xDocumentFrame.is())
            aInternalSet.Put(SfxUnoFrameItem(SID_FILLFRAME, m_xDocumentFrame));

        SfxRequest aRequest(SID_CONFIG, SfxCallMode::SYNCHRON, Args, aInternalSet);
        aRequest.AppendItem( aItem );
        SfxGetpApp()->ExecuteSlot( aRequest );
    }
    else if (&rButton == m_xNewLibButton.get())
    {
        m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        createLibImpl(m_xDialog.get(), aDocument, nullptr, m_xBasicBox.get());
    }
    else if (&rButton == m_xNewModButton.get())
    {
        m_xBasicBox->get_cursor(m_xBasicBoxIter.get());
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
        const ScriptDocument& aDocument( aDesc.GetDocument() );
        const OUString& aLibName( aDesc.GetLibName() );
        createModImpl(m_xDialog.get(), aDocument, *m_xBasicBox, aLibName, OUString(), true);
    }
    else if (&rButton == m_xOrganizeButton.get())
    {
        StoreMacroDescription();

        m_xBasicBox->get_selected(m_xBasicBoxIter.get());
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(m_xBasicBoxIter.get());
        VclPtrInstance< OrganizeDialog > pDlg( nullptr, 0, aDesc ); //TODO
        pDlg->StartExecuteAsync([this](sal_Int32 nRet){
                if ( nRet ) // not only closed
                {
                    m_xDialog->response(Macro_Edit);
                    return;
                }

                Shell* pShell = GetShell();
                if ( pShell && pShell->IsAppBasicModified() )
                    bForceStoreBasic = true;

                m_xBasicBox->UpdateEntries();
            });
    }
}


void MacroChooser::UpdateFields()
{
    auto nMacroEntry = m_xMacroBox->get_selected_index();
    m_xMacroNameEdit->set_text("");
    if (nMacroEntry != -1)
        m_xMacroNameEdit->set_text(m_xMacroBox->get_text(nMacroEntry));
}

void MacroChooser::SetMode (Mode nM)
{
    nMode = nM;
    switch (nMode)
    {
        case All:
        {
            m_xRunButton->set_label(IDEResId(RID_STR_RUN));
            EnableButton(*m_xDelButton, true);
            EnableButton(*m_xNewButton, true);
            EnableButton(*m_xOrganizeButton, true);
            break;
        }

        case ChooseOnly:
        {
            m_xRunButton->set_label(IDEResId(RID_STR_CHOOSE));
            EnableButton(*m_xDelButton, false);
            EnableButton(*m_xNewButton, false);
            EnableButton(*m_xOrganizeButton, false);
            break;
        }

        case Recording:
        {
            m_xRunButton->set_label(IDEResId(RID_STR_RECORD));
            EnableButton(*m_xDelButton, false);
            EnableButton(*m_xNewButton, false);
            EnableButton(*m_xOrganizeButton, false);

            m_xAssignButton->hide();
            m_xEditButton->hide();
            m_xDelButton->hide();
            m_xNewButton->hide();
            m_xOrganizeButton->hide();
            m_xMacroFromTxT->hide();

            m_xNewLibButton->show();
            m_xNewModButton->show();
            m_xMacrosSaveInTxt->show();

            break;
        }
    }
    CheckButtons();
}

OUString MacroChooser::GetInfo( SbxVariable* pVar )
{
    OUString aComment;
    SbxInfoRef xInfo = pVar->GetInfo();
    if ( xInfo.is() )
        aComment = xInfo->GetComment();
    return aComment;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
