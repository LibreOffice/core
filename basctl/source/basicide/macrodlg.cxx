/*************************************************************************
 *
 *  $RCSfile: macrodlg.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-23 12:05:26 $
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

#include <memory>

#ifndef _SFX_IPFRM_HXX
#include <sfx2/ipfrm.hxx>
#endif

#include <ide_pch.hxx>

#pragma hdrstop

#include <macrodlg.hxx>
#include <macrodlg.hrc>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <baside2.hrc>      // ID's fuer Imagese
#include <basobj.hxx>

#ifndef _BASIDE3_HXX
#include <baside3.hxx>
#endif

#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>

#include <moduldlg.hxx>

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#include <bastypes.hxx>
#include <sbxitem.hxx>

#ifndef _SFX_MINFITEM_HXX //autogen
#include <sfx2/minfitem.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


DECLARE_LIST( MacroList, SbMethod* );

MacroChooser::MacroChooser( Window* pParnt, BOOL bCreateEntries ) :
        SfxModalDialog(     pParnt, IDEResId( RID_MACROCHOOSER ) ),
        aMacroNameTxt(      this,   IDEResId( RID_TXT_MACRONAME ) ),
        aMacroNameEdit(     this,   IDEResId( RID_ED_MACRONAME ) ),
        aMacrosInTxt(       this,   IDEResId( RID_TXT_MACROSIN ) ),
        aMacroBox(          this,   IDEResId( RID_CTRL_MACRO ) ),
        aMacroFromTxT(      this,   IDEResId( RID_TXT_MACROFROM ) ),
        aMacrosSaveInTxt(   this,   IDEResId( RID_TXT_SAVEMACRO ) ),
        aBasicBox(          this,   IDEResId( RID_CTRL_LIB ) ),
        aRunButton(         this,   IDEResId( RID_PB_RUN ) ),
        aCloseButton(       this,   IDEResId( RID_PB_CLOSE ) ),
        aAssignButton(      this,   IDEResId( RID_PB_ASSIGN ) ),
        aEditButton(        this,   IDEResId( RID_PB_EDIT ) ),
        aNewDelButton(      this,   IDEResId( RID_PB_DEL ) ),
        aOrganizeButton(    this,   IDEResId( RID_PB_ORG ) ),
        aHelpButton(        this,   IDEResId( RID_PB_HELP ) ),
        aNewLibButton(  this,   IDEResId( RID_PB_NEWLIB ) ),
        aNewModButton(  this,   IDEResId( RID_PB_NEWMOD ) )
{
    FreeResource();

    nMode = MACROCHOOSER_ALL;
    bNewDelIsDel = TRUE;

    // Der Sfx fragt den BasicManager nicht, ob modified
    // => Speichern anschmeissen, wenn Aenderung, aber kein Sprung in
    // die BasicIDE.
    bForceStoreBasic = FALSE;

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
    aBasicBox.SetWindowBits( WB_HASLINES | WB_HASLINESATROOT |
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
    {
        SFX_APP()->SaveBasicContainer();
        SFX_APP()->SaveDialogContainer();
    }
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
        ULONG nPos = 0;
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

short __EXPORT MacroChooser::Execute()
{
    RestoreMacroDescription();
    aRunButton.GrabFocus();

    // #104198 Check if "wrong" document is active
    SvLBoxEntry* pSelectedEntry = aBasicBox.GetCurEntry();
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pSelectedEntry ) );
    SfxObjectShell* pSelectedShell = aDesc.GetShell();

    // App Basic is always ok, so only check if shell was found
    if( pSelectedShell )
    {
        SfxObjectShell* pCurShell = SfxObjectShell::Current();
        if( pCurShell != pSelectedShell )
        {
            // Search for the right entry
            ULONG nRootPos = 0;
            SvLBoxEntry* pRootEntry = aBasicBox.GetEntry( nRootPos );
            while( pRootEntry )
            {
                BasicEntryDescriptor aCmpDesc( aBasicBox.GetEntryDescriptor( pRootEntry ) );
                SfxObjectShell* pCmpShell = aCmpDesc.GetShell();
                if( pCmpShell == pCurShell )
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


void MacroChooser::EnableButton( Button& rButton, BOOL bEnable )
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
        SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
        if ( pShell )   // Muss ja nicht aus einem Document kommen...
        {
            pShell->SetModified();
            SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
            if ( pBindings )
                pBindings->Invalidate( SID_SAVEDOC );
        }

        SbModule* pModule = pMethod->GetModule();
        DBG_ASSERT( pModule, "DeleteMacro: Kein Modul?!" );
        ::rtl::OUString aSource( pModule->GetSource32() );
        USHORT nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1, TRUE );
        pModule->SetSource32( aSource );

        // update module in library
        String aLibName = pBasic->GetName();
        String aModName = pModule->GetName();
        BasicIDE::UpdateModule( pShell, aLibName, aModName, aSource );

        SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
        DBG_ASSERT( pEntry, "DeleteMacro: Entry ?!" );
        aMacroBox.GetModel()->Remove( pEntry );
        bForceStoreBasic = TRUE;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    SbMethod* pMethod = 0;
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    SfxObjectShell* pShell( aDesc.GetShell() );
    String aLibName( aDesc.GetLibName() );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    if ( !BasicIDE::HasModuleLibrary( pShell, aLibName ) )
        BasicIDE::CreateModuleLibrary( pShell, aLibName );

    ::rtl::OUString aOULibName( aLibName );
    Reference< script::XLibraryContainer > xModLibContainer = BasicIDE::GetModuleLibraryContainer( pShell );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
        xModLibContainer->loadLibrary( aOULibName );
    Reference< script::XLibraryContainer > xDlgLibContainer = BasicIDE::GetDialogLibraryContainer( pShell );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
        xDlgLibContainer->loadLibrary( aOULibName );

    BasicManager* pBasMgr = pShell ? pShell->GetBasicManager() : SFX_APP()->GetBasicManager();
    StarBASIC* pBasic = pBasMgr ? pBasMgr->GetLib( aLibName ) : 0;
    if ( pBasic )
    {
        SbModule* pModule = 0;
        String aModName( aDesc.GetName() );
        if ( aModName.Len() )
            pModule = pBasic->FindModule( aModName );
        else if ( pBasic->GetModules()->Count() )
            pModule = (SbModule*)pBasic->GetModules()->Get( 0 );

        if ( !pModule )
        {
            pModule = createModImpl( static_cast<Window*>( this ),
                pShell, aBasicBox, aLibName, aModName );
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
    BOOL bReadOnly = FALSE;
    USHORT nDepth = pCurEntry ? aBasicBox.GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth == 1 || nDepth == 2 )
    {
        SfxObjectShell* pShell( aDesc.GetShell() );
        ::rtl::OUString aOULibName( aDesc.GetLibName() );
        Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
        if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
        {
            bReadOnly = TRUE;
        }
    }

    if ( nMode != MACROCHOOSER_RECORDING )
    {
        // Run...
        BOOL bEnable = pMethod ? TRUE : FALSE;
        if ( ( nMode != MACROCHOOSER_CHOOSEONLY ) && StarBASIC::IsRunning() )
            bEnable = FALSE;
        EnableButton( aRunButton, bEnable );
    }

    // Organisieren immer moeglich ?

    // Assign...
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    EnableButton( aAssignButton, pMethod && pViewFrame ? TRUE : FALSE );

    // Edit...
    EnableButton( aEditButton, pMacroEntry ? TRUE : FALSE );

    // aOrganizeButton
    EnableButton( aOrganizeButton, !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ));

    // aNewDelButton....
    bool bProtected = aBasicBox.IsEntryProtected( pCurEntry );
    bool bShare = ( aDesc.GetLocation() == LIBRARY_LOCATION_SHARE );
    EnableButton( aNewDelButton,
        !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ) && !bProtected && !bReadOnly && !bShare );
    BOOL bPrev = bNewDelIsDel;
    bNewDelIsDel = pMethod ? TRUE : FALSE;
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
        MacroList aMacros;
        USHORT nMacros = pModule->GetMethods()->Count();
        USHORT nMethod;
        for ( nMethod = 0; nMethod  < nMacros; nMethod++ )
        {
            SbMethod* pMethod = (SbMethod*)pModule->GetMethods()->Get( nMethod );
            DBG_ASSERT( pMethod, "Methode nicht gefunden! (NULL)" );
            ULONG nPos = LIST_APPEND;
            // Eventuell weiter vorne ?
            USHORT nStart, nEnd;
            pMethod->GetLineRange( nStart, nEnd );
            for ( ULONG n = 0; n < aMacros.Count(); n++ )
            {
                USHORT nS, nE;
                SbMethod* pM = aMacros.GetObject( n );
                DBG_ASSERT( pM, "Macro nicht in Liste ?!" );
                pM->GetLineRange( nS, nE );
                if ( nS > nStart )
                {
                    nPos = n;
                    break;
                }
            }
            aMacros.Insert( pMethod, nPos );
        }

        aMacroBox.SetUpdateMode( FALSE );
        for ( nMethod = 0; nMethod < nMacros; nMethod++ )
            aMacroBox.InsertEntry( aMacros.GetObject( nMethod )->GetName() );
        aMacroBox.SetUpdateMode( TRUE );

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
    // Das Modul, in dem bei Neu das Macro landet, selektieren,
    // wenn BasicManager oder Lib selektiert.
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    if ( pCurEntry )
    {
        USHORT nDepth = aBasicBox.GetModel()->GetDepth( pCurEntry );
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
            BOOL bFound = FALSE;
            for ( USHORT n = 0; n < aMacroBox.GetEntryCount(); n++ )
            {
                SvLBoxEntry* pEntry = aMacroBox.GetEntry( n );
                DBG_ASSERT( pEntry, "Entry ?!" );
                if ( aMacroBox.GetEntryText( pEntry ).CompareIgnoreCaseToAscii( aEdtText ) == COMPARE_EQUAL )
                {
                    SaveSetCurEntry( aMacroBox, pEntry );
                    bFound = TRUE;
                    break;
                }
            }
            if ( !bFound )
            {
                SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
                // Wenn es den Eintrag gibt ->Select ->Desription...
                if ( pEntry )
                    aMacroBox.Select( pEntry, FALSE );
            }
        }
    }

    CheckButtons();
    return 0;
}



IMPL_LINK( MacroChooser, ButtonHdl, Button *, pButton )
{
    // ausser bei New/Record wird die Description durch LoseFocus uebernommen.
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();

    // #108283# check if view frame is a SfxInPlaceFrame
    SfxDispatcher* pDispatcher = ( pViewFrame && !pViewFrame->ISA( SfxInPlaceFrame ) ) ? pViewFrame->GetDispatcher() : NULL;

    if ( pButton == &aRunButton )
    {
        StoreMacroDescription();

        // #116444# check security settings before macro execution
        if ( nMode == MACROCHOOSER_ALL )
        {
            SbMethod* pMethod = GetMacro();
            if ( pMethod )
            {
                SbModule* pModule = pMethod->GetModule();
                if ( pModule )
                {
                    StarBASIC* pBasic = (StarBASIC*)pModule->GetParent();
                    if ( pBasic )
                    {
                        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
                        if ( pBasMgr )
                        {
                            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
                            if ( pShell )
                            {
                                pShell->AdjustMacroMode( String() );
                                if ( pShell->GetMacroMode() == ::com::sun::star::document::MacroExecMode::NEVER_EXECUTE )
                                {
                                    WarningBox( this, WB_OK, String( IDEResId( RID_STR_CANNOTRUNMACRO ) ) ).Execute();
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if ( nMode == MACROCHOOSER_RECORDING )
        {
            BOOL bValid = BasicIDE::IsValidSbxName( aMacroNameEdit.GetText() );
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
        SfxObjectShell* pShell( aDesc.GetShell() );
        BasicManager* pBasMgr = pShell ? pShell->GetBasicManager() : SFX_APP()->GetBasicManager();
        String aLib( aDesc.GetLibName() );
        String aMod( aDesc.GetName() );
        String aSub( aDesc.GetMethodName() );
        SfxMacroInfoItem aInfoItem( SID_BASICIDE_ARG_MACROINFO, pBasMgr, aLib, aMod, aSub, String() );
        if ( pButton == &aEditButton )
        {
            SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
            if ( pEntry )
                aInfoItem.SetMethod( aMacroBox.GetEntryText( pEntry ) );
            StoreMacroDescription();
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
            }
            else
            {
                SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                SFX_APP()->ExecuteSlot( aRequest );
            }
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
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
                pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
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
                BOOL bValid = BasicIDE::IsValidSbxName( aMacroNameEdit.GetText() );
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
                    if ( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
                    }
                    else
                    {
                        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
                        SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
                        SFX_APP()->ExecuteSlot( aRequest );
                    }
                    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                    pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                    pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
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
        SfxObjectShell* pShell( aDesc.GetShell() );
        BasicManager* pBasMgr = pShell ? pShell->GetBasicManager() : SFX_APP()->GetBasicManager();
        String aLib( aDesc.GetLibName() );
        String aMod( aDesc.GetName() );
        String aSub( aMacroNameEdit.GetText() );
        SbMethod* pMethod = GetMacro();
        DBG_ASSERT( pBasMgr, "BasMgr?" );
        DBG_ASSERT( pMethod, "Method?" );
        String aComment( GetInfo( pMethod ) );
        SfxMacroInfoItem aItem( SID_MACROINFO, pBasMgr, aLib, aMod, aSub, aComment );
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_CONFIG,
                                  SFX_CALLMODE_SYNCHRON, &aItem, 0L );
        }
        else
        {
            SfxAllItemSet Args( SFX_APP()->GetPool() );
            SfxRequest aRequest( SID_CONFIG, SFX_CALLMODE_SYNCHRON, Args );
            aRequest.AppendItem( aItem );
            SFX_APP()->ExecuteSlot( aRequest );
        }
        // Wenn jetzt ein FloatingWindow vom Config-Dlg hochgezogen wurde,
        // muss dieser modale Dlg verschwinden:
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxChildWindow* pChildWin = pCurFrame ? pCurFrame->GetChildWindow(SID_CUSTOMIZETOOLBOX) : NULL;
        if ( pChildWin )
            EndDialog( MACRO_CLOSE );
    }
    else if ( pButton == &aNewLibButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
        SfxObjectShell* pShell( aDesc.GetShell() );
        createLibImpl( static_cast<Window*>( this ), pShell, NULL, &aBasicBox );
    }
    else if ( pButton == &aNewModButton )
    {
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
        SfxObjectShell* pShell( aDesc.GetShell() );
        String aLibName( aDesc.GetLibName() );
        String aModName;
        createModImpl( static_cast<Window*>( this ), pShell,
            aBasicBox, aLibName, aModName, true );
    }
    else if ( pButton == &aOrganizeButton )
    {
        StoreMacroDescription();

        BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( aBasicBox.FirstSelected() ) );
        OrganizeDialog* pDlg = new OrganizeDialog( this, 0, aDesc );
        USHORT nRet = pDlg->Execute();
        delete pDlg;

        if ( nRet ) // Nicht einfach nur geschlossen
        {
            EndDialog( MACRO_EDIT );
            return 0;
        }

        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        if ( pIDEShell && pIDEShell->IsAppBasicModified() )
            bForceStoreBasic = TRUE;

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

void MacroChooser::SetMode( USHORT nM )
{
    nMode = nM;
    if ( nMode == MACROCHOOSER_ALL )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_RUN ) ) );
        EnableButton( aNewDelButton, TRUE );
        EnableButton( aOrganizeButton, TRUE );
    }
    else if ( nMode == MACROCHOOSER_CHOOSEONLY )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_CHOOSE ) ) );
        EnableButton( aNewDelButton, FALSE );
        EnableButton( aOrganizeButton, FALSE );
    }
    else if ( nMode == MACROCHOOSER_RECORDING )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_RECORD ) ) );
        EnableButton( aNewDelButton, FALSE );
        EnableButton( aOrganizeButton, FALSE );

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

