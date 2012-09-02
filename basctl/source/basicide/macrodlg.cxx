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
#include <vcl/msgbox.hxx>

#include <macrodlg.hxx>
#include <macrodlg.hrc>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <baside2.hrc>      // ID's for Images
#include <basobj.hxx>
#include <baside3.hxx>

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>

#include <moduldlg.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbx.hxx>

#include <bastypes.hxx>
#include <sbxitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/request.hxx>

#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <map>

namespace basctl
{

using ::std::map;
using ::std::pair;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

MacroChooser::MacroChooser( Window* pParnt, bool bCreateEntries ) :
    SfxModalDialog(     pParnt, IDEResId( RID_MACROCHOOSER ) ),
    aMacroNameTxt(      this,   IDEResId( RID_TXT_MACRONAME ) ),
    aMacroNameEdit(     this,   IDEResId( RID_ED_MACRONAME ) ),
    aMacroFromTxT(      this,   IDEResId( RID_TXT_MACROFROM ) ),
    aMacrosSaveInTxt(   this,   IDEResId( RID_TXT_SAVEMACRO ) ),
    aBasicBox(          this,   IDEResId( RID_CTRL_LIB ) ),
    aMacrosInTxt(       this,   IDEResId( RID_TXT_MACROSIN ) ),
    aMacrosInTxtBaseStr(aMacrosInTxt.GetText()),
    aMacroBox(          this,   IDEResId( RID_CTRL_MACRO ) ),
    aRunButton(         this,   IDEResId( RID_PB_RUN ) ),
    aCloseButton(       this,   IDEResId( RID_PB_CLOSE ) ),
    aAssignButton(      this,   IDEResId( RID_PB_ASSIGN ) ),
    aEditButton(        this,   IDEResId( RID_PB_EDIT ) ),
    aNewDelButton(      this,   IDEResId( RID_PB_DEL ) ),
    aOrganizeButton(    this,   IDEResId( RID_PB_ORG ) ),
    aHelpButton(        this,   IDEResId( RID_PB_HELP ) ),
    aNewLibButton(      this,   IDEResId( RID_PB_NEWLIB ) ),
    aNewModButton(      this,   IDEResId( RID_PB_NEWMOD ) ),
    bNewDelIsDel(true),
    // the Sfx doesn't aske the BasicManger whether modified or not
    // => start saving in case of a change without a into the BasicIDE.
    bForceStoreBasic(false),
    nMode(All)
{
    FreeResource();

    aMacroBox.SetSelectionMode( SINGLE_SELECTION );
    aMacroBox.SetHighlightRange(); // select over the whole width

    aRunButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aAssignButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aEditButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aNewDelButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aOrganizeButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );

    // Buttons only for MacroChooser::Recording
    aNewLibButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aNewModButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aNewLibButton.Hide();       // default
    aNewModButton.Hide();       // default
    aMacrosSaveInTxt.Hide();    // default

    aMacrosInTxt.SetStyle( WB_NOMULTILINE | WB_PATHELLIPSIS );

    aMacroNameEdit.SetModifyHdl( LINK( this, MacroChooser, EditModifyHdl ) );

    aBasicBox.SetSelectHdl( LINK( this, MacroChooser, BasicSelectHdl ) );

    aMacroBox.SetDoubleClickHdl( LINK( this, MacroChooser, MacroDoubleClickHdl ) );
    aMacroBox.SetSelectHdl( LINK( this, MacroChooser, MacroSelectHdl ) );

    aBasicBox.SetMode( BROWSEMODE_MODULES );
    aBasicBox.SetStyle( WB_TABSTOP | WB_BORDER |
                        WB_HASLINES | WB_HASLINESATROOT |
                        WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                        WB_HSCROLL );

    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );

    if ( bCreateEntries )
        aBasicBox.ScanAllEntries();
}

MacroChooser::~MacroChooser()
{
    if ( bForceStoreBasic )
        SFX_APP()->SaveBasicAndDialogContainer();
}

void MacroChooser::StoreMacroDescription()
{
    EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(aBasicBox.FirstSelected());
    String aMethodName;
    SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
    if ( pEntry )
        aMethodName = aMacroBox.GetEntryText( pEntry );
    else
        aMethodName = aMacroNameEdit.GetText();
    if ( aMethodName.Len() )
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

    aBasicBox.SetCurrentEntry( aDesc );

    String aLastMacro( aDesc.GetMethodName() );
    if ( aLastMacro.Len() )
    {
        // find entry in macro box
        SvLBoxEntry* pEntry = 0;
        sal_uLong nPos = 0;
        SvLBoxEntry* pE = aMacroBox.GetEntry( nPos );
        while ( pE )
        {
            if ( aMacroBox.GetEntryText( pE ) == aLastMacro )
            {
                pEntry = pE;
                break;
            }
            pE = aMacroBox.GetEntry( ++nPos );
        }

        if ( pEntry )
            aMacroBox.SetCurEntry( pEntry );
        else
        {
            aMacroNameEdit.SetText( aLastMacro );
            aMacroNameEdit.SetSelection( Selection( 0, 0 ) );
        }
    }
}

short MacroChooser::Execute()
{
    RestoreMacroDescription();
    aRunButton.GrabFocus();

    // #104198 Check if "wrong" document is active
    SvLBoxEntry* pSelectedEntry = aBasicBox.GetCurEntry();
    EntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pSelectedEntry ) );
    const ScriptDocument& rSelectedDoc( aDesc.GetDocument() );

    // App Basic is always ok, so only check if shell was found
    if( rSelectedDoc.isDocument() && !rSelectedDoc.isActive() )
    {
        // Search for the right entry
        sal_uLong nRootPos = 0;
        SvLBoxEntry* pRootEntry = aBasicBox.GetEntry( nRootPos );
        while( pRootEntry )
        {
            EntryDescriptor aCmpDesc( aBasicBox.GetEntryDescriptor( pRootEntry ) );
            const ScriptDocument& rCmpDoc( aCmpDesc.GetDocument() );
            if ( rCmpDoc.isDocument() && rCmpDoc.isActive() )
            {
                SvLBoxEntry* pEntry = pRootEntry;
                SvLBoxEntry* pLastValid = pEntry;
                while ( pEntry )
                {
                    pLastValid = pEntry;
                    pEntry = aBasicBox.FirstChild( pEntry );
                }
                if( pLastValid )
                    aBasicBox.SetCurEntry( pLastValid );
            }
            pRootEntry = aBasicBox.GetEntry( ++nRootPos );
        }
    }

    CheckButtons();
    UpdateFields();

    if ( StarBASIC::IsRunning() )
        aCloseButton.GrabFocus();

    Window* pPrevDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    short nRet = ModalDialog::Execute();
    // #57314# If the BasicIDE has been activated, don't reset the DefModalDialogParent to the inactive document.
    if ( Application::GetDefDialogParent() == this )
        Application::SetDefDialogParent( pPrevDlgParent );
    return nRet;
}


void MacroChooser::EnableButton( Button& rButton, bool bEnable )
{
    if ( bEnable )
    {
        if (nMode == ChooseOnly || nMode == Recording)
            rButton.Enable(&rButton == &aRunButton);
        else
            rButton.Enable();
    }
    else
        rButton.Disable();
}




SbMethod* MacroChooser::GetMacro()
{
    SbMethod* pMethod = 0;
    SbModule* pModule = aBasicBox.FindModule( aBasicBox.GetCurEntry() );
    if ( pModule )
    {
        SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
        if ( pEntry )
        {
            String aMacroName( aMacroBox.GetEntryText( pEntry ) );
            pMethod = (SbMethod*)pModule->GetMethods()->Find( aMacroName, SbxCLASS_METHOD );
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
        DBG_ASSERT( pBasic, "Basic?!" );
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
        DBG_ASSERT( pModule, "DeleteMacro: Kein Modul?!" );
        ::rtl::OUString aSource( pModule->GetSource32() );
        sal_uInt16 nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1, true );
        pModule->SetSource32( aSource );

        // update module in library
        String aLibName = pBasic->GetName();
        String aModName = pModule->GetName();
        OSL_VERIFY( aDocument.updateModule( aLibName, aModName, aSource ) );

        SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
        DBG_ASSERT( pEntry, "DeleteMacro: Entry ?!" );
        aMacroBox.GetModel()->Remove( pEntry );
        bForceStoreBasic = true;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    SbMethod* pMethod = 0;
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
    ScriptDocument aDocument( aDesc.GetDocument() );
    OSL_ENSURE( aDocument.isAlive(), "MacroChooser::CreateMacro: no document!" );
    if ( !aDocument.isAlive() )
        return NULL;

    String aLibName( aDesc.GetLibName() );

    if ( !aLibName.Len() )
        aLibName = rtl::OUString("Standard");

    aDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    ::rtl::OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ) );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
        xModLibContainer->loadLibrary( aOULibName );
    Reference< script::XLibraryContainer > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ) );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
        xDlgLibContainer->loadLibrary( aOULibName );

    BasicManager* pBasMgr = aDocument.getBasicManager();
    StarBASIC* pBasic = pBasMgr ? pBasMgr->GetLib( aLibName ) : 0;
    if ( pBasic )
    {
        SbModule* pModule = 0;
        String aModName( aDesc.GetName() );
        if ( aModName.Len() )
        {
            // extract the module name from the string like "Sheet1 (Example1)"
            if( aDesc.GetLibSubName() == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) )
            {
                sal_uInt16 nIndex = 0;
                aModName = aModName.GetToken( 0, ' ', nIndex );
            }
            pModule = pBasic->FindModule( aModName );
        }
        else if ( pBasic->GetModules()->Count() )
            pModule = (SbModule*)pBasic->GetModules()->Get( 0 );

        if ( !pModule )
        {
            pModule = createModImpl( static_cast<Window*>( this ),
                aDocument, aBasicBox, aLibName, aModName );
        }

        String aSubName = aMacroNameEdit.GetText();
        DBG_ASSERT( !pModule || !pModule->GetMethods()->Find( aSubName, SbxCLASS_METHOD ), "Macro existiert schon!" );
        pMethod = pModule ? basctl::CreateMacro( pModule, aSubName ) : NULL;
    }

    return pMethod;
}

void MacroChooser::SaveSetCurEntry( SvTreeListBox& rBox, SvLBoxEntry* pEntry )
{
    // the edit would be killed by the highlight otherwise:

    String aSaveText( aMacroNameEdit.GetText() );
    Selection aCurSel( aMacroNameEdit.GetSelection() );

    rBox.SetCurEntry( pEntry );
    aMacroNameEdit.SetText( aSaveText );
    aMacroNameEdit.SetSelection( aCurSel );
}

void MacroChooser::CheckButtons()
{
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
    SvLBoxEntry* pMacroEntry = aMacroBox.FirstSelected();
    SbMethod* pMethod = GetMacro();

    // check, if corresponding libraries are readonly
    bool bReadOnly = false;
    sal_uInt16 nDepth = pCurEntry ? aBasicBox.GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth == 1 || nDepth == 2 )
    {
        ScriptDocument aDocument( aDesc.GetDocument() );
        ::rtl::OUString aOULibName( aDesc.GetLibName() );
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
        bool bEnable = pMethod ? true : false;
        if (nMode != ChooseOnly && StarBASIC::IsRunning())
            bEnable = false;
        EnableButton( aRunButton, bEnable );
    }

    // organising still possible?

    // Assign...
    EnableButton( aAssignButton, pMethod ? true : false );

    // Edit...
    EnableButton( aEditButton, pMacroEntry ? true : false );

    // aOrganizeButton
    EnableButton( aOrganizeButton, !StarBASIC::IsRunning() && nMode == All );

    // aNewDelButton....
    bool bProtected = aBasicBox.IsEntryProtected( pCurEntry );
    bool bShare = ( aDesc.GetLocation() == LIBRARY_LOCATION_SHARE );
    EnableButton(aNewDelButton, !StarBASIC::IsRunning() && nMode == All && !bProtected && !bReadOnly && !bShare);
    bool bPrev = bNewDelIsDel;
    bNewDelIsDel = pMethod ? true : false;
    if (bPrev != bNewDelIsDel && nMode == All)
    {
        String aBtnText( bNewDelIsDel ? IDEResId( RID_STR_BTNDEL) : IDEResId( RID_STR_BTNNEW ) );
        aNewDelButton.SetText( aBtnText );
    }

    if (nMode == Recording)
    {
        // save button
        aRunButton.Enable(!bProtected && !bReadOnly && !bShare);
        // new library button
        aNewLibButton.Enable(!bShare);
        // new module button
        aNewModButton.Enable(!bProtected && !bReadOnly && !bShare);
    }
}



IMPL_LINK_NOARG_INLINE_START(MacroChooser, MacroDoubleClickHdl)
{
    StoreMacroDescription();
    if (nMode == Recording)
    {
        SbMethod* pMethod = GetMacro();
        if ( pMethod && !QueryReplaceMacro( pMethod->GetName(), this ) )
            return 0;
    }

    EndDialog(Macro_OkRun);
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(MacroChooser, MacroDoubleClickHdl)

IMPL_LINK( MacroChooser, MacroSelectHdl, SvTreeListBox *, pBox )
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
    return 0;
}

IMPL_LINK( MacroChooser, BasicSelectHdl, SvTreeListBox *, pBox )
{
    // Is also called if deselected!
    // Two function calls in every SelectHdl because
    // there's no separate DeselectHDL.
    // So find out if select or deselect:
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return 0;

    SbModule* pModule = aBasicBox.FindModule( aBasicBox.GetCurEntry() );

    aMacroBox.Clear();
    if ( pModule )
    {
        String aStr = aMacrosInTxtBaseStr;
        aStr += rtl::OUString(" ");
        aStr += pModule->GetName();

        aMacrosInTxt.SetText( aStr );

        // The macros should be called in the same order that they
        // are written down in the module.

        map< sal_uInt16, SbMethod* > aMacros;
        size_t nMacroCount = pModule->GetMethods()->Count();
        for ( size_t iMeth = 0; iMeth  < nMacroCount; iMeth++ )
        {
            SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( iMeth );
            if( pMethod->IsHidden() )
                continue;
            DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
            sal_uInt16 nStart, nEnd;
            pMethod->GetLineRange( nStart, nEnd );
            aMacros.insert( map< sal_uInt16, SbMethod*>::value_type( nStart, pMethod ) );
        }

        aMacroBox.SetUpdateMode(false);
        for ( map< sal_uInt16, SbMethod* >::iterator it = aMacros.begin(); it != aMacros.end(); ++it )
            aMacroBox.InsertEntry( (*it).second->GetName() );
        aMacroBox.SetUpdateMode(true);

        if ( aMacroBox.GetEntryCount() )
        {
            SvLBoxEntry* pEntry = aMacroBox.GetEntry( 0 );
            DBG_ASSERT( pEntry, "Entry ?!" );
            aMacroBox.SetCurEntry( pEntry );
        }
    }

    UpdateFields();
    CheckButtons();
    return 0;
}



IMPL_LINK( MacroChooser, EditModifyHdl, Edit *, pEdit )
{
    (void)pEdit;

    // select the module in which the macro is put at Neu (new),
    // if BasicManager or Lib is selecting
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    if ( pCurEntry )
    {
        sal_uInt16 nDepth = aBasicBox.GetModel()->GetDepth( pCurEntry );
        if ( ( nDepth == 1 ) && ( aBasicBox.IsEntryProtected( pCurEntry ) ) )
        {
            // then put to the respective Std-Lib...
            SvLBoxEntry* pManagerEntry = aBasicBox.GetModel()->GetParent( pCurEntry );
            pCurEntry = aBasicBox.GetModel()->FirstChild( pManagerEntry );
        }
        if ( nDepth < 2 )
        {
            SvLBoxEntry* pNewEntry = pCurEntry;
            while ( pCurEntry && ( nDepth < 2 ) )
            {
                pCurEntry = aBasicBox.FirstChild( pCurEntry );
                if ( pCurEntry )
                {
                    pNewEntry = pCurEntry;
                    nDepth = aBasicBox.GetModel()->GetDepth( pCurEntry );
                }
            }
            SaveSetCurEntry( aBasicBox, pNewEntry );
        }
        if ( aMacroBox.GetEntryCount() )
        {
            String aEdtText( aMacroNameEdit.GetText() );
            bool bFound = false;
            for ( sal_uInt16 n = 0; n < aMacroBox.GetEntryCount(); n++ )
            {
                SvLBoxEntry* pEntry = aMacroBox.GetEntry( n );
                DBG_ASSERT( pEntry, "Entry ?!" );
                if ( aMacroBox.GetEntryText( pEntry ).CompareIgnoreCaseToAscii( aEdtText ) == COMPARE_EQUAL )
                {
                    SaveSetCurEntry( aMacroBox, pEntry );
                    bFound = true;
                    break;
                }
            }
            if ( !bFound )
            {
                SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
                // if the entry exists ->Select ->Desription...
                if ( pEntry )
                    aMacroBox.Select( pEntry, false );
            }
        }
    }

    CheckButtons();
    return 0;
}



IMPL_LINK( MacroChooser, ButtonHdl, Button *, pButton )
{
    // apart from New/Record the Description is done by LoseFocus
    if ( pButton == &aRunButton )
    {
        StoreMacroDescription();

        // #116444# check security settings before macro execution
        if (nMode == All)
        {
            SbMethod* pMethod = GetMacro();
            SbModule* pModule = pMethod ? pMethod->GetModule() : NULL;
            StarBASIC* pBasic = pModule ? (StarBASIC*)pModule->GetParent() : NULL;
            BasicManager* pBasMgr = pBasic ? FindBasicManager(pBasic) : NULL;
            if ( pBasMgr )
            {
                ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
                if ( aDocument.isDocument() && !aDocument.allowMacros() )
                {
                    WarningBox( this, WB_OK, String( IDEResId( RID_STR_CANNOTRUNMACRO ) ) ).Execute();
                    return 0;
                }
            }
        }
        else if (nMode == Recording )
        {
            if ( !IsValidSbxName(aMacroNameEdit.GetText()) )
            {
                ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
                aMacroNameEdit.SetSelection( Selection( 0, aMacroNameEdit.GetText().Len() ) );
                aMacroNameEdit.GrabFocus();
                return 0;
            }

            SbMethod* pMethod = GetMacro();
            if ( pMethod && !QueryReplaceMacro( pMethod->GetName(), this ) )
                return 0;
        }

        EndDialog(Macro_OkRun);
    }
    else if ( pButton == &aCloseButton )
    {
        StoreMacroDescription();
        EndDialog(Macro_Close);
    }
    else if ( ( pButton == &aEditButton ) || ( pButton == &aNewDelButton ) )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return 0;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        String aLib( aDesc.GetLibName() );
        String aMod( aDesc.GetName() );
        // extract the module name from the string like "Sheet1 (Example1)"
        if( aDesc.GetLibSubName() == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) )
        {
            sal_uInt16 nIndex = 0;
            aMod = aMod.GetToken( 0, ' ', nIndex );
        }
        String aSub( aDesc.GetMethodName() );
        SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLib, aMod, aSub, String() );
        if ( pButton == &aEditButton )
        {
            SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
            if ( pEntry )
                aInfoItem.SetMethod( aMacroBox.GetEntryText( pEntry ) );
            StoreMacroDescription();
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
            SFX_APP()->ExecuteSlot( aRequest );

            if (SfxDispatcher* pDispatcher = GetDispatcher())
                pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
            EndDialog(Macro_Edit);
        }
        else
        {
            if ( bNewDelIsDel )
            {
                DeleteMacro();
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                    pDispatcher->Execute( SID_BASICIDE_UPDATEMODULESOURCE,
                                          SFX_CALLMODE_SYNCHRON, &aInfoItem, 0L );
                CheckButtons();
                UpdateFields();
                //if ( aMacroBox.GetCurEntry() )    // OV-Bug ?
                //  aMacroBox.Select( aMacroBox.GetCurEntry() );
            }
            else
            {
                if ( !IsValidSbxName(aMacroNameEdit.GetText()) )
                {
                    ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
                    aMacroNameEdit.SetSelection( Selection( 0, aMacroNameEdit.GetText().Len() ) );
                    aMacroNameEdit.GrabFocus();
                    return 1;
                }
                SbMethod* pMethod = CreateMacro();
                if ( pMethod )
                {
                    aInfoItem.SetMethod( pMethod->GetName() );
                    aInfoItem.SetModule( pMethod->GetModule()->GetName() );
                    aInfoItem.SetLib( pMethod->GetModule()->GetParent()->GetName() );
                    SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                    SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                    SFX_APP()->ExecuteSlot( aRequest );

                    if (SfxDispatcher* pDispatcher = GetDispatcher())
                        pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
                    StoreMacroDescription();
                    EndDialog(Macro_New);
                }
            }
        }
    }

    else if ( pButton == &aAssignButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return 0;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        String aLib( aDesc.GetLibName() );
        String aMod( aDesc.GetName() );
        String aSub( aMacroNameEdit.GetText() );
        SbMethod* pMethod = GetMacro();
        DBG_ASSERT( pBasMgr, "BasMgr?" );
        DBG_ASSERT( pMethod, "Method?" );
        String aComment( GetInfo( pMethod ) );
        SfxMacroInfoItem aItem( SID_MACROINFO, pBasMgr, aLib, aMod, aSub, aComment );
        SfxAllItemSet Args( SFX_APP()->GetPool() );
        SfxRequest aRequest( SID_CONFIG, SFX_CALLMODE_SYNCHRON, Args );
        aRequest.AppendItem( aItem );
        SFX_APP()->ExecuteSlot( aRequest );
    }
    else if ( pButton == &aNewLibButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        createLibImpl( static_cast<Window*>( this ), aDocument, NULL, &aBasicBox );
    }
    else if ( pButton == &aNewModButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(pCurEntry);
        ScriptDocument aDocument( aDesc.GetDocument() );
        String aLibName( aDesc.GetLibName() );
        String aModName;
        createModImpl( static_cast<Window*>( this ), aDocument,
            aBasicBox, aLibName, aModName, true );
    }
    else if ( pButton == &aOrganizeButton )
    {
        StoreMacroDescription();

        EntryDescriptor aDesc = aBasicBox.GetEntryDescriptor(aBasicBox.FirstSelected());
        OrganizeDialog* pDlg = new OrganizeDialog( this, 0, aDesc );
        sal_uInt16 nRet = pDlg->Execute();
        delete pDlg;

        if ( nRet ) // not only closed
        {
            EndDialog(Macro_Edit);
            return 0;
        }

        Shell* pShell = GetShell();
        if ( pShell && pShell->IsAppBasicModified() )
            bForceStoreBasic = true;

        aBasicBox.UpdateEntries();
    }
    return 0;
}



void MacroChooser::UpdateFields()
{
    SvLBoxEntry*    pMacroEntry = aMacroBox.GetCurEntry();
    String          aEmptyStr;

    aMacroNameEdit.SetText( aEmptyStr );
    if ( pMacroEntry )
        aMacroNameEdit.SetText( aMacroBox.GetEntryText( pMacroEntry ) );
}

void MacroChooser::SetMode (Mode nM)
{
    nMode = nM;
    switch (nMode)
    {
        case All:
        {
            aRunButton.SetText( String( IDEResId( RID_STR_RUN ) ) );
            EnableButton( aNewDelButton, true );
            EnableButton( aOrganizeButton, true );
            break;
        }

        case ChooseOnly:
        {
            aRunButton.SetText( String( IDEResId( RID_STR_CHOOSE ) ) );
            EnableButton( aNewDelButton, false );
            EnableButton( aOrganizeButton, false );
            break;
        }

        case Recording:
        {
            aRunButton.SetText( String( IDEResId( RID_STR_RECORD ) ) );
            EnableButton( aNewDelButton, false );
            EnableButton( aOrganizeButton, false );

            aAssignButton.Hide();
            aEditButton.Hide();
            aNewDelButton.Hide();
            aOrganizeButton.Hide();
            aMacroFromTxT.Hide();

            aNewLibButton.Show();
            aNewModButton.Show();
            aMacrosSaveInTxt.Show();

            Point aHelpPos = aHelpButton.GetPosPixel();
            Point aHelpPosLogic = PixelToLogic( aHelpPos, MapMode(MAP_APPFONT) );
            aHelpPosLogic.Y() -= 34;
            aHelpPos = LogicToPixel( aHelpPosLogic, MapMode(MAP_APPFONT) );
            aHelpButton.SetPosPixel( aHelpPos );
            break;
        }
    }
    CheckButtons();
}

String MacroChooser::GetInfo( SbxVariable* pVar )
{
    String aComment;
    SbxInfoRef xInfo = pVar->GetInfo();
    if ( xInfo.Is() )
        aComment = xInfo->GetComment();
    return aComment;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
