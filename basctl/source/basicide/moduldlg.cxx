/*************************************************************************
 *
 *  $RCSfile: moduldlg.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 12:39:42 $
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

#include <moduldlg.hrc>
#include <moduldlg.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <bastypes.hxx>
#include <basobj.hxx>
#include <baside2.hrc>
#include <sbxitem.hxx>
#include <iderdll.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRYARYCONTAINER2_HPP_
#include <com/sun/star/script/XLibraryContainer2.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINERPASSWORD_HPP_
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


ExtBasicTreeListBox::ExtBasicTreeListBox( Window* pParent, const ResId& rRes )
    : BasicTreeListBox( pParent, rRes )
{
}



ExtBasicTreeListBox::~ExtBasicTreeListBox()
{
}

BOOL __EXPORT ExtBasicTreeListBox::EditingEntry( SvLBoxEntry* pEntry, Selection& )
{
    BOOL bRet = FALSE;

    if ( pEntry )
    {
        USHORT nDepth = GetModel()->GetDepth( pEntry );

        if ( nDepth == 2 )
        {
            SvLBoxEntry* pLibEntry = GetParent( pEntry );
            BasicManager* pBasMgr = BasicIDE::FindBasicManager( GetEntryText( GetParent( pLibEntry ) ) );
            if ( pBasMgr )
            {
                SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
                ::rtl::OUString aOULibName( GetEntryText( pLibEntry ) );
                Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
                Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
                if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                        ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) ) )
                {
                    // allow editing only for libraries, which are not readonly
                    bRet = TRUE;
                }
            }
        }
    }

    return bRet;
}

BOOL __EXPORT ExtBasicTreeListBox::EditedEntry( SvLBoxEntry* pEntry, const String& rNewText )
{
    BOOL bValid = BasicIDE::IsValidSbxName( rNewText );
    String aCurText( GetEntryText( pEntry ) );
    if ( bValid && ( aCurText != rNewText ) )
    {
        SbxItem aSbxItem = GetSbxItem( pEntry );
        aSbxItem.SetName( rNewText );
        SfxObjectShell* pShell = aSbxItem.GetShell();
        String aLibName = aSbxItem.GetLibName();
        USHORT nType = aSbxItem.GetType();

        try
        {
            if ( nType == BASICIDE_TYPE_MODULE )
            {
                BasicIDE::RenameModule( pShell, aLibName, aCurText, rNewText );
            }
            else if ( nType == BASICIDE_TYPE_DIALOG )
            {
                BasicIDE::RenameDialog( pShell, aLibName, aCurText, rNewText );
            }

            BasicIDE::MarkDocShellModified( pShell );
        }
        catch ( container::ElementExistException& )
        {
            ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
            return FALSE;
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }

        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_SBXRENAMED,
                                  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
        }

        // OV-Bug?!
        SetEntryText( pEntry, rNewText );
        SetCurEntry( pEntry );
        SetCurEntry( pEntry );
        Select( pEntry, FALSE );
        Select( pEntry );       // damit Handler gerufen wird => Edit updaten
    }
    if ( !bValid )
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
    return bValid;
}


DragDropMode __EXPORT ExtBasicTreeListBox::NotifyStartDrag( TransferDataContainer& rData, SvLBoxEntry* pEntry )
{
    DragDropMode nMode = SV_DRAGDROP_NONE;

    if ( pEntry )
    {
        USHORT nDepth = GetModel()->GetDepth( pEntry );

        if ( nDepth == 2 )
        {
            nMode = SV_DRAGDROP_CTRL_COPY;

            SvLBoxEntry* pLibEntry = GetParent( pEntry );
            BasicManager* pBasMgr = BasicIDE::FindBasicManager( GetEntryText( GetParent( pLibEntry ) ) );
            if ( pBasMgr )
            {
                SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
                ::rtl::OUString aOULibName( GetEntryText( pLibEntry ) );
                Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
                Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
                if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                        ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) ) )
                {
                    // allow MOVE mode only for libraries, which are not readonly
                    nMode |= SV_DRAGDROP_CTRL_MOVE;
                }
            }
        }
    }

    return nMode;
}


BOOL __EXPORT ExtBasicTreeListBox::NotifyAcceptDrop( SvLBoxEntry* pEntry )
{
    // don't drop on a BasicManager (nDepth == 0)
    USHORT nDepth = pEntry ? GetModel()->GetDepth( pEntry ) : 0;
    BOOL bValid = nDepth ? TRUE : FALSE;

    // don't drop in the same library
    SvLBoxEntry* pSelected = FirstSelected();
    if ( ( nDepth == 1 ) && ( pEntry == GetParent( pSelected ) ) )
        bValid = FALSE;
    else if ( ( nDepth == 2 ) && ( GetParent( pEntry ) == GetParent( pSelected ) ) )
        bValid = FALSE;

    // don't drop on a library, which is not loaded, readonly or password protected
    if ( bValid && ( nDepth == 1 || nDepth == 2) )
    {
        SvLBoxEntry* pLibEntry = 0;
        if ( nDepth == 1 )
            pLibEntry = pEntry;
        else if ( nDepth == 2 )
            pLibEntry = GetParent( pEntry );

        BasicManager* pBasMgr = BasicIDE::FindBasicManager( GetEntryText( GetParent( pLibEntry ) ) );
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            ::rtl::OUString aOULibName( GetEntryText( pLibEntry ) );

            // check if module library is not loaded, readonly or password protected
            Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
            if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
            {
                if ( !xModLibContainer->isLibraryLoaded( aOULibName ) )
                    bValid = FALSE;

                if ( xModLibContainer->isLibraryReadOnly( aOULibName ) )
                    bValid = FALSE;

                Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    bValid = FALSE;
            }

            // check if dialog library is not loaded or readonly
            Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
            if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
            {
                if ( !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
                    bValid = FALSE;

                if ( xDlgLibContainer->isLibraryReadOnly( aOULibName ) )
                    bValid = FALSE;
            }
        }
        else
            bValid = FALSE;
    }

    // check, if module/dialog with this name is already existing in target library
    if ( bValid && ( nDepth == 1 || nDepth == 2 ) )
    {
        // get target parent
        SvLBoxEntry* pNewParent;
        if ( nDepth == 1 )
        {
            pNewParent = pEntry;
        }
        else if ( nDepth == 2 )
        {
            pNewParent = GetParent( pEntry );
        }

        // get target basic
        SbxVariable* pVar = FindVariable( pNewParent ); // parent is Basic
        DBG_ASSERT( pVar && pVar->ISA( StarBASIC ), "Parent ist kein Basic!" );
        StarBASIC* pDestBasic = (StarBASIC*)pVar;

        // get target shell and target library name
        String aDestLibName = pDestBasic->GetName();
        SfxObjectShell* pDestShell = 0;
        BasicManager* pDestBasMgr = BasicIDE::FindBasicManager( pDestBasic );
        if ( pDestBasMgr )
            pDestShell = BasicIDE::FindDocShell( pDestBasMgr );

        // a module/dialog is copied/moved
        SbxItem aSbxItem = GetSbxItem( pSelected );

        // get source module/dialog name
        String aSourceName = aSbxItem.GetName();

        // module/dialog already existing?
        if ( ( aSbxItem.GetType() == BASICIDE_TYPE_MODULE && BasicIDE::HasModule( pDestShell, aDestLibName, aSourceName ) ) ||
             ( aSbxItem.GetType() == BASICIDE_TYPE_DIALOG && BasicIDE::HasDialog( pDestShell, aDestLibName, aSourceName ) ) )
        {
            bValid = FALSE;
        }
    }

    return bValid;
}


BOOL __EXPORT ExtBasicTreeListBox::NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos )
{
    return NotifyCopyingMoving( pTarget, pEntry,
                                    rpNewParent, rNewChildPos, TRUE );
}


BOOL __EXPORT ExtBasicTreeListBox::NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos )
{
//  return FALSE;   // Wie kopiere ich ein SBX ?!
    return NotifyCopyingMoving( pTarget, pEntry,
                                    rpNewParent, rNewChildPos, FALSE );
}


BOOL __EXPORT ExtBasicTreeListBox::NotifyCopyingMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos, BOOL bMove )
{
    DBG_ASSERT( pEntry, "Kein Eintrag?" );  // Hier ASS ok, sollte nicht mit
    DBG_ASSERT( pTarget, "Kein Ziel?" );    // NULL (ganz vorne) erreicht werden
    USHORT nDepth = GetModel()->GetDepth( pTarget );
    DBG_ASSERT( nDepth, "Tiefe?" );
    if ( nDepth == 1 )
    {
        // Target = Basic => Modul/Dialog unter das Basic haengen...
        rpNewParent = pTarget;
        rNewChildPos = 0;
    }
    else if ( nDepth == 2 )
    {
        // Target = Modul/Dialog => Modul/Dialog unter das uebergeordnete Basic haengen...
        rpNewParent = GetParent( pTarget );
        rNewChildPos = GetModel()->GetRelPos( pTarget ) + 1;
    }

    USHORT nDestPos = (USHORT)rNewChildPos; // evtl. anpassen...

    // get target basic
    SbxVariable* pVar = FindVariable( rpNewParent );    // parent is Basic
    DBG_ASSERT( pVar && pVar->ISA( StarBASIC ), "Parent ist kein Basic!" );
    StarBASIC* pDestBasic = (StarBASIC*)pVar;

    // get target shell and target library name
    String aDestLibName = pDestBasic->GetName();
    SfxObjectShell* pDestShell = 0;
    BasicManager* pDestBasMgr = BasicIDE::FindBasicManager( pDestBasic );
    if ( pDestBasMgr )
        pDestShell = BasicIDE::FindDocShell( pDestBasMgr );

    // get sbx item
    SbxItem aSbxItem = GetSbxItem( FirstSelected() );

    // get source shell, library name and module/dialog name
    SfxObjectShell* pSourceShell = aSbxItem.GetShell();
    String aSourceLibName = aSbxItem.GetLibName();
    String aSourceName = aSbxItem.GetName();
    USHORT nType = aSbxItem.GetType();

    // get source basic
    BasicManager* pSourceBasMgr;
    if ( pSourceShell )
        pSourceBasMgr = pSourceShell->GetBasicManager();
    else
        pSourceBasMgr = SFX_APP()->GetBasicManager();

    StarBASIC* pSourceBasic = 0;
    if ( pSourceBasMgr )
        pSourceBasic = pSourceBasMgr->GetLib( aSourceLibName );
    DBG_ASSERT( pSourceBasic, "Woher kommt das Object?" );

    // get dispatcher
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;

    if ( bMove )    // move
    {
        // remove source module/dialog window
        if ( pSourceBasic != pDestBasic )
        {
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_SBXDELETED,
                                      SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }

        try
        {
            if ( nType == BASICIDE_TYPE_MODULE )    // module
            {
                // get module
                ::rtl::OUString aModule = BasicIDE::GetModule( pSourceShell, aSourceLibName, aSourceName );

                // remove module from source library
                BasicIDE::RemoveModule( pSourceShell, aSourceLibName, aSourceName );
                BasicIDE::MarkDocShellModified( pSourceShell );

                // insert module into target library
                BasicIDE::InsertModule( pDestShell, aDestLibName, aSourceName, aModule );
                BasicIDE::MarkDocShellModified( pDestShell );
            }
            else if ( nType == BASICIDE_TYPE_DIALOG )   // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP( BasicIDE::GetDialog( pSourceShell, aSourceLibName, aSourceName ) );

                if ( xISP.is() )
                {
                    // remove dialog from source library
                    BasicIDE::RemoveDialog( pSourceShell, aSourceLibName, aSourceName );
                    BasicIDE::MarkDocShellModified( pSourceShell );

                    // insert dialog into target library
                    BasicIDE::InsertDialog( pDestShell, aDestLibName, aSourceName, xISP );
                    BasicIDE::MarkDocShellModified( pDestShell );
                }
            }
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
    else    // copy
    {
        try
        {
            if ( nType == BASICIDE_TYPE_MODULE )    // module
            {
                // get module
                ::rtl::OUString aModule = BasicIDE::GetModule( pSourceShell, aSourceLibName, aSourceName );

                // insert module into target library
                BasicIDE::InsertModule( pDestShell, aDestLibName, aSourceName, aModule );
                BasicIDE::MarkDocShellModified( pDestShell );
            }
            else if ( nType == BASICIDE_TYPE_DIALOG )   // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP( BasicIDE::GetDialog( pSourceShell, aSourceLibName, aSourceName ) );

                if ( xISP.is() )
                {
                    // insert dialog into target library
                    BasicIDE::InsertDialog( pDestShell, aDestLibName, aSourceName, xISP );
                    BasicIDE::MarkDocShellModified( pDestShell );
                }
            }
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

    // create target module/dialog window
    if ( pSourceBasic != pDestBasic )
    {
        // set sbxitem to target lib
        aSbxItem.SetShell( pDestShell );
        aSbxItem.SetLibName( aDestLibName );

        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
        }
    }

    return 2;   // Aufklappen...
}


OrganizeDialog::OrganizeDialog( Window* pParent )
    :   TabDialog( pParent, IDEResId( RID_TD_ORGANIZE ) ),
        aTabCtrl( this, IDEResId( RID_TC_ORGANIZE ) )
{
    FreeResource();
    aTabCtrl.SetActivatePageHdl( LINK( this, OrganizeDialog, ActivatePageHdl ) );
    aTabCtrl.SetCurPageId( RID_TP_MOD );
    ActivatePageHdl( &aTabCtrl );

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }
}

void OrganizeDialog::SetCurrentModule( const String& rMacroDescr )
{
    TabPage* pTP = aTabCtrl.GetTabPage( RID_TP_MOD );
    if ( pTP )
        ((ObjectPage*)pTP)->SetCurrentModule( rMacroDescr );
}

__EXPORT OrganizeDialog::~OrganizeDialog()
{
    for ( USHORT i = 0; i < aTabCtrl.GetPageCount(); i++ )
        delete aTabCtrl.GetTabPage( aTabCtrl.GetPageId( i ) );
};

short OrganizeDialog::Execute()
{
    Window* pPrevDlgParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    short nRet = TabDialog::Execute();
    Application::SetDefDialogParent( pPrevDlgParent );
    return nRet;
}


IMPL_LINK( OrganizeDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    USHORT nId = pTabCtrl->GetCurPageId();
    // Wenn TabPage noch nicht erzeugt wurde, dann erzeugen
    if ( !pTabCtrl->GetTabPage( nId ) )
    {
        TabPage* pNewTabPage = 0;
        switch ( nId )
        {
            case RID_TP_MOD:
            {
                pNewTabPage = new ObjectPage( pTabCtrl );
                ((ObjectPage*)pNewTabPage)->SetTabDlg( this );
            }
            break;
            case RID_TP_LIB:
            {
                pNewTabPage = new LibPage( pTabCtrl );
                ((LibPage*)pNewTabPage)->SetTabDlg( this );
            }
            break;
            default:    DBG_ERROR( "PageHdl: Unbekannte ID!" );
        }
        DBG_ASSERT( pNewTabPage, "Keine Page!" );
        pTabCtrl->SetTabPage( nId, pNewTabPage );
    }
    return 0;
}




ObjectPage::ObjectPage( Window * pParent ) :
        TabPage(        pParent,IDEResId( RID_TP_MODULS ) ),
        aLibText(       this,   IDEResId( RID_STR_LIB ) ),
        aBasicBox(      this,   IDEResId( RID_TRLBOX ) ),
//      aEdit(          this,   IDEResId( RID_EDIT ) ),
        aEditButton(    this,   IDEResId( RID_PB_EDIT ) ),
        aCloseButton(   this,   IDEResId( RID_PB_CLOSE ) ),
        aNewModButton(  this,   IDEResId( RID_PB_NEWMOD ) ),
        aNewDlgButton(  this,   IDEResId( RID_PB_NEWDLG ) ),
        aDelButton(     this,   IDEResId( RID_PB_DELETE ) )
{
    FreeResource();
    pTabDlg = 0;

    aEditButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aNewModButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aNewDlgButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aBasicBox.SetSelectHdl( LINK( this, ObjectPage, BasicBoxHighlightHdl ) );

//  aEdit.SetModifyHdl( LINK( this, ObjectPage, EditModifyHdl ) );

    aBasicBox.SetDragDropMode( SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY );
    aBasicBox.EnableInplaceEditing( TRUE );

    aBasicBox.SetMode( BROWSEMODE_MODULES | BROWSEMODE_OBJS );
    aBasicBox.SetWindowBits( WB_HASLINES );

    aEditButton.GrabFocus();
    CheckButtons();
}

void __EXPORT ObjectPage::ActivatePage()
{
    aBasicBox.Clear();
    aBasicBox.ScanAllBasics();
    aBasicBox.ExpandAllTrees();

    ImplMarkCurrentModule();
}

void ObjectPage::ImplMarkCurrentModule()
{
    if ( aCurEntryDescr.Len() )
    {
        SvLBoxEntry* pEntry = FindMostMatchingEntry( aBasicBox, aCurEntryDescr );
        if ( pEntry )
            aBasicBox.SetCurEntry( pEntry );
    }
}

void __EXPORT ObjectPage::DeactivatePage()
{
    SvLBoxEntry* pEntry = aBasicBox.GetCurEntry();
    if ( pEntry )
        aCurEntryDescr = CreateEntryDescription( aBasicBox, pEntry );
}

void ObjectPage::CheckButtons()
{
    BOOL bReadOnly = FALSE;
    BOOL bEnableNew = FALSE;

//  String aEditText( aEdit.GetText() );

    // Der Name muss nur im aktuellen Basic/Lib eindeutig sein
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    USHORT nDepth = pCurEntry ? aBasicBox.GetModel()->GetDepth( pCurEntry ) : 0;

    // enable/disable edit button
    if ( nDepth == 2 )
        aEditButton.Enable();
    else
        aEditButton.Disable();

    // check, if corresponding libraries are readonly
    if ( nDepth == 1 || nDepth == 2 )
    {
        SvLBoxEntry* pLibEntry = 0;
        if ( nDepth == 1 )
            pLibEntry = pCurEntry;
        else if ( nDepth == 2)
            pLibEntry = aBasicBox.GetParent( pCurEntry );
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( aBasicBox.GetEntryText( aBasicBox.GetParent( pLibEntry ) ) );
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            ::rtl::OUString aOULibName( aBasicBox.GetEntryText( pLibEntry ) );
            Reference< script::XLibraryContainer2 > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
            if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                 ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
            {
                bReadOnly = TRUE;
            }
        }
    }

    // enable/disable new module/dialog buttons
    if ( bReadOnly )
    {
        aNewModButton.Disable();
        aNewDlgButton.Disable();
    }
    else
    {
        aNewModButton.Enable();
        aNewDlgButton.Enable();
    }

    SvLBoxEntry* pEntry = pCurEntry;
    while ( pEntry && ( ((BasicEntry*)pEntry->GetUserData())->GetType() != OBJTYPE_LIB ) )
        pEntry = aBasicBox.GetParent( pEntry );

/*
    if ( pEntry && aEditText.Len() )
    {
        String aLibText = aBasicBox.GetEntryText( pEntry );
        if ( ( pEntry != pCurEntry ) || ( aLibText != aEditText ) )
            bEnableNew = TRUE;
        pEntry = aBasicBox.FirstChild( pEntry );
        while ( pEntry )
        {
            String aEntryText = aBasicBox.GetEntryText( pEntry );
            if ( aEntryText == aEditText )
            {
                bEnableNew = FALSE;
                break;
            }
            pEntry = aBasicBox.NextSibling( pEntry );
        }
    }
*/
    if ( bEnableNew )
    {
        aDelButton.Disable();
    }
    else
    {
        if ( pCurEntry )
        {
            BYTE nType = ((BasicEntry*)pCurEntry->GetUserData())->GetType();
            if ( !bReadOnly && ( ( nType == OBJTYPE_OBJECT ) || ( nType == OBJTYPE_MODULE ) ) )
                aDelButton.Enable();
            else
                aDelButton.Disable();
        }
    }
}

IMPL_LINK( ObjectPage, BasicBoxHighlightHdl, BasicTreeListBox *, pBox )
{
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return 0;

/*
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    if ( pCurEntry )
    {
        USHORT nDepth = aBasicBox.GetModel()->GetDepth( pCurEntry );
        if ( nDepth == 2 )
            aEdit.SetText( aBasicBox.GetEntryText( pCurEntry ) );
        else
            aEdit.SetText( String() );
    }
*/
    CheckButtons();
    return 0;
}


/*
IMPL_LINK_INLINE_START( ObjectPage, EditModifyHdl, Edit *, pEdit )
{
    CheckButtons();
    return 0;
}
IMPL_LINK_INLINE_END( ObjectPage, EditModifyHdl, Edit *, pEdit )
*/

IMPL_LINK( ObjectPage, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aEditButton )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        SfxDispatcher* pDispatcher = ( pViewFrame && !pViewFrame->ISA( SfxInPlaceFrame ) ) ? pViewFrame->GetDispatcher() : NULL;
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
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        if ( aBasicBox.GetModel()->GetDepth( pCurEntry ) == 2 )
        {
            SbxItem aSbxItem = aBasicBox.GetSbxItem( pCurEntry );
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_SHOWSBX, SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }
        else    // Nur Lib selektiert
        {
            DBG_ASSERT( aBasicBox.GetModel()->GetDepth( pCurEntry ) == 1, "Kein LibEntry?!" );
            String aLib( aBasicBox.GetEntryText( pCurEntry ) );
            String aBasMgr( aBasicBox.GetEntryText( aBasicBox.GetParent( pCurEntry ) ) );
            String aLibAndMgr( CreateMgrAndLibStr( aBasMgr, aLib ) );
            SfxStringItem aLibName( SID_BASICIDE_ARG_LIBNAME, aLibAndMgr );
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_LIBSELECTED, SFX_CALLMODE_ASYNCHRON, &aLibName, 0L );
            }
        }
        EndTabDialog( 1 );
    }
    else if ( pButton == &aNewModButton )
        NewModule();
    else if ( pButton == &aNewDlgButton )
        NewDialog();
    else if ( pButton == &aDelButton )
        DeleteCurrent();
    else if ( pButton == &aCloseButton )
        EndTabDialog( 0 );

    return 0;
}



StarBASIC* ObjectPage::GetSelectedBasic()
{
    StarBASIC* pBasic = 0;
    String aLibName, aModOrDlg, aSub;
    BasicManager* pBasMgr = aBasicBox.GetSelectedSbx( aLibName, aModOrDlg, aSub );
    DBG_ASSERT( pBasMgr, "Kein BasicManager!" );
    SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii("Standard");

    if ( aLibName.Len() )
    {
        BOOL bOK = TRUE;
        ::rtl::OUString aOULibName( aLibName );

        // check, if the module library is not loaded
        Reference< script::XLibraryContainer > xModLibContainer( BasicIDE::GetModuleLibraryContainer( pShell ), UNO_QUERY );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
        {
            // check password
            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
            {
                String aPassword;
                bOK = QueryPassword( xModLibContainer, aLibName, aPassword );
            }

            // load library
            if ( bOK )
                xModLibContainer->loadLibrary( aOULibName );
        }

        // check, if the dialog library is not loaded
        Reference< script::XLibraryContainer > xDlgLibContainer( BasicIDE::GetDialogLibraryContainer( pShell ), UNO_QUERY );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
        {
            // load library
            if ( bOK )
                xDlgLibContainer->loadLibrary( aOULibName );
        }

        // get Basic
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
        {
            if ( bOK )
            {
                pBasic = pBasMgr->GetLib( aLibName );
                DBG_ASSERT( pBasic, "ObjectPage::GetSelectedBasic: Error loading library!" );
            }
        }
    }

    return pBasic;
}

/*
BOOL ObjectPage::UseEditText()
{
    SvLBoxEntry* pSel = aBasicBox.FirstSelected();
    if ( pSel )
    {
        String aSelected( aBasicBox.GetEntryText( pSel ) );
        if ( aSelected == aEdit.GetText() )
            return FALSE;
    }
    return TRUE;
}
*/

void ObjectPage::NewModule()
{
    StarBASIC* pLib = GetSelectedBasic();
    DBG_ASSERT( pLib, "Keine Lib!" );
    if ( pLib )
    {
        String aLibName = pLib->GetName();
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pLib );
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            String aModName;
            createModImpl( static_cast<Window*>( this ), pShell,
                           pLib, aBasicBox, aLibName, aModName, true );
        }
    }
}

void ObjectPage::NewDialog()
{
    StarBASIC* pLib = GetSelectedBasic();
    DBG_ASSERT( pLib, "Keine Lib!" );
    if ( pLib )
    {
        String aLibName = pLib->GetName();
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pLib );
        if ( pBasMgr )
        {
            SfxObjectShell* pShell = BasicIDE::FindDocShell( pBasMgr );
            std::auto_ptr< NewObjectDialog > xNewDlg(
                new NewObjectDialog(this, NEWOBJECTMODE_DLG, true));
            xNewDlg->SetObjectName( BasicIDE::CreateDialogName( pShell, aLibName ) );

            if (xNewDlg->Execute() != 0)
            {
                String aDlgName( xNewDlg->GetObjectName() );
                if (aDlgName.Len() == 0)
                    aDlgName = BasicIDE::CreateDialogName(pShell, aLibName);

                try
                {
                    Reference< io::XInputStreamProvider > xISP( BasicIDE::CreateDialog( pShell, aLibName, aDlgName ) );
                    SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pShell, aLibName, aDlgName, BASICIDE_TYPE_DIALOG );
                    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                    if( pDispatcher )
                    {
                        pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                              SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
                    }
                    SvLBoxEntry* pLibEntry = aBasicBox.FindLibEntry( pLib );
                    DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
                    SvLBoxEntry * pEntry = aBasicBox.insertEntry(
                        aDlgName, IMGID_OBJECT, pLibEntry, false,
                        std::auto_ptr< BasicEntry >(new BasicEntry(
                                                        OBJTYPE_OBJECT)));
                    DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                    aBasicBox.SetCurEntry( pEntry );
                    aBasicBox.Select( aBasicBox.GetCurEntry() );        // OV-Bug?!
                }
                catch ( container::ElementExistException& )
                {
                    ErrorBox( this, WB_OK | WB_DEF_OK,
                            String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
                }
                catch ( container::NoSuchElementException& e )
                {
                    ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
                    DBG_ERROR( aBStr.GetBuffer() );
                }
            }
        }
    }
}

void ObjectPage::DeleteCurrent()
{
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    DBG_ASSERT( pCurEntry, "Kein aktueller Eintrag!" );
    SbxItem aSbxItem = aBasicBox.GetSbxItem( pCurEntry );
    SfxObjectShell* pShell = aSbxItem.GetShell();
    String aLibName = aSbxItem.GetLibName();
    String aName = aSbxItem.GetName();
    USHORT nType = aSbxItem.GetType();

    if ( ( nType == BASICIDE_TYPE_MODULE && QueryDelModule( aName, this ) ) ||
         ( nType == BASICIDE_TYPE_DIALOG && QueryDelDialog( aName, this ) ) )
    {
        aBasicBox.GetModel()->Remove( pCurEntry );
        if ( aBasicBox.GetCurEntry() )  // OV-Bug ?
            aBasicBox.Select( aBasicBox.GetCurEntry() );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            pDispatcher->Execute( SID_BASICIDE_SBXDELETED,
                                  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
        }

        try
        {
            if ( nType == BASICIDE_TYPE_MODULE )
            {
                BasicIDE::RemoveModule( pShell, aLibName, aName );
            }
            else if ( nType == BASICIDE_TYPE_DIALOG )
            {
                BasicIDE::RemoveDialog( pShell, aLibName, aName );
            }

            BasicIDE::MarkDocShellModified( pShell );
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
    }
}



void ObjectPage::EndTabDialog( USHORT nRet )
{
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( nRet );
}


LibDialog::LibDialog( Window* pParent )
    : ModalDialog( pParent, IDEResId( RID_DLG_LIBS ) ),
        aOKButton(      this, IDEResId( RID_PB_OK ) ),
        aCancelButton(  this, IDEResId( RID_PB_CANCEL ) ),
        aStorageName(   this, IDEResId( RID_FT_STORAGENAME ) ),
        aLibBox(        this, IDEResId( RID_CTRL_LIBS ) ),
        aFixedLine(     this, IDEResId( RID_FL_OPTIONS ) ),
        aReferenceBox(  this, IDEResId( RID_CB_REF ) ),
        aReplaceBox(    this, IDEResId( RID_CB_REPL ) )
{
    SetText( String( IDEResId( RID_STR_APPENDLIBS ) ) );
    FreeResource();
}


LibDialog::~LibDialog()
{
}

void LibDialog::SetStorageName( const String& rName )
{
    String aName( IDEResId( RID_STR_FILENAME ) );
    aName += rName;
    aStorageName.SetText( aName );
}

// Helper function
SbModule* createModImpl( Window* pWin, SfxObjectShell* pShell, StarBASIC* pLib,
    BasicTreeListBox& rBasicBox, const String& aLibName, String aModName, bool bMain )
{
    SbModule* pModule = NULL;

    std::auto_ptr< NewObjectDialog > xNewDlg(
        new NewObjectDialog( pWin, NEWOBJECTMODE_MOD, true ) );
    if ( aModName.Len() == 0 )
        aModName = BasicIDE::CreateModuleName( pShell, aLibName );
    xNewDlg->SetObjectName( aModName );

    if (xNewDlg->Execute() != 0)
    {
        if ( xNewDlg->GetObjectName().Len() )
            aModName = xNewDlg->GetObjectName();

        try
        {
            ::rtl::OUString aModule = BasicIDE::CreateModule( pShell, aLibName, aModName, bMain );
            pModule = pLib->FindModule( aModName );

            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, pShell, aLibName, aModName, BASICIDE_TYPE_MODULE );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                      SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
            SvLBoxEntry* pLibEntry = rBasicBox.FindLibEntry( pLib );
            DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
            SvLBoxEntry * pEntry = rBasicBox.insertEntry(
                aModName, IMGID_MODULE, pLibEntry, false,
                std::auto_ptr< BasicEntry >(new BasicEntry(
                                                OBJTYPE_MODULE)));
            DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
            rBasicBox.SetCurEntry( pEntry );
            rBasicBox.Select( rBasicBox.GetCurEntry() );        // OV-Bug?!
        }
        catch ( container::ElementExistException& )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                    String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
        }
        catch ( container::NoSuchElementException& e )
        {
            ByteString aBStr( String(e.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aBStr.GetBuffer() );
        }
    }
    return pModule;
}




