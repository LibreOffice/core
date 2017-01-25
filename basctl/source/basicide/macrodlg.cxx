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


#include <macrodlg.hxx>
#include <macrodlg.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>

#include <iderdll.hxx>
#include <iderdll2.hxx>

#include <moduldlg.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/request.hxx>


#include <map>

namespace basctl
{

using ::std::map;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

MacroChooser::MacroChooser( vcl::Window* pParnt, const Reference< frame::XFrame >& xDocFrame, bool bCreateEntries )
    : SfxModalDialog(pParnt, "BasicMacroDialog", "modules/BasicIDE/ui/basicmacrodialog.ui")
    , m_xDocumentFrame(xDocFrame)
    , bNewDelIsDel(true)
    // the Sfx doesn't ask the BasicManager whether modified or not
    // => start saving in case of a change without a into the BasicIDE.
    , bForceStoreBasic(false)
    , nMode(All)
{
    get(m_pMacroNameEdit, "macronameedit");
    get(m_pMacroFromTxT, "macrofromft");
    get(m_pMacrosSaveInTxt, "macrotoft");
    get(m_pBasicBox, "libraries");
    get(m_pMacrosInTxt, "existingmacrosft");
    m_aMacrosInTxtBaseStr = m_pMacrosInTxt->GetText();
    get(m_pMacroBox, "macros");
    get(m_pRunButton, "run");
    get(m_pCloseButton, "close");
    get(m_pAssignButton, "assign");
    get(m_pEditButton, "edit");
    get(m_pDelButton, "delete");
    get(m_pOrganizeButton, "organize");
    get(m_pNewLibButton, "newlibrary");
    get(m_pNewModButton, "newmodule");

    m_pMacroBox->SetSelectionMode( SelectionMode::Single );
    m_pMacroBox->SetHighlightRange(); // select over the whole width

    m_pRunButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pCloseButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pAssignButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pEditButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pDelButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pOrganizeButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );

    // Buttons only for MacroChooser::Recording
    m_pNewLibButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pNewModButton->SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    m_pNewLibButton->Hide();       // default
    m_pNewModButton->Hide();       // default
    m_pMacrosSaveInTxt->Hide();    // default

    m_pMacrosInTxt->SetStyle( WB_NOMULTILINE | WB_PATHELLIPSIS );

    m_pMacroNameEdit->SetModifyHdl( LINK( this, MacroChooser, EditModifyHdl ) );

    m_pBasicBox->SetSelectHdl( LINK( this, MacroChooser, BasicSelectHdl ) );

    m_pMacroBox->SetDoubleClickHdl( LINK( this, MacroChooser, MacroDoubleClickHdl ) );
    m_pMacroBox->SetSelectHdl( LINK( this, MacroChooser, MacroSelectHdl ) );

    m_pBasicBox->SetMode( BROWSEMODE_MODULES );
    m_pBasicBox->SetStyle( WB_TABSTOP | WB_BORDER |
                        WB_HASLINES | WB_HASLINESATROOT |
                        WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                        WB_HSCROLL );

    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );

    if ( bCreateEntries )
        m_pBasicBox->ScanAllEntries();
}

MacroChooser::~MacroChooser()
{
    disposeOnce();
}

void MacroChooser::dispose()
{
    if ( bForceStoreBasic )
    {
        SfxGetpApp()->SaveBasicAndDialogContainer();
        bForceStoreBasic = false;
    }
    m_pMacroNameEdit.clear();
    m_pMacroFromTxT.clear();
    m_pMacrosSaveInTxt.clear();
    m_pBasicBox.clear();
    m_pMacrosInTxt.clear();
    m_pMacroBox.clear();
    m_pRunButton.clear();
    m_pCloseButton.clear();
    m_pAssignButton.clear();
    m_pEditButton.clear();
    m_pDelButton.clear();
    m_pOrganizeButton.clear();
    m_pNewLibButton.clear();
    m_pNewModButton.clear();
    SfxModalDialog::dispose();
}

void MacroChooser::StoreMacroDescription()
{
    EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(m_pBasicBox->FirstSelected());
    OUString aMethodName;
    SvTreeListEntry* pEntry = m_pMacroBox->FirstSelected();
    if ( pEntry )
        aMethodName = m_pMacroBox->GetEntryText( pEntry );
    else
        aMethodName = m_pMacroNameEdit->GetText();
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

    m_pBasicBox->SetCurrentEntry( aDesc );

    OUString aLastMacro( aDesc.GetMethodName() );
    if ( !aLastMacro.isEmpty() )
    {
        // find entry in macro box
        SvTreeListEntry* pEntry = nullptr;
        sal_uLong nPos = 0;
        SvTreeListEntry* pE = m_pMacroBox->GetEntry( nPos );
        while ( pE )
        {
            if ( m_pMacroBox->GetEntryText( pE ) == aLastMacro )
            {
                pEntry = pE;
                break;
            }
            pE = m_pMacroBox->GetEntry( ++nPos );
        }

        if ( pEntry )
            m_pMacroBox->SetCurEntry( pEntry );
        else
        {
            m_pMacroNameEdit->SetText( aLastMacro );
            m_pMacroNameEdit->SetSelection( Selection( 0, 0 ) );
        }
    }
}

short MacroChooser::Execute()
{
    RestoreMacroDescription();
    m_pRunButton->GrabFocus();

    // #104198 Check if "wrong" document is active
    SvTreeListEntry* pSelectedEntry = m_pBasicBox->GetCurEntry();
    EntryDescriptor aDesc( m_pBasicBox->GetEntryDescriptor( pSelectedEntry ) );
    const ScriptDocument& rSelectedDoc( aDesc.GetDocument() );

    // App Basic is always ok, so only check if shell was found
    if( rSelectedDoc.isDocument() && !rSelectedDoc.isActive() )
    {
        // Search for the right entry
        sal_uLong nRootPos = 0;
        SvTreeListEntry* pRootEntry = m_pBasicBox->GetEntry( nRootPos );
        while( pRootEntry )
        {
            EntryDescriptor aCmpDesc( m_pBasicBox->GetEntryDescriptor( pRootEntry ) );
            const ScriptDocument& rCmpDoc( aCmpDesc.GetDocument() );
            if ( rCmpDoc.isDocument() && rCmpDoc.isActive() )
            {
                SvTreeListEntry* pEntry = pRootEntry;
                SvTreeListEntry* pLastValid = pEntry;
                while ( pEntry )
                {
                    pLastValid = pEntry;
                    pEntry = m_pBasicBox->FirstChild( pEntry );
                }
                if( pLastValid )
                    m_pBasicBox->SetCurEntry( pLastValid );
            }
            pRootEntry = m_pBasicBox->GetEntry( ++nRootPos );
        }
    }

    CheckButtons();
    UpdateFields();

    if ( StarBASIC::IsRunning() )
        m_pCloseButton->GrabFocus();

    return ModalDialog::Execute();
}

void MacroChooser::EnableButton( Button& rButton, bool bEnable )
{
    if ( bEnable )
    {
        if (nMode == ChooseOnly || nMode == Recording)
            rButton.Enable(&rButton == m_pRunButton);
        else
            rButton.Enable();
    }
    else
        rButton.Disable();
}


SbMethod* MacroChooser::GetMacro()
{
    SbMethod* pMethod = nullptr;
    SbModule* pModule = m_pBasicBox->FindModule( m_pBasicBox->GetCurEntry() );
    if ( pModule )
    {
        SvTreeListEntry* pEntry = m_pMacroBox->FirstSelected();
        if ( pEntry )
        {
            OUString aMacroName( m_pMacroBox->GetEntryText( pEntry ) );
            pMethod = pModule->FindMethod( aMacroName, SbxClassType::Method );
        }
    }
    return pMethod;
}


void MacroChooser::DeleteMacro()
{
    SbMethod* pMethod = GetMacro();
    DBG_ASSERT( pMethod, "DeleteMacro: Kein Macro !" );
    if ( pMethod && QueryDelMacro( pMethod->GetName(), this ) )
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
        assert(pModule && "DeleteMacro: Kein Modul?!");
        OUString aSource( pModule->GetSource32() );
        sal_uInt16 nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1, true );
        pModule->SetSource32( aSource );

        // update module in library
        OUString aLibName = pBasic->GetName();
        OUString aModName = pModule->GetName();
        OSL_VERIFY( aDocument.updateModule( aLibName, aModName, aSource ) );

        SvTreeListEntry* pEntry = m_pMacroBox->FirstSelected();
        DBG_ASSERT( pEntry, "DeleteMacro: Entry ?!" );
        m_pMacroBox->GetModel()->Remove( pEntry );
        bForceStoreBasic = true;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    SbMethod* pMethod = nullptr;
    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
    ScriptDocument aDocument( aDesc.GetDocument() );
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
            if( aDesc.GetLibSubName() == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) )
            {
                sal_Int32 nIndex = 0;
                aModName = aModName.getToken( 0, ' ', nIndex );
            }
            pModule = pBasic->FindModule( aModName );
        }
        else if ( !pBasic->GetModules().empty() )
            pModule = pBasic->GetModules().front().get();

        // Retain the desired macro name before the macro dialog box is forced to close
        // by opening the module name dialog window when no module exists in the current library.
        OUString aSubName = m_pMacroNameEdit->GetText();

        if ( !pModule )
        {
            pModule = createModImpl( static_cast<vcl::Window*>( this ),
                aDocument, *m_pBasicBox, aLibName, aModName, false );
        }

        DBG_ASSERT( !pModule || !pModule->FindMethod( aSubName, SbxClassType::Method ), "Macro existiert schon!" );
        pMethod = pModule ? basctl::CreateMacro( pModule, aSubName ) : nullptr;
    }

    return pMethod;
}

void MacroChooser::SaveSetCurEntry( SvTreeListBox& rBox, SvTreeListEntry* pEntry )
{
    // the edit would be killed by the highlight otherwise:

    OUString aSaveText( m_pMacroNameEdit->GetText() );
    Selection aCurSel( m_pMacroNameEdit->GetSelection() );

    rBox.SetCurEntry( pEntry );
    m_pMacroNameEdit->SetText( aSaveText );
    m_pMacroNameEdit->SetSelection( aCurSel );
}

void MacroChooser::CheckButtons()
{
    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
    SvTreeListEntry* pMacroEntry = m_pMacroBox->FirstSelected();
    SbMethod* pMethod = GetMacro();

    // check, if corresponding libraries are readonly
    bool bReadOnly = false;
    sal_uInt16 nDepth = pCurEntry ? m_pBasicBox->GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth == 1 || nDepth == 2 )
    {
        ScriptDocument aDocument( aDesc.GetDocument() );
        OUString aOULibName( aDesc.GetLibName() );
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
        EnableButton(*m_pRunButton, bEnable);
    }

    // organising still possible?

    // Assign...
    EnableButton(*m_pAssignButton, pMethod != nullptr);

    // Edit...
    EnableButton(*m_pEditButton, pMacroEntry != nullptr);

    // Organizer...
    EnableButton(*m_pOrganizeButton, !StarBASIC::IsRunning() && nMode == All);

    // m_pDelButton->...
    bool bProtected = m_pBasicBox->IsEntryProtected( pCurEntry );
    bool bShare = ( aDesc.GetLocation() == LIBRARY_LOCATION_SHARE );
    EnableButton(*m_pDelButton, !StarBASIC::IsRunning() && nMode == All && !bProtected && !bReadOnly && !bShare);
    bool bPrev = bNewDelIsDel;
    bNewDelIsDel = pMethod != nullptr;
    if (bPrev != bNewDelIsDel && nMode == All)
    {
        OUString aBtnText( bNewDelIsDel ? IDEResId(RID_STR_BTNDEL).toString() : IDEResId(RID_STR_BTNNEW).toString() );
        m_pDelButton->SetText( aBtnText );
    }

    if (nMode == Recording)
    {
        // save button
        m_pRunButton->Enable(!bProtected && !bReadOnly && !bShare);
        // new library button
        m_pNewLibButton->Enable(!bShare);
        // new module button
        m_pNewModButton->Enable(!bProtected && !bReadOnly && !bShare);
    }
}


IMPL_LINK_NOARG(MacroChooser, MacroDoubleClickHdl, SvTreeListBox*, bool)
{
    StoreMacroDescription();
    if (nMode == Recording)
    {
        SbMethod* pMethod = GetMacro();
        if ( pMethod && !QueryReplaceMacro( pMethod->GetName(), this ) )
            return false;
    }

    EndDialog(Macro_OkRun);
    return false;
}

IMPL_LINK( MacroChooser, MacroSelectHdl, SvTreeListBox *, pBox, void )
{
    // Is also called if deselected!
    // Two function calls in every SelectHdl because
    // there's no separate DeselectHDL.
    // So find out if select or deselect:
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
    {
        UpdateFields();
        CheckButtons();
    }
}

IMPL_LINK( MacroChooser, BasicSelectHdl, SvTreeListBox *, pBox, void )
{
    // Is also called if deselected!
    // Two function calls in every SelectHdl because
    // there's no separate DeselectHDL.
    // So find out if select or deselect:
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return;

    SbModule* pModule = m_pBasicBox->FindModule( m_pBasicBox->GetCurEntry() );

    m_pMacroBox->Clear();
    if ( pModule )
    {
        m_pMacrosInTxt->SetText( m_aMacrosInTxtBaseStr + " " + pModule->GetName() );

        // The macros should be called in the same order that they
        // are written down in the module.

        map< sal_uInt16, SbMethod* > aMacros;
        size_t nMacroCount = pModule->GetMethods()->Count();
        for ( size_t iMeth = 0; iMeth  < nMacroCount; iMeth++ )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pModule->GetMethods()->Get( iMeth ));
            if( pMethod->IsHidden() )
                continue;
            DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
            sal_uInt16 nStart, nEnd;
            pMethod->GetLineRange( nStart, nEnd );
            aMacros.insert( map< sal_uInt16, SbMethod*>::value_type( nStart, pMethod ) );
        }

        m_pMacroBox->SetUpdateMode(false);
        for ( map< sal_uInt16, SbMethod* >::iterator it = aMacros.begin(); it != aMacros.end(); ++it )
            m_pMacroBox->InsertEntry( (*it).second->GetName() );
        m_pMacroBox->SetUpdateMode(true);

        if ( m_pMacroBox->GetEntryCount() )
        {
            SvTreeListEntry* pEntry = m_pMacroBox->GetEntry( 0 );
            DBG_ASSERT( pEntry, "Entry ?!" );
            m_pMacroBox->SetCurEntry( pEntry );
        }
    }

    UpdateFields();
    CheckButtons();
}


IMPL_LINK_NOARG( MacroChooser, EditModifyHdl, Edit&, void )
{
    // select the module in which the macro is put at Neu (new),
    // if BasicManager or Lib is selecting
    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    if ( pCurEntry )
    {
        sal_uInt16 nDepth = m_pBasicBox->GetModel()->GetDepth( pCurEntry );
        if ( ( nDepth == 1 ) && ( m_pBasicBox->IsEntryProtected( pCurEntry ) ) )
        {
            // then put to the respective Std-Lib...
            SvTreeListEntry* pManagerEntry = m_pBasicBox->GetModel()->GetParent( pCurEntry );
            pCurEntry = m_pBasicBox->GetModel()->FirstChild( pManagerEntry );
        }
        if ( nDepth < 2 )
        {
            SvTreeListEntry* pNewEntry = pCurEntry;
            while ( pCurEntry && ( nDepth < 2 ) )
            {
                pCurEntry = m_pBasicBox->FirstChild( pCurEntry );
                if ( pCurEntry )
                {
                    pNewEntry = pCurEntry;
                    nDepth = m_pBasicBox->GetModel()->GetDepth( pCurEntry );
                }
            }
            SaveSetCurEntry( *m_pBasicBox, pNewEntry );
        }
        if ( m_pMacroBox->GetEntryCount() )
        {
            OUString aEdtText( m_pMacroNameEdit->GetText() );
            bool bFound = false;
            for ( sal_uLong n = 0; n < m_pMacroBox->GetEntryCount(); n++ )
            {
                SvTreeListEntry* pEntry = m_pMacroBox->GetEntry( n );
                DBG_ASSERT( pEntry, "Entry ?!" );
                if ( m_pMacroBox->GetEntryText( pEntry ).equalsIgnoreAsciiCase( aEdtText ) )
                {
                    SaveSetCurEntry(*m_pMacroBox, pEntry);
                    bFound = true;
                    break;
                }
            }
            if ( !bFound )
            {
                SvTreeListEntry* pEntry = m_pMacroBox->FirstSelected();
                // if the entry exists ->Select ->Description...
                if ( pEntry )
                    m_pMacroBox->Select( pEntry, false );
            }
        }
    }

    CheckButtons();
}


IMPL_LINK( MacroChooser, ButtonHdl, Button *, pButton, void )
{
    // apart from New/Record the Description is done by LoseFocus
    if (pButton == m_pRunButton)
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
                    ScopedVclPtrInstance<MessageDialog>(this, IDEResId(RID_STR_CANNOTRUNMACRO), VclMessageType::Warning)->Execute();
                    return;
                }
            }
        }
        else if (nMode == Recording )
        {
            if ( !IsValidSbxName(m_pMacroNameEdit->GetText()) )
            {
                ScopedVclPtrInstance<MessageDialog>(this, IDEResId(RID_STR_BADSBXNAME))->Execute();
                m_pMacroNameEdit->SetSelection( Selection( 0, m_pMacroNameEdit->GetText().getLength() ) );
                m_pMacroNameEdit->GrabFocus();
                return;
            }

            SbMethod* pMethod = GetMacro();
            if ( pMethod && !QueryReplaceMacro( pMethod->GetName(), this ) )
                return;
        }

        EndDialog(Macro_OkRun);
    }
    else if (pButton == m_pCloseButton)
    {
        StoreMacroDescription();
        EndDialog(Macro_Close);
    }
    else if ((pButton == m_pEditButton) || (pButton == m_pDelButton))
    {
        SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
        EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        OUString aLib( aDesc.GetLibName() );
        OUString aMod( aDesc.GetName() );
        // extract the module name from the string like "Sheet1 (Example1)"
        if( aDesc.GetLibSubName() == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) )
        {
            sal_Int32 nIndex = 0;
            aMod = aMod.getToken( 0, ' ', nIndex );
        }
        OUString aSub( aDesc.GetMethodName() );
        SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLib, aMod, aSub, OUString() );
        if (pButton == m_pEditButton)
        {
            SvTreeListEntry* pEntry = m_pMacroBox->FirstSelected();
            if ( pEntry )
                aInfoItem.SetMethod( m_pMacroBox->GetEntryText( pEntry ) );
            StoreMacroDescription();
            SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
            SfxGetpApp()->ExecuteSlot( aRequest );

            if (SfxDispatcher* pDispatcher = GetDispatcher())
            {
                pDispatcher->ExecuteList(SID_BASICIDE_EDITMACRO,
                        SfxCallMode::ASYNCHRON, { &aInfoItem });
            }
            EndDialog(Macro_Edit);
        }
        else
        {
            if ( bNewDelIsDel )
            {
                DeleteMacro();
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                {
                    pDispatcher->ExecuteList( SID_BASICIDE_UPDATEMODULESOURCE,
                                  SfxCallMode::SYNCHRON, { &aInfoItem });
                }
                CheckButtons();
                UpdateFields();
                //if ( m_pMacroBox->GetCurEntry() )    // OV-Bug ?
                //  m_pMacroBox->Select( m_pMacroBox->GetCurEntry() );
            }
            else
            {
                if ( !IsValidSbxName(m_pMacroNameEdit->GetText()) )
                {
                    ScopedVclPtrInstance<MessageDialog>(this, IDEResId(RID_STR_BADSBXNAME))->Execute();
                    m_pMacroNameEdit->SetSelection( Selection( 0, m_pMacroNameEdit->GetText().getLength() ) );
                    m_pMacroNameEdit->GrabFocus();
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
                    EndDialog(Macro_New);
                }
            }
        }
    }
    else if (pButton == m_pAssignButton)
    {
        SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
        EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        OUString aLib( aDesc.GetLibName() );
        OUString aMod( aDesc.GetName() );
        OUString aSub( m_pMacroNameEdit->GetText() );
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
    else if (pButton == m_pNewLibButton)
    {
        SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
        EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        createLibImpl( static_cast<vcl::Window*>( this ), aDocument, nullptr, m_pBasicBox );
    }
    else if (pButton == m_pNewModButton)
    {
        SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
        EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        OUString aLibName( aDesc.GetLibName() );
        OUString aModName;
        createModImpl( static_cast<vcl::Window*>( this ), aDocument,
            *m_pBasicBox, aLibName, aModName, true );
    }
    else if (pButton == m_pOrganizeButton)
    {
        StoreMacroDescription();

        EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(m_pBasicBox->FirstSelected());
        VclPtrInstance< OrganizeDialog > pDlg( this, 0, aDesc );
        sal_uInt16 nRet = pDlg->Execute();
        pDlg.reset();

        if ( nRet ) // not only closed
        {
            EndDialog(Macro_Edit);
            return;
        }

        Shell* pShell = GetShell();
        if ( pShell && pShell->IsAppBasicModified() )
            bForceStoreBasic = true;

        m_pBasicBox->UpdateEntries();
    }
}


void MacroChooser::UpdateFields()
{
    SvTreeListEntry*    pMacroEntry = m_pMacroBox->GetCurEntry();

    m_pMacroNameEdit->SetText( "" );
    if ( pMacroEntry )
        m_pMacroNameEdit->SetText( m_pMacroBox->GetEntryText( pMacroEntry ) );
}

void MacroChooser::SetMode (Mode nM)
{
    nMode = nM;
    switch (nMode)
    {
        case All:
        {
            m_pRunButton->SetText(IDEResId(RID_STR_RUN).toString());
            EnableButton(*m_pDelButton, true);
            EnableButton(*m_pOrganizeButton, true);
            break;
        }

        case ChooseOnly:
        {
            m_pRunButton->SetText(IDEResId(RID_STR_CHOOSE).toString());
            EnableButton(*m_pDelButton, false);
            EnableButton(*m_pOrganizeButton, false);
            break;
        }

        case Recording:
        {
            m_pRunButton->SetText(IDEResId(RID_STR_RECORD).toString());
            EnableButton(*m_pDelButton, false);
            EnableButton(*m_pOrganizeButton, false);

            m_pAssignButton->Hide();
            m_pEditButton->Hide();
            m_pDelButton->Hide();
            m_pOrganizeButton->Hide();
            m_pMacroFromTxT->Hide();

            m_pNewLibButton->Show();
            m_pNewModButton->Show();
            m_pMacrosSaveInTxt->Show();

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
