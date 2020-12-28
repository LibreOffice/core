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
#include <iderid.hxx>
#include <bitmaps.hlst>

#include "moduldlg.hxx"
#include <localizationmgr.hxx>
#include <basidesh.hxx>
#include <basobj.hxx>

#include <basic/basmgr.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/script/XLibraryContainer2.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <vcl/transfer.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <xmlscript/xmldlg_imexp.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::resource;

IMPL_LINK(ObjectPage, EditingEntryHdl, const weld::TreeIter&, rEntry, bool)
{
    bool bRet = false;

    sal_uInt16 nDepth = m_xBasicBox->get_iter_depth(rEntry);
    if (nDepth >= 2)
    {
        EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(&rEntry);
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

    return bRet;
}

IMPL_LINK(ObjectPage, EditedEntryHdl, const IterString&, rIterString, bool)
{
    const weld::TreeIter& rEntry = rIterString.first;
    OUString sNewText = rIterString.second;

    if ( !IsValidSbxName(sNewText) )
    {
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_pDialog->getDialog(),
                                                    VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
        xError->run();
        return false;
    }

    OUString aCurText(m_xBasicBox->get_text(rEntry));
    if ( aCurText == sNewText )
        // nothing to do
        return true;

    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(&rEntry);
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isValid(), "ExtTreeListBox::EditedEntry: no document!" );
    if ( !aDocument.isValid() )
        return false;
    const OUString& aLibName( aDesc.GetLibName() );
    EntryType eType = aDesc.GetType();

    bool bSuccess = eType == OBJ_TYPE_MODULE ?
        RenameModule(m_pDialog->getDialog(), aDocument, aLibName, aCurText, sNewText) :
        RenameDialog(m_pDialog->getDialog(), aDocument, aLibName, aCurText, sNewText);

    if ( !bSuccess )
        return false;

    MarkDocumentModified( aDocument );

    if (SfxDispatcher* pDispatcher = GetDispatcher())
    {
        SbxItem aSbxItem(SID_BASICIDE_ARG_SBX, aDocument, aLibName, sNewText, SbTreeListBox::ConvertType(eType));
        pDispatcher->ExecuteList( SID_BASICIDE_SBXRENAMED,
                            SfxCallMode::SYNCHRON, { &aSbxItem });
    }

    // OV-Bug?!
    m_xBasicBox->set_text(rEntry, sNewText);
    m_xBasicBox->set_cursor(rEntry);
    m_xBasicBox->unselect(rEntry);
    m_xBasicBox->select(rEntry); // so that handler is called => update edit

    return true;
}

void Shell::CopyDialogResources(
    Reference< io::XInputStreamProvider >& io_xISP,
    ScriptDocument const& rSourceDoc,
    OUString const& rSourceLibName,
    ScriptDocument const& rDestDoc,
    OUString const& rDestLibName,
    std::u16string_view rDlgName
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
    bool bSourceLocalized = xSourceMgr->getLocales().hasElements();

    Reference< container::XNameContainer > xDestDialogLib( rDestDoc.getLibrary( E_DIALOGS, rDestLibName, true ) );
    Reference< XStringResourceManager > xDestMgr =
        LocalizationMgr::getStringResourceFromDialogLibrary( xDestDialogLib );
    if( !xDestMgr.is() )
        return;
    bool bDestLocalized = xDestMgr->getLocales().hasElements();

    if( !bSourceLocalized && !bDestLocalized )
        return;

    // create dialog model
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
    Reference< container::XNameContainer > xDialogModel( xContext->getServiceManager()->createInstanceWithContext
        ( "com.sun.star.awt.UnoControlDialogModel", xContext ), UNO_QUERY );
    Reference< io::XInputStream > xInput( io_xISP->createInputStream() );
    ::xmlscript::importDialogModel( xInput, xDialogModel, xContext, rSourceDoc.isDocument() ? rSourceDoc.getDocument() : Reference< frame::XModel >() );

    if( !xDialogModel.is() )
        return;

    if( bSourceLocalized && bDestLocalized )
    {
        LocalizationMgr::copyResourceForDroppedDialog( xDialogModel, rDlgName, xDestMgr, xSourceMgr );
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

// OrganizeDialog
OrganizeDialog::OrganizeDialog(weld::Window* pParent, sal_Int16 tabId )
    : GenericDialogController(pParent, "modules/BasicIDE/ui/organizedialog.ui", "OrganizeDialog")
    , m_xTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
    , m_xModulePage(new ObjectPage(m_xTabCtrl->get_page("modules"), "ModulePage", BrowseMode::Modules, this))
    , m_xDialogPage(new ObjectPage(m_xTabCtrl->get_page("dialogs"), "DialogPage", BrowseMode::Dialogs, this))
    , m_xLibPage(new LibPage(m_xTabCtrl->get_page("libraries"), this))
{
    m_xTabCtrl->connect_enter_page(LINK(this, OrganizeDialog, ActivatePageHdl));

    OString sPage;
    if (tabId == 0)
        sPage = "modules";
    else if (tabId == 1)
        sPage = "dialogs";
    else
        sPage = "libraries";
    m_xTabCtrl->set_current_page(sPage);
    ActivatePageHdl(sPage);

    if (SfxDispatcher* pDispatcher = GetDispatcher())
        pDispatcher->Execute( SID_BASICIDE_STOREALLMODULESOURCES );
}

IMPL_LINK(OrganizeDialog, ActivatePageHdl, const OString&, rPage, void)
{
    if (rPage == "modules")
        m_xModulePage->ActivatePage();
    else if (rPage == "dialogs")
        m_xDialogPage->ActivatePage();
    else if (rPage == "libraries")
        m_xLibPage->ActivatePage();
}

OrganizeDialog::~OrganizeDialog()
{
}

OrganizePage::OrganizePage(weld::Container* pParent, const OUString& rUIFile, const OString &rName, OrganizeDialog* pDialog)
    : m_pDialog(pDialog)
    , m_xBuilder(Application::CreateBuilder(pParent, rUIFile))
    , m_xContainer(m_xBuilder->weld_container(rName))
{
}

OrganizePage::~OrganizePage()
{
}

class SbTreeListBoxDropTarget : public DropTargetHelper
{
private:
    SbTreeListBox& m_rTreeView;

    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override
    {
        // to enable the autoscroll when we're close to the edges
        weld::TreeView& rWidget = m_rTreeView.get_widget();
        rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);

        weld::TreeView* pSource = rWidget.get_drag_source();
        if (!pSource)
            return DND_ACTION_NONE;

        sal_Int8 nMode = DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xEntry(pSource->make_iterator());
        if (pSource->get_selected(xEntry.get()))
        {
            sal_uInt16 nDepth = pSource->get_iter_depth(*xEntry);
            if (nDepth >= 2)
            {
                nMode = DND_ACTION_COPY;
                EntryDescriptor aDesc = m_rTreeView.GetEntryDescriptor(xEntry.get());
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
                        nMode |= DND_ACTION_MOVE;
                }
            }
        }
        return nMode;
    }

    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
    {
        weld::TreeView& rWidget = m_rTreeView.get_widget();
        weld::TreeView* pSource = rWidget.get_drag_source();
        if (!pSource)
            return DND_ACTION_NONE;

        std::unique_ptr<weld::TreeIter> xEntry(rWidget.make_iterator());
        bool bEntry = rWidget.get_dest_row_at_pos(rEvt.maPosPixel, xEntry.get(), true);

        // don't drop on a BasicManager (nDepth == 0)
        sal_uInt16 nDepth = bEntry ? m_rTreeView.get_iter_depth(*xEntry) : 0;
        bool bValid = nDepth != 0;
        // don't drop in the same library
        std::unique_ptr<weld::TreeIter> xSelected(pSource->make_iterator());
        bool bSelected = pSource->get_selected(xSelected.get());
        if (!bSelected)
            bValid = false;
        else if (nDepth == 1)
        {
            std::unique_ptr<weld::TreeIter> xSelParent(pSource->make_iterator(xSelected.get()));
            if (pSource->iter_parent(*xSelParent) && pSource->iter_compare(*xEntry, *xSelParent) == 0)
                bValid = false;
        }
        else if (nDepth == 2)
        {
            std::unique_ptr<weld::TreeIter> xParent(pSource->make_iterator(xEntry.get()));
            std::unique_ptr<weld::TreeIter> xSelParent(pSource->make_iterator(xSelected.get()));
            if (pSource->iter_parent(*xParent) && pSource->iter_parent(*xSelParent) && pSource->iter_compare(*xParent, *xSelParent) == 0)
                bValid = false;
        }

        // don't drop on a library, which is not loaded, readonly or password protected
        // or which already has a module/dialog with this name
        if ( bValid && ( nDepth > 0 ) )
        {
            // get source module/dialog name
            EntryDescriptor aSourceDesc = m_rTreeView.GetEntryDescriptor(xSelected.get());
            const OUString& aSourceName = aSourceDesc.GetName();
            EntryType eSourceType = aSourceDesc.GetType();

            // get target shell and target library name
            EntryDescriptor aDestDesc = m_rTreeView.GetEntryDescriptor(xEntry.get());
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

        if (bValid)
            NotifyCopyingMoving(*xEntry, rEvt.mnAction & DND_ACTION_MOVE);

        return DND_ACTION_NONE;
    }

    void NotifyCopyingMoving(weld::TreeIter& rTarget, bool bMove)
    {
        sal_uInt16 nDepth = m_rTreeView.get_iter_depth(rTarget);
        std::unique_ptr<weld::TreeIter> xNewParent(m_rTreeView.make_iterator(&rTarget));
        int nNewChildPos = 0;
        DBG_ASSERT( nDepth, "Depth?" );
        if ( nDepth >= 2 )
        {
            // Target = module/dialog => put module/dialog under the superordinate Basic
            m_rTreeView.iter_parent(*xNewParent);
            nNewChildPos = m_rTreeView.get_iter_index_in_parent(rTarget) + 1;
        }

        // get target shell and target library name
        EntryDescriptor aDestDesc = m_rTreeView.GetEntryDescriptor(xNewParent.get());
        const ScriptDocument& rDestDoc( aDestDesc.GetDocument() );
        const OUString& aDestLibName( aDestDesc.GetLibName() );

        // get source shell, library name and module/dialog name
        std::unique_ptr<weld::TreeIter> xSelected(m_rTreeView.make_iterator());
        if (!m_rTreeView.get_selected(xSelected.get()))
            xSelected.reset();
        EntryDescriptor aSourceDesc = m_rTreeView.GetEntryDescriptor(xSelected.get());
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
                    SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rSourceDoc, aSourceLibName, aSourceName, SbTreeListBox::ConvertType(eType) );
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

        OUString sText(m_rTreeView.get_text(*xSelected));
        OUString sId(m_rTreeView.get_id(*xSelected));
        std::unique_ptr<weld::TreeIter> xRet(m_rTreeView.make_iterator());
        m_rTreeView.get_widget().insert(xNewParent.get(), nNewChildPos, &sText, &sId, nullptr, nullptr, false, xRet.get());
        if (eType == OBJ_TYPE_MODULE)
            m_rTreeView.get_widget().set_image(*xRet, RID_BMP_MODULE);
        else if (eType == OBJ_TYPE_DIALOG)
            m_rTreeView.get_widget().set_image(*xRet, RID_BMP_DIALOG);
        if (!m_rTreeView.get_row_expanded(*xNewParent))
            m_rTreeView.expand_row(*xNewParent);
        m_rTreeView.select(*xRet);

        if (bMove)
            m_rTreeView.remove(*xSelected);

        // create target module/dialog window
        if ( rSourceDoc != rDestDoc || aSourceLibName != aDestLibName )
        {
            if( pDispatcher )
            {
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, rDestDoc, aDestLibName, aSourceName, SbTreeListBox::ConvertType(eType) );
                pDispatcher->ExecuteList( SID_BASICIDE_SBXINSERTED,
                                      SfxCallMode::SYNCHRON, { &aSbxItem });
            }
        }
    }

public:
    SbTreeListBoxDropTarget(SbTreeListBox& rTreeView)
        : DropTargetHelper(rTreeView.get_widget().get_drop_target())
        , m_rTreeView(rTreeView)
    {
    }
};

// ObjectPage
ObjectPage::ObjectPage(weld::Container* pParent, const OString &rName, BrowseMode nMode, OrganizeDialog* pDialog)
    : OrganizePage(pParent, "modules/BasicIDE/ui/" + OStringToOUString(rName, RTL_TEXTENCODING_UTF8).toAsciiLowerCase() + ".ui",
        rName, pDialog)
    , m_xBasicBox(new SbTreeListBox(m_xBuilder->weld_tree_view("library"), pDialog->getDialog()))
    , m_xEditButton(m_xBuilder->weld_button("edit"))
    , m_xNewModButton(m_xBuilder->weld_button("newmodule"))
    , m_xNewDlgButton(m_xBuilder->weld_button("newdialog"))
    , m_xDelButton(m_xBuilder->weld_button("delete"))
{
    Size aSize(m_xBasicBox->get_approximate_digit_width() * 40,
               m_xBasicBox->get_height_rows(14));
    m_xBasicBox->set_size_request(aSize.Width(), aSize.Height());

    // tdf#93476 The dialogs should be listed alphabetically
    m_xBasicBox->make_sorted();

    m_xEditButton->connect_clicked( LINK( this, ObjectPage, ButtonHdl ) );
    m_xDelButton->connect_clicked( LINK( this, ObjectPage, ButtonHdl ) );
    m_xBasicBox->connect_changed( LINK( this, ObjectPage, BasicBoxHighlightHdl ) );

    if( nMode & BrowseMode::Modules )
    {
        m_xNewModButton->connect_clicked( LINK( this, ObjectPage, ButtonHdl ) );
        m_xNewDlgButton->hide();
    }
    else if ( nMode & BrowseMode::Dialogs )
    {
        m_xNewDlgButton->connect_clicked( LINK( this, ObjectPage, ButtonHdl ) );
        m_xNewModButton->hide();
    }

    m_xDropTarget.reset(new SbTreeListBoxDropTarget(*m_xBasicBox));

    m_xBasicBox->connect_editing(LINK(this, ObjectPage, EditingEntryHdl),
                                 LINK(this, ObjectPage, EditedEntryHdl));

    m_xBasicBox->SetMode( nMode );
    m_xBasicBox->ScanAllEntries();

    m_xEditButton->grab_focus();
    CheckButtons();
}

ObjectPage::~ObjectPage()
{
}

void ObjectPage::ActivatePage()
{
    m_xBasicBox->UpdateEntries();
    CheckButtons();
}

void ObjectPage::CheckButtons()
{
    // enable/disable edit button
    std::unique_ptr<weld::TreeIter> xCurEntry(m_xBasicBox->make_iterator());
    if (!m_xBasicBox->get_cursor(xCurEntry.get()))
        xCurEntry.reset();
    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(xCurEntry.get());
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aLibSubName( aDesc.GetLibSubName() );
    bool bVBAEnabled = aDocument.isInVBAMode();
    BrowseMode nMode = m_xBasicBox->GetMode();

    sal_uInt16 nDepth = xCurEntry ? m_xBasicBox->get_iter_depth(*xCurEntry) : 0;
    if ( nDepth >= 2 )
    {
        if( bVBAEnabled && ( nMode & BrowseMode::Modules ) && ( nDepth == 2 ) )
            m_xEditButton->set_sensitive(false);
        else
            m_xEditButton->set_sensitive(true);
    }
    else
        m_xEditButton->set_sensitive(false);

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
        m_xNewModButton->set_sensitive(false);
        m_xNewDlgButton->set_sensitive(false);
    }
    else
    {
        m_xNewModButton->set_sensitive(true);
        m_xNewDlgButton->set_sensitive(true);
    }

    // enable/disable delete button
    if ( nDepth >= 2 && !bReadOnly && eLocation != LIBRARY_LOCATION_SHARE )
    {
        if( bVBAEnabled && ( nMode & BrowseMode::Modules ) && ( ( nDepth == 2 ) || aLibSubName == IDEResId(RID_STR_DOCUMENT_OBJECTS) ) )
            m_xDelButton->set_sensitive(false);
        else
            m_xDelButton->set_sensitive(true);
    }
    else
        m_xDelButton->set_sensitive(false);
}

IMPL_LINK_NOARG(ObjectPage, BasicBoxHighlightHdl, weld::TreeView&, void)
{
    CheckButtons();
}

IMPL_LINK(ObjectPage, ButtonHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xEditButton.get())
    {
        SfxAllItemSet aArgs( SfxGetpApp()->GetPool() );
        SfxRequest aRequest( SID_BASICIDE_APPEAR, SfxCallMode::SYNCHRON, aArgs );
        SfxGetpApp()->ExecuteSlot( aRequest );

        SfxDispatcher* pDispatcher = GetDispatcher();

        std::unique_ptr<weld::TreeIter> xCurEntry(m_xBasicBox->make_iterator());
        if (!m_xBasicBox->get_cursor(xCurEntry.get()))
            return;
        if (m_xBasicBox->get_iter_depth(*xCurEntry) >= 2)
        {
            EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(xCurEntry.get());
            if ( pDispatcher )
            {
                OUString aModName( aDesc.GetName() );
                // extract the module name from the string like "Sheet1 (Example1)"
                if( aDesc.GetLibSubName() == IDEResId(RID_STR_DOCUMENT_OBJECTS) )
                {
                    aModName = aModName.getToken( 0, ' ' );
                }
                SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(),
                                  aModName, SbTreeListBox::ConvertType( aDesc.GetType() ) );
                pDispatcher->ExecuteList(SID_BASICIDE_SHOWSBX,
                        SfxCallMode::SYNCHRON, { &aSbxItem });
            }
        }
        else    // only Lib selected
        {
            DBG_ASSERT( m_xBasicBox->get_iter_depth(*xCurEntry) == 1, "No LibEntry?!" );
            ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
            std::unique_ptr<weld::TreeIter> xParentEntry(m_xBasicBox->make_iterator(xCurEntry.get()));
            if (m_xBasicBox->iter_parent(*xParentEntry))
            {
                DocumentEntry* pDocumentEntry = reinterpret_cast<DocumentEntry*>(m_xBasicBox->get_id(*xParentEntry).toInt64());
                if (pDocumentEntry)
                    aDocument = pDocumentEntry->GetDocument();
            }
            SfxUnoAnyItem aDocItem( SID_BASICIDE_ARG_DOCUMENT_MODEL, Any( aDocument.getDocumentOrNull() ) );
            OUString aLibName(m_xBasicBox->get_text(*xCurEntry));
            SfxStringItem aLibNameItem( SID_BASICIDE_ARG_LIBNAME, aLibName );
            if ( pDispatcher )
            {
                pDispatcher->ExecuteList(SID_BASICIDE_LIBSELECTED,
                    SfxCallMode::ASYNCHRON, { &aDocItem, &aLibNameItem });
            }
        }
        EndTabDialog();
    }
    else if (&rButton == m_xNewModButton.get())
        NewModule();
    else if (&rButton == m_xNewDlgButton.get())
        NewDialog();
    else if (&rButton == m_xDelButton.get())
        DeleteCurrent();
}

bool ObjectPage::GetSelection( ScriptDocument& rDocument, OUString& rLibName )
{
    bool bRet = false;

    std::unique_ptr<weld::TreeIter> xCurEntry(m_xBasicBox->make_iterator());
    if (!m_xBasicBox->get_cursor(xCurEntry.get()))
        xCurEntry.reset();
    EntryDescriptor aDesc = m_xBasicBox->GetEntryDescriptor(xCurEntry.get());
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
            bOK = QueryPassword(m_pDialog->getDialog(), xModLibContainer, rLibName, aPassword);
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
        createModImpl(m_pDialog->getDialog(), aDocument,
                      *m_xBasicBox, aLibName, OUString(), true);
    }
}

void ObjectPage::NewDialog()
{
    ScriptDocument aDocument( ScriptDocument::getApplicationScriptDocument() );
    OUString aLibName;

    if ( !GetSelection( aDocument, aLibName ) )
        return;

    aDocument.getOrCreateLibrary( E_DIALOGS, aLibName );

    NewObjectDialog aNewDlg(m_pDialog->getDialog(), ObjectMode::Dialog, true);
    aNewDlg.SetObjectName(aDocument.createObjectName(E_DIALOGS, aLibName));

    if (aNewDlg.run() == RET_CANCEL)
        return;

    OUString aDlgName = aNewDlg.GetObjectName();
    if (aDlgName.isEmpty())
        aDlgName = aDocument.createObjectName( E_DIALOGS, aLibName);

    if ( aDocument.hasDialog( aLibName, aDlgName ) )
    {
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(m_pDialog->getDialog(),
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
        std::unique_ptr<weld::TreeIter> xIter(m_xBasicBox->make_iterator());
        bool bRootEntry = m_xBasicBox->FindRootEntry(aDocument, eLocation, *xIter);
        if (bRootEntry)
        {
            if (!m_xBasicBox->get_row_expanded(*xIter))
                m_xBasicBox->expand_row(*xIter);
            bool bLibEntry = m_xBasicBox->FindEntry(aLibName, OBJ_TYPE_LIBRARY, *xIter);
            DBG_ASSERT( bLibEntry, "LibEntry not found!" );
            if (bLibEntry)
            {
                if (!m_xBasicBox->get_row_expanded(*xIter))
                    m_xBasicBox->expand_row(*xIter);
                std::unique_ptr<weld::TreeIter> xSubRootEntry(m_xBasicBox->make_iterator(xIter.get()));
                bool bDlgEntry = m_xBasicBox->FindEntry(aDlgName, OBJ_TYPE_DIALOG, *xIter);
                if (!bDlgEntry)
                {
                    m_xBasicBox->AddEntry(aDlgName, RID_BMP_DIALOG, xSubRootEntry.get(), false,
                                       std::make_unique<Entry>(OBJ_TYPE_DIALOG), xIter.get());
                    assert(xIter && "Insert entry failed!");
                }
                m_xBasicBox->set_cursor(*xIter);
                m_xBasicBox->select(*xIter);
            }
        }
    }
}

void ObjectPage::DeleteCurrent()
{
    std::unique_ptr<weld::TreeIter> xCurEntry(m_xBasicBox->make_iterator());
    if (!m_xBasicBox->get_cursor(xCurEntry.get()))
        xCurEntry.reset();
    DBG_ASSERT( xCurEntry, "No current entry!" );
    EntryDescriptor aDesc( m_xBasicBox->GetEntryDescriptor( xCurEntry.get() ) );
    const ScriptDocument& aDocument( aDesc.GetDocument() );
    DBG_ASSERT( aDocument.isAlive(), "ObjectPage::DeleteCurrent: no document!" );
    if ( !aDocument.isAlive() )
        return;
    const OUString& aLibName( aDesc.GetLibName() );
    const OUString& aName( aDesc.GetName() );
    EntryType eType = aDesc.GetType();

    if ( !(( eType == OBJ_TYPE_MODULE && QueryDelModule(aName, m_pDialog->getDialog()) ) ||
         ( eType == OBJ_TYPE_DIALOG && QueryDelDialog(aName, m_pDialog->getDialog()) )) )
        return;

    m_xBasicBox->remove(*xCurEntry);
    if (m_xBasicBox->get_cursor(xCurEntry.get()))
        m_xBasicBox->select(*xCurEntry);
    if (SfxDispatcher* pDispatcher = GetDispatcher())
    {
        SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDocument, aLibName, aName, SbTreeListBox::ConvertType( eType ) );
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

void ObjectPage::EndTabDialog()
{
    m_pDialog->response(RET_OK);
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
    m_xLibBox->enable_toggle_buttons(weld::ColumnToggleType::Check);
    // tdf#93476 The libraries should be listed alphabetically
    m_xLibBox->make_sorted();
}

LibDialog::~LibDialog()
{
}

void LibDialog::SetStorageName( std::u16string_view rName )
{
    OUString aName = IDEResId(RID_STR_FILENAME) + rName;
    m_xStorageFrame->set_label(aName);
}

// Helper function
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
                        rBasicBox.AddEntry(aModName, RID_BMP_MODULE, xSubRootEntry.get(), false,
                                           std::make_unique<Entry>(OBJ_TYPE_MODULE), xEntry.get());
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
