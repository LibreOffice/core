/*************************************************************************
 *
 *  $RCSfile: macrodlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-11-03 15:20:51 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#include <macrodlg.hxx>
#include <macrodlg.hrc>
#include <basidesh.hrc>
#include <baside2.hrc>      // ID's fuer Imagese
#include <basobj.hxx>

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

DECLARE_LIST( MacroList, SbMethod* );

MacroChooser::MacroChooser( Window* pParnt, BOOL bScanBasics ) :
        SfxModalDialog(     pParnt, IDEResId( RID_MACROCHOOSER ) ),
        aMacroNameTxt(      this,   IDEResId( RID_TXT_MACRONAME ) ),
        aMacroNameEdit(     this,   IDEResId( RID_ED_MACRONAME ) ),
        aMacroBox(          this,   IDEResId( RID_CTRL_MACRO ) ),
        aMacroFromTxT(      this,   IDEResId( RID_TXT_MACROFROM ) ),
        aBasicBox(          this,   IDEResId( RID_CTRL_LIB ) ),
        aRunButton(         this,   IDEResId( RID_PB_RUN ) ),
        aCancelCloseButton( this,   IDEResId( RID_PB_CANCEL ) ),
        aAssignButton(      this,   IDEResId( RID_PB_ASSIGN ) ),
        aEditButton(        this,   IDEResId( RID_PB_EDIT ) ),
        aNewDelButton(      this,   IDEResId( RID_PB_DEL ) ),
        aOrganizeButton(    this,   IDEResId( RID_PB_ORG ) ),
        aHelpButton(        this,   IDEResId( RID_PB_HELP ) ),
        aDescrTxt(          this,   IDEResId( RID_TXT_DESCRIPTION ) ),
        aDescrEdit(         this,   IDEResId( RID_ML_DESCRIPTION ) )
{
    FreeResource();

    nMode = MACROCHOOSER_ALL;
    bNewDelIsDel = TRUE;
    bCancelCloseIsCancel = TRUE;
    bAcceptDescription = TRUE;

    // Der Sfx fragt den BasicManager nicht, ob modified
    // => Speichern anschmeissen, wenn Aenderung, aber kein Sprung in
    // die BasicIDE.
    bForceStoreBasic = FALSE;

    aMacroBox.SetSelectionMode( SINGLE_SELECTION );
    aMacroBox.SetHighlightRange(); // ueber ganze Breite selektieren

    aRunButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aCancelCloseButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aAssignButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aEditButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aNewDelButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );
    aOrganizeButton.SetClickHdl( LINK( this, MacroChooser, ButtonHdl ) );

    aMacroNameEdit.SetModifyHdl( LINK( this, MacroChooser, EditModifyHdl ) );

    aBasicBox.SetSelectHdl( LINK( this, MacroChooser, BasicSelectHdl ) );

    aMacroBox.SetDoubleClickHdl( LINK( this, MacroChooser, MacroDoubleClickHdl ) );
    aMacroBox.SetSelectHdl( LINK( this, MacroChooser, MacroSelectHdl ) );

    aBasicBox.SetMode( BROWSEMODE_MODULES /* | BROWSEMODE_SUBS */ );
    aBasicBox.SetWindowBits( WB_HASLINES );

    aDescrEdit.SetGetFocusHdl( LINK( this, MacroChooser, EditGotFocusHdl ) );
    aDescrEdit.SetLoseFocusHdl( LINK( this, MacroChooser, EditLoseFocusHdl ) );
    aDescrEdit.SetModifyHdl( LINK( this, MacroChooser, DescriptionModifyHdl ) );
    aDescrEdit.SetAccHdl( LINK( this, MacroChooser, EditAccHdl ) );
    aDescrEdit.GetAccelerator().InsertItem( 1, KeyCode( KEY_ESCAPE ) );


    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }

    if ( bScanBasics )
        aBasicBox.ScanAllBasics();
}

MacroChooser::~MacroChooser()
{
    if ( bForceStoreBasic )
        SFX_APP()->SaveBasicManager();
}

void MacroChooser::StoreMacroDescription()
{
    String aLastMacroDescr( CreateEntryDescription( aBasicBox, aBasicBox.FirstSelected() ) );
    if ( aLastMacroDescr.GetTokenCount( ';' ) < 3 )
        aLastMacroDescr += ';';
    aLastMacroDescr += ';';
    SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
    if ( pEntry )
        aLastMacroDescr += aMacroBox.GetEntryText ( pEntry );
    else
        aLastMacroDescr += aMacroNameEdit.GetText();

    /*aLastMacroDescr += pEntry ? aMacroBox.GetEntryText( pEntry ) :
                                aMacroNameEdit.GetText();*/
    IDE_DLL()->GetExtraData()->GetLastMacro() = aLastMacroDescr;
}

void MacroChooser::RestoreMacroDescription()
{
    String aLastMacroDescr( IDE_DLL()->GetExtraData()->GetLastMacro() );
    if ( aLastMacroDescr.Len() )
    {
        // MacroEntry: BasMgr;Bas;Mod;Macro
        SvLBoxEntry* pEntry = FindMostMatchingEntry( aBasicBox, aLastMacroDescr );
        if ( pEntry )
            aBasicBox.SetCurEntry( pEntry );
        String aLastMacro( aLastMacroDescr.GetToken( 3, ';' ) );
        if ( aLastMacro.Len() )
        {
            pEntry = FindMostMatchingEntry( aMacroBox, aLastMacro );
            if ( pEntry )
                aMacroBox.SetCurEntry( pEntry );
            else
            {
                aMacroNameEdit.SetText( aLastMacro );
                aMacroNameEdit.SetSelection( Selection( 0, 0 ) );
            }
        }
    }
}

IMPL_LINK_INLINE_START( MacroChooser, EditGotFocusHdl, MultiLineEdit *, pEdit )
{
    bAcceptDescription = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( MacroChooser, EditGotFocusHdl, MultiLineEdit *, pEdit )

IMPL_LINK( MacroChooser, EditLoseFocusHdl, MultiLineEdit *, pEdit )
{
    if ( bAcceptDescription )
    {
        SbxVariable* pVar = GetMacro();
/*
        if ( !pVar )
        {
            SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
            BYTE nType = pCurEntry ? ((BasicEntry*)pCurEntry->GetUserData())->GetType() : 0;
            if ( ( nType == OBJTYPE_OBJECT ) || ( nType == OBJTYPE_MODULE ) ||
                 ( nType == OBJTYPE_METHOD ) || ( nType == OBJTYPE_LIB ) ||
                 ( nType == OBJTYPE_SUBOBJ ) )
            {
                pVar = aBasicBox.FindVariable( pCurEntry );
            }

        }
*/
        if ( pVar )
            SetInfo( pVar );
    }
    return 0;
}

IMPL_LINK_INLINE_START( MacroChooser, DescriptionModifyHdl, MultiLineEdit *, pEdit )
{
    CheckCancelClose();
    return 0;
}
IMPL_LINK_INLINE_END( MacroChooser, DescriptionModifyHdl, MultiLineEdit *, pEdit )



IMPL_LINK( MacroChooser, EditAccHdl, Accelerator *, pAcc )
{
    if ( pAcc->GetCurKeyCode().GetCode() == KEY_ESCAPE )
    {
        bAcceptDescription = FALSE;
        UpdateFields();
        aCancelCloseButton.GrabFocus();
    }

    return 0;
}

short __EXPORT MacroChooser::Execute()
{
    String aLastMacroDescr( IDE_DLL()->GetExtraData()->GetLastMacro() );

    aBasicBox.ExpandAllTrees();
    if ( aLastMacroDescr.Len() )
    {
        RestoreMacroDescription();
        aRunButton.GrabFocus();
    }
    else
    {
        // Erstes Macro selektieren:
        SvLBoxEntry* pEntry = aBasicBox.GetEntry( 0 );
//      SvLBoxEntry* pFirstEntry = pEntry;
        SvLBoxEntry* pLastValid = pEntry;
        while ( pEntry )
        {
            pLastValid = pEntry;
            pEntry = aBasicBox.FirstChild( pEntry );
        }
        if ( pLastValid )
        {
            // Pruefen, ob er Passwort-geschuetzt ist...
//          String aLib, aDummy1, aDummy2, aDummy3;
//          BasicManager* pBasicManager = aBasicBox.GetSbx( pLastValid, aLib, aDummy1, aDummy2, aDummy3 );
//          USHORT nLib = pBasicManager->GetLibId( aLib );
//          if ( !pBasicManager->HasPassword( nLib ) ||
//                  pBasicManager->IsPasswordVerified( nLib ) )
//          {
                aBasicBox.SetCurEntry( pLastValid );
//          }
//          else
//          {
//              aBasicBox.SetCurEntry( pFirstEntry );
//          }
        }
    }

    CheckButtons();
    UpdateFields();

    if ( StarBASIC::IsRunning() )
        aCancelCloseButton.GrabFocus();

    Window* pPrevDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    short nRet = ModalDialog::Execute();
    // #57314# Wenn die BasicIDE aktiviert wurde, dann nicht den DefModalDialogParent auf das inaktive Dokument zuruecksetzen.
    if ( Application::GetDefDialogParent() == this )
        Application::SetDefDialogParent( pPrevDlgParent );
    return nRet;
}

void MacroChooser::CheckCancelClose()
{
    if ( bCancelCloseIsCancel )
    {
        bCancelCloseIsCancel = FALSE;
        aCancelCloseButton.SetText( String( IDEResId( RID_STR_CLOSE ) ) );
    }
}

void MacroChooser::EnableButton( Button& rButton, BOOL bEnable )
{
    if ( bEnable )
    {
        if ( nMode == MACROCHOOSER_CHOOSEONLY )
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
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
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
            BasicIDE::GetBindings().Invalidate( SID_SAVEDOC );
        }

        SbModule* pModule = pMethod->GetModule();
        DBG_ASSERT( pModule, "DeleteMacro: Kein Modul?!" );
        String aSource( pModule->GetSource() );
        USHORT nStart, nEnd;
        pMethod->GetLineRange( nStart, nEnd );
        pModule->GetMethods()->Remove( pMethod );
        CutLines( aSource, nStart-1, nEnd-nStart+1, TRUE );
        pModule->SetSource( aSource );
        SvLBoxEntry* pEntry = aMacroBox.FirstSelected();
        DBG_ASSERT( pEntry, "DeleteMacro: Entry ?!" );
        aMacroBox.GetModel()->Remove( pEntry );
        bForceStoreBasic = TRUE;
    }
}

SbMethod* MacroChooser::CreateMacro()
{
    String aLib, aMod, aSub;
    BasicManager* pBasMgr = aBasicBox.GetSelectedSbx( aLib, aMod, aSub );
    aSub = aMacroNameEdit.GetText();
    DBG_ASSERT( pBasMgr, "Record/New: Kein BasicManager?" );
    StarBASIC* pBasic = aLib.Len() ? pBasMgr->GetLib( aLib ) : pBasMgr->GetLib( 0 );
    if ( !pBasic )
        pBasic = pBasMgr->GetLib( 0 );
    DBG_ASSERT( pBasic, "Record/New: Kein Basic?" );
    SbModule* pModule = 0;
    if( aMod.Len() )
        pModule = pBasic->FindModule( aMod );
    else if ( pBasic->GetModules()->Count() )
        pModule = (SbModule*)pBasic->GetModules()->Get( 0 );

    if ( !pModule )
    {
        NewObjectDialog* pNewDlg = new NewObjectDialog( this, NEWOBJECTMODE_MOD );
        pNewDlg->SetObjectName( BasicIDE::CreateModuleName( pBasic, aMod ) );
        if ( pNewDlg->Execute() )
        {
            aMod = pNewDlg->GetObjectName();
            if ( !BasicIDE::FindModule( pBasic, aMod ) )
            {
                pModule = BasicIDE::CreateModule( pBasic, aMod, FALSE );
                DBG_ASSERT( pModule , "Modul wurde nicht erzeugt!" );
            }
            else
            {
                ErrorBox( this, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
            }
        }
        delete pNewDlg;
    }

    DBG_ASSERT( !pModule || !pModule->GetMethods()->Find( aSub, SbxCLASS_METHOD ), "Macro existiert schon!" );
    SbMethod* pMethod = pModule ? BasicIDE::CreateMacro( pModule, aSub ) : NULL;
    return pMethod;
}



void MacroChooser::ScanBasic( BasicManager* pBasMgr, const String& rName )
{
    aBasicBox.ScanBasic( pBasMgr, rName );
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
    SvLBoxEntry* pMacroEntry = aMacroBox.FirstSelected();
    SbxVariable* pVar = aBasicBox.FindVariable( pCurEntry );
    SbMethod* pMethod = GetMacro();

    // Run...
    BOOL bEnable = pMethod ? TRUE : FALSE;
    if ( ( nMode != MACROCHOOSER_CHOOSEONLY ) && StarBASIC::IsRunning() )
        bEnable = FALSE;
    EnableButton( aRunButton, bEnable );

    // Organisieren immer moeglich ?

    // Assign...
    EnableButton( aAssignButton, pMethod ? TRUE : FALSE );

    // Edit...
    EnableButton( aEditButton, pMacroEntry ? TRUE : FALSE );

    // aOrganizeButton
    EnableButton( aOrganizeButton, !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ));

    // aNewDelButton....
    EnableButton( aNewDelButton,
        !StarBASIC::IsRunning() && ( nMode == MACROCHOOSER_ALL ) && !aBasicBox.IsEntryProtected( aBasicBox.GetCurEntry() ) );
    BOOL bPrev = bNewDelIsDel;
    bNewDelIsDel = pMethod ? TRUE : FALSE;
    if ( ( bPrev != bNewDelIsDel ) && ( nMode != MACROCHOOSER_CHOOSEONLY ) )
    {
        String aBtnText( bNewDelIsDel ? IDEResId( RID_STR_BTNDEL) : IDEResId( RID_STR_BTNNEW ) );
        aNewDelButton.SetText( aBtnText );
    }
}



IMPL_LINK_INLINE_START( MacroChooser, MacroDoubleClickHdl, SvTreeListBox *, EMPTYARG )
{
    StoreMacroDescription();
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
//              aDescrEdit.SetText( "" );
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
    SfxViewFrame* pCurFrame = SfxViewFrame::Current();
    DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
    SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;

    if ( pButton == &aRunButton )
    {
        StoreMacroDescription();
        EndDialog( MACRO_OK_RUN );
    }
    else if ( pButton == &aCancelCloseButton )
    {
        if ( !bCancelCloseIsCancel )
            StoreMacroDescription();
        EndDialog( MACRO_CLOSE );
    }
    else if ( ( pButton == &aEditButton ) || ( pButton == &aNewDelButton ) )
    {
        String aLib, aMod, aSub;
        BasicManager* pBasMgr = aBasicBox.GetSelectedSbx( aLib, aMod, aSub );
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
                pCurFrame = SfxViewFrame::Current();
                pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
                pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
            }
            EndDialog( MACRO_EDIT );
        }
        else
        {
            if ( bNewDelIsDel )
            {
                DeleteMacro();
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_UPDATEMODULESOURCE,
                                          SFX_CALLMODE_SYNCHRON, &aInfoItem, 0L );
                }
                CheckButtons();
                UpdateFields();
                CheckCancelClose();
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
                    SetInfo( pMethod );
                    aInfoItem.SetMethod( pMethod->GetName() );
                    aInfoItem.SetModule( pMethod->GetModule()->GetName() );
                    aInfoItem.SetLib( pMethod->GetModule()->GetParent()->GetName() );
                    if( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON );
                        pDispatcher->Execute( SID_BASICIDE_EDITMACRO, SFX_CALLMODE_ASYNCHRON, &aInfoItem, 0L );
                    }
                    StoreMacroDescription();
                    EndDialog( MACRO_NEW );
                }
            }
        }
    }
    else if ( pButton == &aAssignButton )
    {
        String aLib, aMod, aSub;
        BasicManager* pBasMgr = aBasicBox.GetSelectedSbx( aLib, aMod, aSub );
        aSub = aMacroNameEdit.GetText();
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
        // Wenn jetzt ein FloatingWindow vom Config-Dlg hochgezogen wurde,
        // muss dieser modale Dlg verschwinden:
        SfxViewFrame* pCurFrame = SfxViewFrame::Current();
        DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
        SfxChildWindow* pChildWin = pCurFrame ? pCurFrame->GetChildWindow(SID_CUSTOMIZETOOLBOX) : NULL;
        if ( pChildWin )
            EndDialog( MACRO_CLOSE );
    }
    else if ( pButton == &aOrganizeButton )
    {
        StoreMacroDescription();

        OrganizeDialog* pDlg = new OrganizeDialog( this );
        String aMacroDescr = CreateEntryDescription( aBasicBox, aBasicBox.FirstSelected() );
        pDlg->SetCurrentModule( aMacroDescr );
        USHORT nRet = pDlg->Execute();
        delete pDlg;

        if ( nRet ) // Nicht einfach nur geschlossen
        {
            EndDialog( MACRO_EDIT );
            return 0;
        }

        if ( SFX_APP()->GetBasicManager()->IsModified() )
            bForceStoreBasic = TRUE;

        aBasicBox.Clear();
        aBasicBox.SetUpdateMode( FALSE );
        aBasicBox.ScanAllBasics();
        aBasicBox.ExpandAllTrees();
        aBasicBox.SetUpdateMode( TRUE );

        RestoreMacroDescription();
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

    aDescrEdit.SetText( aEmptyStr );
    // DescrEdit immer enablen:
    // Entweder Text fuer aktuelles Macro oder fuer Aufzuzeichnendes
//  aDescrEdit.Disable();
    SbxVariable* pVar = GetMacro();
//  if ( !pVar || !GetInfo( pVar).Len() )
//      pVar = aBasicBox.FindVariable( aBasicBox.GetCurEntry() );
    if ( pVar )
    {
//      aDescrEdit.Enable();
        aDescrEdit.SetText( GetInfo( pVar ) );
    }
}

void MacroChooser::SetMode( USHORT nM )
{
    nMode = nM;
    if ( nMode == MACROCHOOSER_ALL )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_RUN ) ) );
        EnableButton( aNewDelButton, TRUE );
        EnableButton( aOrganizeButton, TRUE );
        aDescrEdit.Enable();
    }
    else if ( nMode == MACROCHOOSER_CHOOSEONLY )
    {
        aRunButton.SetText( String( IDEResId( RID_STR_CHOOSE ) ) );
        EnableButton( aNewDelButton, FALSE );
        EnableButton( aOrganizeButton, FALSE );
        aDescrEdit.Disable();
    }
    CheckButtons();
}

void MacroChooser::SetInfo( SbxVariable* pVar )
{
    SbxInfoRef xInfo = pVar->GetInfo();
    if ( !xInfo.Is() )
    {
        xInfo = new SbxInfo;
        pVar->SetInfo( xInfo );
    }
    xInfo->SetComment( aDescrEdit.GetText() );
    pVar->SetModified( TRUE );
    // Eine Method macht kein Modify auf den Parent
    StarBASIC* pBasic = BasicIDE::FindBasic( pVar );
    if ( pBasic )
    {
        pBasic->SetModified( TRUE );
        BasicIDE::MarkDocShellModified( pBasic );
    }
}

String MacroChooser::GetInfo( SbxVariable* pVar )
{
    String aComment;
    SbxInfoRef xInfo = pVar->GetInfo();
    if ( xInfo.Is() )
        aComment = xInfo->GetComment();
    return aComment;
}

