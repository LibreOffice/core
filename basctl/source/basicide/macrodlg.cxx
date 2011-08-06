/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <memory>

#include <ide_pch.hxx>


#include <macrodlg.hxx>
#include <macrodlg.hrc>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <baside2.hrc>      // ID's fuer Imagese
#include <basobj.hxx>
#include <baside3.hxx>

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>

#include <moduldlg.hxx>
#include <basic/sbx.hxx>

#include <bastypes.hxx>
#include <sbxitem.hxx>
#include <sfx2/minfitem.hxx>

#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>

#include <map>
using ::std::map;
using ::std::pair;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

MacroChooser::MacroChooser( Window* pParnt, sal_Bool bCreateEntries ) :
        SfxModalDialog(     pParnt, IDEResId( RID_MACROCHOOSER ) ),
        aMacroNameTxt(      this,   IDEResId( RID_TXT_MACRONAME ) ),
        aMacroNameEdit(     this,   IDEResId( RID_ED_MACRONAME ) ),
        aMacroFromTxT(      this,   IDEResId( RID_TXT_MACROFROM ) ),
        aMacrosSaveInTxt(   this,   IDEResId( RID_TXT_SAVEMACRO ) ),
        aBasicBox(          this,   IDEResId( RID_CTRL_LIB ) ),
        aMacrosInTxt(       this,   IDEResId( RID_TXT_MACROSIN ) ),
        aMacroBox(          this,   IDEResId( RID_CTRL_MACRO ) ),
        aRunButton(         this,   IDEResId( RID_PB_RUN ) ),
        aCloseButton(       this,   IDEResId( RID_PB_CLOSE ) ),
        aAssignButton(      this,   IDEResId( RID_PB_ASSIGN ) ),
        aEditButton(        this,   IDEResId( RID_PB_EDIT ) ),
        aNewDelButton(      this,   IDEResId( RID_PB_DEL ) ),
        aOrganizeButton(    this,   IDEResId( RID_PB_ORG ) ),
        aHelpButton(        this,   IDEResId( RID_PB_HELP ) ),
        aNewLibButton(      this,   IDEResId( RID_PB_NEWLIB ) ),
        aNewModButton(      this,   IDEResId( RID_PB_NEWMOD ) )
{
    FreeResource();

    nMode = MACROCHOOSER_ALL;
    bNewDelIsDel = sal_True;

    // Der Sfx fragt den BasicManager nicht, ob modified
    // => Speichern anschmeissen, wenn Aenderung, aber kein Sprung in
    // die BasicIDE.
    bForceStoreBasic = sal_False;

    aMacrosInTxtBaseStr = aMacrosInTxt.GetText();

    aMacroBox.SetSelectionMode( SINGLE_SELECTION );
    aMacroBox.SetHighlightRange(); // ueber ganze Breite selektieren

    aRunButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aAssignButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aEditButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aNewDelButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aOrganizeButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );

    // Buttons only for MACROCHOOSER_RECORDING
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

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }

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
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( aBasicBox.FirstSelected() ) );
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

    BasicIDEData* pData = IDE_DLL()->GetExtraData();
    if ( pData )
        pData->SetLastEntryDescriptor( aDesc );
}

void MacroChooser::RestoreMacroDescription()
{
    BasicEntryDescriptor aDesc;
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        IDEBaseWindow* pCurWin = pIDEShell->GetCurWindow();
        if ( pCurWin )
            aDesc = pCurWin->CreateEntryDescriptor();
    }
    else
    {
        BasicIDEData* pData = IDE_DLL()->GetExtraData();
        if ( pData )
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
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pSelectedEntry ) );
    const ScriptDocument& rSelectedDoc( aDesc.GetDocument() );

    // App Basic is always ok, so only check if shell was found
    if( rSelectedDoc.isDocument() && !rSelectedDoc.isActive() )
    {
        // Search for the right entry
        sal_uLong nRootPos = 0;
        SvLBoxEntry* pRootEntry = aBasicBox.GetEntry( nRootPos );
        while( pRootEntry )
        {
            BasicEntryDescriptor aCmpDesc( aBasicBox.GetEntryDescriptor( pRootEntry ) );
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
    // #57314# Wenn die BasicIDE aktiviert wurde, dann nicht den DefModalDialogParent auf das inaktive Dokument zuruecksetzen.
    if ( Application::GetDefDialogParent() == this )
        Application::SetDefDialogParent( pPrevDlgParent );
    return nRet;
}


void MacroChooser::EnableButton( Button& rButton, sal_Bool bEnable )
{
    if ( bEnable )
    {
        if ( nMode == MACROCHOOSER_CHOOSEONLY || nMode == MACROCHOOSER_RECORDING )
        {
            // Nur der RunButton kann enabled werden
            if ( &rButton == &aRunButton )
                rButton.Enable();
            else
                rButton.Disable();
        }
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
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
        }

        // Aktuelles Doc als geaendert markieren:
        StarBASIC* pBasic = BasicIDE::FindBasic( pMethod );
        DBG_ASSERT( pBasic, "Basic?!" );
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        DBG_ASSERT( pBasMgr, "BasMgr?" );
        ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
        if ( aDocument.isDocument() )    // Muss ja nicht aus einem Document kommen...
        {
            aDocument.setDocumentModified();
            SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
            if ( pBindings )
                pBindings->Invalidate( SID_SAVEDOC );
        }

        SbModule* pModule = pMethod->GetModule();
        DBG_ASSERT( pModule, "DeleteMacro: Kein Modul?!" );
        ::rtl::OUString aSource( pModule->GetSource32() );
        sal_uInt16 nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1, sal_True );
        pModule->SetSource32( aSource );

        // update module in library
        String aLibName = pBasic->GetName();
        String aModName = pModule->GetName();
        OSL_VERIFY( aDocument.updateModule( aLibName, aModName, aSource ) );

        SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
        DBG_ASSERT( pEntry, "DeleteMacro: Entry ?!" );
        aMacroBox.GetModel()->Remove( pEntry );
        bForceStoreBasic = sal_True;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    SbMethod* pMethod = 0;
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    ScriptDocument aDocument( aDesc.GetDocument() );
    OSL_ENSURE( aDocument.isAlive(), "MacroChooser::CreateMacro: no document!" );
    if ( !aDocument.isAlive() )
        return NULL;

    String aLibName( aDesc.GetLibName() );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

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
            if( aDesc.GetLibSubName().Equals( String( IDEResId( RID_STR_DOCUMENT_OBJECTS ) ) ) )
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
        pMethod = pModule ? BasicIDE::CreateMacro( pModule, aSubName ) : NULL;
    }

    return pMethod;
}

void MacroChooser::SaveSetCurEntry( SvTreeListBox& rBox, SvLBoxEntry* pEntry )
{
    // Durch das Highlight wird das Edit sonst platt gemacht:

    String aSaveText( aMacroNameEdit.GetText() );
    Selection aCurSel( aMacroNameEdit.GetSelection() );

    rBox.SetCurEntry( pEntry );
    aMacroNameEdit.SetText( aSaveText );
    aMacroNameEdit.SetSelection( aCurSel );
}

void MacroChooser::CheckButtons()
{
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    SvLBoxEntry* pMacroEntry = aMacroBox.FirstSelected();
    SbMethod* pMethod = GetMacro();

    // check, if corresponding libraries are readonly
    sal_Bool bReadOnly = sal_False;
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
            bReadOnly = sal_True;
        }
    }

    if ( nMode != MACROCHOOSER_RECORDING )
    {
        // Run...
        sal_Bool bEnable = pMethod ? sal_True : sal_False;
        if ( ( nMode != MACROCHOOSER_CHOOSEONLY ) && StarBASIC::IsRunning() )
            bEnable = sal_False;
        EnableButton( aRunButton, bEnable );
    }

    // Organisieren immer moeglich ?

    // Assign...
    EnableButton( aAssignButton, pMethod ? sal_True : sal_False );

    // Edit...
    EnableButton( aEditButton, pMacroEntry ? sal_True : sal_False );

    // aOrganizeButton
    EnableButton( aOrganizeButton, !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ));

    // aNewDelButton....
    bool bProtected = aBasicBox.IsEntryProtected( pCurEntry );
    bool bShare = ( aDesc.GetLocation() == LIBRARY_LOCATION_SHARE );
    EnableButton( aNewDelButton,
        !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ) && !bProtected && !bReadOnly && !bShare );
    sal_Bool bPrev = bNewDelIsDel;
    bNewDelIsDel = pMethod ? sal_True : sal_False;
    if ( ( bPrev != bNewDelIsDel ) && ( nMode == MACROCHOOSER_ALL ) )
    {
        String aBtnText( bNewDelIsDel ? IDEResId( RID_STR_BTNDEL) : IDEResId( RID_STR_BTNNEW ) );
        aNewDelButton.SetText( aBtnText );
    }

    if ( nMode == MACROCHOOSER_RECORDING )
    {
        // save button
        if ( !bProtected && !bReadOnly && !bShare )
            aRunButton.Enable();
        else
            aRunButton.Disable();

        // new library button
        if ( !bShare )
            aNewLibButton.Enable();
        else
            aNewLibButton.Disable();

        // new module button
        if ( !bProtected && !bReadOnly && !bShare )
            aNewModButton.Enable();
        else
            aNewModButton.Disable();
    }
}



IMPL_LINK_INLINE_START( MacroChooser, MacroDoubleClickHdl, SvTreeListBox *, EMPTYARG )
{
    StoreMacroDescription();
    if ( nMode == MACROCHOOSER_RECORDING )
    {
        SbMethod* pMethod = GetMacro();
        if ( pMethod && !QueryReplaceMacro( pMethod->GetName(), this ) )
            return 0;
    }

    EndDialog( MACRO_OK_RUN );
    return 0;
}
IMPL_LINK_INLINE_END( MacroChooser, MacroDoubleClickHdl, SvTreeListBox *, EMPTYARG )

IMPL_LINK( MacroChooser, MacroSelectHdl, SvTreeListBox *, pBox )
{
    // Wird auch gerufen, wenn Deselektiert!
    // 2 Funktionsaufrufe in jedem SelectHdl, nur weil Olli
    // keinen separatren DeselctHdl einfuehren wollte:
    // Also: Feststellen, ob Select oder Deselect:
    if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
    {
        UpdateFields();
        CheckButtons();
    }
    return 0;
}

IMPL_LINK( MacroChooser, BasicSelectHdl, SvTreeListBox *, pBox )
{
    static String aSpaceStr = String::CreateFromAscii(" ");

    // Wird auch gerufen, wenn Deselektiert!
    // 2 Funktionsaufrufe in jedem SelectHdl, nur weil Olli
    // keinen separatren DeselctHdl einfuehren wollte:
    // Also: Feststellen, ob Select oder Deselect:
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return 0;

    SbModule* pModule = aBasicBox.FindModule( aBasicBox.GetCurEntry() );

    aMacroBox.Clear();
    if ( pModule )
    {
        String aStr = aMacrosInTxtBaseStr;
        aStr += aSpaceStr;
        aStr += pModule->GetName();

        aMacrosInTxt.SetText( aStr );

        // Die Macros sollen in der Reihenfolge angezeigt werden,
        // wie sie im Modul stehen.

        map< sal_uInt16, SbMethod* > aMacros;
        size_t nMacroCount = pModule->GetMethods()->Count();
        for ( size_t iMeth = 0; iMeth  < nMacroCount; iMeth++ )
        {
            SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( iMeth );
            if( pMethod->IsHidden() )
                continue;
            DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
            // Eventuell weiter vorne ?
            sal_uInt16 nStart, nEnd;
            pMethod->GetLineRange( nStart, nEnd );
            aMacros.insert( map< sal_uInt16, SbMethod*>::value_type( nStart, pMethod ) );
        }

        aMacroBox.SetUpdateMode( sal_False );
        for ( map< sal_uInt16, SbMethod* >::iterator it = aMacros.begin(); it != aMacros.end(); ++it )
            aMacroBox.InsertEntry( (*it).second->GetName() );
        aMacroBox.SetUpdateMode( sal_True );

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

    // Das Modul, in dem bei Neu das Macro landet, selektieren,
    // wenn BasicManager oder Lib selektiert.
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    if ( pCurEntry )
    {
        sal_uInt16 nDepth = aBasicBox.GetModel()->GetDepth( pCurEntry );
        if ( ( nDepth == 1 ) && ( aBasicBox.IsEntryProtected( pCurEntry ) ) )
        {
            // Dann auf die entsprechende Std-Lib stellen...
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
            sal_Bool bFound = sal_False;
            for ( sal_uInt16 n = 0; n < aMacroBox.GetEntryCount(); n++ )
            {
                SvLBoxEntry* pEntry = aMacroBox.GetEntry( n );
                DBG_ASSERT( pEntry, "Entry ?!" );
                if ( aMacroBox.GetEntryText( pEntry ).CompareIgnoreCaseToAscii( aEdtText ) == COMPARE_EQUAL )
                {
                    SaveSetCurEntry( aMacroBox, pEntry );
                    bFound = sal_True;
                    break;
                }
            }
            if ( !bFound )
            {
                SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
                // Wenn es den Eintrag gibt ->Select ->Desription...
                if ( pEntry )
                    aMacroBox.Select( pEntry, sal_False );
            }
        }
    }

    CheckButtons();
    return 0;
}



IMPL_LINK( MacroChooser, ButtonHdl, Button *, pButton )
{
    // ausser bei New/Record wird die Description durch LoseFocus uebernommen.
    if ( pButton == &aRunButton )
    {
        StoreMacroDescription();

        // #116444# check security settings before macro execution
        if ( nMode == MACROCHOOSER_ALL )
        {
            SbMethod* pMethod = GetMacro();
            SbModule* pModule = pMethod ? pMethod->GetModule() : NULL;
            StarBASIC* pBasic = pModule ? (StarBASIC*)pModule->GetParent() : NULL;
            BasicManager* pBasMgr = pBasic ? BasicIDE::FindBasicManager( pBasic ) : NULL;
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
        else if ( nMode == MACROCHOOSER_RECORDING )
        {
            sal_Bool bValid = BasicIDE::IsValidSbxName( aMacroNameEdit.GetText() );
            if ( !bValid )
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

        EndDialog( MACRO_OK_RUN );
    }
    else if ( pButton == &aCloseButton )
    {
        StoreMacroDescription();
        EndDialog( MACRO_CLOSE );
    }
    else if ( ( pButton == &aEditButton ) || ( pButton == &aNewDelButton ) )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        DBG_ASSERT( aDocument.isAlive(), "MacroChooser::ButtonHdl: no document, or document is dead!" );
        if ( !aDocument.isAlive() )
            return 0;
        BasicManager* pBasMgr = aDocument.getBasicManager();
        String aLib( aDesc.GetLibName() );
        String aMod( aDesc.GetName() );
        // extract the module name from the string like "Sheet1 (Example1)"
        if( aDesc.GetLibSubName().Equals( String( IDEResId( RID_STR_DOCUMENT_OBJECTS ) ) ) )
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

            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
                pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
            EndDialog( MACRO_EDIT );
        }
        else
        {
            if ( bNewDelIsDel )
            {
                DeleteMacro();
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_UPDATEMODULESOURCE,
                                          SFX_CALLMODE_SYNCHRON, &aInfoItem, 0L );
                }
                CheckButtons();
                UpdateFields();
                //if ( aMacroBox.GetCurEntry() )    // OV-Bug ?
                //  aMacroBox.Select( aMacroBox.GetCurEntry() );
            }
            else
            {
                sal_Bool bValid = BasicIDE::IsValidSbxName( aMacroNameEdit.GetText() );
                if ( !bValid )
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

                    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                    if ( pDispatcher )
                        pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
                    StoreMacroDescription();
                    EndDialog( MACRO_NEW );
                }
            }
        }
    }

    else if ( pButton == &aAssignButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
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
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        createLibImpl( static_cast<Window*>( this ), aDocument, NULL, &aBasicBox );
    }
    else if ( pButton == &aNewModButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
        ScriptDocument aDocument( aDesc.GetDocument() );
        String aLibName( aDesc.GetLibName() );
        String aModName;
        createModImpl( static_cast<Window*>( this ), aDocument,
            aBasicBox, aLibName, aModName, true );
    }
    else if ( pButton == &aOrganizeButton )
    {
        StoreMacroDescription();

        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( aBasicBox.FirstSelected() ) );
        OrganizeDialog* pDlg = new OrganizeDialog( this, 0, aDesc );
        sal_uInt16 nRet = pDlg->Execute();
        delete pDlg;

        if ( nRet ) // Nicht einfach nur geschlossen
        {
            EndDialog( MACRO_EDIT );
            return 0;
        }

        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        if ( pIDEShell && pIDEShell->IsAppBasicModified() )
            bForceStoreBasic = sal_True;

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

void MacroChooser::SetMode( sal_uInt16 nM )
{
    nMode = nM;
    if ( nMode == MACROCHOOSER_ALL )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_RUN ) ) );
        EnableButton( aNewDelButton, sal_True );
        EnableButton( aOrganizeButton, sal_True );
    }
    else if ( nMode == MACROCHOOSER_CHOOSEONLY )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_CHOOSE ) ) );
        EnableButton( aNewDelButton, sal_False );
        EnableButton( aOrganizeButton, sal_False );
    }
    else if ( nMode == MACROCHOOSER_RECORDING )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_RECORD ) ) );
        EnableButton( aNewDelButton, sal_False );
        EnableButton( aOrganizeButton, sal_False );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
