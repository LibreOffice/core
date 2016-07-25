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

#include <sal/config.h>

#include "basidesh.hrc"

#include "moduldlg.hxx"
#include "localizationmgr.hxx"
#include "basidesh.hxx"

#include <basic/basmgr.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <comphelper/processfactory.hxx>
#include <o3tl/make_unique.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <tools/diagnose_ex.h>
#include <xmlscript/xmldlg_imexp.hxx>
#include <svtools/treelistentry.hxx>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::resource;


// ExtTreeListBox

ExtTreeListBox::ExtTreeListBox(vcl::Window* pParent, WinBits nStyle)
    : TreeListBox(pParent, nStyle)
{
}

VCL_BUILDER_DECL_FACTORY(ExtTreeListBox)
{
    WinBits nWinBits = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;
    rRet = VclPtr<ExtTreeListBox>::Create(pParent, nWinBits);
}

bool ExtTreeListBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    bool bRet = false;

    if ( pEntry )
    {
        sal_uInt16 nDepth = GetModel()->GetDepth( pEntry );
        if ( nDepth >= 2 )
        {
            EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
            ScriptDocument aDocument( aDesc.GetDocument() );
            OUString aLibName( aDesc.GetLibName() );
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) ) ||
                    ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) ) ) )
            {
                // allow editing only for libraries, which are not readonly
                bRet = true;
            }
        }
    }

    return bRet;
}

bool ExtTreeListBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    if ( !IsValidSbxName(rNewText) )
    {
        ScopedVclPtrInstance<MessageDialog>::Create(this, IDE_RESSTR(RID_STR_BADSBXNAME))->Execute();
        return false;
    }

    OUString aCurText( GetEntryText( pEntry ) );
    if ( aCurText == rNewText )
        // nothing to do
        return true;

    EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
    ScriptDocument aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "ExtTreeListBox::EditedEntry: no document!" );
    if ( !aDocument.isValid() )
        return false;
    OUString aLibName( aDesc.GetLibName() );
    EntryType eType = aDesc.GetType();

    bool bSuccess = eType == OBJ_TYPE_MODULE ?
        RenameModule(this, aDocument, aLibName, aCurText, rNewText) :
        RenameDialog(this, aDocument, aLibName, aCurText, rNewText);

    if ( !bSuccess )
        return false;

    MarkDocumentModified( aDocument );

    if (SfxDispatcher* pDispatcher = GetDispatcher())
    {
        SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, rNewText, ConvertType( eType ) );
        pDispatcher->ExecuteList( SID_BASICIDE_SBXRENAMED,
                            SfxCallMode::SYNCHRON, { &aSbxItem });
    }

    // OV-Bug?!
    SetEntryText( pEntry, rNewText );
    SetCurEntry( pEntry );
    SetCurEntry( pEntry );
    Select( pEntry, false );
    Select( pEntry );       // so that handler is called => update edit

    return true;
}


DragDropMode ExtTreeListBox::NotifyStartDrag( TransferDataContainer&, SvTreeListEntry* pEntry )
{
    DragDropMode nMode_ = DragDropMode::NONE;

    if ( pEntry )
    {
        sal_uInt16 nDepth = GetModel()->GetDepth( pEntry );
        if ( nDepth >= 2 )
        {
            nMode_ = DragDropMode::CTRL_COPY;
            EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
            ScriptDocument aDocument( aDesc.GetDocument() );
            OUString aLibName( aDesc.GetLibName() );
            // allow MOVE mode only for libraries, which are not readonly
            Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
            Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
            if ( !( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) ) ||
                    ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) ) ) )
            {
                // Only allow copy for localized libraries
                bool bAllowMove = true;
                if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) )
                {
                    // Get StringResourceManager
                    Reference< container::XNameContainer > xDialogLib( aDocument.getLibrary( E_DIALOGS, aLibName, true ) );
                    Reference< XStringResourceManager > xSourceMgr =
                        LocalizationMgr::getStringResourceFromDialogLibrary( xDialogLib );
                    if( xSourceMgr.is() )
                        bAllowMove = ( xSourceMgr->getLocales().getLength() == 0 );
                }
                if( bAllowMove )
                    nMode_ |= DragDropMode::CTRL_MOVE;
            }
        }
    }

    return nMode_;
}


bool ExtTreeListBox::NotifyAcceptDrop( SvTreeListEntry* pEntry )
{
    // don't drop on a BasicManager (nDepth == 0)
    sal_uInt16 nDepth = pEntry ? GetModel()->GetDepth( pEntry ) : 0;
    bool bValid = nDepth != 0;

    // don't drop in the same library
    SvTreeListEntry* pSelected = FirstSelected();
    if ( ( nDepth == 1 ) && ( pEntry == GetParent( pSelected ) ) )
        bValid = false;
    else if ( ( nDepth == 2 ) && ( GetParent( pEntry ) == GetParent( pSelected ) ) )
        bValid = false;

    // don't drop on a library, which is not loaded, readonly or password protected
    // or which already has a module/dialog with this name
    if ( bValid && ( nDepth > 0 ) )
    {
        // get source module/dialog name
        EntryDescriptor aSourceDesc = GetEntryDescriptor(pSelected);
        OUString aSourceName = aSourceDesc.GetName();
        EntryType eSourceType = aSourceDesc.GetType();

        // get target shell and target library name
        EntryDescriptor aDestDesc = GetEntryDescriptor(pEntry);
        ScriptDocument const& rDestDoc = aDestDesc.GetDocument();
        OUString aDestLibName = aDestDesc.GetLibName();

        // check if module library is not loaded, readonly or password protected
        Reference< script::XLibraryContainer2 > xModLibContainer( rDestDoc.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        if ( xModLibContainer.is() && xModLibContainer->hasByName( aDestLibName ) )
        {
            if ( !xModLibContainer->isLibraryLoaded( aDestLibName ) )
                bValid = false;

            if ( xModLibContainer->isLibraryReadOnly( aDestLibName ) )
                bValid = false;

            Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
            if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aDestLibName ) && !xPasswd->isLibraryPasswordVerified( aDestLibName ) )
                bValid = false;
        }

        // check if dialog library is not loaded or readonly
        Reference< script::XLibraryContainer2 > xDlgLibContainer( rDestDoc.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
        if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aDestLibName ) )
        {
            if ( !xDlgLibContainer->isLibraryLoaded( aDestLibName ) )
                bValid = false;

            if ( xDlgLibContainer->isLibraryReadOnly( aDestLibName ) )
                bValid = false;
        }

        // check, if module/dialog with this name is already existing in target library
        if ( ( eSourceType == OBJ_TYPE_MODULE && rDestDoc.hasModule( aDestLibName, aSourceName ) ) ||
            ( eSourceType == OBJ_TYPE_DIALOG && rDestDoc.hasDialog( aDestLibName, aSourceName ) ) )
        {
            bValid = false;
        }
    }

    return bValid;
}

TriState ExtTreeListBox::NotifyMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos )
{
    return NotifyCopyingMoving( pTarget, pEntry,
                                    rpNewParent, rNewChildPos, true );
}

TriState ExtTreeListBox::NotifyCopying( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos )
{
    return NotifyCopyingMoving( pTarget, pEntry,
                                    rpNewParent, rNewChildPos, false );
}

void Shell::CopyDialogResources(
    Reference< io::XInputStreamProvider >& io_xISP,
    ScriptDocument const& rSourceDoc,
    OUString const& rSourceLibName,
    ScriptDocument const& rDestDoc,
    OUString const& rDestLibName,
    OUString const& rDlgName
)
{
    if ( !io_xISP.is() )
        return;

    // Get StringResourceManager
    Reference< container::XNameContainer > xSourceDialogLib( rSourceDoc.getLibrary( E_DIALOGS, rSourceLibName, true ) );
    Reference< XStringResourceManager > xSourceMgr =
        LocalizationMgr::getStringResourceFromDialogLibrary( xSourceDialogLib );
    if( !xSourceMgr.is() )
        return;
    bool bSourceLocalized = ( xSourceMgr->getLocales().getLength() > 0 );

    Reference< container::XNameContainer > xDestDialogLib( rDestDoc.getLibrary( E_DIALOGS, rDestLibName, true ) );
    Reference< XStringResourceManager > xDestMgr =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDestDialogLib );
    if( !xDestMgr.is() )
        return;
    bool bDestLocalized = ( xDestMgr->getLocales().getLength() > 0 );

    if( !bSourceLocalized && !bDestLocalized )
        return;

    // create dialog model
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< container::XNameContainer > xDialogModel( xContext->getServiceManager()->createInstanceWithContext
        ( "com.sun.star.awt.UnoControlDialogModel", xContext ), UNO_QUERY );
    Reference< io::XInputStream > xInput( io_xISP->createInputStream() );
    ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rSourceDoc.isDocument() ? rSourceDoc.getDocument() : Reference< frame::XModel >() );

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
        io_xISP = ::xmlscript::exportDialogModel( xDialogModel, xContext, rDestDoc.isDocument() ? rDestDoc.getDocument() : Reference< frame::XModel >() );
    }
}

TriState ExtTreeListBox::NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry* pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, bool bMove )
{
    (void)pEntry;
    DBG_ASSERT( pEntry, "Kein Eintrag?" );  // ASS is ok here, should not be reached
    DBG_ASSERT( pTarget, "Kein Ziel?" );    // with NULL (right at the beginning)
    sal_uInt16 nDepth = GetModel()->GetDepth( pTarget );
    DBG_ASSERT( nDepth, "Tiefe?" );
    if ( nDepth == 1 )
    {
        // Target = Basic => put module/dialog under the Basic
        rpNewParent = pTarget;
        rNewChildPos = 0;
    }
    else if ( nDepth >= 2 )
    {
        // Target = module/dialog => put module/dialog under the superordinate Basic
        rpNewParent = GetParent( pTarget );
        rNewChildPos = SvTreeList::GetRelPos( pTarget ) + 1;
    }

    // get target shell and target library name
    EntryDescriptor aDestDesc = GetEntryDescriptor(rpNewParent);
    const ScriptDocument& rDestDoc( aDestDesc.GetDocument() );
    OUString aDestLibName( aDestDesc.GetLibName() );

    // get source shell, library name and module/dialog name
    EntryDescriptor aSourceDesc = GetEntryDescriptor(FirstSelected());
    const ScriptDocument rSourceDoc( aSourceDesc.GetDocument() );
    OUString aSourceLibName( aSourceDesc.GetLibName() );
    OUString aSourceName( aSourceDesc.GetName() );
    EntryType eType = aSourceDesc.GetType();

    // get dispatcher
    SfxDispatcher* pDispatcher = GetDispatcher();

    if ( bMove )    // move
    {
        // remove source module/dialog window
        if ( rSourceDoc != rDestDoc || aSourceLibName != aDestLibName )
        {
            if( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rSourceDoc, aSourceLibName, aSourceName, ConvertType( eType ) );
                pDispatcher->ExecuteList( SID_BASICIDE_SBXDELETED,
                      SfxCallMode::SYNCHRON, { &aSbxItem });
            }
        }

        try
        {
            if ( eType == OBJ_TYPE_MODULE ) // module
            {
                // get module
                OUString aModule;
                if ( rSourceDoc.getModule( aSourceLibName, aSourceName, aModule ) )
                {
                    // remove module from source library
                    if ( rSourceDoc.removeModule( aSourceLibName, aSourceName ) )
                    {
                        MarkDocumentModified( rSourceDoc );

                        // insert module into target library
                        if ( rDestDoc.insertModule( aDestLibName, aSourceName, aModule ) )
                            MarkDocumentModified( rDestDoc );
                    }
                }
            }
            else if ( eType == OBJ_TYPE_DIALOG )    // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP;
                if ( rSourceDoc.getDialog( aSourceLibName, aSourceName, xISP ) )
                {
                    Shell::CopyDialogResources( xISP, rSourceDoc,
                        aSourceLibName, rDestDoc, aDestLibName, aSourceName );

                    // remove dialog from source library
                    if (RemoveDialog(rSourceDoc, aSourceLibName, aSourceName))
                    {
                        MarkDocumentModified(rSourceDoc);

                        // insert dialog into target library
                        if ( rDestDoc.insertDialog( aDestLibName, aSourceName, xISP ) )
                            MarkDocumentModified(rDestDoc);
                    }
                }
            }
        }
        catch (const uno::Exception& )
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
                OUString aModule;
                if ( rSourceDoc.getModule( aSourceLibName, aSourceName, aModule ) )
                {
                    // insert module into target library
                    if ( rDestDoc.insertModule( aDestLibName, aSourceName, aModule ) )
                        MarkDocumentModified( rDestDoc );
                }
            }
            else if ( eType == OBJ_TYPE_DIALOG )    // dialog
            {
                // get dialog
                Reference< io::XInputStreamProvider > xISP;
                if ( rSourceDoc.getDialog( aSourceLibName, aSourceName, xISP ) )
                {
                    Shell::CopyDialogResources( xISP, rSourceDoc,
                        aSourceLibName, rDestDoc, aDestLibName, aSourceName );

                    // insert dialog into target library
                    if ( rDestDoc.insertDialog( aDestLibName, aSourceName, xISP ) )
                        MarkDocumentModified( rDestDoc );
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
            pDispatcher->ExecuteList( SID_BASICIDE_SBXINSERTED,
                                  SfxCallMode::SYNCHRON, { &aSbxItem });
        }
    }

    return TRISTATE_INDET;   // open...
}

// OrganizeDialog
OrganizeDialog::OrganizeDialog(vcl::Window* pParent, sal_Int16 tabId,
    EntryDescriptor& rDesc )
    : TabDialog( pParent, "OrganizeDialog",
        "modules/BasicIDE/ui/organizedialog.ui" )
    , m_aCurEntry( rDesc )
{
    get(m_pTabCtrl, "tabcontrol");

    m_pTabCtrl->SetActivatePageHdl(LINK(this, OrganizeDialog, ActivatePageHdl));

    if( tabId == 0 )
    {
        m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId("modules"));
    }
    else if ( tabId == 1 )
    {
        m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId("dialogs"));
    }
    else
    {
        m_pTabCtrl->SetCurPageId(m_pTabCtrl->GetPageId("libraries"));
    }

    ActivatePageHdl(m_pTabCtrl);

    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
}

OrganizeDialog::~OrganizeDialog()
{
    disposeOnce();
}

void OrganizeDialog::dispose()
{
    if (m_pTabCtrl)
    {
        for ( sal_uInt16 i = 0; i < m_pTabCtrl->GetPageCount(); i++ )
            VclPtr<vcl::Window>(m_pTabCtrl->GetTabPage( m_pTabCtrl->GetPageId( i ) )).disposeAndClear();
    }
    m_pTabCtrl.clear();

    TabDialog::dispose();
};

short OrganizeDialog::Execute()
{
    return TabDialog::Execute();
}

IMPL_LINK_TYPED( OrganizeDialog, ActivatePageHdl, TabControl *, pTabCtrl, void )
{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();

    if ( !pTabCtrl->GetTabPage( nId ) )
    {
        OString sPageName(pTabCtrl->GetPageName(nId));
        VclPtr<TabPage> pNewTabPage;
        if (sPageName == "modules")
        {
            VclPtrInstance<ObjectPage> pObjectPage(pTabCtrl, "ModulePage", BROWSEMODE_MODULES);
            pNewTabPage.reset(pObjectPage);
            pObjectPage->SetTabDlg(this);
            pObjectPage->SetCurrentEntry(m_aCurEntry);
        }
        else if (sPageName == "dialogs")
        {
            VclPtrInstance<ObjectPage> pObjectPage( pTabCtrl, "DialogPage", BROWSEMODE_DIALOGS );
            pNewTabPage.reset(pObjectPage);
            pObjectPage->SetTabDlg(this);
            pObjectPage->SetCurrentEntry(m_aCurEntry);
        }
        else if (sPageName == "libraries")
        {
            VclPtrInstance<LibPage> pLibPage( pTabCtrl );
            pNewTabPage.reset(pLibPage);
            pLibPage->SetTabDlg( this );
        }
        else
        {
            OSL_FAIL( "PageHdl: Unbekannte ID!" );
        }
        DBG_ASSERT( pNewTabPage, "Keine Page!" );
        pTabCtrl->SetTabPage( nId, pNewTabPage );
    }
}


// ObjectPage


ObjectPage::ObjectPage(vcl::Window *pParent, const OString &rName, sal_uInt16 nMode)
    : TabPage(pParent, rName, "modules/BasicIDE/ui/" +
        OStringToOUString(rName, RTL_TEXTENCODING_UTF8).toAsciiLowerCase() +
        ".ui")
{
    get(m_pBasicBox, "library");
    Size aSize(m_pBasicBox->LogicToPixel(Size(130, 117), MAP_APPFONT));
    m_pBasicBox->set_height_request(aSize.Height());
    m_pBasicBox->set_width_request(aSize.Width());
    get(m_pEditButton, "edit");
    get(m_pNewModButton, "newmodule");
    get(m_pNewDlgButton, "newdialog");
    get(m_pDelButton, "delete");

    pTabDlg = nullptr;

    m_pEditButton->SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    m_pDelButton->SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
    m_pBasicBox->SetSelectHdl( LINK( this, ObjectPage, BasicBoxHighlightHdl ) );

    if( nMode & BROWSEMODE_MODULES )
    {
        m_pNewModButton->SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
        m_pNewDlgButton->Hide();
    }
    else if ( nMode & BROWSEMODE_DIALOGS )
    {
        m_pNewDlgButton->SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
        m_pNewModButton->Hide();
    }

    m_pBasicBox->SetDragDropMode( DragDropMode::CTRL_MOVE | DragDropMode::CTRL_COPY );
    m_pBasicBox->EnableInplaceEditing(true);
    m_pBasicBox->SetMode( nMode );
    m_pBasicBox->SetStyle( WB_BORDER | WB_TABSTOP |
                        WB_HASLINES | WB_HASLINESATROOT |
                        WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                        WB_HSCROLL );
    m_pBasicBox->ScanAllEntries();

    m_pEditButton->GrabFocus();
    CheckButtons();
}

ObjectPage::~ObjectPage()
{
    disposeOnce();
}

void ObjectPage::dispose()
{
    m_pBasicBox.clear();
    m_pEditButton.clear();
    m_pNewModButton.clear();
    m_pNewDlgButton.clear();
    m_pDelButton.clear();
    pTabDlg.clear();
    TabPage::dispose();
}

void ObjectPage::SetCurrentEntry (EntryDescriptor& rDesc)
{
    m_pBasicBox->SetCurrentEntry( rDesc );
}

void ObjectPage::ActivatePage()
{
    m_pBasicBox->UpdateEntries();
}

void ObjectPage::DeactivatePage()
{
}

void ObjectPage::CheckButtons()
{
    // enable/disable edit button
    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
    ScriptDocument aDocument( aDesc.GetDocument() );
    OUString aLibName( aDesc.GetLibName() );
    OUString aLibSubName( aDesc.GetLibSubName() );
    bool bVBAEnabled = aDocument.isInVBAMode();
    sal_uInt16 nMode = m_pBasicBox->GetMode();

    sal_uInt16 nDepth = pCurEntry ? m_pBasicBox->GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth >= 2 )
    {
        if( bVBAEnabled && ( nMode & BROWSEMODE_MODULES ) && ( nDepth == 2 ) )
            m_pEditButton->Disable();
        else
        m_pEditButton->Enable();
    }
    else
        m_pEditButton->Disable();

    // enable/disable new module/dialog buttons
    LibraryLocation eLocation( aDesc.GetLocation() );
    bool bReadOnly = false;
    if ( nDepth > 0 )
    {
        Reference< script::XLibraryContainer2 > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
        Reference< script::XLibraryContainer2 > xDlgLibContainer( aDocument.getLibraryContainer( E_DIALOGS ), UNO_QUERY );
        if ( ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && xModLibContainer->isLibraryReadOnly( aLibName ) ) ||
             ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && xDlgLibContainer->isLibraryReadOnly( aLibName ) ) )
        {
            bReadOnly = true;
        }
    }
    if ( bReadOnly || eLocation == LIBRARY_LOCATION_SHARE )
    {
        m_pNewModButton->Disable();
        m_pNewDlgButton->Disable();
    }
    else
    {
        m_pNewModButton->Enable();
        m_pNewDlgButton->Enable();
    }

    // enable/disable delete button
    if ( nDepth >= 2 && !bReadOnly && eLocation != LIBRARY_LOCATION_SHARE )
    {
        if( bVBAEnabled && ( nMode & BROWSEMODE_MODULES ) && ( ( nDepth == 2 ) || aLibSubName == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) ) )
            m_pDelButton->Disable();
        else
        m_pDelButton->Enable();
    }
    else
        m_pDelButton->Disable();
}

IMPL_LINK_TYPED( ObjectPage, BasicBoxHighlightHdl, SvTreeListBox*, pBox, void )
{
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return;

    CheckButtons();
}

IMPL_LINK_TYPED( ObjectPage, ButtonHdl, Button *, pButton, void )
{
    if (pButton == m_pEditButton)
    {
        SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
        SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
        SfxGetpApp()->ExecuteSlot( aRequest );

        SfxDispatcher* pDispatcher = GetDispatcher();
        SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
        DBG_ASSERT( pCurEntry, "Entry?!" );
        if ( m_pBasicBox->GetModel()->GetDepth( pCurEntry ) >= 2 )
        {
            EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
            if ( pDispatcher )
            {
                OUString aModName( aDesc.GetName() );
                // extract the module name from the string like "Sheet1 (Example1)"
                if( aDesc.GetLibSubName() == IDE_RESSTR(RID_STR_DOCUMENT_OBJECTS) )
                {
                    sal_Int32 nIndex = 0;
                    aModName = aModName.getToken( 0, ' ', nIndex );
                }
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(),
                                  aModName, TreeListBox::ConvertType( aDesc.GetType() ) );
                pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX,
                        SfxCallMode::SYNCHRON, { &aSbxItem });
            }
        }
        else    // Nur Lib selektiert
        {
            DBG_ASSERT( m_pBasicBox->GetModel()->GetDepth( pCurEntry ) == 1, "Kein LibEntry?!" );
            ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
            SvTreeListEntry* pParentEntry = m_pBasicBox->GetParent( pCurEntry );
            if ( pParentEntry )
            {
                DocumentEntry* pDocumentEntry = static_cast<DocumentEntry*>(pParentEntry->GetUserData());
                if (pDocumentEntry)
                    aDocument = pDocumentEntry->GetDocument();
            }
            SfxUsrAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, makeAny( aDocument.getDocumentOrNull() ) );
            OUString aLibName( m_pBasicBox->GetEntryText( pCurEntry ) );
            SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
            if ( pDispatcher )
            {
                pDispatcher->ExecuteList(SID_BASICIDE_LIBSELECTED,
                    SfxCallMode::ASYNCHRON, { &aDocItem, &aLibNameItem });
            }
        }
        EndTabDialog();
    }
    else if (pButton == m_pNewModButton)
        NewModule();
    else if (pButton == m_pNewDlgButton)
        NewDialog();
    else if (pButton == m_pDelButton)
        DeleteCurrent();
}

bool ObjectPage::GetSelection( ScriptDocument& rDocument, OUString& rLibName )
{
    bool bRet = false;

    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    EntryDescriptor aDesc = m_pBasicBox->GetEntryDescriptor(pCurEntry);
    rDocument = aDesc.GetDocument();
    rLibName = aDesc.GetLibName();
    if ( rLibName.isEmpty() )
        rLibName = "Standard" ;

    DBG_ASSERT( rDocument.isAlive(), "ObjectPage::GetSelection: no or dead ScriptDocument in the selection!" );
    if ( !rDocument.isAlive() )
        return false;

    // check if the module library is loaded
    bool bOK = true;
    OUString aLibName( rLibName );
    Reference< script::XLibraryContainer > xModLibContainer( rDocument.getLibraryContainer( E_SCRIPTS  ) );
    if ( xModLibContainer.is() && xModLibContainer->hasByName( aLibName ) && !xModLibContainer->isLibraryLoaded( aLibName ) )
    {
        // check password
        Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
        if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aLibName ) && !xPasswd->isLibraryPasswordVerified( aLibName ) )
        {
            OUString aPassword;
            bOK = QueryPassword( xModLibContainer, rLibName, aPassword );
        }

        // load library
        if ( bOK )
            xModLibContainer->loadLibrary( aLibName );
    }

    // check if the dialog library is loaded
    Reference< script::XLibraryContainer > xDlgLibContainer( rDocument.getLibraryContainer( E_DIALOGS ) );
    if ( xDlgLibContainer.is() && xDlgLibContainer->hasByName( aLibName ) && !xDlgLibContainer->isLibraryLoaded( aLibName ) )
    {
        // load library
        if ( bOK )
            xDlgLibContainer->loadLibrary( aLibName );
    }

    if ( bOK )
        bRet = true;

    return bRet;
}

void ObjectPage::NewModule()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    OUString aLibName;

    if ( GetSelection( aDocument, aLibName ) )
    {
        OUString aModName;
        createModImpl( static_cast<vcl::Window*>( this ), aDocument,
                    *m_pBasicBox, aLibName, aModName, true );
    }
}

void ObjectPage::NewDialog()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    OUString aLibName;

    if ( GetSelection( aDocument, aLibName ) )
    {
        aDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

        ScopedVclPtrInstance< NewObjectDialog > aNewDlg(this, ObjectMode::Dialog, true);
        aNewDlg->SetObjectName( aDocument.createObjectName( E_DIALOGS, aLibName ) );

        if (aNewDlg->Execute() != 0)
        {
            OUString aDlgName = aNewDlg->GetObjectName();
            if (aDlgName.isEmpty())
                aDlgName = aDocument.createObjectName( E_DIALOGS, aLibName);

            if ( aDocument.hasDialog( aLibName, aDlgName ) )
            {
                ScopedVclPtrInstance<MessageDialog>::Create(this, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED2))->Execute();
            }
            else
            {
                Reference< io::XInputStreamProvider > xISP;
                if ( !aDocument.createDialog( aLibName, aDlgName, xISP ) )
                    return;

                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, aDlgName, TYPE_DIALOG );
                if (SfxDispatcher* pDispatcher = GetDispatcher())
                {
                    pDispatcher->ExecuteList( SID_BASICIDE_SBXINSERTED,
                        SfxCallMode::SYNCHRON, { &aSbxItem });
                }
                LibraryLocation eLocation = aDocument.getLibraryLocation( aLibName );
                SvTreeListEntry* pRootEntry = m_pBasicBox->FindRootEntry( aDocument, eLocation );
                if ( pRootEntry )
                {
                    if ( !m_pBasicBox->IsExpanded( pRootEntry ) )
                        m_pBasicBox->Expand( pRootEntry );
                    SvTreeListEntry* pLibEntry = m_pBasicBox->FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
                    DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
                    if ( pLibEntry )
                    {
                        if ( !m_pBasicBox->IsExpanded( pLibEntry ) )
                            m_pBasicBox->Expand( pLibEntry );
                        SvTreeListEntry* pEntry = m_pBasicBox->FindEntry( pLibEntry, aDlgName, OBJ_TYPE_DIALOG );
                        if ( !pEntry )
                        {
                            pEntry = m_pBasicBox->AddEntry(
                                aDlgName,
                                Image( IDEResId( RID_IMG_DIALOG ) ),
                                pLibEntry, false,
                                o3tl::make_unique<Entry>(OBJ_TYPE_DIALOG));
                            DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                        }
                        m_pBasicBox->SetCurEntry( pEntry );
                        m_pBasicBox->Select( m_pBasicBox->GetCurEntry() );        // OV-Bug?!
                    }
                }
            }
        }
    }
}

void ObjectPage::DeleteCurrent()
{
    SvTreeListEntry* pCurEntry = m_pBasicBox->GetCurEntry();
    DBG_ASSERT( pCurEntry, "Kein aktueller Eintrag!" );
    EntryDescriptor aDesc( m_pBasicBox->GetEntryDescriptor( pCurEntry ) );
    ScriptDocument aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isAlive(), "ObjectPage::DeleteCurrent: no document!" );
    if ( !aDocument.isAlive() )
        return;
    OUString aLibName( aDesc.GetLibName() );
    OUString aName( aDesc.GetName() );
    EntryType eType = aDesc.GetType();

    if ( ( eType == OBJ_TYPE_MODULE && QueryDelModule( aName, this ) ) ||
         ( eType == OBJ_TYPE_DIALOG && QueryDelDialog( aName, this ) ) )
    {
        m_pBasicBox->GetModel()->Remove( pCurEntry );
        if ( m_pBasicBox->GetCurEntry() )  // OV-Bug ?
            m_pBasicBox->Select( m_pBasicBox->GetCurEntry() );
        if (SfxDispatcher* pDispatcher = GetDispatcher())
        {
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, aName, TreeListBox::ConvertType( eType ) );
            pDispatcher->ExecuteList( SID_BASICIDE_SBXDELETED,
                                  SfxCallMode::SYNCHRON, { &aSbxItem });
        }

        try
        {
            bool bSuccess = false;
            if ( eType == OBJ_TYPE_MODULE )
                bSuccess = aDocument.removeModule( aLibName, aName );
            else if ( eType == OBJ_TYPE_DIALOG )
                bSuccess = RemoveDialog( aDocument, aLibName, aName );

            if ( bSuccess )
                MarkDocumentModified( aDocument );
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}


void ObjectPage::EndTabDialog()
{
    DBG_ASSERT( pTabDlg, "TabDlg nicht gesetzt!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( 1 );
}

LibDialog::LibDialog( vcl::Window* pParent )
    : ModalDialog(pParent, "ImportLibDialog", "modules/BasicIDE/ui/importlibdialog.ui")
{
    get(m_pStorageFrame, "storageframe");
    get(m_pReferenceBox, "ref");
    get(m_pReplaceBox, "replace");
    get(m_pLibBox, "entries");
    m_pLibBox->set_height_request(m_pLibBox->GetTextHeight() * 8);
    m_pLibBox->set_width_request(m_pLibBox->approximate_char_width() * 32);
}

LibDialog::~LibDialog()
{
    disposeOnce();
}

void LibDialog::dispose()
{
    m_pStorageFrame.clear();
    m_pLibBox.clear();
    m_pReferenceBox.clear();
    m_pReplaceBox.clear();
    ModalDialog::dispose();
}


void LibDialog::SetStorageName( const OUString& rName )
{
    OUString aName = IDE_RESSTR(RID_STR_FILENAME) + rName;
    m_pStorageFrame->set_label(aName);
}

// Helper function
SbModule* createModImpl( vcl::Window* pWin, const ScriptDocument& rDocument,
    TreeListBox& rBasicBox, const OUString& rLibName, const OUString& _aModName, bool bMain )
{
    OSL_ENSURE( rDocument.isAlive(), "createModImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return nullptr;

    SbModule* pModule = nullptr;

    OUString aLibName( rLibName );
    if ( aLibName.isEmpty() )
        aLibName = "Standard" ;
    rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );
    OUString aModName = _aModName;
    if ( aModName.isEmpty() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    ScopedVclPtrInstance< NewObjectDialog > aNewDlg(pWin, ObjectMode::Module, true);
    aNewDlg->SetObjectName( aModName );

    if (aNewDlg->Execute() != 0)
    {
        if (!aNewDlg->GetObjectName().isEmpty() )
            aModName = aNewDlg->GetObjectName();

        try
        {
            OUString sModuleCode;
            // the module has existed
            if( rDocument.hasModule( aLibName, aModName ) )
                return nullptr;
            rDocument.createModule( aLibName, aModName, bMain, sModuleCode );
            BasicManager* pBasMgr = rDocument.getBasicManager();
            StarBASIC* pBasic = pBasMgr? pBasMgr->GetLib( aLibName ) : nullptr;
                if ( pBasic )
                    pModule = pBasic->FindModule( aModName );
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDocument, aLibName, aModName, TYPE_MODULE );
            if (SfxDispatcher* pDispatcher = GetDispatcher())
            {
                pDispatcher->ExecuteList( SID_BASICIDE_SBXINSERTED,
                      SfxCallMode::SYNCHRON, { &aSbxItem });
            }
            LibraryLocation eLocation = rDocument.getLibraryLocation( aLibName );
            SvTreeListEntry* pRootEntry = rBasicBox.FindRootEntry( rDocument, eLocation );
            if ( pRootEntry )
            {
                if ( !rBasicBox.IsExpanded( pRootEntry ) )
                    rBasicBox.Expand( pRootEntry );
                SvTreeListEntry* pLibEntry = rBasicBox.FindEntry( pRootEntry, aLibName, OBJ_TYPE_LIBRARY );
                DBG_ASSERT( pLibEntry, "Libeintrag nicht gefunden!" );
                if ( pLibEntry )
                {
                    if ( !rBasicBox.IsExpanded( pLibEntry ) )
                        rBasicBox.Expand( pLibEntry );
                    SvTreeListEntry* pSubRootEntry = pLibEntry;
                    if( pBasic && rDocument.isInVBAMode() )
                    {
                        // add the new module in the "Modules" entry
                        SvTreeListEntry* pLibSubEntry = rBasicBox.FindEntry( pLibEntry, IDE_RESSTR(RID_STR_NORMAL_MODULES) , OBJ_TYPE_NORMAL_MODULES );
                        if( pLibSubEntry )
                        {
                            if( !rBasicBox.IsExpanded( pLibSubEntry ) )
                                rBasicBox.Expand( pLibSubEntry );
                            pSubRootEntry = pLibSubEntry;
                        }
                    }

                    SvTreeListEntry* pEntry = rBasicBox.FindEntry( pSubRootEntry, aModName, OBJ_TYPE_MODULE );
                    if ( !pEntry )
                    {
                        pEntry = rBasicBox.AddEntry(
                            aModName,
                            Image( IDEResId( RID_IMG_MODULE ) ),
                            pSubRootEntry, false,
                            o3tl::make_unique<Entry>(OBJ_TYPE_MODULE));
                        DBG_ASSERT( pEntry, "InsertEntry fehlgeschlagen!" );
                    }
                    rBasicBox.SetCurEntry( pEntry );
                    rBasicBox.Select( rBasicBox.GetCurEntry() );        // OV-Bug?!
                }
            }
        }
        catch (const container::ElementExistException& )
        {
            ScopedVclPtrInstance<MessageDialog>::Create(pWin, IDE_RESSTR(RID_STR_SBXNAMEALLREADYUSED2))->Execute();
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return pModule;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
