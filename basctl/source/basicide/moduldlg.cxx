/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: moduldlg.cxx,v $
 * $Revision: 1.34 $
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


#include <moduldlg.hrc>
#include <moduldlg.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <bastypes.hxx>
#include <baside3.hxx>
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
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <xmlscript/xmldlg_imexp.hxx>

#include "localizationmgr.hxx"
#include <basic/sbx.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::resource;


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
            BasicEntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
            ScriptDocument aDocument( aDesc.GetDocument() );
            ::rtl::OUString aOULibName( aDesc.GetLibName() );
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                    ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) ) )
            {
                // allow editing only for libraries, which are not readonly
                bRet = TRUE;
            }
        }
    }

    return bRet;
}

BOOL __EXPORT ExtBasicTreeListBox::EditedEntry( SvLBoxEntry* pEntry, const String& rNewText )
{
    BOOL bValid = BasicIDE::IsValidSbxName( rNewText );
    if ( !bValid )
    {
        ErrorBox( this, WB_OK | WB_DEF_OK, String( IDEResId( RID_STR_BADSBXNAME ) ) ).Execute();
        return FALSE;
    }

    String aCurText( GetEntryText( pEntry ) );
    if ( aCurText == rNewText )
        // nothing to do
        return TRUE;

    BasicEntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
    ScriptDocument aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "ExtBasicTreeListBox::EditedEntry: no document!" );
    if ( !aDocument.isValid() )
        return FALSE;
    String aLibName( aDesc.GetLibName() );
    BasicEntryType eType( aDesc.GetType() );

    bool bSuccess = ( eType == OBJ_TYPE_MODULE )
        ?   BasicIDE::RenameModule( this, aDocument, aLibName, aCurText, rNewText )
        :   BasicIDE::RenameDialog( this, aDocument, aLibName, aCurText, rNewText );

    if ( !bSuccess )
        return FALSE;

    BasicIDE::MarkDocumentModified( aDocument );

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, rNewText, ConvertType( eType ) );
        pDispatcher->Execute( SID_BASICIDE_SBXRENAMED,
                                SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
    }

    // OV-Bug?!
    SetEntryText( pEntry, rNewText );
    SetCurEntry( pEntry );
    SetCurEntry( pEntry );
    Select( pEntry, FALSE );
    Select( pEntry );       // damit Handler gerufen wird => Edit updaten

    return TRUE;
}


DragDropMode __EXPORT ExtBasicTreeListBox::NotifyStartDrag( TransferDataContainer&, SvLBoxEntry* pEntry )
{
    DragDropMode nMode_ = SV_DRAGDROP_NONE;

    if ( pEntry )
    {
        USHORT nDepth = GetModel()->GetDepth( pEntry );
        if ( nDepth == 2 )
        {
            nMode_ = SV_DRAGDROP_CTRL_COPY;
            BasicEntryDescriptor aDesc( GetEntryDescriptor( pEntry ) );
            ScriptDocument aDocument( aDesc.GetDocument() );
            ::rtl::OUString aOULibName( aDesc.GetLibName() );
            // allow MOVE mode only for libraries, which are not readonly
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
                    ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) ) )
            {
                // Only allow copy for localized libraries
                bool bAllowMove = true;
                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) )
                {
                    // Get StringResourceManager
                    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, aOULibName, TRUE ) );
                    Reference< XStringResourceManager > xSourceMgr =
                        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
                    if( xSourceMgr.is() )
                        bAllowMove = ( xSourceMgr->getLocales().getLength() == 0 );
                }
                if( bAllowMove )
                    nMode_ |= SV_DRAGDROP_CTRL_MOVE;
            }
        }
    }

    return nMode_;
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
    // or which already has a module/dialog with this name
    if ( bValid && ( nDepth == 1 || nDepth == 2 ) )
    {
        // get source module/dialog name
        BasicEntryDescriptor aSourceDesc( GetEntryDescriptor( pSelected ) );
        String aSourceName( aSourceDesc.GetName() );
        BasicEntryType eSourceType( aSourceDesc.GetType() );

        // get target shell and target library name
        BasicEntryDescriptor aDestDesc( GetEntryDescriptor( pEntry ) );
        const ScriptDocument& rDestDoc( aDestDesc.GetDocument() );
        String aDestLibName( aDestDesc.GetLibName() );
        ::rtl::OUString aOUDestLibName( aDestLibName );

        // check if module library is not loaded, readonly or password protected
        Reference< script::XLibraryContainer2 > xModLibContainer( rDestDoc.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aOUDestLibName ) )
        {
            if ( !xModLibContainer->isLibraryLoaded( aOUDestLibName ) )
                bValid = FALSE;

            if ( xModLibContainer->isLibraryReadOnly( aOUDestLibName ) )
                bValid = FALSE;

            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOUDestLibName ) && !xPasswd->isLibraryPasswordVerified( aOUDestLibName ) )
                bValid = FALSE;
        }

        // check if dialog library is not loaded or readonly
        Reference< script::XLibraryContainer2 > xDlgLibContainer( rDestDoc.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOUDestLibName ) )
        {
            if ( !xDlgLibContainer->isLibraryLoaded( aOUDestLibName ) )
                bValid = FALSE;

            if ( xDlgLibContainer->isLibraryReadOnly( aOUDestLibName ) )
                bValid = FALSE;
        }

        // check, if module/dialog with this name is already existing in target library
        if ( ( eSourceType == OBJ_TYPE_MODULE && rDestDoc.hasModule( aDestLibName, aSourceName ) ) ||
            ( eSourceType == OBJ_TYPE_DIALOG && rDestDoc.hasDialog( aDestLibName, aSourceName ) ) )
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


void BasicIDEShell::CopyDialogResources( Reference< io::XInputStreamProvider >& io_xISP,
    const ScriptDocument& rSourceDoc, const String& rSourceLibName, const ScriptDocument& rDestDoc,
    const String& rDestLibName, const String& rDlgName )
{
    if ( !io_xISP.is() )
        return;

    // Get StringResourceManager
    Reference< container::XNameContainer > xSourceDialogLib( rSourceDoc.getLibrary( E_DIALOGS, rSourceLibName, TRUE ) );
    Reference< XStringResourceManager > xSourceMgr =
        LocalizationMgr::getStringResourceFromDialogLibrary( xSourceDialogLib );
    if( !xSourceMgr.is() )
        return;
    bool bSourceLocalized = ( xSourceMgr->getLocales().getLength() > 0 );

    Reference< container::XNameContainer > xDestDialogLib( rDestDoc.getLibrary( E_DIALOGS, rDestLibName, TRUE ) );
    Reference< XStringResourceManager > xDestMgr =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDestDialogLib );
    if( !xDestMgr.is() )
        return;
    bool bDestLocalized = ( xDestMgr->getLocales().getLength() > 0 );

    if( !bSourceLocalized && !bDestLocalized )
        return;

    // create dialog model
    Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference< container::XNameContainer > xDialogModel = Reference< container::XNameContainer >( xMSF->createInstance
        ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.awt.UnoControlDialogModel" ) ) ), UNO_QUERY );
    Reference< io::XInputStream > xInput( io_xISP->createInputStream() );
    Reference< XComponentContext > xContext;
    Reference< beans::XPropertySet > xProps( xMSF, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    OSL_VERIFY( xProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext );
    ::xmlscript::importDialogModel( xInput, xDialogModel, xContext );

    if( xDialogModel.is() )
    {
        if( bSourceLocalized && bDestLocalized )
        {
            Reference< resource::XStringResourceResolver > xSourceStringResolver( xSourceMgr, UNO_QUERY );
            LocalizationMgr::copyResourceForDroppedDialog( xDialogModel, rDlgName, xDestMgr, xSourceStringResolver );
        }
        else if( bSourceLocalized )
        {
            LocalizationMgr::resetResourceForDialog( xDialogModel, xSourceMgr );
        }
        else if( bDestLocalized )
        {
            LocalizationMgr::setResourceIDsForDialog( xDialogModel, xDestMgr );
        }
        io_xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext );
    }
}


BOOL __EXPORT ExtBasicTreeListBox::NotifyCopyingMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
                        SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos, BOOL bMove )
{
    (void)pEntry;
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

    // get target shell and target library name
    BasicEntryDescriptor aDestDesc( GetEntryDescriptor( rpNewParent ) );
    const ScriptDocument& rDestDoc( aDestDesc.GetDocument() );
    String aDestLibName( aDestDesc.GetLibName() );

    // get source shell, library name and module/dialog name
    BasicEntryDescriptor aSourceDesc( GetEntryDescriptor( FirstSelected() ) );
    const ScriptDocument rSourceDoc( aSourceDesc.GetDocument() );
    String aSourceLibName( aSourceDesc.GetLibName() );
    String aSourceName( aSourceDesc.GetName() );
    BasicEntryType eType( aSourceDesc.GetType() );

    // get dispatcher
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;

    if ( bMove )    // move
    {
        // remove source module/dialog window
        if ( rSourceDoc != rDestDoc || aSourceLibName != aDestLibName )
        {
            if( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rSourceDoc, aSourceLibName, aSourceName, ConvertType( eType ) );
                pDispatcher->Execute( SID_BASICIDE_SBXDELETED,
                                      SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }

        try
        {
            if ( eType == OBJ_TYPE_MODULE ) // module
            {
                // get module
                ::rtl::OUString aModule;
                if ( rSourceDoc.getModule( aSourceLibName, aSourceName, aModule ) )
                {
                    // remove module from source library
                    if ( rSourceDoc.removeModule( aSourceLibName, aSourceName ) )
                    {
                        BasicIDE::MarkDocumentModified( rSourceDoc );

                        // insert module into target library
                        if ( rDestDoc.insertModule( aDestLibName, aSourceName, aModule ) )
                            BasicIDE::MarkDocumentModified( rDestDoc );
                    }
                }
            }
            else if ( eType == OBJ_TYPE_DIALOG )    // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP;
                if ( rSourceDoc.getDialog( aSourceLibName, aSourceName, xISP ) )
                {
                    BasicIDEShell::CopyDialogResources( xISP, rSourceDoc,
                        aSourceLibName, rDestDoc, aDestLibName, aSourceName );

                    // remove dialog from source library
                    if ( BasicIDE::RemoveDialog( rSourceDoc, aSourceLibName, aSourceName ) )
                    {
                        BasicIDE::MarkDocumentModified( rSourceDoc );

                        // insert dialog into target library
                        if ( rDestDoc.insertDialog( aDestLibName, aSourceName, xISP ) )
                            BasicIDE::MarkDocumentModified( rDestDoc );
                    }
                }
            }
        }
        catch ( uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    else    // copy
    {
        try
        {
            if ( eType == OBJ_TYPE_MODULE ) // module
            {
                // get module
                ::rtl::OUString aModule;
                if ( rSourceDoc.getModule( aSourceLibName, aSourceName, aModule ) )
                {
                    // insert module into target library
                    if ( rDestDoc.insertModule( aDestLibName, aSourceName, aModule ) )
                        BasicIDE::MarkDocumentModified( rDestDoc );
                }
            }
            else if ( eType == OBJ_TYPE_DIALOG )    // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP;
                if ( rSourceDoc.getDialog( aSourceLibName, aSourceName, xISP ) )
                {
                    BasicIDEShell::CopyDialogResources( xISP, rSourceDoc,
                        aSourceLibName, rDestDoc, aDestLibName, aSourceName );

                    // insert dialog into target library
                    if ( rDestDoc.insertDialog( aDestLibName, aSourceName, xISP ) )
                        BasicIDE::MarkDocumentModified( rDestDoc );
                }
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // create target module/dialog window
    if ( rSourceDoc != rDestDoc || aSourceLibName != aDestLibName )
    {
        if( pDispatcher )
        {
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDestDoc, aDestLibName, aSourceName, ConvertType( eType ) );
            pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
        }
    }

    return 2;   // Aufklappen...
}

OrganizeDialog::OrganizeDialog( Window* pParent, INT16 tabId, BasicEntryDescriptor& rDesc )
    :TabDialog( pParent, IDEResId( RID_TD_ORGANIZE ) )
    ,aTabCtrl( this, IDEResId( RID_TC_ORGANIZE ) )
    ,m_aCurEntry( rDesc )
{
    FreeResource();
    aTabCtrl.SetActivatePageHdl( LINK( this, OrganizeDialog, ActivatePageHdl ) );
    if( tabId == 0 )
    {
        aTabCtrl.SetCurPageId( RID_TP_MOD );
    }
    else if ( tabId == 1 )
    {
        aTabCtrl.SetCurPageId( RID_TP_DLG );
    }
    else
    {
        aTabCtrl.SetCurPageId( RID_TP_LIB );
    }

    ActivatePageHdl( &aTabCtrl );

    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
    SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
    if( pDispatcher )
    {
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
    }
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
                pNewTabPage = new ObjectPage( pTabCtrl, IDEResId( RID_TP_MODULS ), BROWSEMODE_MODULES );
                ((ObjectPage*)pNewTabPage)->SetTabDlg( this );
                ((ObjectPage*)pNewTabPage)->SetCurrentEntry( m_aCurEntry );
            }
            break;
            case RID_TP_DLG:
            {
                pNewTabPage = new ObjectPage( pTabCtrl, IDEResId( RID_TP_DLGS ), BROWSEMODE_DIALOGS );
                ((ObjectPage*)pNewTabPage)->SetTabDlg( this );
                ((ObjectPage*)pNewTabPage)->SetCurrentEntry( m_aCurEntry );
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

ObjectPage::ObjectPage( Window * pParent, const ResId& rResId, USHORT nMode ) :
        TabPage(        pParent, rResId ),
        aLibText(       this,   IDEResId( RID_STR_LIB ) ),
        aBasicBox(      this,   IDEResId( RID_TRLBOX ) ),
        aEditButton(    this,   IDEResId( RID_PB_EDIT ) ),
        aCloseButton(   this,   IDEResId( RID_PB_CLOSE ) ),
        aNewModButton(  this,   IDEResId( RID_PB_NEWMOD ) ),
        aNewDlgButton(  this,   IDEResId( RID_PB_NEWDLG ) ),
        aDelButton(     this,   IDEResId( RID_PB_DELETE ) )
{
    FreeResource();
    pTabDlg = 0;

    aEditButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aCloseButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    aBasicBox.SetSelectHdl( LINK( this, ObjectPage, BasicBoxHighlightHdl ) );

    if( nMode & BROWSEMODE_MODULES )
    {
        aNewModButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
        aNewDlgButton.Hide();
    }
    else if ( nMode & BROWSEMODE_DIALOGS )
    {
        aNewDlgButton.SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
        aNewModButton.Hide();
    }

    aBasicBox.SetDragDropMode( SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_CTRL_COPY );
    aBasicBox.EnableInplaceEditing( TRUE );
    aBasicBox.SetMode( nMode );
    aBasicBox.SetWindowBits( WB_HASLINES | WB_HASLINESATROOT |
                             WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                             WB_HSCROLL );
    aBasicBox.ScanAllEntries();

    aEditButton.GrabFocus();
    CheckButtons();
}

void ObjectPage::SetCurrentEntry( BasicEntryDescriptor& rDesc )
{
    aBasicBox.SetCurrentEntry( rDesc );
}

void __EXPORT ObjectPage::ActivatePage()
{
    aBasicBox.UpdateEntries();
}

void __EXPORT ObjectPage::DeactivatePage()
{
}

void ObjectPage::CheckButtons()
{
    // enable/disable edit button
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    USHORT nDepth = pCurEntry ? aBasicBox.GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth == 2 )
        aEditButton.Enable();
    else
        aEditButton.Disable();

    // enable/disable new module/dialog buttons
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    LibraryLocation eLocation( aDesc.GetLocation() );
    BOOL bReadOnly = FALSE;
    if ( nDepth == 1 || nDepth == 2 )
    {
        ScriptDocument aDocument( aDesc.GetDocument() );
        ::rtl::OUString aOULibName( aDesc.GetLibName() );
        Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
        if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && xModLibContainer->isLibraryReadOnly( aOULibName ) ) ||
             ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && xDlgLibContainer->isLibraryReadOnly( aOULibName ) ) )
        {
            bReadOnly = TRUE;
        }
    }
    if ( bReadOnly || eLocation == LIBRARY_LOCATION_SHARE )
    {
        aNewModButton.Disable();
        aNewDlgButton.Disable();
    }
    else
    {
        aNewModButton.Enable();
        aNewDlgButton.Enable();
    }

    // enable/disable delete button
    if ( nDepth == 2 && !bReadOnly && eLocation != LIBRARY_LOCATION_SHARE )
        aDelButton.Enable();
    else
        aDelButton.Disable();
}

IMPL_LINK( ObjectPage, BasicBoxHighlightHdl, BasicTreeListBox *, pBox )
{
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return 0;

    CheckButtons();
    return 0;
}

IMPL_LINK( ObjectPage, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aEditButton )
    {
        SfxAllItemSet aArgs( SFX_APP()->GetPool() );
        SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
        SFX_APP()->ExecuteSlot( aRequest );

        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        if ( aBasicBox.GetModel()->GetDepth( pCurEntry ) == 2 )
        {
            BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
            if ( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(),
                                  aDesc.GetName(), aBasicBox.ConvertType( aDesc.GetType() ) );
                pDispatcher->Execute( SID_BASICIDE_SHOWSBX, SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
        }
        else    // Nur Lib selektiert
        {
            DBG_ASSERT( aBasicBox.GetModel()->GetDepth( pCurEntry ) == 1, "Kein LibEntry?!" );
            ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
            SvLBoxEntry* pParentEntry = aBasicBox.GetParent( pCurEntry );
            if ( pParentEntry )
            {
                BasicDocumentEntry* pBasicDocumentEntry = (BasicDocumentEntry*)pParentEntry->GetUserData();
                if ( pBasicDocumentEntry )
                    aDocument = pBasicDocumentEntry->GetDocument();
            }
            SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( aDocument.getDocumentOrNull() ) );
            String aLibName( aBasicBox.GetEntryText( pCurEntry ) );
            SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
            if ( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_LIBSELECTED, SFX_CALLMODE_ASYNCHRON, &aDocItem, &aLibNameItem, 0L );
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

bool ObjectPage::GetSelection( ScriptDocument& rDocument, String& rLibName )
{
    bool bRet = false;

    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    rDocument = aDesc.GetDocument();
    rLibName = aDesc.GetLibName();
    if ( !rLibName.Len() )
        rLibName = String::CreateFromAscii( "Standard" );

    DBG_ASSERT( rDocument.isAlive(), "ObjectPage::GetSelection: no or dead ScriptDocument in the selection!" );
    if ( !rDocument.isAlive() )
        return false;

    // check if the module library is loaded
    BOOL bOK = TRUE;
    ::rtl::OUString aOULibName( rLibName );
    Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS  ) );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) && !xModLibContainer->isLibraryLoaded( aOULibName ) )
    {
        // check password
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
        {
            String aPassword;
            bOK = QueryPassword( xModLibContainer, rLibName, aPassword );
        }

        // load library
        if ( bOK )
            xModLibContainer->loadLibrary( aOULibName );
    }

    // check if the dialog library is loaded
    Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aOULibName ) && !xDlgLibContainer->isLibraryLoaded( aOULibName ) )
    {
        // load library
        if ( bOK )
            xDlgLibContainer->loadLibrary( aOULibName );
    }

    if ( bOK )
        bRet = true;

    return bRet;
}

void ObjectPage::NewModule()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    String aLibName;

    if ( GetSelection( aDocument, aLibName ) )
    {
        String aModName;
        createModImpl( static_cast<Window*>( this ), aDocument,
                    aBasicBox, aLibName, aModName, true );
    }
}

void ObjectPage::NewDialog()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    String aLibName;

    if ( GetSelection( aDocument, aLibName ) )
    {
        aDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

        std::auto_ptr< NewObjectDialog > xNewDlg(
            new NewObjectDialog(this, NEWOBJECTMODE_DLG, true));
        xNewDlg->SetObjectName( aDocument.createObjectName( E_DIALOGS, aLibName ) );

        if (xNewDlg->Execute() != 0)
        {
            String aDlgName( xNewDlg->GetObjectName() );
            if (aDlgName.Len() == 0)
                aDlgName = aDocument.createObjectName( E_DIALOGS, aLibName);

            if ( aDocument.hasDialog( aLibName, aDlgName ) )
            {
                ErrorBox( this, WB_OK | WB_DEF_OK,
                        String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
            }
            else
            {
                Reference< io::XInputStreamProvider > xISP;
                if ( !aDocument.createDialog( aLibName, aDlgName, xISP ) )
                    return;

                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, aDlgName, BASICIDE_TYPE_DIALOG );
                BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
                SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
                SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
                if( pDispatcher )
                {
                    pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                            SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
                }
                LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
                SvLBoxEntry* pRootEntry = aBasicBox.FindRootEntry( aDocument, eLocation );
                if ( pRootEntry )
                {
                    if ( !aBasicBox.IsExpanded( pRootEntry ) )
                        aBasicBox.Expand( pRootEntry );
                    SvLBoxEntry* pLibEntry = aBasicBox.FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
                    DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
                    if ( pLibEntry )
                    {
                        if ( !aBasicBox.IsExpanded( pLibEntry ) )
                            aBasicBox.Expand( pLibEntry );
                        SvLBoxEntry* pEntry = aBasicBox.FindEntry( pLibEntry, aDlgName, OBJ_TYPE_DIALOG );
                        if ( !pEntry )
                        {
                            pEntry = aBasicBox.AddEntry(
                                aDlgName,
                                Image( IDEResId( RID_IMG_DIALOG ) ),
                                Image( IDEResId( RID_IMG_DIALOG_HC ) ),
                                pLibEntry, false,
                                std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_DIALOG ) ) );
                            DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                        }
                        aBasicBox.SetCurEntry( pEntry );
                        aBasicBox.Select( aBasicBox.GetCurEntry() );        // OV-Bug?!
                    }
                }
            }
        }
    }
}

void ObjectPage::DeleteCurrent()
{
    SvLBoxEntry* pCurEntry = aBasicBox.GetCurEntry();
    DBG_ASSERT( pCurEntry, "Kein aktueller Eintrag!" );
    BasicEntryDescriptor aDesc( aBasicBox.GetEntryDescriptor( pCurEntry ) );
    ScriptDocument aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isAlive(), "ObjectPage::DeleteCurrent: no document!" );
    if ( !aDocument.isAlive() )
        return;
    String aLibName( aDesc.GetLibName() );
    String aName( aDesc.GetName() );
    BasicEntryType eType( aDesc.GetType() );

    if ( ( eType == OBJ_TYPE_MODULE && QueryDelModule( aName, this ) ) ||
         ( eType == OBJ_TYPE_DIALOG && QueryDelDialog( aName, this ) ) )
    {
        aBasicBox.GetModel()->Remove( pCurEntry );
        if ( aBasicBox.GetCurEntry() )  // OV-Bug ?
            aBasicBox.Select( aBasicBox.GetCurEntry() );
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, aName, aBasicBox.ConvertType( eType ) );
            pDispatcher->Execute( SID_BASICIDE_SBXDELETED,
                                  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
        }

        try
        {
            bool bSuccess = false;
            if ( eType == OBJ_TYPE_MODULE )
                bSuccess = aDocument.removeModule( aLibName, aName );
            else if ( eType == OBJ_TYPE_DIALOG )
                bSuccess = BasicIDE::RemoveDialog( aDocument, aLibName, aName );

            if ( bSuccess )
                BasicIDE::MarkDocumentModified( aDocument );
        }
        catch ( container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION();
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
SbModule* createModImpl( Window* pWin, const ScriptDocument& rDocument,
    BasicTreeListBox& rBasicBox, const String& rLibName, String aModName, bool bMain )
{
    OSL_ENSURE( rDocument.isAlive(), "createModImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return NULL;

    SbModule* pModule = NULL;

    String aLibName( rLibName );
    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );
    rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );
    if ( !aModName.Len() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    std::auto_ptr< NewObjectDialog > xNewDlg(
        new NewObjectDialog( pWin, NEWOBJECTMODE_MOD, true ) );
    xNewDlg->SetObjectName( aModName );

    if (xNewDlg->Execute() != 0)
    {
        if ( xNewDlg->GetObjectName().Len() )
            aModName = xNewDlg->GetObjectName();

        try
        {
            ::rtl::OUString sModuleCode;
            rDocument.createModule( aLibName, aModName, bMain, sModuleCode );
            BasicManager* pBasMgr = rDocument.getBasicManager();
            if ( pBasMgr )
            {
                StarBASIC* pBasic = pBasMgr->GetLib( aLibName );
                if ( pBasic )
                    pModule = pBasic->FindModule( aModName );
            }
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName, BASICIDE_TYPE_MODULE );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_SBXINSERTED,
                                      SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            }
            LibraryLocation eLocation = rDocument.getLibraryLocation( aLibName );
            SvLBoxEntry* pRootEntry = rBasicBox.FindRootEntry( rDocument, eLocation );
            if ( pRootEntry )
            {
                if ( !rBasicBox.IsExpanded( pRootEntry ) )
                    rBasicBox.Expand( pRootEntry );
                SvLBoxEntry* pLibEntry = rBasicBox.FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
                DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
                if ( pLibEntry )
                {
                    if ( !rBasicBox.IsExpanded( pLibEntry ) )
                        rBasicBox.Expand( pLibEntry );
                    SvLBoxEntry* pEntry = rBasicBox.FindEntry( pLibEntry, aModName, OBJ_TYPE_MODULE );
                    if ( !pEntry )
                    {
                        pEntry = rBasicBox.AddEntry(
                            aModName,
                            Image( IDEResId( RID_IMG_MODULE ) ),
                            Image( IDEResId( RID_IMG_MODULE_HC ) ),
                            pLibEntry, false,
                            std::auto_ptr< BasicEntry >( new BasicEntry( OBJ_TYPE_MODULE ) ) );
                        DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                    }
                    rBasicBox.SetCurEntry( pEntry );
                    rBasicBox.Select( rBasicBox.GetCurEntry() );        // OV-Bug?!
                }
            }
        }
        catch ( container::ElementExistException& )
        {
            ErrorBox( pWin, WB_OK | WB_DEF_OK,
                    String( IDEResId( RID_STR_SBXNAMEALLREADYUSED2 ) ) ).Execute();
        }
        catch ( container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return pModule;
}




