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

#include <strings.hrc>
#include <bitmaps.hlst>

#include "moduldlg.hxx"
#include <localizationmgr.hxx>
#include <basidesh.hxx>

#include <basic/basmgr.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/weld.hxx>
#include <tools/diagnose_ex.h>
#include <xmlscript/xmldlg_imexp.hxx>
#include <vcl/treelistentry.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

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

VCL_BUILDER_FACTORY_CONSTRUCTOR(ExtTreeListBox, WB_TABSTOP)

bool ExtTreeListBox::EditingEntry( SvTreeListEntry* pEntry, Selection& )
{
    bool bRet = false;

    if ( pEntry )
    {
        sal_uInt16 nDepth = GetModel()->GetDepth( pEntry );
        if ( nDepth >= 2 )
        {
            EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
            const ScriptDocument& aDocument( aDesc.GetDocument() );
            const OUString& aLibName( aDesc.GetLibName() );
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
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(GetFrameWeld(),
                                                    VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
        xError->run();
        return false;
    }

    OUString aCurText( GetEntryText( pEntry ) );
    if ( aCurText == rNewText )
        // nothing to do
        return true;

    EntryDescriptor aDesc = GetEntryDescriptor(pEntry);
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "ExtTreeListBox::EditedEntry: no document!" );
    if ( !aDocument.isValid() )
        return false;
    const OUString& aLibName( aDesc.GetLibName() );
    EntryType eType = aDesc.GetType();

    bool bSuccess = eType == OBJ_TYPE_MODULE ?
        RenameModule(GetFrameWeld(), aDocument, aLibName, aCurText, rNewText) :
        RenameDialog(GetFrameWeld(), aDocument, aLibName, aCurText, rNewText);

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
            const ScriptDocument& aDocument( aDesc.GetDocument() );
            const OUString& aLibName( aDesc.GetLibName() );
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
    if (!pSelected)
        bValid = false;
    else if ( ( nDepth == 1 ) && ( pEntry == GetParent( pSelected ) ) )
        bValid = false;
    else if ( ( nDepth == 2 ) && ( GetParent( pEntry ) == GetParent( pSelected ) ) )
        bValid = false;

    // don't drop on a library, which is not loaded, readonly or password protected
    // or which already has a module/dialog with this name
    if ( bValid && ( nDepth > 0 ) )
    {
        // get source module/dialog name
        EntryDescriptor aSourceDesc = GetEntryDescriptor(pSelected);
        const OUString& aSourceName = aSourceDesc.GetName();
        EntryType eSourceType = aSourceDesc.GetType();

        // get target shell and target library name
        EntryDescriptor aDestDesc = GetEntryDescriptor(pEntry);
        ScriptDocument const& rDestDoc = aDestDesc.GetDocument();
        const OUString& aDestLibName = aDestDesc.GetLibName();

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

TriState ExtTreeListBox::NotifyCopyingMoving( SvTreeListEntry* pTarget, SvTreeListEntry const * pEntry,
                        SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos, bool bMove )
{
    DBG_ASSERT( pEntry, "No entry?" );   // ASS is ok here, should not be reached
    DBG_ASSERT( pTarget, "No target?" ); // with NULL (right at the beginning)
    sal_uInt16 nDepth = GetModel()->GetDepth( pTarget );
    DBG_ASSERT( nDepth, "Depth?" );
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
    const OUString& aDestLibName( aDestDesc.GetLibName() );

    // get source shell, library name and module/dialog name
    EntryDescriptor aSourceDesc = GetEntryDescriptor(FirstSelected());
    const ScriptDocument& rSourceDoc( aSourceDesc.GetDocument() );
    const OUString& aSourceLibName( aSourceDesc.GetLibName() );
    const OUString& aSourceName( aSourceDesc.GetName() );
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
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
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
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
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
    EntryDescriptor const & rDesc )
    : TabDialog( pParent, "OrganizeDialog",
        "modules/BasicIDE/ui/organizedialog.ui" )
    , m_aCurEntry( rDesc )
{
    get(m_pTabCtrl, "tabcontrol");

    sal_uInt16 nPageCount = m_pTabCtrl->GetPageCount();
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        sal_uInt16 nPageId = m_pTabCtrl->GetPageId(nPage);
        m_pTabCtrl->SetTabPage(nPageId, nullptr);
    }

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

IMPL_LINK( OrganizeDialog, ActivatePageHdl, TabControl *, pTabCtrl, void )
{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();

    if ( !pTabCtrl->GetTabPage( nId ) )
    {
        OString sPageName(pTabCtrl->GetPageName(nId));
        VclPtr<TabPage> pNewTabPage;
        if (sPageName == "modules")
        {
            VclPtrInstance<ObjectPage> pObjectPage(pTabCtrl, "ModulePage", BrowseMode::Modules);
            pNewTabPage.reset(pObjectPage);
            pObjectPage->SetTabDlg(this);
            pObjectPage->SetCurrentEntry(m_aCurEntry);
        }
        else if (sPageName == "dialogs")
        {
            VclPtrInstance<ObjectPage> pObjectPage( pTabCtrl, "DialogPage", BrowseMode::Dialogs );
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
            OSL_FAIL( "PageHdl: Unknown ID" );
        }
        DBG_ASSERT( pNewTabPage, "No page" );
        pTabCtrl->SetTabPage( nId, pNewTabPage );
    }
}


// ObjectPage


ObjectPage::ObjectPage(vcl::Window *pParent, const OString &rName, BrowseMode nMode)
    : TabPage(pParent, rName, "modules/BasicIDE/ui/" +
        OStringToOUString(rName, RTL_TEXTENCODING_UTF8).toAsciiLowerCase() +
        ".ui")
{
    get(m_pBasicBox, "library");
    Size aSize(m_pBasicBox->LogicToPixel(Size(130, 117), MapMode(MapUnit::MapAppFont)));
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

    if( nMode & BrowseMode::Modules )
    {
        m_pNewModButton->SetClickHdl( LINK( this, ObjectPage, ButtonHdl ) );
        m_pNewDlgButton->Hide();
    }
    else if ( nMode & BrowseMode::Dialogs )
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

void ObjectPage::SetCurrentEntry (EntryDescriptor const & rDesc)
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
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aLibSubName( aDesc.GetLibSubName() );
    bool bVBAEnabled = aDocument.isInVBAMode();
    BrowseMode nMode = m_pBasicBox->GetMode();

    sal_uInt16 nDepth = pCurEntry ? m_pBasicBox->GetModel()->GetDepth( pCurEntry ) : 0;
    if ( nDepth >= 2 )
    {
        if( bVBAEnabled && ( nMode & BrowseMode::Modules ) && ( nDepth == 2 ) )
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
        if( bVBAEnabled && ( nMode & BrowseMode::Modules ) && ( ( nDepth == 2 ) || aLibSubName == IDEResId(RID_STR_DOCUMENT_OBJECTS) ) )
            m_pDelButton->Disable();
        else
        m_pDelButton->Enable();
    }
    else
        m_pDelButton->Disable();
}

IMPL_LINK( ObjectPage, BasicBoxHighlightHdl, SvTreeListBox*, pBox, void )
{
    if ( !pBox->IsSelected( pBox->GetHdlEntry() ) )
        return;

    CheckButtons();
}

IMPL_LINK( ObjectPage, ButtonHdl, Button *, pButton, void )
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
                if( aDesc.GetLibSubName() == IDEResId(RID_STR_DOCUMENT_OBJECTS) )
                {
                    aModName = aModName.getToken( 0, ' ' );
                }
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(),
                                  aModName, TreeListBox::ConvertType( aDesc.GetType() ) );
                pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX,
                        SfxCallMode::SYNCHRON, { &aSbxItem });
            }
        }
        else    // only Lib selected
        {
            DBG_ASSERT( m_pBasicBox->GetModel()->GetDepth( pCurEntry ) == 1, "No LibEntry?!" );
            ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
            SvTreeListEntry* pParentEntry = m_pBasicBox->GetParent( pCurEntry );
            if ( pParentEntry )
            {
                DocumentEntry* pDocumentEntry = static_cast<DocumentEntry*>(pParentEntry->GetUserData());
                if (pDocumentEntry)
                    aDocument = pDocumentEntry->GetDocument();
            }
            SfxUnoAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( aDocument.getDocumentOrNull() ) );
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
        createModImpl(GetFrameWeld(), aDocument,
                      *m_pBasicBox, aLibName, true);
    }
}

void ObjectPage::NewDialog()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    OUString aLibName;

    if ( GetSelection( aDocument, aLibName ) )
    {
        aDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

        NewObjectDialog aNewDlg(GetFrameWeld(), ObjectMode::Dialog, true);
        aNewDlg.SetObjectName(aDocument.createObjectName(E_DIALOGS, aLibName));

        if (aNewDlg.run() != RET_CANCEL)
        {
            OUString aDlgName = aNewDlg.GetObjectName();
            if (aDlgName.isEmpty())
                aDlgName = aDocument.createObjectName( E_DIALOGS, aLibName);

            if ( aDocument.hasDialog( aLibName, aDlgName ) )
            {
                std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(GetFrameWeld(),
                                                            VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
                xError->run();
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
                    DBG_ASSERT( pLibEntry, "LibEntry not found!" );
                    if ( pLibEntry )
                    {
                        if ( !m_pBasicBox->IsExpanded( pLibEntry ) )
                            m_pBasicBox->Expand( pLibEntry );
                        SvTreeListEntry* pEntry = m_pBasicBox->FindEntry( pLibEntry, aDlgName, OBJ_TYPE_DIALOG );
                        if ( !pEntry )
                        {
                            pEntry = m_pBasicBox->AddEntry(
                                aDlgName,
                                Image(StockImage::Yes, RID_BMP_DIALOG),
                                pLibEntry, false,
                                std::make_unique<Entry>(OBJ_TYPE_DIALOG));
                            DBG_ASSERT( pEntry, "Insert entry failed!" );
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
    DBG_ASSERT( pCurEntry, "No current entry!" );
    EntryDescriptor aDesc( m_pBasicBox->GetEntryDescriptor( pCurEntry ) );
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isAlive(), "ObjectPage::DeleteCurrent: no document!" );
    if ( !aDocument.isAlive() )
        return;
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aName( aDesc.GetName() );
    EntryType eType = aDesc.GetType();

    if ( ( eType == OBJ_TYPE_MODULE && QueryDelModule(aName, GetFrameWeld()) ) ||
         ( eType == OBJ_TYPE_DIALOG && QueryDelDialog(aName, GetFrameWeld()) ) )
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
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }
    }
}

void ObjectPage::EndTabDialog()
{
    DBG_ASSERT( pTabDlg, "TabDlg not set!" );
    if ( pTabDlg )
        pTabDlg->EndDialog( 1 );
}

LibDialog::LibDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "modules/BasicIDE/ui/importlibdialog.ui", "ImportLibDialog")
    , m_xStorageFrame(m_xBuilder->weld_frame("storageframe"))
    , m_xLibBox(m_xBuilder->weld_tree_view("entries"))
    , m_xReferenceBox(m_xBuilder->weld_check_button("ref"))
    , m_xReplaceBox(m_xBuilder->weld_check_button("replace"))
{
    m_xLibBox->set_size_request(m_xLibBox->get_approximate_digit_width() * 28,
                                m_xLibBox->get_height_rows(8));
}

LibDialog::~LibDialog()
{
}

void LibDialog::SetStorageName( const OUString& rName )
{
    OUString aName = IDEResId(RID_STR_FILENAME) + rName;
    m_xStorageFrame->set_label(aName);
}

// Helper function
SbModule* createModImpl(weld::Window* pWin, const ScriptDocument& rDocument,
    TreeListBox& rBasicBox, const OUString& rLibName, bool bMain )
{
    OSL_ENSURE( rDocument.isAlive(), "createModImpl: invalid document!" );
    if ( !rDocument.isAlive() )
        return nullptr;

    SbModule* pModule = nullptr;

    OUString aLibName( rLibName );
    if ( aLibName.isEmpty() )
        aLibName = "Standard" ;
    rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );
    OUString aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    NewObjectDialog aNewDlg(pWin, ObjectMode::Module, true);
    aNewDlg.SetObjectName(aModName);

    if (aNewDlg.run() != RET_CANCEL)
    {
        if (!aNewDlg.GetObjectName().isEmpty())
            aModName = aNewDlg.GetObjectName();

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
                DBG_ASSERT( pLibEntry, "LibEntry not found!" );
                if ( pLibEntry )
                {
                    if ( !rBasicBox.IsExpanded( pLibEntry ) )
                        rBasicBox.Expand( pLibEntry );
                    SvTreeListEntry* pSubRootEntry = pLibEntry;
                    if( pBasic && rDocument.isInVBAMode() )
                    {
                        // add the new module in the "Modules" entry
                        SvTreeListEntry* pLibSubEntry = rBasicBox.FindEntry( pLibEntry, IDEResId(RID_STR_NORMAL_MODULES) , OBJ_TYPE_NORMAL_MODULES );
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
                            Image(StockImage::Yes, RID_BMP_MODULE),
                            pSubRootEntry, false,
                            std::make_unique<Entry>(OBJ_TYPE_MODULE));
                        DBG_ASSERT( pEntry, "Insert entry failed!" );
                    }
                    rBasicBox.SetCurEntry( pEntry );
                    rBasicBox.Select( rBasicBox.GetCurEntry() );        // OV-Bug?!
                }
            }
        }
        catch (const container::ElementExistException& )
        {
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(pWin,
                                                        VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
            xError->run();
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }
    }
    return pModule;
}

SbModule* createModImpl(weld::Window* pWin, const ScriptDocument& rDocument,
    SbTreeListBox& rBasicBox, const OUString& rLibName, const OUString& _aModName, bool bMain )
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

    NewObjectDialog aNewDlg(pWin, ObjectMode::Module, true);
    aNewDlg.SetObjectName(aModName);

    if (aNewDlg.run() != RET_CANCEL)
    {
        if (!aNewDlg.GetObjectName().isEmpty())
            aModName = aNewDlg.GetObjectName();

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
            std::unique_ptr<weld::TreeIter> xIter(rBasicBox.make_iterator());
            bool bRootEntry = rBasicBox.FindRootEntry(rDocument, eLocation, *xIter);
            if (bRootEntry)
            {
                if (!rBasicBox.get_row_expanded(*xIter))
                    rBasicBox.expand_row(*xIter);
                bool bLibEntry = rBasicBox.FindEntry(aLibName, OBJ_TYPE_LIBRARY, *xIter);
                DBG_ASSERT( bLibEntry, "LibEntry not found!" );
                if (bLibEntry)
                {
                    if (!rBasicBox.get_row_expanded(*xIter))
                        rBasicBox.expand_row(*xIter);
                    std::unique_ptr<weld::TreeIter> xSubRootEntry(rBasicBox.make_iterator(xIter.get()));
                    if (pBasic && rDocument.isInVBAMode())
                    {
                        // add the new module in the "Modules" entry
                        std::unique_ptr<weld::TreeIter> xLibSubEntry(rBasicBox.make_iterator(xIter.get()));
                        bool bLibSubEntry = rBasicBox.FindEntry(IDEResId(RID_STR_NORMAL_MODULES) , OBJ_TYPE_NORMAL_MODULES, *xLibSubEntry);
                        if (bLibSubEntry)
                        {
                            if (!rBasicBox.get_row_expanded(*xLibSubEntry))
                                rBasicBox.expand_row(*xLibSubEntry);
                            rBasicBox.copy_iterator(*xLibSubEntry, *xSubRootEntry);
                        }
                    }

                    std::unique_ptr<weld::TreeIter> xEntry(rBasicBox.make_iterator(xSubRootEntry.get()));
                    bool bEntry = rBasicBox.FindEntry(aModName, OBJ_TYPE_MODULE, *xEntry);
                    if (!bEntry)
                    {
                        rBasicBox.AddEntry(aModName, RID_BMP_MODULE, xEntry.get(), false,
                                           std::make_unique<Entry>(OBJ_TYPE_MODULE));
                    }
                    rBasicBox.set_cursor(*xEntry);
                    rBasicBox.select(*xEntry);
                }
            }
        }
        catch (const container::ElementExistException& )
        {
            std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(pWin,
                                                        VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
            xError->run();
        }
        catch (const container::NoSuchElementException& )
        {
            DBG_UNHANDLED_EXCEPTION("basctl.basicide");
        }
    }
    return pModule;
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
